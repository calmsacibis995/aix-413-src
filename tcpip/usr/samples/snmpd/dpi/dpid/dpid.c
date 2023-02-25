static char sccsid[] = "@(#)11	1.3  src/tcpip/usr/samples/snmpd/dpi/dpid/dpid.c, snmp, tcpip411, GOLD410 3/29/94 15:26:13";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/dpid/dpid.c
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
 *  DPID is a SMUX to DPI converter for SNMP.  The converter is used to allow 
 *  DPI agents to talk with the AIX SNMP agent.  The converter changes DPI
 *  messages into SMUX protocol messages. 
 */

#include <sys/errno.h>
#include <signal.h>
#include <stdio.h>
#include <varargs.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <isode/snmp/smux.h>
#include <isode/tailor.h>
#include <fcntl.h>

#include "snmp_dpi.h"

#define COLD_START

/* Externalized functions */
void advise();
void adios();
void del_from_mask();

/* Global debug flag */
int	debug = 0;

/* Global SMUX information structures */
static struct snmp_dpi_registered_objects *regobjs = NULL; /* list of objects */
static OID subtree = NULLOID;                              /* tree to register*/
static struct smuxEntry *se = NULL;                        /* SMUX info       */

/* File Descriptors for the DPI registration port and the SMUX agent port */
int DPI_fd = NOTOK; 
int DPI_port_num;
static int smux_fd = NOTOK;

/* Descriptions of file descriptors that were registered on the converter */
#define FDT_DPI_LISTEN	1
#define FDT_DPI_PEER	2
#define FDT_SMUX_AGENT	3

/* File Descriptors to listen on and other select information */
#define MAX_FDS		32
static int fd_tbl[MAX_FDS];
static unsigned long select_mask = 0;
static  int     nbits = FD_SETSIZE;

/* The logging structure for advise and adios. */
static LLog     _pgm_log = {
    "dpid.log", NULLCP, NULLCP, LLOG_FATAL | LLOG_EXCEPTIONS,
    LLOG_FATAL, -1, LLOGCLS | LLOGCRT | LLOGZER, NOTOK
};
static  LLog   *pgm_log = &_pgm_log;

/* Variable for general operations */
static  char   *myname = "dpid";
int		rock_and_roll = 0;

/* External functions */
extern void process_trap();
extern char *DPI_port_objID;


