static char sccsid[] = "@(#)49  1.3  src/rspc/kernext/pci/stok/stok_intr.c, pcitok, rspc41J, 9516A_all 4/14/95 05:13:14";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: rw_intr
 *		stok_intr
 *		stok_lan_status
 *		stok_mac_recv
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
/*
 * NAME:     stok_intr
 *
 * FUNCTION: Skyline driver interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * CALLED FROM:
 *      The FLIH
 *
 * CALLS TO:
 *      rw_intr
 *      srb_response
 *      stok_enter_limbo
 *      stok_bug_out
 *      stok_mac_recv
 *
 * INPUT:
 *      p_ihs           - point to the interrupt structure.
 *
 * RETURNS:
 *      INTR_SUCC - our interrupt
 *      INTR_FAIL - not our interrupt
 */
/*****************************************************************************/
stok_intr(
struct intr *p_ihs)  /* This also points to device control area */
{
  stok_dev_ctl_t   *p_dev_ctl = (stok_dev_ctl_t *)p_ihs;
  ndd_statblk_t    stat_blk;   /* status block */
  ndd_t           *p_ndd = &(NDD);
  ushort           sisr_status_reg;
  ushort           cmd_reg, bctl;
  int              rc = INTR_FAIL;             /* Return code            */
  int              ioa, i;
  int              ipri;
  int              fr_len;
  int              buf_ptr;
  ushort           data[10];
  ushort           lan_status;
  ushort 	   lapwwc; 
  ushort 	   command; 

