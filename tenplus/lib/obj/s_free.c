#if !defined(lint)
static char sccsid[] = "@(#)96	1.5  src/tenplus/lib/obj/s_free.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:34";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_free
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
/* s_free: free up structured allocations                                   */
/* Does nothing if passed a NULL pointer.                                   */
/* Does fatal error exit if passed invalid pointer.                         */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include <stdlib.h>
#include <libobj.h>

void s_free (char *cp)
     /* pointer to data area of structured allocation */
    {
    register SHEADER *shp;    /* pointer to structured allocation */
    register void (*freer) (void *); /* free function for the datatype   */

#ifdef TRACE

    debug ("s_free:  called w/ cp = 0%o\n", cp);

#endif

    if (cp == NULL) /* do nothing if pointer is NULL */
	return;

#ifdef CAREFUL

    (void) s_typecheck (cp, "s_free", T_ANY); /* make sure type is valid */

#endif

    shp = obj_shp (cp); /* generate salloc pointer */

    freer = (void(*)(void *))dt_freer (&g_dtypes [sh_type (shp)]); /* free data area of object */

    if (freer == NULL)
	freer = (void(*)(void *))e_free;

    (*freer) (cp);

    if (sh_name (shp) != NULL) /* if name is non-NULL, free the name */
	free (sh_name (shp));

    shp->sh__type = T_FREED;  /* mark object as invalid */
    free (shp);              /* free object            */
    }

