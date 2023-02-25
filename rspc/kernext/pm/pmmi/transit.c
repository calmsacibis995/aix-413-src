static char sccsid[] = "@(#)09  1.19  src/rspc/kernext/pm/pmmi/transit.c, pwrsysx, rspc41J 6/6/95 23:52:27";
/*
 *   COMPONENT_NAME: PWRSYSX
 *
 *   FUNCTIONS:
 *           state_transition(),alert_user_with_BEEP()
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

#include <sys/errno.h>
#include <sys/sleep.h>
#include <sys/pm.h>
#include <sys/pmdev.h>
#include <sys/malloc.h>
#include <sys/device.h>
#include "pmmi.h"


/*---------------------------------*/
/* PMMI internal control data      */
/*---------------------------------*/
/*---------------------------------*/
/* External Grobal variable        */
/*---------------------------------*/
extern pm_parameters_t   pm_parameters;
extern pmmd_IF_t pmmd_IF; 
extern pm_hibernation_t pm_hibernation;
extern pm_control_data_t pm_control_data;
extern pm_machine_profile_t  pm_machine_profile;
extern void set_new_registered_PM_awareDD_to_enable(); 
extern int total_memory;	/* unit = byte */
extern char LastRejectDD[0x10];
extern transit_lock_t  transit_lock;
extern event_query_control_t event_query_control;
extern int still_screen_dim;


/*---------------------------------*/
/* External functions              */
/*---------------------------------*/
extern int broadcast2DD(broadcast_block_t *bc_blk);
extern int init_hibernation(pm_hibernation_t *);
extern int start_hibernation(int *);
extern int term_hibernation(void);
extern int term_devdump();


/*---------------------------------*/
/* Function proto type             */
/*---------------------------------*/
void state_transition(pm_system_state_t *arg);
void alert_user_with_BEEP(void);
void update_BEEP_setting();
void forced_terminate();	

/*---------------------------------*/
/* For pmjob_on_INTLEVEL           */
/*---------------------------------*/
int				pmjob_on_INTLEVEL();
struct _pm_off_level_data {
	struct intr		pm_off_level;
	int			off_level_done;
	Simple_lock		lock_for_off_level;
	int			pm_off_level_event;
	int			state;
} pm_off_level_data = {{0}, TRUE, {0}, EVENT_NULL, NULL};


/*
 * NAME:  System State transition
 *
 * FUNCTION:  Change system state
 *
 * NOTES:
 *
 * Parameters:
 *      ctrl - Specifies a system state
 *
 */
