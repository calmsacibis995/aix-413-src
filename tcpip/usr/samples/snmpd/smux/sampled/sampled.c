static char sccsid[] = "@(#)47	1.5  src/tcpip/usr/samples/snmpd/smux/sampled/sampled.c, snmp, tcpip411, GOLD410 5/14/94 13:08:18";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: main adios advise arginit doit_smux envinit get_smux init_test 
 *            mibinit o_sample1 s_sample1 send_trap set_smux start_smux  
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1992, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	/usr/samples/snmpd/smux/sampled/sampled.c
 */

/* 
 * Tab Stops should be set to 8 
 */

/*
 *
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/*
 * INTERNATIONAL BUSINESS MACHINES CORP. PROVIDES THIS SOURCE CODE
 * EXAMPLE, ASSOCIATED LIBRARIES AND FILES "AS IS," WITHOUT WARRANTY 
 * OF ANY KIND EITHER EXPRESSED OR IMPLIED INCLUDING BUT NOT LIMITED 
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  The entire risk as to the quality and performance 
 * of this example is with you.  Should any part of this source code 
 * example prove defective you (and not IBM or an IBM authorized 
 * dealer) assume the entire cost of all necessary servicing, repair, 
 * or correction.
 */

/*
 * Sample SMUX peer that demonstrates the implementations of MIB 
 * variables of the following syntax:
 *   INTEGER
 *   TimeTicks
 *   OBJECT IDENTIFIER
 *   DisplayString
 * and enterprise-specific trap with variable bindings for MIB
 * variables with the following syntax:
 *   INTEGER
 *   DisplayString
 * The peer also demonstates how to run as a daemon process.  As well as the
 * setting of a DisplayString variable.
 */

#include <sys/errno.h>
#include <signal.h>
#include <stdio.h>
#include <varargs.h>
#include <isode/snmp/smux.h>
#include <isode/snmp/objects.h>
#include <sys/ioctl.h>
#ifdef	BSD42
#include <sys/file.h>
#endif
#ifdef	SYS5
#include <fcntl.h>
#endif
#include <isode/tailor.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include "stats.h"
#include "sqeping.h"

/*    DATA */

/* The logging information structure. */
/*  "sampled.log", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS | LLOG_NOTICE,*/
static LLog	_pgm_log = {
    "sampled.log", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS,
    LLOG_FATAL, -1, LLOGCLS | LLOGCRT | LLOGZER, NOTOK
};
static	LLog   *pgm_log = &_pgm_log;

/* Debug variable set from the command line */
int	debug = 0;

/* Time variable to hold when the program started */
struct timeval my_boottime;

/* Default name of the program. */
static	char   *myname = "sampled";

/* Global variable to hold the tree to be registered. */
static	OID	subtree = NULLOID;

/* Global variable to hold SMUX information read from /etc/snmpd.peers */
static	struct smuxEntry *se = NULL;

/* General information flags, contain state of SMUX protocol */
static	int	rock_and_roll = 0;
static	int	dont_bother_anymore = 0;

/* File descriptors and select masks. */
static	fd_set	ifds;
static	fd_set	ofds;
static	int	nbits = FD_SETSIZE;
static	int	smux_fd = NOTOK;
static	int	ping_fd = NOTOK;

/* The logging procedures that handle reporting things */
void	adios (), advise ();

/*    MAIN */

/* ARGSUSED */


/*
 * This is the main procedure for sampled.  It contains the main processing
 * loop.  The main part of the program handles arguments (arginit), sets the 
 * environment (envinit), initializes select masks, opens the port over which 
 * to receive ping packets (mkPINGport).  If the ping port can not be opened, 
 * the program is exited.  After these items are completed, the main loop is
 * entered. 
 * 
 * When the program starts, the arginit function parses the arguments, sets 
 * the debugging level, changes directory to /usr/tmp, and opens the log file.
 * The envinit function starts sampled running as a daemon, reads the mib 
 * configuration file, and tries to open the SMUX connection.
 *
 * Inside the loop the flow is as follows:
 *       Set timeout value for select
 *       Set read and write masks for select
 *       Enter select
 *       Handle ping arrivals
 *       Handle SMUX connection and arrivals
 *       Goto the top
 * 
 * The set timeout portion is set to handle certain instances.  The base time
 * value is determined by the next_timeout routine.  This routine decides
 * if the program is expecting to send out more pings.  If it is, it sets a 
 * time value to the time to elapse before a new ping should be sent.  
 * Otherwise, it sets the value to -1.  If the snmpd connection is down and 
 * the time value is -1 and we have a reason to keep trying
 * (dont_bother_anymore = 0), then the timeout value is set to 5 seconds.  
 * Otherwise the timeout value is set to -1, which means to wait for data 
 * indefinitely.
 *
 * The masks are initially set to zero.  The read_mask always contains the
 * ping port to receive data and may contain the SMUX connection.  The SMUX
 * connection is added to the read_mask if the connection is up and the 
 * rock_and_roll flag is set.  This signifies that all the handshaking is done
 * between snmpd and sampled.  If the SMUX connection is up and the 
 * rock_and_roll flag is not set, the SMUX connection is added to the 
 * write_mask.  The write_mask is used to determine if the SNMP daemon is 
 * accepting data.  Normally the write_mask is empty, except when the 
 * connection is being established.  
 *
 * The program then enters select.  If select fails, we are going to exit
 * because things must be really bad.  
 *
 * The program now handles pings.  If the ping port has data to read, we will
 * loop through and read a ping packet and then parse it to see if it is one of
 * ours (parsePING).  Notice that normally recvfrom is a blocking command.
 * That is, if there is no data, the command will block for one.  This can be 
 * changed when the port is opened.  The mkPINGport command handles this.
 *
 * The program now handles the SMUX connection.  If the SMUX connection is 
 * down, try to restart it by calling smux_init.  If the connection is up,
 * but the file descriptor is in the write_mask, try to continue to bring up the
 * SMUX connection by calling start_smux.  If the SMUX connection is in the 
 * read_mask, call doit_smux to handle the request.  The rock_and_roll variable
 * is used to initially make a decision on which routine to call:  start_smux 
 * or doit_smux.
 *
 * After these things have happened, the program goes back to the top of the 
 * loop.
 */
