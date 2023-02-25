static char sccsid[] = "@(#)07  1.17  src/rspc/usr/bin/pmd/pmd_main.c, pwrcmd, rspc41J, 9524G_all 6/15/95 09:01:48";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS:main,
 *           cleanup_for_main_thread,
 *           check_lockfiles,
 *           pmd_lockfile,
 *           main_initialize,
 *           daemon_initialize,
 *           main_loop,
 *           main_thread_sleep,
 *           process_state_change,
 *           set_pmlib_latest_error,
 *           set_target_state,
 *           set_target_state_by_event,
 *           set_target_state_by_request,
 *           state_change_1st_pass,
 *           state_change_2nd_pass,
 *           examine_hibernation_volume,
 *           file_exists,
 *           exec_shellscript,
 *           process_shutdown,
 *           check_processes,
 *           getdev,
 *           is_tty_process,
 *           kill_syncd,
 *           revive_syncd,
 *           kill_lft_session,
 *           kill_tty_session,
 *           lock_screen,
 *           check_program,
 *           check_utmp,
 *           get_process_data,
 *           run_xlock,
 *           run_LockDisplay,
 *           check_event_entry,
 *           set_completion_notice,
 *           reset_state_change_process_flag,
 *           invoke_query,
 *           set_count_flag,
 *           reset_count_flag,
 *           set_pmlib_event_entry,
 *           process_event_notice,
 *           state2level,
 *           set_event_priority,
 *           state2distance,
 *           set_syscall_busy;
 *           reset_syscall_busy;
 *           create_mutexes,
 *           create_condtion_variables,
 *           create_major_threads,
 *           create_signal_waiter_thread,
 *           check_pmcore_configured.
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

/*-------------------------------*/ 
/* Subroutines for Main Thread   */
/*-------------------------------*/

#include "pmd.h"
#include "pmd_global_var.h"

/*-----------------*/
/* local variables */
/*-----------------*/

/* PM Daemon Lockfiles */

char lockfile_ph1[] = "/etc/pmd_lock";  /* lock file for the pmd of phase1 */
char lockfile_ph2[] = "/etc/pmd_lock2"; /* lock file for the pmd of phase2 */

/* session handling */
int tty_session_number = 0;   /* for checking the existence of tty session */ 
int lft_session_flag = FALSE; /* for checking the existence of lft session */ 
int xserver_flag = FALSE;     /* for checking the existence of xserver */ 
int syncd_flag = FALSE;       /* for checking the existence of sync daemon */

int syncd_killed = FALSE;

pid_t lft_pid = 0;
pid_t *ttys_pid = NULL;
uid_t xserver_uid = 0;
pid_t syncd_pid = 0;
int   syncd_interval = 60;

struct procinfo *mproc = NULL;   /* used for making the process table */
long nproc = 0;                  /* used for count the number of process */

/* processing mode */
int continuous_processing = FALSE;

/* standard priority to check prior event */
int standard_priority=1;

/* signal set */
sigset_t sigset_main;

/*
 * NAME:     main
 *
 * FUNCTION: PM Daemon main routine
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_ERROR : processed unsuccessfully.
 */

int 
main(int argc, char **argv)
{
   Debug1("PM daemon ( pmd ) version %s \n",PMD_VERSION);

   /* Check installation of PM Core */
   if (check_pmcore_configured()!=PM_SUCCESS){
       exit(EXIT_FAILURE);
   }

   /* initialization for misc variables */
   daemon_initialize();

   /* open message catalog */
   
   setlocale( LC_ALL,"");
   catd = catopen( MF_PMD, NL_CAT_LOCALE );

   /* check lock files */

   if (check_lockfiles()!=PM_SUCCESS){
       exit(EXIT_FAILURE);
   }

   /* preparation procedure */

   /* create various attributes for threads */
   create_mutexes();
   create_condition_variables();

   /* initialization for misc variables */
   main_initialize();

   /* check runtime debug mode */
   RDebug0(1,"Run Time Debug Mode Activated.\n");
   /* run time debug mode is available below this line */

   /* reset thread_birth */
   thread_birth = 0;       /* mutex is not necessary */

   /* create threads */
   create_major_threads();

   /* NOTE.1: till here global variables could be set without any lock. */
   /*         since there are only main thread and signal waiter thread.*/
   /* NOTE.2: 2 threads will be created, and birth of each thread will  */
   /*         increment the thread_birth, which will be up to 2.        */
    
   /* necessary to synchronize between threads */
   /* wait till the threads creation is done.  */

   while( thread_birth <2 ){
       sleep(1);
   }

   /* query battery information (initialization call)   */
   /* NOTE: this call is necessary since pmd will send  */
   /*       back previously queried battery information */
   /*       during the state transition to pm library.  */
   query_battery();

   /* send the present (initial) state to pm core */
   if (pmd_parameters.pm_present_state.state != PM_SYSTEM_FULL_ON){
        Debug1("default state of ODM data is %s \n",
               pmstate2asc(pmd_parameters.pm_present_state.state));
        Debug0("now call pm_control_state to change into that state\n");
         
        Mutex_Lock(&pmd_parameters_mutex);
        pm_control_state( PM_CTRL_START_SYSTEM_STATE ,
                         (caddr_t)&pmd_parameters.pm_present_state);
        Mutex_Unlock(&pmd_parameters_mutex);
    }

   /* push cleanup handler */
   pthread_cleanup_push(cleanup_for_main_thread,NULL);


   /* main loop start */
   main_loop();

   /* NOT REACHED */

   /* pop cleanup handler */
   /* it is to be called when cleanup handler is no longer needed.*/
   pthread_cleanup_pop(0);
   
   return PM_ERROR;
}

/*
 * NAME:     cleanup_for_main_thread
 *
 * FUNCTION: cleanup handler for main thread
 *
 * NOTES:    executed on the thread environment (socket_thread).
 * 
 * RETURN VALUE DESCRIPTION:
 *           NONE
 */
void 
cleanup_for_main_thread(void* arg)
{
  Debug0("clean up handler for main thread called.\n");

  /* unlock and unlink lockfiles */  

  /* unlink lockfile for phase1 */
  Debug1("unlink %s\n",lockfile_ph1);
  unlink(lockfile_ph1);

  /* unlink lockfile for phase2 */
  Debug1("unlink %s\n",lockfile_ph2);
  unlink(lockfile_ph2);

  return;
}


/*
 * NAME:     check_lockfiles
 *
 * FUNCTION: check the existence of lockfiles
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : lockfile created successfully.
 *           PM_ERROR   : lockfile exists.
 */
int 
check_lockfiles()
{

   Debug0("check_lockfiles\n");
   switch(pmd_lockfile()){
   case PMD_LOCKFILE_OK:
       return PM_SUCCESS;
       break;
   case PMD_LOCKFILE_PH1:
       /* pmd for phase1 running */
       /* unlink lockfile for phase2  before exit*/
       unlink(lockfile_ph2);
       fprintf(stderr,MSGSTR(ERR_LOCKFILE1,DFLT_ERR_LOCKFILE1),lockfile_ph1);
       break;
   case PMD_LOCKFILE_PH2:
       fprintf(stderr,MSGSTR(ERR_LOCKFILE2,DFLT_ERR_LOCKFILE2),lockfile_ph2);
       break;
   default:
       fprintf(stderr,MSGSTR(ERR_LOCKFILES,DFLT_ERR_LOCKFILES));
       break;
   }
   return PM_ERROR;
}
/*
 * NAME:     pmd_lockfile
 *
 * FUNCTION: check lockfiles for PM Daemon (phase1 and phase2)
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *          PMD_LOCKFILE_OK   : no lockfile had existed and one's been created 
 *          PMD_LOCKFILE_PH1  : lockfile for phase1 already existed.
 *          PMD_LOCKFILE_PH2  : lockfile for phase2 already existed.
 *       
 */
int
pmd_lockfile()
{
    int fd1,fd2;

    Debug0("pmd_lockfile()\n");

    fd2 = open(lockfile_ph2, 
              O_RDWR|O_CREAT,
              0600);

    if(lockf(fd2,F_TLOCK,0) !=0 ){
        Debug0("lockf failed\n");
        return PMD_LOCKFILE_PH2;
    }

    fd1 = open(lockfile_ph1, 
              O_RDWR|O_CREAT,
              0600);

    if(lockf(fd1,F_TLOCK,0) !=0 ){
        Debug0("lockf failed\n");
        return PMD_LOCKFILE_PH1;
    }

    return PMD_LOCKFILE_OK;
}


/*
 * NAME:     main_initialize
 *
 * FUNCTION: initialize routine for main thread
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int
main_initialize()
{
   pm_parameters_t pm_user_parameters;

   Debug0("main_initialize\n");

   /* set signal mask */

   sigemptyset(&sigset_main);
   sigaddset(&sigset_main,SIGCHLD);
   sigthreadmask(SIG_BLOCK,&sigset_main,NULL);

   /* initialize data structure */

   /* reset pm_user_parameters */
   memset(&pm_user_parameters,0,sizeof(pm_user_parameters));

   /* reset pmd_parameters */
   memset(&pmd_parameters,0,sizeof(pmd_parameters_t));

   /* reset pm_target_state */
   pmd_parameters.pm_target_state.state = PM_SYSTEM_ENABLE;

   /* reset pm_present_state */
   pmd_parameters.pm_present_state.state = PM_SYSTEM_ENABLE;

   /* reset the error-causing application's pid and applications' answer */
   appl_pid = 0;
   appl_answer = PMD_UNANIMOUS_OK;

   /* reset the predicates */
   state_change_predicate = FALSE; /* initialize state_change_predicate */
   response_predicate = FALSE;     /* initialize response_predicate */

   /* reset flags for event and request */
   event_flag = PMD_FLAG_NONE;
   request_flag = PMD_FLAG_NONE;

   /* reset entry_flags */
   event_entry   = PM_NONE;
   request_entry = PM_NONE;

   /* reset the event entry to be sent to pm library */
   pmlib_event_entry = PMLIB_EVENT_NONE;

   /* reset initiator flags */
   initiate_by_event   = FALSE;
   initiate_by_request = FALSE;

   /* reset system call bysy flags */
   syscall_busy = FALSE;


   /* reset flags for prior event and successive processing */
   prior_event_flag = FALSE;
   continuous_processing = FALSE;

   /* reset count flags */
   count_confirmation_flag = FALSE;
   count_event_query_flag = FALSE;

   /* reset count variables */
   event_queried_application = 0;
   confirmed_application = 0;   

   /* reset client chain */
   pclientinfo_head = NULL;
   client_number = 0;

   /* reset the latest error to be reported to pm library */

   memset(pmlib_latest_error,0,sizeof(pmlib_state_t));
   pmlib_latest_error.state = pmd_parameters.pm_target_state.state;
   pmlib_latest_error.error = PMLIB_NO_ERROR;
   pmlib_latest_error.pid   = 0;
   memset(pmlib_latest_error.name,0,sizeof(pmlib_latest_error.name));

   /* flush event queue */
   flush_event_queue();

   /* reset event queue index */
   event_queue_index = 0;

   /* reset battery */
   battery_toggled = FALSE;
   battery_mode = PM_AC;
   low_battery = FALSE;

   /* set main_thread id */
   main_thread = pthread_self();

   /* read residual data  & odm_data */
   
   if ( read_odm_data(&pmd_parameters) != PM_SUCCESS ){
       fprintf(stderr,MSGSTR(ERR_ODM_RD_READ,DFLT_ERR_ODM_RD_READ));

       /* unlink lockfile for phase1 */
       Debug1("unlink %s\n",lockfile_ph1);
       unlink(lockfile_ph1);

       /* unlink lockfile for phase2 */
       Debug1("unlink %s\n",lockfile_ph2);
       unlink(lockfile_ph2);

       exit(EXIT_FAILURE);
   }

   /* initialize event_priority_table */
   memset(event_priority_table,0,sizeof(event_priority_table));
   set_event_priority(PM_SYSTEM_ENABLE);

   /* NOTE: standard priority is enable in the initial state.    */
   /* The system state is set to full_on or enable initially,    */
   /* and it is the enable state that full_on state can move to  */
   /* So it is resonable to set_event_priority according to the  */
   /* standard sate, PM_STATE_ENABLE                             */
   /* THIS should be called after read_odm_data(),               */
   /* since the set_event_priority() refers the actions assigned */
   /* to the event.                                              */

   /* set parameters into pm core */
   pmd2userparam(&pmd_parameters,&pm_user_parameters);

   Debug0("now set parameters into pm core\n");

   if( pm_control_parameter(PM_CTRL_SET_PARAMETERS,
                            (caddr_t)&pm_user_parameters) != PM_SUCCESS ){
       fprintf(stderr,MSGSTR(ERR_SET_PARAM,DFLT_ERR_SET_PARAM));
   }

   /* create the hibernation volume if system supports hibernation */

   if ( pmd_parameters.system_support & PM_SYSTEM_HIBERNATION ){
       create_hibernation_volume();
   }
   /* NOTE: this routine will expand the hibernation logical volume      */
   /*       according to the total memory size, if there is one already. */

   /* examine the hibernation volume */
   
   Debug0("now examine the hibernation volume \n");

   if (examine_hibernation_volume() == PM_SUCCESS ){
       Debug0("Hibernation volume is ready\n");

       /* remove the hibernation volume, in case the system does not support 
          transition to hibernation*/
       
       if (!( pmd_parameters.system_support & PM_SYSTEM_HIBERNATION )){
           Debug0("now remove the hibernation volume \n");
           remove_hibernation_volume();
       }
   }
   else{
       Debug0("Hibernation volume is not ready\n");
   }

   /* NOTE : inside examine_hibernation volume() and       */
   /*        remove_hibernation_volume, hibernation volume */
   /*        information is passed to pm core.             */
   /* NOTE2: inside examine_hibernation volume(), if no    */
   /*        hibernation volume is available, it tries to  */
   /*        make one once.                                */

   return PM_SUCCESS;
}

