#if !defined(lint)
static char sccsid[] = "@(#)62	1.5  src/tenplus/lib/short/i_strlen.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:33";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_strlen
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
/* File: i_strlen.c - strlen analog for SHORT objects                       */
/****************************************************************************/

#include <database.h>

/****************************************************************************/
/* i_strlen:  find length of short string by looking for null terminator    */
/****************************************************************************/

int i_strlen (short *s)
    {
    register count;

    count = 0;
    while (*s++)
	count++;

    return (count);
    }
