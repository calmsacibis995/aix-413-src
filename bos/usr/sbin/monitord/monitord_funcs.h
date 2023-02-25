/* @(#)94	1.1  src/bos/usr/sbin/monitord/monitord_funcs.h, netls, bos411, 9428A410j 4/2/94 14:48:07  */
/*
 *   COMPONENT_NAME: netls
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _MONITORD_FUNCS_H
#define _MONITORD_FUNCS_H

/****************************************************************************
* Function prototypes:  monitord.c                                          *
*****************************************************************************/
int check_args(int, char *[]);		/* checks monitord arguments	    */
long get_saved_hbeat();			/* checks for non-default hbeat     */
void main(int, char *[]);		/* main program			    */


/****************************************************************************
* Function prototypes:  monitord_svr.c                                      *
****************************************************************************/
void actions(int, pid_t);		/* Handles a login license request  */
void add_pid(pid_t);			/* Adds a licensed login process    */
void daemon_start();			/* Sets up a process as a daemon    */
int  find_index(pid_t);			/* Finds a login in the pid_list    */
void init_pid_list();			/* Initializes the pid list         */
#ifdef NETLS_DEBUG
void print_pids();			/* Prints the licensed login info   */
#endif
void remove_pid(pid_t);			/* Removes a licensed login process */
void server(int);			/* Starts up monitord as a daemon   */
void sig_child();			/* Handles SIGCLD for monitord      */
void signal_failure(pid_t);		/* Signals no login license avail   */
void signal_success(pid_t);		/* Signals license login available  */


/****************************************************************************
* Function prototypes:  monitord_netls.c                                    *
****************************************************************************/
void continue_ok();                     /* Starts the heartbeat             */
void drop_license(nls_lic_trans_id_t);  /* Returns a license to the pool    */
int  get_license();                     /* Get a lic, enter queues if none  */
void gettime(char *);                   /* Gets the current time            */
void heartbeat();                       /* Checks in with server before     */
                                        /*  chk_per expires                 */
void initialize_netls();                /* Locate lic servers, get a job id */
                                        /*  available                       */
nls_lic_trans_id_t lic_trans();         /* Gets a license and returns the   */
                                        /*  transaction id                  */
void log_netls(int, long, char *);      /* Logs netls errors                */
void log_softstop();                    /* Logs softstops                   */
void log_sys(int, char *);              /* Logs system errors               */
void quit_clean(int);                   /* Cleans up and exits              */
void quit_signal();                     /* Logs an error and exits for      */
                                        /*  caught signals                  */
void set_quit_handler();                /* Set handlers for cleanup on exit */


#endif  /* _MONITORD_FUNCS_H */
