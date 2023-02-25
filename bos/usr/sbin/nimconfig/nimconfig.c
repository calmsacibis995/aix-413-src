static char sccsid[] = "@(#)68  1.26.1.4  src/bos/usr/sbin/nimconfig/nimconfig.c, cmdnim, bos411, 9437A411a 9/10/94 17:26:59";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: cfg_exit
 *		check_input
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

#include "cmdnim_mstr.h"
#include "cmdnim_ip.h"
#include <sys/wait.h>
#include <varargs.h>

extern int	mstr_exit();
extern int	debug;

#define MKITAB_CMD "nim:2:wait:/usr/bin/startsrc -g nim >/dev/console 2>&1" 
#define MKITAB "/usr/sbin/mkitab" 
#define RMITAB "/usr/sbin/rmitab" 
#define RMSSYS "/usr/bin/rmssys" 


/*---------------------------- globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] = {
	{ ATTR_PIF_NAME,		ATTR_PIF_NAME_T,  	TRUE, 	NULL },
	{ ATTR_MASTER_PORT,	ATTR_MASTER_PORT_T,	TRUE, 	NULL },
	{ ATTR_NETNAME,		ATTR_NETNAME_T,  		TRUE, 	NULL },
	{ ATTR_PLATFORM,		ATTR_PLATFORM_T, 		FALSE, 	NULL },
	{ ATTR_RING_SPEED,	ATTR_RING_SPEED_T, 	FALSE, 	NULL },
	{ ATTR_CABLE_TYPE,	ATTR_CABLE_TYPE_T, 	FALSE, 	NULL },
	{ ATTR_FORCE,			ATTR_FORCE_T, 			FALSE, 	NULL },
	{ ATTR_VERBOSE,		ATTR_VERBOSE_T, 		FALSE, 	NULL },
	{ 0,		    			NULL, 					FALSE, 	NULL }
};


int	do_cleanup = FALSE;
char	net_name[MAX_TMP]; 
LIST	params;

/* --------------------------------------------------------------------*/


/* ---------------------------- verbose_txt 
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
 * ---------------------------------------------------------------------------*/


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

} /* end of verbose_txt */

/* --------------------------- bld_args 
 *
 * NAME: bld_args 
 *
 * FUNCTION: Takes args and adds them to the list.
 *
 * RETURNS: (int)
 *		SUCCESS	= list added to the list ok
 *		FAILURE	= Ops.. add to list error 
 *
 * ------------------------------------------------------------------- */ 

 int
bld_args(va_alist)
 va_dcl 
 {
	va_list	ptr; 
	LIST *lptr; 

	va_start(ptr); 

	lptr = va_arg(ptr, LIST *); 

	while (*ptr != NULL)
		if (add_to_LIST( lptr, va_arg(ptr, char *) ) !=SUCCESS)
			return(FAILURE);

	return(SUCCESS);

 } /* end of bld_args */

/* --------------------------- doUsage 
 *
 * NAME: doUsage 
 *
 * FUNCTION: Print out the useage message 
 *
 * NOTES : calls nene, cmd_what & exits with rc passed as parameter
 *
 * ------------------------------------------------------------------- */ 

void
doUsage(int rc)
{
        niminfo.errstr = MSG_msg(MSG_NIMCONFIG_SYNTAX);
        nene(0, NULL, NULL, NULL);
	cmd_what(valid_attrs);
	exit( rc );

} /* end of doUsage */

/* --------------------------- check_input
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
 * --------------------------------------------------------------------------*/

int
check_input()

