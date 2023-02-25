static char sccsid[] = "@(#)04  1.12  src/rspc/usr/bin/pmd/pmd_core_event.c, pwrcmd, rspc41J, 9523A_all 6/5/95 23:31:01";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: core_event_interface,
 *            core_event_loop,
 *            get_core_event,
 *            check_proper_state_change_by_event,
 *            set_state_by_event,
 *            check_event_queue,
 *            check_prior_event,
 *            flush_event_queue,
 *            check_misc_event,
 *            check_lid_event,
 *            search_event_queue,
 *            pm_system_event_query(fake system call for debugging),
 *            display_event_queue
 *            display_event_priority_table
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------------------------*/ 
/* Subroutines for Event Thread */
/*------------------------------*/

#include "pmd.h"
#include "pmd_global_ext.h"

/*----------------*/
/* local variable */
/*----------------*/

/* signal set */
sigset_t sigset_core_event;

#ifdef PM_DEBUG2
int sockfd_ev,servlen_ev,clilen_ev;            /* for Debugging */
struct sockaddr_un serv_addr_ev,cli_addr_ev;
char mesg[2048];
int n,clilen;
#endif /* PM_DEBUG2 */

/*
 * NAME:     core_event_interface
 *
 * FUNCTION: handles event from PM core.
 *
 * NOTES:    executed on the thread environment (core_event_thread)
 * 
 * RETURN VALUE DESCRIPTION:
 *           NONE     
 */
void *core_event_interface(void *arg)
{
    Debug0("*** Event Interface Start ***\n");


    /* increment thread_birth */

    Mutex_Lock(&thread_birth_mutex);

    thread_birth++;

    Mutex_Unlock(&thread_birth_mutex);

    /* set signal mask */
    signal_ignore(&sigset_core_event);

#ifdef PM_DEBUG2
    create_socket(&sockfd_ev,&cli_addr_ev,&serv_addr_ev,PMD_UNIXDG_PATH_EV);
#endif /* PM_DEBUG2 */

    core_event_loop();

    pthread_exit(NULL);

}

/*
 * NAME:     core_event_loop
 *
 * FUNCTION: handles event from PM core.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_ERROR : unsuccessefully processed.
 */
int 
core_event_loop()
{
    for(;;){
        get_core_event();
    }
    /* NOT REACHED */
    return PM_ERROR;
}


/*
 * NAME:     get_core_event
 *
 * FUNCTION: put the events aroused from PM core into the event_queue
 *           and check the priority of them.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : successefully processed.
 */
