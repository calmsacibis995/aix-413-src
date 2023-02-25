/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)11	1.5  src/tenplus/lib/obj/s_print.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:10";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_print
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
/* s_print:  prints out a structured allocation.                            */
/* Does fatal error exit if passed invalid pointer.                         */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

void s_print (char *cp)
    /* cp:  pointer to data area of structured allocation */
    {
    register DTYPE *dtp;        /* pointer to datatype descriptor   */
    register void (*printer) (void *, int); /* pointer to datatye print routine */
    static indent = 0;          /* current indentation level        */

#ifdef TRACE

    debug ("s_print:  called w/ cp = 0%o\n", cp);

#endif

#ifdef CAREFUL

    /* s_typecheck is not used because we want to allow type zero */

    if (((signed)(obj_type (cp)) < 0) || (obj_type (cp) >= MAX_TYPES) || 
      (dt_name (g_dtypes + obj_type (cp)) == NULL))
	fatal ("s_print:  illegal type (%d)\n", obj_type (cp));

#endif

    dtp = &g_dtypes [obj_type (cp)]; /* get datatype pointer */
    printer = (void(*)(void *, int))dt_printer (dtp);      /* and print function   */

    if (printer == NULL)
	printer = (void(*)(void *, int))e_print;

    if (g_debugfp != NULL)
	{
	pindent (indent);

	(void) fprintf (g_debugfp,
		 "salloc of name '%s', type '%s', count %d, flag 0%o:\n",
		 obj_name (cp) ? obj_name (cp) : "NULL",
		 dt_name (dtp), obj_count (cp), obj_flag (cp));
	}

    indent += 5; /* increase indentation level for printout of array */
    (*printer) (cp, indent); /* call datatype specific function */
    indent -= 5; /* restore indentation level */
    }

