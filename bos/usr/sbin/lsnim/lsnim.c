static char     sccs_id[] = " @(#)42 1.42  src/bos/usr/sbin/lsnim/lsnim.c, cmdnim, bos41J, 9513A_all  3/27/95  16:39:41";

/*
 *   COMPONENT_NAME: CMDNIM
 *
 *   FUNCTIONS: ./usr/sbin/lsnim/lsnim.c
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

/*---------------------------- ways to display data --------------------------*/
#define DISPLAY_MISSING			0x0001
#define DISPLAY_OP_QUERY		0x0002
#define DISPLAY_ATTR				0x0004
#define DISPLAY_LONG				0x0008
#define DISPLAY_PULL				0x0010
#define DISPLAY_OPS				0x0020
#define DISPLAY_HELPS			0x0040
#define DISPLAY_COLON			0x0080
#define DISPLAY_PREDEFINED		0x0100
#define DISPLAY_RELATION		0x0200
#define DISPLAY_SUBCLASSES		0x0400
#define DISPLAY_CONTROL_OPS	0x0800

/*---------------------------- module globals    -----------------------------*/
ATTR_ASS_LIST attr_ass;

char *class=NULL;										/* class name */
int class_num=0;										/* class num */
char *subclass=NULL;									/* subclass name */
int subclass_num=0;									/* subclass num */
char *type=NULL;										/* type name */
int type_num=0;										/* type num */
char *operation=NULL;								/* operation name */
int specific_attr=0;										/* integer rep of attribute */
int display_mask=0;									/* display mask */
int (*anchor)()=NULL;								/* ptr to display function */
FILE *output=stdout;									/* ptr to output stream */
int override_mask=FALSE;							/* force display of all attrs */
int printed_title=TRUE;								/* TRUE if title needs printing */
char *obj_name=NULL;									/* object name for title */
int aligned_yet=FALSE;								/* TRUE if align_output called */
int piped_output=FALSE;								/* TRUE if popen was called */
LIST objects;											/* LIST of object names */

/* arrays of SMIT tags for NIM interface info */
char *IF_TAGS[] = { "if_net", "if_hostname", "if_haddr", "if_adpt" };
char *ROUTING_TAGS[] = { "dest", "gateway" };

/*---------------------------- lsnim_error       ------------------------------
*
* NAME: lsnim_error
*
* FUNCTION:
*		invokes nim_error, but closes any open pipes before that happens (or
*			else we'll hang in nim_exit waiting for the pipe to terminate)
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
*			errno					= error code
*			str1					= str1 of error msg
*			str2					= str2 of error msg
*			str3					= str3 of error msg
*		global:
*
* RETURNS: (int)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
lsnim_error(	int errno,
					char *str1,
					char *str2,
					char *str3 )

{

	if ( piped_output )
		pclose( output );

	nim_error( errno, str1, str2, str3 );

} /* end of lsnim_error */

/*******************************************************************************
******************* list manipulation routines            **********************
*******************************************************************************/

/*---------------------------- list_add          ------------------------------
*
* NAME: list_add
*
* FUNCTION:
*		adds the specified <newname> to the <names> list
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			names					= ptr to LIST
*			newname				= name to add
*		global:
*
* RETURNS: (int)
*		SUCCESS					= <newname> added (or was already there)
*		FAILURE					= malloc error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
list_add(	LIST *names,
				char *newname )

{	int k;
	int unique=TRUE;

	/* check for uniqueness */
	for (k=0; k < names->num; k++)
		if (! strcmp( newname, names->list[k] ) )
		{	unique = FALSE;
			break;
		}
	if (! unique )
		return( SUCCESS );

	/* make sure there's enough space */
	if (! get_list_space( names, 1, FALSE ) )
		return( FAILURE );

	/* add this resource into the list */
	if ( (names->list[ names->num ] = (char *) 
									malloc( strlen( newname ) + 1 )) == NULL )
		return( FAILURE );

	strcpy( names->list[ (names->num)++ ], newname );

	return( SUCCESS );

} /* end of list_add */

/*---------------------------- list_res_on_net   ------------------------------
*
* NAME: list_res_on_net
*
* FUNCTION:
*		returns a list of network names which can communicate with the specified
*			net
*		the potential entries in the list may be filtered by either <subclass>,
*			<type>, or <attr>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			names					= ptr to LIST
*			netname				= name of source net
*			subclass				= pdattr.subclass to use as a filter
*			type					= pdattr.type to use as a filter
*			attr					= pdattr.attr to use as a filter
*		global:
*
* RETURNS: (LIST *)
*		SUCCESS					= names added
*		FAILURE					= no objects found
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
list_res_on_net(	LIST *names,
						char *netname,
						int subclass,
						int type,
						int attr )

{	char query[MAX_CRITELEM_LEN];
	int i,j,k;
	int unique;
	struct nim_attr *netref;
	struct listinfo ninfo;
	struct nim_attr *res;
	struct listinfo rinfo;
	int filter_output;
	struct nim_object obj;
	struct nim_attr rstate;
	struct nim_pdattr pdattr;
	char available_state[10];

	sprintf( available_state, "%d", STATE_AVAILABLE );

	/* the goal here is to find all the machines which are connected to the */
	/*		 specified net and serve a resource */

	/* get all the ATTR_IFs which reference <netname> */
	sprintf( query, "%s *", netname );
	if ( get_attr( &netref, &ninfo, 0, query, 0, ATTR_IF ) <= 0 )
		return( FAILURE );

	filter_output = ( (subclass > 0 ) || 
							(type > 0 ) ||
							(attr > 0) ) ? TRUE : FALSE;

	/* for each network interface... */
	for (i=0; i < ninfo.num; i++)
	{
		/* get all the resources this machine serves */
		if ( get_attr( &res, &rinfo, netref[i].id, NULL, 0, ATTR_SERVES ) <= 0 )
			continue;

		/* add entries in the list for the resources */
		for (j=0; j < rinfo.num; j++)
		{	/* get this resource's nim_object */
			sprintf( query, "name='%s'", res[j].value );
			if ( odm_get_first( nim_object_CLASS, query, &obj ) <= 0 )
				continue;

			/* make sure resource is "available" */
			sprintf( query, "id=%d and pdattr=%d", obj.id, ATTR_RSTATE );
			if ( odm_get_first( nim_attr_CLASS, query, &rstate ) <= 0 )
				continue;
			if ( strcmp(rstate.value, available_state) )
				continue;

			/* filter list based on subclass or type? */
			if ( filter_output )
			{	/* yep - need this resource's pdattr */
				sprintf( query, "attr=%s", obj.type_Lvalue );
				if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
					continue;

				if (	subclass > 0 )
				{	if (! attr_in_list( subclass, pdattr.subclass ) )
						continue;
				}

				if ( type > 0 )
				{	if  (! attr_in_list( type, pdattr.type ) )
						continue;
				}

				if ( attr > 0 )
				{	if ( attr != pdattr.attr )
						continue;
				}
			}

			list_add( names, res[j].value );
		}

		odm_free_list( res, &rinfo );
	}

	odm_free_list( netref, &ninfo );

	names->list[ names->num ] = NULL;

	return( SUCCESS );

} /* end of list_res_on_net */

/*******************************************************************************
******************* colon_output manipulation routines    **********************
*******************************************************************************/

struct colon_output {								/* struct for storage of data */
	char *tag;											/*		used to either print colon */
	char *str;											/*		format and/or store unique */
	struct colon_output *next;						/*		names */
};
struct colon_output *colon_output_head=NULL;	/* ptr to head of linked list */
struct colon_output *colon_output_tail=NULL;	/* ptr to tail of class name */

/*---------------------------- pflush              ----------------------------
*
* NAME: pflush
*
* FUNCTION:
*		displays the information which has been stored in the colon_output linked
*			list
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
*			colon_output		= linked list of info to be displayed in colon format
*
* RETURNS: (int)
*		SUCCESS					= info displayed in colon format
*
* OUTPUT:
*		writes information to output
*-----------------------------------------------------------------------------*/

int
pflush()

{	struct colon_output *node=colon_output_head;

	/* colon format requested? */
	if ( (display_mask & DISPLAY_COLON) && (node != NULL) )
	{	/* print out all of the tags */
		fprintf( output, "#" );
		while ( node != NULL )
		{	fprintf( output, "%s:", node->tag );
			node = node->next;
		}
		fprintf( output, "\n" );

		/* print the values */
		node = colon_output_head;
		while ( node != NULL )
		{	fprintf( output, "%s:", node->str );
			node = node->next;
		}
		fprintf( output, "\n" );

		/* flush the output */
		fflush( output );

		/* free the malloc'd space */
		while ( colon_output_head != NULL )
		{	free( colon_output_head->tag );
			free( colon_output_head->str );
			node = colon_output_head->next;
			free( colon_output_head );
			colon_output_head = node;
		}
		colon_output_tail = NULL;
	}

	/* return */
	return( SUCCESS );

} /* end of pflush */
	
/*---------------------------- pdone             ------------------------------
*
* NAME: pdone
*
* FUNCTION:
*		flushes anything in the colon_output list and closes the output FILE
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
* RETURNS: (int)
*		SUCCESS
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
pdone()

{	int rc=SUCCESS;

	/* flush any output */
	pflush();

	/* was output a pipe? */
	if ( output != stdout )
		pclose( output );

	/* return */
	return( rc );

} /* end of pdone */
	
