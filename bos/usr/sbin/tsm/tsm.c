static char sccsid[] = "@(#)18  1.49.2.11  src/bos/usr/sbin/tsm/tsm.c, cmdsauth, bos411, 9438B411a 9/20/94 12:02:10";
/*
 * COMPONENT_NAME: (CMDSAUTH) security: authentication functions
 *
 * FUNCTIONS:	getusername, main, onlogout, onsak, onquit, portprotection,
 *		revoke_line, runtsh, setsignals, settsmenv, tpath, tsm,
 *		tsm_sakenabled, tsmbld_saklist, tsmlogout, tsmnotty,
 *		tsmqry_saklist, tsmsakmgr, tsmset_sakval, xalloc, xioctl,
 *		xtcgetattr, xtcsetattr
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/audit.h>
#include <sys/termio.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/priv.h>
#include <sys/acl.h>
#include <sys/vnode.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>	/* for ILS */
#include <login.h>	/* for interface to login () */
#include <userpw.h>
#include <usersec.h>	/* for S_NAMELEN */
#include <locale.h>	/* for LC_ALL */
#include <limits.h>
#include <fcntl.h>
#include "tsm.h"
/* added for console support */
#include <sys/mode.h>
#include <sys/console.h>
#include <sys/sysconfig.h>
#include <sys/device.h>

nl_catd catd;		/* The master of all tsm CATDs */

int	multibytecodeset = 0;	/* Flag for single or multibytes */

char	*sak_default = "false";

TSMSAK_LIST_t list_head = NULL;

/*
 * Set to TSMGETTY if we are running getty else TSMLOGIN.
 */
int tsm_prog;

#ifdef DEBUGX
FILE	*errfp;
#endif

/************************/
/* 	Globals 	*/
/************************/

char	*Progname;

char	*portname;	/* Name of actual login port 			*/
char	*pseudopname;	/* Name of requested login port 		*/
char	*synonym_list[TSM_MAX_SYNS];/* NULL terminated list of synonyms */
char	*portsak;	/* SAK attribute from /etc/ports for portname 	*/
int 	nsyns = 0;      /* number if synonyms associated with this port */
int	saked = 0;	/* Flag indicating SIGUSR1 was caught 		*/
int	sakrcvd = 0;	/* Flag indicating SIGUSR1 was ever caught 	*/
int	logout = 0;	/* Flag indicating SIGMSG was caught for logout */
int	root_trusted = FALSE;	/* Has the root window ever been trusted*/
tpath_t	user_tpath;	/* Enumerated type indicated SAK processing 	*/
pid_t	childpid = -1;	/* Process ID of login shell			*/

/*
 *  NAME: 	main - terminal state manager
 *
 *  FUNCTION: 	Manages the terminal for a login session
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	Command.
 *
 *  NOTES:
 *	This command performs the 'getty' and 'login' functions.
 *	It can be invoked by:
 *  	USAGE:
 *	getty  [-u] [-r]
 *		-d	debugging
 *		-u	means bi-directional communication
 *		-r	means same as -u with an initial delay
 *
 *	login [ -h <host> [ -f ] ] [ user ]
 *		-h <host> remote hostname
 *		-f <remote-user> name of user on remote host
 *
 *	First, decide what name this command was invoked as and call the
 *	appropriate function. Also, if MAXLOGINS is defined check to see
 *	if the maximum number of logins has been exceeded.
 *
 *  RETURNS:
 *	If this command fails it will exit with an error code > 0.
 */

int
main (ac, av)
int	ac;
char	*av[];

{
	char *classPtr ;

	/*
	 * Set environment variables.  Some of these we need like "LANG" and
	 * "ODMDIR".
	 */
	settsmenv();

	/*
	 * Open the message and initialize any location dependent information.
	 */
	setlocale (LC_ALL, "");
	catd = catopen (MF_TSM, NL_CAT_LOCALE);

	/*
	 * Initialize multibytecodeset to support SSDP (single/multibyte path).
	 */
	if (MB_CUR_MAX > 1)
		multibytecodeset = 1;

	/*
	 * Setup syslog so it will not do an open until it needs to and let it
	 * know who we are.
	 */
	openlog("tsm", LOG_ODELAY, LOG_AUTH);

	#ifdef	DEBUGX
	/*
	 * Open a stream for printing debugging messages
	 */
	if (! (errfp = fopen (TSM_CONSOLE, "w")))
		errfp = stderr;
	else
		setbuf (errfp, 0);
	#endif

	/*
	 * Set up the signals.  SIGUSR1 must be enabled so trusted
	 * path can be established inside of TSM.
	 */
	setsignals ();

	/*
	 * Initialize auditing.
	 */
	setuserdb(S_READ);
	if ( ! getuserattr("root", S_AUDITCLASSES, &classPtr, SEC_LIST) )
	{
		int len;

		for(len = 0; classPtr[len] || classPtr[len + 1]; len++)
			;

		len += 2;	/* we must count the double nulls that
                                   terminate this list */

		(void) auditproc(0, AUDIT_EVENTS, classPtr, len);
	}
	enduserdb();
	auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);

	/*
	 * Extract the program name from the argument list.  There may
	 * be some leading crud, so step over it.
	 */
	if (Progname = strrchr (av[0], '/'))
		Progname++;
	else
		Progname = av[0];

	/*
	 * tsm may be called as "login" by a network daemon.  Go simulate
	 * the login if Progname has "login" in it.
	 */

	if (strstr (Progname, TSM_LOGIN)) {

		tsm_prog = TSMLOGIN;
		tsmloginsetup();
		tsmlogin (ac, av);

	}

	tsm_prog = TSMGETTY;
	tsmnotty();

	/*
	 * Executed either as getty or tsm.  Who cares, call main tsm
	 * routine to perform getty operations and set up terminal state
	 * management.
	 */
	tsm (ac, av);

	/*NOTREACHED*/
}

