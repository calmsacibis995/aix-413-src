static char sccs_id[] = " @(#)44 1.10  src/bos/usr/sbin/nimesis/nimesis.c, cmdnim, bos411, 9428A410j  2/25/94  16:07:26";
/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/nimesis/nimesis.c
 *		daemonize
 *		main
 *		nimreq
 *		nimserv
 *		setup
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*----------------------------- MAIN
 *
 * NAME: MAIN
 *
 * FUNCTION:
 *	This is the main function of the NIM server daemon NIMESIS.  
 *	It calls setup which performs the required items allowing us to 
 *	become a daemon it also initializes the  TCP environment.   The function 
 *	nimServ is called to listen for incomming requests and to handle the 
 *	processing of them. 
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *
 * RETURNS:
 *
 * EXITS:
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

#include "cmdnim_mstr.h"
#include "nimesis.h"
#include <varargs.h>

ATTR_ASS_LIST attr_ass;


/*----------------------------- MAIN 
 *
 * NAME:	 MAIN
 *
 * FUNCTION:	Main for nimesis
 *		nimserv is the main driver of this dross.
 *
 * EXITS:
 *	SUCCESS	 = Everything work ok
 *	!SUCCESS = something went wrong
 *
 *----------------------------------------------------------------------------*/

main(argc, argv)
int	argc;
char	*argv[];
{
	int	rc=SUCCESS;

	NIM_SOCKINFO	sockinfo; 
	NIM_SOCKINFO	sockreg; 


	/* 
	 * If we get all the way through the setup function it will 
	 * return SUCCESS which means that we can start to listen.
	 */
	rc = setup(argc, argv, &sockinfo, &sockreg);

	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	if ( rc == SUCCESS) {
		/* 
		 * The initial setup was ok so now call the nimserv function
		 * to read and service the requests from our clients.
		 */
		rc=nimserv(&sockinfo, &sockreg);
	}
	exit(rc);
}

/*----------------------------- my_sigs 
 *
 * NAME:	 my_sigs
 *
 * FUNCTION:	Set up signal stuff for NIMESIS.
 *
 * DATA STRUCTURES:
 *		parameters:  	NONE
 *		global:		NONE
 * RETURNS:
 *	SUCCESS	= Everything work ok
 *
 *----------------------------------------------------------------------------*/
int 
my_sigs()
{
extern errsig_handler(); 

   errsig[SIGINT] = errsig_handler;
   errsig[SIGHUP] = errsig_handler;
   errsig[SIGQUIT] = errsig_handler;
   errsig[SIGABRT] = errsig_handler;
   errsig[SIGSEGV] = errsig_handler;
   errsig[SIGSYS] = errsig_handler;
   errsig[SIGTERM] = errsig_handler;
   errsig[SIGCONT] = SIG_IGN;
   errsig[SIGCHLD] = SIG_IGN;
   
   return( SUCCESS );
}

/*----------------------------- setup 
 *
 * NAME:	setup
 *
 * FUNCTION: 	Perform set up for cmdnim daemon nimesis.		
 *
 * DATA STRUCTURES:
 *		parameters:
 *			argc	: The count of arguments passed in to program
 *			argv	: The arguments.
 *			sockinfo: Socket info for Registered NIM Clients.
 *			sockreg : Socket info for NIM Client registration REQ.
 *		global:
 *			debug	: May set the debug flag.
 *
 * RETURNS:
 *	SUCCESS	= Everything work ok
 *	FAILURE	= Error in the setup process
 *
 * EXITS:
 *
 *----------------------------------------------------------------------------*/