/*---------------------------- unique_name       ------------------------------
*
* NAME: unique_name
*
* FUNCTION:
*		returns TRUE if the specified name hasn't already been displayed
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			name					= name of object
*		global:
*			colon_output nodes
*
* RETURNS: (int)
*		TRUE						= name hasn't been seen before
*		FALSE						= name already referenced
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
unique_name(	char *name )

{	struct colon_output *node=colon_output_head;

	/* look for the specified name */
	while ( node != NULL )
	{	if ( ! strcmp(node->tag, ATTR_NAME_T) )
			if ( ! strcmp(node->str, name) )
				return( FALSE );
		node = node->next;
	}

	return( TRUE );

} /* end of unique_name */
	
/*---------------------------- add_node          -------------------------------
*
* NAME: add_node
*
* FUNCTION:
*		adds a new colon_output node
*		the colon_output linked list can be used to either:
*			1) cache info to be printed later in colon format
*			2) keep track of unique names
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			tag					= colon_output.tag
*			str					= colon_output.str
*		global:
*			colon_output		= linked list of info to be displayed in colon format
*
* RETURNS: (int)
*		SUCCESS					= new node added
*		FAILURE					= unable to create new node (malloc error?)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
add_node(	char *tag, 
				char *str )

{	struct colon_output *node;
	int total_len = 0;
	int len = 0;
	int extra = 0;
	char *ptr = str;
	char *newline = NULL;

	/* initialize a new colon_output node */
	if ( (node = malloc( sizeof(struct colon_output) )) <= 0 )
		return( FAILURE );

	if ( (node->tag = malloc( strlen(tag) + 1 )) <= 0 )
		return( FAILURE );
	strcpy( node->tag, tag );

	/* substitute '\n' with "\\n" so that it doesn't mess up what SMIT expects */
	while ( (ptr != NULL) && (*ptr != NULL_BYTE) )
	{
		/* any newline chars? */
		if ( (newline = strchr( ptr, '\n' )) != NULL )
		{
			/* split str into 2 parts */
			*newline = NULL_BYTE;

			/* add in space for extra '\' that we're going to add */
			extra = 3;
		}
		else
			extra = 1;

		/* get space for this part of str */
		if ( total_len == 0 )
		{
			total_len = strlen( ptr ) + extra;
			node->str = nim_malloc( total_len );
			node->str[0] = NULL_BYTE;
		}
		else
		{
			len = strlen( ptr ) + extra;
			node->str = nim_realloc( node->str, total_len, len );
			total_len += len;
		}

		/* append this part of the string to final str */
		strcat( node->str, ptr );

		/* move ptr */
		ptr = newline;

		/* are we adding an extra '\' ? */
		if ( newline != NULL )
		{
			/* append "\\n" */
			strcat( node->str, "\\n" );

			/* move the pointer past this newline */
			ptr = newline + 1;
		}
	}

	/* link in new node */
	node->next = NULL;
	if ( colon_output_head == NULL )
	{	colon_output_head = node;
		colon_output_tail = node;
	}
	else
	{	colon_output_tail->next = node;
		colon_output_tail = node;
	}

	/* return */
	return( SUCCESS );

} /* end of add_node */

/*---------------------------- pfield            -------------------------------
*
* NAME: pfield
*
* FUNCTION:
*		displays the specified field if colon output not desired; otherwise, the
*			field is stored in the colon_output linked list in order to print it 
*			later
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			format				= format string to use
*			tag					= SMIT tag to use for colon format
*			str1					= string to print; if colon format specified, str1
*											will only be printed if str2 is NULL
*			str2					= string to print
*		global:
*			colon_output		= linked list of info to be displayed in colon format
*			printed_title		= TRUE if title already printed
*			obj_name				= name of object which info being displayed for
*
* RETURNS: (int)
*		SUCCESS					= information displayed
*
* OUTPUT:
*		writes the specified string to stdout
*-----------------------------------------------------------------------------*/
/*??? candidate for vargs ???*/

int
pfield(	char *format, 
			char *tag, 
			char *str1, 
			char *str2 )

{	struct colon_output *node;
	int is_name;
	char *str;

/*??? consider limiting str to 80 chars (for comments especially ??? */

	/* need to print a title? */
	if ( (! printed_title) && (obj_name != NULL) && (obj_name[0] != NULL_BYTE) )
	{
		printed_title = TRUE;
		pfield("%s:\n", ATTR_NAME_T, obj_name, NULL );
	}

/*??? is it ok to perform unique name search here???? */
	is_name = (strcmp( tag, ATTR_NAME_T ) == 0) ? TRUE : FALSE;

	if ( (is_name) && (! unique_name(str1)) )
		return( SUCCESS );

	str = ( str2 != NULL ) ? str2 : str1;

	if ( (is_name) || (display_mask & DISPLAY_COLON) )
		add_node( tag, str );

	/* display info now? */
	if ( (display_mask & DISPLAY_COLON) == 0 )
		fprintf( output, format, str1, str2 );

	/* return */
	return( SUCCESS );

} /* end of pfield */
	
/*---------------------------- set_obj_name      ------------------------------
*
* NAME: set_obj_name
*
* FUNCTION:
*		initializes the global vars <obj_name> & <printed_title>, which are
*			used by pfield
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			name					= name of object
*		global:
*			obj_name
*			printed_title
*
* RETURNS: (int)
*		SUCCESS
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
set_obj_name(	char *name )

{
	obj_name = name;
	printed_title = FALSE;

	return( SUCCESS );

} /* end of set_obj_name */

/*******************************************************************************
******************* alignment                             **********************
*******************************************************************************/

/*---------------------------- align_output      -------------------------------
*
* NAME: align_output
*
* FUNCTION:
*		opens a pipe to the awk command in order to align columns in a specific
*			way, which is dependent upon the awk script which is specified
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls lsnim_error on popen failure
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			awk					= awk script to send output to
*		global:
*
* RETURNS: (int)
*		SUCCESS					= successful open
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
align_output( char *awk )

{	

	if ( ! aligned_yet )
	{
		/* open a pipe to awk using the awk script which will align the output */
		if ( (output = popen( awk, "w" )) == NULL )
			lsnim_error( ERR_SYS, "popen", strerror(errno), NULL );

		piped_output = TRUE;
		aligned_yet = TRUE;
	}

	return( SUCCESS );

} /* end of align_output */
	
/*******************************************************************************
******************* predefined info manipulation          **********************
*******************************************************************************/

/*---------------------------- d_pdattr          ------------------------------
*
* NAME: d_pdattr
*
* FUNCTION:
*		displays info for the given <pdattr>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			pdattr				= ptr to nim_pdattr struct
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_pdattr(	struct nim_pdattr *pdattr )

{	struct nim_pdattr *pd_list;
	struct listinfo info;
	int i;

	/* display the pdattr? */
	if ( ! (pdattr->mask & PDATTR_MASK_DISPLAY) && ! (override_mask) )
		/* no displayable - return now */
		return( SUCCESS );

	/* what to display? */
	if ( (display_mask & DISPLAY_OPS) || (display_mask & DISPLAY_CONTROL_OPS) )
	{	
		if ( display_mask & DISPLAY_OPS )
			get_pdattr( &pd_list, &info, 0,
							ATTR_SUBCLASS_OPERATION, pdattr->attr, 0, NULL );
		else
			get_pdattr( &pd_list, &info, 0,
							ATTR_SUBCLASS_CONTROL, pdattr->attr, 0, NULL );

		/* display the info */
		pfield( "%s:\n", ATTR_SUBCLASS_TYPE_T, pdattr->name, NULL );
		for (i=0; i < info.num; i++)
		{
			if ( (pd_list[i].mask & PDATTR_MASK_DISPLAY) == 0 )
				continue;
			if ( display_mask & DISPLAY_HELPS )
				pfield( "   .%s. %s\n", ATTR_SUBCLASS_OPERATION_T,
							pd_list[i].name, ATTR_msg(pd_list[i].h_num) );
			else
				pfield( "   .%s. %s\n", ATTR_SUBCLASS_OPERATION_T, 
							pd_list[i].name, pd_list[i].value );
		}

		odm_free_list( pd_list, &info );
	}
	else if ( display_mask & DISPLAY_LONG )
	{	/* display detailed info */
		pfield( "%s:\n", ATTR_SUBCLASS_TYPE_T, pdattr->name, NULL );
		pfield( "   .%s.  %s\n", ATTR_CLASS_T, 
					ATTR_CLASS_T, ATTR_msg(pdattr->class) );
		pfield( "   .%s.  %s\n", ATTR_TYPE_T, ATTR_TYPE_T, 
					ATTR_msg(pdattr->attr) );

		/* get all the nim_pdattr associated with this type */
		if ( get_pdattr( &pd_list, &info, 0, 0, pdattr->attr, 0, NULL ) > 0 )
		{	for (i=0; i < info.num; i++)
				if ( (pd_list[i].mask & PDATTR_MASK_DISPLAY) || (override_mask) )
				{	if ( display_mask & DISPLAY_HELPS )
						pfield( "   .%s.  %s\n", pd_list[i].name, 
									pd_list[i].name, ATTR_msg(pd_list[i].h_num) );
					else
						pfield( "   .%s.  %s\n", pd_list[i].name, 
									pd_list[i].name, pd_list[i].value );
				}
			odm_free_list( pd_list, &info );
		}
	}
	else
	{	/* display the type */
		if ( display_mask & DISPLAY_HELPS )
			pfield( ".%s.  %s\n", ATTR_SUBCLASS_TYPE_T,
						pdattr->name, ATTR_msg(pdattr->h_num) );
		else
			pfield( "%s\n", ATTR_SUBCLASS_TYPE_T, pdattr->name, NULL );
	}

	return( SUCCESS );

} /* end of d_pdattr */
	
