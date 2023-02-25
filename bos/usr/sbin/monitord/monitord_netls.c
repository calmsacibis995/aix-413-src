static char sccsid[] = "@(#)95	1.2  src/bos/usr/sbin/monitord/monitord_netls.c, netls, bos411, 9428A410j 4/6/94 09:42:24";
/*
 *   COMPONENT_NAME: netls
 *
 *   FUNCTIONS: continue_ok
 *		drop_license
 *		get_license
 *		gettime
 *		heartbeat
 *		initialize_netls
 *		lic_trans
 *		log_netls
 *		log_softstop
 *		log_sys
 *		quit_clean
 *		quit_signal
 *		set_quit_handler
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
#include "monitord.h"
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include "ppfm.h" 

/*
 * netls vendor/product identifying values
 */

static nls_vnd_id_t  vid 	= VENDOR_ID;
static long    	     key        = VENDOR_KEY,
               	     prod_id 	= NL_PRODUCT_ID,
               	     vlen       = NL_PRODUCT_VLEN,
                     lic_type   = NETLS_ANY,
                     num_lics   = 1;  
static char    	    *version 	= NL_PRODUCT_VERSION;



/*
 * Variables shared between modules.
 */

extern struct pid_lic pid_list[];	   /* licensed pids and trans ids   */
extern int last_index;			   /* last index in pid_list array  */
extern int pidpipe;                        /* pipe for license requests     */
extern h_interval;                         /* license heartbeat interval    */
extern nl_catd admin_catd;                 /* message catalog               */

/*
 * variables declared static to make them global only to the functions
 * in this file.
 */

static pfm__cleanup_rec    crec;           /* struct for cleanup routine    */
static nls_lic_trans_id_t  lic_trans_id;   /* transaction id for NetLS      */
static nls_lic_annot_t	   lic_annot;      /* annotation for prod lic       */
static nls_job_id_t	   job_id;         /* job id for lic request        */
static status__t	   stat_t;         /* return status of cleanup      */
static nls_time_t	   exp_date,       /* expiration of requested lics  */
                           chk_per;        /* heartbeat interval     */
static int  job_set = 0,                   /* flag if job_id properly set   */
            heart_set = 0,                 /* flag if heartbeat is going    */
            license_granted = LICENSE_NOT_GRANTED; /*  possible values:     */
                                                   /*  LICENSE_NOT_GRANTED  */ 
                                                   /*  LICENSE_GRANTED      */ 


/*
 * NAME:	set_quit_handler
 *
 * FUNCTION: 	Sets up the NCS handlers for abnormal program exits
 *
 * RETURNS:	Nothing
 */
void set_quit_handler()
{
  /*
   * NCS Cleanup
   * The pfm__cleanup routine handles cleaning up NCS after a fatal
   * application error.  This makes sure that any resources in
   * use are released as quickly as possible in the event of a
   * fault in the application.                                  
   */


  /* Tell monitord to ignore signals.  The upcoming pfm__init()      */
  /* call will reset what it needs to for catching any fatal errors, */
  /* and cleaning up NCS.                                            */

  signal(SIGPIPE, SIG_IGN);
  signal(SIGABRT, SIG_IGN);
  signal(SIGEMT, SIG_IGN);
  signal(SIGURG, SIG_IGN);
  signal(SIGSTOP, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGCONT, SIG_IGN);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTTOU, SIG_IGN);
  signal(SIGIO, SIG_IGN);
  signal(SIGXCPU, SIG_IGN);
  signal(SIGXFSZ, SIG_IGN);
  signal(SIGMSG, SIG_IGN);
  signal(SIGWINCH, SIG_IGN);
  signal(SIGPWR, SIG_IGN);
  signal(SIGUSR1, SIG_IGN);
  signal(SIGUSR2, SIG_IGN);
  signal(SIGPROF, SIG_IGN);
  signal(SIGDANGER, SIG_IGN);
  signal(SIGVTALRM, SIG_IGN);
  signal(SIGMIGRATE, SIG_IGN);
  signal(SIGPRE, SIG_IGN);
  signal(SIGVIRT, SIG_IGN);
  signal(SIGGRANT, SIG_IGN);
  signal(SIGRETRACT, SIG_IGN);
  signal(SIGSOUND, SIG_IGN);
  signal(SIGSAK, SIG_IGN);


  /* pfm__init sets the NCS cleanup handler */
  pfm__init(pfm__init_signal_handlers);

  /* pfm__cleanup marks the point to which the stack will be rewound if  */
  /* a fatal error is caught by NCS.  The next line of execution will    */
  /* be the line following the pfm__cleanup call.  When the code jumps   */
  /* to this point, the (stat_t.all != pfm__cleanup_set) will be true,   */
  /* and quit_signal will be called.                                     */

  stat_t = pfm__cleanup(&crec);

  if (stat_t.all != pfm__cleanup_set)
  {
    /* ignore return status of pfm__rls_cleanup because we are */
    /* just going to exit anyway.                              */
    pfm__rls_cleanup(&crec, &stat_t);

    quit_signal();
  }

  /* Set fatal error signals to call quit_signal handler for logging and */
  /* license cleanup.  The call to pfm__cleanup will have also mapped    */
  /* these signals to pfm signals.                                       */
  signal(SIGTERM, quit_signal);
  signal(SIGHUP, quit_signal);
  signal(SIGINT, quit_signal);
  signal(SIGQUIT, quit_signal);
  signal(SIGILL, quit_signal);
  signal(SIGTRAP, quit_signal);
  signal(SIGFPE, quit_signal);
  signal(SIGBUS, quit_signal);
  signal(SIGSEGV, quit_signal);
  signal(SIGSYS, quit_signal);
}


