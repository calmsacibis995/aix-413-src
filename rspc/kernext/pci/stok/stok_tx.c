static char sccsid[] = "@(#)53  1.3  src/rspc/kernext/pci/stok/stok_tx.c, pcitok, rspc41J, 9518A_all 5/2/95 14:39:28";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: stok_collap
 *		stok_common_output
 *		stok_output
 *		stok_priority
 *		stok_send
 *		stok_tx_done
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

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/lock_def.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/malloc.h>
#include <sys/dump.h>
#include <sys/watchdog.h>
#include <sys/intr.h>
#include <sys/mbuf.h>
#include <sys/err_rec.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/iocc.h>
#include <sys/dma.h>
#include <sys/cdli.h>
#include <sys/ndd.h>
#include <unistd.h>

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
/*
 * NAME: stok_collapse
 *
 * FUNCTION : The driver will allocates new buffer chain  to copy the data from
 *            the tx packet,  and the new buffer chain will be less than 11.
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * INPUT:
 *    	p_dev_ctl - pointer to device control structure
 *	txq_firs  - pointer to the software txq
 *	wrk	  - pointer to the Tx channel structure
 *
 * CALLED FROM:
 *  	stok_send
 *
 * RETURNS: 
 *      0         - successful
 *      ENOMEM    - unable to allocate required memory
 *
 */
/*****************************************************************************/
int stok_collapse (
stok_dev_ctl_t  *p_dev_ctl,
struct  mbuf    *txq_first,
tx_t	        *wrk)		
{

  struct mbuf *mbufp;
  struct mbuf *head;    /*  point to the first collapse mbuf            */
  struct mbuf *m;       /*  point to the tail of the collapse mbuf      */
  struct mbuf *n;       /*  point to the new buffer just got allocated  */
  uchar       *odata;   /*  data pointer of the tx packet mbuf          */
  uchar       *ndata;   /*  data pointer of the collapse mbuf           */
  int         len;      /*  length of the tx packet mbuf                */
  int         copy_len; /*  length to be copy to the collapse mbuf      */

  n = m_getclust(M_DONTWAIT, MT_HEADER);
  if (!n) {
          return(ENOMEM);
  }
  head = m = n;
  m->m_flags |= M_PKTHDR;
  m->m_nextpkt = NULL;
  ndata = mtod(n, caddr_t);
  n->m_len = 0;

  mbufp = wrk->hw_txq_in;
  while (mbufp) {
          odata = mtod(mbufp, caddr_t);
          len = mbufp->m_len;
          while (len) {
                  /* get new "gather" mbuf if data won't fit */
                  if (n->m_len >= CLBYTES) {
                         n = m_getclust(M_DONTWAIT,MT_DATA);
                         if (!n) {
                                  m_freem(head);
          			  return(ENOMEM);
                          }
                          m->m_next = n;
                          m = n;
                          ndata = mtod(n, caddr_t);
			  n->m_len = 0;
                  }

                  /*
                   * Calc len to copy
                   */
                  if ((n->m_len + len) > CLBYTES){
                          copy_len = CLBYTES - n->m_len;
                  } else {
                          copy_len = len;
                  }

                  bcopy(odata + (mbufp->m_len-len), ndata + n->m_len, copy_len);
                  len -= copy_len;
                  n->m_len += copy_len;
          }
          mbufp = mbufp->m_next;
  }
  if (wrk->sw_txq_first == wrk->sw_txq_last) {
  	wrk->sw_txq_first = wrk->sw_txq_last = head;
  } else {
  	wrk->sw_txq_first = head;
  }
  wrk->sw_txq_first->m_pkthdr.len = wrk->hw_txq_in->m_pkthdr.len;
  m_freem(wrk->hw_txq_in);
  wrk->hw_txq_in = head;
  return (0);

}
/*****************************************************************************/
/*
 * NAME: stok_send
 *
 * FUNCTION: Moves a frame from software txq to hardware txq and gives the 
 *           tx descriptor to adapter
 *
 * EXECUTION ENVIRONMENT: process and interrupt
 *
 * INPUT:
 *      p_dev_ctl - pointer to device control structure
 *	wrk	  - pointer to the Tx channel structure
 *    	ioa       - IO address 
 *
 * CALLED FROM:
 *  	stok_common_output
 *	stok_tx_done    
 *
 */
