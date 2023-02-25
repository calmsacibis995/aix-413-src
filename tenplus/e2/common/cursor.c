#if !defined(lint)
static char sccsid[] = "@(#)18	1.10  src/tenplus/e2/common/cursor.c, tenplus, tenplus411, GOLD410 3/24/93 09:26:44";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	boxopen, boxregion, curkey, cursor, markregion,
 *		complement_region, invert_text_region, showregion,
 *		setcursor, textregion, textopen, clipcursor
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
/* file:  cursor.c                                                          */
/*                                                                          */
/* cursor key handler for the editor                                        */
/****************************************************************************/

#include "def.h"
#include "keynames.h"
#include "INeditor_msg.h"

extern int  g_highlight;
extern int  g_invtext;
extern ATTR **g_goalscreen;

int g_regionflag;                         /* true if defining a region    */
int invert_region = FALSE;                /* set if a region is supposed to
					     be in inverse video */

int oldtop, oldbot, oldleft, oldright;    /* used to clear old box region */
int old_line, old_column;                 /* used to clear old text region */

static int lastline, lastcol;             /* to handle wrapping which may
					     already have been  handled in
					     curkey() */

static tvline;                           /* zero based tv coords of cursor */
static tvcol;

static tvbottom;                         /* bounds of window (zero based)  */
static tvright;

void curkey (void);
void showregion (int, int, int, int, int);

/****************************************************************************/
/* boxopen:  opens up a rectangular region of the file                      */
/* Assumes that the cursor is at (top, left)                                */
/*                                                                          */
/* arguments:              int top, bottom, left, right - box to open       */
/*                                          in zero-based field coords      */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     Insert a rectangular region of spaces in the field, and then scroll  */
/*     if necessary to put the opened region on the screen.  If the opened  */
/*     region is larger than the window, scroll to its upper left corner.   */
/****************************************************************************/

void boxopen (int top, int bottom, int left1, int right1)
    {
    int line;        /* zero based field line             */
    int count;       /* number of columns being opened    */
    int tleft;
    int tcount;

    ATTR *text1;     /* text from line                    */
    int actual_width;

#ifdef DEBUG
    debug ("boxopen:  top = %d, bottom = %d, left1 = %d, right1 = %d",
	   top, bottom, left1, right1);
#endif

    flush_window ();
    count = right1 - left1;

    for (line = top; line <= bottom; line++)
	{
	text1 = getli (line);

	/* cursor is not in invariant text, so text below is ok */
	tleft = calc_line_position2(text1, left1, &actual_width);
        if (actual_width < left1) {
            tcount = count - (actual_width + char_width(&text1[tleft]) -
                     left1);
            skipattr_idx(text1, &tleft);
        }
        else tcount = count;

	if (tleft <= (obj_count (text1) - 1))
	    {
	    text1 = (ATTR *) i_insert ((short *)text1, tleft, tcount);
	    i_smear ((short)(SPACE | FIRST), tcount, (short *)&text1 [tleft]);
	    putli (line, text1);
	    }
	s_free ((char *)text1);
	}
    }

/****************************************************************************/
/* boxregion:  handles the BOX_MARK command                                 */
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
/*     Convert the current window coordinates of the cursor to field        */
/*     coordinates, and then invoke markregion to mark a one-character      */
/*     box region at those field coordinates.                               */
/****************************************************************************/

void boxregion (void)
    {
    WSTRUCT *wp;         /* current window                    */
    int line;            /* zero based field coords of cursor */
    int column;

    wp = g_wp;
    line = w_line (wp) + w_ftline (wp);
    column = w_col (wp) + w_ftcol (wp);

#ifdef DEBUG
    debug ("boxregion:  line = %d, column = %d", line, column);
#endif

    markregion (BOXREGION, line, column, line, column);
    }

/****************************************************************************/
/* curkey:  handles cursor motions and HOME                                 */
/* Fixes up wi and wj in current window                                     */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_keyvalue, g_warray                             */
/*                         tvline, tvcol, tvbottom, tvright, g_formname     */
/*                                                                          */
/* globals changed:        tvline, tvcol                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     The current g_keyvalue is examined to see which of the cursor        */
/*     movement keys it is.  curkey handles the UPARROW, DOWNARROW,         */
/*     LEFTARROW, RIGHTARROW, HOME, TAB and BACKTAB  commands.              */
/*     All this procedure is responsible for is figuring out where the curs */
/*     has gone as a result of a cursor movement command.                   */
/****************************************************************************/

