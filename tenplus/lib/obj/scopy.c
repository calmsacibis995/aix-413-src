/*LINTLIBRARY*/
#ifndef lint
#if !defined(lint)
static char sccsid[] = "@(#)21	1.5  src/tenplus/lib/obj/scopy.c, tenplus, tenplus411, GOLD410 7/18/91 13:45:50";
#endif /* lint */
#endif

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	scopy
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
/* scopy:  returns an allocated copy of a string                            */
/* Returns NULL if string is NULL.                                          */
/* Returns ERROR on allocation failure                                      */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"
#include    <stdlib.h>
#include    <string.h>
#include    <libobj.h>

char *scopy (char *oldstring)
    /*oldstring: string to copy */
    {
    register char *newstring;

    if (oldstring == NULL)
	return (NULL);

    newstring = malloc (strlen (oldstring) + 1);

    if (newstring == NULL) /* watch for allocation failure */
	{
	if (g_allocfcn == NULL) /* if no special alloc failure routine */
	    fatal ("scopy:  allocation failure");

	(*g_allocfcn) (); /* call user's allocation failure routine */
	g_errno = E_ALLOC;
	return ((char *) ERROR);
	}
    (void) strcpy (newstring, oldstring);
    return (newstring);
    }

