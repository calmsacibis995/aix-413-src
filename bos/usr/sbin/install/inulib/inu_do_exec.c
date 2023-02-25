static char     sccsid[] = "@(#)59      1.12  src/bos/usr/sbin/install/inulib/inu_do_exec.c, cmdinstl, bos411, 9428A410j 5/25/94 13:03:13";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_do_exec
 *		consume_child
 *		catch_child
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <inulib.h>
#include <fcntl.h>
#include <inudef.h>
#include <signal.h>


/* ------------------------------------------------------------------------
 *
 * NAME: inu_do_exec
 *
 * FUNCTION:
 *     Run the given command using fork and exec.
 *     stdout and stderr of the command are captured so that
 *     they can be sent to both the logfile and stderr.
 *
 * EXECUTION ENVIRONMENT:
 *     installp command
 *
 * DATA STRUCTURES:
 *     parameters:
 *        cmd       -- Command to be run, and the arguments 
 *
 *     global:
 *        do_log    -- Indicates whether logging is turned on
 *                     or not.
 *
 * RETURNS: (int)
 *           Return code from exec command
 *
 * NOTE: Command to be executed has absolute path spcecified.
 *       If system calls fail, we quit though we can continue in 
 *       cases because we probably are in a bad shape when this 
 *       happens.
 *
 *
 * NOTE: If you change this routine make sure you test bos.inst installs 
 * also. Saw some errors only at bos.inst time.
 * ----------------------------------------------------------------------- */

static int child_alive;
static void catch_child(int);
static void consume_child(int);
static pid_t pid;	/* child pid */
static int status;	/* status of exec */


int
inu_do_exec (char * cmd)
{
   char buf[MEM_BUF_SZ]="";           /* for reading from pipe */
   extern char *inu_cmdname;   /* for messages */
   struct sigaction action;
   struct sigaction oaction;

   int fd[2],                  /* to do the pipe */
       rc,                     /* return code */
       do_log = FALSE,         /* log indicator */
       tempfd;                 /* fd of stdout file */

  extern int inu_doing_logging;

  do_log = inu_doing_logging;

  /* setup sigaction struct */
  bzero(&action,sizeof(action));
  action.sa_handler = catch_child;
  if (do_log)
  {
     /* create a pipe */
     if (pipe (&fd[STDIN_FILENO]) == -1) 
     {
        inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                     CMN_ERR_SYSCALL_D), inu_cmdname, "pipe");
        return (INUSYSERR);
     }
     /* logging needs to do something different with the child */
     action.sa_handler = consume_child;
  }

  /* Set up to catch child termination signal.
   * indicate that our child is almost alive
   */
  child_alive=1;
  pid=0; 
  sigaction (SIGCHLD, &action, &oaction);

  if ((pid=fork ()) < 0) 
  {
     sigaction (SIGCHLD, &oaction, (void *)0);
     inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                  CMN_ERR_SYSCALL_D), inu_cmdname, "fork");
      return (INUSYSERR);
  }

  if (pid == 0)
  {  /* child */

     if (do_log) 
     {
        /* dup stdout to output pipe */
        if (dup2 (fd[STDOUT_FILENO], STDOUT_FILENO) == -1) 
        {
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                        CMN_ERR_SYSCALL_D), inu_cmdname, "dup");
           exit (INUSYSERR);
        } 

        /* dup stderr to tempfd */
        if (dup2 (STDOUT_FILENO, STDERR_FILENO) == -1) 
        {
           inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                        CMN_ERR_SYSCALL_D), inu_cmdname, "dup");
           exit (INUSYSERR);
        } 

 
        close (fd[STDIN_FILENO]);   /* don't care if it fails */
     }

     /* run command */
     execl ("/usr/bin/sh", "sh", "-c", cmd, (char *)0); 

     /* If we get here, we are in trouble */
     inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
                                  CMN_ERR_SYSCALL_D), inu_cmdname, "exec");
     exit (INUSYSERR);
  }


  if (do_log) 
  {
      /* Now we are in the parent i.e. (pid > 0) */

      close (fd[STDOUT_FILENO]);

      /* send stderr to log */
      while (child_alive)
      {
	  
	  /* keep trying to read until the error we get is not an interupt
	   * and our child process hasn't terminated.
	   *
	   * Think twice before changing this. Some failures only happen
	   * at bos.inst time frame.
	   */
	  while( (((rc=read(fd[STDIN_FILENO],buf,MEM_BUF_SZ-1))== -1)
		       && errno==EINTR) && child_alive);

	  /* stoping this logging adventure
	   *    1) if we have a read error 
	   *	2) the child terminated 
	   *	...
	   */
	  if(rc<=0)
		break;

          buf[rc]='\0';  /* null termination*/  
          inu_msg (SCREEN_LOG, buf); 
      }

      /* use our default signal handle now */
      action.sa_handler = catch_child;
      sigaction (SIGCHLD, &action, (void *)0);

      /* we should not be getting here with a live child unless there
       * where some kind of read error. If the child is alive when we
       * close this it may cause the child terminate abnormally below,
       * but if we don't close it here it could cause a hang when our
       * child tryies to write to stdout/stderr.
       */
      close (fd[STDIN_FILENO]);

      /* this should always be the case (consume_child) ... */
      if(pid==0)
      {
  	  sigaction (SIGCHLD, &oaction, (void *)0);
	  return(MASK(status));
      }

   } /* if do_log */

   /* wait on our child */
   while (waitpid (pid, &status, 0) == -1) 
   {
   	if (errno != EINTR)
   	{
  	     sigaction (SIGCHLD, &oaction, (void *)0);
      	     inu_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_ERR_SYSCALL_E, 
			CMN_ERR_SYSCALL_D), inu_cmdname, "waitpid");
      	     return (INUSYSERR);
   	} 
   }
   sigaction (SIGCHLD, &oaction, (void *)0);

   return (MASK (status));  
}

static void catch_child(int sig)
{
	child_alive=0;
}

static void consume_child(int sig)
{
	int rc;

	/* are we childless? */
	if(pid==0 || pid== -1)
		return;

	/* eat the child that inu_do_exec started */
	while ((rc=waitpid (pid, &status, WNOHANG)) == -1 && errno == EINTR);

	/* somebody else's child */
	if (rc!=pid)
		return;

	/* indicate that we had lunch */
	child_alive=0;
	pid=0;
	return;
}
