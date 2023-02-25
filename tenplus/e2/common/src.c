#if !defined(lint)
static char sccsid[] = "@(#)93	1.7  src/tenplus/e2/common/src.c, tenplus, tenplus411, GOLD410 3/23/93 11:54:02";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	findstring, getword, go, keyreplace,
 *		keysearch, putcurs, search 
 * 
 * ORIGINS:  9, 10
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
/* File:   src.c - procedures relating to SEARCH and REPLACE                */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

static char srchstring [MAXL];           /* last search string  */
static char rplcstring [MAXL];           /* last replace string */
char *g_searchstr = &srchstring [0];    /* global copy of search string */

int findstring (ATTR *, char *, int, int);

/****************************************************************************/
/* findstring:  tries to find string starting at given position             */
/* Returns column where string starts, or ERROR if string cannot be found.  */
/*                                                                          */
/* arguments:              ATTR *text      - line to be searched            */
/*                         char *string    - string to search for           */
/*                         int   index     - where to start looking         */
/*                         int   direction - +1 for +SRCH, -1 for -SRCH     */
/*                                                                          */
/* return value:           offset in text of start of string,               */
/*                         or ERROR if string not found                     */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     findstring looks for the string within the text, searching either    */
/*     forward or backward from the given index position within the text.   */
/*     If the string is longer than the text, return ERROR immediately.     */
/*     Otherwise copy the string into a buffer, removing underlining, and   */
/*     do the search.  Loop through the buffer on a character-by-character  */
/*     basis, moving forward or backward from the initial index.  Do not    */
/*     bother looking for the start of the string too close to the end of   */
/*     the text.                                                            */
/****************************************************************************/

int findstring (ATTR *search_txt, char *search_str, int index1, int direction)
{

    int str_len, 	/* length of string to search for */
	txt_len, 	/* length of entire search string */
	fwd_len,        /* length of the search area for forwards */
        ret_val;  	/* return value offset into text of match */

    char *txt_ptr,      /* Text to search */
	 *tmp_ptr,        
	 *ret_ptr;      /* String that is matched */

    char *last_found_ptr = NULL;
    txt_len = i_strlen ((short *)search_txt); 
    str_len = strlen(search_str);
    
    /* No search string or string to search for is longer than the text! */ 
    if (str_len == 0 || str_len > txt_len)
	ret_val=ERROR;
    else
       {
       if (MultibyteCodeSet)
           {
           while (!firstbyte(search_txt[index1]))
	       {
	       if (direction == 1)
	           index1++;
	       else 
	           index1--;
	       }
           }

       /* remove attributes and begin the search */    
       txt_ptr = a2string(search_txt, txt_len);

       /* forward search */
       if (direction == 1) 
           {
           tmp_ptr = txt_ptr+index1;
           fwd_len = strlen (tmp_ptr);

	   if (str_len > fwd_len)
	      ret_val=ERROR; 
	   else
	       {
	       if ((ret_ptr = strstr(tmp_ptr, search_str))==NULL) 
	           ret_val=ERROR;
	       else
	           ret_val=index1 + ( ret_ptr - tmp_ptr ); /* Offset */ 
	       }
           }

       /* Backward search */
       else 
           {

           /* last_found_ptr is NULL when search_str was not found
              before index1. Otherwise it points to the last occurence
              of search_str before index1. */
          
           tmp_ptr = txt_ptr;

           /* keep on finding occurences of search_str until non found
              anymore or last found occurence is beyond index */
          
           while ((tmp_ptr = strstr(tmp_ptr, search_str)) != NULL &&
                 tmp_ptr < txt_ptr + index1)
              {
              last_found_ptr = tmp_ptr;
              skipmb(&tmp_ptr);
              }

          if (last_found_ptr == NULL)
               ret_val = ERROR;
          else ret_val = (last_found_ptr - txt_ptr);
 
          }

       s_free(txt_ptr);
    }

    return(ret_val); /* Return offset */
}


/****************************************************************************/
/* getword:  gets next word from window and puts into buffer                */
/*                                                                          */
/* arguments:              char *buffer - to put word into                  */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Copy characters into the buffer from the current window,             */
/*     starting at the current cursor position and stopping at              */
/*     the first space or end-of-line.  Null-terminate the buffer           */
/*     string.                                                              */
/****************************************************************************/

