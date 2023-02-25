#if !defined(lint)
static char sccsid[] = "@(#)80	1.9  src/tenplus/lib/util/packup.c, tenplus, tenplus411, GOLD410 3/3/94 18:06:30";
#endif /* lint */

/*
 *  COMPONENT_NAME: (INED) INed Editor
 *
 *  FUNCTIONS:	packup
 * 
 *  ORIGINS:  9, 10, 27
 *
 *  IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <database.h>
#include <chardefs.h>

#include <libutil.h>
#include <libshort.h>
#include <libobj.h>

#define AMOUNT (100 * MB_LEN_MAX)

/****************************************************************************/
/* packup:  convert string into external (file) form.                     */
/* Converts leading spaces to tabs, handles underlined chars and removes    */
/* trailing spaces.  Returns a s_alloc'ed string.                           */
/*                                                                          */
/* arguments:              ATTR *string - to be converted for export        */
/*                                                                          */
/* return value:           char * - the external (file) form of the string  */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     packup converts the given string from its internal (editor) form   */
/*     into the form it is stored in the file.  All groups of eight spaces  */
/*     at the beginning of the line are converted into single tab character */
/*     Every underlined character is converted into the three-character     */
/*     sequence underscore-backspace-character.  Trailing, nonunderlined    */
/*     whitespace (tabs and spaces) are removed from the end of the line;   */
/*     isspace()/iswspace() not used to this because they include formfeed  */
/*     vertical tab etc. in space characters, in addition to spaces and tabs*/
/*     So, we trim only spaces ( not double byte spaces  ) and tabs.        */
/****************************************************************************/

char *packup (ATTR *string )
    /* string:  string to be converted       */
    {
    register char *cp;      /* pointer to buffer        */
    register i;             /* loop variable            */
    register count;         /* number of leading spaces */
    unsigned int size;      /* current size of buffer   */
    unsigned int cpindex;   /* cp index into buffer     */
    char *buffer;           /* translation buffer       */
    char *limit;            /* the limit                */

#ifdef DEBUG
    debug ("packup:  string = : %s",string);
#endif

    if (de_attr(*string) == 0)
	return (s_string (""));

    size = AMOUNT;
    buffer = s_alloc (size, T_CHAR, NULL);
    cp = buffer; /* put leading tabs and spaces in buffer */

    count = 0; /* get number of leading spaces */

    /* In the next for loop, a direct comparison with SPACE occurs,
       instead of using iswspace. This is because iswspace not only
       returns TRUE for spaces, but also for tabs, newlines, etc.
       In this case, we want to make sure we have a space and nothing
       else.
    */
    /* NOTE: for attributes, *string == SPACE iff attributes are null */
    for ( ; *string == (SPACE | FIRST); skipattr(&string))
	    count++;

    for (i = 0; i < (count / 8); i++)
	*cp++ = '\t';

    for (i = 0; i < (count % 8); i++)
	*cp++ = SPACE;

    /* leave room for trailing underlined char  */
    limit = buffer + size - 4;

    /* convert and xfer rest of characters in string */
    while (*string)
	{
	if (cp >= limit)
	    {
	    size = size + AMOUNT;
	    cpindex = cp - buffer;
	    buffer = s_realloc (buffer, size);
	    cp = buffer + cpindex;
	    limit = buffer + size - 4;
	    }

	if (is_underlined(*string))  /* if character is in underlined-normal font */
	    {
	    *cp++ = '_';
	    *cp++ = '\b';
	    }

         if (MultibyteCodeSet) {
            char mbchar[10];
            for (i = 0; i < MB_CUR_MAX; i++)
               mbchar[i] = de_attr(string[i]);
            i = mblen(mbchar, MB_CUR_MAX);
	    /* copy at least 1 char */
	    if (i < 1) i = 1;
            while (i-- > 0)
               *cp++ = de_attr(*string++);
         }
         else  *cp++ = de_attr(*string++);
    }

    if (MultibyteCodeSet) {
       size_t  len_wbuffer = cp - buffer + 1;
       wchar_t *wbuffer = malloc(len_wbuffer * sizeof(wchar_t));
       int     wcp_indx = mbstowcs(wbuffer, buffer, len_wbuffer) - 1; 
       size_t  trimmed_len;
 
       for ( ; wcp_indx >= 0; wcp_indx--)
           if (( wbuffer[wcp_indx] != L' ' && wbuffer[wcp_indx] != L'\t' ) ||
               ((wcp_indx > 0) && (wbuffer[wcp_indx - 1] == L'\b')))
             break;
 
       wbuffer[wcp_indx + 1] = L'\0';
       /* N.B. the following three lines should not be needed but
          there is currently a bug such that wcstombs returns a 1 instead of 0
          for multibyte languages when given an empty string */
       if (wcp_indx < 0)
           trimmed_len = 0;
       else
           trimmed_len = wcstombs(buffer, wbuffer, len_wbuffer);
       free(wbuffer);
#ifdef DEBUG
       debug ("packup:  strlen (buffer) = %d", strlen (buffer));
       debug ("packup:  returning buffer '%s'", buffer);
#endif
       return(s_realloc(buffer, trimmed_len));

    } else {
       for (cp-- ; cp >= buffer; cp--) /* trim trailing spaces */
       {
   	  if ( (*cp != ' ' && *cp != '\t' ) || ((cp != buffer) && (* (cp - 1) == '\b')))
   	    break;
       }
       *(cp+1) = '\0'; /* terminate buffer */
#ifdef DEBUG
       debug ("packup:  strlen (buffer) = %d", strlen (buffer));
       debug ("packup:  returning buffer '%s'", buffer);
#endif
       return (s_realloc (buffer, cp - buffer + 1));
    }
 }

