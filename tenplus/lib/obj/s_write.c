/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)20	1.6  src/tenplus/lib/obj/s_write.c, tenplus, tenplus411, GOLD410 3/23/93 12:06:24";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_write
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* s_write:  writes a structured allocation to a file                       */
/*                                                                          */
/* The disk format for a structured allocation is:                          */
/*                                                                          */
/*   1 byte   type                                                          */
/*   1 byte   flag                                                          */
/*   1 byte   length of name (n)                                            */
/*   n bytes  name                                                          */
/*   2 bytes  count (number of items)                                       */
/*                                                                          */
/*   this is followed by the data for each of the count items.  The format  */
/*   of these items is determined by the read and write routines for the    */
/*   datatype involved.  This may involve recursive calls to s_write in the */
/*   case of POINTER sallocs                                                */
/*                                                                          */
/*   If the flag byte is zero, the name is NULL, and the count is less than */
/*   255, an abreviated version of this format is used with only the type   */
/*   byte (with the high bit on) and a one byte count.                      */
/*                                                                          */
/* A NULL pointer is represented by a NULL byte in the file.                */
/*                                                                          */
/* Does fatal error exit if passed an invalid pointer.                      */
/* Returns ERROR on I/O error, else RET_OK.                                 */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include <string.h>
#include <libobj.h>

int s_write (char *cp, FILE *fp)
    /* cp:                        pointer to object data area     */
    /* fp:                        file pointer to write object on */
    {
    register length;           /* length of name                  */
    register type;             /* object datatype                 */
    register count;            /* object item count               */
    register int (*writer) (void *, FILE *); /* write function for the datatype */


#ifdef TRACE

    debug ("s_write:  called w/ cp = 0%o, fp = 0%o\n", cp, fp);

#endif

    if (cp == NULL) /* handle NULL pointers */
        {
	(void) fputc ('\0', fp);
	return (RET_OK);
        }
#ifdef CAREFUL

    (void) s_typecheck (cp, "s_write", T_ANY); /* make sure it is a valid pointer */

#endif

    /* if object has a name, get length */

    length = (obj_name (cp) != NULL) ? strlen (obj_name (cp)) : 0;
    type = obj_type (cp);
    count = obj_count (cp);

    if ((obj_flag (cp) == 0) && (length == 0) && (count <= 255))
        {
        (void) fputc (type | 0x80, fp); /* use abreviated form of header */
        (void) fputc (count, fp);
        }
    else
        {
        (void) fputc (type, fp);          /* one byte of salloc type */
        (void) fputc (obj_flag (cp), fp); /* one byte of flag bits   */
        (void) fputc (length, fp);        /* one byte length of name */

        if (length > 0) /* if there is a name, write it out */
            (void) fwrite (obj_name (cp), 1, length, fp);

        puti (count, fp);         /* 2 bytes of count        */
        }
    writer = dt_writer (&g_dtypes [type]); /* get write function */

    if (writer == NULL)
	writer = (int(*)(void *, FILE *))e_write;

    /* Note:  the datatype specific writer set g_errno */

    if ((*writer) (cp, fp) == ERROR) /* write out data */
        return (ERROR);

    return (RET_OK);
    }

