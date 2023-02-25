static char sccsid[] = "@(#)06	1.58.1.23  src/bos/usr/sbin/tsm/tsmlogin.c, cmdsauth, bos41J, 9517B_all 4/26/95 12:23:31";
/*
 * COMPONENT_NAME: (CMDSAUTH) Terminal State Manager
 *
 * FUNCTIONS:	auditlogin, check_utmp, chgportaccess, exitlogin,
 *		getenvvars, interruptmotd, login, loginlog, passwd_timeout,
 *		setdefsenv, showmotd,
 *		tsm_port_in_utmp, tsmlogin, tsmloginsetup, updateloginlog
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/audit.h>
#include <sys/termio.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/priv.h>
#include <sys/access.h>
#include <sys/id.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <login.h>
#include <userpw.h>
#include <usersec.h>
#include <values.h>
#include <pwd.h>		/* for return from getpwent() */
#include <userpw.h>		/* for PW_NAMELEN */
#include <utmp.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>
#include "tsm.h"

static	int	stopmotd = 0;	/* motd stops on interrupt */
static	int	preserve = 0;	/* preserve environment flag */
static	int	termset = 0;	/* TERM ENV variable already set */
static	int	envvarnum = 0;	/* Environment variable number (L?) */

static	struct	utmp	utmp;	/* Entry from /etc/utmp */

/*
 * NAME: getenvvars
 *
 * FUNCTION: parses environment variables from a string
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS:
 *	Nothing.
 */
void
getenvvars(char *string, char **env, int *nenv)
{
	char *ptr, *str;

	while (strlen (string) != 0) {
		ptr = strpbrk (string, " \t");
		if (ptr != NULL) {
			*ptr = '\0';
		}
		if (strncmp ("TERM=", string, 5) == 0) {
			termset = TRUE;
		}
		if (strncmp ("IFS=", string, 4) == 0 ||
		    strncmp ("SHELL=", string, 6) == 0 ||
		    strncmp ("PATH=", string, 5) == 0 ||
		    strncmp ("HOME=", string, 5) == 0) {
			printf (MSGSTR (M_BADENV, DEF_BADENV), string);
		} else {
			if (*nenv == 0)
				env[(*nenv)++] = PENV_USRSTR;
			if (strpbrk (string, "=")) {
				env[(*nenv)++] = string;
			} else {
				str = (char *)malloc (strlen(string) + 5);
				sprintf (str, "L%d=%s", envvarnum++, string);
				env[(*nenv)++] = str;
			}
		}
		if (ptr != NULL) {
			string = ptr + 1;
		} else {
			string = NULL;
		}
	}
}

/*
 * NAME: tsmlogin
 *
 * FUNCTION: perform login function on a port
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	tsmlogin reads a login name or gets it from its list of arguments
 *	and attempts to authenticate that user.
 *
 * RETURNS: Does not return.
 */

