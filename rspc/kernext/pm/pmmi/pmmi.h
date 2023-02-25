/* @(#)05   1.13  src/rspc/kernext/pm/pmmi/pmmi.h, pwrsysx, rspc41J 6/7/95 23:37:24 */
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: Header file for PM kernel externsion machine independent code 
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


#ifndef _H_PMMI 
#define _H_PMMI

/* Debug print */
#ifdef PM_DEBUG
#define	DBG(msg) printf("msg\n");
#define DOUT(nmb) CALL_PMMD_WO_LOCK(pmmd_dbgout,nmb)
#else
#define DBG(msg)
#define DOUT(nmb) 
#endif

/*************************************************************************/
/*   Battery control on the interface with PMMD		   	   	 */	
/*************************************************************************/
/*************************************************************************/
/*   Miscellaneous definitions                                           */
/*************************************************************************/
/* Definition for PM Kernel Process */
#define PM_SYSTEM_IDLE_INTERVAL     5
#define PM_KPROC_INTERVAL     1

/* PM command */
#define PM_NOW    1
#define PM_REBOOT 2
#define PM_BOTH      3

/* PM interrupt mask */
#define	Enable_PM_int	1
#define	Disable_PM_int	0


/*----------------------------------------------------*/
/* Definitions for Basic device save/restore function */
/*----------------------------------------------------*/
/*
 * int planardevice_controli(int); 
 */
#define	Initialize_planar_device 0
#define	Save_planar_device	1
#define	Restore_planar_device	2



/*************************************************************************/
/*   Interface with hibernation module in pmmi                           */
/*************************************************************************/
/* return code */
  /* hibernation initialization */


/*************************************************************************/
/*   Interface with broadcast module in pmmi                     	 */
/*************************************************************************/
/*
int broadcast_event(broadcast_block_t *bc_blk)
*/
/* bc_blk */
typedef	struct	_broadcast_block{
	int	control;		/* (input) pm event to PM aware DD */
	int	attribute;		/* to specify with attribute       */  
	char	device_logical_name[16];/* (output) ptr to PMDD name if fail */ 
	int	order_in_list;		/* position number in DD list    */
	struct pm_handle * pm_handle_ptr; /* point to forcal pm_handle   */ 
} broadcast_block_t;

/* specified position of DD list */
#define FirstOne	0
#define	LastOne		1

#define BACKWARD	0
#define	FORWARD		1


/*************************************************************************/
/*   PMMI internal data/interface definitions for pmmd			 */
/*************************************************************************/

/* queue for unresolved external event */
struct _ext_event_queue {
	int	event;
	struct _ext_event_queue *next;
};

typedef struct _ext_event_queue ext_event_q_t;

typedef struct _event_query_control{
        Simple_lock     lock;
	int		event_wait; 	/* sleep flag for event query */
	volatile int	now_queried; 	/* Indicate if event query is called.*/
	int		in_sleep;	/* To handle a critical window */
	ext_event_q_t   *event_queue_begin; /* list for queued events*/	
	ext_event_q_t 	*event_empty_queue_begin; /* list for empty cells */ 
	ext_event_q_t	pickup_event;   /* Used for picked-up event */
	ext_event_q_t	queue[16]; 	/* Event queue ( 16 cells) */
} event_query_control_t;


/* PMMI control block */
typedef struct _pm_control_data{
	int	phase_1_only;	/* Set in case of old PMD or old PM aware DD*/
        volatile int system_state;/* Indicate the current system state */
	int	pmmi_init_comp;	/* Set after calling second_init */
	int	pmd_type;	/* Type of PMD which is currently running */
	caddr_t	former_xmalloc_ptr;/* addrees obtained for device logical name*/
	int  * 	bd_xmalloc_ptr; /* address which was obtained for broadcasting*/
	int  *  bd_xmalloc_work;/* working pointer of bd_xmalloc_ptr */
	void *	xpm_sector;	/* hibernation volume information (sector map)*/
        int     system_idle_count;/* Counter to count up the system idleness */
	int	last_resume_event;/* Indicate the type of resume event */
	int	last_active_specified_time; /* actual time when specified time 
								was adopted.*/
	int	alternate_syncd_stimer; /* timer for issuing sync() in kproc */
	int	alternate_syncd_ltimer; /* timer for issuing sync() in kproc */
	int	IgnoreOwnActivity;      /* to cancel the activity by syncd   */
	int	IgnoreIdleCmdActivity; /* to cancel the activity by idle cmd */
	int	beep_condition;  /* R/W flag to control PM BEEP */
	int	shutdown_on_going; /* shutdown caused by power sw is on-going*/
	int 	RTCresumeVariety;/* indicator that susp2hib by RTC is enabled.*/
	int 	TemporalDisableAcDcBEEP; /* Diable beep once after resume */
	int	LID_ACDC_ignore; /* ignore event at the beginning of resume */
	int	RTC_everyday;	 /* RTC resume time has a "don't care date"*/
	int	GoingFromSuspendToHibernation; /* transit from susp to hiber */
	int	standby_delay;	/* to measure the time after entering standby*/
} pm_control_data_t;


