#if !defined(lint)
static char sccsid[] = "@(#)74	1.5  src/tenplus/lib/obj/noop.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:38";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	noop
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
/* noop:                                                                    */
/*   Some functions (eg. i_addtype) take function pointers as arguments.    */ 
/*   noop is passed to these functions when no 'real' function is           */
/*   required, ie. as a kind of NULL function.                              */ 
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"


void noop (void)
    {
    }
