static char sccsid[] = "@(#)21	1.38.2.12  src/bos/usr/sbin/tsm/tsmgetty.c, cmdsauth, bos41J, 9520A_all 5/4/95 14:36:21";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS:	catch_brk gettylog, gettysetup, saklongjmp, tsmgetty, utmp_fill
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/termio.h>
#include <sys/audit.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <sys/stream.h>
#include <sys/select.h>
#include <unistd.h>
#include <utmp.h>
#include <setjmp.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <userpw.h>
#include <usersec.h>
#include "tsm.h"

static	jmp_buf	sakjmp;			/* context to jump to on SIGUSR1 */
static	jmp_buf intjmp;			/* context to jump to on SIGINT  */
static	int	intjmpset;
extern	struct	termios runtty;		/* for opt logger defaults	 */

int brkrcvd = 0;			/* global break received counter */

/*
 * NAME: catch_brk
 *
 * FUNCTION: Catch SIGINT
 *
 * EXECUTION ENVIRONMENT:
 *	Signal handler
 *
 * NOTES:
 *	Increments the count of break signals received.  If intjmpset is set,
 *	then it will longjmp to the context stored in the intjmp jump buffer.
 *
 * RETURNS:
 *	Nothing.  longjmp()s if intjmpset is TRUE.
 */

void
catch_brk(sig,code,context)
int	sig, code;
struct	sigcontext *context;
{
	/* increment the breaks received counter */
	brkrcvd++;
	DPRINTF(("got signal, brkrcvd=%d\n",brkrcvd));
	if (intjmpset) {
		/*
		 * Some ports are unable to send/receive characters while the
		 * break signal is being received.  Wait for 1/2 a second (the
		 * break signal length) to ensure the port is ready to receive
		 * characters before we continue.
		 */
		usleep(500 * 1000);
		longjmp(intjmp,brkrcvd);
	}
}

/*
 * NAME: saklongjmp
 *
 * FUNCTION: Catch SIGUSR1
 *
 * EXECUTION ENVIRONMENT:
 *	Signal handler
 *
 * RETURNS:
 *	Implicit return via longjmp.
 */

void
saklongjmp ()
{
	struct	acl	*acl;
	struct	acl	*acl_fget ();

	DPRINTF (("rec'd saklongjmp signal"));

	/*
	 * TBD: do a sakquery and make sure this is the correct channel
	 */

	/*
	 * Set a flag indicating SAK was received and return to the caller.
	 * All file descriptors which are open for this line must be closed,
	 * except fd 0.  If any other file descriptors for the port are
	 * open TSM will be killed ...
	 */

	saked = 1;
	sakrcvd = 1;

	close (1);
	close (2);

	frevoke (0);
	fchown (0, 0, 0);
	if (acl = acl_fget (0)) {
		acl->u_access = 0;
		acl->g_access = 0;
		acl->o_access = 0;
		acl->acl_len = ACL_SIZ;

		acl_fput (0, acl, 1);
	}
	frevoke (0);

	dup (0);
	dup (0);

	longjmp (sakjmp, 1);
}

/*
 * NAME: gettysetup
 *
 * FUNCTION: Read a line of input and set the correct initial tty modes
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	Gettysetup is responsible for determining the correct line speed
 *	and cycling between the various baud rates and tty modes.
 *
 * RETURNS:
 *	Nothing.
 */

