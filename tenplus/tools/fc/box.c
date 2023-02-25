#if !defined(lint)
static char sccsid[] = "@(#)64	1.5  src/tenplus/tools/fc/box.c, tenplus, tenplus411, GOLD410 7/18/91 13:55:59";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	bx_init, bx_free, bx_print
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
/* File: box.c - object functions for the box datatype.                     */
/****************************************************************************/

#include    <stdio.h>
#include <chardefs.h>
#include    <database.h>
#include    "box.h"
#include    <libobj.h>


int T_BOX;            /* DTYPES type for boxes */

static void bx_print(BOX *, int);
static void bx_free(BOX *[] );


/****************************************************************************/
/* bx_init: initialize the box data type.                                   */
/****************************************************************************/

void bx_init(void)
{
    T_BOX = s_addtype( "box", sizeof( BOX ), (void(*) (void *, int))bx_print, 
                       (int (*)(void *, FILE *))noop, 
                       (int (*)(void *, FILE *))noop, 
                       (void (*)(void *))bx_free );
}


/****************************************************************************/
/* bx_free: frees the allocated parts of a struct box.                      */
/****************************************************************************/

static void
bx_free(BOX *boxes [] )
{
    register int  i;
    register BOX *bp;

    for (i = 0; i < obj_count( boxes ); i++) {
	bp = (BOX *) &boxes[i];


	if (bp->pickname != NULL)
	    s_free( bp->pickname );

	if (bp->prefix != NULL)
	    s_free( bp->prefix );

	if (bp->suffix != NULL)
	    s_free( bp->suffix );

	if (bp->tablin != NULL)
	    s_free( bp->tablin );

	if (bp->zoom != NULL)
	    s_free( bp->zoom );
    }
}


/****************************************************************************/
/* bx_print: print out a box                                                */
/****************************************************************************/

static void
bx_print(BOX * bp, int indent )
{
    if (g_debugfp != NULL)
    {
	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp,
		       "...(top = %d, left = %d), (bot = %d, right = %d)\n",
		       bp->top, bp->left, bp->bot, bp->right );

	if (bp->gangp) {
	    nputc( SPACE, g_debugfp, indent );
	    (void) fprintf( g_debugfp,
			   "points to ganged box at 0%o\n", bp->gangp ); 
	}

	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp, "bits = 0%o\n", bp->bits );
	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp, "prefix = '%s', suffix = '%s'\n",
		       bp->prefix, bp->suffix );

	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp, "left margin = %d, right margin = %d\n",
		       bp->lmarg, bp->rmarg );
	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp, "tabs = '%s'\n", bp->tablin );

	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp, "zoom form = '%s'\n", bp->zoom );

	nputc( SPACE, g_debugfp, indent );
	(void) fprintf( g_debugfp, "pickname = '%s', isboxed = %d\n",
		       bp->pickname, bp->isboxed );
    }
}