  TRACE_DBG(STOK_OTHER, "SinB", p_dev_ctl,(ulong)p_ihs,p_dev_ctl->device_state);

  ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  if ((p_dev_ctl->device_state == CLOSED) ||
      (p_dev_ctl->device_state == SUSPEND_STATE) ||
      (p_dev_ctl->device_state == DEAD)) {
                  TRACE_BOTH(STOK_ERR, "Sin1", p_dev_ctl, INTR_FAIL, 0);
                  unlock_enable(ipri, &SLIH_LOCK);
                  return(INTR_FAIL);
  }

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)iomem_att(&WRK.iomap);
  /* 
   * Gets the system interrupt status register for this card            
   */
  PIO_GETSRX(ioa + SISR, &sisr_status_reg);
  if (WRK.pio_rc) {
        TRACE_BOTH(STOK_ERR, "Sin2", p_dev_ctl, WRK.pio_rc, 0);
  	iomem_det((void *)ioa);         /* restore I/O Bus                   */
        stok_bug_out(p_dev_ctl, NDD_HARD_FAIL,NDD_PIO_FAIL,0,TRUE,FALSE,FALSE);
        unlock_enable(ipri, &SLIH_LOCK);
        return(INTR_FAIL);
  }

  /* 
   * Processes interrupt from the adapter 
   */
  while (sisr_status_reg & SISR_MSK) {

	/*
         * Valid adapter interrupt, sets retcode to success           
	 */
        rc = INTR_SUCC;
        /*********************************************************************/
        /* 
	 * We have a Master interrupt status register intr :   
  	 * when one or more of the interrupt status condition bits in the Bus 
	 * Master Interrupt Status Register is a ONE.
         */
        /*********************************************************************/

        if (sisr_status_reg & MISR_INT) {
                PIO_GETSRX(ioa + MISR, &cmd_reg);
		/*
		 * To clear the MISR interrupt bit in the System Interrupt 
		 * Status Register, we must clear the Bus Master Interrupt 
		 * Status Register.
                 */
                PIO_PUTSRX( ioa + MISR, ~cmd_reg);
                if ((p_dev_ctl->device_state == OPENED) ||
                    (p_dev_ctl->device_state == CLOSE_PENDING)) {
			/* Handle the Tx or Rx related interrupt */
                        if (rw_intr(p_dev_ctl, cmd_reg, ioa) == FALSE) {
				break;
			}
                }

        }

        /*********************************************************************/
        /* 
	 * SRB response interrupt : The Adapter has recognized a SRB request 
         * and has set the return code in the SRB
         */
        /*********************************************************************/
        else if (sisr_status_reg & SRB_RSP) {

                if ((p_dev_ctl->device_state == OPEN_PENDING) |
                	(p_dev_ctl->device_state == LIMBO)) {
			/*
                         * Gets offset for SRB  
			 */
                       if (WRK.srb_address == 0) {
                        	PIO_GETSRX(ioa + LAPWWO, &WRK.srb_address);
			}
                }
                srb_response(p_dev_ctl, ioa);
                PIO_PUTSRX(ioa + SISR_RUM, ~SRB_RSP); /*RUM.SISR.bit5*/
        }

        /*****************************************************/
        /* LAP access violation error interrupt              */
        /*****************************************************/
        else if (sisr_status_reg & LAP_ACC) {
		if (WRK.limbo_err != 10) {
                	stok_logerr(p_dev_ctl, ERRID_STOK_BUS_ERR, __LINE__,
                              __FILE__, NDD_BUS_ERROR, sisr_status_reg, 0);
		}
		WRK.limbo_err = 10;
                stok_enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_BUS_ERROR, 
				sisr_status_reg, 0);
        }

        /*****************************************************/
        /* LAP data parity or BUs parity error interrupt     */
        /*****************************************************/
        else if ((sisr_status_reg & LAP_PRTY) |
        	(sisr_status_reg & MC_PRTY)) {
		if (WRK.limbo_err != 11) {
                	stok_logerr(p_dev_ctl,ERRID_STOK_BUS_ERR, __LINE__,
                              __FILE__, NDD_BUS_ERROR, sisr_status_reg, 0);
		}
		WRK.limbo_err = 11;
                stok_enter_limbo(p_dev_ctl, TRUE, TRUE, 0, NDD_BUS_ERROR, 
				sisr_status_reg, 0);
        }

        /*****************************************************/
        /* Adapter check interrupt                           */
        /*****************************************************/
        else if (sisr_status_reg & ADAPT_CHK) {

                PIO_GETSRX(ioa + LAPWWC, &lapwwc);
  		PIO_PUTSRX(ioa + LAPE, 0x00);
  		PIO_PUTSRX(ioa + LAPA, lapwwc); 
  		for (i=0; i < 3; i++) {
        		PIO_GETSX(ioa + LAPD_I, &data[i]);
  		}
                PIO_PUTSRX(ioa + SISR_RUM, ~ADAPT_CHK);/*RUM.SISR.bit8*/

		if (WRK.limbo_err != 12) {
                	stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__,
                        	__FILE__, data[0], data[1], data[2]);

		}
		WRK.limbo_err = 12;
                stok_enter_limbo(p_dev_ctl, TRUE, TRUE, 0, 
					NDD_ADAP_CHECK, sisr_status_reg, 0);

        }
        /**********************************************************************/
        /* 
	 * ASB Free interrupt : The adapter has read the ARB response provided 
	 * in the  ASB, and ASB is available for other response
         */
        /**********************************************************************/
        else if (sisr_status_reg & ASB_FREE) {
                PIO_PUTSRX(ioa + SISR_RUM, ~ASB_FREE);/*RUM.SISR.bit4*/
        }
        /*********************************************************************/
        /* 
	 * ARB command interrupt : The ARB comtains a command for the drivers
         * to act on
         */
        /*********************************************************************/
        else if (sisr_status_reg & ARB_CMD) {

                PIO_PUTSRX(ioa + LAPE, 0x00);
                PIO_PUTSRX(ioa + LAPA, WRK.arb_address);
                for (i = 0; i < 6; i++)
		{
                        PIO_GETSX(ioa + LAPD_I, &data[i]);
		}
                PIO_PUTCX(ioa + SISR_RUM, ~ARB_CMD); /* SUM.SISR.bit3 */
                command = data[0] >> 8;

                if (command == RECEIVE_DATA) {
			fr_len = data[5];
			buf_ptr = data[3];
                        stok_mac_recv(p_dev_ctl, fr_len, buf_ptr);
                } else if (command == LAN_STATUS_CHANGE) {
			lan_status = data[3];
			if (stok_lan_status(p_dev_ctl, lan_status)) {
				break;
			}
                }

                PIO_PUTCX(ioa + LISR_SUM, ARB_FREE); /*SUM.LISR.bit1*/
        }
        /*****************************************************/
        /* 
         * TRB response interrupt : The adapter has recognized a transmit 
	 * request and set the return code in the TRB
         */
        /*****************************************************/
        else if (sisr_status_reg & TRB_RSP) {
                PIO_PUTSRX(ioa + LAPE, 0x00);
                PIO_PUTSRX(ioa + LAPA, WRK.trb_address);
                for (i=0; i < 4; i++) {
                        PIO_GETSX(ioa + LAPD_I, &data[i]);
                }
                PIO_PUTCX(ioa + SISR_RUM, ~TRB_RSP);/* RUM.SISR.bit2 */
        }

        /* 
	 * Gets the system interrupt status register for this card    
	 */
        PIO_GETSRX(ioa + SISR, &sisr_status_reg);
  } /* end while adapter is still interrupting                         */

  iomem_det((void *)ioa);               /* restore I/O Bus                   */
  if (WRK.pio_rc) {
       	stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL,0,TRUE,FALSE,FALSE);
       	TRACE_BOTH(STOK_ERR,"Sin3", p_dev_ctl, WRK.pio_rc, 0);
	return(ERROR);
  }
  TRACE_DBG(STOK_OTHER, "SinE", p_dev_ctl, rc, 0);
  unlock_enable(ipri, &SLIH_LOCK);
  return(rc);

} /* end stok_intr                                                            */