/*
 * NAME:	initialize_netls
 *
 * FUNCTION: 	Establishes contact with the license server, and gets
 *              a jobid for this application execution.  If an error
 *		occurs, then the function will exit, since there is
 *		no point in continuing.
 *
 * RETURNS:	Nothing
 */
void initialize_netls()
{
  long local_status = 0;

  /* initialize netls, and get a job_id */
  netls_init(vid, key, &job_id, &local_status);
  if (local_status == status__ok)
  {
    job_set = 1;
  }
  else
  {
    log_netls(0, local_status, catgets(admin_catd, MONITORD_ADMIN, INIT_FAIL,
              "SERVER INITIALIZATION FAILED\n"));
    log_sys(1, catgets(admin_catd, MONITORD_ADMIN, NO_SERVERS,
    "Monitord could not find any servers with valid AIX licenses installed.\n"));
    quit_clean(-1);
  }

  return;
}


/*
 * NAME:	get_license
 *
 * FUNCTION: 	attempt to get a license.  If a license is granted, then
 *              return success.  If no licenses are available, log the
 *              reason for failure.
 *
 * RETURNS:	LICENSE_GRANTED     if a license was checked out.
 *		LICENSE_NOT_GRANTED if a license was not checked out.
 */
int get_license()  /* return license type */
{
  long local_status = 0;

  /* set the check period to the determined heartbeat interval */
  chk_per = (long) h_interval;

  /* set (license_granted = LICENSE_NOT_GRANTED) as default */
  license_granted = LICENSE_NOT_GRANTED;

  lic_type = NETLS_CONCURRENT;
  /* request a concurrent access license */
  netls_request_license(&job_id, &lic_type, prod_id, version,
        vlen, num_lics, chk_per, &lic_trans_id, &exp_date, 
        lic_annot, &local_status);

  #ifdef NETLS_DEBUG
  printf("concurrent license request status = %d\n", local_status);
  #endif
  if (local_status == status__ok)
    license_granted = LICENSE_GRANTED;
  else
  {
    if (local_status == netls_no_lics)
    {
      lic_type = NETLS_USEONCE;
      /* request a use-once license */
      netls_request_license(&job_id, &lic_type, prod_id, version,
          vlen, num_lics, chk_per, &lic_trans_id, &exp_date, 
          lic_annot, &local_status);
      #ifdef NETLS_DEBUG
      printf("use-once license request status = %d\n", local_status);
      #endif
      if (local_status == status__ok)
        license_granted = LICENSE_GRANTED;
    }
  }

  if (license_granted == LICENSE_NOT_GRANTED)
  {
    #ifdef NETLS_DEBUG
    printf("license request status = %d\n", local_status);
    #endif
    /* if failure is not due to server connection problem */
    if ( (local_status == netls_no_lics) ||
         (local_status == netls_not_enough_lics) ||
         (local_status == netls_lic_not_fnd) )
      log_netls(0, local_status, catgets(admin_catd,MONITORD_ADMIN, 
                NO_LICS, "NO LICENSE AVAILABLE\n"));
    else
      /* failure must have been due to inability to contact server */
      log_netls(0, local_status, catgets(admin_catd, MONITORD_ADMIN, 
                 SERVER_UNAVAIL, "SERVER UNAVAILABLE FOR LICENSE CHECKOUT\n"));
  }
  

  return(license_granted);
}



