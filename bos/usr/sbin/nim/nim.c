static char     sccs_id[] = " @(#)43 1.25  src/bos/usr/sbin/nim/nim.c, cmdnim, bos411, 9439C411e  9/30/94  14:06:44";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/nim.c
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

#include "cmdnim_mstr.h"
#include <search.h> /* for a call to qsort */

extern int valid_pdattr_ass();
extern int ch_pdattr_ass();

/* currently, NIM is not allowing scheduled operations */
#define ALLOW_SCHEDULING	0

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

VALID_ATTR valid_attrs[] =
{	
	{0,							NULL, 						FALSE, ch_pdattr_ass }
};

char *name=NULL;								/* NIM name of object to create */
int type=0;										/* object's type */
int nim_pull_op=FALSE;
	/* set if pull_request was specified on nim cmd line */
char operation[MAX_TMP];					/* operation to perform */
char at[MAX_TMP];								/* time to perform operation at */
NIM_OBJECT( obj, info )						/* object's NIM info */
LIST meth_args;								/* args for method */

/* Used for usage msg generation. */
#define BUF_MAX  	1024
#define MAX_COL        55

/*---------------------------- comparision_function ---------------------------
*
* NAME: comparison_function
*
* FUNCTION: 	Returns result of strcmp of two parameters passed.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		Exists purely for qsort's purposes.
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*
* RETURNS: 
*		
*
* OUTPUT:     
*-----------------------------------------------------------------------------*/
static int
comparison_function
(
const void * first_arg,
const void * second_arg
)
{
  struct nim_pdattr * first_el;
  struct nim_pdattr * second_el;

  first_el = ( (struct nim_pdattr *) first_arg);
  second_el = ( (struct nim_pdattr *) second_arg);

  return (strcmp (first_el->name, second_el->name));

} /* end of comparison_function */

