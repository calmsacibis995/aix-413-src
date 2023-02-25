#if !defined(lint)
static char sccsid[] = "@(#)53	1.10  src/tenplus/e2/common/wrp.c, tenplus, tenplus411, GOLD410 3/23/93 11:54:49";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	center, clearlines, delli, delsp, insli, inssp, join,
 *		movlf, movrt, split 
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
/* file:  wrp.c                                                             */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

void clearlines (int, int);

/****************************************************************************/
/* center:  centers text on current line                                    */
/*                                                                          */
/* arguments:              int   argtype  - whether called w/ NOARGS, ...   */
/*                         char *strvalue - argument if called w/ STRINGARG */
/*                         int   numvalue - argument if called w/ NUMBERARG */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     center centers one or more lines of text between the margins or, if  */
/*     there are no margins in the current field, between the left and      */
/*     right sides of the field.  If the argtype is NUMBERARG, then         */
/*     numvalue is the number of lines to center; if it is LINEREGION,      */
/*     center all the lines in the region; otherwise center the current     */
/*     line only.  Each line is centered by finding the first and last      */
/*     non-white character and then centering between the boundaries.  If a */
/*     line is all spaces, it is unchanged.  If it is too wide for the      */
/*     boundaries, it is put flush with the left margin.                    */
/****************************************************************************/

