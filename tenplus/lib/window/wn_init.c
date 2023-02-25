#if !defined(lint)
static char sccsid[] = "@(#)91	1.5  src/tenplus/lib/window/wn_init.c, tenplus, tenplus411, GOLD410 7/18/91 13:49:07";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	wn_init
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
 
#include <stdio.h>
#include <database.h>
#include "window.h"

#include <libobj.h>

/****************************************************************************/
/* wn_init:  initialize window datatype.                                    */
/****************************************************************************/

void wn_init(void)
    {
    if (s_settype ("window", T_WSTRUCT, sizeof (struct window), wn_print,
      wn_read, wn_write, wn_free) == ERROR)
	fatal ("wn_init: Cannot create data type window");
    }