main (argc, argv)
int	argc;
char  **argv;
{
    int	    nfds;

    arginit (argv);
    envinit ();

    /* Zero out the masks and max file descripters to check */
    FD_ZERO (&ifds);
    FD_ZERO (&ofds);
    nfds = 0;

    /* Open ping port */
    if ((ping_fd = mkPINGport()) == NOTOK) {
        adios("failed", "can't create RAW socket for ping");
    }

    /* Main Loop */
    for (;;) {
	int	n,
		secs;
	fd_set	rfds,
		wfds;
        struct timeval  timeout;
        struct timeval *t_ptr;

        /* Zero out the time value and select masks */
        timeout.tv_usec = 0;
	rfds = ifds;	/* struct copy */
	wfds = ofds;	/*   .. */

        /*
         * Get timeout value from the ping list.  If a timer is not required
         * return -1.
         */
        timeout.tv_sec = next_timeout();

        /*
         * If the SMUX connection is down and timeout is not needed by the ping
         * list, set the timer to 5 seconds and leave the select masks empty.
         */
	if (smux_fd == NOTOK && !dont_bother_anymore && timeout.tv_sec == -1) {
            timeout.tv_sec = 5;
	    t_ptr = &timeout;
        }
	else {
            /* 
             * The SMUX connection is up and the timer is not needed, so wait 
             * indefinitely.
             */
	    if (timeout.tv_sec == -1) 
	        t_ptr = 0;
            else 
            /* 
             * The SMUX connection is up and the timer is needed, use it.
             */
                t_ptr = &timeout;

            /* 
             *  If the SMUX connection is up and we are done with the SMUX <->
             *  snmpd handshaking, just listen for packets by adding the SMUX
             *  connection to the read select mask.  Otherwise if SMUX is up,
             *  add it to the write mask to try and continue connection bring
             *  up.
             */
            if (smux_fd != NOTOK) {
	        if (rock_and_roll)
	  	    FD_SET (smux_fd, &rfds);
	        else
		    FD_SET (smux_fd, &wfds);
            }
        }

        /*
         * Make sure that we include the file descripters in the maximum number
         * we tell select to select on.
         */
	if (smux_fd >= nfds)
	    nfds = smux_fd + 1;
	if (ping_fd >= nfds)
	    nfds = ping_fd + 1;

        /* Set the ping port in the read mask. */
	FD_SET (ping_fd, &rfds);

        /* Do the select */
	if ((n = select (nfds, &rfds, &wfds, NULLFD, t_ptr)) == NOTOK)
	    adios ("failed", "xselect");

/* check fd's for other purposes here... */

        /* 
         * Handle ping replies -- No error recovery for ping port loss 
         *
         * Make sure that select thinks there is data to read, then while there
         * is data to read, read a packet and parse it.  Recvfrom will return
         * a negative value when it is out of data.
         *
         * Error recovery on the ping port might be done by checking the errno
         * on the recvfrom to see why it returned -1.
         */
        if (FD_ISSET (ping_fd, &rfds)) {
            unsigned char buffer[1024];
            int len, fromlen, rc = 0;
            struct sockaddr_in from;

            /*
             * Read packet from fd and mark ping as received
             */
            while (rc >= 0) {
                fromlen = sizeof(from);
                len = sizeof(buffer);
                rc = recvfrom(ping_fd, buffer, len, 0, &from, &fromlen);
                if (rc >= 0) 
                   parsePING(buffer, rc, &from);
            }
        }

	/* 
	 * Handle SMUX replies 
	 *
	 * If the SMUX connection is down, try to restart it. (smux_init)
	 *
	 * If the connection is up but rock_and_roll equals 0, try to continue
	 * the connection bring up. (start_smux)
	 *
	 * Otherwise handle the request. (doit_smux)
	 */
	if (smux_fd == NOTOK && !dont_bother_anymore) {
	    if ((smux_fd = smux_init (debug)) == NOTOK)
	        advise (LLOG_EXCEPTIONS, NULLCP, "smux_init: %s [%s]",
	                smux_error (smux_errno), smux_info);
  	    else {
	        rock_and_roll = 0;
	    }
	}
	else {
	    if (rock_and_roll) {
		if (FD_ISSET (smux_fd, &rfds)) 
		    doit_smux ();
	    }
	    else
		if (FD_ISSET (smux_fd, &wfds))
		    start_smux ();
        }
    }
}

/*    MISCELLANY */

/*
 * Function: arginit
 *
 * Input: vec - arguments from the command line (char **)
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The function changes directory to /usr/tmp.  The function then 
 *    determines the program's name.  After that, the program initializes 
 *    logging level and sets the log header.  Parse the rest of the arguments.
 *    Only one argument is allowed, -d.  If other things are there, the program 
 *    exits after displaying the usage message.  If -d is specified, the debug
 *    flag is incremented (set).
 *
 */
static	arginit (vec)
char	**vec;
{
    register char  *ap;

    (void) chdir ("/usr/tmp");

    /* Getting the name of the program */
    if (myname = rindex (*vec, '/'))
	myname++;
    if (myname == NULL || *myname == NULL)
	myname = *vec;
    if (strncmp (myname, "smux.", 5) == 0 && myname[5] != NULL)
	myname += 5;

    /* Initializing base log levels and setting the log message header. */
    isodetailor (myname, 0);
    ll_hdinit (pgm_log, myname);

    /* Parse rest of the arguments */
    for (vec++; ap = *vec; vec++) {
	if (*ap == '-')
	    switch (*++ap) {
		case 'd':
		    debug++;
		    continue;

		default: 
		    adios (NULLCP, "-%s: unknown switch", ap);
	    }

	adios (NULLCP, "usage: %s [switches]", myname);
    }
}

/*  */

/*
 * Function: envinit
 *
 * Input: none
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The envinit function handles setting the initial environment.  The 
 *    first thing it does is get the maximum number of file descripters allowed
 *    open for the process (getdtablesize).  If the debug flag is off, the 
 *    function tries to run as a daemon.  This is done by calling fork.  If
 *    fork is successful, the parent process will die and the child process
 *    will continue to dup and close stdin, stdout, and stderr.  The child 
 *    process then closes the tty if needed and sets process variables.
 *    After that, whether it runs as a daemon or not, the function closes all
 *    the possible file descripters.  It sets signals to ignore sigpipe and
 *    rewrites the log message headers.  The function then calls mibinit to
 *    try to bring up the SMUX connection and read initial mib information.
 *    After mibinit, the function writes a file with its process id.  Then it
 *    sets the current time as the program's start time and prints a starting 
 *    message to the log.
 *
 */
static  envinit () {
    int     i,
            sd;
    char    file[BUFSIZ];
    FILE   *fp;

    /* Get the maximum number of file descriptors we are allowed to have. */
    nbits = getdtablesize ();

    /* If debug is off, lets run as a daemon. */
    if (debug == 0) { 
        /*
         * Fork creates another process running at the position after the fork.
         * If fork fails, a -1 return code, wait a bit and continue as if
         * we were in debug mode without messages.  If the fork works, the 
         * first or parent process will see fork return with process id of the 
         * second or child process.  The parent process will wait for the child
         * to be running and then exit.  The child process will see the fork
         * return with a zero, which causes it to continue.
         */
	switch (fork ()) {
	    case NOTOK: 
                /* 
                 * Case where the fork failed.
                 */
	        sleep (5);
	        printf("Cannot fork");

	    case OK: 
                /* 
                 * Case where the fork succeeded.  This is where the child
		 * ends up.
                 */
	        break;

	    default: 
                /* 
                 * Case where the fork succeeded.  This is where the parent
		 * ends.
                 */
 	        sleep (1);
	        _exit (0);
	}

        /*
         * Dup stdin, stderr, and stdout to /dev/null.  So, no output goes to
         * those file descripters.
         */
	if ((sd = open ("/dev/null", O_RDWR)) == NOTOK) {
	    adios ("/dev/null", "unable to read");
	}
	if (sd != 0)
	    (void) dup2 (sd, 0), (void) close (sd);
	(void) dup2 (0, 1);
	(void) dup2 (0, 2);

#ifdef	SETSID
        /* set sid of the child process */
	if (setsid () == NOTOK)
	    advise (LLOG_EXCEPTIONS, "failed", "setsid");
#endif

#ifdef	TIOCNOTTY
        /* Close the tty if we currently own it */
	if ((sd = open ("/dev/tty", O_RDWR)) != NOTOK) {
	    (void) ioctl (sd, TIOCNOTTY, NULLCP);
	    (void) close (sd);
	}
#else
#ifdef	SYS5
        /*
         * Set the process group id and set to ignore the interrupt and quit
         * signals.
         */
	(void) setpgrp ();
	(void) signal (SIGINT, SIG_IGN);
	(void) signal (SIGQUIT, SIG_IGN);
#endif
#endif
    }

    /* Close all file descripters we are allowed to have open */
#ifndef	sun		/* have to do this because of YP... */
    for (sd = 3; sd < nbits; sd++)
	if (pgm_log -> ll_fd != sd)
	    (void) close (sd);
#endif

    /* Ignore the pipe failure signal */
    (void) signal (SIGPIPE, SIG_IGN);

    /* Change both parts of the header of the log again */
    /* 
     * ll_dbinit (pgm_log, myname); - This command should be used
     * when debugging is needed.  It turns all the log files on.
     */
    ll_hdinit (pgm_log, myname);

    /* Set up initial SMUX connection and object data. */
    mibinit ();

    /*
     * Write the process id of the child process into a file, so the user can
     * check to see that what it is.
     */
    (void) sprintf (file, "/etc/%s.pid", myname);
    if (fp = fopen (file, "w")) {
	(void) fprintf (fp, "%d\n", getpid ());
	(void) fclose (fp);
    }
    
    /*
     * Set up the start time for this SMUX peer daemon.
     * This time is used for a sample MIB variable for the TimeTicks
     * syntax.
     */
    if (gettimeofday (&my_boottime, (struct timezone *) 0) == NOTOK) {
	    advise (LLOG_EXCEPTIONS, NULLCP, "gettimeofday failed");
		    bzero ((char *) &my_boottime, sizeof my_boottime);
    }

    /* Write an intermediate starting message to the log */
    advise (LLOG_NOTICE, NULLCP, "starting");
}

