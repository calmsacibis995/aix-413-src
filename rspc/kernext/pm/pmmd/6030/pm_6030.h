/* @(#)75   1.6  src/rspc/kernext/pm/pmmd/6030/pm_6030.h, pwrsysx, rspc41J, 9520A_all 5/10/95 13:10:14 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for PM kernel externsion machine dependent code 
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

#ifndef _H_PM_6030
#define _H_PM_6030

#ifndef _KERNEL
#define _KERNEL
#endif /* _KERNEL */

#include <sys/lock_def.h>
#include <sys/intr.h>
#include <sys/adspace.h>
#include <sys/pm.h>


#ifdef PM_DEBUG
#define DBGSTOP  dbgstep();
#define DBGNMB(number)  (void)write_isadev_nolock(0x3bc,number);
#define	DumpOut(number) (void)sDumpOut(number,0,0,0);
#define	DumpRegisters()	(void)sDumpRegisters() 
#else
#define DBGSTOP
#define DBGNMB(number)
#define	DumpOut(number) 
#define	DumpRegisters()	
#endif


/* machine model number */
#define	MODEL_6030	0x5D
#define MODEL_6020	0x4F


/* global data size for planar control handles */
#define	planar_control_handle_max	32


typedef struct _pmmd_control_data {
	int	resume_trigger;
	int	Suspend2HibByRTC;
} pmmd_control_data_t;


typedef struct _pm_HW_state_data{
	Simple_lock   lock;
	int	LID_state;	  /* LID state(need to maintain H/W) */ 
	int	ACDC_state;	  /* AC/DC state(need to maintain H/W) */ 
	int	video_dd_request; /* graphics DD's last on/off request */	
} pm_HW_state_data_t;

/* LID state */
/* Refer to ./sys/pm.h for backward compatibility */

/* AC/DC state */
#define in_AC		0	/* powered by AC */
#define in_DC		1	/* powered by DC (battery) */

 
typedef struct	_pm_ODM_basic_data {    
	int     pmc_base;
	int     pmc_bus_id;
	int     xioc_base;
	int     xioc_bus_id;
	int     intr_level;
	int     intr_priority;
} pm_ODM_basic_data_t;


typedef struct 	_pm_planar_current_value {
	int     lcd_data;
	int     crt_data;
	int     gcc_data;
	int     camera_data;
	int     audio_data;
	int     cdrom_data;
	int	scsi_term;
} pm_planar_current_value_t;


/*----------------------------------------------------*/
/* Definitions for Basic device save/restore function */
/*----------------------------------------------------*/
/*
 * int planardevice_control(int); 
 */
#define	Initialize_planar_device 0
#define	Save_planar_device	1
#define	Restore_planar_device	2



/******************************************************/
/******************************************************/
/*						      */
/*    For Phase 1 PM(4.1.1) backward compatibility    */
/*						      */
/******************************************************/
/******************************************************/
/* PM Machine Dependent Data for model 6020/6030 */
struct _pm_md_data {
	Simple_lock             lock;
	struct pm_machine_data  machine_data;
	int                     pmc_base;
	int                     pmc_bus_id;
	struct io_map           pmc_map;
	int                     xioc_base;
	int                     xioc_bus_id;
	struct io_map           xioc_map;
	struct intr             intr_data;
};

struct _pm_planar_id_struct {
	int     lcd;
	int     crt;
	int     gcc;
	int     camera;
	int     audio;
	int	audio_mute;
	int     cpu;
	int     l2;
	int     psusus;
	int     psumain;
	int     cdrom;
};

struct _pm_device_id {
	struct pm_machine_data  machine_data;
	int     pmc_base;
	int     pmc_bus_id;
	int     xioc_base;
	int     xioc_bus_id;
	int     intr_level;
	int     intr_priority;
	struct _pm_planar_id_struct pmpis;
};

#endif	/* _H_PM_6030 */
