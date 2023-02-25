#if !defined(lint)
static char sccsid[] = "@(#)49	1.5  src/tenplus/lib/short/i_insert.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:06";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_insert
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
/* File:  i_insert.c - s_insert analog for SHORT * objects                  */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include "libobj.h"
#include "libshort.h"

/****************************************************************************/
/* i_insert:  insert count items before given index in ap object            */
/****************************************************************************/

short *i_insert (short *ap, int index, int count)
    {
#ifdef CAREFUL

    (void) s_typecheck (ap, "i_insert", T_ATTR);

#endif

    if (index > (obj_count (ap) - 1))
	return (i_realloc (ap, index + count));
    else
	return ((short *) s_insert ((char *)ap, index, count));
    }
