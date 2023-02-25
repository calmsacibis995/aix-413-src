static char sccsid[] = "@(#)40  1.1  src/bos/usr/samples/ndd/entsamp_close.c, entsamp, bos411, 9428A410j 1/14/94 13:39:35";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_close
 *		entsamp_cleanup
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
#include "entsamp.h"


extern int entsamp_open();
extern struct cdt *entsamp_cdt_func();
extern entsamp_dd_ctl_t entsamp_dd_ctl;	

void entsamp_cleanup();


/*****************************************************************************/
/*
 * NAME:     entsamp_close
 *
 * FUNCTION: Ethernet driver close routine.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      NS user by using the ns_free() service
 *
 * INPUT:
 *      p_ndd           - pointer to the ndd.
 *
 * RETURNS:  
 *	0 - successful
 */
/*****************************************************************************/
entsamp_close(
  ndd_t		*p_ndd)		/* pointer to the ndd in the dev_ctl area */

{
  entsamp_dev_ctl_t   *p_dev_ctl = (entsamp_dev_ctl_t *)(p_ndd->ndd_correlator);
  int ipri;



  TRACE_SYS(HKWD_ENTSAMP_OTHER, "ClsB", (ulong)p_ndd, 0, 0);
  
  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  if (p_dev_ctl->device_state == OPENED) {
  	p_dev_ctl->device_state = CLOSE_PENDING;
  }
  unlock_enable(ipri, &SLIH_LOCK);

  /* wait for the transmit queue to drain */
  while (p_dev_ctl->device_state == CLOSE_PENDING &&
		(p_dev_ctl->tx_pending || p_dev_ctl->txq_len)) {
		DELAYMS(1000);		/* delay 1 second */
  }
	
  /* de-activate the adapter */
  ipri = disable_lock(PL_IMP, &SLIH_LOCK);
	
  /*
   * De-activate the device right now. 
   * Perform device-specific operations to reset/disable the adapter. 
   * 
   * A section of device-specific code was omitted here.
   */
  p_dev_ctl->device_state = CLOSED;
  p_ndd->ndd_flags &= ~NDD_RUNNING;

  unlock_enable(ipri, &SLIH_LOCK);	

  /* cleanup all the resources allocated for open */
  entsamp_cleanup(p_dev_ctl);

  lock_write(&DD_LOCK);
  entsamp_cdt_del("entsamp_dev_ctl", (char *)p_dev_ctl, 
	sizeof(entsamp_dev_ctl_t));

  entsamp_dd_ctl.open_count--;
  if(!entsamp_dd_ctl.open_count) {
	entsamp_cdt_del("entsamp_dd_ctl", (char *)&entsamp_dd_ctl,
		  sizeof(entsamp_dd_ctl_t));
	dmp_del(entsamp_cdt_func);
  }
  lock_done(&DD_LOCK);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "ClsE", 0, 0, 0);
  unpincode(entsamp_open);
  return(0);


}
  
/*****************************************************************************/
/*
 * NAME:     entsamp_cleanup
 *
 * FUNCTION: Cleanup all the resources used by opening the adapter.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_close
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the dev_ctl area.
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_cleanup(
	entsamp_dev_ctl_t	*p_dev_ctl) /* pointer to the dev_ctl area */
{



  TRACE_SYS(HKWD_ENTSAMP_OTHER, "CclB", (ulong)p_dev_ctl, 0, 0);

  /* cleanup the transmit/receive buffers */
  if (WRK.rv_buf_alocd)
        entsamp_rv_free(p_dev_ctl);

  if (WRK.tx_buf_alocd)
        entsamp_tx_free(p_dev_ctl);

  /* Free the DMA clannel */
  if (WRK.channel_alocd) {
	d_clear(WRK.dma_channel);
	WRK.channel_alocd = FALSE;

  } 

  /* Clear the transmit watchdog timer */
  if (WRK.tx_wdt_inited) {
	while (w_clear(&(TXWDT)));
	WRK.tx_wdt_inited = FALSE;
  }
  /* Clear the control operation watchdog timer */
  if (WRK.ctl_wdt_inited) {
	while (w_clear(&(CTLWDT)));
	WRK.ctl_wdt_inited = FALSE;
  }

  /* De-register the interrupt handler */
  if (WRK.intr_inited) {
	i_clear(&(IHS));
	WRK.intr_inited = FALSE;
  }

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "CclE", 0, 0, 0);


} 

