/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)15	1.6  src/tenplus/lib/obj/s_settype.c, tenplus, tenplus411, GOLD410 3/23/93 12:06:03";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_settype
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
/* s_settype:  adds new datatype to g_dtypes array at indicated position    */
/* Returns success (RET_OK) or failure (ERROR)                              */
/****************************************************************************/

#include <string.h>
#include <stdio.h>
#include "database.h"

int s_settype (char *name, int position, int size, void (*printer)(void *, int),
              int (*reader)(void *, FILE *), int (*writer)(void *, FILE *), 
              void (*freer)(void *))

    /*name:       data type name                             */
    /*position:   position in g_dtypes array                 */
    /*size:       size of object                             */
    /*printer:    debugging routine to print object contents */
    /*reader:     routine for reading object from file       */
    /*writer:     routine for writing object to file         */
    /*freer:      routine for freeing data in object         */
    {
    register DTYPE *dtp; /* pointer to new table slot */

#ifdef CAREFUL
    if (position >= MAX_TYPES || position <= 0)
	return (ERROR);
#endif

    dtp = &g_dtypes [position];

    /* if something already in desired slot, and its not what we are
       trying to put there, return an error condition */
    if ((dt_name (dtp) != NULL) && (! strcmp(dt_name (dtp), name) == 0))
	return (ERROR);

    dtp->dt__name    = name;      /* stuff entries in slot */
    dtp->dt__size    = size;
    dtp->dt__printer = printer;
    dtp->dt__reader  = reader;
    dtp->dt__writer  = writer;
    dtp->dt__freer   = freer;

    return (RET_OK);
    }

