#if !defined(lint)
static char sccsid[] = "@(#)81	1.5  src/tenplus/lib/obj/puti.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:02";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	puti
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
/* puti:  put 2 byte integer onto supplied file pointer                     */
/* Writes bytes in high/low order to file.                                  */
/****************************************************************************/

#include    <stdio.h>

void puti (int i, FILE *fp)
    /* i:          integer to write         */
    /* fp:         file pointer to write on */
    {
    (void) fputc ((i >> 8) & 0xff, fp); /* high byte */
    (void) fputc (i & 0xff, fp);        /* low byte  */
    }