void
tsmlogin (ac, av)
int	ac;
char	**av;
{
	char	input[BUFSIZ];	 /* input buffer for user login name */
	char	msg[64];	 /* output error message */
	char	*hostname = 0;	 /* pointer to hostname argument */
	char	*username = 0;	 /* pointer to username argument */
	char    *env[128];	 /* Passed in env variables     */
	char    *ptr;		 /* Temp pointers user to parse
					    the username                 */
	int     nenv =      0;   /* Number of passed in env vars */
	int	snenv;           /* Saved value for nenv */
	int	stermset;        /* Saved value for termset */
	int	senvvarnum;      /* Saved value for envvarnum */
	int	opt;		 /* option flag being processed */
	int	remote = 0;	 /* current login is remote */
	int	try;		 /* count of login attempts */
	int	fflag =0;        /* Login already validated (rlogin/telnet) */
	char	*banner;         /* Login herald to display */

	/*
	 * login is called with arguments from rlogind.  Each of these
	 * arguments must be processed in turn.
	 */
	while ((opt = getopt (ac, av, "h:fp")) != EOF) {
		switch (opt) {
		case 'h':

			/* 
			 * Remote host name.  The ut_host field will be
			 * filled with the argument value.  The invoker
			 * must have a real UID of root.
			 */

			if (getuid()) {
			    sprintf(msg, MSGSTR(M_NOPRIV, DEF_NOPRIV));
			    tsm_err(msg, 1, FALSE);
			}
			hostname = optarg;
			remote++;
			break;
		case 'f':

			/*
			 * Pre-authenticated user.  User will not be
			 * prompted for a password, but the user name
			 * must be given on the command line.
			 */

			if (getuid()) {
			    sprintf(msg, MSGSTR(M_NOPRIV, DEF_NOPRIV));
			    tsm_err(msg, 1, FALSE);
			}
			remote++;
			fflag++;
			break;
		case 'p':
			preserve++;
			break;
		case '?':
			sprintf (msg ,MSGSTR (M_UNKLOGOPT, DEF_UNKLOGOPT), optopt);
			tsm_err (msg, 1, FALSE);
			break;
		default:
			break;
		}
	}

	/*
	 * If there is a remaining argument, use it as the
	 * username.  If there was no username and the -f flag was set,
	 * exit with an error -- -f requires a name!
	 */

	if (av[optind])
		username = av[optind++];
	else if (fflag)
		tsm_err (MSGSTR (M_FAILREMLOG, DEF_FAILREMLOG), 1, FALSE);
	
	_truncate_username(username);

	/*
	 * Check to make sure our pid exists in utmp; if it doesn't, then we
	 * bail out since we weren't execed from the lowest login shell.  We
	 * need the utmp entry to be current so we can check to see if this
	 * is a remote login.
	 */
	if(!check_utmp()) {
		tsm_err (MSGSTR (M_NOTLOWEST, DEF_NOTLOWEST), 1, 7);
	}

	/*
	 * This check relies on the next block of code having been executed
	 * by the login which may have preceeded this login.  It is used to
	 * enforce the rlogin attribute for a user when login is run from
	 * the command line.
	 */

	if (utmp.ut_host[0] != '\0') {
		remote = 1;

		/*
		 * Need to set hostname for check below and so any error
		 * messages giving the hostname will produce the correct
		 * output.
		 */

		if (! hostname) {
			static	char	lhostname[sizeof utmp.ut_host + 1];

			strncpy (lhostname, utmp.ut_host, sizeof utmp.ut_host);
			lhostname[sizeof utmp.ut_host] = 0;
			hostname = lhostname;
		}
	}

	/*
	 * Hostname is required for all remote logins.
	 */

	if (remote) {
		if (! hostname) {
			tsm_err (MSGSTR (M_FAILREMLOG, DEF_FAILREMLOG),1,FALSE);
		}

	}

	/*
	 * The remaining arguments on the command line are environment
	 * variables of the form ENV=VALUE; these should be added to the user's
	 * environment.
	 */
	while (av[optind])
		getenvvars (av[optind++], env, &nenv);
	snenv = nenv;
	stermset = termset;
	senvvarnum = envvarnum;

	/*
	 * If this is login and a username was not passed on the command line,
	 * then our first prompt should be with the full herald from login.cfg;
	 * otherwise, we prompt with "login: ".
	 */
	if (tsm_prog == TSMLOGIN && !username)
		banner = herald;
	else
		banner = NULL;

	/*
	 * Now try MAXTRIES times to log the user into the system.  After that
	 * many failues, we exit.
	 */
	for (try = 0;try < MAXTRIES;try++) {
		struct termios orig_runtty;

		/*
		 * Save the runmodes so that they can be restored back from
		 * what setoptionalmodes() will do to them.
		 */
		orig_runtty = runtty;

		/*
		 * if the username was not provided as an argument,
		 * then it must be prompted for
		 * Invalidates fflag as we should already have the name
		 */
		if (! username) {
			fflag = 0;
			xtcsetattr (INFD, TCSANOW, &logtty);
			getusername(input, banner);
			banner = NULL;
			username = input;
		}

		/*
		 * Set up the runmodes and any special termio processing based
		 * on the user's input (uppercase conversion and nl->cr
		 * mapping).
		 */
		setoptionalmodes(username);

		/*
		 * Grab any environment variables that might be specified after
		 * the login name.
		 */
		ptr = strpbrk (username, " \t");
		if (ptr != NULL) {
			*ptr = '\0';
			ptr++;
			getenvvars(ptr, env, &nenv);
		}

		/*
		 * now call login to handle the authentication of the
		 * user.
		 */
		DPRINTF (("calling login (%s, %s)",
		    username, hostname ? hostname:"unknown"));

		login (username, hostname, env , nenv, fflag,remote);

		username = NULL;
		nenv = snenv;
		termset = stermset;
		envvarnum = senvvarnum;
		runtty = orig_runtty;
	}
	exit (EPERM);
}

