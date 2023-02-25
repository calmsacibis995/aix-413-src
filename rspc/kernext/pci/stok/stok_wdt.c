static char sccsid[] = "@(#)55  1.3  src/rspc/kernext/pci/stok/stok_wdt.c, pcitok, rspc41J, 9520B_all 5/18/95 09:54:28";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: stok_bug_out
 *		stok_cmd_timeout
 *		stok_enter_limbo
 *		stok_lan_connected
 *		stok_re_open_adapter
 *		stok_start_recover
 *		stok_start_timeout
 *		stok_tx_buf_clearup
 *		stok_tx_timeout
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

extern stok_dd_ctl_t stok_dd_ctl;

/****************************************************************************/
/*
 * NAME: stok_bug_out
 *
 * FUNCTION:
 *     This function moves the device handler into the
 *     dead state.  A fatal error has just been detected.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine executes on the interrupt level or process thread.
 *
 * NOTES:
 *
 * (RECOVERY OPERATION:) Information describing both hardware and
 *      software error recovery.
 *
 */
/****************************************************************************/
stok_bug_out( stok_dev_ctl_t  *p_dev_ctl,
    		uint         option0,
    		uint         option1,
    		uint         option2,
		uint	     slih_lock,
		uint	     tx_lock,
		uint	     cmd_lock)
{
   
  int ipri;
  ndd_statblk_t    stat_blk;   /* status block */

  TRACE_BOTH(STOK_ERR, "bubB", (int)p_dev_ctl, 0, 0);

  /* Gets locks */
  if (slih_lock == FALSE) {
  	ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  }
  if (tx_lock == FALSE) {
   	disable_lock(PL_IMP, &TX_LOCK);
  }
  if (cmd_lock == FALSE) {
        disable_lock(PL_IMP, &CMD_LOCK);
  }

  TX1_FRAME_PENDING = 0;
  TX2_FRAME_PENDING = 0;

  if (p_dev_ctl->device_state != DEAD) {   
  	stok_tx_buf_clearup(p_dev_ctl, &TX2);
	if (DDS.priority_tx) {
  		stok_tx_buf_clearup(p_dev_ctl, &TX1);
	}

  	/*
  	 * Frees the Rx buffer queues
  	 */
  	if (WRK.mhead) {
        	m_freem(WRK.mhead);
  	}

  	/*
  	 * Stops the watchdog timers
  	 */
  	w_stop (&(CMDWDT));
  	w_stop (&(TXWDT));
  	w_stop (&(LIMWDT));
  	w_stop (&(LANWDT));
  	w_stop (&(HWEWDT));

  	reset_adapter(p_dev_ctl);

  	/*
  	 * update NDD frags 
  	 */
  	p_dev_ctl->device_state = DEAD;
 	NDD.ndd_flags &= ~(NDD_RUNNING | NDD_LIMBO);
  	NDD.ndd_flags |= NDD_DEAD;

  	/*
  	 *       Build the asynchronous status block
  	 */
  	stat_blk.code = NDD_HARD_FAIL;
  	stat_blk.option[0] = option0;
  	stat_blk.option[1] = option1;
  	stat_blk.option[2] = option2;
  	(*(NDD.nd_status))(&NDD, &stat_blk);

  	/* wakeup outstanding ioctl or open event */
       	e_wakeup((int *)&WRK.ctl_event);
  }
  TRACE_BOTH(STOK_ERR, "bugE", p_dev_ctl, 0, 0); 
  if (cmd_lock == FALSE) {
        unlock_enable(PL_IMP, &CMD_LOCK);
  }
  if (tx_lock == FALSE) {
  	unlock_enable(PL_IMP, &TX_LOCK);
  }
  if (slih_lock == FALSE) {
        unlock_enable(ipri, &SLIH_LOCK);
  }

}  /* end function stok_bug_out() */

