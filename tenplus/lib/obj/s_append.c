#if !defined(lint)
static char sccsid[] = "@(#)90	1.5  src/tenplus/lib/obj/s_append.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:23";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_append
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
/* s_append:  appends a new pointer to a pointer array                      */
/* Returns ERROR on allocation failure.  Accepts NULL child.                */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

POINTER *s_append (register POINTER *parent, register char *child)
    /*parent:  parent pointer array */
    /*child :  child to append      */
    {
    register POINTER *newnode; /* new parent node      */
    register count;            /* size of parent       */

#ifdef TRACE

    debug ("s_append called w/ parent = 0%o, child = 0%o\n", parent, child);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)parent, "s_append (parent)", T_POINTER);

    if (child != NULL)
	(void) s_typecheck (child, "s_append (child)", T_ANY);

#endif

    /* insert space for child */

    count = obj_count (parent);
    newnode = (POINTER *) s_realloc ((char *)parent, (unsigned)count + 1);

    if (newnode == (POINTER *) ERROR)
	return ((POINTER *) ERROR); /* Note:  g_errno already set */

    newnode [count] = child;
    return (newnode);
    }

