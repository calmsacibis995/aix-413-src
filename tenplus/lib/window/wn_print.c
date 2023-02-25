#if !defined(lint)
static char sccsid[] = "@(#)92	1.6  src/tenplus/lib/window/wn_print.c, tenplus, tenplus411, GOLD410 3/23/93 12:09:43";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	wn_print
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
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
/* wn_print: prints one struct window                                       */
/****************************************************************************/

void wn_print (register WSTRUCT *windows, register indent)
    {
    register i;

#ifdef DEBUG
    debug("wn_print: Called");
#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)windows, "wn_print", T_WSTRUCT );

#endif

    if (g_debugfp == NULL)
	return;

    for (i = 0; i < obj_count (windows); i++)
	{
	pindent (indent);
	(void) fprintf (g_debugfp, "window [%d]:\n", i);
	_wnprint (&windows [i], indent + 5);
	}
    }

