static char sccsid[] = "@(#)42  1.1  src/bos/usr/samples/ndd/entsamp_out.c, entsamp, bos411, 9428A410j 1/14/94 13:40:04";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_output
 *		entsamp_xmit
 *		entsamp_txq_put
 *		entsamp_txq_get
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
#include "entsamp.h"




/*****************************************************************************/
/*
 * NAME:     entsamp_output
 *
 * FUNCTION: Ethernet driver output routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ndd_output field in the NDD on the NDD chain.
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *	p_mbuf		- pointer to a mbuf (chain) for outgoing packets
 *
 * RETURNS:  
 *	0 - successful
 *	EAGAIN - transmit queue is full
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN - device is down
 */
/*****************************************************************************/
entsamp_output(
  ndd_t		*p_ndd,		/* pointer to the ndd in the dev_ctl area */
  struct mbuf	*p_mbuf)	/* pointer to a mbuf (chain) */

{
  entsamp_dev_ctl_t   *p_dev_ctl = (entsamp_dev_ctl_t *)(p_ndd->ndd_correlator);
  struct mbuf *p_cur_mbuf;
  struct mbuf *buf_tofree;
  int first = TRUE;
  int bus;
  struct mbuf *entsamp_txq_put();
  int ipri;



  TRACE_SYS(HKWD_ENTSAMP_XMIT, "TtxB", (ulong)p_ndd, (ulong)p_mbuf, 0);
  
  ipri = disable_lock(PL_IMP, &TX_LOCK);

  if (p_dev_ctl->device_state != OPENED) {
	if (p_dev_ctl->device_state == DEAD) {
		unlock_enable(ipri, &TX_LOCK);
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Ttx1", ENETDOWN, 
			p_dev_ctl->device_state, 0);
		return(ENETDOWN);
	}
	else {
		unlock_enable(ipri, &TX_LOCK);
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Ttx2", ENETUNREACH, 
			p_dev_ctl->device_state, 0);
		return(ENETUNREACH);
	}
  }

  /*
   * if there is a transmit queue, put the packet onto the queue.
   */
  if (p_dev_ctl->txq_first) {
	/*
	 *  if the txq is full, return EAGAIN. Otherwise, queue as
	 *  many packets onto the transmit queue and free the
	 *  rest of the packets, return no error.
	 */
	if (p_dev_ctl->txq_len >= DDS.xmt_que_size) {
					
		unlock_enable(ipri, &TX_LOCK);

		while (p_mbuf) {
			p_ndd->ndd_genstats.ndd_xmitque_ovf++;
			p_mbuf = p_mbuf->m_nextpkt;
		}
  		TRACE_SYS(HKWD_ENTSAMP_ERR, "Ttx3", EAGAIN, 0, 0);
		return(EAGAIN);
	}
	else {
		buf_tofree = entsamp_txq_put(p_dev_ctl, p_mbuf);
		unlock_enable(ipri, &TX_LOCK);

		while (p_cur_mbuf = buf_tofree) {
			p_ndd->ndd_genstats.ndd_xmitque_ovf++;
			buf_tofree = buf_tofree->m_nextpkt;
			p_cur_mbuf->m_nextpkt = NULL;
			m_freem(p_cur_mbuf);
				
		}
  		TRACE_SYS(HKWD_ENTSAMP_XMIT, "Ttx4", 0, 0, 0);
		return(0);
	}

  }

  bus = (int)BUSMEM_ATT((ulong)DDS.bus_id, DDS.bus_mem_addr);

  while (p_mbuf) {
	p_cur_mbuf = p_mbuf;

	/*
	 * If there is txd available, try to transmit the packet.
	 */
	if (!(WRK.txd_avail->flags & BDC_IN_USE)) {
		
		if (!entsamp_xmit(p_dev_ctl, p_cur_mbuf, bus)) {
		    /*
		     * Transmit OK, free the packet
		     */

	  	    p_mbuf = p_mbuf->m_nextpkt;
		    p_cur_mbuf->m_nextpkt = NULL;;
		    m_freem(p_cur_mbuf);
		    first = FALSE;
		}
		else {
		    /*
		     * Transmit error. Call hardware error recovery
		     * function. If this is the first packet,
		     * return error. Otherwise, free the reset packets
		     * and return error. 
		     */
		    if (first) {
					
  			BUSMEM_DET(bus);
			unlock_enable(ipri, &TX_LOCK);

			entsamp_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);

  			TRACE_SYS(HKWD_ENTSAMP_ERR, "Ttx5", ENETDOWN, 0, 0);
			return(ENETDOWN);
		    }
		    else {
  			BUSMEM_DET(bus);

			/* increment the error counter */
			while (p_cur_mbuf = p_mbuf) {
				p_mbuf = p_mbuf->m_nextpkt;
				p_cur_mbuf->m_nextpkt = NULL;
				m_freem(p_cur_mbuf);
			}

  			unlock_enable(ipri, &TX_LOCK);

			entsamp_hard_err(p_dev_ctl, FALSE, FALSE, NDD_PIO_FAIL);

  			TRACE_SYS(HKWD_ENTSAMP_XMIT, "Ttx6", 0, 0, 0);
  			return(0);

		    }
		}
	}    /* if there is txd available */
	else {
		buf_tofree = entsamp_txq_put(p_dev_ctl, p_cur_mbuf);
  		BUSMEM_DET(bus);
		unlock_enable(ipri, &TX_LOCK);
		
		while (p_cur_mbuf = buf_tofree) {
			p_ndd->ndd_genstats.ndd_xmitque_ovf++;
			buf_tofree = buf_tofree->m_nextpkt;
			p_cur_mbuf->m_nextpkt = NULL;
			m_freem(p_cur_mbuf);
		}
  		TRACE_SYS(HKWD_ENTSAMP_XMIT, "Ttx7", 0, 0, 0);
		return(0);

	}

  }    /* while */

  BUSMEM_DET(bus);

  unlock_enable(ipri, &TX_LOCK);

  TRACE_SYS(HKWD_ENTSAMP_XMIT, "TtxE", 0, 0, 0);
  return(0);

}