void center (int argtype, char *strvalue, unsigned numvalue)
    {
    WSTRUCT *wp=g_wp; /* current window                  */
    ATTR *first;      /* first non-space in line         */
    ATTR *last;       /* last non-space in line          */
    ATTR *text1;      /* text of current line            */

    int i;            /* loop counter                    */
    int rightmar;     /* right margin column             */
    int leftmar;      /* left margin column              */
    int disp_blank=0; /* display width of leading spaces */
    int spaces;       /* final # of leading spaces       */
    int diff;         /* # of spaces to add/delete       */
    int count;        /* number of lines to center       */
    int line;         /* zero base field line            */
    int ulline;       /* upper left line of region       */
    int ulcol;        /* upper left col of region        */
    int lrline;       /* lower right line of region      */
    int lrcol;        /* lower right col of region       */
    int width;        /* width of text to center         */

#ifdef DEBUG
    debug ("center called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    switch (argtype)
	{
	case NOARGS:
	    count = 1;
	    line = w_line (wp) + w_ftline (wp);
	    break;

	case NUMBERARG:
	    if ((numvalue + w_line (wp) + w_ftline (wp)) >= MAXFLINE)
		{
		numtoobig ();
		return;
		}

	    count = numvalue;
	    line = w_line (wp) + w_ftline (wp);
	    break;

	case LINEREGION:
	case BOXREGION:
	case TEXTREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    count = lrline - ulline + 1;
	    line = ulline;
	    break;

	default:
	    fatal ("Unexpected argument type %d in center", argtype);
	    break;
	}

    /* determine margins */
    if (w_ismar (wp))
	{
	rightmar = w_rmar (wp);
	leftmar  = w_lmar (wp);
	}
    else
	{
	rightmar = w_lrcol (wp);
	leftmar  = w_lmar (wp);
	}

    /* center count lines */
    for (i = 0; i < count; i++) 
	{

        /* calculate the display width up to the first non-space */
	text1 = getli (line+i);
	for (first = text1, disp_blank = 0 ; *first; skipattr(&first))
            {
	    if (isattrspace(first))
                disp_blank += char_width(first);
            else
		break;
            }

        /* if whole line is spaces */
	if (*first == 0) 
	    {
	    s_free ((char *)text1);
	    continue;
	    }

        /* locate last non-space */
	for (last=text1 + i_strlen((short *)text1)-1; 
                                    last >= first; backupattr(&last))
	    if (!isattrspace(last))
		break;

	/* number of spaces is the width between the margins
	   minus the width of the text divided by two */

	width = calc_column(first, (last - first));
	spaces   = ((rightmar - leftmar) - width) / 2;
	if (spaces <= 0)
	    {
	    s_free ((char *)text1);
	    continue;
	    }

        /* calculate the difference */
	diff = (spaces + leftmar) - disp_blank;
	if (diff > 0)
	    {
	    text1 = (ATTR *) i_insert ((short *)text1, 0, diff);
	    i_smear ((short)(SPACE | FIRST), diff, (short *)text1);
	    }
	else if (diff < 0)
	    text1 = (ATTR *) i_delete ((short *)text1, 0, -diff);

	putli (line+i, text1);
	s_free ((char *)text1);
	}
    }

/****************************************************************************/
/* clearlines:  clears specified array lines in current window              */
/*                                                                          */
/* arguments:              int top, bottom - zero-based window coords       */
/*                                           of lines to be cleared         */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     clearlines loops through the specified lines of the cache            */
/*     of the current window, smearing spaces across each line.             */
/*     Previous contents of cleared lines are freed.                        */
/****************************************************************************/

void clearlines (int top, int bottom)
    {
    WSTRUCT *wp=g_wp;            /* current window              */
    POINTER *cache=w_cache(wp);  /* WG array for window         */

    int   line;
    int   width = w_lrcol (wp) + w_ftcol (wp) + 1;
    short *cache_ptr;

#ifdef DEBUG
    debug ("clearlines: top = %d, bottom = %d", top, bottom);
#endif

    for (line = top; line <= bottom; line++)
	{
	s_free (cache [line]);

        cache_ptr = (short *) s_nzalloc (width + 1, T_ATTR, (char *) NULL);
        cache_ptr [width] = 0;

	cache [line] = (char *) cache_ptr;
	i_smear ((short)(SPACE | FIRST), width, (short *)cache [line]);
	}
    }

/****************************************************************************/
/* delli: delete count lines from window at specified line                  */
/*                                                                          */
/* arguments:              int line  - first line to delete in window coord */
/*                         int count - how many lines to delete             */
/*                         int save  - TRUE if deleted lines should be save */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_helper, g_hooks, g_wnode                 */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     If the given count is negative or zero, return immediately.          */
/*     If the save flag is given, save the lines before deleting them.      */
/*     If a helper is running and it has set hooks on deletion, invoke      */
/*         Hdel to do whatever helper-specific manipulation is desired.     */
/*     Finally, invoke delline to actually delete the lines, and then       */
/*     loop through the current gangs to move up the tv image for any       */
/*     ganged field that touches the deleted lines.                         */
/****************************************************************************/

void delli (int line, int count, int save1)
    {
    WSTRUCT *wp=g_wp;       /* current window                          */
    int wincount;           /* number of lines in the window to remove */

#ifdef DEBUG
    debug ("delli:  i = %d, n = %d, save1 = %d", line, count, save1);
#endif

    if (count <= 0)
        return;

    wincount = min (count - 1, w_lrline (wp) - line);
    if (save1)
        savelines (line, count);

    if ((g_helper) && (g_hooks [UDEL])) /* if helper wants UDEL */
	Hdel (w_name (wp), line + w_ftline (wp), count);

    delline (line + w_ftline (wp), count) ;

    thrugngs
	    {
	    wp = g_wp;

	    /* scroll wG, tv and screen */
	    if (count <= w_lrline (wp))
		tvup (count, line, w_lrline (wp));
	    }
    endthrugngs

    /* redisplay window and gangs */
    wp = g_wp;
    getgngs (w_lrline (wp) - wincount, w_lrline (wp));
    }

/****************************************************************************/
/* delsp:  deletes count chars at specified line position                   */
/*                                                                          */
/* arguments:              int line     - starting line                     */ 
/*                         int position - line position                     */
/*                         int count    - number of characters to delete    */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     delsp deletes count characters from the given window line, starting  */
/*     at the given position. If the count exceeds the number of characters */
/*     after the position in the line, only the remainder of the line is    */
/*     deleted.  The deletion is done by copying any characters after the   */
/*     last deleted position into the first deleted position, smearing      */
/*     spaces over the last part of the line, invoking tvlf to update the   */
/*     screen.                                                              */
/****************************************************************************/

void delsp (int line, int position, int count)
    {
    WSTRUCT *wp=g_wp;    /* current window                          */
    ATTR    *text2;      /* one line from cache                     */

    int     column;      /* screen column relating to 'position'    */
    int     length;      /* number of chars in current line         */

#ifdef DEBUG
    debug ("delsp: line = %d, position = %d, count = %d",
	line, position, count);
#endif

    text2  = (ATTR *) w_text (wp, line);
    length = (obj_count (text2) - 1);
    count  = min (count, length - position);

    (void) i_strcpy ((short *)(text2 + position),
                     (short *)(text2 + position + count));

    i_smear ((short)(SPACE | FIRST), count, (short *)(text2 + length - count));
    w_setmod (wp, line);
    wp->w__cache [line] = (char *) text2;

    /* scroll tv and screen */
    column = calc_column(text2, position) - w_ftcol(wp);
    tvlf (count, line, line, column, w_lrcol (wp)); 
    }

/****************************************************************************/
/* insli: insert count lines before tv line                                 */
/*                                                                          */
/* arguments:              int line  - starting line, 0-based window crds   */
/*                         int count - number of lines to insert            */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_wnode, g_helper, g_hooks                 */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     insli inserts count lines before the given line.  If the             */
/*     count is negative or zero, it returns immediatly.                    */
/*     Otherwise it adds new lines to the file, if necessary, so            */
/*     that there are enough for the insertion.  If a helper is             */
/*     running and it has set a hook on insertion, invoke Hins              */
/*     before actually doing the insertion.  Finally, do the                */
/*     insertion by looping through the ganged fields, scrolling            */
/*     down any that cross the affected lines, and invoking                 */
/*     clearlines and sendbox to put spaces into the inserted               */
/*     region and display it.                                               */
/****************************************************************************/

void insli (int line, int count)
    {
    WSTRUCT *wp=g_wp;       /* current window                          */
    int wincount;           /* number of lines in the window to remove */

#ifdef DEBUG
    debug ("insli:  line = %d, count = %d", line, count);
#endif

    if (count <= 0)
        return;

    wincount = min (count - 1, w_lrline (wp) - line);
    putgng (w_lrline (wp) - wincount, w_lrline (wp));
    insline (line + w_ftline (wp), count); /* do insert in file */

    if ((g_helper) && (g_hooks [UINS])) /* if helper wants UINS */
	Hins (w_name (wp), w_line (wp) + w_ftline (wp), count);

    thrugngs
	    {
	    wp = g_wp;

	    /* scroll wG, tv and screen */
	    if (count <= w_lrline (wp))
		tvdn (count, line, w_lrline (wp));

	    clearlines (line, line + wincount);
	    sendbox (line, line + wincount, 0, w_lrcol (wp));
	    }
    endthrugngs;
    }

/****************************************************************************/
/* inssp: insert n spaces at tv line i, position j.                         */
/*                                                                          */
/* arguments:              int line     - window line in which to insert    */
/*                         int position - line position for insertion       */
/*                         int count    - number of spaces to insert        */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     inssp inserts count spaces at the given position of the given line.  */
/*     If possible, spaces at the end of the line are used.  If necessary,  */
/*     i_insert is used to re-allocate the window cache for the line and    */
/*     spaces are smeared in the inserted positions. The setmod bit is set    */
/*     for the line to show it has been modified and tvrt is invoked to     */
/*     update the goal screen.                                              */
/****************************************************************************/

void inssp (int line, int position, int count)
    {
    WSTRUCT *wp=g_wp;    /* current window                          */
    ATTR    *text3;      /* one line of cache                       */
    ATTR    *cp1;        /* pointer into line of cache              */
    ATTR    *cp2;        /* pointer into line of cache              */

    int column;          /* screen column relating to 'position'    */
    int mustalloc=TRUE;

#ifdef DEBUG
    debug ("inssp: line = %d, position = %d, count = %d",line,position,count);
#endif

    text3 = (ATTR *) w_text (wp, line);
    if (count <= ((obj_count (text3) - 1) - position - 1))
	{
	cp1 = text3 + (obj_count (text3) - 1) - count;
        skipattrspaces(&cp1);
	if (cp1 == (text3 + (obj_count (text3) - 1)))
	    {
	    cp2 = text3 + (obj_count (text3) - 2);
	    cp1 = cp2 - count;
	    while (cp1 >= text3 + position) *cp2-- = *cp1--;
	    mustalloc = FALSE;
	    }
	}

    if (mustalloc)
	text3 = (ATTR *) i_insert ((short *)text3, position, count);

    i_smear ((short)(SPACE | FIRST), count, (short *)(text3 + position));
    wp->w__cache [line] = (char *) text3;
    w_setmod (wp, line);

    /* scroll tv and screen */
    column = calc_column(text3, position) - w_ftcol(wp);
    tvrt (count, line, line, column, w_lrcol (wp)); 
    }

/****************************************************************************/
/* join:  joins one line to the next at a specified location                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     join is used to join one line to the next.  If the current field     */
/*     is indexed, join invokes Eerror and returns.  Otherwise it searches  */
/*     for the first non-white-space character on the line below the cursor */
/*     and uses textpick to close the text region between the cursor and    */
/*     that character position.  If there are no non-white characters       */
/*     on the next line, all remaining characters on the current line       */
/*     are blanked out and the next line is deleted.                        */
/****************************************************************************/

void join (void)
    {
    WSTRUCT *wp=g_wp;    /* current window                      */
    ATTR    *text4;      /* text of next line                   */
    ATTR    *text4p;     /* points into text4                   */

    int i;               /* used for find first non-blank char  */
    int line;            /* line in 0 based field coords        */
    int joincol;         /* screen column relating to 'i'       */

#ifdef DEBUG
    debug ("join called");
#endif

    if (getstat (w_flags (wp), INDEXED))
	{
	Eerror (M_EIDXJOIN, "Cannot join lines in the field where the cursor is located.");
	return;
	}

    line  = w_line (wp) + w_ftline (wp);
    text4 = getli (line + 1);

    /* find first non-blank char, let i be the index of that char */
    text4p = text4;
    skipattrspaces(&text4p);
    i = text4p - text4;

    /* if all chars are blank */
    if (text4[i] == 0)
	i = 0;

    joincol = calc_column(text4, i);
    textpick (line, w_col(wp) + w_ftcol(wp), line + 1, joincol, TRUE);
    s_free ((char *)text4);
    }

/****************************************************************************/
/* movlf:  move window left in file a given number of cols                  */
/*                                                                          */
/* arguments:              int count - how many columns to scroll left      */
/*                                                                          */
/* return value:           int - how many columns actually moved            */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     movlf moves the window left by the specified number of columns.      */
/*     If the count is larger than the distance from the left edge of       */
/*     the field to the left edge of the window, the window only scrolls    */
/*     to the left edge.  The actual scrolling is done by invoking          */
/*     tvrt to move the tv right (thus, the window moving left),            */
/*     and fixtabs to redisplay the new tabstops.                           */
/****************************************************************************/

int movlf (int count)
    {
    WSTRUCT *wp=g_wp;    /* current window              */
    ATTR    *text5;      /* one line of cache           */
    int pos, new_ftcol;

#ifdef DEBUG
    debug ("movlf: ftcol = %d, count = %d", w_ftcol(wp), count);
#endif

    if (count == 0)
	return (0);

    /* scroll tv & screen. Make sure wp->w__ftcol does not split
       a multi width character on the line with the cursor. This
       to make sure the cursor winds up at the right position. */
    text5 = (ATTR *)w_text(wp, wp->w__line);
    pos = calc_line_position2(text5, wp->w__ftcol - count, &new_ftcol);
    count = wp->w__ftcol - new_ftcol;

#ifdef DEBUG
    debug ("    : new_ftcol = %d, pos = %d, count = %d", 
           new_ftcol, pos, count);
#endif

    wp->w__ftcol = new_ftcol;

    /* scroll tv and screen */
    tvrt (count, 0, w_lrline(wp), 0, w_lrcol(wp));
    fixtabs();
    return (count);
    }

/****************************************************************************/
/* movrt:  move window right in file a given number of cols                 */
/*                                                                          */
/* arguments:              int count - how many columns to scroll right     */
/*                                                                          */
/* return value:           int - how many columns actually moved            */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     movrt moves the window right by the specified number of columns.     */
/*     If the count is larger than the distance from the right edge of      */
/*     the field to the right edge of the window, the window only scrolls   */
/*     to the right edge.  The actual scrolling is done by invoking         */
/*     tvlf to move the tv left (thus, the window moving right) and         */
/*     fixtabs to redisplay the new tabstops.                               */
/****************************************************************************/

int movrt (int count)
    {
    WSTRUCT *wp=g_wp;      /* current window                   */
    ATTR    *text5;        /* one line of cache                */

    int line;              /* loop counter                     */
    int width;             /* display width of 'text5'         */
    int length;            /* size of 'text5' in bytes         */
    int minwidth;          /* minimum width for all lines      */

    int pos, new_ftcol;

#ifdef DEBUG
    debug ("movrt:  n = %d", count);
#endif

    count = min (count, NAMESIZE - (w_ftcol (wp) + 1 + w_lrcol (wp)));
    if (count <= 0)
	return (0);

    
    /* Make sure new wp->w__ftcol will not split a multi width 
       character on the line with the cursor. This
       to make sure the cursor winds up at the right position.
       Adjust count accordingly.
    */
    text5 = (ATTR *)w_text(wp, wp->w__line);
    pos = calc_line_position2(text5, wp->w__ftcol + count, &new_ftcol);

    /* if the new left hand side position (which is in w__ftcol)
       is to the rigth of the line the cursor is on, the extra
       space will be filled with spaces (see the for loop below),
       which are single width, single byte.
       Take this into account by adjusting new_ftcol.
    */
    if ((new_ftcol < wp->w__ftcol + count) && (text5[pos] == (ATTR)0))
        new_ftcol = wp->w__ftcol + count;

    count = new_ftcol - wp->w__ftcol;

#ifdef DEBUG
    debug ("movrt :  new_ftcol = %d, count = %d, wp->w__ftcol = %d", 
           new_ftcol, count, wp->w__ftcol);
#endif

    minwidth = w_ftcol (wp) + 1 + w_lrcol (wp) + count;
    for (line = 0; line <= w_lrline (wp); line++)
	{
	text5     = (ATTR *) w_text (wp, line);
        length    = obj_count(text5) - 1;
	width     = attr_text_width(text5);
	if (width < minwidth)
	    {
	    text5 = (ATTR *) i_realloc ((short *)text5, 
                                        length + minwidth - width);
	    i_smear ((short)(SPACE | FIRST), minwidth-width, 
                            (short *)(text5 + length));
	    wp->w__cache [line] = (char *) text5;
	    }
#ifdef DEBUG
    debug ("movrt :  width = %d, minwidth = %d, length = %d, line = %d",
           width, minwidth, length, line);
    {
       char *cp = a2string(wp->w__cache[line], minwidth);
       debug("w__cache[line] = '%s'", cp);
       s_free(cp);
    }  
#endif

	}

    /* scroll tv & screen. */
    wp->w__ftcol = new_ftcol;
    tvlf (count, 0, w_lrline (wp), 0, w_lrcol (wp)); 

    fixtabs ();
    return (count);
    }

/****************************************************************************/
/* split:  splits a line at the current cursor position                     */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     split breaks the current line into two at the current cursor         */
/*     position.  If the current field is indexed, it invokes Eerror        */
/*     and returns.  Otherwise, it puts the characters after the cursor     */
/*     on the left margin of the newly-created next line, and blanks out    */
/*     the current line after the cursor.                                   */
/****************************************************************************/

void split (void)
    {
    WSTRUCT *wp=g_wp;    /* current window                          */

#ifdef DEBUG
    debug ("split called");
#endif

    if (getstat (w_flags (wp), INDEXED))
	{
	Eerror (M_EIDXSPLIT, "Cannot split lines in the field where the cursor is located.");
	return;
	}

    textopen (w_line (wp) + w_ftline (wp), w_ftcol (wp) + w_col (wp),
	      w_line (wp) + w_ftline (wp) + 1, w_lmar (wp));
    }
