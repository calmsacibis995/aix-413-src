static char sccsid[] = "@(#)66	1.7  src/rspc/kernext/pci/kent/kent_tx.c, pcient, rspc41J, 9515A_all 4/11/95 21:21:32";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: free_tx
 *		init_tx
 *		kent_output
 *		kent_tx_to
 *		send_packet
 *		tx_complete
 *		undo_tx
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
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/sleep.h>
/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: kent_output()
 *                                                                    
 * FUNCTION: Transmit entry point from kernel.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine can be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 *		EAGAIN		- no room on the transmit queue for any of
 *				  packets from the transmit call.
 *		ENETDOWN	- adapter is dead transmits can't fail
 *		EINVAL		- bad parameter or illegal call
 */  


int
kent_output( 	ndd_t 		*p_ndd,
		struct mbuf	*p_mbuf)
{
	kent_acs_t	*p_acs = (kent_acs_t *)p_ndd->ndd_correlator;
	int 		iomem;
	int		ipri;
	struct mbuf	*p_tmp, *p_freelist = 0;
	

	KENT_TTRACE("TkoB",p_acs,p_acs->tx.in_use,p_acs->tx.nxt_tx);
	 /*
         * Accounting for MIBs - ifHCOutUcastPkts, ifHCOutMulticastPkts
         * and ifHCOutBroadcastPkts.
         */
        p_tmp = p_mbuf;

        while (p_tmp)
        {
                if (p_tmp->m_flags & M_BCAST)
                {
                        if (++NDDSTATS.ndd_ifOutBcastPkts_lsw == 0)
                                NDDSTATS.ndd_ifOutBcastPkts_msw++;
                }
                else
                if (p_tmp->m_flags & M_MCAST)
                {

                       if (++NDDSTATS.ndd_ifOutMcastPkts_lsw == 0)
                                NDDSTATS.ndd_ifOutMcastPkts_msw++;
                }
                else
                {
                        if (++NDDSTATS.ndd_ifOutUcastPkts_lsw == 0)
                                NDDSTATS.ndd_ifOutUcastPkts_msw++;
                }
                p_tmp = p_tmp->m_nextpkt;
        }


	ipri = disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);

	/* 
	 * if the driver is in a power management mode of suspend/hibernate
	 * then reject the transmit (as if the driver was in limbo)
	 */
	if (p_acs->dev.state == SUSPEND_STATE)
	{
		KENT_ETTRACE("Tko1",p_acs, p_acs->dev.state,0);
		unlock_enable(ipri, &p_acs->tx.lock);
		return (ENETDOWN);
	}


	if (p_acs->dev.state != OPEN_STATE)
	{
		KENT_ETTRACE("Tko2",p_acs, p_acs->dev.state,0);
		unlock_enable(ipri, &p_acs->tx.lock);
		return (ENETDOWN);
	}

	tx_complete(p_acs);

	if (p_acs->tx.in_use == p_acs->dds.tx_que_sz)
	{
		while (p_mbuf)
		{
			NDDSTATS.ndd_xmitque_ovf++;
			p_mbuf = p_mbuf->m_nextpkt;
		}
		KENT_TTRACE("TkoE",EAGAIN,p_acs->tx.in_use,p_acs->tx.nxt_tx);
		unlock_enable(ipri,&p_acs->tx.lock);
		return(EAGAIN);
	}

	while (p_mbuf)
	{
		if (p_acs->tx.in_use < p_acs->dds.tx_que_sz)
		{
			send_packet(p_acs,p_mbuf);

			p_tmp = p_mbuf;
			p_mbuf = p_mbuf->m_nextpkt;
			p_tmp->m_nextpkt = p_freelist;
			p_freelist = p_tmp;
		}
		else
		{
			while (p_mbuf)
			{
				NDDSTATS.ndd_xmitque_ovf++;
				NDDSTATS.ndd_opackets_drop++;

				p_tmp = p_mbuf;
				p_mbuf = p_mbuf->m_nextpkt;
				p_tmp->m_nextpkt = p_freelist;
				p_freelist = p_tmp;
			}
		}
	}

	iomem = (int) iomem_att(&p_acs->dev.iomap);
	CSR_WRITE(iomem, 0, KENT_CSR0_TDMD|KENT_CSR0_IENA);
	iomem_det((void *)iomem);

	w_start(&(p_acs->tx.wdt));

	unlock_enable(ipri, &p_acs->tx.lock);

	if (p_freelist)
		m_freem(p_freelist);

	/* 
	 * Do not check for a pio error in this routine, it could result in a 
	 * deadlock if the transmit routine was called on the interrupt thread 
	 * and then tried to get the slih lock.
	 */

	KENT_TTRACE("TkoE",0,p_acs->tx.in_use,p_acs->tx.nxt_tx);
	return(0);
} /* end kent_output() */


