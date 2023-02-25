/* @(#)06       1.3  src/rspc/usr/bin/pmd/pmd_global_var.h, pwrcmd, rspc41J, 9520A_all 5/15/95 10:35:45 */
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

#ifndef _H_PMD_GLOBAL_VAR
#define _H_PMD_GLOBAL_VAR

/*------------------*/
/* Global variables */
/*------------------*/

/* spcific paramerters for power management daemon */

pmd_parameters_t pmd_parameters;   

/* message catlog handle */

nl_catd   catd;    

/* event priority table */

int event_priority_table[PMD_TOTAL_EVENT_NUMBER];

/* event queue */

event_queue_t event_queue[ PMD_EVENT_QUEUE_LENGTH ] = 
{
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0},
	{0,0,0},{0,0,0},{0,0,0},{0,0,0}
};
int event_queue_index=0;

/* list of the information of clients */

clientinfo_t *pclientinfo_head = NULL;
int client_number = 0;

/* the latest error to be sent to PM library */

pmlib_state_t pmlib_latest_error;  /* used for query-error */
 
/* the numbers of queried/confirmed applications */

int event_queried_application = 0;
int confirmed_application = 0;

/* synchronization variable for the creation of threads */

int thread_birth = 0;

/* Threads ( initial thread is main_thread ) */

pthread_t main_thread;          /* initial thread */
pthread_t signal_waiter_thread; /* sub-thread created by main_thread */
pthread_t core_event_thread;    /* sub-thread created by main_thread */
pthread_t socket_thread;        /* sub-thread created by main_thread */

/* MUlTiple EXclusives for common resource*/

pthread_mutex_t state_change_cond_mutex;
pthread_mutex_t response_cond_mutex;    
pthread_mutex_t event_flag_mutex; 
pthread_mutex_t event_entry_mutex;
pthread_mutex_t request_flag_mutex; 
pthread_mutex_t request_entry_mutex;     
pthread_mutex_t pmd_parameters_mutex;    
pthread_mutex_t pmlib_latest_error_mutex;
pthread_mutex_t pmlib_event_entry_mutex; 
pthread_mutex_t battery_mutex;      
pthread_mutex_t count_event_query_flag_mutex;   
pthread_mutex_t count_confirmation_flag_mutex;   
pthread_mutex_t client_number_mutex;   
pthread_mutex_t application_answer_mutex;   
pthread_mutex_t client_chain_mutex;   
pthread_mutex_t prior_event_flag_mutex;   
pthread_mutex_t event_queue_mutex;   
pthread_mutex_t event_priority_table_mutex;   
pthread_mutex_t syscall_busy_mutex;   
pthread_mutex_t thread_birth_mutex;   

/* condition variables for thread synchronization */

pthread_cond_t  state_change_condvar;    /* for state_change */
pthread_cond_t  response_condvar;        /* for timedwait    */ 
                                      
/* predicate for condition waiting */

int state_change_predicate = FALSE;      /* for state_change */
int response_predicate = FALSE;          /* for timedwait    */


/* Flags */

int event_flag = PMD_FLAG_NONE;       /* Event flag */
                      /* PMD_FLAG_BUSY : event_flag change disabled */
                      /* PMD_FLAG_NONE : no event */
                      /* PM_EVENT_XXXX : aroused event */

int request_flag = PMD_FLAG_NONE;     /* Request flag */      
                      /* PMD_FLAG_BUSY : request_flag disabled */
                      /* PMD_FLAG_NONE : no request */
                      /* PM_EVENT_XXXX : aroused event */

int count_confirmation_flag = FALSE;  
int count_event_query_flag = FALSE;   
int prior_event_flag = FALSE;

int initiate_by_event   = FALSE;
int initiate_by_request = FALSE;

int syscall_busy = FALSE;

/* entry variables */

int event_entry = PM_NONE;       /* Hardware event entry aroused from pm-core */
int request_entry = PM_NONE;     /* Software request from pm-library */
int pmlib_event_entry = PMLIB_EVENT_NONE; /* PMLIB_EVENT_XXX event entry */

/* misc variables */

int battery_toggled = FALSE;  /* True : battery supplier changed */
                              /* False: battery supplier unchanged */
int battery_mode = PM_AC;     /* AC / DC */
int low_battery = FALSE;      /* True : PM_EVENT_LOW_BATTERY aroused */
                              /* False: normal */

int appl_answer = PMD_UNANIMOUS_OK;
                      /* used between main thread and apl event thread */
int appl_pid = 0;     /* used for error caused application's pid */

#endif /* _H_PMD_GLOBAL_VAR */