{
	struct servent *serv;
	int	rc, va_indx, attr_indx;
	VALID_ATTR 	 * va_ptr;
	ATTR_ASS 	 * aa_ptr;

	/* 
	 * check for required attrs 
	 */
	for (va_indx = 0; valid_attrs[va_indx].pdattr; va_indx++) {
		va_ptr = &valid_attrs[va_indx];
		if ( va_ptr->required ) {
			for (attr_indx = 0; 
			    attr_indx < attr_ass.num; attr_indx++) {
				aa_ptr = attr_ass.list[attr_indx];
				if ( aa_ptr->pdattr == va_ptr->pdattr )
					break;
			}
			/* 
			 * is the required attr present? 
			 */
			if ( attr_indx == attr_ass.num ) 
			{
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

		case ATTR_NETNAME   :
			strncpy(net_name, aa_ptr->value, MAX_TMP); 
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
			/*
			 *  Verify port number is not used (also port number 
			 *  plus one. ) 
			 */
			niminfo.master_port = atoi( aa_ptr->value );
			if (((serv = getservbyport(niminfo.master_port, "tcp")) != NULL ) && 
			    (strcmp(serv->s_name, NIM_SERVICE ) != 0 )) {
				nim_error(ERR_ON_PORT, 
				    (char *)niminfo.master_port, 
				    serv->s_name, NULL);
			}
			if (((serv = getservbyport(niminfo.master_port + 1, "tcp")) != NULL ) && 
			    (strcmp(serv->s_name, NIM_REG_SERVICE ) != 0 )) {
				nim_error(ERR_ON_PORT, 
				    (char *)niminfo.master_port + 1, 
				    serv->s_name, NULL);
			}
			break;
		}
	}
} /* end of check_input */

/* --------------------------- rebuild_niminfo
 *
 * NAME: rebuild_niminfo
 *
 * FUNCTION:
 *	User want to rebuild the /etc/niminfo file. 
 *
 * NOTES:
 *		calls nim_error
 *
 * --------------------------------------------------------------------------*/

 void
 rebuild_niminfo()
 {
	NIM_OBJECT( master, minfo )	
	LIST	list;
	FILE	*fp;

	if ( access(NIM_ENV_FILE, R_ACC) == 0) 
		nim_error( ERR_EXISTS, NIM_ENV_FILE, NULL, NULL );

	if ( lag_object( 0, ATTR_MASTER_T, &master, &minfo ) == FAILURE )
		nim_error( ERR_DNE, ATTR_MASTER_T, NULL, NULL );

	/* generate a LIST of res_access structs */
	if ( LIST_res_access( master, &list ) == FAILURE )
		nim_error( 0, 0, NULL, NULL, NULL );

	if ( (fp=fopen(NIM_ENV_FILE, "w+")) == NULL )
		nim_error( ERR_FILE_ACCESS, NIM_ENV_FILE, NULL, NULL );

	/* make the niminfo file */
	if ( mk_niminfo( master, &list, fp ) != SUCCESS )
		nim_error( 0, 0, NULL, NULL, NULL );

	fclose( fp );

	exit(0);
 }

/* --------------------------- parse_args
 *
 * NAME: parse_args
 *
 * FUNCTION: parses command line arguments
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
 * ---------------------------------------------------------------------------*/

int
parse_args(	int argc, char *argv[] )

{
	extern char	*optarg;
	extern int	optind, optopt;
	int	c;


	/* loop through all args */
	while ( (c = getopt(argc, argv, "ra:v")) != -1 ) {
		switch (c) {

		/*
		 * Rebuild the /etc/niminfo file if possible
		 */
		case 'r':
			rebuild_niminfo();
			break; 

		/* 
		 * attribute assignment 
		 */
		case 'a':
			if (!parse_attr_ass(&attr_ass, valid_attrs, optarg, FALSE ) )
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
		 *  Tell user what we're doing. 
		 */
		case 'v':
			verbose++;
			printf("\n");
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
			doUsage (ERR_BAD_OPT);
			break;
		}
	}

	return( SUCCESS );

} /* end of parse_args */


/* --------------------------- cfg_exit
 *
 * NAME: cfg_exit
 *
 * FUNCTION:
 *		Special exit for this command.  This will remove the 
 *	/etc/niminfo file if do_cleanup != 0.
 *
 * DATA STRUCTURES:
 *		global: 
 *			niminfo.
 *
 * ---------------------------------------------------------------------- */

void
cfg_exit()
{
	ODMQUERY
	char	*src_args[] = { RMSSYS, "-snimesis", NULL };
	char	*srcD_args[] = { RMSSYS, "-snimd", NULL };
	char	*rmitab_args[] = { RMITAB, "nim", NULL };
	FILE	*FP=NULL; 
	int	rc; 

	/* 
	 * IFF we are in an error exit condition then remove the niminfo 
	 * file (this sez that the master package has not been successfully 
	 * configured). Also undo anything that we may have done. 
	 * Ignore any errors as we are unable to do anything abt it. 
	 * 
	 */
	if ( do_cleanup != 0 ) {
		unlink(NIM_ENV_FILE);
		/*
		 * Remove objects from database
		 */
		odm_rm_obj( nim_object_CLASS, NULL );
		odm_rm_obj( nim_attr_CLASS, NULL );

		/* stop nimesis and remove from SRC */
		client_exec(&rc, &FP, src_args);
		client_exec(&rc, &FP, srcD_args);

		/* remove inittab entry */
		client_exec(&rc, &FP, rmitab_args);
	}

	/* 
	 * Now do normal master exit processing
	 */
	mstr_exit();

} /* end of cfg_exit */


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
 * RETURNS: (int)
 *		SUCCESS	= setup the nim psuedo environment.
 *		FAILURE = Unable to setup.
 *
 * OUTPUT:
 *----------------------------------------------------------------------------*/

setup(int argc, char *argv[] )

{

								
	struct utsname uname_info;
	static char	base_name[MAX_TMP];
	char	arg_sc[MAX_TMP];
	char	arg_is[MAX_TMP];
	char	tmp[MAX_TMP];
	char	*ptr;
	int	rc; 			/* generic return code */
	int	i;
	FILE	 * out = NULL;
	struct servent *servPtr;
	char perm[MAX_TMP], host[MAX_TMP]; 
	char  *loclArgs[] =  { C_CH_RHOST, perm, host, NULL };

	setbuf( (FILE * ) stdout, NULL);
	setbuf( (FILE * ) stderr, NULL);
	/* 
	 * initialize the niminfo struct 
	 */
	memset( &niminfo, '\0', sizeof(struct nim_info ) );

   /* alloc space for errstr */
	niminfo.errstr = malloc( MAX_NIM_ERRSTR + 1 );


	/* 
	 * make sure that an exit routine is called upon exit 
	 */
	atexit( cfg_exit );

        /*
         * internationalization
         */
        setlocale( LC_ALL, "" );
	niminfo.msgcat_fd = catopen(MF_CMDNIM, NL_CAT_LOCALE);

	/* 
	 * Get Mr ODM up and mount the NIM classes. 
	 */
	odm_initialize();
	if (((nim_object_CLASS = odm_mount_class(NIM_OBJECT_CLASSNAME)) == -1) || 
	    nim_object_CLASS == NULL)
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	if (((nim_attr_CLASS = odm_mount_class(NIM_ATTR_CLASSNAME)) == -1) || 
	    nim_attr_CLASS == NULL)
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	if (((nim_pdattr_CLASS = odm_mount_class(NIM_PDATTR_CLASSNAME)) == -1) || 
	    nim_pdattr_CLASS == NULL)
		nim_error(ERR_ODM, (char *) odmerrno, NULL, NULL);

	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_CLASS) );
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
		add_attr( &attr_ass, ATTR_CPUID, ATTR_CPUID_T, 
		    uname_info.machine );
		strcpy( niminfo.cpu_id, uname_info.machine );
	}
	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_CPU) );

	/* 
	 * error policy controls behavior of nim_error 
	 */
	niminfo.err_policy = ERR_POLICY_DISPLAY;


	/* 
	 * load the regular expression arrays 
	 */
	for (i = 0; i < MAX_NIM_ERE; i++) {
		if ( (nimere[i].reg = malloc(sizeof(regex_t))) == NULL )
			ERROR( ERR_SYS, "malloc-", strerror(errno), NULL );
		if ( (rc = regcomp( nimere[i].reg, nimere[i].pattern, REG_EXTENDED)) != 0) {
			regerror( rc, nimere[i].reg, tmp, MAX_TMP );
			ERROR( ERR_SYS, "recomp", tmp, NULL );
		}
	}

	parse_args( argc, argv );

	do_cleanup = ( get_id(ATTR_MASTER_T) == 0L );
	if ( do_cleanup == 0 )
		ERROR( ERR_ALREADY_MASTER, NULL, NULL, NULL );
	
	/* 
	 * If the niminfo file exists check to make sure its not already
	 * a master... 
	 */
	if ( access(NIM_ENV_FILE, R_ACC) == 0) {
		if ( get_nim_env() == SUCCESS ) {
			if ( strcmp( niminfo.nim_type, ATTR_MASTER_T) == 0 )
				ERROR( ERR_ALREADY_MASTER, NULL, NULL, NULL );
			sprintf(perm, "-a%s=%s", ATTR_PERMS_T, ATTR_REMOVE_T );
			sprintf(host, "-a%s=%s", ATTR_HOSTNAME_T, niminfo.master.hostname );
			client_exec(&rc, NULL, loclArgs);
			unlink(NIM_ENV_FILE);

		}
		else
			ERROR( 0, NULL, NULL, NULL );
	}
	else if ( errno != ENOENT )
		ERROR( ERR_ERRNO_LOG, "set_up", NIM_ENV_FILE, NULL);


	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_OK_IP) );

	check_input();

	return(SUCCESS);

} /* end of setup */
	
