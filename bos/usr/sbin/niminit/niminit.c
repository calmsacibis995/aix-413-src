static char     sccs_id[] = " @(#)65 1.24.1.6  src/bos/usr/sbin/niminit/niminit.c, cmdnim, bos41J, 9524A_all  6/11/95  15:24:04";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: check_input
 *		main
 *		parse_args
 *		setup
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

#include "cmdnim_cmd.h"
#include "cmdnim_ip.h"
#include <varargs.h> 

/*---------------------------- globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = {
	{ ATTR_NAME, 		ATTR_NAME_T, 	   	TRUE, 	NULL },
	{ ATTR_MASTER, 		ATTR_MASTER_T, 		TRUE, 	NULL },
	{ ATTR_MASTER_PORT, 	ATTR_MASTER_PORT_T, 	TRUE, 	NULL },
	{ ATTR_ADPT_ADDR, 	ATTR_ADPT_ADDR_T,   	FALSE, 	NULL },
	{ ATTR_PIF_NAME, 	ATTR_PIF_NAME_T,   	FALSE, 	NULL },
	{ ATTR_RING_SPEED,	ATTR_RING_SPEED_T,	FALSE, 	NULL },
	{ ATTR_CABLE_TYPE,	ATTR_CABLE_TYPE_T,	FALSE, 	NULL },
        { ATTR_PLATFORM,        ATTR_PLATFORM_T,        FALSE,  NULL },
	{ ATTR_ADPT_ADDR, 	ATTR_ADPT_ADDR_T,  	FALSE, 	NULL },
	{ ATTR_IPLROM_EMU, 	ATTR_IPLROM_EMU_T,	FALSE, 	NULL },
	{ ATTR_COMMENTS, 	ATTR_COMMENTS_T,   	FALSE, 	NULL },
	{ 0, 			NULL, 			FALSE, 	NULL }
};

#define RMITAB "/usr/sbin/rmitab" 
#define MKITAB "/usr/sbin/mkitab" 

#define MKITAB_ARG NIMCLIENT " -S running > /dev/console 2>&1 # inform nim we're running" 
#define MKITAB_ENTRY "nimc:2:wait:" MKITAB_ARG

int	verbose;
int	do_cleanup; 

/*---------------------------------------------------------------------*/

/*----------------------------- verbose_txt 
 *
 * NAME: verbose_txt
 *
 * FUNCTION:   Checks the verbose flag, if set uses the variable input 
 *    parameters to write a line of text to stdout. 
 *
 *    Call this chap like this: 
 *    verbose_txt("%s: a format string", argv[0] );
 *
 *
 * DATA STRUCTURES:
 *    parameters:
 *    global:
 *       verbose : checks to see if set. 
 *
 * RETURNS:
 *    0 - Not in verbose mode.
 *    1 - everything is ok (probably)
 *
 *----------------------------------------------------------------------------*/


verbose_txt(va_alist)
va_dcl
{
	extern	int verbose;

	va_list  ptr;

	char	*fmt;

	if (!verbose)
		return(0);

	va_start(ptr);
	fmt = va_arg(ptr, char * );

	vprintf(fmt, ptr);
	fflush(stdout);
	return(1);
}

/* --------------------------- doUsage
 *
 * NAME: doUsage
 *
 * FUNCTION: Printsout the syntax and usage messages. 
 *
 * NOTES:	calls nene, cmd_what & exits with rc passed as parameter
 *		
 * --------------------------------------------------------------------------- */

void
doUsage(int rc)
{
	niminfo.errstr = MSG_msg(MSG_NIMINIT_SYNTAX);
	nene(0, NULL, NULL, NULL);
	cmd_what(valid_attrs);
	exit( rc );
}

