#if !defined(lint)
static char sccsid[] = "@(#)87	1.9  src/tenplus/lib/util/unpackup.c, tenplus, tenplus411, GOLD410 3/3/94 18:08:46";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	unpackup
 * 
 * ORIGINS:  9, 10, 27
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "chardefs.h"
#include "database.h"
#include "libshort.h"
#include "libutil.h"
#include "libobj.h"

#define AMOUNT (200 * MB_LEN_MAX)

/****************************************************************************/
/* unpackup:  convert string into internal (editor) format.                 */
/* Converts tabs to spaces, handles underlined chars and makes sure result  */
/* is at least length chars long.  Returns an allocated string (ATTR)       */
/*                                                                          */
/* arguments:          char *start - string to unpack to internal form      */
/*                     int  width  - displqy width to pad string out to     */
/*                                                                          */
/* return value:       ATTR * - the unpacked version of the string          */
/*                                                                          */
/* globals referenced: none                                                 */
/*                                                                          */
/* globals changed:    none                                                 */
/*                                                                          */
/* synopsis:                                                                */
/*     unpackup converts a string from the form it is stored in the file    */
/*     to the less compressed form used within the editor.                  */
/*                                                                          */
/*     Tab characters are converted into up to eight space characters       */
/*     (depending on which column the tab happens to occur).  Triplet       */
/*     sequences of the form underscore-backspace-character are converted   */
/*     into single underlined characters.  If the converted string          */
/*     ends up less than the given length, it is padded with space          */
/*     characters out to the necessary length and then null-terminated.     */
/****************************************************************************/

ATTR *unpackup (char *start, int width)
    {
    int i;                    /* loop variable                 */
    int attr_to_copy;         /* number of ATTRs to copy       */ 
    int count;                /* space count                   */
    int pos;                  /* position of 'width' in string */
    int actual_width;         /* actual width of 'pos' in string */

    char *string;             /* input string pointer          */
    unsigned int size;        /* current size of buffer        */
    unsigned int cpindex;     /* cp index into buffer          */

    ATTR  *cp;                /* pointer to buffer             */
    ATTR  *buffer;            /* translation buffer            */
    ATTR  *limit;             /* the limit                     */

#ifdef DEBUG
    debug ("unpackup:  string = '%s', width = %d", start, width);
#endif

    /* empty string? */
    if (start == NULL)
      return((ATTR *) i_string(NULL));

    /* allocate space for the conversion */
    size   = max(AMOUNT, width);
    buffer = (ATTR *) s_alloc (size, T_ATTR, NULL);
    cp     = buffer;

    /* leave room for a tab at the end of a long line...        */
    limit = buffer + size - 9;

    /* process the string to its end */
    string = start;
    while (*string)	
        {
	if (cp >= limit)
	    {
	    size    = size + AMOUNT;
	    cpindex = cp - buffer;
	    buffer  = (ATTR *) i_realloc ((short *)buffer, size);
	    cp      = buffer + cpindex;
	    limit   = buffer + size - 9;
	    }

         /* classify the character */
         switch (*string)
		{

                /* convert tabs to spaces */
		case '\t':

		    count = 8 - ((cp - buffer) % 8);

		    /* NOTE:  this is okay for attributes too */
		    for (i = 0; i < count; i++)
			*cp++ = SPACE | FIRST;

                    /* skip over tab */
                    string++ ;
                    break;

                /* check for an underlined character indicated by '_\b'. Note
                   that a single '_' will fall through to the 'default' case */

		case '_':

		    if ((string [1] == '\b') && (string[2] != '\0'))
			{
			string += 2;

                        *cp++ = *string++ | UNDERLINE | FIRST;
                        if (MultibyteCodeSet) 
                            {
                            attr_to_copy = mblen(string - 1, MB_CUR_MAX);
			    /* copy at least 1 char */
			    if (attr_to_copy < 1) attr_to_copy = 1;
                            while ( --attr_to_copy > 0)
                                *cp++ = *string++ | UNDERLINE;
                            }
                        
			 break;
                         }

		/* handle all other characters */
		default:

                    *cp++ = *string++ | FIRST;
                    if (MultibyteCodeSet) 
                        {
                        attr_to_copy = mblen(string - 1, MB_CUR_MAX);
			/* copy at least 1 char */
			if (attr_to_copy < 1) attr_to_copy = 1;
                        while ( --attr_to_copy > 0)
                            *cp++ = *string++ ;
                        }
        	}
	}

    /* pad out the string to the required size */
    pos = calc_line_position2(buffer, width, &actual_width);
    if (buffer[pos] == (ATTR)0)
        pos += width - actual_width;

    while (cp < &buffer[pos])
	*cp++ = SPACE | FIRST;

    /* reallocate the buffer to the correct size */
    buffer = (ATTR *) i_realloc ((short *)buffer, cp - buffer);

#ifdef DEBUG
    debug ("unpackup:  returning buffer length = %d", cp - buffer);
    debug ("unpackup:  returning buffer");
    if (g_debugfp)
	s_print (buffer);
#endif

    return (buffer);
    }