/*    MIB */

/*
 * Function: mibinit
 *
 * Input: none
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The mibinit function handles the initial bring up of the SMUX 
 *    connection and the initialization of the mib list.  The function first
 *    reads the entry in /etc/snmpd.peers.  This should contain an object
 *    identifier that signifies who the peer is and a password to be verified
 *    by snmpd.  The next thing it does is read the mib list (readobjects).  If
 *    this or the previous command fail, the program logs an error and exits.
 *    The mib list is a list of objects that the snmp functions, like test2obj,
 *    use to determine if an object is known.  The next thing to do is to get
 *    the tree that needs to be registered.  This could be done just before
 *    the smux_register call.  If the text2obj call fails, the program exits.
 *    The next thing done is to set the values of the mibs we read in.  The mib
 *    list structure has some pointer to functions to be called when that 
 *    object has a request.  These are set by the test_init function.  The 
 *    final thing is to try and start the SMUX connection.  This is done by
 *    the smux_init command.  This command sends a message to snmpd that a SMUX
 *    agents wants to connect.  The SNMP daemon upon receipt, accepts the
 *    socket and waits for a simple open message.  That is why we head back to
 *    the main loop and have the write_mask part of select to make sure that
 *    snmpd is ready to receive information.  If successful, we set 
 *    rock_and_roll to zero to make sure we do the write_mask part of select.
 *    Otherwise we log an error, and leave the SMUX file descripter, smux_fd,
 *    set to -1.
 */
static  mibinit () {
    OT	    ot;
    extern char  *snmp_address;

    /* Read the /etc/snmpd.peers file and read entry for sampled */
    if ((se = getsmuxEntrybyname ("sampled")) == NULL)
	adios (NULLCP, "no SMUX entry for \"%s\"", "sampled");

    /* Read the file specified for the mib definitions */
    if (readobjects ("/usr/samples/snmpd/smux/sampled/sampled.defs") == NOTOK)
	adios (NULLCP, "readobjects: %s", PY_pepy);

    /* Get the object for the tree to register */
    if ((ot = text2obj ("myMIBs")) == NULL) 
	  adios (NULLCP, "text2obj (\"%s\") fails", "test");
    subtree = ot -> ot_name;

    /*
     * From the list of objects read in, we need to set certain values
     * so that when we are required to handle a request we know what to do.
     */
    init_test ();	    /* test-specific enterprise */

    /* Specify the IP address for this SMUX peer.  This IP address
     * must be the same one specified in the smux entry in the
     * /etc/snmpd.conf file.  If this variable is not set, then snmpd
     * will only authenticate the SMUX peer if the SMUX entry in the
     * /etc/snmpd.conf file is 127.0.0.1.  The variable, snmp_address,
     * is an external variable, so snmpd gets the value.  In libisode.a,
     * the value of snmp_address is "loopback".  You override this value 
     * by setting snmp_address here.  Setting snmp_address to an IP address
     * on a remote machine will make snmpd access sampled on that remote
     * machine.  If you want to run sampled on a remote machine, be sure
     * to set the IP address in the smux entry for sampled in 
     * /etc/snmpd.conf as well.
     */
#ifdef NO
    snmp_address = "129.35.129.235";
#endif

    /* 
     * Try to open the SMUX connection.  If it fails, log an error.  If it
     * passes, set rock_and_roll to zero so that we will try to continue the
     * connection bring up when we get into the main loop.
     */
    if ((smux_fd = smux_init (debug)) == NOTOK)
	advise (LLOG_EXCEPTIONS, NULLCP, "smux_init: %s [%s]",
		smux_error (smux_errno), smux_info);
    else {
	rock_and_roll = 0;
    }
}

/*  */
/*
 * Function: start_smux
 *
 * Input: none
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The start_smux function handles the rest of the connection bring up
 *    not done by smux_init.  The SMUX protocol requires that after the
 *    smux_init, a smux_simple_open be done.  Using the identity and password
 *    read in by the getsmuxEntrybyname command, the smux_simple_open tells 
 *    the SNMP daemon who wants to talk with it.  The SNMP daemon matches the
 *    data it receives with a smux entry in its configuration file.  If it 
 *    everything is fine, nothing happens.  If the SNMP daemon does not like 
 *    the simple open, it returns a packet.  The way to implement a SMUX
 *    peer is to do the smux_simple_open and smux_register back to back since
 *    you can not guarantee and do not want a response back from the
 *    smux_simple_open.  If smux_simple_open returns with a smux_errno of 
 *    inProgress, it means that the connection is not completed yet, please
 *    wait.  In this case, we head back to the main loop to wait for the SNMP
 *    daemon to be ready to receive information again.
 *
 *    After the smux_simple_open, we log a message indicating the status of the
 *    simple open.  This is just whether or not the message was sent correctly.
 *    At this point, the connection is considered complete.  So, we set 
 *    rock_and_roll equal to 1.  If the simple open fails from the SNMP daemon,
 *    the connection will be closed and the connection will have to be restarted
 *    because the error packet appears to be out of order.  See doit_smux.
 *
 *    For the SNMP daemon to know what variables the peer supports, the peer
 *    must register a part of the MIB tree with snmpd.  After the connection is
 *    up, the registration is accepted.  A SMUX peer may support multiple trees.
 *    sampled only supports and registers one tree.  When registering, register
 *    as few trees as possible to increase search time and response.  But 
 *    remember that all MIBs under the registered tree are handled by the SMUX
 *    peer from the perspective of snmpd.  A peer can register over a tree
 *    that snmpd supports.   Be aware that if the peer registers the root of a
 *    tree that contains variables the peer *DOES NOT* support but snmpd
 *    *DOES* support, snmpd will defer to the SMUX peer to answer requests for
 *    MIB variables under that tree.
 */
static	start_smux () {
    if (smux_simple_open (&se -> se_identity, "SMUX UNIX daemon",
			  se -> se_password, strlen (se -> se_password))
	    == NOTOK) {
	if (smux_errno == inProgress)
	    return;

	advise (LLOG_EXCEPTIONS, NULLCP, "smux_simple_open: %s [%s]",
		smux_error (smux_errno), smux_info);
losing: ;
	smux_fd = NOTOK;
	return;
    }
    advise (LLOG_NOTICE, NULLCP, "SMUX open: %s \"%s\"",
	    oid2ode (&se -> se_identity), se -> se_name);
    rock_and_roll = 1;

    if (smux_register (subtree, -1, readOnly) == NOTOK) {
	advise (LLOG_EXCEPTIONS, NULLCP, "smux_register: %s [%s]",
		smux_error (smux_errno), smux_info);
	goto losing;
    }
    advise (LLOG_NOTICE, NULLCP, "SMUX register: readOnly %s in=%d",
	    oid2ode (subtree), -1);
}

/*  */
/*
 * Function: doit_smux
 *
 * Input: none
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The doit_smux function handles the incoming requests.  The function
 *    does a smux_wait to make sure that data is on the file descriptor to read.
 *    If smux_wait finds data, it reads the data and puts it in event.  If no
 *    data is on the file descriptor, the connection has gone down.  So, reset
 *    the file descripter to -1 and return.  If data was read successfully, 
 *    the function does a switch on the packet type.  If the packet is of the 
 *    type type_SNMP_SMUX__PDUs_simple, type_SNMP_SMUX__PDUs_registerRequest,
 *    type_SNMP_SMUX__PDUs_get__response, type_SNMP_SMUX__PDUs_trap, or does
 *    not match a known type, the connection should be closed and a protocol 
 *    error is set and returned.  The other types are handled and functions
 *    are called to handle the specific processing.
 */