/*---------------------------- check_input
 *
 * NAME: check_input
 *
 * FUNCTION:
 *	checks to make sure that the information supplied by user is sufficient
 *	to complete operation
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		global:
 *			valid_attrs
 *			attr_ass
 *
 * RETURNS: (int)
 *		SUCCESS	= nothing missing
 *		FAILURE	= definition incomplete
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

int
check_input()

{
	struct servent *serv;
	int	va_indx, attr_indx;
	VALID_ATTR 	 * va_ptr;
	ATTR_ASS 	 * aa_ptr;

	/* 
	 * check for required attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( va_ptr->required ) {
			for (attr_indx = 0; attr_indx < attr_ass.num; attr_indx++) {
				aa_ptr = attr_ass.list[attr_indx];
				if ( aa_ptr->pdattr == va_ptr->pdattr )
					break;
			}
			/* 
			 * is the required attr present? 
			 */
			if ( attr_indx == attr_ass.num ) {
				export_errstr( ERR_MISSING_ATTR, va_ptr->name, NULL, NULL );
				doUsage(ERR_MISSING_ATTR);
			}
		}
	}

	/* 
	 * Now lets check for specific attrs 
	 * and do whatever we need to do with them... 
	 */
	for (attr_indx = 0; attr_indx < attr_ass.num; attr_indx++) {
		aa_ptr = attr_ass.list[attr_indx];
		switch ( aa_ptr->pdattr ) {
		case ATTR_MASTER : 
			{
				/* make sure master's hostname can be resolved */
				if ( verify_hostname( aa_ptr->value, NULL, NULL ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
				niminfo.master.hostname=aa_ptr->value;
			}
			break;

		case ATTR_PIF_NAME  :
			/* 
	 		 * Get the interfaces network address, IP Address 
			 * netmask , and device name...
	 		 */
			if ( getnet( &attr_ass, aa_ptr->value) != SUCCESS )
				nim_error(0, NULL, NULL, NULL);
			break;

		case ATTR_MASTER_PORT  :
			niminfo.master_port = atoi( aa_ptr->value );
                        if (((serv = getservbyport(niminfo.master_port, "tcp")) != NULL ) &&
				(strcmp(serv->s_name, NIM_SERVICE ) != 0 )) {
				nim_error(ERR_ON_PORT, (char *)niminfo.master_port,
					serv->s_name, NULL);
			}
			if (((serv = getservbyport(niminfo.master_port + 1, "tcp")) != NULL ) &&
				(strcmp(serv->s_name, NIM_REG_SERVICE ) != 0 )) {
				nim_error(ERR_ON_PORT, (char *)niminfo.master_port + 1, 
					serv->s_name, NULL);
			}
			break;

		}
	}
} /* end of check_input */

/*---------------------------- bldPkt
 *
 * NAME: bldPkt
 *
 * FUNCTION:
 *	Builds the attributes into one big buffer for 
 *	the nimreg portion of nimesis.
 *
 * DATA STRUCTURES:
 *		parameters:
 *			Pkt  = The buffer to stuff the attrs into.
 *		global:
 *			attr_ass = The global attrs.
 *
 * RETURNS: (int)
 *		SUCCESS	
 *
 *-----------------------------------------------------------------------------*/
int
bldPkt(char *Pkt )

{
	int		attr_indx, attr_cntr=0;
	ATTR_ASS 	* aa_ptr;
	char		tmp[MAX_TMP]; 

	/* 
	 * Loop through the attrs to find the attrs to send to the 
	 * master. 
	 */
	for (attr_indx = 0; attr_indx < attr_ass.num; attr_indx++) {
		aa_ptr = attr_ass.list[attr_indx];
		switch ( aa_ptr->pdattr ) {

		/* 
		 * These can be from the cmd line input. 
		 */
		case ATTR_NAME     :
		case ATTR_IPLROM_EMU  :
		case ATTR_PIF_NAME :
		case ATTR_RING_SPEED :
		case ATTR_CABLE_TYPE :
                case ATTR_PLATFORM :
		case ATTR_COMMENTS :

		/* 
		 * These are procured internally to this program
		 */
		case ATTR_NET_ADDR  :
		case ATTR_SNM       :
		case ATTR_CPUID     :
		case ATTR_ADPT_NAME :
		case ATTR_ADPT_ADDR :
				if ( aa_ptr->seqno > 0 )
					sprintf( tmp, "%s%d=%s\n", aa_ptr->name,
								aa_ptr->seqno, aa_ptr->value);
				else
					sprintf(tmp, "%s=%s\n", aa_ptr->name, aa_ptr->value);
				strcat(Pkt, tmp);
				attr_cntr++;
			break;

		}
	}
	return(attr_cntr);
}

/*---------------------------- parse_args
 *
 * NAME: parse_args
 *
 * FUNCTION:
 *		parses command line arguments
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		calls nim_error
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			argc  = argc from main
 *			argv  = argv from main
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS	= no syntax errors on command line
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

int
parse_args(	int argc, char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;
	int	syntax_err = FALSE;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "a:v")) != -1 ) {
		switch (c) {

		/* 
		 * attribute assignment 
		 */
		case 'a':
			if (!parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE ) )
			{
				if (niminfo.errno == ERR_INVALID_ATTR ||
					niminfo.errno == ERR_VALUE)
				{
                               		nene( 0, NULL, NULL, NULL);
					doUsage(ERR_INVALID_ATTR);
				}
				else
					nim_error( 0, NULL, NULL, NULL );
			}
			break;
		/* 
		 * set verbose mode on
		 */
		case 'v':
			verbose++;
			verbose_txt("\n%s\n", argv[0]);
			break;

		/* 
		 * option is missing a required argument 
		 */
		case ':':
			nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;
		/* 
		 * unknown option 
		 */
		case '?':
			doUsage(ERR_BAD_OPT);
			break;
		}
	}
	return( SUCCESS );
} /* end of parse_args */