/*
 * NAME:      check_pmcore_configured
 *
 * FUNCTION:  check if PM core has been configured.
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *            PM_SUCCESS : configured available.
 *            PM_ERROR   : not available.
 */
int
check_pmcore_configured()
{
    if (system("/usr/sbin/lsdev -CS1 |grep pmc >/dev/null")!=0){
        fprintf(stderr,MSGSTR(ERR_LSDEV,DFLT_ERR_LSDEV));
        return PM_ERROR;
    }
    Debug0("pm core is available\n");
    return PM_SUCCESS;

}

/*
 * NAME:     daemon_initialize
 *
 * FUNCTION: initialization as daemon process
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int
daemon_initialize()
{
    int fd;
    pid_t pid;
    
    Debug0("daemon initialize \n");

    /* "fork" to make pid different from pgid.           */
    /* this is a prerequirement for the call to setsid() */
    /* to get session leader                             */

    switch( pid = fork() ){
    case 0:
        /* child process ( continue ) */
        break;
    case -1:
        /* fork error */
        fprintf(stderr,MSGSTR(ERR_FORK,DFLT_ERR_FORK));
        exit(EXIT_SUCCESS);
        break;
    default:
        /* parent process */
        sleep(3);
        exit(EXIT_SUCCESS);
        break;
    }

    /* child continues */

    /* create signal_waiter_thread */
    create_signal_waiter_thread();   

    /* NOTE: this signal_waiter_thread is to be created after "fork", */
    /* because forked multi-threaded process has only the thread that */
    /* issued "fork"                                                  */

    /* become session leader */
    setsid();

    /* change current working directory */
    chdir("/");

    /* clear our file mode creation mask */
    umask(0);

    /* close all the file descriptors */

    /* Normally, daemon program has no file descriptor open. */
    /* pm daemon uses stderr instead of syslog.              */
    /* for(fd=0;fd<NOFILE;fd++){                             */
    /*    close(fd);                                         */
    /* }                                                     */

    return PM_SUCCESS;
}

/*
 * NAME:     main_loop
 *
 * FUNCTION: main routine
 *
 * RETURN VALUE DESCRIPTION:
 *           NONE
 */

void
main_loop()
{
   for(;;){
      
      /* sleeping till an event or a request occurs and wakes me up */   
      main_thread_sleep();

      /* set system call busy flag to prevent other system calls from */
      /* causing dead locks.                                          */
      set_syscall_busy();


      /* NOTE:                                                      */
      /* multiple calls to the system calls,                        */
      /*         pm_control_state();                                */
      /*         pm_control_parameter();                            */
      /*         pm_battery_control();                              */
      /* may cause API jam , since these three system calls         */
      /* are using lock internally.                                 */
      /*                                                            */
      /*                    transition on going                     */
      /*                     pm_control_state()                     */
      /* < main >   -------X----o----o-----o----X---------------->  */
      /*                 start  ^    ^     ^    end                 */
      /*                                                            */
      /*                             pm_battery_control()           */
      /* < socket > -----------------o (Locked)                     */
      /*            o (Locked)                                      */
      /*            pmlib_get_event_notice()                        */
      /*                                                            */

      /* processing state transition */
      process_state_change();
      
      /* reset system call busy flag */
      reset_syscall_busy();

      /* sending events to library */
      process_event_notice(); 
  }
  /* NOT REACHED */
}


/*
 * NAME:     main_thread_sleep
 *
 * FUNCTION: blocked till other threads wake up.
 *
 * NOTES:    if continuos_processing flag is TRUE,
 *           main thread will not be blocked and
 *           proceed to the next state change.
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully. ( awakened )
 */
int
main_thread_sleep()
{
    Debug0("main_thread_sleep\n");
    Debug1("==== Present State is : %s \n",
           pmstate2asc(pmd_parameters.pm_present_state.state));

    /* lock condition variable */
    Mutex_Lock(&state_change_cond_mutex);

    if (continuous_processing){

       Debug0("*main thread : continuous processing detected TRUE.\n"); 
       Debug0("         due to prior_event_flag or lid open/close.\n");

       /* if prior_event_flag is true, main thread will not sleep,
          to process the prior event in the next stage */
       
       /* lock prior event flag */
       Mutex_Lock(&prior_event_flag_mutex);

       prior_event_flag = FALSE;

       /* lock prior event flag */
       Mutex_Unlock(&prior_event_flag_mutex);

       /* reset the continuous processing flag for the next stage */
       continuous_processing = FALSE;
    }
    else{

        /* now sleeping */
        while(state_change_predicate==FALSE){
            Cond_Wait(&state_change_condvar,&state_change_cond_mutex);
        }

   }
    
    /* unlock condition variable */
    Mutex_Unlock(&state_change_cond_mutex);

    Debug0("main thread awaken\n");

    /* lock condition variable */
    Mutex_Lock(&state_change_cond_mutex);  

    state_change_predicate=FALSE;
    
    /* unlock condition variable */
    Mutex_Unlock(&state_change_cond_mutex);
    
    return PM_SUCCESS;
}

/*
 * NAME:     process_state_change
 *
 * FUNCTION: processes the change of the system state.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
process_state_change()
{
    int rc=PM_ERROR;
    pm_system_state_t transition;
    event_queue_t lid_event;

    Debug0("process_state_change\n");

    /* initialize data structures */
    memset(&transition,0,sizeof(transition));
    memset(&lid_event,0,sizeof(lid_event));

    /* reset target state & latest error before state change start */
    
    /* lock pmd_parameters */
    Mutex_Lock(&pmd_parameters_mutex);

    memset(&pmd_parameters.pm_target_state,0,sizeof(pm_system_state_t));

    /* unlock pmd_parameters */
    Mutex_Unlock(&pmd_parameters_mutex);

    /* set error-caused application pid to zero */

    /* lock application_answer */
    Mutex_Lock(&application_answer_mutex);

    appl_answer = PMD_UNANIMOUS_OK;
    appl_pid    = 0; /* default value */
    
    /* unlock application_answer */
    Mutex_Unlock(&application_answer_mutex);
    
    /* NOTE:                                   */
    /*      Following is the previous design.  */
    /*      The latest occured error should be */
    /*      reported to pm library. So we need */
    /*      not reset the latest_error here.   */
    /*-----------------------------------------*/
    /* set_pmlib_latest_error(PMLIB_NO_ERROR); */
    /*-----------------------------------------*/

    /* start trials to change state */

    /* first set the target state of transition */
    if (set_target_state() == PM_SUCCESS){
        
        /* set the standard priority to detect the more prior events */
        standard_priority = 
            state2distance(pmd_parameters.pm_target_state.state,
                           pmd_parameters.pm_present_state.state)+1;

        Debug1("set standard_priority to %d \n",standard_priority);

        /* set event priority table */
        set_event_priority(pmd_parameters.pm_present_state.state);

        /* flush the event queue */
        flush_event_queue();

        /* Tell pm core that the new transition has started.              */
        /* If target is shutdown, pm core need not know it.               */
        /* In this case, the suspend LED will not start to blink,         */
        /* otherwise, its blinking means the beginning of transition.     */

        if (pmd_parameters.pm_target_state.state != PM_SYSTEM_SHUTDOWN){
            /* notice of transition start to pm core */ 

            Debug0("now call TRANSITION_START\n");

            transition.state = PM_TRANSITION_START;
            pm_control_state(PM_CTRL_START_SYSTEM_STATE, 
                             (caddr_t)&transition);
        }

        /* 1st pass trial */
        if (state_change_1st_pass() == PMD_PASSED ){
            Debug0("1st pass ok \n");

            /* NOTE: Following is the previous design.               */
            /* For the present library, pmd need not wait any more.  */
            /* ------------------------------------------------------*/
            /* sleep for a while.                                    */
            /* lest the signal to notice the result of confirmations */
            /* should interrupt the return socket,                   */
            /* otherwise API calls should fail with errno 4.         */
            /* sleep(1);                                             */
            /* ------------------------------------------------------*/

            if (state_change_2nd_pass() == PMD_PASSED ){
                Debug0("2nd pass ok \n");

                /* NOTE:                                             */
                /* In case of transition to SUSPEND and HIBERNATION, */
                /* after completion of system call, the event_queue  */
                /* will be flushed, inside state_change_2nd_pass().  */
                /* This is for that the return from this routine     */
                /* with success means RESUME from those power saving */
                /* state. In resume, the queue should be cleaned up. */

                /* set the standard priority to detect the prior event */
                standard_priority = 
                    state2distance(pmd_parameters.pm_target_state.state,
                                   pmd_parameters.pm_present_state.state)+1;

                Debug1("set standard_priority to %d \n",standard_priority);

                /* set the present state, after successful completion */
                /* of the state transition.                           */

                /* lock pmd_parameters */
                Mutex_Lock(&pmd_parameters_mutex);

                pmd_parameters.pm_present_state = 
                    pmd_parameters.pm_target_state ;

                /* unlock pmd_parameters */
                Mutex_Unlock(&pmd_parameters_mutex);

                /* set event priority table */
                set_event_priority(pmd_parameters.pm_present_state.state);

                rc = PM_SUCCESS;
            }
            else{
                Debug0("2nd pass fail.\n");
                rc = PM_ERROR;
            }

            /* wait for the query from the applications */
            invoke_query(PMD_INVOKE_EVENT_QUERY);

        }
        else{
            Debug0("1st pass fail.\n");
            rc = PM_ERROR;
        }   

        /* As the same processing as of TRANSITION_START */
        if (pmd_parameters.pm_target_state.state != PM_SYSTEM_SHUTDOWN){
            /* notice of transition complete to pm core */
            transition.state = PM_TRANSITION_END;
            pm_control_state(PM_CTRL_START_SYSTEM_STATE,
                             (caddr_t)&transition);
        }         

        /* lock pmd_parameters */
        Mutex_Lock(&pmd_parameters_mutex);
    
        pmd_parameters.pm_target_state = pmd_parameters.pm_present_state ;
    
        /* unlock pmd_parameters */
        Mutex_Unlock(&pmd_parameters_mutex);

        /* First, check the events for LID open and close      */
        /* to ensure that lid works correctly as a trigger for */
        /* state transition between ENABLE and STANDBY,        */
        /* if lid_close_action is STANDBY.                     */

        if (  (pmd_parameters.lid_close_action == PM_SYSTEM_STANDBY)&&
              (check_lid_event(&lid_event)==PM_SUCCESS)&&
              (
               (pmd_parameters.pm_present_state.state == PM_SYSTEM_STANDBY &&
                lid_event.state == PM_SYSTEM_ENABLE) ||
               (pmd_parameters.pm_present_state.state == PM_SYSTEM_ENABLE &&
                lid_event.state == PM_SYSTEM_STANDBY) 
              )
            ){

            Debug1("lid_event.state = %s \n",pmstate2asc(lid_event.state));

            /* lock request flag */
            Mutex_Lock(&request_flag_mutex);
        
            request_flag=PMD_FLAG_NONE;

            /* unlock request flag */
            Mutex_Unlock(&request_flag_mutex);

            /* lock event flag */
            Mutex_Lock(&event_flag_mutex);
            
            /* if the present event queue has the event prior to */
            /* lid event, set event_flag to that state           */
            /* (NOTE: event_flag should not be the event ,but    */
            /*  the STATE assigned to that event.)               */

            if ( state2distance(event_entry,lid_event.state) >0){
                /* if prior event is more prior than STANDBY */
                event_flag=event_entry;
            }
            else{
                /* set event_flag to STANDBY or ENABLE */
                event_flag=lid_event.state;
            }
            /* unlock event flag */
            Mutex_Unlock(&event_flag_mutex);

            /* set the continuous precessing flag , since there is */
            /* prior event to be processed.                        */

            continuous_processing = TRUE;

        } /* end of if */
        else{
            if (!prior_event_flag){
                /* if there is no prior event, flush queue and reset flags */

                flush_event_queue();
            
                reset_state_change_process_flag();
                continuous_processing = FALSE;

            }/* end of if */ 
            else{
                /* if there is any prior event, wo'nt flush queue and then */
                /* set event_flag to event_entry and reset request_flag.   */
                /* (event_entry is target state of the most prior event.   */
                /* It is ensured by the prior_event_flag that at least     */
                /* one prior event exists. )                               */
                /* then, proceeds to next stage, main thread won't sleep.  */

                /* lock event flag */
                Mutex_Lock(&event_flag_mutex);
        
                event_flag=event_entry;

                /* unlock event flag */
                Mutex_Unlock(&event_flag_mutex);
        
                /* lock request flag */
                Mutex_Lock(&request_flag_mutex);
        
                request_flag=PMD_FLAG_NONE;

                /* unlock request flag */
                Mutex_Unlock(&request_flag_mutex);

                /* set the continuous precessing flag , since there is */
                /* prior event to be processed.                        */

                continuous_processing = TRUE;

            } /* end of else*/       
        }/* end of else*/       
    }/* end of if */
    else{
        /* target state is not correct , or set to PM_NONE */
        /* flush the event queue and the reset the flags   */

        flush_event_queue();

        reset_state_change_process_flag();
        continuous_processing = FALSE;

        rc = PM_ERROR;

    }

    /* NOTE:      now pmd will sleep for a while */
    /* to avoid failure to get signal and ensure */
    /* that the socket returning to the pm aware */
    /* applications be reached successfully.     */
    sleep(1);

    return rc;
}


