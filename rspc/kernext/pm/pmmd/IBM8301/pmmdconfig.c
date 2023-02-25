static char sccsid[] = "@(#)31	1.9  src/rspc/kernext/pm/pmmd/IBM8301/pmmdconfig.c, pwrsysx, rspc41J, 9524E_all 6/14/95 20:36:46";
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
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#define  _RSPC
#define  _RSPC_UP_PCI
#include <sys/system_rspc.h>
#undef   _RSPC_UP_PCI
#undef	 _RSPC 

#include <pmmi.h>
#include "pm_sig750.h"


/*---------------------------------*/
/*	external functions 	   */
/*---------------------------------*/
extern  int pmmd_entry(int cmd, caddr_t arg);
extern	int pm_intr(struct intr *p_intr);
extern	void cpu_idle();
extern	void turn_power_off();
/*
 *  This code removed for defect 180490 ... see comments below
 *  extern  int pm_planar_control_audio(dev_t devno, int devid, int cmd);
 */
extern	int pm_planar_control_ent(dev_t devno, int devid, int cmd);

/*---------------------------------*/
/*	external vars   	   */
/*---------------------------------*/
extern pmmd_IF_t		pmmd_IF;
extern struct _pm_kernel_data	pm_kernel_data;

/*---------------------------------*/
/*	global var declaration     */
/*---------------------------------*/
struct _pm_md_data pm_md_data = { 0 };

/*---------------------------------*/
/*	proto type declaration     */
/*---------------------------------*/
int  pmmdconfig(int cmd, struct uio *uiop);
void init_signetics();


/*
 * NAME:  pmmdconfig
 *
 * FUNCTION:  configure pmmd
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      This routine is entry point to configure PM machine dependent
 *      part.
 *
 * DATA STRUCTURES:
 *      _pm_device_id - PM Machine dependet data structure
 *      uio - user i/o area structure
 *
 * RETURN VALUE DESCRIPTION:	
 *      0  : PM initialization success.
 *      -1 : error.
 */
