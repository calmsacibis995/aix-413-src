#if !defined(lint)
static char sccsid[] = "@(#)44	1.6  src/tenplus/e2/common/window.c, tenplus, tenplus411, GOLD410 3/23/93 11:54:41";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	fission, leavewin, switchwin, window
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
/* File: window.c - support for multiple windows                            */
/****************************************************************************/

#include "def.h"
#include "INeditor_msg.h"

int fission (void);
extern ATTR g_partchar;

/****************************************************************************/
/* fission: split current window in two; fix invariant text                 */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - RET_OK or ERROR                            */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_wnode                          */
/*                                                                          */
/* globals changed:        g_wp, g_warray, g_wnode                          */
/*                                                                          */
/* synopsis:                                                                */
/*     fission attempts to split the current window into two.  If the       */
/*     cursor is on the top line of the window (but not in a corner) the    */
/*     window will be split vertically.  Otherwise, unless it is on the     */
/*     bottom line, the window is split horizontally.  If the cursor is in  */
/*     a corner or on the bottom line, fission gives up with a call to      */
/*     Eerror.                                                              */
/*                                                                          */
/*     The new window is set up to point at the same field as the old,      */
/*     but starting at the top of the field instead of the current position */
/*     in the old window.  The old window is shrunk to make room for the    */
/*     new and graphics characters are written into the line or column      */
/*     separating the windows.  Finally, sendbox is invoked to send the new */
/*     window to the screen.                                                */
/****************************************************************************/

