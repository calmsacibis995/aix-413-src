#if !defined(lint)
static char sccsid[] = "@(#)47	1.5  src/tenplus/lib/short/i_cat.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:02";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_cat
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
/* File:  i_cat.c - s_cat annalog for SHORT objects                         */
/* needed because of overallocation of SHORT objects to be null-terminated  */
/****************************************************************************/

#include <stdio.h>
#include <database.h>

#include <libobj.h>

/****************************************************************************/
/* i_cat:  concatenate two SHORT objects.                                   */
/*                                                                          */
/* This differs from standard s_cat because the first argument must have    */
/* its null terminating element removed before calling standards s_cat.     */
/****************************************************************************/

short *i_cat (short *a, short *b)
    /*a:    to concatenate onto */
    /*b:    to concatenate - FREED BY s_cat!!! */
    {
    if (a == NULL)
	return (b);

    if (b == NULL)
	return (a);

#ifdef CAREFUL

    (void) s_typecheck (a, "s_cat (a)", T_ATTR);
    (void) s_typecheck (b, "s_cat (b)", T_ATTR);

#endif

    a = (short *) s_delete ((char *)a, obj_count (a) - 1, -1);  /* remove NULL */
    return ((short *) s_cat ((char *) a, (char *) b));
    }