void curkey (void)
    {
    WSTRUCT *wp;        /* first (ulhc) window */
    ATTR    *wp_text;   /* cache text          */

    int     pos, right2;
    int actual_col;

    wp = g_wp;

#ifdef DEBUG
    debug ("curkey:  keyvalue = %d, tvline = %d, tvcol = %d", g_keyvalue,
	tvline, tvcol);
    debug ("         tvbottom = %d, tvright = %d", tvbottom, tvright);
    debug ("         w_col(wp) = %d, w_line(wp) = %d", w_col(wp), w_line(wp));
    debug ("         w_lrline(wp) = %d, w_lrcol(wp) = %d", w_lrline(wp), w_lrcol(wp));
    debug ("         w_ftline(wp) = %d, w_ftcol(wp) = %d", w_ftline(wp), w_ftcol(wp));
    debug ("         w_tvloc(wp) = %d", w_tvloc(wp));
    debug ("         w_ttline(wp) = %d, w_ttcol(wp) = %d", w_ttline(wp), w_ttcol(wp));
    debug ("         w_tbline(wp) = %d, w_tbcol(wp) = %d", w_tbline(wp), w_tbcol(wp));
    debug ("         w_lmar(wp) = %d, w_rmar(wp) = %d", w_lmar(wp), w_rmar(wp));
    debug ("         g_invtext = %d", g_invtext);
#endif

    switch (g_keyvalue)
	{
	case UP_ARROW:
	    if (--tvline < 0)
		tvline = tvbottom;

	    if (tvline > w_tbline(wp) || tvline < w_ttline(wp))
                break;

            if (tvcol < w_ttcol(wp) || 
                tvcol > w_lrcol(wp) + w_ttcol(wp)) 
                break;

	    /* get the current line text and calculate the line position */
	    wp_text = (ATTR *) w_text(wp, (tvline-w_ttline(wp)));
	    pos = calc_line_position2(wp_text, 
		    tvcol + w_ftcol(wp) - w_ttcol(wp), &actual_col);
            tvcol = actual_col + w_ttcol(wp) - w_ftcol(wp);

	    break;

	case DOWN_ARROW:
	    if (++tvline > tvbottom)
		tvline = 0;

	    if (tvline > w_tbline(wp) || tvline < w_ttline(wp))
                break;

            if (tvcol < w_ttcol(wp) || 
                tvcol > w_lrcol(wp) + w_ttcol(wp)) 
                break;

	    /* get the current line text and calculate the line position */
	    wp_text = (ATTR *) w_text(wp, (tvline-w_ttline(wp)));
	    pos = calc_line_position2(wp_text, 
		    tvcol + w_ftcol(wp) - w_ttcol(wp), &actual_col);
            tvcol = actual_col + w_ttcol(wp) - w_ftcol(wp);

	    break;

	case LEFT_ARROW:

            /* NOTE: it is assumed that the cursor is always on the
                     left most column of a multiwidth character.
            */

            if (--tvcol < 0)
                  tvcol = tvright;

	    if (tvline > w_tbline(wp) || tvline < w_ttline(wp)) 
                break;

	    /* get the current line text and calculate the line position */
            if (tvcol >= w_ttcol(wp) && 
                tvcol <= w_lrcol(wp) + w_ttcol(wp)) {
                 /* cursor now on right most column of character.
                    Make sure cursor is on left most column.
                 */

                 wp_text = (ATTR *) w_text(wp, (tvline-w_ttline(wp)));
                 pos = calc_line_position(wp_text,
                           tvcol + w_ftcol(wp) - w_ttcol(wp));
    
                 /* calculate the new screen column of the cursor.
                    The width of a char is always bigger or equal than 1,
                    and tvcol has already been decremented once.
                 */
                 tvcol -= (char_width(wp_text + pos) - 1);
             }
    
	    break;

	case RIGHT_ARROW:

	    if (tvline > w_tbline(wp) || tvline < w_ttline(wp)) {
                if (++tvcol > tvright)
                     tvcol = 0;
                break;
            }

	    /* get the current line text and calculate the line position */
            if (tvcol >= w_ttcol(wp) && 
                tvcol <= w_lrcol(wp) + w_ttcol(wp)) {
                 wp_text = (ATTR *) w_text(wp, (tvline-w_ttline(wp)));
                 pos = calc_line_position(wp_text,
                           tvcol + w_ftcol(wp) - w_ttcol(wp));
    
                 /* calculate the new screen column of the cursor */
                 tvcol += char_width(wp_text + pos);
            }
            else
                 tvcol++ ;

	    /* check the right margin position */
            if (tvcol > tvright)
                  tvcol = 0;

	    break;

	case HOME:

	    /* go to first non CATENTRY window unless multi-window */
	    if (!seq (g_formname, "*win*"))
		{
		int i;

		for (i = 1; i < obj_count(g_warray); i++)
		    {
		    wp = &g_warray[i];
		    if (!getstat(w_flags(&g_warray[i]), CATENTRY))
			break;
		    }
		}

	    tvline = w_ttline (wp);
	    tvcol = w_ttcol (wp);
	    break;

	case TAB:
	case MINUS_TAB:

	    if (g_keyvalue == TAB)
		wintab ();
	    else
		winbtab ();

	    /* retrieve information set by win?tab */
	    wp = g_wp;
	    tvcol = w_ttcol (wp);
	    tvline = w_ttline (wp) + w_line (wp);
	    break;
	}

	/* needed to update the cursor locally      */
	Si_display (FALSE);

#ifdef DEBUG
    debug ("curkey:  tvline = %d, tvcol = %d",tvline, tvcol);
    debug ("         tvbottom = %d, tvright = %d", tvbottom, tvright);
#endif

	St_goto (tvline, tvcol);
	St_flush ();
    }