/*---------------------------- get_pdattr_str ---------------------------
*
* NAME: get_pdattr_str  (get predefined attributes string)
*
* FUNCTION: 	Constructs a string comprised of a sorted list of unique 
*		nim objects defined by the attribute id passed as a parameter.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls odm_get_list to search for objects classified by the
*		 attribute in question.
*		sorts the output with qsort, relies on comparison_function()
*		 for sort criteria;
*		mallocs storage for the list which is constructed;
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*	local:
*		out_buf         = ptr to storage containing constructed string
*		list		= points to the pdattr list returned from 
*				  odm_get_list
*		info		= info about list returned by odm_get_list
*
*		offset		= used to keep track of position of last 
*				  character on line being constructed.
*		name_len 	= length of attr name being added to list 
*		name_end_len	= length of string being added to the end of
*				  a name (to provide tabs and/or comma sep.)
*		query		= buffer for query of pdattr database
*
*	global:
*		BUF_MAX		= used to determine when to realloc buffer
*		MAX_COL         = used to determine when to start a new line
*
*	parameters:
*		attr_id		= id for attribute being searched
*		header		= header string to prepend to output
*		prefix		= 2ndary hdr to prepend to output
*		trailer		= string to append to output string
*			
*
* RETURNS: 
*		pointer to string of predefined attributes
*
* OUTPUT:     
*-----------------------------------------------------------------------------*/
char *
get_pdattr_str (int attr_id, char * header, char * prefix, char * trailer)
{
	char * out_buf;
	char * name_ender;
	int offset;
	int name_len;
	int name_end_len;
	int i;
	char query[MAX_CRITELEM_LEN];
        NIM_PDATTR( list, info )

	/*
	 * Determine the appropriate query.
	 */
	switch (attr_id)
	{
		case ATTR_SUBCLASS_OPERATION:
			sprintf (query, "subclass like '*,%d,*'", 
				ATTR_SUBCLASS_OPERATION);
			break;

		case ATTR_CLASS_MACHINES:
        		sprintf (query, "class = %d and subclass like '*,%d,*'",
				 ATTR_CLASS_MACHINES, ATTR_SUBCLASS_TYPE);
			break;

		case ATTR_CLASS_NETWORKS:
        		sprintf (query, "class = %d and subclass like '*,%d,*'",
				ATTR_CLASS_NETWORKS, ATTR_SUBCLASS_TYPE);
			break;

		case ATTR_CLASS_RESOURCES:
        		sprintf (query, "class = %d and subclass like '*,%d,*'",
				ATTR_CLASS_RESOURCES, ATTR_SUBCLASS_TYPE);
			break;
	}

	/*
	 * Get a linked list of attributes matching the query.
	 */
        if ( (int) (list = (struct nim_pdattr *)
                odm_get_list( nim_pdattr_CLASS, query, &info, 50, 1 )) == -1 )
                return (NULL_BYTE);

        qsort (list, info.num, sizeof (struct nim_pdattr),
               comparison_function);

	/*
	 * Loop through the linked list concatenating unique attribute names
	 * to the output string.
	 */
	out_buf = (char *) malloc (BUF_MAX + 1);
	sprintf (out_buf, "%s\t\t%s", header, prefix);
	offset = strlen (prefix) + 2;
	for (i=0; i < info.num; i++)
		if ((i == 0) || (strcmp (list[i].name, list[i-1].name) != 0))
		{
			name_len = strlen (list[i].name);
			if ((name_len + offset) > MAX_COL)
			{
				name_ender = ",\n\t\t";
				offset = 2;
			}
			else
				name_ender = ", ";
			name_end_len = strlen (name_ender);	
			if ((strlen (out_buf) + name_len + name_end_len) >= BUF_MAX)
				out_buf = (char *) realloc (out_buf, strlen (out_buf) + BUF_MAX);
			if (i == 0)
			{
				strcat (out_buf, list[i].name);
				offset += name_len;
			}
			else
			{
				strcat (out_buf, name_ender);
				strcat (out_buf, list[i].name);
				offset += (name_len + name_end_len);
			}
		}
	if ((strlen (out_buf) + strlen (trailer))  >= BUF_MAX)
		out_buf = (char *) realloc (out_buf, strlen (out_buf) + BUF_MAX);
	strcat (out_buf, trailer);
	return (out_buf);

} /* end of get_pdattr_str */

/*---------------------------- get_valid_ops_str ---------------------------
*
* NAME: get_valid_ops_str
*
* FUNCTION: 	Constructs a string comprised of a dynamically created
*		list of valid NIM operations. 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls get_pdattr_str to look up predefined info
*		!!!     When called via nimclient, returns a static	!!!
*		!!! 	predefined list.				!!!
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES: 
*
* RETURNS: 
*		pointer to ops string
*
* OUTPUT:     
*-----------------------------------------------------------------------------*/
char *
get_valid_ops_str ()
{

	if (nim_pull_op)	
	{
		char * tmpstr;

		tmpstr = (char *) malloc (
				strlen((char *) MSG_msg(MSG_VALID_OPS)) +
				strlen(NIMCLIENT_VAL_OPS_STR));
		sprintf (tmpstr, "%s%s", 
				(char *) MSG_msg (MSG_VALID_OPS), 
				NIMCLIENT_VAL_OPS_STR);
		return (tmpstr);
	}
	else
		return (get_pdattr_str (ATTR_SUBCLASS_OPERATION, 
			(char *) MSG_msg (MSG_VALID_OPS), "","\n"));

} /* end of get_valid_ops_str */

