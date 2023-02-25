static char sccsid[] = "@(#)43  1.1  src/bos/usr/samples/ndd/entsamp_intr.c, entsamp, bos411, 9428A410j 1/14/94 13:40:16";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_intr
 *		entsamp_rv_intr
 *		entsamp_tx_intr
 *		entsamp_exec_intr
 *
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 THE SOURCE CODE EXAMPLES PROVIDED BY IBM ARE ONLY INTENDED TO ASSIST IN THE
 DEVELOPMENT OF A WORKING SOFTWARE PROGRAM.  THE SOURCE CODE EXAMPLES DO NOT
 FUNCTION AS WRITTEN:  ADDITIONAL CODE IS REQUIRED.  IN ADDITION, THE SOURCE
 CODE EXAMPLES MAY NOT COMPILE AND/OR BIND SUCCESSFULLY AS WRITTEN.
 
 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 IBM does not warrant that the contents of the source code examples, whether
 individually or as one or more groups, will meet your requirements or that
 the source code examples are error-free.

 IBM may make improvements and/or changes in the source code examples at
 any time.

 Changes may be made periodically to the information in the source code
 examples; these changes may be reported, for the sample device drivers
 included herein, in new editions of the examples.

 References in the source code examples to IBM products, programs, or
 services do not imply that IBM intends to make these available in all
 countries in which IBM operates.  Any reference to an IBM licensed
 program in the source code examples is not intended to state or imply
 that only IBM's licensed program may be used.  Any functionally equivalent
 program may be used.

 * RISC System/6000 is a trademark of International Business Machines 
   Corporation.
 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/lock_def.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/intr.h>
#include <sys/timer.h>
#include <sys/watchdog.h>
#include <sys/dma.h>
#include <sys/malloc.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/iocc.h>
#include <sys/sleep.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>
#include <sys/ethernet_mibs.h>
#include <sys/cdli_entuser.h>
#include <net/spl.h>


#include "entsamp_mac.h"
#include "entsamp_pio.h"
#include "entsamp_hw.h"
#include "entsamp.h"
#include "entsamp_errids.h"

uchar ent_broad_adr[ENT_NADR_LENGTH] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void entsamp_rv_intr();
void entsamp_tx_intr();
void entsamp_exec_intr();

/*****************************************************************************/
/*
 * NAME:     entsamp_intr
 *
 * FUNCTION: Ethernet driver interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      The FLIH
 *
 * INPUT:
 *      p_ihs		- point to the interrupt structure.
 *
 * RETURNS:  
 *	INTR_SUCC - our interrupt
 *	INTR_FAIL - not our interrupt
 */
/*****************************************************************************/
entsamp_intr(
	struct intr *p_ihs)	/* This also points to device control area */

{
  	entsamp_dev_ctl_t	*p_dev_ctl = (entsamp_dev_ctl_t *)p_ihs;
	uchar status_reg;
	uchar command_reg;
	int ioa; 
	int bus;
	int rc;
	int ipri;
	int pio_rc = 0;


	
	TRACE_SYS(HKWD_ENTSAMP_OTHER, "SinB", (ulong)p_ihs, 0, 0);

	ipri = disable_lock(PL_IMP, &SLIH_LOCK);

	if (p_dev_ctl->device_state < LIMBO) {
			unlock_enable(ipri, &SLIH_LOCK);
			TRACE_SYS(HKWD_ENTSAMP_ERR, "Sin2", INTR_FAIL, 
				p_dev_ctl->device_state, 0);
			return(INTR_FAIL);
	}

	ioa = (int)BUSIO_ATT((ulong)DDS.bus_id, DDS.io_port);

	/*
	 * read the status register, if status register can't be read
	 * then assume not our interrupt
	 */
	ENT_GETCX((char *)(ioa + STATUS_REG), &status_reg);
	if (pio_rc) {
		BUSIO_DET(ioa);

		entsamp_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);

