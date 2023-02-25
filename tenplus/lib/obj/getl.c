#if !defined(lint)
static char sccsid[] = "@(#)61	1.5  src/tenplus/lib/obj/getl.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:07";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	getl
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
/* getl: get long from supplied file pointer                                */
/****************************************************************************/

#include    <stdio.h>
#include  <database.h>
#include <libobj.h>

long getl (FILE *fp)
    /* fp:   file pointer to read from */
    {
    long hi;            /* high word of long         */
    long low;           /* low word of long          */
    long l;             /* value of long             */

    hi = geti (fp);
    low = geti (fp);
    l = (hi & 0xffffL) << 16;
    l = l | (low & 0xffffL);
    return (l);
    }