/* --------------------------- init_exit
 *
 * NAME: init_exit
 *
 * FUNCTION: Remove the niminfo file if the global flag tells us 
 *			    to do it...
 *
 * DATA STRUCTURES:
 *		Global: 
 *			do_cleanup = Flag to say remove the niminfo file.
 *
 * ------------------------------------------------------------ */

void
init_exit()
{ 
	char	*rmitab_arg[] = { RMITAB, "nimc",  NULL };
	FILE	*FP=NULL; 
	int	rc; 

	if (do_cleanup) { 
		unlink(NIM_ENV_FILE);
		client_exec(&rc, &FP, rmitab_arg);
	}
	nim_exit(); 
}

/*---------------------------- setup
 *
 * NAME: setup
 *
 * FUNCTION:
 *		Get the environment ready to contact the master.
 *
 * DATA STRUCTURES:
 *		parameters:
 *			argc  = argc from main
 *			argv  = argv from main
 *
 * RETURNS: (int)
 *		SUCCESS	= setup the nim psuedo environment.
 *		FAILURE = Unable to setup.
 *
 *-----------------------------------------------------------------------------*/

setup(int argc, char *argv[] )

{

	extern int	nim_exit();
	struct utsname uname_info;
static	char	base_name[MAX_TMP];
	char	tmp[MAX_TMP];
	char	*ptr;
	int	rc; 			/* generic return code */
	int	i;
	FILE	 * out = NULL;
	struct servent *servPtr;
 
	setbuf( (FILE *) stdout, NULL); 
	setbuf( (FILE *) stderr, NULL); 
	/* 
	 * initialize the niminfo struct 
	 */
	memset( &niminfo, '\0', sizeof(struct nim_info ) );

	/* alloc space for errstr */
	niminfo.errstr = malloc( MAX_NIM_ERRSTR + 1 );


	/* 
	 * make sure that an exit routine is called upon exit 
	 */
	atexit( init_exit );

	/* 
	 * initialize errsig 
	 */
	memset( errsig, NULL_BYTE, sizeof(void *) * SIGMAX );
	nim_siginit();


	/* 
	 * enable signals (uses the errsig array) 
	 */
	enable_err_sig();

	/* 
  	 * command name (from argv[0]) is really just the basename.
	 */
	if ( (ptr = strrchr(argv[0], '/')) == NULL )
		ptr = argv[0];
	else
		ptr++;

	strncpy(base_name, ptr, MAX_TMP);

	/* 
	 * set the umask to 0x022 
	 */
	umask( S_IWGRP | S_IWOTH );

	/* 
	 * Do the variable init 
	 */
	nim_init_var( base_name );

	/* 
	 * Reset the glock flag 
	 */
	niminfo.glock_held = FALSE;

	/* 
	 * this machine's cpu id 
	 */
	if ( uname(&uname_info) > -1 ) {
		add_attr( &attr_ass, ATTR_CPUID, ATTR_CPUID_T, uname_info.machine );
		strcpy( niminfo.cpu_id, uname_info.machine );
	}
	/* 
	 * error policy controls behavior of nim_error 
	 */
	niminfo.err_policy = ERR_POLICY_DISPLAY;

        /*
         * internationalization
         */
        setlocale( LC_ALL, "" );
	niminfo.msgcat_fd = catopen(MF_CMDNIM, NL_CAT_LOCALE);

	/* 
	 * load the regular expression arrays 
	 */
	for (i = 0; i < MAX_NIM_ERE; i++) {	
		if ( (nimere[i].reg = malloc(sizeof(regex_t))) == NULL )
			ERROR( ERR_SYS, "malloc-", strerror(errno), NULL );
		if ( (rc = regcomp( nimere[i].reg, nimere[i].pattern, REG_EXTENDED)) != 0)
		{
			regerror( rc, nimere[i].reg, tmp, MAX_TMP );
			ERROR( ERR_SYS, "recomp", tmp, NULL );
		}
	}

	/* parse command line */
	parse_args( argc, argv );

	/* look for the /usr/sbin/nim command: if present, it indicates that the */
	/*		bos.sysmgt.nim.master package has been installed */
	/* when it has, we don't want the niminit command to run */
	if ( access( NIM, R_ACC ) == 0)
		ERROR( ERR_IS_MASTER, NULL, NULL, NULL );

	/* 
	 * Test to see if the /etc/niminfo (or what ever we call it) file 
	 * exists. If it does then this machine probably already has been 
	 * registered with the master so exit.
	 */
	if ( access(NIM_ENV_FILE, R_ACC) == 0)
		ERROR( ERR_EXISTS, NIM_ENV_FILE, NULL, NULL);

	if ( errno != ENOENT )
		ERROR( ERR_ERRNO_LOG, "set_up", NIM_ENV_FILE, NULL);

 	/* 
 	 * Get nimesis port number if its in the /etc/services file..
 	 */
	if ( (servPtr=getservbyname(NIM_SERVICE, "tcp")) != NULL ) 
		niminfo.master_port = servPtr->s_port;

	check_input( );
	return(SUCCESS);
}

