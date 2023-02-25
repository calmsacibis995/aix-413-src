static char sccsid[] = "@(#)56	1.1  src/rspc/kernext/pci/kent/kent_dump.c, pcient, rspc41J, 9507B 1/25/95 11:26:24";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: kent_dump
 *		kent_dump_output
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "kent_proto.h"
#include <sys/errno.h>
#include <sys/lock_alloc.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/device.h>


/*
 * NAME:     kent_dump
 *
 * FUNCTION: provides access to the dump functions of the driver
 *
 * EXECUTION ENVIRONMENT:  Kernel process thread
 *
 *	This function can be called with interrupts disabled to
 *	the same level as our operation level, a less favored
 *	level, or wide open. We need to provide for the most secure
 *	thread by disabling interrupts of other threads to the most 
 *	protected case.
 *
 * NOTES: 
 *
 * RETURNS:  
 *
 * 	0	- Success
 *	ENOMEM 	- No mbufs were available
 *	ETIMEDOUT - Passed wait time with nothing to receive
 *	ENETDOWN - Indicates Network is in unrecoverable state
 */

int kent_dump( 
		ndd_t 	*p_ndd,
            	int 	cmd,
            	caddr_t arg)
{
	kent_acs_t		*p_acs = (kent_acs_t *)p_ndd->ndd_correlator;
	short			len, offset;
	int 			i,j,x;
	struct mbuf 		*p_tmp;
	struct dmp_query 	*p_query;
	kent_desc_t		*p_rx;
	struct dump_read	*p_read;
	struct timestruc_t  	current_time,   /* Keeps the current time */
                        	timeout_time,   /* Time out value         */
                        	temp_time;
	uchar			netid;
	extern			int kent_dump_output();
	int			stat1, stat2;
	char			*p_buf;
	int 			iomem;



	KENT_TRACE("dkdB",p_acs, cmd, arg);

	switch (cmd)
	{
	case DUMPINIT:
		break;
	
	case DUMPQUERY:
		p_query = (struct dmp_query *)arg;
		p_query->min_tsize = p_acs->ndd.ndd_mintu;
		p_query->max_tsize = p_acs->ndd.ndd_mtu;
		break;	

	case DUMPSTART:
		/* 
		 * The driver needs to remove any promiscuous/multicast modes
		 * and clear any multicast addresses from the adapter so that
		 * it doesn't receive too many extraneous packets that might
		 * prevent the dump from working.
		 */
		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_TRACE("dkd1",p_acs, cmd, p_acs->dev.state);
			return (ENETDOWN);
		}

		/*
		 * over lay the dump_output routine over the normal output
		 */
		p_acs->ndd.ndd_output = (int(*)())kent_dump_output;

		p_acs->ndd.ndd_flags &= ~(NDD_PROMISC|NDD_MULTICAST);

		/* 
		 * clear the transmit variables so that the adapter does not
		 * transmit any packets that might still be sitting on the 
		 * queue.
		 */
		p_acs->tx.nxt_tx = 0;
		p_acs->tx.nxt_cmplt = 0;
		p_acs->tx.in_use = 0;

		start_adapter(p_acs);

		/* 
		 * writing a 0 to csr0 will clear the IENA bit (and should
	 	 * prevent any interrupts (this shouldn't happen in a dump but
		 * seems to from time to time.
		 */
        	iomem = (int) iomem_att(&p_acs->dev.iomap);
        	CSR_WRITE(iomem, 0, 0);
        	iomem_det((void *)iomem);
		
		/* 
		 * make sure the driver owns all of the transmit descriptors.
		 */
		for (i=0; i<p_acs->dds.tx_que_sz; i++)
			p_acs->tx.desc[i].stat1 &= ~KENT_TX_STAT1_OWN;

		break;

	case DUMPEND:
		p_acs->dev.state = DEAD_STATE;
		break;

	case DUMPTERM:
		break;

	case DUMPREAD:

		if (p_acs->dev.state == DEAD_STATE)
		{
			KENT_TRACE("dkd2",p_acs, cmd, p_acs->dev.state);
			return(ENETDOWN);
		}

		p_read = (struct dump_read *)arg;

    		/*
     		 *  Set up the time information (get the current time)
     		 */
    		MS2TIME((p_read->wait_time*10), temp_time);
    		curtime(&current_time);
    		ntimeradd(current_time, temp_time, timeout_time);

		while (TRUE)
		{
			/* get the current descriptor's status fields */
			p_rx = &p_acs->rx.desc[p_acs->rx.nxt_rx];
			stat1 = SWAP_LONG(p_rx->stat1);
			stat2 = SWAP_LONG(p_rx->stat2);

			/*
			 * if the OWN bit is set
                 	 * there is nothing to receive yet
                 	 */
			if (!(stat1 & KENT_RX_STAT1_OWN))
			{
				/* 
				 * If the packet has not had an error, then
				 * receive the data.
				 */
 				if (!(stat1 & KENT_RX_STAT1_OFLO ||
					stat1 & KENT_RX_STAT1_CRC ||
                        		stat1 & KENT_RX_STAT1_BUFF))
				{

					len = KENT_BLEN_MASK & stat2;

					/* 
					 * Set up the length, removing the
					 * mac header from the data.
					 * this is done to simulate what
					 * the demuxer does before it passes
					 * a packet to a user.
					 */
					p_read->dump_bufread->m_next = NULL;
					p_read->dump_bufread->m_len =
						len - KENT_MAC_SZ;
					p_read->dump_bufread->m_data += 
						KENT_MAC_SZ;
                			p_read->dump_bufread->m_flags |= 
						M_PKTHDR;
					p_read->dump_bufread->m_pkthdr.len = 
						len-KENT_MAC_SZ;

					p_buf = 
						&p_acs->rx.p_buffer[
						(p_acs->rx.nxt_rx * 
						KENT_BUFF_SZ)];

					/*
					 * copy the data into the user
					 * provided mbuf (minus the 
					 * mac header).
					 */
					bcopy(p_buf+KENT_MAC_SZ,
						MTOD(p_read->dump_bufread,
							caddr_t),
						len-KENT_MAC_SZ);

					/* 
					 * rearm the descriptor and increment 
					 * the receive index.
					 */
					ARM_RX_DESC(p_rx->stat1, 
						p_rx->stat2);

					if (++p_acs->rx.nxt_rx == 
							p_acs->dds.rx_que_sz)
						p_acs->rx.nxt_rx = 0;

					break;
				}

				/* 
				 * there was a packet but not the right one,
				 * so re-arm the descriptor, increment the 
				 * receive descriptor and continue on.
				 */
				ARM_RX_DESC(p_rx->stat1, p_rx->stat2);

				if (++p_acs->rx.nxt_rx == p_acs->dds.rx_que_sz)
					p_acs->rx.nxt_rx = 0;

			}

            		/*
             		 *  Check to see if timeout has been reached.
             	 	 */
            		if (ntimercmp(current_time, timeout_time, >)) 
			{
				KENT_TRACE("dkd3",p_acs,p_acs->rx.nxt_rx, 
					stat1);

				return(ETIMEDOUT);
			}
                	curtime(&current_time);

		}

		break;
	};

	KENT_TRACE("dkdE",p_acs, 0, 0);
	return (0);
}