void
state_transition(pm_system_state_t *arg)
{
   int                  pmmd_rc;
   time_t   		dat;
   broadcast_block_t	bd;
   LED_block_t		LED_block;
   BEEP_block_t		BEEP_block;
   resume_time_data_t	resume_time_data;
   rsm_en_event_t 	rsm_en_event;
   rsm_trig_blk_t 	rsm_trig_blk;
   dev_t		devno_area;
   int			oldpri;

   
   switch (arg->state) {
   case PM_SYSTEM_FULL_ON:
	DBG(===================================)
	DBG( Going to SYSTEM_FULL_ON)
	DBG(===================================)
	bd.control = PM_DEVICE_FULL_ON;
        (void)broadcast2DD(&bd);
	pm_control_data.system_state = PM_SYSTEM_FULL_ON;
	arg->event = NULL;
	pm_control_data.last_resume_event == PM_EVENT_NONE;
	return;

   case PM_SYSTEM_ENABLE:
	DBG(===================================)
	DBG( Going to SYSTEM_ENABLE)
	DBG(===================================)
	bd.control = PM_DEVICE_ENABLE;
        (void)broadcast2DD(&bd);
	still_screen_dim = FALSE;
	pm_control_data.system_state = PM_SYSTEM_ENABLE;
	arg->event = NULL;
	pm_control_data.last_resume_event = PM_EVENT_NONE;
	return;


   case PM_SYSTEM_STANDBY:
	DBG(===================================)
	DBG( Going to SYSTEM_STANDBY)
	DBG(===================================)
	bd.control = PM_DEVICE_DPMS_STANDBY; /* turn off screen immediately */
        (void)broadcast2DD(&bd);

	/* pmkproc uses device standby timer of each PM aware DDs. */
	pm_control_data.system_state = PM_SYSTEM_STANDBY; 

	arg->event = NULL;
	pm_control_data.last_resume_event = PM_EVENT_NONE;
	return;


   case PM_SYSTEM_SUSPEND:
	DBG(===================================)
	DBG( Going to SYSTEM_SUSPEND)
	DBG(===================================)
	goto power_transition_common;

   case PM_SYSTEM_HIBERNATION:
	DBG(===================================)
	DBG( Going to SYSTEM_HIBERNATION)
	DBG(===================================)

power_transition_common:
	DBG(===================================)
	DBG( Check a PM aware DD newly loaded  )
	DBG(===================================)
	/* Clean up if a PM aware DD newly registered(loaded) is running 
	 * on full_on mode even during PM_SYSTEM_ENABLE state.
 	 */ 
	set_new_registered_PM_awareDD_to_enable(); 



	DBG(===================================)
	DBG(  Preparation of broadcasting      ) 
	DBG(===================================)
	  if (PrepareSuspHib()) {
		arg->event =
		pm_control_data.last_resume_event = PM_EVENT_GENERAL_ERROR;
                return; 
	  } /* endif */


	DBG(===================================)
	DBG(   Preparation for hibernation     ) 
	DBG(===================================)
         /*
          * Prepare to confirm the availability of hibernation volume
	  * and the file of wakeup code. Even in entering suspend, 
	  * this preparation is needed since the transition from suspend
          * to hibernation may occur during suspend state. 
          */
#ifdef PM_DEBUG
	printf("--Dump data of pm_hibernation volume information---\n");
	printf("pm_hibernation.devno:	%08X\n",pm_hibernation.devno);
	printf("pm_hibernation.ppnum: 	%08X\n",pm_hibernation.ppnum);
	printf("pm_hibernation.ppsize: 	%08X\n",pm_hibernation.ppsize);
	printf("pm_hibernation.snum: 	%08X\n",pm_hibernation.snum);
	printf("pm_hibernation.slist: 	%08X\n",pm_hibernation.slist);
	printf("------- Dump data of the first sector list --------\n");
	printf("(sector address, sector length):(%08X,  %08X)\n", \
				(pm_hibernation.slist)->start,	  \
			  	(pm_hibernation.slist)->size);
#endif
	if (pm_hibernation.slist != NULL) { 
        	if (init_hibernation(&pm_hibernation) != PM_SUCCESS) {
			DBG(Invalid hibernation volume or no wakeup file) 
			TerminateSuspHib();
	 		if (arg->state == PM_SYSTEM_HIBERNATION) {
				arg->event = PM_EVENT_REJECT_BY_HIB_VOL; 
				pm_control_data.last_resume_event = 
					     PM_EVENT_REJECT_BY_HIB_VOL; 
                		return;
			} else {
	 			pm_hibernation.slist = NULL;
        		} /* endif */
        	} /* endif */
        } /* endif */


	DBG(===================================)
	DBG(       PMMD interface lock         )
	DBG(===================================)
         /*
          * Set PMMD interface exclusive
          */
         simple_lock(&(pmmd_IF.lock));


	DBG(===================================)
	DBG(Broadcasting PM_PAGE_FREEZE_NOTICE)
	DBG(===================================)
         /*
          * Broadcast the event to all of PM aware DD
          */
 	 bd.control = PM_PAGE_FREEZE_NOTICE;
         if (broadcast2DD(&bd) == PM_ERROR) {
		memcpy(arg->name,&(bd.device_logical_name),0x10);
		memcpy(LastRejectDD,&(bd.device_logical_name),0x10);
         	simple_unlock(&(pmmd_IF.lock));
         	term_hibernation();
		TerminateSuspHib();
		arg->event =
		pm_control_data.last_resume_event = PM_EVENT_REJECT_BY_DD;
                return;		/* Retry may be executed later */
	 } /* endif */


	 if (pm_parameters.core_data.ringing_resume == TRUE) {
	DBG(===================================)
	DBG( 	PM_RING_RESUME_ENABLE_NOTICE   )
	DBG(===================================)
 	 	bd.control = PM_RING_RESUME_ENABLE_NOTICE; 
                (void)broadcast2DD(&bd);
	 } /* endif */


	DBG(===================================)
	DBG(     Quiesence user process        )
	DBG(===================================)
	 /* All user processes regardless of being in
	  * either kernel mode or user mode are stopped
	  * here. 
	  */
	  pm_proc_stop(PM_STOP_KTHREADS);


	DBG(===================================)
	DBG(        Issue SYNC()               )
	DBG(===================================)
	 /* Flush all of the unresolved data */
	 sync();


	if (arg->state == PM_SYSTEM_HIBERNATION) {
	DBG(==========================================)
	DBG( Swap out unused pages as much as possible)
	DBG(==========================================)
	/* The "total_memory" is obtained in "init_hibernation"
         * which is beforehand called above. 
	 *
	 * We request to free memory as much as possible. 
	 * Hence, the current system memory size is used here.
	 */
		(void)pm_free_memory((total_memory-0x1000000)/0x1000); 

	} /* done if hibernation */


	DBG(===================================)
	DBG(Broadcasting state change request)
	DBG(===================================)
	 if (arg->state == PM_SYSTEM_SUSPEND) {
		bd.control = PM_DEVICE_SUSPEND;
	 } else {
		bd.control = PM_DEVICE_HIBERNATION;
	 } /* endif */
         if (broadcast2DD(&bd) != PM_SUCCESS) {
		memcpy(arg->name,&(bd.device_logical_name),0x10);
		memcpy(LastRejectDD,&(bd.device_logical_name),0x10);
		(void)pm_proc_start();
         	simple_unlock(&(pmmd_IF.lock));
         	term_hibernation();
		TerminateSuspHib();
		arg->event =
		pm_control_data.last_resume_event = PM_EVENT_REJECT_BY_DD;
                return;		/* Retry may be executed later */
         } /* endif */


	DBG(===================================)
	DBG( Basic device context saving comp  )
	DBG(===================================)
         /*
          * Save basic devices
          */
          CALL_PMMD_WO_LOCK(save_basic_devices,NULL)


	DBG(===================================)
	DBG(RTC alarm resume/ sus2hiber by RTC )
	DBG(===================================)
         /*
          * Set RTC(specified time) resume condition
	  *  ( They may be zero meaning "disable".) 
          */
	 resume_time_data.resume_at_specified_time 
			= pm_parameters.core_data.resume_time;
	 if (pm_hibernation.slist != NULL) { 
	 	resume_time_data.duration_for_susp2hib
			= pm_parameters.core_data.sus_to_hiber;
	 } else {
	 	resume_time_data.duration_for_susp2hib
			= NULL; 
	 } /* endif */
	 resume_time_data.selected_resume_time 
			= NULL;
         CALL_PMMD_WO_LOCK(set_resume_time, &resume_time_data)

	/* selected_resume_time is setup by PMMD(set_resume_time) */
	 pm_control_data.RTCresumeVariety 
			= resume_time_data.selected_resume_time;


        DBG(===================================)
        DBG( Adjust RTC chip using system time )
        DBG(===================================)
        CALL_PMMD_WO_LOCK(adjust_system_time, NULL)


	/*
	 * The last stage prior to starting suspend/hibernation.
	 * No return after here.
	 */ 
	pm_control_data.GoingFromSuspendToHibernation = FALSE;

SuspendHibernationCoreEntry:
	DBG(===================================)
	DBG(  	Set up Resume triggers         ) 
	DBG(===================================)
	 /* 
	  * Enable external PM event for resume 
	  *     ( enable LID,Power switch, KBD/Mouse events etc. )
	  */
	 rsm_en_event = pm_machine_profile.resume_events;
	 if (pm_hibernation.slist == NULL) { 
                rsm_en_event.critical_low_battery_hibernation_from_suspend 
			= NOT_SUPPORTED;
	 } /* endif */

         /*
          * Set ring resume condition
          */
	 if (pm_parameters.core_data.ringing_resume == TRUE) {
	DBG(===================================)
	DBG(  Set Ring resume trigger enabled  )	
	DBG(===================================)
                rsm_en_event.MDM_ring_resume_from_suspend = RESUME_ENABLE; 
                rsm_en_event.MDM_ring_resume_from_hibernation = RESUME_ENABLE; 
         } else {
	DBG(===================================)
	DBG(  Set Ring resume trigger disabled )	
	DBG(===================================)
               	rsm_en_event.MDM_ring_resume_from_suspend = RESUME_DISABLE; 
               	rsm_en_event.MDM_ring_resume_from_hibernation = RESUME_DISABLE; 
         } /* endif */

	CALL_PMMD_WO_LOCK(set_resume_event, &rsm_en_event)
	DBG(resume event trigger setup has completed)


         /***************************************************************/
         /***** Suspend/hibernation core job has started.           *****/
         /***************************************************************/

	 DOUT(0x30)

	 /* Stop all processes */
	 pm_proc_stop(PM_STOP_KOTHREADS);

	 pm_off_level_data.state = arg->state;
	 pm_off_level_data.off_level_done = FALSE;

	 /* Start hibernation dump in the interrupt environment */
	 INIT_OFFL0(&(pm_off_level_data.pm_off_level), pmjob_on_INTLEVEL, 0);
	 i_sched((struct intr *)(&pm_off_level_data));

	 /* Wait for the completion of interrrupt routine */
	 oldpri = disable_lock(INTMAX, &(pm_off_level_data.lock_for_off_level));
	 while(!(pm_off_level_data.off_level_done)){
		e_sleep_thread(&(pm_off_level_data.pm_off_level_event),
			&(pm_off_level_data.lock_for_off_level), LOCK_HANDLER);
	 }
	 unlock_enable(oldpri, &(pm_off_level_data.lock_for_off_level));

         /***************************************************************/
         /*
          * <<< Hibernation core job >>>     |  <<< suspend core job >>>
          * Saving CPU context to memoryr    | 	Saving CPU context to memory
          * Store system memory to disk      |	
          * Main power off		     |	Main power off
          * -------- power off ------------- |  ------- power off ---------
          * Restore system memory from disk  |	  
          * Restore CPU context from memory  |	Restore CPU context from memory
          */
         /***************************************************************/



            	/* Get resume event from PMMD
		 * This calling also make an effect of reconnection with 
		 * slave CPU. This must be done prior to the following
 		 * LED job since it's performed by slave CPU.
             	 */
	DBG(=================================================)
	DBG( Slave CPU I/F for resume/Check the resume event )
	DBG(=================================================)
	     	DOUT(0x34)
		rsm_trig_blk.state = arg->state;
             	CALL_PMMD_WO_LOCK_W_RC(get_resume_event, &rsm_trig_blk,pmmd_rc)

             	arg->event 
		= pm_control_data.last_resume_event = rsm_trig_blk.event;

#ifdef PM_DEBUG
	printf("Resume trigger is %D\n",pm_control_data.last_resume_event);
#endif
		if (pmmd_rc == TurnAround2Suspend) {
		/* 
		 * if RTC resume but the date/time doesn't match
		 * the specified one, we go to suspend again.
		 */
			DBG(Suspend again due to not matched date) 
			goto SuspendHibernationCoreEntry;

		} else if (pmmd_rc == AbnormalResume) {
			arg->event =
			pm_control_data.last_resume_event = 
				PM_EVENT_GENERAL_ERROR;
			DBG(Slave CPU init error at resume from hibernation)

		} else if (pmmd_rc == HibernationEnterFail) {

		      	if(pm_control_data.GoingFromSuspendToHibernation==TRUE){
				arg->state = PM_SYSTEM_SUSPEND;
	 			pm_control_data.system_state=PM_SYSTEM_SUSPEND;
				/* No choice, just go back to suspend again */
			DBG(Suspend again due to error on suspend2hibernation)
				goto SuspendHibernationCoreEntry;
			} /* endif */

			arg->event =
			pm_control_data.last_resume_event = 
				PM_EVENT_REJECT_BY_HIB_VOL;		
			DBG(Hibernation enter error)
		} /* endif */



	    	/* 
		 * Transition indicator start 
		 */
	DBG(================================)
	DBG(     LED blink start   	    )
	DBG(================================)
	     	DOUT(0x35)
             	LED_block.LED_type=suspend_LED;
             	LED_block.control=LED_blink;
             	CALL_PMMD_WO_LOCK(LED_control,&LED_block)
             	LED_block.LED_type=power_on_LED;
             	LED_block.control=LED_on;
             	CALL_PMMD_WO_LOCK(LED_control,&LED_block)



	DBG(==================================)
	DBG(  Check suspend to hibernation    ) 
	DBG(==================================)
         if ((pm_control_data.last_resume_event 
				== PM_EVENT_CRITICAL_LOW_BATTERY) 
				||
	     ((pm_control_data.last_resume_event ==  PM_EVENT_RTC) &&
	      (pm_control_data.RTCresumeVariety==SuspendToHibernationByRTC))){

		arg->state = PM_SYSTEM_HIBERNATION;
	 	pm_control_data.system_state = PM_SYSTEM_HIBERNATION;
		pm_control_data.GoingFromSuspendToHibernation = TRUE;
		goto SuspendHibernationCoreEntry;

	 } /* endif */



		/* 
		 * System time is updated here. 
		 */
	DBG(================================)
	DBG(     System time update 	    )
	DBG(================================)
		DOUT(0x33)
		CALL_PMMD_WO_LOCK(reconstruct_systime,NULL)


	    	/* 
	     	* By means of this "state" below, PMD can know if suspend to 
	     	* hibernation has occurred. This state must be suspend if so 
	     	* though it is resume from hibernation. 
	     	*/
	     	DOUT(0x37)
             	arg->state = pm_control_data.system_state;
		DBG(transit.c return from get_resume_event)



	DBG(================================)
	DBG(Broadcasting resume notice stage)
	DBG(================================)
         /*
          * Broadcast the event to all of PM aware DD
          */
 	 bd.control = PM_DEVICE_RESUME;
         (void)broadcast2DD(&bd);


	DBG(==================================)
	DBG(      Restart user process        )
	DBG(==================================)
	(void)pm_proc_start();


	DBG(==================================)
	DBG(Broadcasting PAGE_UNFREEZE_NOTICE)
	DBG(==================================)
 	 bd.control = PM_PAGE_UNFREEZE_NOTICE;
         (void)broadcast2DD(&bd);


	DBG(===================================)
	DBG(       PMMD interface unlock       )
	DBG(===================================)
         /*
          * Stop PMMD interface to be exclusive
          */
         simple_unlock(&(pmmd_IF.lock));


	DBG(===================================)
	DBG(      Hibernation post process     )
	DBG(===================================)
         /*
          * Release extra memory obtained to enter hibernation
          */
         term_hibernation();


	DBG(=========================================)
	DBG(  Freeing memory for transition job      )
	DBG(=========================================)
	  TerminateSuspHib();


	DBG(=========================================)
	DBG(  Getting out of suspend/hibernation job )
	DBG(=========================================)
	 pm_control_data.system_state = PM_SYSTEM_ENABLE;
         return;




   case PM_TRANSITION_START:  /* received only in case of suspend or hib */

	 if (arg->event == PM_EVENT_TERMINATE) {

	 	forced_terminate();	/* However, not equal to unconfig */	

	 } else if (pm_control_data.shutdown_on_going != TRUE) {
	 /* Because Power_LED is already blinking 
	  * when main power sw off is detected. 
	  */
         	LED_block.LED_type=suspend_LED;
         	LED_block.control=LED_blink;
         	CALL_PMMD(LED_control,&LED_block)

                if (ScanPMawareDD(&devno_area) == PM_ERROR) {
                        arg->event = PM_EVENT_PHASE1_DD;
		} else {
	 		arg->event = NULL;
                } /* endif */
		memset(arg->name,NULL,0x10);

	 } /* endif */
         return;


   case PM_TRANSITION_END:    /* received only in case of suspend or hib */

         LED_block.LED_type=suspend_LED;
         LED_block.control=LED_off;
         CALL_PMMD(LED_control,&LED_block)
         LED_block.LED_type=power_on_LED;
         LED_block.control=LED_on;
         CALL_PMMD(LED_control,&LED_block)
	 arg->event = NULL;
	 memcpy(arg->name,LastRejectDD,0x10);

         if (pm_parameters.core_data.pm_beep == TRUE) {
	     if((pm_control_data.last_resume_event == PM_EVENT_REJECT_BY_DD) 
					||
		(pm_control_data.last_resume_event == PM_EVENT_GENERAL_ERROR)
					||
                (pm_control_data.last_resume_event == PM_EVENT_PHASE1_DD)
                                        ||
                (pm_control_data.last_resume_event == PM_EVENT_NOT_SUPPORTED)
					||
		(pm_control_data.last_resume_event 
						== PM_EVENT_REJECT_BY_HIB_VOL)){

               		BEEP_block.BEEP_type = reject_beep; 
                	pm_control_data.last_resume_event = NULL;
	     } else {
               		BEEP_block.BEEP_type = suspend_beep; 
	     } /* endif */
               BEEP_block.control = BEEP_oneshot;
               CALL_PMMD(BEEP_control,&BEEP_block)
         } /* endif */
         return;

   } /* endswitch */

   DBG(Abnormal End of state_transition())
   return; 	/* Usually, never comes here. */

} /* state_transition */



