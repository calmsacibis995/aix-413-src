static char sccsid[] = "@(#)66  1.4  src/rspc/kernext/pm/pmmd/6030/pmmdconfig.c, pwrsysx, rspc41J, 9517A_all 4/24/95 08:12:41";
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
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/pm.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>

#include "../../pmmi/pmmi.h"
#include "pm_6030.h"
#include "planar.h"
#include "slave.h"


/*---------------------------------*/
/*	external functions 	   */
/*---------------------------------*/
extern	int pm_event_kproc();
extern	int establish_pmmi_interface(int);
extern 	int planardevice_control(int);
extern  void init_battery_func(int cmd); 


/*---------------------------------*/
/*	external data symbols      */
/*---------------------------------*/
extern	struct _pm_md_data pm_md_data;
extern BATTERY_CTRL battery_control; /* Wait for battery report comp */
extern pm_machine_profile_t	machine_profile;
extern pmmd_IF_t pmmd_IF;

/*---------------------------------*/
/*	proto type declaration     */
/*---------------------------------*/
void create_machine_data();			/* for backward compatibility */
void create_planar_id_struct
	(struct _pm_planar_id_struct *p);   	/* for backward compatibility */
int  pmmdconfig(int cmd, struct uio *uiop);


/* * NAME:  pmmdconfig
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
    pm_ODM_basic_data_t pmdi;
    struct _pm_planar_id_struct	pm_planar_id_struct;

    DBG(pmmdconfig.c entry)
	
    switch(cmd){
    case PM_CONFIG :
	pincode(pm_event_kproc);
	if(uiomove((char *)&pmdi, 
		   sizeof(pm_ODM_basic_data_t), 
		   UIO_WRITE, uiop)
		  ){
	    unpincode(pm_event_kproc);
	    return -1;
	}

	lock_alloc(&(pm_md_data.lock), LOCK_ALLOC_PIN,
					PM_CORE_LOCK_CLASS, -1);
	simple_lock_init(&(pm_md_data.lock));


	simple_lock(&(pm_md_data.lock));

	/* For Power Management Controller */
	pm_md_data.pmc_base = pmdi.pmc_base;
	pm_md_data.pmc_bus_id = pmdi.pmc_bus_id;

	/* For eXtended I/O Controller */
	pm_md_data.xioc_base = pmdi.xioc_base;
	pm_md_data.xioc_bus_id = pmdi.xioc_bus_id;

	pm_md_data.intr_data.level = pmdi.intr_level;
	pm_md_data.intr_data.priority = pmdi.intr_priority;

	create_machine_data();		/* for backward compatibility */
	simple_unlock(&(pm_md_data.lock));

	create_planar_id_struct(&pm_planar_id_struct);	
					/* for backward compatibility */


	DBG(calling establish_pmmi_interface in pmmdconfig.c)
	if(establish_pmmi_interface(PM_CONFIG) != 0) {
	    unpincode(pm_event_kproc);
	    return -1;
	}
	DBG(calling planardevice_control(init) in pmmdconfig.c)
	(void)planardevice_control(Initialize_planar_device);

	if(init_planar(cmd, &pm_planar_id_struct) != 0){
	    lock_free(&(pm_md_data.lock));
	    unpincode(pm_event_kproc);
	    return -1;
	}

	DBG(Call init_interrupt_job)
	if(init_interrupt_job(cmd, &pm_planar_id_struct) != 0){
	    init_planar(PM_UNCONFIG, NULL);
	    lock_free(&(pm_md_data.lock));
	    unpincode(pm_event_kproc);
	    return -1;
	}

	DBG(Call init_battery_func)
	(void)init_battery_func(cmd);

	DBG(exit of pmmdconfig.c)
	return 0;


    case PM_UNCONFIG :
        simple_lock(&(pmmd_IF.lock));
	(void)init_planar(cmd, NULL);
	(void)init_interrupt_job(cmd, NULL);
	(void)init_battery_func(cmd);
	establish_pmmi_interface(PM_UNCONFIG);
        simple_unlock(&(pmmd_IF.lock));

	lock_free(&(pm_md_data.lock));
	unpincode(pm_event_kproc);
	DBG(exit of pmmdconfig.c)
	return 0;
    }

    return -1;
}


/*
 * NAME: create_machine_data
 *
 * FUNCTION: build the data of the structure of "machine_data" 
 *	  	for backward compatiblity.
 * 
 * ENVIRONMENT: This routine needs to be called while pm_md_data 
 * 		structure is already locked.
 * INPUT: none
 * OUTPUT: none
 *
 */
/* for backward compatibility for Woodfield phase_1 PM */
void
create_machine_data()		
{
	
	pm_md_data.machine_data.model = MODEL_6020;
        pm_md_data.machine_data.lid = SUPPORTED; 
        pm_md_data.machine_data.battery = SUPPORTED;
        pm_md_data.machine_data.soft_switch = SUPPORTED;
        pm_md_data.machine_data.suspend_ext_kbd = SUPPORTED;
        pm_md_data.machine_data.suspend_ext_sig = NOT_SUPPORTED;
        pm_md_data.machine_data.suspend_ri = SUPPORTED;
        pm_md_data.machine_data.suspend_rtc = SUPPORTED;
        pm_md_data.machine_data.suspend_lid = SUPPORTED;
        pm_md_data.machine_data.hibernation_ext_kbd = SUPPORTED;
        pm_md_data.machine_data.hibernation_ext_sig = NOT_SUPPORTED;
        pm_md_data.machine_data.hibernation_ri = NOT_SUPPORTED;
        pm_md_data.machine_data.hibernation_rtc = NOT_SUPPORTED;
	return;
}


/*
 * NAME: create_planar_id_struct
 *
 * FUNCTION: build the data of the structure of "planar_id_struct"
 *	  	for backward compatiblity.
 * 
 * INPUT: none
 * OUTPUT: none
 *
 */
void
create_planar_id_struct(struct _pm_planar_id_struct *p)	
/* for backward compatibility */
{
        p->lcd = PMDEV_LCD;
        p->crt = PMDEV_CRT;
        p->gcc = PMDEV_GCC;
        p->camera = PMDEV_CAMERA;
        p->audio = PMDEV_AUDIO;
	p->audio_mute = PMDEV_AUDIO_EXT_MUTE; 
        p->cpu = PMDEV_CPU;
        p->l2 = NULL;
        p->psusus = PMDEV_PSUSUS;
        p->psumain = PMDEV_PSUMAIN;
        p->cdrom = NULL;	
		/* This cd-rom id is not available any more */
	return;
}