/**************************       main         ********************************/

main( int argc, char *argv[] )

{

	int	rc; 			/* generic return code */
	int	rmt_sd, rmt_err;
	int	on=1, attr_indx, attr_cnt, buffer_size; 
	ATTR_ASS 	* aa_ptr;

	FILE	*nimFP, *out=NULL;
	char	tmp[512];
	char	perms[MAX_TMP];
	char	host[MAX_TMP];
	char	*hostname;
	char	Pkt[MAX_NIM_PKT];
	char	*bufPtr; 

	char	*ping_args[]= { PING, "-c 1", NULL, NULL};
	char	*add_route_arg[] = { C_ADD_ROUTES, NULL };
	char	*rhost_args[] = { C_CH_RHOST, perms, host, NULL };
	char	*mkitab_args[] = { MKITAB, "-i", "cron", MKITAB_ENTRY,  NULL };

	/* examine SMIT_NO_EXEC env variable */
	smit_no_exec( argc, argv );

	/* 
	 * If setup finds and error it will cause the program 
	 * to exit... 
	 */
	if (setup(argc, argv) != SUCCESS)
		nim_error(0, NULL, NULL, NULL);

	/*
	 * Lets see if we can ping the master 
	 */
	ping_args[2] = niminfo.master.hostname;
	verbose_txt("Testing if we can access the Master @ %s\n", ping_args[2] );
	if (client_exec(&rc, &out, ping_args ) == FAILURE )
		nim_error( ERR_NO_PING, niminfo.master.hostname, NULL, NULL );

	/* 
	 * The client_exec worked but is the master out on the net ? 
	 */
	if ( rc == 1 )
		nim_error( ERR_NO_PING, niminfo.master.hostname, NULL, NULL );

	/* 
	 * OK, now we are to the point of sending the machine registration 
	 * to the master... 
	 * Send a pucker-up request to the NIM Master via rcmd (using NIM 
	 * well known port plus one)
	 */
	verbose_txt("Establishing contact with NIM master \n");
	rmt_sd = rcmd( &(niminfo.master.hostname), niminfo.master_port + 1, 
	    niminfo.master_uid, niminfo.master_uid, niminfo.nim_name, &rmt_err );

	/*
	 * If rcmd had a problem then error out 
	 */
	if (rmt_sd < 0 )
		nim_error( ERR_ERRNO_LOG, MSG_msg (MSG_INIT_TO_MSTR), 
				MSG_msg (MSG_RCMD), NULL );
	/*
	 * Set the socket option to add SO_KEEPALIVE (just incase the 
	 * other end goes away.. we need to know these things). 
	 */ 
	 if ( setsockopt(rmt_sd, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, 
				    sizeof(on) ) == -1 ) 
		  ERROR( ERR_ERRNO_LOG, "To_Master", "setsockopt", NULL ); 

	/* 
	 * Else Build the request packet.. 
	 */
	if ( (attr_cnt=bldPkt(Pkt)) > 0 ) {
		/* 
		 * If we have stuff to send then attr_cnt will be positive, so 
		 * we need to send the attribute count so nimreg knows what to expect. 
		 */ 
		sprintf(tmp, "%d", attr_cnt); 
		if ( write(rmt_sd, tmp, strlen(tmp) + 1 ) < 0 ) {
			close(rmt_sd);
			nim_error( ERR_ERRNO_LOG, MSG_msg (MSG_SEND_ATTR_COUNT), 
															MSG_msg (MSG_WRITE), NULL );
		}	
		/* 
		 * Because the "other-end" expects each attribute to be null terminated 
		 * we are going to convert the \n to \0... and then ship the whole lot 
		 * off in one large buffer. 
		 */ 
		buffer_size = strlen(Pkt)+1; 

		bufPtr=Pkt;
		while ( *bufPtr ) {
			if (*bufPtr == '\n' )
				*bufPtr = '\0';
			bufPtr++;
		}

		/* 
	 	 * Now write the registration packet to the master...  (inc NULL)
	 	 */
		verbose_txt("Writing request to master \n");
		if ( write(rmt_sd, Pkt, buffer_size ) < 0 ) {
			close(rmt_sd);
			nim_error( ERR_ERRNO_LOG, MSG_msg (MSG_SEND_TO_MASTER),
															MSG_msg (MSG_WRITE), NULL );
		}
	
		/* 
	 	 * Collect any remote status 
	 	 */
		if ( (nimFP=fopen(NIM_ENV_FILE, "a+")) == NULL )
			nim_error(ERR_ERRNO_LOG, MSG_msg (MSG_FOPEN), 
														MSG_msg (MSG_NEW_NIMINFO), NULL);
		do_cleanup++;
		verbose_txt("Building niminfo file \n");
		exec_status(rmt_sd, rmt_err, &rc, &nimFP, 0);
		niminfo.errno = rc;
	}
	else 
		rc=FAILURE; 

	if ( rc != 0 )
		nim_error(0, NULL, NULL, NULL); 

	fflush(nimFP);
	/* 
	 * Close the file and call a client method to add the routes
	 */ 
	fclose(nimFP); 	
	verbose_txt("Granting master rhost permission \n");
	sprintf(perms,"-a%s=%s", ATTR_PERMS_T, ATTR_DEFINE_T);
	sprintf(host, "-a%s=%s", ATTR_HOSTNAME_T, niminfo.master.hostname);
	if ( client_exec(&rc, NULL, rhost_args ) != SUCCESS )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning( 0, NULL, NULL, NULL );
/*		warning( ERR_FILE_MOD, "/.rhosts", NULL, NULL ); */

	/* 
	 * Call the c_add_routes method to read in the niminfo file and  
	 * add the routes. 
	 */
	verbose_txt("Adding routes from niminfo file \n");
	if ( client_exec(&rc, NULL, add_route_arg) != SUCCESS )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning(ERR_ADD_ROUTE, NULL, NULL, NULL);
	do_cleanup=0;
	verbose_txt("Adding entry to inittab \n");
	client_exec(&rc, NULL, mkitab_args );
	exit(0);
}

