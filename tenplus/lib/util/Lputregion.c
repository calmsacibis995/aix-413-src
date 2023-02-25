/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)69	1.5  src/tenplus/lib/util/Lputregion.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:53";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Lputregion
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
 
/****************************************************************************/
/* File:  Lputregion.c                                                      */
/****************************************************************************/

#include <stdio.h>

/****************************************************************************/
/* Lputregion:  write region coordinates into a string.                     */
/* Returns pointer to STATIC string - should not be freed!                  */
/****************************************************************************/

char *Lputregion (int ulline, int ulcol, int lrline, int lrcol)
    /* ulline:         upper left hand line number    */
    /* ulcol:          upper left hand column number  */
    /* lrline:         lower right hand line number   */
    /* lrcol:          lower right hand column number */
    {
    static char regionstring [50];    /* Lputregion output value */

    (void) sprintf (regionstring, "%d %d %d %d", ulline, ulcol, lrline, lrcol);
    return (regionstring);
    }
