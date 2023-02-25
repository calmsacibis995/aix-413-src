/* @(#)00       1.17  src/rspc/usr/bin/pmd/pmd.h, pwrcmd, rspc41J, 9524E_all 6/15/95 08:03:21 */
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

/***********************************************************/
/**         Power Management Daemon header file          ***/
/***********************************************************/

#ifndef _H_PMD
#define _H_PMD

#define _THREAD_SAFE 
/*****************/
/* Include Files */
/*****************/

#include <pthread.h>  /* this must be the first include file. */
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <sys/types.h>

#include <sys/dir.h>
#include <dirent.h>

#include <unistd.h>
#include <sys/time.h>
#include <sys/sleep.h>     

#include <signal.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include <nl_types.h>
#include <locale.h>

#include <procinfo.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <utmp.h>
#include <pwd.h>

#include <sys/resource.h>

#include <sys/lock_def.h>  
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/syspest.h>
#include <errno.h>

#include <sys/pm.h>
#include <pmlib.h>
#include <pmlib_internal.h>

#include <sys/lvdd.h>
#include <lvm.h>

#include "pmd_msg.h"
#include "pmd_msg_default.h"

#include "pmsptpnp.h"
/***********************/
/*   Define Macros     */
/***********************/

/*---------------------*/
/* Defines misc macro  */
/*---------------------*/

#define MSGSTR(msgnum, string) catgets(catd, MS_PMD, msgnum, string)

#define Free(pointer) if (pointer != NULL){\
                         free((caddr_t)pointer);\
                         pointer=NULL;\
                      }

#define Free_libc(pointer) if (pointer!=NULL){\
                              lvm_freebuf((caddr_t)pointer);\
                              pointer=NULL;\
                           }

/* NOTE: Since the libc_r is linked to this program, */
/*       we cannot be too careful to free memory     */
/*       which is allocated inside the libc library. */
/*       If it is the case, Free_libc() should be    */
/*       called, otherwise memoryleak would occur.   */
/*       That is, we must ensure that liberation of  */
/*       memory is done by the same library as it is */
/*       allocated by.                               */
/*       lvm_freebuf is in use to free the memory    */
/*       allocated by libc for the release 41J,      */
/*       for only shared libraries have the loader   */
/*       section to specify the library, and there   */
/*       seems no shared libraries but liblvm        */
/*       which just calls free with libc.            */
/*       ( refer to the defect 180042 for this. )    */

#define Mutex_Lock( p_mutex ) \
    if ( pthread_mutex_lock( p_mutex ) != 0 ) \
       fprintf(stderr,MSGSTR(ERR_MUTEX_LOCK,DFLT_ERR_MUTEX_LOCK))

#define Mutex_Unlock( p_mutex ) \
    if ( pthread_mutex_unlock( p_mutex ) != 0 ) \
       fprintf(stderr,MSGSTR(ERR_MUTEX_UNLOCK,DFLT_ERR_MUTEX_UNLOCK))

#define Cond_Wait( p_condvar, p_condmutex ) \
    if ( pthread_cond_wait(p_condvar,p_condmutex) != 0) \
       fprintf(stderr,MSGSTR(ERR_COND_WAIT,DFLT_ERR_COND_WAIT))

#define Mutex_Init( p_mutex, p_attr ) \
    if ( pthread_mutex_init(p_mutex,p_attr) != 0) \
       fprintf(stderr,MSGSTR(ERR_MUTEX_INIT,DFLT_ERR_MUTEX_INIT))

#define Cond_Init( p_condvar, p_attr ) \
    if ( pthread_cond_init(p_condvar,p_attr) != 0) \
       fprintf(stderr,MSGSTR(ERR_COND_INIT,DFLT_ERR_COND_INIT))
       
#define Thread_Create( p_thread, p_attr , p_routine, arg ) \
    if (pthread_create( p_thread, p_attr , p_routine, arg ) != 0) \
       fprintf(stderr,MSGSTR(ERR_CREATE_THREAD,DFLT_ERR_CREATE_THREAD))