/*
 * NAME: send_packet()
 *                                                                    
 * FUNCTION: put the packet sent on the next descriptor
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine will be called with interrupts disabled.  
 *	It assumes the transmit lock is held, and that there is an
 *	available descriptor.
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
send_packet( 	kent_acs_t 	*p_acs,
		struct mbuf	*p_mbuf)
{
	kent_desc_t 	*p_desc = &p_acs->tx.desc[p_acs->tx.nxt_tx];
	char		*p_buf = &p_acs->tx.p_buffer[(p_acs->tx.nxt_tx *
				KENT_BUFF_SZ)];
	int		tmp;

	/*
	 * Call ndd_trace if it is enabled
	 */
	if (p_acs->ndd.ndd_trace) 
	{
		(*(p_acs->ndd.ndd_trace))(&p_acs->ndd, p_mbuf, 
			p_mbuf->m_data, p_acs->ndd.ndd_trace_arg);
	}
	
	KENT_TTRACE("WQUE",p_acs->dds.md_sla, p_mbuf, p_mbuf->m_pkthdr.len);

	/* Copy the packet out to the descriptor's buffer */
	m_copydata(p_mbuf, 0, p_mbuf->m_pkthdr.len, p_buf);

	/*
	 * Set up the descriptor
	 * 	- clear stat2
	 *  	- set up stat1 and then byte swap it (done in two locations to
	 *		guarantee the own bit isn't set prior to the rest of
	 * 		the descriptor being setup
	 */
	p_desc->stat2 = 0;
	tmp = (~(p_mbuf->m_pkthdr.len)+1) & KENT_BLEN_MASK;
	tmp |= KENT_TX_STAT1_STP | KENT_TX_STAT1_ENP | KENT_TX_STAT1_OWN |
			KENT_TX_STAT1_ONES;
	p_desc->stat1 = SWAP_LONG(tmp);

	if (++p_acs->tx.in_use > NDDSTATS.ndd_xmitque_max)
		NDDSTATS.ndd_xmitque_max = p_acs->tx.in_use;
	
	if (++p_acs->tx.nxt_tx == p_acs->dds.tx_que_sz)
		p_acs->tx.nxt_tx = 0;

	KENT_TTRACE("WEND",p_acs->dds.md_sla, p_mbuf, p_mbuf->m_pkthdr.len);

	return;
}

/*
 * NAME: tx_complete()
 *                                                                    
 * FUNCTION: handle any completion code for a transmit descriptor after the
 *		transmit has completed
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine will be called with interrupts disabled.  
 *	It assumes the transmit lock is held, and that there is an
 *	available descriptor.
 *                                                                   
 * NOTES:
 *	For the most part this routine just handles the statistics.  It is 
 *	called from the kent_output routine and the tx wdt handler.
 *
 * RETURNS: 
 */  

void
tx_complete( 	kent_acs_t 	*p_acs)
{
	kent_desc_t 	*p_desc;
	char		*p_buf;
	int		len;
	int		stat1, stat2;

	KENT_TTRACE("TtcB",p_acs, p_acs->tx.in_use, p_acs->tx.nxt_cmplt);
	while (p_acs->tx.in_use > 0)
	{
		p_desc = &p_acs->tx.desc[p_acs->tx.nxt_cmplt];
		p_buf = &p_acs->tx.p_buffer[(p_acs->tx.nxt_cmplt *
				KENT_BUFF_SZ)];
		stat1 = SWAP_LONG(p_desc->stat1);
		stat2 = SWAP_LONG(p_desc->stat2);

		if (stat1 & KENT_TX_STAT1_OWN)
			break;

		if (stat1 & KENT_TX_STAT1_MORE)
			ENTSTATS.m_coll_frames++;
	
		if (stat1 & KENT_TX_STAT1_ONE)
			ENTSTATS.s_coll_frames++;
	
		if (stat2 & KENT_TX_STAT2_UFLO)
			ENTSTATS.underrun++;

		if (stat2 & KENT_TX_STAT2_RTRY)
			ENTSTATS.excess_collisions++;

		if (stat2 & KENT_TX_STAT2_LCOL)
			ENTSTATS.late_collisions++;
	
		if (stat2 & KENT_TX_STAT2_LCAR)
			ENTSTATS.carrier_sense++;
	
		if (stat1 & KENT_TX_STAT1_DEF)
			ENTSTATS.defer_tx++;
	
		if (stat2 & KENT_TX_STAT2_TRC)
			ADAPSTATS.coll_freq[((stat2 & KENT_TX_STAT2_TRC)-1)]++;

		if (stat1 & KENT_TX_STAT1_ERR)
		{
			NDDSTATS.ndd_oerrors++;
		}
		else
		{
			if (*p_buf & KENT_MCAST)
			{
				ENTSTATS.mcast_tx_ok++;
			}
			else if (KENT_IS_BCAST(p_buf))
			{
				ENTSTATS.bcast_tx_ok++;
			}
			
			if (++NDDSTATS.ndd_opackets_lsw == 0)
				NDDSTATS.ndd_opackets_msw++;

			len = KENT_BLEN_MASK & stat1;
			len =  KENT_BLEN_MASK & (~(len) + 1);
			NDDSTATS.ndd_obytes_lsw += len;
			if (NDDSTATS.ndd_obytes_lsw < len)
				NDDSTATS.ndd_obytes_msw++;

		}
		p_acs->tx.in_use--;
		if (++p_acs->tx.nxt_cmplt == p_acs->dds.tx_que_sz)
			p_acs->tx.nxt_cmplt = 0;

		p_desc->stat1 = SWAP_LONG(KENT_TX_STAT1_ONES);
		p_desc->stat2 = 0;

	}
	KENT_TTRACE("TtcE",p_acs, p_acs->tx.in_use, p_acs->tx.nxt_cmplt);
}