/*
 *  NAME: login
 *                                                                    
 *  FUNCTION: authenticates a user and initiates a login session
 *                                                                    
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  NOTES:
 *	This function is called primarily by the login command. It:
 *		1> Identifies and authenticates the user:
 *			Authentication methods are specified by administration
 *		2> Sets the user credentials:
 *			User ids 
 *		3> Establishes the user login environment:
 *			Changes the terminal ownership to the user
 *
 * 	In addition this function:
 *		Shows the message of the day (/etc/motd).
 *		Maintains UTMP files, and LAST LOGIN files.
 *		Cuts an audit record.
 *                                                                   
 * RETURNS: 0 if successful else errno is set and returned as follows:
 *		EINVAL 	if the user argument is not specified 
 *		ENOTTY 	if stdin or stdout or stderr do not specify a tty device
 *		EACCES 	if the terminal ownership and mode cannot be changed
 *		EPERM 	if the user identification and authentication fails
 *		EACCES 	if the user credentials cannot be set
 *		ENOEXEC if the user login shell fails exec 
 */

int
login (uname, hostname, env, nenv, fflag, remote)
char	*uname;			/* user name and environment information */
char	*hostname;		/* hostname from remote system */
char    **env;
int	nenv;
int	fflag;
int	remote;
{
	int	fd;			/* file descriptor for closing files */
	int     mode;			/* Mode to pass into checkuseracct for
					   valdiation (S_LOGIN for local logins)
					   S_RLOGIN for remotes              */
	int	ret;			/* return code from libs call */
	uid_t 	curuid, uid;
	char	term[PATH_MAX];		/* terminal name */
	char 	port[64];
	char	buf[BUFSIZ];		/* buffer used for building prompt
					   and logname                       */
	char	*cp;			/* scratch character pointer */
	char 	ttylist[PATH_MAX*2];	/* list for all names of current tty */
	int	ttymodes = 0622;	/* mode bits for port protection */
	int	semid;
	struct	sembuf	sop;
	int	sak;
	int	trust;
	char	*lname;			/* long user name.  This possibly    */
					/* contains a DCE cell name	     */
	char	*opasswd = NULL;	/* User's old passwd; used for required
					   password change. */
	char	*tpasswd, passwd[MAX_PASS+1];
	char	*msg;			/* parm for authenticate	     */
	int	reenter;		/* parm for authenticate	     */
	extern	int login_restricted;   /* for sysauth routine		     */

        /*
         * Save long user name since this may contain a cell specification.
         * This name will be used with all routines that are DCE cognizant.
         * It will also be saved in the user's environment as the LOGIN
         * environment variable.
         */
        if ((lname = strdup(uname)) == (char *)NULL)
	{
		pmsg (MSGSTR (M_BADLOGIN, DEF_BADLOGIN));
		return(-1);
	}

	/* 
	 * Prepare normal AIX user name (max 8 bytes).  Strip off possible
 	 * cell specification, and truncate long names on a character 
	 * boundary.
	 */
	_truncate_username(lname);
	_normalize_username(uname, lname);

	/*
         * Get the terminal name from the utmp file
         * Getty will already have it but login won't
         */
	portname = ttyname (0);

	/*
	 * validate user account and authenticate user
	 * If the fflag is sent to us from a process with UID == 0
	 * then we don't have to check for a passwd (This is used
	 * in conjunction with telnetd/rlogind).
	 */

	if(getpwnam(lname) == NULL)
	{
		if (!SYSTEM_is_NONE(uname))
		{
			char buf[BUFSIZ];

			sprintf(buf, MSGSTR(M_PASSWD,DEF_PASSWD),lname);
			if(_passwdentry(buf, passwd) == -1)
				passwd_timeout();
		}
		loginfailed(uname, hostname, portname);
		pmsg(MSGSTR(M_BADLOGIN, DEF_BADLOGIN));
		return(-1);
	}

	/*
	 * On remote logins we want to check for the rlogin flag (S_RLOGIN)
	 * in /etc/security/user instead of the login flag (S_LOGIN
 	 */
	mode = (remote) ? S_RLOGIN : S_LOGIN;

	/*
	 * Make a list of the portname and pseudoname to send to 
	 * loginrestrictions so both values can be checked for a match
	 */
	if (strcmp(portname,pseudopname) == 0)
		strcpy(ttylist, portname);
	else {
		strcpy(ttylist, portname);
		strcat(ttylist, ",");
		strcat(ttylist, pseudopname);
	}

	/* Call loginrestrictions, but don't fail yet if the user
	 * is restricted.  Instead, pass this information along to
	 * sysauth (via the login_restricted variable).  Standard
	 * sysauth need to check loginrestrictions before calling
	 * passwdexpired (tsm_chpass) for a possible password change.
	 */

	ret = loginrestrictions(lname, mode, ttylist, &msg);
	login_restricted = (ret) ? 1 : 0;

	/* Need to call enduserdb() here because loginrestrictions
	 * is careless.  If this isn't done, subsequent getuserattr()
	 * calls may return cached information based on the value of 
	 * AUTHSTATE before authentication, instead of after.  This
	 * could lead to local info being returned instead of DCE
	 * (or vice versa).
	 */

	enduserdb();

	if(!fflag)
		if(tsm_authenticate(lname, S_PRIMARY | S_SECONDARY, &opasswd))
		{
			if (opasswd)
				memset(opasswd, 0, strlen(opasswd));
			loginfailed(uname, hostname, portname);
			pmsg (MSGSTR (M_BADLOGIN, DEF_BADLOGIN));
			return(-1);
		}

	/* If the user authentication succeeded, only now we will
	 * let them see any messages from loginrestrictions.
	 */

	if(msg)
	{
		puts(msg);
		free(msg);
		msg = NULL;
	}

	if(login_restricted)
	{
		if (opasswd)
			memset(opasswd, 0, strlen(opasswd));
		loginfailed(uname, hostname, portname);
		return(-1);
	}

	if (preserve)
		termtype = getenv ("TERM");

	if (!termset && termtype) {
		if (nenv == 0)  {
			env[nenv++] = PENV_USRSTR;
		}
		settermtype (env[nenv++] = term);
	}

	/*
	 * If we are login lets get portsak since we don't
	 * have it and need it in the case of tpath = always.
	 * Getty already knows about sak, but check it anyway.
	 * We will tie protective port modes to sak_enabled.
	 * This must be 0600 to prevent users from using
	 * "send" type escape sequences, but it will default
	 * to the conventional 0622.
	 */

	if (tsm_sakenabled(portname)) {
		portsak = "true";
		ttymodes = 0600;
	} else 
		portsak = "false";

	/*
	 * Verify trusted path is required, and if so that the device
	 * is capable of supporting it.
	 */

	if (tpath (uname)) {
		if (!strcmp(portsak, "false")) {
			exitlogin (uname, portname,
			    MSGSTR (M_NOTPATH, DEF_NOTPATH));
		}
	} else if (user_tpath == TPATH_NOSAK)
		portsak = "false";

	/*
	 * Record the login in the UTMP_FILE and WTMP_FILEs.
	 */

	loginlog (uname, portname, hostname);

	/*
	 * Set the terminal ownership and modes.  Fail if the
	 * modes can't be set.
	 */

	getuserattr(lname, S_ID, &uid, SEC_INT);

	if (chgportaccess (lname, portname, uid, ttymodes))
		exitlogin (uname, portname,
		    MSGSTR (M_FAILTERM, DEF_FAILTERM));


	if (tsm_iscurr_console(portname, port) != -1)
		if (chgportaccess (lname, port, uid, ttymodes))
			exitlogin (uname, port,
			    MSGSTR (M_FAILTERM, DEF_FAILTERM));

	/*
	 * The SAK manager might be needed.  If we are getty and this
	 * port has sak enabled, then we fork the users login shell and
	 * become the SAK manager.
	 */

	if (tsm_prog == TSMGETTY && !strcmp(portsak, "true")) {

		/*
		 * Close stdout and stderr so frevoke doesn't kill us.
		 */

		close (1);
		close (2);

		/*
		 * Kill all other processes on this terminal.
		 */

		frevoke (0);

		/*
		 * Register the SAK manager.
		 */

		sak = TCSAKON;
		ioctl (0, TCSAK, &sak);

		/*
		 * Turn on trusted path if the user
		 * SAK'd at some time, AND if the user
		 * is permitted on the trusted path
		 */

		if ( ((! sakrcvd) && (user_tpath == TPATH_ON)) ||
		    user_tpath == TPATH_NOSAK ||
		    user_tpath == TPATH_NOTSH)
			trust = TCUNTRUSTED;
		else {
			trust = TCTRUSTED;
			root_trusted = TRUE;
		}

		ioctl (0, TCTRUST, &trust);

		/*
		 * We are not totally guaranteed that we
		 * are not the controlling terminal.
		 * The above close should do it but
		 * if another process has it open
		 * the device driver never sees the close
		 * and we won't work. 
		 * So lets disassociate ourself from the
		 * terminal.
		 */

		tsmnotty();

		/*
		 * First, we create a semaphore used to tell
		 * the child when it can continue execution.
		 */

		semid = semget (IPC_PRIVATE, 1, 0600);
		sop.sem_num = 0;
		sop.sem_op = 1;
		sop.sem_flg = 0;
		semop (semid, &sop, 1);

		childpid = fork ();
		if (childpid == -1) {
			tsm_err (MSGSTR (M_FAILEXEC, DEF_FAILEXEC), 1,
				 FALSE);
		}

		if (childpid) {

			/*
			 * Parent updates utmp so that the 
			 * new process is a user process
			 * closes 0 to aviod dieing on
			 * revokes and becomes the sakmanager
			 */
			updateloginlog (getpid(), childpid);
			close (0);

			/*
			 * Decrement the semaphore so our child
			 * can continue execution and start the
			 * user's shell.
			 */
			sop.sem_num = 0;
			sop.sem_op = -1;
			sop.sem_flg = 0;
			semop (semid, &sop, 1);

			/*
			 * Start the SAK/activity manager.
			 */
			tsmsakmgr (uname, lname);
		}

		/*
		 * Wait till the semaphore clears to continue
		 * execution.
		 */

		sop.sem_num = 0;
		sop.sem_op = 0;
		sop.sem_flg = 0;
		semop (semid, &sop, 1);

		/*
		 * The semaphore is not needed anymore, so
		 * remove it.
		 */

		semctl (semid, 1, IPC_RMID, 0);

		/*
		 * Dup STDOUT and STDERR from STDIN
		 */

		dup (0);
		dup (0);

		/*
		 * Start a new session ID.  Re-open the
		 * login port and then close it.  This
		 * -should- make the current process the
		 * session leader and the login terminal
		 * the controlling terminal for =this=
		 * process.
		 */

		setsid();
		fd = open (portname , O_RDWR);
		close (fd);

		/*
		 * Somtimes the default process group id 
		 * is not being set correctly 
		 * (has to do with console always being 
		 * open).  So we will always set it 
		 */

		tcsetpgrp (0, getpid ());

	} else { /* if (tsm_prog == TSMGETTY) && ... */
		/*
		 * There is no SAK manager, so turn
		 * off SAK and mark the terminal
		 * UNTRUSTED.
		 */

		sak = TCSAKOFF;
		ioctl (0, TCSAK, &sak);

		trust = TCUNTRUSTED;
		ioctl (0, TCTRUST, &trust);

		/*
		 * Somtimes the default process group id 
		 * is not being Set correctly 
		 * (has to do with console always being 
		 * open and only if compiled on R2). 
		 * SO we will always set it 
		 */

		tcsetpgrp (0, getpid ());
	}
	#ifdef	DEBUGX
	printf ("sak = %s\n", sak == TCSAKON ? "on":"off");
	printf ("trust = %s\n", trust == TCTRUSTED ? "on":"off");
	#endif
	/* 
	 * Reset the various signals which have been
	 * assigned to signal handlers or set to be
	 * ignored.
	 */

	signal (SIGUSR1, SIG_DFL);
	signal (SIGSAK, SIG_IGN);
	signal (SIGINT, SIG_DFL);
	signal (SIGQUIT, SIG_DFL);
	signal (SIGHUP, SIG_DFL);
	signal (SIGMSG, SIG_DFL);

	loginsuccess(uname, hostname, portname, &msg);
	showmotd (uname, msg);
	if(msg)
		free(msg);

	if (setpcred (lname, NULL) != 0) {
		/*
		 * Unable to set the credentials, log the failure and
		 * exit.
		 */
		exitlogin (uname, portname,
		    errno != EPERM ? MSGSTR (M_FAILCRED, DEF_FAILCRED)
		    : MSGSTR (M_NOPRIV, DEF_NOPRIV));
	}

	/*
	 * Grab the message from the message catalog in case the
	 * setpenv() fails.  The setpenv() call will close all our
	 * file descriptors; if we do this catgets() after the
	 * setpenv(), it will return the default message instead of
	 * the translated message.
	 */

	cp = MSGSTR (M_FAILEXEC, DEF_FAILEXEC);

	/*
	 * Start the user's shell.  The user will get the login
	 * shell specified in /etc/passwd unless one of the
	 * following is true:
	 *  a) tpath=always for this user, or
	 *  b) tpath=on for this user, sak is enabled for this
	 *     port, and the user hit the sak key during the
	 *     login process,
	 * in which case the user will get the trusted shell.
	 * In the first case, a check was already done above to
	 * disallow a tpath=always user from logging into a
	 * terminal that has sak disabled.
	 */

	if (( user_tpath == TPATH_ALWAYS ) ||
	    (( user_tpath == TPATH_ON ) &&
	     ( trust == TCTRUSTED ))) {

		/*
		 * The user should get the trusted shell as
		 * their initial shell.
		 */

		if (nenv == 0)
			env[nenv++] = PENV_USRSTR;

		env[nenv++] = "SHELL=/usr/bin/tsh";

		/*
		 * Now start the trusted shell.
		 */

		setdefsenv (env, &nenv, uname, lname);
		setpenv(lname, PENV_INIT | PENV_KLEEN, env,
			"/usr/bin/tsh");
	} else {
		/*
		 * Now start the user's login shell.
		 */

		setdefsenv (env, &nenv, uname, lname);
		setpenv(lname, PENV_INIT | PENV_KLEEN, env, NULL);
	}

	/*
	 * Failed trying to exec() the login shell, exit
	 * with a failure.
	 */

	exitlogin (uname, portname, cp);
}

