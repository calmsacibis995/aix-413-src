#if !defined(lint)
static char sccsid[] = "@(#)50	1.5  src/tenplus/lib/obj/e_free.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:40";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	e_free
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
/* e_free:  print routine for object of type zero (invalid)                 */
/* Does fatal error exit if called.                                         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void e_free (void)
    {
    fatal ("e_free:  attempt to free an invalid datatype");
    }

