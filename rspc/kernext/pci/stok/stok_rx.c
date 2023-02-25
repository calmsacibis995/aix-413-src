static char sccsid[] = "@(#)52  1.3  src/rspc/kernext/pci/stok/stok_rx.c, pcitok, rspc41J, 9520B_all 5/18/95 09:54:23";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: TRACE_SYS
 *		arm_rx_list
 *		discard_packet
 *		stok_rx
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
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
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/param.h>
#include <sys/poll.h>
#include <sys/sleep.h>
#include <sys/trchkid.h>
#include <sys/err_rec.h>
#include <sys/mbuf.h>
#include <sys/dump.h>
#include <sys/ndd.h>
#include <sys/cdli.h>

#include <sys/cdli_tokuser.h>
#include <sys/generic_mibs.h>
#include <sys/tokenring_mibs.h>

#include "stok_dslo.h"
#include "stok_mac.h"
#include "stok_dds.h"
#include "stok_dd.h"
#include "tr_stok_errids.h"
#ifdef KTD_DEBUG
#include "intercept_functions.h"
#endif

/*****************************************************************************/
/*                                                                           */
/*  NAME: arm_rx_list                                                        */
/*                                                                           */
/*  FUNCTION:                                                                */
/*    Rearms the receive list indexed by i and update the receive chain.     */
/*                                                                           */
/*  EXECUTION ENVIRONMENT:                                                   */
/*      This routine can be executed by both the interrupt and process call  */
/*      threads of the driver.                                               */
/*                                                                           */
/*****************************************************************************/

void arm_rx_list (
stok_dev_ctl_t *p_dev_ctl,  /* pointer to the device control area */
int            i,           /* which receive list */
struct mbuf    *m)          /* MBUF to rearm      */
{
  int	ioa;

  /*                                                              
   * Setup the receive list 
   */
  WRK.rx_desc_p[i]->rx_status    = 0;
  WRK.rx_desc_p[i]->data_pointer = TOENDIANL((ulong)WRK.rx_addr[i]);
  WRK.rx_desc_p[i]->data_len     = TOENDIANW(PAGESIZE); 
  WRK.rx_desc_p[i]->fr_len       = 0;

  ioa = (int)iomem_att(&WRK.iomap);
  PIO_PUTLRX(ioa + RxLBDA_L, WRK.rx_dma_desc_p[i]);
  iomem_det((void *)ioa);                    /* restore I/O Bus              */
  if (WRK.pio_rc) {
        stok_bug_out(p_dev_ctl,NDD_HARD_FAIL, NDD_PIO_FAIL,0,FALSE,FALSE,FALSE);
  }
}

/*****************************************************************************/
/* NAME : discard_packet
*
* FUNCTION: discard the bad packet. 
*
* EXECUTION ENVIRONMENT: process only
*
* INPUT: 
*	p_dev_ctl - pointer to device control structure.
*
* CALLED FROM:
*       stok_rx
*
* CALL TO:
*       arm_rx_list
*/
/*****************************************************************************/
void discard_packet (
stok_dev_ctl_t       *p_dev_ctl)  /* pointer to the device control area */

{

  long    		status;
  struct mbuf 		*m, *m0;

  NDD.ndd_genstats.ndd_ierrors++;

  /*
   * Frees any mbuf that have allocated for the receive frame but have not yet
   * passed up to the higher layer.  
   */
  if (WRK.mhead) {
        m_freem(WRK.mhead);
        WRK.mhead = NULL;
  }

  do {
  	status  = TOENDIANL(WRK.rx_desc_p[WRK.read_index]->rx_status);

  	if (!(status & BUF_PROCESS)) {
  		break;
	}

  	arm_rx_list(p_dev_ctl, WRK.read_index, WRK.rx_mbuf[WRK.read_index]);

	INC_DESC(WRK.read_index, DDS.rx_que_sz);
  } while (!(status & END_OF_FRAME));

  if (status & END_OF_FRAME) {
 	WRK.eof = TRUE;
  } else {
	WRK.eof = FALSE;
  }

} /* end of discard_packet */

