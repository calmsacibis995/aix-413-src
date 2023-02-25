#if !defined(lint)
static char sccsid[] = "@(#)59	1.5  src/tenplus/lib/obj/g_dtype.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:03";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
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
/* g_dtype:  global definitions which pertain directly to the global type   */
/* descriptor table.                                                        */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"
#include    <libobj.h>

/***** global type descriptor table *****/

/* each entry contains:                                                    */
/*     char *dt__name;          data type name                             */
/*     int   dt__size;          size of object                             */
/*     int (*dt__printer)();    debugging routine to print object contents */
/*     int (*dt__reader)();     routine for reading object from file       */
/*     int (*dt__writer)();     routine for writing object to file         */
/*     int (*dt__freer)();      routine for freeing data in object         */

DTYPE g_dtypes [MAX_TYPES] =
    {
    "freed",     0,                 e_print, e_read, e_write, e_free,
    "pointer",   sizeof (POINTER),  p_print, p_read, p_write, p_free,
    "record",    sizeof (RECORD),   r_print, r_read, r_write, noop,
    "character", sizeof (char),     c_print, c_read, c_write, noop,
    "sfile",     sizeof (SFILE),    sf_print, e_read, e_write, sf_free,
    };
