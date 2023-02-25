#if !defined(lint)
static char sccsid[] = "@(#)82	1.5  src/tenplus/lib/obj/putl.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:04";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	putl
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
/* putl: put long onto supplied file pointer                                */
/****************************************************************************/

#include  <stdio.h>
#include  <database.h>
#include  <libobj.h>

void putl (long l, FILE *fp)
    /* l:              long value to output     */
    /* fp:             file pointer to write on */
    {
    register i;
    i = (l >> 16) & 0xffff;
    puti (i, fp);
    i = l & 0xffff;
    puti (i, fp);
    }