/*
 * NAME:  Alert user with beep
 *
 * FUNCTION:  Beeper
 *
 * NOTES:
 *      Return with job incomplete (For instance, one shot beep is not
 *      completed to be generated.
 *
 */
void
alert_user_with_BEEP()
{
	 BEEP_block_t	BEEP_block;

         if (pm_parameters.core_data.pm_beep == TRUE) {
               BEEP_block.BEEP_type = reject_beep;
               BEEP_block.control = BEEP_oneshot;
               CALL_PMMD_WO_LOCK(BEEP_control,&BEEP_block)
         } /* endif */
         return;
}



/*
 * FUNCTION:  update_BEEP_setting
 *
 * NOTES:
 *      When beep setting is changed from system call: PM_CONTROL_PARAMETER,
 *      this routine is called.
 *
 */
void 
update_BEEP_setting()
{
	BEEP_block_t	BEEP_block;
	
	BEEP_block.BEEP_type = All_types;
	if (pm_parameters.core_data.pm_beep == PM_BEEP_OFF) {
 		BEEP_block.control = BEEP_disable;	
	} else { 
 		BEEP_block.control = BEEP_enable;	
	} /* endif */
	CALL_PMMD(BEEP_control,&BEEP_block)

	/* pm_control_data is exported to pmmd. And it's used for 
         * enabling/disabling PM BEEP. 
 	 */
	pm_control_data.beep_condition = pm_parameters.core_data.pm_beep;

	return;

} /* update_BEEP_setting */



