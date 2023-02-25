#if !defined(lint)
static char sccsid[] = "@(#)07	1.5  src/tenplus/lib/obj/s_path.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:00";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_path
 * 
 * ORIGINS:  9, 10, 27
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
/* s_path:  finds the path of a node in the tree                            */
/* Returns an salloc'ed pointer array to all the nodes in the path from     */
/* the tree to the node, or ERROR if the node is not in the tree.           */
/* Does reverse search if G_REVERSE flag is set                             */
/* Handles trees with NULL pointers in them.                                */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

POINTER *s_path (register POINTER *tree, register char *node)
    /*tree:      tree pointer                */
    /*node:      node to find                */
    {
    register i;                 /* index into pointer array    */
    register start;            /* start value for loop index       */
    register end;              /* end value for loop index         */
    register delta;            /* increment for loop index         */
    register POINTER *path;     /* path array pointer          */

#ifdef TRACE

    debug ("s_path:  tree = 0%o, node = 0%o\n", tree, node);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)tree, "s_path (tree)", T_ANY);
    (void) s_typecheck (node, "s_path (node)", T_ANY);

#endif

    /* if node is root of tree, return pointer to tree */

    if ((POINTER *) node == tree)
	return (s_pointer ((char *)tree));

    g_errno = E_MISC; /* all errors in this routine are miscellaneous */

    if (obj_type (tree) != T_POINTER)
	return ((POINTER *) ERROR);

    if (g_getflag (G_REVERSE)) /* if doing reverse search */
	{
	start = obj_count (tree) - 1;
	end = -1;
	delta = -1;
	}
    else
	{
	start = 0;
	end = obj_count (tree);
	delta = 1;
	}
    for (i = start; i != end; i += delta) /* for all children */
	{
	if (tree [i] == NULL) /* watch out for NULL pointers */
	    continue;

	path = s_path ((POINTER *)tree [i], node); /* search for node in child */

	if (path != (POINTER *) ERROR)
	    return (s_prefix (path, (char *)tree));
	}
    return ((POINTER *) ERROR); /* node not in children */
    }

