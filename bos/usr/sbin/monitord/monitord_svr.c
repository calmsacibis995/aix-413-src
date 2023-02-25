static char sccsid[] = "@(#)96	1.1  src/bos/usr/sbin/monitord/monitord_svr.c, netls, bos411, 9428A410j 4/2/94 14:49:22";
/*
 *   COMPONENT_NAME: netls
 *
 *   FUNCTIONS: actions
 *		add_pid
 *		daemon_start
 *		find_index
 *		init_pid_list
 *		print_pids
 *		remove_pid
 *		server
 *		sig_child
 *		signal_failure
 *		signal_success
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
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <stdio.h>
#include <procinfo.h>
#include <errno.h>
#include "monitord.h"
#include <sys/file.h>
#include <sys/ioctl.h>


extern int softstop;			/* softstop flag               */
extern nl_catd admin_catd; 		/* Message catalog pointer     */

/* 
 * Global Variables for monitord.
 */

struct pid_lic pid_list[MAX_PIDS];	/* list of licensed processes  */
int last_index = -1;			/* last used index in pid_list */
int h_interval = 0;	                /* heartbeat interval          */
int pidpipe;				/* named pipe for lic requests */


/*
 * NAME:	server
 *
 * FUNCTION: 	Call daemon_start() to setup and continue as a daemon process.
 *          	Perform any setup required for global variables and data 
 *          	structures.  The heartbeat interval was previously determined
 *          	and is passed in as a parameter to this function to be set
 *          	globally.
 *
 * RETURNS:	Nothing
 */
void server(int hbeat_interval)
{
  int read_rc = 0;
  struct monitord_request client_request;

  #ifdef NETLS_DEBUG
  printf("I'm gonna spawn a daemon!\n");
  #endif

  daemon_start();			/* setup as a daemon,            */

  init_pid_list();			/* initialize data structures    */

  h_interval = hbeat_interval;          /* set global heartbeat interval */

  #ifdef NETLS_DEBUG
  printf("heartbeat interval set to %d\n", h_interval);
  #endif

  set_quit_handler();			  /* setup a handler to cleanup  */
                                          /* NetLS when interrupts rec'd */
                                          /* Any SIG_IGN must be put     */
                                          /* inside this function        */ 

  initialize_netls();                     /* establish a connection with */
                                          /* the NetLS server            */

  /* remove any leftover named pipe from a failed monitord               */
  unlink(MONITORD_PIPE);

  /* create and open the named pipe that will be used by the login to    */
  /* request and release licenses.					 */
  /* The mknod parameters are as follows: (pathname, mode, device)       */
  /* The mode could have been specified as S_IFIFO|S_IRUSR|S_IWUSR       */
  /* The device parameter is ignored for named pipes.                    */
  if (mknod(MONITORD_PIPE, 010600, 0) < 0)
  {
    log_sys(1, catgets(admin_catd, MONITORD_ADMIN, PIPE_NOCREATE,
             "0349-015 The monitord pipe could not be created.\n"));
    quit_clean(-1);
  }

  if ((pidpipe = open(MONITORD_PIPE, O_RDWR, 0600)) < 0)
  {
    log_sys(1, catgets(admin_catd, MONITORD_ADMIN, PIPE_NOOPEN,
             "0349-016 The monitord pipe could not be opened.\n"));
    quit_clean(-1);
  }

  /* set errno to 0 so the action while loop will not start off checking */
  /* an old errno value.						 */
  errno = 0;

  /* Monitor the pipe to read requests for license operations.  Pass the */
  /* info to actions() to determine and perform the operation.  The read */
  /* action may be interrupted by a heartbeat alarm.                     */

  while(1)
  {
    read_rc = read(pidpipe, &client_request, sizeof(client_request));

    if ((read_rc < 0) && (errno != EINTR))
    {
      log_sys(1, catgets(admin_catd, MONITORD_ADMIN, PIPE_NOREAD, 
               "0349-017 The monitord pipe could not be read.\n"));
      quit_clean(-1);
    }
    else
      if (errno == EINTR)	/* the read was interrupted by a heartbeat */
        errno = 0;		/* alarm.  Reset errno and re-enter loop.  */
      else
      {
        /* if the read was not complete, then read out what remains of the */
        /* pipe request information.					   */
        while (read_rc < sizeof(client_request))
        {
          read_rc = read_rc + read(pidpipe, &client_request        + read_rc, 
                                            sizeof(client_request) - read_rc);
        }
        #ifdef NETLS_DEBUG
        printf("client request_type = %d\n", client_request.request_type);
        printf("client login_pid = %d\n", client_request.login_pid);
        #endif

        actions(client_request.request_type, client_request.login_pid);

      }
  }

  /*** This point should never be reached ***/
  /*NOTREACHED*/
  log_sys(1, catgets(admin_catd, MONITORD_ADMIN, LOOP_EXIT,
           "0349-018 The action loop has been exited unexpectedly.\n"));
  quit_clean(-1);
}