/****************************************************************************/
/* cursor:  handles all the cursor keys and HOME                            */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray,                                  */
/*                         g_keyvalue, g_intflag, g_wnode, g_usechar,       */
/*                         g_cursorflag, tvline, tvcol, tvbottom, tvright   */
/*                         LI, CO                                           */
/*                                                                          */
/* globals changed:        g_intflag, g_wp,                                 */
/*                         g_usechar, g_cursorflag                          */
/*                         tvline, tvcol, tvbottom, tvright                 */
/*                                                                          */
/* synopsis:                                                                */
/*     cursor does all the processing involved in cursor movement.          */
/*     It invokes curkey to handle each cursor key, does raw output         */
/*     to the INtext screen, and keeps track of break processing and        */
/*     moving into a different window using cursor movement.  It            */
/*     terminates when the next input key is not a cursor key or one        */
/*     that can trivially be handled in this context, such as INSERT        */
/*     and CANCEL.                                                          */
/****************************************************************************/

void cursor (void)
    {
    WSTRUCT *wp;       /* current window                       */
    WSTRUCT *newW;     /* used in loop                         */
    int count;         /* number of windows                    */
    int motion;        /* flag says last key a cursor move     */

    wp = g_wp;

#ifdef DEBUG
    debug ("cursor:  w_line = %d, w_col = %d", w_line (wp), w_col (wp));
    debug ("         w_ttcol(wp) = %d, w_tbcol(wp) = %d", w_ttcol (wp), w_tbcol (wp));
    debug ("         w_ftcol(wp) = %d, w_lrcol(wp) = %d", w_ftcol (wp), w_lrcol (wp));
#endif

    /* switch to invariant text window */
    tvline   = w_line (wp) + w_ttline (wp);
    tvcol    = w_col (wp) + w_ttcol (wp);
    tvbottom = LI - 1;
    tvright  = CO - 1;

    curkey ();

    /* if the cursor is still in the window, we're done */
    if (!(tvline < (w_ttline (wp)) ||
	   tvline >= w_tbline (wp) ||
	   tvcol < (w_ttcol (wp))  ||
	   tvcol >= w_tbcol (wp)))
	{
	wp->w__line = tvline - w_ttline (wp);
	wp->w__col  = tvcol - w_ttcol (wp);
	return;
	}

#ifdef DEBUG
    debug ("cursor: cursor is not in the window");
    debug ("         w_line = %d, w_col = %d", w_line (wp), w_col (wp));
    debug ("         w_ttcol(wp) = %d, w_tbcol(wp) = %d", w_ttcol (wp), w_tbcol (wp));
#endif

    /* handle the case of the roaming cursor aka change field   */
    count = obj_count (g_warray);

    /* code to make uparrow into adjacent window work           */
    /* set g_keyvalue to garbage here so that we fall into      */
    /* the default clause of the switch below the first time.   */

    motion = TRUE;
    g_keyvalue = 0;

    for (;;)
	{
	if (g_keyvalue)
	    {
	    nextin ();
	    motion = FALSE;
	    }

	switch (g_keyvalue)
	    {
	    case CANCEL:
		g_intflag = FALSE;
		continue; /* ignore CANCEL (and BREAK) */

	    case SAVE:
	    case USE:
	    case EXIT:
	    case REFRESH:
	    case ZOOM_OUT:
		callfunction (g_edfunction [g_keyvalue], NOARGS, "", 0);
		g_invtext = FALSE;
		return;

	    case FONT:
	    case INSERT_MODE:
		callfunction (g_edfunction [g_keyvalue], NOARGS, "", 0);
		continue;

	    case UP_ARROW:
	    case DOWN_ARROW:
	    case LEFT_ARROW:
	    case RIGHT_ARROW:
	    case HOME:
	    case TAB:
	    case MINUS_TAB:
		curkey ();
		motion = TRUE;

	    default:

		/* Look for window containing the cursor */
		for (newW = &g_warray [1]; newW < &g_warray [count]; newW++)
		    if ((tvline >= (w_ttline (newW))) &&
			(tvline <= (w_tbline (newW))) &&
			(tvcol >= (w_ttcol (newW)))   &&
			(tvcol <= (w_tbcol (newW)))   &&
			/* Prevent CATENTRY being modified */
			(!getstat (w_flags (newW),CATENTRY)))
			break;

		/* if cursor not in a window */
		if (newW >= &g_warray [count])
		    {
		    g_invtext = TRUE; /* now in invariant text */
		    if (getstat (w_flags (wp), DOUMOD))
			{
			putgng (0, w_lrline (wp));
			Si_display (FALSE);
			St_goto (tvline, tvcol);
			St_flush ();
			}

		    if (g_keyvalue == HELP)
			{
			helpbox (H_ECURSOR, TRUE);
			killbox (FALSE);
			Si_display (FALSE);
			St_goto (tvline, tvcol);
			St_flush ();
			}
		    else if (!motion)
			{
			Ebell ();
			}

		    /* force non-NULL to get input */
		    g_keyvalue = ERROR;
		    continue;
		    }

		/* if in a different window, switch to window under cursor */
		if (newW != wp)
		    {
		    leavewin (newW);
		    g_wnode = (char *) ERROR;
		    g_wp = newW;
		    wp = newW;
		    Edisplay ();
		    }

                if (g_invtext)
                    {
       	            /* get the current line text and calculate the line
                       position to make sure that the cursor is on the left
                       side of the current character.
                    */
       	            ATTR *wp_text = (ATTR *) w_text(wp, (tvline-w_ttline(wp)));
                    int pos, actual_col;

                    if (g_keyvalue == LEFT_ARROW)
			{
        	        pos = calc_line_position2(wp_text,
                                tvcol + w_ftcol(wp) - w_ttcol(wp), &actual_col);
                        tvcol = actual_col + w_ttcol(wp) - w_ftcol(wp);

#ifdef DEBUG
                          debug("cursor: adjust left. pos = %d, tvcol = %d, w_ttcol(wp) = %d, w_ftcol(wp) = %d",
                             pos, tvcol, w_ttcol(wp), w_ftcol(wp));
#endif
                        }
                    else if (g_keyvalue == RIGHT_ARROW)
			{
                        pos = calc_line_position2(wp_text,
                                tvcol + w_ftcol(wp) - w_ttcol(wp), &actual_col);
                        if (actual_col < tvcol + w_ftcol(wp) - w_ttcol(wp))
			    actual_col += char_width(&wp_text[pos]);
                        tvcol = actual_col + w_ttcol(wp) - w_ftcol(wp);

#ifdef DEBUG
                          debug("cursor: adjust right. pos = %d, tvcol = %d, w_ttcol(wp) = %d, w_ftcol(wp) = %d",
                             pos, tvcol, w_ttcol(wp), w_ftcol(wp));
#endif
                        }
                    }

		wp->w__line = tvline - w_ttline (wp);
		wp->w__col = tvcol - w_ttcol (wp);

		/* reuse the command */
		if (!motion)
		    g_usechar = TRUE;

		/* fix cursor, we are now in a potentially editable window */
		g_cursorflag = TRUE;
		g_invtext = FALSE;
		return;

	    } /* end switch */

	} /* end forever(;;) */
    }

