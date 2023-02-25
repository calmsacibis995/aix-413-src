/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)33	1.5  src/tenplus/lib/obj/sf_print.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:18";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_print
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
/* sf_print:  prints out the contents of a sfile object                     */
/* Does fatal error exit if called with a non-SFILE object                  */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void sf_print (register SFILE *sfp, register indent)
    /*sfp:     data pointer for sfile object  */
    /*indent:  indentation level for printout */
    {
#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_print", T_SFILE);

    if (obj_count (sfp) != 1)
        fatal ("sf_print:  count (%d) not one", obj_count (sfp));

#endif

    if (g_debugfp != NULL) 
	{
	pindent (indent);
	(void) fprintf (g_debugfp,
       "fp = 0%o, index = %d, eofloc = %ld uid = %d, gid = %d, time = %ld\n",
		 sf_fp (sfp), sf_index (sfp), sf_eofloc (sfp),
		 sf_uid (sfp), sf_gid (sfp), sf_time (sfp));
	s_print ((char *)sf_records (sfp));
	}
    }