/*
 * NAME:     set_pmlib_latest_error
 *
 * FUNCTION: set the latest error value to be sent to the library.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 */

int 
set_pmlib_latest_error(int cause)
{
    Debug1("set pmlib_latest_error ( cause:%x )\n",cause);

    /* lock pmlib_latest_error */
    Mutex_Lock(&pmlib_latest_error_mutex);

    pmlib_latest_error.state = pmd_parameters.pm_target_state.state;
    pmlib_latest_error.error = cause;

    pmlib_latest_error.pid   = appl_pid;
    strcpy(pmlib_latest_error.name,pmd_parameters.pm_target_state.name);

    /* lock pmlib_latest_error */
    Mutex_Unlock(&pmlib_latest_error_mutex);

    return PM_SUCCESS;

}

/*
 * NAME:     set_target_state
 *
 * FUNCTION: set the target state. 
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *        INPUT:   none
 *        OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : invalid arguments specified.
 */
int 
set_target_state()
{
    int lc_event_flag;   /* true if event occurred */
    int lc_request_flag; /* request if event occurred */
                         /* to ensure that branch condition be atomic */

    Debug0("set_target_state \n");

    /* lock event flag*/
    Mutex_Lock(&event_flag_mutex);

    /* lock request flag */
    Mutex_Lock(&request_flag_mutex);

    /* check if event occurred */

    lc_event_flag =((event_flag != PMD_FLAG_NONE) && 
                    (event_flag != PMD_FLAG_BUSY));

    /* NOTE: Following line has been discarded to ensured that the event */
    /* has the priority to the request                                   */
    /*    && (request_flag==PMD_FLAG_NONE));                             */

    /* check if request occurred */

    lc_request_flag = ((request_flag != PMD_FLAG_NONE ) && 
                       (request_flag != PMD_FLAG_BUSY ) &&
                       (event_flag==PMD_FLAG_NONE));
    
    /* unlock request flag */
    Mutex_Unlock(&request_flag_mutex);
    
    /* unlock event */
    Mutex_Unlock(&event_flag_mutex);
    
    Debug1("lc_event_flag =%d \n",lc_event_flag);
    Debug1("lc_request_flag =%d \n",lc_request_flag);

    /* set initiation flags */
    initiate_by_request = FALSE;
    initiate_by_event = FALSE;

    if (lc_event_flag){
        if (set_target_state_by_event() != PM_SUCCESS){
             return PM_ERROR;
        } 
        /* set initiation flags */
        initiate_by_event = TRUE;
        initiate_by_request = FALSE;
    }
    else{
        if (lc_request_flag){
            if (set_target_state_by_request() != PM_SUCCESS ){
                return PM_ERROR;
            }
            /* set initiation flags */
            initiate_by_request = TRUE;
            initiate_by_event = FALSE;
        }
        else{
            /* impossible state */
            /* main thread has been awakened by illegal trigger */
            return PM_ERROR;
        }
    }
    return PM_SUCCESS;
}


/*
 * NAME:     set_target_state_by_event
 *
 * FUNCTION: set the target state by the event from PM core.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
set_target_state_by_event()
{

    Debug0("set_target_state_by_event\n");

    /* set the target state of transition by event_flag */

    /* lock pmd_parameters*/
    Mutex_Lock(&pmd_parameters_mutex);

    pmd_parameters.pm_target_state.state=event_flag;

    /* unlock pmd_parameters*/
    Mutex_Unlock(&pmd_parameters_mutex);

    /* lock the event flag to reject the next event during processing */
    /* the current event.                                             */

    /* lock event flag*/
    Mutex_Lock(&event_flag_mutex);

    event_flag=PMD_FLAG_BUSY;

    /* unlock event flag*/
    Mutex_Unlock(&event_flag_mutex);
    
    Debug1("target state by event :%s \n",
           pmstate2asc(pmd_parameters.pm_target_state.state));

    /* if target state is not proper , return PM_ERROR */

    if ( pmd_parameters.pm_target_state.state == PM_NONE ||
         pmd_parameters.pm_target_state.state == 
               pmd_parameters.pm_present_state.state
        ){
        Debug0("failed\n");

        return PM_ERROR;
    }

    return PM_SUCCESS;
}

/*
 * NAME:     set_target_state_by_request
 *
 * FUNCTION: set pm_target_state by request.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 */
int 
set_target_state_by_request()
{
    Debug0("set_target_state_by_request\n");
    
    /* set the target state of transition by request_flag */

    Mutex_Lock(&pmd_parameters_mutex);

    pmd_parameters.pm_target_state.state = request_flag;

    Mutex_Unlock(&pmd_parameters_mutex);

    /* lock the event flag to reject the next event during processing */
    /* the current event.                                             */

    /* lock request flag */
    Mutex_Lock(&request_flag_mutex);

    request_flag=PMD_FLAG_BUSY;
    
    /* unlock request flag */
    Mutex_Unlock(&request_flag_mutex);
    
    Debug1("request state :%s \n",
           pmstate2asc(pmd_parameters.pm_target_state.state));

    return PM_SUCCESS;
}

/*
 * NAME:     state_change_1st_pass
 *
 * FUNCTION: 1st trial of state change.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PMD_PASS      : processed successfully.
 *           PMD_BLOCKED   : processed unsuccessfully.
 */
int 
state_change_1st_pass()
{
    int rc = PMD_BLOCKED;

    Debug0("state change 1st_pass \n");

    switch(pmd_parameters.pm_target_state.state){
    case PM_SYSTEM_FULL_ON:
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_FULL_ON);
        break;
    case PM_SYSTEM_ENABLE:
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_ENABLE);
        break;
    case PM_SYSTEM_STANDBY:
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_STANDBY);
        break;
    case PM_SYSTEM_SUSPEND:
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_SUSPEND);
        break;
    case PM_SYSTEM_HIBERNATION:
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_HIBERNATION);
        break;
    case PM_SYSTEM_SHUTDOWN: 
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_SHUTDOWN);
        break;
    case PM_SYSTEM_TERMINATE: 
        /* power management termination by API call */ 
        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_TO_TERMINATE);
        break;
    default:
        /* error handling */
        break;
    }/* end of switch*/

    /* wait for all the application to confirm */
    invoke_query(PMD_INVOKE_CONFIRMATION);

    /* inside invoke_query(),                     */
    /* appl_answer is reset to PMD_UNANIMOUS_OK,  */
    /* and appl_pid to 0.                         */

    switch( appl_answer ){
    case PMD_UNANIMOUS_OK:

        /* all the application accepted the transition unanimously */

        Debug0("appl_answer: unanimous_ok \n");
        set_pmlib_event_entry(PMLIB_EVENT_START_TO_CHANGE_STATE);
        set_pmlib_latest_error(PMLIB_NO_ERROR);
        rc = PMD_PASSED;
        break;

    case PMD_PARTIAL_OK:

        /* some applications accepted the transition but the others */
        /* sent no answer, and timeout occurred.                    */

        Debug0("appl_answer: partial_ok \n");
        set_pmlib_event_entry(PMLIB_EVENT_FORCE_TO_CHANGE_STATE);
        set_pmlib_latest_error(PMLIB_NO_ERROR);
        rc = PMD_PASSED;
        break;

    case PMD_REJECT:

        /* at least one application rejected the transition */

        Debug0("appl_answer: reject \n");

        /* if optional battery is low and system is going to sleep, */
        /* or system is going to terminate, system will go ahead    */
        /* instead of the applications answer.                      */

        if ((search_event_queue( PM_EVENT_LOW_BATTERY )&&
             pmd_parameters.pm_target_state.state == PM_SYSTEM_HIBERNATION)||
            (pmd_parameters.pm_target_state.state == PM_SYSTEM_TERMINATE)
           ){
            Debug0("but low battery ...cannot help processing. \n");
            set_pmlib_event_entry(PMLIB_EVENT_FORCE_TO_CHANGE_STATE);
            set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_APPL);
            rc = PMD_PASSED;
        }/* end of if */
        else{
            set_pmlib_event_entry(PMLIB_EVENT_NOTICE_OF_REJECTION);
            set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_APPL);
            rc = PMD_BLOCKED;
        }/* end of else */
    }/* end of switch */
    
    /* check the prior_event_flag */

    check_prior_event(standard_priority);

    if (prior_event_flag){
        Debug0("prior_event_flag detected\n"); 

        set_pmlib_event_entry(PMLIB_EVENT_NOTICE_OF_REJECTION);
        set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_EVENT);
        rc = PMD_BLOCKED;
    }

    /* NOTE: Following is the previous design.               */
    /* For the present library, pmd need not wait any more.  */
    /* ------------------------------------------------------*/
    /* sleep for a while.                                    */
    /* lest the signal to notice the result of confirmations */
    /* should interrupt the return socket,                   */
    /* otherwise API calls should fail with errno 4.         */
    /* sleep(1);                                             */
    /* ------------------------------------------------------*/

    /* wait for the query from the applications */
    invoke_query(PMD_INVOKE_EVENT_QUERY);
    
    return rc;
}

/*
 * NAME:     state_change_2nd_pass
 *
 * FUNCTION: 2nd trial of state change.
 *
 * NOTES:    
 * 
 * RETURN VALUE DESCRIPTION:
 *           PMD_PASS      : processed successfully.
 *           PMD_BLOCKED   : processed unsuccessfully.
 */