/* pmd type */
#define	NO_PMD		0	/* PMD is not loaded yet. */
#define PHASE1_PMD	1	/* Phase_1 PMD is loaded. */
#define	PHASE2_PMD	2	/* Phase_2 PMD is loaded. */


/* To serialize uniguration */
typedef	struct	_transit_lock {
        Simple_lock     lock;
	volatile int	on_going;	/* Set during state transition */
} transit_lock_t;
	
/* To cofirm the correct battery status */
typedef struct  _battery_status_ready {
	int	event_wait;
	volatile int	available;  /*Set if the first battery query succeeds.*/
} battery_status_ready_t;

/*..................................*/
/* PMMD interface block */
/*..................................*/
typedef struct _pmmd_IF {
	Simple_lock	lock;
	int	(*pmmd_entry)();
} pmmd_IF_t;



/*************************************************************************/
/* PMMD command (PMMI ---> PMMD) 					 */
/*************************************************************************/
#define	set_pmmi_callback	0
#define adjust_system_time	1			
#define	LED_control		2
#define	BEEP_control		3
#define save_basic_devices	4
#define	restore_basic_devices	5
#define	enter_suspend		6 
#define	enter_hibernation	7
#define	set_resume_time		8
#define	get_resume_event	9
#define	set_resume_event	10
#define	battery_job		11
#define	TurnPowerOff		12
#define	reconstruct_systime	13
#define EmergencyScreenOn  	14

#define	ForceTerminate		16
#define	pmmd_dbgout		99

/*..................................*/
/* callback entry block */
/*..................................*/
typedef struct	_callback_entry_block{
	int (*pmmi_event_receive_entry)();
	int (*hibernation_entry)();
} callback_entry_block_t;

/* event of "pmmi_event_receive_entry" called by pmmd */
/*
#define PM_EVENT_LID_CLOSE          2  
#define PM_EVENT_KEYBOARD           5   
#define PM_EVENT_LOW_BATTERY        6   
#define PM_EVENT_AC                 15  
#define PM_EVENT_DC                 16 
#define PM_EVENT_MOUSE              18 
#define PM_EVENT_EXTRA_INPUTDD      19  
#define PM_EVENT_EXTRA_BUTTON       20 
#define PM_EVENT_POWER_SWITCH_OFF   28  
#define	PM_EVENT_BATTERY_STATUS_RDY 29  
 <<<  These events are a part of PM events defined in pm.h. >>>
*/


/*..................................*/
/*  LED control 		    */
/*..................................*/
typedef	struct	_LED_block{
	int	LED_type;
	int	control;
} LED_block_t;

/* LED type */
#define suspend_LED	1
#define	power_on_LED	2

/* LED control */
#define	LED_off		0
#define	LED_on		1
#define	LED_blink	2

/*..................................*/
/* BEEP control */
/*..................................*/
typedef	struct	_BEEP_block{
	int	BEEP_type;
	int	control;
} BEEP_block_t;

/* BEEP type */
#define	suspend_beep		1
#define	partial_suspend_beep	2
#define	reject_beep		3
#define	AC_DC_beep		4
#define	hibernation_beep	5
#define All_types		6

/* BEEP control */
#define	BEEP_off	0
#define	BEEP_on		1
#define	BEEP_oneshot	2
#define BEEP_disable    3
#define BEEP_enable	4

/*..................................*/
/* for save_basic_devices */ 
/*..................................*/
/* caddr_t arg = NULL */

/*..................................*/
/* for restore_basic_devices */ 
/*..................................*/
/* caddr_t arg = NULL */

/*..................................*/
/* for enter_suspend	*/ 
/*..................................*/
/* caddr_t arg = NULL */

/*..................................*/
/* for enter_hibernation  */
/*..................................*/
/* caddr_t arg = NULL */

/* return code in entering suspend or hibernation */
#define	RESUMEwSUCCESS			0
#define MEMORY_CANNOT_OBTAINED          -1   /* error at pre-check 	*/
#define NO_HIBERNATION_VOLUME		-2   /* error at pre-check 	*/
#define INVALID_HIBERNATION_VOLUME 	-3   /* error when entering hib */
#define	NO_EXECUTABLE_WAKEUPCODE 	-4   /* error when entering hib */
#define HIBERNATION_GENERAL_ERROR 	-5   /* error when entering hib */


/*..................................*/
/* for set_resume_time */
/*..................................*/
typedef	struct	_resume_time_data{
	time_t	resume_at_specified_time;
	int	duration_for_susp2hib; 
	int	selected_resume_time;
} resume_time_data_t; 

/* selected_resume_time */
#define	NoRTCResume	0
#define	ResumeByRTC	1
#define	SuspendToHibernationByRTC  2 

/*..................................*/
/* for get_resume_event */	
/*..................................*/
typedef	struct	_rsm_trig_blk {
	int	event;
	int	state;
} rsm_trig_blk_t;