/*---------------------------- pd_class          ------------------------------
*
* NAME: pd_class
*
* FUNCTION:
*		displays predefined info about a specific class
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			class_num			= predefined class to display info for
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
pd_class(	int class_num )

{	NIM_PDATTR( pdattr, info )
	int i;
	int detailed_info = (	(display_mask & DISPLAY_LONG) ||
									(display_mask & DISPLAY_OPS) ||
									(display_mask & DISPLAY_CONTROL_OPS) );

	/* get all the object types in this class */
	if ( get_pdattr( &pdattr, &info, class_num, 
							ATTR_SUBCLASS_TYPE, 0, 0, NULL ) <= 0 )
		return( FAILURE );

	/* display the info */
	for (i=0; i < info.num; i++)
		if ( (pdattr[i].mask & PDATTR_MASK_DISPLAY) || (override_mask) )
		{
			if ( detailed_info )
				d_pdattr( pdattr+i );
			else if ( display_mask & DISPLAY_HELPS )
				pfield( ".%s.  %s\n", ATTR_SUBCLASS_TYPE_T, 
							pdattr[i].name, ATTR_msg(pdattr[i].h_num) );
			else
				pfield( "%s\n", ATTR_SUBCLASS_TYPE_T, 
							pdattr[i].name, NULL );
		}

	odm_free_list( pdattr, &info );

	return( SUCCESS );

} /* end of pd_class */
	
/*---------------------------- pd_subclass       ------------------------------
*
* NAME: pd_subclass
*
* FUNCTION:
*		display predefined information about a specific subclass of objects
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
*			subclass
*			subclass_num
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
pd_subclass()

{	struct nim_pdattr *pdattr;
	struct listinfo info;
	int i;

	/* get all the predefined types which are a member of this subclass */
	if ( get_pdattr( &pdattr, &info, 0, subclass_num, 0, 0, NULL ) <= 0 )
		return( FAILURE );

	/* display the info */
	for (i=0; i < info.num; i++)
		d_pdattr( pdattr+i );

	odm_free_list( pdattr, &info );

	return( SUCCESS );

} /* end of pd_subclass */
	
/*---------------------------- pd_type           ------------------------------
*
* NAME: pd_type
*
* FUNCTION:
*		displays predefined information based upon the specified pdattr.attr
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
*			type
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= error encountered (probably ODM error)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
pd_type()

{	char query[MAX_CRITELEM_LEN];
	struct nim_pdattr pdattr;

	/* get the nim_pdattr for the specified type */
	sprintf( query, "attr=%d", type_num );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		return( FAILURE );

	/* display the info */
	d_pdattr( &pdattr );

	return( SUCCESS );

} /* end of pd_type */
	
/*---------------------------- all_pd_subclasses     ---------------------------
*
* NAME: all_pd_subclasses
*
* FUNCTION:
*		displays the names of the predefined subclasses
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
* RETURNS: (int)
*		SUCCESS					= subclasses displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
all_pd_subclasses()

{	struct nim_pdattr *pdattr;
	struct listinfo info;
	int i;

	/* display the object classes */
	if ( get_pdattr( &pdattr, &info, 0, 
							ATTR_SUBCLASS, NULL, 0, NULL ) <= 0 )
		return( FAILURE );

	for (i=0; i < info.num; i++)
		if ( (pdattr[i].mask & PDATTR_MASK_DISPLAY) || (override_mask) )
		{	if ( display_mask & DISPLAY_HELPS )
				pfield( ".%s.  %s\n", ATTR_SUBCLASS_T, 
							pdattr[i].name, ATTR_msg(pdattr[i].h_num) );
			else
				pfield( "%s\n", ATTR_SUBCLASS_T, pdattr[i].name, NULL );
		}

	return( SUCCESS );

} /* end of all_pd_subclasses */

/*---------------------------- pd_attr           ------------------------------
*
* NAME: pd_attr
*
* FUNCTION:
*		display help information about specified predefined attributes
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
*
* OUTPUT:
*		writes help information to stdout
*-----------------------------------------------------------------------------*/

int
pd_attr()

{	int i;
	ODMQUERY
	struct nim_pdattr pdattr;

	for (i=0; i < attr_ass.num; i++)
	{
		/* get the predefined attr */
		sprintf( query, "attr=%d", attr_ass.list[i]->pdattr );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		{
			/* warn the user and continue */
			warning( ERR_ATTR_NAME, attr_ass.list[i]->name, NULL, NULL );
			continue;
		}

		/* display the pdattr? */
		if ( ! (pdattr.mask & PDATTR_MASK_DISPLAY) && ! (override_mask) )
			/* not displayable - skip it */
			continue;

		/* display detailed help info */
		pfield( "%s::\n", pdattr.name, pdattr.name, NULL );
		pfield( "%s\n", pdattr.name, VERBOSE_msg( pdattr.verbose_num ), NULL );
	}

	return( SUCCESS );

} /* end of pd_attr */

/*---------------------------- all_pd_classes        ---------------------------
*
* NAME: all_pd_classes
*
* FUNCTION:
*		displays the names of the predefined classes
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
* RETURNS: (int)
*		SUCCESS					= classes displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
all_pd_classes()

{	NIM_PDATTR( classes, cinfo )
	int i;

	/* display the object classes */
	if ( get_pdattr( &classes, &cinfo, 0, 
							ATTR_SUBCLASS_CLASS, NULL, 0, NULL ) <= 0 )
		return( FAILURE );

	for (i=0; i < cinfo.num; i++)
		if ( (classes[i].mask & PDATTR_MASK_DISPLAY) || (override_mask) )
		{
			if (	(display_mask & DISPLAY_LONG) ||
					(display_mask & DISPLAY_OPS) ||
					(display_mask & DISPLAY_CONTROL_OPS) )
				pd_class( classes[i].attr );
			else if ( display_mask & DISPLAY_HELPS )
				pfield( ".%s.  %s\n", ATTR_SUBCLASS_CLASS_T, 
							classes[i].name, ATTR_msg(classes[i].h_num) );
			else
				pfield( "%s\n", ATTR_SUBCLASS_CLASS_T, classes[i].name, NULL );
		}

	odm_free_list( classes, &cinfo );

	return( SUCCESS );

} /* end of all_pd_classes */
	
/*---------------------------- d_pdinfo          ------------------------------
*
* NAME: d_pdinfo
*
* FUNCTION:
*		displays predefined information
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
*			class
*			class_num
*			subclass
*			type
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= error encountered (probably ODM error)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_pdinfo()

{	
	/* print info about the class or a specific type? */
	if ( class != NULL )
		/* class specific info */
		pd_class( class_num );
	else if ( subclass != NULL )
		/* subclass specific info */
		pd_subclass();
	else if ( type != NULL )
		/* type specific info */
		pd_type();
	else if ( display_mask & DISPLAY_SUBCLASSES )
		/* display subclass names */
		all_pd_subclasses();
	else if ( display_mask & DISPLAY_ATTR )
		/* display predefined info about attrs */
		pd_attr();
	else
		/* display class names */
		all_pd_classes();

	return( SUCCESS );

} /* end of d_pdinfo */
	
/*******************************************************************************
******************* customized info manipulation          **********************
*******************************************************************************/

/*---------------------------- display_obj        ------------------------------
*
* NAME: display_obj
*
* FUNCTION:
*		returns TRUE if specified object matches the subclass or type specified
*			by the user; FALSE otherwise
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			objname				= name of nim_object
*		global:
*
* RETURNS: (int)
*		TRUE						= specified object passes through the filter
*		FALSE						= specified object should not be displayed
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
display_obj(	char *objname )

{	struct nim_object obj;
	struct nim_pdattr pdattr;
	ODMQUERY

	/* get the object */
	sprintf( query, "name='%s'", objname );
	if ( odm_get_first( nim_object_CLASS, query, &obj ) <= 0 )
		return( FALSE );

	/* get the pdattr corresponding to its type */
	sprintf( query, "attr=%s", obj.type_Lvalue );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
		return( FALSE );

	/* is object displayable? */
	if ( ! (pdattr.mask & PDATTR_MASK_DISPLAY) && ! (override_mask) )
		return( FALSE );

	/* filter based on subclass or type? */
	if ( (subclass_num) || (type_num) )
	{
		if (	(	(subclass_num) && 
					(! attr_in_list(subclass_num,pdattr.subclass)) ) ||
				(	(type_num) && (type_num != pdattr.attr) ) )
			return( FALSE );
	}

	return( TRUE );

} /* end of display_obj */
	
