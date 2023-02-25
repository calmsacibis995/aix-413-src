/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)88	1.5  src/tenplus/lib/obj/s_addtype.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:17";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_addtype
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
/* s_addtype:  adds new datatype to g_dtypes array.                         */
/* Returns new datatype number.  Does a fatal error if table is full        */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

static next_free_slot = MAX_TYPES - 1; /* index into table of the next slot
					  which may be available */

int s_addtype (char *name, int size, void (*printer)(void *, int), 
               int (*reader)(void *, FILE *), 
               int (*writer)(void *, FILE *), void (*freer)(void *))
    /* name:        data type name                             */
    /* size:        size of object                             */
    /* printer:     debugging routine to print object contents */
    /* reader:      routine for reading object from file       */
    /* writer:      routine for writing object to file         */
    /* freer:       routine for freeing data in object         */
    {
    register DTYPE *dtp; /* pointer to new table slot */

    dtp = &g_dtypes [next_free_slot];

    /* if the next slot isn't free or its reserved then fatal out */
    if (dt_name (dtp) != NULL || next_free_slot < RESERVED_TYPES)
	fatal ("s_addtype:  no more datatype slots available");

    dtp->dt__name    = name;      /* stuff entries in slot */
    dtp->dt__size    = size;
    dtp->dt__printer = printer;
    dtp->dt__reader  = reader;
    dtp->dt__writer  = writer;
    dtp->dt__freer   = freer;

    return (next_free_slot--); /* return index and bump counter */
    }

