#if !defined(lint)
static char sccsid[] = "@(#)80	1.5  src/tenplus/lib/obj/pindent.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:59";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	pindent
 * 
 * ORIGINS:  9, 10, 27
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
/* pindent:  prints count characters on g_debugfp                           */
/****************************************************************************/

#include    <stdio.h>
#include <chardefs.h>
extern FILE  *g_debugfp;    /* debugging file pointer      */

void pindent (register count)
     /* number of times to write char */
   { 
    if (g_debugfp != NULL)
	while (count-- > 0)
	    (void) fputc (SPACE, g_debugfp);
    }