/*****************************************************************************/
/*
 * NAME:     stok_start_timeout
 *
 * FUNCTION: Skyline driver error recover timeout
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
stok_start_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  stok_dev_ctl_t *p_dev_ctl = (stok_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(stok_dev_ctl_t, hwe_wdt));

  TRACE_BOTH(STOK_ERR, "SstB", (ulong)p_dev_ctl, 0, 0);

  if (p_dev_ctl->device_state != DEAD) {
  	stok_bug_out(p_dev_ctl, NDD_ADAP_CHECK, 0, 0, FALSE, FALSE, FALSE);
  	stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__,
                   __FILE__, NDD_ADAP_CHECK, NDD_LIMBO_EXIT, 0, FALSE);
  }

  TRACE_BOTH(STOK_ERR, "SstE", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     stok_lan_connected
 *
 * FUNCTION: Skyline driver lan connected timer
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
stok_lan_connected(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  stok_dev_ctl_t *p_dev_ctl = (stok_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(stok_dev_ctl_t, lan_wdt));
  ndd_statblk_t stat_blk;   /* status block */
  ndd_t         *p_ndd = &(NDD);
  int           ipri;
  int	        ioa;

  TRACE_SYS(STOK_OTHER, "SlnB", (ulong)p_dev_ctl, 0, 0);

  if (p_dev_ctl->device_state != DEAD) { 
 	/* Gets locks */
  	ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  	disable_lock(PL_IMP, &TX_LOCK);
  	disable_lock(PL_IMP, &CMD_LOCK);

        if (WRK.re_limbo) {
                WRK.re_limbo = FALSE;
  		unlock_enable(PL_IMP, &CMD_LOCK);
  		unlock_enable(PL_IMP, &TX_LOCK);
  		unlock_enable(ipri, &SLIH_LOCK);
		stok_enter_limbo(p_dev_ctl,TRUE,TRUE, 0, 0,0,0);
        } else {
        	/*
        	 * Updates the adapter device state & flags
        	 */
        	WRK.dev_stime = lbolt;
		WRK.limbo_err = 0;
        	p_dev_ctl->device_state = OPENED;
        	NDD.ndd_flags |= NDD_RUNNING;
        	stok_logerr(p_dev_ctl, ERRID_STOK_RCVRY_EXIT, __LINE__, 
				__FILE__,0,0,0);

                /*
                 * Enables the Rx & TX channel
                 */
  		ioa = (int)iomem_att(&WRK.iomap);
                PIO_PUTSRX(ioa + BMCTL_RUM, CHNL_ENABLE);
  		iomem_det((void *)ioa);     /* restore I/O Bus              */

                if (stok_rx_enable(p_dev_ctl)) {
                        TRACE_BOTH(STOK_ERR, "Sln1", p_dev_ctl,
                                      p_dev_ctl->device_state, 0);
                        return;
                }

        	/*
        	 * Pass the status block to demuxer
        	 */
       	 	NDD.ndd_flags &= ~NDD_LIMBO;
        	stat_blk.code = NDD_LIMBO_EXIT;
        	stat_blk.option[0] = NDD_CONNECTED;
        	(*(NDD.nd_status))(p_ndd, &stat_blk);
 
  		unlock_enable(PL_IMP, &CMD_LOCK);
  		unlock_enable(PL_IMP, &TX_LOCK);
  		unlock_enable(ipri, &SLIH_LOCK);
	}
  }

  TRACE_SYS(STOK_OTHER, "SlnE", (ulong)p_dev_ctl, 0, 0);


}

/*****************************************************************************/
/*
 * NAME:     stok_start_recover
 *
 * FUNCTION: Skyline driver recover timer
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
stok_start_recover(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  stok_dev_ctl_t *p_dev_ctl = (stok_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(stok_dev_ctl_t, lim_wdt));


  TRACE_SYS(STOK_OTHER, "SsrB", (ulong)p_dev_ctl, 0, 0);

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != CLOSE_PENDING)) {
  	/* start recover process */
  	if (stok_re_open_adapter(p_dev_ctl, TRUE, FALSE)) {
  		TRACE_BOTH(STOK_ERR, "Ssr1", (ulong)p_dev_ctl, 0, 0);
 		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_ADAP_CHECK, 0, 
				FALSE, FALSE, FALSE);
        	stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, __LINE__,
                     		__FILE__, NDD_HARD_FAIL, NDD_DEAD, OPENED);
  	}
  }
  TRACE_SYS(STOK_OTHER, "SsrE", (ulong)p_dev_ctl, 0, 0);

}