/****************************************************************************/
/* setcursor:  This function is not actually used it is needed to           */ 
/*             keep functionality in the keynames.h file                    */
/*                                                                          */
/* arguments:              none                                             */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     none						    */
/*                                                                          */
/* globals changed:       none					   	    */ 
/*                                                                          */
/****************************************************************************/


void setcursor (void)
{
}

/****************************************************************************/
/* markregion:  handles box and text region operations                      */
/*                                                                          */
/* arguments:              int type - BOXREGION or TEXTREGION region        */
/*                         int oldline, oldcol - old region mark            */
/*                         int newline, newcol - other end of region        */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_dispname, g_keyvalue, g_edfunction,      */
/*                         g_goalscreen,                                    */
/*                         g_argbits, g_readonly, g_hooks,                  */
/*                         g_filename, g_usechar, tvline, tvcol             */
/*                                                                          */
/* globals changed:        g_dispname, g_usechar                            */
/*                                                                          */
/* synopsis:                                                                */
/*     markregion first converts the region boundaries from field           */
/*     coordinates into window coordinates, and then invokes showregion     */
/*     to display the region and process cursor movements.  When the        */
/*     user finishes making cursor movements, markregion sees whether       */
/*     the non-cursor key was a screen movement key such as +PAGE and,      */
/*     if so, updates the screen display and lets the user continue         */
/*     marking the region with cursor movements.  When the user finally     */
/*     finishes marking the region by hitting a non-cursor, non-screen-     */
/*     movement key, it examines the region-terminating key to decide       */
/*     what to do.  If the terminating key is a modifier, it checks that    */
/*     neither the file nor the field are read-only.                        */
/*                                                                          */
/*     Depending on the shape of the region and the original region type,   */
/*     the region is classified into a line, box, or text region.  (A line  */
/*     region is one where the original type was box/line and the end mark  */
/*     is directly under or over the start mark.)  Finally, the region      */
/*     classification and the terminating command are used to find an offse */
/*     into a function table, and the function in that row of the table is  */
/*     invoked to complete the processing of the region.                    */
/****************************************************************************/