/*---------------------------- d_state           ------------------------------
*
* NAME: d_state
*
* FUNCTION:
*		displays the help text for the specified STATE
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
*			pdattr_name			= pdattr.name
*			state					= STATE_xxx value
*		global:
*
* RETURNS: (int)
*		SUCCESS			
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_state(	char *pdattr_name,
			int state )

{	ODMQUERY
	struct nim_pdattr pdattr;

	/* get the nim_pdattr for this state */
	sprintf( query, "class=%d and type=%d", ATTR_CLASS_STATES, state );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) == NULL )
	{
		/* display the raw STATE */
		pfield( "   .%s.  %s\n", pdattr_name, pdattr_name, STATE_msg(state) );
	}
	else
	{
		/* display the help message */
		pfield( "   .%s.  %s\n", pdattr_name, pdattr_name,
					STATE_msg( pdattr.h_num ) );
	}

	return( SUCCESS );

} /* end of d_state */

/*---------------------------- d_attr            ------------------------------
*
* NAME: d_attr
*
* FUNCTION:
*		displays info about nim_attrs
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= object that pertains to these attrs
*			attrs					= ptr to array of nim_attrs
*			info					= ptr to listinfo struct
*		global:
*			specific_attr
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_attr(	struct nim_object *obj,
			struct nim_attr *attrs,
			struct listinfo *info )

{	int i,j;
	char tmp[MAX_TMP];
	struct nim_if nimif;
	char *net;
	char *gateway;
	NIM_PDATTR( pdattr, pdattri)

	/* for each attribute... */
	for (i=0; i < info->num; i++)
	{	
		/* should it be displayed? */
		if ( attr_ass.num > 0 )
		{
			/* is this attr in the list of attrs to be displayed? */
			if (	((j = find_attr_ass( &attr_ass, attrs[i].pdattr->attr )) < 0) ||
					(	(attr_ass.list[j]->seqno > 0) && 
						(attr_ass.list[j]->seqno != attrs[i].seqno)) )
				continue;
		}
		/* 
		 * Need to make sure that the pdattr is the correct one based on 
		 * the characteristics of the object.
		 */
		if ( get_pdattr( &pdattr, &pdattri, 0, 0, obj->type->attr, 
			attrs[i].pdattr->attr, NULL) <= 0 )
			continue;

		if ( ((pdattr->mask & PDATTR_MASK_DISPLAY) == 0) && 
				(! override_mask) )
			continue;

		/* filter based type? */
		if ( (type_num > 0) && (! attr_in_list(type_num, pdattr->type)) )
			continue;

		/* does attr require special display processing? */
		if (	(pdattr->attr == ATTR_IF) && (display_mask & DISPLAY_COLON) )
		{
			/* convert NIM interface stanza into struct */
			if ( s2nim_if( attrs[i].value, &nimif ) == FAILURE )	
				continue;

			/* print each field separately for the SMIT dialogue */
			pfield( "   .%s.  %s\n", IF_TAGS[0], IF_TAGS[0], nimif.network );
			pfield( "   .%s.  %s\n", IF_TAGS[1], IF_TAGS[1], nimif.hostname );
			pfield( "   .%s.  %s\n", IF_TAGS[2], IF_TAGS[2], nimif.hard_addr );
			pfield( "   .%s.  %s\n", IF_TAGS[3], IF_TAGS[3], nimif.adapter );
		}
		else if ( (pdattr->attr == ATTR_ROUTING) && (display_mask & DISPLAY_COLON) )
		{
			/* separate the NIM route into 2 fields */
			if ( two_fields( attrs[i].value, &net, &gateway ) == FAILURE )
				continue;

			/* print each field separately for the SMIT dialogue */
			pfield( "   .%s.  %s\n", ROUTING_TAGS[0], ROUTING_TAGS[0], net );
			pfield( "   .%s.  %s\n", ROUTING_TAGS[1], ROUTING_TAGS[1], gateway );
		}
		else if ( pdattr->mask & PDATTR_MASK_SEQNO_REQ )
		{	
			sprintf( tmp, "%s%d", pdattr->name, attrs[i].seqno );
			pfield( "   .%s.  %s\n", pdattr->name, tmp, attrs[i].value );
		}
		else if ( pdattr->mask & PDATTR_MASK_VAL_IS_STATE )
			d_state( pdattr->name, (int) strtol(attrs[i].value,NULL,0) );
		else
			/* generic attr - display it now */
			pfield( "   .%s.  %s\n", pdattr->name, 
						pdattr->name, attrs[i].value );
	}

	return( SUCCESS );

} /* end of d_attr */

/*---------------------------- d_specific_attr   ------------------------------
*
* NAME: d_specific_attr
*
* FUNCTION:
*		displays info about a specific attributes
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj				= ptr to nim_object struct
*		global:
*			specific_attr
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_specific_attr(	struct nim_object *obj )

{	NIM_ATTR( attrs, info )
	int i;
	int found = FALSE;

	/* get all the attrs of the specified type which exist for <obj> */
	if ( get_attr( &attrs, &info, obj->id, NULL, 0, specific_attr ) < 0 )
		return( FAILURE );

	/* display them */
	set_obj_name( obj->name );
	d_attr( obj, attrs, &info );

	/* free the attrs */
	odm_free_list( attrs, &info );

	return( SUCCESS );

} /* end of d_specific_attr */

/*---------------------------- d_ll              ------------------------------
*
* NAME: d_ll
*
* FUNCTION:
*		displays long listing for the specified nim_object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj				= ptr to nim_object struct
*		global:
*
* RETURNS: (int)
*		SUCCESS				= info displayed
*		FAILURE				= error encountered (probably ODM problem)
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_ll( struct nim_object *obj )

{	struct nim_object *object;
	struct listinfo info;
	int i;
	char tmp[MAX_TMP];

	if (! display_obj( obj->name ) )
		return( FAILURE );

	/* <obj> only contains info with a depth of 1 - get info to depth of 3 */
	if ( get_object( &object, &info, obj->id, NULL, 0, 0 ) <= 0 )
		return( FAILURE );

	/* display all the info about this machine */
	set_obj_name( object->name );
	if ( override_mask )
	{	sprintf( tmp, "%d", object->id );
		pfield("   .%s.  %s\n", "id", "id", tmp );
	}
	pfield("   .%s.  %s\n", ATTR_CLASS_T, ATTR_CLASS_T, ATTR_msg(object->class));
	pfield("   .%s.  %s\n", ATTR_TYPE_T, ATTR_TYPE_T, 
											ATTR_msg(strtol(object->type_Lvalue,NULL,0)) );

	/* display the attrs */
	d_attr( object, object->attrs, object->attrs_info );

	/* cleanup */
	odm_free_list( object, &info );

	return( SUCCESS );

} /* end of d_ll */
	
/*---------------------------- d_mac_use         ------------------------------
*
* NAME: d_mac_use
*
* FUNCTION:
*		displays the machines which use the specified resource
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			name					= resource name
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= couldn't find the object for <name>
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_mac_use(	char *name )

{	NIM_ATTR( alloc, ainfo )
	ODMQUERY
	int type;							/* type of <name> */
	int j;
	struct nim_object mac;

	/* get the resource's type */
	if ( (type = get_type( 0, name )) == 0 )
		return( FAILURE );

	/* get all the nim_attrs of type <type> whose value is <name> */
	get_attr( &alloc, &ainfo, 0, name, 0, type );
	for (j=0; j < ainfo.num; j++)
	{	/* get the nim_object corresponding to the attr id */
		sprintf( query, "id=%d", alloc[j].id );
		if (	(odm_get_first( nim_object_CLASS, query, &mac ) > 0) &&
				(mac.class == ATTR_CLASS_MACHINES) &&
				(display_obj( mac.name )) )
			pfield(".%s.\n", ATTR_NAME_T, mac.name, NULL );
	}

	if ( ainfo.num )
		odm_free_list( alloc, &ainfo );

	/* return */
	return( SUCCESS );

} /* end of d_mac_use */
	
/*---------------------------- mac_mac           ------------------------------
*
* NAME: mac_mac
*
* FUNCTION:
*		displays relationships machine objects
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mac_mac(	struct nim_object *obj )

{	NIM_ATTR( serves, sinfo )
	NIM_ATTR( alloc, ainfo )
	ODMQUERY
	struct nim_attr attr;
	int i,j;
	char *name;

/*????????? machines can be both servers & clients: do we need some way ???*/
/*????????? for user to tell us what they want to see -OR- take this ???*/
/*????????? approach & enforce a precidance of our own??? */


	/* treat anchor a server or client? */
	if ( get_attr( &serves, &sinfo, obj->id, NULL, 0, ATTR_SERVES ) < 0 )
		return ( FAILURE );
	else if ( sinfo.num > 0 )
	{	/* the anchor serves at least one resource */
		/* display the machines which use a resource served by the anchor */
		for (i=0; i < sinfo.num; i++)
			d_mac_use( serves[i].value );
	}
	else if ( get_attr( &alloc, &ainfo, obj->id, NULL, 0, 0 ) < 0 )
		return ( FAILURE );
	else if ( ainfo.num > 0 )
	{	/* sort through all the attrs looking for ones which represent resources*/
		for (i=0; i < ainfo.num; i++)
			if ( alloc[i].pdattr->class == ATTR_CLASS_RESOURCES )
			{	/* this is an allocated resource */
				/* find the server */
				sprintf( query, "value='%s' and pdattr=%d", 
							alloc[i].value, ATTR_SERVES );
				if ( odm_get_first( nim_attr_CLASS, query, &attr ) <= 0 )
					continue;

				name = get_name( attr.id );

				if ( display_obj( name ) )
					pfield(".%s.\n", ATTR_NAME_T, name, NULL );

				free( name );
			}
	}

	return( SUCCESS );

} /* end of mac_mac */