/*
 * NAME:	sig_child
 *
 * FUNCTION:	Handler set up to handle children if necessary to avoid
 * 	       	zombies.
 *
 * RETURNS:	Nothing
 */
void sig_child()
{
  pid_t pid;
  int status;

  while( (pid = wait3(&status, WNOHANG, (struct rusage *) 0)) > 0);
}


/*
 * NAME:	daemon_start
 *
 * FUNCTION:	Sets the running process up as a daemon, spawning children
 *		if necessary.
 *
 * RETURNS:	Nothing
 */
void daemon_start()
{
  int fd;
  int childpid;

  /* only need to execute this code to detach from parent if not started by */
  /* init.                                                                  */
  if (getppid() != 1)
  {
    /* ignore terminal interrupt signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    /* fork off a child which will become a process group leader and then   */
    /* break off from the control terminal.                                 */
    if ( (childpid = fork()) < 0)
      {
      log_sys(1, catgets(admin_catd, MONITORD_ADMIN, FORK_CHILD,
          "0349-012 Monitord is unable to fork a child for daemon creation.\n"));
      exit(-1);
      }
    else
      if (childpid > 0)
      {
        #ifdef NETLS_DEBUG
        printf("As the original command, I am now exiting.\n");
        #endif
        exit(0);
      }
  
      #ifdef NETLS_DEBUG
      printf("Lose controlling tty\n");
      #endif
  
      /* call setsid() to become the process group leader */
      if (setsid() == -1)
      {
        log_sys(1, catgets(admin_catd, MONITORD_ADMIN, CHANGE_GRP,
             "0349-013 Monitord is unable to change process groups.\n"));
        exit(-1);
      }
  
      /* break off association with the control terminal */
      if ( (fd = open("/dev/tty", O_RDWR)) >= 0)
      {
        ioctl(fd, TIOCNOTTY, (char *) NULL);
        close(fd);
      }
  }


  #ifdef NETLS_DEBUG
  printf("change directory to root\n");
  #endif
  chdir("/");

  umask(0);

  #ifdef NETLS_DEBUG
  printf("have SIGCLD handled\n");
  #endif
  signal(SIGCLD, sig_child);

  /* close any open file descriptors */
  #ifndef NETLS_DEBUG
  for (fd = 0; fd < NOFILE; fd++)
    close(fd);
  #endif

}


/*
 * NAME:	actions
 *
 * FUNCTION: 	Based on the request_type parameter, call the appropriate
 *            	function to either get a license for login_pid, or release
 *            	a license held by login_pid.
 *
 * RETURNS:	Nothing
 */
void actions(int request_type, pid_t login_pid)
{
  if (request_type == RELEASE_LIC)
  {
    #ifdef NETLS_DEBUG
    printf(">>release!\n");
    #endif
    remove_pid(login_pid);
  }
  else 
  {
    #ifdef NETLS_DEBUG
    printf(">>request!\n");
    #endif
    add_pid(login_pid);
  }

  #ifdef NETLS_DEBUG
  print_pids();
  #endif
}


/*
 * NAME:	add_pid
 *
 * FUNCTION:	Request a license from the license server for login_pid.  
 *            	Login_pid may already be in the list of licensed pids in
 *	      	special timing cases where the pid is recycled before the 
 *	      	license release message is sent.  For that reason, each 
 *	      	licensed pid will have a counter associated with it.
 *
 * RETURNS:	Nothing
 */
void add_pid(pid_t login_pid)
{
  nls_lic_trans_id_t trans_id = -1;
  int dup_index = -1;

  if (last_index == (MAX_PIDS - 1))          /* the pid array is full.     */
  {
    #ifdef NETLS_DEBUG
    printf("no more room at the inn\n");
    #endif
    log_sys(1, catgets(admin_catd, MONITORD_ADMIN, PIDMAXOUT,
             "0349-014 Monitord ran out of local storage for login tracking.\n"));
    signal_failure(login_pid);
  }
  else
  {
    dup_index = find_index(login_pid);       /* check if this pid is a dup */
                                             /* of a licensed login        */
    if (dup_index >= 0)
    {
      pid_list[dup_index].count++;           /* dup, so just incr. counter */
      signal_success(login_pid);
    }
    else
    {
      trans_id = lic_trans();                /* request a license          */
   
      if (trans_id == -1)                    /* if no license available    */
      {
        if (softstop)
        {
	  #ifdef NETLS_DEBUG
	  printf("softstopping!!\n");
	  #endif
	  signal_success(login_pid);
	  log_softstop();
        }
        else
          signal_failure(login_pid);
      }
      else                                   /* add the pid to the list of */
      {                                      /* licensed logins            */
        last_index++;
        pid_list[last_index].pid = login_pid;
        pid_list[last_index].count = 1;
        pid_list[last_index].transid = trans_id;
        #ifdef NETLS_DEBUG
        printf("successfully added\n");
        #endif
        signal_success(login_pid);
      }
    }
  }

  return;
}