/* --------------------------- undo              ------------------------------
 *
 * NAME: undo
 *
 * FUNCTION:
 *		backs out changes made by nimconfig when an error is detected
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *			error					= error number
 *			str1					= str1 of err msg
 *			str2					= str2 of err msg
 *			str3					= str3 of err msg
 *		global:
 *
 * RETURNS: (int)
 *
 * OUTPUT:
 * --------------------------------------------------------------------------
 */

int
undo(	int error_code,
		char *str1,
		char *str2,
		char *str3 )

{	
	if ( error_code > 0 )
		errstr( error_code, str1, str2, str3 );
	protect_errstr = TRUE;

	nim_error( 0, NULL, NULL, NULL );

} /* end of undo */

/* --------------------------- mk_master         
 *
 * NAME: mk_master
 *
 * FUNCTION:
 *		creates a machine object to represent the master
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= master object created
 *		FAILURE					= fatal error
 *
 * OUTPUT:
 * ---------------------------------------------------------------------------
 */

int
mk_master()

{
	long	m_id; 
	int plat;
	char *platform;
	NIM_PDATTR( plat_pd, pinfo )

	/* was platform specified? */
	if ( (plat = find_attr_ass( &attr_ass, ATTR_PLATFORM )) >= 0 )
	{
		platform = attr_ass.list[plat]->value;

		/* verify that specified platform is valid */
		if ( get_pdattr(  &plat_pd, &pinfo, ATTR_CLASS_MACHINES,
								ATTR_SUBCLASS_PLATFORM, 0, 0, platform ) <= 0 )
			nim_error( ERR_PLATFORM, platform, NULL, NULL );

		/* make sure platform type supports master configuration type */
		if ( ! attr_in_list( ATTR_MASTER, plat_pd->type ) )
			nim_error(  ERR_TYPE_PLATFORM, ATTR_MASTER_T, platform, NULL );
	}
	else
	{
		/* use default */
		if ( get_pdattr( &plat_pd, &pinfo, ATTR_CLASS_MACHINES, 0,
								ATTR_MASTER, ATTR_PLATFORM, NULL ) > 0 )
			platform = plat_pd->value;
		else
			platform = ATTR_RS6K_T;
	}

	/* create the master object */
	if (((m_id=mk_object(ATTR_MASTER_T,ATTR_MASTER_T, 0 )) == FAILURE)||
			(mk_attr(	m_id, NULL, "yes", 0, ATTR_RESERVED, 
							ATTR_RESERVED_T ) == FAILURE) ||
			(mk_attr(	m_id, NULL, platform, 0, ATTR_PLATFORM, 
							ATTR_PLATFORM_T ) == FAILURE) ||
			(mk_attr(	m_id, NULL, ATTR_BOOT_T, 0, ATTR_SERVES,
							ATTR_SERVES_T ) == FAILURE) ||
			(mk_attr(	m_id, NULL, ATTR_NIM_SCRIPT_T, 0, ATTR_SERVES,
							ATTR_SERVES_T ) == FAILURE) ||
			(mk_attr(	m_id, NULL, ATTR_msg(H_ATTR_MASTER), 0,
							ATTR_COMMENTS, ATTR_COMMENTS_T ) == FAILURE) )
		undo( 0, NULL, NULL, NULL );

	/* set its state */
	set_state( m_id, NULL, ATTR_MSTATE, STATE_RUNNING );

	/* check its definition */
	ck_mac_def( m_id, NULL );

	return( SUCCESS );

} /* end of mk_master */