static	doit_smux () {
    struct type_SNMP_SMUX__PDUs *event;

    /*
     * This loop handles multiple messages.  smux_wait is used to check the
     * file descriptor for messages.  If the smux_wait's timeout parameter is
     * set to zero, it returns without waiting for messages.  If no
     * messages are on the queue, smux_wait returns a NOTOK with a smux_errno
     * equal to inProgress.  This is the signal that all messages currently on
     * the queue have been read.
     */
    while (1) {
        /*
         * See if data is there.  If so, read it into event.  If not, close the 
         * connection.
         */
        advise (LLOG_NOTICE, NULLCP, "doit_smux(): calling smux_wait()");
        if (smux_wait (&event, 0) == NOTOK) {
	    if (smux_errno == inProgress)
	        return;

	    advise (LLOG_EXCEPTIONS, NULLCP, "smux_wait: %s [%s]",
  		    smux_error (smux_errno), smux_info);
losing: ;
	    smux_fd = NOTOK;
	    return;
        }
 
        /*
         *  Main switch to determine what function will handle the packet type.
         */
        switch (event -> offset) {
	case type_SNMP_SMUX__PDUs_registerResponse:
            /* 
             * If a register response packet is received, make sure it is not
             * a failure.  If it is a failure, close the connection and set the
             * dont_bother_anymore flag because the SMUX daemon doesn't know 
             * about the registered tree.  If it is successful, send a cold
             * start trap.
             */
	    {
		struct type_SNMP_RRspPDU *rsp = event -> un.registerResponse;

		if (rsp -> parm == int_SNMP_RRspPDU_failure) {
		    advise (LLOG_NOTICE, NULLCP,
			    "SMUX registration of %s failed",
			    oid2ode (subtree));
		    dont_bother_anymore = 1;
		    (void) smux_close (goingDown);
		    goto losing;
		}
		else
		    advise (LLOG_NOTICE, NULLCP,
			    "SMUX register: readOnly %s out=%d",
			    oid2ode (subtree), rsp -> parm);
	    }
	    if (smux_trap (int_SNMP_generic__trap_coldStart,
			   0, (struct type_SNMP_VarBindList *) 0) == NOTOK) {
		advise (LLOG_EXCEPTIONS, NULLCP, "smux_trap: %s [%s]",
			smux_error (smux_errno), smux_info);
		goto losing;
	    }
	    break;

        /* 
         * Handle the get and get-next requests
         */
	case type_SNMP_SMUX__PDUs_get__request:
	case type_SNMP_SMUX__PDUs_get__next__request:
            advise(LLOG_NOTICE, NULLCP, "doit_smux(): get-next or get request");
	    get_smux (event -> un.get__request, event -> offset);
	    break;

        /*
         * The SNMP daemon wants the connection closed, so close it.
         */
	case type_SNMP_SMUX__PDUs_close:
	    advise (LLOG_NOTICE, NULLCP, "SMUX close: %s",
		    smux_error (event -> un.close -> parm));
	    goto losing;

        /*
         * Handle protocol errors
         */
	case type_SNMP_SMUX__PDUs_simple:
	case type_SNMP_SMUX__PDUs_registerRequest:
	case type_SNMP_SMUX__PDUs_get__response:
	case type_SNMP_SMUX__PDUs_trap:
	    advise (LLOG_EXCEPTIONS, NULLCP, "unexpectedOperation: %d",
		    event -> offset);
	    (void) smux_close (protocolError);
	    goto losing;

        /*
         * Handle the set.  The set in SNMP is supposed to be a two stage 
         * operation, but due to the poor quality of the SMUX protocol defined
         * in the RFCs, two stages are unreliable.  So, I have chosen to 
         * handle multiple sets in a packet that actually take place on the 
         * receipt of a set.  This is not the only implementation.  I feel this
         * implementation has the highest probablity of success all the time.
         * A two stage commit could be done with a lot of timers, storage, and 
         * a hope that packets are never lost or corrupted.  So, when a set 
         * arrives the packet is parsed and variables are sent.  Things are not
         * reset.  A two stage commit could be written that only handles single
         * variables.  This would be fairly safe, but managers and other 
         * requesters would have to know not to send multiple sets.  This is 
         * not really controllable from the SMUX agent.  The main problem with
         * sets is that there is not a method to handle packet loss or timeout
         * or corruption.  So, do the best one can with what one has.
         */
	case type_SNMP_SMUX__PDUs_set__request:
            advise(LLOG_NOTICE, NULLCP, "doit_smux(): got a set request");
	    set_smux (event -> un.get__request, 
                      event -> un.get__request -> variable__bindings,
                      event -> offset);
	    break;

        /*
         * Handle the commmit of a set or the rollback of a set, for SMUX this
         * is a noop.
         */
	case type_SNMP_SMUX__PDUs_commitOrRollback:
            advise(LLOG_NOTICE, NULLCP, 
                   "doit_smux(): got a commit or rollback request");
	    break;

        /*
         * Handle unknown packets.
         */
	default:
	    advise (LLOG_EXCEPTIONS, NULLCP, "badOperation: %d",
		    event -> offset);
	    (void) smux_close (protocolError);
            goto losing;
        }
    }
}

/*  */
/*
 * Function: get_smux
 *
 * Input: pdu - the data that was read in (type_SNMP_GetRequest__PDU *)
 *        offset - the command to type to execute (int)
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The pdu contains a list of variables that need to be processed.  The 
 *    function loops through the variables in the list and handles each in turn.
 *    If an error occurs in a variable, processing stops with that variable and
 *    and the error is returned indicating which variable failed.  The variable
 *    position in the list is kept by the idx variable.  After all the 
 *    variables are processed or an error occurs, the function sends the 
 *    response to the SNMP daemon using smux_response.  Within the loop for 
 *    the processing of variables, the variable is examined to find the 
 *    appropriate structure in the MIB list.  The MIB list was read in by the 
 *    mibinit function.  On gets, the function expects to find an exact match
 *    for the variable.  On get-nexts, the function takes the variable and if
 *    it is an instance, it searchs for the next instance in the list.  If the 
 *    variable is not an instance, the MIB list is used to try and get the 
 *    next instance from that starting position.  This is done by using the
 *    name2inst and next2inst commands.  Once the MIB structure is obtained for
 *    the variable desired, the structure is checked to make sure that the 
 *    ot_getfnx field has a value.  The variable is then checked to make sure 
 *    that the variable is accessible.  If the variable is not accessible or
 *    it can not be found a response of no_such_name is returned.  The function
 *    then calls the ot_getfnx function to actually get the desired variable.
 *    The variables value is stored in the existing bind listing.  This binding
 *    list is used as the list sent in the response.  If the ot_getfnx function
 *    returns int_SNMP_error__status_noError, the ot_getfnx function executed
 *    successfully.  If it returns NOTOK, the ot_getfnx is telling the get_smux
 *    function that the variable requested is not there and that the request was
 *    a get-next.  So, move to the next variable in the list and try again.  On
 *    any other response, an error is returned and sent in the response packet.
 */