/*
 *  NAME: 	tsm - terminal state manager
 *
 *  FUNCTION: 	establishes terminal characteristics and determines whether
 *		or not terminal management is needed.
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	This function establishes the initial state of your terminal,
 *	logs in the user, and then manages the login
 *	session. This management involves handling SAK (secure 
 *	attention key) signals. If SAK management is not requested then
 *	the user's login session is exec'd otherwise the login
 *	session is fork'd and this parent process stays. 
 *
 *  RETURNS:
 *	If this command fails it will exit with an error code > 0.
 */

void
tsm (ac, av)
int	ac;			/* arg count */
char	**av;			/* flags and port name */
{
	int	mode = 0;	/* mode for getty routine */
	char	*ptr;		/* for parsing arguments */
	char	input[BUFSIZ];	/* the buffer for input from the terminal */
	char	*newargv[5];	/* Faked-up arguments for tsmlogin */
	int	newargc = 0;	/* Count of faked-up arguments for tsmlogin */
	struct	stat	stat_buf;
	struct  qry_devsw qdevsw;
	struct  cons_config config;
	struct  cfg_dd cfgdata;        /* Config DD   */
	static char cpath[256];

	/*
	 * Parse arguments -
	 *
	 *	-u -- bi-directional getty
	 *	-r -- bi-directional with wait for a carriage return
	 */

	for (av++;*av && **av == '-';av++) {
		for (ptr = *av + 1; *ptr; ptr++) {
			switch (*ptr) {
			case 'u':
				mode |= GETTY_UU;
				break;
			case 'r':
				mode |= GETTY_UU | GETTY_DELAYED;
				break;
			default:
				break;
			}
		}
	}

	/*
	 * There must be at least one remaining argument, which is the
	 * port name to play getty on.
	 */

	if (! (portname = *av)) {
		char err_msg[256];
		sprintf (err_msg , MSGSTR (M_NOARGS, DEF_NOARGS));
		tsm_err (err_msg , 1, FALSE);
	}

	/*
	 * pseudopname will represent the actual name passed while 
	 * portname may change if the name passed in is a
	 * pass-thru port (ie /dev/console)
	 */
	pseudopname = portname;

	/*
	 * Check if name passed in is a logical name representing
	 * A different physical name 
	 */

	if ( stat(portname, &stat_buf) == 0) {
		if (S_ISCHR(stat_buf.st_mode))
		{
			qdevsw.devno = stat_buf.st_rdev;
			sysconfig (SYS_QDVSW, &qdevsw, sizeof(qdevsw));
			if (qdevsw.status & DSW_CONSOLE)
			{
				cfgdata.kmid = 0;
				cfgdata.devno = stat_buf.st_rdev;
				cfgdata.cmd = CONSOLE_CFG;
				cfgdata.ddsptr = (char *) (&config);
				cfgdata.ddslen = sizeof(config);
				config.cmd = CONSGETDFLT;
				config.path = cpath;
				if(sysconfig( SYS_CFGDD, &cfgdata,
				    sizeof(struct cfg_dd)) ==0 )
					portname = cpath;
			}
		}
	}


	/*
	 * Initialize the port and try to read in the user name.
	 */

	tsmgetty (mode, input);

	/*
	 * Build the argument list for "login".  Only one string may be
	 * entered as input and it must be the user name.  Use the "--"
	 * argument to signal getopt() that there are no flags present.
	 */

	newargv[newargc++] = "login";
	if (*input) {
		newargv[newargc++] = "--";
		newargv[newargc++] = input;
	}
	newargv[newargc] = NULL;

	tsmlogin (newargc, newargv);

	/*NOTREACHED*/
}

