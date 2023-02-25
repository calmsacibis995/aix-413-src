#if !defined(lint)
static char sccsid[] = "@(#)79	1.5  src/tenplus/lib/util/p2string.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:39";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	p2string
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
#include <edexterns.h>

/*****************************************************************************
* p2string:  converts a POINTER array into a string w/ '\n' as line breaks   *
* Returns an s_alloc'ed string.                                              *
*****************************************************************************/

char *p2string (register POINTER *pointer)
    {
    register i;             /* loop counter              */
    register char *string;  /* string from pointer array */
    register length;        /* length of return string   */
    register char *retstr;  /* return string             */

#ifdef DEBUG
    debug ("p2string:  pointer = '%s'", pointer);
#endif

    (void) s_typecheck ((char *)pointer, "p2string (pointer)", T_POINTER);
    length = 0;

    for (i = 0; i < obj_count (pointer); i++)
	{
	string = pointer [i];

	if (string)
	    length += strlen (string);
	}
    if (obj_count (pointer) > 1)
	length += obj_count (pointer) - 1; /* make room for \n chars */

    retstr = s_alloc (length, T_CHAR, NULL);

    for (i = 0; i < obj_count (pointer); i++)
	{
	string = pointer [i];

	if (i > 0) /* put in line break if necessary */
	    (void) strcat (retstr, "\n");

	if (string)
	    (void) strcat (retstr, string);
	}
#ifdef DEBUG
    debug ("p2string:  length = %d, retstr = '%s'", length, retstr);
#endif
    return (retstr);
    }