/*---------------------------- mk_boot           ------------------------------
 *
 * NAME: mk_boot
 *
 * FUNCTION:
 *		creates a resource object to represent the boot resource
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= master object created
 *		FAILURE					= fatal error
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

int
mk_boot()

{	char	tmp[MAX_TMP];
	long	b_id; 

	sprintf( tmp, "%d", STATE_AVAILABLE );

	/* create the boot object */
	if (((b_id=mk_object( ATTR_BOOT_T, ATTR_BOOT_T, 0 ))== FAILURE)||
			(mk_attr(	b_id, NULL, "yes", 0, ATTR_RESERVED, 
							ATTR_RESERVED_T ) == FAILURE) ||
			(mk_attr(	b_id, NULL, ATTR_MASTER_T, 0, ATTR_SERVER,
							ATTR_SERVER_T ) == FAILURE) ||
			(mk_attr(	b_id, NULL, BOOT_LOCATION, 0, ATTR_LOCATION,
							ATTR_LOCATION_T ) == FAILURE) ||
			(mk_attr(	b_id, NULL, tmp, 0, ATTR_RSTATE,
							ATTR_RSTATE_T ) == FAILURE) ||
			(mk_attr(	b_id, NULL, "0", 0, ATTR_ALLOC_COUNT,
							ATTR_ALLOC_COUNT_T ) == FAILURE) ||
			(mk_attr(	b_id, NULL, ATTR_msg(H_ATTR_BOOT), 0,
							ATTR_COMMENTS, ATTR_COMMENTS_T ) == FAILURE) )
		undo( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of mk_boot */

/*---------------------------- mk_nim_script   ------------------------------
 *
 * NAME: mk_nim_script
 *
 * FUNCTION:
 *		creates a resource object to represent the nim_script resource
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *		sets errstr on failure
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *		parameters:
 *		global:
 *
 * RETURNS: (int)
 *		SUCCESS					= master object created
 *		FAILURE					= fatal error
 *
 * OUTPUT:
 *-----------------------------------------------------------------------------*/

int
mk_nim_script()

{	char tmp[MAX_TMP];
	long	s_id;

	sprintf( tmp, "%d", STATE_AVAILABLE );

	/* create the ns object */
	if (((s_id=mk_object( ATTR_NIM_SCRIPT_T, ATTR_NIM_SCRIPT_T,0)) == FAILURE) ||
			(mk_attr(	s_id, NULL, "yes", 0, ATTR_RESERVED, 
							ATTR_RESERVED_T ) == FAILURE) ||
			(mk_attr(	s_id, NULL, ATTR_MASTER_T, 0, ATTR_SERVER,
							ATTR_SERVER_T ) == FAILURE) ||
			(mk_attr(	s_id, NULL, NIM_SCRIPT_LOCATION, 0, 
							ATTR_LOCATION, ATTR_LOCATION_T ) == FAILURE) ||
			(mk_attr(	s_id, NULL, tmp, 0, ATTR_RSTATE,
							ATTR_RSTATE_T ) == FAILURE) ||
			(mk_attr(	s_id, NULL, "0", 0, ATTR_ALLOC_COUNT,
							ATTR_ALLOC_COUNT_T ) == FAILURE) ||
			(mk_attr(	s_id, NULL, ATTR_msg(H_ATTR_NIM_SCRIPT), 0,
							ATTR_COMMENTS, ATTR_COMMENTS_T ) == FAILURE) )
		undo( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of mk_nim_script */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{

	int	rc;
	int	is_token_ring=0, i, rmt_sd, rmt_err;
	int	is_ent=0;
	int	attr_indx, attr_cnt, buffer_size;
	ATTR_ASS 	 * aa_ptr;

	FILE	 *out=NULL, *nimFP;
	struct hostent *hent;
	char	tmp[512];
	char	Pkt[MAX_NIM_PKT];
	char	*ptr=NULL;

	int	options; 
	unsigned long	ipaddr; 
	char	master_pif[MAX_VALUE]; 
	char	net_type[MAX_VALUE]; 
	char	speed[MAX_TMP]; 
	char	cable[MAX_TMP];
	char	net_addr[MAX_TMP];
	char	*net_name, *nett;
	char	snm[MAX_TMP];
	char	master_port[MAX_TMP]; 

	char	*source=NULL;
	char	location[MAX_VALUE]; 
	char	*loc;
	char	src[MAX_VALUE]; 
	char	srvr[MAX_VALUE];

	char	*src_args[] = { MKSSYS, "-snimesis", "-p", NIMESIS, "-a", "-s", 
		"-i/dev/null", "-u0","-R", "-S", "-n15", "-f3", "-Gnim", NULL };

	char	*srcD_args[] = { MKSSYS, "-snimd", "-p", NIMESIS, "-a", "-sd", 
		"-i/dev/null", "-u0", "-R", "-S", "-n15", "-f3", NULL };

	char	*go_args[] = { STARTSRC, "-s", "nimesis", NULL	};

	char	*mkitab_args[] = { MKITAB, "-i", "rctcpip", MKITAB_CMD, NULL };

	/* examine SMIT_NO_EXEC env variable */
	smit_no_exec( argc, argv );

	/* become the process group leader so that locking inheritance works 
	 *		correctly 
	 */
	if ( setpgid( 0, getpid() ) == -1 )
		/* if we're envoked via motif smit we are already the 
		 * group leader , so if the error is EPERM is not really 
		 * an error... 
		 */
		if ( errno != EPERM )
			nim_error( ERR_ERRNO, NULL, NULL, NULL );

	/* 
	 * If setup finds an error it will cause the program 
	 * to exit... 
	 */
	if (setup(argc, argv) != SUCCESS)
		nim_error(0, NULL, NULL, NULL);
   errsig[SIGCHLD] = SIG_IGN;

	if ( get_list_space(&params, DEFAULT_CHUNK_SIZE, TRUE) == FALSE )
		nim_error(0, NULL, NULL, NULL);

	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_VALID) );

	/* override all state checking */
	force = TRUE;

	/* create object to represent the master */
	mk_master();

	/* create objects to represent the "boot" and "nim_script" resources */
	mk_boot();
	mk_nim_script();

	/* 
	 *  Create the niminfo file.
	 */
	if ( (nimFP = fopen(NIM_ENV_FILE, "w+")) == NULL )
		nim_error(ERR_ERRNO_LOG, "fopen", "New niminfo", NULL);
	fprintf(nimFP, "# %s\n", niminfo.cmd_name );
	fprintf(nimFP, "%s=%s\n", NIM_NAME, ATTR_MASTER_T);
	fprintf(nimFP, "%s=%s\n", NIM_CONFIGURATION, ATTR_MASTER_T);
	fprintf(nimFP, "%s=%d\n", NIM_MASTER_PORT, niminfo.master_port);

	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_NIMINFO) );

	/* 
	 * Resolve the ip address to a name for later use 
	 * in the PIF. 
	 */
	ipaddr = inet_addr(attr_value(&attr_ass, ATTR_NET_ADDR));

	verbose_txt("Attempting to resolve 0x%x\n", ipaddr);
	if ((hent = gethostbyaddr(&ipaddr, sizeof(ipaddr), AF_INET)) == NULL)
		nim_error( ERR_IP_RESOLVE, 
		    attr_value(&attr_ass, ATTR_NET_ADDR), "hostname", NULL);

	verbose_txt("      0x%x is known as %s\n", ipaddr, hent->h_name);
	fprintf(nimFP, "%s=%s\n", NIM_MASTER_HOSTNAME, hent->h_name);
	fclose(nimFP);

	/* 
	 * create the network object for the master. Use the 
	 * nim m_mknet method to do it. 
	 */

	sprintf( net_addr, "-a%s=%s", ATTR_NET_ADDR_T, 
	    attr_value(&attr_ass, ATTR_NET_ADDR));

	sprintf( snm, "-a%s=%s", ATTR_SNM_T, 
	    attr_value(&attr_ass, ATTR_SNM) );

	/* 
	 * need to know network type. 
	 */
	nett = attr_value(&attr_ass, ATTR_ADPT_NAME); 

	if (	(strncmp( nett, "en" , 2) == 0) ||
			(strncmp( nett, "et" , 2) == 0) )
	{
		sprintf( net_type, "-t%s", ATTR_ENT_T  );
		is_ent++;
	}
	else if (	( strncmp( nett, "to" , 2) == 0 ) ||
					( strncmp( nett, "tr" , 2) == 0 ) )
	{
		sprintf( net_type, "-t%s", ATTR_TOK_T  );
		is_token_ring++;
	}
	else if ( strncmp( nett, "fd" , 2) == 0 )
		sprintf( net_type, "-t%s", ATTR_FDDI_T  );

	/* if this is token ring, ATTR_RING_SPEED is required */
	if ( is_token_ring )
	{
		if (	((i = find_attr_ass(&attr_ass,ATTR_RING_SPEED)) < 0) ||
				(attr_ass.list[i]->value[0] == NULL_BYTE) )
			nim_error( ERR_MISSING_ATTR, ATTR_RING_SPEED_T, NULL, NULL );
		else
			sprintf( speed, "-a%s1=%s", ATTR_RING_SPEED_T,attr_ass.list[i]->value);
	}

	/* ATTR_CABLE_TYPE is required for ethernets */
	if ( is_ent )
	{
		if (	((i = find_attr_ass(&attr_ass,ATTR_CABLE_TYPE)) < 0) ||
				(attr_ass.list[i]->value[0] == NULL_BYTE) )
			nim_error( ERR_MISSING_ATTR, ATTR_CABLE_TYPE_T, NULL, NULL );
		else
			sprintf( cable, "-a%s1=%s", ATTR_CABLE_TYPE_T,attr_ass.list[i]->value);
	}

	bld_args( &params, M_MKNET, net_addr, snm, net_type, NULL );
	bld_args( &params,(net_name=attr_value(&attr_ass,ATTR_NETNAME)),NULL );
		
	if (verbose) 
		for ( i=0; params.list[i] != NULL; i++)
			verbose_txt("\t %d - %s \n", i, params.list[i]); 

	if ( client_exec(&rc, NULL, params.list) != SUCCESS )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, 0 );

	/* 
	 * Now build a primary interface (if1 attr) for the master and add it to the 
	 * data base via the m_chmaster method. NOTE that the 
	 * adapter name is missing from the PIF -- This is ok 
	 * as the master does not need to know this about it's 
	 * self... 
	 */
	reset_LIST( &params );
	sprintf( master_pif, "-a%s=%s %s %s", PIF, net_name, hent->h_name, 
	    attr_value(&attr_ass, ATTR_ADPT_ADDR));
	sprintf(master_port, "-a%s=%d", ATTR_MASTER_PORT_T, niminfo.master_port);
	bld_args( &params, M_CHMASTER, master_pif, master_port, NULL );

	if ( is_token_ring )
		bld_args( &params, speed, NULL );

	if ( is_ent )
		bld_args( &params, cable, NULL );

	bld_args( &params, ATTR_MASTER_T, NULL );

	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_CONNECT) );
	if (verbose) 
		for ( i=0; params.list[i] != NULL; i++)
			verbose_txt("\t %d - %s \n", i, params.list[i]); 

	if ( client_exec(&rc, NULL, params.list) != SUCCESS )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, 0 );

	/* inittab entry for nimesis */
	if ( client_exec(&rc, NULL, mkitab_args) != SUCCESS )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		nim_error( ERR_CMD, MKITAB, niminfo.errstr, 0 );

	/* 
	 * Add nimesis to the src database.
	 */
	verbose_txt("%s\n",  MSG_msg(MSG_NIMCONF_SRC) );
	if ( client_exec(&rc, NULL, src_args) != SUCCESS )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning( ERR_CMD, MKSSYS, niminfo.errstr, 0 );

	if ( client_exec(&rc, NULL, srcD_args) != SUCCESS )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning( ERR_CMD, MKSSYS, niminfo.errstr, 0 );

	if ( client_exec(&rc, NULL, go_args) != SUCCESS )
		warning( 0, NULL, NULL, NULL );
	else if ( rc > 0 )
		warning( ERR_CMD, STARTSRC, niminfo.errstr, 0 );

	do_cleanup = 0;

	exit(0);
}