/*---------------------------- mac_res           ------------------------------
*
* NAME: mac_res
*
* FUNCTION:
*		displays relationships machine & resource objects
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mac_res(	struct nim_object *obj )

{	NIM_ATTR( attrs, ainfo )
	ODMQUERY
	int i;
	int len = 0;
	int max_len = 0;
	int comments;
	char *tmp = NULL;
	NIM_OBJECT( res, rinfo )

/*?????? also - use type_num as a filter ?????? */

	/* get all the attr for this object */
	if ( get_attr( &attrs, &ainfo, obj->id, NULL, 0, 0 ) < 0 )
		return ( FAILURE );

#if 0
???? only want to display the resources which have been allocated because
???? the NIM smit screens only need that info
	/* display the resources this machine serves */
	for (i=0; i < ainfo.num; i++)
		if ( attrs[i].pdattr->attr == ATTR_SERVES )
		{	/* filter based on subclass or type? */
			if ( ! display_obj( attrs[i].value ) )
				continue;

			pfield("%s\n", ATTR_SERVES_T, attrs[i].value, NULL );
		}
#endif

	/* display the resources which have been allocated to this machine */
	for (i=0; i < ainfo.num; i++)
		if ( attrs[i].pdattr->class == ATTR_CLASS_RESOURCES )
		{	
			/* filter based on subclass or type? */
			if ( ! display_obj( attrs[i].value ) )
				continue;

			/* get the resource's object */
			if ( get_object( &res, &rinfo, 0, attrs[i].value, 0, 0 ) <= 0 )
				continue;

			/* resource has comments? */
			comments = find_attr( res, NULL, NULL, 0, ATTR_COMMENTS );

			/* how much tmp space is needed? */
			len = strlen(res->name) + strlen(res->type->name);
			if ( comments >= 0 )
				len += strlen( res->attrs[comments].value );

			/* We need to add 5 more bytes to the len.  */
			/* This is for two periods, two spaces, and */
			/* the null character in the string.        */
			len += 5;

			/* need more space? */
			if ( len > max_len )
			{
				if ( tmp != NULL )
					free( tmp );
				tmp = nim_malloc( len );
				max_len = len;
			}

			/* format the string */
			if ( comments >= 0 )
				sprintf( tmp, ".%s. %s %s", res->name, res->type->name,
							res->attrs[comments].value );
			else
				sprintf( tmp, ".%s. %s", res->name, res->type->name );

			/* display the info */
			pfield("%s\n", res->type->name, tmp, NULL );
		}

	/* free the list */
	odm_free_list( attrs, &ainfo );

	return( SUCCESS );

} /* end of mac_res */

/*---------------------------- mac_net           ------------------------------
*
* NAME: mac_net
*
* FUNCTION:
*		displays relationships machine & network objects
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
mac_net(	struct nim_object *obj )

{	LIST ifs;
	int i;
	struct nim_if *nimif;

	/* generate a LIST of nim_if structs for this machine */
	if ( LIST_ifs( obj->id, NULL, &ifs ) == FAILURE )
		return( FAILURE );

	/* display the network names */
	for (i=0; i < ifs.num; i++)
	{	nimif = (struct nim_if *) ifs.list[i];
		if ( display_obj( nimif->network ) )
			pfield( ".%s.\n", ATTR_NAME_T, nimif->network, NULL );
	}

	free_list( &ifs );

	return( SUCCESS );

} /* end of mac_net */

/*---------------------------- res_mac           ------------------------------
*
* NAME: res_mac
*
* FUNCTION:
*		displays the names of machines which use the specified resource
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
res_mac(	struct nim_object *obj )

{	
	return( d_mac_use( obj->name ) );

} /* end of res_mac */
	
/*---------------------------- res_net           ------------------------------
*
* NAME: res_net
*
* FUNCTION:
*		displays the names of the networks which have the ability to access this
*			resource; ie, displays the networks which can connect to the server of
*			the specified resource
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= name of nim_object resource
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
res_net(	struct nim_object *obj )

{	ODMQUERY
	struct nim_attr server;
	LIST list;
	int i,j;
	NIM_ATTR( nets, ninfo )
	struct nim_if *nimif;

	/* find the server of this resource */
	sprintf( query, "id=%d and pdattr=%d", obj->id, ATTR_SERVER );
	if ( odm_get_first( nim_attr_CLASS, query, &server ) <= 0 )
		return( FAILURE );

	/* get a LIST of the server's interfaces */
	if ( LIST_ifs( 0, server.value, &list ) == FAILURE )
		return( FAILURE );

	/* for each network the server connects to */
	for (i=0; i < list.num; i++)
	{	/* any network the server directly connects to can definitely get */
		/*		to it, so print it now */
		nimif = (struct nim_if *) list.list[i];
		if ( display_obj( nimif->network ) )
			pfield( ".%s.\n", ATTR_NAME_T, nimif->network, NULL );

		/* find all the ATTR_ROUTING attrs which reference this network */
		sprintf( query, "%s *", nimif->network );
		if ( get_attr( &nets, &ninfo, 0, query, 0, ATTR_ROUTING ) <= 0 )
			continue;

		/* display the network names */
		for (j=0; j < ninfo.num; j++)
			if ( display_obj( nets[j].value ) )
				pfield( ".%s.\n", ATTR_NAME_T, get_name(nets[j].id), NULL );

		odm_free_list( nets, &ninfo );
	}

	/* free the LIST */
	free_list( &list );

	return( SUCCESS );

} /* end of res_net */

/*---------------------------- net_mac           ------------------------------
*
* NAME: net_mac
*
* FUNCTION:
*		displays the machines which are connected to the specified network
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
net_mac(	struct nim_object *obj )

{	char query[MAX_CRITELEM_LEN];
	struct nim_attr *attr;
	struct listinfo info;
	int i;
	char *name;

/*?????? also - use type_num as a filter ?????? */

	/* get all the nim_attrs which reference the specified network */
	sprintf( query, "%s *", obj->name );
	if ( get_attr( &attr, &info, 0, query, 0, 0 ) <= 0 )
		return( FAILURE );

	/* look for attrs of the ATTR_SUBCLASS_IF subclass */
	for (i=0; i < info.num; i++)
		if ( attr_in_list( ATTR_SUBCLASS_IF, attr[i].pdattr->subclass ) )
		{	name = get_name( attr[i].id );
			if ( display_obj( name ) )
				pfield( ".%s.\n", ATTR_NAME_T, name, NULL );
			free( name );
		}

	return( SUCCESS );

} /* end of net_mac */
	
/*---------------------------- d_res_on_net      ------------------------------
*
* NAME: d_res_on_net
*
* FUNCTION:
*		displays the resources which are served by machines that are connected to
*			the specified network
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls lsnim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			netname				= name of network
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_res_on_net(	char *netname )

{	int i;
	LIST names;
	char **list;

	/* generate the list of resources served by machines on this net */
	if (! get_list_space( &names, 20, TRUE ) )
		lsnim_error( 0, NULL, NULL, NULL );
	if ( list_res_on_net( &names, netname, subclass_num, 0, type_num ) )
		for (i=0; i < names.num; i++)
			pfield( ".%s.\n", ATTR_NAME_T, names.list[i], NULL );

	return( SUCCESS );

} /* end of d_res_on_net */
	
/*---------------------------- net_res           ------------------------------
*
* NAME: net_res
*
* FUNCTION:
*		displays the resources which can be accessed from the specified network
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object for a network object
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
net_res(	struct nim_object *obj )

{	struct nim_attr *routing;
	struct listinfo rinfo;
	int i;
	char **list;

	/* first, print out all the resources served by machines connected to */
	/*		this network */
	d_res_on_net( obj->name );

#if 0
/*???? I don't think we want to show POTENTIAL resource access -
		that's what "-L" is for - just show the resources served by machines
		on this net ?????*/

	/* get all the ATTR_ROUTING nim_attrs for this network */
	if ( get_attr( &routing, &rinfo, obj->id, NULL, 0, ATTR_ROUTING ) < 0 )
		return( FAILURE );

	/* now, print out the resources served by the networks which this net */
	/*		can communicate with */
	for (i=0; i < rinfo.num; i++)
	{	/* separate the net name from the other info */
		if ( ! first_field( routing[i].value ) )
			continue;

		d_res_on_net( routing[i].value );
	}
#endif

	odm_free_list( routing, &rinfo );

	return( SUCCESS );

} /* end of net_res */

