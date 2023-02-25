#if !defined(lint)
static char sccsid[] = "@(#)52	1.5  src/tenplus/lib/obj/e_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:42";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	e_print
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
/* e_print:  print routine for object of type zero (invalid)                */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

/*ARGSUSED*/
void e_print (register char *cp, register indent)
      /* data pointer for invalid array */
     /* indentation level for printout */
    {
    if (g_debugfp != NULL)
	{
	pindent (indent);
	(void) fprintf (g_debugfp,
		 "e_print:  attempt to print an invalid datatype\n");
	}  
    }

