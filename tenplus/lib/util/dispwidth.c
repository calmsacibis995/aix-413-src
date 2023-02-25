static char sccsid[] = "@(#)71  1.7  src/tenplus/lib/util/dispwidth.c, tenplus, tenplus411, GOLD410 4/13/94 16:52:16";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS: calc_column  char_width  text_width  calc_line_position
 *            attr_text_width calc_line_position2 calc_line_position3
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdlib.h>
#include <string.h>

#include "chardefs.h"
#include "database.h"
#include "libutil.h"
#include "libobj.h"
#include "libshort.h"

#define MAXL (300 * MB_LEN_MAX) /* maximum number of bytes in a line */

/****************************************************************************/
/* calc_column: line position to screen column conversion function.         */
/*                                                                          */
/* arguments:              ATTR *text   - the text line                     */
/*                         int  pos     - the position in 'text[]'          */
/*                                                                          */
/* return value:           int column   - the screen column                 */
/*                                                                          */
/* globals referenced:     MultibyteCodeSet                                 */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/* converts a position within an ATTR text line into a screen               */
/* column location by calculating the display width of the text in the line */
/* up to the supplied line position.                                        */
/*                                                                          */
/****************************************************************************/

int calc_column (ATTR *text, int pos)
{
    char *char_text;    /* text as a 'char' string     */
    char *cp;           /* pointer into 'char_text'    */

    wchar_t wc;

    int  nbytes;        /* bytes per multibyte         */
    int  column=0;      /* screen column location      */
    int  count=0;       /* byte counter                */

#ifdef DEBUG
    debug ("calc_column: text = '%.*s', position = %d",
				 i_strlen((short *)text),text,pos);
#endif

    /* If we are dealing with a single byte code set and __max_disp_width = 1
       then all characters occupy 1 column position so 'pos' automatically
       becomes the column location. Similarly optimise the case when the
       line position is zero. */

       if ((pos == 0) || (!MultibyteCodeSet && __max_disp_width == 1))
	   return(pos);


    /* If we are dealing with a multibyte code set or __max_disp_width <> 1
       then the column location must be calculated from the display widths
       of the characters in the text line. In order to do this the
       attributes must be removed from the line and then each character
       converted into a wide character. */

    char_text = a2string(text, i_strlen((short *)text));
    cp = char_text;

    while (pos > count)
	{
	nbytes = mbtowc(&wc, cp, MB_CUR_MAX);
	/* move by at least 1 char */
	if (nbytes < 1) nbytes = 1;
	column += wc_text_width(wc);
	cp += nbytes;
	count += nbytes;
	}

#ifdef DEBUG
    debug ("calc_column:  calculated column = %d using %d bytes",
				     column, count);
#endif

    s_free(char_text);
    return (column);
}

/****************************************************************************/
/* char_width: determine the display width of an ATTR character.            */
/*                                                                          */
/* arguments:              ATTR *ch     - character for width calculation   */
/*                                                                          */
/* return value:           int width    - the display width                 */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/* determines the display width of an ATTR character. Where __max_disp_width*/
/* is 1 the display width of 1 is returned immediately.                     */
/*                                                                          */
/****************************************************************************/

int char_width (ATTR *ach)
{
    char ch[MB_LEN_MAX];   /* 'ach' with attributes removed    */
    char *cp;              /* pointer into 'ch'                */

    ATTR    *ap;
    wchar_t wc;

    int  nbytes;           /* bytes per multibyte              */
    int  bytecnt;	   /* byte count for attribute removal */

    /* If __max_disp_width = 1 in a single byte code set, then all
       characters in the current code set have a display width of 1 */

       if ((__max_disp_width == 1) && !MultibyteCodeSet)
	   return(1);


    /* remove the attributes from the received ATTR character */
    ap = ach;
    cp = ch;
    bytecnt = 1;
    do { *cp++ = de_attr(*ap++); bytecnt++; }
       while (!firstbyte(*ap) && (bytecnt <= MB_CUR_MAX));

    /* convert character to wide format to determine its display width */
    cp = ch;
    nbytes = mbtowc(&wc, cp, MB_CUR_MAX);
    return (wc_text_width(wc));
}

/****************************************************************************/
/* text_width: determine the display width of a char string                 */
/*                                                                          */
/* arguments:              char *text   - string for width calculation      */
/*                                                                          */
/* return value:           int width    - the display width                 */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/* determines the display width of a char string.  Where __max_disp_width   */
/* is 1 the display width is the string length.                             */
/*                                                                          */
/****************************************************************************/

