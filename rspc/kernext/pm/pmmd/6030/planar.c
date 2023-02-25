static char sccsid[] = "@(#)61  1.7  src/rspc/kernext/pm/pmmd/6030/planar.c, pwrsysx, rspc41J, 9517A_all 4/24/95 10:57:30";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Planar control routines for Power Management (PM)
 *              init_planar, init_pmc_data, init_isadev_data, enable_pmc,
 *              enable_pm_planar_control, init_pm_planar_control,
 *              unconfig_pm_planar_control
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
 *
 *
 *   STATUS:
 *      07/13/94
 *      08/03/94  add cpu_idle, unconfig_pm_planar_control
 *      08/04/94  remove unnecessary initialize
 *      08/05/94  separate I/O routine  w, w/o lock
 *      08/05/94  separate pinned part and unpinned part
 *      08/24/94  L2 cache support
 *      03/08/95  L2 cache support is dropped.
 */

/* INCLUDE */

#include <sys/adspace.h>
#include <sys/ioacc.h>
#include <sys/lock_def.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/pm.h>
#include "planar.h"
#include "pm_6030.h"


/* DATA,STRUCTURE DEFINITION */

extern struct _pm_pmc_data      pm_pmc_data;

extern struct pm_planar_control_handle ppch[planar_control_handle_max];

extern struct _pm_isadev_data  pm_isadev_data;


/* FUNCTION PROTOTYPE DEFINITION */

static void init_pmc_data();
static void init_isadev_data();
static int init_pm_planar_control();
static void unconfig_pm_planar_control();

/* EXTERN */

extern int  pm_planar_control_psumain();
extern struct _pm_kernel_data pm_kernel_data;

extern int  pm_planar_control_lcd();
extern int  pm_planar_control_crt();
extern int  pm_planar_control_gcc();
extern int  pm_planar_control_camera();
extern int  pm_planar_control_audio();
extern int  pm_planar_control_audio_mute();
extern int  pm_planar_control_cpu();
extern int  pm_planar_control_psusus();
extern int  pm_planar_control_cdrom();
extern void cpu_idle();
extern void turn_power_off();


/******************************************************************************
 * NAME: init_planar
 *
 * FUNCTION: Initialize planar for PM
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      cmd: PM_CONFIG or PM_UNCONFIG
 *      pmpis: a pointer to the _pm_planar_id_struct
 *
 * RETURN VALUE:
 *      0:  success
 *      -1: error
 */
int
init_planar(int cmd, struct _pm_planar_id_struct *pmpis)
{
        switch( cmd ){
        case PM_CONFIG :
                lock_alloc(&(pm_pmc_data.lock), LOCK_ALLOC_PIN,
                                                PM_CORE_LOCK_CLASS, -1);
                simple_lock_init(&(pm_pmc_data.lock));
                (void)init_pmc_data();
                lock_alloc(&(pm_isadev_data.lock), LOCK_ALLOC_PIN,
                                                PM_CORE_LOCK_CLASS, -1);
                simple_lock_init(&(pm_isadev_data.lock));
                (void)init_isadev_data();

                /* MSR DPM */
                SetHid0Dpm();

                return(init_pm_planar_control( pmpis ));
        case PM_UNCONFIG :

                /* MSR DPM */
                ResetHid0Dpm();

                (void)unconfig_pm_planar_control( pmpis );
                lock_free(&(pm_isadev_data.lock));
                lock_free(&(pm_pmc_data.lock));
                break;

        default :
                return -1;
        }
        return 0;

}


/******************************************************************************
 * NAME: init_pmc_data
 *
 * FUNCTION: Initialize Power Management Controller data
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      none
 *
 * RETURN VALUE:
 *      none
 */
static void
init_pmc_data()
{
extern struct _pm_md_data  pm_md_data;

        simple_lock(&(pm_pmc_data.lock));

        pm_pmc_data.base = pm_md_data.pmc_base;
        pm_pmc_data.bus_id = pm_md_data.pmc_bus_id;
        pm_pmc_data.map.key = IO_MEM_MAP;
        pm_pmc_data.map.flags = 0;
        pm_pmc_data.map.size = SEGSIZE;
        pm_pmc_data.map.bid = pm_pmc_data.bus_id;
        pm_pmc_data.map.busaddr = 0;

        simple_unlock(&(pm_pmc_data.lock));
}

/******************************************************************************
 * NAME: init_isadev_data
 *
 * FUNCTION: Initialize isa data
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      none
 *
 * RETURN VALUE:
 *      none
 */
static void
init_isadev_data()
{
extern struct _pm_md_data  pm_md_data;

        simple_lock(&(pm_isadev_data.lock));

        pm_isadev_data.bus_id = pm_md_data.xioc_bus_id;
        pm_isadev_data.map.key = IO_MEM_MAP;
        pm_isadev_data.map.flags = 0;
        pm_isadev_data.map.size = SEGSIZE;
        pm_isadev_data.map.bid = pm_isadev_data.bus_id;
        pm_isadev_data.map.busaddr = 0;

        simple_unlock(&(pm_isadev_data.lock));
}

/******************************************************************************
 * NAME: enable_pmc
 *
 * FUNCTION: PM control chip enable
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      none
 *
 * RETURN VALUE:
 *      none
 */
void
enable_pmc()
{
    modify_pmc( INDEXPMC_PM_CNTRL,
                        BITPMCPMCNTRL_PM_EN, SET_BITS );
    return;
}

/******************************************************************************
 * NAME: enable_pm_planar_control
 *
 * FUNCTION: planar power on
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      pmpis: a pointer to the _pm_planar_id_struct
 *
 * RETURN VALUE:
 *      none
 */
