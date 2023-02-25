char    sccs_id[] = " @(#)50 1.24  src/bos/usr/sbin/nimclient/nimclient.c, cmdnim, bos411, 9437A411a  9/10/94  17:27:10";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: MySigs
 *		ch_push
 *		disable
 *		enable
 *		main
 *		resAlloc
 *		resList
 *		rootSinc
 *		state
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
#include "nimclient.h"

/*---------------------------- module globals    -----------------------------*/
VALID_ATTR valid_attrs[] =
{
	{0,                     NULL,                   FALSE,   NULL}
};

int (*func)() = NULL;
int disp_mntpnt = FALSE;


/*----------------------------- MySigs 
 *
 * NAME:	 MySigs
 *
 * FUNCTION:
 *	Perform set up for signals for nimclient
 *
 * EXECUTION ENVIRONMENT:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	SUCCESS	= Everything work ok
 *
 *----------------------------------------------------------------------------*/
MySigs()
{
	extern errsig_handler();

	errsig[SIGINT] = errsig_handler;
	errsig[SIGHUP] = errsig_handler;
	errsig[SIGALRM] = errsig_handler;
	errsig[SIGQUIT] = errsig_handler;
	errsig[SIGABRT] = errsig_handler;
	errsig[SIGSEGV] = errsig_handler;
	errsig[SIGSYS] = errsig_handler;
	errsig[SIGTERM] = errsig_handler;
	errsig[SIGCONT] = SIG_IGN;
	errsig[SIGPIPE] = SIG_IGN;

	return( SUCCESS );
}


/*----------------------------- STATE
 *
 * NAME: STATE
 *
 * FUNCTION:	Pass through the flags given to us to the 
 *		remote state update method
 *
 * DATA STRUCTURES:
 *		parameters:
 *			argc : count of args 
 *			argv : array of args 
 *
 * RETURNS:
 *		SUCCESS   -  Everything is "cool" 
 *		!SUCCESS  -  We bumped up against an error 8-(
 *
 *-----------------------------------------------------------------------------*/

STATE(int argc, char *argv[], int *rc )
{

	int	argcnt=0;

	FILE	*pipeFP; 

	char	cmd_str[80];
	char	date_str[30]; 
	char	tz_str[30];
	char	**Args;

	Args = (char **) nim_malloc( (argc + 1) * sizeof( char * ) );
	Args[0] = M_CHSTATE;
	for (argcnt=1; argcnt != argc; argcnt++)
		Args[argcnt] = argv[argcnt]; 
	Args[argcnt++] = niminfo.nim_name; 
	Args[argcnt++] = NULL;

	if (To_Master(ATTR_MASTER_T,rc, NULL , argcnt, Args) != SUCCESS )
		return(FAILURE);

	return(SUCCESS);
}

/* ---------------------------- resList
 *
 * NAME: resList
 *
 * FUNCTION:	Build a command line for lsnim for this nim client
 *		to return the resources that are available.
 *
 * DATA STRUCTURES:
 *		parameters:
 *
 * RETURNS:
 *		SUCCESS   -  Everything is "cool" 
 *		!SUCCESS  -  We bumped up against an error 8-(
 *
 * -------------------------------------------------------------------------*/

resList(int argc, char *argv[], int *rc)
{

	char 	*Args[METH_MAX_ARGS];
	FILE	*rmt_out; 
	int	argcnt; 

	rmt_out=(FILE *)stdout; 
	
	Args[0] = LSNIM; 

	/*
	 * copy the argv pointers
	 */ 
	for (argcnt=1; argcnt != argc; argcnt++)
		Args[argcnt] = argv[argcnt]; 
	/*
	 * Add our nim name
	 */ 
	Args[argcnt++] = niminfo.nim_name ; 

	return( To_Master(ATTR_MASTER_T, rc, &rmt_out, argcnt, Args) );
}

/* ---------------------------- resAlloc
 *
 * NAME: resAlloc
 *
 * FUNCTION:	Build a command line for lsnim for this nim client
 *		to return the resources that are allocated.
 *
 * DATA STRUCTURES:
 *		parameters:
 *
 * RETURNS:
 *		SUCCESS   -  Everything is "cool" 
 *		!SUCCESS  -  We bumped up against an error 8-(
 *
 * ------------------------------------------------------------------------*/

resAlloc(int argc, char *argv[], int *rc)
{
	char	class_res[MAX_TMP]; 
	char 	*Args[] = { LSNIM, class_res,  niminfo.nim_name, NULL };
	FILE	*rmt_out; 

	rmt_out=(FILE *)stdout; 
	sprintf(class_res, "-c%s", ATTR_CLASS_RESOURCES_T);
	return( To_Master(ATTR_MASTER_T, rc, &rmt_out, 3, Args) );
}