int 
setup ( int	argc, 
	char 	*argv[], 
	NIM_SOCKINFO *sockinfo, 
	NIM_SOCKINFO *sockreg )
{	 

#define	DEBUG_PARM	'd'
#define	SRCMSTR   	's'
#define	ALLOWED_PARMS	"ds"

extern	int	mstr_exit();
extern	int	debug;
extern	char	*bugFile;
extern	FILE	*bugFP;
extern	int	optind;
extern	char	*optarg;


	int	rc;
	int	kidpid;	        /* The pid of a child		  	*/
	int	option; 	/* The current option to look at  	*/
	int	hasTTY;		/* tty file descriptor		  	*/
	int	loopCnt;	/* Handy loop counter		  	*/
	int	srcMstr=0;	/* Flag to say if src master started us */
 	char	criteria[80]; 
		
	/* 
	 *   Get the parameters that have been passed to us, scan them 
	 *   and set up any environment changes based on the information.
	 */
	while ( (option=getopt(argc, argv, ALLOWED_PARMS)) != EOF ) {
		switch(option) {
			/* 
			 * If the d option is picked flag debug mode
			 */
			case DEBUG_PARM:
				debug++;
				break;
			/* 
			 * If the s option is used assume we're started from
			 * srcmstr 
			 */
			case SRCMSTR:
				srcMstr++;
				break;
		}
	}
	
	/* 
	 * When we define the command line with mkssys command we need to specify 
	 * -s so we know that we're started with src master. 
	 */
	if (!srcMstr) 
		if ( daemonize() != SUCCESS )
			return(FAILURE);

	/* 
	 * Call the nim init function to set-up our environment, as we're the master 
	 * we usr the master exit function and we also set-up our own signals.
	 */ 
	mstr_init(argv[0], ERR_POLICY_DAEMON, my_sigs, mstr_exit);


	/* 
	 * If user wishes to see what is going on (what is wrong with ip trace ?) 
	 * attempt to open the debug file
	 */
	if (debug) { 
		bugopen();	
	}

	/*
	 * Remove any dangling locks 
	 */ 
	sprintf( criteria, "pdattr=%d", ATTR_LOCKED );
	odm_rm_obj( nim_attr_CLASS, criteria ); 



	buglog("NIMesis (%d) : Process started in debug-mode", niminfo.pid);
	chdir(NIM_DIR);

	/*
	 *  Now that we're done with the Daemon stuff we need to start 
	 *  up the TCP/IP stuff.
	 *
	 *  First: The registered NIM clients connect to this socket. 
	 */
	sockinfo->addr.sin_addr.s_addr	= INADDR_ANY;
	sockinfo->addr.sin_family 	= AF_INET;
	sockinfo->addr.sin_port   	= htons( niminfo.master_port );	
	if ( (rc=getsocket(sockinfo)) == SUCCESS ) {
		if (bind(sockinfo->fd,&sockinfo->addr,sizeof(sockinfo->addr))<0){
			nene(ERR_ERRNO_LOG, "setup", "bind-MASTER", NULL);
			return(FAILURE);
		}
	}
	buglog("NIMesis (%d) : BIND %d", niminfo.pid, niminfo.master_port);

	/* 
	 * Second: The potential NIM clients can request registration 
	 * on this socket. 
	 */
	sockreg->addr.sin_addr.s_addr	= INADDR_ANY;
	sockreg->addr.sin_family 	= AF_INET;
	sockreg->addr.sin_port   	= htons( niminfo.master_port+1 );	
	if ( (rc=getsocket(sockreg)) == SUCCESS ) {
		if (bind(sockreg->fd,&sockreg->addr,sizeof(sockreg->addr))<0){
			nene(ERR_ERRNO_LOG, "setup", "bind-REG", NULL);
			return(FAILURE);
		}
	}
	buglog("NIMesis (%d) : BIND %d", niminfo.pid, niminfo.master_port+1);

	signal(SIGPIPE, SIG_IGN);
	return(SUCCESS);
} /* end of setup */

/*----------------------------- daemonize 
 *
 * NAME:	 daemonize
 *
 * FUNCTION:
 *	Do the required steps to fork and make us a daemon		
 *
 * DATA STRUCTURES:
 *		parameters:
 *			<none> 
 *		global:
 *			Uses debug to find out if we fork or not.
 *
 * RETURNS:
 *	SUCCESS	
 *	FAILURE	
 *
 *----------------------------------------------------------------------------*/

int
daemonize()
{	
	int	rc=SUCCESS;	

	int	kidpid;
	int	hasTTY;		/* tty file descriptor		  */
	int	loopCnt;	/* Handy loop counter		  */

	/*
         *  -- Make us a daemon -- 
	 *  Fork (letting parent die), close any open file descriptors
	 *  (should not be too many of those),Remove the connection of the TTY.
         */
	if ( (kidpid = fork() ) < 0 ) {
		nene(ERR_ERRNO_LOG, "daemonize", "fork", NULL);
		return(FAILURE);
	}
	/*
	 * It worked,  so if we are the parent exit. 
	 */
	if ( kidpid > 0 )
		exit(0);

	/* 
	 * --- Child Process continues from here ---
	 * 
	 *   Now change our process group and let go the controlling 
	 * TTY.. 
	 */

	if (setpgrp(0, getpid() ) != -1) {
		/* 
		 * If we had a controlling tty drop it. 
		 */
		if (  (hasTTY = open( "/dev/tty", O_RDWR)) >=0 ) {
			ioctl( hasTTY, TIOCNOTTY, (char *) NULL);
			close( hasTTY );
		}
	}
	else 
		rc=FAILURE;
	return(rc);
} 

/*----------------------------- do_accept 
 *
 * NAME:	do_accept
 *
 * FUNCTION:	Accepts requests for NIM services from NIM clients. 
 *
 * DATA STRUCTURES:
 *		parameters:
 *			inMsg : socket info of new connection. 
 *			sinfo : socket info of listen'er... 
 *
 * RETURNS:
 *	SUCCESS	= Accepted !
 *	FAILURE = NOT Accepted !
 *
 *----------------------------------------------------------------*/

int
do_accept ( NIM_SOCKINFO *inMsg, 
	    NIM_SOCKINFO *sinfo)
{
	int	addrSize = sizeof(inMsg->addr);

	inMsg->fd=accept(sinfo->fd,(struct sockaddr *)&(inMsg->addr),&addrSize);
	if ( inMsg->fd == -1 ) {
		/* 
	 	 * The accept got an error nene and return fail.
	 	 */
		nene(ERR_ERRNO_LOG, "nimserv", "accept", NULL);
		return(FAILURE);
	}
	return(SUCCESS);
}

