#if !defined(lint)
static char sccsid[] = "@(#)85	1.5  src/tenplus/lib/util/treecopy.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:51";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	treecopy
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* File: treecopy.c - copy a structured allocation composed of objects      */
/*   of standard type.                                                      */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

#include <libobj.h>

/****************************************************************************/
/* treecopy: produce a duplicate of a structured allocation.                */
/*   Returns ERROR for invalid type.                                        */
/****************************************************************************/

POINTER * treecopy( POINTER *tree )
{
    if (tree == NULL)
	return( NULL );

    (void) s_typecheck( (char *)tree, "treecopy", T_ANY );

    if (obj_type( tree ) == T_CHAR) {
	char *str;

	str = s_string((char *)tree);
	s_newname( str, obj_name( tree ));
	obj_setflag( str, obj_flag( tree ) );
	return( (POINTER *) str );
    }

    if (obj_type( tree ) == T_POINTER ) {
	register int      i;
	register POINTER *newtree;

	newtree = (POINTER *) s_alloc( obj_count( tree ), T_POINTER,
				       obj_name( tree ) );
	s_newname( (char *)newtree, obj_name(tree));
	obj_setflag( newtree, obj_flag( tree ) );

	for (i = 0; i < obj_count (tree); i++)
	    newtree[i] = (POINTER) treecopy((POINTER *)tree[i]);

	return( newtree );
    }

    return( (POINTER *) ERROR ); /* not T_CHAR or T_POINTER */
}