/*---------------------------- net_net           ------------------------------
*
* NAME: net_net
*
* FUNCTION:
*		displays the networks which the specified network has a route to
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
net_net(	struct nim_object *obj )

{	struct nim_attr *attr;
	struct listinfo info;
	int i;

	/* get all the ATTR_ROUTING nim_attrs for this object */
	if ( get_attr( &attr, &info, obj->id, NULL, 0, ATTR_ROUTING ) <= 0 )
		return( FAILURE );

	/* display the names of the networks this object connects to */
	for (i=0; i < info.num; i++)
	{	/* separate the net name from the rest of the info */
		if ( ! first_field( attr[i].value ) )
			continue;

		if ( display_obj( attr[i].value ) )
			pfield( ".%s.\n", ATTR_NAME_T, attr[i].value, NULL );
	}

	return( SUCCESS );

} /* end of net_net */

/*---------------------------- d_relation        ------------------------------
*
* NAME: d_relation
*
* FUNCTION:
*		displays relationships between nim_objects
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= nim_object to use as anchor for the relationship
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_relation(	struct nim_object *obj )

{	int rc=SUCCESS;
	struct nim_pdattr *pdattr;
	struct listinfo info;

	/* what is the class of objects to display a relationship with? */
	if ( subclass != NULL )
	{	if ( get_pdattr( &pdattr, &info, 0, 0, 0, subclass_num, NULL ) <= 0 )
			return( FAILURE );
		class_num = pdattr->class;
	}
	else if ( type != NULL )
	{	if ( get_pdattr( &pdattr, &info, 0, 0, 0, type_num, NULL ) <= 0 )
			return( FAILURE );
		class_num = pdattr->class;
	}

	if ( objects.num > 1 )
		fprintf( output, 
					"------------------------ %s ------------------------\n",
					obj->name );

	switch (obj->class)
	{	case ATTR_CLASS_MACHINES	:
					/* what class is the anchor? */
					switch (class_num)
					{	case ATTR_CLASS_MACHINES	:	rc = mac_mac( obj );
																break;
						case ATTR_CLASS_RESOURCES	:	rc = mac_res( obj );
																break;
						case ATTR_CLASS_NETWORKS	:	rc = mac_net( obj );
																break;
					}
					break;
		case ATTR_CLASS_RESOURCES	:
					/* what class is the anchor? */
					switch (class_num)
					{	case ATTR_CLASS_MACHINES	:	rc = res_mac( obj );
																break;
						case ATTR_CLASS_RESOURCES	:	/* no relationship to show */
																break;
						case ATTR_CLASS_NETWORKS	:	rc = res_net( obj );
																break;
					}
					break;
		case ATTR_CLASS_NETWORKS	:
					/* what class is the anchor? */
					switch (class_num)
					{	case ATTR_CLASS_MACHINES	:	rc = net_mac( obj );
																break;
						case ATTR_CLASS_RESOURCES	:	rc = net_res( obj );
																break;
						case ATTR_CLASS_NETWORKS	:	rc = net_net( obj );
																break;
					}
					break;
	}

	return( rc );

} /* end of d_relation */

/*---------------------------- d_ops             ------------------------------
*
* NAME: d_ops
*
* FUNCTION:
*		displays the NIM operations which can be applied to the specified object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj					= ptr to nim_object
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= error encountered
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_ops(	struct nim_object *obj )

{	char query[MAX_CRITELEM_LEN];
	NIM_PDATTR( pdattr, info )
	int i;
	int subclass = ATTR_SUBCLASS_OPERATION;

	if ( display_mask & DISPLAY_CONTROL_OPS )
		subclass = ATTR_SUBCLASS_CONTROL;

	/* get the operations which apply to this object */
	if ( get_pdattr(	&pdattr, &info, obj->class, subclass, 
							strtol(obj->type_Lvalue,NULL,0), 0, NULL ) <= 0 )
		return( FAILURE );

	/* display the operations */
	set_obj_name( obj->name );
	for (i=0; i < info.num; i++)
	{
		if ( pdattr[i].mask & PDATTR_MASK_DISPLAY )
			pfield("   .%s.  %s\n", ATTR_SUBCLASS_OPERATION_T, pdattr[i].name,
						ATTR_msg(pdattr[i].h_num) );
	}

	/* cleanup */
	odm_free_list( pdattr, &info );

	/* return */
	return( SUCCESS );

} /* end of d_ops */

/*---------------------------- d_basic           ------------------------------
*
* NAME: d_basic
*
* FUNCTION:
*		displays the basic info about the specified nim_object
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			obj						= ptr to nim_object
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_basic(	struct nim_object *obj )

{

	if ( ! display_obj( obj->name ) )
		return( FAILURE );

	pfield( ".%s.\t", ATTR_NAME_T, obj->name, NULL );
	pfield( "%s\t", ATTR_CLASS_T, ATTR_msg(obj->class), NULL );
	pfield( "%s\n", ATTR_TYPE_T, 
							ATTR_msg( strtol(obj->type_Lvalue,NULL,0) ), NULL );

	/* return */
	return( SUCCESS );

} /* end of d_basic */

/*---------------------------- d_objects         ------------------------------
*
* NAME: d_objects
*
* FUNCTION:
*		displays information about nim_objects
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			query					= ODM query to use when retrieving objects
*		global:
*
* RETURNS: (int)
*		SUCCESS					= x
*		FAILURE					= x
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_objects(	char *query )

{	NIM_OBJECT( obj, info )
	struct nim_pdattr pdattr;
	int i;

	/* get all the specified nim_objects */
	/* note that we don't use get_object here because get_object uses an ODM */
	/*		depth of 3; since there could be a lot of nim_objects, getting all */
	/*		of them to a depth of 3 could cause problems; therefore, we'll just */
	/*		get the first level here, then get more later if needed */
	if ( (obj = (struct nim_object *)
					odm_get_list( nim_object_CLASS, query, &info, 50, 1 )) == -1 )
		return( FAILURE );

	/* default output alignment (if not already aligned) */
	align_output( AWK_lsnim_L );

	/* display info for each nim_object */
	for (i=0; i < info.num; i++)
	{	
		/* get the pdattr corresponding to this object's type */
		sprintf( query, "attr=%s", obj[i].type_Lvalue );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
			continue;

		/* can this type be displayed? */
		if ( ! (pdattr.mask & PDATTR_MASK_DISPLAY) && ! (override_mask) )
			continue;

		/* filter based on subclass? */
		if ( ! (display_mask & DISPLAY_RELATION) && (subclass_num > 0) )
		{
			if ( ! attr_in_list( subclass_num, pdattr.subclass ) )
				continue;
		}

		/* how should we display the info? */
		if ( display_mask & DISPLAY_ATTR )
			d_specific_attr( obj+i );
		else if ( display_mask & DISPLAY_LONG )
			d_ll( obj+i );
		else if ( display_mask & DISPLAY_RELATION )
			d_relation( obj+i );
		else if (	(display_mask & DISPLAY_OPS) || 
						(display_mask & DISPLAY_CONTROL_OPS) )
			d_ops( obj+i );
		else 
			d_basic( obj+i );
	}

	/* return */
	return( SUCCESS );

} /* end of d_objects */
	
/*---------------------------- d_customized      ------------------------------
*
* NAME: d_customized
*
* FUNCTION:
*		displays NIM customized information
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
* RETURNS: (int)
*		SUCCESS					= x
*		FAILURE					= x
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_customized()

{	ODMQUERY
	NIM_OBJECT( obj, info )
	struct nim_pdattr pdattr;
	int i;

	/* initialize the query */
	query[0] = NULL_BYTE;
	if ( objects.num <= 0 )
	{	/* any filters specified? */
		if ( class != NULL )
			sprintf( query, "class=%d", class_s2i(class) );
/*?????? what about subclass ????????*/
/*????? need to form an OR ??????*/
		else if ( type != NULL )
			sprintf( query, "type='%d'", type_num );

		return( d_objects( query ) );
	}

	/* display each object specified by user */
	for (i=0; i < objects.num; i++)
	{
		sprintf( query, "name='%s'", objects.list[i] );
		printed_title = FALSE;
		d_objects( query );
	}

} /* end of d_customized */

/*---------------------------- d_op_query         ------------------------------
*
* NAME: d_op_query
*
* FUNCTION:
*		displays information about what is required in order to perform an
*			operation by invoking the corresponding method with the "-q" (query)
*			option
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls lsnim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*			operation
*			type
*			type_num
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_op_query()

{	NIM_PDATTR( pdattr, info )
	char *args[] = { NULL, "-q", NULL };
	int rc;

	/* is this operation allowed for the specified type? */
	if ( get_pdattr( &pdattr, &info, 0, 
							ATTR_SUBCLASS_OPERATION, type_num, 0, operation ) <= 0 )
		lsnim_error( ERR_OP_NOT_ALLOWED, operation, ATTR_msg(type_num), NULL );

	/* invoke the method corresponding to the specified operation */
	args[0] = pdattr->value;
	if (	(client_exec( &rc, NULL, args ) == FAILURE) ||
			(rc > 0) )
		lsnim_error( 0, NULL, NULL, NULL );

	return( SUCCESS );

} /* end of d_op_query */
	