/*
 *  NAME:	interruptmotd
 *
 *  FUNCTIONS:	Catch an interrupt signal and flag it
 *
 *  EXECUTION ENVIRONMENT:
 *	Signal handler
 *
 *  RETURNS:
 *	No return value
 */

void
interruptmotd(void)
{
	stopmotd = 1;
}

/*
 *  NAME:	showmotd
 *
 *  FUNCTION:	Write a file to the screen (stdin) 
 * 
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS:
 *	No return value
 */

void
showmotd (char *user, char *lastloginfo)
{
	FILE	*fp;
	register int	c;
	int 	newline = FALSE;
	char localhost[MAXHOSTNAMELEN + 1];
	char *home;
	char hush[PATH_MAX];

	/*
	 * if ".hushlogin" exists in the user's home directory we don't
	 * display any messages during login time.
	 */
	if (getuserattr(user, S_HOME, &home, '\0') == 0) {
		strcpy (hush, home);
		strcat (hush, "/.hushlogin");
		if (access(hush, F_OK) == 0)
			return;
	}

	/*
	 * Initialize stopmotd flag and SIGINT signal handler.
	 * The SIGINT handler will reset by exec().
	 */

	stopmotd = 0;
	signal (SIGINT, (void (*) (int))interruptmotd);

	/*
	 * Open the MOTD file and print while there are contents
	 * remaining or until the user presses the <INTR> key.
	 */

	if ((fp = fopen ("/etc/motd", "r")) != NULL) {
		while (! stopmotd && (c = getc (fp)) != EOF)
			putchar (c);

		(void) fclose (fp);
	}

	puts(lastloginfo);
}

