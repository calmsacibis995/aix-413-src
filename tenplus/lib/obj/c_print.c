#if !defined(lint)
static char sccsid[] = "@(#)46	1.5  src/tenplus/lib/obj/c_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:28";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	c_print
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
/* c_print:  prints out a character object                                  */
/* Does fatal error exit if passed non-CHAR object.                         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void c_print (register char *cp, register indent)
      /* data pointer for character array */
      /* indentation level for printout   */
    {

#ifdef CAREFUL

    (void) s_typecheck (cp, "c_print", T_CHAR);

#endif

    if (g_debugfp != NULL)
	{
	pindent (indent);
	(void) fprintf (g_debugfp, "'%s'\n", cp);
	}
    }