int 
get_core_event()
{
   int action;
   event_queue_t event_queue_entry;

   memset(&event_queue_entry,0,sizeof(event_queue_entry));

   RDebug0(1,"now pm_system_event_query()\n");

   pm_system_event_query(&event_queue_entry.event);

   RDebug1(1,"pmd: I got the event : 0x%x\n",event_queue_entry.event);

#ifdef PM_DEBUG
   display_event_priority_table();
#endif /* PM_DEBUG */

   /* check misc event*/
   check_misc_event(event_queue_entry.event);

   /* process events */
   if (pmd_parameters.pm_present_state.state == PM_SYSTEM_FULL_ON &&
       event_queue_entry.event != PM_EVENT_TERMINATE ){

       /* If system state is FULL_ON, all the other events than           */
       /* POWER_SWITCH_OFF, TERMINATE, LOW_BATTERY are to be ignored.     */
       /* In case of TERMINATE, pm daemon will notice to the applications */
       /* in spite of the FULL_ON state.                                  */

       if (event_queue_entry.event == PM_EVENT_POWER_SWITCH_OFF ){
       
           Debug0("system state is FULL_ON ,and main switch pressed \n");
           Debug0("now go on to shutdown system... \n");
           
           process_shutdown();
           /* NOT_REACHED */ 
       }
       else if (event_queue_entry.event == PM_EVENT_LOW_BATTERY ){
       
           Debug0("system state is FULL_ON and battery capacity is low.\n");
           Debug0("now go on to shutdown system... \n");
           
           process_shutdown();
           /* NOT_REACHED */ 
       }
   }
   else{
       /* if system state is not full_on, or event is TERMINATE */

       event_queue_entry.priority = 
           event_priority_table[EVENT_HASH(event_queue_entry.event)];
       event_queue_entry.state    = 
           set_state_by_event(event_queue_entry.event);
   
       /* add entry to event queue */
   
       Mutex_Lock(&event_queue_mutex); 
       
       event_queue[event_queue_index % PMD_EVENT_QUEUE_LENGTH] = 
           event_queue_entry;

       if (event_queue_index > PMD_EVENT_QUEUE_LENGTH  ){
           event_queue_index -= PMD_EVENT_QUEUE_LENGTH ;
       }
       
       Mutex_Unlock(&event_queue_mutex); 

       /* lock event entry */
       Mutex_Lock(&event_entry_mutex);
       
       /* pick up the event of the highest priority 
          ,the action state assigned to which is set to event_entry*/
   
       check_event_queue(&event_entry);
   
       /* unlock event entry */
       Mutex_Unlock(&event_entry_mutex);
   
       RDebug1(1,"event_entry is %s\n",pmstate2asc(event_entry));
       RDebug1(1,"present state is %s\n",
              pmstate2asc(pmd_parameters.pm_present_state.state));

       if (check_proper_state_change_by_event(event_entry) == PM_SUCCESS){   
           if (check_state_change_process_flag() == PM_SUCCESS ){
       
               /* lock event flag */
               Mutex_Lock(&event_flag_mutex);
               
               event_flag=event_entry;

               /* unlock event flag */
               Mutex_Unlock(&event_flag_mutex);
               
#ifdef PM_DEBUG2
sprintf(mesg,"event accepted and sent to Main thread (#%d)\n",event_flag);
#endif /* PM_DEBUG2 */

               /* wakeup main thread */
               
               wakeup_main_thread(&state_change_predicate,
                                  &state_change_condvar,
                                  &state_change_cond_mutex);
           }
           else{
#ifdef PM_DEBUG2
               sprintf(mesg,"Main thread busy , event rejected\n");
#endif /* PM_DEBUG2 */
               
               /* NO OPERATION IN EFFECT */
           }
       }
       else{
#ifdef PM_DEBUG2
           sprintf(mesg,
                   "The event is not proper for state change and rejected\n");
#endif /* PM_DEBUG2 */
           
           /* NO OPERATION IN EFFECT */
       }
       
       /* increment index for event queue */
       
       event_queue_index ++;

   } /* end of else */

#ifdef PM_DEBUG2
   if (sendto(sockfd_ev,
	      mesg,
	      strlen(mesg),
	      0,
	      (struct sockaddr *)&cli_addr_ev,
	      clilen)!=strlen(mesg)){
       Debug0("event_interface:sendto error\n");
   }
#endif /* PM_DEBUG2 */

   /* if event entry is PM_EVENT_TERMINATE, exit from event thread */

   if (event_queue_entry.event==PM_EVENT_TERMINATE){
       Debug0("now terminating event thread ...... \n");
       pthread_exit((void *)-1);
   }
   return PM_SUCCESS;

}

/*
 * NAME:     check_proper_state_change_by_event
 *
 * FUNCTION: check the target state is neither PM_NONE nor present state.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : appropriate event to wake up the main thread.
 *           PM_ERROR   : not proper event.
 */
int
check_proper_state_change_by_event(int pm_state)
{
    /* in case of event to be ignored */
    RDebug1(1,"check_proper_state_change_by_event(%s)\n",
	    pmstate2asc(pm_state));

    if ( pm_state != PM_NONE &&
         pm_state != pmd_parameters.pm_present_state.state 
       ){
        return PM_SUCCESS;
    }
    else{
        return PM_ERROR;
    }
}


/*
 * NAME:     set_state_by_event
 *
 * FUNCTION: returns the state assigned to the specified event.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SYSTEM_XXXXX : the system state determined by the specified
 *                             event.
 */
int
set_state_by_event(int pm_event)
{
    switch(pm_event){

    /* events for Hardware interrupts */

    case PM_EVENT_LID_CLOSE:
        return (pmd_parameters.lid_close_action & 
		pmd_parameters.system_support);
        break;
    case PM_EVENT_LOW_BATTERY:
        return (pmd_parameters.low_battery_action &
		pmd_parameters.system_support);
        break;
    case PM_EVENT_SYSTEM_IDLE_TIMER:
        return (pmd_parameters.system_idle_action &
		pmd_parameters.system_support);
        break;
    case PM_EVENT_SPECIFIED_TIME:
        return (pmd_parameters.specified_time_action &
		pmd_parameters.system_support); 
       break;
    case PM_EVENT_POWER_SWITCH_OFF:
        return (pmd_parameters.main_switch_action &
		pmd_parameters.system_support);
        break;
    case PM_EVENT_LOWBAT_W_PHASE1DD:
        return PM_SYSTEM_SHUTDOWN;
        break;

    /* in case of resume events from standby */

    case PM_EVENT_LID_OPEN:
    case PM_EVENT_MOUSE:
    case PM_EVENT_KEYBOARD:
    case PM_EVENT_EXTRA_INPUTDD:
    case PM_EVENT_EXTRA_BUTTON:
    case PM_EVENT_POWER_SWITCH_ON:
        return PM_SYSTEM_ENABLE;
        break;

    /* special event for phase1 compatibility */

    case PM_EVENT_P1SWRQ_STANDBY:
        return PM_SYSTEM_STANDBY;
        break;
    case PM_EVENT_P1SWRQ_SHUTDOWN:
        return PM_SYSTEM_SHUTDOWN;
        break;

    /* event for unconfiguring Power Management */ 

    case PM_EVENT_TERMINATE:
        return PM_SYSTEM_TERMINATE;
        break;
    default:
        /* invalid event aroused */
        return PM_NONE;
        break;
    }
}