/*
 *  NAME:	exitlogin
 *
 *  FUNCTION:	audit failure and exit
 *
 *  RETURNS:
 *	No return value
 */

void
exitlogin (uname, ttyn, msg)
char	*uname;		/* user logging in */
char	*ttyn;		/* Terminal name */
char	*msg;		/* error message or NULL if successful */
{

	/*
	 * Cut an audit record and return the errno code.
	 */

	auditlogin (uname, msg, AUDIT_FAIL);

	if (! errno)
		errno = EACCES;

	exit (errno);
}

/*
 * NAME: auditlogin
 *
 * FUNCTION: Print a msg if specified and audit the result of login
 *
 * RETURNS: void
 *
 */

void
auditlogin (uname, msg, result)
char	*uname;			/* user logging in */
char	*msg;			/* error message or NULL if successful */
int	result;			/* result of login: 0 success, 1 failure */
{
	if (msg)
		pmsg (msg);

	(void) auditwrite ("USER_Login", result,
	    uname, strlen (uname) + 1, NULL);
}

/*
 * NAME: check_utmp
 *
 * FUNCTION: check for a utmp entry matching the current process pid
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	Returns true if a utmp entry exists that matching our current process
 *	id; false otherwise.
 *
 * RETURNS:
 *	int
 */

