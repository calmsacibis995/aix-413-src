static char sccsid[] = "@(#)83  1.22  src/rspc/kernext/pm/pmmi/core.c, pwrsysx, rspc41J, 9524E_all 6/14/95 11:53:33";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS: pm_kproc,pm_set_event,
 *		event_during_full_on,event_during_enable,
 *		event_during_standby,event_during_suspend,
 *		system_idle_check,check_graphical_input_activity,
 *              check_device_idle,
 *		pm_start_system_full_on,pm_start_system_enable,
 *		pm_start_system_enable,pm_start_system_standby,
 *		pm_start_system_suspend
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *   (C) COPYRIGHT International Business Machines Corp. 1994,1995
 *   All Rights Reserved
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/sleep.h>
#include <sys/syspest.h>
#include <sys/lock_def.h>
#include <procinfo.h>
#include <sys/rtc.h>		/* for real time clock related defines	*/
#include <sys/time.h>		/* for the timeval structure		*/
#include <sys/device.h>

#include <sys/pm.h>
#include "pmmi.h"

/* Handles for PM from PMkernel and PMcore */
extern struct _pm_kernel_data   pm_kernel_data;
extern pm_parameters_t 	pm_parameters;
extern pm_control_data_t pm_control_data;
extern pmmd_IF_t pmmd_IF;
extern pm_battery_data_t battery_status;
extern	transit_lock_t	transit_lock;
extern battery_status_ready_t battery_status_ready;
extern pm_machine_profile_t pm_machine_profile;


/* PM core data */
struct _pm_core_data   pm_core_data
    = {{0}, 0, 0, PM_EVENT_NONE, 0, 0, 0, 0, PM_LID_OPEN, 0, 0, 0};

/* control PM kproc */
int pm_kproc();
int pm_kflag = TRUE;
int pm_term  = FALSE;

/* event_words for PM  */
int pm_queried_event = EVENT_NULL;