/*****************************************************************************/
/*
 * NAME:     rw_intr
 *
 * FUNCTION: Skyline driver read/write interrupt routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * INPUT:
 *  	p_dev_ctl - pointer to device control structure.
 *      cmd_reg   - value in the interrupt register.
 *
 * CALLED FROM:
 *      stok_intr
 *
 * CALLS TO:
 *      stok_rx
 *      stok_tx_done
 *
* RETURN:  
*	0 = Successful return
*      >1 = Error return
*
*/
/*****************************************************************************/
int rw_intr (
stok_dev_ctl_t  *p_dev_ctl,
int        	cmd_reg,
int		ioa)
{
  register int       i, rc, index;
  volatile tx_list_t xmitlist;
  xmit_elem_t        *xelm;         /* transmit element structure */
  ushort             bmctl;
  ulong              bda, fda, stat;
  volatile rx_list_t recvlist;

  /*********************************************/
  /* Test if a receive interrupt occurred      */
  /*********************************************/
  TRACE_DBG(STOK_OTHER, "SrwB", p_dev_ctl, cmd_reg, 0);
  if ((cmd_reg & RECEIVE_MSK) && (p_dev_ctl->device_state == OPENED)) {
  /* Receive interrupt occurred - process it */
        /* 
	 * Increments counters for receive interrupts processed 
	 */
        if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX) {
                NDD.ndd_genstats.ndd_recvintr_msw++;
	}
        NDD.ndd_genstats.ndd_recvintr_lsw++;

        /* 
	 * Receives a frame with no status posted 
	 */
        if (cmd_reg & Rx_NOSTA) {
                /*
                 * Checks if any status in the RxStat register, and
                 * update the status field in the buffer descriptor
                 */
                PIO_GETLX(ioa + RxStat_L,  &stat);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(STOK_ERR, "Srw1", p_dev_ctl, WRK.pio_rc, 0);
        		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
			return (ERROR);
  		}
                if (stat) {
                        PIO_GETLRX(ioa + RxBDA_L,  &bda);
  			if (WRK.pio_rc) {
  				TRACE_BOTH(STOK_ERR, "Srw2", p_dev_ctl,
							WRK.pio_rc, 0);
        			stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
					NDD_PIO_FAIL, 0, TRUE, FALSE, FALSE);
				return (ERROR);
  			}

                        for (i=0; i < DDS.rx_que_sz; i++) {
                                if (bda == (int)WRK.rx_dma_desc_p[i]) {
                                        break;
				}
                        } /* end of for */

                        stat |= BUF_PROCES; /* Set BUF PROCESS bit*/

                        /*  Updates the receive status
                         */
                        WRK.rx_desc_p[i]->rx_status = stat;

                }
        } /* end of Rx_NOSTA */

	/*
         * Receives a frame 
	 */
        if (cmd_reg & (Rx_EOF)) {
                /* Process the receive frames */
                stok_rx(p_dev_ctl, cmd_reg, ioa);
        }

        /* 
	 * Receives channel halt/receive channel with no buffer available
         * in the system
        */
        if (cmd_reg & (Rx_HALT | Rx_NBA)) {
                /*
                 * Frees any mbuf that have allocated for the receive frame but
                 * have not yet passed up to the higher layer.  
                 */
                if (WRK.mhead) {
                        m_freem(WRK.mhead);
                }

                /*
                 * Resets the Rx channel disable in BMCTL only when it is set
                 */
                for (i=0; i < 5; i++) {
                        PIO_GETSRX(ioa + BMCTL_SUM, &bmctl);
                        if (bmctl & RX_DISABLE) {
                                PIO_PUTSRX(ioa + BMCTL_SUM, ~RX_DISABLE);
				break;
                        }
                        io_delay(1000);
                }

		stok_rx_enable(p_dev_ctl);

                /* 
	 	 * Allows the adapter generate the HALT/NBA intr again 
		 */
                PIO_PUTSRX( ioa + MISRMask, RV_UNMASK);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(STOK_ERR, "Srw3",p_dev_ctl, WRK.pio_rc, 0);
        		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
			return (ERROR);
  		}
        }

  } /* endif for RECEIVE  interrupt             */

  /*****************************************************/
  /* Adapter Tx interrupt has occurred - channel 1     */
  /*****************************************************/
  /* Test if a transmit 1 interrupt occurred             */
  if (cmd_reg & XMIT_DONE_MSK_1) {
        if (cmd_reg & TX1_NOSTA) {
		/*
		 * First gets the current transmit status and current 
		 * transmit descriptor. Then finds the index of current 
		 * transmit descriptor in the Tx descriptor list and update 
		 * the status field.
		 */
                PIO_GETLX(ioa + TX1Stat_L,  &stat);
                PIO_GETLRX(ioa + TX1FDA_L,  &fda);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(STOK_ERR, "Srw4",p_dev_ctl, WRK.pio_rc, 0);
        		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
			return(ERROR);
  		}

                for (i=0; i < MAX_TX_LIST; i++) {
                        if (fda == (int)TX1.tx_dma_desc_p[i]) {
                        	break;
			}
                } /* end of for */

                stat |= BUF_PROCES; /* Set BUF PROCESS bit*/
                TX1.tx_desc_p[i]->xmit_status = stat;

        } /* end of TX1_NOST */

        stok_tx_done (p_dev_ctl, ioa, cmd_reg, &TX1);

  } /* endif for transmit 1 interrupt               */

  /*****************************************************/
  /* Adapter Tx interrupt has occurred - channel 2     */
  /*****************************************************/
  /* Test if a transmit 2 interrupt occurred           */
  if (cmd_reg & XMIT_DONE_MSK_2) {

        if (cmd_reg & TX2_NOSTA) {
		/*
		 * First gets the current transmit status and current 
		 * transmit descriptor. Then finds the index of current 
		 * transmit descriptor in the Tx descriptor list and update 
		 * the status field.
		 */
                PIO_GETLX(ioa + TX2Stat_L,  &stat);
                PIO_GETLRX(ioa + TX2FDA_L,  &fda);
  		if (WRK.pio_rc) {
  			TRACE_BOTH(STOK_ERR, "Srw5",p_dev_ctl, WRK.pio_rc, 0);
        		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
							TRUE, FALSE, FALSE);
			return(ERROR);
  		}

                for (i=0; i < MAX_TX_LIST; i++) {
                        if (fda == (int)TX2.tx_desc_p[i]) {
                        	break;
			}
                } /* end of for */

                stat |= BUF_PROCES; /* Set BUF PROCESS bit*/
                TX2.tx_desc_p[i]->xmit_status = stat;

        } /* end of TX2_NOST */

        stok_tx_done (p_dev_ctl, ioa, cmd_reg, &TX2);

  } /* endif for transmit 2 interrupt               */

  TRACE_SYS(STOK_OTHER, "SrwE", p_dev_ctl, cmd_reg, 0);
  return (OK);

}