/*
 * NAME:	tsmlogout
 *
 * FUNCTION:	Terminate a user session
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	tsmlogout performs logout cleanup processing for TSM.  The port
 *	is cleaned and the user's login shell killed.
 *
 * RETURNS:
 *	This routine never returns.
 */

void
tsmlogout ()
{
	int	ind, fd;
	int	trust = TCUNTRUSTED;
	int	sak = TCSAKOFF;
	char    console_port[64];

	/*
	 * First, we do a revoke on each port (and the console if we are the
	 * console) and the synonyms.  We also reset the trustedness and sak
	 * manager for these ports.
	 */

	if ((tsm_iscurr_console (portname , console_port)) != -1) {
		fd = open (console_port, O_RDWR|O_NOCTTY|O_NDELAY);
		ioctl (fd, TCTRUST, &trust);
		ioctl (fd, TCSAK, &sak);
		close (fd);
		revoke_line (console_port);
	}

	fd = open (portname, O_RDWR|O_NOCTTY|O_NDELAY);
	ioctl (fd, TCTRUST, &trust);
	ioctl (fd, TCSAK, &sak);
	close (fd);
	revoke_line (portname);

	for ( ind = 0; ind < nsyns; ind++ ) {
		fd = open (synonym_list[ind], O_RDWR|O_NOCTTY|O_NDELAY);
		ioctl (fd, TCTRUST, &trust);
		ioctl (fd, TCSAK, &sak);
		close (fd);
		revoke_line (synonym_list[ind]);
	}

	/*
	 * Kill the child root process and the process group as well.  If
	 * childpid is -1, then we haven't even logged in yet and there
	 * is no child shell to kill off; so we do nothing.
	 */

	if (childpid != -1) {
		killpg (childpid, SIGKILL);
		kill (childpid, SIGKILL);
	}

	exit (0);
}

/*
 * NAME:	tsmsakmgr
 *
 * FUNCTION:	Perform SAK key management
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 * NOTES:
 *	Manages SAK during a login session.  Called if login device requires
 *	SAK management.  tsmsakmgr manages the SAK signals sent from the
 *	terminal device driver.
 *
 * RETURNS:
 *	This routine never returns.
 */

void
tsmsakmgr (user, luser)
char	*user;			/* user name of login user (max 8 bytes) */
char	*luser;			/* user's complete login name 	         */
{
	pid_t	pid;
	int	status;
	int 	old_child = 0;          /* Pid of last root child to die */
	char	msg[BUFSIZ];

	/*
	 * Sanity check the root child  process.  Several assumptions are
	 * made based on the child process existing and we must make
	 * an effort to insure the child is really existent.
	 */

	if (kill (childpid, 0) == -1) {
		tsm_err (MSGSTR (M_FAILEXEC, DEF_FAILEXEC), 1, FALSE);
	}

	/*
	 * Main loop.
	 *
	 * Loop waiting on children to die or signals to be received.  When
	 * a child dies its process ID is checked against the login process
	 * ID.
	 *
	 * There are two signals which can be received: SIGSAK and SIGLOGOUT
	 * [ which is really SIGMSG ].  SIGSAK causes the user to be placed
	 * in the trusted shell, if permitted.  SIGLOGOUT causes the entire
	 * login session to be terminated.
	 */

	while (1) {

		/*
		 * Wait for an event.  The return status will be used to
		 * decide if this is a login process or a signal causing
		 * wait to return.
		 */

		pid = wait (&status);

		/*
		 * If our child has exited or we have received the logout
		 * signal, then we log the user off and exit.
		 */

		if ((pid == childpid) || logout) {
			tsmlogout();
		}

		/*
		 * Test for SAK key.
		 *
		 * A SAK key was detected.  The Trusted Shell needs to be
		 * executed if the user has permission to be on the trusted
		 * path.  If not, we exit with an error message.
		 */

		if (saked) {

			setsignals ();
			saked = 0;

			/*
			 * Open the port so that DTR does not drop when we kill
			 * the login shell currently running on the port.
			 */

			if(INFD != open(portname, O_RDWR | O_NOCTTY)) {
				sprintf(msg, MSGSTR(M_CANTOPEN, DEF_CANTOPEN),
					portname);
				tsm_err(msg, 1, FALSE);
			}

			/*
			 * Kill the login shell.
			 */

			kill (childpid, SIGKILL);

			/*
			 * Wait for the login shell to exit.
			 */

			while ((pid = wait (0)) != childpid && pid != -1) {
			}

			/*
			 * Run the trusted shell for the user.
			 */

			old_child = childpid;
			childpid = runtsh (portname, user, luser);

			/*
			 * Update /etc/utmp to allow logins to be successfully
			 * run from the new shell.
			 */

			updateloginlog (old_child, childpid);
		}

		/*
		 * This was just some random signal which woke the
		 * process up out of the wait() call.  Continue on as
		 * if nothing happened ...
		 */
	}

	tsmlogout ();
}

