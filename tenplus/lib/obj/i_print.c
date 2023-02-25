#if !defined(lint)
static char sccsid[] = "@(#)65	1.5  src/tenplus/lib/obj/i_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:14";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_print
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
/* i_print:  prints out an integer array                                    */
/* Does fatal error exit if passed non-int object.                          */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

extern int T_INT;

void i_print (int *ip, int indent)
    /* ip:               data pointer for integer array */
    /* indent:           indentation level for printout */
    {
    register i;       /* index into the integer array   */

#ifdef CAREFUL

    (void) s_typecheck ((char *)ip, "i_print", T_INT);

#endif

    if (g_debugfp != NULL)
	{
	for (i = 0; i < obj_count (ip); i++)
	    {
	    pindent (indent);
	    (void) fprintf (g_debugfp, "[%d] = %d\n", i, ip [i]);
	    }
	}
    }