/*
 * FUNCTION:  forced_terminate
 *
 * NOTES:
 *     	In case that PM daemon is killed with sig_term, this function is
 *      called. PM core is not unconfigured in this case. PM core doesn't 
 *	care whether or not pmd works. As a result, external PM event will 
 *	be ignored since pmd doesn't run after coming here. Main power sw 
 *	event will be handled by failsafe routine even in such a case. 
 *	Note that when pmd is forcedly killed by sig_kill and loaded again
 *	without unconfiguring pm core, pm core will automatically unconfigure
 *	when "pm_system_event_query" is called by the new pmd.
 */
void
forced_terminate()
{
   	pm_system_state_t state_data;


	/* Flush event queue */
	(void)init_event_queue();


	/* 
	 * wait pm daemon to call "pm_system_event_query" syscall. 
	 */
        while (!(event_query_control.now_queried)) { 
		delay(HZ/2);	
	} /* endwhile */
	DBG("Confirm PM daemon has already queried to PM core")


	/* 
	 * Now, we are going to transit the system full_on state 
	 * All the PM aware DDs are set to full_on state.
	 */
	state_data.state = PM_SYSTEM_FULL_ON;	
	(void)state_transition(&state_data);  
	DBG(Full_on transition is complete)


        /*
         * Inform PM daemon of PM core unconfiguration
         */
	simple_lock(&(event_query_control.lock));
        (void)insert_new_event_to_Q(PM_EVENT_TERMINATE);
	simple_unlock(&(event_query_control.lock));
	DBG(Make a fake event of PM_EVENT_TERMINATE)

        /*
         * Already confirmed if PM daemon already calls
         * PM core with pm_system_event_query syscal.
         */
        e_wakeup(&event_query_control.event_wait);

	return;

}