/*
 * NAME: runtsh
 *
 * FUNCTION: Execute trusted shell on current login port.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	User process.  Creates a sub-process invoking the trusted
 *	shell.
 *
 * RETURNS: Process ID of new child, or -1 if an error occured.
 */

int
runtsh (char *sakport, char *user, char *luser)
{
	int 	ret;
	int	pid = 0;
	int     truston  = TCTRUSTED;
	int     nenv = 0;
	int     fd;
	char    *env[8];
	char    port[64];
	char	term[PATH_MAX];
	int	semid;
	struct	sembuf	sop;

	/*
	 * If the user is not allowed to be on the trusted path, then print out
	 * a message to that effect and log them off.
	 */

	if (user_tpath == TPATH_NOTSH) {
		char errmsg[255];

		sprintf(errmsg, "\n%s\n", MSGSTR (M_NOTSH, DEF_NOTSH));
		write(0, errmsg, strlen(errmsg));
		tsmlogout ();
	}

	/*
	 * Clean up the port.  Set the port protection so only the owner can
	 * read/write to it, kill all other process that might have the port
	 * open, and turn on the trustedness of the port.
	 */

	portprotection (0600 , sakport);

	frevoke(0);

	ioctl (0, TCTRUST, &truston);

	/*
	 * Disassociate ourselves with our controlling terminal.
	 */

	tsmnotty();

	/*
	 * Create a semaphore used to tell the child when it can continue
	 * execution.
	 */

	semid = semget (IPC_PRIVATE, 1, 0600);
	sop.sem_num = 0;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	semop (semid, &sop, 1);

	if (pid = fork ()) {

		root_trusted = TRUE;
		close (0);

		/*
		 * Decrement the semaphore so our child can continue execution
		 * and start the user's trusted shell.
		 */
		sop.sem_num = 0;
		sop.sem_op = -1;
		sop.sem_flg = 0;
		semop (semid, &sop, 1);

		return (pid);

	}

	/*
	 * Wait till the semaphore clears to continue execution.
	 */

	sop.sem_num = 0;
	sop.sem_op = 0;
	sop.sem_flg = 0;
	semop (semid, &sop, 1);

	/*
	 * The semaphore is not needed anymore, so remove it.
	 */

	semctl (semid, 1, IPC_RMID, 0);

	/*
	 * Dup STDOUT and STDERR from STDIN
	 */

	dup (0);
	dup (0);

	/*
	 * Reset the session id so that frevoke can work
	 * This MUST be done before the open as controlling terminal
	 */

	setsid ();
	setpgrp ();
	ret = open (sakport, O_RDWR );
	close (ret);

	/*
	 * ensure we are controlling process group
	 */

	tcsetpgrp (0, getpid ());

	/*
	 * Reset the port settings
	 */

	tcsetattr(0, TCSANOW, &runtty);

	/*
	 * If we are the console lets deal with it too
	 */

	if ((tsm_iscurr_console ( sakport , port)) != -1) {
		portprotection (0600 , port);
		revoke (port);
		fd = open (port , O_RDWR);
		ioctl (fd , TCTRUST , &truston);
		tcsetpgrp (fd, getpid ());
		close (fd);
	}

	/*
	 * Re-initialize terminal modes and run trusted shell
	 */

	env[nenv++] = PENV_USRSTR;
	env[nenv++] = "SHELL=/usr/bin/tsh";

	if (termtype) {
		settermtype (env[nenv++] = term);
	}

	setdefsenv (env , &nenv , user, luser);
	if (setpcred (luser, NULL) != 0) {
		/*
		 * Unable to set the credentials, so we exit.
		 */
		tsm_err (errno != EPERM ? MSGSTR (M_FAILCRED, DEF_FAILCRED)
					: MSGSTR (M_NOPRIV, DEF_NOPRIV), 1,
			 FALSE);
	}
	setpenv (luser, PENV_INIT | PENV_KLEEN, env, "/usr/bin/tsh");
	tsm_err (MSGSTR (M_FAILEXEC, DEF_FAILEXEC), 1, FALSE);

	/*NOTREACHED*/
}

/*
 *  NAME:	onsak
 *
 *  FUNCTION:	Catch SIGUSR1
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	Signal handler
 *
 *  RETURNS:
 *	Implicit return to longjump setter
 */

void
onsak ()
{
	DPRINTF (("onsak received"));

	saked = 1;
	sakrcvd = 1;
}

/*
 *  NAME:	onlogout
 *
 *  FUNCTION:	Catch SIGMSG and interpret as logout signal
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	Signal handler
 *
 *  RETURNS:
 *	Implicit return to longjump setter
 */

void
onlogout()
{
	DPRINTF (("rec'd logout signal"));

	logout = 1;
}