/*
 * NAME:	heartbeat
 * 
 * FUNCTION:	Checks in with license server, letting server know that the
 *              application is still alive.  The continue_hbeats variable
 *              will keep heartbeats going as long as there is one entry
 *              in the pid_list holding a valid license.
 *
 * RETURNS:	Nothing
 */
void heartbeat()
{
  long local_status = 0;
  int i = 0,
      continue_hbeats = 0;

  signal(SIGALRM, SIG_IGN);

  #ifdef NETLS_DEBUG
  printf("HEARTBEAT!!!\n");
  #endif

  for (i=0; i <= last_index; i++)
  {
    if (pid_list[i].transid >= 0)
    {
      continue_hbeats++;             /* bump up the number of heartbeats   */
      #ifdef NETLS_DEBUG
      printf(" >>heartbeat for %d\n", pid_list[i].pid);
      #endif
      netls_check_license(pid_list[i].transid, chk_per, &local_status);

      if (local_status != status__ok)
      {
	continue_hbeats--;           /* decrement number of heartbeat by 1 */
        log_netls(0, local_status, catgets(admin_catd, MONITORD_ADMIN,
                   HEART_FAIL, "HEARTBEAT FAILED\n"));

        #ifdef NETLS_DEBUG
        printf("\nremove %d from heartbeats...\n", pid_list[i].pid);
        #endif
        pid_list[i].transid = -1;
      }
    }
  }

  if (continue_hbeats > 0)
  {
    alarm(chk_per - 10);
    signal(SIGALRM, heartbeat);
  }
  else 
  {
    #ifdef NETLS_DEBUG
    printf("no pids to heartbeat\n");
    #endif
    heart_set = 0;                       /* reset flag to no heartbeat */
  }

  return;
}



/*
 * NAME:	continue_ok
 *
 * FUNCTION:	Display a success message, and setup the heartbeat to 
 *              periodically check in with the server.  The heartbeat
 *		is set up to get a 10 second head start to allow for
 *              a slow network.
 *
 * RETURNS:	Nothing
 */
void continue_ok()
{
  #ifdef NETLS_DEBUG
  printf("\nLicense Granted.\n");
  #endif

  /* only want to set heartbeat if not already done. */
  if (heart_set == 0)
  {
    alarm(chk_per - 10);
    signal(SIGALRM, heartbeat);
    heart_set = 1;
  }
}


/*
 * NAME:	drop_license
 *
 * FUNCTION:	Releases a license held for a specified transaction id.
 *		The transaction id corresponds to a licensed login process.
 *              If an error is returned for the release action, log the
 *		reason for the error.
 *
 * RETURNS:	Nothing
 */
void drop_license(nls_lic_trans_id_t trans_id)
{
  long local_status = 0;

  netls_release_license(trans_id, &local_status);
  if (local_status != status__ok)
  {
    if ((local_status == netls_lic_not_fnd) ||
        (local_status == netls_not_bound) ||
        (local_status == netls_non_matching_tid))
      log_netls(0, local_status, catgets(admin_catd, MONITORD_ADMIN, 
                BAD_LIC_RELEASE, "INVALID LICENSE RELEASE\n"));
    else
    {
      #ifdef NETLS_DEBUG
      printf("%x", local_status);
      #endif
      log_netls(0, local_status, catgets(admin_catd, MONITORD_ADMIN,
		NO_SVR_RELEASE, "SERVER UNAVAILABLE FOR LICENSE RELEASE\n"));
    }
  }
}


/*
 * NAME:	gettime
 * 
 * FUNCTION:	Puts a human-readable string containing the current time into
 *           	the timestring parameter.
 *
 * RETURNS:	Nothing
 */
