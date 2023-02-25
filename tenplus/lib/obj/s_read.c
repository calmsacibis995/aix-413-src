/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)12	1.5  src/tenplus/lib/obj/s_read.c, tenplus, tenplus411, GOLD410 7/19/91 14:50:12";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_read
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
/* s_read:  reads a structured allocation from a file                       */
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
/*   datatype involved.  This may involve recursive calls to s_read in the  */
/*   case of POINTER sallocs                                                */
/*                                                                          */
/*   If the flag byte is zero, the name is NULL, and the count is less than */
/*   255, an abreviated version of this format is used with only the type   */
/*   byte (with the high bit on) and a one byte count.                      */
/*                                                                          */
/* A NULL pointer is represented by a NULL byte in the file.                */
/*                                                                          */
/* Returns ERROR on I/O error, else a pointer to the object read in.        */
/* Returns ERROR w/ g_errno = E_TYPECHECK if object type is illegal.        */
/****************************************************************************/

#include <stdio.h>
#include "database.h"

#include <libobj.h>

char *s_read (register FILE *fp)
    /*fp:         file pointer to read from     */
    {
    register type;             /* datatype being read in        */
    register flag;             /* flag bits for object          */
    register length;           /* length of name                */
    register count;            /* size of data array for object */
    register char *cp;         /* data pointer for object       */
    register int (*reader) (void *, FILE *); /* read function for data type   */
    char namebuf [NAMESIZE];   /* input buffer for names        */


#ifdef TRACE

    debug ("s_read:  called w/ fp = 0%o\n", fp);

#endif

    type = fgetc (fp);                /* one byte of salloc type */

    if (type == 0)                /* handle NULL pointers    */
        return (NULL);

    if (type & 0x80) /* if abreviated header */
        {
        type &= 0x7f;               /* strip high bit */
        flag = 0;                   /* no flag        */
        length = 0;                 /* no name        */
        count = fgetc (fp) & 0xff;   /* one byte count */
        }
    else
        {
        flag = fgetc (fp);   /* one byte of flag bits   */
        length = fgetc (fp); /* one byte length of name */

        if (length >= NAMESIZE) /* if name too long */
            fatal ("s_read:  length (%d) too large", length);

        if (length > 0) /* if there is a name */
            {
            (void)fread (namebuf, 1, length, fp);  /* read in name            */
	    namebuf [length] = '\0';         /* null terminate name     */
            }
        count = geti (fp); /* read in count */
        }
    if (count < 0)
        {
        g_errno = E_MISC;
        return ((char *) ERROR);
        }
    /* check type against valid types */

    if ((type <= 0) || (type >= MAX_TYPES) || 
      (dt_name (g_dtypes + type) == NULL))
        {
        g_errno = E_TYPECHECK;
        return ((char *) ERROR);
        }
    /* allocate object and use its name if it has one */

    cp = s_alloc (count, type, (length > 0) ? namebuf : NULL);

    if (cp == (char *) ERROR)    /* check for allocation failure */
        return ((char *) ERROR); /* Note:  g_errno already set   */

    obj_setflag (cp, flag);                 /* set flag bits in object      */
    reader = dt_reader (&g_dtypes [type]);   /* get read function            */
    if (reader == NULL)
	reader = (int(*)(void *, FILE *))e_read;

    /* Note:  the datatype specific readers set g_errno */

    if ((*reader) (cp, fp) == ERROR)        /* read in each object in array */
        return ((char *) ERROR);            /* check for I/O error          */

    return (cp);
    }