int 
state_change_2nd_pass()
{
    time_t retry_time;
    time_t start_time;
    int rc = PMD_BLOCKED;  /* return code */
    int rc_sc = PM_ERROR;  /* return code of system call */
    int retry_period;
    int retry_interval;
    pm_system_state_t pm_target_state;
    pm_parameters_t pm_user_parameters;

    Debug0("state change 2nd_pass \n");
    
    /* initialize data structures */
    memset(&pm_user_parameters,0,sizeof(pm_user_parameters));
    memset(&pm_target_state,0,sizeof(pm_target_state));

    /* if system is going to terminate (pm core will be unconfigured) */
        
    if (pmd_parameters.pm_target_state.state == PM_SYSTEM_TERMINATE){
        process_terminate(PMD_TERMINATE_WITHOUT_RMDEV);
        /* NOT REACHED */
    }

    /* check the process and set lft & tty session_flag     */
    /*                           xserver_flag               */
    /*                           set syncd_flag             */
    if ( check_processes() != PM_ERROR ){

        /* if system is going to sleep in the suspend or   */
        /* hibernation state, xlock, lft & tty termination */
        /* will be operated.                               */

        if(pmd_parameters.pm_target_state.state == 
           PM_SYSTEM_SUSPEND ||
           pmd_parameters.pm_target_state.state == 
           PM_SYSTEM_HIBERNATION){
            
            /* invoke xlock */
            if (xserver_flag && 
                pmd_parameters.resume_passwd ){
        
                /* locking the screen */
                lock_screen();

                /* wait for a while to make sure */
                /* the xlock has come up.        */
                sleep(7);
            }
            
            /* kill lft session */
            if (lft_session_flag &&
                pmd_parameters.kill_lft_session ){
                kill_lft_session();
            }
            /* kill tty session */
            if (tty_session_number && 
                pmd_parameters.kill_tty_session){
                kill_tty_session();
            }
        }        

        /* NOTE : To make sure the xlock, lft & tty termination       */
        /*        will be done as soon as possible, and there should  */
        /*        be no security hole, xlock, lft & tty termination   */
        /*        will be operated before entering sleeping state.    */
        /*        Drawback is that we cannot recover the lft & tty    */
        /*        sessions nor unlock xlock when the state transition */
        /*        has been rejected on some reason.                   */

        /* kill sync daemon in case of transition to standby */

        if (pmd_parameters.pm_target_state.state == PM_SYSTEM_STANDBY ){

          /* set syncd killed flag (= is correct, not to be replaced by ==) */
            if (syncd_killed = (pmd_parameters.kill_syncd && syncd_flag)){
                kill_syncd();
            }

            /* send the flag if syncd killed or not to pm core */

            pmd2userparam(&pmd_parameters,&pm_user_parameters);
            pm_user_parameters.core_data.kill_syncd = syncd_killed;

            /* NOTE:                                                     */
            /* pmd_parameters.kill_syncd is if syncd is to be terminated */
            /* pm_user_parameters.kill_syncd is if syncd has been killed */

            Debug0("now set the flag if syncd killed or not to pm core\n");

            if( pm_control_parameter(PM_CTRL_SET_PARAMETERS,
                             (caddr_t)&pm_user_parameters) != PM_SUCCESS ){
                fprintf(stderr ,MSGSTR(ERR_SET_PARAM,DFLT_ERR_SET_PARAM));
            }   
        }

        /* if system is going to shutdown */

        if (pmd_parameters.pm_target_state.state == PM_SYSTEM_SHUTDOWN){
            process_shutdown();

            /* not reached in successful case */
            set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
            set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_SYSTEM); 
            rc = PMD_BLOCKED;

        }
        else if ((pmd_parameters.pm_target_state.state == 
                  PM_SYSTEM_HIBERNATION) &&
                 (examine_hibernation_volume() != PM_SUCCESS) 
                 ){
        
            /* if going to hibernation, examine hibernation volume and  */
            /* set the hibernation volume information to pm core.       */

            /* NOTE: if target is suspend, by critical low battery and  */
            /* duration timer, the system might lead to hibernation.    */
            /* So, hibernation volume information is necessary.         */
            /* this is implemented at the beginning of retry sequence   */
            /* of transition requests. ( See below )                    */
            
            Debug0("Hibernation volume is not ready\n");
            set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
            set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_HIB_VOL);
            rc = PMD_BLOCKED;
        }
        else if ((pmd_parameters.pm_target_state.state==PM_SYSTEM_SUSPEND ||
                  pmd_parameters.pm_target_state.state==PM_SYSTEM_HIBERNATION)
                 &&(exec_shellscript(PMD_SLEEP)!=PM_SUCCESS)){

            /* in case of hibernation or suspend, execute shell script */
           
            Debug0("shell script is wrong\n");
            set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
            set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_SYSTEM);
            rc = PMD_BLOCKED;
        }
        else{

            if (pmd_parameters.pm_target_state.state == PM_SYSTEM_SUSPEND){

                /* NOTE: if target is suspend, by critical low battery and  */
                /* duration timer, the system might lead to hibernation.    */
                /* So, hibernation volume information is necessary.         */

                examine_hibernation_volume();

                /* the difference between transition to suspend and */
                /* hibernation from the view point of examining the */
                /* hibernation volume, the latter case should be    */
                /* rejected if hibernation volume is not availble,  */
                /* but the former does not care the availability.   */

            }       

            /* start retry sequence to change state */
            start_time = time(NULL);
                
            do{
                Debug0("retry sequence... \n");
            
                if (initiate_by_event && low_battery 
                    && battery_mode == PM_AC && 
                  (pmd_parameters.pm_target_state.state == PM_SYSTEM_SUSPEND ||
                 pmd_parameters.pm_target_state.state == PM_SYSTEM_HIBERNATION)
                    ){
                
                    Debug0("low battery & ac and target is sleep by event\n");
                    set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
                    set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_SYSTEM);
                        
                    /* reset low battery flag */
                
                    /* lock battery */
                    Mutex_Lock(&battery_mutex);
                        
                    low_battery = FALSE;
                
                    /* unlock battery */
                    Mutex_Unlock(&battery_mutex);
                
                    rc = PMD_BLOCKED;
                    break;
                }
            
                /* check prior event flag */
            
                check_prior_event(standard_priority);
            
                if (prior_event_flag){
                
                    /* if there is prior event, stop the current */
                    /* state transition                          */

                    Debug0("prior_event_flag detected\n"); 
                
                    set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
                    set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_EVENT);
                    rc = PMD_BLOCKED;
                
                    break;
                }       
                    
                /* execute system call for state transition */
            
                pm_target_state = pmd_parameters.pm_target_state;
            
                RDebug0(1,"pmd: now call pm_control_state()\n");

                if ( pm_control_state(PM_CTRL_START_SYSTEM_STATE,
                                  (caddr_t)&pm_target_state ) 
                    != PM_SUCCESS){
                
                    /* if the argument is invalid, */
                    /* system call fails.          */

                    Debug0("system call failed \n");

                    set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
                    set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_SYSTEM);
                    rc = PMD_BLOCKED;
                
                    break;
                }

                RDebug0(1,"pmd: now exit from pm_control_state()\n");
            
                Mutex_Lock(&pmd_parameters_mutex);
                pmd_parameters.pm_target_state = pm_target_state;
                Mutex_Unlock(&pmd_parameters_mutex);
                    
                Debug0(" ===Main thread:now processing state change \n");

                /* if target is hibernation,                         */
                /* PM core re-examine the hibernation volume         */
                /* and if it is not ready, set pm_target_state.event */
                /* to PM_EVENT_REJECT_BY_HIB_VOL inside system-call. */
                /* in this case, retry sequence should be terminated */

                if ( pmd_parameters.pm_target_state.state ==
                    PM_SYSTEM_HIBERNATION &&
                    pmd_parameters.pm_target_state.event == 
                    PM_EVENT_REJECT_BY_HIB_VOL){
                
                    Debug0("system call failed due to hib_vol not ready\n");
                    set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
                    set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_HIB_VOL);
                
                    rc = PMD_BLOCKED;
                    break;
                }

                /* if target is hibernation and suspend,             */
                /* PM core examine if there are phase1 DD configured */
                /* and if there are, it sets pm_target_state.event   */
                /* to PM_EVENT_PHASE1_DD inside system-call.         */
                /* in this case, retry sequence should be terminated */

                if ( ( ( pmd_parameters.pm_target_state.state == 
                        PM_SYSTEM_HIBERNATION)||
                      ( pmd_parameters.pm_target_state.state == 
                       PM_SYSTEM_SUSPEND)
                      ) &&
                    ( pmd_parameters.pm_target_state.event == 
                     PM_EVENT_PHASE1_DD )
                    ){
                
                    Debug0("system call failed due to hib_vol not ready\n");
                    set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
                    set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_SYSTEM);
                
                    rc = PMD_BLOCKED;
                    break;
                }
                    
                if ( pmd_parameters.pm_target_state.event != 
                    PM_EVENT_REJECT_BY_DD ){
                
                    /* system call succeeded */
                
                    /*----------------*/
                    /* in resume case */
                    /*----------------*/

                    /* resume from standby */
                    if (pmd_parameters.pm_target_state.state == 
                        PM_SYSTEM_ENABLE && 
                        pmd_parameters.pm_present_state.state == 
                        PM_SYSTEM_STANDBY){
                        Debug0("resume from standby\n");
                    
                        /* in case of resume from standby */
                    
                        /* set_pmlib_event_entry */
                        set_completion_notice();
                        set_pmlib_latest_error(PMLIB_NO_ERROR);

                        /* revives sync daemon */
                        if (syncd_killed){           
                            revive_syncd();
                        }
                    
                        rc = PMD_PASSED;
                        break;
                    }
                
                    /* resume from suspend and hibernation */
                    if(pmd_parameters.pm_target_state.state == 
                       PM_SYSTEM_SUSPEND ||
                       pmd_parameters.pm_target_state.state == 
                       PM_SYSTEM_HIBERNATION){

                        /* execute shell script on resume */
                        exec_shellscript(PMD_RESUME);
                    
                        /* flush event queue */
                            
                        flush_event_queue();

                        /* In resume from the state in which the CPU */
                        /* halts, event queue should be cleaned up.  */
                    }   
                
                    set_completion_notice(); /* set_pmlib_event_entry */
                    set_pmlib_latest_error(PMLIB_NO_ERROR);
                
                    rc = PMD_PASSED;
                    break;
                }
            
                /* continue to re-try to change system state,  */
                /* if system-call failed with REJECT_DD error. */
                        
                set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
                set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_DEVICE);
                retry_time+=PMD_RETRY_INTERVAL;
                rc = PMD_BLOCKED;

                retry_time = time(NULL);

            }while( retry_time < start_time + PMD_RETRY_PERIOD );
        }
    }/* end of if check_process() != error */
    else{
        /* check_process() failed due to the memory fault */
        set_pmlib_event_entry(PMLIB_EVENT_FAIL_TO_CHANGE_STATE);
        set_pmlib_latest_error(PMLIB_ERROR_REJECT_BY_SYSTEM);
        rc = PMD_BLOCKED;
    }

    /* 2nd pass completed */

    /* cleanup_for_check_processes */
    cleanup_for_check_processes();

    return rc;
}

/*
 * NAME:     create_hibernation_volume
 *
 * FUNCTION: create the hibernation logical volume
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : processed unsuccessfully.
 */
int
create_hibernation_volume()
{
    Debug0("create_hibernation_volume\n");

    if (system("/usr/bin/mkpmhlv -m ") !=0){
        return PM_ERROR;;
    }
    return PM_SUCCESS;
}
/*
 * NAME:     examine_hibernation_volume
 *
 * FUNCTION: if target is hibernation , examine hibernation volume 
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 */
int
examine_hibernation_volume()
{
    int hib_vol_rc = 0;
    int rc = PM_ERROR;
    char lv_name[] = PMD_HIB_VOL_NAME;
    pm_hibernation_t hib_vol_info; 
    char cmd_buf[64];
          
    Debug0("examine_hibernation_volume\n");

    /* initialize data structures */
    memset(cmd_buf,0,sizeof(cmd_buf));
    memset(&hib_vol_info,0,sizeof(hib_vol_info));

    sprintf(cmd_buf,"/usr/sbin/rmlv -f %s",lv_name);

    if ( ( hib_vol_rc = getpmlv(lv_name,&hib_vol_info) ) != 0 ){

        /* try to make another hibernation volume */
        Debug0("no hibernation_volume available\n");
        Debug0("try to make it once\n");
        
        /* first deletes the wrong one ( if any ) */
        if ( errno != ENOENT ){
            /* getpmlv sets errno to ENOENT if there is no hibernation */
            Debug0("now delete the wrong hibernation volume if any\n");
            system(cmd_buf);                
        }
        /* free the memory for sector list */
        Debug0("Free allocated memory\n");
        Free(hib_vol_info.slist);

        /* if system supports hibernation try to make another */
        if ( pmd_parameters.system_support & PM_SYSTEM_HIBERNATION ){

            /* then make another hibernation volume.  */
            if ( system("/usr/bin/mkpmhlv -m") == 0 ){

                Debug0("mkpmhlv succeeded.\n");

                hib_vol_rc = getpmlv(lv_name,&hib_vol_info);

                Debug1("hib_vol_rc = %d\n",hib_vol_rc);
            }
            else{
                Debug0("mkpmhlv failed.\n");
            }
        }
    }
    Debug1("hib_vol_rc = %d\n",hib_vol_rc);
    Debug0("check again\n");

    /* check again the hibernation volume */
    /* and send the information about hibernation to pm core  */
    if ( hib_vol_rc !=0 ){
        fprintf(stderr,MSGSTR(ERR_HIB_VOL_SEARCH,DFLT_ERR_HIB_VOL_SEARCH));
        rc = PM_ERROR;

        /* clean up the data structure */
        memset(&hib_vol_info,0,sizeof(pm_hibernation_t));

        if (pm_control_parameter(PM_CTRL_SET_HIBERNATION_VOLUME,
                                 (caddr_t)NULL) != PM_SUCCESS ){
            fprintf(stderr,MSGSTR(ERR_HIB_VOL_INFO,DFLT_ERR_HIB_VOL_INFO));
            rc = PM_ERROR;
        }
    }
    else{
        Debug0("hibernation volume available\n");
        rc = PM_SUCCESS;
        if (pm_control_parameter(PM_CTRL_SET_HIBERNATION_VOLUME,
                                 (caddr_t)&hib_vol_info) != PM_SUCCESS ){
            fprintf(stderr,MSGSTR(ERR_HIB_VOL_INFO,DFLT_ERR_HIB_VOL_INFO));
            rc = PM_ERROR;
        }
    }

    Debug0("Free allocated memory\n");
    /* free the memory for sector list */
    Free(hib_vol_info.slist);
    

    return rc;
}
/*
 * NAME:     remove_hibernation_volume
 *
 * FUNCTION: remove the hibernation volume ,
 *           if the machine does not support the transition
 *           to hibernation.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 */