int text_width (char *text)
{
    wchar_t *wtext;        /* 'text' in wide format          */

    int  width;            /* display width of 'text'        */
    int  len;              /* length of 'text'               */
    int  nwide;

    /* If __max_disp_width = 1 in a single byte code set, then the
       display width will be the same as the string length */ 

       if (__max_disp_width == 1 && !MultibyteCodeSet)
	   return(strlen(text));

    /* convert string to wide format to determine its display width */
    len    = strlen(text) + 1;
    wtext  = malloc(len * sizeof(wchar_t));
    nwide  = mbstowcs(wtext, text, len);
    width  = wcswidth(wtext, nwide);
    if (width < 0)
	width = 0;

    free(wtext);
    return (width);
}


/****************************************************************************/
/* calc_line_position: screen column to line position conversion function.  */
/*                                                                          */
/* arguments:              ATTR *text   - the line of text                  */
/*                         int  col     - screen column                     */
/*                                                                          */
/* return value:           int position   - the position in 'text[]' that   */
/*                                          equates to 'col'                */
/*                                          If text is too short (ie. it    */
/*                                          doesn't reach to col), the index*/
/*                                          of the terminating null char    */
/*                                          is returned.                    */
/*                                                                          */
/* globals referenced:     MultibyteCodeSet                                 */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/* converts a screen column to the equivalent position within a text line.  */
/* For single byte code sets where __max_disp_width = 1 the screen column   */
/* will always equal the line position. All other cases are required to     */
/* calculate the display width of each character to make the required       */
/* conversion.                                                              */
/****************************************************************************/

int calc_line_position (ATTR *text, int col)
{
  int dummy;

#ifdef DEBUG
    debug ("calc_line_position: text = '%.*s', column = %d",
			 i_strlen((short *)text),text,col);
#endif

  return calc_line_position2(text, col, &dummy);
}


/****************************************************************************/
/* calc_line_position2: screen column to line position conversion function. */
/*                                                                          */
/* arguments:              ATTR *text   - the line of text                  */
/*                         int  col     - screen column                     */
/*                         int  actual_width - output parameter. Returns    */
/*                                        the width of the line up to but   */
/*                                        not including the ATTR whose      */
/*                                        index is returned.                */
/*                                                                          */
/* return value:           int position   - the position in 'text[]' that   */
/*                                          equates to 'col'                */
/*                                          If text is too short (ie. it    */
/*                                          doesn't reach to col), the index*/
/*                                          of the terminating null char    */
/*                                          is returned.                    */
/*                                                                          */
/* globals referenced:     MultibyteCodeSet                                 */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/* converts a screen column to the equivalent position within a text line.  */
/* For single byte code sets where __max_disp_width = 1 the screen column   */
/* will always equal the line position. All other cases are required to     */
/* calculate the display width of each character to make the required       */
/* conversion.                                                              */
/****************************************************************************/

int calc_line_position2 (ATTR *text, int col, int *actual_width)
{

#ifdef DEBUG
    debug ("calc_line_position2: text = '%.*s', column = %d",
				 i_strlen((short *)text),text,col);
#endif

    /* If we are dealing with a single byte code set and __max_disp_width = 1
       then all characters occupy 1 column position so 'col' automatically
       becomes the column location. Similarly optimise the case when the
       column position is zero. */

    if ((col == 0) || (!MultibyteCodeSet && __max_disp_width == 1)) {
         int textlen = i_strlen((short *) text);
         *actual_width = min(textlen, col);
         return(min(textlen, col));
    }
    else {

	int mlen;

        /* scratch wide character */
        wchar_t wc;
    
        /* text as a 'char' string     */
        char *char_text = a2string(text, i_strlen((short *)text));

        /* pointer into 'char_text'    */
        char *cp = char_text;
    
        /* number of columns in front of character pointed to by cp */
        int  count=0;

        /* second pointer in char_text. After one iteration through
           the coming while loop, cp2 points at the character before
           the character pointed at by cp.
        */
        char *cp2 = cp;

        /* actual_width holds the number of columns in front of the
           character pointed at by cp2.
        */
        *actual_width = 0;
    
        /* If we are dealing with a multibyte code set or __max_disp_width <> 1
           then the line position must be calculated from the display widths
           of the characters in the text line. In order to do this the
           attributes must be removed from the line and then each character
           converted into a wide character. */
    
    
        while (*cp && count <= col) {
 
               cp2 = cp;
               *actual_width = count;

               if (count == col)
                    break;

        	mlen = mbtowc(&wc, cp, MB_CUR_MAX);
		/* move by at least 1 char */
		if (mlen < 1) mlen = 1;
        	cp += mlen;
        	count += wc_text_width(wc);
    	}
        
        /* The following holds at this point:
           a. *actual_width <= col && count >= col (column col reached) OR
           b. *cp = 0                              (end of string reached)
        */

        if (count <= col) {
             cp2 = cp;
             *actual_width = count;
        }            
    
#ifdef DEBUG
        debug ("calc_line_position2: calculated position %d using %d columns\n",
    				cp2 - char_text, count);
#endif
    
        s_free(char_text);
        return (cp2 - char_text);
    }
}