		unlock_enable(ipri, &SLIH_LOCK);
		TRACE_SYS(HKWD_ENTSAMP_ERR, "Sin3", INTR_FAIL, pio_rc, 0);
		return(INTR_FAIL);
	}

	TRACE_DBG(HKWD_ENTSAMP_OTHER, "Sin4", (ulong)status_reg, 0, 0);
	
	if (status_reg & CWR_MSK) {

		ENT_GETCX((char *)(ioa + COMMAND_REG), &command_reg);
		if (pio_rc) {
			BUSIO_DET(ioa);

			entsamp_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);

			unlock_enable(ipri, &SLIH_LOCK);
			TRACE_SYS(HKWD_ENTSAMP_ERR, "Sin5", INTR_FAIL, 
				pio_rc, 0);
			return(INTR_FAIL);
		}

		rc = INTR_SUCC;
		TRACE_DBG(HKWD_ENTSAMP_OTHER, "Sin6", (ulong)command_reg, 0, 0);

		/*
	 	* get addressability to adapter memory
	 	*/
		bus = (int)BUSMEM_ATT(DDS.bus_id, DDS.bus_mem_addr);

		if ((command_reg & RECEIVE_MSK) && 
			(p_dev_ctl->device_state == OPENED)) {
			if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX)
				NDD.ndd_genstats.ndd_recvintr_msw++;
			NDD.ndd_genstats.ndd_recvintr_lsw++;

			entsamp_rv_intr(p_dev_ctl, command_reg, bus, ioa);
		}

		if (command_reg & XMIT_MSK) {
			if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX)
				NDD.ndd_genstats.ndd_xmitintr_msw++;
			NDD.ndd_genstats.ndd_xmitintr_lsw++;
   			
			entsamp_tx_intr(p_dev_ctl, command_reg, bus);
		}

		if (command_reg & EXECUTE_MSK) {
			entsamp_exec_intr(p_dev_ctl, command_reg, bus);
		}

		BUSMEM_DET(bus);

	}
	else {		/* not our interrupt */
		rc = INTR_FAIL;
		TRACE_SYS(HKWD_ENTSAMP_ERR, "Sin7", INTR_FAIL, status_reg, 0);
	}
		

	BUSIO_DET(ioa);

	unlock_enable(ipri, &SLIH_LOCK);

	TRACE_SYS(HKWD_ENTSAMP_OTHER, "SinE", rc, 0, 0);
	return(rc);


}

/*****************************************************************************/
/*
 * NAME:     entsamp_rv_intr
 *
 * FUNCTION: Ethernet driver receive interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_intr
 *
 * INPUT:
 *      p_dev_ctl	- point to the device control area
 *	command_reg	- value from command register
 *	bus		- bus memory access handle 
 *	ioa		- io address access handle
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_rv_intr(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* point to the device control area */
  uchar		command_reg,	/* value in the command register */
  int 		bus,		/* bus memory access handel */
  int		ioa)		/* io address access handle */

