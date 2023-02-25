/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)01	1.5  src/tenplus/lib/obj/s_indxnm.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:46";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_indexname
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
/* s_indexname:  appends index name of child in parent to a string          */
/* Helper routine for s_pathname.  Does fatal error if child not in parent  */
/* Does reverse search if G_REVERSE flag is set                             */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

void s_indexname (char *string, POINTER *parent, char *child)
    /* string:                      place to put index name */
    /* parent:                      parent pointer array    */
    /* child:                       child node pointer      */
    {
    register i;               /* index                      */
    register start;           /* start value for loop index */
    register end;             /* end value for loop index   */
    register delta;           /* increment for loop index   */

#ifdef TRACE

    debug ("s_indexname called w/ parent = 0%o, child = 0%o\n",
	   parent, child);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)parent, "s_indexname (parent)", T_POINTER);
    (void) s_typecheck ((char *)child, "s_indexname (child)", T_ANY);

#endif

    if (g_getflag (G_REVERSE)) /* if doing reverse search */
	{
	start = obj_count (parent) - 1;
	end = -1;
	delta = -1;
	}
    else
	{
	start = 0;
	end = obj_count (parent);
	delta = 1;
	}
    for (i = start; i != end; i += delta)  /* for all children */
	if (parent [i] == child)             /* found child ?    */
	    {
	    (void) sprintf (string, "%d", i); /* convert index to ascii */
	    return;
	    }
    fatal ("s_indexname:  child (0%o) not in parent (0%o)", child, parent);
    }

