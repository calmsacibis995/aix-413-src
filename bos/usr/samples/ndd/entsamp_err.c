static char sccsid[] = "@(#)44  1.1  src/bos/usr/samples/ndd/entsamp_err.c, entsamp, bos411, 9428A410j 1/14/94 13:40:30";
/*
 * COMPONENT_NAME: (ENTSAMP) IBM BOS Sample Program
 *
 * FUNCTIONS: 
 *		entsamp_tx_timeout
 *		entsamp_ctl_timeout
 *		entsamp_hard_err
 *		entsamp_restart
 *		entsamp_hard_fail
 *		entsamp_stimer
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
#include "entsamp_errids.h"


void entsamp_hard_err();
void entsamp_hard_fail();

/*****************************************************************************/
/*
 * NAME:     entsamp_tx_timeout
 *
 * FUNCTION: Ethernet driver transmit watchdog timer timeout routine.
 *	     Try to recover the adapter once.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer function
 *
 * INPUT:
 *      p_wdt	- pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_tx_timeout(
  struct watchdog *p_wdt) 	/* pointer to watchdog timer structure */

{
  entsamp_dev_ctl_t   *p_dev_ctl = (entsamp_dev_ctl_t *)
			((ulong)p_wdt - offsetof(entsamp_dev_ctl_t, tx_wdt));


  TRACE_SYS(HKWD_ENTSAMP_OTHER, "ttoB", (ulong)p_wdt, 0, 0);

  entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_TMOUT, __LINE__, __FILE__, 0, 0, 0);
  entsamp_hard_err(p_dev_ctl, FALSE, TRUE, NDD_TX_TIMEOUT);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "ttoE", 0, 0, 0);


}

/*****************************************************************************/
/*
 * NAME:     entsamp_ctl_timeout
 *
 * FUNCTION: Ethernet driver ioctl watchdog timer timeout routine.
 *	     Set control operation status and wake up the ioctl routine.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Timer function
 *
 * INPUT:
 *      p_wdt	- pointer to the watchdog structure in device control area
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
void
entsamp_ctl_timeout(
  struct watchdog *p_wdt) 	/* pointer to watchdog timer structure */

{
  entsamp_dev_ctl_t   *p_dev_ctl = (entsamp_dev_ctl_t *)
			((ulong)p_wdt - offsetof(entsamp_dev_ctl_t, ctl_wdt));


  TRACE_SYS(HKWD_ENTSAMP_OTHER, "tcoB", (ulong)p_wdt, 0, 0);

  entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_TMOUT, __LINE__, __FILE__, 0, 0, 0);
  entsamp_hard_err(p_dev_ctl, FALSE, TRUE, NDD_TX_TIMEOUT);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "tcoE", 0, 0, 0);


}
  
/*****************************************************************************/
/*
 * NAME:     entsamp_hard_err
 *
 * FUNCTION: Ethernet driver hardware error handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_intr 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	from_slih	- flag for enterring from process or interrupt level
 *	restart		- flag for restarting the adapter or not
 *	err_code	- error code for status blocks
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
entsamp_hard_err(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int		from_slih, 		/* flag for process or interrupt */
  int		restart,		/* flag for restart the adapter */
  int		err_code)		/* error reason code */


{
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);
  struct mbuf	*p_mbuf;
  int ipri_slih;
  int ipri;



  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NheB", (ulong)p_dev_ctl, from_slih, restart);
  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NheC", err_code, 0, 0);
  
  /*
   * Stop the watchdog timers
   */
  w_stop(&(CTLWDT));
  w_stop(&(TXWDT));

  /* get locks */
  if (!from_slih) {
	ipri_slih = disable_lock(PL_IMP, &SLIH_LOCK);
  }
  
  ipri = disable_lock(PL_IMP, &TX_LOCK);
  disable_lock(PL_IMP, &CMD_LOCK);

  /* wakeup outstanding ioctl event */
  if (p_dev_ctl->ctl_pending) {
  	e_wakeup((int *)&p_dev_ctl->ctl_event);
  }

  /* free up the mbufs sitting in the txq */
  if (p_dev_ctl->txq_len) {
	while (p_mbuf = p_dev_ctl->txq_first) {
		p_dev_ctl->txq_first = p_mbuf->m_nextpkt;
		m_freem(p_mbuf);
	}
	p_dev_ctl->txq_len = 0;
	p_dev_ctl->txq_last = NULL;
  }
  
  if (restart && p_dev_ctl->device_state == OPENED) {

	p_dev_ctl->device_state = LIMBO;
	if (entsamp_restart(p_dev_ctl, err_code)) {
		entsamp_hard_fail(p_dev_ctl, err_code);
  		p_dev_ctl->ctl_status = ENETDOWN;
	}
	else {
  		p_dev_ctl->ctl_status = 0;
	}
  }
  else {
	entsamp_hard_fail(p_dev_ctl, err_code);
  	p_dev_ctl->ctl_status = ENETDOWN;
  }

  unlock_enable(PL_IMP, &CMD_LOCK);
  unlock_enable(ipri, &TX_LOCK);

  if (!from_slih) {
	unlock_enable(ipri_slih, &SLIH_LOCK);
  }


  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NheE", 0, 0, 0);
}

