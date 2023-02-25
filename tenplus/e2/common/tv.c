#if !defined(lint)
static char sccsid[] = "@(#)26	1.7  src/tenplus/e2/common/tv.c, tenplus, tenplus411, GOLD410 3/23/93 11:54:25";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	is_long, sendbox, tvdn, tvlf, tvrt, tvup
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
/* file:  tv.c                                                              */
/****************************************************************************/

#include "def.h"
    
ATTR g_place_filler = (ATTR) (FIRST | '-');
                            /*  a character which holds the display columns
                                where there is not enough room for a 
                                complete Attr on the goalscreen */


/****************************************************************************/
/* is_long: return true if line is longer than length.                      */
/*                                                                          */
/* arguments:            ATTR *line   - to be examined                      */
/*                       int   length - to check against                    */
/*                                                                          */
/* return value:         TRUE if longer, otherwise FALSE                    */
/*                                                                          */
/* globals referenced:   none                                               */
/*                                                                          */
/* globals changed:      none                                               */
/*                                                                          */
/* synopsis:                                                                */
/*   is_long returns TRUE if the given line contains a non-space and        */
/*   non-null character after the given length position.  If the line is    */
/*   shorter than length, or if all characters after length are spaces or   */
/*   nulls, it returns FALSE.                                               */
/****************************************************************************/

int is_long (ATTR *line, int length)
    {
    ATTR *cp, *ep;

    if (line)
	{
	ep = line + (obj_count (line) - 1);
	for (cp = line + length + 1; cp < ep; skipattr(&cp))
	  {
	    if (*cp != (ATTR) '\0' && !isattrspace(cp))
		return (TRUE);
            if (*cp == (ATTR) '\0')
		break;
          }
	}

    return (FALSE);
    }

/****************************************************************************/
/* sendbox:  redisplay a box                                                */
/* This routine fixes up the tv buffer and display globals that fixbox uses */
/*                                                                          */
/* arguments:              int top, bottom, left, right -                   */
/*                                 zero-based window coords of box to send  */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_warray, LI, CO                           */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If the current window is invisible, do nothing.                      */
/*     For each line in the sendbox, copy characters from the window cache  */
/*     into the tv array.                                                   */
/****************************************************************************/

