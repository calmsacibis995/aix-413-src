static char sccsid[] = "@(#)08  1.6  src/rspc/usr/bin/pmd/pmd_signal_waiter.c, pwrcmd, rspc41J, 9523A_all 6/5/95 23:31:03";
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: signal_waiter_interface,
 *            process_terminate,
 *            cancel_thread,
 *            commit_suicide,
 *            cleanup_for_terminate.
 *            signal_ignore.
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

/*--------------------------------------*/ 
/* Subroutines for Signal Waiter Thread */
/*--------------------------------------*/

#include "pmd.h"
#include "pmd_global_ext.h"

/*------------------*/
/* local variables  */
/*------------------*/

/*
 * NAME:     signal_waiter_interface
 *
 * FUNCTION: handles signals.
 *
 * NOTES:    executed on the thread environment (signal_waiter_thread).
 * 
 * RETURN VALUE DESCRIPTION:
 *           NONE
 */
void *
signal_waiter_interface(void *arg)
{
    sigset_t signal_set;    /* a set of signals to be waited */
    int sig;                /* signal to be caught */
    pm_system_state_t transition; /* for acknowledge to pm core */
                                  /* which is issued before pmd */
                                  /* is being terminated */   

    memset(&transition,0,sizeof(transition));

    sigemptyset(&signal_set);      /* empty the set of signals */

    sigaddset(&signal_set,SIGHUP); /* add SIGHUP  to the set of signals */
    sigaddset(&signal_set,SIGINT); /* add SIGINT  to the set of signals */
    sigaddset(&signal_set,SIGQUIT);/* add SIGQUIT to the set of signals */
    sigaddset(&signal_set,SIGTERM);/* add SIGTERM to the set of signals */
    sigaddset(&signal_set,SIGTTOU);/* add SIGTTOU to the set of signals */

    /* block signals during a critical section of the code */
    sigthreadmask(SIG_BLOCK,&signal_set,NULL);


    /* NOTE 
             SIGKILL ,SIGSTOP, SIGCONT signal is not allowed to be blocked,
             which cases cause no error though.
    */
    
    /* waiting synchronization between threads */

    /* signal handling */
    for(;;){
        sigwait(&signal_set,&sig);
        
        switch(sig){
        case SIGHUP:
            Debug0("SIGHUP caught \n");
            /* ignore the signal */
            break;
        case SIGINT:
            Debug0("SIGINT caught \n");
            /* ignore the signal */
            break;
        case SIGQUIT:
            Debug0("SIGQUIT caught \n");
            /* ignore the signal */
            break;
        case SIGTERM:
            Debug0("SIGTERM caught \n");
	    transition.state = PM_TRANSITION_START;
	    transition.event = PM_EVENT_TERMINATE;
	    pm_control_state(PM_CTRL_START_SYSTEM_STATE, 
			     (caddr_t)&transition);
            break;
        case SIGTTOU:
            Debug0("SIGTTOU caught \n");
            /* ignore the signal */
            break;
        default:
            Debug1("unexpected signal[%d] has been caught \n",sig);
            /* ignore the signal */
            break;
        }

    }
    pthread_exit(NULL);
}


/*
 * NAME:      process_terminate
 *
 * FUNCTION:  terminate all the threads
 *
 * NOTES:  this routine will be called by main_thread and signal_waiter_thread
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_ERROR : processed unsuccessfully.
 */
int 
process_terminate(int ctrl)
{
    /* rmdev */
    if (ctrl == PMD_TERMINATE_WITH_RMDEV){
        if (system("/usr/sbin/rmdev -l pmc0")!=0){
            fprintf(stderr,MSGSTR(ERR_RMDEV,DFLT_ERR_RMDEV));
        }
    }

    /* push cleanup handler */
    pthread_cleanup_push(cleanup_for_terminate,NULL);

    /* cancel socket_thread with proper cleanup handler */
    RDebug0(2,"now cancel core_event_thread\n");
    cancel_thread(&core_event_thread);

    /* cancel socket_thread with proper cleanup handler */
    RDebug0(2,"now cancel main_thread\n");
    cancel_thread(&main_thread);

    /* cancel socket_thread with proper cleanup handler */
    RDebug0(2,"now cancel socket_thread\n");
    cancel_thread(&socket_thread);

    /* cancel socket_thread with proper cleanup handler */
    RDebug0(2,"now cancel signal_waiter_thread\n");
    cancel_thread(&signal_waiter_thread);


    /* commit suicide */
    RDebug0(2,"now cancel calling thread\n");
    commit_suicide();
    
    /* NOT REACHED */

    /* pop cleanup handler */
    pthread_cleanup_pop(0);
         /* to be called when cleanup handler is no longer needed.*/


    return PM_ERROR;
}