int fission (void)
    {
    WSTRUCT *newwp;            /* wp for newly made window                */
    WSTRUCT *wp=g_wp;          /* wp for current window at start          */
    WSTRUCT *warray=g_warray;  /* da window array                         */
    ATTR    *cp;
    ATTR    *line;

    int curindex;              /* index in g_warray of window being split */
    int newindex;              /* index in g_warray of new window         */
    int i;
    int curline=w_line(wp);    /* current line                            */
    int curcol=w_col(wp);      /* current column                          */
    int bline;                 /* line for border (horiz window)          */
    int bcol;                  /* column for border (vert window)         */
    int pos;                   /* position in text relating to 'bcol'     */

    /* get rid of any modified lines before splitting up screen */
    flush_window ();

    /* determine indices in g_warray, since the realloc may move it */
    curindex = wp - warray;
    newindex = obj_count(warray);

    /* add another window to the window structure */
    warray = (WSTRUCT *) s_realloc ((char *) warray, newindex + 1);

    /* get pointers to the windows in question */
    wp    = &warray[curindex];
    newwp = &warray[newindex];

    /* the policy is to make horizontal windows unless curline = 0 */
    if (curline != 0)
	{
        ATTR *dummy_ptr; /* pointer set by drawline. Its value is not
                            used. */

	if (curline < 1 || curline > (w_tbline(wp) - w_ttline(wp)) - 1)
	    goto nodice;

	newwp->w__flags = w_flags(wp);

	/* make the upper lefthand corner of the new window  
	   one below the cursor, so we can add the border later
	  (the border hits the cursor) */

	/* compute the beginning line of the new window */
	bline             = w_ttline(wp) + curline + abs(w_ttline (warray));
	newwp->w__lrline  = w_lrline(wp) - curline - 1;
	newwp->w__modflgs = s_alloc(w_lrline (newwp) + 1, T_CHAR, NULL);
	newwp->w__lrcol   = w_lrcol(wp);

	/* subtract space from the current window   */
	wp->w__lrline    = curline - 1;
	newwp->w__ftline = newwp->w__ftcol = 0;
	newwp->w__ttline = curline + w_ttline(wp) + 1;
	newwp->w__ttcol  = w_ttcol(wp);
	newwp->w__tbline = w_tbline(wp);
	newwp->w__tbcol  = w_tbcol(wp);
	wp->w__tbline    = w_ttline (newwp) - 2;
	newwp->w__tabs   = w_tabs(wp) ? s_string (w_tabs(wp)) : NULL;
	newwp->w__lmar   = w_lmar(wp);
	newwp->w__rmar   = w_rmar(wp);
	newwp->w__name   = s_string (w_name(wp));

	/* finally, put the new horizontal line into the invtext. The
           line characters are assumed to have a display width of 1 */
           
	bcol  = abs(w_ttcol(warray)) +  w_ttcol(newwp) - 1;
        drawline((ATTR)(G_TOL | FIRST), (ATTR)(G_TOR | FIRST), (ATTR)(G_HLINE | FIRST),
                 bline, bcol, 
                 bcol + abs(w_tbcol(newwp)) - w_ttcol(newwp) + 2,
                 &dummy_ptr, NULL, (ATTR **) (warray->w__cache)); 

	/* now, fix the screen */
	g_wp = warray;
	sendbox (bline, bline, bcol, abs(w_ttcol(warray)) + w_tbcol(newwp) + 1);
	g_wp = newwp;

	/* "HOME" cursor in window we just split */
	warray [curindex].w__line = 0;
	warray [curindex].w__col  = 0;
	g_warray = warray;
	g_wnode  = (char *) ERROR;
	return (RET_OK);
	}

    /* vertical window case */
    else
	{
	if (curcol < 1 || curcol > (w_tbcol(wp) - w_ttcol(wp)) - 1)
	    goto nodice;

	newwp->w__flags = w_flags(wp);

	/* make the upper lefthand corner of the new window
	   one below the cursor, so we can add the border later
	   the border hits the cursor. */

	bcol              = abs (w_ttcol(warray)) + w_ttcol(wp) + curcol;
	newwp->w__lrline  = w_lrline(wp);
	newwp->w__modflgs = s_alloc (w_lrline(newwp) + 1, T_CHAR, NULL);
	newwp->w__lrcol   = w_lrcol(wp) - curcol - 1;

	/* subtract space from the current window   */
	wp->w__lrcol     = curcol - 1;
	newwp->w__ftline = newwp->w__ftcol = 0;
	newwp->w__ttline = w_ttline(wp);
	newwp->w__ttcol  = w_ttcol(wp) + curcol + 1;
	newwp->w__tbline = w_tbline(wp);
	newwp->w__tbcol  = w_tbcol(wp);
        wp->w__tbcol     = w_ttcol(newwp) - 2;
	newwp->w__tabs   = w_tabs(wp) ? s_string (w_tabs(wp)) : NULL;
	newwp->w__name   = s_string (w_name(wp));

	/* make the new window have "standard" margins if the old window
	   had margins in effect. Otherwise, leave the margins off.
	   In any event, leave the current window's margins alone */

	if (w_ismar(wp))
	    {
	    newwp->w__lmar = w_lmar(wp);
	    newwp->w__rmar = w_rmar(wp);
	    }

	/* finally, put the new vertical line into the invtext */
	bline   = w_ttline (newwp) + abs(w_ttline (warray));
	w_text (warray, bline - 1) = 
                 (char *) overwrite_char(
                        (ATTR *)w_text (warray, bline - 1),
                        bcol, (ATTR)(G_TEE | FIRST), 
                        (ATTR)(G_HLINE | FIRST)); 

	for (i = bline; i <= abs(w_ttline(warray)) + w_tbline(newwp); i++)
	    {
	    line = (ATTR *) w_text (g_wp, i-w_ttline(newwp));
	    w_text (warray, i) = 
                    (char *) overwrite_char((ATTR *)w_text (warray, i),
                         bcol, (ATTR)(G_VLINE | FIRST),
	                 (ATTR)(SPACE | FIRST));
                         
	    }

	w_text (warray, i) = 
             (char *) overwrite_char((ATTR *) w_text (warray, i),
                            bcol, (ATTR)(G_ITEE | FIRST),
                            (ATTR)(G_HLINE | FIRST));

	/* now, fix the screen */
	g_wp = warray;
	sendbox (bline-1, abs(w_ttline(warray))+w_tbline(newwp)+1, bcol, bcol);
	g_wp = newwp;

	/* "HOME" cursor in window we just split */
	warray[curindex].w__line = 0;
	warray[curindex].w__col  = 0;
	g_warray = warray;
	g_wnode  = (char *) ERROR;
	return (RET_OK);
	}

    /* in case the window wouldn't work where the user wanted to put it */
nodice:
    warray   = (WSTRUCT *) s_delete ((char *)warray, newindex, 1);
    g_wp     = &warray[curindex];
    g_wnode  = (char *) ERROR;
    g_warray = warray;
    Eerror (M_EWINPOS, "Cannot split the window at the current cursor position.");
    return (ERROR);
    }

