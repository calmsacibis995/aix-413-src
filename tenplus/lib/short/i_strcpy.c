#if !defined(lint)
static char sccsid[] = "@(#)59	1.5  src/tenplus/lib/short/i_strcpy.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:28";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_strcpy
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
/* File:  i_strcpy.c:  strcpy analog for SHORTs                             */
/****************************************************************************/

#include <database.h>

/****************************************************************************/
/* i_strcpy:  copy one SHORT into another                                   */
/****************************************************************************/

void i_strcpy (short *s, short *t)
    {
    while (*s++ = *t++)
	;
    }