void
gettysetup (input)
char	*input;
{
	static int  more_sets = TRUE;
	/*
	 * Flag indicating that we have receiveds something other than a break.
	 * Break is an indication to cycle modes; receiving a break for all
	 * settings causes getty to exit.
	 */
	static int  have_usrinput = FALSE;

	/*
	 * Enable SAK in the device driver if SAK was requested for this port.
	 */
	setsecurity ();

	/*
	 * SETUP:
	 *      read in a user name ...
	 *      if we succeed -
	 *              set up the final parameters
	 *      if get a break
	 *              try the next set of parameters
	 */

	setdefaultmodes ();

	initset();

	/*
	 * set up setjmp location here. We will return here if
	 * we receive a SIGINT (received a break on this port).
	 */
	if (setjmp(intjmp)!=0) {
		/*
		 * come here after receiving a break.
		 * advance to the next mode and decrement
		 * the breaks received counter
		 */
		DPRINTF(("returned from longjmp\n"));
		more_sets = nextset();
		brkrcvd--;
	} else
		intjmpset = TRUE;

	/*
	 * if we have received any breaks before we did the setjmp()
	 * above, then we want to handle them now by advancing through
	 * the mode sets once for each break we received.
	 */
	for (; (brkrcvd > 0) && more_sets; brkrcvd--, more_sets = nextset())
		DPRINTF(("advancing to next set, brkrcvd=%d\n",brkrcvd));

	/*
	 * loop through here until we've tried all mode
	 * possibilities or we have user input to look at
	 */
	while (more_sets && (have_usrinput == FALSE)) {

		/*
		 * Set the current tty login modes, ownership, and protection.
		 * The set changes for each cycle.
		 */

		setmodes ();
		setownership ();
		setprotection ();
		xtcsetattr(INFD, TCSANOW, &logtty);

		/*
		 * See if SAK processing is being requested and
		 * set up the signal handler for SIGUSR1 and SIGSAK
		 */

		if (portsak) {
			DPRINTF (("Setting SAK to saklongjmp"));

			/*
			 * Set up the SAK signal handler.  After
			 * receipt of a SAK signal, go back to
			 * before the setjmp() and save the
			 * environment again.
			 */

			while(setjmp(sakjmp));
			signal(SIGUSR1, saklongjmp);
			signal(SIGSAK, saklongjmp);
		}

                /*
                 * If there is a logger program for this port, then execute it
                 * instead of tsmlogin.
                 */
		if(logger && logger[0]) {
			int i, nenv = 0;
			char err_msg[255], *env[10], term[PATH_MAX];

			if(access (logger, X_OK) != -1) {
				/*
				 * If the logger program is not trusted, then
				 * turn off the trustedness of the terminal.
				 */
				if(tcb (logger, TCB_QUERY) != TCB_ON) {
					i = TCUNTRUSTED;
					xioctl (0, TCTRUST, &i);
				}

                                /*
                                 * Set up the default environment.
                                 */
                                if (termtype) {
                                        env[nenv++] = PENV_USRSTR;
                                        settermtype (env[nenv++] = term);
                                }
                                setdefsenv (env, &nenv, "root","root");

				/*
				 * Configure the tty to have the same settings
				 * as if the person had logged in.
				 */
				xtcsetattr(INFD, TCSANOW, &runtty);

                                /*
                                 * Exec the logger program.
                                 */
                                setpenv (NULL, PENV_INIT, env, logger);

                                /*
                                 * Could not execute the logger program; 
				 * print a warning message and exit.  
				 * setpenv() messes with the privilege vectors;
				 * if we were to continue, the setpcred() in 
				 * tsmlogin will fail.
                                 */
                                sprintf (err_msg, MSGSTR (M_NOLOGGER, DEF_NOLOGGER),logger);
                                tsm_err (err_msg, 1, FALSE);
                        } else {
                                /*
                                 * The logger program does not exist or is 
				 * not executable; print a warning message 
				 * and continue with tsm login.
                                 */
                                sprintf (err_msg, MSGSTR (M_NOLOGGER, DEF_NOLOGGER),logger);
                                tsm_err (err_msg, TSM_NOEXIT, FALSE);
                        }
		}

		/*
		 * Each pass through the modes gets a fixed time limit
		 * to complete.  Set the time for this attempt.
		 */

		settimeout ();

		DPRINTF(("about to call getusername(), herald=%s\n",herald));
		if (getusername (input, herald)) {
			/*
			 * now have the correct tty modes [ and a first
			 * attempt at a user ID ]
			 */

			alarm (0);
			have_usrinput = TRUE;
		}

		/* advance to the next mode in the set */
		more_sets = nextset();

	}
	if (have_usrinput == FALSE)	{
		/*
		 * This means we received breaks for all possible settings
		 * Hence bail and try again
		 */

                /*
                 * syslog(LOG_NOTICE, MSGSTR(M_SETTINGS, DEF_SETTINGS));
                 * exit (1);
                 *
                 * Changing the syslog call to tsm_err for consistency
                 * in error handling throughout tsm.  The syslog call
                 * does not add the tty number to the format.
                 */
                tsm_err(MSGSTR(M_SETTINGS,DEF_SETTINGS), 1, FALSE);

	}

	intjmpset = FALSE;
	setsignals ();		/* put signal back so saklongjmp doesn't
					   get called all over again. */
}

