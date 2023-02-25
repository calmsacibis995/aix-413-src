/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)58	1.5  src/tenplus/lib/obj/fgetline.c, tenplus, tenplus411, GOLD410 7/19/91 14:48:00";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fgetline
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
/* file:  fgetline.c                                                        */
/*                                                                          */
/*     usage:  length = fgetline (fp, buffer, size)                         */
/*                                                                          */
/* The fgetline routine reads the next (newline terminated) line from the   */
/* specified file pointer, putting it into the specified buffer and returns */
/* the length.  The newline character is not put in the buffer and does not */
/* count in the length of the line.  It returns EOF on end of file and size */
/* on buffer overflow.  The maximum number of chars in a valid line is      */
/* (size - 1) to leave room for the terminating NULL character.             */
/****************************************************************************/

#include    <stdio.h>

/****************************************************************************/
/* fgetline:  reads a line into a buffer of given length and returns length */
/* gets characters from specified file pointer.                             */
/* Returns EOF of EOF, and the maximal size on overflow.                    */
/****************************************************************************/

int fgetline (FILE *fp, char *s, int size)
    /* fp:            where to get characters */
    /* s:             where to put line       */
    /* size:          size of line buffer     */
    {
    register int c;  /* input character */
    register length; /* number of characters read into line */

    for (length = 0; ; length++)
	{
	c = fgetc (fp);

	if (c == '\n') /* if end of line, all done */
	    break;

	if (feof (fp)) /* if end of file, return any chars read in so far */
	    if (length == 0) /* if no chars read in, return EOF */
		return (EOF);
	    else
		break;

	/* Make sure we haven't overflowed the buffer.  If we have no */
	/* more room, read in the rest of the line and return.        */

	if (length >= (size - 1))
	    {
	    length = size; /* return length of size to indicate overflow */

	    for(;;) /* eat all chars up to end of line */
		{
		c = fgetc (fp);
		if ((c == '\n') || (c == EOF))
		    break;
		}
	    break; /* terminate buffer and return */
	    }
	*s++ = c; /* normal character, so put into buffer */
	}
    *s = '\0'; /* terminate string */
    return (length);
    }