int
pmmdconfig(int cmd, struct uio *uiop)
{
	int	reg,rc=0;
        struct rspcsio *p;

	DBG(pmmdconfig.c entry)
    switch(cmd){
    case PM_CONFIG :

	/*
	 * Pin the code
	 */
	pincode(pmmdconfig);

	/*
	 * Copy the info passed from config
	 */
	if(uiomove((char *)&(pm_md_data.pmdi), 
		   sizeof(pm_ODM_basic_data_t), 
		   UIO_WRITE, uiop)
		  ){
	    unpincode(pmmdconfig);
	    return -1;
	}

	/*
	 * Allocate and initialize lock
	 */
	lock_alloc(&(pm_md_data.lock), LOCK_ALLOC_PIN, PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(pm_md_data.lock));

	/*
	 * Establish pmmd_entry interface with PMMI
	 */
	pmmd_IF.pmmd_entry = (int(*)())pmmd_entry;

	/*
	 * Set up bid, etc. for iomem_att calls.
	 */
	pm_md_data.io_map.key = IO_MEM_MAP;
	pm_md_data.io_map.flags = 0;
	pm_md_data.io_map.size = SEGSIZE;
	pm_md_data.io_map.bid = pm_md_data.pmdi.isa_bus_id;
	pm_md_data.io_map.busaddr = 0;

	pm_md_data.pm_RTC_data.base = (&(p->rtc_index) - (char *)p);
	pm_md_data.pm_RTC_data.map.key = IO_MEM_MAP;
	pm_md_data.pm_RTC_data.map.flags = 0;
	pm_md_data.pm_RTC_data.map.size = SEGSIZE;
	pm_md_data.pm_RTC_data.map.bid = pm_md_data.pmdi.isa_bus_id;
	pm_md_data.pm_RTC_data.map.busaddr = 0;

	/*
	 * Call PmSystemInitialize to set up OO data structs
	 */
	DBG(calling planardevice_control(init) in pmmdconfig.c)
	pm_md_data.io_addr = (void *)iomem_att(&(pm_md_data.io_map));
	PmSystemInitialize();
	iomem_det(pm_md_data.io_addr);

	/*
	 * Set up planar_control handlers (ethernet and audio)
	 */
	pm_md_data.ent_planar_ctl.devid = PMDEV_PCI0000;
	pm_md_data.ent_planar_ctl.control = pm_planar_control_ent;
	pm_md_data.ent_planar_ctl.next = NULL;
	rc = pm_register_planar_control_handle(&pm_md_data.ent_planar_ctl,
	    PM_REGISTER | PMDEV_MAJOR_NUMBER);
	if (rc) {
		DBG(Failed to register ent planar_ctl)
		lock_free(&(pm_md_data.lock));
		unpincode(pmmdconfig);
		return -1;
	}

/*
 *  Due to defect 180490 this code has been removed.  There is a hardware
 *  problem when turning off and back on the audio controller with no
 *  delay in between.  Since the power savings of turning off the
 *  audio controller is minimal, this funcionality is removed.
 *
 *	pm_md_data.audio_planar_ctl.devid = PMDEV_AUDIO;
 *	pm_md_data.audio_planar_ctl.control = pm_planar_control_audio;
 *	pm_md_data.audio_planar_ctl.next = NULL;
 *	rc = pm_register_planar_control_handle(&pm_md_data.audio_planar_ctl,
 *	    PM_REGISTER);
 *	if (rc) {
 *		DBG(Failed to register audio planar_ctl)
 *		(void)pm_register_planar_control_handle(
 *		    &pm_md_data.ent_planar_ctl, PM_UNREGISTER);
 *		lock_free(&(pm_md_data.lock));
 *		unpincode(pmmdconfig);
 *		return -1;
 *	}
 */
	/*
	 * Register interrupt handler
	 */
	DBG(Call init_interrupt_job)
	pm_md_data.intr.next     = NULL;
	pm_md_data.intr.handler  = (int (*) ())pm_intr;
	pm_md_data.intr.bus_type = BUS_BID;
	pm_md_data.intr.flags    = INTR_LEVEL;
	pm_md_data.intr.level    = pm_md_data.pmdi.intr_level;
	pm_md_data.intr.priority = pm_md_data.pmdi.intr_priority;
	pm_md_data.intr.bid      = pm_md_data.pmdi.isa_bus_id;

        if (i_init(&(pm_md_data.intr)) != INTR_SUCC) {
	    DBG(i_init failed)
            pm_md_data.intr.handler = NULL;
/*
 *  This code removed for 180490 ... see comment above
 *	    (void) pm_register_planar_control_handle(
 *		&pm_md_data.audio_planar_ctl, PM_UNREGISTER);
 */
	    (void)pm_register_planar_control_handle(&pm_md_data.ent_planar_ctl,
		PM_UNREGISTER);
	    lock_free(&(pm_md_data.lock));
	    unpincode(pmmdconfig);
	    return -1;
	}

	/*
	 * Establish cpu_idle() and turn_power_off() interfaces with PMMI
	 */
        pm_kernel_data.cpu_idle = NULL;
        pm_kernel_data.cpu_idle_pm_core = cpu_idle;
        pm_kernel_data.turn_power_off = NULL;
        pm_kernel_data.turn_power_off_pm_core = turn_power_off;

	/*
	 *  Initialize the Signetics hardware/firmware
	 */
	init_signetics();

	DBG(exit of pmmdconfig.c)
	return 0;


    case PM_UNCONFIG :
	/*
	 * Clear pmmd_entry interface with PMMI
	 */
	simple_lock(&(pmmd_IF.lock));
	pmmd_IF.pmmd_entry = NULL;
	simple_unlock(&(pmmd_IF.lock));

	/*
	 * Clear cpu_idle() and turn_power_off() interfaces with PMMI
	 */
        /* unregister cpu_idle from pm_kernel_data */
        pm_kernel_data.cpu_idle_pm_core = NULL;

	/* make sure that kernel knows the pointer change */
	for(;;){
		if(pm_kernel_data.cpu_idle == NULL){
			break;
		}
		/* wait until kernel knows the pointer change */
		delay(10);
	}

        /* unregister turn_power_off from pm_kernel_data */
        pm_kernel_data.turn_power_off_pm_core = NULL;

	/* make sure that kernel knows the pointer change */
	for(;;){
		if(pm_kernel_data.turn_power_off == NULL){
			break;
		}
		/* halt process already began. */
		/* machine will be turned off before PM can be unloaded. */
		delay(10);
	}

	/*
	 * Unregister planar_control_handlers
	 */
/*
 *  This code removed for defect 180490 ... see comments above.
 *	(void) pm_register_planar_control_handle(
 *		&pm_md_data.audio_planar_ctl, PM_UNREGISTER);
 */
	(void)pm_register_planar_control_handle(&pm_md_data.ent_planar_ctl,
		PM_UNREGISTER);

	/*
	 * Uninitialize Signetics
	 * 1) Disable PMI in PM control register 2
	 * 2) Disable the failsafe timer (clear ENABLE_FAILSAFE)
	 */

	/*
	 * When writing port 82B, be sure not to write a 1 to bit 0
	 * because it'll reset the signetics.
	 */
	reg = read_port(pm_md_data.pmdi.pmc_port2);
	write_port(pm_md_data.pmdi.pmc_port2, ((reg & ~PMI_ENABLE) & 0x0fe));

	write_ctl_bit(ENABLE_FAILSAFE_BYTE, ENABLE_FAILSAFE_BIT, 0);

	if (pm_md_data.intr.handler != NULL) {
		i_clear(&(pm_md_data.intr));
		pm_md_data.intr.handler = NULL;
	}

	lock_free(&(pm_md_data.lock));
	unpincode(pmmdconfig);
	DBG(exit of pmmdconfig.c)
	return 0;
    }

    return -1;
}