void gettime(char *timestring)
{
  time_t now;

  time(&now);
  strcpy(timestring, ctime(&now));
  timestring[strlen(timestring) - 1] = '\0';

  return;
}



/* 
 * NAME:	log_netls
 *
 * FUNCTION: 	Fetches the netls error message for an error code, and
 * 		combines it with the "viol" parameter, which is a contextual
 * 		description of the violation.  The error is then written
 *		to the monitord error log.
 *
 * RETURNS:	Nothing
 */
void log_netls(int to_console, long proc_stat, char *viol)
{
  char	msg[200],       /* msg corresponding to the error code        */
        log_msg[250];   /* err msg and some other logging information */

  int	log_msgl;       /* parameters for api calls                   */
  int   pnl = 1;
  long  msgl;
  int lockptr;
  int monitord_log;     /* error log for monitord        */
  int errout;		/* file descriptor for writing to the console */

  netls_get_err_msg(" ", pnl, proc_stat, msg, &msgl);

  gettime(log_msg);

  strcat(log_msg, "  ");
  strcat(log_msg, viol);
  strcat(log_msg, msg);
  strcat(log_msg, "\n");
  log_msgl = strlen(log_msg);

  #ifdef NETLS_DEBUG
  printf("%s\n", msg);
  #endif

  monitord_log = open(NETLS_ERROR_LOG, O_WRONLY|O_CREAT|O_APPEND, 0600);

  /* log the error to the monitord error log */
  if (monitord_log >= 0)
  {

    if ((lockptr = dup(monitord_log)) >= 0)
    {
      lseek(lockptr, 0L, SEEK_SET);
      lockf(lockptr, F_LOCK, 0L);
    }

    write(monitord_log, log_msg, log_msgl);

    if (lockptr >= 0)
    {
      lseek(lockptr, 0L, SEEK_SET);
      lockf(lockptr, F_ULOCK, 0L);
      close(lockptr);
    }

    close(monitord_log);
  }

  if (to_console > 0)
  {
    errout = open("/dev/console", O_WRONLY|O_APPEND, 0600);
    if (errout >= 0)
    {
      write(errout, log_msg, strlen(log_msg));
      close(errout);
    }
  }

  return;
}



/*
 * NAME:	lic_trans
 *
 * FUNCTION:	Calls get_license to try to check out a license.  If the
 *	        call is successful, then the transaction id for the license
 *	        is returned so it can be tracked by the calling routine.
 *
 * RETURNS:	The license transaction id of the checked-out license.
 */
nls_lic_trans_id_t lic_trans()
{
  nls_lic_trans_id_t trans_id = -1;
  int  license_ok = LICENSE_NOT_GRANTED;     /* status of license request   */

  license_ok = get_license();                /* attempt to get a license    */

  switch(license_ok)
  {
    case LICENSE_GRANTED  : continue_ok();   /* setup the heartbeat         */
                            trans_id = lic_trans_id;

                  default : break;
  }

  return(trans_id);
}


/*
 * NAME:	quit_clean
 *
 * FUNCTION:	Executes netls_cleanup and exits.  This function should
 *              be called in place of exit() to ensure that licenses
 *              are properly returned.  This function also removes 
 *              the communcications pipe and the lock file, and closes
 *		the message catalog.  The input parameter for this 
 *		function is passed as the return value for exit().
 *
 * RETURNS:	Nothing
 */
void quit_clean(int ex_ret)
{
  long local_status = 0;

  /* release any licenses held by this jobid, but only if the licenses are */
  /* still valid.                                                          */
  if ( (job_set == 1) && (heart_set == 1) )
  {
    #ifdef NETLS_DEBUG
    printf("call netls_cleanup.\n");
    #endif
    netls_cleanup(&job_id, &local_status);   
    if (local_status != status__ok)         
      log_netls(1, local_status, catgets(admin_catd, MONITORD_ADMIN, NOCLEAN,
                "CLEANUP FAILED -- LICENSES MAY STILL BE HELD\n"));
    job_set = 0;
  }

  #ifdef NETLS_DEBUG
  printf("remove lock file\n");
  #endif

  /* ignore error.  admin can remove the file by hand. */
  unlink(LOCK_FILE);

  #ifdef NETLS_DEBUG
  printf("remove pipe\n");
  #endif

  /* ignore errors.  pipe will be re-created at next startup. */
  close(pidpipe);
  unlink(MONITORD_PIPE);

  catclose(admin_catd);
  exit(ex_ret);
}