static	get_smux (pdu, offset)
register struct type_SNMP_GetRequest__PDU *pdu;
int	offset;
{
    int	    idx,
	    status;
    object_instance ois;
    register struct type_SNMP_VarBindList *vp;

    /* Initialize the variable counter, used to determine position */
    idx = 0;

    /* 
     * Loop through each variable in the variable-bindings list.
     */
    for (vp = pdu -> variable__bindings; vp; vp = vp -> next) {
	register OI	oi;
	register OT	ot;
	register struct type_SNMP_VarBind *v = vp -> VarBind;

	idx++;

        /*
         *  Get MIB list structure associated with the requested variable.
         */
	if (offset == type_SNMP_SMUX__PDUs_get__next__request) {
            advise(LLOG_NOTICE, NULLCP, "get_smux(): got a get_next request");
	    if ((oi = name2inst (v -> name)) == NULLOI
		    && (oi = next2inst (v -> name)) == NULLOI)
		goto no_name;

	    if ((ot = oi -> oi_type) -> ot_getfnx == NULLIFP)
		goto get_next;
	}
	else
	    if ((oi = name2inst (v -> name)) == NULLOI
	            || (ot = oi -> oi_type) -> ot_getfnx == NULLIFP) {
no_name: ;
		pdu -> error__status = int_SNMP_error__status_noSuchName;
		goto out;
	    }

try_again: ;
        /* 
         *  Check the access level of the variable.  This value is determined
         *  when the mib definitions are read in by readobjects in mibinit.
         *  The mib definition file is in sampled.defs.  The base form of the
         *  file is sampled.my.
         */
	switch (ot -> ot_access) {
	    case OT_NONE:
	        if (offset == type_SNMP_SMUX__PDUs_get__next__request)
		    goto get_next;
	        goto no_name;

	    case OT_RDONLY:
	    case OT_RDWRITE:
		break;
	}
		
        /*
         *  Call the variable handling function, and switch on the response.
         *  The variable's value is stored in v.  v is part of the variable
         *  binding list.  On get-nexts, the name in v is changed to hold the
         *  variable being returned.
         */
	switch (status = (*ot -> ot_getfnx) (oi, v, offset)) {
	    case NOTOK:	    /* get-next wants a bump */
get_next: ;
		oi = &ois;
		for (;;) {
		    if ((ot = ot -> ot_next) == NULLOT) {
			pdu -> error__status =
					    int_SNMP_error__status_noSuchName;
			goto out;
		    }
		    oi -> oi_name = (oi -> oi_type = ot) -> ot_name;
		    if (ot -> ot_getfnx)
			goto try_again;
		}

	    case int_SNMP_error__status_noError:
		break;

	    default:
		pdu -> error__status = status;
		goto out;
	}
    }
    idx = 0;

out: ;
    /*
     *  Complete the packet and send it.
     */
    pdu -> error__index = idx;

    if (smux_response (pdu) == NOTOK) {
	advise (LLOG_EXCEPTIONS, NULLCP, "smux_response: %s [%s]",
		smux_error (smux_errno), smux_info);
	smux_fd = NOTOK;
    }
}

/*  */

/*
 * Function: set_smux
 *
 * Input: pdu - the data that was read in (type_SNMP_GetRequest__PDU *)
 *        var_binds - 
 *        offset - the command to type to execute (int)
 *
 * Output: none
 *
 * Return: none
 *
 * Note: The set_smux routine is almost identical to the get_smux routine.  The
 *    reason they are separate is to allow the user to play with sets.  See 
 *    doit_smux for a comment about the set functionality.  The set_smux 
 *    function takes a list of variable name/variable value pairs and loops
 *    through them performing the set operations.  Since sampled is setting the
 *    variable on the set, it does not care about receiving rollbacks or 
 *    commits.  The code is identical to get_smux except that there is not a
 *    get-next part to handle the getting of the next variable and the variable
 *    access is checked to make sure it is readWrite.
 */
static	set_smux (pdu, var_binds, offset)
register struct type_SNMP_SetRequest__PDU *pdu;
register struct type_SNMP_VarBindList *var_binds;
int	offset;
{
    int	    idx,
	    status;
    object_instance ois;
    register struct type_SNMP_VarBindList *vp;

    /*
     * Loop through the variables, using idx to keep track of position.
     */
    idx = 0;
    for (vp = var_binds; vp; vp = vp -> next) {
	register OI	oi;
	register OT	ot;
	register struct type_SNMP_VarBind *v = vp -> VarBind;

	idx++;

        /*
         * Get the mib list entry for this variable.  If it does not exist,
         * error out.
         */
	if ((oi = name2inst (v -> name)) == NULLOI
	   || (ot = oi -> oi_type) -> ot_setfnx == NULLIFP) {
                status = int_SNMP_error__status_noSuchName;
		goto out2;
	    }

        /*
         * Check the access level of the variable.  If it isn't readWrite,
         * error out.
         */
	switch (ot -> ot_access) {
	    case OT_RDWRITE:
		break;

	    default:
                status = int_SNMP_error__status_noSuchName;
		goto out2;
	}

        /* 
         * Call set function and handle the response.
         */
	switch (status = (*ot -> ot_setfnx) (oi, v, offset)) {
	    case int_SNMP_error__status_noError:
		break;

	    default:
		goto out2;
	}
    }
    idx = 0;

out2: ;
    /*
     * Set error codes and return the packet.
     */
    pdu -> error__index = idx;
    pdu -> error__status = status;

    if (smux_response (pdu) == NOTOK) {
	advise (LLOG_EXCEPTIONS, NULLCP, "smux_response: %s [%s]",
		smux_error (smux_errno), smux_info);
	smux_fd = NOTOK;
    }
}

/*  */

/* Defines used to recognize the variable in the handling function */
#define	sampleInt		0
#define	sampleTick		1
#define	sampleTrap		2
#define	sampleObjectID		3
#define	sampleString		4
#define	testInt			5
#define windowNumber		6
#define windowRunning		7
#define windowOpen		8
#define windowProgram		9
#define ping2Number		10

/* Defines used to recognize the variable in the handling function */
#define ping2Addr		1
#define ping2Min		2
#define ping2Max		3
#define ping2Ave		4
#define ping2Tries		5
#define ping2Responses		6

/* Defines to make changing the windows examples easier. */
/* last_win and prog_win are used to hold the current value of that variable */
#define WIN_SIZE 	30
#define PROG_SIZE	100
char last_win[WIN_SIZE] = "";
char prog_win[PROG_SIZE] = "/bin/ksh";

/*
 * Function: s_sample1
 *
 * Input: oi - a pointer to the object identifier of the variable wanted. (OI)
 *        v - the name/value pair from the pdu (type_SNMP_VarBind *)
 *        offset - the command to type to execute (int)
 *
 * Output: none
 *
 * Return: status of operation.  Used in response.
 *
 * Note: The s_sample1 function handles the setting of the windowOpen and 
 *    windowProgram variables.  If another variable is received, a no_such_name
 *    error code is returned.  The function checks to make sure that this is 
 *    a set request.  The function then reads the variable from the name/value
 *    pair and stores in it the mib object.  The mib object contains a field
 *    ot_save where it is put.  The next thing is to convert to a string.
 *    The value stored in ot_save should have been an encoded string.  If this
 *    fails, the value was incorrect so exit and return an error.  At this point
 *    of converison, the user should place a conversion routine for the 
 *    appropriate data type.  It is placed here, because in this case both 
 *    variables are strings.  The types of integer, counter, gauge, and 
 *    timeticks are stored as is in the pointer.  If the value is an OID, it is
 *    stored in the pointer as a pointer to the OID.  The string is stored as
 *    a qbstring and an ipaddress is stored as a qbstring as well.  After the
 *    value is obtained, we switch on the specific variable to get the proper
 *    action to take.  If an error occur, the processing is stopped and the 
 *    error is returned. 
 */
static int s_sample1 (oi, v, offset) 
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OT         ot = oi -> oi_type;
    register OS         os = ot -> ot_syntax;
    register char       *s = NULLCP;
    int                 ifvar;
    int status = int_SNMP_error__status_noError;

    advise(LLOG_NOTICE, NULLCP, "s_sample1(): offset = %d", offset);
    ifvar = (int) ot -> ot_info;

    /* initial checking... */
    switch (offset) {
        case type_SNMP_SMUX__PDUs_set__request:
            /*
             * Get the variable from the name/value pair and place it in the
             * mib structure.  If failure, return the error.
             */
            if ((status = s_generic (oi, v, 
                                     offset==type_SNMP_SMUX__PDUs_set__request ?
                                     type_SNMP_PDUs_set__request : offset)) 
                != int_SNMP_error__status_noError) {
              advise (LLOG_EXCEPTIONS, NULLCP,
                      "GENERR in s_sample1: s_generic failed");
              return status;
            }
            /*
             * Get the variable from the mib structure into a usable form.
             * On failure, return generr.
             */
            if ((s = qb2str ((struct qbuf *) ot -> ot_save)) == NULLCP) {
                advise (LLOG_EXCEPTIONS, NULLCP,
                        "GENERR in s_system: qb2str failed");
                if (ot -> ot_save)
                    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
                return int_SNMP_error__status_genErr;
            }
            break;

        default:
            /* 
             * If it isn't a set, return an error.
             */
            advise(LLOG_NOTICE, NULLCP, "s_sample1(): Unexpected PDU type");
            return int_SNMP_error__status_genErr;
    }

    /*
     * Handle each variable as on its own. 
     */
    switch (ifvar) {
        case windowOpen:
                    {
                      /*
                       * When this variable is set, it tries to use the value
                       * of the variable as the display option of an aixterm
                       * call.  The aixterm is attempted with a program running
                       * in it as specified by windowProgram.0.  The default
                       * program is /bin/ksh.  The system call may not be the
                       * best way to do this, but it is an example.  Notice
                       * that last_win and prog_win are the variables that 
                       * hold the data.
                       *
                       * WARNING --- WARNING --- WARNING
                       * This could create a root window on that X-windows 
                       * session, if successful.  This is a security problem.
                       * This is only an example.
                       * WARNING --- WARNING --- WARNING
                       */
                      char comm[180];

                      if (strlen (s) > WIN_SIZE)
		      {
                          status = int_SNMP_error__status_badValue;
			  free(s);
			  return status;
		      }

		      sprintf(comm, "aixterm -display %s -fullcursor -T snmpd -e %s &", s, prog_win); 
		      system(comm);
                      strncpy(last_win, s, WIN_SIZE);
                      status = int_SNMP_error__status_noError;
                    }
                    break;

        case windowProgram:
                    /*
                     * Set the program to be run on a windowOpen.0 request.
                     */
                    if (strlen (s) > PROG_SIZE)
		    {
                        status = int_SNMP_error__status_badValue;
			  free(s);
			  return status;
		    }

                    strncpy(prog_win, s, PROG_SIZE);
                    status = int_SNMP_error__status_noError;
                    break;

        default:
            /*
             * Variables that we don't know about or shouldn't be set.
             */
            status = int_SNMP_error__status_noSuchName;
    }

    /*
     *  Free the space allocated from str2qb.  For the routines that return
     *  pointers after conversion, the values should be freed when done with
     *  them.
     */
    if (s != NULLCP)
        free (s);

    return status;
}