void markregion (int type, int oldline, int oldcol, int newline1, int newcol)
    {
    WSTRUCT *wp;           /* current window                         */
    ATTR *oline;           /* line containing region blot, NOT REG!  */
    ATTR oldchar;          /* character under region blot, NOT REG!  */
    ATTR blot[2];

    int temp;
    int tmp;               /* temporary line or col (0 window based) */
    int fromline;          /* top line (0 based window coords)       */
    int fromcol;           /* top column (0 based window coords      */
    int toline;            /* bottom line (0 based window coords)    */
    int tocol;             /* bottom column (0 based window coords)  */
    int blotline, blotcol; /* screen line and col for the blot       */
    int blotpos;           /* line position equivalent of blotcol    */

    char *odispname;       /* old g_dispname                         */
    static char *box, *text2;

#ifdef DEBUG
    debug ("markregion:  type = %s, ol = %d, oc = %d, nl = %d, nc = %d",
	    type == BOXREGION ? "box" : "text", oldline, oldcol, newline1,
	    newcol);
#endif

    wp      = g_wp;
    blot[0] = G_BLOT;
    blot[1] = '\0';

    odispname = g_dispname; /* fix name at bottom of screen */
    if (type == BOXREGION)
	{
	if (box == NULL)
	    box = Egetmessage(M_EBOXSTR,"****BOX/LINE****",FALSE);
	g_dispname = box;
	}
     else
	{
	if (text2 == NULL)
	    text2 = Egetmessage (M_ETEXTSTR,"******TEXT******",FALSE);
	g_dispname = text2;
	}

    fixname ();
    Si_display (FALSE);
    g_regionflag = TRUE; /* prevents timeout reads for watchfiles */

    for (;;)
	{
	/* calculate window coordinates from field coordinates */
	fromcol = oldcol - w_ftcol (wp);    /* fix fromcol */
	if (fromcol < 0)
	    fromcol = 0;
	else if (fromcol > w_lrcol (wp))
	    fromcol = w_lrcol (wp);

	tocol = newcol - w_ftcol (wp);      /* fix tocol */
	if (tocol < 0)
	    tocol = 0;
	else if (tocol > w_lrcol (wp))
	    tocol = w_lrcol (wp);

	fromline = oldline - w_ftline (wp); /* fix fromline */
	if (fromline < 0)
	    {
	    fromline = 0;
	    if (type == TEXTREGION)
		fromcol = 0;
	    }
	else if (fromline > w_lrline (wp))
	    {
	    fromline = w_lrline (wp);
	    if (type == TEXTREGION)
		fromcol = w_lrcol (wp);
	    }
	toline = newline1 - w_ftline (wp);   /* fix toline */
	if (toline < 0)
	    {
	    toline = 0;
	    if (type == TEXTREGION)
		tocol = 0;
	    }
	else if (toline > w_lrline (wp))
	    {
	    toline = w_lrline (wp);
	    if (type == TEXTREGION)
		tocol = w_lrcol (wp);
	    }

	blotline = fromline + w_ttline (wp);
	blotpos = calc_line_position2(g_goalscreen[blotline],
                                     fromcol + w_ttcol (wp), &blotcol);
	if (!g_highlight)
	    {
	    oline = (ATTR *) g_goalscreen [blotline];
	    oldchar = oline [blotpos];

	    /* put a blot on one corner of the region       */
	    Si_ovector (blot, 1, blotline, blotcol);
	    Si_display (FALSE);
	    }

	showregion (type, blotline, blotcol, toline, tocol);
	if (!g_highlight)
	    Si_ovector (& oldchar, 1, blotline, blotcol);

	wp->w__line = (tvline - w_ttline (wp));
	wp->w__col = (tvcol - w_ttcol (wp));
	switch (g_keyvalue)
	    {
	    case LEFT:       /* these commands move the screen */
	    case RIGHT:
	    case PLUS_LINE:
	    case MINUS_LINE:
	    case PLUS_PAGE:
	    case MINUS_PAGE:
	    case BEGIN_LINE:
	    case END_LINE:
	    case TAB:
	    case MINUS_TAB:

		callfunction (g_edfunction [g_keyvalue], NOARGS, "", 0);
		newline1 = w_line (wp) + w_ftline (wp);
		newcol = w_col (wp) + w_ftcol (wp);
		Edisplay ();
		break;

	    case HELP:

		callfunction (g_edfunction [HELP], type, "", 0);
		newline1 = w_line (wp) + w_ftline (wp);
		newcol = w_col (wp) + w_ftcol (wp);
		Edisplay ();
		killbox (FALSE);
		break;

	    default:
		goto done;
	    }
	}

done:  /* user has finished specifying region */

    g_regionflag = FALSE;
    newline1 = w_line (wp) + w_ftline (wp);
    newcol = w_col (wp) + w_ftcol (wp);
    g_dispname = odispname;

#ifdef DEBUG
    debug ("    oldline = %d, oldcol = %d, newline1 = %d, newcol = %d",
	   oldline, oldcol, newline1, newcol);
#endif

    /* if text region on one line, treat like box */
    if (newline1 == oldline)
	type = BOXREGION;

    /* this is to catch if window shifts left or right */
    tmp = w_ftcol (wp);
    if (g_keyvalue == CANCEL)
	putcurs (oldline, oldcol);

    /* if we landed above where we started */
    if (newline1 < oldline)
	{
	temp     = oldline;
	oldline  = newline1;
	newline1 = temp;
	temp     = oldcol;
	oldcol   = newcol;
	newcol   = temp;
	}

    /* if wrong corners */
    if ((type == BOXREGION) && (newcol < oldcol))
	{
	temp = oldcol;
	oldcol = newcol;
	newcol = temp;
	}

    if (g_keyvalue != CANCEL)
	putcurs (oldline, oldcol);

    if (w_ftcol (wp) != tmp)
	Edisplay ();

    /* distinguish between BOXREGION and LINEREGION */
    if ((type == BOXREGION) && (oldcol == newcol))
	type = LINEREGION;

#ifdef DEBUG
    debug("markregion: g_keyvalue = %d  g_argbits = %d  type = %d",
	   g_keyvalue, g_argbits [g_keyvalue], type);
#endif

    if ((g_keyvalue == CANCEL) || (g_keyvalue == ENTER))
	return;

    /* call the function if region type matches argbit region type */
    if ((type & g_argbits [g_keyvalue]) == type)
	{
	callfunction (g_edfunction [g_keyvalue], type,
			Lputregion (oldline, oldcol, newline1, newcol), 0);
	return;
	}

    /* complain about command incompatible with regions */
    if (type == BOXREGION)
	Eerror (M_EARGBOX, "The specified command does not accept box region input.","");
    else if (type == LINEREGION)
	Eerror (M_EARGLINE, "The specified command does not accept line region input.","");
    else
	Eerror (M_EARGTEXT, "The specified command does not accept text region input.","");
    }