/****************************************************************************/
/* calc_line_position3: does the same as calc_line_position, but where      */
/*                      calc_line_position returns the index of the         */
/*                      character at or before the specified column,        */
/*                      calc_line_position3 returns the index of the        */
/*                      character at or AFTER the specified column.         */
/*                      If text is too short (ie. it                        */
/*                      doesn't reach to col), the index                    */
/*                      of the terminating null char                        */
/*                      is returned.                                        */
/****************************************************************************/

int calc_line_position3 (ATTR *text, int col)
{
  int idx;
  int actual_width;

#ifdef DEBUG
    debug ("calc_line_position3: text = '%.*s', column = %d",
			 i_strlen((short *)text),text,col);
#endif

  idx = calc_line_position2(text, col, &actual_width);
  if ((text[idx] != (ATTR)0) && (actual_width < col))
      skipattr_idx(text, &idx);

  return(idx);
}


/****************************************************************************/
/* attr_text_width: returns display width of ATTR string.                   */
/*                                                                          */
/* arguments:              ATTR *text   - the line of text (should not      */
/*                                        be longer than MAXL)              */
/*                                                                          */
/* return value:           int width    - the width of the ATTR string      */
/*                                                                          */
/* globals referenced:     MultibyteCodeSet                                 */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/****************************************************************************/

int attr_text_width(ATTR *text)
{
  char mbstr[MAXL + 1];
  char *mbp = mbstr;
  wchar_t wstr[MAXL + 1];
  ATTR *ap = text;
  int len;

  if (__max_disp_width == 1 && !MultibyteCodeSet)
       return(i_strlen((short *)text));

  /* copy text into mbstr, getting rid of attributes */
  while (*ap != (ATTR)0) 
    *mbp++ = de_attr(*ap++);
  *mbp = '\0';

  /* convert to wide character string */
  (void) mbstowcs(wstr, mbstr, MAXL + 1);

  /* get and return width of wide character string */
  len = wcswidth(wstr, MAXL + 1);
  if (len < 0)
    len = 0;
  return(len);
}


/****************************************************************

OVERWRITE_CHAR

Overwrites the character located at a specified column in a 
specified line by a specified overwrite character. If the victim
character takes more display positions than the overwrite character,
the exces space is filled with the specified fill character. Because
all these operations might involve deleting or inserting ATTRs in the
line (which might result in a new line being created on the heap),
the pointer to the line
as passed in the line parameter might be invalid after this 
function has terminated.
Therefore, a pointer to the modified line is returned. 

arguments: 
---------
line                    line containing victim character;

column                  column position of the victim character;

overwrite_ch            character that will be put at column position;

fill_char               if victim character is wider than overwrite
                        character, the exces space is filled with this
                        character.

return value:
------------
pointer to new version of the line.

No globals referenced or changed.

Note: it is assumed that both overwrite_ch and fill_char use one
      ATTR and are one position wide.

****************************************************************/

ATTR *overwrite_char(ATTR *line, int column, ATTR overwrite_ch,
                     ATTR fill_char)
{
  if (__max_disp_width == 1 && !MultibyteCodeSet) {
      line[column] = overwrite_ch;
      return(line);
  }
  else {
    
      /* new version of line */
      ATTR *new_line = line;
    
      /* width of the line up to the victim character */
      int pre_width;
    
      /* index of the victim character in the line */
      int victim_idx = calc_line_position2(line, column, &pre_width);
    
      /* length in ATTRs of the victim character */
      int victim_len = attrlen(&(line[victim_idx]));
    
      /* width of the victim character in columns */
      int victim_width = char_width(&(line[victim_idx]));
    
      /* the victim character will be overwritten by victim_width
         characters that all take one ATTR. So make sure that
         the number of ATTRs matches the number the victim_width.
      */
      if (victim_len > victim_width)
          new_line = (ATTR *) i_delete((short *) line, victim_idx, 
                                       victim_len - victim_width);
      else if (victim_width > victim_len)
               new_line = (ATTR *) i_insert((short *) line, victim_idx, 
                                            victim_width - victim_len);
    
      /* replace the victim character with fill characters */
      i_smear((short) fill_char, victim_width, 
              (short *)&(new_line[victim_idx]));

      /* put the overwrite character at the column position */
      line[victim_idx + column - pre_width] = overwrite_ch;
    
      return(new_line);
  }   

}


