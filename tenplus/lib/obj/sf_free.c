#if !defined(lint)
static char sccsid[] = "@(#)26	1.5  src/tenplus/lib/obj/sf_free.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:06";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_free
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
/* sf_free:  frees up data area of sfile object                             */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void sf_free (register SFILE *sfp)
    /*sfp:  data pointer for sfile object   */
    {
#ifdef TRACE

      debug ("sf_free:  called on sfile '%s'\n", obj_name (sfp));

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_free", T_SFILE);

    if (obj_count (sfp) != 1)
	fatal ("sf_free:  count (%d) not one", obj_count (sfp));

#endif

    s_free ((char *)sf_records (sfp)); /* free records array in sfile object */
    }

