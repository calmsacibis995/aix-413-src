static char sccsid[] = "@(#)41  1.2  src/rspc/kernext/pci/stok/stok_close.c, pcitok, rspc41J, 9515A_all 4/11/95 12:38:31";
/*
 *   COMPONENT_NAME: pcitok
 *
 *   FUNCTIONS: stok_close
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

extern int stok_config();
extern struct cdt *cdt_func();
extern stok_dd_ctl_t stok_dd_ctl;

/*****************************************************************************/
/*
 * NAME:     stok_close
 *
 * FUNCTION: Skyline driver close routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * CALLED FROM:
 *      NS user by using the ndd_close field in the NDD on the NDD chain.
 *
 * CALLS TO :
 *      reset_adapter
 *      stok_tx_setup_undo
 *      stok_rx_undo
 *      cdt_del
 *      dmp_del
 *
 * RETURNS:  
 *      0 if successful
 *      errno value on failure
 */
/*****************************************************************************/
stok_close(
ndd_t   *p_ndd)              /* pointer to the ndd in the dev_ctl area */

{
  stok_dev_ctl_t *p_dev_ctl = (stok_dev_ctl_t *)(p_ndd->ndd_correlator);
  stok_multi_t   *p_multi, *p_temp;
  int rc, ioa;
  ndd_statblk_t  stat_blk;   /* status block */
  int ipri;

  TRACE_BOTH(STOK_OTHER, "ClsB", (ulong)p_dev_ctl, 0, 0);

  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  /*
   * if the driver is in a power management mode of suspend/hibernate then put
   * the caller to sleep until the driver resumes normal operation except for
   * open this needs to be done under a lock so that the state check is valid
   */
  if (p_dev_ctl->device_state == SUSPEND_STATE) {
        e_sleep_thread (&WRK.pmh_event, &SLIH_LOCK, LOCK_HANDLER);
  }

  if (p_dev_ctl->device_state == OPENED) {
        p_dev_ctl->device_state = CLOSE_PENDING;

        /* release the lock */
        unlock_enable(ipri, &SLIH_LOCK);

  	/*
  	 * Waits for the transmit queue to drain 
  	 */
 	 while (p_dev_ctl->device_state == CLOSE_PENDING &&
        	(TX1_FRAME_PENDING || TX2_FRAME_PENDING)) {
         	io_delay(100);   
  	}

        /* get the lock again */
        ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  }

  /*
   * De-activate the adapter 
   * Disables the adapter interrupts and turn on soft reset 
   */
  reset_adapter(p_dev_ctl);

  p_dev_ctl->device_state = CLOSED;
  p_ndd->ndd_flags = NDD_BROADCAST;

  unlock_enable(ipri, &SLIH_LOCK);
  /**************************************************************************
   * cleanup all the resources allocated for open 
   **************************************************************************/
  /*
   * Frees the watchdog timers
   */
  w_stop (&(TXWDT));
  w_clear (&(TXWDT));
  w_stop (&(LANWDT));
  w_clear (&(LANWDT));
  w_stop (&(LIMWDT));
  w_clear (&(LIMWDT));
  w_stop (&(HWEWDT));
  w_clear (&(HWEWDT));
  w_stop (&(CMDWDT));
  w_clear (&(CMDWDT));

  /*
   * De-register the interrupt handler 
   */
  i_clear(&(IHS));

  /*
   * Frees the multicast table extensions 
   */
  p_multi = WRK.multi_table.next;
  while (p_multi) {
        p_temp = p_multi->next;
        xmfree(p_multi, pinned_heap);
        p_multi = p_temp;
  }

  WRK.multi_count  = 0;

  /*
   * Frees the Rx buffer queues
   */
  if (WRK.mhead) {
        m_freem(WRK.mhead);
        WRK.mhead = NULL;
  }

  /*
   * Frees any resources that were allocated for the rx & tx 
   */
  stok_rx_setup_undo (p_dev_ctl);
  if (DDS.priority_tx) {
  	stok_tx_setup_undo (p_dev_ctl, &TX1);
  	if (TX1.sw_txq_first) {
        	m_freem(TX1.sw_txq_first);
        	TX1.sw_txq_first = NULL;
  	}
  }

  stok_tx_setup_undo (p_dev_ctl, &TX2);
  if (TX2.sw_txq_first) {
        m_freem(TX2.sw_txq_first);
        TX2.sw_txq_first = NULL;
  }

  /*
   * Frees the DMA clannel 
   */
  D_MAP_CLEAR (WRK.dma_channel);

  lock_write(&DD_LOCK);
  cdt_del("dev_ctl", (char *)p_dev_ctl, sizeof(stok_dev_ctl_t));
  stok_dd_ctl.open_count--;
  if(!stok_dd_ctl.open_count) {
        dmp_del(cdt_func);
        cdt_del("dd_ctl", (char *)&stok_dd_ctl,sizeof(stok_dd_ctl_t));
        xmfree(CDT, pinned_heap);           /* Frees the cdt area     */
  }
  lock_done(&DD_LOCK);

  /*
   * Frees the locks in the dev_ctl area
   */
  lock_free(&TX_LOCK);
  lock_free(&CMD_LOCK);
  lock_free(&SLIH_LOCK);

  TRACE_BOTH(STOK_OTHER, "ClsE", (ulong)p_dev_ctl, 0, 0);
  unpincode(stok_config);

  return(0);
}