#define Attr_Init( p_attr ) \
    if (pthread_attr_init( p_attr ) != 0) \
       fprintf(stderr,MSGSTR(ERR_ATTR_INIT,DFLT_ERR_ATTR_INIT))

#define Attr_Destroy( p_attr ) \
    if (pthread_attr_destroy( p_attr ) != 0) \
       fprintf(stderr,MSGSTR(ERR_ATTR_DSTRY,DFLT_ERR_ATTR_DSTRY))

#define Attr_Undetached( p_attr  ) \
   if (pthread_attr_setdetachstate( p_attr, PTHREAD_CREATE_UNDETACHED ) != 0) \
       fprintf(stderr,MSGSTR(ERR_ATTR_DETACH,DFLT_ERR_ATTR_DETACH))


#define PROCSIZE sizeof(struct procinfo)
#define U_SIZE   sizeof(struct userinfo)

#define PMD_VERSION  "0.9.1"             /* version of pm daemon           */

#define PMD_FLAG_NONE -1                 /* state transition flag          */
#define PMD_FLAG_BUSY -2

#define PMD_INVOKE_EVENT_QUERY  1
#define PMD_INVOKE_CONFIRMATION 2

#define PMD_RETRY_PERIOD 20
#define PMD_RETRY_INTERVAL 1

#define PMD_BLOCKED -1
#define PMD_PASSED  0

#define PMD_REJECT  -1
#define PMD_UNANIMOUS_OK 0
#define PMD_PARTIAL_OK 1

#define PMD_CONTINUE_LOOP   0
#define PMD_PROCEED         1
#define PMD_TIMEOUT         2
#define PMD_ERROR           -1

#define PMD_RESPONSE_TIMEOUT 10

#define PMD_LOCKFILE_OK   0
#define PMD_LOCKFILE_PH1  1 
#define PMD_LOCKFILE_PH2  2

#define PMD_SLEEP   1
#define PMD_RESUME  2

#define PMD_WITHOUT_LOCK   0
#define PMD_WITH_LOCK      1

#define PMD_LEVEL_NONE            0
#define PMD_LEVEL_FULL_ON         1
#define PMD_LEVEL_ENABLE          2
#define PMD_LEVEL_STANDBY         3
#define PMD_LEVEL_SUSPEND         4
#define PMD_LEVEL_HIBERNATION     5
#define PMD_LEVEL_SHUTDOWN        6
#define PMD_LEVEL_TERMINATE       7

#define PMD_P2_EVENT_NUMBER       32
#define PMD_TOTAL_EVENT_NUMBER    (PMD_P2_EVENT_NUMBER + 2)
                                  /* there are 2 events for compatibility */
                                  /* to phase1 request */

#define EVENT_HASH(event) ((event<PMD_P2_EVENT_NUMBER)? \
    event : (event - PM_EVENT_P1SWRQ_STANDBY + PMD_P2_EVENT_NUMBER))

                                  /* event hash table    */
                                  /* 0 - 31  --->  0 - 31 */
                                  /*100 - 101 ---> 32 - 33*/

#define EVENT_REVHASH(hashed_event) ((hashed_event<PMD_P2_EVENT_NUMBER)? \
 hashed_event : (hashed_event + PM_EVENT_P1SWRQ_STANDBY - PMD_P2_EVENT_NUMBER))

                                  /* event hash table        */
                                  /*  0 - 31  --->   0 - 31  */
                                  /* 32 - 33  ---> 100 - 101 */
#define PMD_EVENT_QUEUE_LENGTH 16

#define PMD_HIB_VOL_NAME "pmhibernation"

#define PMD_SHELLSCRIPT_SLEEP  "/etc/pmd.suspend"
#define PMD_SHELLSCRIPT_RESUME "/etc/pmd.resume"

#define PMD_TERMINATE_WITHOUT_RMDEV   0
#define PMD_TERMINATE_WITH_RMDEV      1