/*
 * Function: o_sample1
 *
 * Input: oi - a pointer to the object identifier of the variable wanted. (OI)
 *        v - the name/value pair from the pdu (type_SNMP_VarBind *)
 *        offset - the command to type to execute (int)
 *
 * Output: none
 *
 * Return: status of operation.  Used in response.
 *
 * Note: The o_sample1 function handles the getting of the variables.  These
 *    variables are not in a table.  The ping2xxxx variables are in a table, 
 *    except for ping2Number.  Tables are handled differently than normal
 *    instance variables.  The o_sample1 function makes sure that on a get 
 *    request the variable is an instance.  If it isn't, a no_such_name is 
 *    returned.  On get-next request, the variable is checked to make sure 
 *    it is not an instance and that a new object identifier can be created
 *    that is the instance for of the variable.  If this fails, a NOTOK is 
 *    returned to signify that the next variable should be gotten.  After this,
 *    the function switches on the variable reuested to determine what value to
 *    return.  Each section gets the requested variable and uses a routine 
 *    specific to the type of variable to encode the answer in the name/value
 *    pair variable.  This is how the variable's value is returned.  Notice
 *    for get-nexts, even the name is rewritten to indicate the actual variable
 *    returned.  If variable requested is not one of the variables supported,
 *    a no_such_name is returned.
 */