/****************************************************************************/
/* leavewin:  fix screen when leaving a multiple window                     */
/*                                                                          */
/* arguments:              WINDOW *wp - window moved into                   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_formname, g_wp, g_warray, g_dispname           */
/*                                                                          */
/* globals changed:        g_dispname                                       */
/*                                                                          */
/* synopsis:                                                                */
/*     If the g_formname global is not *win* (indicating a window), simply  */
/*     flush the current window and return.  Otherwise loop through all     */
/*     windows other than the current window in the window array and,       */
/*     if you encounter a different window into the same file as the        */
/*     current window, refresh the screen.  In any case, ensure that the    */
/*     d_dispname global is set to the name of the input window pointer,    */
/*     whether or not g_dispname is already correct.                        */
/****************************************************************************/

void leavewin (WSTRUCT *wp)
    {
    int i;

    flush_window ();
    if (!seq (g_formname, "*win*"))
	return;

    /* if the file is multiply mapped, repaint every time       */
    /* we leave a copy                                          */

    for (i = 1; i < obj_count (g_warray); i++)
	{
	if (g_wp != &g_warray [i] &&
	    seq (w_filename (g_wp), w_filename (&g_warray [i])))
		{
		getall ();      /* repaint only what is necessary */
		}
	}
    }

/****************************************************************************/
/* switchwin:  take editor into correct file for window.                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_filename, g_dispname, g_readonly,        */
/*                         g_wnode                                          */
/*                                                                          */
/* globals changed:        g_wnode                                          */
/*                                                                          */
/* synopsis:                                                                */
/*     If the g_filename global does not correspond to the file name of the */
/*     current window, we need to switch to the file for the current window */
/*     Set the g_dispname global to NULL and invoke setfile to go to the    */
/*     window's file.  In the multiple-window case, the window's flags will */
/*     protect readonly files, so the g_readonly global can safely be set   */
/*     false.  Finally, clear out the g_wnode global so that it will be     */
/*     reset at the next display.                                           */
/****************************************************************************/

void switchwin (void)
    {
    WSTRUCT *wp=g_wp;

#ifdef DEBUG
    debug ("switchwin: setfile ('%s') %s", w_filename (wp),
	seq (g_filename, w_filename (wp)) ? "suppressed" : "done");
#endif

    if (!seq (g_filename, w_filename (wp)))
	{

	/* When first setting up multiple windows,
	   we get here if the file doesn't exist with w_filename (wp)
	   NULL. Setfile does absolutely nothing in this case, which
	   lets us get away with not checking the error return.
	   I would prefer to check things, but I don't see how to
	   in the copious text-space available (read: none)     */

	setfile (w_filename (wp), FALSE, FALSE);

	/* if (setfile (w_filename (wp), FALSE, FALSE) == ERROR)
	    fatal ("switchwin: setfile (%s) failed", w_filename (wp)); */

	}

    /* the window's flags will take care of preventing improper writes  */
    g_wnode = (char *) ERROR;
    }

