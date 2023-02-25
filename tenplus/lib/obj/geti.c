#if !defined(lint)
static char sccsid[] = "@(#)60	1.5  src/tenplus/lib/obj/geti.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:05";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	geti
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
/* geti:  gets a 2 byte integer from supplied file pointer                  */
/* Reads bytes in high/low order                                            */
/****************************************************************************/

#include    <stdio.h>


int geti (FILE *fp)
    /* fp:         file pointer to read from */
    {
    register i;

    i = fgetc (fp) << 8;
    i |= fgetc (fp);
    return (i);
    }