int
remove_hibernation_volume()
{
    int hib_vol_rc = 0;
    int rc = PM_ERROR;
    char lv_name[] = PMD_HIB_VOL_NAME;
    pm_hibernation_t hib_vol_info; 
    char cmd_buf[32];
          
    Debug0("remove hibernation volume\n");

    /* initialize data structures */
    memset(cmd_buf,0,sizeof(cmd_buf));
    memset(&hib_vol_info,0,sizeof(hib_vol_info));

    sprintf(cmd_buf,"/usr/sbin/rmlv -f %s",lv_name);
   
    if ( ( hib_vol_rc = getpmlv(lv_name,&hib_vol_info) ) == 0 ){
        
        Debug0("hibernation volume exists\n");

        /* try to remove hibernation volume */
        Debug0("try to remove hibernation volume\n");        

        if ( system(cmd_buf) != 0 ){
            fprintf(stderr,MSGSTR(ERR_HIB_VOL_REMOVE,DFLT_ERR_HIB_VOL_REMOVE));
            rc = PM_ERROR;
        }
        else{
            rc = PM_SUCCESS;
            Debug0("hibernation volume removed \n");
        }
    }
    else{
        fprintf(stderr,MSGSTR(ERR_HIB_VOL_SEARCH,DFLT_ERR_HIB_VOL_SEARCH));
        rc = PM_ERROR;
    }

    /* free the memory for sector list */
    Debug0("Free allocated memory\n");
    Free(hib_vol_info.slist);

    /* get hibernation volume info again */

    if (( hib_vol_rc = getpmlv(lv_name,&hib_vol_info) ) != 0){
        Debug0("hibernation volume is not available\n");

        /* clean up the data structure */
        memset(&hib_vol_info,0,sizeof(pm_hibernation_t));

        if (pm_control_parameter(PM_CTRL_SET_HIBERNATION_VOLUME,
                                 (caddr_t)NULL) != PM_SUCCESS ){
            fprintf(stderr,MSGSTR(ERR_HIB_VOL_INFO,DFLT_ERR_HIB_VOL_INFO));
            rc =  PM_ERROR;
        }
    }
    else{
        Debug0("hibernation volume is still available\n");

        if (pm_control_parameter(PM_CTRL_SET_HIBERNATION_VOLUME,
                                 (caddr_t)&hib_vol_info) != PM_SUCCESS ){
            fprintf(stderr,MSGSTR(ERR_HIB_VOL_INFO,DFLT_ERR_HIB_VOL_INFO));
            rc =  PM_ERROR;
        }
    }

    Debug0("Free allocated memory\n");
    Free(hib_vol_info.slist);

    return rc;
}

    
/*
 * NAME:     file_exists
 *
 * FUNCTION: check if the specified file exists or not.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           TRUE : file exists.
 *           FALSE: file not exists.
 */
int 
file_exists(char *pathname)
{
    int rc = FALSE;
    Debug1("file (path %s)",pathname);

    if (access(pathname,0)==0){
        Debug0("exists.\n");
        rc = TRUE;
    }
    else{
        Debug0("does not exist.\n");
        rc = FALSE;
    }
    return rc;
}

/*
 * NAME:     exec_shellscript
 *
 * FUNCTION: execute shell script on resume and before sleep.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
exec_shellscript(int ctrl)
{
    int rc;

    switch(ctrl){

    case PMD_SLEEP:
        if (file_exists(PMD_SHELLSCRIPT_SLEEP)){
            Debug0("now execute shell script \n");
            if (system(PMD_SHELLSCRIPT_SLEEP)==0){
                rc = PM_SUCCESS;
            }
            else{
                rc = PM_ERROR;
            }
        }
        else{
            /* if file does not exist, exit with success */
            /* since it it optional script               */
            rc = PM_SUCCESS;
        }
        break;
    case PMD_RESUME:
        if (file_exists(PMD_SHELLSCRIPT_RESUME)){
            Debug0("now execute shell script \n");
            if (system(PMD_SHELLSCRIPT_RESUME)==0){
                rc = PM_SUCCESS;
            }
            else{
                rc = PM_ERROR;
            }
        }
        else{
            /* if file does not exist, exit with success */
            /* since it it optional script               */
            rc = PM_SUCCESS;
        }
        break;
    default:
        Debug0("exec_shellscript:invalid ctrl.\n");
        rc = PM_ERROR;
        break;
    }

    return rc;
} 

/*
 * NAME:     process_shutdown
 *
 * FUNCTION: shutdown system
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_ERROR : processed unsuccessfully.
 */
int 
process_shutdown()
{
    Debug0("process shutdown \n");
    system("shutdown -h -F");

    /* NOT REACHED */
    return PM_ERROR;
}


/* initial devl structure size (NDEV) and exponential growth factor (EDEV) */

#define NDEV    512
#define EDEV    2
#define DEVMPX  01
static int     ndev = 0;               /* number of valid devl structures */
static int     maxdevl = 0;            /* number of devl's allocated   */
struct _devl {
        dev_t   dev;
        char    dflags;
};
static struct _devl *devl;

/*
 * NAME:  getdev
 *
 * FUNCTION: get device information.
 *
 * RETURNS: PM_SUCCESS : processed successfully.
 */
int
getdev(char *dir)
{
        DIR *dirp;
        struct dirent *dp;
        struct stat sbuf1;
        char devname[MAXPATHLEN];
        int name_size;

        ndev=0;
        if ((dirp = opendir(dir)) == NULL) {

        }
        if (chdir(dir) == -1) {

        }
        while ((dp = readdir(dirp)) != NULL) {
                while(ndev >= maxdevl) {
                        if (maxdevl == 0) {
                            maxdevl = NDEV;
                            devl = (struct _devl *) malloc(
                                (size_t)(sizeof(struct _devl) * maxdevl));
                            RDebug2(8,
                                    "malloc@getdev(0x%x,0x%x)\n",
                                    devl,
                                    (sizeof(struct _devl)*maxdevl));
                        }
                        else {
                            maxdevl *= EDEV;

                            RDebug1(8,
                                    "before realloc@getdev(0x%x,0x%x)\n",
                                    devl);

                            devl = (struct _devl *) realloc((void *)devl,
                                (size_t)(sizeof(struct _devl) * maxdevl));

                            RDebug2(8,
                                    "realloc@getdev(0x%x,0x%x)\n",
                                    devl,
                                    (sizeof(struct _devl)*maxdevl));
                        }
                }
                /* set device name */
                if(dp->d_ino == 0)
                        continue;
                if(lstat(dp->d_name, &sbuf1) < 0)
                        continue;
                sprintf(devname, "%s/%s", dir, dp->d_name);

                /* search subdirectories */
                if ((sbuf1.st_mode&S_IFMT) == S_IFDIR) {
                        /* If not "." or ".." */
                        if (strcmp(dp->d_name, ".") &&
                            strcmp(dp->d_name, "..") &&
                            !access(devname,X_OK)) {
                                getdev(devname);
                                /* Return to correct dir. */
                                chdir(dir);
                        }
                        continue;
                }
                if (((sbuf1.st_mode&S_IFMT) != S_IFCHR) || 
                    (sbuf1.st_type == VLNK))
                        continue;
                /* if this is a subdirectory, prepend the subdir name
                 * onto the device name, e.g. pts/0
                 */

                /* select only the tty devices */
                if (strncmp(devname+5,"tty",3)==0){
                    devl[ndev].dev = sbuf1.st_rdev;
                    devl[ndev].dflags = (sbuf1.st_type == VMPC) ? DEVMPX : 0;
                    ndev++;
                }
        }
        closedir(dirp);
        return PM_SUCCESS;
}

/*
 * NAME:  is_tty_process
 *
 * FUNCTION: check if the specified process is running on tty device.
 *
 * RETURNS: 
 *          TRUE   : if specified process is running on tty.
 *          FALSE  : if specified process is not running on tty.
 *          
 */
int
is_tty_process(struct userinfo *uinfo)
{
        register i, c;
        register char *p, *q;
        static char buf[32];

        if (uinfo->ui_ttyp==0)
                return FALSE;     

        /* check tty processes */
        for (i=0; i<ndev; i++) {
                if (devl[i].dev == uinfo->ui_ttyd) {
                        Debug1("[ %s ]\n",uinfo->ui_comm);
                        return TRUE;
                }
        }
        /* NOTE: devl has the device numbers of tty device. */
        /*       refer to getdev() routine.                 */

        return FALSE; 
}

#define NTTY    512
#define ETTY    2
static int     ntty = 0;            
static int     maxtty = 0;          