/*
 * NAME: init_tx()
 *                                                                    
 * FUNCTION: initialize the memory and services to setup the transmit section
 *		of the driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine cannot be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

int
init_tx( 	kent_acs_t 	*p_acs)
{
	int		l_desc, l_buffer;
	int		i,j;
	int		rc;
	char		*p_buf;

	KENT_TTRACE("TitB",p_acs,0,0);
	

	l_desc = p_acs->dds.tx_que_sz * sizeof(kent_desc_t);
	l_buffer = p_acs->dds.tx_que_sz * KENT_BUFF_SZ;

	p_acs->tx.desc = xmalloc(l_desc, DMA_L2PSIZE, pinned_heap);

	if (p_acs->tx.desc == NULL)
	{
		KENT_ETTRACE("Tit1",p_acs,0,0);
		return (ENOMEM);
	}

	p_acs->tx.p_buffer = xmalloc(l_buffer, DMA_L2PSIZE, pinned_heap);

	if (p_acs->tx.p_buffer == NULL)
	{
		KENT_ETTRACE("Tit2",p_acs,0,0);
		xmfree(p_acs->tx.desc, pinned_heap);
		return (ENOMEM);
	}

	/*
	 * The adapter requires that the tranmit descriptors be in contiguous
	 * memory.   The PCI dma services do not guarantee that mapped
	 * pages are contiguous, so the transmit descriptors must fit within
	 * a single page or the driver needs to use rmalloc.
	 */
	if ((rc = D_MAP_PAGE(p_acs->dev.dh,DMA_READ,p_acs->tx.desc, 
		&p_acs->tx.p_d_desc, xmem_global)) != DMA_SUCC)
        {
                KENT_ETTRACE("Tit3",rc, p_acs->tx.desc, p_acs->tx.p_d_desc);
		xmfree(p_acs->tx.desc,pinned_heap);
		xmfree(p_acs->tx.p_buffer, pinned_heap);
                return (ENOMEM);
        }

	bzero(p_acs->tx.desc, l_desc);
	
	for (i=0, p_buf = p_acs->tx.p_buffer; i<p_acs->dds.tx_que_sz; 
		i++, p_buf += KENT_BUFF_SZ)
	{
		if ((rc = D_MAP_PAGE(p_acs->dev.dh, DMA_READ, p_buf, 
			&p_acs->tx.desc[i].addr, xmem_global)) != DMA_SUCC)
		{
			KENT_ETTRACE("Tit4",rc, p_buf, 
				p_acs->tx.desc[i].addr);
			for (j=0; j<i; j++)
			{
				p_acs->tx.desc[j].addr = 
					SWAP_LONG(p_acs->tx.desc[j].addr);
				D_UNMAP_PAGE(p_acs->dev.dh, 
					(&p_acs->tx.desc[j].addr));
			}

			D_UNMAP_PAGE(p_acs->dev.dh,&p_acs->tx.p_d_desc);

			xmfree(p_acs->tx.desc, pinned_heap);
			xmfree(p_acs->tx.p_buffer, pinned_heap);
			return(ENOMEM);
		}
		p_acs->tx.desc[i].addr = SWAP_LONG(p_acs->tx.desc[i].addr);
		p_acs->tx.desc[i].stat1 = SWAP_LONG(KENT_TX_STAT1_ONES);
	}

	lock_alloc(&p_acs->tx.lock, LOCK_ALLOC_PIN, XKENT_TX_LOCK, 
		p_acs->dev.seq_number);
	simple_lock_init(&p_acs->tx.lock);

	p_acs->tx.wdt.restart = KENT_TX_WDT_RESTART; 
	p_acs->tx.wdt.func = kent_tx_to;
	while (w_init (&p_acs->tx.wdt));

	p_acs->tx.in_use = 0;
	p_acs->tx.nxt_tx = 0;
	p_acs->tx.nxt_cmplt = 0;
	
	KENT_TTRACE("TitE",p_acs,0,0);
	return(0);
}


