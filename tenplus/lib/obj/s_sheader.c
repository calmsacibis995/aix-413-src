/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)16	1.5  src/tenplus/lib/obj/s_sheader.c, tenplus, tenplus411, GOLD410 7/18/91 13:45:33";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	s_sheader
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
/* s_sheader:  reads header information into an sheader structure           */
/* Returns a pointer to a static structure, ERROR on read errors, or        */
/* NULL for NULL records                                                    */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

SHEADER *s_sheader (register FILE *fp)
    /*fp:          file pointer to read from     */
    {
    register type;                  /* datatype being read in   */
    register length;                /* length of name           */
    static char namebuf [NAMESIZE]; /* input buffer for names   */
    static SHEADER header;          /* header structure         */


#ifdef DEBUG
    debug ("s_sheader:  called w/ fp = 0%o", fp);
#endif

    type = fgetc (fp);                /* one byte of type */

    if (type == '\0')                 /* handle NULL pointers */
	return (NULL);

    header.sh__type = type; header.sh__name = NULL;

    if (type & 0x80) /* if abreviated header */
	{
	type &= 0x7f;                          /* strip high bit */
	header.sh__flag = 0;                   /* no flag        */
	header.sh__count = fgetc (fp) & 0xff;  /* one byte count */
	}
    else
	{
	header.sh__flag = fgetc (fp);  /* one byte of flag bits   */
	length = fgetc (fp);           /* one byte length of name */

	if (length >= NAMESIZE) /* if name too long */
	    fatal ("s_sheader:  length (%d) too large", length);

	if (length > 0) /* if there is a name */
	    {
	    (void)fread (namebuf, 1, length, fp);  /* read in name            */
	    namebuf [length] = '\0';         /* null terminate name     */
	    header.sh__name = namebuf;

	    }
	header.sh__count = geti (fp); /* read in count */
	}
    header.sh__type = type;

#ifdef DEBUG
    debug ("s_sheader:  type = %d, name = '%s', count = %d, flag = %d",
	   type, header.sh__name ? header.sh__name : "NULL",
	   header.sh__count, header.sh__flag);
#endif

    if ((!feof (fp)) && (!ferror (fp))) /* check for I/O error */
	return (&header);

    g_errno = E_IO;
    return ((SHEADER *) ERROR);
    }