/*
 * NAME:     check_event_queue
 *
 * FUNCTION: check the event queue to find the most prior event.
 *
 * NOTES:    *p_event_entry : target state.
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : successfully processed.
 */
int
check_event_queue(int *p_event_entry)
{
    int priority=1; /* ignore the event of priority 0 */
    int i;
    int revhist = 0;
    int most_prior_event = PM_EVENT_NONE;
    Debug0("check_event_queue \n");

    /* set PM_NONE to *p_event_entry if all the entries have priority 0 */
    *p_event_entry = PM_NONE;

    Mutex_Lock(&event_queue_mutex);

    for(i= 0 ; i < PMD_EVENT_QUEUE_LENGTH;i++){
        if (event_queue[revhist=
                        ((event_queue_index+i+PMD_EVENT_QUEUE_LENGTH)
                        %PMD_EVENT_QUEUE_LENGTH)
                        ].priority >= priority){
            priority = event_queue[revhist].priority;
            *p_event_entry = event_queue[revhist].state;
            most_prior_event = event_queue[revhist].event;
        }
    }

    Mutex_Unlock(&event_queue_mutex);

#ifdef PM_DEBUG
    display_event_queue();
#endif /* PM_DEBUG */

    RDebug2(1,"event_entry is %s (event:%d)\n",pmstate2asc(*p_event_entry),
            most_prior_event);

    return PM_SUCCESS;
}


/*
 * NAME:     check_lid_event
 *
 * FUNCTION: check the event queue to cancel the events 
 *           related to lid open/close
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS   : successfully processed.
 *           PM_ERROR     : unsuccessfully processed.
 */
int
check_lid_event(event_queue_t *p_lid_event)
{
    int i;
    int history=0;
    int rc=PM_ERROR;

    Debug0("check_lid_event \n");
    memset(p_lid_event,0,sizeof(event_queue_t));
    p_lid_event->event = PM_NONE;

    Mutex_Lock(&event_queue_mutex);

    for(i= 0 ; i < PMD_EVENT_QUEUE_LENGTH;i++){
        history=(event_queue_index-i+PMD_EVENT_QUEUE_LENGTH)
                %PMD_EVENT_QUEUE_LENGTH;

        if (event_queue[history].event == PM_EVENT_LID_OPEN ||
            event_queue[history].event == PM_EVENT_LID_OPEN ){

            *p_lid_event = event_queue[history];

            /* return with the event of the youngest history */

            rc=PM_SUCCESS;
            break;
        }
    }
    Mutex_Unlock(&event_queue_mutex);


    return rc;
}

/*
 * NAME:     search_event_queue
 *
 * FUNCTION: search the event queue for specified event
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           TRUE  : specified event is found.
 *           FALSE : not found.
 */
int
search_event_queue(int pm_event)
{
    int i;

    Debug1("search_event_queue for event :%d\n",pm_event);

    Mutex_Lock(&event_queue_mutex);

    for(i= 0 ; i < PMD_EVENT_QUEUE_LENGTH;i++){
        if (event_queue[(event_queue_index-i+PMD_EVENT_QUEUE_LENGTH)
                        %PMD_EVENT_QUEUE_LENGTH
                        ].event == pm_event){
            Debug0("found\n");
            return TRUE;
        }
    }

    Mutex_Unlock(&event_queue_mutex);
    
    Debug0("not found\n");

    return FALSE;
}


/*
 * NAME:     check_prior_event
 *
 * FUNCTION: check the event queue to find the event of which priority is
 *           higher than the specified priority.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS   : successfully processed.
 */
