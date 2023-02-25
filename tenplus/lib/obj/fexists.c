#if !defined(lint)
static char sccsid[] = "@(#)57	1.5  src/tenplus/lib/obj/fexists.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:56";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fexists
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
/* fexists:  returns TRUE if a file exists, else FALSE                      */
/****************************************************************************/

#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <database.h>

int fexists (char *filename)
    {
    struct stat buf;

#ifdef TRACE

    (void) fprintf (stderr, "fexists:  called w/ filename = '%s'\n", filename);

#endif

    if (stat (filename, &buf) != ERROR)
	return (TRUE);
    else
	return (FALSE);
    }