main(argc, argv)
int argc;
char *argv[];
{
	int					rc, fd;
	int					numfds, i;
	int					secs;
	unsigned long		read_mask;
	unsigned long		write_mask;
	struct timeval		timeout;
	struct	sockaddr	fromAddr;
	int					from_len;
	int					sd;

	/* 
	 * Parse arguments, any argument other than -d is a failure
	 */
	i = 1;
	while (i < argc) {
		if (strcmp("-d", argv[i]) == 0) debug = 1;
		else {
			printf("usage:  %s [-d]\n", argv[0]);
			exit(1);
		}
		i++;
	}

	/* 
	 * Set the myname variable to the name of the program executing.
	 * The default is dpid.
	 */
	if (myname = rindex (argv[0], '/'))
		myname++;
	if (myname == NULL || *myname == NULL)
		myname = argv[0];
	if (strncmp (myname, "smux.", 5) == 0 && myname[5] != NULL)
		myname += 5;

	/*
	 * Set up initial logging and initial debug levels within the ISODE code.
	 */
	isodetailor (myname, 0);
	ll_hdinit (pgm_log, myname);

	/*
	 * Determine the maximum number of file descriptors this dpid process
     * can select on.
	 */
	nbits = getdtablesize ();

	/*
	 * If the debug flag is off, we will run as a daemon process, otherwise
	 * we run from the current shell.  Logging will proceed as normal.  If
     * the flag is off, the logging file will be in /usr/tmp.  Otherwise, it
     * will be in the current directory.
	 */
	if (debug == 0) {
		/*
		 * First fork and spawn the child process, then exit the parent
		 * process.
		 */
		switch (fork ()) {
			case NOTOK:
				/* Cannot fork, so let's run as a normal process */
				sleep (5);
				printf("Cannot fork");

			case OK:
				/* 
				 * The normal exit from the switch by the child process.  It
				 * will now be the dpid that goes on to initialize and execute
				 */
				break;

			default:
				/* 
				 * The normal exit from the switch by the parent process.  It
				 * will now wait to make sure the child has started and then
				 * exit.
				 */
				sleep (1);
				_exit (0);
		}

		/* Change the current directory to /usr/tmp */
		(void) chdir ("/usr/tmp");

		/*
		 * Open /dev/null so that stdin, stdout, and stderr may be dupped to
		 * it and then closed.
 		 */
		if ((sd = open ("/dev/null", O_RDWR)) == NOTOK) {
			adios ("/dev/null", "unable to read");
		}
		if (sd != 0)
			(void) dup2 (sd, 0), (void) close (sd);
		(void) dup2 (0, 1);
		(void) dup2 (0, 2);

#ifdef  SETSID
		/* set our sid */
		if (setsid () == NOTOK)
			advise (LLOG_EXCEPTIONS, "failed", "setsid");
#endif
#ifdef  TIOCNOTTY
		/* Close the assigned tty, if we have one */
		if ((sd = open ("/dev/tty", O_RDWR)) != NOTOK) {
			(void) ioctl (sd, TIOCNOTTY, NULLCP);
			(void) close (sd);
		}
#else
#ifdef  SYS5
		/* Set our process group and ignore the interrupt and quit signals */
		(void) setpgrp ();
		(void) signal (SIGINT, SIG_IGN);
		(void) signal (SIGQUIT, SIG_IGN);
#endif
#endif
	}

	/* Set the logging headers */
	ll_dbinit (pgm_log, myname);

#ifndef sun 
	/* Close all file descriptors except the logging file descriptor */
	for (sd = 3; sd < nbits; sd++)
		if (pgm_log -> ll_fd != sd)
			(void) close (sd);
#endif

	/* Ignore the pipe closing signal */
	signal (SIGPIPE, SIG_IGN);

	/* 
	 * SMUX initialization
	 *
	 * The first thing that needs to be done is to get the entry for this
	 * program from /etc/snmpd.peers.  This entry contains the name of the
	 * program, the identity object identifier, and a validation password.
	 * These should match an entry in /etc/snmpd.conf.  The getsmuxEntrybyname
	 * function uses the name of the program to retrieve the other two values
	 * (i.e., the object id and the password) and a pointer to the structure
	 * that contains them, smuxEntry.
	 */
	se = getsmuxEntrybyname (myname);
	if (se == NULL) 
		adios("Failed", "no SMUX entry for \"%s\"\n", myname);

	/*
	 * The next part of initialization is to inform the SNMP daemon that
	 * there is a SMUX proxy out here to talk to it.  This is done by calling
	 * the smux_init routine.  The smux_init routine takes a desired debug
	 * level and returns the file descriptor to use to talk to the SNMP daemon.
	 */
	if ((smux_fd = smux_init(debug)) == NOTOK) {
		advise(LLOG_EXCEPTIONS, "Failed", 
			"smux_init() failed %s [%s]\n",
			smux_error (smux_errno), smux_info);
	}
	else {
		/*
		 * If the smux_init is successful, we need to set the port on which
		 * we will listen for DPI agent register requests.
		 */
		if ((DPI_fd == NOTOK) && ((rc = create_DPI_port()) != 0)) {
			/* 
			 * If this fails, close the SNMP port.  Once within the main loop
			 * a timer will be set and we will try again later.  Tell the log
			 * about this failure though.
             */
			advise(LLOG_EXCEPTIONS, "Failed", 
				"could not create DPI port\n");
			del_from_mask(smux_fd);
			smux_close();
			smux_fd = NOTOK;
		}
		else {
			/* 
			 * Both ports have been opened successfully, so add them to the 
			 * select mask.  The select mask is used to tell select which file
			 * descriptors to examine for input or output.
			 */	
			add_fd(DPI_fd, FDT_DPI_LISTEN);
			add_fd(smux_fd, FDT_SMUX_AGENT);
		}
	}

	/*
	 * This is the main processing loop.  The general flow is:
	 *		set read and write select masks
	 *		set timeout value for select
	 *		select on mask and timeout value
	 *		handle re-initiailization of file descriptors
	 *		handle read requests
	 *		handle write availabilities
	 *		start loop over
	 *
	 * The setting of masks and timeout values is dependent upon the state 
	 * of the SMUX and DPI file descriptors and the state of the SMUX protocol.
	 * If the smux_fd is down, set write_mask to 0, read_mask to known file
	 * descriptors and set the timeout to 5 seconds.  This is so that we can
	 * try to reinitialize the SMUX port frequently.  The read_mask allows 
	 * us to handle a DPI request even if the SMUX port is down.
	 * If the SMUX file descriptor is open but the protocol has not finished 
	 * the initial hand shaking (signified by rock_and_roll equaling 0), set 
	 * the read_mask to the DPI file descriptors, the write_mask to the 
 	 * SMUX file descriptor, and timeout to -1 (don't timeout).  This makes 
	 * sure that we can write to the SNMP daemon.  Otherwise, set time out to
	 * -1 (don't timeout), clear the write_mask, and set read_mask to all known
	 * file descriptors.
	 *
	 * Call select with the set parameters.  If select fails, exit because 
	 * things are really bad.
	 *
	 * After select, we have three things to do: 
	 *		re-initialize			 	- if needed
	 *		handle read requests		- if needed
	 *		handle write availability	- if needed
	 *
	 * Re-Initialization:
	 *		if the SMUX file descriptor is not valid, attempt to reinitialize
	 *		the file descriptor.  If successful, make sure that the DPI file
	 *		descriptor is valid.  If it is invalid, try to reopen it.  If
	 *		both are ok, then set the file dsecriptor types, otherwise close
	 * 		both.
	 *
	 * Read Requests:
	 *		For all the file descriptors signified in the returned read mask,
	 *		handle the requests.  Switch on the file descriptor type to 
	 *		determine how to handle the data read in.  If the type is:
	 *
	 *			FDT_DPI_LISTEN	Handle the register request of a new DPI
	 *							agent.
	 *
	 *			FDT_DPI_PEER	Handle a response/request from a specific
	 *							DPI agent(has already registered).
	 *
	 *			FDT_SMUX_AGENT	Handle a response/request from the SNMP
	 *							daemon.
	 *
	 * Write Availabilities:
	 *		If the smux_fd is defined and it is in the write_mask, then snmpd
	 *		is awaiting a response.  The rest of the initialization is 
	 *		performed.  The already registered trees are re-registered if they
	 *		exist.  If the SNMP daemon is running and goes down, we would like
	 *		to be able to pick up where we left off.  So, a list of registered
	 *		trees is kept (when a DPI agent registers a tree, it is added to the
	 *		list).
	 *
	 */
	while (1) {	/* loop forever */
		struct snmp_dpi_registered_objects	*ptr;
		int	k;

		/* Initialize default timeout and default masks */
		secs = NOTOK;
		write_mask = read_mask = 0;

		/* Timeout = 5 secs and read_mask = known fds */ 
		if (smux_fd == NOTOK) {
			secs = 5L;
			read_mask = select_mask;
		}
		/* read_mask = known fds */ 
		else if (rock_and_roll) {
  			read_mask = select_mask;
		}
		/* write_mask = SMUX file descriptor and read_mask = known fds */ 
		else {
			write_mask = (1 << smux_fd);
			read_mask = select_mask & ~(1 << smux_fd);
		}

		/* Select on all */
		numfds = xselect(MAX_FDS, &read_mask, &write_mask, 0, secs);
		/* If error, exit */
		if (numfds == NOTOK) {
			adios("Failed", "xselect");
		}

		/* Handle re-initialization if needed. */
		if (smux_fd == NOTOK) {
			/* Reinitialize SMUX fd */
			if ((smux_fd = smux_init (debug)) == NOTOK)
				advise (LLOG_EXCEPTIONS, NULLCP, 
					"smux_init: %s [%s]", 
					smux_error (smux_errno), 
					smux_info);
			else {
				rock_and_roll = 0;
				/* Reinitialize DPI fd */
				if ((DPI_fd == NOTOK) &&
				    (rc = create_DPI_port()) != 0) {
					advise(LLOG_EXCEPTIONS,"Failed",
					 "could not create DPI port\n");
					del_from_mask(smux_fd);
					smux_close();
					smux_fd = NOTOK;
				}
				else {
					/* Everything is OK */
					add_fd(DPI_fd, FDT_DPI_LISTEN);
					add_fd(smux_fd, FDT_SMUX_AGENT);
				}
			}
		}

		/* Handle read requests */
		fd = 0;
		while (read_mask != 0) {
			if ((read_mask & 1) != 0) {
				switch (fd_tbl[fd]) {
				case FDT_DPI_LISTEN:
				  if (debug) 
				    advise(LLOG_EXCEPTIONS,
					"DEBUG", "accept DPI peer\n");
				  from_len = sizeof(fromAddr);
				  rc = accept(fd, &fromAddr, &from_len);
				  if (rc != -1) 
				    add_fd(rc, FDT_DPI_PEER);
				  break;
				case FDT_DPI_PEER:
				  rc = read_DPI_request(fd);
				  break;
				case FDT_SMUX_AGENT:
				  if (debug) 
				    advise(LLOG_EXCEPTIONS,
					"DEBUG", "SMUX request\n");
				  rc = doit_smux();
				  if (rc != 0) {
					advise(LLOG_FATAL,"Failed", 
					  "lost connection to agent\n");
					del_from_mask(smux_fd);	
					smux_close();
					smux_fd = NOTOK;
				  }
				  break;
				} /* end switch */
			} /* end if */
			fd++;
			read_mask >>= 1;
		}

		/* Handle write availabilities */
		if ((smux_fd != NOTOK) && 
		    (write_mask & (1 << smux_fd)) == (1 << smux_fd)) {
			start_smux();

			/* Handle re-registering trees */
			ptr = regobjs;
			k = smux_fd;
			while ((ptr) && (k != NOTOK)) {
				if (ptr -> owning_fd != NOTOK) {
					rc = smux_register (ptr->object,
						-1, readWrite);
					if (rc == NOTOK) {
						del_from_mask(k);
						smux_fd = k = NOTOK;
						advise(LLOG_EXCEPTIONS, "Failed", 
								"smux_register (%s):  %s [%s]\n", ptr->object,
								smux_error(smux_errno), smux_info);
					}
				}
				ptr = ptr -> next;
			}
		}
	}

	/* NOTREACHED */
}


