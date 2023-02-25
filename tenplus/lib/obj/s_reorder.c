/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)14	1.5  src/tenplus/lib/obj/s_reorder.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:19";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_reorder
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
/* s_reorder:  reorders children in POINTER array using a name list         */
/*                                                                          */
/* Algorithm:                                                               */
/*                                                                          */
/*   This routine goes through the children in the array, looking to see    */
/*   if their name in in the name list.  Children on that list are moved    */
/*   to the front of the POINTER array.  At any point in the processing,    */
/*   the array contains a set of children in the list, followed by a set    */
/*   of children out of the list, followed by the children that haven't     */
/*   been tested yet If the child being tested is in the list, the routine  */
/*   swaps that point with the first pointer that is not in the list.  It   */
/*   keeps track of the boundary between the first two sets with the        */
/*   "firstout" local and the local variable "i" is the index of the child  */
/*   being tested:                                                          */
/*                                                                          */
/*       -------------------------------------------------------------      */
/*       | children w/ names | children w/ names | children w/ names |      */
/*       | in the list       | not in the list   | that need testing |      */
/*       -------------------------------------------------------------      */
/*                           ^                   ^                          */
/*                           firstout            i                          */
/*                                                                          */
/*   If all the names in the list match the names of children in the array  */
/*   it returns. Otherwise it adds the missing children to the front of the */
/*   array.  Returns a pointer to the sorted array.                         */
/****************************************************************************/

#include    <string.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    "database.h"
#include    <libobj.h>

POINTER *s_reorder (POINTER *record, POINTER *names)
    {
    register i;         /* child index           */
    register j;         /* names index           */
    register firstout;  /* index of first out    */
    register char *tmp; /* used to swap pointers */

#ifdef DEBUG
    debug ("s_reorder:  record = 0%o, names = 0%o", record, names);
#endif

#ifdef CAREFUL
    (void) s_typecheck ((char *)record, "s_reorder (record)", T_ANY);
    (void) s_typecheck ((char *)names, "s_reorder (names)", T_POINTER);
#endif

    if (obj_type (record) != T_POINTER)
	/* return */ exit(2) /* exit here instead of return,
                                because code using this function
                                does not check for error return */;

    firstout = 0;

    for (i = 0; i < obj_count (record); i++)
	if (record [i] && s_inlist (obj_name (record [i]), names))
	    {
	    if (i != firstout)
		{
		tmp = record [firstout];
		record [firstout] = record [i];
		record [i] = tmp;
		}
	    firstout++;
	    }
    if (firstout >= obj_count (names))
	return (record);

    for (i = 0; i < obj_count (names); i++)
	{
	for (j = 0; j < firstout; j++)
	    if (record [j] && strcmp(obj_name (record [j]), names [i]) == 0)
		break;

	if (j >= firstout) /* if this name not in record */
	    {
	    record = (POINTER *) s_insert ((char *)record, firstout, 1);
	    record [firstout++] = s_alloc (0, T_POINTER, names [i]);
	    }
	}
    return (record);
    }