/*----------------------------- enable
 *
 * NAME: enable
 *
 * FUNCTION:	Enable the masters "pushabliity" on the client 
 *
 * RETURNS:
 *		SUCCESS   -  Everything is "cool" 
 *		!SUCCESS  -  We bumped up against an error 8-(
 *
 *-------------------------------------------------------------------------*/

enable( int argc, char *argv[], int *rc)
{
	char	perm[MAX_TMP]; 
	char	host[MAX_TMP]; 
	char	pullreq[MAX_TMP]; 

	char 	*mstrArgs[] =  { NIM, "-ochange", pullreq, perm, niminfo.nim_name, 
									NULL };
	char 	*loclArgs[] =  { C_CH_RHOST, perm, host, NULL };

	sprintf(perm, "-a%s=", ATTR_CONTROL_T );
	sprintf(pullreq, "-a%s=yep", ATTR_PULL_REQUEST_T );

   	if (To_Master(ATTR_MASTER_T, rc, NULL, 5, mstrArgs) != SUCCESS )
   		return(FAILURE);
   
   	if (*rc > 0)
       		return( FAILURE );
     	
	sprintf(perm, "-a%s=%s", ATTR_PERMS_T, ATTR_DEFINE_T );
	sprintf(host, "-a%s=%s", ATTR_HOSTNAME_T, niminfo.master.hostname );

	return( client_exec(rc, NULL, loclArgs) );
}


/*----------------------------- disable
 *
 * NAME: enable
 *
 * FUNCTION:	Enable the masters "pushabliity" on the client 
 *
 * RETURNS:
 *		SUCCESS   -  Everything is "cool" 
 *		!SUCCESS  -  We bumped up against an error 8-(
 *
 *-------------------------------------------------------------------------*/

disable( int argc, char *argv[], int *rc)

{
	char	perm[MAX_TMP]; 
	char	pullreq[MAX_TMP]; 

	char 	*mstrArgs[] =  { NIM, "-ochange", pullreq, perm, niminfo.nim_name, 
									NULL };

	sprintf(perm, "-a%s=%s push_off", ATTR_CONTROL_T, niminfo.nim_name );
	sprintf(pullreq, "-a%s=yep", ATTR_PULL_REQUEST_T );

   	if (To_Master(ATTR_MASTER_T, rc, NULL, 5, mstrArgs) != SUCCESS )
   		return(FAILURE);
   
   	if (*rc > 0)
       		return( FAILURE );

	return( SUCCESS );
}

/*---------------------------- exec_lsnim        ------------------------------
*
* NAME: exec_lsnim
*
* FUNCTION:
*		executes the lsnim command on the master, passing all the parameters
*			which appear after "-l"
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls error on failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			start					= first arg in argv to pass
*			end					= last arg in argv to pass
*			argv					= argv
*		global:
*
* RETURNS: (int)
*
* OUTPUT:
*		allows output from lsnim to be displayed directly
*-----------------------------------------------------------------------------*/

int
exec_lsnim(	int start,
				int end,
				char *argv[] )

