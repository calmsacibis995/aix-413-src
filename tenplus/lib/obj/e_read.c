#if !defined(lint)
static char sccsid[] = "@(#)53	1.5  src/tenplus/lib/obj/e_read.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:44";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	e_read
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
/* e_read:  print routine for object of type zero (invalid)                 */
/* Does falal error exit if called.                                         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void e_read (void)
    {
    fatal ("e_read:  attempt to read an invalid datatype");
    }

