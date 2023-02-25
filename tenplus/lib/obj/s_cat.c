/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)91	1.5  src/tenplus/lib/obj/s_cat.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:25";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_cat
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
/* s_cat:  concatinates two structured allocations into one                 */
/* The old allocations are freed.  Returns ERROR on allocation error.       */
/* Handles NULL object pointers.                                            */
/* Does fatal error exit if the two objects are not of the same type        */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

char *s_cat (char *obj1, char *obj2)
    /*obj1, obj2:           pointer to objects          */
    {
    register char *source;      /* used for xfering data       */
    register char *destination; /* used for xfering data       */
    register nbytes;            /* number of bytes to xfer     */
    register count1;            /* number of items in obj1     */
    register char *newcp;       /* pointer to new object       */
    register size;              /* object item size            */
    register type1;             /* datatype of object 1        */
    register type2;             /* datatype of object 2        */

#ifdef TRACE

    debug ("s_cat:  called w/ obj1 = 0%o, obj2 = 0%o\n", obj1, obj2);

#endif

    if (obj1 == NULL) /* handle NULL object pointers */
	return (obj2);

    if (obj2 == NULL)
	return (obj1);

#ifdef CAREFUL

    (void) s_typecheck (obj1, "s_cat (obj1)", T_ANY);
    (void) s_typecheck (obj2, "s_cat (obj2)", T_ANY);

#endif

    type1 = obj_type (obj1); /* get datatypes */
    type2 = obj_type (obj2);

    if (type1 != type2)
	fatal ("s_cat:  obj1 of type '%s' and obj2 is of type '%s'",
	       dt_name (&g_dtypes [type1]), dt_name (&g_dtypes [type2]));

    count1 = obj_count (obj1); /* remember number of items in object 1 */
    newcp = s_realloc (obj1, count1 + obj_count (obj2));

    if (newcp == (char *) ERROR)
	return ((char *) ERROR); /* Note:  g_errno already set */

    /* xfer data from obj2 to new object */

    size = dt_size (&g_dtypes [type1]);
    source = obj2;
    destination = newcp + (size * count1);
    nbytes = size * obj_count (obj2);

    while (nbytes-- > 0)
	*destination++ = *source++;

    s_freenode (obj2);       /* free up obj2 without recursion */
    return (newcp);
    }

