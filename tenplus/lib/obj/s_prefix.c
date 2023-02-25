#if !defined(lint)
static char sccsid[] = "@(#)10	1.5  src/tenplus/lib/obj/s_prefix.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:08";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_prefix
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
/* s_prefix:  prefixs a new pointer to a pointer array                      */
/* Returns ERROR on allocation failure                                      */
/* Accepts a NULL child pointers.                                           */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

POINTER *s_prefix (register POINTER *parent, register char *child)
    /*parent:   parent pointer array */
    /*child:    child to prefix      */
    {
    register POINTER *newnode; /* new parent node      */

#ifdef TRACE

    debug ("s_prefix called w/ parent = 0%o, child = 0%o\n",
	   parent, child);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)parent, "s_prefix (parent)", T_POINTER);

    if (child != NULL)
	(void) s_typecheck (child, "s_prefix (child)", T_ANY);

#endif

    newnode = (POINTER *) s_insert ((char *)parent, 0, 1); /* insert space for child */

    if (newnode == (POINTER *) ERROR)
	return ((POINTER *) ERROR); /* Note:  g_errno already set */

    *newnode = child;
    return (newnode);
    }