/****************************************************************************/
/* complement_region: invert the video of specified region                  */
/*                                                                          */
/* arguments:              x1, y1 - upper left hand corner                  */
/*                         x2, y2 - lower right hand corner                 */
/*                                                                          */
/* return value:           none                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*    invert the specified rectangle (given in character coordinates)       */
/****************************************************************************/

void complement_region (int x1, int y1, int x2, int y2)

    {
    int cx1, cx2, actual;

    ATTR *goal_line;

    /* ensure dirty region is large enough */
    Si_dirty (y1, y2);
    for (; y1 <= y2; y1++)
	{
	goal_line = g_goalscreen [y1];
	g_goalcrc [y1] = ++g_uniqnum;

        cx1 = calc_line_position3(goal_line, x1);
        cx2 = calc_line_position2(goal_line, x2, &actual);
	/* Handle multi-byte last character in box region */
	if ((goal_line[cx2] != (ATTR)0) && (actual < x2)) 
		{
		skipattr_idx(goal_line, &cx2);
		cx2--;
		}	

	for (; cx1 <= cx2; cx1++)
	    attr_flip (goal_line[cx1], STANDOUT);

	}
	Si_dirty ( y1, y2);
    }

/****************************************************************************/
/* invert_text_region: invert the video of the given text region            */
/*                                                                          */
/* arguments:              fromcol, fromline - start of text region         */
/*                         firstcol, lastcol - start and end of window in t */
/*                                             coordinates                  */
/*                         cursor_line, cursor_column - end of text region  */
/*                                                                          */
/* return value:           none                                             */
/*                                                                          */
/* globals referenced:     none                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*    display or remove the indicated text region in inverted video         */
/****************************************************************************/

void invert_text_region (int fromcol, int fromline,
				      int firstcol, int lastcol1,
				      int cursor_line, int cursor_column)
    {
# ifdef DEBUG
debug("invert_text_region : start col/line = %d,%d",fromcol, fromline);
debug("                   : end col/line   = %d,%d", cursor_column,cursor_line);
debug("                   : window region  = %d,%d",firstcol, lastcol1);
# endif

    /* take care of fromline line */
    if (cursor_line == fromline)
	{
	if (cursor_column < fromcol)
            complement_region (cursor_column, fromline, fromcol, fromline);
	else
            complement_region (fromcol, fromline, cursor_column, fromline);
	return;
	}
    else
	if (cursor_line < fromline)
	    {
            complement_region (firstcol, fromline, fromcol, fromline);
	    complement_region (cursor_column, cursor_line, lastcol1,
							   cursor_line);
	    }
	else
	    {
            complement_region (fromcol, fromline, lastcol1, fromline);
	    complement_region (firstcol, cursor_line, cursor_column,
						      cursor_line);
	    }

    /* take care of lines between fromline and cursor line */
    if (cursor_line < fromline - 1)
        complement_region (firstcol, cursor_line+1, lastcol1, fromline - 1);
    else
	complement_region (firstcol, fromline+1, lastcol1, cursor_line - 1);
    }