/*
 *  NAME:	onquit
 *
 *  FUNCTION:   Catch SIGQUIT and exit
 *
 *  EXECUTION ENVIRONMENT:	Signal handler
 *
 *  RETURNS:	exits
 */

void
onquit()
{
        DPRINTF (("received quit signal"));

        tsmlogout();
}

/*
 *  NAME:	setsignals
 *
 *  FUNCTION:	Set SIGUSR1 and SIGMSG signal catchers
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  RETURNS:
 *	No return value
 */

void
setsignals ()
{
	struct sigaction sa;

	sa.sa_handler = onsak;
	sa.sa_mask.losigs = 0;
	sa.sa_mask.hisigs = 0;
	sa.sa_flags = 0;
	sigaction (SIGUSR1, &sa, (struct sigaction *)NULL);
	sigaction (SIGSAK, &sa, (struct sigaction *)NULL);

	sa.sa_handler = onlogout;
	sa.sa_mask.losigs = 0;
	sa.sa_mask.hisigs = 0;
	sa.sa_flags = 0;
	sigaction (SIGMSG, &sa, (struct sigaction *)NULL);
	sigaction (SIGHUP, &sa, (struct sigaction *)NULL);

        sa.sa_handler = onquit;
        sa.sa_mask.losigs = 0;
        sa.sa_mask.hisigs = 0;
        sa.sa_flags = 0;
        sigaction (SIGQUIT, &sa, (struct sigaction *)NULL);

}

/*
 *  NAME:	xioctl
 *
 *  FUNCTION:	ioctl call with error checking
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  RETURNS:
 *	Result of ioctl call.  Prints error message and exits on error.
 */

int
xioctl (fd, cmd, arg)
int	fd;
int	cmd;
char	*arg;
{
	int	rc; 		/* Return code of ioctl 	*/
	char    err_msg[256];   /* error message to report      */

	if ((rc = ioctl (fd, cmd, arg)) < 0) {
		sprintf(err_msg , MSGSTR(M_IOCTLFAIL, DEF_IOCTLFAIL), errno);
		tsm_err(err_msg, 1, FALSE);
	}

	return rc;
}

/*
 *  NAME:	xtcgetattr
 *
 *  FUNCTION:	tcgetattr call with error checking
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  RETURNS:
 *	Result of tcgetattr call.  Prints error message and exits on error.
 */

int
xtcgetattr(fd, arg)
int	fd;
struct termios *arg;
{
	int	rc; 		/* Return code of tcgetattr 	*/
	char    err_msg[256];   /* error message to report      */

	if ((rc = tcgetattr(fd, arg)) < 0) {
		sprintf(err_msg, MSGSTR(M_TCGATTRFAIL, DEF_TCGATTRFAIL), errno);
		tsm_err(err_msg, 1, FALSE);
	}

	return rc;
}

/*
 *  NAME:	xtcsetattr
 *
 *  FUNCTION:	tcsetattr call with error checking
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  RETURNS:
 *	Result of tcsetattr call.  Prints error message and exits on error.
 */

int
xtcsetattr (fd, cmd, arg)
int	fd;
int	cmd;
struct termios *arg;
{
	int	rc; 		/* Return code of tcsetattr 	*/
	char    err_msg[256];   /* error message to report      */

	if ((rc = tcsetattr (fd, cmd, arg)) < 0) {
		sprintf(err_msg, MSGSTR(M_TCSATTRFAIL, DEF_TCSATTRFAIL), errno);
		tsm_err(err_msg, 1, FALSE);
	}

	return rc;
}

/*
 *  NAME:	xalloc
 *
 *  FUNCTION:	malloc with error checking.  Zero returned region.
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  RETURNS:
 *	Pointer to allocated region.  Prints error message and exits on error.
 */

char *
xalloc (len)
int	len;
{
	char	*p;

	if ((p = malloc (len)) == NULL)
		tsm_err(MSGSTR (M_ALLOCFAIL, DEF_ALLOCFAIL), 1, FALSE);

	memset (p, 0, len);
	return (p);
}

/*
 *  NAME:	getusername 
 *
 *  FUNCTION:	prompt for and return a user name
 *
 *  EXECUTION ENVIRONMENT:
 *	
 *	User process
 *
 *  NOTES:
 *	This routine is called after terminal characteristics are
 *	established.  It is called from the 'getty' as well as the
 *	'login' functions of tsm.  It is called to get the name of
 *	a user to attempt authentication for.
 *
 *  RETURNS:
 *	Non-zero if a name was input, zero otherwise.
 */

