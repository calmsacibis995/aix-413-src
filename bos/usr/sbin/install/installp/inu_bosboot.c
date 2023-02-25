static char sccsid[] = "@(#)15  1.7  src/bos/usr/sbin/install/installp/inu_bosboot.c, cmdinstl, bos411, 9428A410j 3/6/94 19:30:38";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_bosboot_ispending
 *		inu_bosboot_isneeded
 *		inu_bosboot_perform
 *		inu_bosboot_setpending
 *		inu_bosboot_verify
 *		inu_run_cmd
 *	        sig_alarm
 *		sig_child
 *		
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

/*
 * Routines to handle bosboot process (both network and local) for 
 * installp.
 */
#include <stdio.h>
#include <swvpd0.h>
#include <sys/errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#include <inudef.h>
#include <installp.h>
#include <instl_options.h>

#include <sys/time.h>
#include <sys/events.h>

static int sig_alarm ();	  /* Signal handler for alarm			     */
static int sig_child ();   /* Signal handler for child death		     */
static int leave;	  /* Set when the child dies, and signals the parent */
static int status;	  /* Return code from the child			     */ 
static int pid;		  /* Child's process id				     */
extern char *inu_cmdname; /* Command name 				     */

/*
 * Function: sig_child
 * 
 * Purpose:  SIGCLD signal catcher.  The inu_run_cmd () function sets this
 *	     routine up to catch the child death signal from it's forked
 *	     process.  This routine will signal inu_run_cmd () that the child
 *	     has died by setting the "leave" variable to TRUE.  It will 
 *	     also return the child's return code in the "status" variable.
 */ 
static int 
sig_child ()
{
	leave = TRUE;  		  /* signal to the parent        */
	waitpid (pid, &status, 0); /* Be sure and MASK the status */
}

/*
 * Function: sig_alarm
 * 
 * Purpose:  SIGALRM signal catcher.  The inu_run_cmd () function sets this
 *	     routine up to catch the alarm signal which will occur every
 *	     thirty seconds.  This routine will output a period at this
 *	     point signalling the user that work is still in progress.
 */ 
static int
sig_alarm ()
{
	fputs (".", stdout);
}

/*
 * Function: inu_run_cmd
 *
 * Purpose:  Forks and execs the given command.  This function delivers all
 *	     output and the return code from the command back to the caller.
 *	     The caller is responsible for freeing the memory allocated for
 *	     the command output.
 *
 * Returns:  nonzero 	-- Failure to run the specified command.
 *	     zero 	-- The command was successfully run. 
 */
