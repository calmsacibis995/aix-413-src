#if !defined(lint)
static char sccsid[] = "@(#)18	1.5  src/tenplus/lib/obj/s_string.c, tenplus, tenplus411, GOLD410 7/18/91 13:45:38";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_string
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
/* s_string:  converts a string into a salloc'ed character array            */
/* Returns pointer to character array containing string, or ERROR on        */
/* allocation failure.                                                      */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <string.h>
#include <libobj.h>

char *s_string (char *string)
     /* string to make into a char array */
    {
    register char *cp;

    cp = s_alloc (strlen (string), T_CHAR, NULL);

    if (cp == (char *) ERROR)
	return ((char *) ERROR); /* Note:  g_errno already set */

    (void) strcpy (cp, string);
    return (cp);
    }

