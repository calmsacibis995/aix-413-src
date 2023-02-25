#if !defined(lint)
static char sccsid[] = "@(#)67	1.6  src/tenplus/tools/fc/fc.c, tenplus, tenplus411, GOLD410 3/23/93 12:18:05";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, error, Efatal, Eerror
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/****************************************************************************/
/* File: fc.c: functions used only by fc and not by the helper.             */
/****************************************************************************/

#include    <stdio.h>
#include    <stdarg.h>
#include    <stdlib.h>
#include    <locale.h>

#include    "chardefs.h"
#include    "database.h"
#include    "window.h"
#include    "libshort.h"
#include    "libobj.h"
#include    "fc.h"
#include    "box.h"

int MultibyteCodeSet;

/****************************************************************************/
/* main: analyze arguments and call compile.                                */
/****************************************************************************/

main(int argc, char *argv[] )
{
    int i;

    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
#ifdef DEBUG
    g_debugfp = fopen( "fc.out", "w" );
    if (g_debugfp == NULL) {
	g_debugfp = stderr;
	error( "Unable to open debugging output file" );
    }
#endif

    /* Initialize data types */
    i_addtype();
    (void)wn_init();
    (void)bx_init();

    if (argc < 2) {
	error( "Usage: %s input-file ...", argv[0] );
	(void)exit( -1 );
    }

    for (i = 1; i < argc; i++) {

#ifdef DEBUG
	debug( "fc about to compile \"%s\"", argv[i] );
#endif

	g_filename = s_string( argv[i] );
	(void) compile();
	(void)s_free( g_filename );
    }

    (void)exit( 0 );
    return(0);
    /*NOTREACHED*/
}

/****************************************************************************/
/* error: meant to be ifdef'ed with call to ederrf, and such.               */
/****************************************************************************/

void error(char *format, ... )
{
    char *a1; char *a2; char *a3; char *a4; char *a5 ;
    va_list ap;

    va_start(ap, format);
    a1 = va_arg(ap, char *);
    a2 = va_arg(ap, char *);
    a3 = va_arg(ap, char *);
    a4 = va_arg(ap, char *);
    a5 = va_arg(ap, char *);

    (void) fprintf( stderr, format, a1, a2, a3, a4, a5 );
    va_end(ap);

    (void) putc( '\n', stderr );
}

/****************************************************************************/
/* Efatal: special version for the fc executable.                           */
/****************************************************************************/

void Efatal(  char *format, ... )
{
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5 ;
    va_list ap;

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);

    fatal( format, arg1, arg2, arg3, arg4, arg5 );
    va_end(ap);
}

/****************************************************************************/
/* Eerror: special version for the fc executable. (same as Efatal for fc)   */
/****************************************************************************/

void Eerror(  char *format, ... )
{
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5 ;
    va_list ap;

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);

    fatal( format, arg1, arg2, arg3, arg4, arg5 );
    va_end(ap);
}
