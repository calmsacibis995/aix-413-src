/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)68	1.5  src/tenplus/lib/util/s2pointer.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:46";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s2pointer
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
 
#include <stdio.h>
#include <database.h>
#include <string.h>
#include <libobj.h>

/****************************************************************************/
/* s2pointer:  convert string with \r's in it to pointer array              */
/*                                                                          */
/* arguments:              char *string - to be converted to array          */
/*                                                                          */
/* return value:           POINTER * - the corresponding pointer array      */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     s2pointer returns a pointer array corresponding to the given string, */
/*     with array elements delimited by \r characters.  The \r characters   */
/*     themselves are not included at the end of the array lines.  Treats   */
/*     '\n' like '\r'.                                                      */
/****************************************************************************/

POINTER *s2pointer (char *string)
    /*string;            message to put in box       */
    {
    register indx;         /* list index                  */
    register char *cp;      /* pointer to next \r          */
    register char *start;   /* pointer to string           */
    register count;         /* number of lines             */
    register POINTER *list; /* message as POINTER array    */
    register char *text;    /* text of line                */

#ifdef DEBUG
    debug ("s2pointer:  string = '%s'", string);
#endif

    count = 1; /* get number of lines */

    for (cp = string; *cp; cp++)
	if ((*cp == '\r') || (*cp == '\n'))
	    count++;

    list = (POINTER *) s_alloc (count, T_POINTER, (char *) NULL);
    start = string;

    for (indx = 0; ; indx++)
	{
	for (cp = start; *cp; cp++)
	    if ((*cp == '\r') || (*cp == '\n'))
		break;

	if (*cp == '\0') /* if no more return chars, use rest of string */
	    {
	    list [indx] = s_string (start);
	    break;
	    }
	text = s_alloc (cp - start, T_CHAR, (char *) NULL);
	(void) strncpy (text, start, cp - start);
	list [indx] = text;

	start = cp + 1; /* set path to first character after return */
	}
#ifdef DEBUG
/*    objprint ("s2pointer (list)", list);
 */
#endif

    return (list);
    }

