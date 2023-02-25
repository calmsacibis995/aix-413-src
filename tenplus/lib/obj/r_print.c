#if !defined(lint)
static char sccsid[] = "@(#)83	1.5  src/tenplus/lib/obj/r_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:07";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	r_print
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
/* r_print:  prints out the contents of a record object                     */
/* Does falal error exit if called with a non-RECORD object                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void r_print (register RECORD *records, register indent)
    /*records: data pointer for record array  */
    /*indent :      indentation level for printout */
    {
    register i;               /* index into record array        */

#ifdef CAREFUL

    (void) s_typecheck ((char *)records, "r_print", T_RECORD);

#endif

    if (g_debugfp != NULL)
	{
	for (i = 0; i < obj_count (records); i++)
	    {
	    pindent (indent);
	    (void) fprintf (g_debugfp, "record [%d] is ", i);

	    if (r_fileloc (&records [i]) == NORECORD)
		(void) fprintf (g_debugfp, "NULL\n");
	    else
		(void) fprintf (g_debugfp, "at location %ld\n",
			 r_fileloc (&records [i]));
	    }
	}
    }

