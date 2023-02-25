#if !defined(lint)
static char sccsid[] = "@(#)02	1.5  src/tenplus/lib/obj/s_inlist.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:49";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_inlist
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
/* s_inlist:  determines if a child is in the list                          */
/* Takes a string and a POINTER array that points to strings and returns    */
/* TRUE if the given string is in the list, else NO.                         */
/****************************************************************************/

#include    <string.h>
#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int s_inlist (register char *string, register POINTER *list)
    /*string:   string to search for */
    /*list:     list of strings      */
    {
    register i; /* string list index */

#ifdef DEBUG
    debug ("inlist:  string = '%s', list = 0%o", string, list);
#endif

#ifdef CAREFUL
    (void) s_typecheck ((char *)list, "s_inlist (list)", T_POINTER);
#endif

    for (i = 0; i < obj_count (list); i++)
	if (list [i] && strcmp(list [i], string) == 0)
	    return (TRUE);

    return (FALSE);
    }