/*---------------------------- get_valid_types_str ---------------------------
*
* NAME: get_valid_types_str
*
* FUNCTION: 	Constructs a string comprised of a dynamically created list 
*		of predefined machine, network and resource types.
*		
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls get_pdattr_str to look up predefined info.
*		mallocs storage for string being returned
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES: 
*		out_buf         = ptr to storage containging msg
*		mach, net, res	= tmp ptrs to machine, network, 
*				  and resource strings (resp)
*
* RETURNS: 
*		pointer to "types" string
*
* OUTPUT:     
*-----------------------------------------------------------------------------*/
char *
get_valid_types_str ()
{
	char * mach, * net, * res, * out_buf;

        /*
         * get list of predefined machines
         */
	mach = get_pdattr_str (ATTR_CLASS_MACHINES, 
			(char *) MSG_msg (MSG_VALID_TYPES), "machines: ", "\n");
	
        /*
         * get list of predefined networks
         */
	net = get_pdattr_str (ATTR_CLASS_NETWORKS, "", "networks: ", "\n");
	
        /*
         * get list of predefined resources
         */
	res = get_pdattr_str (ATTR_CLASS_RESOURCES, "", "resources: ", "\n");

        /*
         * combine the lists constructed above
         */
	out_buf = (char *) malloc (strlen (mach) + strlen (net) + strlen (res));
	sprintf (out_buf, "%s%s%s", mach, net, res);
	return (out_buf);

} /* end of get_valid_types_str */