static int  o_sample1 (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    static int  count = 0;
    static int  testInt_count = 0;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    char buf[30];
    OID object_id;
    FILE *fp;
    int win;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_SMUX__PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1
		    || oid -> oid_elements[oid -> oid_nelem - 1] != 0) {
		return int_SNMP_error__status_noSuchName;
	    }
	    break;

	case type_SNMP_SMUX__PDUs_get__next__request:
            /* 
             * Make sure that the oid and the ot_name are the same length. 
             * The MIB list, list variables support by sampled and read in by 
             * mibinit, maintains the name of the variable as an OID without
             * the .0 on the end.  For get-next's, if it has a .0, then 
             * we want the variable after so its length will be different and
             * we will return NOTOK.  This means that the function that called
             * this routine will have to send us a new OID.  If the lengths
             * are the same, then extend the OID so that it can contain the 0
             * at the end.  If it cannot be extended, return NOTOK.  Free the
             * old name and set it to the new name.
             */
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((new = oid_extend (oid, 1)) == NULLOID)
		    return NOTOK;
		new -> oid_elements[new -> oid_nelem - 1] = 0;

		if (v -> name)
		    free_SNMP_ObjectName (v -> name);
		v -> name = new;
	    }
	    else
		return NOTOK;
	    break;

	default:
            /*
             * If it is not a get or a get-next request, return a gen_err.
             */
	    return int_SNMP_error__status_genErr;
    }

    /*
     * The switch to determine the action for each variable.  Each section
     * gets the appropriate value and encodes the value in v, a name/value pair.
     *
     * Note that ping2Number is handled here.  Since it is not part of the 
     * ping table, it is easier to implement the table in one function and this
     * counter in another.
     */
    switch (ifvar) {
        case ping2Number:
	    return o_integer (oi, v, count_stats_record());

	case sampleInt:
	    return o_integer (oi, v, ++count);

        case sampleTick:
	    {
	        struct timeval  now;
                integer diff;

		if (gettimeofday (&now, (struct timezone *) 0) == NOTOK) {
	   	    advise (LLOG_EXCEPTIONS, NULLCP, "GENERR: gettimeofday");
		    return int_SNMP_error__status_genErr;
		}

                diff = (now.tv_sec - my_boottime.tv_sec) * 100
			+ ((now.tv_usec - my_boottime.tv_usec) / 10000);
                return o_number (oi, v, diff);
	    }

	case sampleTrap:
	    return o_integer (oi, v, send_trap());

	case sampleObjectID:
	     sprintf (buf, "sampleObjectID.0");
             if ((object_id = text2oid (buf)) == NULLOID)
	            adios ("sampleObjectID text2oid(\"%s\") failed", buf);
	     return o_specific (oi, v, (caddr_t) object_id);

	case sampleString:
	     sprintf (buf, "This is a string example.");
	     return o_string (oi, v, buf, strlen(buf));

	case testInt:
	    return o_integer (oi, v, ++testInt_count);

	case windowNumber:
	{
	    char filename[L_tmpnam];
	    char comm[160];

	    tmpnam(filename);

	    sprintf(comm, "ps -ef | grep aixterm | wc -l > %s", filename);
	    system(comm);
            fp = fopen(filename, "r");
            fscanf(fp, "%d", &win); 
            fclose(fp);
            win--;
            sprintf(comm, "rm -rf %s", filename);
	    system(comm);
	    return o_integer (oi, v, win);
	}
	case windowRunning:
	{
	    char filename[L_tmpnam];
	    char comm[160];

	    tmpnam(filename);

	    sprintf(comm, "ps -ef | grep /usr/lpp/X11/bin/X | wc -l > %s", 
		    filename);
	    system(comm);
            fp = fopen(filename, "r");
            fscanf(fp, "%d", &win); 
            fclose(fp);
            sprintf(comm, "rm -rf %s", filename);
	    system(comm);
	    return o_integer (oi, v, win > 1 ? 1 : 0);
	}
	case windowOpen:
	    return o_string (oi, v, last_win, strlen(last_win));

	case windowProgram:
	    return o_string (oi, v, prog_win, strlen(prog_win));

	default:
            /* If none of the above, return noSuchName. */
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

/*
 * send_trap()
 *
 * Send an enterprise-specific trap with variable bindings.  So far this
 * sample sends two variable bindings - an integer and a string.  The
 * variables have to be defined in the sampled.my file.
 */
int send_trap()
{
    struct type_SNMP_VarBindList *bind_list;
    struct type_SNMP_VarBind    *vb;
    OI oi;
    char buf[30];
    int  rc;

    /* 
     * Create a variable binding list structure to build up a list of variable 
     * bindings.
     */
    if ((bind_list = (struct type_SNMP_VarBindList *)
        calloc (1, sizeof *bind_list)) == NULL) {
no_mem:;
        advise (LLOG_EXCEPTIONS, NULLCP, "send_trap(): out of memory");
	rc = 0;
        goto out;
    }

    /* 
     * Create a variable binding structure for the variable and hook this 
     * VarBind into the VarBindList.
     */
    if ((vb = (struct type_SNMP_VarBind *) calloc (1, sizeof *vb)) == NULL)
        goto no_mem;
    bind_list -> VarBind = vb;

    /*
     * Create an object instance for the first variable.
     * Put the name of this variable into the name of the variable binding 
     * structure.
     */
    sprintf (buf, "sampleTrap.0");
    if ((oi = text2inst (buf)) == NULLOI) {
        advise (LLOG_EXCEPTIONS, NULLCP, "send_trap(): lost OI for %s", buf);
        return 0;
    }
    if ((vb -> name = oid_cpy (oi -> oi_name)) == NULLOID)
        goto no_mem;

    /*
     * Put the value of the variable into the variable binding structure.
     * In this example, I just am putting in the value of "100".  I could 
     * have used any integer value.
     */
    if (o_integer (oi, vb, 100) != int_SNMP_error__status_noError) {
        advise (LLOG_EXCEPTIONS, NULLCP, "send_trap(): o_integer failed");
	rc = 0;
        goto out;
    }

    /* 
     * Create a second variable binding list structure to add to the list.
     */
    if ((bind_list -> next = (struct type_SNMP_VarBindList *)
        calloc (1, sizeof *bind_list)) == NULL)
            goto no_mem;

    /* 
     * Create a variable binding structure for the next binding.
     */
    if ((vb = (struct type_SNMP_VarBind *) calloc (1, sizeof *vb)) == NULL)
        goto no_mem;
    bind_list -> next -> VarBind = vb;
     
    /* 
     * Create an object instance for the second variable.
     * Put the name of this variable into the name of the variable binding 
     * structure.
     */
    sprintf (buf, "sampleString.0");
    if ((oi = text2inst (buf)) == NULLOI) {
        advise (LLOG_EXCEPTIONS, NULLCP, "send_trap(): lost OI for %s", buf);
        return 0;
    }
    if ((vb -> name = oid_cpy (oi -> oi_name)) == NULLOID)
        goto no_mem;

    /*
     * Put the value of the variable into the variable binding structure.
     * This example is for a string value.
     */
    strcpy (buf, "This is a string example.");
    if (o_string (oi, vb, buf, strlen(buf)) != int_SNMP_error__status_noError) {
        advise (LLOG_EXCEPTIONS, NULLCP, "send_trap(): o_string failed");
	rc = 0;
        goto out;
    }

    /* 
     * Send the trap to snmpd.  I used the enterprise-specific trap number 96
     * as an example.  
     */
    if (smux_trap (int_SNMP_generic__trap_enterpriseSpecific, 96, bind_list) 
	    == NOTOK) {
        advise (LLOG_NOTICE, NULLCP, "smux_trap(): %s [%s]\n", 
		smux_error(smux_errno), smux_info);
        rc = 0;
	goto out;
    }
    
    /* Successful */
    rc = 1;

out:;
    if (bind_list)
	    free_SNMP_VarBindList (bind_list);
    return rc;
}

/*  */

/*
 * Function: o_pinger
 *
 * Input: oi - a pointer to the object identifier of the variable wanted. (OI)
 *        v - the name/value pair from the pdu (type_SNMP_VarBind *)
 *        offset - the command to type to execute (int)
 *
 * Output: none
 *
 * Return: status of operation.  Used in response.
 *
 * Note: This function handles the getting of variables in the ping table.  
 *    This is a table with six variables that contain as their instances the
 *    destination address of the pings for that entry.  So, a valid variable
 *    name would be pingAve.127.0.0.1.  This would give the average time for
 *    a ping to be sent and received to address 127.0.0.1.  All the variables
 *    for 127.0.0.1 would end with the address.  These values at the end of the
 *    base object identifier are table indexs.  Tables can use only one type of 
 *    value for the index.  In this case, the IP address is used.  For the case
 *    of the interfaces table, the number of the entry in the table is used as 
 *    the index.  Tables are only single dimensional arrays.  Tables may not 
 *    contain tables.
 *
 *    This function receives a request.  If the request is a get, the function 
 *    makes sure that the variable has a valid IP address as the instance.  If
 *    the request is a get-next request, the function sees if the variable name
 *    has an instance or not.  If an instance is there, the function reads the
 *    instance and converts it to an address.  From the address, it gets the 
 *    next numerical entry in the ping table.  If there is not an entry for the
 *    variable after the given address, NOTOK is returned to signify the need
 *    to continue looking for the next variable.  If the address doesn't 
 *    contain an instance, the first entry in the table is selected.  For 
 *    either case, on failure, NOTOK is returned.  On success, the variable
 *    name, of the structure v, is reset to the new name that was just found. 
 *
 *    The function then switches on the variable.  It gets the appropriate 
 *    value for the entry and the variable name.  It then encodes the value in 
 *    v -> value and returns the results of the encoding.  If no variable is 
 *    found for the request, noSuchName is returned.
 */
static int  o_pinger (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    OID object_id;
    register unsigned int *ip;
    int i;
    char addrstr[20] = "";
    unsigned long address = -1;
    struct rtt_stats *statsrec;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_SMUX__PDUs_get__request:
            /*
             * Make sure that there are four parts to make an IP address from.
             */
	    if (oid -> oid_nelem < ot -> ot_name -> oid_nelem + 4)
		return int_SNMP_error__status_noSuchName;

            /*
             * Convert from integers to a string.
             */
            ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
            for(i = 0; i < 4; i++) {
	          char extra[5];
              sprintf(extra, "%d", *ip++);
              strcat(addrstr, extra);
              if (i != 3) strcat(addrstr, ".");
            }

            /*
             * Convert from string to address in an integer.
             */
            address = inet_addr(addrstr); 
            /*
             * If the address is invalid or it is not in the table, return with
             * noSuchName.
             */
            if ((address == -1) || 
                ((statsrec = find_stats_record(address, 0)) == 0))
		        return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_SMUX__PDUs_get__next__request:
            /* See if the variable name has an instance. */
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

                /* 
                 * Find the first record in the table 
                 */
                if ((statsrec = find_stats_record(0, -1)) == 0)
                    return NOTOK;

                /*
                 * Get its address and append it to a copy of the base OID and
                 * reset the variable name to it.
                 */
                address = statsrec -> address;
		if ((new = oid_extend (oid, 4)) == NULLOID)
		    return NOTOK;
                for (i = 0; i < 4; i++)
		    new -> oid_elements[new -> oid_nelem - 1 - i] =
                            ((address >> (i*8)) & 0xff);

		if (v -> name)
		    free_SNMP_ObjectName (v -> name);
		v -> name = new;
	    }
            /*
             * This is a bad case that should never come up.  It is if the 
             * variable name is less than the basic oid.
             */
	    else if (oid -> oid_nelem < ot -> ot_name -> oid_nelem)
                return NOTOK;
            else {
                /*
                 * There is an instance.  Get the extra parts.  If there are not
                 * 4 parts, extend the address with 0s.  Get the next address.
                 * Find the entry in the table.  Create a new variable name and
                 * reset the variable name in v. 
                 */
                
		OID	new;

                for(i = 0; i < oid->oid_nelem - ot->ot_name->oid_nelem; i++) {
                    char extra[5];
                    sprintf(extra, "%d", oid->oid_elements[ot->ot_name->oid_nelem+i]);
                    strcat(addrstr, extra);
                    if (i != 3) strcat(addrstr, "."); 
                }
                for(i = oid->oid_nelem - ot->ot_name->oid_nelem; i < 4; i++) {
                    strcat(addrstr, "0");
                    if (i != 3) strcat(addrstr, "."); 
                }
                address = inet_addr(addrstr); 
                if ((address == -1) ||
                    ((statsrec = find_stats_record(address, -1)) == 0))
		    return NOTOK;

		address = statsrec -> address;
                i = 4 - oid->oid_nelem + ot->ot_name->oid_nelem;
		if ((new = oid_extend (oid, i)) == NULLOID)
		    return NOTOK;
                for (i = 0; i<4; i++)
		    new -> oid_elements[new -> oid_nelem - 1 - i] =
                            ((address >> (i*8)) & 0xff);

		if (v -> name)
		    free_SNMP_ObjectName (v -> name);
		v -> name = new;
            }
	    break;

	default:
            /* 
             * If the request is not a get or a get-next, return a genErr.
             */
	    return int_SNMP_error__status_genErr;
    }

    /*
     * Get the value for each variable as appropriate.
     */
    switch (ifvar) {
        case ping2Addr: 
        {
            struct sockaddr_in netaddr;
            
            netaddr.sin_addr.s_addr = statsrec -> address;
            return o_ipaddr (oi, v, &netaddr);
        }

        case ping2Min:
            return o_integer (oi, v, statsrec -> minRTT);

        case ping2Max:
            return o_integer (oi, v, statsrec -> maxRTT);

        case ping2Ave:
            return o_integer (oi, v, 
                              statsrec -> RTTsum / statsrec -> RTTresponses);

        case ping2Tries:
            return o_integer (oi, v, statsrec -> RTTtries);

        case ping2Responses:
            return o_integer (oi, v, statsrec -> RTTresponses);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*
 * Function: s_pinger
 *
 * Input: oi - a pointer to the object identifier of the variable wanted. (OI)
 *        v - the name/value pair from the pdu (type_SNMP_VarBind *)
 *        offset - the command to type to execute (int)
 *
 * Output: none
 *
 * Return: status of operation.  Used in response.
 *
 * Note: This function handles the setting of variables in the table.  As a 
 *    side effect of setting the setable ping table variable, a row is created.
 *    The only setable variable is pingTries.  The setting of this variable 
 *    creates a table entry and starts a series of n pings, where n is the 
 *    value of the variable, pingTries. 
 *   
 *    The function checks to make sure that the variable has a valid address
 *    as an instance.  If not, noSuchName is returned.  If the request is not
 *    a set, a generr is returned.  After the variable is validated, a switch
 *    on the variable is done to get the proper handling code.  For this
 *    function, only one variable is setable.  If the request is not for that 
 *    variable, noSuchName is returned.  If it is the correct variable, the 
 *    function calls another function that adds an entry and starts the 
 *    pinging.  This done after the value has been checked for validity.
 */
static int s_pinger (oi, v, offset) 
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OT         ot = oi -> oi_type;
    register OS         os = ot -> ot_syntax;
    register OID        oid = oi -> oi_name;
    int                 ifvar;
    int status = int_SNMP_error__status_noError;
    unsigned long address = -1;
    register unsigned int *ip;
    int i;
    char addrstr[20] = "";

    ifvar = (int) ot -> ot_info;

    /* initial checking... */
    switch (offset) {
        case type_SNMP_SMUX__PDUs_set__request:
            /*
             * Make sure that the variable name has a valid instance address.
             */
            if (oid -> oid_nelem - ot->ot_name->oid_nelem != 4)
	        	return int_SNMP_error__status_noSuchName;

            ip = oid -> oid_elements + ot->ot_name->oid_nelem;
            for(i = 0; i < 4; i++) {
	            char extra[5];
                sprintf(extra, "%ld", *ip++);
                strcat(addrstr, extra);
                if (i != 3) strcat(addrstr, ".");
            }

            address = inet_addr(addrstr); 

            if (address == -1) 
		        return int_SNMP_error__status_noSuchName;
			break;

        default:
            /*
             * If not a set request, return generr.
             */
            advise(LLOG_NOTICE, NULLCP, "s_pinger(): Unexpected PDU type");
            return int_SNMP_error__status_genErr;
    }

    /* Switch on variable */
    switch (ifvar) {
        case ping2Tries:
                    /* Make sure value of the variable is valid. */
                    if (v -> value -> un.number <= 0)
		    {
                        status = int_SNMP_error__status_badValue;
			return status;
		    }

                    /* Make entry and start pings. */
                    queue_PINGs(address, v -> value -> un.number);
                    status = int_SNMP_error__status_noError;
                    break;

        default:
	    /* If other than the one variable, return noSuchName. */
            status = int_SNMP_error__status_noSuchName;
    }
    return status;
}