/*---------------------------- d_operation_info   ------------------------------
*
* NAME: d_operation_info
*
* FUNCTION:
*		displays information about what is required in order to perform an
*			operation by invoking the corresponding method with the "-q" (query)
*			option
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*		calls lsnim_error
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*		global:
*			operation
*			type
*			type_num
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_operation_info()

{	ODMQUERY
	struct nim_object obj;
	int i;

	/* object name or type must be specified */
	if ( objects.num == 0 )
	{	if ( display_mask & DISPLAY_MISSING )
			lsnim_error( ERR_MISSING_OPERAND, MSG_msg(MSG_NAME), NULL, NULL );
		else if ( type == NULL )
			lsnim_error( ERR_SYNTAX, MSG_msg(MSG_LSNIM_SYNTAX), NULL, NULL );

		return( d_op_query() );
	}	
	else
	{	
		/* for each object specified by the user... */
		for (i=0; i < objects.num; i++)
		{
			/* get the object's type */
			sprintf( query, "name='%s'", objects.list[i] );
			if ( odm_get_first( nim_object_CLASS, query, &obj ) <= 0 )
				lsnim_error( ERR_DNE, objects.list[i], NULL, NULL );
			type_num = (int) strtol( obj.type_Lvalue, NULL, 0 );

			/* print title if user specified more than one object */
			if ( objects.num > 1 )
				fprintf( output, 
							"------------------------ %s ------------------------\n",
							objects.list[i] );
	
			/* print info about the operation */
			d_op_query();
		}
	}

} /* end of d_operation_info */

/*---------------------------- d_pull_list       ------------------------------
*
* NAME: d_pull_list
*
* FUNCTION:
*		displays the resources listed in <names> in a format which the NIM "pull
*			interfaces" expect, which is:
*						<object name> <object type> <comments>
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RECOVERY OPERATION:
*
* DATA STRUCTURES:
*		parameters:
*			names					= ptr to LIST
*		global:
*
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*		writes resource info to stdout in the NIM pull format
*-----------------------------------------------------------------------------*/

int
d_pull_list(	LIST *names )

{	int i;
	char query[MAX_CRITELEM_LEN];
	struct nim_object res;
	struct nim_attr comments;

	for (i=0; i < names->num; i++)
	{	/* get the resource's nim_object */
		sprintf( query, "name='%s'", names->list[i] );
		if ( odm_get_first( nim_object_CLASS, query, &res ) > 0 )
		{	
			/* skip reserved objects */
			sprintf( query, "id=%d and pdattr=%d", res.id, ATTR_RESERVED );
			if ( odm_get_first( nim_attr_CLASS, query, &comments ) > 0 )
				continue;

			/* get the ATTR_COMMENTS for this resource */
			sprintf( query, "id=%d and pdattr=%d", res.id, ATTR_COMMENTS );
			if ( odm_get_first( nim_attr_CLASS, query, &comments ) <= 0 )
				comments.value = NULL;

			/* display the info */
			fprintf( output, ".%s. %s %s\n", names->list[i], 
						ATTR_msg( strtol( res.type_Lvalue, NULL, 0 ) ),
						comments.value );
		}

	}

	return( SUCCESS );

} /* end of d_pull_list */
	
/*---------------------------- d_pull            ------------------------------
*
* NAME: d_pull
*
* FUNCTION:
*		displays resources which may be allocated to the specified machine in
*			a format which the NIM "pull resources" interface is expecting
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
* RETURNS: (int)
*		SUCCESS					= info displayed
*		FAILURE					= ODM error?
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
d_pull()

{	char query[MAX_CRITELEM_LEN];
	struct nim_object obj;
	struct nim_attr pif;
	struct nim_attr *nets;
	struct listinfo ninfo;
	struct nim_pdattr pdattr;
	LIST names;
	long id;
	int i,j;
	int indent;

	/* initialize the LIST */
	if (! get_list_space( &names, 20, TRUE ) )
		lsnim_error( 0, NULL, NULL, NULL );

	indent = ( objects.num > 1 );

	/* for each object specified by user... */
	for (j=0; j < objects.num; j++)
	{
		printed_title = FALSE;

		/* get the nim_object & its corresponding nim_pdattr */
		sprintf( query, "name='%s'", objects.list[j] );
		if ( odm_get_first( nim_object_CLASS, query, &obj ) <= 0 )
			lsnim_error( 0, NULL, NULL, NULL );
		sprintf( query, "attr=%s", obj.type_Lvalue );
		if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
			lsnim_error( 0, NULL, NULL, NULL );
	
		/* is object a machine? */
		if ( obj.class != ATTR_CLASS_MACHINES )
			lsnim_error( ERR_OBJ_CLASS,objects.list[j],ATTR_CLASS_MACHINES_T,NULL);
	
		/* get the machine's PIF */
		sprintf( query, "id=%d and seqno=1 and pdattr=%d", obj.id, ATTR_IF );
		if ( odm_get_first( nim_attr_CLASS, query, &pif ) <= 0 )
			lsnim_error( ERR_OBJ_MISSING_ATTR, PIF, objects.list[j], NULL );
	
		/* generate list of resources which reside on machine's primary network */
		if ( ! first_field( pif.value ) )
			lsnim_error(ERR_BAD_OBJECT,ATTR_CLASS_MACHINES_T,objects.list[j],NULL);
		list_res_on_net(	&names, pif.value, subclass_num, pdattr.attr, type_num);
	
		/* get all the ATTR_ROUTING associated with machine's primary network */
		if ( (id = get_id( pif.value )) > 0 )
		{	if ( get_attr( &nets, &ninfo, id, NULL, 0, ATTR_ROUTING ) > 0 )
			{	for (i=0; i < ninfo.num; i++)
				{	if ( ! first_field( nets[i].value ) )
						continue;
	
					list_res_on_net(	&names, nets[i].value, subclass_num, 
											pdattr.attr, type_num );
				}
	
				odm_free_list( nets, &ninfo );
			}
		}
	
		/* display the name ONLY if more than 1 object specified by user */
		if ( objects.num > 1 )
			fprintf( output, 
						"------------------------ %s ------------------------\n",
						objects.list[j] );
	
		/* display the info */
		d_pull_list( &names );
	
		/* free the cache */
		reset_LIST( &names );
	}

	return( SUCCESS );

} /* end of d_pull */
	
/*---------------------------- d_next_seqno      ------------------------------
*
* NAME: d_next_seqno
*
* FUNCTION:
*		displays the lowest, unused seqno for the specified attr and object
*		this information is used by the NIM smit dialogues
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
*
* OUTPUT:
*		writes the lowest, unused seqno to stdout
*-----------------------------------------------------------------------------*/

int
d_next_seqno()

{	long id;
	NIM_ATTR( attrs, ainfo )
	int i,j;
	int seqno = 1;

	/* for each object specified by the user... */
	for (j=0; j < objects.num; j++)
	{
		/* get object's id */
		if ( (id = get_id( objects.list[j] )) <= 0 )
			return( FAILURE );
	
		/* get all the attrs of the type specified by the user */
		if ( get_attr( &attrs, &ainfo, id, NULL, 0, attr_ass.list[0]->pdattr )> 0)
			while ( seqno < MAX_SEQNO )
			{
				for (i=0; i < ainfo.num; i++)
				{
					if ( seqno == attrs[i].seqno )
						break;
				}
				if ( i == ainfo.num )
					break;
				seqno++;
			}
		
		printf("%d\n", seqno );

	}

	return( SUCCESS );

} /* end of d_next_seqno */

/*---------------------------- validate_attr     ------------------------------
*
* NAME: validate_attr
*
* FUNCTION:
*		validates an attribute name and adds it to the attr_ass if it's valid
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
*			attr					= attribute name
*		global:
*			attr_ass			= array of attribute names to be displayed
*
* RETURNS: (int)
*		SUCCESS					= <attr> is valid and has been added to attr_ass
*		FAILURE					= <attr> invalid or unable to add to attr_ass
*
* OUTPUT:
*-----------------------------------------------------------------------------*/

int
validate_attr(	char *attr )

{	regmatch_t mname[ERE_ATTR_NAME_NUM];
	int seqno = 0;
	ODMQUERY
	struct nim_pdattr pdattr;

	/* appropriate attr name? (must begin with a letter) */
	if (	(regexec(	nimere[ATTR_NAME_ERE].reg, attr, 
	    					ERE_ATTR_NAME_NUM, mname, 0 ) != 0) || 
	    	(mname[1].rm_so < 0) ) 
		ERROR( ERR_VALUE, attr, MSG_msg(MSG_ATTR_NAME), NULL )

	/* seqno appended to attr name? */
	if ( mname[2].rm_eo > mname[2].rm_so ) 
	{	
		/* convert seqno into number */
		seqno = (int) strtol( attr + mname[2].rm_so, NULL, 0 );

		/* make sure seqno is between 1 and MAX_SEQNO */
		if (	(seqno < 1) || (seqno > MAX_SEQNO) )
			ERROR( ERR_BAD_SEQNO, attr, MAX_SEQNO, NULL )

		/* separate seqno from name */
		attr[ mname[1].rm_eo ] = NULL_BYTE;
	}

	/* get the predefined which corresponds to this attr */
	sprintf( query, "name=%s", attr );
	if ( odm_get_first( nim_pdattr_CLASS, query, &pdattr ) <= 0 )
	{
		/* warn the user and continue */
		warning( ERR_ATTR_NAME, attr, NULL, NULL );
		return( SUCCESS );
	}

	/* add to attr_ass */
	return( add_attr_ass( &attr_ass, pdattr.attr, attr, NULL, seqno ) );

} /* end of validate_attr */

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
*		calls lsnim_error
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