/*****************************************************************************/
void stok_send (
stok_dev_ctl_t       *p_dev_ctl,
tx_t	             *wrk,		
int                   ioa)
{

  ndd_t			*p_ndd = (ndd_t *)&(NDD);
  struct  mbuf          *mbufp, *txq_first, *p_mbuf;
  ushort                buf_count;
  int                   i, j, cnt, ct, no_space = FALSE, len;

  TRACE_SYS(STOK_TX, "TssB", p_dev_ctl, wrk->tx_elem_next_in, 
						wrk->tx_elem_next_out); 

  /*
   * What DMA address adapter get for transmit are based on the
   * length of the package and the length of each mbuf
   *   o  For package length less than or equal to the 2K bytes.
   *      Copys data into transmit buffer which already setup
   *      for DMA
   *   o  For package length greater than 2K bytes
   *	    - If mbuf len less than or equal to the 2K bytes. 
   
   *          for DMA
   *	    - If mbuf length greater than the 2K bytes. 
   *          Setup DMA transfer directly from the mbuf data
   *          to the adapter
   */
  while (wrk->sw_txq_first && (wrk->tx_frame_pending < MAX_TX_LIST)) {
        i = wrk->tx_elem_next_in;

	if (wrk->hw_txq_out == NULL) {
		p_mbuf = NULL;
		wrk->hw_txq_out = wrk->hw_txq_in = wrk->sw_txq_first;
	} else {
		p_mbuf = wrk->hw_txq_in;
		wrk->hw_txq_in->m_nextpkt  = wrk->sw_txq_first;
		wrk->hw_txq_in = wrk->sw_txq_first;
 	}
        txq_first  = wrk->sw_txq_first->m_nextpkt;
        wrk->hw_txq_in->m_nextpkt = NULL;

        wrk->tx_desc_p[i]->xmit_status   = 0;
        wrk->tx_desc_p[i]->frame_len     = 0;

        if (wrk->hw_txq_in->m_pkthdr.len <= HAFT_PAGE) {
                if (wrk->tx_buf_use_count < wrk->max_buf_list) {
                	j = wrk->tx_buf_next_in;
                	/*
                	 * Copys data into transmit buffer which already
			 * setup for DMA 
                	 */
                        MBUF_TO_MEM(wrk->hw_txq_in, wrk->tx_buf[j]);

                        /*
                         * Setup buffer descriptor entry and update
                         * transmit counter
                         */
                        wrk->tx_desc_p[i]->buf_count     = TOENDIANW(1);
                        wrk->tx_desc_p[i]->xbuf[0].data_pointer =
        	                       TOENDIANL((ulong)wrk->tx_addr[j]);
                        wrk->tx_desc_p[i]->xbuf[0].buf_len =
                                TOENDIANW((ushort)wrk->hw_txq_in->m_pkthdr.len);

			INC_DESC(wrk->tx_buf_next_in, wrk->max_buf_list);
                        wrk->tx_buf_use_count++;
                } else {
  			TRACE_BOTH(STOK_ERR, "Tss1", p_dev_ctl, 
					wrk->tx_elem_next_in, 
					wrk->tx_buf_use_count);
          		wrk->hw_txq_in->m_nextpkt = txq_first;
			wrk->hw_txq_in = p_mbuf;
			if (wrk->hw_txq_in == NULL) {
				wrk->hw_txq_out = wrk->hw_txq_in;
			} else {
        			wrk->hw_txq_in->m_nextpkt = NULL;
			}
                        return;
                }

        } else {
                cnt = 0;
        	mbufp = wrk->hw_txq_in;
                while ((mbufp) && (cnt != 12)) {
                        mbufp = mbufp->m_next;
                        cnt++;
                }

                if (cnt > 11)  {
                /* if mbuf chain is more then 11 then collapsing 
                 * The number 11 is just an arbritrate number,  Can't explain
                 *  why 11.  But it seems to be a good number
		 */
			if (stok_collapse(p_dev_ctl, txq_first, wrk)) {
          			wrk->hw_txq_in->m_nextpkt = txq_first;
				wrk->hw_txq_in = p_mbuf;
				if (wrk->hw_txq_in == NULL) {
					wrk->hw_txq_out = wrk->hw_txq_in;
				} else {
        				wrk->hw_txq_in->m_nextpkt = NULL;
				}
				return;	
			} else {
				if (p_mbuf) {
          				p_mbuf->m_nextpkt = wrk->hw_txq_in;
				} else {
          				wrk->hw_txq_out = wrk->hw_txq_in;
				}
			}
                }

                cnt = 0;
                mbufp = wrk->hw_txq_in;
                while (mbufp) {

                        if ((mbufp->m_len <= HAFT_PAGE) && 
                            (wrk->tx_buf_use_count < wrk->max_buf_list)){
                		/*
                		 * Copys data into transmit buffer which already
				 * setup for DMA 
				 * and then update the transmit counter
                		 */
                        	j = wrk->tx_buf_next_in;
                                bcopy(mtod(mbufp, caddr_t), 
					   wrk->tx_buf[j],
                                           mbufp->m_len);
                                 wrk->tx_desc_p[i]->xbuf[cnt].data_pointer =
                                             TOENDIANL((ulong)wrk->tx_addr[j]);
                        	wrk->tx_desc_p[i]->xbuf[cnt++].buf_len =
                                                TOENDIANW((ushort)mbufp->m_len);

				 INC_DESC(wrk->tx_buf_next_in, 
						wrk->max_buf_list);
                                 wrk->tx_buf_use_count++;

                        } else if(mbufp->m_len > HAFT_PAGE) {

                                /*
                                 * Setup the DMA channel for block mode DMA 
				 * transfer directly from the mbuf data
				 * to the adapter
				 * Each DMA area is limit to 4K
                                 */
				len = 0;
				while (len < mbufp->m_len) {
					if(wrk->tx_dma_use_count==MAX_DMA_LIST){
  						TRACE_BOTH(STOK_ERR, "Tss0", 
							p_dev_ctl, 
							wrk->tx_buf_use_count, 
							wrk->tx_dma_use_count);
                                		no_space = TRUE;
                                		break;
					}

					ct = wrk->tx_dma_next_in;
        				D_MAP_PAGE(WRK.dma_channel, DMA_READ, 
				           	mtod(mbufp, uchar *) + len, 
				           	&wrk->tx_dma_addr[ct], 
						xmem_global);

                                       wrk->tx_desc_p[i]->xbuf[cnt].data_pointer
						 = TOENDIANL((ulong)wrk->tx_dma_addr[ct]);

					if (len == 0) {
						len = PAGESIZE - (mtod(mbufp,int) & 0xFFF);
						if (mbufp->m_len <= len) {
							len = mbufp->m_len; 
						}
                        			wrk->tx_desc_p[i]->xbuf[cnt++].buf_len =
                                               		TOENDIANW((ushort)len);
					} else {
						len += PAGESIZE;

						if (len < mbufp->m_len) {
                        				wrk->tx_desc_p[i]->xbuf[cnt++].buf_len =
                                                		TOENDIANW((ushort)PAGESIZE);
						} else {
                        				wrk->tx_desc_p[i]->xbuf[cnt++].buf_len =
                                       		        	TOENDIANW((ushort)
								(mbufp->m_len-(len-PAGESIZE)));
						}
					}

					INC_DESC(wrk->tx_dma_next_in, MAX_DMA_LIST );
                                	wrk->tx_dma_use_count++;
				}
				if (no_space == TRUE) {
					break;
				}

                        } else {
  				TRACE_BOTH(STOK_ERR, "Tss2", p_dev_ctl, 
						wrk->tx_buf_use_count, 
						wrk->tx_dma_use_count);
                                no_space = TRUE;
                                break;
                        }
			mbufp = mbufp->m_next;
                } /* end of while (mbufp) */

                if(no_space == TRUE) {
                        DEVSTATS.no_txq_resource++;
                        /*
                         * Not enough tx buffer or DMA address available for
			 * the transmit frame
                         */
			/*
			 * undo the transmit counter & the d_map_page
			 */
        		mbufp = wrk->hw_txq_in;
                        while (cnt != 0) {
                                if (mbufp->m_len > HAFT_PAGE) {
					len = 0;
					while(len < mbufp->m_len) {
						D_UNMAP_PAGE(WRK.dma_channel,
						     &wrk->tx_dma_addr
							[wrk->tx_dma_next_in]);
						DEC_DESC(wrk->tx_dma_next_in, 
							 MAX_DMA_LIST);
                                        	wrk->tx_dma_use_count--;
						if (len == 0) {
							len = PAGESIZE - (mtod(mbufp,int) & 0xFFF);
						} else {
							len += PAGESIZE;
						}
					}
                                } else {
					DEC_DESC(wrk->tx_buf_next_in,
					          wrk->max_buf_list);
                                        wrk->tx_buf_use_count--;
                                }
                                mbufp = mbufp->m_next,
                                cnt--;
                        }

          		wrk->hw_txq_in->m_nextpkt = txq_first;
			wrk->hw_txq_in = p_mbuf;
			if (wrk->hw_txq_in == NULL) {
				wrk->hw_txq_out = wrk->hw_txq_in;
			} else {
        			wrk->hw_txq_in->m_nextpkt = NULL;
			}
                        return;
                } /* end of if(no_space == TRUE) */
                wrk->tx_desc_p[i]->buf_count     = TOENDIANW((ushort) (cnt));
        } /* end of else */

        /*
         * Calls ndd_trace if it is enabled
         */
        if (p_ndd->ndd_trace) {
                (*(p_ndd->ndd_trace))(p_ndd, wrk->hw_txq_in,
                         wrk->hw_txq_in->m_data, p_ndd->ndd_trace_arg);
        }

        /*
         * Gives the buffer descriptor address to the adapter
         */
        PIO_PUTLRX(ioa + wrk->lfda, (int)wrk->tx_dma_desc_p[i]);
        wrk->tx_frame_pending++;
        wrk->sw_txq_len--;
        wrk->sw_txq_first  = txq_first;
        INC_DESC(wrk->tx_elem_next_in, MAX_TX_LIST);

  } /* end of while */
  TRACE_SYS(STOK_TX, "TssE", p_dev_ctl, wrk->tx_elem_next_in, 
						wrk->tx_elem_next_out);
}