/*
 * NAME: free_tx()
 *                                                                    
 * FUNCTION: free up the memory and services used by the transmit section
 *		of the driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine cannot be called with interrupts disabled.  
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
free_tx( 	kent_acs_t 	*p_acs)
{
	int		i,j;

	KENT_TTRACE("TftB",p_acs,0,0);
	
	w_stop(&(p_acs->tx.wdt));
	while (w_clear (&(p_acs->tx.wdt)));

	lock_free(&p_acs->tx.lock);

	D_UNMAP_PAGE(p_acs->dev.dh, &p_acs->tx.p_d_desc);

	for (i=0; i<p_acs->dds.tx_que_sz; i++)
	{
		SWAP_LONG(p_acs->tx.desc[i].addr);
		D_UNMAP_PAGE(p_acs->dev.dh,(&p_acs->tx.desc[i].addr));
	}

	xmfree(p_acs->tx.desc,pinned_heap);
	xmfree(p_acs->tx.p_buffer, pinned_heap);

	KENT_TTRACE("TftE",p_acs,0,0);
	return;
}

/*
 * NAME: undo_tx()
 *                                                                    
 * FUNCTION: clean up the transmit section in the event of a limbo condition
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
undo_tx(	kent_acs_t *p_acs)
{
	int i;

	KENT_TTRACE("TutB",p_acs,p_acs->tx.in_use,p_acs->tx.nxt_tx);

	NDDSTATS.ndd_opackets_drop += p_acs->tx.in_use;

	for (i=0; i<p_acs->dds.tx_que_sz; i++)
	{
		p_acs->tx.desc[i].stat1 = SWAP_LONG(KENT_TX_STAT1_ONES);
		p_acs->tx.desc[i].stat2 = 0;
	}

	p_acs->tx.in_use = 0;
	p_acs->tx.nxt_tx = 0;
	p_acs->tx.nxt_cmplt = 0;

	KENT_TTRACE("TutE",p_acs,NDDSTATS.ndd_opackets_drop,0);
	return;
}

/*
 * NAME: kent_tx_to()
 *                                                                    
 * FUNCTION: handle a transmit timeout.  This can happen anytime there isn't
 *	a user transmitting regularly.  It is called anytime there has been
 *	a transmit call and then none for KENT_TX_WDT_RESTART (10 secs). 
 *	The driver will try to complete at least some of the packets on the 
 *	transmit queue.  If none can be completed it will assume the adapter
 * 	has died and call limbo.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 */  

void
kent_tx_to(	struct  watchdog *p_wdt)
{
	kent_acs_t 	*p_acs;
	int		ipri;
	int		begin_in_use;

	p_acs = (kent_acs_t *) ((uint) p_wdt - 
		(offsetof (kent_tx_t, wdt) + offsetof (kent_acs_t, tx)));

	KENT_TTRACE("TktB",p_acs,p_acs->tx.in_use,p_acs->tx.nxt_cmplt);

	ipri = disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);

	if (p_acs->dev.state == SUSPEND_STATE || p_acs->tx.in_use == 0)
	{
		unlock_enable(ipri, &p_acs->tx.lock);
		return;
	}

	begin_in_use = p_acs->tx.in_use;

	tx_complete(p_acs);
	
	if (begin_in_use == p_acs->tx.in_use)
	{
		ENTSTATS.tx_timeouts++;
		kent_logerr(p_acs,ERRID_KENT_TX_ERR,__LINE__,__FILE__,
			p_acs->tx.in_use, p_acs->tx.nxt_tx,0);
		unlock_enable(ipri, &p_acs->tx.lock);
		ipri = disable_lock(KENT_OPLEVEL, &p_acs->dev.slih_lock);
		enter_limbo(p_acs);
		unlock_enable(ipri, &p_acs->dev.slih_lock);
		return;
	}

	if (p_acs->tx.in_use > 0)
		w_start(&(p_acs->tx.wdt));
	
	unlock_enable(ipri, &p_acs->tx.lock);

	KENT_TTRACE("TktE",p_acs,p_acs->tx.in_use,p_acs->tx.nxt_cmplt);
	return;
}



