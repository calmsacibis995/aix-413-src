#if !defined(lint)
static char sccsid[] = "@(#)54	1.5  src/tenplus/lib/obj/e_write.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:47";
#endif /* lint */
  
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	e_write
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
/* e_write:  print routine for object of type zero (invalid)                */
/* Does falal error exit if called.                                         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void e_write (void)
    {
    fatal ("e_write:  attempt to write an invalid datatype");
    }