int
check_utmp()
{
	struct utmp *ut;
	pid_t pid = getpid();

	setutent();				/* open/rewind utmp */
	while ((ut = getutent()) && ut->ut_pid != pid);

	if (ut)
		utmp = *ut;

	endutent();
	return(ut != NULL);
}

/*
 * NAME: loginlog
 *                                                                    
 * FUNCTION: log the login
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * NOTES:
 *	record login in utmp and wtmp files
 *                                                                   
 * RETURNS:
 *	void
 */

void
loginlog (user, tty, hostname)
char	*user;
char	*tty;
char	*hostname;
{
	struct utmp *ut;
	pid_t pid = getpid();

	setutent();				/* open/rewind utmp */
	while ((ut = getutent()) && ut->ut_pid != pid);
	if (ut) {				/* found it */
		utmp_fill(ut, tty, user, hostname, pid, USER_PROCESS);
		(void) pututline(ut);
	} else {
		/*
		 * If a utmp entry does not exist with our pid in it, then the
		 * user must have another shell running above the lowest; in
		 * which case we exit.
		 */
		exitlogin (user, tty, MSGSTR (M_NOTLOWEST, DEF_NOTLOWEST));
	}

	/*
	 * The utmp entry is appended to the wtmp file to maintain a log
	 * of all login activity.
	 */
	(void) append_wtmp(WTMP_FILE, ut);

	endutent();				/* close utmp */
}