/*
 * NAME: tsmgetty
 *
 * FUNCTION: Initialize terminal modes for login processing
 *
 * EXECUTION:
 *	User process
 *
 * NOTES:
 *	The appropriate file descriptors are closed.  The owner
 *	and permissions of the port are change and the port
 *	re-opened.  The output and error descriptors are dup()'d
 *	from the input descriptor.  The line is cleaned using
 *	frevoke().
 *
 * RETURNS:
 *	No return value.  May exit if UUGETTY mode is requested and the
 *	port is in use.
 */

void
tsmgetty (mode, input)
int	mode;
char	*input;
{
	struct termios origterm, newterm;
	char buffer[PATH_MAX];
	char *newname, *device;
	char err_msg[256];
	int ind, protection, effective_id;
	uid_t uid = 0, owner;
	gid_t group;
	fd_set selfd;
	struct sigaction sa;

	/*
	 * Set up signal handler for SIGINT
	 */
	DPRINTF(("setting up signal handler\n"));
	intjmpset = FALSE;
	sa.sa_handler = (void (*)())catch_brk;
	sa.sa_mask.losigs =  0;
	sa.sa_mask.hisigs =  0;
	sa.sa_flags   = SA_RESTART;
	if (sigaction(SIGINT,&sa,(struct sigaction *)NULL)) {
		/*
		 * Error setting up signal handler
		 */
		DPRINTF(("sig handler failed, errno=%d\n",errno));
		return;
	}
	DPRINTF(("set up signal handler\n"));

	/*
	 * We sleep for 250ms in order to allow DTR to drop for this long.
	 * This is necessary regardless of the mode (share, delay, etc)
	 * in order to allow modems to detect falling DTR.
	 */
	usleep(250 * 1000);

	/*
	 * Determine whether this user is allowed to run tsm.  Only the root
	 * user and members of the uucp group are allowed to run tsm.  This is
	 * required to provide support for ct.
	 */
	uid = getuid();
	if (uid != 0) {
		char *groups = NULL;

		setuserdb(S_READ);
		(void) getuserattr(IDtouser(uid), S_GROUPS, &groups, SEC_LIST);

		while (groups != NULL && *groups != '\0') {
			if (0 == strcmp(groups, "uucp"))
				break;
			groups += strlen(groups) + 1;
		}
		if (groups == NULL || *groups == '\0') {
			sprintf(err_msg, MSGSTR(M_FAILTERM, DEF_FAILTERM),
				portname);
			tsm_err(err_msg, 9, FALSE);
		}
		enduserdb();
	}

	/*
	 * Remove the "/dev" from the front of the port name.
	 */
	device = undev(portname);

	/*
	 * Place this getty process into /etc/utmp.
	 */
	gettylog(portname);

	/*
	 * Get the characteristics of this port from ODM.
	 */
	getparms();

	/*
	 * Check if sak is enabled on this port and set the global variable.
	 */
	if (tsm_sakenabled (pseudopname)) {
		portsak = "true";
	} else {
		portsak = NULL;
	}

	/*
	 * Change the ownership and permission of this port and its synonyms.
	 */
	if (mode & GETTY_UU) {
		protection = 0662;
		setpwdb(S_READ);
		setuserdb(S_READ);
		if (0 > getuserattr("uucp", S_ID, &owner, SEC_INT) &&
		    0 > getuserattr("nuucp", S_ID, &owner, SEC_INT))
			owner = 0;
		if (0 > getgroupattr("uucp", S_ID, &group, SEC_INT))
			group = 0;
		enduserdb();
		endpwdb();
	} else {
		protection = 0600;
		if (sakrcvd && portsak)
			owner = group = 0;
		else {
			owner = getuid();
			group = getgid();
		}
	}
	(void) chown (portname, owner, group);
	(void) portprotection(protection, portname);
	for (ind = 0; ind < nsyns; ind++) {
		(void) chown (synonym_list[ind], owner, group);
		(void) portprotection(protection, synonym_list[ind]);
	}

	setsid();

	/*
	 * If CLOCAL is turned on in the ODM logmodes setting, then we must
	 * set CLOCAL manually since it is forced off on the first open.
	 * Otherwise do a blocking open on the port.
	 */
	(void) close(INFD);
	if (local_port) {
		struct termios t;
                if (INFD != (ind = open(portname, O_RDWR|O_NDELAY))) {
			sprintf(err_msg, MSGSTR (M_CANTOPEN, DEF_CANTOPEN),
				portname);
			tsm_err(err_msg, 1, FALSE);
		}
		xtcgetattr(INFD, &t);
		t.c_cflag |= CLOCAL;
		if ((mode & GETTY_UU) && ttylocked(device)) {
			/*
			 * If this is a uugetty, got to check to make sure
			 * someone isn't using the port.  If they are, wait for
			 * them to finish, clean up port and exit
			 */
			do ttywait(device); while (ttylock(device));
			frevoke(INFD);
			if (local_hupcl)
				t.c_cflag |= HUPCL;
			t.c_cflag &= ~(CLOCAL);
			tcsetattr(INFD, TCSANOW, &t);
			exit(0);	/* bail and try again */
		}
		xtcsetattr(INFD, TCSANOW, &t);
		(void) close(OUTFD);
		if (OUTFD != (ind = open(portname, O_RDWR))) {
			sprintf(err_msg , MSGSTR (M_CANTOPEN, DEF_CANTOPEN),
				portname);
			tsm_err(err_msg, 1, TRUE);
		}
		if (INFD != (ind = dup2(OUTFD, INFD))) {
			sprintf(err_msg, "%s\nINFD != %d\n",
				MSGSTR(M_DUPFAIL, DEF_DUPFAIL), ind);
			tsm_err (err_msg, 255, FALSE);
		}
	} else {
		/* block, wait for carrier (DCD) */
		if (INFD != open(portname, O_RDWR)) {
			sprintf(err_msg , MSGSTR (M_CANTOPEN, DEF_CANTOPEN),
				portname);
			tsm_err(err_msg, 1, FALSE);
		}
	}

	/*
	 * get the actual port name. 
	 */
	newname = ttyname(INFD);

	/*
	 * If name changed, try to restore original port
	 */
	if (strcmp(portname, newname)) {
		if (mode & GETTY_UU)
			(void) portprotection (0662, newname);
		else
			(void) portprotection (0600, newname);
		(void) portprotection (0666, portname);
		portname = newname;
		(void) chown (portname, owner, group);
		for (ind = 0; ind < nsyns; ind++) {
			(void) chown (synonym_list[ind], owner, group);
			(void) portprotection(protection, synonym_list[ind]);
		}
		gettylog (portname);
	}

	xtcgetattr(INFD, &origterm);
	if (mode & GETTY_UU) {
		effective_id = geteuid();
		seteuid(getuid());
		/*
		 * If CLOCAL is turned on and not a local port, then exit.
		 */
		if (origterm.c_cflag & CLOCAL && !local_port) {
			/*
			 * someone/thing left clocal turned on
			 */
			while (ttylock(device)) ttywait(device);
			if (local_hupcl)
				origterm.c_cflag |= HUPCL;
			origterm.c_cflag &= ~(CLOCAL);
			tcsetattr(INFD, TCSANOW, &origterm);
			ttyunlock(device);	/* unlock device */
			exit(0);	/* bail and try again */
		}
		if (mode & GETTY_DELAYED) {
			/*
			 * Block waiting for a character before grabbing the
			 * lock.  This is to support modems that can't drive
			 * carrier, or always drive carrier.
			 */
			newterm = origterm;
			newterm.c_lflag = 0;
			newterm.c_cflag |= CREAD;
			newterm.c_cc[VMIN] = 1;
			newterm.c_cc[VTIME] = 0;

			if (ttylocked(device)) {
				while(ttylock(device)) ttywait(device);
				frevoke(INFD);
				if (local_hupcl)
					origterm.c_cflag |= HUPCL;
				origterm.c_cflag &= ~(CLOCAL);
				tcsetattr(INFD, TCSANOW, &origterm);
				exit(0);	/* bail and try again */
			}
			tcsetattr(INFD, TCSANOW, &newterm);

			FD_ZERO(&selfd);
			FD_SET(0, &selfd);
			if(select(1, &selfd, NULL, NULL, NULL) < 0)
				exit(0);
			if(!ttylock(device))
			{
				*buffer = 0;
				if(read (INFD, buffer, 1) < 0 ||
				   *buffer == '\004')
					exit(0);
				tcsetattr(INFD, TCSANOW, &origterm);
			}
			else
			{
				while(ttylock(device)) ttywait(device);
				frevoke(INFD);
				if (local_hupcl)
					origterm.c_cflag |= HUPCL;
				origterm.c_cflag &= ~(CLOCAL);
				tcsetattr(INFD, TCSANOW, &origterm);
				exit(0);        /* bail and try again */
			}
		} else {
			if(ttylock(device))
			{
				while(ttylock(device)) ttywait(device);
				frevoke(INFD);
				if (local_hupcl)
					origterm.c_cflag |= HUPCL;
				origterm.c_cflag &= ~(CLOCAL);
				tcsetattr(INFD, TCSANOW, &origterm);
				exit(0);        /* bail and try again */
			}
		}
		seteuid(effective_id);
	}

	frevoke(INFD);

	/*
	 * Sleep for 1 second to allow DTR and RTS to stabilize.
	 */
	usleep(250 * 4000);

	/*
	 * Duplicate the file descriptor for the port to descriptors 1 and 2
	 * (stdout and stderr).
	 */
	(void) close(OUTFD);
	if (OUTFD != (ind = dup(INFD))) {
		sprintf(err_msg, "%s\nOUTFD != %d\n",
		    MSGSTR(M_DUPFAIL, DEF_DUPFAIL), ind);
		tsm_err(err_msg, 255, FALSE);
	}
	(void) close(ERRFD);
	if (ERRFD != (ind = dup(INFD))) {
		sprintf(err_msg, "%s\nERRFD != %d\n",
		    MSGSTR(M_DUPFAIL, DEF_DUPFAIL), ind);
		tsm_err(err_msg, 255, FALSE);
	}

	/*
	 * Do the actual mode cycling and prompting for user name.
	 */
	gettysetup (input);

	DPRINTF (("leaving getty\n"));
}