/* 
 *  Function: add_to_mask
 *
 *  Inputs: fd - file descriptor (int)
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Global: sets bit for fd in global select_mask variable
 */
static void add_to_mask(fd)
int fd;
{
	select_mask |= (1 << fd);
}

/* 
 *  Function: del_from_mask
 *
 *  Inputs: fd - file descriptor (int)
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Global: removes bit for fd in global select_mask variable
 */
void del_from_mask(fd)
int fd;
{
	select_mask &= ~(1 << fd);
}

/* 
 *  Function: add_fd
 *
 *  Inputs:	fd 		- file descriptor (int)
 *			type	- type of file descriptor (int)
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Global:	sets the global array fd_tbl to type at the fd position in the 
 *			array and calls add_to_mask with fd as the parameter.
 */
add_fd(fd,type)
int fd;
int type;
{
	fd_tbl[fd] = type;
	add_to_mask(fd);
}


/* 
 *  Function: start_smux
 *
 *  Inputs:	none
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Note: This function handles the SMUX initialization after the port has been
 *		opened.  The SMUX protocol says that after the smux_init is done, a 
 *		simple open is required.  This tells the daemon who is talking to it.
 *		After that the trees it will be monitoring will be registered.	
 */
static	start_smux()
{
	int	rc;

	rc = smux_simple_open (&se->se_identity, "DPI->SMUX daemon",
	    se->se_password, strlen(se->se_password));
	if (rc == NOTOK) {
		smux_fd = NOTOK;
		advise(LLOG_EXCEPTIONS, "Failed", "smux_simple_open: %s [%s]\n",
			smux_error(smux_errno), smux_info);
		return (rc);
	}
	if (debug) 
		advise(LLOG_EXCEPTIONS, "DEBUG", 
			"smux_simple_open succeeded\n");
	rock_and_roll = 1;

	/* DPI port is provided by this program */
	rc = register_object(DPI_port_objID, -1);
	return (rc);
}