int
check_prior_event(int standard_priority)
{
    int i;
    int prior_event_flag_cand = FALSE;

    Debug0("check_prior_event \n");

    /* lock prior event flag */
    Mutex_Lock(&prior_event_flag_mutex);

    prior_event_flag = FALSE;

    /* unlock prior_event_flag */
    Mutex_Unlock(&prior_event_flag_mutex);

    /* lock event_queue*/
    Mutex_Lock(&event_queue_mutex);

    for(i= 0 ; i<PMD_EVENT_QUEUE_LENGTH;i++){
        if (event_queue[i].priority > standard_priority){
          prior_event_flag_cand = TRUE;
          break;
      }
    }

    /* unlock event_queue */
    Mutex_Unlock(&event_queue_mutex);


    /* lock prior event flag */
    Mutex_Lock(&prior_event_flag_mutex);

    prior_event_flag = prior_event_flag_cand;

    /* unlock prior_event_flag */
    Mutex_Unlock(&prior_event_flag_mutex);

    Debug1("prior_event_flag = %d \n",prior_event_flag);

    return PM_SUCCESS;
}

/*
 * NAME:     flush_event_queue
 *
 * FUNCTION: flushes the queue for the events from PM core.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS     : successfully processed.
 */
int
flush_event_queue()
{
    Debug0("flush event queue \n");
    Mutex_Lock(&event_queue_mutex);

    memset((char*)event_queue,0,sizeof(event_queue_t)*PMD_EVENT_QUEUE_LENGTH);
    event_queue_index = 0;

    Mutex_Unlock(&event_queue_mutex);
    return PM_SUCCESS;
}
/*
 * NAME:     check_misc_event
 *
 * FUNCTION: check the other miscellaneous events than for state change.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS     : successfully processed.
 */
int
check_misc_event(int pm_event)
{
    /* lock battery */
    Mutex_Lock(&battery_mutex);	

    switch(pm_event){
    case PM_EVENT_AC:

        /* battery mode is DC */

	battery_mode = PM_AC;
	battery_toggled = TRUE;
	
	set_pmlib_event_entry(PMLIB_EVENT_AC);
	set_pmlib_latest_error(PMLIB_NO_ERROR);

	/* broadcast signal */
        RDebug0(1,"now broadcast signal");
	broadcast_signal(pclientinfo_head);

	break;
    case PM_EVENT_DC:
      
        /* battery mode is DC */

	battery_mode = PM_DC;
	battery_toggled = TRUE;
	set_pmlib_event_entry(PMLIB_EVENT_DC);
	set_pmlib_latest_error(PMLIB_NO_ERROR);

	/* broadcast signal */
        RDebug0(1,"now broadcast signal");
	broadcast_signal(pclientinfo_head);

	break;
    case PM_EVENT_LOW_BATTERY:	
	low_battery = TRUE;
	break;
    }

    /* unlock battery */
    Mutex_Unlock(&battery_mutex);

    return PM_SUCCESS;
}


#ifdef PM_DEBUG2

/*
 * NAME:     pm_system_event_query
 *
 * FUNCTION: system call
 *
 * NOTES:    this is the fake system call for debugging purpose.
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS     : successfully processed.
 */
int 
pm_system_event_query(int *event)
{

        clilen = sizeof(struct sockaddr);
        n= recvfrom(sockfd_ev,
                    mesg,
                    2048,
                    0,
                    (struct sockaddr *)&cli_addr_ev,
                    &clilen);
        
        if(n<0){
            Debug0("event_interface:recvfrom error\n");
            return ;
        }

        *event = atoi(mesg);
  
    return PM_SUCCESS;
}
#endif /* PM_DEUBG2 */

/*
 * NAME:     display_event_queue
 *
 * FUNCTION: display the event queue.
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS     : successfully processed.
 */
int
display_event_queue()
{

int i;

    for(i= 0 ; i<PMD_EVENT_QUEUE_LENGTH;i++){
        RDebug1(1,"queue [%2d]===",i);
        RDebug1(1,"history(%2d)===",
               (event_queue_index-i+PMD_EVENT_QUEUE_LENGTH)
               %PMD_EVENT_QUEUE_LENGTH);
        RDebug1(1,"event [%2d] ===",
                event_queue[i].event);
        RDebug1(1,"priority [%+2d] ===",
                event_queue[i].priority);
        RDebug1(1,"state    [%s] \n",
                pmstate2asc(event_queue[i].state));

    }
    return PM_SUCCESS;

}


/*
 * NAME:     display_event_priority_table
 *
 * FUNCTION: display the event priority table.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS     : successfully processed.
 */
int
display_event_priority_table()
{
    int event;

    for(event=0;event<PMD_TOTAL_EVENT_NUMBER;event++){
        Debug2("event [%+2d] : priority [%+2d]\n",
               EVENT_REVHASH(event),
               event_priority_table[event]);
    }

    return PM_SUCCESS;
}