/*
 * NAME:	remove_pid
 *
 * FUNCTION:	Release a license for kill_pid.  If the counter for 
 *		kill_pid is greater than one, then just decrement the 
 *              counter (a pid has been recycled, and another release
 *		action should be received for this pid in the future).
 *              If the counter is one, then release the license and 
 *		delete kill_pid from the pid_list.  Pid_list deletes 
 *		are implemented as position swaps between the deleted 
 *		entry and the last entry in the array.  This keeps the 
 *		pid_list packed.
 *
 * RETURNS:	Nothing
 */
void remove_pid(pid_t kill_pid)
{
  int kill_index = -1;

  if (last_index == -1)
  {
    #ifdef NETLS_DEBUG
    printf("empty pid_list, silly\n");
    #endif
    return;
  }

  kill_index = find_index(kill_pid);

  if (kill_index < 0)
  {
    #ifdef NETLS_DEBUG
    printf("pid not found\n");
    #endif
    return;
  }
  else
  {
    if (pid_list[kill_index].count > 1)
      pid_list[kill_index].count--;
    else
    {
      #ifdef NETLS_DEBUG
      printf("kill pid: %d\n", pid_list[kill_index].pid);
      #endif

      drop_license(pid_list[kill_index].transid);

      if (kill_index < last_index)  /* only need to swap if not equal */
      {
        pid_list[kill_index].pid = pid_list[last_index].pid;
        pid_list[kill_index].count = pid_list[last_index].count;
        pid_list[kill_index].transid = pid_list[last_index].transid;
      }

      pid_list[last_index].pid = 0;
      pid_list[last_index].count = 0;
      pid_list[last_index].transid = 0;
  
      last_index--;
      #ifdef NETLS_DEBUG
      printf("successfully removed\n");
      #endif
    }
  } 

  return;
}


#ifdef NETLS_DEBUG
/*
 * NAME:	print_pids
 * 
 * FUNCTION:	Prints the contents of the pid_list.
 *
 * RETURNS:	Nothing
 */
void print_pids()
{
  int i = 0;

  if (last_index < 0)
    printf("pid list is empty\n");
  for (i=0; i <= last_index; i++)
  {
    printf("  >      pid = %d\n", pid_list[i].pid);
    printf("       count = %d\n", pid_list[i].count);
    printf("     transid = %d\n", pid_list[i].transid);
  }
}
#endif


/*
 * NAME:	find_index
 *
 * FUNCTION:	Searches the pid_list for a specified login process id.
 *		If the id is found, its index in the pid_list array is
 *		returned.  If it is not found, -1 is returned.
 *
 * RETURNS:	Nothing
 */
int find_index(pid_t login_pid)
{
  int found = 0,
      chk_index = 0,
      pid_index = -1;
     
  while ((found == 0) && (chk_index <= last_index))
  {
    if (pid_list[chk_index].pid == login_pid)
    {
      found = 1;
      pid_index = chk_index;
    }
    else
      chk_index++;
  }

  return(pid_index);
}


/*
 * NAME:	init_pid_list
 *
 * FUNCTION: 	initializes the pid_list so a valid entry will always
 *		be non-zero.
 *
 * RETURNS:	Nothing
 */
void init_pid_list()
{
  int i = 0;

  for (i = 0 ; i < MAX_PIDS; i++)
  {
    pid_list[i].pid = 0;
    pid_list[i].count = 0;
    pid_list[i].transid = 0;
  }
}


/*
 * NAME:	signal_success
 * 
 * FUNCTION: 	sends a signal to the login_pid to let it know that
 *		its license request succeeded.
 *
 * RETURNS:	Nothing
 */
void signal_success(pid_t login_pid)
{
  kill(login_pid, SIGUSR1);
  
  return;
}


/*
 * NAME:	signal_failure
 *
 * FUNCTION:	sends a signal to the login_pid to let it know that
 *		its license request failed.
 *
 * RETURNS:	Nothing
 */
void signal_failure(pid_t login_pid)
{
  kill(login_pid, SIGUSR2);
  
  return;
}