void sendbox (int top, int bottom, int left1, int right1)
    {
    WSTRUCT *wp=g_wp;       /* current window                  */
    ATTR *cp;               /* text line in cache              */

    int line;               /* used to fix tv array            */
    int len;
    int longline;           /* long line indicator             */
    int leftpos;            /* position of left side of box    */
    int rightpos;           /* position of right side of box   */

    extern ATTR g_dotchar;  /* char to fill dotted fields with */

    int lwidth;             /* width of line before left1      */
    int rwidth;             /* width of line before right1     */
    int new_left1;          /* adjusted left1 for 1/2 multibyte*/
    int new_right1;         /* adjusted right1 for 1/2 multibyte*/

#ifdef  DEBUG
    debug ("sendbox (window %d): top = %d, bot = %d, left1 = %d, right1 = %d",
	g_wp - g_warray, top, bottom, left1, right1);
    debug ("sendbox continued: w_ttcol(wp) = %d, w_ttline(wp) = %d,\n w_lrline(wp) = %d, w_lrcol(wp) = %d, w_ftline(wp) = %d, w_ftcol(wp) = %d\n",
           w_ttcol(wp), w_ttline(wp), w_lrline(wp), w_lrcol(wp),
           w_ftline(wp), w_ftcol(wp));
    debug ("sendbox continued(2): CO = %d, LI = %d\n", CO, LI);
    debug ("sendbox continued(3): w_tbcol(wp) = %d ",w_tbcol(wp));
           
#endif

    /* if invisible window */
    if (getstat (w_flags (wp), INVISIBLE))
        return;

    if ((w_ttline (wp) + bottom) >= 0 &&
	w_ttline (wp) + top < LI - 1  &&
	w_ttcol (wp) + right1 >= 0    &&
	w_ttcol (wp) + left1 < CO)
	{

	/* change top and left to display visible part of screen */
	if (left1 + w_ttcol (wp) < 0)
	    left1 = -w_ttcol (wp);
	if (top + w_ttline (wp) < 0)
	    top = -w_ttline (wp);
	}
    else
	return;

    /* clip in case of big form on small screen */
    if (bottom + w_ttline (wp) > LI - 2)
	bottom = (LI - w_ttline (wp)) - 2;

    if (right1 + w_ttcol (wp) > CO - 1)
	right1 = CO - w_ttcol (wp) - 1;

    /* catch initial case before file opened */
    if (w_cache (wp) == NULL)
	return;

    longline = FALSE;
    for (line = top; line <= bottom; line++)
	{

	cp = (ATTR *) w_text (wp, line);
	if (longline != TRUE)
            {
            len = calc_line_position(cp, w_ftcol(wp)+w_tbcol(wp)-w_ttcol(wp));
	    longline = is_long (cp, len);
            }

	/* determine where in cache to start copying from */
	new_left1 = left1 + w_ftcol(wp);
	new_right1 = right1 + w_ftcol(wp) +1;
	leftpos  = calc_line_position2(cp, new_left1, &lwidth);
	rightpos = calc_line_position2(cp, new_right1, &rwidth);

#ifdef DEBUG
debug("sendbox: multibyte stuff lwidth %d rwidth %d new_left1 %d new_right1 %d",
		lwidth, rwidth, new_left1, new_right1);
#endif

	if (MultibyteCodeSet || (__max_disp_width > 1))
	{
	    if (new_left1 > lwidth)
	    {
	        /* part of a multibyte at the start of text */
	        lwidth += char_width(&cp[leftpos]);
	        skipattr_idx(cp,&leftpos);
	        for ( ; new_left1 < lwidth ; new_left1++)
		{
	            Si_ovector(&g_place_filler, 1, line+w_ttline(wp),
	                       new_left1 + w_ttcol(wp) - w_ftcol(wp));
		}
	    };
            if (new_right1 > rwidth)
            {
		/* part of a multibyte at the end of text */
		for ( ; new_right1 > new_left1 ; new_right1-- )
		{
                    Si_ovector(&g_place_filler, 1, line+w_ttline(wp),
                            new_right1 + w_ttcol(wp) - 1 - w_ftcol(wp));
		}
            if (rightpos - leftpos < i_strlen(g_goalscreen[line+w_ttline(wp)]))
                i_smear((short)(SPACE | FIRST), (i_strlen(g_goalscreen[line+w_ttline(wp)]) - 2), (short *)g_goalscreen[line+w_ttline(wp)]+1);    
            }
	}

	Si_ovector (cp+leftpos, (rightpos-leftpos), 
			 line + w_ttline(wp),
			 new_left1 - w_ftcol(wp) + w_ttcol(wp));
	
	/* process dotted field. 'g_dotchar' is never set to a multibyte
	   space, so comparison with SPACE is safe here. */

	if (getstat (w_flags (wp), DOTTED) && g_dotchar != SPACE)
	    dodots (line, wp);
	}

    /* see if right arrow is needed */
    if (longline != TRUE && getstat (w_flags (wp), RIGHTARROW))
	{
	for (line = 0; line <= w_lrline (wp); line++)
	    {

	    /* don't recheck lines */
	    if (line >= top && line <= bottom)
		continue;

	    len = calc_line_position(cp, w_ftcol(wp)+w_tbcol(wp)-w_ttcol(wp));
	    longline = is_long ((ATTR *) w_text (wp, line), len);
	    if (longline == TRUE)
		break;
	    }
	}

    if (longline == TRUE)
	wp->w__flags |= RIGHTARROW;
    else
	wp->w__flags &= ~RIGHTARROW;
    }

/****************************************************************************/
/* tvdn:  scroll tv, screen and windows down count lines                    */
/*                                                                          */
/* arguments:              int count - number of lines to scroll down       */
/*                         int top, bottom - lines to be scrolled           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_curcrc, g_goalcrc, g_goalscreen          */
/*                         g_curscreen                                      */
/*                                                                          */
/* globals changed:        g_goalcrc                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     If count is negative, or larger than the height of the box, do       */
/*     nothing                                                              */
/*                                                                          */
/*     If there is a popbox on the screen, kill it.                         */
/*                                                                          */
/*     For each line in the box to scroll, copy the modified bits array     */
/*     and the cache array from the line down to the line which is the      */
/*     given number of lines below it.  Clear the source line cache after   */
/*     copying its contents down.                                           */
/*     Use sendbox to send moved line to the screen.                        */
/*     Finally, make sure the g_goalcrc global has the correct updated      */
/*     screen.                                                              */
/****************************************************************************/
void tvdn (int count, int top, int bottom)
    {
    WSTRUCT *wp=g_wp;             /* current window               */
    POINTER *cache=w_cache(wp);   /* cache from line array        */

    int line;                     /* destination line for xfers   */
    int xline;                    /* line being xfered            */
    int tvline, xtvline;

    char *modflgs=w_modflgs(wp);  /* modflgs array from window    */

#ifdef DEBUG
    debug ("tvdn: count = %d, top = %d, bot = %d\n", count, top, bottom);
#endif

#ifdef CAREFUL
    if ((count <= 0) || (count > bottom - top)) /* watch out for bad calls */
	return;
#endif

    /* remove pop up box if displayed */
    killbox (TRUE); 

    for (line = bottom; top <= line - count; line--)
	{
	xline = line - count;
	modflgs [line] = modflgs [xline]; /* xfer modified bits */
	modflgs [xline] = 0;

	s_free (cache [line]); /* xfer cache array entry */
	cache [line] = cache [xline];
	cache [xline] = 0;

	sendbox (line, line, 0, w_lrcol(wp));

	tvline = line + w_ttline (wp);
	xtvline = xline + w_ttline (wp);
	if (tvline < 0 || xtvline < 0 ||
	    tvline > LI - 2 || xtvline > LI - 2)
	    continue;

	/* avoid declaring that blank lines created by the scroller
	   are right... */

	if (scr_seq (g_goalscreen [tvline], g_curscreen [xtvline]))
	    g_goalcrc [tvline] = g_curcrc [xtvline];
	}
    }