/*---------------------------- get_usage_str   ------------------------------
*
* NAME: get_usage_str
*
* FUNCTION: 	Constructs a usage msg from various independent components.
*		
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls get_valid_ops_str(), get_valid_types_str()
*		mallocs storage for string being returned.
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES: 
*			out_buf         = ptr to storage containging usage msg
*			ops             = ptr to string of valid ops
*			types           = ptr to string of valid types
*
* RETURNS: 
*			pointer to usage msg
*
* OUTPUT:     
*-----------------------------------------------------------------------------*/
char *
get_usage_str ()
{
	char * out_buf;
	char * ops;
	char * types;

	ops = get_valid_ops_str();
	if (! nim_pull_op)	
	{
		types = get_valid_types_str();
		out_buf = (char *) malloc (
				strlen ((char *)MSG_msg (MSG_NIM_SYNTAX)) +
				strlen (ops) +
				strlen (types) +
				strlen ((char *) MSG_msg (MSG_USG_HINT1)) +
				strlen ((char *) MSG_msg (MSG_USG_HINT2)) + 
				5 /* includes whitespace */ );
		sprintf (out_buf, "%s\n%s\n%s\n%s\n%s", 
				MSG_msg (MSG_NIM_SYNTAX),
				ops,	
				types,
				(char *) MSG_msg (MSG_USG_HINT1),
				(char *) MSG_msg (MSG_USG_HINT2));
	}
	else
	{
		out_buf = (char *) malloc (
				strlen ((char *)MSG_msg (MSG_NIMCLIENT_SYNTAX))+
				strlen (ops) +
				strlen ((char *) MSG_msg (MSG_USG_HINT1_CL)) +
				strlen ((char *) MSG_msg (MSG_USG_HINT2_CL)) + 
				5 /* includes whitespace */ );

		sprintf (out_buf, "%s\n%s\n%s\n%s", 
				MSG_msg (MSG_NIMCLIENT_SYNTAX),
				ops,	
				(char *) MSG_msg (MSG_USG_HINT1_CL),
				(char *) MSG_msg (MSG_USG_HINT2_CL));
	}
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
	int obj_type=0;
	int rc;
	NIM_PDATTR( op, opinfo )
	char tmp[MAX_TMP];

	/* loop through all args */
#if ALLOW_SCHEDULING
	while ( (c = getopt(argc, argv, "a:t:o:FS:v")) != -1 )
#else
	while ( (c = getopt(argc, argv, "a:t:o:Fv")) != -1 )
#endif
	{	switch (c)
		{	
			case 'a': /* attrs for the method */
				if (	(! parse_attr_ass( &attr_ass, valid_attrs, optarg, FALSE )) ||
						(add_to_LIST( &meth_args, "-a" ) == FAILURE) ||
						(add_to_LIST( &meth_args, optarg ) == FAILURE) )
				{
					if (niminfo.errno == ERR_ATTR_NAME ||
						niminfo.errno == ERR_NO_CH_ATTR)
						nim_error_plus (
						0, NULL, NULL, NULL,  
						nim_pull_op ? 
						(char *) MSG_msg (MSG_USG_HINT2_CL) :
						(char *) MSG_msg (MSG_USG_HINT2));
					else
						if (niminfo.errno == ERR_VALUE)
							nim_error_plus (
							0, NULL, NULL, NULL,
							get_usage_str());
						else
							nim_error( 
							0, NULL, NULL, NULL );
				}
				if ( strcmp(attr_ass.list[0]->name, ATTR_PULL_REQUEST_T) == 0 )
					nim_pull_op=TRUE;
			break;
				
			case 'F': /* force flag - add an ATTR_FORCE attr */
				sprintf( tmp, "-a%s=yes", ATTR_FORCE_T );
				if ( add_to_LIST( &meth_args, tmp ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
				force = TRUE;
				break;

			case 'v':
				if ( add_to_LIST( &meth_args, "-v" ) == FAILURE )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case 't': /* object type */
				if ( type > 0 )
					nim_error( ERR_SYNTAX, MSG_msg(MSG_NIM_SYNTAX), NULL, NULL );
				else if ( (type = type_s2i( optarg )) <= 0 )
					nim_error_plus(	ERR_VALUE, optarg, MSG_msg(MSG_NIM_TYPE), NULL, get_valid_types_str());

				if (	(add_to_LIST( &meth_args, "-t" ) == FAILURE) ||
						(add_to_LIST( &meth_args, optarg ) == FAILURE) )
					nim_error( 0, NULL, NULL, NULL );
				break;

			case 'o': /* operation */
				if ( operation[0] != NULL_BYTE )
					nim_error( ERR_SYNTAX, MSG_msg(MSG_NIM_SYNTAX), NULL, NULL );
				else if ( get_pdattr(	&op, &opinfo, 0, ATTR_SUBCLASS_OPERATION, 
												0, 0, optarg ) <= 0 )
					nim_error_plus(	ERR_VALUE, optarg, MSG_msg(MSG_NIM_OPERATION), NULL, get_valid_ops_str());
				strncpy( operation, optarg, MAX_TMP );
				break;

			case 'S': /* scheduled */
				strncpy( at, optarg, MAX_TMP );
				break;

			case ':': /* option is missing a required argument */
				nim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
				break;

			case '?': /* unknown option */
				/* getopt printed the detailed message, now
				   do usage */
				niminfo.errstr = get_usage_str();
				nene( 0, NULL, NULL, NULL);
				exit( ERR_BAD_OPT );
				break;
		}
	}


	/* check for errors */

	/* required is: */
	/* 	- object name (the only operand allowed) */
	/*		- operation */
	if ( operation[0] == NULL_BYTE )
		nim_error_plus( ERR_MISSING_OPT, 'o', NULL, NULL, get_usage_str());
	else if ( optind == argc )
		nim_error_plus( ERR_MISSING_OPERAND, MSG_msg(MSG_OBJ_NAME), NULL, NULL, get_usage_str() );
	else if ( optind < (argc-1) )
		nim_error( ERR_SYNTAX, MSG_msg(MSG_NIM_SYNTAX), NULL, NULL );

	name = argv[optind];

	if ( add_to_LIST( &meth_args, name ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* "-t" can also be required, but its context specific */
	if ( op->attr == ATTR_DEFINE )
	{  /* "-t" is required */
		if ( type == 0 )
			nim_error_plus( ERR_MISSING_OPT, 't', NULL, NULL, MSG_msg(MSG_NIM_SYNTAX) );

		/* "-S" not allowed */
		if ( at[0] != NULL_BYTE )
			nim_error( ERR_CONTEXT, "-S", NULL, NULL );
	}
	else 
	{	/* object must already exist, so "-t" not allowed */
		if ( type > 0 )
			nim_error_plus( ERR_CONTEXT, "-t", NULL, NULL, MSG_msg(MSG_NIM_SYNTAX) );

		/* get the object's info */
		if ( lag_object( 0, name, &obj, &info ) == FAILURE )
			nim_error( 0, NULL, NULL, NULL );
		type = obj->type->attr;
	}

	/* return */
	return( SUCCESS );

} /* end of parse_args */
	
/*---------------------------- do_ops            ------------------------------
*
* NAME: do_ops
*
* FUNCTION:
*		performs the operations specified by the user
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
*		global:
*			obj
*			at
*			meth_args
*
* RETURNS: (int)
*		SUCCESS					= all operations successful
*		FAILURE					= error during operation
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
do_ops()

{	int rc;
 	int rc2;
	NIM_PDATTR( op, opinfo )
	NIM_PDATTR( sop, sopinfo )
	int meth_to_exec = 3;

	/* make sure operation is supported for specified type */
	if ( get_pdattr(	&op, &opinfo, 0, ATTR_SUBCLASS_OPERATION, type,
							0, operation ) <= 0 )
		nim_error_plus (	ERR_OP_NOT_ALLOWED, 
					operation, 
					ATTR_msg(type), 
					NULL, 
					nim_pull_op ? 
					(char *) MSG_msg (MSG_USG_HINT1_CL) : 
					(char *) MSG_msg (MSG_USG_HINT1));

	/* full pathname of method */
	meth_args.list[ meth_to_exec ] = op->value;

	/* execute now or schedule? */
	if ( at[0] != NULL_BYTE )
	{	/* make sure schedule operation supported for this type */
		if ( get_pdattr(	&sop, &sopinfo, 0, ATTR_SUBCLASS_OPERATION, type,
								ATTR_SCHEDULE, NULL ) <= 0 )
			nim_error(	ERR_OP_NOT_ALLOWED, ATTR_msg(ATTR_SCHEDULE), 
							ATTR_msg(type), NULL );

		/* initialize the rest of the param LIST */
		meth_args.list[0] = sop->value;
		meth_args.list[1] = at;
		meth_args.list[2] = ATTR_MASTER_T;
		meth_to_exec = 0;
	}

	/* execute the method now */
	if ( client_exec( &rc, NULL, &meth_args.list[ meth_to_exec ] ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );
	else if ( rc == ERR_INVALID_ATTR )
		nim_error_plus(	ERR_METHOD, 
				niminfo.nim_name, 
				niminfo.errstr, 
				NULL, 
				nim_pull_op ? 
				(char *) MSG_msg (MSG_USG_HINT2_CL) : 
				(char *) MSG_msg (MSG_USG_HINT2) );
	else if (rc > 0)
		nim_error( ERR_METHOD, niminfo.nim_name, niminfo.errstr, NULL );

	return( SUCCESS );

} /* end of do_ops */
	
/**************************       main         ********************************/

main( int argc, char *argv[] )

{
	/* examine SMIT_NO_EXEC env variable */
	smit_no_exec( argc, argv );

	/* become the process group leader so that locking inheritance works */
	/*		correctly */
	if ( setpgid( 0, getpid() ) == -1 )
		if ( errno != EPERM )
			nim_error( ERR_ERRNO, NULL, NULL, NULL );

	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_COMMAND, NULL );
	if (	(get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE) ||
			(get_list_space( &meth_args, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE) )
		nim_error( 0, NULL, NULL, NULL );

	/* initialize globals */
	obj = NULL;
	operation[0] = NULL_BYTE;
	at[0] = NULL_BYTE;

	/* initialize method arg LIST */
	if ( get_list_space( &meth_args, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		nim_error( 0, NULL, NULL, NULL );

	/* reserve the first 4 entries in meth_args for: */
	/*		0 = schedule method */
	/*		1 = time to execute (for scheduled operations) */
	/*		2 = target to execute the method on */
	/*		3 = method to execute */
	meth_args.num = 4;

	/* parse command line */
	parse_args( argc, argv );

	/* check for control */
	if (	(obj != NULL) && (! force) )
		check_control(obj);

	/* do the requested operation */
	do_ops();

	exit( 0 );

}
