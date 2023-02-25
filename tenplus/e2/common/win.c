#if !defined(lint)
static char sccsid[] = "@(#)36	1.6  src/tenplus/e2/common/win.c, tenplus, tenplus411, GOLD410 3/23/93 11:54:33";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	backtab, chwin, tab, winbtab, winlf, wintab
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
/* File:   win.c                                                            */
/****************************************************************************/

#include "def.h"
#include "keynames.h"

/****************************************************************************/
/* backtab:  handles the -TAB command                                       */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current window does not have tab stops, either go to the      */
/*     first column of the window or, if already there, go to the previous  */
/*     window.  Otherwise, if the cursor is on the left margin, go to the   */
/*     right one.  If the cursor is not on the left margin, loop backwards  */
/*     through the columns, stopping at the first tab stop or at the left   */
/*     margin.                                                              */
/****************************************************************************/

void backtab (void)
    {
    WSTRUCT *wp=g_wp;       /* current window            */
    int     tablen;         /* display width of tab rack */
    ATTR    *line;          /* The current line */
    int     actual_col;

#ifdef DEBUG
    debug ("backtab called");
#endif

    /* if no tabs */
    if (!w_istabs (wp))
	{
	if (w_col (wp) > 0) /* if in middle of field go to the first column */
	    wp->w__col = 0;
	else
	    winbtab ();     /* tab to the previous window */

	return;
	}

    line = (ATTR *) w_text(wp, w_line(wp));

    /* on left edge, so wrap to right edge */
    if (w_ftcol (wp) + w_col (wp) == 0)
	{
	(void) calc_line_position2(line, w_lrcol(wp), &actual_col);
	wp->w__col = actual_col; 
	return;
	}

    /* find previous tab stop (or 1st column). The tab rack always contains
       ASCII (single width) characters */

    tablen = strlen (w_tabs (wp));
    while (w_ftcol (wp) + w_col (wp) > 0)
	{
	if (w_col (wp) == 0)
	    wp->w__col += movlf (min((w_lrcol(wp)/3 + 1),w_ftcol(wp)));

	if (w_ftcol (wp) + w_col (wp) < tablen)
	    wp->w__col--;
	else
	    {
	    wp->w__col = tablen - w_ftcol (wp) - 1;
	    if (w_col (wp) < 0)
		wp->w__col += movlf(-w_col (wp)); 
	    }


	if (!ismbspace(&w_tabchar (wp, w_col (wp) + w_ftcol (wp))))
	    {
	    /* Ensure w_col is on a character boundary */
	    (void) calc_line_position2(line, w_col(wp) + w_ftcol(wp), 
		    &actual_col);
	    wp->w__col = actual_col - w_ftcol(wp);
	    return;
	    }
	}
    }

/****************************************************************************/
/* chwin:  handles the CHWIN command                                        */
/*                                                                          */
/* arguments:              int argtype - whether called with NOARGS,        */
/*                                       EMPTYARG, ...                      */
/*                         char *strvalue - argument if argtype is          */
/*                                       STRINGARG                          */
/*                                  (not used - given only for consistency) */
/*                         int   numvalue - argument if argtype is          */
/*                                        NUMBERARG                         */
/*                                  (not used - given only for consistency) */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the user hit CHWIN, change to the next window.  Otherwise he hit  */
/*     ENTER CHWIN, so go to the previous window.                           */
/****************************************************************************/
void chwin (int argtype, char *strvalue, int numvalue)
    {
#ifdef DEBUG
    debug ("chwin called:  argtype = %d, strvalue = '%s', numvalue = %d",
	argtype, strvalue, numvalue);
#endif

    if (obj_count (g_warray) == 2) /* if only one field */
	return;

    if (argtype == NOARGS)
	wintab ();
    else
	winbtab ();
    }

/****************************************************************************/
/* tab:  handler for the TAB command                                        */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        g_wp                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current window has no tab stops, go to the next window.       */
/*     Otherwise loop the cursor through the columns of the window,         */
/*     stopping at the first tab stop.  If you come to the right margin     */
/*     before finding a tab stop, put the cursor on the first column.       */
/****************************************************************************/

