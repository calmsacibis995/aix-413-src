#if !defined(lint)
static char sccsid[] = "@(#)55	1.5  src/tenplus/lib/obj/fatal.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:50";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fatal
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
/* fatal:  prints out a message, closes the terminal and does an error exit */
/* If G_COREDUMP bit set in g_flags does an abort exit                      */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"
#include    <stdarg.h>
#include    <stdlib.h>

void fatal (char *format, ...)
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    va_start(ap, format);
    if (g_debugfp != NULL)
	{
        arg1 = va_arg(ap, char *);
        arg2 = va_arg(ap, char *);
        arg3 = va_arg(ap, char *);
        arg4 = va_arg(ap, char *);
        arg5 = va_arg(ap, char *);

	(void) fprintf (g_debugfp, format, arg1, arg2, arg3, arg4, arg5);
	(void) fputc ('\n', g_debugfp);
	(void) fflush (g_debugfp);
        va_end(ap);

	if (g_getflag (G_COREDUMP)) /* get core dump if flag is set */
	    (void) abort ();
	else
	    exit (ERROR);
	}
    va_end(ap);
    }