/*****************************************************************************/
/*
 * NAME:     entsamp_restart
 *
 * FUNCTION: Ethernet driver network recovery handler.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_hard_err 
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	err_code	- error code for status blocks
 *
 * RETURNS:  
 *	0 - OK
 *      EIO - PIO error occurred during the start
 *      ENOCONNECT - adapter error occurred during the start
 */
/*****************************************************************************/
entsamp_restart(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int	err_code)			/* error reason code */


{

  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);
  int ioa, rc; 
  int pio_rc = 0;



  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NrsB", (ulong)p_dev_ctl, err_code, 0);

  /* pass a status block to demuxer */
  bzero(&stat_blk, sizeof(ndd_statblk_t));
  stat_blk.code = NDD_LIMBO_ENTER;
  stat_blk.option[0] = err_code;
  (*(p_ndd->nd_status))(p_ndd, &stat_blk);

  /*
   * Performs device-specific operations to reset and re-initialize
   * the device. 
   *
   * A section of device-specific code was omitted here.
   */

  /* save the device start time for statistics */
  p_dev_ctl->dev_stime = lbolt;

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NrsE", 0, 0, 0);
  return(0);

}
/*****************************************************************************/
/*
 * NAME:     entsamp_hard_fail
 *
 * FUNCTION: Set the adapter to DEAD state, notify user and cleanup any
 *	     outstanding transmit/ioctl requests.
 *
 * EXECUTION ENVIRONMENT: process and interrupt 
 *
 * NOTES:
 *
 * CALLED FROM:
 *      entsamp_hard_err 
 *	entsamp_stimer
 *
 * INPUT:
 *      p_dev_ctl       - pointer to the device control area
 *	err_code	- error code for status blocks
 *
 * RETURNS:  
 *	none.
 */
/*****************************************************************************/
void
entsamp_hard_fail(
  entsamp_dev_ctl_t	*p_dev_ctl,	/* pointer to device control area */
  int		err_code)		/* error reason code */


{
  ndd_statblk_t  stat_blk;   /* status block */
  ndd_t		*p_ndd = &(NDD);



  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NhfB", (ulong)p_dev_ctl, err_code, 0);
  
  p_dev_ctl->device_state = DEAD;
  p_ndd->ndd_flags &= ~NDD_RUNNING;
  p_ndd->ndd_flags |= NDD_DEAD;
  entsamp_logerr(p_dev_ctl, ERRID_ENTSAMP_FAIL, __LINE__, __FILE__, 0, 0, 0);

  /*
   * Performs device-specific operations to de-activate the device.
   *
   * A section of device-specific code was omitted here.
   */

  /* pass a status block to demuxer */
  bzero(&stat_blk, sizeof(ndd_statblk_t));
  stat_blk.code = NDD_HARD_FAIL;
  stat_blk.option[0] = err_code;
  (*(p_ndd->nd_status))(p_ndd, &stat_blk);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "NhfE", 0, 0, 0);

}
/*****************************************************************************/
/*
 * NAME:     entsamp_stimer
 *
 * FUNCTION: Timeout function for the system timer used for error recovery.
 *
 * EXECUTION ENVIRONMENT: interrupt only
 *
 * NOTES:
 *
 * CALLED FROM:
 *	system timer
 *
 * INPUT:
 *      p_systimer	- pointer to the timer structure.
 *
 * RETURNS:  
 * 	none.
 */
/*****************************************************************************/
void
entsamp_stimer(
  struct trb *p_systimer)	/* pointer to the timer structure */

{

  entsamp_dev_ctl_t	*p_dev_ctl = (entsamp_dev_ctl_t *)
					(p_systimer->func_data);
  int   ipri;


  TRACE_SYS(HKWD_ENTSAMP_OTHER, "tstB", (ulong)p_systimer, (ulong)p_dev_ctl, 0);

  /* get the slih lock */
  ipri = disable_lock(PL_IMP, &SLIH_LOCK);

  /*
   * if the device state is not LIMBO, the device may be closed or dead, or
   * there is something wrong, don't continue the restart procedure
   */
  if (p_dev_ctl->device_state != LIMBO) {
	unlock_enable(ipri, &SLIH_LOCK);
  	TRACE_SYS(HKWD_ENTSAMP_ERR, "tst0", p_dev_ctl->device_state, 0, 0);
        return;
  }

  /*
   * Performs device-specific operations to continue the procedure of
   * reset and re-initialize the device. The timer was first started
   * in the entsamp_restart() in order to continue the multi-step 
   * restart procedure asynchronously.
   *
   * A section of device-specific code was omitted here.
   */


  unlock_enable(ipri, &SLIH_LOCK);

  TRACE_SYS(HKWD_ENTSAMP_OTHER, "tstE", 0, 0, 0);

} 