void tab (void)
    {
    WSTRUCT *wp=g_wp;   /* current window */
    int tablen;         /* length of tab rack */
    ATTR    *line;      /* The current line */
    int pos, actual_col;
    char *cp;

#ifdef DEBUG
    debug ("tab called");
#endif

    /* if no tabs, goto next window */
    if (!w_istabs (wp)) 
	{
	wintab ();
	return;
	}

    /* field has tabs, so goto next tab stop. The tab rack always contains
       ASCII (single width) characters */

    tablen = strlen (w_tabs (wp));
    while (w_col (wp) + w_ftcol (wp) < tablen - 1)
	{
	wp->w__col++;
	if (w_col (wp) > w_lrcol (wp))
	    {

	    /* verify another tab stop exists before shifting screen */
	    cp = w_tabs (wp) + w_col (wp);
            skipmbspaces(&cp);
	    if (*cp)
		wp->w__col -= movrt (w_lrcol (wp)/3 + 1);
	    }


	if (!ismbspace(&w_tabchar (wp, w_col (wp) + w_ftcol (wp))))
	    {
            line = (ATTR *) w_text(wp, w_line(wp));

	    /* Ensure w_col is on a character boundary */
	    pos = calc_line_position2(line, w_col(wp) + w_ftcol(wp), 
		    &actual_col);
	    if (actual_col < w_col(wp) + w_ftcol(wp))
	        actual_col += char_width(&line[pos]);
	    wp->w__col = actual_col - w_ftcol(wp);

	    return;
	    }
	}

    /* if no tab stop to right, wrap around to first column */
    wp->w__col = 0; 
    }

/****************************************************************************/
/* winbtab:  goes to previous window in window list                         */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_wnode                          */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current window is above window 1 in the g_warray global,      */
/*     back to the previous window.  (Note that window 0 is reserved for    */
/*     the tab rack or other invariant text).  Otherwise, set the current   */
/*     window to the last visible window in the window array.               */
/*                                                                          */
/*     Put the cursor on the left side of the new current window.           */
/*     If the screen line we just left in the previous window intersects    */
/*     the new window, keep the cursor on the same line; otherwise put      */
/*     it on the top line of the window.                                    */
/*                                                                          */
/*     Set the g_wnode global to ERROR to force the correct display.        */
/****************************************************************************/

void winbtab (void)
    {
    WSTRUCT *wp;      /* current window                */
    WSTRUCT *startwp; /* temporary loop variable  */
    int tvline;       /* current cursor line (0 based) */

#ifdef DEBUG
    debug ("winbtab called");
#endif

    startwp = wp = g_wp;
    tvline = w_ttline (wp) + w_line (wp);

    /* backup cyclicly to the first editable window */
    do
    {
	if (--wp == g_warray)
	    wp = g_warray + obj_count (g_warray) - 1;


        if (getstat (w_flags (wp), CATENTRY))/* Skip any dispayonly fields */
	    continue;

	if ((g_keyvalue == NEXT_WINDOW) &&
	    ( !getstat (w_flags (wp), INVISIBLE))) /* skip invisible fields */
	    break;  /* go into window even if it is readonly */

	if ( !getstat (w_flags (wp), INVISIBLE | USERRO) ||
	    !getstat (w_flags (wp), INVISIBLE | NOZOOM))
	    break;
    }
    while (wp != startwp);

    /* only adjust cursor in non-multi-win environment and multi field forms */
    if ( !seq (g_formname, "*win*") && obj_count (g_warray) != 2)
	{
	wp->w__col = 0; /* put cursor on left edge of window */

	/* if old tv line is in window, use same line */
	if ((tvline >= w_ttline (wp)) && (tvline <= w_tbline (wp)))
	    wp->w__line = tvline - w_ttline (wp);
	else
	    wp->w__line = 0; /* else use top line of window */
	}

    leavewin (wp);
    g_wp = wp;
    g_wnode = (char *) ERROR; /* invalidate g_wnode */
    }

/****************************************************************************/
/* winlf:  moves to lower left window relative to cursor                    */
/* Returns ERROR if there is no such window, else RET_OK                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           int - RET_OK if moved; ERROR if no such window   */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_wnode                          */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Loop through the windows in the g_warray global, looking for a       */
/*     window whose bottom line is below and to the left of the line after  */
/*     the cursor.  If there is more than one such window, choose the       */
/*     window whose top line is closest to the cursor and is farthest to    */
/*     the left.  Skip the current window and any invisible windows.        */
/*     If no windows meet the criterion, return ERROR.                      */
/*                                                                          */
/*     If the line below the current position is in the new window, put     */
/*     the cursor on the left edge of that line; otherwise put it in the    */
/*     upper left corner.  Invalidate the g_wnode global to ensure the      */
/*     correct  display.                                                    */
/****************************************************************************/