/*****************************************************************************/
/*
 * NAME:     stok_tx_timeout
 *
 * FUNCTION: Skyline driver transmit channel 2 watchdog timer timeout routine.
 *       Call hardware error handler.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS: 
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
stok_tx_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{

  stok_dev_ctl_t   *p_dev_ctl = (stok_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(stok_dev_ctl_t, tx_wdt));
  ushort	 bmctl;
  int	         ioa;
  int            ipri;

  /* 
   * Gets access to the I/O bus to access I/O registers                 
   */
  ipri = disable_lock(PL_IMP, &TX_LOCK);
  ioa = (int)iomem_att(&WRK.iomap);
  PIO_GETSRX(ioa + BMCTL_SUM, &bmctl);
  iomem_det((void *)ioa);                    /* restore I/O Bus              */

  TRACE_BOTH(STOK_ERR, "St2B", (ulong)p_dev_ctl, bmctl, 0);
  TOKSTATS.tx_timeouts++;

  if ((TX1_FRAME_PENDING == 0) && (TX2_FRAME_PENDING == 0)) {
  	unlock_enable(ipri, &TX_LOCK);
  	TRACE_BOTH(STOK_ERR, "St21", (ulong)p_dev_ctl, 0, 0);
	return;
  }

  stok_logerr(p_dev_ctl,ERRID_STOK_TX_TIMEOUT, __LINE__, __FILE__, 
		TX1_FRAME_PENDING, TX2_FRAME_PENDING, bmctl);

  if (p_dev_ctl->device_state != CLOSE_PENDING) {
  	stok_enter_limbo(p_dev_ctl, TRUE, FALSE, 0, NDD_TX_TIMEOUT, 2, 0);
  } else {
	TX1_FRAME_PENDING = 0;
	TX2_FRAME_PENDING = 0;
  } 
  unlock_enable(ipri, &TX_LOCK);
  TRACE_BOTH(STOK_ERR, "St2E", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     stok_cmd_timeout
 *
 * FUNCTION: Skyline driver command watchdog timer timeout routine.
 *       Set control operation status and wake up the ioctl routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * CALLED FROM:
 *      Timer 
 *
 * INPUT:
 *      p_wdt  - pointer to the watchdog structure in device control area
 *
 * RETURNS: 
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
stok_cmd_timeout(
struct watchdog *p_wdt)   /* pointer to watchdog timer structure */

{
  stok_dev_ctl_t *p_dev_ctl = (stok_dev_ctl_t *)
                        ((ulong)p_wdt - offsetof(stok_dev_ctl_t, cmd_wdt));

  TRACE_BOTH(STOK_ERR, "SctB", (ulong)p_dev_ctl, 0, 0);
  if (WRK.limbo_err != 17) {
  	stok_logerr(p_dev_ctl,ERRID_STOK_CTL_ERR,__LINE__,__FILE__,
			NDD_CMD_FAIL,0,0);
  }
  WRK.limbo_err = 17;
 
  /*
   * Wakeup the ioctl event
   */

  if ((p_dev_ctl->device_state != DEAD) &&
      (p_dev_ctl->device_state != CLOSE_PENDING)) {
  	stok_enter_limbo(p_dev_ctl, TRUE, FALSE, 0, NDD_CMD_FAIL, 0, 0);
  } else {
  	e_wakeup((int *)&WRK.ctl_event);
  }
  TRACE_BOTH(STOK_ERR, "SctE", (ulong)p_dev_ctl, 0, 0);

}

/*****************************************************************************/
/*
 * NAME:     stok_enter_limbo
 *
 * FUNCTION: Skyline driver hardware error handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * CALLED FROM:
 *      stok_intr 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *  	at_int_lvl	- running at interrupt level code or not
 *  	from_slih	- flag for enterring from process or interrupt level
 *      delay           - time to delay befor restart the recover
 * 	option0 	- status block option 0 
 * 	option1 	- status block option 1
 * 	option2 	- status block option 2 
 *
 * RETURNS:  
 *       none.
 */
/*****************************************************************************/
stok_enter_limbo(
	stok_dev_ctl_t  *p_dev_ctl,	/* pointer to device control area */
	int  		at_int_lvl,	/* flag for running at interrupt/proc */
	int  		from_slih, 	/* flag for process or interrupt */
	int		delay,		/* delay time			 */
	int		option0,	/* status block option 0 */
	int		option1,	/* status block option 1 */
	int		option2)	/* status block option 2 */

{
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);
  int 		ipri, rc;

  TRACE_BOTH(STOK_ERR, "NheB",(ulong)p_dev_ctl, from_slih, at_int_lvl);

  /* Gets locks */
  if (!from_slih) {
  	ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  }

  disable_lock(PL_IMP, &TX_LOCK);
  disable_lock(PL_IMP, &CMD_LOCK);

  if (!((p_dev_ctl->device_state == DEAD) |
        (p_dev_ctl->device_state == CLOSE_PENDING))){
  	/*
  	 * update NDD frags * device state
  	 */
  	NDD.ndd_flags &=  ~NDD_RUNNING;
  	NDD.ndd_flags |=  NDD_LIMBO;
  	p_dev_ctl->device_state = LIMBO;

  	/*
  	 * No status block will be send if there is no reason code (option 0)
  	 */
  	if (option0) {
  		/*
   		 * Pass the status block to demuxer
   		 */
  		stat_blk.code = NDD_LIMBO_ENTER;
  		stat_blk.option[0] = option0;
  		stat_blk.option[1] = option1;
  		stat_blk.option[2] = option2;
  		(*(NDD.nd_status))(&NDD, &stat_blk);
  	}

  	/*
  	 * Stops the watchdog timers
  	 */
  	w_stop(&(CMDWDT));
  	w_stop(&(TXWDT));
  	w_stop (&(LIMWDT));
  	w_stop (&(LANWDT));
  	w_stop (&(HWEWDT));

  	/* wakeup outstanding ioctl or open event */
      	e_wakeup((int *)&WRK.ctl_event);

  	if (reset_adapter(p_dev_ctl)) {
 		TRACE_BOTH(STOK_ERR, "Nhe1", (int)p_dev_ctl, WRK.pio_rc, 0);
 		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, NDD_ADAP_CHECK, 0, 
                             TRUE, TRUE, TRUE);
  	} else { 

		WRK.srb_address = 0;
  		if (delay) {
  			LIMWDT.restart     = delay;
  			w_start (&(LIMWDT));        /* starts watchdog timer  */
  		} else {
 			if (rc = stok_re_open_adapter(p_dev_ctl, TRUE, FALSE)) {
  				TRACE_BOTH(STOK_ERR, "Nhe2", (ulong)p_dev_ctl, 
						rc, 0);
 				stok_bug_out(p_dev_ctl, NDD_HARD_FAIL,  
                                             NDD_ADAP_CHECK, 0,
                                             TRUE, TRUE, TRUE);
       				stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, 
					__LINE__, __FILE__, NDD_HARD_FAIL, 
                                        rc, OPENED);
  			}
  		}
  	}
  }

  unlock_enable(PL_IMP, &CMD_LOCK);
  unlock_enable(PL_IMP, &TX_LOCK);

  if (!from_slih) {
        unlock_enable(ipri, &SLIH_LOCK);
  }

  TRACE_BOTH(STOK_ERR, "NheE", p_dev_ctl, 0, 0);
}

