#if !defined(lint)
static char sccsid[] = "@(#)69	1.5  src/tenplus/lib/obj/l_addtype.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:26";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	l_addtype
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
/* l_addtype:  adds long object type                                        */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int T_LONG;

void l_addtype (void)
    {
    T_LONG = s_addtype ("long", sizeof (long), l_print, l_read, l_write, noop);
    }