/*****************************************************************************/
/*
 * NAME:     entsamp_xmit
 *
 * FUNCTION: transmit packets to the adapter.
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_output
 *	entsamp_tx_intr
 *
 * INPUT:
 *	p_dev_ctl	- pointer to the device control area
 *	p_mbuf		- pointer to a packet in mbuf
 *	bus		- handle for accessing I/O bus
 *
 * RETURNS:  
 *	0 - OK
 * 	-1 - error occurred during transmit
 */
/*****************************************************************************/
entsamp_xmit(
  entsamp_dev_ctl_t  *p_dev_ctl,	/* pointer to the device control area */
  struct mbuf	*p_mbuf,	/* pointer to the packet in mbuf */
  int		bus)		/* handle for I/O bus accessing */

{

  	ndd_t   *p_ndd = &(NDD);
	entsamp_bdc_t	*p_txd = WRK.txd_avail;
	int count;
	int offset;
	int rc;
	int pio_rc = 0;



  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "TxmB", (ulong)p_ndd, (ulong)p_mbuf, bus);

	/*
	 * Call ndd_trace if it is enabled
	 */
	if (p_ndd->ndd_trace) {
		(*(p_ndd->ndd_trace))(p_ndd, p_mbuf, 
			p_mbuf->m_data, p_ndd->ndd_trace_arg);
	}

	/*
	 * Use the first available txd to transmit the packet
	 */
	
	count = (p_mbuf->m_pkthdr.len <= (DMA_PSIZE / 2)) ? 
		p_mbuf->m_pkthdr.len : (DMA_PSIZE / 2);
	p_txd->flags |= BDC_IN_USE;

	/* increment the tx_pending count */
	p_dev_ctl->tx_pending++;

	/*
	 * copy data into tranmit buffer and do processor cache flush
         */
        m_copydata(p_mbuf, 0, count, p_txd->buf);

	/*
	 * Pad short packet with garbage
	 */
	if (count < ENT_MIN_MTU)
		count = ENT_MIN_MTU;
	p_txd->tx_len = count;

	/*
	 * We require the M_BCAST and M_MCAST flags set for broadcast or
	 * multicast packets before this functionis called. Now save
	 * the flag in the txd.
	 */
	if (p_mbuf->m_flags & M_BCAST) 
		p_txd->flags |= BDC_BCAST;
	if (p_mbuf->m_flags & M_MCAST) 
		p_txd->flags |= BDC_MCAST;

 	vm_cflush(p_txd->buf, count);

	/* 
	 * Setup the transmit buffer descriptor and start transmitting
	 * the packet.
	 *
	 * A section of device-specific code was omitted here.
         */

	/*
	 * update the txd pointers
	 */
	WRK.txd_last = WRK.txd_avail;
	WRK.txd_avail = WRK.txd_avail->next;

	if (pio_rc) {
  	  	TRACE_SYS(HKWD_ENTSAMP_ERR, "Txm2", (long)-1, pio_rc, 0);
		return(-1);
	}
		
        /* start watchdog timer */
        w_start(&(TXWDT));

  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "TxmE", 0, 0, 0);
	return(0);



}