{	int rc=SUCCESS;
	extern char *optarg;
	extern int optind, optopt;
	int c;
	int syntax_err=FALSE;
	int i;

	/* loop through all args */
	while ( (c = getopt(argc, argv, "A:a:c:FlLoOpPq:s:St:Z")) != -1 )
	{	switch (c)
		{	
			case 'A': /* display the next, unused seqno for the specified attr */
				if ( validate_attr( optarg ) == FAILURE )
					lsnim_error( 0, NULL, NULL, NULL );
				anchor = d_next_seqno;
			break;

			case 'a': /* info about a specific attribute */
				if (	(class_num != 0) ||
						(subclass_num != 0) ||
						(type_num != 0) ||
						(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_OP_QUERY) ||
						(display_mask & DISPLAY_LONG) ||
						(display_mask & DISPLAY_PULL) ||
						(display_mask & DISPLAY_OPS) ||
						(display_mask & DISPLAY_CONTROL_OPS) )
					syntax_err = TRUE;
				else
				{	
					/* validate attribute name & add to attr_ass */
					if ( validate_attr( optarg ) == FAILURE )
						lsnim_error( 0, NULL, NULL, NULL );
					display_mask = display_mask | DISPLAY_ATTR;
				}
			break;

			case 'c': /* class */
				if (	(class_num != 0) ||
						(subclass_num != 0 ) ||
						(type_num != 0) ||
						(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_PULL) )
					syntax_err = TRUE;
				else
				{	/* make sure class is valid */
					if ( (class_num = class_s2i(optarg)) <= 0 )
						lsnim_error( ERR_VALUE, optarg, MSG_msg(MSG_NIM_CLASS), NULL);

					class = optarg;
				}
			break;

			case 'F': /* WARNING - this is an undocumented option */
						 /* this option causes the PDATTR_MASK_DISPLAY to be ignored*/
						 /* the end result being more info displayed than normal */
						 /* this is REALLY important for debugging purposes */
				override_mask = TRUE;
			break;

			case 'l': /* long listing */
				if (	(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_OP_QUERY) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_PULL) ||
						(display_mask & DISPLAY_OPS) ||
						(display_mask & DISPLAY_CONTROL_OPS) )
					syntax_err = TRUE;
				else
					display_mask = display_mask | DISPLAY_LONG;
			break;

			case 'L': /* listing formatted for the pull interface */
				if (	(anchor != NULL) ||
						(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_OP_QUERY) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_LONG) ||
						(display_mask & DISPLAY_OPS) ||
						(display_mask & DISPLAY_COLON) ||
						(display_mask & DISPLAY_PREDEFINED) ||
						(display_mask & DISPLAY_CONTROL_OPS) )
					syntax_err = TRUE;
				else
				{	anchor = d_pull;
					display_mask = display_mask | DISPLAY_PULL;
				}
			break;

			case 'o': /* display control operations */
				if (	(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_OP_QUERY) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_LONG) ||
						(display_mask & DISPLAY_OPS) ||
						(display_mask & DISPLAY_PULL) )
					syntax_err = TRUE;
				else
					display_mask = display_mask | DISPLAY_CONTROL_OPS;
			break;

			case 'O': /* display all operations */
				if (	(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_OP_QUERY) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_LONG) ||
						(display_mask & DISPLAY_CONTROL_OPS) ||
						(display_mask & DISPLAY_PULL) )
					syntax_err = TRUE;
				else
					display_mask = display_mask | DISPLAY_OPS;
			break;

			case 'p': /* display predefined info */
			case 'P': /* display predefined helps */
				if (	(anchor != NULL) ||
						(display_mask & DISPLAY_PULL) )
					syntax_err = TRUE;
				else
				{	anchor = d_pdinfo;
					display_mask = display_mask | DISPLAY_PREDEFINED;
					if ( c == 'P' )
						display_mask = display_mask | DISPLAY_HELPS;
				}
			break;

			case 'q': /* display required resources (query) */
				if (	(class_num != 0) ||
						(subclass_num != 0 ) ||
						(anchor != NULL) || 
						(display_mask) )
					syntax_err = TRUE;
				else
				{	operation = optarg;
					anchor = d_operation_info;
					display_mask = DISPLAY_OP_QUERY;
				}
			break;

			case 's': /* subclass */
				if (	(class_num != 0) ||
						(subclass_num != 0 ) ||
						(type_num != 0) ||
						(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_OP_QUERY) )
					syntax_err = TRUE;
				else
				{	/* make sure subclass is valid */
					if ( (subclass_num = subclass_s2i(optarg)) <= 0 )
						lsnim_error(ERR_VALUE, optarg,MSG_msg(MSG_NIM_SUBCLASS),NULL);

					subclass = optarg;
				}
				break;

			case 'S': /* display subclasses */
				if (	(class_num != 0) ||
						(subclass_num != 0 ) ||
						(type_num != 0) ||
						(display_mask & DISPLAY_MISSING) ||
						(display_mask & DISPLAY_OP_QUERY) ||
						(display_mask & DISPLAY_ATTR) ||
						(display_mask & DISPLAY_LONG) ||
						(display_mask & DISPLAY_PULL) ||
						(display_mask & DISPLAY_OPS) ||
						(display_mask & DISPLAY_CONTROL_OPS) )
					syntax_err = TRUE;
				else
					display_mask = display_mask | DISPLAY_SUBCLASSES;
				break;

			case 't': /* object type */
				if (	(class_num != 0) ||
						(subclass_num != 0 ) ||
						(type_num != 0) ||
						(display_mask & DISPLAY_MISSING) )
					syntax_err = TRUE;
				else
				{	/* make sure type is valid */
					if ( (type_num = type_s2i(optarg)) <= 0 )
						lsnim_error( ERR_VALUE, optarg, MSG_msg(MSG_NIM_TYPE), NULL );

					type = optarg;
				}
			break;

			case 'Z': /* SMIT format (colon separated output) */
				if ( display_mask & DISPLAY_PULL )
					syntax_err = TRUE;
				else
					display_mask = display_mask | DISPLAY_COLON;
			break;

			case ':': /* option is missing a required argument */
				lsnim_error( ERR_MISSING_OPT_ARG, optopt, NULL, NULL );
			break;

			case '?': /* unknown option */
				/* getopt will have printed op specific msg. */
				niminfo.errstr = MSG_msg(MSG_LSNIM_SYNTAX);
				nene(0, NULL, NULL, NULL);
				exit( ERR_BAD_OPT );
			break;
		}

		if ( syntax_err )
			lsnim_error( ERR_SYNTAX, MSG_msg(MSG_LSNIM_SYNTAX), NULL, NULL );
	}

	/* check for errors */
	if ( optind < argc )
	{	if ( display_mask & DISPLAY_PREDEFINED )
			lsnim_error( ERR_SYNTAX, MSG_msg(MSG_LSNIM_SYNTAX), NULL, NULL );
		if ( (class_num) || (subclass_num) || (type_num) )
		{	/* display relations */
			if ( display_mask & DISPLAY_LONG )
				lsnim_error( ERR_SYNTAX, MSG_msg(MSG_LSNIM_SYNTAX), NULL, NULL );
			display_mask = display_mask | DISPLAY_RELATION;
		}

		/* for each object name specified... */
		for (i=optind; i < argc; i++)
		{
			/* make sure object exists */
			if ( get_object( NULL, NULL, 0, argv[i], 0, 0 ) < 1 )
				lsnim_error( ERR_DNE, argv[i], NULL, NULL );

			/* add it to the list */
			if ( add_to_LIST( &objects, argv[i] ) == FAILURE )
				lsnim_error( 0, NULL, NULL, NULL );
		}

	}

	/* align output? */
	if ( display_mask & DISPLAY_COLON )
		aligned_yet = TRUE;
	else if ( display_mask & DISPLAY_PULL )
		align_output( AWK_lsnim_L );
	else if (	(display_mask & DISPLAY_ATTR) && 
					(display_mask & DISPLAY_PREDEFINED) )
		align_output( AWK_lsnim_pa );
	else if ( display_mask & DISPLAY_HELPS )
		align_output( AWK_lsnim_P );
	else if (	(display_mask & DISPLAY_LONG) || 
					(display_mask & DISPLAY_ATTR) ||
					(display_mask & DISPLAY_OPS) ||
					(display_mask & DISPLAY_CONTROL_OPS) )
		align_output( AWK_lsnim_l );

	/* set defaults */
	if ( anchor == NULL )
		anchor = d_customized;

	/* return */
	return( rc );

} /* end of parse_args */

/**************************       main         ********************************/

main( int argc, 
		char *argv[] )

{
	/* initialize NIM environment */
	mstr_init( argv[0], ERR_POLICY_DISPLAY, NULL );
	if ( get_list_space( &attr_ass, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		lsnim_error( 0, NULL, NULL, NULL );
	if ( get_list_space( &objects, DEFAULT_CHUNK_SIZE, TRUE ) == FAILURE )
		lsnim_error( 0, NULL, NULL, NULL );

	/* parse command line */
	parse_args( argc, argv );

	/* print requested info - note that the NULL value here is important */
	(*anchor)( NULL );

	/* finishing displaying information */
	pdone();

	exit( 0 );
}