/*****************************************************************************/
/*
 * NAME: stok_tx_done
 *
 * FUNCTION: Handle any completion code for a transmit descriptor after the
 *	     transmit has completed
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * INPUT: 
 *	p_dev_ctl - pointer to device control structure
 *      ioa       - IO address 
 *      misr_reg  - MISR register 
 *
 * CALLED FROM:
 *  	rw_intr
 *
 * RETURNS: 
 *
 */
/*****************************************************************************/

void stok_tx_done (
stok_dev_ctl_t  *p_dev_ctl,
int             ioa,
int		misr_reg,
tx_t		*wrk)
{
  ulong                           status; /* status of transmit         */
  int                             rc, i, j, out, First, pio_rc, cnt, len;
  int                             ipri, frame_pending;
  ndd_t                           *p_ndd = (ndd_t *)&(NDD);
  int                             first, last;
  ushort                          bmctl, bctl, buf_count;
  ulong                           dbas, dba, lfda, ct, elem_next_out = 0;
  struct  	mbuf    	  *mbufp,  *free_buf = NULL, *p_buf;

  TRACE_SYS(STOK_TX, "TtdB", p_dev_ctl, 0, 0); 
  ipri = disable_lock(PL_IMP, &TX_LOCK);

  /*
   * Stops the watchdog timer 
   */
  w_stop(&TXWDT);

  /*
   * Tx done interrupt occurred - process it 
   * increment RAS * counters for transmit interrupts processed
   */
  if (NDD.ndd_genstats.ndd_xmitintr_lsw == ULONG_MAX) {
          NDD.ndd_genstats.ndd_xmitintr_msw++;
  }
  NDD.ndd_genstats.ndd_xmitintr_lsw++;

  while (wrk->tx_frame_pending > 0) {

        out = wrk->tx_elem_next_out;

        /* 
	 * Gets the status from the tx buffer descriptor and verify 
         * the status.  (The description of status field are defined
         * in hardward Functional Specification )
         */
        status  = TOENDIANL(wrk->tx_desc_p[out]->xmit_status);

	/*
	 * Checks the status of the frame.  If something wrong, cleanup and
         * try to retransmit that frame one more time
         */
        if (status & ERR_STATUS) { 
	/* Something went wrong with the transmit package */

 		NDD.ndd_genstats.ndd_oerrors++;
                if (status & TX_BUS_ERR) { 
		 	stok_logerr(p_dev_ctl, ERRID_STOK_DMAFAIL, __LINE__, 
					__FILE__, WRK.dma_channel, status, out);

  		}

                if (status & TX_UNDERRUN) { /* Tx underrun */
                          DEVSTATS.xmit_underrun++;
  		}
  	/* 
	 * The transmit frame was processed 
	 */ 
        } else if (status & BUF_PROCESS) {

        	/* For netpmon performance tool */
        	TRACE_SYS(STOK_TX, TRC_WEND, p_dev_ctl->seq_number,
                	(int)wrk->sw_txq_first, wrk->tx_frame_pending); 

		/*
                 * Updates the standard counter 
		 */
                if (NDD.ndd_genstats.ndd_opackets_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_opackets_msw++;
		}
                NDD.ndd_genstats.ndd_opackets_lsw++;

                if ((ULONG_MAX - wrk->hw_txq_out->m_pkthdr.len) <
                        NDD.ndd_genstats.ndd_obytes_lsw) {
                        NDD.ndd_genstats.ndd_obytes_msw++;
		}
                NDD.ndd_genstats.ndd_obytes_lsw+= wrk->hw_txq_out->m_pkthdr.len;

                /*
                 * Determines if it's a broadcast or multicast packet
                 */
  		if (wrk->hw_txq_out->m_flags & M_MCAST)  {
                  	TOKSTATS.mcast_xmit++;
		}
  		if (wrk->hw_txq_out->m_flags & M_BCAST) {
                  	TOKSTATS.bcast_xmit++;
		}

        } else {
                break; /* No final status at all */
        }

	/*
	 * Based on the length of packet or length of each mbuf to update
         * the transmit counter.
         * For packet_len/mbuf_len less than 2K bytes, update the 
         * wrk->tx_buf_use_count.  Otherwise update the wrk->tx.dma_use_count
         */
	mbufp = wrk->hw_txq_out;
	if (mbufp->m_pkthdr.len > HAFT_PAGE) {
		while (mbufp) {
			if (mbufp->m_len > HAFT_PAGE) {
				len = 0;
				while (len < mbufp->m_len) {
					D_UNMAP_PAGE(WRK.dma_channel, 
				       		&wrk->tx_dma_addr
							[wrk->tx_dma_next_out]);
					INC_DESC(wrk->tx_dma_next_out, 
						 MAX_DMA_LIST);
					wrk->tx_dma_use_count--;

					if (len == 0) {
						len = PAGESIZE - (mtod(mbufp,int) & 0xFFF);
					} else {
						len += PAGESIZE;
					}
				}
			} else {
				wrk->tx_buf_use_count--;
			}
			mbufp = mbufp->m_next;
		}
	} else {
		wrk->tx_buf_use_count--;
	}

	/*
         * Marks transmit element as free 
	 */
        mbufp = wrk->hw_txq_out;
        while(mbufp->m_next != NULL) {
                mbufp = mbufp->m_next;
        }
        mbufp->m_next = free_buf;
        free_buf = wrk->hw_txq_out;
	wrk->hw_txq_out = wrk->hw_txq_out->m_nextpkt;

        wrk->tx_frame_pending--;
        INC_DESC(wrk->tx_elem_next_out, MAX_TX_LIST);

  } /* end of while */

  /* 
   * Checks if any package is pending in Tx queue, if they are then try
   * to transmit them
   */
  stok_send(p_dev_ctl, wrk, ioa);

  /* 
   * Checks if need to start the watch dog timer
   */
  if ((TX1_FRAME_PENDING) | (TX2_FRAME_PENDING)) {
       	w_start (&(TXWDT));
  }

  unlock_enable(ipri, &TX_LOCK);

  if (free_buf) {
        m_freem(free_buf);
  }

  TRACE_SYS(STOK_TX, "TtdE", p_dev_ctl, 0, 0);
}