/* 
 *  Function: register_tree_smux
 *
 *  Inputs:	obj_id	The object identifier of the root of the tree to
 *					register in character string notation. (char *)
 *			fd		The file descriptor that handles the tree. (int)
 *
 *  Outputs: none
 *
 *  Returns: the return code of the register command. (int)
 *
 *  Note: This function takes the object indentifier, converts it to an OID
 *        and adds it to the registration list if the smux<->snmpd connection
 *        is down.  If the link is not down, the list is not updated until
 *        after the registration is attempted.  If the registration fails,
 *        the list is not updated and the smux<->snmpd connection is closed.
 *        Otherwise, the list is updated on successful completion.
 */
register_tree_smux(obj_id, fd)
char *obj_id;
int fd;
{
	char	tree_oid[512];
	int	rc;
	struct snmp_dpi_registered_objects	*ptr;
	int	k = smux_fd;

	strcpy(tree_oid, obj_id);
	tree_oid[strlen(obj_id) - 1] = '\0';	/* zap trailing dot */

	if (subtree != NULLOID) oid_free(subtree);

	subtree = str2oid(tree_oid);

	if (smux_fd == NOTOK) {
		if ((ptr = (struct snmp_dpi_registered_objects *)
			  calloc(1, sizeof(struct snmp_dpi_registered_objects)))
		     == NULL) {
			adios("Failed", "Cannot get memory");
		}

		ptr -> object = oid_cpy(subtree);
		ptr -> owning_fd = fd;
		ptr -> next = regobjs;
		regobjs = ptr;
	}

	rc = smux_register (subtree, -1, readWrite);
	if (rc == NOTOK) {
		del_from_mask(k);
		smux_fd = NOTOK;
		advise(LLOG_EXCEPTIONS, "Failed", 
			"smux_register (%s):  %s [%s]\n", tree_oid, 
			smux_error(smux_errno), smux_info);
	}

	if (smux_fd != NOTOK) {
		if ((ptr = (struct snmp_dpi_registered_objects *)
			  calloc(1, sizeof(struct snmp_dpi_registered_objects)))
		     == NULL) {
			adios("Failed", "Cannot get memory");
		}

		ptr -> object = oid_cpy(subtree);
		ptr -> owning_fd = fd;
		ptr -> next = regobjs;
		regobjs = ptr;
	}
	return (rc);
}