{

	int rc;
	int pio_rc = 0;
	uchar bd_stat;
	entsamp_bdc_t *p_rd;
        int bufoff;
	short count;
	int broad, mcast;
	ndd_t	*p_ndd = (ndd_t *)&(NDD);
	struct mbuf  *p_mbuf;



	TRACE_SYS(HKWD_ENTSAMP_RECV, "SrvB", (ulong)p_dev_ctl, 
		(ulong)command_reg, 0);

	/*
         * Process the receive interrupt
	 * start processing the receive list
	 */
	if (command_reg == RX_P_RCVD || command_reg == RX_ERROR) {

	  while (TRUE) {

                bd_stat = 0;
                p_rd = WRK.rvd_first;
                bufoff = bus + p_rd->bd_off;
                broad = mcast = FALSE;
		/*
		 * Examine the receive buffer pool, if a received packet
		 * is found, get the status of the packets and continue
		 * to process the packet, otherwise, exit the while loop.
		 */
                ENT_GETCX((char *)(bufoff + offsetof(BUFDESC, status)),
                        &bd_stat);

                if (pio_rc) {
                        entsamp_hard_err(p_dev_ctl, TRUE, FALSE, NDD_PIO_FAIL);
                        TRACE_SYS(HKWD_ENTSAMP_ERR, "Srv1", pio_rc, 0, 0);
                        return;
                }

                /*
                 * exit if no more packets to process
                 */
                if (!(bd_stat & CMPLT_BIT_MASK))
                        break;

                /*
                 * Check dma status
                 */
                if ((rc = d_complete(WRK.dma_channel, DMA_READ|DMA_NOHIDE,
                        p_rd->buf, ENT_MAX_MTU, &WRK.rvbuf_xmem,
                        p_rd->dma_io)) != DMA_SUCC) {

                        entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_DMAFAIL,
                                __LINE__, __FILE__, WRK.dma_channel, p_rd->buf,
                                ENT_MAX_MTU);
                        entsamp_hard_err(p_dev_ctl, TRUE, FALSE, ENT_DMA_FAIL);
                        TRACE_SYS(HKWD_ENTSAMP_ERR, "Srv2", rc, 0, 0);
                        return;
                }

		/*
		 * Is this a good packet or bad packet? 
		 */
		
		if (!(bd_stat &= BD_ERR_MASK)) { /* no error, good packet */

			if (p_ndd->ndd_genstats.ndd_ipackets_lsw == ULONG_MAX)
			  p_ndd->ndd_genstats.ndd_ipackets_msw++;
			p_ndd->ndd_genstats.ndd_ipackets_lsw++;
			if ((ULONG_MAX - count) < 
			  p_ndd->ndd_genstats.ndd_ibytes_lsw)
			  p_ndd->ndd_genstats.ndd_ibytes_msw++;
			p_ndd->ndd_genstats.ndd_ibytes_lsw += count;

			/* check if broadcast or multicast */
			if (p_rd->buf[0] & MULTI_BIT_MASK) {
			  if (SAME_NADR(p_rd->buf, ent_broad_adr)) {
				broad = TRUE;
				MIB.Generic_mib.ifExtnsEntry.bcast_rx_ok++;
			  }
			  else {
				mcast = TRUE;
 				MIB.Generic_mib.ifExtnsEntry.mcast_rx_ok++;

			  }
			}

			if (count <= MHLEN) {
				p_mbuf = m_gethdr(M_DONTWAIT, MT_HEADER);
			}
                        else {
                                p_mbuf = m_getclust(M_DONTWAIT,MT_HEADER);
			}
			if (p_mbuf) {
				p_mbuf->m_len = count;
				p_mbuf->m_pkthdr.len = count;
				p_mbuf->m_flags |= M_PKTHDR;
				bcopy(p_rd->buf, MTOD(p_mbuf, char *), count);
				if (broad)
					p_mbuf->m_flags |= M_BCAST;
				if (mcast)
					p_mbuf->m_flags |= M_MCAST;

       		                (*(p_ndd->nd_receive))(p_ndd, p_mbuf);

			}
			else {
				p_ndd->ndd_genstats.ndd_nobufs++;
				p_ndd->ndd_genstats.ndd_ipackets_drop++;
			}

		}  /* if good packet */
		else {    /* this is a bad packet */

    			ndd_statblk_t  stat_blk;   /* new status block */


		/*
		* Got a bad frame,  put it in a mbuf,
		* create a status block and pass it to the demuxer.
		*/


			if (count <= MHLEN) {
                                  p_mbuf = m_gethdr(M_DONTWAIT, MT_HEADER);
			}
                        else {
                                  p_mbuf = m_getclust(M_DONTWAIT,MT_HEADER);
			}
			if (p_mbuf) {
				  p_mbuf->m_len = count;
				  p_mbuf->m_pkthdr.len = count;
				  p_mbuf->m_flags |= M_PKTHDR;
				  bcopy(p_rd->buf, MTOD(p_mbuf, char *), count);

				  bzero(&stat_blk, sizeof(ndd_statblk_t));
    				  stat_blk.code = NDD_BAD_PKTS;
    				  stat_blk.option[0] = (ulong)bd_stat;
    				  stat_blk.option[1] = (ulong)p_mbuf;

      				  (*(p_ndd->nd_status))(p_ndd, &stat_blk);

				  /* we own the mbuf, so we free it */
				  m_free(p_mbuf);   
			}
			else {
				  p_ndd->ndd_genstats.ndd_nobufs++;
			  	  p_ndd->ndd_genstats.ndd_ipackets_drop++;
			}


		}

		/*
		 * Invalidate processor cache.
		 * Note that only the range that was touched by
		 * the above bcopy needs to be invalidated.
		 */
		cache_inval(p_rd->buf, count);

	  }  /* while */


	}
  
	TRACE_SYS(HKWD_ENTSAMP_RECV, "SrvE", 0, 0, 0);


}

