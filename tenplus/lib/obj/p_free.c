#if !defined(lint)
static char sccsid[] = "@(#)76	1.5  src/tenplus/lib/obj/p_free.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:46";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	p_free
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
/* p_free:  frees up the data area of a pointer object                      */
/* Does falal error exit if called with non-POINTER object.                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

void p_free (register POINTER *pointers)
    /*pointers:  data pointer for pointer array   */
    {
    register i;                 /* index into the pointer array     */

#ifdef CAREFUL

    (void) s_typecheck((char *)pointers, "p_free", T_POINTER);

#endif

    for (i = 0; i < obj_count (pointers); i++)
	if (pointers [i])
	    s_free (pointers [i]);
    }

