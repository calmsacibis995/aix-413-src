/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)94	1.5  src/tenplus/lib/obj/s_fixtree.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:32";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_fixtree
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
/* s_fixtree:  updates a node pointer in a tree                             */
/* Returns TRUE if oldnode found, else FALSE                                */
/* Returns TRUE if oldnode equals newnode in any case.                      */
/* Does reverse search if G_REVERSE flag is set                             */
/* Does not typecheck either old or new node.                               */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

int s_fixtree (POINTER *tree, char *oldnode, char *newnode, int flag)
    /* tree:                           tree pointer        */
    /* oldnode:                        node to be replaced */
    /* newnode:                        new node pointer    */
    /* flag:                           stop after one hit  */
    {
    register i;             /* index into pointer array    */
    register start;         /* start value for loop index  */
    register end;           /* end value for loop index    */
    register delta;         /* increment for loop index    */
    register found;         /* TRUE if oldnode found       */

#ifdef TRACE

   debug ("s_fixtree:  tree = 0%o, oldnode = 0%o, newnode = 0%o, flag = %s\n",
	  tree, oldnode, newnode, flag ? "TRUE" : "FALSE");

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)tree, "s_fixtree (tree)", T_POINTER);

#endif

    if (oldnode == newnode) /* if no change, do nothing */
	return (TRUE);

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
    found = FALSE;

    for (i = start; i != end; i += delta) /* for all top level pointers */
	if (tree [i] == oldnode) /* if they match the old node */
	    {
	    tree [i] = newnode; /* update the pointer */
	    found = TRUE;

	    if (flag) /* if flag set, stop on first hit */
		return (TRUE);
	    }
	else /* else, recurse using pointer as tree */
	    if ((tree [i] != NULL) &&  /* don't recurse on NULL pointers */
		(obj_type (tree [i]) == T_POINTER)) /* or non-pointer arrays */
		if (s_fixtree ((POINTER *)tree [i], oldnode, newnode, flag))
		    {
		    found = TRUE; /* set found flag for this level */

		    if (flag) /* if flag set, stop on first hit */
			return (TRUE);
		    }
    return (found);
    }