int
getusername(user, prompt)
char	*user, *prompt;
{
	char	*p;		/* pointers into user's buffers */
	char	buf[BUFSIZ];
	size_t	nbytes;
	wchar_t	*wcp;
	wchar_t	*wcbuf;

	/*
	 * Input the user name from the keyboard and return.  The username
	 * is not to be validated.  Any leading whitespace is skipped.
	 * The first leading non-whitespace string is copied into the callers
	 * 'user' parameter.
	 *
	 * Once a non-whitespace-only string has been entered control
	 * returns to the caller.
	 */

	for (user[0] = '\0'; user[0] == '\0';) {

		if (prompt == NULL)
			prompt = MSGSTR (M_LOGIN, DEF_LOGIN);

		if (! getinput(buf, BUFSIZ, prompt))
			return 0;

		for (p = buf; *p && isspace (*p); p++)
			;

		strcpy(user, p);
	}

	/*
	 * Check for lowercase and uppercase characters, and set the lower and
	 * upper flags accordingly.  Multi-byte support is needed to check for
	 * lower and upper case multi-byte characters.
	 */

	if (multibytecodeset) {
		nbytes = strlen (user) + 1;
		wcbuf = (wchar_t *)malloc (nbytes * sizeof(wchar_t));
		mbstowcs (wcbuf, user, nbytes);

		/*
		 * Set upper and lower flags for later use.
		 */
		for (wcp = wcbuf; *wcp; wcp++)
			if (iswlower (*wcp))
				lower++;
			else if (iswupper (*wcp))
				upper++;

		free (wcbuf);
	} else {
		/*
		 * Set upper and lower flags for later use.
		 */
		for (p = user; *p; p++)
			if (islower (*p))
				lower++;
			else if (isupper (*p))
				upper++;
	}

	/*
	 * user now points to the leading non-space character on the line.
	 */

	return 1;
}

/*
 *  NAME: 	tpath
 *
 *  FUNCTION: 	Determine if user requested sak processing
 *
 *  EXECUTION ENVIRONMENT:
 *
 *	User process
 *
 *  RETURNS:
 *	If trusted path is required for this user return non-zero.
 */

int
tpath (user)
char	*user;
{
	char	*tpathval = NULL;

	/*
	 * If we cannot find an attribute for the sak key
	 * we will default to nosak since this most closely
	 * represents a standard (ie no security type functions)
	 * unix
	 */
	if (getuserattr (user, S_TPATH, &tpathval, SEC_CHAR))
		tpathval = "nosak";

	if (strcmp (tpathval, "notsh") == 0)
		user_tpath = TPATH_NOTSH;
	else if (strcmp (tpathval, "always") == 0)
		user_tpath = TPATH_ALWAYS;
	else if (strcmp (tpathval, "on") == 0)
		user_tpath = TPATH_ON;
	else	/* Default case ... */
		user_tpath = TPATH_NOSAK;

	return user_tpath == TPATH_ALWAYS;
}

/*
 *  NAME:	portprotection
 *
 *  FUNCTION:	set the port protection
 *
 *  EXECUTION:
 *
 *	User process
 *
 *  RETURNS:
 *	Zero if the file mode was successfully changed, non-zero on error
 */

int
portprotection (int mode, char *port)
{
	struct	acl	*acl;

	/*
	 * Get the file modes and append the low order permission bits
	 * with the next higher three bits.  YOU MUST REMEMBER TO NOT
	 * UPSET THE SVTXT BIT.
	 */

	if (acl = acl_get (port)) {
		acl->u_access = (mode >> 6) & 07;
		acl->g_access = (mode >> 3) & 07;
		acl->o_access = mode & 07;
		acl->acl_len = ACL_SIZ;

		if (acl_put (port, acl, 1))
			return -1;
	} else
		return -1;
	return 0;
}

/*
 *  NAME:	revoke_line
 *
 *  FUNCTION:	revoke access to a port
 *
 *  EXECUTION:
 *
 *	User process
 *
 *  RETURNS:
 *	Zero if the access was successfully revoked, non-zero on failure
 */

int
revoke_line (port)
register char *port;
{
	struct	acl	*acl;

	if (chown (port, 0, 0) || (acl = acl_get (port)) == 0)
		return -1;

	acl->u_access = 0;
	acl->g_access = 0;
	acl->o_access = 0;
	acl->acl_len = ACL_SIZ;
	if (acl_put (port, acl, 1))
		return -1;

	/*
	 * If secure processing is a possibility on the port then kill
	 * all process accessing the port before we exit
	 */

	if (strcmp( portsak, "true") == 0) {
		revoke (port);
	}
	return(0);
}

/*
 *  NAME:	tsm_sakenabled
 *
 *  FUNCTION:	determine if sak is enabled for the passed in port
 *
 *  EXECUTION:
 *
 *	User process
 *
 *  RETURNS:
 *	True if sak is enabled else false             
 */

