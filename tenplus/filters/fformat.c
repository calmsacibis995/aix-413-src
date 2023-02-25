#if !defined(lint)
static char sccsid[] = "@(#)63	1.5  src/tenplus/filters/fformat.c, tenplus, tenplus411, GOLD410 7/18/91 12:38:14";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, init, clean
 * 
 * ORIGINS:  9, 10
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
 
/* =======================================================================
 *
 *  format.c        -   main, initialization, and cleanup routines for format.
 *
 * =======================================================================
 */

#include "fdefines.h"
#include "fexterns.h"
#include "filters.h"
#include <stdlib.h>
#include <locale.h>

char	sobuf[BIGBLKSIZ];	/* for use in buffered standard output */
char * bigblock = NULL ;    /* NULL if we don't have it; otherwise addr-of */
int MultibyteCodeSet;

int main( int argc, char **argv )
{
    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;

    init() ;
    cmd_line( argc, argv ) ;
    (void)parse_drive() ;
    clean() ;
    return(0); /* return put here to make lint happy */
}   /* main() */

/* ============================== init() ================================
 *
 *  This routine performs various initializations for format; it must be
 *      called after cmd_line()
 *  --  Enables buffered output
 *
 * =====================================================================
 */
void init(void )
{
    setbuf( stdout, sobuf ) ;           /* go for buffered standard output */

    /* now try to get the big-block */
    bigblock = xalloc( BIGBLKSIZ ) ;

}   /* end init()  */



void clean(void)
{
    /* why bother freeing the big-block */
    if (input != NULL) 
      (void) fclose( input ) ;
    exit( EXITOK ) ;
}
