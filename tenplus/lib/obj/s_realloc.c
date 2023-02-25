/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)13	1.5  src/tenplus/lib/obj/s_realloc.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:15";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_realloc
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
/* s_realloc: reallocate a structured allocation to be n items long, where  */
/* n may be bigger or smaller than the original n. Returns a pointer to the */
/* reallocated object, which may or may not be the same as the supplied     */
/* pointer, or ERROR if there is no space available.  If the buffer has to  */
/* be moved, the old one is marked as freed (if the header for the old buff */
/* is not inside the new buffer).                                           */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include <stdlib.h>
#include <libobj.h>

char *s_realloc (char *cp, unsigned newcount)
    /* cp:              pointer to old data area */
    /* newcount:        new count of data items  */
    {
    register SHEADER *oldshp; /* old salloc pointer       */
    register SHEADER *newshp; /* new salloc pointer       */
    register i;               /* used to transfer data    */
    register char *newdata;   /* pointer to new data area */
    register unsigned newlength;       /* new length of data area  */
    register unsigned oldlength;       /* old length of data area  */
    register oldcount;        /* old count of data items  */
    register type;            /* salloc data type         */

#ifdef TRACE

    debug ("s_realloc:  called w/ cp = 0%o, newcount = %d\n",
	   cp, newcount);

#endif

#ifdef CAREFUL

    (void) s_typecheck (cp, "s_realloc", T_ANY); /* make sure datatype is valid */

    if ((signed)newcount < 0)
	fatal ("s_realloc:  negative count (%d)", newcount);

#endif

    if (newcount > OBJSIZE)
	fatal ("s_realloc: size of structured object exceeded %d.", OBJSIZE);

    oldshp = obj_shp (cp);

    newlength = dt_size (&g_dtypes [sh_type (oldshp)]) * newcount;

    oldcount = sh_count (oldshp);    /* so we can clear the new real estate */
    oldlength = dt_size (&g_dtypes [sh_type (oldshp)]) * oldcount;

    type = sh_type (oldshp);      /* save type                  */
    oldshp->sh__type = T_FREED;   /* mark old buffer as invalid */

    /* add size of header to request plus an extra byte     */
    /* the extra byte at the end of the data is set to NULL */
    /* Note that realloc will copy the name, type and data  */

    newshp = (SHEADER *) realloc ((char *)oldshp, (size_t)(newlength + HEADERSIZE + 1));

    if (newshp == NULL) /* watch for allocation failure */
	{
	if (g_allocfcn == NULL) /* if no special alloc failure routine */
	    fatal ("s_realloc:  allocation failure");

	(*g_allocfcn) (); /* call user's allocation failure routine */
	g_errno = E_ALLOC;
	oldshp->sh__type = type; /* restore old type */
	return ((char *) ERROR);
	}
    newshp->sh__type = type;      /* restore type in object  */
    newshp->sh__count = newcount; /* put new count in object */
    newdata = sh_data (newshp);
    newdata [newlength] = '\0'; /* zero the extra byte after the data area */

    /* zero out new data are if object has grown */

    for (i = oldlength; i < newlength; i++)
	newdata [i] = '\0';

    return (sh_data (newshp));
    }