/*
 * FUNCTION:  pmjob_on_INTLEVEL()
 *
 * NOTES:
 *     	This function is to execute hibernation dump in the interrupt
 *      environment.
 *      This is invoked using i_sched().
 */
int
pmjob_on_INTLEVEL(struct _pm_off_level_data *pm_off_level_data)
{
	 if (pm_off_level_data->state == PM_SYSTEM_SUSPEND) {
	DBG(===================================)
	DBG(   Enter suspend  		       )
	DBG(===================================)
	 	pm_control_data.system_state = PM_SYSTEM_SUSPEND;
         	CALL_PMMD_WO_LOCK(enter_suspend,NULL) 
	 } else {
	DBG(===================================)
	DBG(  Enter hibernation		       )
	DBG(===================================)
	 	pm_control_data.system_state = PM_SYSTEM_HIBERNATION;
         	CALL_PMMD_WO_LOCK(enter_hibernation,NULL)
	 } /* endif */
         /***************************************************************/
         /*
          * <<< Hibernation core job >>>     |  <<< suspend core job >>>
          * Saving CPU context to memoryr    | 	Saving CPU context to memory
          * Store system memory to disk      |	
          * Main power off		     |	Main power off
          * -------- power off ------------- |  ------- power off ---------
          * Restore system memory from disk  |	  
          * Restore CPU context from memory  |	Restore CPU context from memory
          */
         /***************************************************************/



	/* #################################################### */
	/* Resume procesure from either suspend or hibernation. */
	/* #################################################### */
            	/*
             	* disable all PCI devices
             	*/
                disable_pci_devices();

            	/*
             	* Restore basic devices
             	*/
	     	DOUT(0x32)
             	CALL_PMMD_WO_LOCK(restore_basic_devices,NULL)
	DBG(================================)
	DBG(   Restore_basic_device comp    ) 
	DBG(================================) 
 		/* no printf is effective.    */
		/* Until restore_basic_device,*/


	   	/* Only effective when returning from hibernatin wakeup
	    	* Need to reinitialize dev dump routine which was used 
		* for hibernation 
		* When failing to enter hibernation, only hibernation code
		* knows the actual necessity of "term_devdump". So in such
		* a case, hibernatin code itself calls it. So only at resume
		* from hibernation, it is called here. 
		*/
	DBG(================================)
	DBG(     Calling term_devdump()	    )
	DBG(================================)
	     	DOUT(0x36)
	     	term_devdump(); 	

	pm_off_level_data->off_level_done = TRUE;
	e_wakeup(&(pm_off_level_data->pm_off_level_event));
	return 0;
}
