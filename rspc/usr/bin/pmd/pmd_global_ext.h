/* @(#)05       1.3  src/rspc/usr/bin/pmd/pmd_global_ext.h, pwrcmd, rspc41J, 9520A_all 5/15/95 10:35:47 */
/*
 * COMPONENT_NAME: PWRCMD
 *
 * FUNCTIONS: Power Management Daemon
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

#ifndef _H_PMD_GLOBAL_EXT
#define _H_PMD_GLOBAL_EXT

/*------------------*/
/* Global variables */
/*------------------*/

/* spcific paramerters for power management daemon */

extern pmd_parameters_t pmd_parameters;   

/* message catlog handle */

extern nl_catd   catd;    

/* event priority table */

extern int event_priority_table[PMD_TOTAL_EVENT_NUMBER];

/* event queue */

extern event_queue_t event_queue[ PMD_EVENT_QUEUE_LENGTH ];
extern int event_queue_index;

/* list of the information of clients */

extern clientinfo_t *pclientinfo_head;
extern int client_number;

/* the latest error to be sent to PM library */

extern pmlib_state_t pmlib_latest_error;  /* used for query-error */
 
/* the numbers of queried/confirmed applications */

extern int event_queried_application;
extern int confirmed_application;

/* synchronization variable for the creation of threads */

extern int thread_birth;

/* Threads ( initial thread is main_thread ) */

extern pthread_t main_thread;          /* initial thread */
extern pthread_t signal_waiter_thread; /* sub-thread created by main_thread */
extern pthread_t core_event_thread;    /* sub-thread created by main_thread */
extern pthread_t socket_thread;        /* sub-thread created by main_thread */

/* MUlTiple EXclusives for common resource*/

extern pthread_mutex_t state_change_cond_mutex;
extern pthread_mutex_t response_cond_mutex;    
extern pthread_mutex_t event_flag_mutex; 
extern pthread_mutex_t event_entry_mutex;
extern pthread_mutex_t request_flag_mutex; 
extern pthread_mutex_t request_entry_mutex;     
extern pthread_mutex_t pmd_parameters_mutex;    
extern pthread_mutex_t pmlib_latest_error_mutex;
extern pthread_mutex_t pmlib_event_entry_mutex; 
extern pthread_mutex_t battery_mutex;      
extern pthread_mutex_t count_event_query_flag_mutex;   
extern pthread_mutex_t count_confirmation_flag_mutex;   
extern pthread_mutex_t client_number_mutex;   
extern pthread_mutex_t application_answer_mutex;   
extern pthread_mutex_t client_chain_mutex;   
extern pthread_mutex_t prior_event_flag_mutex;   
extern pthread_mutex_t event_queue_mutex;   
extern pthread_mutex_t event_priority_table_mutex;   
extern pthread_mutex_t syscall_busy_mutex;   
extern pthread_mutex_t thread_birth_mutex;   

/* condition variables for thread synchronization */

extern pthread_cond_t  state_change_condvar;    /* for state_change */
extern pthread_cond_t  response_condvar;        /* for timedwait    */ 
                                      
/* predicate for condition waiting */

extern int state_change_predicate;      /* for state_change */
extern int response_predicate;          /* for timedwait    */


/* Flags */

extern int event_flag;/* Event flag */
                      /* PMD_FLAG_BUSY : event_flag change disabled */
                      /* PMD_FLAG_NONE : no event */
                      /* PM_EVENT_XXXX : aroused event */

extern int request_flag; /* Request flag */      
                         /* PMD_FLAG_BUSY : request_flag disabled */
                         /* PMD_FLAG_NONE : no request */
                         /* PM_EVENT_XXXX : aroused event */

extern int count_confirmation_flag ;
extern int count_event_query_flag ;
extern int prior_event_flag ;

extern int initiate_by_event;
extern int initiate_by_request;

extern int syscall_busy;

/* entry variables */

extern int event_entry;       /* Hardware event entry aroused from pm-core */
extern int request_entry;     /* Software request from pm-library */
extern int pmlib_event_entry; /* PMLIB_EVENT_XXX event entry */

/* misc variables */

extern int battery_toggled;          /* True : battery supplier changed */
                                     /* False: battery supplier unchanged */
extern int battery_mode;
extern int low_battery;      /* True : PM_EVENT_LOW_BATTERY aroused */
                             /* False: normal */

extern int appl_answer;      /* used between main thread and apl event thread */
extern int appl_pid;         /* used for error caused application's pid */

#endif /* _H_PMD_GLOBAL_EXT */