/* resume event variety which can be detected by PMMD */
/*
#define PM_EVENT_NONE               0   
#define PM_EVENT_LID_OPEN           1  
#define PM_EVENT_KEYBOARD           5 
#define PM_EVENT_CRITICAL_LOW_BATTERY 7 
#define PM_EVENT_RING               8  
#define PM_EVENT_RTC                17
#define PM_EVENT_MOUSE              18  
#define PM_EVENT_EXTRA_BUTTON       20
#define PM_EVENT_POWER_SWITCH_ON    27  
*/ 
#define	NormalResume		1	
#define	TurnAround2Suspend	2
#define	AbnormalResume		-1
#define	HibernationEnterFail	-2

/*..................................*/
/* for set resume enabled event	    */
/*..................................*/
typedef	struct 	_rsm_en_event {
	int	MDM_ring_resume_from_suspend;
	int	MDM_ring_resume_from_hibernation;
	int	critical_low_battery_hibernation_from_suspend;
	int	GP_input_pin_for_resume_from_suspend;
	int	GP_input_pin_for_resume_from_hibernation;
	int	internal_keyboard_resume_from_suspend;
	int	external_keyboard_resume_from_hibernation;
	int	internal_mouse_resume_from_suspend;
	int	external_mouse_resume_from_hibernation;
	int	LID_open_resume_from_suspend;
	int	LID_open_resume_from_hibernation;
	int	main_power_switch_resume_from_suspend;
	int	main_power_switch_resume_from_hibernation;
        int	resume_from_suspend_at_specified_time;
        int	resume_from_hibernation_at_specified_time;
        int	hibernation_from_suspend_at_specified_time;
        int	resume_button_support;
	int	reserve[12];
} rsm_en_event_t;


/* resume event enable/disable */
#define	RESUME_ENABLE	1
#define	RESUME_DISABLE	0 

/*..................................*/
/* machine profile */
/*..................................*/
typedef	struct	_pm_machine_profile{
	int	suspend_support;
	int	hibernation_support;
	int	LID_support;
	int	battery_support;
	int	software_controllable_main_power_switch_support;
	int	fail_safe_HW_main_power_off_switch_support;
        int	inhibit_auto_transition;
 rsm_en_event_t resume_events;
	char	mach_name[32];		/* Null terminated string */
} pm_machine_profile_t;

/* support level */ 
#define	SUPPORTED	1
#define	NOT_SUPPORTED	0
#define	UNKNOWN		-1


/*..................................*/
/* battery control */
/*..................................*/
typedef	struct _battery_control{
	int	control;
 	pm_battery_data_t *battery_status;	
} battery_control_t;

/* control */
#define	discharge_battery 1
#define	query_battery	  2

/*..................................*/
/* main power off */
/*..................................*/
/* caddr_t arg = NULL */

/*..................................*/
/*   EmergencyScreenOn		    */
/*..................................*/
typedef struct _ScreenOn_req_blk {
	int	LCD_on;
	int	CRT_on;
} ScreenOn_req_blk_t;


/* 
 * slave CPU's battery measurement is not accurate. So this 120	
 * seconds doesn't mean actual 2 minutes. 
 */
#define shuttime	120	


/*************************************************************************/
/* PMMD comamnd transfer macro						 */
/*************************************************************************/
#define	CALL_PMMD(pmmd_job_id, pjob_info)	\
	simple_lock(&(pmmd_IF.lock));		\
	if (pmmd_IF.pmmd_entry) {		\
	(void)(*(pmmd_IF.pmmd_entry))(pmmd_job_id, pjob_info);	\
	}					\
	simple_unlock(&(pmmd_IF.lock));

#define	CALL_PMMD_W_RC(pmmd_job_id, pjob_info, rc)	\
	simple_lock(&(pmmd_IF.lock));		\
	if (pmmd_IF.pmmd_entry) {		\
	rc = (*(pmmd_IF.pmmd_entry))(pmmd_job_id, pjob_info);	\
	}					\
	simple_unlock(&(pmmd_IF.lock));

#define	CALL_PMMD_WO_LOCK(pmmd_job_id, pjob_info)	\
	if (pmmd_IF.pmmd_entry) {		\
	(void)(*(pmmd_IF.pmmd_entry))(pmmd_job_id, pjob_info);\
	}

#define	CALL_PMMD_WO_LOCK_W_RC(pmmd_job_id, pjob_info, rc)	\
	if (pmmd_IF.pmmd_entry) {		\
	rc = (*(pmmd_IF.pmmd_entry))(pmmd_job_id, pjob_info);\
	}


/*************************************************************************/
/* PCI devices which ROS doesn't disable				 */
/* The structure has data for only PCI devices which ROS doesn't disable.*/
/*************************************************************************/
#define PCIDEV_NUMMAX	10	/* This is enough, because only PCI      */
				/* graphics devices should be included.  */

typedef struct _pcidev {
	int	num;				/* number of devices */
	int	devfunc[PCIDEV_NUMMAX];		/* DevFuncNumber */
	int	bid[PCIDEV_NUMMAX];		/* BID of PCI bus */
} pcidev_t;

#endif   /* _H_PMMI */