{	int i;
	int rc;
	LIST params;

	/* initialize the LIST */
	if ( get_list_space( &params, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* first parameter is LSNIM */
	if ( add_to_LIST( &params, LSNIM ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* add in parameters the user specified after the "-l" option */
	for (i=start; i < end; i++)
		if ( add_to_LIST( &params, argv[i] ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );

	/* execute LSNIM on the master */
	if (	(To_Master(	ATTR_MASTER_T, &rc, NULL, params.num, 
							&params.list[0] ) == FAILURE) || (rc > 0) )
		nim_error( 0, NULL, NULL, NULL );

	/* all done - no errors */
	exit( 0 );

} /* end of exec_lsnim */

/*---------------------------- get_usage_str   ------------------------------
*
* NAME: get_usage_str
*
* FUNCTION:     Constructs a usage msg from various independent components.
*
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*               mallocs storage for string being returned.
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*                       out_buf         = ptr to storage containging usage msg
*
* RETURNS:
*                       pointer to usage msg
*
* OUTPUT:    
*-----------------------------------------------------------------------------*/

char *
get_usage_str ()
{
	char * out_buf;
	out_buf = (char *) malloc (
			strlen ((char *)MSG_msg (MSG_NIMCLIENT_SYNTAX)) +
			strlen ((char *)MSG_msg (MSG_VALID_OPS)) +
                        strlen (NIMCLIENT_VAL_OPS_STR) +
                        strlen ((char *) MSG_msg (MSG_USG_HINT1_CL)) +
                        strlen ((char *) MSG_msg (MSG_USG_HINT2_CL)) + 
			4 /* this includes the whitespace */ );
	sprintf (out_buf, "%s\n%s%s\n%s\n%s", MSG_msg (MSG_NIMCLIENT_SYNTAX),
					MSG_msg (MSG_VALID_OPS),
                                        NIMCLIENT_VAL_OPS_STR,
					MSG_msg (MSG_USG_HINT1_CL),
					MSG_msg (MSG_USG_HINT2_CL));
	return (out_buf);

} /* end of get_usage_str */

/*---------------------------- parse_args        ------------------------------
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
*			argc			= argc from main
*			argv			= argv from main
*		global:
*
* RETURNS: (int)
*		SUCCESS					= no syntax errors on command line
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
parse_args(	int argc, 
				char *argv[] )

{	extern char *optarg;
	extern int optind, optopt;
	int c;
	char tmp[MAX_TMP];
	int attrs = FALSE;
	int subclass = FALSE;
	int type = FALSE;
	int syntax_error = FALSE;

	if (argc == 1)
		nim_error_plus( ERR_MISSING_OPT, 'o', NULL, NULL, get_usage_str());

	/* loop through all args */

	while ( (c = getopt(argc, argv, "la:dFo:pPR:S:")) != -1 )
	{	switch (c)
		{	
			case 'l': /* execute lsnim on the master */
				exec_lsnim( optind, argc, argv );
			break;

			case 'a': /* attrs associated with operation or state */
				if (	((func != NULL) && (func != do_nim) && (func != STATE)) ||
						(subclass) || (type) )
					syntax_error = TRUE;
				attrs = TRUE;

				/* is this ATTR_DISP_MNTPNT? */
				disp_mntpnt = (strstr( optarg, ATTR_DISP_MNTPNT_T ) != NULL );
			break;

			case 'd': /* set date to master's */
				if ( (func != NULL) || (attrs) || (subclass) || (type) )
					syntax_error = TRUE;
				func = set_date;
			break;

			case 'F': /* Set the force bit */
				force++; 
			break;
				
			case 'o': /* execute NIM operation */
				if ( (func != NULL) || (subclass) || (type) )
					syntax_error = TRUE;
				func = do_nim;
			break;
				
			case 'p': /* enable master's push permissions */
				if ( (func != NULL) || (attrs) || (subclass) || (type) )
					syntax_error = TRUE;
				func = enable;
			break;
				
			case 'P': /* disable master's push permissions */
				if ( (func != NULL) || (attrs) || (subclass) || (type) )
					syntax_error = TRUE;
				func = disable;
			break;
				
			case 'R': /* pass RESULT back to master */
				if ( (func != NULL) || (attrs) || (subclass) || (type) )
					syntax_error = TRUE;
				func = STATE;
			break;
				
			case 'S': /* pass STATE back to master */
				if ( (func != NULL) || (subclass) || (type) )
					syntax_error = TRUE;
				func = STATE;
			break;
				
			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				niminfo.errstr = get_usage_str();
				nene( 0, NULL, NULL, NULL);
				exit( ERR_BAD_OPT );
			break;
		}

		if ( syntax_error )
			break;
	}
	syntax_error+=( optind != argc );
	if ( (syntax_error) || (func == NULL) )
		nim_error_plus ( ERR_SYNTAX, NULL, NULL, NULL, get_usage_str());

	/* return */
	return( SUCCESS );

} /* end of parse_args */

/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	int	parmNdx;
	int	rc=0; 

	/* examine SMIT_NO_EXEC env variable */
	smit_no_exec( argc, argv );

	/* 
	 * Do NIM setup stuff.
	 */
	nim_init( argv[0], ERR_POLICY_FOREGROUND, MySigs, NULL );
	
	/* 
	 * Only a client can execute nimclient, if the user attempts 
	 * to execute this from the master, slapped wrists are in 
	 * order... 
	 */
	if ( strcmp(niminfo.nim_type, ATTR_MASTER_T) == 0 )
		nim_error(ERR_IS_MASTER, NULL, NULL, NULL);

	/* 
	 * If parse_cmdline finds an error it will exit.
	 */
	parse_args( argc, argv );

	/* 
	 * Now that we have all the options validated let the function rip
	 * it is the functions responsiblity to check the exec status of 
	 * and local or remote command executed. We expect nim_error to 
	 * be called if an error is detected. NIM_ERROR will exit !!
	 */
	niminfo.errno = 0; 
	(*func)(argc, argv, &niminfo.errno);
	if ( niminfo.errno > 0 ) 
		nim_error(ERR_METHOD, ATTR_MASTER_T, niminfo.errstr, NULL);
	
	exit(0);
}