/*-----------------------------*/
/* Define Macros for Debugging */
/*-----------------------------*/

#ifdef PM_DEBUG2
#define PMD_UNIXDG_PATH_EV "/tmp/.s.event"
#define UNIXDG_TMP_EV      "/tmp/.s.evXXXX"
#endif /* PM_DEBUG2 */

#ifdef PM_DEBUG
#define Debug0(format)                fprintf(stderr,format)
#define Debug1(format,arg1)           fprintf(stderr,format,arg1)
#define Debug2(format,arg1,arg2)      fprintf(stderr,format,arg1,arg2)
#define Debug3(format,arg1,arg2,arg3) fprintf(stderr,format,arg1,arg2,arg3)
#else 
#define Debug0(format)                
#define Debug1(format,arg1)           
#define Debug2(format,arg1,arg2)      
#define Debug3(format,arg1,arg2,arg3) 
#endif

#define RDebug0(number,format) if (pmd_parameters.reserved1 & (number)) \
    fprintf(stderr,format)
#define RDebug1(number,format,arg1) if (pmd_parameters.reserved1 & (number)) \
    fprintf(stderr,format,arg1)
#define RDebug2(number,format,arg1,arg2) \
if (pmd_parameters.reserved1 & (number)) \
    fprintf(stderr,format,arg1,arg2)
#define RDebug3(number,format,arg1,arg2,arg3)\
 if (pmd_parameters.reserved1 & (number)) \
    fprintf(stderr,format,arg1,arg2,arg3)

/****************************/
/*   external variables     */
/****************************/

extern int errno;

/*************************/
/*   type definition     */
/*************************/

typedef struct _clientinfo{
    pid_t pid;
    uid_t uid;
    uid_t euid;
    int event_query_count;
    int confirmation_count;
    struct _clientinfo *next;
    struct _clientinfo *prev;
} clientinfo_t;

typedef struct _pmd_parameters{

/* system data for pm daemon */
   pm_system_state_t pm_present_state;
   pm_system_state_t pm_target_state;

   int   system_support;         /* supported state */
   int   firmware_support;     /* firmware residual data */

/* user data for pm daemon */

   int   system_idle_action;     /* system idle action */
   int   lid_close_action;       /* lid close action */
   int   main_switch_action;     /* main power switch action */
   int   low_battery_action;     /* low battery action */
   int   specified_time_action;  /* specified time action, sus or hiber */
   int   resume_passwd;          /* enable/disable resume password */
   int   kill_lft_session;       /* continue/kill LFT session */
   int   kill_tty_session;       /* continue/kill TTY session */
   int   permission;             /* permitted state by superuser */ 

/* user data for core */

   int  system_idle_time;          /* system idle time in seconds */
   int  pm_beep;                   /* enable/disable beep */
   int  ringing_resume;            /* enable/disable ringing resume */
   time_t  resume_time;            /* specified time to resume */
   int  sus_to_hiber;              /* duration from hiber to resume */
   time_t  specified_time;         /* specified time to sus or hiber */
   int  kill_syncd;                /* continue kill syncd */
   int  reserved1;                 /* for runtime debugging (for pmd) */
   int  reserved2;                 /* for runtime debugging (for pmcore)*/

} pmd_parameters_t;


typedef struct _event_queue_t{
    int event;
    int state;
    int priority;
} event_queue_t;

/**********************/
/* Function prototype */
/**********************/

/* For main thread */