int 
inu_run_cmd (char *child,  	/* command to exec			*/
	    char **childout, 	/* stdout and stderr from the command 	*/ 
	    int  *rc)		/* return code from the command		*/
{
	char *outbuf, 		/* output from command 		*/	
	     *outptr, 		/* output buffer traversal 	*/
	     *outend;		/* output buffer end 		*/
   	int  i, 	  		/* number of chars read 	*/
       	     outfd[2]; 		/* file pointers from pipe   	*/
	timer_t  timerid;
   	struct itimerstruc_t timer, otimer;
	struct sigaction action, 
			 oactionchild, 
			 oactionalarm, 
			 oactionint, 
			 oactionquit;

	if ((outbuf = (char *)malloc (BUFSIZ * sizeof (char))) == (char *)NULL)
	{
		instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
		                     CMN_MALLOC_FAILED_D), inu_cmdname);
		return (FAILURE);
	}

	*childout = outptr = outbuf;
	outend = &outbuf[BUFSIZ - 1];
	outbuf[0] = (char)NULL;

	leave = 0;	/* Initialize the statics.        */ 
	status = 0;	/* This is imperative on reentry. */
	pid = 0;

   	if (pipe (outfd) < 0) return (FAILURE);

	/* 
	 * Set up our catcher for receiving child death signals.
	 * I'm using sigaction () here so I can return installp to
	 * it's previous signal handling state after I'm done.
	 */
	sigemptyset (&(action.sa_mask));
	action.sa_flags = 0;
	action.sa_handler = (void (*) (int))sig_child;
	sigaction (SIGCLD, &action, &oactionchild);

	/*
	 * Ignore keyboard generated signals
	 */
	action.sa_handler = SIG_IGN;
        sigemptyset (&(action.sa_mask));
        action.sa_flags = 0 ;
        sigaction (SIGINT, &action, &oactionint);
        sigaction (SIGQUIT, &action, &oactionquit);

   	if ((pid = fork ()) == 0)
   	{
        	sigaction (SIGINT, &oactionint,   (struct sigaction *)NULL);
        	sigaction (SIGQUIT, &oactionquit,  (struct sigaction *)NULL);
        	sigaction (SIGCLD , &oactionchild, (struct sigaction *)NULL);

       		close (1);             
       		dup (outfd[1]);  

       		close (2);           
      		dup (outfd[1]);

       		fcntl (outfd[1], F_SETFL, O_NDELAY);
       		execl ("/bin/sh", "sh", "-c", child, 0);
       		return (1);
   	}

   	if (pid == -1) return (FAILURE);

   	close (outfd[1]);            /* close the parents write pipe    */

	/* Set up to catch ALARM signal. */
	action.sa_mask.losigs = 0;
	action.sa_mask.hisigs = 0;
	action.sa_flags = 0;
	action.sa_handler = (void (*) (int))sig_alarm;
	sigaction (SIGALRM, &action, &oactionalarm);

      	/* Set the alarm clock to time out every 30 seconds. */
      	timer.it_interval.tv_sec = 30;
      	timer.it_interval.tv_nsec = 0;
      	timer.it_value.tv_sec = 30;
      	timer.it_value.tv_nsec = 0;
      	timerid = gettimerid (TIMERID_ALRM, DELIVERY_SIGNALS);
      	incinterval (timerid, &timer, &otimer);
	do
    	{
       		do
        	{
          		if ((i = read (outfd[0], outptr, outend - outptr)) < 0)
           		{
             			if (errno == EINTR) 
					i = 1; 
				else 
					return (FAILURE);
           		}
          		else
             			outptr += i;
	
			if (outptr >= outend)
			{
				int newsiz = outend - outbuf + BUFSIZ;
				char *oldbuf = outbuf;

				if ((outbuf = (char *)realloc
					 (outbuf, newsiz)) == (char *)NULL)
				{
					instl_msg (FAIL_MSG, MSGSTR (MS_INUCOMMON, 
						CMN_REALLOC_FAILED_E, 
						CMN_REALLOC_FAILED_D), 
						inu_cmdname);
					return (FAILURE);
				}
				*childout = outbuf;
				outptr = &outbuf[outptr - oldbuf];
				outend = &outbuf[newsiz - 1];
			}
        	}
       		while (i > 0);
       		*outptr = (char)NULL;
	}
	while ( ! leave);

	/* return to previous state */
      	incinterval (timerid, &otimer, (struct itimerstruc_t *)NULL);
	sigaction (SIGALRM, &oactionalarm, (struct sigaction *)NULL);
	sigaction (SIGCLD,  &oactionchild, (struct sigaction *)NULL);
        sigaction (SIGINT,  &oactionint,   (struct sigaction *)NULL);
        sigaction (SIGQUIT, &oactionquit,  (struct sigaction *)NULL);

	fputs ("\n", stdout);	/* output final newline */

	*rc = MASK (status);	/* set up the correct command return code */

	return (SUCCESS);
}

/*
 * Function: inu_bosboot_perform
 *
 * Purpose:  This function does bosboot processing for the system.  As such
 * 	     it calls a system script that may perform multiple bosboots
 *	     depending on the machine environment
 *
 * Returns:  nonzero -- bosboot processing did not occur
 *           zero    -- bosboot has successfully completed.
 */
int
inu_bosboot_perform ()
{
	instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_BOSBOOT_PERFORM_E, 
			INP_BOSBOOT_PERFORM_D), inu_cmdname);

	if (inu_do_exec (BOSBOOT_PERFORM))
	{
		instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BOSBOOT_FAILED_E, 
			INP_BOSBOOT_FAILED_D), inu_cmdname, inu_cmdname);
		return FAILURE;
	}
	else
        {
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, 
                                       INP_BOSBOOT_PERFORM_COMPLETE_I, 
                                       INP_BOSBOOT_PERFORM_COMPLETE_D), 
                                       inu_cmdname);

                return SUCCESS;
        }
}

