/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)70	1.5  src/tenplus/lib/obj/l_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:29";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	l_print
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
/* l_print:  prints out an array of longs                                   */
/* Does fatal error exit if passed non-long object.                         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

extern int T_LONG;

void l_print (register long *lp, register indent)
    /*lp:  data pointer for integer array */
    /*indent:    indentation level for printout */
    {
    register i;         /* index into the integer array   */

#ifdef CAREFUL

    (void) s_typecheck ((char *)lp, "l_print", T_LONG);
 
#endif

    if (g_debugfp != NULL)
	{
	for (i = 0; i < obj_count (lp); i++)
	    {
	    pindent (indent);
	    (void) fprintf (g_debugfp, "[%d] = %ld\n", i, lp [i]);
	    }
	}
    }