/*
 * NAME:      cleanup_for_terminate
 *
 * FUNCTION:  cleanup handler for terminating pm daemon.
 *
 * NOTES:  
 * 
 * RETURN VALUE DESCRIPTION:
 *            NONE
 */
void 
cleanup_for_terminate(void* arg)
{

  Debug0("clean up handler for termination called.\n");

  /* message for terminate */
  fprintf(stderr,MSGSTR(INFO_TERMINATE,DFLT_INFO_TERMINATE));

  /* close message catalog          */
  catclose(catd);

  return;

}

/*
 * NAME:      cancel_thread
 *
 * FUNCTION:  cancel other threads than calling thread.
 *
 * NOTES:  
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : processed unsuccessfully.
 */
int
cancel_thread(pthread_t *p_thread )
{
    int join_status;

    pthread_t calling_thread;

    calling_thread = pthread_self();


#ifdef PM_DEBUG

    /* pthread_equal returns non-zero , if two threads are the same */

    if ( pthread_equal( calling_thread, socket_thread ) != 0 ){
        Debug0("calling thread is socket_thread \n");
    }
    if ( pthread_equal( calling_thread, core_event_thread ) != 0 ){
        Debug0("calling thread is core_event_thread \n");
    }
    if ( pthread_equal( calling_thread, socket_thread ) != 0 ){
        Debug0("calling thread is main_thread \n");
    }
    if ( pthread_equal( calling_thread, signal_waiter_thread ) != 0 ){
        Debug0("calling thread is signal_waiter_thread \n");
    }
#endif /* PM_DEBUG*/

    if ( pthread_equal( calling_thread, *p_thread ) != 0 ){

        /* in case *p_thread is calling thread , NOP */
        /* no operation */
        Debug0(" Calling thread is the same as one to be canceled.\n");
    }
    else{
        
        /* cancel specified thread with proper cleanup handler */
        pthread_cancel(*p_thread);
        
        /* wait for socket_thread to be terminated */
        pthread_join(*p_thread,(void **)&join_status );
    
        RDebug1(2,"join status = %d \n",join_status);
        
        if (join_status != -1){
            fprintf(stderr,MSGSTR(ERR_CANCEL_THREAD,DFLT_ERR_CANCEL_THREAD));
            return PM_ERROR;
        }
    }

    return PM_SUCCESS;
}

/*
 * NAME:      commit_suicide
 *
 * FUNCTION:  calling thread exits with the status -1. 
 *
 * NOTES:  
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_ERROR : processed unsuccessfully.
 */
int 
commit_suicide()  /* :-) */
{
    int join_status;
    pm_system_state_t transition; /* for acknowledge to pm core   */
                                  /* which is issued before pmd's */
                                  /* last thread terminates.      */

#ifdef PM_DEBUG
    pthread_t calling_thread;

    Debug0("commit_suicide \n");
    calling_thread = pthread_self();

    /* pthread_equal returns non-zero , if two threads are the same */

    if ( pthread_equal( calling_thread, socket_thread ) != 0 ){
        Debug0("calling thread is socket_thread \n");
    }
    if ( pthread_equal( calling_thread, core_event_thread ) != 0 ){
        Debug0("calling thread is core_event_thread \n");
    }
    if ( pthread_equal( calling_thread, socket_thread ) != 0 ){
        Debug0("calling thread is main_thread \n");
    }
    if ( pthread_equal( calling_thread, socket_thread ) != 0 ){
        Debug0("calling thread is signal_thread \n");
    }

#endif /* PM_DEBUG */

    /* call transition end to tell pm-core */
    /* that from now on any system-call    */
    /* will not be invoked.                */

    memset(&transition,0,sizeof(transition));

    transition.state = PM_TRANSITION_END;
    transition.event = PM_EVENT_TERMINATE;

    Debug0("now call TRANSITION END\n");
    pm_control_state(PM_CTRL_START_SYSTEM_STATE, 
		     (caddr_t)&transition);


    /* now commit suicide */

    pthread_exit((void *)-1);

    /* NOT REACHED */

    return PM_ERROR;
}



/*
 * NAME:      signal_ignore
 *
 * FUNCTION:  ignore critical signal 
 *
 * NOTES:  this routine is for other threads than signal_waiter_thread
 * 
 * RETURN VALUE DESCRIPTION:
 *        PM_SUCCESS : processed successfully.
 *        PM_ERROR   : processed unsuccessfully.
 */
int
signal_ignore(sigset_t *p_signal_set)
{
    if (p_signal_set == NULL){
	return PM_ERROR;
    }

    sigemptyset(p_signal_set);      /* empty the set of signals */

    /* block signals during a critical section of the code */
    sigthreadmask(SIG_BLOCK,p_signal_set,NULL);

    return PM_SUCCESS;

}