int
tsm_sakenabled (char * port)
{

	static int list_built = FALSE;

	/*
	 * Check to see if the port has saked enabled
	 * If not check to see if the port is a synonyn
	 * for our port and we have sak_enabled. If so
	 * we allow the sak UNLESS the port has sak specificlly
	 * set to false
	 */
	if (!list_built)    {
		list_built = TRUE;
		tsmbld_saklist();
	}
	return (tsmqry_saklist (port));
}

/*
 *  NAME:	settsmenv
 *
 *  FUNCTION:	Set environment variables that we needed    
 *              Only called  by getty not login because
 *              login has probably had its environment by the shell
 *
 *  EXECUTION:
 *
 *	User process
 *
 */

void
settsmenv ()
{
	FILE	*fp;		/* File pointer to /etc/environment    */
	char 	buffer[BUFSIZ];
	int 	count; 		/* Number of vars we still need to get */
	int 	orig_count;     /* Orignal number needed               */
	char    *newenv;        /* Newly allocated space for env       */
	char	*buf, *p;
	char    *list[5];
	int i;

	/*
	 * set up what we are looking for
	 * too add more just append to the list update the
	 * declaration and count
	 */

	orig_count = count = 5;

	/*
	 * for now all we need is LANG, LC_MESSAGES, ODMDIR, NLSPATH, and TZ
	 * LANG and LC_MESSAGES are for the setlocale to work,
	 * ODMDIR is for the odm_ routines in tsmports to work
	 * NLSPATH and TZ are for the date conversion routines used to print
	 * the last login time.
	 */

	list[0] = "LANG";
	list[1] = "ODMDIR";
	list[2] = "NLSPATH";
	list[3] = "TZ";
	list[4] = "LC_MESSAGES";

	/*
	 * Use this local space to read in the data then , whn
	 * we want to store it we use malloc to get some non-local
	 * storage space of the correct size
	 */

	buf = &buffer[0];
	if ((fp = fopen ("/etc/environment", "r")) != NULL)
	{
		while (count)
		{
			/* get environment variable */
			if (fgets (buf, BUFSIZ, fp) == 0)
			{
				break;
			}
			/* ignore comments and lines without '=' */
			if ((*buf == '#') || (strchr (buf, '=') == 0))
			{
				continue;
			}
			for (i = 0; i < orig_count; i++)	{
				if (list[i] == NULL)	{
					continue;
				}
				if ((strlen (buf) > strlen(list[i])) &&
				    !strncmp(buf,list[i], strlen(list[i]))
				    && (buf[strlen(list[i])] == '=')) {
					/*
					 * replace newline character with NULL
					 */
					for (p = buf; *p; p++)
					{
						if (*p == '\n')
						{
							*p = '\0';
							break;
						}
					}
					/*
					 * Add to new environment.  Alloc space
					 * to ensure it is NOT local as putenv
					 * uses our space and does not copy it.
					 * This means we can't free the space
					 * either.
					 */
					newenv = (char *) malloc (strlen (buf) + 1);
					strcpy (newenv , buf);
					putenv (newenv);
					count--;
					list[i] = NULL;
					break;
				}
			}
		}
		fclose (fp);
	}
	return;
}

/*
 *  NAME:	tsmbld_saklist
 *
 *  FUNCTION:	Build a list of ports and their sak_enabled value
 *
 *  EXECUTION:
 *
 *	User process
 *
 */

void
tsmbld_saklist()
{
	AFILE_t af;
	ATTR_t ar;

	char	*sak, *syn;
	char	*tptr;

	af = afopen ("/etc/security/login.cfg");
	if (af == NULL)
		return;

	while ((ar = afnxtrec(af)) != NULL) {
		if (strcmp(ar->AT_value, "defport") == 0)
		{
			char	*sd;

			sd = afgetatr(ar, "sak_enabled");
			if (sd && (strcmp(sd, "true") == 0))
				sak_default = "true";
			continue;
		}
		if (strstr (ar -> AT_value , TSM_DEV))	{
			if ((sak = afgetatr (ar , "sak_enabled")) == NULL)
				sak = sak_default;
			syn = afgetatr (ar , "synonym");
			if (strcmp (sak , "true") == 0)  {
				int len;
				char *tptr1;
				tsmset_sakval ( ar -> AT_value , 
				    TSM_SAK);
				/*
				 * Grab first element ( See comment below)
				 */
				tptr1 = syn;
				len = strlen (tptr1);
				/*
				 * syn is a NULL sperated, double NULL
				 * terminated list of elements.  Each element
				 * must be passwd into tsm_parse as it may
				 * contain several entries seperated by ODM
				 * delimeters.
				 *
				 * Loop thru all elements (until we get a
				 * double NULL).
				 */
				while (syn && *syn) 	{
					/*
					 * Parse this element
					 */
					while (tptr1)	{
						tptr = listdup(tsm_parse (&tptr1));
						/*
						 * Set the status
						 */
						tsmset_sakval ( tptr , 
						    TSM_SAK_CH);
					}
					/* 
					 * Grab the next element (NULL
					 * seperated so go one passed string
					 * end) if it is NULL this means 2
					 * NULLS in a row and we are done
					 */
					tptr1 = syn + len + 1;
					len = strlen (tptr1);
					syn = tptr1;
				}
			}
			else {
				if (strcmp (sak , "false") == 0)  {
					tsmset_sakval (  ar -> AT_value , 
					    TSM_NOSAK);
				}
				if (syn) 	{
					tptr = listdup(tsm_parse (&syn));
					while (tptr)	{
						tsmset_sakval ( tptr , 
						    TSM_NOSAK_CH);
						tptr = listdup(tsm_parse (&syn));
					}
				}
			}
		}
	}
	afclose (af);
}

