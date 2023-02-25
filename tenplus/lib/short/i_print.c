#if !defined(lint)
static char sccsid[] = "@(#)54	1.5  src/tenplus/lib/short/i_print.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:12";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sht_print
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
/* sht_print:  prints out an integer array                                    */
/* Does fatal error exit if passed non-int object.                          */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"
#include <libobj.h>


void sht_print (register short *ip, register indent)
   /*ip:          data pointer for integer array */
   /*indent:      indentation level for printout */
    {
    register i;       /* index into the integer array   */
    register m;       /* value of i % 3 at each step    */

#ifdef CAREFUL

    (void) s_typecheck (ip, "sht_print", T_ATTR);

#endif

    /* This is not really the right place to test g_debugfp,
       but for the moment the only things printing are SHORTs anyway. */

    if (! g_debugfp)
	return;

    for (i = 0; i < obj_count (ip); i++)
	{
	m = i % 4;

	pindent (m ? 4 : indent);       /* indent by 4 unless m is 0 */
	(void) fprintf (g_debugfp, "[%4d] =%3d'%c'",
	    i, (ip [i] >> 8), (ip [i] & 0377));
	if (m == 3)
	    (void) fprintf (g_debugfp, "\n");
	}

    if (i % 4)  /* need final return unless we just did it */
	(void) fprintf (g_debugfp, "\n");
    }
