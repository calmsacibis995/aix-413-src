#if !defined(lint)
static char sccsid[] = "@(#)67  1.2  src/tenplus/lib/short/i_strncmp.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:38";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:   i_strncmp
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
/* i_strncmp.c:  strncmp analog for SHORT strings                           */
/****************************************************************************/

#include <stdio.h>

int i_strncmp (short *a, short *b, int len)
{

    while(len--)
	{
	if (*a++ != *b++)
	    return(FALSE);
	}

    return (TRUE);
}

