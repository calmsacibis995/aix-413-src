#if !defined(lint)
static char sccsid[] = "@(#)77	1.5  src/tenplus/lib/obj/p_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:49";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	p_print
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
/* p_print:  prints out the contents of a pointer object                    */
/* Does falal error exit if called with non-POINTER object.                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void p_print (register POINTER *pointers, register indent)
    /*pointers: data pointer for pointer array   */
    /*indent:            indentation level for printout   */
    {
    register i;                 /* index into the pointer array     */

#ifdef CAREFUL

    (void) s_typecheck ((char *)pointers, "p_print", T_POINTER);

#endif

    if (g_debugfp != NULL)
	{
	for (i = 0; i < obj_count (pointers); i++)
	    {
	      pindent (indent);
	      (void) fprintf (g_debugfp, "pointer [%d] at 0%o\n", i, pointers [i]);

	      if (pointers [i]) /* ignore null pointers */
		  s_print (pointers [i]); /* print out what pointer is */
					  /* pointing at */ 
	    }
	}
    }

