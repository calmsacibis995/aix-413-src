#if	!defined(lint)
static char sccsid[] = "@(#)07	1.8  src/tenplus/e2/common/cmds.c, tenplus, tenplus411, GOLD410 3/24/93 09:27:41";
#endif	/* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	bol, chfont, delch, e2close, e2do, e2open, e2return,
 *		eol, fbutton, insert, left, linefeed, margin, mnline,
 *		mnpage, newline, nextitem, nospecials, numtoobig,
 *		pick, plline, plpage, print, Refresh, right, save,
 *		tabset, use
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

extern fbackup(char *);
 
/****************************************************************************/
/* file:  cmds.c                                                            */
/*                                                                          */
/* internal command handlers                                                */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "editorprf.h"
#include "INeditor_msg.h"

/****************************************************************************/
/* bol:  beginning of line (BOL) command handler                            */
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
/*     Step through the characters of the current line looking for the      */
/*     first non-space character, calculating the display width of each     */ 
/*     character. If the line is all spaces place the cursor in column 0    */
/*     otherwise place the cursor in the calculated column.                 */
/****************************************************************************/

void bol (void)
{
    WSTRUCT *wp=g_wp;     /* pointer to current window   */ 
    ATTR *cp;             /* used to find first char     */
    int indent_cols=0;    /* number of columns to indent */ 

#ifdef DEBUG
    debug ("bol called");
#endif

    for (cp = (ATTR *) w_text (wp, w_line (wp)); isattrspace(cp); )
	{
        indent_cols += char_width(cp);
	skipattr(&cp);
	}

    /* if all chars are space */
    if (*cp == (ATTR) '\0') 
	indent_cols = 0;

    /* position the cursor */
    putcurs (w_ftline (wp) + w_line (wp), indent_cols);
}

/****************************************************************************/
/* chfont:  handles the FONT command                                        */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_font                                           */
/*                                                                          */
/* globals changed:        g_font                                           */
/*                                                                          */
/* synopsis:                                                                */
/*     If no argument is given, toggle the font back to the previously      */
/*     used font.  If there is no previously used font, toggle into         */
/*     the continuous underline font.                                       */
/*                                                                          */
/*     If a string argument is given, switch into the specified font:       */
/*         r - roman; w - word underlining; c - continuous underlining;     */
/*         or g - graphics characters.                                      */
/****************************************************************************/