/*
 * NAME:	log_sys
 *
 * FUNCTION:	log any errors that are not the result of a netls failure.
 *	        If the to_console parameter is greater than zero, then the
 *	        error message will go to the console also.  Writes to the
 *	        log file are locked since the libs routine might also be
 *	        writing to the log file for timeouts.  These errors are
 *	        not written to the regular netls log.
 *
 * RETURNS:	Nothing
 */
void log_sys(int to_console, char *failmsg)
{
  char	log_msg[250];   /* err msg and some other logging information */

  int	log_msgl;       /* parameters for api calls                   */
  int lockptr;
  int errout;
  int monitord_log;     /* error log for monitord        */

  gettime(log_msg);

  strcat(log_msg, "  ");
  strcat(log_msg, "MONITORD FAILURE\n : ");
  strcat(log_msg, failmsg);
  log_msgl= strlen(log_msg);

  #ifdef NETLS_DEBUG
  printf("%s\n", log_msg);
  #endif

  monitord_log = open(NETLS_ERROR_LOG, O_WRONLY|O_CREAT|O_APPEND, 0600);
  if (monitord_log >= 0)
  {

    if ((lockptr = dup(monitord_log)) >= 0)
    {
      lseek(lockptr, 0L, SEEK_SET);
      lockf(lockptr, F_LOCK, 0L);
    }

    write(monitord_log, log_msg, log_msgl);

    if (lockptr >= 0)
    {
      lseek(lockptr, 0L, SEEK_SET);
      lockf(lockptr, F_ULOCK, 0L);
      close(lockptr);
    }

    close(monitord_log);
    close(lockptr);
  }

  if (to_console > 0)
  {
    errout = open("/dev/console", O_WRONLY|O_APPEND, 0600);
    if (errout >= 0)
    {
      write(errout, log_msg, strlen(log_msg));
      close(errout);
    }
  }

  return;
}


/*
 * NAME:	log_softstop
 *
 * FUNCTION:	Writes the softstop occurrence to the monitord log.
 *
 * RETURNS: 	Nothing
 */
void log_softstop()
{
  char log_msg[250];   /* err msg and some other logging information */

  int log_msgl;       /* parameters for api calls                   */
  int lockptr;
  int monitord_log;     /* error log for monitord        */

  gettime(log_msg);

  strcat(log_msg, "  ");
  strcat(log_msg, catgets(admin_catd, MONITORD_ADMIN, REQUEST_FAILED,
         "MONITORD WAS UNABLE TO OBTAIN A USER LOGIN LICENSE\n"));
  strcat(log_msg, catgets(admin_catd, MONITORD_ADMIN, SOFT_POLICY,
         " : Softstop policy.  Login allowed.\n"));
  log_msgl= strlen(log_msg);

  #ifdef NETLS_DEBUG
  printf("%s\n", log_msg);
  #endif

  monitord_log = open(NETLS_ERROR_LOG, O_WRONLY|O_CREAT|O_APPEND, 0600);
  if (monitord_log >= 0)
  {

    if ((lockptr = dup(monitord_log)) >= 0)
    {
      lseek(lockptr, 0L, SEEK_SET);
      lockf(lockptr, F_LOCK, 0L);
    }

    write(monitord_log, log_msg, log_msgl);

    if (lockptr >= 0)
    {
      lseek(lockptr, 0L, SEEK_SET);
      lockf(lockptr, F_ULOCK, 0L);
      close(lockptr);
    }

    close(monitord_log);
  }

  return;
}



/*
 * NAME:	quit_signal
 *
 * FUNCTION:	Writes to the log file and shuts down monitord when a 
 * 		signal is received.  Also tells log_sys() to write the
 *		message to the console to notify the user.
 *
 * RETURNS:	Nothing
 */
void quit_signal()
{
  log_sys(1, catgets(admin_catd, MONITORD_ADMIN, SIG_CAUGHT,
          "A signal was caught by monitord.  Monitord shutting down.\n"));
  quit_clean(-1);
}