/* ids/events for pm_control_state() */
/* This array is only used for phase1 PM */
struct pm_state	pmstate[PM_MAX_REQUESTED_EVENT_NUMBER] =
	{{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
		{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
		{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
		{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
int	pm_event_id = 0;

#define         ID2INDEX(id)    ((id) % (PM_MAX_REQUESTED_EVENT_NUMBER))

#define STATE_CHANGE_AND_WAKEUP(state, event)			\
		pm_core_data.system_next_state = (state);	\
		pm_core_data.system_event = (event);		\
		e_wakeup( &pm_queried_event );

/* input activity */
int	input_activity;
int	input_activity_long;
int	tty_activity;
int	tty_activity_long;

int	still_screen_dim = FALSE;
int 	pm_check_idle_time = 0;
int     GettingOutOfStandby = FALSE;
#ifdef PM_DEBUG
int	current_tick=0;
#endif


/*--------------------------------*/
/*    proto type declaration	  */
/*--------------------------------*/
void check_specified_time_event();			
void check_necessity_of_SYNC();	
void secs_to_date(register struct tms *timestruct);
void check_device_idle();
void set_new_registered_PM_awareDD_to_enable(); 
int  see_DD_with_specified_attrib(int attrib, struct pm_handle **specified_DD);
void check_audio_input_activity();
void check_sync_time(int * pchecked_timer);
void check_screen_dim();
void battery_query(pm_battery_data_t * last_battery_data);
void pm_kproc_main_job();




/*
 * NAME:  pm_kproc
 *
 * FUNCTION:  PM kernel process (PM kproc)
 *
 * EXECUTION ENVIRONMENT:
 *      It can page fault.
 *
 * NOTES:
 *      This routine is PM kproc (PM kernel process).
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	This routine doesn't return. (eternal loop)
 *	Only when pm_kflag != TRUE, it returns with 0.
 */
int
pm_kproc()
{

    setpinit();	/* Change the parent process to init process */
    DBG(pmkproc is running)
    battery_query(&battery_status);	

    while(pm_kflag) {
    /* 
     * If transition is on-going, pm-kproc just sleeps 
     * without anything to do.
     */
    if (transit_lock.on_going != TRUE) {

	if (((pm_control_data.phase_1_only == TRUE) &&  
	     (pm_core_data.system_state == PM_SYSTEM_FULL_ON)) 
			 	||
	    ((pm_control_data.phase_1_only == FALSE) &&  
	     (pm_control_data.system_state == PM_SYSTEM_FULL_ON))) 
	{
		input_activity = input_activity_long = FALSE; 
		tty_activity = tty_activity_long = FALSE;
	} else {
		input_activity = check_graphical_input_activity();
		input_activity_long |= input_activity;
		tty_activity = check_tty_activity();
		tty_activity_long |= tty_activity;
	}


	/*--------------------------------------------*/
	/*  		System FULL_ON state          */
	/*--------------------------------------------*/
	if (((pm_control_data.phase_1_only == TRUE) &&  
	     (pm_core_data.system_state == PM_SYSTEM_FULL_ON)) 
			 	||
	    ((pm_control_data.phase_1_only == FALSE) &&  
	     (pm_control_data.system_state == PM_SYSTEM_FULL_ON))) 
	{
		DBG(PMKPROC: Sleep due to FULL_ON state)
		delay( 10 * PM_KPROC_INTERVAL * HZ );
		battery_query(&battery_status);
		pm_check_idle_time = 0;
        	pm_control_data.standby_delay = 0;
	        GettingOutOfStandby = FALSE;


	/*--------------------------------------------*/
	/*  		System STANDBY state 	      */
	/*--------------------------------------------*/
	} else if (((pm_control_data.phase_1_only == TRUE) &&  
	            (pm_core_data.system_state == PM_SYSTEM_STANDBY)) 
			 	||
	    	   ((pm_control_data.phase_1_only == FALSE) &&  
	            (pm_control_data.system_state == PM_SYSTEM_STANDBY))) 
	{
	    ++(pm_control_data.standby_delay);

            if (pm_control_data.standby_delay < 3) {
		input_activity = tty_activity = FALSE;
		input_activity_long = tty_activity_long = FALSE;
            } /* endif */

	    if (pm_control_data.standby_delay == 5) {
		simple_lock(&(transit_lock.lock));
	       	if ((pm_control_data.system_state == PM_SYSTEM_STANDBY)
				&&
		    (transit_lock.on_going == FALSE)) {

			setSystemStandby();	

		} /* endif */
		simple_unlock(&(transit_lock.lock));

	    } /* endif */

	    if (GettingOutOfStandby == FALSE) {
	    	if( input_activity == TRUE ) {
		/* Only inform PMD(PM Daemon) of this activity. And then, 
		 * PMD will request to change system state from standby   
		 * to PM enable.
		 */
			tty_activity = FALSE;
			still_screen_dim = FALSE;
			GettingOutOfStandby = TRUE;
			receive_ext_event(PM_EVENT_KEYBOARD);
	    	} /* endif */

		if( tty_activity == TRUE ) {
		/* tty activity also breaks system standby. However, 
		 * pm_graphics_output device won't receive pm_enable
		 * request so that screen dim of standby is desired to
		 * scceed even though system state will have changed to 
		 * system pm enable state. 
		 */
			still_screen_dim = TRUE;
			GettingOutOfStandby = TRUE;
			receive_ext_event(PM_EVENT_KEYBOARD);
		} /* endif */
	    } /* endif */

	    pm_kproc_main_job();


	/*--------------------------------------------*/
	/*  	System ENABLE state (phase-1 only)    */
	/*--------------------------------------------*/
	} else if ((pm_control_data.phase_1_only == TRUE) &&  
	           (pm_core_data.system_state == PM_SYSTEM_ENABLE)) 
	{ 
	   /* 
	    *  The following is a bug fix code for phase_1 graphics DD 
	    */
	   if( input_activity == TRUE ) {
		struct pm_handle *p;
		p = pm_kernel_data.graphical_output_handle_head;

		while( p != NULL ) {
	    	    if( p->mode == PM_DEVICE_IDLE ) {
			 if( p->handler != NULL ) {
		    	     (p->handler)(p->private, PM_DEVICE_ENABLE);
		    	     p->idle_counter = 0;
			 }
	    	    }
	    	    p = p->next2;
		} /* while */
	     } /* endif */ 

	    /* 
	     * If input_activity is TRUE, screen dim is terminated. If no 
	     * input_activity for a specified period, screen is made dim. 
	     */
	     check_screen_dim();

	     pm_kproc_main_job();

	/*--------------------------------------------*/
	/*  	System ENABLE state (phase-2 only)    */ 
	/*--------------------------------------------*/
	} else if ((pm_control_data.phase_1_only == FALSE) &&  
	            (pm_control_data.system_state == PM_SYSTEM_ENABLE))
	{
             pm_control_data.standby_delay = 0;
	     GettingOutOfStandby = FALSE;
 
	    /* 
	     * If input_activity is TRUE, screen dim is terminated. If no 
	     * input_activity for a specified period, screen is made dim. 
	     */
	     check_screen_dim();

	     pm_kproc_main_job();

	} /* endif */


      } else { /* when on-going */
	input_activity = input_activity_long = 0; 
	tty_activity = tty_activity_long = 0;
        pm_control_data.standby_delay = 0;
	delay( PM_KPROC_INTERVAL * HZ );
      } /* endif */


    } /* endwhile */


    /* 
     * Setting this flag results in being able to inform unconfig routine 
     * that pm kproc has just got out of the loop.
     */
    pm_term = TRUE;
    return 0;

} /* pm_kproc */



/*
 * NAME: pm_kproc_main_job 
 *
 * FUNCTION: Calling various routines which are run once per 1,5,or10 second(s).
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * RETURN VALUE DESCRIPTION:	
 *	none
 */
void 
pm_kproc_main_job()
{
	/*--------------------------------------------*/
	/* 	Jobs in 1 second		      */ 
	/*--------------------------------------------*/
	/* Check if it is the time specified by PM daemon for PM event.
	 */
	check_specified_time_event();

	/*
	 * If audio input device is active and audio output device is in 
	 * idle mode, audio input device is set to PM_DEVICE_ENABLE mode.
   	 */
	check_audio_input_activity();



	/*--------------------------------------------*/
	/* 	Jobs in 5 second		      */ 
	/*--------------------------------------------*/
	/* 
	 * Entering here once a 5 seconds 
 	 */  
	if( pm_check_idle_time % PM_SYSTEM_IDLE_INTERVAL == 0 ) {

	    /* Seeing whether or not a new PM aware DD has just been 
	     * registered. If exists, it's set to enable mode.
	     */
	    set_new_registered_PM_awareDD_to_enable(); 


	    /* Seeing system idleness (actually seeing graphical input DD) 
	     */ 
	    system_idle_check();

	    /* 
	     * Seeing devices idleness 
	     */
	    check_device_idle();

	    /* 
	     * Check once 10 seconds
	     */
	    if (pm_check_idle_time % (PM_SYSTEM_IDLE_INTERVAL*2) == 0) {
		/* 
		 * input = global data 
		 */
	    	battery_query(&battery_status);	
		pm_check_idle_time = 0;

	    } /* endif */	

#ifdef PM_DEBUG
/*	printf("%d.",current_tick+=5); */
#endif
	} /* endif */


	/*--------------------------------------------*/
	/* 	Sleep for 1 second  		      */
	/*--------------------------------------------*/
	/* sleep here for 1 second
 	 */
	delay( PM_KPROC_INTERVAL * HZ );
	pm_check_idle_time += PM_KPROC_INTERVAL;

	return;

} /* pm_kproc_main_job */




/*
 * NAME: check_specified_time_event 
 *
 * FUNCTION:  Check whether the current time is the specified time. 
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 *
 * DATA STRUCTURES:
 *      pm_parameters.core_data - Data structure in PM core
 *	global "time" in memory is utilied.
 *
 * RETURN VALUE DESCRIPTION:	
 *	none
 */
void
check_specified_time_event()
{
     struct   timestruc_t ct;
     struct   tms mytm1;                 /* date to secs conversion structure */
     time_t   mytime;
     uchar    cvt_hrs, cvt_mins, cvt_no_secs;
     

     if (pm_parameters.core_data.specified_time != NULL) {

	/* 
	 * We need to consider that pm kproc can't always run once a second.
	 * Here is a consideration that pm kproc can detect the specified
 	 * time if it's ensured that pm kproc can run at least once 5 seconds.
	 */

	if (pm_parameters.core_data.specified_time <= 3600*24) { 
		/* every day request ? */

		curtime(&ct);
		mytm1.secs = ct.tv_sec;
		mytm1.ms = ct.tv_nsec / (NS_PER_SEC/100); 
				/* nano secs to 100th secs */
		secs_to_date(&mytm1);

		/* BCD to decimal */
		cvt_hrs = (mytm1.hrs/16 ) * 10 + (mytm1.hrs % 16);
		cvt_mins = (mytm1.mins/16 ) * 10 + (mytm1.mins % 16);
		cvt_no_secs = (mytm1.no_secs/16 ) * 10 + (mytm1.no_secs % 16);

		mytime = (3600*cvt_hrs + 60*cvt_mins + cvt_no_secs);

	} else {

		mytime = time;

	} /* endif */

	if ((pm_parameters.core_data.specified_time >= mytime) && 
	    (pm_parameters.core_data.specified_time <= mytime+5)) {

		if ((pm_control_data.last_active_specified_time >= mytime) && 
		    (pm_control_data.last_active_specified_time <= mytime+5)){
			return;
		} /* endif */
		pm_control_data.last_active_specified_time = mytime;

		DBG(Calling "receive_ext_event" due to specified time action)
		receive_ext_event(PM_EVENT_SPECIFIED_TIME);

	} /* endif */

     } /* endif */
     return;	

} /* check_specified_time_event */




/*
 * NAME: check_necessity_of_SYNC 
 *
 * FUNCTION:  Check whether SYNC() should be issued. 
 *
 * INPUT: struct pm_handle *p
 *
 * RETURN VALUE DESCRIPTION:	
 *	none
 */
void									
check_necessity_of_SYNC(struct pm_handle *p)
{
   uint 	status; 
   
   if ((pm_control_data.system_state == PM_SYSTEM_STANDBY)  &&
	    (pm_parameters.core_data.kill_syncd == TRUE)) {

        if(p->devno) {

	   if (devswqry(p->devno, &status, NULL)==0) {

	        if (status & DSW_BLOCK) {
#ifdef PM_DEBUG
     printf("%s is checked as a block device driver\n",p->device_logical_name);
#endif
	 	    if ((pm_control_data.IgnoreOwnActivity == FALSE) &&
			(pm_control_data.IgnoreIdleCmdActivity==0)   ){

			 if ((p->activity == 1) || (p->activity == -1)) {
#ifdef PM_DEBUG
     printf("%s's activity is detected\n", p->device_logical_name); 
#endif
			     pm_control_data.alternate_syncd_stimer 
					    = (60/PM_SYSTEM_IDLE_INTERVAL);
			     if (!(pm_control_data.alternate_syncd_ltimer)){
					pm_control_data.alternate_syncd_ltimer 
					    = (60/PM_SYSTEM_IDLE_INTERVAL)*5;
		  	     } /* endif */	
		  	 } /* endif */	
		     } /* endif */	
	         } /* endif */	
	    } /* endif */
        } /* endif */
   } /* endif */ 
   return;

} /* check_necessity_of_SYNC */



/*
 * NAME:  check_sync_time
 *
 * FUNCTION: check the time related to sync control. 
 *
 * NOTE: This routine expects to be called once a PM_SYSTEM_IDLE_INTERVAL 
 *
 * RETURN VALUE DESCRIPTION:	
 *	     none
 */
void
check_sync_time(int * pchecked_timer)
{
	switch (*pchecked_timer) {
	case 0:
		/* do nothing */
		break;

	case 1:
		/* issue sync() instead of sync daemon during standby mode */
		sync();	
		pm_control_data.alternate_syncd_stimer = 0;
	 	pm_control_data.alternate_syncd_ltimer = 0; 

		/* 
		 * set this flag to avoid to misunderstand the activity 
		 * caused by the sync() performed above. 
		 */
	 	pm_control_data.IgnoreOwnActivity = TRUE; 
		DBG(sync() is issued due to block device activity)
		break; 

	default:
#ifdef PM_DEBUG
		printf ("sync timer = %X\n", (*pchecked_timer));
#endif
		--(*pchecked_timer); 
		break; 

	} /* endswitch */
	return;
}


/*
 * NAME:  manage_sync_timer
 *
 * FUNCTION: check the time related to sync control. 
 *
 * NOTE: This routine expects to be called once a PM_SYSTEM_IDLE_INTERVAL 
 *
 * RETURN VALUE DESCRIPTION:	
 *	     none
 */
void
manage_sync_timer()
{
   if ((pm_control_data.system_state == PM_SYSTEM_STANDBY)  &&
	    (pm_parameters.core_data.kill_syncd == TRUE)) {

	pm_control_data.IgnoreOwnActivity = FALSE;
	if (pm_control_data.IgnoreIdleCmdActivity) { 
		(pm_control_data.IgnoreIdleCmdActivity)--; 
	}

	/* If 1 minute has passed without any following access 
	 * after the last access to one of the block device drivers,
 	 * sync() is issued.
	 */
	check_sync_time(&(pm_control_data.alternate_syncd_stimer));

	/* If the access to one of the block device drivers is 
	 * continuously performed for more than 5 minutes,
	 * sync() is issued.
	 */ 
	check_sync_time(&(pm_control_data.alternate_syncd_ltimer));

   } else { /* if (!(PM_SYSTEM_STANDBY && kill_syncd == TRUE)) */

	pm_control_data.alternate_syncd_stimer = 0;
	pm_control_data.alternate_syncd_ltimer = 0; 
	pm_control_data.IgnoreOwnActivity = FALSE;
	pm_control_data.IgnoreIdleCmdActivity = 0;

   } /* endif */ 
   return;
}


		
/*
 * NAME:  set_new_registered_PM_awareDD_to_enable
 *
 * FUNCTION: Check whether or not a new registered PM aware DD
 *	     exists and set it to PM enable mode if exists. 
 *
 * NOTES:    Registering itself as a PM aware DD occurs
 *           asynchronously with PM kernel externsion. So it's 
 *	     possible that a new registered PM aware DD has 
 *	     unknowingly run with full_on mode though the system
 *	     state is in PM enable state.
 *
 * RETURN VALUE DESCRIPTION:	
 *	     none
 */
void
set_new_registered_PM_awareDD_to_enable() 
{
   struct pm_handle *p;
   
   if (pm_control_data.system_state == PM_SYSTEM_ENABLE) {

   	for(p = pm_kernel_data.handle_head; p!=NULL; p=p->next1) {

		if (p->mode == PM_DEVICE_FULL_ON) {
			p->idle_counter = 0;
			(p->handler)(p->private, PM_DEVICE_ENABLE);
#ifdef PM_DEBUG
			if (p->pm_version == PM_PHASE1_SUPPORT_ONLY) {
	printf ("PM_DEVICE_ENABLE is sent to (devno: %08X)\n", (p->devno));
			} else {
	printf ("PM_DEVICE_ENABLE is sent to %s\n",(p->device_logical_name));
			}
#endif
		} /* endif */

   	} /* endfor */

   } /* endif */

   return;

} /* set_new_registered_PM_awareDD_to_enable */



/*
 * NAME:  pm_set_event
 *
 * FUNCTION:  sets event type and wakes up a process waiting for a event.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	This routine should be called after getting a lock.
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	PM_SUCCESS : no error.
 */
int
pm_set_event(int event)
{
    switch(pm_core_data.system_state) {
      case PM_SYSTEM_FULL_ON:
	return(event_during_full_on(event));

      case PM_SYSTEM_ENABLE:
	return(event_during_enable(event));

      case PM_SYSTEM_STANDBY:
	return(event_during_standby(event));

      case PM_SYSTEM_SUSPEND:
	return(event_during_suspend(event));
    }

    /* If the PM system state is wrong, go back to PM_SYSTEM_ENABLE */
    STATE_CHANGE_AND_WAKEUP(PM_SYSTEM_ENABLE, event);
    return PM_SUCCESS;
}


/*
 * NAME:  event_during_full_on
 *
 * FUNCTION:  processes an event in system full on state.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	This routine should be called after getting a lock.
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	PM_SUCCESS : no error.
 */
static int
event_during_full_on(int event)
{
    switch( event ) {
      case PM_EVENT_MAIN_POWER:
	STATE_CHANGE_AND_WAKEUP(PM_SYSTEM_SHUTDOWN, event);
	break;

      case PM_EVENT_SOFTWARE_REQUEST:
	pm_core_data.system_event = event;
	e_wakeup( &pm_queried_event );
	break;

      case PM_EVENT_LID_CLOSE:
	pm_core_data.lid_state = PM_LID_CLOSE;
	break;

      case PM_EVENT_LID_OPEN:
	pm_core_data.lid_state = PM_LID_OPEN;
	break;

      case PM_EVENT_TERMINATE:
	STATE_CHANGE_AND_WAKEUP(PM_TERMINATE, event);
	break;

      default:
	pm_core_data.system_event = PM_EVENT_NONE;
	break;
    }

    return PM_SUCCESS;
}



/*
 * NAME:  event_during_enable
 *
 * FUNCTION:  processes an event in system PM enable state.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	This routine should be called after getting a lock.
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	PM_SUCCESS : no error.
 */
static int
event_during_enable(int event)
{
    switch( event ) {
      case PM_EVENT_LID_CLOSE:
	pm_core_data.lid_state = PM_LID_CLOSE;
	STATE_CHANGE_AND_WAKEUP(pm_core_data.lid_close_action, event);
	break;

      case PM_EVENT_LID_OPEN:
	pm_core_data.lid_state = PM_LID_OPEN;
	break;

      case PM_EVENT_MAIN_POWER:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.main_switch_action, event);
	break;

      case PM_EVENT_LOW_BATTERY:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.low_battery_action, event);
	break;

      case PM_EVENT_SYSTEM_IDLE_TIMER:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.system_idle_action, event);
	break;

      case PM_EVENT_SOFTWARE_REQUEST:
	pm_core_data.system_event = event;
	e_wakeup( &pm_queried_event );
	break;

      case PM_EVENT_TERMINATE:
	STATE_CHANGE_AND_WAKEUP(PM_TERMINATE, event);
	break;

      default:
	pm_core_data.system_event = PM_EVENT_NONE;
	break;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  event_during_standby
 *
 * FUNCTION:  processes an event in system standby state.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	This routine should be called after getting a lock.
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	PM_SUCCESS : no error.
 */
static int
event_during_standby(int event)
{
    struct pm_state pms;

    switch( event ) {
      case PM_EVENT_LID_OPEN:
	pm_core_data.lid_state = PM_LID_OPEN;
	pms.event = event;
	pm_start_system_enable(&pms);
	pm_core_data.system_event = pms.event;
	break;

      case PM_EVENT_LID_CLOSE:
	pm_core_data.lid_state = PM_LID_CLOSE;
	STATE_CHANGE_AND_WAKEUP(pm_core_data.lid_close_action, event);
	break;

      case PM_EVENT_MAIN_POWER:
      case PM_EVENT_GRAPHICAL_INPUT:
      case PM_EVENT_PD_CLICK:
      case PM_EVENT_KEYBOARD:
	pms.event = event;
	pm_start_system_enable(&pms);
	pm_core_data.system_event = pms.event;
	break;

      case PM_EVENT_LOW_BATTERY:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.low_battery_action, event);
	break;

      case PM_EVENT_SYSTEM_IDLE_TIMER:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.system_idle_action, event);
	break;

      case PM_EVENT_SOFTWARE_REQUEST:
	pm_core_data.system_event = event;
	e_wakeup( &pm_queried_event );
	break;

      case PM_EVENT_TERMINATE:
	STATE_CHANGE_AND_WAKEUP(PM_TERMINATE, event);
	break;

      default:
	pm_core_data.system_event = PM_EVENT_NONE;
	break;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  event_during_suspend
 *
 * FUNCTION:  processes an event in system suspend state.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	This routine should be called after getting a lock.
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	PM_SUCCESS : no error.
 */
static int
event_during_suspend(int event)
{
    struct pm_state pms;

    switch( event ) {
      case PM_EVENT_LID_OPEN:
	pm_core_data.lid_state = PM_LID_OPEN;
	pms.event = event;
	pm_start_system_enable(&pms);
	pm_core_data.system_event = pms.event;
	break;

      case PM_EVENT_LID_CLOSE:
	pm_core_data.lid_state = PM_LID_CLOSE;
	break;

      case PM_EVENT_MAIN_POWER:
      case PM_EVENT_KEYBOARD:
	pms.event = event;
	pm_start_system_enable(&pms);
	pm_core_data.system_event = pms.event;
	break;

      case PM_EVENT_LOW_BATTERY:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.low_battery_action, event);
	break;

      case PM_EVENT_SYSTEM_IDLE_TIMER:
	STATE_CHANGE_AND_WAKEUP(pm_core_data.system_idle_action, event);
	break;

      case PM_EVENT_PD_CLICK:
	pms.event = event;
	pm_start_system_enable(&pms);
	pm_core_data.system_event = pms.event;
	break;

      case PM_EVENT_SOFTWARE_REQUEST:
	pm_core_data.system_event = event;
	e_wakeup( &pm_queried_event );
	break;

      case PM_EVENT_TERMINATE:
	STATE_CHANGE_AND_WAKEUP(PM_TERMINATE, event);
	break;

      default:
	pm_core_data.system_event = PM_EVENT_NONE;
	break;
    }

    return PM_SUCCESS;
}


/*
 * NAME:  system_idle_check
 *
 * FUNCTION:  checks system idle.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *	PM_SUCCESS : no error.
 */
static int
system_idle_check()
{
    if (pm_control_data.phase_1_only) {
    	if (input_activity_long != TRUE) {
		if( pm_core_data.system_idle_time != 0 )  {
	    		pm_core_data.system_idle_count 
						+= PM_SYSTEM_IDLE_INTERVAL;
		} else {
	    		pm_core_data.system_idle_count = 0;
		}
		input_activity_long = FALSE;
        } else {
		pm_core_data.system_idle_count = 0;
        }

        if( (pm_core_data.system_idle_count >= pm_core_data.system_idle_time) &&
	    (pm_core_data.system_idle_time != 0) &&
	    (pm_core_data.system_state == PM_SYSTEM_ENABLE) ) {

		simple_lock(&(pm_core_data.lock));
		STATE_CHANGE_AND_WAKEUP(pm_core_data.system_idle_action,
					PM_EVENT_SYSTEM_IDLE_TIMER);
		pm_core_data.system_idle_count = 0;
		simple_unlock(&(pm_core_data.lock));
	} /* endif */	

    } else {
    	if( (input_activity_long != TRUE) && (tty_activity_long != TRUE) ) {
		if (pm_parameters.core_data.system_idle_time != 0) {
			pm_control_data.system_idle_count 
						+= PM_SYSTEM_IDLE_INTERVAL;
		} /* endif */ 
	} else {
		pm_control_data.system_idle_count = 0;
		input_activity_long = FALSE;
		tty_activity_long = FALSE;
#ifdef PM_DEBUG
		current_tick = 0;
#endif
	} /* endif */	

        if( (pm_control_data.system_idle_count >= 
				pm_parameters.core_data.system_idle_time) &&
	    (pm_parameters.core_data.system_idle_time != 0) &&
	    ((pm_control_data.system_state == PM_SYSTEM_ENABLE) ||
	     (pm_control_data.system_state == PM_SYSTEM_STANDBY))  ){

		DBG(Call "receive_ext_event")
		receive_ext_event(PM_EVENT_SYSTEM_IDLE_TIMER);
#ifdef PM_DEBUG
		current_tick = 0;
#endif
		pm_control_data.system_idle_count = 0;
	} /* endif */	

    } /* endif */

    return PM_SUCCESS;

} /* system_idle_check */




/*
 * NAME:  check_graphical_input_activity
 *
 * FUNCTION:  checks graphical input devices activity.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      TRUE  : There's an activity in a graphical input device.
 *      FALSE : There's no activity in all of graphical input devices.
 */
static int
check_graphical_input_activity()
{
   /* This is a dummy for calling the function. */
    struct pm_handle * desired_DD;  
    
    return (see_DD_with_specified_attrib(PM_GRAPHICAL_INPUT, &desired_DD));

} /* check_graphical_input_activity */



/*
 * NAME: check_tty_activity 
 *
 * FUNCTION:  checks tty activity to manage the system idle timer. Whenever
 *	      tty activity is seen, system idle timer never expires though
 *	      user enable the system idle timer. 
 *
 * NOTES:     Seeing the activity flag of PM aware DD whose attribute field
 *	      indicates PM_REMOTE_TERMINAL
 *	
 * DATA STRUCTURES:
 *
 * RETURN VALUE DESCRIPTION:	
 *      TRUE  : There's an activity at least in one of TTY devices 
 *      FALSE : There's no activity in all of TTY devices 
 */
int
check_tty_activity()
{
   /* This is a dummy for calling the function. */
    struct pm_handle * desired_DD;  
    
    return (see_DD_with_specified_attrib(PM_REMOTE_TERMINAL, &desired_DD));

} /* check_tty_activity */



/*
 * NAME:  check_audio_input_activity
 *
 * FUNCTION:  checks audio input activity. And if it's active, but audio 
 *	      output device is in PM_DEVICE_IDLE mode, audio output device is 
 *	      awakened to PM_DEVICE_ENABLE mode.
 *
 * NOTES:     Here, audio input device means the device which causes audio 
 *	      chip/sub-system to have further activity. For example, CD-ROM 
 *	      drive audio portion is audio input device if its audio output 
 *	      is expected to be pass-through audio chip/sub-system which may 
 *	      be set into low power mode asynchronously with CD-ROM drive 
 *	      activity.
 *
 * RETURN VALUE DESCRIPTION:	
 *	      none because the required task is completely done internally.
 */
void
check_audio_input_activity()
{
    struct pm_handle * desired_DD = NULL;

    /* Check if PM aware DD with attribute,PM_AUDIO_INPUT has just
     * had an activity. (The following 2nd argument is a dummy.)
     */
    if (see_DD_with_specified_attrib(PM_AUDIO_INPUT, &desired_DD)) {
	

	/* Get the pointer of pm_handle of PM aware DD 
	 * with PM_AUDIO_OUTPUT attrib 
	 */
    	(void)see_DD_with_specified_attrib(PM_AUDIO_OUTPUT, &desired_DD);
#ifdef PM_DEBUG
	printf("PM_AUDIO_OUTPUT aware DD's pm handle: %08X\n", desired_DD);
#endif
	if (desired_DD) {

		/* If the audio output device is in idle mode, it must
		 * be awakened due to active audio input device.
	 	 */
		if ((desired_DD->mode) == PM_DEVICE_IDLE) {

		   (desired_DD->handler)(desired_DD->private, PM_DEVICE_ENABLE);

		  DBG(Wakeup audio output device due to audio input DD activity)

    		} /* endif */
    	} /* endif */

    } /* endif */
	
    return;

} /* check_audio_input_activity */



/*
 * NAME:  see_DD_with_specified_atrib
 *
 * FUNCTION:  checks the activity the PM aware DD whose attribute is specified. 
 *
 * INPUT:     attrib    = desired device attribute in pm_handle 
 *	      **specified_DD = ponter to pm_handle whose attribute is specified
 *
 * RETURN VALUE DESCRIPTION:	
 *      TRUE  : There's an activity of PM aware DD with the specified attribute 
 *      FALSE : There's no activity of PM aware DD with the specified attribute 
 */
int
see_DD_with_specified_attrib(int attrib, struct pm_handle **specified_DD)
{ 
    struct pm_handle *p = pm_kernel_data.handle_head;
    int	rc = FALSE;
#ifdef PM_DEBUG
    char tty_activity_name[]="sa0";
#endif

    if( p == NULL ) {	/* There is no PM aware DD registered. */ 

	return TRUE; 	/* For failsafe, "TRUE" must be returned here. */

    } /* endif */

    while( p != NULL ) {
	if( p->attribute & attrib) {

	    if (rc != TRUE) {
	    	*specified_DD = p;
	    }

	    if( p->activity != 0 ) {
#ifdef PM_DEBUG
	    /* to support kernel debugger, tty activity msg is omitted.*/
            if (strcmp(p->device_logical_name,tty_activity_name) != 0) {
		printf("%s indicates activity\n", (p->device_logical_name));
            } /* endif */
#endif
		if ((attrib == PM_GRAPHICAL_INPUT) && (p->activity == 1)) {
			p->activity = 0;
		} /* endif */
		rc = TRUE;
	    }
	}
	p = p->next1;
    } /* endwhile */

    return rc;

} /* see_DD_with_specified_attrib */



/*
 * NAME:  check_input_activity_idle_time
 *
 * FUNCTION: check if the idle time for graphical input activity expires. 
 *	     If expired, idle command(DPMS_standby/DPMS_suspend/DPMS_off or   
 *	     device idle is sent to graphical output device.
 *
 * INPUT:    struct pm_handle *p points to the pm_handle structure of 
 *	     graphical input device.
 *
 * RETURN VALUE DESCRIPTION:	
 *	none
 */
void
check_input_activity_idle_time(struct pm_handle *p)
{
    int	mytime;
    int DPMS_mode;

    switch (p->mode) {
    case PM_DEVICE_ENABLE: 
    	if (pm_control_data.phase_1_only == FALSE) {
		if (mytime = p->device_idle_time1) {
			DPMS_mode = PM_DEVICE_DPMS_STANDBY;
		} else if (mytime = p->device_idle_time2) {
			DPMS_mode = PM_DEVICE_DPMS_SUSPEND; 
		} else if (mytime = p->device_idle_time) {
			DPMS_mode = PM_DEVICE_DPMS_OFF;
		}
	} else {
		mytime = p->device_idle_time;
	     	DPMS_mode = PM_DEVICE_IDLE;  
	} /* endif */
	break;
	
    case PM_DEVICE_DPMS_STANDBY:
	if (mytime = p->device_idle_time2) {
		DPMS_mode = PM_DEVICE_DPMS_SUSPEND; 
	} else if (mytime = p->device_idle_time) {
		DPMS_mode = PM_DEVICE_DPMS_OFF;
	}
	break;

    case PM_DEVICE_DPMS_SUSPEND:
	mytime = p->device_idle_time;
	DPMS_mode = PM_DEVICE_DPMS_OFF;
	break;

    default:
	mytime = 0;	/* for fail safe */

    } /* endif */

    if( (mytime > 0) && (p->idle_counter >= mytime) && (p->handler != NULL) ){

	(void)(p->handler)(p->private, DPMS_mode); 

#ifdef PM_DEBUG
	switch (DPMS_mode) {
	case PM_DEVICE_DPMS_STANDBY:
		DBG(Enter DPMS mode = PM_DEVICE_DPMS_STANDBY) 
		break;
	case PM_DEVICE_DPMS_SUSPEND: 
		DBG(Enter DPMS mode = PM_DEVICE_DPMS_SUSPEND) 
		break;
	case PM_DEVICE_DPMS_OFF:
		DBG(Enter DPMS mode = PM_DEVICE_IDLE) 
		break;
	} /* endswitch */
#endif		
   } /* endif for no activity for the specified time */

   return;

} /* check_input_activity_idle_time */



/*
 * NAME:  check_necessity_of_device_idle
 *
 * FUNCTION:   
 * 	      in idle for a while, screen is made dim. And if either keyboard
 *	      or mouse indicates its activity, screen is turned on. 
 *
 * DATA STRUCTURES:
 *      pm_kernel_data - PM data structure in kernel
 *      pm_handle - PM handle structure
 *
 * RETURN VALUE DESCRIPTION:	
 *	none
 */
int
check_necessity_of_device_idle(struct pm_handle * p)
{
    if (pm_control_data.phase_1_only == TRUE) {
    	if(!((pm_core_data.system_state == PM_SYSTEM_ENABLE) &&
					(p->device_idle_time == 0))) {
    	   if(!((pm_core_data.system_state == PM_SYSTEM_STANDBY) &&
		  			(p->device_standby_time == 0))){
    		if(!((pm_core_data.system_state == PM_SYSTEM_SUSPEND) &&
		  			(p->device_standby_time == 0))){
			return TRUE;
		}
	   }
	}

    } else {
	if(p->attribute & PM_GRAPHICAL_OUTPUT) {
		if(p->device_idle_time1 +
		   p->device_idle_time2 +
		   p->device_idle_time) {
			return TRUE;	
		}
	} else {
    		if(!((pm_control_data.system_state == PM_SYSTEM_ENABLE) &&
					(p->device_idle_time == 0))) {
    	   		if(!((pm_control_data.system_state 
						== PM_SYSTEM_STANDBY) &&
		  			(p->device_standby_time == 0))){
				return TRUE;
	   		} 
    		}
    	}
    } 
    return FALSE;

} /* check_necessity_of_device_idle */



/*
 * NAME:  check_device_idle_time
 *
 * FUNCTION: check if the idle time of the device other than graphical input 
 * 	     device expires. If expired, idle command(PM_DEVICE_IDLE) is sent 
 *	     to the device.
 *
 * INPUT:    struct pm_handle *p points to the pm_handle structure of 
 *	     the device. 
 *
 * RETURN VALUE DESCRIPTION:	
 *	none
 */
void
check_device_idle_time(struct pm_handle * p)
{
    int	mytime;
    int DPMS_mode;
    if( p->mode == PM_DEVICE_ENABLE ) {

  	if( pm_control_data.phase_1_only == TRUE) {
    		if( pm_core_data.system_state == PM_SYSTEM_STANDBY ) {
			mytime = p->device_standby_time;
    		} else if (pm_core_data.system_state == PM_SYSTEM_SUSPEND ) {
			mytime = p->device_standby_time;
	    	} else {
			mytime = p->device_idle_time;
	    	}
	} /* endif for phase_1 */

 	else 
	{
	    	if( pm_control_data.system_state == PM_SYSTEM_STANDBY ) {
			mytime = p->device_standby_time;
	    	} else {
			mytime = p->device_idle_time;
	    	}
	} /* endif for phase_2 */


	if( (mytime > 0) && 
	    (p->idle_counter >= mytime) && 
	    (p->handler != NULL) ) {

		p->idle_counter = 0;
		/*
		 * As for exit from PM_DEVICE_IDLE mode, each PM 
		 * aware DD must wake up on its own in response to
		 * the corresponding device request to it.
		 */
		pm_control_data.IgnoreIdleCmdActivity = 2;
		(void)(p->handler)(p->private, PM_DEVICE_IDLE); 

#ifdef PM_DEBUG
		if (p->pm_version == PM_PHASE1_SUPPORT_ONLY) {
		printf("Send PM_DEVICE_IDLE to (devno: %08X)\n",(p->devno));
		} else {
		printf("Send PM_DEVICE_IDLE to (name: %s)\n",
						(p->device_logical_name));	
		} /* endif */
#endif
	} /* endif */

    } /* endif of pm->mode == PM_DEVICE_ENABLE */

    return;

} /* check_device_idle_time */



/*
 * NAME:  check_screen_dim
 *
 * FUNCTION:  checks if graphical input device is in idle for more than
 *	      the specified period. If detected as idle, graphical output
 *	      device is sent PM_DPMS_STANDBY/PM_DPMS_SUSPEND/PM_DPMS_OFF.
 * NOTES:
 *
 * DATA STRUCTURES:
 *      pm_kernel_data - PM data structure in kernel
 *      pm_handle - PM handle structure
 *
 * RETURN VALUE DESCRIPTION:	
 *  	none
 */
void
check_screen_dim()
{
    struct pm_handle *p = pm_kernel_data.handle_head;

    if((transit_lock.on_going != TRUE) 
		&&
      (pm_control_data.system_state == PM_SYSTEM_ENABLE)) {

      simple_lock(&(transit_lock.lock));
      while( p != NULL ) {
	if(p->attribute & PM_GRAPHICAL_OUTPUT) {

	  if (input_activity != TRUE) {

		if (check_necessity_of_device_idle(p)) {
    			p->idle_counter += PM_KPROC_INTERVAL;
    			check_input_activity_idle_time(p);

		} /* endif */

	  } /* endif for the case of no activity */
	  else 
	  {
		p->idle_counter = 0;

		if( (p->mode != PM_DEVICE_ENABLE) && (p->handler != NULL) ) {
 
#ifdef PM_DEBUG			
		     switch (p->mode) {
	 	     case PM_DEVICE_DPMS_STANDBY:
			   DBG(Current DPMS mode (PM_DEVICE_DPMS_STANDBY)) 
			   break;
		     case PM_DEVICE_DPMS_SUSPEND: 
			   DBG(Current DPMS mode (PM_DEVICE_DPMS_SUSPEND)) 
			   break;
		     case PM_DEVICE_DPMS_OFF:
			   DBG(Current DPMS mode (PM_DEVICE_DPMS_OFF)) 
			   break;
		     } /* endswitch */

		     if (pm_control_data.phase_1_only == TRUE) {
		DBG(Exit from display low power mode due to G-input activity)
		     } else {
		DBG(Exit from DPMS low power mode due to G-input activity)
		     }
		DBG(Current DPMS mode = PM_DEVICE_ENABLE) 
#endif
		     (void)(p->handler)(p->private, PM_DEVICE_ENABLE); 
			

		}/* endif */

	    } /* endif for the case of the occurrence of activity */
	    

	} /* endif for PM_GRAPHICAL_OUTPUT */

	p = p->next1;

      } /* while */
      simple_unlock(&(transit_lock.lock));
    } /* endif */

    return;

} /* check_screen_dim */


/*
 * NAME:  check_device_idle
 *
 * FUNCTION:  checks device idle. If a device is idle, PM_DEVICE_IDLE is
 *	      sent to the PM aware DD.
 *
 * NOTE: This routine expects to be called once a PM_SYSTEM_IDLE_INTERVAL 
 *
 * DATA STRUCTURES:
 *      pm_kernel_data - PM data structure in kernel
 *      pm_handle - PM handle structure
 *
 * RETURN VALUE DESCRIPTION:	
 *  	none
 */
void
check_device_idle()
{
    struct pm_handle *p = pm_kernel_data.handle_head;

    while( p != NULL ) {
	/* For device other than graphical input/output device */
	if (!((p->attribute & PM_GRAPHICAL_OUTPUT) ||
	      (p->attribute & PM_GRAPHICAL_INPUT )) ) {

	   /* Seeing block DD's activity for issuing sync() instead of syncd
	    */ 
	   check_necessity_of_SYNC(p);	

           if( p->activity > 0 ) {
	   /* If divice is in idle mode, it must move on to enable mode 
	    * on it own when the correponding activity occurs.
	    */
                p->activity = 0;
                p->idle_counter = 0;

           } else if( p->activity < 0 ) {
                p->idle_counter = 0;

           } else {
		if (check_necessity_of_device_idle(p)) {
    			p->idle_counter += PM_SYSTEM_IDLE_INTERVAL;
			check_device_idle_time(p);
		} /* endif */
	   } /* endif */

	} /* endif for non-graphical input/output device */

	p = p->next1;

    } /* while */

    manage_sync_timer();

    return;

} /* check_device_idle */




/*
 * NAME:  pm_start_system_full_on
 *
 * FUNCTION:  brings entire system into system full-on state
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_handle - PM handle structure of PM aware DDs
 *      pm_kernel_data - PM data structure in kernel
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS : Successfully processed
 *      PM_ERROR   : Failed to call handler
 */
int
pm_start_system_full_on(struct pm_state *pms)
{
    struct pm_handle *p = pm_kernel_data.handle_head;

    pms->event = PM_EVENT_NONE;
    while( p != NULL ) {
	if( (p->handler == NULL) ||
	    ((p->handler)( p->private, PM_DEVICE_FULL_ON ) == PM_ERROR) ) {
	    pms->event = PM_EVENT_ERROR;
	    pms->devno = p->devno;
	}
	p = p->next1;
    }

    pm_core_data.system_state = PM_SYSTEM_FULL_ON;
    if( pms->event == PM_EVENT_ERROR ) {
	return PM_ERROR;
    }
    return PM_SUCCESS;
}


/*
 * NAME:  pm_start_system_enable
 *
 * FUNCTION:  brings entire system into system PM enable state
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_handle - PM handle structure
 *      pm_kernel_data - PM data structure in kernel
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS : Successfully processed
 *      PM_ERROR   : Failed to call handler
 */
int
pm_start_system_enable(struct pm_state *pms)
{
    struct pm_handle *p = pm_kernel_data.handle_head;
    int	state = pm_core_data.system_state;

    pm_core_data.system_state = PM_SYSTEM_ENABLE;
    while( p != NULL ) {
	if( (p->attribute & PM_GRAPHICAL_OUTPUT) &&
			(p->mode == PM_DEVICE_ENABLE) &&
			(state == PM_SYSTEM_SUSPEND) ) {
	    if( p->handler != NULL ) {
		(p->handler)( p->private, PM_DEVICE_IDLE );
	    }
	}
	if( (p->attribute & PM_GRAPHICAL_INPUT) ||
			(p->attribute & PM_GRAPHICAL_OUTPUT) ||
			(p->activity < 0) ) {
	    if( (p->handler == NULL) ||
		((p->handler)( p->private, PM_DEVICE_ENABLE ) == PM_ERROR) ) {
		pms->event = PM_EVENT_ERROR;
		pms->devno = p->devno;
	    }
	    p->idle_counter = 0;
	} else if( p->mode == PM_DEVICE_FULL_ON ) {
	    if( (p->handler == NULL) ||
		((p->handler)( p->private, PM_DEVICE_ENABLE ) == PM_ERROR) ) {
		pms->event = PM_EVENT_ERROR;
		pms->devno = p->devno;
	    }
	    p->idle_counter = 0;
	} else if( p->mode == PM_DEVICE_IDLE ) {
	    ;	/* do nothing */
	}
	p = p->next1;
    }

    if(pmstate[ID2INDEX(pm_event_id - 1)].state == state) {
	pmstate[ID2INDEX(pm_event_id - 1)].event = pms->event;
	pmstate[ID2INDEX(pm_event_id - 1)].devno = pms->devno;
    }
    pm_core_data.system_idle_count = 0;
    pm_core_data.system_next_state = PM_NONE;
    if( pms->event == PM_EVENT_ERROR ) {
	return PM_ERROR;
    }
    return PM_SUCCESS;
}


/*
 * NAME:  pm_start_system_standby
 *
 * FUNCTION:  brings entire system into system standby state.
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_handle - PM handle structure
 *      pm_kernel_data - PM data structure in kernel
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS : Successfully processed
 *      PM_ERROR   : Failed to call handler
 */
int
pm_start_system_standby(struct pm_state *pms)
{
    struct pm_handle *p = pm_kernel_data.handle_head;

    pms->event = PM_EVENT_NONE;
    while( p != NULL ) {
	if( p->attribute & PM_GRAPHICAL_OUTPUT ) {
	    if( (p->handler == NULL) || ((p->mode != PM_DEVICE_IDLE) &&
		((p->handler)( p->private, PM_DEVICE_IDLE ) == PM_ERROR)) ) {
		pms->event = PM_EVENT_ERROR;
		pms->devno = p->devno;
	    }
	} else if( p->attribute & PM_GRAPHICAL_INPUT ) {
	    if( p->activity > 0 ) {
		p->activity = 0;
	    }
	}
	p = p->next1;
    }

    pm_core_data.system_state = PM_SYSTEM_STANDBY;
    check_device_idle();
    pm_planar_control(0, pm_core_data.pmdev_psusus, PM_PLANAR_LOWPOWER1);
    if( pms->event == PM_EVENT_ERROR ) {
	return PM_ERROR;
    }
    return PM_SUCCESS;
}


/*
 * NAME:  pm_start_system_suspend
 *
 * FUNCTION:  brings entire system into suspend state phase 1
 *
 * EXECUTION ENVIRONMENT:
 *      This routine can be called from either the process or interrupt
 *      environment.
 *      It can not page fault.
 *
 * NOTES:
 *	<<<<< This routine is only called for PM phase1 >>>>
 *
 * DATA STRUCTURES:
 *      pm_handle - PM handle structure
 *      pm_kernel_data - PM data structure in kernel
 *      pm_core_data - Data structure in PM core
 *
 * RETURN VALUE DESCRIPTION:	
 *      PM_SUCCESS : Successfully processed
 *      PM_ERROR   : Failed to call handler
 */
int
pm_start_system_suspend(struct pm_state *pms)
{
    pm_system_state_t	dummy;
    struct pm_handle *p = pm_kernel_data.handle_head;

    sync(); sync(); sync();

    pms->event = PM_EVENT_NONE;
    while( p != NULL ) {
	if( p->attribute & PM_GRAPHICAL_OUTPUT ) {
	    if( (p->handler == NULL) || ((p->mode != PM_DEVICE_IDLE) &&
		((p->handler)( p->private, PM_DEVICE_IDLE ) == PM_ERROR)) ) {
		pms->event = PM_EVENT_ERROR;
		pms->devno = p->devno;
	    }
	} else if( p->attribute & PM_GRAPHICAL_INPUT ) {
	    if( p->activity > 0 ) {
		p->activity = 0;
	    }
	}
	p = p->next1;
    }

    pm_core_data.system_state = PM_SYSTEM_SUSPEND;
    check_device_idle();
    pm_planar_control(0, pm_core_data.pmdev_psusus, PM_PLANAR_OFF);
/*
    state_transition(PM_SYSTEM_SUSPEND, (caddr_t)&dummy); 
*/


    if( pms->event == PM_EVENT_ERROR ) {
	return PM_ERROR;
    }
    return PM_SUCCESS;
}



/*
 * ------------------------------------------------------------------
 *  The following code is brought from ./kernel/proc/POWER/gettod.c
 * ------------------------------------------------------------------
 */
#define  SECPERMINUTE    (60)
#define  SECPERHOUR      (SECPERMINUTE*60)
#define  SECPERDAY       (SECPERHOUR*24)
#define  SECPERYEAR      (SECPERDAY*365)
#define  NO_MONTHS       (12)
#define  DAYSPERYEAR     (365)
#define  MINPERHOUR      (60)
#define  HOURSPERDAY     (24)
#define  SECSTOYEAR(y)   (((y)*SECPERYEAR)+((((y)+1)/4)*SECPERDAY))

/* The following are used as array indices */
#define  JAN_INDEX   0
#define  FEB_INDEX   (JAN_INDEX + 1)

/* An array of the number of days per month. */
static char dpm[NO_MONTHS] = {
        31,  /* JAN */        /* Note: When used, a copy is */
        28,  /* FEB */        /*   made so that FEB can be  */
        31,  /* MAR */        /*   changed to 29 without    */
        30,  /* APR */        /*   affecting re-entrancy.   */
        31,  /* MAY */
        30,  /* JUN */
        31,  /* JUL */
        31,  /* AUG */
        30,  /* SEP */
        31,  /* OCT */
        30,  /* NOV */
        31   /* DEC */
};


/*
 * NAME:  secs_to_date
 *
 * FUNCTION: Convert seconds since the Epoch to MM DD YY.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called under a process from settimer() to save the
 *	system time to the time-of-day chip.
 *
 * NOTES:  POSIX 1003.1 defines 'seconds since the Epoch' as "a value to be
 *	interpreted as the number of seconds between a specified time and
 *	the Epoch".  1003.1 further specifies that "a Coordinated Universal
 *	Time name . . . is related to a time represented as seconds since
 *	the Epoch according to the expression:
 *		tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *		(tm_year-70)*31536000 + ((tm_year-69)/4)*86400"
 */
void
secs_to_date(register struct tms *timestruct)
{
	register unsigned int    i, tempsecs;
	char     tempdpm[NO_MONTHS];

	/* Make a copy of the days per month structure for re-entrancy.*/
	for (i=0; i < NO_MONTHS; i++)  {
		tempdpm[i] = dpm[i];
	}

	/*  Make a copy of the number of seconds since the Epoch.  */
	tempsecs = timestruct->secs;

	/*  Initialize all the values.  */
	timestruct->yrs = 0;
	timestruct->mths = timestruct->dom = 0;
	timestruct->hrs = timestruct->mins = timestruct->no_secs = 0;
	timestruct->jul_100 = timestruct->jul_dig = 0;

	/*
	 * Calculate the current year.  We guess at the year by assuming
	 * there are no leapyears.  If the number of seconds in all the
	 * years up to this point is more than the number of seconds I
	 * am converting, I must have guessed wrong, so I take off the
	 * year I shouldn't have added in.
	 */

	timestruct->yrs = tempsecs / SECPERYEAR;
	if ((i = SECSTOYEAR(timestruct->yrs)) > tempsecs) {
		timestruct->yrs--;
		i = SECSTOYEAR(timestruct->yrs);
	}
	tempsecs -= i;

	/*  
	 * Correct days per month for February if this is a leap year.  
	 * Add 2 for base of 68.  A leap year. 
	 */
	if(!((timestruct->yrs+2) % 4))  {
		tempdpm[FEB_INDEX] = 29;
	}

	/*  Calculate the current month.  */
	while(tempsecs >= (tempdpm[timestruct->mths] * SECPERDAY))  {
		tempsecs -= (tempdpm[timestruct->mths] * SECPERDAY);
		if(((int)timestruct->jul_dig + tempdpm[timestruct->mths]) > 99){
			timestruct->jul_100++;
			timestruct->jul_dig = (int)timestruct->jul_dig +
					      tempdpm[timestruct->mths] - 100;
		}
		else  {
			timestruct->jul_dig += tempdpm[timestruct->mths];
		}
		timestruct->mths++;
	}
	timestruct->mths++;

	/*  Calculate the current day.  */
	timestruct->jul_dig += timestruct->dom = tempsecs / SECPERDAY;
	tempsecs %= SECPERDAY;
	timestruct->dom++;
	timestruct->jul_dig++;
	if(timestruct->jul_dig > 99)  {
		timestruct->jul_100++;
		timestruct->jul_dig -= 100;
	}

	/*  Calculate the current hour.  */
	timestruct->hrs = tempsecs / SECPERHOUR;
	tempsecs %= SECPERHOUR;

	/*  Calculate the current minute.  */
	timestruct->mins = tempsecs / SECPERMINUTE;
	tempsecs %= SECPERMINUTE;

	/*  The remainder is the current seconds.  */
	timestruct->no_secs = tempsecs;

	/*  Convert to Binary Coded Decimal.  */
	timestruct->ms = (int)timestruct->ms / 10 * 16 +
				((int)timestruct->ms % 10);
	timestruct->mins = (int)timestruct->mins / 10 * 16 +
				((int)timestruct->mins % 10);
	timestruct->hrs = (int)timestruct->hrs / 10 * 16 +
				((int)timestruct->hrs % 10);
	timestruct->dom = (int)timestruct->dom / 10 * 16 +
				((int)timestruct->dom % 10);
	timestruct->mths = (int)timestruct->mths / 10 * 16 +
				((int)timestruct->mths % 10);
	timestruct->yrs = (int)timestruct->yrs / 10 * 16 +
				((int)timestruct->yrs % 10);
	timestruct->no_secs = (int)timestruct->no_secs / 10 * 16 + 
				((int)timestruct->no_secs % 10);
	timestruct->jul_dig = (int)timestruct->jul_dig / 10 * 16 + 
				((int)timestruct->jul_dig % 10);
}


/*
 * NAME:  date_to_secs
 *
 * FUNCTION: Convert MM DD YY to seconds since the Epoch.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This routine is called under a process at system initialization to
 *	restore the system time from the time-of-day chip.
 *
 * NOTES:  POSIX 1003.1 defines 'seconds since the Epoch' as "a value to be
 *	interpreted as the number of seconds between a specified time and
 *	the Epoch".  1003.1 further specifies that "a Coordinated Universal
 *	Time name . . . is related to a time represented as seconds since
 *	the Epoch according to the expression:
 *		tm_sec + tm_min*60 + tm_hour*3600 + tm_yday*86400 +
 *		(tm_year-70)*31536000 + ((tm_year-69)/4)*86400"
 */
date_to_secs(struct tms *timestruct)
{
	register int	year;
	register int	day;

	year = (((int)timestruct->yrs / 16 * 10) + ((int)timestruct->yrs % 16));
	day = ((int)timestruct->jul_100 * 100) + 
		((int)timestruct->jul_dig / 16 * 10) +
		((int)timestruct->jul_dig % 16);
	/*  Correct for julian day starting at 1 rather than 0.  */
	day--;
	timestruct->secs =	
		((int)timestruct->no_secs / 16 * 10) + 
		((int)timestruct->no_secs % 16) + 
		(((int)timestruct->mins / 16 * 10) * 60) +
		(((int)timestruct->mins % 16) * 60) +
		(((int)timestruct->hrs / 16 * 10) * 3600) +
		(((int)timestruct->hrs % 16) * 3600) +
		(day * 86400) +
		(year * 31536000) +
		(((year + 1)/4) * 86400);
	timestruct->ms = (int)(timestruct->ms / 16 * 10) +
		(int)(timestruct->ms % 16);
}



/*
 * NAME:  battery_query
 *
 * FUNCTION: Query the current battery status, AC/DC state,
 *	     battery availability itself. 
 *
 * NOTE: Until gathering battery status is complete, PMMD
 *	 doesn't return unless the procedure has an error.
 *
 * DATA STRUCTURES:
 *        pm_battery_data_t - PM battery information structure
 *
 * RETURN VALUE DESCRIPTION:
 *	  none 	
 */
void
battery_query(pm_battery_data_t * last_battery_data)
{
    battery_control_t	battery_control;

    if (pm_machine_profile.battery_support != 0) {

    battery_control.control = query_battery;
    battery_control.battery_status = last_battery_data;
    /* 
     * Calling PMMD with lock 
     */
    CALL_PMMD(battery_job,&battery_control)

    if (battery_status_ready.available == FALSE) {
	/* 
	 * Even though battey query has not been done yet, 
	 * this wakeup doesn't make any side-effect.
	 */
    	battery_status_ready.available = TRUE;
	e_wakeup(&battery_status_ready.event_wait);

    } /* endif */

    } /* endif */

    return;

} /* battery_query */