void chfont (int argtype, char *strvalue, int numvalue)
    {
    int newfont;                       /* new font to change to   */
    static lastfont = 2;              /* last non-normal font    */
    static POINTER *fonts = NULL;

#ifdef DEBUG
    debug ("chfont called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (argtype == NOARGS)
	{
	newfont = lastfont;
	lastfont = g_font;
	g_font = newfont;
	return;
	}
    newfont  = lastfont; /* remember alt font in case of default below */
    lastfont = g_font; /* remember current font */

    switch (*strvalue)
	{
	case '\0': /* this handles ENTER FONT */
	case 'r':
	    g_font = R_FONT;
	    return;

	case 'w':
	    g_font = W_FONT;
	    return;

	case 'c':
	    g_font = C_FONT;
	    return;

	case 'g':
	    g_font = G_FONT;
	    return;

	default:
	    if (fonts == NULL)
		fonts = s2pointer(Egetmessage(M_EFONTS, "Roman\nword underline\ncontinuous underline\ngraphics", FALSE));
	    Eerror (M_ECHFONT,
	    "The available fonts are:\n\
	    \tr\tRoman\n\
	    \tw\tword underlining\n\
	    \tc\tcontinuous underlining\n\
	    \tg\tgraphics characters\n\
	    The current font is %s.", fonts[g_font]);

	    lastfont = newfont; /* restore alt font */
	}
    }

/****************************************************************************/
/* delch:  handles the DELETE command                                       */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If no arguments are given, delete the character under the cursor.    */
/*     If empty argument (ENTER DELETE), delete all characters from the     */
/*     cursor position to the end of the current line.                      */
/****************************************************************************/

void delch (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;       /* pointer to current window            */
    ATTR *cp;               /* text line with attributes            */ 

    int wline=w_line(wp);   /* line in window coords                */
    int line;               /* zero based field coords              */
    int pos;                /* position in text line                */
    int count;              /* number of chars to delete            */
    int nbytes;             /* Number of bytes in the mb char       */

#ifdef DEBUG
    debug ("delch called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    /* get the text line and determine where the character to be deleted is */
    cp  = (ATTR *) w_text (wp, wline);
    pos = calc_line_position(cp, (w_col(wp)+w_ftcol(wp)));

    if (argtype == NOARGS)
        {
        nbytes=1;
	if (MultibyteCodeSet)
            {
            count = pos + 1;
            while (!firstbyte(cp[count]))
                {
                ++count;
                ++nbytes;
                }
	    }

	delsp (wline, pos, nbytes);
	return;
        }

    /* argtype must be EMPTYARG - delete to end of line */
    count = (obj_count (cp) - 1);
    while (backupattr_idx(cp, &count), isattrspace(&(cp[count])))
	if (count < pos)
	    break;

    /* if there is anything to delete */
    if (count >= pos)
	{
	count += attrlen(&(cp[count]));
	line = wline + w_ftline (wp);
	boxpick (line, line, (w_col(wp)+w_ftcol(wp)), calc_column(cp, count), TRUE);
	}
    }

/****************************************************************************/
/* e2close:  handles the CLOSE command                                      */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If given no arguments, pick up the current line.  If given a         */
/*     numeric argument, pick up that many lines starting at the            */
/*     current line.  If given an empty argument (ENTER CLOSE),             */
/*     pick up that portion of the current line starting at the             */
/*     cursor.  If given a region argument, invoke either delli,            */
/*     textpick, or boxpick to close a region of the desired shape.         */
/****************************************************************************/

void e2close (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;     /* pointer to current window  */
    int      ulline;      /* upper left line of region  */
    int      ulcol;       /* upper left col of region   */
    int      lrline;      /* lower right line of region */
    int      lrcol;       /* lower right col of region  */

#ifdef DEBUG
    debug ("e2close called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    switch (argtype)
	{
	case NOARGS:
	    delli (w_line (wp), 1, TRUE);
	    break;

	case EMPTYARG:
	    join ();
	    break;

	case NUMBERARG:
	    if (numvalue > MAXFLINE)
		{
		numtoobig ();
		return;
		}
	    delli (w_line (wp), numvalue, TRUE);
	    break;

	case LINEREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    delli (ulline - w_ftline (wp), lrline - ulline + 1, TRUE);
	    break;

	case TEXTREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    textpick (ulline, ulcol, lrline, lrcol, TRUE);
	    break;

	case BOXREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    boxpick (ulline, lrline, ulcol, lrcol, TRUE);
	    break;
#ifdef CAREFUL
	default:
	    fatal ("Unexpected argtype %d in e2close", argtype);
	    break;
#endif
	}
    }

/****************************************************************************/
/* e2do:  handles the EXECUTE command                                       */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current field is indexed, bail out with an error message.     */
/*                                                                          */
/*     Otherwise:  if given no argument, execute the previous filter        */
/*     (or give an error message if there is no previous filter);           */
/*     if given an empty argument (ENTER EXECUTE), report the current       */
/*     filter string; if given a string argument, execute that filter.      */
/****************************************************************************/

void e2do (int argtype, char * strvalue, int numvalue)
    {
    static char *dostring = NULL; /* last filter string */

#ifdef DEBUG
    debug ("e2do called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (getstat (w_flags (g_wp), INDEXED))
	{
	Eerror (M_EIDXFILTER, "Do not use the DO command here.",
                "");
	return;
	}

    switch (argtype)
	{
	case NOARGS:

	    if (dostring == NULL)
		Eerror (M_ENOFILTER, "There is no filter string set.","");
	    else
		dofilter (dostring);

	    return;

	case EMPTYARG:

	    Eerror (M_EFILTER,  "The current filter command line is \n%s", dostring);
	    return;

	case STRINGARG:

	    s_free (dostring);
	    dostring = s_string (strvalue);
	    dofilter (dostring);
	}
    }

/****************************************************************************/
/* e2open:  handles the OPEN command                                        */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If given no arguments, insert a blank line before the current        */
/*     line.  If given a numeric argument, insert that many blank           */
/*     lines before the current line.  If given an empty argument           */
/*     (ENTER OPEN), end the current line at the current cursor             */
/*     position, open a new line just below it, and put the                 */
/*     remaining contents of the current line into that new next            */
/*     line, starting at column 0.  If given a region argument,             */
/*     invoke either insli, textopen, or boxopen to open a region of        */
/*     the desired shape.                                                   */
/****************************************************************************/

void e2open (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;     /* pointer to current window  */
    int      ulline;      /* upper left line of region  */
    int      ulcol;       /* upper left col of region   */
    int      lrline;      /* lower right line of region */
    int      lrcol;       /* lower right col of region  */
    int      length;      /* max of field length and cursor position */

#ifdef DEBUG
    debug ("e2open called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    switch (argtype)
	{
	case NOARGS:
	    insli (w_line (wp), 1);
	    break;

	case EMPTYARG:
	    split ();
	    break;

	case NUMBERARG:
	    length = datalength ();
	    length = max (length, w_line (wp) + w_ftline (wp));

	    if ((numvalue + length) >= MAXFLINE)
		{
		numtoobig ();
		return;
		}
	    insli (w_line (wp), numvalue);
	    break;

	case LINEREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    insli (ulline - w_ftline (wp), lrline - ulline + 1);
	    break;

	case TEXTREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    textopen (ulline, ulcol, lrline, lrcol);
	    break;

	case BOXREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    boxopen (ulline, lrline, ulcol, lrcol);
	    break;

#ifdef CAREFUL
	default:
	    fatal ("Unexpected argtype %d in e2open", argtype);
	    break;
#endif
	}
    }

/****************************************************************************/
/* e2return:  handler for the RETURN command                                */
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
/*   If there are more lines in current field, put cursor on next line.     */
/*   Otherwise, if the field is scrollable scroll it, else put cursor in    */
/*   lower left window relative to the cursor position.                     */
/****************************************************************************/

void e2return (void)
    {
    WSTRUCT *wp=g_wp;

#ifdef DEBUG
    debug ("e2return called");
#endif

    if ((w_line (wp) < w_lrline (wp)) ||
	(getstat (w_flags (wp), CRSCROLL)))
	newline ();           		    /* this can cause scrolling */
    else
	(void) winlf ();
    }

/****************************************************************************/
/* eol:  end of line (END_LINE) command handler                             */
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
/*     Loop through the characters of the current line backwards,           */
/*     stopping at the first (rightmost) non-space character.  If           */
/*     all characters on the line are spaces, put the cursor in the         */
/*     first column; otherwise put the cursor just to the right of          */
/*     the first (rightmost) non-space character.                           */
/****************************************************************************/

void eol (void)
    {
    int fdline;         /* cursor line in 0 based field coords */
    int col;            /* column to put the cursor            */

    WSTRUCT *wp=g_wp;   /* pointer to current window           */
    ATTR    *text1;     /* text for current line               */
    ATTR    *cp;        /* used to find last char              */
    int     len_text1;  /* length in ATTRs of text1.           */

#ifdef DEBUG
    debug ("eol called. ");
#endif

    text1  = (ATTR *) w_text (wp, w_line (wp));
    fdline = w_ftline (wp) + w_line (wp);
    len_text1 = i_strlen((short *)text1);

#ifdef DEBUG
    debug ("eol:  text1 = %s", text1);
    debug ("eol:  len_text1 = %d", len_text1);
#endif

    if (len_text1 > 0) {
        cp = text1 + len_text1;
        backupattr(&cp); 
        for (;;)
    	{
    
    	/* first non-space - put the cursor in the next column */
    	if (!isattrspace(cp))
    	    {
            skipattr(&cp);
            col = calc_column(text1, cp-text1);
    	    putcurs (fdline, col); 
    	    return;
    	    }
        if (cp > text1)
              backupattr(&cp);
        if (cp <= text1)
              break;
    	}
    }

    /* no text on line */
    putcurs (fdline, 0);
    }

/****************************************************************************/
/* fbutton:  error handler for function buttons                             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     fbutton is invoked only when the user hits a function key when       */
/*     no helper is active, or in a helper which does not use all the       */
/*     function buttons.  It simply issues an error message explaining      */
/*     the situation.                                                       */
/****************************************************************************/

void fbutton (void)
    {
    char g_keyvalue_str[20];

    (void) sprintf(g_keyvalue_str, "%d", g_keyvalue);
    Eerror (M_EFBUTTON, "Function key %d is not defined for this type of file.",
        g_keyvalue_str); /* NOTE:  this assumes U1 - U8 are defined as 1-8 */
    }

/****************************************************************************/
/* insert:  handler for the INSERT-MODE command                             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_imodeflag g_wmodeflag                          */
/*                                                                          */
/* globals changed:        g_imodeflag g_wmodeflag                          */
/*                                                                          */
/* synopsis:                                                                */
/*     insert simply toggles the g_imodeflag global back and forth between  */
/*     TRUE and FALSE to change the editor mode between INSERT and          */
/*     OVERWRITE. If we are dealing with a multibyte codeset the            */
/*     g_wmodeflag is toggled between WORDWRAP and CHARWRAP                 */
/****************************************************************************/

void insert (int type)
    {
#ifdef DEBUG
     debug ("insert called");
#endif

    if (MultibyteCodeSet)
	{
	if (type == EMPTYARG)
	    g_wmodeflag ^= 1;
	else
	    if (type == NOARGS)
		g_imodeflag ^= 1;
	}
    else
	g_imodeflag ^= 1;
    }

/****************************************************************************/
/* left:  handler for the LEFT command                                      */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     left moves the cursor to the left (i.e. the window moves right).     */
/*     If given a numeric argument, try to move the cursor that many        */
/*     spaces left; otherwise move the cursor 1/3 window width to the       */
/*     left.  In either case, do not move the cursor beyond the left        */
/*     margin.                                                              */
/****************************************************************************/

void left (int argtype, char *strvalue, int numvalue)
    {
    int count;         /* number of columns to move */
    WSTRUCT *wp=g_wp;  /* pointer to current window */

#ifdef DEBUG
    debug ("left: argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
    debug ("left: lrcol = %d, ftcol = %d, w_col = %d",
	w_lrcol(wp), w_ftcol(wp), w_col(wp));
#endif

    /* don't bother if where already on the left hand side */
    if (w_ftcol(wp) == 0)
        return;

    if (argtype == EMPTYARG)
	{
	movlf (min(w_lrcol(wp)-w_col(wp), w_ftcol(wp)));
	wp->w__col = w_lrcol(wp);
	return;
	}

    count = (argtype == NUMBERARG) ? numvalue : (w_lrcol(wp) / 3) + 1;
    wp->w__col += movlf(min(count, w_ftcol(wp)));

    if (w_col(wp) > w_lrcol(wp))
	wp->w__col = w_lrcol(wp);
    }

/****************************************************************************/
/* linefeed:  handler for the LINEFEED command                              */
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
/*   If in a ganged field and not on last line, put cursor on next line of  */
/*       the leftmost field in the gang.                                    */
/*   If on bottom line of ganged field that isn't scrollable, beep.         */
/*   If on bottom line of scrollable ganged field, scroll gang and put the  */
/*       cursor of next line of the leftmost field in the gang.             */
/*   If in non-ganged field try to move to a lower left window.  If there   */
/*       isn't one, beep.                                                   */
/****************************************************************************/

void linefeed (void)

    {
    WSTRUCT *wp=g_wp;        /* current window              */

    flush_window ();

#ifdef DEBUG
    debug ("linefeed:  w_line = %d, w_nxtgang = %d",
	    w_line (wp), w_nxtgang (wp));
#endif

    if (w_nxtgang (wp)) /* if ganged */
	{
	if ((!getstat (w_flags (wp), CRSCROLL)) &&
	    (w_line (wp) == w_lrline (wp)))
	    {
	    Ebell ();
	    return;
	    }
	thrugngs
	    if (getstat (w_flags (g_wp), HGANGED)) /* find left most window */
		{
		g_wp->w__line = w_line (wp); /* put cursor on same line */
		g_wnode = (char *) ERROR;
		break;
		}
	endthrugngs;

	newline (); /* this can cause scrolling */
	return;
	}

    if (winlf () == ERROR)
	Ebell ();
    }

/****************************************************************************/
/* margin:  handler for the MARGIN command                                  */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If given no arguments, try to set the left margin to the current     */
/*     cursor column.  If given an empty argument (ENTER MARGIN), try to    */
/*     set the right margin to the current cursor column.  Setting the left */
/*     margin directly on top of the right has the effect of turning        */
/*     off the right margin, while setting the right margin on top of the   */
/*     left turns off both margins. The left margin is actually always      */
/*     present in column 1, but is only displayed if it is in a column      */
/*     other than column 1.  Error messages are given if the user           */
/*     attempts to set the left margin to the right of the right, or        */
/*     right to left of left, or if the field is part of a gang, or         */
/*     if the user attempts to set the right margin in the rightmost        */
/*     column.  An error message is also given if a left margin is set to   */
/*     the right of the right side of the window, and there is no right     */
/*     margin, since the right side of the window is the default right      */
/*     margin.                                                              */
/****************************************************************************/

void margin (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;     /* pointer to current window  */
    int      margcol;     /* current cursor column      */
    int      ulline;      /* upper left line of region  */
    int      ulcol;       /* upper left col of region   */
    int      lrline;      /* lower right line of region */
    int      lrcol;       /* lower right col of region  */

#ifdef DEBUG
    debug ("margin called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (w_nxtgang (wp))
	{
	Eerror (M_EGNGMARGIN, "Do not use margins in list fields.","");
	return;
	}

    if (w_lrline (wp) == 0) /* 1 line field */
	{
	Eerror (M_EONELINE, "Do not set margins in a one line field.","");
	return;
	}

    margcol = w_col (wp) + w_ftcol (wp);
    switch (argtype)
	{
	case NOARGS:
	    if ((w_rmar (wp) && margcol > w_rmar (wp)) ||
		(w_rmar (wp) == 0 && margcol > w_lrcol (wp)))
		{
		Eerror (M_ELMARGIN,
		    "Do not set the left margin to the right of the right margin.","");
		return;
		}

	    /* lmar on top of rmar shuts right margin off */
	    else if (w_rmar (wp) && margcol == w_rmar (wp))
		{
		wp->w__rmar = 0;

		/* now solitary left margin may be too far right; cancel it */
		if (w_lmar (wp) > w_lrcol (wp))
		    wp->w__lmar = 0;
		}
	    else
		wp->w__lmar = margcol;
	    break;

	case EMPTYARG:


	    if (margcol < w_lmar (wp))
		{
		Eerror (M_ERMARGIN,
		    "Cannot set the right margin to the left of the left margin.","");
		break;
		}
	    /* rmar on top of lmar shuts both margins off */
	    else if (margcol == w_lmar (wp))
		wp->w__lmar = wp->w__rmar = 0;
	    else
		wp->w__rmar = margcol;
	    break;

	case BOXREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    wp->w__lmar = ulcol;
	    wp->w__rmar = lrcol;
	    break;
	case TEXTREGION:
	case LINEREGION:
	    Eerror (M_ENOTEXTLINE, "MARGIN does not accept TEXT region input.","");
	    break;

#ifdef CAREFUL
	default:
	    fatal ("Unexpected argtype %d in margin", argtype);
	    break;
#endif
	}
    }

/****************************************************************************/
/* mnline:  handler for -LINE command                                       */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     mnline moves the cursor up toward the beginning of the field,        */
/*     i.e. moves the window down the screen.  If given a numeric           */
/*     argument, it moves that many lines up; if given no argument,         */
/*     it moves up 1/3 of the window height; if given an empty argument     */
/*     (ENTER -LINE) it moves up enough lines so that the current line      */
/*     becomes the bottom line of the window.  In no case does the          */
/*     cursor get above the first line of the field.                        */
/****************************************************************************/

void mnline (int argtype, char *strvalue, int numvalue)
    {
    int count;           /* number of lines to open   */
    WSTRUCT *wp=g_wp;    /* pointer to current window */

#ifdef DEBUG
    debug ("mnline called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (argtype == EMPTYARG)
	{
	(void) movup (w_lrline (wp) - w_line (wp));
	wp->w__line = w_lrline (wp);
	return;
	}

    count = (argtype == NUMBERARG) ? numvalue : (w_lrline (wp) / 3) + 1;
    count = movup (count);
    wp->w__line = min (w_line (wp) + count, w_lrline (wp));
    }

/****************************************************************************/
/* mnpage:  handler for the -PAGE command                                   */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     mnpage moves the cursor toward the beginning of the field.           */
/*     If given a numeric argument, it moves that many pages toward         */
/*     the beginning; otherwise it moves one page back.  A "page"           */
/*     is defined to be the number of lines that fits in the current        */
/*     window.  In no case does it move beyond the beginning of the field.  */
/****************************************************************************/

void mnpage (int argtype, char *strvalue, int numvalue)
    {
    int count;           /* number of pages to go back */
    WSTRUCT *wp=g_wp;    /* pointer to current window  */

#ifdef DEBUG
    debug ("mnpage called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    count = (argtype == NUMBERARG) ? numvalue : 1;
    (void) movup ((w_lrline (wp) + 1) * count);
    }

/****************************************************************************/
/* newline:  does a carriage return (scrolling if necessary)                */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Move the cursor to the next line, scrolling the screen if            */
/*     necessary.  The new cursor is positioned at the first non-space      */
/*     character of the new line, if it exists; otherwise directly          */
/*     under the first non-space character of the line moved from,          */
/*     if it exists; otherwise at the left margin.                          */
/****************************************************************************/

void newline (void)

    {
    WSTRUCT *wp=g_wp;            /* pointer to current window         */
    ATTR *cp;                    /* used to find indentatin           */

    int thisindent=w_lmar(wp);   /* indentation of current line       */
    int nextindent=w_lmar(wp);   /* indentation of next line          */
    int newcolumn;               /* new col in 0 based field coords   */
    int pos;                     /* left margin position in text line */
#ifdef DEBUG
     debug ("newline:  w_line = %d, leftmargin = %d",
	     w_line (wp), w_ismar (wp) ? w_lmar (wp) : 0);
#endif

    /* get the text line and determine the position of the left margin */
    cp  = (ATTR *) w_text(wp, w_line(wp));
    pos = calc_line_position(cp, w_lmar(wp));

    /* count up the columns used by leading white space */
    for (cp = cp+pos; isattrspace(cp); skipattr(&cp))
	thisindent += char_width(cp);

    /* move to next line, via scrolling if necessary */
    if (w_line (wp) < w_lrline (wp))
	wp->w__line++;
    else 
        if (getstat (w_flags (wp), CRSCROLL))
	    movdn (1);

    /* count up the columns used by leading white space on the new line */
    cp  = (ATTR *) w_text(wp, w_line(wp));
    for (cp = cp+pos; isattrspace(cp); skipattr(&cp))
	nextindent += char_width(cp);

    /* if line has non-space char, put cursor on it */
    if (nextindent <= w_lrcol (wp))
        putcurs (w_ftline(wp) + w_line(wp), nextindent);

    /* if previous line has non-space, use old indent */
    else
        if (thisindent <= w_lrcol(wp))
            putcurs (w_ftline(wp) + w_line(wp), thisindent);

        /* otherwise, use left margin */
        else
            putcurs (w_ftline(wp) + w_line(wp), w_lmar(wp));

#ifdef DEBUG
     debug ("newline:  thisindent = %d, nextindent = %d, w_lrcol = %d",
	     thisindent, nextindent, w_lrcol (wp));
#endif
    }

/****************************************************************************/
/* nextitem:  handles the NEXTITEM and PREVITEM commands                    */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If invoked with no arguments, go to the next (or previous) sibling   */
/*     if it exists.  If given a string argument, attempt to go to the      */
/*     sibling named.                                                       */
/****************************************************************************/

void nextitem (int argtype, char *strvalue, int numvalue)
    {
#ifdef DEBUG
    debug ("nextitem called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    switch (argtype)
	{
	case NOARGS:
	    nextsibling (g_keyvalue == NEXT);
	    return;

	case STRINGARG:
	    Eerror (M_EBADNEXT, "Do not specify a string paramter to NEXT or PREVIOUS.",
                    "");
	    return;

	case NUMBERARG:
	    if (numvalue > MAXFLINE)
		{
		numtoobig ();
		return;
		}
	    if (gotosibling (strvalue) == ERROR)
		Eerror (M_ENOSIBLING, "Do not use NEXT or PREVIOUS here.",
				     "");
	}
    }

/****************************************************************************/
/* nospecials:  prints error message for unused function button             */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     nospecials is invoked when the user hits the LOCAL-MENU button and   */
/*     the helper has set no hook on that key.  It simply puts up a message */
/****************************************************************************/

void nospecials (void)

    {
    Eerror (M_ENOSPECIALS, "There is no LOCAL-MENU for this data.","");
    }

/****************************************************************************/
/* numtoobig:  puts up error message for bounds checking code               */
/****************************************************************************/

void numtoobig (void)
    {
    Eerror (M_ENUMTOOBIG, "The numeric parameter is too large.","");
    }

/****************************************************************************/
/* pick:  handles the PICK command                                          */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If given a numeric argument, pick up that many lines starting        */
/*     at the current line; if given any of the three region argument       */
/*     types, pick up a region of the desired shape; otherwise pick up      */
/*     the current line.  In either case, position the cursor at the        */
/*     current column on the line below the last line picked up.            */
/****************************************************************************/

void pick (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;     /* pointer to current window  */
    int      ulline;      /* upper left line of region  */
    int      ulcol;       /* upper left col of region   */
    int      lrline;      /* lower right line of region */
    int      lrcol;       /* lower right col of region  */

#ifdef DEBUG
    debug ("pick called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    switch (argtype)
	{
	case NOARGS:
	    numvalue = 1;       /* fall through to NUMBERARG case */

	case NUMBERARG:
	    if (numvalue > MAXFLINE)
		{
		numtoobig ();
		return;
		}
	    savelines (w_line (wp), numvalue);
	    if (w_lrline (wp)) /* don't scroll single line fields */
		putcurs (w_line (wp) + w_ftline (wp) + numvalue,
			 w_col (wp) + w_ftcol (wp));
	    break;

	case LINEREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    savelines (ulline - w_ftline (wp), lrline - ulline + 1);
	    break;

	case TEXTREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    textpick (ulline, ulcol, lrline, lrcol, FALSE);
	    break;

	case BOXREGION:
	    Lgetregion (strvalue, &ulline, &ulcol, &lrline, &lrcol);
	    boxpick (ulline, lrline, ulcol, lrcol, FALSE);
	    break;

#ifdef CAREFUL
	default:
	    fatal ("Unexpected argtype %d in pick", argtype);
	    break;
#endif
	}
    }

/****************************************************************************/
/* plline:  handler for the +LINE command                                   */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     plline moves the cursor toward the end of the field.                 */
/*     If given a numeric argument, it moves that many lines.               */
/*     If given no arguments, it moves 1/3 of the window length             */
/*     lines.  If given an empty argument (ENTER +LINE), it moves           */
/*     enough lines to make the current line be the first line              */
/*     of the window.  In no case does the cursor go beyond the             */
/*     end of the field.                                                    */
/****************************************************************************/

void plline (int argtype, char *strvalue, int numvalue)
    {
    int count;           /* number of lines to open   */
    WSTRUCT *wp=g_wp;    /* pointer to current window */

#ifdef DEBUG
    debug ("plline called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (argtype == EMPTYARG)
	{
	movdn (w_line (wp));
	wp->w__line = 0;
	return;
	}
    if ((argtype == NUMBERARG) &&
	((numvalue + w_line (wp) + w_ftline (wp)) >= MAXFLINE))
	{
	numtoobig ();
	return;
	}

    count = (argtype == NUMBERARG) ? numvalue : (w_lrline (wp) / 3) + 1;
    wp->w__line = max (w_line (wp) - count, 0);
    movdn (count);
    }

/****************************************************************************/
/* plpage:  handler for the +PAGE command                                   */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     plpage moves the cursor down toward the end of the field             */
/*     one or more pages.  If given a numeric argument, it moves            */
/*     that many pages; otherwise it moves ahead one page.  In              */
/*     neither case does it move beyond the end of the field.               */
/****************************************************************************/

void plpage (int argtype, char *strvalue, int numvalue)
    {
    int count;           /* number of lines to open   */
    WSTRUCT *wp=g_wp;    /* pointer to current window */

#ifdef DEBUG
    debug ("plpage called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if ((argtype == NUMBERARG) &&
	((w_line (wp) + w_ftline (wp) + (numvalue * (w_lrline (wp) + 1)))
	  >= MAXFLINE))
	{
	numtoobig ();
	return;
	}

    count = (argtype == NUMBERARG) ? numvalue : 1;
    movdn ((w_lrline (wp) + 1) * count);
    }

/****************************************************************************/
/* print:  handles the PRINT command                                        */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_helper, g_filename, g_curdir                   */
/*                                                                          */
/* globals changed:        g_helper                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     print begins by remembering the name of the current helper.          */
/*     Then, if a hook is set on the pseudo-key CHANGEFILE, it calls        */
/*     Hbefore with the current filename as argument.  Then it invokes      */
/*     the print helper to perform the actual print.  Finally, it           */
/*     restores the helper that was originally in effect.  If that          */
/*     helper was the directory helper, it must restore the helper          */
/*     with the current directory as argument; otherwise it restores        */
/*     the helper on the current file.                                      */
/****************************************************************************/

void print (void)
    {
#ifdef DEBUG
    debug ("print called");
#endif

    /* guarantee HMOD appropriately called */
    putgng (0, w_lrline (g_wp));

    /* poke the index helper so we don't end up on /tmp by mistake      */
    if (g_hooks [CHANGEFILE])
	Hbefore (CHANGEFILE, STRINGARG, g_filename, 0);

    pushstate (TRUE);
    openhelper ("print", g_filename, NULL);
    }

/****************************************************************************/
/* Refresh:  handler for the REFRESH command                                */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_dispflag                       */
/*                                                                          */
/* globals changed:        g_wp, g_dispflag                                 */
/*                                                                          */
/* synopsis:                                                                */
/*     Refresh resets the terminal and flushes the current window.  Then    */
/*     it sets the g_dispflag global to DISPINIT to force everything to     */
/*     be redisplayed, regardless of whether it should already be on the    */
/*     screen or not.                                                       */
/****************************************************************************/

void Refresh (void)

    {
    WSTRUCT *oldW=g_wp;   /* used to remember window   */

#ifdef DEBUG
    debug ("Refresh called");
#endif

    flush_window ();
    g_wp = g_warray;

    /* send invariant text */
    sendbox (0, w_lrline (g_wp), 0, w_lrcol (g_wp)); 
    g_wp = oldW;

    /* do getlines on all windows and force redisplay of everything */
    getall (); 
    g_dispflag = DISPINIT; 
    }

/****************************************************************************/
/* right:  handler for the RIGHT command                                    */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     right moves the cursor to the right (i.e. the window moves left)     */
/*     If given a numeric argument, try to move the cursor that many        */
/*     spaces right; otherwise move the cursor 1/3 window width to the      */
/*     right.  In either case, do not move the cursor beyond the right      */
/*     margin.                                                              */
/****************************************************************************/

void right (int argtype, char *strvalue, int numvalue)
    {
    int count;             /* number of columns to move */
    WSTRUCT *wp=g_wp;      /* pointer to current window */

#ifdef DEBUG
    debug ("right:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (argtype == EMPTYARG)
	{
	movrt (w_col (wp));
	wp->w__col = 0;
	return;
	}

    count = (argtype == NUMBERARG) ? numvalue : (w_lrcol (wp) / 3) + 1;
    wp->w__col -= movrt (count);

    if (w_col (wp) < 0)
	wp->w__col = 0;
    }

/****************************************************************************/
/* save:  handler for the SAVE command                                      */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_recdirty, g_fakefile, g_filename, g_curdir     */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If invoked with no arguments, see whether the current file has       */
/*     been changed and, if so, save it.  If it has not been changed,       */
/*     put up an error message.                                             */
/*                                                                          */
/*     If invoked with a string argument, first save the current file       */
/*     and then copy it into the named file.                                */
/****************************************************************************/

void save (int argtype, char *strvalue, int numvalue)
    {
    static char *user_profile;

    FILE *infp;               /* used to copy file to specified name */
    FILE *outfp;

    char filename [PATH_MAX]; /* used for ENTER SAVE processing      */
    char *absname;            /* absolute version of filename        */

    flush_window (); /* get new info off screen */

#ifdef DEBUG
    debug ("save:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
    debug ("save:  g_recdirty = %s, g_filename = %s, sf_eofloc (g_sfp) = %D",
	g_recdirty ? "YES" : "NO", g_fakefile ? "YES" : "NO",
	sf_eofloc (g_sfp));
    debug ("save:  obj_getflag (g_sfp, SF_MODIFIED) = %s",
	obj_getflag (g_sfp, SF_MODIFIED) ? "YES" : "NO");
#endif

    if ((argtype == NOARGS) &&    /* if save w/o file name and     */
	(!g_recdirty) &&       /* if file has not been modified */
	(((!g_fakefile) && (!obj_getflag (g_sfp, SF_MODIFIED))) ||
	 (g_fakefile && (sf_eofloc (g_sfp) == EMPTYEOF))))
	{
	Emessage (M_ENOTSAVED, "File %s has not been modified.", g_filename);
	return;
	}
    if (!g_fakefile) /* Esaveascii has it's own message */
	Emessage (M_ESAVE1, "Saving file %s.", g_filename);


    /* if the file being saved is the user's editor profile, then it should
       be reread to get the new information */

    if (user_profile == NULL)
	{
	/* determine the absolute path of the user's editorprf file */
	user_profile = Efixvars (USER_PROFILE);
	/* remove double slash in case of home directory of / */
	if (user_profile [0] == DIR_SEPARATOR && user_profile [1] == DIR_SEPARATOR)
	    user_profile = s_delete (user_profile, 0, 1);
	}

    Esavefile ();

#ifdef DEBUG
    debug ("save:  g_filename = '%s', user_profile = '%s'",
	g_filename, user_profile);
#endif

    if (seq (g_filename, user_profile))
	{
	Emessage (M_ESAVE2, "Reading editor profile %s.", g_filename);
	(void) readprofile (user_profile);
	}

    if (argtype == NOARGS || argtype == EMPTYARG)
	return;

    (void) strcpy (filename, strvalue);

    absname = abspath (filename, g_curdir);

    if (seq (absname, g_filename))
	{
	s_free (absname);
	return;
	}

    saveascii (absname); /* just in case it's an ASCII file */
    if (fbackup (absname) == ERROR)
	{
	s_free (absname);
	return;
	}

    if ((outfp = efopen (absname, "w")) == NULL)
	{
	s_free (absname);
	return;
	}

    infp = ffopen (g_filename, "r", "save");

    Emessage (M_ESAVEFROMCOPY, "Copying from %s.", g_filename);
    Emessage (M_ESAVETOCOPY,   "Copying to %s.", absname);

    copyfile (infp, outfp);
    (void)fclose (infp); /* not checking, how could it fail? */

    if (fclose (outfp) == EOF)
	Eerror (M_ESAVECLOSE, "Cannot close file %s.\n\
The disk may be full. Use local problem reporting procedures.", absname);

    /* if saving the file as the alternate file, throw out the forms cache
       to eliminate the caching that has been done for the alternate file */

    if (seq (absname, altname ()))
	{
	s_free ((char *)g_formcache);
	g_formcache = NULL;
	}

    s_free (absname);
    }

/****************************************************************************/
/* tabset:  handler for the TABSET command                                  */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current field does not have tab stops, do nothing.            */
/*     Otherwise, if invoked with no argument, set a tab stop at            */
/*     the current cursor column; if invoked with an empty argument         */
/*     (ENTER TABSET) remove a tab stop at the current cursor column.       */
/****************************************************************************/

void tabset (int argtype, char *strvalue, int numvalue)
    {
    WSTRUCT *wp=g_wp;    /* pointer to current window */
    int tablen;          /* length of tab rack */
    int tabpos;          /* position in tab rack to put character */

#ifdef DEBUG
    debug ("tabset called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (w_tabs (wp) == NULL) /* if can't set tabs */
	{
	Eerror (M_ENOSETTAB, "Tabs are not allowed in the field where the cursor is located.","");
	return;
	}

    /* expand tab rack if necessary. Note the tab rack is always ASCII, 
       hence a display width of 1 per character can be assumed  */

    if ((tabpos = w_col (wp) + w_ftcol (wp)) >= (tablen = strlen (w_tabs (wp))))
	{
	wp->w__tabs = s_realloc (w_tabs (wp), tabpos + 2);
	wp->w__tabs [tabpos+1] = '\0';
	smear (SPACE, tabpos + 1 - tablen, w_tabs (wp) + tablen);
	}

    /* make sure new tab rack is displayed */
    g_goalcrc [0] = ++g_uniqnum;
    if (argtype == NOARGS)
	{
	wp->w__tabs [tabpos] = 't';
	return;
	}

    /* ENTER TABSET removes the tab */
    wp->w__tabs [tabpos] = SPACE;
    }

/****************************************************************************/
/* use:  handler for the USE button                                         */
/*                                                                          */
/* arguments:              int   argtype  - NOARGS, EMPTYARG, ...           */
/*                         char *strvalue - argument if STRINGARG           */
/*                         int   numvalue - argument if NUMBERARG           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_formname                                       */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If invoked with no argument, change to the alternate file.           */
/*     If invoked with an empty argument (ENTER USE), change to             */
/*     the file whose name starts at the current cursor position.           */
/*     If invoked with a string argument, change to the named file.         */
/****************************************************************************/

void use (int argtype, char *strvalue, int numvalue)
    {
    char *dirname;
    char *file;
    char *cp;
    extern char g_helpstate;
    int number;
    wchar_t *pwcs;
    wchar_t *pwc;

    char filename [PATH_MAX]; /* used for ENTER USE processing */

#ifdef DEBUG
    debug ("use called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    switch (argtype)
	{
	case NOARGS:
	    if (g_helpstate == INHELPFILE)
		{
		g_helpstate = NOTHELPING;
		cleanstack ();
		popstate (TRUE);
		catclose(g_helpfilecatd);
		return;
		}
            altfile ();
            break;

        case EMPTYARG:
	    getword (filename);
	    if (!*filename)
		{
		Eerror (M_EEMPTYNAME, "The cursor must be on a file name or character string.","");
		return;
		}
	    strvalue = filename;
	    /* fall through into STRINGARG code below */

	case STRINGARG:
	    /* check file name validity */
	    /* first truncate trailing spaces */
            if (MultibyteCodeSet) {
                 size_t lenw = strlen(strvalue) + 1;
                 wchar_t *wstrvalue = malloc(lenw * sizeof(wchar_t));
                 int widx = mbstowcs(wstrvalue, strvalue, lenw) - 1;
                 while ((widx >= 0) && iswspace(wstrvalue[widx]))
                    wstrvalue[widx--] = L'\0';
                 (void) wcstombs(strvalue, wstrvalue, lenw);
                 free(wstrvalue);
            }
            else {
	       cp = strvalue + strlen (strvalue) - 1;
	       while (isspace(*cp) && cp >= strvalue)
                   *cp-- = '\0';
            }
  
            /* check for weird characters */
            cp = strvalue;
  
            number = strlen(cp) + 1;
  
            if (MultibyteCodeSet)
            {
                pwc = pwcs=(wchar_t *)malloc( number * sizeof(wchar_t));
                (void) mbstowcs(pwcs,strvalue,number);
                while (*pwc != L'\0')
                {
                  if (!iswprint (*pwc))
                  {
            	  if(Econfirm (M_EODDFNAME, "There are characters that are not printable in the file name\n\
%s\nPress EXECUTE to edit the specified file or press CANCEL or HELP",strvalue,strvalue))
            	    break;
            	else return; /* they didn't chose to edit it */
                  }
                  else pwc++;
                }
                free(pwcs);
            }
            else
            {
              while (*cp != '\0')
              {
                if (!isprint (*cp))
                {
            	  if(Econfirm (M_EODDFNAME, "There are characters that are not printable in the file name\n\
%s\nPress EXECUTE to edit the specified file or press CANCEL or HELP",strvalue,strvalue))
            	    break;
            	else return; /* they didn't chose to edit it */
                }
                else cp++;
              } 
            }

	    if (seq (g_formname, "*win*") && *strvalue != DIR_SEPARATOR)
		{
		dirname = trimpath (s_string (w_filename (g_wp)));
		if (dirname == NULL)
		    dirname = s_string ("/");
		file = pathcat (dirname, s_string (strvalue));
		usefile (file);
		s_free (file);
		}
	    else
		usefile (strvalue);
	}/*Switch*/
    }