/*****************************************************************************/
/*
 * NAME: stok_priority
 *
 * FUNCTION: write function for kernel
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
 *      p_mbuf          - pointer to a mbuf (chain) for outgoing packets
 *
 * RETURNS:
 *      0 - successful
 *      EAGAIN - transmit queue is full
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN - device is down
 */
/*****************************************************************************/
int stok_priority ( 
  ndd_t          *p_ndd,
  struct mbuf    *p_mbuf)
{

  stok_dev_ctl_t   	*p_dev_ctl = (stok_dev_ctl_t *)(p_ndd->ndd_correlator);

  return(stok_common_output(p_dev_ctl, p_mbuf, &TX1));
}

/*****************************************************************************/
/*
 * NAME: stok_output
 *
 * FUNCTION: write function for kernel
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
 *      p_mbuf          - pointer to a mbuf (chain) for outgoing packets
 *
 * RETURNS:
 *      0 - successful
 *      EAGAIN - transmit queue is full
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN - device is down
 */
/*****************************************************************************/
int stok_output ( 
  ndd_t          *p_ndd,
  struct mbuf    *p_mbuf)
{
  stok_dev_ctl_t   	*p_dev_ctl = (stok_dev_ctl_t *)(p_ndd->ndd_correlator);

  return(stok_common_output(p_dev_ctl, p_mbuf, &TX2));
}