void getword (char *buffer)
{
    WSTRUCT *wp=g_wp;    /* current window                 */
    ATTR    *cp;         /* used to find word              */
    wchar_t *wcs_base;
    wchar_t *wcs_ptr;
    char    *cp_dattr;  /* string to extract word from    */
    int n;

    cp = (ATTR *) w_text (wp, w_line(wp))+(w_ftcol(wp)+w_col (wp));
    cp_dattr = a2string(cp,i_strlen((short *)cp));

    n=strlen(cp_dattr) +1;

    wcs_base = wcs_ptr = (wchar_t*) malloc(n * sizeof(wchar_t));
    mbstowcs(wcs_base,cp_dattr,n);


    while (!iswspace(*wcs_ptr) && *wcs_ptr != '\0')
        wcs_ptr++;

    *wcs_ptr='\0';

    wcstombs(buffer,wcs_base,MAXL);

}

/****************************************************************************/
/* go:  handles the GOTO command                                            */
/*                                                                          */
/* arguments:              int   argtype  - how called: NOARGS, EMPTYARG... */
/*                         char *strvalue - argument if argtype is          */
/*                                          STRINGARG                       */
/*                         int   numvalue - argument if argtype is          */
/*                                          NUMBERARG                       */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If called with a numeric argument, go to the given line.             */
/*     (If the given line is less than 1, go to line 1.  Note that          */
/*     the user is free to go arbitrarily far beyond the end of the         */
/*     file.  This is at least harmless.)                                   */
/*                                                                          */
/*     If called with no argument, go to the beginning of the file          */
/*     unless the cursor is already on the first line; if it is,            */
/*     go to the end.                                                       */
/*                                                                          */
/*     If called with an empty argument (ENTER GOTO), go to the end         */
/*     of the file, even if the cursor is already on the last line.         */
/****************************************************************************/