/*
 * NAME:  init_signetics
 *
 * FUNCTION:  Initialize signetics PM controller
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process environment
 *      or the interrupt environment.  It can not page fault.
 *
 * NOTES:
 *      This routine is called from the pmmdconfig entry point and from
 *	PmSystemResume() after a hibernation resume.
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      None.
 */
void
init_signetics()
{
	int	reg;

	/*
	 *  Initialize Signetics
	 *  1)  Enable AIX PM implementation (set SYSTEM_ID0 to '1')
	 *  2)  Change the default failsafe value (FAILSAFE_HOLD)
	 *  3)  Reload the failsafe timer (set CLR_FAILSAFE to 1)
	 *  4)  Enable the failsafe timer (set ENABLE_FAILSAFE to 1)
	 *  5)  Clear PMI_OFF so there is no interrupt right before
	 *      power-off
	 *  6)  Make sure the LED is not blinking (clear FAST_FLASH_LED)
	 *  7)  Make sure the LED is on (write LED_CONTROL pin = 0)
	 *  8)  Make sure the MODEM RING is disabled (clear EXT_WAKEUP)
	 *  9)  Make sure the ALARM is disabled (clear ACTIVATE_ALARM)
	 *  10) Enable PMI in PM control register 2 (82B)
	 */

	write_ctl_bit(SYSTEM_ID0_BYTE, SYSTEM_ID0_BIT, 1);

	write_data_word(FAILSAFE_HOLD, FAILSAFE_HOLD_VAL);

	write_ctl_bit(CLR_FAILSAFE_BYTE, CLR_FAILSAFE_BIT, 1);

	write_ctl_bit(ENABLE_FAILSAFE_BYTE, ENABLE_FAILSAFE_BIT, 1);

	write_ctl_bit(PMI_OFF_BYTE, PMI_OFF_BIT, 0);

	write_ctl_bit(FAST_FLASH_LED_BYTE, FAST_FLASH_LED_BIT, 0);

	write_host_pin(LED_CONTROL, 0);

	write_ctl_bit(EXT_WAKEUP_ENABLE_BYTE, EXT_WAKEUP_ENABLE_BIT, 0);

	write_ctl_bit(ACTIVATE_ALARM_BYTE, ACTIVATE_ALARM_BIT, 0);

	/*
	 * When writing port 82B, be sure not to write a 1 to bit 0
	 * because it'll reset the signetics.
	 */
	reg = read_port(pm_md_data.pmdi.pmc_port2);
	write_port(pm_md_data.pmdi.pmc_port2, ((reg | PMI_ENABLE) & 0x0fe));
}