/*****************************************************************************/
/*
 * NAME: stok_common_output
 *
 * FUNCTION: write function 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      stok_output and stok_priority
 *
 * CALLS TO
 *      stok_send
 *
 * RETURNS:
 *      0 - successful
 *      EAGAIN - transmit queue is full
 *      ENETUNREACH - device is currently unreachable
 *      ENETDOWN - device is down
 */
/*****************************************************************************/
int stok_common_output ( stok_dev_ctl_t *p_dev_ctl,
	        	 struct mbuf	*p_mbuf,
  	        	 tx_t    	*wrk)
{


  int          	i,rc,ioa, pio_rc;
  int          	room;
  struct  mbuf 	*mbufp;
  struct  mbuf 	*buf_to_free, *free_buf;
  struct  mbuf 	*p_last;
  int 		ipri;
  ndd_t         *p_ndd = &(NDD);
  struct  mbuf  *buf_to_count;

  TRACE_SYS(STOK_TX, "TxcB", (ulong)p_dev_ctl, (ulong)p_mbuf, wrk);
  /*
   * Determines if it's a broadcast or multicast packet and update the MIB
   * counter : ifHCOutUcastPkts, ifHCOutMulticastPkts, ifHCOutBroadcastPkts.
   */
  buf_to_count = p_mbuf;
  while (buf_to_count) {
        if (buf_to_count->m_flags & M_MCAST)  {
                if (NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_ifOutMcastPkts_msw++;
                }
                NDD.ndd_genstats.ndd_ifOutMcastPkts_lsw++;
        } else if (buf_to_count->m_flags & M_BCAST) {
                if (NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_ifOutBcastPkts_msw++;
                }
                NDD.ndd_genstats.ndd_ifOutBcastPkts_lsw++;
        } else {
                if (NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw == ULONG_MAX) {
                        NDD.ndd_genstats.ndd_ifOutUcastPkts_msw++;
                }
                NDD.ndd_genstats.ndd_ifOutUcastPkts_lsw++;
        }
        buf_to_count = buf_to_count->m_nextpkt;
  }

  ipri = disable_lock(PL_IMP, &TX_LOCK);

  if (p_dev_ctl->device_state != OPENED) {
  	unlock_enable(ipri, &TX_LOCK);
  	if (p_dev_ctl->device_state == DEAD) {
  		TRACE_BOTH(STOK_ERR, "Txc1", ENETDOWN, 0, 0);
  		return(ENETDOWN);
  	} else {
  		TRACE_BOTH(STOK_ERR, "Txc2", ENETUNREACH, 0, 0);
  		return(ENETUNREACH);
  	}
  }

  /*
   *  If the txq is full, return EAGAIN. Otherwise, queue as
   *  many packets onto the transmit queue and free the
   *  rest of the packets, return no error.
   */
  if (wrk->sw_txq_len == DDS.xmt_que_sz) {

  	unlock_enable(ipri, &TX_LOCK);
  	buf_to_free = p_mbuf;
        while (buf_to_free) {
  		NDD.ndd_genstats.ndd_xmitque_ovf++;
                buf_to_free = buf_to_free->m_nextpkt;
        }
  	TRACE_SYS(STOK_TX,"Txc3",p_dev_ctl,NDD.ndd_genstats.ndd_xmitque_ovf, 0);
  	return(EAGAIN);
  } else {
  	room = DDS.xmt_que_sz - wrk->sw_txq_len;

  	buf_to_free = p_mbuf;
  	while (room && buf_to_free) {
  		p_last = buf_to_free;
  		wrk->sw_txq_len++;
  		room--;
  		buf_to_free = buf_to_free->m_nextpkt;

        	/* For netpmon performance tool */
        	TRACE_SYS(STOK_TX, TRC_WQUE, p_dev_ctl->seq_number, 
				(int)buf_to_free, room); 
  	}

  	if (wrk->sw_txq_first) {
  		wrk->sw_txq_last->m_nextpkt = p_mbuf;
  	} else {
  		wrk->sw_txq_first = p_mbuf;
        }
	
  	wrk->sw_txq_last = p_last;
  	wrk->sw_txq_last->m_nextpkt = NULL;
  }

  /*
   * wrk->sw_txq_len == # of packets in software queue 
   * wrk->tx_frame_pending == # of packets in hardware queue
   */
  if ((int)&TX2 == (int)wrk) {
  	if ((wrk->sw_txq_len + wrk->tx_frame_pending) >
       	             NDD.ndd_genstats.ndd_xmitque_max) {
      		NDD.ndd_genstats.ndd_xmitque_max =
              		wrk->sw_txq_len + wrk->tx_frame_pending;
  	}
  } else {
  	if ((wrk->sw_txq_len + wrk->tx_frame_pending) > TOKSTATS.reserve2) {
      		TOKSTATS.reserve2 = wrk->sw_txq_len + wrk->tx_frame_pending;
  	}
  }

  ioa = (int)iomem_att(&WRK.iomap);
  stok_send(p_dev_ctl, wrk, ioa);
  iomem_det((void *)ioa);                      /* restore I/O Bus      */

  /* 
   * At least one tx buffer is waiting for transfer 
   */

  if ((TX1_FRAME_PENDING) | (TX2_FRAME_PENDING)) {
  	w_start (&(TXWDT));
  }

  unlock_enable(ipri, &TX_LOCK);

  if (buf_to_free) {
        free_buf = NULL;
        while (buf_to_free)
        {
                NDD.ndd_genstats.ndd_xmitque_ovf++;
                p_ndd->ndd_genstats.ndd_opackets_drop++;
                mbufp = buf_to_free;
                while(mbufp->m_next != NULL) {
                        mbufp = mbufp->m_next;
                }
                mbufp->m_next = free_buf;
                free_buf = buf_to_free;

                buf_to_free = buf_to_free->m_nextpkt;
        }
        m_freem(free_buf);
  }

  TRACE_SYS(STOK_TX, "TxcE", p_dev_ctl, 0, 0);
  return(0);
}