/* 
 *  Function: unregister_tree_smux
 *
 *  Inputs: obj_id  The object identifier of the root of the tree to delete
 *                  in character string notation. (char *)
 *          fd      The file descriptor that handles the tree. (int)
 *
 *  Outputs: none
 *
 *  Returns: the return code of the register command. (int)
 *
 *  Note: This function takes the object indentifier and converts it to an OID. 
 *        The smux_register command is called to delete the registered tree.
 *        If this fails, the smux<->snmpd file descriptor is closed.  The tree
 *        is removed from the registration list.
 */
unregister_tree_smux(obj_id, fd)
char *obj_id;
int fd;
{
	char	tree_oid[512];
	int	rc;
	int	k = smux_fd;
	struct snmp_dpi_registered_objects	*ptr;
	struct snmp_dpi_registered_objects	*ptr2;

	strcpy(tree_oid, obj_id);
	tree_oid[strlen(obj_id) - 1] = '\0';	/* zap trailing dot */

	if (subtree != 0) oid_free(subtree);

	subtree = str2oid(tree_oid);
	rc = smux_register (subtree, -1, delete);
	if (rc == NOTOK) {
		del_from_mask(k);
		advise(LLOG_EXCEPTIONS, "Failed",
			"smux_register (%s):  %s [%s]\n", tree_oid, 
			smux_error(smux_errno), smux_info);
	}

	if ((regobjs -> owning_fd == fd) && 
	    (oid_cmp(regobjs -> object, subtree) == 0)) {
		ptr = regobjs;
		regobjs = regobjs -> next;
		if (ptr -> object)
			oid_free(ptr -> object);
		free(ptr);
	}
	else {
		ptr = regobjs;
		ptr2 = regobjs;
		while ((ptr) && ((ptr -> owning_fd != fd) || 
		       (oid_cmp(ptr -> object, subtree) != 0))) {
			ptr2 = ptr;
			ptr = ptr -> next;
		}
		if (ptr) {
			ptr2 -> next = ptr -> next;
			if (ptr -> object)
				oid_free(ptr -> object);
			free(ptr);
		}
	}

	return (rc);
}

