static char sccsid[] = "@(#)81  1.2  src/rspc/kernext/pci/stok/stok_pmh.c, pcitok, rspc41J, 9516A_all 4/14/95 15:34:52";
/*
 *   COMPONENT_NAME: PCITOK
 *
 *   FUNCTIONS: stok_pmh
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

/*
 * NAME: stok_pmh()
 *                                                                    
 * FUNCTION: power management handler
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	This routine runs on the process thread.  It assumes interrupts have 
 *	not been disabled.
 *	
 *                                                                   
 * NOTES:
 *
 * RETURNS: 
 *		0		- successful
 *		ENETDOWN	- Indicates that the device is permanently down.
 * 		EINVAL		- Indicates that an invalid parameter was 
 * 					specified.
 *		ENOMEM		- Unable to allocate required memory
 *		E2BIG		- Indicates that the structure passed to the
 *					device driver was not large enough to 
 *					hold all the addresses.
 */  

int stok_pmh (
stok_dev_ctl_t  *p_dev_ctl,
int             ctrl)
{
  int 		ipri;
  int 		rc = 0;

  TRACE_SYS(STOK_OTHER, "PkpB", p_dev_ctl, ctrl, p_dev_ctl->device_state); 
  TRACE_SYS(STOK_OTHER, "PkpC", &WRK.pmh, WRK.pmh.next1, &WRK.pmh.next1); 

  switch(ctrl) {
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
		p_dev_ctl->pm_state = p_dev_ctl->device_state;
		WRK.pmh.activity = -1;
		WRK.pmh.mode = ctrl;  

		read_adapter_log(p_dev_ctl, TRUE);
		/* 
		 * if the driver is closed or closing then nothing needs to be 
		 * done to put the adapter in hibernate/suspend state.  Just 
		 * remember the state to prevent future calls. The closing state
	 	 * will reset the driver in the future, and as the driver does 
		 * not know which resources have been freed it is best for it 
		 * to do nothing.  
		 */
		if (p_dev_ctl->device_state == CLOSED ||
			p_dev_ctl->device_state == CLOSE_PENDING) {
			p_dev_ctl->device_state = SUSPEND_STATE;
			break;
		}


		/* 
		 * Be sure to get the same locks as you would for a limbo
		 * call, need to prevent critical code from running while the
		 * adapter is reset.
		 */
        	ipri = disable_lock(PL_IMP, &SLIH_LOCK);
  		disable_lock(PL_IMP, &TX_LOCK);
  		disable_lock(PL_IMP, &CMD_LOCK);

		p_dev_ctl->device_state = SUSPEND_STATE;

        	/*
        	* Stops the watchdog timers
         	*/
        	w_stop (&(CMDWDT));
        	w_stop (&(TXWDT));
        	w_stop (&(LIMWDT));
        	w_stop (&(LANWDT));
        	w_stop (&(HWEWDT));

        	/* wakeup outstanding ioctl or open event */
        	e_wakeup((int *)&WRK.ctl_event);

		reset_adapter(p_dev_ctl);

  		unlock_enable(PL_IMP, &CMD_LOCK);
  		unlock_enable(PL_IMP, &TX_LOCK);
        	unlock_enable(ipri, &SLIH_LOCK);

		break;

	case PM_DEVICE_FULL_ON:
	case PM_DEVICE_ENABLE:

		WRK.pmh.mode = ctrl;  

		/* 	
		 * if the driver is not in suspend state, then nothing other
		 * than the mode needs to be changed.
		 */
		if (p_dev_ctl->device_state != SUSPEND_STATE) {
			break;
		}
	
		p_dev_ctl->device_state = p_dev_ctl->pm_state;
		WRK.pmh.activity = 1;

		/* 
		 * The driver needs to be sure to configure the one time
		 * adapter configurations again, as the hibernate is the 
		 * equivalent of a power off.  These one time actions 
		 * normally occur during config.
		 */
  		if ((p_dev_ctl->device_state != DEAD) &&
      			(p_dev_ctl->device_state != CLOSED) &&
      			(p_dev_ctl->device_state != CLOSE_PENDING)) {
			
        		/* start reconfig PCI regs process */
        		if (cfg_pci_regs(p_dev_ctl)) {
                		TRACE_BOTH(STOK_ERR, "Pkp2", p_dev_ctl, 0, 0);
                		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
					     NDD_ADAP_CHECK, 0,
                                	     FALSE, FALSE, FALSE);
                		stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, 
						__LINE__, __FILE__, 
						NDD_HARD_FAIL, NDD_DEAD,OPENED);
				e_wakeup(&WRK.pmh_event);
				break;
        		}
        		/* start reopen process */
			p_dev_ctl->device_state = LIMBO;
			WRK.srb_address = 0;
        		if (stok_re_open_adapter(p_dev_ctl, TRUE, FALSE)) {
                		TRACE_BOTH(STOK_ERR, "Pkp3", p_dev_ctl, 0, 0);
                		stok_bug_out(p_dev_ctl, NDD_HARD_FAIL, 
					     NDD_ADAP_CHECK, 0,
                                	     FALSE, FALSE, FALSE);
                		stok_logerr(p_dev_ctl, ERRID_STOK_ADAP_CHECK, 
						__LINE__, __FILE__, 
						NDD_HARD_FAIL, NDD_DEAD,OPENED);
        		}
  		}

		/* 
		 * Be sure to wake up all users waiting for the device to resume
		 */
		e_wakeup(&WRK.pmh_event);
		break;

	case PM_DEVICE_DPMS_SUSPEND:
	case PM_DEVICE_DPMS_STANDBY:
		WRK.pmh.mode = ctrl;
		break;

	case PM_RING_RESUME_ENABLE_NOTICE:
		break;

	case PM_PAGE_FREEZE_NOTICE:
		if (rc = pincode(stok_config))
		{
  			TRACE_SYS(STOK_OTHER, "Pkp4", p_dev_ctl, ctrl, 0); 
			return(PM_ERROR);
		}
		break;
		
	case PM_PAGE_UNFREEZE_NOTICE:
		unpincode(stok_config);
		break;

	default:
  		TRACE_SYS(STOK_OTHER, "Pkp5", p_dev_ctl, ctrl, 0); 
		break;
	};

  	TRACE_SYS(STOK_OTHER, "PkpE", p_dev_ctl, ctrl, p_dev_ctl->device_state); 

	return (PM_SUCCESS);
}