/*
 *  NAME:	tsmset_sakval
 *
 *  FUNCTION:	Set the sak_enabled of a port
 *              RULES : Direct settings override synonym values
 *			If both are direct or both are synonym
 *			values then "false" overrides "true"
 *			If no value then set to false "false"
 *                      (direct is defined as a port with its
 * 			 own stanza entry in /etc/security/login.cfg
 * 			  and a "sak_enabled" attribute of its own)
 *
 *  EXECUTION:
 *
 *	User process
 *            
 *  RETURNS
 *	void
 *
 */

void
tsmset_sakval (char * port, int status)
{
	TSMSAK_LIST_t ptr;

	/*
	 *  DO we already have an entry for this port
	 */
	for (ptr = list_head; ptr ; ptr = ptr -> next)
	{
		if (strcmp (ptr -> port , port) == 0)
		{
			break;
		}
	}
	/*
	 * New entry  - allocate space, set default values and link to list
	 */

	if (!ptr) {
		ptr  =  (TSMSAK_LIST_t)
		    malloc (sizeof (TSMSAK_LIST));
		ptr -> next = list_head;
		strcpy (ptr -> port, port);
		ptr -> status = TSM_SAK_NOTSET;
		list_head = ptr;
	}
	/*
	 * Set value if we can
	 */
	switch (ptr -> status) {
	/*
	 * No orignal values so anything is Ok
	 */
	case TSM_SAK_NOTSET:
		ptr -> status = status;
		break;

	/*
	 * Value is YES with CHANGE (Yes from synonym) 
	 * Anything can override it
	 */
	case TSM_SAK_CH:
		ptr -> status = status;
		break;

	/*
	 * Value is YES without CHANGE (Yes from direct) 
	 * Only a NO without CHANGE can override it
	 */
	case TSM_SAK:
		if (status == TSM_NOSAK) {
			ptr -> status = status;
		}
		break;

	/*
	 * Value is NO with CHANGE (No from synonym) 
	 * Can be overriden by anything except for YES with CHANGE
	 */
	case TSM_NOSAK_CH:
		if (status != TSM_SAK_CH) {
			ptr -> status = status;
		}
		break;

	/*
	 * Value is NO without CHANGE (No from direct) 
	 * Cannot be overriden
	 */
	case TSM_NOSAK:
		break;

	}
}

/*
 *  NAME:	tsmqry_sakval
 *
 *  FUNCTION:	Query the sak list to see if a port
 *               is enabled for sak                           
 *
 *  EXECUTION:
 *
 *	User process
 *            
 *  RETURNS
 *	TRUE if the sak enabled value of the port is
 *	YES w/wo CHANGE 
 *      else FALSE
 *
 */

int
tsmqry_saklist ( char * port)
{
	TSMSAK_LIST_t	ptr;

	for (ptr = list_head; ptr ; ptr = ptr -> next) {
		if (strcmp(ptr->port, port) == 0)
		{
			/*
			 * Found the ports entry
			 */
			switch (ptr->status)
			{
			case TSM_SAK:
			case TSM_SAK_CH:
				return (TRUE);
			default:
				return (FALSE);
			}
		}
	}

	if (strcmp(sak_default, "true") == 0)
		return (TRUE);
	else
		return (FALSE);
}

/*
 *  NAME:	tsmnotty
 *
 *  FUNCTION:	disables us from our controlling terminal
 *
 *  EXECUTION:
 *
 *	User process
 *            
 *  RETURNS
 *	void
 *
 */

void
tsmnotty()
{
	int fd;
	/*
	 * If the open or the ioctl returns an error we are not the
	 * current controlling process anyway
	 * so this is not an error
	 */

	if ((fd = open ("/dev/tty", O_RDONLY )) >= 0) {
		/*
		 * Open succeeded that means our controlling terminal
		 * has a controlling process - Lets attempt to blow
		 * it away if the ioctl fails we arent't the controlling
		 * process group so the current is OK
		 */
		ioctl (fd , TIOCNOTTY, NULL);
		close (fd);
	}
}
