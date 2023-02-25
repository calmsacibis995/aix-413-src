static char sccsid[] = "@(#)15	1.2  src/rspc/kernext/pci/kent/kent_pmh.c, pcient, rspc41J, 9519B_all 5/10/95 15:53:19";
/*
 *   COMPONENT_NAME: PCIENT
 *
 *   FUNCTIONS: kent_pmh
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

#include "kent_proto.h"
#include <sys/errno.h>
#include <sys/lockname.h>
#include <sys/cdli.h>
#include <sys/mbuf.h>
#include <sys/mdio.h>
/* 
 * get access to the global device driver control block
 */
extern kent_tbl_t	kent_tbl;
extern struct cdt	*p_kent_cdt;

/*
 * NAME: kent_pmh()
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

int
kent_pmh( 
	kent_acs_t 	*p_acs,
        int             ctrl)
{
	int 		ipri;
	int 		rc = 0;

	KENT_TRACE("PkpB",p_acs, ctrl, p_acs->dev.state);
	KENT_TRACE("PkpC",&p_acs->pmh,p_acs->dev.pmh_state, 0);
	
	switch(ctrl)
	{
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
		p_acs->dev.pmh_state = p_acs->dev.state;
		p_acs->pmh.activity = -1;
		p_acs->pmh.mode = ctrl;  

		/* 
		 * if the driver is closed or closing then nothing needs to be 
		 * done to put the adapter in hibernate/suspend state.  Just 
		 * remember the state to prevent future calls.  The closing state
	 	 * will reset the driver in the future, and as the driver does 
		 * not know which resources have been freed it is best for it 
		 * to do nothing.  
		 */
		if (p_acs->dev.state == CLOSED_STATE ||
			p_acs->dev.state == CLOSING_STATE)
		{
			p_acs->dev.state = SUSPEND_STATE;
			break;
		}

		/* 
		 * Be sure to get the same locks as you would for a limbo
		 * call, need to prevent critical code from running while the
		 * adapter is reset.
		 */
		ipri = disable_lock(KENT_OPLEVEL, &p_acs->dev.slih_lock);
		disable_lock(KENT_OPLEVEL, &p_acs->tx.lock);
		disable_lock(KENT_OPLEVEL, &p_acs->dev.ctl_lock);

		p_acs->dev.state = SUSPEND_STATE;

		if (p_acs->dev.pmh_state != DEAD_STATE)
			reset_adapter(p_acs);

		unlock_enable(KENT_OPLEVEL, &p_acs->dev.ctl_lock);
		unlock_enable(KENT_OPLEVEL, &p_acs->tx.lock);
		unlock_enable(ipri, &p_acs->dev.slih_lock);
		
		break;

	case PM_DEVICE_FULL_ON:
	case PM_DEVICE_ENABLE:

		p_acs->pmh.mode = ctrl;  

		/* 	
		 * if the driver is not in suspend state, then nothing other
		 * than the mode needs to be changed.
		 */
		if (p_acs->dev.state != SUSPEND_STATE)
			break;
	
		p_acs->dev.state = p_acs->dev.pmh_state;
		p_acs->pmh.activity = 1;

		/* 
		 * The driver needs to be sure to configure the one time
		 * adapter configurations again, as the hibernate is the 
		 * equivalent of a power off.  These one time actions 
		 * normally occur during config.
		 */
		init_adapter(p_acs);

		if (rc == 0 && p_acs->dev.state == OPEN_STATE)
		{	
			rc = start_adapter(p_acs);

			/* 
		 	 * if there was a failure succeed in resuming but go 
			 * to the dead state.
			 */

			if (rc || p_acs->dev.pio_rc) 
			{
				bugout(p_acs);
			}
		}

		/* 
		 * Be sure to wake up all users waiting for the device to resume
		 */
		e_wakeup(&p_acs->dev.pmh_event);
		break;

	case PM_DEVICE_DPMS_SUSPEND:
	case PM_DEVICE_DPMS_STANDBY:
		p_acs->pmh.mode = ctrl;
		break;

	case PM_RING_RESUME_ENABLE_NOTICE:
		break;

	case PM_PAGE_FREEZE_NOTICE:
		if (rc = pincode(kent_config))
		{
			KENT_ETRACE("Pkp1",p_acs, rc, ctrl);
			return(PM_ERROR);
		}
		break;
		
	case PM_PAGE_UNFREEZE_NOTICE:
		unpincode(kent_config);
		break;

	default:
		KENT_ETRACE("Pkp2",p_acs, ctrl, 0);
		break;
	};

	KENT_TRACE("PkpE",p_acs, ctrl, p_acs->dev.state);
	return (PM_SUCCESS);
}
