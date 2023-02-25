/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)22	1.5  src/tenplus/lib/obj/seq.c, tenplus, tenplus411, GOLD410 7/18/91 13:45:53";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	seq
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
 
#include <stdio.h>

/****************************************************************************/
/* seq:  string comparison routine.                                         */
/* Treats NULL as the empty string                                          */
/*                                                                          */
/* arguments:              char *s1, *s2 - the strings to be compared       */
/*                                                                          */
/* return value:           int - TRUE or FALSE to indicate equality or not      */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     seq returns TRUE if the given strings are equal.  For purposes of     */
/*     comparison, the NULL pointer is regarded as equivalent to the        */
/*     pointer pointing to NULL.  That is, seq returns TRUE if s1 is NULL    */
/*     and *s2 is NULL or vice versa, and otherwise returns TRUE iff they    */
/*     are character-by-character equivalent.                               */
/****************************************************************************/

int seq (register char *s1, register char *s2)
    {
    if ((s1 == NULL) || (*s1 == '\0'))
	return ((s2 == NULL) || (*s2 == '\0'));

    if (s2 == NULL)
	return (FALSE);

    while (*s1 == *s2)
	{
	if (*s1 == '\0') /* if we got to NULL bytes on both strings */
	    return (TRUE);

	s1++;
	s2++;
	}
    return (FALSE);
    }