/*****************************************************************************/
/*
 * NAME:     entsamp_txq_put
 *
 * FUNCTION: put packets onto the transmit queue.
 *
 * EXECUTION ENVIRONMENT: process and interrupt level.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_output
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	p_mbuf		- pointer to a mbuf chain
 *
 * RETURNS:  
 *	NULL - all mbufs are queued.
 *	mbuf pointer - point to a mbuf chain which contains packets
 *		       that overflows the transmit queue.
 */
/*****************************************************************************/
struct mbuf *
entsamp_txq_put(
	entsamp_dev_ctl_t  *p_dev_ctl,	/* pointer to the device control area */
	struct mbuf	*p_mbuf)	/* pointer to a mbuf chain */

{

	int room;
	int pkt_cnt;
	struct mbuf *p_last, *p_over;



  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "TqpB", (ulong)p_dev_ctl, 
		(ulong)p_mbuf, 0);

	pkt_cnt = 0;
	room = DDS.xmt_que_size - p_dev_ctl->txq_len;
	if (room > 0) {
		p_last = p_mbuf;
		room--;
		pkt_cnt++;
		while (room && p_last->m_nextpkt) {
			room--;
			pkt_cnt++;
			p_last = p_last->m_nextpkt;
		}
		p_over = p_last->m_nextpkt;  /* overflow packets */
		
		if (p_dev_ctl->txq_first) {
			p_dev_ctl->txq_last->m_nextpkt = p_mbuf;
			p_dev_ctl->txq_last = p_last;
		}
		else {
			p_dev_ctl->txq_first = p_mbuf;
			p_dev_ctl->txq_last = p_last;
		}
		p_last->m_nextpkt = NULL;
		p_dev_ctl->txq_len += pkt_cnt;
		if (NDD.ndd_genstats.ndd_xmitque_max < p_dev_ctl->txq_len)
			NDD.ndd_genstats.ndd_xmitque_max = p_dev_ctl->txq_len;
	}
	else {
		p_over = p_mbuf;
	}

  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "TqpE", (ulong)p_over, 0, 0);

	return(p_over);


}

/*****************************************************************************/
/*
 * NAME:     entsamp_txq_get
 *
 * FUNCTION: get a packet off the transmit queue.
 *
 * EXECUTION ENVIRONMENT: interrupt level.
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_tx_intr
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *
 * RETURNS:  
 *	NULL - queue is empty
 *	mbuf pointer - point to a mbuf chain which contains a packet.
 */
/*****************************************************************************/
struct mbuf *
entsamp_txq_get(
  entsamp_dev_ctl_t	*p_dev_ctl)	/* pointer to the device control area */

{

	struct mbuf *p_first = NULL;

  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "TqgB", (ulong)p_dev_ctl, 0, 0);

	if (p_dev_ctl->txq_first) {
		p_first = p_dev_ctl->txq_first;
		p_dev_ctl->txq_first = p_first->m_nextpkt;
		p_first->m_nextpkt = NULL;
		p_dev_ctl->txq_len--;
	}
  	TRACE_SYS(HKWD_ENTSAMP_OTHER, "TqgE", (ulong)p_first, 0, 0);

	return(p_first);

}
