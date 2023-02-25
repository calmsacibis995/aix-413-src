/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)81	1.5  src/tenplus/lib/util/pathcat.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:43";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	pathcat
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

#include <string.h> 
#include <stdio.h>
#include <database.h>

#include <libobj.h>

/****************************************************************************/
/* pathcat: takes two alloced strings and concatenates them with a slash    */
/*          in between. If either string is 0 length, the slash is omitted. */
/*          If s1 is "/" "/s2" is returned. The result is returned as a     */
/*          structured allocation.                                          */
/****************************************************************************/

char *pathcat (register char *s1, register char *s2)
    {
#ifdef DEBUG
    debug ("pathcat:  s1 = '%s', s2 = '%s'", s1, s2);
#endif

    if ((s1 == NULL) || (*s1 == '\0'))
	return (s_string (s2));

    if ((s2 == NULL) || (*s2 == '\0'))
	return (s_string (s1));

    if (strcmp (s1, "/") == 0) /* watch out for the root as first arg */
	return (s_cat (s1, s2));

    return (s_cat (s_cat (s1, s_string ("/")), s2));
    }