/* 
 *  Function: create_DPI_port
 *
 *  Inputs:	none
 *
 *  Outputs: none
 *
 *  Returns: the new DPI port (int)
 *
 *  Note: This function creates a socket and sets it up for listening.
 */
static int create_DPI_port()
{
	struct sockaddr_in new_sock;
	int ret_code;
	int socklen;

	DPI_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (DPI_fd < 0) {
		perror("create_DPI_port::socket");
		return(DPI_fd);
	}
	bzero(&new_sock, sizeof(new_sock));
	new_sock.sin_family = AF_INET;
	ret_code = bind(DPI_fd, &new_sock, sizeof(new_sock));
	if (ret_code < 0) {
		perror("create_DPI_port::bind");
		close(DPI_fd);
		DPI_fd = -1;
		return(ret_code);
	}
	listen(DPI_fd, 5);
	socklen = sizeof(new_sock);
	ret_code = getsockname(DPI_fd, &new_sock, &socklen);
	if (ret_code == -1) {
		perror("create_DPI_port:getsockname");
		return(-1);
	}
	DPI_port_num = ntohs(new_sock.sin_port);
	return(0);	/* all OK */
}

/* 
 *  Function: doit_smux
 *
 *  Inputs:	none
 *
 *  Outputs: none
 *
 *  Returns: the return code of the operation (int)
 *
 *  Note: The function handles the SMUX requests and response.  The smux_wait
 *		call makes sure there is data to read and reads it.  The switch 
 *		decides how to handle each type of packet that is received.
 */
static	doit_smux () {
	struct type_SNMP_SMUX__PDUs *event;
	int k = smux_fd;

    /*
     * This loop handles multiple messages.  smux_wait is used to check the
     * file descriptor for messages.  If the smux_wait's timeout parameter is 
     * set to zero, it returns without waiting for messages.  If no 
     * messages are on the queue, smux_wait returns a NOTOK with a smux_errno
     * equal to inProgress.  This is the signal that all messages currently on
     * the queue have been read. 
     */
    while (1) {

	if (smux_wait (&event, 0) == NOTOK) {
		if (smux_errno == inProgress)
			return(OK);

		del_from_mask(k);
		advise(LLOG_EXCEPTIONS, "Failed", "smux_wait: %s [%s]",
			smux_error(smux_errno), smux_info);
		return (NOTOK);
	}

	switch (event -> offset) {
	case type_SNMP_SMUX__PDUs_registerResponse:
		{
			struct type_SNMP_RRspPDU *rsp = event -> un.registerResponse;

			if (rsp -> parm == int_SNMP_RRspPDU_failure) {
				advise(LLOG_EXCEPTIONS, "Failed", 
					"SMUX registration of %s failed",
					sprintoid(subtree));
				del_from_mask(smux_fd);
				smux_close (goingDown);
				return (NOTOK);
			}
			if (debug)
				advise(LLOG_EXCEPTIONS, "DEBUG",
					"SMUX register: %s out=%d\n",
					sprintoid(subtree), rsp -> parm);

#ifdef COLD_START
			if (smux_trap (int_SNMP_generic__trap_coldStart,
			    0, (struct type_SNMP_VarBindList *) 0) == NOTOK) {
				advise(LLOG_EXCEPTIONS, "Failed",
					"smux_trap: %s [%s]",
					smux_error (smux_errno), smux_info);
				del_from_mask(k);
				smux_fd = NOTOK;
				return (NOTOK);
			}
#endif
		}
		break;

	case type_SNMP_SMUX__PDUs_get__request:
	case type_SNMP_SMUX__PDUs_get__next__request:
		get_smux (event -> un.get__request, event -> offset);
		break;
	case type_SNMP_SMUX__PDUs_set__request:
		get_smux (event -> un.get__request, event -> offset);
		break;

	case type_SNMP_SMUX__PDUs_close:
		del_from_mask(smux_fd);
		advise(LLOG_EXCEPTIONS, "Failed", "SMUX close: %s\n",
			smux_error (event -> un.close -> parm));
		smux_fd = NOTOK;
		return (NOTOK);

	case type_SNMP_SMUX__PDUs_simple:
	case type_SNMP_SMUX__PDUs_registerRequest:
	case type_SNMP_SMUX__PDUs_get__response:
	case type_SNMP_SMUX__PDUs_trap:
		advise(LLOG_EXCEPTIONS, "Failed", "unexpectedOperation: %d\n",
			event -> offset);
		del_from_mask(smux_fd);
		smux_close (protocolError);
		return (NOTOK);

	case type_SNMP_SMUX__PDUs_commitOrRollback:
/*		return (OK);	 do nothing */
		break;

	default:
		advise(LLOG_EXCEPTIONS, "Failed", "badOperation: %d\n",
			event -> offset);
		del_from_mask(smux_fd);
		smux_close (protocolError);
		return (NOTOK);
	}
    }

	return (OK);
}