/*  */
/*
 *  Function: init_test
 *
 *  Inputs: none
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Note: This function sets the functions and descripter for each variable 
 *     supported by sampled.  The MIB list, read in by readobjects, is made up
 *     of OT structures.  These structures contain information about each MIB
 *     variable.  The fields that need to be set at this point are the 
 *     ot_getfnx, ot_setfnx, and ot_info fields.  The ot_getfnx field contains
 *     a pointer to a function that handles get and get-next requests for that 
 *     MIB variable.  The ot_setfnx field contains a pointer to a function that
 *     handles set requests for that MIB variable.  The ot_info field is used 
 *     as a short cut when dealing with the MIBs with the handling function.  
 *     The reason for this is that OID to OID comparison is slow and if the 
 *     ot_info information is ordered the get-next operation is easier.
 */
init_test () {
    register OT	    ot;

    if (ot = text2obj ("sampleInt"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) sampleInt;

    if (ot = text2obj ("sampleTick"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) sampleTick;

    if (ot = text2obj ("sampleTrap"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) sampleTrap;

    if (ot = text2obj ("sampleObjectID"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) sampleObjectID;

    if (ot = text2obj ("sampleString"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) sampleString;

    if (ot = text2obj ("windowNumber"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) windowNumber;

    if (ot = text2obj ("windowRunning"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) windowRunning;

    if (ot = text2obj ("windowOpen"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_setfnx = s_sample1,
	ot -> ot_info = (caddr_t) windowOpen;

    if (ot = text2obj ("windowProgram"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_setfnx = s_sample1,
	ot -> ot_info = (caddr_t) windowProgram;

    if (ot = text2obj ("testInt"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) testInt;

    if (ot = text2obj ("ping2Number"))
	ot -> ot_getfnx = o_sample1,
	ot -> ot_info = (caddr_t) ping2Number;

    if (ot = text2obj ("ping2Addr"))
	ot -> ot_getfnx = o_pinger,
	ot -> ot_info = (caddr_t) ping2Addr;
	
    if (ot = text2obj ("ping2Min"))
	ot -> ot_getfnx = o_pinger,
	ot -> ot_info = (caddr_t) ping2Min;
	
    if (ot = text2obj ("ping2Max"))
	ot -> ot_getfnx = o_pinger,
	ot -> ot_info = (caddr_t) ping2Max;
	
    if (ot = text2obj ("ping2Ave"))
	ot -> ot_getfnx = o_pinger,
	ot -> ot_info = (caddr_t) ping2Ave;
	
    if (ot = text2obj ("ping2Tries"))
	ot -> ot_getfnx = o_pinger,
	ot -> ot_setfnx = s_pinger,
	ot -> ot_info = (caddr_t) ping2Tries;

    if (ot = text2obj ("ping2Responses"))
	ot -> ot_getfnx = o_pinger,
	ot -> ot_info = (caddr_t) ping2Responses;
}

/*    ERRORS */

/*
 *  Function: adios
 *
 *  Inputs: what - the thing that went wrong (char *)
 *          fmt  - the string to be printed in printf format (char*)
 *          variables - variable needed to fill in the fmt.
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Note: The adios function calls the logging function with the variables
 *      above and a LLOG_FATAL error code.  The function then exits with a
 *      return code of 1, thereby terminating the sampled process.
 */
#ifndef	lint
void	adios (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);
    
    _ll_log (pgm_log, LLOG_FATAL, ap);

    va_end (ap);

    _exit (1);
}
#else
/* VARARGS */

void	adios (what, fmt)
char   *what,
       *fmt;
{
    adios (what, fmt);
}
#endif

/*
 *  Function: advise
 *
 *  Inputs: code - the logging level to associate with this error (int)
 *          what - the thing that went wrong (char *)
 *          fmt  - the string to be printed in printf format (char*)
 *          variables - variable needed to fill in the fmt.
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Note: The advise function calls the logging function with the variables
 *      above.  This is a usability front end to the logging functions.
 */
#ifndef	lint
void	advise (va_alist)
va_dcl
{
    int	    code;
    va_list ap;

    va_start (ap);
    
    code = va_arg (ap, int);

    _ll_log (pgm_log, code, ap);

    va_end (ap);
}
#else
/* VARARGS */

void	advise (code, what, fmt)
char   *what,
       *fmt;
int	code;
{
    advise (code, what, fmt);
}
#endif