/****************************************************************************/
/* showregion:  handles the display for boxregion and textregion            */
/* Starts the display at the given line and column position                 */
/*                                                                          */
/* arguments:              int type              - BOXREGION or TEXTREGION  */
/*                         int fromline, fromcol - region start             */
/*                         int toline, tocol     - region end               */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_argbits,                                 */
/*                         g_keyvalue, g_cursorflag                         */
/*                         tvline, tvcol, tvbottom, tvright                 */
/*                                                                          */
/* globals changed:        g_cursorflag                                     */
/*                         tvline, tvcol, tvbottom, tvright                 */
/*                                                                          */
/* synopsis:                                                                */
/*     showregion is used after a region has started being marked in        */
/*     the markregion procedure.  It is used to expand the region           */
/*     as the user hits cursor-movement and screen-movement keys.           */
/****************************************************************************/

void showregion (int type, int fromline, int fromcol, int toline, int tocol)
    {
    WSTRUCT *wp;                     /* current window                 */
    int firstcol, lastcol2;           /* first and last col in window   */
    old_line = oldtop = ERROR;       /* so we don't call complement_region
					the first time in FOREVER loop */
    wp = g_wp;

    firstcol = w_ttcol (wp);
    lastcol2 = w_tbcol (wp);

#ifdef DEBUG
    debug ("showregion:  type = %s", type == BOXREGION ? "box" : "text");
#endif

    /* set up file-global tv coords. */
    tvline = toline + w_ttline (wp);
    tvcol = tocol + w_ttcol (wp);
	    tvbottom = w_lrline (wp) + w_tbline (wp);
	    tvright = w_lrcol (wp) + w_tbcol (wp);

    /* set up the region the first time through. This makes screen-motion
       functions leave the region displayed. */

    for (;;)
	{

	/* fix the cursor so it won't wrap */
	clipcursor (wp);

	/* this paints the region */
	if (g_highlight)
	    {
	    if (type == BOXREGION)
	        {

	        /* remove the highlighting from the old region */
	        if (oldtop != ERROR)
	            complement_region (oldleft, oldtop, oldright, oldbot);
	    	
	        oldtop = min (fromline, tvline);
	        oldbot = max (fromline, tvline);
	        oldleft = min (fromcol, tvcol);
	        oldright = max (fromcol, tvcol);
    
	        /* invert the new region */
	        complement_region (oldleft, oldtop, oldright, oldbot);
	        }
	    else
		{
	        if (old_line != ERROR)
		    invert_text_region (fromcol, fromline, firstcol, lastcol2,
					old_line, old_column);
     
	        old_line = tvline;
	        old_column = tvcol;
		invert_text_region (fromcol, fromline, firstcol, lastcol2,
				    old_line, old_column);
	        }
	    }
	if (Si_display (FALSE))
	    St_flush();

	/* fix the screen */
	St_goto (tvline, tvcol);
	St_flush ();
	if (Si_display (FALSE))
	    St_flush();

	nextin ();
	switch (g_keyvalue)
	    {
	    case UP_ARROW:
	    case DOWN_ARROW:
	    case LEFT_ARROW:
	    case RIGHT_ARROW:

		curkey ();
		continue;

	    case HOME:
		tvline = w_ttline (wp);
		tvcol = w_ttcol (wp);
		continue;

	    case TAB:

		/* if current window has tab stops use them */
		if (w_istabs (wp))
		    break;
		else
		    tvcol = (tvcol + 8) & ~7;

		if (tvcol > tvright)
		    tvcol = w_ttcol (wp);

		continue;

	    case MINUS_TAB:

		/* if current window has tab stops use them */
		if (w_istabs (wp))
		    break;
		else
		    tvcol -= (tvcol % 8) ? (tvcol % 8) : 8;

		if (tvcol < 0)
		    tvcol = tvright;

		continue;

	    case RETURN:

		tvcol = w_ttcol (wp);
		if (++tvline > tvbottom)
		    tvline = w_ttline (wp);

		continue;

	    } /* end switch */

	/* other keys fall through the switch without reaching a continue */

	/* remove the highlighting from the selected region */
	if (g_highlight)
	    {
	    invert_region = FALSE;

	    /* uninvert the region */
	    if (type == BOXREGION)
	        complement_region (oldleft, oldtop, oldright, oldbot);
	    else
		invert_text_region (fromcol, fromline, firstcol, lastcol2,
		  old_line, old_column);
	    }

	/* if key should stop region */
	if (g_argbits [g_keyvalue] & REGIONARG)
	    {
	    g_cursorflag = TRUE;
	    return;
	    }

	/* complain about command incompatible with regions */
	if (type == BOXREGION)
	    Eerror (M_EARGBOX, "The specified command does not accept box region input.","");
	else
	    Eerror (M_EARGTEXT, "The specified command does not accept text region input.","");

	/* reinvert region after error has been displayed */
	if (g_highlight)
	    {
	    if (Si_display (FALSE)) /* fix the screen */
		St_flush();

	    /* fix the cursor */
	    St_goto (tvline, tvcol);
	    invert_region = TRUE;

	    /* invert the region */
	    if (type == BOXREGION)
	        complement_region (oldleft, oldtop, oldright, oldbot);
	    else
		invert_text_region (fromcol, fromline, firstcol, lastcol2,
				    old_line, old_column);
	    }

	} /* end FOREVER */
    }

