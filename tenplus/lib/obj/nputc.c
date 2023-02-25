/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)75	1.5  src/tenplus/lib/obj/nputc.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:42";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	nputc
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
/* nputc:  puts out a character on a file pointer a given number of times   */
/****************************************************************************/

#include    <stdio.h>

void nputc (char c, FILE *fp, int count)
    /* c:          character to output           */
    /* fp:         file pointer to write on      */
    /* count:      number of times to write char */
    {
    while (count-- > 0)
	(void) putc (c, fp);
    }

