#if !defined(lint)
static char sccsid[] = "@(#)05	1.5  src/tenplus/lib/obj/s_newname.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:57";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_newname
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
/* s_newname:  gives a structured allocation a new name                     */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include <stdlib.h>
#include <libobj.h>

void s_newname (char *cp, char *name)
    /* cp:                    pointer to data area of object */
    /* name:                  new name for object            */
    {
    register SHEADER *shp; /* salloc pointer for object      */

#ifdef TRACE

    debug ("s_newname:  called w/ cp = 0%o, name = '%s'\n", cp, name);

#endif

#ifdef CAREFUL

    (void) s_typecheck (cp, "s_newname", T_ANY);

#endif

    shp = obj_shp (cp);

    if (sh_name (shp) != NULL)
	free (sh_name (shp));

    shp->sh__name = scopy (name);
    }