/****************************************************************************/
/* tvlf:  scrolls the tv array and screen left                              */
/*                                                                          */
/* arguments:              int count - number of columns to scroll left     */
/*                         int top, bottom, left, right -                   */
/*                                   zero based box coords of box to scroll */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If count is zero or negative, do nothing.                            */
/*     If there is a popbox on the screen, kill it.                         */
/*     Invoke sendbox on the given box.                                     */
/****************************************************************************/

void tvlf (int count, int top, int bottom, int left3, int right3)
    {
#ifdef DEBUG
    debug ("tvlf: count = %d, top = %d, bot = %d, left3 = %d, right3 = %d",
	count, top, bottom, left3, right3);
#endif

#ifdef CAREFUL
    if (count <= 0) /* watch out for bad calls */
	return;
#endif

    /* remove pop up box if displayed */
    killbox (TRUE); 
    sendbox (top, bottom, left3, right3);
    }

/****************************************************************************/
/* tvrt:  scrolls the tv array and screen right                             */
/*                                                                          */
/* arguments:              int count - number of columns to scroll right    */
/*                         int top, bottom, left, right -                   */
/*                                   zero based box coords of box to scroll */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp                                             */
/*                                                                          */
/* globals changed:        none                                             */
/*                                                                          */
/* synopsis:                                                                */
/*     If count is zero or negative, do nothing                             */
/*     If there is a popbox on the screen, kill it.                         */
/*     Invoke sendbox on the given box.                                     */
/****************************************************************************/

void tvrt (int count, int top, int bottom, int left4, int right4)
    {
#ifdef DEBUG
    debug ("tvrt: count = %d, top = %d, bot = %d, left4 = %d, right4 = %d",
	count, top, bottom, left4, right4);
#endif

#ifdef CAREFUL
    if (count <= 0)  /* watch out for bad calls */
	return;
#endif

    /* remove pop up box if displayed */
    killbox (TRUE); 
    sendbox (top, bottom, left4, right4);
    }

/****************************************************************************/
/* tvup:  scroll tv, screen and windows up count lines                      */
/*                                                                          */
/* arguments:              int count - number of lines to scroll up         */
/*                         int top, bottom - lines to be scrolled           */
/*                                                                          */
/* return value:           void                                             */
/*                                                                          */
/* globals referenced:     g_wp, g_curcrc, g_goalcrc, g_goalscreen,         */
/*                         g_curscreen                                      */
/*                                                                          */
/* globals changed:        g_goalcrc                                        */
/*                                                                          */
/* synopsis:                                                                */
/*     If count is negative, or larger than the height of the box, do       */
/*     nothing.                                                             */
/*                                                                          */
/*     If there is a popbox on the screen, kill it.                         */
/*                                                                          */
/*     For each line in the box to scroll, copy the modified bits array     */
/*     and the cache array from the line up to the line which is the given  */
/*     number of lines above it.  Clear the source line cache after copying */
/*     its contents up.  Use sendbox to send moved line to the screen.      */
/*     Finally, make sure the g_goalcrc global has the correct updated      */
/*     screen.                                                              */
/****************************************************************************/
void tvup (int count, int top, int bottom)
    {
    WSTRUCT *wp=g_wp;              /* current window               */
    POINTER *cache=w_cache(wp);    /* cache from line array        */

    int line;                      /* destination line for xfers   */
    int xline;                     /* line being xfered            */
    int tvline, xtvline;

    char *modflgs=w_modflgs(wp);   /* modflgs array from window    */

#ifdef DEBUG
    debug ("tvup: count = %d, top = %d, bot = %d\n", count, top, bottom);
#endif

#ifdef CAREFUL
    if ((count <= 0) || (count > bottom - top)) /* watch out for bad calls */
	return;
#endif

    /* remove pop up box if displayed */
    killbox (TRUE); 

    for (line = top; line <= bottom - count; line++)
	{
	xline = line + count;
	modflgs [line] = modflgs [xline]; /* xfer modified bits */
	modflgs [xline] = 0;

	s_free (cache [line]); /* xfer cache array entry */
	cache [line] = cache [xline] ;
	cache [xline] = 0;

	sendbox (line, line, 0, w_lrcol (wp));

	tvline = line + w_ttline (wp);
	xtvline = xline + w_ttline (wp);
	if (tvline < 0 || xtvline < 0 ||
	    tvline > LI - 2 || xtvline > LI - 2)
	    continue;

	/* avoid declaring that blank lines created by the scroller
	   are right... */

	if (scr_seq (g_goalscreen [tvline], g_curscreen [xtvline]))
	    g_goalcrc [tvline] = g_curcrc [xtvline];
	}
    }