/*****************************************************************************/
/*
 * NAME:     entsamp_tx_intr
 *
 * FUNCTION: Ethernet driver transmit interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_intr
 *
 * INPUT:
 *      p_dev_ctl	- point to the device control area
 *	command_reg	- value from command register
 *	bus		- bus memory access handle
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_tx_intr(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* point to the device control area */
  uchar		command_reg,	/* value in the command register */
  int		bus)		/* bus memory access handle */

{
	
	int rc;
	int restart_timer = FALSE;
	entsamp_bdc_t  *p_td;
	ndd_t	*p_ndd = (ndd_t *)&(NDD);
	uchar status;
	struct mbuf *p_mbuf;
	int count;
	int pio_rc = 0;
	int ipri;



	TRACE_SYS(HKWD_ENTSAMP_XMIT, "StxB", (ulong)p_dev_ctl, 
		(ulong)command_reg, bus);

	ipri = disable_lock(PL_IMP, &TX_LOCK);


	if ((command_reg == TX_P_SENT) || (command_reg == TX_ERROR)) {

		/*
		 * Stop the watchdog timer
		 */
		w_stop(&TXWDT);
		restart_timer = TRUE;

		while (TRUE) {
			/*
		 	 * Examine the transmit buffer pool, if a packet
			 * transmission is completed, get the status of 
			 * the packet and continue the transmit-done
			 * procedure, otherwise, exit the while loop.
		 	 */

                        p_td = WRK.txd_first;
                        if (!(p_td->flags & BDC_IN_USE))
                                break;

                        /*
                         * get status byte from buffer descriptor
                         */
                        ENT_GETCX((char *)(bus + p_td->bd_off),
                                                &status);

                        if (pio_rc) {
                                unlock_enable(ipri, &TX_LOCK);
                                entsamp_hard_err(p_dev_ctl, TRUE, FALSE,
                                        NDD_PIO_FAIL);
                                TRACE_SYS(HKWD_ENTSAMP_ERR, "Stx1", pio_rc,
                                        0, 0);
                                return;
                        }

                        if (!(status & CMPLT_BIT_MASK)) {
                                break;
			}

			/* 
			 * Is xmit completed with error or not?
			 */
			count = p_td->tx_len;
			if (status & BD_ERR_MASK)  {   /* error */
				p_ndd->ndd_genstats.ndd_oerrors++;
			}
			else {
	  		  if (p_ndd->ndd_genstats.ndd_opackets_lsw == 
				ULONG_MAX) 
	    			p_ndd->ndd_genstats.ndd_opackets_msw++;
	    		  p_ndd->ndd_genstats.ndd_opackets_lsw++;

	  		  if ((ULONG_MAX - count) < 
				p_ndd->ndd_genstats.ndd_obytes_lsw)
	    			p_ndd->ndd_genstats.ndd_obytes_msw++;
	    		  p_ndd->ndd_genstats.ndd_obytes_lsw += count;

			  if (p_td->flags & BDC_BCAST) {
 				MIB.Generic_mib.ifExtnsEntry.bcast_tx_ok++;
			  }
			  if (p_td->flags & BDC_MCAST) {
				MIB.Generic_mib.ifExtnsEntry.mcast_tx_ok++;
			  }
				
			}
                        /*
                         * Check dma status
                         */
                        if ((rc = d_complete(WRK.dma_channel, DMA_WRITE_ONLY,
                                p_td->buf, (size_t)count,
                                (struct xmem *)&WRK.txbuf_xmem,
                                (char *)p_td->dma_io)) != DMA_SUCC) {

                                entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_DMAFAIL,
                                        __LINE__, __FILE__, WRK.dma_channel,
                                        p_td->buf, count);

                                unlock_enable(ipri, &TX_LOCK);
                                entsamp_hard_err(p_dev_ctl, TRUE, FALSE,
                                        ENT_DMA_FAIL);
                                TRACE_SYS(HKWD_ENTSAMP_ERR, "Stx2", rc, 0, 0);
                                return;
                        }


                        /*
                         * mark transmit element as free
                         */
                        p_td->flags = BDC_INITED;
                        WRK.txd_first = p_td->next;
                        p_dev_ctl->tx_pending--;

                }
        }



        /*
         * if there is packets on the transmit queue to be transmitted now.
         */
        while(!(WRK.txd_avail->flags & BDC_IN_USE)) {
                if (p_mbuf = entsamp_txq_get(p_dev_ctl)) {
                        if (!entsamp_xmit(p_dev_ctl, p_mbuf, bus)) {
                                /*
                                 * Transmit OK, free the packet
                                 */
                                m_freem(p_mbuf);
                                restart_timer = FALSE;
                        }
                        else {
                                /*
                                 * Transmit error. free the packet.
                                 */
                                NDD.ndd_genstats.ndd_oerrors++;
                                m_freem(p_mbuf);

                                unlock_enable(ipri, &TX_LOCK);
                                entsamp_hard_err(p_dev_ctl, TRUE, FALSE,
                                        NDD_PIO_FAIL);
                                TRACE_SYS(HKWD_ENTSAMP_ERR, "Stx3", 0, 0, 0);
                                return;
                        }
                }
                else
                        break;
        }

        /* re-arm the watchdog timer */
        if (restart_timer && p_dev_ctl->tx_pending) {
                w_start(&(TXWDT));
        }

	unlock_enable(ipri, &TX_LOCK);

	TRACE_SYS(HKWD_ENTSAMP_XMIT, "StxE", 0, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     entsamp_exec_intr
 *
 * FUNCTION: Ethernet driver execute command interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_intr
 *
 * INPUT:
 *      p_dev_ctl	- point to the device control area
 *	command_reg	- value from command register
 *	bus		- bus memory access handle
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_exec_intr(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* point to the device control area */
  uchar		command_reg,	/* value in the command register */
  int		bus)		/* bus memory access handle */

{

  int ipri;




	TRACE_SYS(HKWD_ENTSAMP_OTHER, "SexB", (ulong)p_dev_ctl, 
		(ulong)command_reg, bus);


	/* commands completed during non-limbo mode */

	if (p_dev_ctl->device_state != LIMBO) {

	  ipri = disable_lock(PL_IMP, &CMD_LOCK);
	  /*
	   * Stop the watchdog timer
	   */
	  w_stop(&(CTLWDT));

	  /*
	   * Examine the control command mailbox, if a command
	   * is completed, get the status of the command and
	   * wakeup the sleeping process which is waiting for the 
	   * command completion.
	   *
	   * A section of the sample code is omitted. 
	   */


	  /* wakeup the ioctl event */
	  e_wakeup((int *)&p_dev_ctl->ctl_event);

	  unlock_enable(ipri, &CMD_LOCK);
	}



	TRACE_SYS(HKWD_ENTSAMP_OTHER, "SexE", 0, 0, 0);

}