/*
 *  NAME: chgportaccess - change port access rights
 *                                                                    
 *  FUNCTION: change ownership and mode of the terminal on behalf of the user
 *                                                                    
 *  EXECUTION ENVIRONMENT:
 *	User process
 *
 *  RETURNS: 0 if successful, anything else indicates a failure
 */

int
chgportaccess (char *user, char *tty, uid_t uid, int ttymodes)
{
	gid_t	gid;		/* gid to change to */
	char	*pgrp;		/* primary group for user */
	int     ind;

	/*
	 * Get the primary GID for this user.
	 */

	if (getuserattr (user, S_PGRP, &pgrp, '\0') ||
	    grouptoID (pgrp, &gid) != 1)
		return -1;

	/*
	 * Change the device ownership and file modes for
	 * the new login user.
	 */

	/*
 	 * Can the permissions on the port as well as any synonyms
 	 * 
 	 */

	(void) chown (tty, uid, gid);
	(void) portprotection (ttymodes, tty);

	/*
 	 * If this is the orignal port change any synonyms as well
 	 */
	if (strcmp (tty , portname) == 0) {
		for (ind = 0; ind < nsyns; ind++ )	{
			(void) chown (synonym_list[ind], uid, gid);
			(void) portprotection (ttymodes, synonym_list[ind]);
		}
	}

	return 0;
}

