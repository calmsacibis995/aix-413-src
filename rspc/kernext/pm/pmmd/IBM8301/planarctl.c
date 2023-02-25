static char sccsid[] = "@(#)28	1.5  src/rspc/kernext/pm/pmmd/IBM8301/planarctl.c, pwrsysx, rspc41J, 9524E_all 6/14/95 20:37:02";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Power Management Kernel Extension Code
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/pm.h>

#include <pmmi.h>
#include "pm_sig750.h"


/*---------------------------------*/
/*	external functions 	   */
/*---------------------------------*/

/*---------------------------------*/
/*	external vars   	   */
/*---------------------------------*/
extern pm_md_data_t		pm_md_data;
extern struct _pm_control_data	pm_control_data;

/*---------------------------------*/
/*	global var declaration     */
/*---------------------------------*/

/*---------------------------------*/
/*	proto type declaration     */
/*---------------------------------*/
void	cpu_idle();
void	turn_power_off();
/*
 *  This code removed for defect 180490 ... see comments in pmmdconfig.c
 * int	pm_planar_control_audio(dev_t devno, int devid, int cmd);
 */
int	pm_planar_control_ent(dev_t devno, int devid, int cmd);


/*
 * NAME:  cpu_idle
 *
 * FUNCTION:  Routine called from waitproc to idle the CPU if possible.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	The 604 we can put in NAP mode.  The 601 we can't do anything.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      None.
 */
void
cpu_idle()
{
	if (__power_604() &&
	    (pm_control_data.system_state == PM_SYSTEM_STANDBY)) {
		SetMsrPow();
	}
}

/*
 * NAME:  turn_power_off
 *
 * FUNCTION:  Routine called from reboot system call to turn power off
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *	This routine is also called from pmmd_entry(TURN_POWER_OFF)
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      None.
 */
void
turn_power_off()
{
	/*
	 * Signetics FW work-around (PPS defects 1352 & 1353):
	 * the LED will continue to flash when powered back on unless
	 * we stop it here.  We also have to make sure we leave the LED on.
	 */
	write_ctl_bit(FAST_FLASH_LED_BYTE, FAST_FLASH_LED_BIT, 0);

	write_host_pin(LED_CONTROL, 0);

	/*
	 * Disable failsafe timer before turning power off so it
	 * will be instantaneous.
	 */
	write_ctl_bit(ENABLE_FAILSAFE_BYTE, ENABLE_FAILSAFE_BIT, 0);

	write_ctl_bit(PM_EXEC_OFF_BYTE, PM_EXEC_OFF_BIT, 1);
}

/*
 * NAME:  pm_planar_control_audio
 *
 * FUNCTION:  provides PM functions for audio device
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      None.
 *
 * This code was removed for defect 180490 ... see comments in pmmdconfig.c
 *
 * int
 * pm_planar_control_audio(dev_t devno, int devid, int cmd)
 * {
 *	switch (cmd) {
 *	case PM_PLANAR_ON:
 *		write_host_pin(AUDIO_PWR_DOWN, 1);
 *		return PM_SUCCESS;
 *	case PM_PLANAR_OFF:
 *		write_host_pin(AUDIO_PWR_DOWN, 0);
 *		return PM_SUCCESS;
 *	case PM_PLANAR_LOWPOWER1:
 *	case PM_PLANAR_LOWPOWER2:
 *		return PM_ERROR;
 *	case PM_PLANAR_QUERY:
 *		return (PM_PLANAR_ON | PM_PLANAR_OFF);
 *	case PM_PLANAR_CURRENT_LEVEL:
 *		if (read_host_pin(AUDIO_PWR_DOWN) == 0) {
 *			return PM_PLANAR_OFF;
 *		} else {
 *			return PM_PLANAR_ON;
 *		}
 *	default:
 *		return PM_ERROR;
 *	}
 * }
 */

/*
 * NAME:  pm_planar_control_ent
 *
 * FUNCTION:  provides PM functions for integrated ethernet
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      None.
 */
int
pm_planar_control_ent(dev_t devno, int devid, int cmd)
{
	/*
	 * This planar_ctl routine was registered for all pluggable PCI
	 * devices.  We can only do something for the integrated ethernet.
	 * Currently we know (for Carolina) that the bus number is 0, and
	 * the devfunc is 0x60 (device number 12), so we'll just hardcode them.
	 */
#define ENT_BUS		0
#define ENT_DEVFUNC	0x60

	if (devid == (PMDEV_PCI0000 | (ENT_BUS << 8) | ENT_DEVFUNC)) {
		switch (cmd) {
		case PM_PLANAR_ON:
			write_host_pin(ETHER_SLEEP, 1);
			return PM_SUCCESS;
		case PM_PLANAR_OFF:
			return PM_ERROR;
		case PM_PLANAR_LOWPOWER1:
			write_host_pin(ETHER_SLEEP, 0);
			return PM_SUCCESS;
		case PM_PLANAR_LOWPOWER2:
			return PM_ERROR;
		case PM_PLANAR_QUERY:
			return (PM_PLANAR_ON | PM_PLANAR_LOWPOWER1);
		case PM_PLANAR_CURRENT_LEVEL:
			if (read_host_pin(ETHER_SLEEP) == 1) {
				return PM_PLANAR_ON;
			} else {
				return PM_PLANAR_LOWPOWER1;
			}
		default:
			return PM_ERROR;
		}
	} else {
		/*
		 *  Not the integrated ethernet.  Return an error.
		 */
		return PM_ERROR;
	}
}
