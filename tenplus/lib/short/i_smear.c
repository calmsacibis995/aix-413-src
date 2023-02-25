#if !defined(lint)
static char sccsid[] = "@(#)58	1.5  src/tenplus/lib/short/i_smear.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:25";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_smear
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
/* File:  i_smear.c - SHORT * analog to regular string smear                */
/****************************************************************************/

#include <database.h>

/****************************************************************************/
/* i_smear:  smear given SHORT character in the string                      */
/****************************************************************************/

void i_smear (short ch, int count, short *addr)
    {
    while (count--)
	*addr++ = ch;
    }