/*
 * Function: inu_bosboot_verify
 *
 * Purpose:  This function should ensure that a bosboot will be successful.
 *	     The intent is to do a quick check at the beginning of the 
 *	     install and if a bosboot will be required but couldn't be
 *	     successful then end the install.  Otherwise process the images
 *	     and the REAL bosboot at the end of install should be ok.
 *
 * Returns:  nonzero -- bosboot processing cannot be accomplished
 *	     zero    -- bosboot processing should be successful
 */
int
inu_bosboot_verify ()
{

	instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, INP_BOSBOOT_VERIFY_E, 
			INP_BOSBOOT_VERIFY_D), inu_cmdname);

	if (inu_do_exec (BOSBOOT_VERIFY))
	{
		instl_msg (FAIL_MSG, MSGSTR (MS_INSTALLP, INP_BOSBOOT_FAILED_E, 
				INP_BOSBOOT_FAILED_D), inu_cmdname, inu_cmdname);
		return FAILURE;
	}
	else
        {
                instl_msg (INFO_MSG, MSGSTR (MS_INSTALLP, 
                                       INP_BOSBOOT_VERIFY_COMPLETE_I, 
                                       INP_BOSBOOT_VERIFY_COMPLETE_D), 
                                       inu_cmdname);
                return SUCCESS;
        }
}

/*
 * Function: inu_bosboot_isneeded
 *
 * Purpose:  This function takes the user's selected options list and 
 *	     determines if any of these are going to require a bosboot.
 *
 * Returns:  nonzero -- an image will require a bosboot
 *	     zero    -- no bosboot should be required.
 */
int
inu_bosboot_isneeded (Option_t *SopPtr)
{
	Option_t *op;
	struct lpp srch_lpp;

	/*
	 * Traverse the selected options looking for the "need bosboot" flag 
  	 */
        for (op = SopPtr->next; op != NIL (Option_t); op = op->next) 
	{
		if (IF_BOSBOOT (op->op_type))
			return (TRUE);
	}
	return (FALSE);
}

/*
 * Function: inu_bosboot_ispending
 *
 * Purpose:  This routine checks the global bosboot bit that resides
 *	     in the vpd database __SWVPD_CTL__ record.  If this bit
 *	     is set a bosboot has been set to occur at the end of install
 *
 * Returns:  nonzero -- bosboot is set to occur
 *	     zero    -- no bosboot is necessary
 */
int
inu_bosboot_ispending ()
{
	struct lpp ctl_lpp;  /* VPD LPP control structure */
	
	memset (&ctl_lpp, NULL, sizeof (struct lpp));
	strcpy (ctl_lpp.name, LPP_CTL_NAME);

	inu_set_vpdtree (VPDTREE_ROOT);

	if (vpdget (LPP_TABLE, LPP_NAME, &ctl_lpp) == VPD_OK)
	{
		if (IF_BOSBOOT (ctl_lpp.cp_flag))
			return (TRUE);

	}
	return (FALSE);
}

/*
 * Function: inu_bosboot_setpending
 *
 * Purpose:  This routine sets the global bosboot bit that resides
 *	     in the vpd database __SWVPD_CTL__ record.  If this bit
 *	     is set a bosboot will occur at the end of install
 *
 * Options:  yesno == nonzero  -- bosboot bit will be set 
 *	     yesno == zero     -- bosboot bit will be cleared 
 *
 * Returns:  nonzero -- bosboot bit can't be changed
 *	     zero    -- bosboot bit has been changed
 */
int
inu_bosboot_setpending (int yesno)
{
	struct lpp ctl_lpp;  /* VPD LPP control record */
	
	memset (&ctl_lpp, NULL, sizeof (struct lpp));
	strcpy (ctl_lpp.name, LPP_CTL_NAME);

	inu_set_vpdtree (VPDTREE_ROOT);

	if (vpdget (LPP_TABLE, LPP_NAME, &ctl_lpp) == VPD_OK)
	{
		if (yesno)
			ctl_lpp.cp_flag |= LPP_BOSBOOT;
		else
			ctl_lpp.cp_flag &= ~LPP_BOSBOOT;

		if (vpdchg (LPP_TABLE, LPP_NAME, &ctl_lpp) == VPD_OK)
			return (SUCCESS);

	}
	return (FAILURE);
}
