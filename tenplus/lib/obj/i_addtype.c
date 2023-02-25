/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)64	1.5  src/tenplus/lib/obj/i_addtype.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:12";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_addtype
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
/* i_addtype:  adds int object type                                         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int T_INT;

void i_addtype (void)
    {

    T_INT = s_addtype ("int", sizeof (int), i_print, i_read, i_write, noop);
    }

