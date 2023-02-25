#if !defined(lint)
static char sccsid[] = "@(#)69  1.2  src/tenplus/lib/util/a2string.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:58";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   a2string
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
 
#include <database.h>
#include <libobj.h>
#include <chardefs.h>

/****************************************************************************/
/* a2string: converts an ATTR string into a char string                     */
/*                                                                          */
/* arguments:              ATTR *string - string to be converted            */
/*                         int length   - length of string to convert       */
/*                                                                          */
/* return value:           char *buffer - pointer to converted string       */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     a2string converts the given string from its internal (ATTR)          */
/*     form into the form in which it is stored in the file.  It leaves     */
/*     leading and trailing spaces intact.                                  */
/****************************************************************************/

char *a2string(ATTR *string, int length)
{
    int i=0;                /* string length counter        */
    unsigned char *buffer;  /* translation buffer           */
    unsigned char *cp;      /* pointer to buffer            */

#ifdef DEBUG
    debug ("a2string:  string = %s", string);
#endif

    if (!*string)
	return (s_string (""));

    buffer = (unsigned char *) s_alloc (2 * length, T_CHAR, NULL);
    cp = buffer;

    /* convert and transfer characters in string */
    while (*string && i++ < length)
	*cp++ = (unsigned char) de_attr(*string++);

    /* null terminate buffer */
    *cp = '\0';
    return (s_realloc ((char *)buffer, cp - buffer));
}