/****************************************************************************/
/* textregion:  handles the TEXT_MARK command                               */
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
/*     textregion converts the current cursor position from window          */
/*     coordinates into field coordinates, and then invokes the             */
/*     markregion procedure to mark the text region starting there.         */
/****************************************************************************/

void textregion (void)
    {
    WSTRUCT *wp;      /* current window                    */
    int line;         /* zero based field coords of cursor */
    int column;

    wp = g_wp;
    line = w_line (wp) + w_ftline (wp);
    column = w_col (wp) + w_ftcol (wp);

# ifdef DEBUG
    debug ("textregion:  line = %d, column = %d", line, column);
# endif

    if(w_nxtgang(wp))
	Eerror (M_ENOTEXTMARK, "Use a BOX region rather than a TEXT region.","");
    else
	markregion (TEXTREGION, line, column, line, column);
    }

/****************************************************************************/
/* textopen:  opens up a text-shaped region of the file                     */
/* Assumes that the (top, left) of the text-shaped region is in the         */
/* current window.                                                          */
/*                                                                          */
/* arguments:              int topline, topcol, botline, botcol -           */
/*                                 start and end points of region           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     textopen opens a region extending from the top mark to the end       */
/*     of its line, through all succeeding lines up to the bottom mark      */
/*     line, and up to the bottom mark on the bottom mark line.  Text       */
/*     in between the marks is picked up, the region is smeared with        */
/*     space characters, and the picked-up text is put down immediately     */
/*     after the screen position of the bottom mark.                        */
/****************************************************************************/

void textopen (int topline, int topcol, int botline, int botcol)
    {
    WSTRUCT *wp;       /* current window   */
    ATTR *text3;       /* text of line     */
    ATTR *saved;       /* text to insert   */

    int i;             /* used to find last char */
    int i_len;         /* length of last char */
    int index1;        /* index of topcol  */

#ifdef DEBUG
    debug ("textopen:  tl = %d, tc = %d, bl = %d, bc = %d",
	   topline, topcol, botline, botcol);
#endif

    wp = g_wp;
    text3 = (ATTR *) w_text (wp, topline - w_ftline (wp));

    index1 = calc_line_position3(text3, topcol);

    for (i = (obj_count (text3) - 2); i >= 0; backupattr_idx(text3, &i))
	if (!isattrspace(& text3 [i]) )
	    break;

    i_len = attrlen( &text3[i] );

    if (i >= index1)
	{
	saved = (ATTR *) s_alloc (i - index1 + i_len + 1, T_ATTR, NULL);
	(void) i_strncpy ((short *)saved,
			  (short *)&text3 [index1], i - index1 + i_len);

	/* This assumes that the (topline, topcol) is in the current window */
	delsp (topline-w_ftline (wp), index1, i - index1 + i_len);
	}
    else
	saved = NULL;

    Einsline ("", topline + 1, botline - topline);

    if (saved)
	{
	saved = (ATTR *) i_insert ((short *)saved, 0, botcol);
	i_smear ((short)(SPACE | FIRST), botcol, (short *)saved);
	putli (botline, saved);
	s_free ((char *)saved);
	}
    }

/****************************************************************************/
/* clipcursor: make sure cursor is within current window                    */
/*                                                                          */
/* arguments:              WINDOW *wp - window to be clipped                */
/*                                                                          */
/* return value:           int                                              */
/*                                                                          */
/* globals referenced:     tvline, tvcol                                    */
/*                                                                          */
/* globals changed:        tvline, tvcol                                    */
/*                                                                          */
/* synopsis:                                                                */
/*     If the cursor has gone off the bottom of the screen, put it on the   */
/*     top line; if off the top, put it on the bottom.  Similarly, if it    */
/*     has gone off the right side of the screen, put it on the left; and   */
/*     if off the left, put it on the right.                                */
/****************************************************************************/

int clipcursor (WSTRUCT *wp)
    {

    if (tvline < (w_ttline (wp)))
	tvline = w_tbline (wp);
    else if (tvline > w_tbline (wp))

	/* if on status line, wrap according to lastline */
	if (tvline == LI - 1 && lastline == w_ttline (wp))
	    tvline = w_tbline (wp);
	else
	    tvline = w_ttline (wp);

    lastline = tvline;
    if (tvcol < (w_ttcol (wp)))
	if (tvcol == 0 && lastcol == CO - 1)
	    tvcol = w_ttcol (wp);
	else
	    tvcol = w_tbcol (wp);
    else if (tvcol > w_tbcol (wp))
	if (tvcol == CO - 1 && lastcol == w_ttcol (wp))
	    tvcol = w_tbcol (wp);
	else
	    tvcol = w_ttcol (wp);

    lastcol = tvcol;
    }