/*
 * NAME:     check_processes
 *
 * FUNCTION: check the existence of lft, tty sessions ,and x-server
 *
 * NOTES:    naming convention after ps command.
 *
 * CAUTION:  memory allocated for mproc global variable is not to be
 *           liberated.
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int
check_processes()
{
    int i;
    struct userinfo up;
    pid_t  pid;
    struct stat devstat;
    char lft[] = "/dev/lft0";
    char tty[] = "/dev/tty0";
    dev_t lft_devno = -1;
    dev_t tty_major_devno = -1;
    char args[256];

    Debug0("check_processes \n");

    /* initialize data structures */

    memset(args,0,sizeof(args));
    memset(&up,0,sizeof(up));
    memset(&devstat,0,sizeof(devstat));

    /* initialize flags & pid, uid*/

    lft_session_flag = FALSE;
    tty_session_number = 0;
    xserver_flag = FALSE;
    syncd_flag   = FALSE;

    lft_pid = 0;      /* lft process id */
    xserver_uid = 0;  /* userid of xserver */
    syncd_pid = 0;    /* sync daemon pid */

    /* get information about special file "dev/lft0" */
    if (stat(lft,&devstat) == 0){
        lft_devno = devstat.st_rdev; /* get lft device number */
    }

    /* get information about special file "dev/tty0" */
    if (stat(tty,&devstat) == 0){
        tty_major_devno = (devstat.st_rdev>>16); /* get tty device number */
    }

    Debug1("lft_devno = %d \n",lft_devno);
    Debug1("tty_major_devno = %d \n",tty_major_devno);


    /* get process data */
    /* (nproc : number of process) */

    if ( (mproc = get_process_data(&nproc)) == NULL || nproc == -1){

        /* if get_process_data failed, return with error */
        /* and transition will fail.                     */

        return PM_ERROR;
    }
    Debug1("number of processes : %d \n",nproc);

    /* get the information about all devices */
    getdev("/dev");

    /* check lft, tty, xserver */

    for(i=0;i<nproc;i++){

        /* get user-information from process data*/
        getuser(&mproc[i],PROCSIZE,&up,U_SIZE);

        /* select user processes */
        if (!(mproc[i].pi_flag & SKPROC ) ){

            /* select X server process */
            /* (up.ui_comm : (truncated)program name) */
            if (strcmp(up.ui_comm,"X" ) == 0 ){

                /* set xserver_flag and _uid */
                xserver_flag = TRUE;
                xserver_uid = mproc[i].pi_uid;
            }

            /* select Sync daemon process */
            /* (up.ui_comm : (truncated)program name) */
            if (strcmp(up.ui_comm,"syncd" ) == 0 ){
                syncd_flag = TRUE;
                /* set syncd pid */
                syncd_pid = mproc[i].pi_pid;

                if( getargs(&mproc[i], PROCSIZE, args, sizeof(args))
                   == 0 ) {
                    syncd_interval = strtol(args+strlen(args)+1,NULL,10);
                    
                      Debug1(" syncd arg(%s)\n",args+strlen(args)+1);
                      Debug1(" syncd int(%d)\n",syncd_interval);
                }
            }

            /* select lft session process */
            if ( (up.ui_ttyd == lft_devno )&&(mproc[i].pi_ppid ==1)){
                if (check_utmp(mproc[i].pi_pid) == TRUE){
                    /* set lft_flag and _uid */
                    lft_session_flag = TRUE;
                    lft_pid = mproc[i].pi_pid;
                }
            }

            /* select tty session processes */
      
            if ( is_tty_process(&up) &&
                 (up.ui_ttyd !=lft_devno)
               ){
                if (check_utmp(mproc[i].pi_pid) == TRUE){
                    /* set tty_flag and _uid */

                    while(tty_session_number >= maxtty) {
                        if (maxtty == 0) {
                            maxtty = NTTY;
                            ttys_pid = (pid_t *) malloc(
                                (size_t)(sizeof(pid_t) * maxtty));
                            RDebug2(8,
                                    "malloc@tty(0x%x,0x%x)\n",
                                    ttys_pid,
                                    (sizeof(pid_t)*maxtty));
                        }
                        else {
                            maxtty *= ETTY;
                            RDebug1(8,
                                    "before realloc@tty(0x%x)\n",
                                    devl);

                            ttys_pid = (pid_t *) realloc((void *)ttys_pid,
                                (size_t)(sizeof(pid_t) * maxtty));

                            RDebug2(8,
                                    "realloc@tty(0x%x,0x%x)\n",
                                    ttys_pid,
                                    (sizeof(pid_t)*maxtty));
                        }
                    }
                    if (ttys_pid!=NULL){
                        RDebug2(32,"set tty process number ttys_pid[%d] =%d\n",
                           tty_session_number,
                           mproc[i].pi_pid);
                        ttys_pid[tty_session_number] = mproc[i].pi_pid;
                        tty_session_number++;
                    }
                    else{
                        fprintf(stderr,MSGSTR(ERR_MEMORY,DFLT_ERR_MEMORY));
                        return PM_ERROR;
                    }
                }
            }
        }
    }
        
    Debug1("syncd pid : %d \n",syncd_pid );
    Debug1("xserver uid : %d \n",xserver_uid );
    Debug1("lft_session_flag :%d \n",lft_session_flag);
    Debug1("lft_pid :%d \n",lft_pid);
    Debug1("tty_session_number :%d \n",tty_session_number);

#ifdef PM_DEBUG
    for(i=0;i<tty_session_number;i++){
        Debug1("ttys_pid:%d \n",ttys_pid[i]);
    }
#endif /* PM_DEBUG */

    /* NOTE : Memory allocated for mproc structure and ttys_pid have not */
    /*        been liberated.                                            */
    /*        after calling this routine, cleanup_for_check_processes    */
  
    return PM_SUCCESS;
}

/*
 * NAME:     cleanup_for_check_processes
 *
 * FUNCTION: clean up the allocated memroy and make it possible for it 
 *           to be reclaimed.
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 */
int
cleanup_for_check_processes()
{
    /* free memory for mproc, allocated in check_processes */
    if (mproc!=NULL){
        RDebug1(8,"now   free@cleanup_for_check_processes(0x%x)\n",(mproc));
        Free(mproc);
        RDebug1(8,"after free@cleanup_for_check_processes(0x%x)\n",(mproc));
    }
    return PM_SUCCESS;
}


/*
 * NAME:     kill_syncd
 *
 * FUNCTION: kill sync daemon.
 * 
 * DATA STRUCTURES:
 *  INPUT:   syncd_pid
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
kill_syncd()
{
    int kill_status;

    Debug0("kill_sync_daemon\n");

    if (syncd_pid == 0 || syncd_pid ==1 || syncd_pid == 2){
        fprintf(stderr,MSGSTR(ERR_INVAL_PID,DFLT_ERR_INVAL_PID),"syncd");
        return PM_ERROR;
    }

    /* kill sync daemon */
    if( (kill_status = kill(syncd_pid , SIGTERM)) != 0 ){
        fprintf(stderr,MSGSTR(ERR_KILL_TTY,DFLT_ERR_KILL_LFT));
    }

    return PM_SUCCESS;
}


/*
 * NAME:     revive_syncd
 *
 * FUNCTION: revive sync daemon
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
revive_syncd()
{
    int rc;
    char cmd_buf[256];
    int status=0;

    Debug0("revive syncd\n");

    memset(cmd_buf,0,sizeof(cmd_buf));
  
    if (syncd_interval<=0){
        syncd_interval=60;
    }
    sprintf(cmd_buf,"%d",syncd_interval);

    Debug1("revive command is %s \n",cmd_buf);

    /* cleanup_for_check_processes */
    cleanup_for_check_processes();

    /* check_processes() */
    check_processes();

    /* if check_processes failed due to the memory fault, */
    /* syncd_flag is set to FALSE.                        */
    /* in this case another syncd is to be spawned.       */

    /* cleanup_for_check_processes */
    cleanup_for_check_processes();

    /* revive sync daemon only when no sync daemon is alive  */
    /* this is for the case in which the sync daemon invoked */
    /* during standby state.                                 */

    if (!syncd_flag){
        if (fork()==0){
            if(fork()==0){
                execl("/usr/sbin/syncd","syncd",cmd_buf,0);
                fprintf(stderr,MSGSTR(ERR_SYNCD,DFLT_ERR_SYNCD));
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
	/* wait for SIGCHLD */
	wait(&status);
        return PM_SUCCESS;
    }
    else{
        return PM_ERROR;
    }

}


/*
 * NAME:     kill_lft_session
 *
 * FUNCTION: kill LFT terminal session
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *  INPUT:   lft_pid,pmd_parameters
 *  OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
kill_lft_session()
{
    int kill_status;


    Debug0("kill_lft_session\n");

    if (lft_pid == 0 || lft_pid ==1 || lft_pid ==2){
        fprintf(stderr,MSGSTR(ERR_INVAL_PID,DFLT_ERR_INVAL_PID),"LFT session");
        return PM_ERROR;
    }

    /* kill lft session */

    if (!xserver_flag){
        if( (kill_status = kill(lft_pid , SIGKILL)) != 0 ){
            fprintf(stderr,MSGSTR(ERR_KILL_TTY,DFLT_ERR_KILL_LFT));
        }
    }
    else{
        Debug0("xserver was running. lft was not killed  \n");
    }

    return PM_SUCCESS;
}



/*
 * NAME:     kill_tty_session
 *
 * FUNCTION: kill TTY terminal session
 *
 * NOTES: 
 * 
 * DATA STRUCTURES:
 *           INPUT:   ttys_pid[],tty_session_number
 *           OUTPUT:  none
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully
 */
int 
kill_tty_session()
{
    int i;
    int kill_status;
    RDebug0(32,"kill_tty_session\n");

    /* kill all the tty sessions */
    for(i=0;i<tty_session_number;i++){
        RDebug1(32,"tty process pid:%d\n",ttys_pid[i]);
        if (ttys_pid[i] != 0 && ttys_pid[i] !=1 ){
            /* kill tty session */
            if( (kill_status = kill(ttys_pid[i] , SIGKILL)) != 0 ){
                fprintf(stderr,MSGSTR(ERR_KILL_TTY,DFLT_ERR_KILL_LFT));
            }
            RDebug1(32,"kill_status:%d\n",kill_status);
        }
        else{
            fprintf(stderr,
                    MSGSTR(ERR_INVAL_PID,DFLT_ERR_INVAL_PID),
                    "TTY session");
        }
    }

    return PM_SUCCESS;
}

/*
 * NAME:     lock_screen
 *
 * FUNCTION: invoke xlock to secure terminal when resume.
 *
 * NOTES:    prior to a call to this function, checl_processes()
 *           should be called and set mproc and nproc global 
 *           variables.
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : processed unsuccessfully.
 */
int 
lock_screen()
{
    pid_t dtlogin_pid;
    uid_t dtsession_uid;
  

    Debug0("lock screen \n");

    /* check the validity of mproc and nproc */
    if ( mproc == NULL || nproc == -1){

        /* if check_process() failed and the process */
        /* information is not available, just invoke */
        /* xlock with root password.                 */
        run_xlock(0);
        return PM_ERROR;
    }

    /* first, check the existence of dtlogin at console*/
    /* and get pid of the parent process of "dtlogin <:0>" */

    if( check_program("dtlogin",
                     "dtlogin <:0>",
                     -1,          /* no check of parent pid */
                     &dtlogin_pid,
                     NULL)){

        /* then ,check the children of "dtlogin <:0>" */
        Debug0("dtlogin found\n");
        Debug1("dtlogin pid =%d\n",dtlogin_pid);

        if ( check_program("dtgreet",
                           NULL,
                           dtlogin_pid,
                           NULL,
                           NULL)
            ){
            /* No operation */
        }
        else{
            if (check_program("dtsession",
                              NULL,
                              dtlogin_pid,
                              NULL,
                              &dtsession_uid)
                ){
                
                Debug0("run_LockDisplay\n");
                run_LockDisplay(dtsession_uid);
            }
            else{
                if (!check_program("xlock",
                                   NULL,
                                   -1,    /* no check of parent process id */
                                   NULL,
                                   NULL)
                    ){
                    Debug0("run_xlock\n");
                    run_xlock(xserver_uid);
                }
            }
        }
    }
    else{
        if (!check_program("xlock",
                           NULL,
                           -1,    /* no check of parent process id */
                           NULL,
                           NULL)
            ){
            Debug0("run_xlock\n");
            run_xlock(xserver_uid);
        }/* end of if */
    } /* end of if else */

    return PM_SUCCESS;
}



/*
 * NAME:     check_program
 *
 * FUNCTION: check if specified program is running
 *
 * NOTES:    make sure that mproc contains process information.
 *
 * DATA STRUCTURES:
 * INPUT   : nproc,
 *           program,
 *           arg1,
 *           ppid,
 *           p_pid,
 *           p_uid
 * OUTPUT  : *p_pid
 *           *p_uid
 *
 * RETURN VALUE DESCRIPTION:  
 *           TRUE  : program running
 *           FALSE : program not running
 */
int
check_program(char *program,
              char *detail,
              pid_t ppid,
              pid_t *p_pid,
              uid_t *p_uid)
{
    int i;
    struct userinfo up;
    char args[256];
    int rc = FALSE;


    for( i = 0; i<nproc; i++ ) {
        /* get user information */
        getuser(&mproc[i], PROCSIZE, &up, U_SIZE);

        /* select user process */
        if( !(mproc[i].pi_flag & SKPROC) ) {

            if( strncmp( up.ui_comm, program ,strlen(program)) == 0 ) {

                Debug1("check program:prgname matched (%s)\n",program);

                if( ppid == -1 || mproc[i].pi_ppid == ppid ) {

                /* no ppid check if it is -1 */
                Debug1("check program:ppid matched (%d)\n",ppid);

                    if( detail == NULL ) {  /* no argument check */
        
                        if( p_uid != NULL ) {
                            *p_uid = mproc[i].pi_uid;  /* set uid */
                        }
                        if( p_pid != NULL ) {
                            *p_pid = mproc[i].pi_pid;  /* set pid */
                        }

                        rc = TRUE;
                    }
                    /* argument check */
                    else{

                        Debug0("check_program:arg_check\n");

                        if( getargs(&mproc[i], PROCSIZE, args, sizeof(args))
                           == 0 ) {

                            if( strncmp(args,detail,strlen(detail)) == 0 ) {
                                Debug1("check_program:arg_check matched(%s)\n",
                                       detail);

                                if( p_uid != NULL ) {
                                    *p_uid = mproc[i].pi_uid;  /* set uid */
                                }
                                if( p_pid != NULL ) {
                                    *p_pid = mproc[i].pi_pid;  /* set pid */
                                }

                                rc = TRUE;


                            } /* end of if (argument matched )*/
                        }/* end of if ( getargs succeeded )*/
                    }/* end of else if ( argument set or not )*/    
                } /* end of if (ppid matched )*/
            } /* end of if (program name matched )*/
        } /* end of if (select user process) */
    } /* end of for */


    return rc;

}