/*****************************************************************************/
/*
* NAME: stok_re_open_adapter
*
* FUNCTION: Start the initalization of the adapter: load PCI registers and
*           start the sequence for open the adapter.
*
* EXECUTION ENVIRONMENT:
*      interrupt thread.
*
* Output: 
*  An adapter open.
*
* Called From: 
*  stok_restart
*
* INPUT:
*       p_dev_ctl       - pointer to the DEvice control area
*  	at_int_lvl	- flag for running at interrupt or process level
*  	from_slih	- flag for enterring from process or interrupt level
*
*
* RETURN:  
*      0 = OK
*      ENOCONNECT - unable to activate and setup the adapter
*      ENOMEM - unable to allocate required memory
*
*/
/*****************************************************************************/
int stok_re_open_adapter (
stok_dev_ctl_t  *p_dev_ctl,	/* pointer to device control area */
int  		at_int_lvl,	/* flag for running at interrupt/proc */
int  		from_slih)	/* flag for process or interrupt */

{
  ndd_t         *p_ndd = &(NDD);
  int    	ioa, iocc, pio_rc;
  int    	cntl_reg;
  ushort 	data;
  int 		ipri_slih;
  ndd_statblk_t stat_blk;   		 /* status block */
  int    	i;                       /* loop counter                      */
  int    	rc;                      /* Local return code                 */

  TRACE_BOTH(STOK_ERR, "NraB", p_dev_ctl, at_int_lvl, from_slih);

  /*********************************************************************/
  /* Reset up the transmit memory area                                 */
  /*********************************************************************/
  stok_tx_buf_clearup(p_dev_ctl, &TX2);
  if (DDS.priority_tx) {
	stok_tx_buf_clearup(p_dev_ctl, &TX1);
  }

  /*********************************************************************/
  /* Frees the pending Rx buffer                                       */
  /*********************************************************************/
  if (WRK.mhead) {
      	m_freem(WRK.mhead);
	WRK.eof = TRUE;
  }

  /*********************************************************************/
  /* Enable Rx channel                                                 */
  /*********************************************************************/
  if (stok_rx_enable(p_dev_ctl)) {  /* Something went wrong      */
        TRACE_BOTH(STOK_ERR, "Nra1", p_dev_ctl, ENOMEM, 0);
        return(ENOMEM);
  }

  if (stok_act(p_dev_ctl, at_int_lvl)) {
        TRACE_BOTH(STOK_ERR, "Nra2", p_dev_ctl, ENOMEM, 0);
        return(ENOCONNECT);
  }

  TRACE_BOTH(STOK_ERR, "NraE", p_dev_ctl, at_int_lvl, from_slih);
  return(0);
} /* end stok_re_open_adapter                                               */