void go (int argtype, char *strvalue, unsigned numvalue)
    {
    WSTRUCT *wp=g_wp;    /* current window        */
    int filelength;
    int line;

#ifdef DEBUG
    debug ("go called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    filelength = Enumlines ("");
    if (argtype == NUMBERARG)
	{
	if (numvalue > MAXFLINE)
	    {
	    numtoobig ();
	    return;
	    }

        /* input in 1-based coords; use 0-based */
	line = numvalue - 1;

	/* if line is less than 0 or will be */
	if ((line < 0) || (line +1 < 0))
	    line = 0;

	putcurs (line, 0);
	}

    /* last line is numlines - 1 */
    else if (((w_line (wp) + w_ftline (wp)) == 0) || (argtype == EMPTYARG))
	putcurs (filelength - 1, 0);   

    else putcurs (0, 0);
    }

/****************************************************************************/
/* keyreplace:  handler for the REPLACE command                             */
/*                                                                          */
/* arguments:              int   argtype  - how called: NOARGS, EMPTYARG... */
/*                         char *strvalue - argument if argtype is          */
/*                                          STRINGARG                       */
/*                         int   numvalue - argument if argtype is          */
/*                                          NUMBERARG                       */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, srchstring, rplcstring                     */
/*                                                                          */
/* globals changed:        rplcstring                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     If invoked with NOARGS, use the previous replacement string;         */
/*     otherwise set the replacement string to the given string.            */
/*     If the replacement string is NULL, the replace command is like       */
/*     a text-region delete operation.                                      */
/*                                                                          */
/*     If the current search string is not positioned under the cursor,     */
/*     return immediately.  Otherwise replace the search string at the      */
/*     current cursor position with the replacement string, underlining     */
/*     it if the search string in the current window was underlined.        */
/*     Finally, invoke sendbox to tell the display code of the change.      */
/****************************************************************************/


void keyreplace (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;   /* current window               */
    ATTR *rtext;        /* replace string as ATTR       */
    ATTR *text2;        /* text for current line        */


    int line;
    ATTR    *text3;      /* one line of cache                       */

    int i;              /* used to maintain underlining */
    int position;       /* position in 'text2'          */
    int length;         /* string length of rplcstring  */
    int disp_len;       /* display width of rplcstring  */
    int flag;           /* TRUE if match is underlined  */
    int search_len;
    

    if (argtype != NOARGS)
	(void) strcpy (rplcstring, strvalue);

    text2    = (ATTR *) w_text (wp, w_line (wp));
    position = calc_line_position(text2, w_col(wp)+w_ftcol(wp));
    if (findstring (text2, srchstring, position, 1) != position)
	{
	Ebell ();
	return;
	}

    flag       = is_underlined (text2[position]);
    disp_len   = text_width(rplcstring);
    length     = strlen(rplcstring);
    search_len = strlen(srchstring);

    /* remove the search string iand stop if no replacement string */
    delsp (w_line(wp), position, search_len);
    if (length == 0)
        return;

    /* make space for the replacement string   */
    inssp (w_line (wp), position, disp_len);  

    if (length > disp_len)
    {
       line = w_line(wp);
       text3 = (ATTR *) w_text (wp, line);
       text3 = (ATTR *) i_insert ((short *)text3, position, (length - disp_len));
       i_smear ((short)(SPACE | FIRST), (length - disp_len), (short *)(text3 + position));
       wp->w__cache [line] = (char *) text3;  
    }



    text2 = (ATTR *) w_text (wp, w_line (wp));
    rtext = unpackup (rplcstring, 0);

    /* put the replacement string into the text */
    (void) i_strncpy ((short *)&text2[position], (short *)rtext, length);
    s_free ((char *)rtext);

    /* underline if match is underlined */
    if (flag)
	for (i = 0; i < length; i++)
	    text2[position + i] = underline (text2[position + i]);

    sendbox (w_line (wp), w_line (wp), w_col (wp),
	min (w_lrcol (wp), w_col (wp) + disp_len - 1));

    }

/****************************************************************************/
/* keysearch:  handler for the +SRCH and -SRCH commands                     */
/*                                                                          */
/* arguments:              int   argtype  - how called: NOARGS, EMPTYARG... */
/*                         char *strvalue - argument if argtype is          */
/*                                          STRINGARG                       */
/*                         int   numvalue - argument if argtype is          */
/*                                          NUMBERARG                       */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, srchstring                           */
/*                                                                          */
/* globals changed:        srchstring                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     If invoked with a string argument, set the global search string to   */
/*     the given value.  If invoked with an empty argument (ENTER +SRCH     */
/*     or ENTER -SRCH), set the search string to the word at the current    */
/*     cursor position.  If invoked with no arguments, use the previous     */
/*     value of the global search string or, if it was never initialized,   */
/*     invoke Eerror to complain.                                           */
/*                                                                          */
/*     Examine the g_keyvalue global to decide whether to search forwards   */
/*     or backwards.  Finally, invoke the search procedure to actually      */
/*     perform the search.                                                  */
/****************************************************************************/

void keysearch (int argtype, char *strvalue, int numvalue)
    {
    int direction = (g_keyvalue == PLUS_SEARCH ? 1 : -1);

    if (argtype == STRINGARG)
	(void) strcpy (srchstring, strvalue);

    /* get next word from text */
    else if (argtype == EMPTYARG)
	getword (srchstring); 

#ifdef DEBUG
    debug ("keysearch: dir = %d, string = '%s'", direction, srchstring);
#endif

    if (*srchstring)
	search (srchstring, direction);
    else
	Eerror (M_ESRCSTRING, "There is no remembered search string.");
    }

/****************************************************************************/
/* putcurs:  move cursor to new field position                              */
/*                                                                          */
/* arguments:              int line, column - 0-based field coords to go to */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If either the line or column argument is less than 0, set it to 0.   */
/*     Then see whether the given line and column field position falls with */
/*     the current window boundaries.  If it is, do not move the window.    */
/*                                                                          */
/*     If the desired line is not in the window, move the window up or down */
/*     so that the line is halfway down the window.  If the desired column  */
/*     is not in the window, move it left or right so that the column is    */
/*     one-third of the way across the window from the left side.           */
/*                                                                          */
/*     Finally, convert the line and column to window coordinates and       */
/*     position the cursor there.                                           */
/****************************************************************************/

void putcurs (int line, int column)
    {
    WSTRUCT *wp=g_wp;               /* current window                         */

    int bestline=w_lrline(wp) / 2;  /* preferred landing position for w_line  */
    int bestcol=w_lrcol(wp) / 3;    /* preferred position for w_col           */
    int newfline;                   /* new w_ftline if scolling needed        */
    int newfcol;                    /* new w_ftcol if scolling needed         */

#ifdef DEBUG
    debug ("putcurs:  line = %d, col = %d", line, column);
#endif

    if (line < 0)
	line = 0;

    if (column < 0)
	column = 0;

    newfline = max (line - bestline, 0);
    newfcol  = max (column - bestcol, 0);

    if (line < w_ftline (wp))
	(void) movup (w_ftline (wp) - newfline);

    if (line > (w_ftline (wp) + w_lrline (wp)))
	movdn (newfline - w_ftline (wp));

    if (column < w_ftcol (wp))
	(void) movlf (w_ftcol (wp) - newfcol);

    if (column > (w_ftcol (wp) + w_lrcol (wp)))
	(void) movrt (newfcol - w_ftcol (wp));

    wp->w__line = line - w_ftline (wp);
    wp->w__col  = min (column - w_ftcol (wp), w_lrcol(wp));
    }

/****************************************************************************/
/* search:  search for given string in specified direction                  */
/*                                                                          */
/* arguments:              char *string    - to search for                  */
/*                         int   direction - +1 for +SRCH, -1 for -SRCH     */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_intflag                                  */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Search for the given string in the current field, starting from      */
/*     the current cursor position offset by one column in the given        */
/*     direction.  (The offset is necessary to prevent finding the same     */
/*     match twice.)  If the string is not on the current line, begin       */
/*     moving through the file in the given direction, line by line,        */
/*     searching for it on each successive line.                            */
/*                                                                          */
/*     If the string is found, invoke putcurs to move the cursor to the     */
/*     matching position and then return.  If the user hits BREAK, invoke   */
/*     Eerror to say the search was aborted and then return.  If you cross  */
/*     over one screenheight's worth of lines without finding a match,      */
/*     invoke Emessage to reassure the user the search is happening and     */
/*     then continue to search.  Finally, if you reach the end or beginning */
/*     of the file without finding a match, invoke Eerror to say the search */
/*     failed and then return.                                              */
/****************************************************************************/

void search (char *string, int direction)
    {
    WSTRUCT *wp=g_wp;                         /* current window              */
    ATTR    *text3;                           /* text for line               */

    int line;                                 /* current search line         */
    int column;                               /* column related to 'position'*/
    int position;                             /* current search position     */
    int fieldlen;                             /* # lines in field            */
    int oldline=w_line(wp)+w_ftline(wp);      /* starting line               */
    int msgline;                              /* line to print message on    */
    int first_line = TRUE;

#ifdef DEBUG
    debug ("search:  direction = %d, string = '%s'", direction, string);
#endif

    msgline  = oldline + (LI * direction);
    fieldlen = Enumlines ("");
    line     = oldline;

    /* cursor off end of file -  make +SRCH fall past while loop */
    if (line >= fieldlen) 
	line = fieldlen + direction;

    while ((line >= 0) && (line < fieldlen))
	{
        
        /* get the line of text and determine where to start */
	text3 = getli (line);
        if (first_line == TRUE)
           {
           position = calc_line_position(text3,w_col(wp)+w_ftcol(wp))+direction;
           first_line = FALSE;
           }

        /* reposition ourselves at the beggining or end of a new line */
	if (line != oldline)
	    position = direction > 0 ? 0 : i_strlen ((short *)text3);

        /* try to find a match */
	position = findstring (text3, string, position, direction);

        /* if a match found put the cursor on the match */
	if (position != ERROR)
	    {
            column = calc_column(text3, position);
	    putcurs (line, column);
	    s_free ((char *)text3);
	    return;
	    }

        /* see if user has hit BREAK */
	if (g_intflag)
	    {
	    s_free ((char *)text3);
	    Eerror (M_ESRCBREAK, "The command was stopped by BREAK.");
	    return;
	    }

        /* tell the world we're doing something */
	s_free ((char *)text3);
	if (line == msgline)
	    {
	    if (direction > 0)
	       Emessage (M_EPLUSSEARCH, "Search Down: %s", string);
	    else
	       Emessage (M_EMINUSSEARCH, "Search Up: %s", string);
	    }

	line += direction;
	}

    Eerror (M_ESRCFAILED, "There are no occurrences of string %s.", string);
    }