/* 
 *  Function: get_smux
 *
 *  Inputs: pdu - the get request already parsed packet
 *          offset - the type of transaction taking place
 *
 *  Outputs: none
 *
 *  Returns: the return code of the operation (int)
 *
 *  Note: The function handles the SMUX get and get-next requests.  The
 *        function cycles through the variables in the variable binding
 *        list of the PDU. For each variable, process_req is called to
 *        handle the getting of the individual variables.  process_req
 *        returns an error code on failure that is placed in the reponse
 *        PDU.  process_req returns the answers in new_oid and new_pe.
 *        These are placed in the response PDU (the same one used to receive
 *        the request).  After all the variables are obtained, the response
 *        is sent and control heads back to the main loop.
 */
static	get_smux (pdu, offset)
register struct type_SNMP_GetRequest__PDU *pdu;
int	offset;
{
	int	    index, rc;
	register struct type_SNMP_VarBindList *vp;

	pdu->error__index = 0;	/* no error yet */
	pdu->error__status = 0;

	index = 0;
	for (vp = pdu -> variable__bindings; vp; vp = vp -> next) {
		register struct type_SNMP_VarBind *v;
		OID		new_oid;
		struct type_SMI_ObjectSyntax		*new_pe;

		v = vp -> VarBind;
		index++;
		rc = process_req(v->name, v->value, offset, &new_oid, &new_pe);
		if (rc != 0) {
			/* indicate error position */
			pdu->error__index = index;
			pdu->error__status = rc;
			break;
		}
		oid_free(v->name);
		v->name = new_oid;
		free_SMI_ObjectSyntax(v->value);
		v->value = new_pe;
	}

	if (smux_response (pdu) == NOTOK) {
		advise(LLOG_EXCEPTIONS, "Failed", "smux_response: %s [%s]\n",
			smux_error (smux_errno), smux_info);
		del_from_mask(smux_fd);
		smux_fd = NOTOK;
		return (NOTOK);
	}
	return (OK);
}

/* 
 *  Function: must_read
 *
 *  Inputs: fd - file descriptor to read from
 *          buffer - the place to put data
 *          required_len - the amount to read
 *
 *  Outputs: none
 *
 *  Returns: the return code of the operation (int)
 *
 *  Note: The function handles the read from a file descriptor.  This is used
 *		by the DPI functions.  The file descriptor is placed into a select mask
 *		and select is used to make sure there is data to read.  A timeout of 
 * 		8 seconds is used to keep from waiting forever.  If select fails or
 *		times out, the select return code is returned.  Otherwise, the data,
 *		up to the amount of required_len, is read.  If the data is in multiple 
 *		packets and the packets after the first have problems, the must_read
 *		routine will return the error returned by select, but the buffer will
 *		contain all that was read until the select error.
 */