void cleanup_for_main_thread(void *);
int check_lockfiles();
int pmd_lockfile();
int main_initialize();
int daemon_initialize();
void main_loop();
int main_thread_sleep();
int process_state_change();
int set_pmlib_latest_error(int);
int set_target_state();
int set_target_state_by_event();
int set_target_state_by_request();
int state_change_1st_pass();
int state_change_2nd_pass();
int create_hibernation_volume();
int examine_hibernation_volume();
int remove_hibernation_volume();
int file_exists(char*);
int exec_shellscript(int);
int process_shutdown();
int check_processes();
int getdev(char *);
int cleanup_for_check_process();
int kill_syncd();
int revive_syncd();
int kill_lft_session();
int kill_tty_session();
int lock_screen();
int check_program(char*,char*,pid_t,pid_t*,uid_t*);
int check_utmp(pid_t);
struct procinfo *get_process_data(long *);
int run_xlock(uid_t);
int run_LockDisplay(uid_t);
int set_completion_notice();
int reset_state_change_process_flag();
int invoke_query(int);
int set_count_flag(int);
int set_pmlib_event_entry(int);
int process_event_notice();
int state2level(int);
int set_event_priority(int);
int state2distance(int,int);
int create_mutexes();
int create_condition_variables();
int create_major_threads();       
int create_signal_waiter_thread();       
int check_pmcore_configured();
int set_syscall_busy();
int reset_syscall_busy();

/* For socket thread */

void *socket_interface(void *);
void socket_loop();
int create_socket();
int close_socket();
void cleanup_for_socket_thread(void *);
int receive_data();
int process_data();
int check_version();
int switch_api_operations();
int process_request_state();
int reset_request_entry_count(clientinfo_t *);
int count_request_entry(int,pid_t,clientinfo_t *);
int request_count_reached(int,clientinfo_t *);
int check_state_change_process_flag();
int check_request_state_permission();
int process_get_event_notice();
int process_request_battery();
int query_battery();
int discharge_battery();
int process_request_parameter();
int ddlist_filter(pmlib_internal_device_names_t *);
int check_superuser_privilege();
int process_register_application();
int register_application(clientinfo_t *);
clientinfo_t *search_entry(clientinfo_t *,pid_t,int);
int broadcast_signal(clientinfo_t *);
int unregister_application(clientinfo_t *,int);
int send_data();
int wakeup_main_thread(int *, pthread_cond_t *,pthread_mutex_t *);
int pm2pmlib(int);
int pmlib2pm(int);
int is_pm_state(int);
int is_pm_action_state(int);
int is_pm_permission(int);
int is_pm_system_support(int);
int is_pmlib_state(int);
int is_pmlib_permission(int);
int is_pmlib_system_support(int);
int is_pmlib_onoff(int);
int is_boolean(int);
int is_timer(int);
int is_specified_time(int);
int is_pm_supported(int);
int pmd2userparam(pmd_parameters_t *,pm_parameters_t *);
int user2pmdparam(pm_parameters_t *,pmd_parameters_t *);
int pmlibonoff2boolean(int);
int pm2pmlibdevinfo(pm_device_info_t *,pmlib_device_info_t *);
int pm2pmlibbattery(pm_battery_data_t *,pmlib_battery_t *);
char *pmstate2asc(int);           

#ifdef PM_DEBUG
int show_entry(clientinfo_t *);   /* for debugging */
#endif /* PM_DEBUG */

/* For core_event thread */

void *core_event_interface(void *);
int core_event_loop();
int get_core_event();
int check_proper_state_change_by_event(int);
int set_state_by_event(int);
int check_event_queue(int *);
int check_prior_event(int);
int check_misc_event(int);
int flush_event_queue();
int search_event_queue(int);

int display_event_queue();
int display_event_priority_table();

/* For signal_waiter_thread */

void *signal_waiter_interface(void *);
int commit_suicide();                  /* :-) */
int cancel_thread(pthread_t * );
int process_terminate();
void cleanup_for_terminate(void *);
int signal_ignore(sigset_t *);

#ifdef PM_DEBUG

/* in other files */
int check_pm_attribute(int, PM_attribute);
int read_odm_data(pmd_parameters_t *); 
int pm_battery_control(int,struct pm_battery *);
int pm_control_state(int,caddr_t);
int pm_control_parameter(int,caddr_t);
int getpmlv(char *,pm_hibernation_t *);
#endif /* PM_DEBUG */

#endif /* _H_PMD */
