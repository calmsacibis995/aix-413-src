#if !defined(lint)
static char sccsid[] = "@(#)09	1.5  src/tenplus/lib/obj/s_pointer.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:05";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_pointer
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
/* s_pointer:  returns a one item pointer array with a given child          */
/* Returns ERROR on allocation failure                                      */
/* Accepts NULL child pointer.                                              */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

POINTER *s_pointer (register char *child)
     /* child to put into array */
    {
    register POINTER *parent; /* new parent for child    */

#ifdef TRACE

    debug ("s_pointer called w/ child = 0%o\n", child);

#endif

#ifdef CAREFUL

    if (child != NULL)
	(void) s_typecheck (child, "s_pointer", T_ANY);

#endif

    parent = (POINTER *) s_alloc (1, T_POINTER, NULL);

    if (parent == (POINTER *) ERROR)
	return ((POINTER *) ERROR); /* Note:  g_errno already set */

    *parent = child;
    return (parent);
    }