/*
 * NAME:     check_utmp
 *
 * FUNCTION: check that pid exists in utmp entry. 
 *            
 * NOTES:
 *
 * DATA STRUCTURES:
 *           utmp - utmp structure
 *
 * RETURN VALUE DESCRIPTION:  
 *           TRUE - pid is valid.
 *           FALSE - pid is invalid.
 */
int
check_utmp(pid_t pid)
{
    struct utmp *ut;

    setutent();            /* open/rewind utmp */
    while( (ut = getutent()) != NULL ) {
        if( ut->ut_pid == pid )
            return TRUE;
    }
    endutent();

    return FALSE;
}


/*
 * NAME:     get_process_data
 *
 * FUNCTION: Routine stolen from ps.  Grabs enough memory to read in
 *           the proc structure, set nprocs and returns a pointer the
 *           the interesting procs returned from the getproc system call.
 *
 * RETURN VALUE DESCRIPTION:  
 *           pointer to the proc structure or NULL if failed.
 */
struct procinfo *
get_process_data(long *nproc)
{
    long multiplier = 5;
    struct procinfo *Proc = NULL;

    /* now allocate memory for process information */
    if ( (Proc = 
          (struct procinfo *) malloc((size_t)sizeof(unsigned long)))
        == NULL){

        /* run out of space */
        fprintf(stderr,MSGSTR(ERR_MEMORY,DFLT_ERR_MEMORY));
        return NULL;
    }

    RDebug2(8,
            "now    malloc @get_process_data (0x%x,0x%x)\n",
            Proc,
            sizeof(unsigned long));

    *nproc = 0;
    while (((*nproc = getproc (Proc, *nproc, PROCSIZE)) == -1) &&
           errno == ENOSPC){
       
        /* reset the errno */
        errno = 0;

        /* check nproc */
        *nproc = *(long *) Proc;      /* num of active proc structures*/
        *nproc += (multiplier <<= 1); /* Get extra in case busy system*/

        RDebug1(8,      
                "before realloc@get_process_data (0x%x)\n",
                Proc);


        /* reallocate as much memory as can hold entire active process table */
        if ( (Proc = (struct procinfo *)realloc((void *)Proc,
                                                (size_t)(PROCSIZE * (*nproc))))
            == NULL){ 
            /* run out of space before we could read */
            /* in the entire proc structure.         */
            fprintf(stderr,MSGSTR(ERR_PROCDATA,DFLT_ERR_PROCDATA));
            return NULL;
        }       

        RDebug2(8,      
                "after  realloc@get_process_data (0x%x,0x%x)\n",
                Proc,
                (PROCSIZE * (*nproc)));
        RDebug1(8,
                "*nproc=%d\n",
                *nproc);

    }

    /* errno != ENOSPC , getting out of whil loop. */

    if (*nproc == -1){
        /* if errno is other than ENOSPC and *nproc == -1 */
        /* there may be something wrong with getproc().   */
        return NULL;
    }

    return (Proc);

}

/*
 * NAME:     run_xlock
 *
 * FUNCTION: invoke xlock for the user using X on console 
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  
 *           pid of invoked chile process is returned.
 *           When an error occurred, -1 is returned.
 */
int
run_xlock(uid_t xs_uid)
{
    char *xs_username;
    int rc;
    struct passwd *xs_pwent;
    char logname[32],login[32],home[PATH_MAX+8];
    int status = 0;
    pid_t xlock_pid = 0;

    xs_username = (char *)IDtouser(xs_uid);

    /* LOGIN environment */
    sprintf(login, "LOGIN=%s", xs_username);
    
    /* LOGNAME environment */
    sprintf(logname, "LOGNAME=%s", xs_username);
    
    /* HOME environment */
    xs_pwent = getpwuid(xs_uid);
    if( xs_pwent == NULL ) {
        return PM_ERROR;
    }
    sprintf(home, "HOME=%s", xs_pwent->pw_dir);

    Debug0("now fork \n");
    if (fork()==0){
        /* child process*/
        if( (xlock_pid=fork())==0){
            /* grandchild process (xlock) */
            putenv("AUTHSTATE=compat");
            putenv("DISPLAY=:0.0");
            putenv(logname);
            putenv(login);
            putenv(home);
            setuid(xs_uid);
            execl("/usr/lpp/X11/bin/xlock","xlock",0);    /* run xlock */
            fprintf(stderr,MSGSTR(ERR_XLOCK,DFLT_ERR_XLOCK));
            exit(EXIT_FAILURE);
        }
        setpriority(PRIO_PROCESS,xlock_pid,-20);
        exit(EXIT_SUCCESS);
    }
    /* wait for SIGCHLD */
    wait(&status);

    return rc;
}


/*
 * NAME:     run_LockDisplay
 *
 * FUNCTION: invoke LockDisplay by dtaction for the user using X on console 
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:  
 *           pid of invoked child process is returned.
 *           When an error occurred, -1 is returned.
 */
int
run_LockDisplay(uid_t dts_uid)
{
    char *dts_username;
    int rc;
    char cmd[256];
    struct passwd *dts_pwent;
    char logname[32],login[32],home[PATH_MAX+8];
    int status = 0;
    pid_t lockdisplay_pid = 0;

    /* user name for su - */
    dts_username = (char *)IDtouser(dts_uid);

    /* build command */
    sprintf(cmd,                                       
            "(/bin/ksh -c `/usr/dt/bin/dtsearchpath`;\
              nice -20 /usr/dt/bin/dtaction LockDisplay)",dts_username   );

    /* LOGIN environment */
    sprintf(login, "LOGIN=%s", dts_username);
    
    /* LOGNAME environment */
    sprintf(logname, "LOGNAME=%s", dts_username);
    
    /* HOME environment */
    dts_pwent = getpwuid(dts_uid);
    if( dts_pwent == NULL ) {
        return PM_ERROR;
    }
    sprintf(home, "HOME=%s", dts_pwent->pw_dir);

    Debug0("now fork \n");
    if (fork()==0){
        if ((lockdisplay_pid=fork())==0){
            putenv("DISPLAY=:0.0");
            putenv(logname);
            putenv(login);
            putenv(home);
            setuid(dts_uid);

            Debug0(cmd);
            if (system(cmd)!=0){
                fprintf(stderr,MSGSTR(ERR_XLOCK,DFLT_ERR_XLOCK));
                exit(EXIT_FAILURE);     
            }
            exit(EXIT_SUCCESS);
        }
        setpriority(PRIO_PROCESS,lockdisplay_pid,-20);
	Debug0("fork child end\n");
        exit(EXIT_SUCCESS);
    }
    /* wait for SIGCHLD */
    wait(&status);

    return rc;
}


/*
 * NAME:      set_completion_notice
 *
 * FUNCTION:  set pmlib_event_notice according to the pm_target_state
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *            PM_SUCCESS : processed successfully.
 *            PM_ERROR   : invalid arguments specified.
 */
int 
set_completion_notice()
{
     RDebug0(1,"set_completion_notice\n");

    /* present system state  : pm_target_state.state  ( non-resume case )*/
    /* previous system state : pm_present_state  ( non-resume case )*/

    /* present system state  : PM_SYSTEM_ENABLE ( resume )*/
    /* previous system state : pm_target_state.state  ( resume )*/


   switch(pmd_parameters.pm_target_state.state){

   case PM_SYSTEM_HIBERNATION:  /* resume case */
       RDebug0(16,"resume from hibernation \n");

       set_pmlib_event_entry(PMLIB_EVENT_RESUME_FROM_HIBERNATION);

       /* lock pmd_parameters */
       Mutex_Lock(&pmd_parameters_mutex);

       pmd_parameters.pm_target_state.state = PM_SYSTEM_ENABLE;

       /* unlock pmd_parameters */
       Mutex_Unlock(&pmd_parameters_mutex);

       /* NOTE: in case of hibernation, transition completion means */
       /* resume from hibernation. To be consistent with other case */
       /* set pm_target_state to PM_SYSTEM_ENABLE.                 */

       break;
   case PM_SYSTEM_SUSPEND:     /* resume case */
       RDebug0(16,"resume from suspend \n");

       set_pmlib_event_entry(PMLIB_EVENT_RESUME_FROM_SUSPEND);

       /* lock pmd_parameters */
       Mutex_Lock(&pmd_parameters_mutex);

       pmd_parameters.pm_target_state.state = PM_SYSTEM_ENABLE;

       /* unlock pmd_parameters */
       Mutex_Unlock(&pmd_parameters_mutex);

       /* NOTE: in case of suspend, transition completion means */
       /* resume from suspend. To be consistent with other case */
       /* set pm_target_state to PM_SYSTEM_ENABLE.             */

       break;
   case PM_SYSTEM_STANDBY:
       RDebug0(16,"enter standby \n");
       set_pmlib_event_entry(PMLIB_EVENT_NOTICE_COMPLETION); 
       break;
   case PM_SYSTEM_ENABLE:
       switch(pmd_parameters.pm_present_state.state){
       case PM_SYSTEM_FULL_ON:
           RDebug0(16,"enter enable \n");
           set_pmlib_event_entry(PMLIB_EVENT_NOTICE_COMPLETION); 
           /* NOTE: case of FULL_ON -> ENABLE */
           break;
       case PM_SYSTEM_STANDBY:
           RDebug0(16,"resume from standby \n");
           set_pmlib_event_entry(PMLIB_EVENT_RESUME_FROM_STANDBY);
           /* NOTE: case of STANDBY -> ENABLE */
           break;
       default:
           /* error  */
           break;
       }
       break;
   case PM_SYSTEM_FULL_ON:
           RDebug0(16,"enter full_on \n");
           set_pmlib_event_entry(PMLIB_EVENT_NOTICE_COMPLETION); 
       break;
   default:
       /* error */
       break;
   }

    return PM_SUCCESS;
}

/*
 * NAME:     reset_state_change_process_flag
 *
 * FUNCTION: enable request_flag and event_flag 
 *           to be set to request_entry and event_entry
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
reset_state_change_process_flag()
{
    Debug0("reset event_flag and request_flag \n");

    /* reset event_flag and request_flag */

    /* lock event flag*/
    Mutex_Lock(&event_flag_mutex);

    event_flag=PMD_FLAG_NONE;
    Debug0("event acceptable\n");

    /* unlock event flag*/
    Mutex_Unlock(&event_flag_mutex);

    /* lock request flag */
    Mutex_Lock(&request_flag_mutex);

    request_flag=PMD_FLAG_NONE;
    Debug0("request acceptable\n");

    /* unlock request flag */
    Mutex_Unlock(&request_flag_mutex);

    return PM_SUCCESS;
}

/*
 * NAME:     invoke_query
 *
 * FUNCTION: wait for response of application to the broadcast signal.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
invoke_query(int ctrl)
{
  int wait_result = PMD_CONTINUE_LOOP;
  struct timespec timeout;
  time_t seconds;


  RDebug1(16,"invoke_query(ctrl :%d)\n",ctrl);

  /* in case of invoking confirmation, initialize error-caused pid and answer*/

  if (ctrl == PMD_INVOKE_CONFIRMATION){
      /* reset error-caused application pid. & answer */
      /* lock application_answer */
      Mutex_Lock(&application_answer_mutex);
      
      appl_answer = PMD_UNANIMOUS_OK;
      appl_pid    = 0 ;/* error caused application pid */
      
      /* unlock application_answer */
      Mutex_Unlock(&application_answer_mutex);

  }

  /* if there is no application registered, just return and proceed */

  if (client_number == 0){

      return PM_ERROR;
  }

  /* lock pmlib_event_entry */
  Mutex_Lock(&pmlib_event_entry_mutex);

  /* set timeout variable */
  seconds = time (NULL);
  seconds += PMD_RESPONSE_TIMEOUT;
  timeout.tv_sec = (unsigned long)seconds;
  timeout.tv_nsec = 0L;  

  /* reset request entry */
  reset_request_entry_count(pclientinfo_head);

  /* set_count_flag */
  set_count_flag(ctrl);

  /* pmlib_event_entry set already */

  /* lock response_cond mutex */
  Mutex_Lock(&response_cond_mutex);

  /* broadcast signal */
  broadcast_signal(pclientinfo_head);

  /* condition timedwait start */ 
  /* wait till predicate true or timedout */

  while(wait_result == PMD_CONTINUE_LOOP){
      switch(pthread_cond_timedwait(&response_condvar,
                                    &response_cond_mutex,
                                    &timeout)){
      case 0:
          if (response_predicate){
              wait_result = PMD_PROCEED;
              break;
          }
      case ETIMEDOUT:
          wait_result = response_predicate ? PMD_PROCEED : PMD_TIMEOUT;
          RDebug0(16,"invoke_query: timeout \n");
          break;

      default:
          wait_result = PMD_ERROR;
          break;
      } /* end of switch */
  }

  /* unlock response_cond mutex */
  Mutex_Unlock(&response_cond_mutex);
  
  if (wait_result == PMD_TIMEOUT && ctrl == PMD_INVOKE_CONFIRMATION){
      appl_answer = PMD_PARTIAL_OK;
  }

  /* reset request entry */
  reset_request_entry_count(pclientinfo_head);

  /* reset_count_flag */
  reset_count_flag(ctrl);

  /* reset the response predicate */

  /* lock response_cond mutex */
  Mutex_Lock(&response_cond_mutex);
  
  response_predicate = FALSE;

  /* unlock response_cond mutex */
  Mutex_Unlock(&response_cond_mutex);

  /* unlock pmlib_event_entry_mutex */
  Mutex_Unlock(&pmlib_event_entry_mutex);

  /* NOTE:      now pmd will sleep for a while */
  /* to avoid failure to get signal and ensure */
  /* that the socket returning to the pm aware */
  /* applications be reached successfully.     */
  sleep(1);

  return PM_SUCCESS;
}