/****************************************************************************/
/*
* NAME: stok_tx_buf_clearup
*
* FUNCTION: clearup the transmit list and Re_initialize the transmit list
*           variables.
*
* EXECUTION ENVIRONMENT:
*      This routine runs only under the process thread.
*
* INPUT:
*       p_dev_ctl - pointer to device contril structure
*
* CALLED FROM:
*       stok_re_open_adapter
*
*/
/****************************************************************************/

int stok_tx_buf_clearup(
  stok_dev_ctl_t  *p_dev_ctl,
  tx_t		*wrk)
{
  ndd_t         	*p_ndd = &(NDD);
  struct  mbuf          *mbufp, *p_mbuf;

  TRACE_BOTH(STOK_ERR, "NxrB", p_dev_ctl, 0, 0);

  /* 
   * Cleanup the transmit chain 
   */
  NDD.ndd_genstats.ndd_opackets_drop += wrk->tx_frame_pending; 

  if (wrk->hw_txq_out) { 
	p_mbuf = wrk->hw_txq_out;
  	while (wrk->tx_frame_pending) {
  		mbufp = p_mbuf;
          	while (mbufp) {
                  	if (mbufp->m_len > HAFT_PAGE) {
                          	D_UNMAP_PAGE(WRK.dma_channel,
                                       &wrk->tx_dma_addr[wrk->tx_dma_next_in]);
                  	} 
                  	mbufp = mbufp->m_next;
          	}
          	p_mbuf = p_mbuf->m_nextpkt;
  	  	wrk->tx_frame_pending--;
  	 }

          m_freem(wrk->hw_txq_out);
          wrk->hw_txq_out = NULL;
  }

  if (wrk->sw_txq_first) {
        m_freem(wrk->sw_txq_first);
        wrk->sw_txq_first = NULL;
        NDD.ndd_genstats.ndd_opackets_drop += wrk->sw_txq_len; 
  }

  wrk->tx_elem_next_out = 0;
  wrk->tx_elem_next_in  = 0;
  wrk->tx_frame_pending = 0;
  wrk->tx_buf_next_in   = 0;
  wrk->tx_buf_use_count = 0;
  wrk->tx_dma_next_out  = 0;
  wrk->tx_dma_next_in   = 0;
  wrk->tx_dma_use_count = 0;
  wrk->sw_txq_len   	= 0;
  wrk->sw_txq_first 	= 0;
  wrk->sw_txq_last 	= 0;

  TRACE_BOTH(STOK_ERR, "NxrE", p_dev_ctl, 0, 0);
}  /* end function stok_tx_buf_clearup */