int must_read(fd,buffer,required_len)
int fd;
unsigned char *buffer;
int required_len;
{
	struct timeval timeout;
	register int bytes;
	register int bytes_to_read;
	register int bytes_read;
	unsigned long read_mask;

	bytes_to_read = required_len;
	bytes_read = 0;

	while (bytes_to_read > 0) {	/* while more to get */
		read_mask = 1 << fd;	/* check for data */
		timeout.tv_usec = 0;
		timeout.tv_sec = 8;
		bytes = select(fd+1,&read_mask,0,0,&timeout);
		if (bytes == 1) {	/* select returned OK */
			bytes = read(fd,buffer + bytes_read,bytes_to_read);
		}
		if (bytes <= 0) return(bytes);
		else {
			bytes_read += bytes;
			bytes_to_read -= bytes;
		}
	}
	return (bytes_read);
}

/* 
 *  Function: read_DPI_request
 *
 *  Inputs:	fd - file descriptor to read from
 *
 *  Outputs: none
 *
 *  Returns: the return code of the operation (int)
 *
 *  Note: The function handles the read from a file descriptor.  This function
 *		is used to handle the unexpected/unsolicited packets from the DPI
 *		peers.  The only legal packets are registration and trap packets.
 *		First, must_read is called to determine the length of the packet.
 *		Then the packet is read and parsed, and anything other than
 *		registers or traps is discarded.  If an error occurs, the function
 *		returns a -1.
 */
static read_DPI_request(fd)
int fd;
{
	unsigned char bfr[1024];
	int rc, len;
	struct snmp_dpi_hdr *hdr;

	if (debug)
		advise(LLOG_EXCEPTIONS, "DEBUG", "read_DPI_request(%d)\n", fd);

	rc = must_read(fd,bfr,2);
	if (rc != 2) {
		if (debug)
			advise(LLOG_EXCEPTIONS, "DEBUG",
				"could not read DPI request length\n");

		unregister_fd(fd);
		return(-1);
	}
	len = (bfr[0] << 8) + bfr[1];
	rc = must_read(fd,bfr+2,len);
	if (rc != len) {
		advise(LLOG_EXCEPTIONS, "Failed",
			"could not read DPI request body\n");
		unregister_fd(fd);
		return(-1);
	}
	hdr = pDPIpacket(bfr);
	if (hdr == 0) {
		advise(LLOG_EXCEPTIONS, "Failed",
			"could not parse DPI request packet\n");
		unregister_fd(fd);
		return(-1);
	}
	switch (hdr->packet_type) {
	case SNMP_DPI_REGISTER:
		register_object(hdr->packet_body.dpi_get->object_id,fd);
		break;
	case SNMP_DPI_TRAP:
		process_trap(hdr->packet_body.dpi_trap->generic,
		    hdr->packet_body.dpi_trap->specific,
		    hdr->packet_body.dpi_trap->info);
		break;
	default:
		break;
	} /* end switch */
	fDPIparse(hdr);
	return(0);
}

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
 *		above and an LLOG_FATAL error code.  The function then exits with a
 *		return code of 1, thereby terminating the dpid process.
 */
#ifndef lint
void    adios (va_alist)
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

void    adios (what, fmt)
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
 *          fmt - the string to be printed in printf format (char*)
 *          variables - variable needed to fill in the fmt.
 *
 *  Outputs: none
 *
 *  Returns: none
 *
 *  Note: The advise function calls the logging function with the variables 
 *		above.  This is a usability front end to the logging functions.
 */
#ifndef lint
void    advise (va_alist)
va_dcl
{
	int     code;
	va_list ap;

	va_start (ap);

	code = va_arg (ap, int);

	_ll_log (pgm_log, code, ap);

	va_end (ap);
}
#else
/* VARARGS */

void    advise (code, what, fmt)
char   *what,
       *fmt;
int     code;
{
	advise (code, what, fmt);
}
#endif