void
enable_pm_planar_control( struct _pm_planar_id_struct *pmpis )
{

        /* enable Power Management Controller */
        enable_pmc();

        /* turn on LCD */
        pm_planar_control(0, pmpis->lcd, PM_PLANAR_ON);

        /* turn on CRT */
        pm_planar_control(0, pmpis->crt, PM_PLANAR_ON);

        /* turn on graphic controller */
	/* Graphics DD has no plan to utilize WDC pm feature */
        pm_planar_control(0, pmpis->gcc, PM_PLANAR_ON);

}


/******************************************************************************
 * NAME: init_pm_planar_control
 *
 * FUNCTION: planar PM initialization
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *
 * INPUT:
 *      pmpis: a pointer to the _pm_planar_id_struct
 *
 * RETURN VALUE:
 *      0:  success
 *      -1: error
 */
int
init_pm_planar_control( struct _pm_planar_id_struct *pmpis )
{
    int     i;

    for(i = 0; i <= planar_control_handle_max; i++){
		ppch[i].devid = NULL;
                ppch[i].control = NULL; 
        	ppch[i].next = NULL;
    } /* endfor */


    for(i = 0;;) {

        /* LCD */
        ppch[i].devid = pmpis->lcd;
        ppch[i].control = pm_planar_control_lcd;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* CRT */
        /* register CRT PM control */
        ppch[i].devid = pmpis->crt;
        ppch[i].control = pm_planar_control_crt;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* Graphic Controller */
        /* register Graphic Controller PM control */
        ppch[i].devid = pmpis->gcc;
        ppch[i].control = pm_planar_control_gcc;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* CCD */
        ppch[i].devid = pmpis->camera;
        ppch[i].control = pm_planar_control_camera;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* Audio */
        ppch[i].devid = pmpis->audio;
        ppch[i].control = pm_planar_control_audio;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* Audio mute */
        ppch[i].devid = pmpis->audio_mute;
        ppch[i].control = pm_planar_control_audio_mute;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER ) 
								!= PM_SUCCESS){
		break;
        }
	i++;

        /* CPU */
        ppch[i].devid = pmpis->cpu;
        ppch[i].control = pm_planar_control_cpu;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* Suspend Power */
        ppch[i].devid = pmpis->psusus;
        ppch[i].control = pm_planar_control_psusus;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

        /* Main Power */
        ppch[i].devid = pmpis->psumain;
        ppch[i].control = pm_planar_control_psumain;
        if(pm_register_planar_control_handle(&(ppch[i]), PM_REGISTER)
                                                                != PM_SUCCESS){
		break;
        }
	i++;

#if 0
        /* CD-ROM as INTERNAL_SCSI */
        ppch[i].devid = PMDEV_INTERNAL_SCSI;
        ppch[i].control = pm_planar_control_cdrom;
        if(pm_register_planar_control_handle(&(ppch[i]),
                        PM_REGISTER | PMDEV_MAJOR_NUMBER) != PM_SUCCESS){
		break;
        }
	i++;

        /* CD-ROM as UNKNOWN_SCSI */
        ppch[i].devid = PMDEV_UNKNOWN_SCSI;
        ppch[i].control = pm_planar_control_cdrom;
        if(pm_register_planar_control_handle(&(ppch[i]),
                        PM_REGISTER | PMDEV_MAJOR_NUMBER) != PM_SUCCESS){
		break;
        }
	i++;
#endif

	/* for next new planar control registration */
	/*  NEXT planar control is here */


        /* register cpu_idle to pm_kernel_data */
        pm_kernel_data.cpu_idle = NULL;
        pm_kernel_data.cpu_idle_pm_core = cpu_idle;

        /* register turn_power_off to pm_kernel_data */
        pm_kernel_data.turn_power_off = NULL;
        pm_kernel_data.turn_power_off_pm_core = turn_power_off;

        /* All Planar Power On */
        enable_pm_planar_control(pmpis);

        return 0;

    } /* endfor */

    unconfig_pm_planar_control(pmpis);
    return -1;

} /* init_pm_planar_control */


/******************************************************************************
 * NAME: unconfig_pm_planar_control
 *
 * FUNCTION: planar PM unconfig.  Detach from registered chain.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from the process environment only.
 *      It can page fault.
 *
 * NOTES:
 *      Refer init_pm_planar_control() to know ppch[] item number for
 *      each device.
 *
 * INPUT:
 *      pmpis: a pointer to the _pm_planar_id_struct
 *
 * RETURN VALUE:
 *      none
 */
void
unconfig_pm_planar_control( struct _pm_planar_id_struct *pmpis )
{
	int	i;
	volatile struct _pm_kernel_data	*pmkdptr=&pm_kernel_data;
	
	for (i = 0; i < planar_control_handle_max; i++) {
 
        	if(ppch[i].control != NULL){
                	pm_register_planar_control_handle
					(&(ppch[i]), PM_UNREGISTER);
        	} /* endif */

	} /* endfor */

        /* unregister cpu_idle from pm_kernel_data */
        pmkdptr->cpu_idle_pm_core = NULL;

	/* make sure that kernel knows the pointer change */
	for(;;){
		if(pmkdptr->cpu_idle == NULL){
			break;
		}
		/* wait until kernel knows the pointer change */
		delay(10);
	}

        /* unregister turn_power_off from pm_kernel_data */
        pmkdptr->turn_power_off_pm_core = NULL;

	/* make sure that kernel knows the pointer change */
	for(;;){
		if(pmkdptr->turn_power_off == NULL){
			break;
		}
		/* halt process already began. */
		/* machine will be turned off before PM can be unloaded. */
		delay(10);
	}
}