/*****************************************************************************/
/*
* NAME: stok_mac_recv
*
* FUNCTION:  Receive the packets from the adapter.
*
* EXECUTION ENVIRONMENT: interrupt only
*
* INOUT:
*       p_dev_ctl - pointer to device control structure.
*       fr_len    - length of frame.
*       buf_ptr   - data pointer.
*
* CALLED FROM:
*       stok_intr
*
* RETURN:  
*	0 = Successful return
*      >1 = Error return
*
*/
/*****************************************************************************/
int stok_mac_recv (
stok_dev_ctl_t *p_dev_ctl,     /* point to the device control area */
uint          fr_len,
uint          buf_ptr)
{
  typedef       struct {
        uchar        cmd;
        uchar        rsv;
        uchar        retcode;
        uchar        rsv3[3];
        ushort       buf_ptr;
  } o_parm_t;

  o_parm_t      o_parm;
  ushort        *parm = (ushort *)&o_parm;
  struct mbuf   *m;
  ushort        *mem, id;
  ushort        recv_fs, buf_len, next_buf;
  ndd_t         *p_ndd = (ndd_t *)&(NDD);
  uint          lapd;
  uint          count = 0;
  int           ioa, i, x;
  uchar bcast1_addr[CTOK_NADR_LENGTH] = { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  uchar bcast2_addr[CTOK_NADR_LENGTH] = { 0xC0,0x00,0xFF,0xFF,0xFF,0xFF};

  TRACE_SYS(STOK_OTHER, "RmrB", (ulong)p_dev_ctl, fr_len, buf_ptr);
  /* For netpmon performance tool */
  TRACE_SYS(STOK_RV, TRC_RDAT, p_dev_ctl->seq_number, WRK.read_index, fr_len);

  parm  = (ushort * )&o_parm.cmd;

  /* 
   * Increments counters for receive interrupts processed 
   */
  if (NDD.ndd_genstats.ndd_recvintr_lsw == ULONG_MAX) {
        NDD.ndd_genstats.ndd_recvintr_msw++;
  }
  NDD.ndd_genstats.ndd_recvintr_lsw++;

  /*
   * Gets access to the I/O bus to access I/O registers                 
   */
  ioa = (int)iomem_att(&WRK.iomap);

  /*
   * If the data is less than 256 bytes then get a small mbuf instead of
   * a cluster.
   */
  if (fr_len <= (MHLEN)) {
        m = m_gethdr(M_DONTWAIT, MT_HEADER);
  } else {
        m = m_getclust(M_DONTWAIT, MT_HEADER);
  }

  if (m == NULL) {
        NDD.ndd_genstats.ndd_nobufs++;
  } else {

        /* 
	 * Copys data from adapter buffer to mbuf  
	 */
  	m->m_flags |= M_PKTHDR;
  	m->m_nextpkt = NULL;
        m->m_len = fr_len;
        mem = MTOD( m, ushort * );
        next_buf = buf_ptr;
        while (next_buf) {
                /* (WRITE).LAPA = Addr of first adapter receive buf */
                PIO_PUTSRX(ioa + LAPA, next_buf);
                lapd = ioa + LAPD_I;

                PIO_GETSX(lapd, &next_buf);
                PIO_GETSX(lapd, &recv_fs);
                PIO_GETSX(lapd, &buf_len);
                for (i=0; i < buf_len; i++) {
                        PIO_GETSX(lapd, mem++);
                }
                count += buf_len;
        }

  	if (WRK.pio_rc) {
  		TRACE_BOTH(STOK_ERR,"Rmr1", p_dev_ctl, WRK.pio_rc, 0);
  		iomem_det((void *)ioa);  /* restore I/O Bus  */
        	stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_PIO_FAIL, 0, 
						TRUE, FALSE, FALSE);
		return(WRK.pio_rc);
  	}

        if (count != fr_len) {
  		iomem_det((void *)ioa);  /* restore I/O Bus  */
                m_freem(m);
                return (fr_len - count);
        }

        m->m_pkthdr.len = m->m_len = count;

        /* Checks if broadcast or multicast */
        if (*(mtod(m, caddr_t) + 2) & MULTI_BIT_MASK) {
                if ((SAME_NADR((mtod(m, caddr_t) + 2), bcast1_addr)) |
                    (SAME_NADR((mtod(m, caddr_t) + 2), bcast2_addr)))
                {
                        TOKSTATS.bcast_recv++;
                        m->m_flags |= M_BCAST;
                } else {
                        TOKSTATS.mcast_recv++;
                        m->m_flags |= M_MCAST;
                }
        }
        /* For netpmon performance tool */
        TRACE_SYS(STOK_RV, TRC_RNOT, p_dev_ctl->seq_number,
                                          WRK.mhead, WRK.mhead->m_pkthdr.len);

        /* 
	 * Sends the frame up 
         */
        (*(p_ndd->nd_receive))(p_ndd, m);

        /* For netpmon performance tool */
        TRACE_SYS(STOK_RV, TRC_REND, p_dev_ctl->seq_number, WRK.read_index, 0);


        /* 
	 * Updates status counter 
	 */
        if (p_ndd->ndd_genstats.ndd_ipackets_lsw == ULONG_MAX)
                p_ndd->ndd_genstats.ndd_ipackets_msw++;
        p_ndd->ndd_genstats.ndd_ipackets_lsw++;

        if ((ULONG_MAX - fr_len) < p_ndd->ndd_genstats.ndd_ibytes_lsw)
                p_ndd->ndd_genstats.ndd_ibytes_msw++;
        p_ndd->ndd_genstats.ndd_ibytes_lsw += fr_len;

  }

  /* 
   * Builds an adapter status block (ASB) to respond to the ARB  
   */
  o_parm.cmd     = RECEIVE_DATA;
  o_parm.retcode = 0;
  o_parm.buf_ptr = buf_ptr;

  PIO_PUTSRX(ioa + LAPE, 0x00);
  PIO_PUTSRX(ioa + LAPA, WRK.srb_address);
  for (i = 0; i < 4; i++) {
          PIO_PUTSX(ioa + LAPD_I, *(parm + i));
  }

  /* 
   * Sets under Mask LISR.bit4 
   */
  PIO_PUTCX(ioa + LISR_SUM, ASB_RSP); /* SUM.LISR.bit.4 */
  iomem_det((void *)ioa);               /* restore I/O Bus                   */
  if (WRK.pio_rc) {
 	TRACE_BOTH(STOK_ERR,"Rmr2", p_dev_ctl, WRK.pio_rc, 0);
	return(WRK.pio_rc);
  }
  TRACE_DBG(STOK_OTHER, "RmrE", (ulong)p_dev_ctl, fr_len, buf_ptr);
  return(OK);

} /* end stok_mac_recv routine                                                */