/****************************************************************************/
/* window: Set up current window to point at another file.                  */
/*                                                                          */
/* arguments:              int   flag   - TRUE if should open window on     */
/*                                        file named in string;             */
/*                                        FALSE if should use g_filename    */
/*                         char *string - file to use if flag is set        */
/*                         int   number - argument not used; given only for */
/*                                        consistency with other handlers   */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_filename, g_wp, g_dispname, g_warray,          */
/*                         g_formname, g_sfp, g_readonly                    */
/*                                                                          */
/* globals changed:        g_dispname, g_readonly, g_formname, g_wp,        */
/*                         g_warray                                         */
/*                                                                          */
/* synopsis:                                                                */
/*     window is used either to open a new window on the screen, or close a */
/*     except the one containing the cursor.  If the input flag is NOT set  */
/*     (or if the input string is the same as g_filename), an additional    */
/*     window is opened on the current file.  If the input flag is set and  */
/*     the given string is not null, a window is opened on the named file.  */
/*     Finally, if the flag is set but a null string argument is given,     */
/*     this procedure closes all windows except the one containing the      */
/*     cursor.                                                              */
/*                                                                          */
/*     It is an error to try to close windows when there is only one on     */
/*     the screen.  It is also an error to try to open a window when either */
/*     the original or the new file is not a text file.                     */
/*                                                                          */
/*     window and its related procedures use the pseudo-form-name "*win*"   */
/*     to indicate that the file is being looked at through a window rather */
/*     than the standard text form.  They also set the user-read-only flag  */
/*     on and window into a readonly file, so that the g_readonly global    */
/*     can be false and ignored in subsequent processing of multiple        */
/*     windows.  The zoom form for text in windows is also set to the       */
/*     pseudo-form "*win*" so that the print helper also understands that   */
/*     multiple windows are on the screen.                                  */
/****************************************************************************/

void window (int flag, char *string, int number)
    {
    WSTRUCT *wp=g_wp;
    WSTRUCT *warray;
    char *ofilename;
    int line, column;

    /* WINDOW by itself means make another window on the same file      */
    if (flag == NOARGS)
	string = g_filename;

    /* ENTER WINDOW means clobber all but the current window  */
    if (flag == EMPTYARG)
	{
#ifdef  DEBUG
	debug ("window: kill windows, map '%s'", w_filename (wp));
#endif

	if (!seq (g_formname, "*win*"))
	    {
	    Eerror (M_ENOWINDOWS, "There are no windows to remove.");
	    return;
	    }

	flush_window ();

	/* save cursor                                          */
	line = Eline ();
	column = Ecolumn ();

	if (setfile (w_filename (wp), TRUE, TRUE) == ERROR)
	    fatal ("window: setfile failure on \"%s\"", w_filename (wp));

	/* restore cursor                                       */
	Eputcursor ("", line, column);
	return;
	}

    /* make it more difficult to damage a non-text file         */
    if (!seq (g_formname, "*win*"))
	{
	if (obj_count (g_warray) > 2 || (g_spath && *g_spath))
	    {
	    Eerror (M_ENONTEXT,
		    "Use WINDOW only with text files.");
	    return;
	    }

	/* kludge to fix window 1 before fissioning it                  */
	if (g_readonly)
	    {
	    wp->w__flags |= USERRO;
	    }
	}

    /* split the current window, either horizontally or vertically      */
    /* leaves g_wp pointing at the new window, g_wnode=ERROR            */

    /* remember that we've changed the form        */
    if (fission () == ERROR)
	return;

    s_free (g_formname);
    g_formname = s_string ("*win*");

#ifdef  DEBUG
    debug ("window: setfile ('%s')", string);
#endif

    ofilename = s_string (g_filename);
    setfile (string, FALSE, FALSE);

    /* avoid putting a non-text file in a window    */
    /* g_sfp is NULL if attempted to edit a directory */

    if (g_sfp == NULL || obj_getflag (sf_records (g_sfp), RC_NONTEXT))
	{
	if (setfile (ofilename, TRUE, TRUE) == ERROR)
	    fatal ("window: setfile failed");

	Eerror (M_ENONTEXT,
		"Use WINDOW only with text files.");
	}

    s_free (ofilename);

    /* necessary so we don't point at outer space after fission ()      */
    wp = g_wp;

    /* remember file name to display later                              */
    wp->w__filename = s_string (g_filename);

    /*    kludge - make the window readonly if the file in it
	  should be. The flag bits are already restored properly when
	  switching windows, so we save the text space needed to fix
	  up g_readonly.                                                */

    if (g_readonly)
	{
	wp->w__flags |= USERRO;
	}
    else    /* in case split window WAS readonly    */
	wp->w__flags &= ~USERRO;

    /* this kludge makes the print-helper know, too     */
    warray = g_warray;
    s_free (warray->w__zoom);
    warray->w__zoom = s_string (g_formname);

    getgngs (0, w_lrline (wp)); /* display new file in window */
    }