/*
 * NAME:     set_count_flag
 *
 * FUNCTION: sets the count flag
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
set_count_flag(int ctrl)
{
    /* during these flags TRUE, query request or confirmation will be
       counted */

    switch(ctrl){
    case PMD_INVOKE_EVENT_QUERY:
        /* lock count_event_query_flag */
        Mutex_Lock(&count_event_query_flag_mutex);
    
        count_event_query_flag = TRUE;

        /* unlock count_event_query_flag */
        Mutex_Unlock(&count_event_query_flag_mutex);
        
        break;
    case PMD_INVOKE_CONFIRMATION:
        /* lock count_confirmation_flag */
        Mutex_Lock(&count_confirmation_flag_mutex);
    
        count_confirmation_flag = TRUE;

        /* unlock count_confirmation_flag */
        Mutex_Unlock(&count_confirmation_flag_mutex);
        
        break;
    default:
        return PM_ERROR;
    }/* end of switch */

  return PM_SUCCESS;
}


/*
 * NAME:     reset_count_flag
 *
 * FUNCTION: resets the count_flag
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
 int 
reset_count_flag(int ctrl)
{
    /* during these flags FALSE, query request or confirmation will not be
       counted */

    switch(ctrl){
    case PMD_INVOKE_EVENT_QUERY:
        /* lock count_event_query_flag */
        Mutex_Lock(&count_event_query_flag_mutex);
    
        count_event_query_flag = FALSE;

        /* unlock count_event_query_flag */
        Mutex_Unlock(&count_event_query_flag_mutex);
        
        break;
    case PMD_INVOKE_CONFIRMATION:
        /* lock count_confirmation_flag */
        Mutex_Lock(&count_confirmation_flag_mutex);
        
        count_confirmation_flag = FALSE;

        /* unlock count_confirmation_flag */
        Mutex_Unlock(&count_confirmation_flag_mutex);
        
        break;
    default:
        return PM_ERROR;
    }/* end of switch */

  return PM_SUCCESS;
}



/*
 * NAME:     set_pmlib_event_entry
 *
 * FUNCTION: set events for application.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
set_pmlib_event_entry(int event)
{
    RDebug1(16,"set_pmlib_event_entry (event:%x)\n",event);

     /* lock pmlib_event_entry mutex */
    Mutex_Lock(&pmlib_event_entry_mutex);

    pmlib_event_entry = event;

    switch(battery_mode ){

    case PM_AC:
        pmlib_event_entry |= PMLIB_EVENT_AC;
        break;
    case PM_DC:
        pmlib_event_entry |= PMLIB_EVENT_DC;
        break;
    default:
        Debug0("Invalid Power Supply Mode \n");
    }

    /* NOTE:                                      */
    /* Power supply mode is always ORed to the event */

    /* unlock pmlib_event_entry mutex */
    Mutex_Unlock(&pmlib_event_entry_mutex);
 
    return PM_SUCCESS;
}

/*
 * NAME:     process_event_notice
 *
 * FUNCTION: processes notification of misc events.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
process_event_notice()
{
    Debug0("process_event_notice \n");  

    /* lock battery mutex */
    Mutex_Lock(&battery_mutex);
    
    if (battery_toggled){
        Debug0("battery toggled \n");
        switch (battery_mode ){
        case PM_AC :
            set_pmlib_event_entry(PMLIB_EVENT_AC);
            break;
        case PM_DC:
            set_pmlib_event_entry(PMLIB_EVENT_DC);
            break;
        default:
            Debug0("pmd :battery mode error\n");
        }
        /* broadcast signal to invoke event query */
           broadcast_signal(pclientinfo_head);

        battery_toggled = FALSE;
    }
    /* unlock battery mutex */
    Mutex_Unlock(&battery_mutex);

    return PM_SUCCESS;
}

/*
 * NAME:     state2level
 *
 * FUNCTION: convert the system state to level of power management
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           PMD_LEVEL_XXXX : integer assigned to PM_SYSTEM_XXXX state.
 *           PM_ERROR       : invalid arguments specified.
 */
int
state2level(int pm_state)
{
    if (is_pm_state(pm_state)){
      switch(pm_state){
      case PM_NONE:
          return PMD_LEVEL_NONE;       /* currently 0 */
      case PM_SYSTEM_FULL_ON:
          return PMD_LEVEL_FULL_ON;    /* currently 1 */
      case PM_SYSTEM_ENABLE:
          return PMD_LEVEL_ENABLE;     /* currently 2 */
      case PM_SYSTEM_STANDBY:
          return PMD_LEVEL_STANDBY;    /* currently 3 */
      case PM_SYSTEM_SUSPEND:
          return PMD_LEVEL_SUSPEND;    /* currently 4 */
      case PM_SYSTEM_HIBERNATION:
          return PMD_LEVEL_HIBERNATION;/* currently 5 */
      case PM_SYSTEM_SHUTDOWN:
          return PMD_LEVEL_SHUTDOWN;   /* currently 6 */
      case PM_SYSTEM_TERMINATE:
          return PMD_LEVEL_TERMINATE;  /* currently 7 */
      default:
          return PM_ERROR;
    }
  }
  else{
      return PM_ERROR;
  }
}

/*
 * NAME:     set_event_priority
 *
 * FUNCTION: set the priority to each event.
 *
 * NOTES: pm_state
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS     : processed successfully.
 *           PM_ERROR       : processed unsuccessfully.
 */
int
set_event_priority(int pm_state)
{

    int event;
    int state;

    RDebug1(1,"set event priority according to %s\n",
           pmstate2asc(pm_state));

    /* clear priority table */

    /* Lock event priority table */
    Mutex_Lock(&event_priority_table_mutex);

    for(event=0;event<PMD_TOTAL_EVENT_NUMBER;event++){
        event_priority_table[event]=0;
        state = set_state_by_event(EVENT_REVHASH(event));                     

            if (state != PM_NONE && state != pm_state ){
                event_priority_table[event]=
                    state2distance(state,pm_state)+1;
            }  
    }
    
#if PM_DEBUG
    for(event=0;event<PMD_TOTAL_EVENT_NUMBER;event++){
        state = set_state_by_event(EVENT_REVHASH(event));                     
        Debug3("event [%2d] ==priority [%+2d] == assigned state [%s]\n",
               EVENT_REVHASH(event),
               event_priority_table[event],
               pmstate2asc(state));
    }
#endif

    /* Lock event priority table */
    Mutex_Unlock(&event_priority_table_mutex);
    
    return PM_SUCCESS;
}

/*
 * NAME:     state2distance
 *
 * FUNCTION: measure the distance between two system state.
 *
 * NOTES:     distance is difference of level of 2 state.
 *             
 *          # distance =   0            (state1 <= state2 or state is PM_NONE)
 *                       state1-state2  (state1 >  state2)
 *             
 * RETURN VALUE DESCRIPTION:
 *           Integer        : distance.
 *           PM_ERROR       : invalid arguments specified.
 */
int
state2distance(int state1,int state2)
{
    int level1,level2;
    int distance;

    if ( (level1 = state2level(state1) ) == PM_ERROR ||
         (level2 = state2level(state2) ) == PM_ERROR ){
        return PM_ERROR;
    }
    else{

        if ( level1 == PMD_LEVEL_NONE ||
             level2 == PMD_LEVEL_NONE ){
            return distance = 0;
        }
        else{
            if ((distance = level1 - level2)<0){
                distance = 0;
            }
        }
        return distance;
    }
}

/*
 * NAME:     set_syscall_busy()
 *
 * FUNCTION: set syscall_busy flag to TRUE
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 */
int
set_syscall_busy()
{
    Mutex_Lock(&syscall_busy_mutex);

    syscall_busy = TRUE;
    Mutex_Unlock(&syscall_busy_mutex);

    return PM_SUCCESS;
}

/*
 * NAME:     reset_syscall_busy()
 *
 * FUNCTION: reset syscall_busy flag to FALSE
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 */
int
reset_syscall_busy()
{
    Mutex_Lock(&syscall_busy_mutex);

    syscall_busy = FALSE;
    Mutex_Unlock(&syscall_busy_mutex);

    return PM_SUCCESS;
}

/*
 * NAME:     create_mutexes
 *
 * FUNCTION: creates mutexes for mutual exclusive resources.
 *
 * NOTES: 
 * 
 *
 * RETURN VALUE DESCRIPTION:
 *           PM_SUCCESS : processed successfully.
 *           PM_ERROR   : invalid arguments specified.
 */
int 
create_mutexes()
{

  Mutex_Init(&state_change_cond_mutex,NULL);
  Mutex_Init(&response_cond_mutex,NULL);
  Mutex_Init(&event_flag_mutex,NULL);
  Mutex_Init(&event_entry_mutex,NULL);
  Mutex_Init(&request_flag_mutex,NULL);
  Mutex_Init(&request_entry_mutex,NULL);
  Mutex_Init(&pmd_parameters_mutex,NULL);
  Mutex_Init(&pmlib_latest_error_mutex,NULL);
  Mutex_Init(&pmlib_event_entry_mutex,NULL);
  Mutex_Init(&battery_mutex,NULL);
  Mutex_Init(&count_event_query_flag_mutex,NULL);
  Mutex_Init(&count_confirmation_flag_mutex,NULL);
  Mutex_Init(&application_answer_mutex,NULL);
  Mutex_Init(&client_number_mutex,NULL);
  Mutex_Init(&client_chain_mutex,NULL);
  Mutex_Init(&event_queue_mutex,NULL);
  Mutex_Init(&event_priority_table_mutex,NULL);
  Mutex_Init(&prior_event_flag_mutex,NULL);
  Mutex_Init(&syscall_busy_mutex,NULL);
  Mutex_Init(&thread_birth_mutex,NULL);

  return PM_SUCCESS;
}

/*
 * NAME:      create_condition_variables
 *
 * FUNCTION:  create condition variables for waking up main thread.
 *
 * NOTES: 
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : invalid arguments specified.
 */
int 
create_condition_variables()
{
    Cond_Init(&state_change_condvar, NULL);
    Cond_Init(&response_condvar ,NULL);
    return PM_SUCCESS;
}

/*
 * NAME:      create_major_threads
 *
 * FUNCTION:  create threads
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *            PM_SUCCESS : processed successfully.
 *            PM_ERROR   : invalid arguments specified.
 */
int 
create_major_threads()
{
    pthread_attr_t attr;

    Attr_Init(&attr);
    Attr_Undetached(&attr);

    Thread_Create(&core_event_thread,
                  &attr,
                  core_event_interface,
                  NULL);
    Thread_Create(&socket_thread,
                  &attr,
                  socket_interface,
                  NULL);

    Attr_Destroy(&attr);

    return PM_SUCCESS;
}

/*
 * NAME:      create_signal_waiter_thread
 *
 * FUNCTION:  create signal_waiter_thread
 *
 * NOTES: 
 *
 * RETURN VALUE DESCRIPTION:
 *            PM_SUCCESS : processed successfully.
 *            PM_ERROR   : invalid arguments specified.
 */
int 
create_signal_waiter_thread()
{
    pthread_attr_t attr;

    Attr_Init(&attr);
    Attr_Undetached(&attr);

    Thread_Create(&signal_waiter_thread,
                  &attr,
                  signal_waiter_interface,
                  NULL);

    Attr_Destroy(&attr);

    return PM_SUCCESS;
}
