/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)03	1.5  src/tenplus/lib/obj/s_insert.c, tenplus, tenplus411, GOLD410 7/19/91 14:49:51";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_insert
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
/* s_insert:  inserts count items before given index in a structured object */
/* Returns pointer to new object, or ERROR if there is no space left.       */
/* Does fatal exit on invalid type or negative index.  If index is off end  */
/* of array, enough items are added to have (index + count) items in array. */
/*                                                                          */
/*        index  old         new                                            */
/*        -----  ---         ---                                            */
/*              *****       *****                                           */
/*          0   * a *       * a *                                           */
/*              *****.      ***** -----         insertion of three items    */
/*          1   * b * .     *   *   h           before index two in an      */
/*              *****  .    *****   o           object with five items      */
/*          2   * c *   .   *   *   l                                       */
/*              *****.   .  *****   e                                       */
/*          3   * d *     . *   *                                           */
/*              *****  x   .***** -----                                     */
/*          4   * e *   f   * b *                                           */
/*              *****.   e  ***** <-- original source (for xfer)            */
/*          5         .   r * c *                                           */
/*                     .    *****                                           */
/*          6           .   * d *                                           */
/*                       .  *****                                           */
/*          7             . * e *                                           */
/*                         .***** <-- original destination (for xfer)       */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

char *s_insert (char *cp, int index, int count)
    /* cp:                    pointer to data area of object   */
    /* index:                 index to insert items before     */
    /* count:                 number of items to insert        */
    {
    register char *newcp;       /* new data pointer                 */
    register size;              /* size of items for datatype       */
    register char *source;      /* used for xfering and clearing    */
    register char *destination; /*   the data area of new object    */
    register nbytes;            /* number of bytes to xfer or clear */
    register type;              /* object datatype                  */

#ifdef TRACE

    debug ("s_insert:  called w/ cp = 0%o, index = %d, count = %d\n",
	   cp, index, count);

#endif

#ifdef CAREFUL

    (void) s_typecheck (cp, "s_insert", T_ANY); /* make sure it's valid   */

    if (index < 0)
	fatal ("s_insert:  negative index (%d)", index);

#endif

    if (index > obj_count (cp)) /* if index off end of array */
	return (s_realloc (cp, index + count));

    /* Note:  the use of 'type' is necessary because of a compiler bug */

    type = obj_type (cp);
    size = dt_size (&g_dtypes [type]);          /* item size */
    nbytes = (obj_count (cp) - index) * size;  /* number of bytes to xfer */

    newcp = s_realloc (cp, obj_count (cp) + count);

    if (newcp == (char *) ERROR) /* return ERROR on allocation failure */
	return ((char *) ERROR); /* Note:  g_errno already set         */

    /* transfer the data down to make room for inserted items    */
    /* the destination is a pointer to the last data byte.       */
    /* the source is the destination minus the size of the hole. */

    destination = newcp + (obj_count (newcp) * size) - 1;
    source = destination - (count * size); /* subtract hole size to get src */

    while (nbytes--)
	*destination-- = *source--;

    /* zero out the inserted items */

    nbytes = count * size;                /* size of hole    */
    destination = newcp + (index * size); /* pointer to hole */

    while (nbytes--)
	*destination++ = '\0';

    return (newcp);
    }

