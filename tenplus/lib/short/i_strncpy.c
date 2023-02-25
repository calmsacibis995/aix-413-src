#if !defined(lint)
static char sccsid[] = "@(#)63	1.5  src/tenplus/lib/short/i_strncpy.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:41";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_strncpy
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
/* i_strncpy.c:  strncpy analog for SHORT strings                           */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

/****************************************************************************/
/* i_strncpy:  strncpy analog                                               */
/****************************************************************************/

short *i_strncpy (short *a, short *b, int n)
    {           /** CAVEAT:  no checking at all! **/
    int i;

    for (i = 0; i < n; i++)
	a[i] = b[i];

    return (a);
    }

