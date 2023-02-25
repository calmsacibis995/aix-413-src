#if !defined(lint)
static char sccsid[] = "@(#)48	1.5  src/tenplus/lib/short/i_delete.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:04";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_delete
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
/* File:  i_delete.c - s_delete analog for SHORT * objects                  */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

#include <libobj.h>

/****************************************************************************/
/* i_delete:  delete count items before given index in ap object            */
/****************************************************************************/

short *i_delete (short *ap, int index, int count)
    /*ap:       in which to delete in    */
    /*index:    where to start deletion  */
    /*count:    how many items to delete */
    {
#ifdef CAREFUL

    (void) s_typecheck (ap, "i_delete", T_ATTR);

#endif

    if (count == -1)
	count = obj_count (ap) - 1 - index;     /* one extra for null */

    return ((short *) s_delete ((char *)ap, index, count));
    }