/*----------------------------- nimserv 
 *
 * NAME:	 nimserv
 *
 * FUNCTION:
 * This function accepts requests for NIM services from NIM clients. 
 *
 * DATA STRUCTURES:
 *		parameters:
 *			sockinfo : Pointer to socket information struct.
 *
 * RETURNS:
 *	SUCCESS	
 *	FAILURE
 *
 *----------------------------------------------------------------*/

int 
nimserv ( NIM_SOCKINFO *sockinfo, 
	  NIM_SOCKINFO *sockreg )
{
	int	rc=SUCCESS;

	fd_set	inBound, sockSet; 
	int	maxfd; 

	NIM_SOCKINFO	inMsg;

	/*
	 * Lets listen to the ports for incoming requests.
	 */
   	if (listen(sockinfo->fd, 10) == -1 ) {
		nene(ERR_ERRNO_LOG, "nimserv", "listen-info", NULL);
		return(FAILURE);
	}
	buglog("NIMesis (%d) : METHODS on %d ", getpid(),  niminfo.master_port);

   	if (listen(sockreg->fd, 10) == -1 ) {
		nene(ERR_ERRNO_LOG, "nimserv", "listen-reg", NULL);
		return(FAILURE);
	}
	buglog("NIMesis (%d) : NIMREG  on %d ", getpid(),  niminfo.master_port+1);

	/* 
	 * Set up bit masks for the select, sockSet sez which fd we are 
	 * intrested in, and inBound will say which has pending requests
	 */ 
	FD_ZERO(&inBound); 
	FD_ZERO(&sockSet); 
	FD_SET(sockinfo->fd, &sockSet); 
	FD_SET(sockreg->fd, &sockSet);

	/* 
	 * Calc and set the max fd (highest) we are at.. 
	 */
	maxfd = ( (sockreg->fd > sockinfo->fd) ? sockreg->fd : sockinfo->fd);

	/* 
	 * The select deals with the NUMBER of file descriptors. As we start 
	 * at zero we'll need to bump maxfd.. 
	 */
	maxfd++;

	/* 
	 * for as long as we dont have a failure DO; 
	 */
	while (1) { 
		/* 
		 * reset/set the inBound mask 
		 */
		inBound = sockSet;
		if (select(maxfd, &inBound, 0, 0, NULL) < 0) {
			/*
		 	 * The select came back with an error, check to see
		 	 * if we need to re-try else error out
			 */
			if (errno != EINTR) {
				nene(ERR_ERRNO_LOG, "nimserv", "select", NULL);
				return(FAILURE);
			}	
			continue;
		}

		/*
		 * Do we have a regular NIM Client request ? 
		 */
		if (FD_ISSET(sockinfo->fd, &inBound) ) {
			if (do_accept( &inMsg, sockinfo) == FAILURE) {
				return(FAILURE);
			}
			nimreq(&inMsg, NOT_REGISTRATION);
			close(inMsg.fd);
		}

		/*
		 * Do we have a registration request from potential NIM Client ? 
		 */
		if (FD_ISSET(sockreg->fd, &inBound) ) {
			if (do_accept( &inMsg, sockreg) == FAILURE) {
				return(FAILURE);
			}
			nimreq(&inMsg, IS_REGISTRATION);
			close(inMsg.fd);
		}
	}
}


/*----------------------------- nimreq 
 *
 * NAME:	 nimreq
 *
 * FUNCTION:
 * This function processes requests from NIM clients. 
 * 
 * NOTES:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *
 * RETURNS:
 *	SUCCESS		
 *	FAILURE	
 *
 *
 *----------------------------------------------------------------------------*/

int 
nimreq ( NIM_SOCKINFO *inMsg, 
	 int isReg)
{



	int	reqKid;

	int	rc=SUCCESS;

	if ( (reqKid = fork() ) < 0 ) {
		/* 
	 	 * Can NOT fork another process... 
	 	 */ 
		nene(ERR_ERRNO_LOG, "nimreq", "fork", NULL);
		return(FAILURE);
	}
	else { 
		/*
	 	* It worked,  so if we are the parent exit. 
	 	*/
		if ( reqKid > 0 )
			return(SUCCESS);
	}

	/* become the process group leader so that locking inheritance works */
	/*		correctly */
	if ( setpgid( 0, getpid() ) == -1 )
		exit( errno );

	/* 
	 * Because we have forked we need to setup the nim info 
	 * variable stuff again. 
	 */
	nim_init_var("NIMkid");

	/* 
	 * Now update the child signal stuff
	 */
   	errsig[SIGCHLD] = SIG_DFL;
   	signal(SIGCHLD, SIG_DFL);

	rc=nimkid(inMsg, isReg);

	exit(rc);

} 
