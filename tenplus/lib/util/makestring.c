/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)77	1.5  src/tenplus/lib/util/makestring.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:30";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	makestring
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
#include <database.h>
#include <libobj.h>

/****************************************************************************/
/* makestring: make alloced copy of string, checking for errors             */
/*             N.B. -- if s is NULL, makestring returns NULL!               */
/*                                                                          */
/* arguments:              char *s - string to be copied                    */
/*                                                                          */
/* return value:           char *  - allocated and checked copy, or NULL    */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given string is NULL, return NULL (not, beware, an allocated  */
/*     null string).  Otherwise return an allocated copy of the string.     */
/****************************************************************************/

char *makestring (char *s)
    {
    register char * retval;

    if (s == NULL)
	return NULL;

    retval = s_string (s);
    return retval;
    }

