#if !defined(lint)
static char sccsid[] = "@(#)49	1.5  src/tenplus/lib/obj/debug.c, tenplus, tenplus411, GOLD410 7/19/91 14:47:38";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	debug
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
 
#include <stdio.h>
#include <stdarg.h>

/****************************************************************************/
/* debug:  debugging version of printf; output to "po" file                 */
/*                                                                          */
/* arguments:      (variable)    char * b - printf-style format descriptor  */
/*                               char * c,d,e,f,g,h,i - zero or more extra  */
/*                                                 printf arguments         */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_debugfp                                        */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If g_debugfp is NULL, do nothing.  Otherwise use fprintf to write    */
/*     the given debug message onto the g_debugfp file, append a newline,   */
/*     and flush it so that we are sure the messages really gets there.     */
/****************************************************************************/

void debug (char * b, ...) 
    {
    va_list ap;
    char * c; char * d; char * e; char * f; char * g; char * h; char *  i;
    extern FILE *g_debugfp;

    va_start(ap, b);
    if (g_debugfp)
	{
        c = va_arg(ap, char *);
        d = va_arg(ap, char *);
        e = va_arg(ap, char *);
        f = va_arg(ap, char *);
        g = va_arg(ap, char *);
        h = va_arg(ap, char *);
        i = va_arg(ap, char *);

	(void) fprintf (g_debugfp, b, c, d, e, f, g, h, i);
	(void) fputc ('\n', g_debugfp);
	(void) fflush (g_debugfp);
	}
    va_end(ap);
    }