int winlf (void)
    {
    WSTRUCT *wp=g_wp;   /* current window              */
    WSTRUCT *tstwp;     /* window pointer              */
    WSTRUCT *bestwin;   /* closest window              */

    int i;              /* window array index          */
    int tvline;         /* next tv line (0 based)      */
    int tvcol;          /* current tv column (0 based) */

    tvline = w_ttline (wp) + w_line (wp) + 1; /* get next line in tv coords */
    tvcol = w_ttcol (wp) + w_col (wp);        /* get current col in tv coords */
    bestwin = NULL;

#ifdef DEBUG
    debug ("winlf:  tvline = %d, tvcol = %d", tvline, tvcol);
#endif

    for (i = 1; i < obj_count (g_warray); i++)
	{
	tstwp = &g_warray [i];

	if (tstwp == wp) /* skip current window */
	    continue;

	if ((getstat (w_flags (tstwp), INVISIBLE)) &&
	   /* ignore invisible windows */
	   /* ignore displayonly windows */
	   (getstat (w_flags (tstwp), CATENTRY)))
	    continue;

	if ((w_ttcol (tstwp) > tvcol) ||  /* if window to right of column */
	    (w_tbline (tstwp) < tvline))  /* or above next tv line        */
	    continue;

	/* if no best window yet or this one is a better window */

#ifdef DEBUG
	debug ("winlf: tstwp = 0%o, ttcol = %d, bestwin = 0%o, ttcol = %d",
	    tstwp, w_ttcol (tstwp), bestwin, bestwin ? w_ttcol (bestwin) : 0);
#endif

	/* if no current best window or this one is better that bestwin */
	if ((bestwin == NULL) ||
	    (w_ttline (tstwp) < w_ttline (bestwin)) ||
	    ((w_ttline (tstwp) == w_ttline (bestwin)) &&
	     (w_ttcol (tstwp) < w_ttcol (bestwin))))
	    bestwin = tstwp;
	}

    if (bestwin == NULL)
	return (ERROR);

    leavewin (wp);       /* write out any modified lines */
    g_wp = bestwin;      /* move to new window           */
    wp = g_wp;

    /* if next line is in window move cursor to that line in new window */
    if ((tvline >= w_ttline (wp)) && (tvline <= w_tbline (wp)))
	wp->w__line = tvline - w_ttline (wp);
    else
	wp->w__line = 0;      /* otherwise, use top line of new window  */

    wp->w__col = 0;           /* go to left edge of new window */
    g_wnode = (char *) ERROR; /* invalidate g_wnode            */
    return (RET_OK);
    }

/****************************************************************************/
/* wintab:  goes to next window in the window list                          */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, g_wnode                          */
/*                                                                          */
/* globals changed:        g_wp, g_wnode                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     Move to the next position in the g_warray global after the entry for */
/*     the current window.  If off the end of the array, or if the new wind */
/*     window is invisible, move to window 1 instead.  (Window 0 is reserv- */
/*     ed for invariant text.)  If the new window intersects the line the   */
/*     cursor is on, move the cursor to the left edge of the new window on  */
/*     the same line.  Otherwise put the cursor in the upper left corner of */
/*     the new window.  Set the g_wnode global to ERROR to force the proper */
/*     display.                                                             */
/****************************************************************************/

void wintab (void)
    {
    WSTRUCT *wp;      /* current window                */
    WSTRUCT *startwp; /* loop variable            */
    int tvline;       /* current cursor line (0 based) */

#ifdef DEBUG
    debug ("wintab called");
#endif

    startwp = wp = g_wp;
    tvline = w_ttline (wp) + w_line (wp);

    /* advance cyclicly to next editable field */
    do
    {
	if (++wp == g_warray + obj_count (g_warray))
	    wp = g_warray + 1;
        
	/* Skip over Displayonly fields */
	if (getstat (w_flags (wp),CATENTRY))
	   continue;

	if ((g_keyvalue == NEXT_WINDOW) &&
	    ( !getstat (w_flags (wp), INVISIBLE))) /* skip invisible fields */
	    break;  /* go into window even if it is readonly */

	if ( !getstat (w_flags (wp), INVISIBLE | USERRO) ||
	    !getstat (w_flags (wp), INVISIBLE | NOZOOM))
	    break;
    }
    while (wp != startwp);

    /* only adjust the cursor line & column for non-multi-window
       environment and multi field forms                        */

    if ( !seq (g_formname, "*win*") && obj_count (g_warray) != 2)
	{
	wp->w__col = 0; /* put cursor on left edge of window */

	/* if old tv line is in window, use same line */
	if ((tvline >= w_ttline (wp)) && (tvline <= w_tbline (wp)))
	    wp->w__line = tvline - w_ttline (wp);
	else
	    wp->w__line = 0; /* else use top line of window */
	}

    leavewin (wp);
    g_wp = wp;
    g_wnode = (char *) ERROR; /* invalidate g_wnode */
    }

