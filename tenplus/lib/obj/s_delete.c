/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)92	1.5  src/tenplus/lib/obj/s_delete.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:28";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_delete
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
/* s_delete:  deletes count items at a given index in a structured object   */
/* Returns pointer to new object, or ERROR if there is no space left.       */
/* A count of minus one means delete the rest of the data array.            */
/* Does fatal error exit on invalid pointer, negative index, index off data */
/* array, or count too large.                                               */
/*                                                                          */
/*        index  old         new                                            */
/*        -----  ---         ---                                            */
/*              *****       *****                                           */
/*          0   * a *       * a *                                           */
/*              *****      .*****               deletion of three items     */
/*          1   *   *     . * b *               starting at index two in    */
/*              *****    .  *****               an object with seven items  */
/*          2   *   *   .   * c *                                           */
/*              *****  .    *****                                           */
/*          3   *   * .     * d *                                           */
/*              *****.   r  *****                                           */
/*          4   * b *   e   * e *                                           */
/*              *****  f   .*****                                           */
/*          5   * c * x   .                                                 */
/*              *****    .                                                  */
/*          6   * d *   .                                                   */
/*              *****  .                                                    */
/*          7   * e * .                                                     */
/*              *****.                                                      */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

char *s_delete (char *cp, int index, int count)
    /* cp                           pointer to data area of object   */
    /* index                        index to delete items at         */
    /* count                        number of items to delete        */
    {
    register size;               /* size of items for datatype       */
    register char *source;       /* used for xfering data area       */
    register char *destination;  /* used for xfering data area       */
    register nbytes;             /* number of bytes to xfer          */
    register type;               /* object datatype                  */

#ifdef TRACE

    debug ("s_delete:  called w/ cp = 0%o, index = %d, count = %d\n",
	     cp, index, count);

#endif

#ifdef CAREFUL

    (void) s_typecheck (cp, "s_delete", T_ANY); /* make sure its valid */

    /* make sure count is legal */

    if (((index + count) > obj_count (cp)) || (count < -1) || (count == 0))
	fatal ("s_delete:  illegal count (%d)", count);

#endif

    /* If count is minus one, set it to the number of items after index. */

    if (count == -1)
	count = obj_count (cp) - index;

    /* the number of bytes to transfer is the number of items after */
    /* the deleted region times the size of the items               */

    /* Note:  the use of 'type' is necessary because of a compiler bug */

    type = obj_type (cp);
    size = dt_size (&g_dtypes [type]);                 /* item size */
    nbytes = (obj_count (cp) - index - count) * size; /* no. of bytes to xfer */

    /* transfer the data down to remove the deleted items       */
    /* the destination is the first byte of item index          */
    /* the source is the destination plus the size of the hole. */

    destination = cp + (index * size);
    source = destination + (count * size); /* add hole size to get source */

    while (nbytes--)
	*destination++ = *source++;

    /* Note that s_realloc will return the right error if necessary */

    return (s_realloc (cp, obj_count (cp) - count));
    }