/*****************************************************************************/
/*
* NAME: stok_lan_status
*
* FUNCTION:  Receive the packets from the adapter.
*
* EXECUTION ENVIRONMENT: interrupt only
*
* INPUT:
*       p_dev_ctl     - pointer to device control structure.
*       lan_status    - length of frame.
*
* CALLED FROM:
*       stok_intr
*
* CALLS TO:
*       stok_enter_limbo
*       stok_bug_out
*
* RETURN:  
*	OK = Successful return
*       ERROR = Error return
*
*/
/*****************************************************************************/
int stok_lan_status (
stok_dev_ctl_t *p_dev_ctl,     /* point to the device control area */
uint         lan_status)
{

  ndd_statblk_t    stat_blk;   /* status block */
  ndd_t           *p_ndd = &(NDD);

  TRACE_SYS(STOK_OTHER, "SlsB", (ulong)p_dev_ctl, lan_status, 0);
  if ((lan_status & LOBE_WIRE_FAULT) | (lan_status & CABLE_NOT_CONNECTED)) {
	if (WRK.limbo_err != 14) {
		stok_logerr(p_dev_ctl, ERRID_STOK_WIRE_FAULT, 
				   __LINE__, __FILE__, lan_status, 0, 0);
	}
	WRK.limbo_err = 14;
	w_stop (&(LANWDT));
        TOKSTATS.lobewires++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_LOBEWIREFAULT;

      	stok_enter_limbo(p_dev_ctl, TRUE, TRUE, 30,TOK_WIRE_FAULT,0,lan_status);
        TRACE_BOTH(STOK_ERR,"Sls1",p_dev_ctl, lan_status, 0);
	return (ERROR);
  }

  if (lan_status & AUTO_REMOVAL_ERROR) {
	stok_logerr(p_dev_ctl, ERRID_STOK_AUTO_RMV, 
			__LINE__, __FILE__, lan_status, 0, 0);
	w_stop (&(LANWDT));
        TOKSTATS.removes++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_AUTOREMOVAL_ERR;

	if (WRK.limbo_err == 15) {
		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
				NDD_AUTO_RMV, 0, TRUE, FALSE, FALSE);
	} else {
         	stok_enter_limbo(p_dev_ctl,TRUE,TRUE,30,NDD_AUTO_RMV,0,
					lan_status);
	}
	WRK.limbo_err = 15;
       	TRACE_BOTH(STOK_ERR,"Sls2",p_dev_ctl, lan_status, 0);
	return(ERROR);
  }

  if (lan_status & REMOVE_RECEIVE) {
  /*
   * We have been kicked off the ring by the LAN manager.
   */
       	stok_logerr(p_dev_ctl, ERRID_STOK_RMV_ADAP, 
			__LINE__, __FILE__, lan_status, 0, 0);
	w_stop (&(LANWDT));
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_REMOVE_RX;

	if (WRK.limbo_err != 16) {
		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL,
				TOK_RMV_ADAP, 0, TRUE, FALSE, FALSE);
	} else {
       		stok_enter_limbo(p_dev_ctl, TRUE, TRUE, 30, TOK_RMV_ADAP, 
					0, lan_status);
	}
	WRK.limbo_err = 16;
       	TRACE_BOTH(STOK_ERR,"Sls3",p_dev_ctl, lan_status, 0);
	return(ERROR);
  }

  if (lan_status & SIGNAL_LOSS) {
	TOKSTATS.signal_loss++;
         MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_SIGNAL_LOSS;
  }

  if (lan_status & HARD_ERROR) {
        TOKSTATS.hard_errs++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_HARD_ERROR;

	if (!WRK.hard_err) {
        	/*
       		 * Pass the status block to demuxer
       		 */
       		 stat_blk.code = NDD_STATUS;
       		 stat_blk.option[0] = TOK_BEACONING;
       		 stat_blk.option[1] = 0;
       		 stat_blk.option[2] = lan_status;
       		 (*(NDD.nd_status))(p_ndd, &stat_blk);
		 WRK.hard_err = TRUE;
	}

       	TRACE_BOTH(STOK_ERR,"Sls4", p_dev_ctl,lan_status, 0);
  }

  if (lan_status & TRANSMIT_BEACON) {
       	stok_logerr(p_dev_ctl, ERRID_STOK_WIRE_FAULT, 
			__LINE__, __FILE__, lan_status, 0, 0);
        TOKSTATS.tx_beacons++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_TX_BEACON;

  }

  if (lan_status & SINGLE_STATION) {
	TOKSTATS.singles++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_SINGLESTATION;
  }

  if (lan_status & RING_RECOVERY) {
	TOKSTATS.recoverys++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_RINGRECOVERY;
	WRK.hard_err = FALSE;
  }

  if (lan_status & COUNTER_OVERFLOW){
        read_adapter_log(p_dev_ctl, TRUE);
  }

  if (lan_status & SR_BRIDGE_COUNTER_OVERFLOW) {
        read_adapter_log(p_dev_ctl, TRUE);
  }

  if (lan_status & SOFT_ERROR) {
        TOKSTATS.soft_errs++;
        MIB.Token_ring_mib.Dot5Entry.ring_status = TR_MIB_SOFT_ERR;
  }

  TRACE_SYS(STOK_OTHER, "SlsE", (ulong)p_dev_ctl, lan_status, 0);
  return(OK);
}