/*****************************************************************************/
/*                                                                           
* NAME: stok_rx                                                            
*
* FUNCTION:  Receive the packets from the adapter. 
*
* EXECUTION ENVIRONMENT: interrupt only
*
* INPUT: 
*  	p_dev_ctl - pointer to device control structure.
*       cmd_reg   - value in the status register.
*
* CALLED FROM:
*        stok_intr
*
* CALLS TO:
*       arm_rx_list
*	discard_packet
*/
/*****************************************************************************/
void stok_rx ( 
stok_dev_ctl_t *p_dev_ctl,     /* point to the device control area */
uchar         cmd_reg,        /* value in the status register    */
int	      ioa)
{
  long			status;
  struct mbuf 		*rx_mbuf, *new_mbuf, *p_mbuf;
  int			rc, i, len;
  ushort  		bctl, pkthdr_len;
  ndd_statblk_t  	stat_blk;   /* status block */
  ndd_t   		*p_ndd = (ndd_t *)&(NDD);
  uchar bcast1_addr[CTOK_NADR_LENGTH] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uchar bcast2_addr[CTOK_NADR_LENGTH] = { 0xC0,0x00,0xFF,0xFF,0xFF,0xFF};
  int	bda;
  uint         rx_addr;    /* dma addrs of mbufs                 */ 

  TRACE_SYS(STOK_RV, "RrvB", (ulong)p_dev_ctl, (ulong)cmd_reg, 0);

  while (TRUE) {
  	i = WRK.read_index;               /* start at next rx list */

  	status  = TOENDIANL(WRK.rx_desc_p[i]->rx_status);

  	/* 
	 * Checks if any more buffer is being process
	 */
  	if (!(status & BUF_PROCESS)) {
  		break;
        }

  	rx_mbuf = WRK.rx_mbuf[i];
  	rx_mbuf->m_len = TOENDIANW(WRK.rx_desc_p[i]->data_len); /* buffer len */
  	pkthdr_len = TOENDIANW(WRK.rx_desc_p[i]->fr_len);
	/*
  	 * Checks the status of the receive buffer 
	 */
        if (status & RX_ERR) {
  		TRACE_BOTH(STOK_ERR, "Rrv1", p_dev_ctl, status, 0);

  		if (status & RX_BUS_ERR) {
                	stok_logerr(p_dev_ctl, ERRID_STOK_DMAFAIL, __LINE__, 
				    __FILE__, WRK.dma_channel, status, i);

		}

  		DEVSTATS.rx_frame_err++;
  		discard_packet(p_dev_ctl);

        } else if ((((status & PROTOCOL_ERR1) && (!WRK.promiscuous_count))) |
        	   (((status & PROTOCOL_ERR2) && (WRK.promiscuous_count)))) {
  		/* 
		 *  bad frame with receive overrun/Protocol chip error 
  		 *  with promiscuous mode off/promiscuous mode on
  		 */
  		TRACE_BOTH(STOK_ERR, "Rrv2", p_dev_ctl, status, 0);
                if (status & RECV_OVERRUN)  {
  			DEVSTATS.recv_overrun++;
		} else {
  			DEVSTATS.rx_frame_err++;
 		}
  		discard_packet(p_dev_ctl);

	} else if (!WRK.eof) {
  		discard_packet(p_dev_ctl);

  	} else { /* good frame receive */

                /* For netpmon performance tool */
                TRACE_SYS(STOK_RV, TRC_RDAT, p_dev_ctl->seq_number, 
				WRK.read_index, rx_mbuf->m_len);

  		/* Gets the mbuf for receive frame */
  		if (rx_mbuf->m_len <= (MHLEN)) {
  			new_mbuf = m_gethdr(M_DONTWAIT, MT_DATA);
  		} else if (rx_mbuf->m_len <= HAFT_PAGE) {
  			new_mbuf = m_getclustm(M_DONTWAIT, MT_DATA, 
                                               rx_mbuf->m_len);
  		} else { 
  			new_mbuf = m_getclust(M_DONTWAIT, MT_DATA);
		}

  		if (new_mbuf == NULL) {
  			NDD.ndd_genstats.ndd_nobufs++;
  			NDD.ndd_genstats.ndd_ipackets_drop++;
  			discard_packet(p_dev_ctl);
  		} else {
  			new_mbuf->m_next = NULL;

			if (rx_mbuf->m_len <= HAFT_PAGE) {
  				/*
			 	 * Copys data from d_master'ed mbuf to the new 
				 * one
			 	 */
  				bcopy(mtod(rx_mbuf, caddr_t), 
				      mtod(new_mbuf, caddr_t),
				      rx_mbuf->m_len);

  				new_mbuf->m_len = rx_mbuf->m_len;

  				arm_rx_list(p_dev_ctl, i, rx_mbuf);
			} else {
  				/* Try to D_MAP_PAGE the new buffer then call 
				 * arm_rx_list to  re_arm the rx queue with 
				 * the new buffer.  If D_MAP_PAGE is "FALSE", 
				 * just copy the data in the rx buffer to the 
				 * new buffer and re_arm the rx buffer.
				 */
				rx_addr = WRK.rx_addr[i];
  				if (D_MAP_PAGE (WRK.dma_channel, DMA_READ, 
						MTOD( new_mbuf, uchar * ), 
  	    	  				&rx_addr, xmem_global)) {
  					bcopy(mtod(rx_mbuf, caddr_t), 
				      		mtod(new_mbuf, caddr_t),
				      		rx_mbuf->m_len);

  					new_mbuf->m_len = rx_mbuf->m_len;

  					arm_rx_list(p_dev_ctl, i, rx_mbuf);
					
				} else {
					D_UNMAP_PAGE(WRK.dma_channel, 
                                                    &WRK.rx_addr[i]);
  					WRK.rx_mbuf[i] = new_mbuf;
					WRK.rx_addr[i] = rx_addr;
  					arm_rx_list(p_dev_ctl, i, new_mbuf);
					new_mbuf = rx_mbuf;
				}
			}

  			/*  
			 *  See if we are at the beginning of a new frame;  
			 *  if so, begin a new linked list of mbufs. If we 
			 *  are not at the start of a frame, simply add this 
			 *  mbuf to the end of the current list.    
                   	 */
  			if (WRK.mhead == NULL) {
  				WRK.mhead = WRK.mtail = p_mbuf = new_mbuf;
  				WRK.mhead->m_flags |= M_PKTHDR;
  				WRK.mhead->m_nextpkt = NULL;

  			} else {
				p_mbuf = WRK.mtail;
  				WRK.mtail->m_next = new_mbuf;
  				WRK.mtail = new_mbuf;
  			}

  		/*  
		 *  If this is the end of the frame, submit this list 
                 *  of mbufs to the receive handler.   
                 */
  		if (status & END_OF_FRAME) {
  			/* Checks if broadcast or multicast */
  			if (*(mtod(WRK.mhead, caddr_t) + 2) & 
				MULTI_BIT_MASK) {
  			      if((SAME_NADR((mtod(WRK.mhead, caddr_t) + 2),
  					bcast1_addr)) |
  			    	 (SAME_NADR((mtod(WRK.mhead, caddr_t) + 2), 
					bcast2_addr)))
  				{
  					TOKSTATS.bcast_recv++;
  					new_mbuf->m_flags |= M_BCAST;
  				} else {
  					TOKSTATS.mcast_recv++;
  					new_mbuf->m_flags |= M_MCAST;
  				}
  			}
			/* removes the 4 bytes hardware CRC */
			if (WRK.mtail->m_len > 4) {
  				WRK.mtail->m_len -= 4;
			} else {
				len = WRK.mtail->m_len - 4;
				WRK.mtail = p_mbuf;	
  				WRK.mtail->m_next = NULL;
				WRK.mtail->m_len += len;
				m_freem(new_mbuf);
			}

  			WRK.mhead->m_pkthdr.len = pkthdr_len - 4;

  			if (NDD.ndd_genstats.ndd_ipackets_lsw == ULONG_MAX) {
  				NDD.ndd_genstats.ndd_ipackets_msw++;
			}
  			NDD.ndd_genstats.ndd_ipackets_lsw++;

  			if ((ULONG_MAX - WRK.mhead->m_pkthdr.len) <
  			  	NDD.ndd_genstats.ndd_ibytes_lsw) {
  				NDD.ndd_genstats.ndd_ibytes_msw++;
			}
  			NDD.ndd_genstats.ndd_ibytes_lsw += 
  					WRK.mhead->m_pkthdr.len;

                        /* For netpmon performance tool */
                        TRACE_SYS(STOK_RV, TRC_RNOT, 
				  p_dev_ctl->seq_number, 
				  WRK.mhead, WRK.mhead->m_pkthdr.len);
  			(*(NDD.nd_receive))(p_ndd, WRK.mhead);

                        /* For netpmon performance tool */
                        TRACE_SYS(STOK_RV, TRC_REND, p_dev_ctl->seq_number, 
				  WRK.read_index, 0); 

  			WRK.mhead = WRK.mtail  = NULL;
  		}
		INC_DESC(WRK.read_index, DDS.rx_que_sz);
  	}
  } /* end of check status */
} /* end of while */
TRACE_SYS(STOK_RV, "RrvE", (ulong)p_dev_ctl, (ulong)cmd_reg, 0);

} /* end stok_rx routine                                                    */