/*
 * NAME: gettylog
 *
 * FUNCTION: Place the getty process in utmp.
 *
 * EXECUTION ENVIRONMENT:
 *	User process.
 *
 * RETURNS:
 *	Nothing.
 */

void
gettylog (char *tty)
{
	struct utmp *ut;		/* pointer to entry in utmp file */
	pid_t pid = getpid();

	setutent();
	while ((ut = getutent()) && ut->ut_pid != pid);
	if (ut) {				/* found it */
		utmp_fill(ut, tty, "", "", pid, LOGIN_PROCESS);
		(void) pututline(ut);
	}
	endutent();
}

void
utmp_fill(struct utmp *ut, char *tty, char *user, char *host, pid_t pid,
	  int type)
{
	if (user) {
		bzero(ut->ut_user, sizeof ut->ut_user);
		strncpy(ut->ut_user, user, sizeof ut->ut_user);
	}
	if (tty) {
		if (!strncmp(tty, "/dev/", 5))
			tty += 5;
		bzero(ut->ut_line, sizeof ut->ut_line);
		strncpy(ut->ut_line, tty, sizeof ut->ut_line);
	}
	ut->ut_type = type;
	ut->ut_pid = pid;
	ut->ut_time = time((time_t *)0);
	if (host) {
		bzero(ut->ut_host, sizeof ut->ut_host);
		strncpy(ut->ut_host, host, sizeof ut->ut_host);
	}
}