/*
 * NAME: kent_dump_output()
 *                                                                    
 * FUNCTION: Transmit entry point from kernel for the dump services.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine can be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 *		ENETDOWN	- adapter is dead transmits can't fail
 */  


int
kent_dump_output( 	ndd_t 		*p_ndd,
		struct mbuf	*p_mbuf)
{
	kent_acs_t	*p_acs = (kent_acs_t *)p_ndd->ndd_correlator;
	int 		rc = EAGAIN;
	int 		iomem,tmp;
	int		ipri;
	char *		p_buf = &p_acs->tx.p_buffer[(p_acs->tx.nxt_tx *
                                KENT_BUFF_SZ)];
        kent_desc_t     *p_desc = &p_acs->tx.desc[p_acs->tx.nxt_tx];
	struct mbuf	*p_tmp, *p_freelist = 0;
        struct timestruc_t      current_time,   /* Keeps the current time */
                                timeout_time,   /* Time out value         */
                                temp_time;
	int		stat1;

	KENT_TTRACE("dkoB",p_acs,p_acs->tx.in_use,p_acs->tx.nxt_tx);

	if (p_acs->dev.state != OPEN_STATE)
	{
		KENT_ETTRACE("dko1",p_acs, p_acs->dev.state,0);
		return (ENETDOWN);
	}

 	/*
         *  Set up time information
         */
        MS2TIME(KENT_TX_WDT_RESTART, temp_time);
        curtime(&current_time);
        ntimeradd(current_time, temp_time, timeout_time);

	stat1 = SWAP_LONG(p_desc->stat1);

	/* 
	 * wait until the adapter is done with the next descriptor (this should
	 * not ever delay the driver, hopefully the adapter will have sent the
	 * packet by the time the driver has looped the descriptor list).
	 */
	while (stat1 & KENT_TX_STAT1_OWN)
	{
		/*
                 *  Check to see if timeout has been reached.
                 */
                if (ntimercmp(current_time, timeout_time, >))
                {
                        KENT_ETTRACE("dko2",p_acs,p_acs->tx.nxt_tx,stat1);
			p_acs->dev.state = DEAD_STATE;
                        return (ENETDOWN);
                }
                curtime(&current_time);
		stat1 = SWAP_LONG(p_desc->stat1);
	}

	/* 
	 * copy the mbuf into the transmit buffer.  Need to use the m_len field
	 * instead of the pkthdr.len field, because the dump driver code 
	 * does not fill out the pkthdr.len field.
	 */
	bcopy(mtod(p_mbuf,char *), p_buf, p_mbuf->m_len);

	/*
	 * Set up the descriptor
	 * 	- clear stat2
	 *  	- set up stat1 and then byte swap it (done in two locations to
	 *		guarantee the own bit isn't set prior to the rest of
	 * 		the descriptor being setup
	 */
	p_desc->stat2 = 0;
	tmp = (~(p_mbuf->m_len)+1) & KENT_BLEN_MASK;
	tmp |= KENT_TX_STAT1_STP | KENT_TX_STAT1_ENP | KENT_TX_STAT1_OWN |
			KENT_TX_STAT1_ONES;
	p_desc->stat1 = SWAP_LONG(tmp);

	/* 
	 * tell the adapter to start transmitting (do not set the IENA bit as
	 * we do not want any interrupts happening ever again).
	 */
        iomem = (int) iomem_att(&p_acs->dev.iomap);
        CSR_WRITE(iomem, 0, KENT_CSR0_TDMD);
        iomem_det((void *)iomem);

	if (++p_acs->tx.nxt_tx == p_acs->dds.tx_que_sz)
		p_acs->tx.nxt_tx = 0;

	KENT_TTRACE("dkoE",p_acs,p_acs->tx.in_use, p_acs->tx.nxt_tx);
	return(0);
}
