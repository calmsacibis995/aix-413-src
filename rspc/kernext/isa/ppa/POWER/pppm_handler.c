static char sccsid[] = "@(#)84  1.1 src/rspc/kernext/isa/ppa/POWER/pppm_handler.c, isappa, rspc41J, 9510A 3/6/95 23:24:05";
/*
 *
 *   COMPONENT_NAME: (ISAPPA) Parallel Port Device Driver
 *
 *   FUNCTIONS: pppm_handler
 *              pppm_save_regs
 *              pppm_restore_regs
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

#ifdef	PM_PHASE2

#include <sys/ppdd.h>
#include <sys/lpio.h>

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/sleep.h>
#include <sys/intr.h>

#include <sys/pmdev.h>

int	pppm_save_regs();
int	pppm_restore_regs();

int
pppm_handler(caddr_t private, int ctrl)
{
	struct	pp *pp = (struct pp *)private;
	int	opri;
	int	rc;

	switch (ctrl) {
	case PM_DEVICE_SUSPEND:
	case PM_DEVICE_HIBERNATION:
#ifdef	_POWER_MP
		opri = disable_lock(INTMAX, &pp->intr_lock);
#else	/* _POWER_MP */
		opri = i_disable(INTMAX);
#endif	/* _POWER_MP */
		/* if the DD is opened, reject the request */
		if (pp->pmbusy) {
			rc = PM_ERROR;
		} else {
			/* If mode is in SUSPEND/HIBERNATION,			*/
			/* PM core does not request SUSPEND/HIBERNATION again.	*/
			/* No need to check whether already saved or not.	*/
			pppm_save_regs(pp);
			pp->pmh->mode = ctrl;
			rc = PM_SUCCESS;
		}
#ifdef	_POWER_MP
		unlock_enable(opri, &pp->intr_lock);
#else	/* _POWER_MP */
		i_enable(opri);
#endif	/* _POWER_MP */
		if (rc == PM_ERROR)
			return(PM_ERROR);
#if	0
		/* No need to call pm_planar_control, since control reaches here	*/
		/* if the device is not busy, i.e. the device is powered off.		*/
		if ((ctrl == PM_DEVICE_SUSPEND) && (pp->pmbusy))
			pm_planar_control(pp->pmh->devno, PMDEV_PARALLEL, PM_PLANAR_OFF);
#endif
		break;
	case PM_DEVICE_ENABLE:
	case PM_DEVICE_IDLE:
		switch (pp->pmh->mode) {
		case PM_DEVICE_SUSPEND:
		case PM_DEVICE_HIBERNATION:
#if	0
			/* No need to call pm_planar_control, since control reaches here	*/
			/* if the device was not busy, i.e. the device was powered off.		*/
			if (pp->pmbusy)
				pm_planar_control(pp->pmh->devno, PMDEV_PARALLEL, PM_PLANAR_ON);
#endif
#ifdef	_POWER_MP
			opri = disable_lock(INTMAX, &pp->intr_lock);
#else	/* _POWER_MP */
			opri = i_disable(INTMAX);
#endif	/* _POWER_MP */
			pp->pmh->mode = ctrl;
			pppm_restore_regs(pp);
			if (pp->pmblock) {
				pp->pmblock = FALSE;
				e_wakeup(&(pp->pppmblock));
			}
#ifdef	_POWER_MP
			unlock_enable(opri, &pp->intr_lock);
#else	/* _POWER_MP */
			i_enable(opri);
#endif	/* _POWER_MP */
			break;
		default:
			pp->pmh->mode = ctrl;
		}
		break;
	case PM_DEVICE_FULL_ON:
		if (pp->pmh->mode != PM_DEVICE_ENABLE)
			return(PM_ERROR);
		pp->pmh->mode = ctrl;
		break;
	case PM_PAGE_FREEZE_NOTICE:
		rc = pincode(ppddpin);
		if (rc != 0)
			return(PM_ERROR);
		break;
	case PM_PAGE_UNFREEZE_NOTICE:
		unpincode(ppddpin);
		break;
	}
	return(PM_SUCCESS);
}

int
pppm_save_regs(struct pp *pp)
{
	pp->pmctrlreg = ppreadctrl(pp);
	pp->pmdatareg = ppreaddata(pp);
}

int
pppm_restore_regs(struct pp *pp)
{
	ppwritedata(pp, pp->pmdatareg);
	ppwritectrl(pp, pp->pmctrlreg);
}

#endif	/* PM_PHASE2 */