/*
 * NAME: setdefsenv
 *
 * FUNCTION: Set the default system envronment varaibles
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *
 * RETURNS: 
 */

void
setdefsenv (char **env , int *nenv , char *user, char *lname)
{
	char *buf;

	/*
	 * Place LOGIN into regular environment.  Malloc the data area in 
	 * case the env variable is modified somewhere else
	 */
	if ((buf = (char *)malloc
	    (sizeof("LOGIN=") + strlen(lname))) == (char *)NULL)
		return;

	if (*nenv == 0)
		env[(*nenv)++] = PENV_USRSTR;

	strcpy(buf, "LOGIN=");
	strcat(buf, lname);
	env[(*nenv)++] = buf;

	/*
	 * Tell setpenv these are SYSTEM variables
	 */
	env[(*nenv)++] = PENV_SYSSTR;

	/*
	 * Place LOGNAME into protected environment.  Malloc the data area in 
	 * case the env variable is modified somewhere else
	 */
	if ((buf = (char *)malloc
	    (sizeof("LOGNAME=") + strlen(user))) == (char *)NULL)
		return;

	strcpy(buf, "LOGNAME=");
	strcat(buf, user);
	env[(*nenv)++] = buf;

	/*
	 * End of environment variables
	 */

	env[(*nenv) ++] = (char *) 0;
}

/*
 * NAME: updateloginlog
 *                                                                    
 * FUNCTION: Place the child process pid in the login table
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *
 * RETURNS: void
 */

void
updateloginlog (int curpid, int newpid)
{
	struct utmp *ut;

	setutent();
	while ((ut = getutent()) && ut->ut_pid != curpid);
	if (ut) {				/* found entry */
		ut->ut_pid = newpid;		/* stuff in new pid */
		(void) pututline(ut);		/* write it out */
	}
	endutent();				/* close and go home */
}

/*
 * NAME: tsm_port_in_utmp
 *
 * FUNCTION: Determine if the given port is in /etc/utmp and is an init, login,
 *	     or user process.
 *
 * EXECUTION ENVIRONMENT:
 *	User process
 *
 * RETURNS:
 *	The type of process on the port if the process is an init, login, or
 *	user process, EMPTY otherwise. 
 */

int
tsm_port_in_utmp(char * port)
{
	register struct utmp *ut;
	register int t;
	register int ret = EMPTY;

	if (!strncmp(port, "/dev/", 5))
		port += 5;
	setutent();				/* rewind */
	while (ut = getutent())
		if (((t = ut->ut_type) == LOGIN_PROCESS ||
		    t == USER_PROCESS ||
		    t == INIT_PROCESS) &&
		    !strncmp(ut->ut_line, port, sizeof(ut->ut_line))) {
			ret = ut->ut_type;
			break;
		}
	endutent();
	return ret;
}

void
tsmloginsetup()
{
	/*
	 * Get the terminal name from the utmp file
	 */
	portname = ttyname (0);

	/*
	 * ODM needs the logical name in some cases if we
	 * are here we are login and have not got the
	 * logical name yet so get it
	 */

	if (tsm_iscurr_console(portname , NULL)  == -1)
		pseudopname = portname;
	else 
		pseudopname = TSM_CONSOLE;

	/*
 	 * Get and set all the port characteristics
 	 */

	getparms ();
	initset ();
	setmodes ();
	setownership ();
	setprotection ();
	setdefaultmodes ();
}

/*
 * The write did not finish presumably because of noise on the tty
 * port.  We carefully turn off echo, flush input and output, do a
 * resume and an unblock, and then exit.
 */
void
passwd_timeout()
{
	runtty.c_iflag = 0;			/* turn off everything */
	runtty.c_oflag = 0;
	runtty.c_lflag = 0;
	xtcsetattr(INFD, TCSANOW, &runtty);
	tcflush(INFD, TCIOFLUSH);		/* flush input and output */
	tcflow(INFD, TCOON);		/* resume */
	tcflow(INFD, TCION);		/* unblock */
	tsm_err(MSGSTR(M_NOPAS, DEF_NOPAS), 1, TRUE);
}

void
readpasswd(char *message, char *passwd)
{
	if(_passwdentry(message, passwd))
		passwd_timeout();
}

