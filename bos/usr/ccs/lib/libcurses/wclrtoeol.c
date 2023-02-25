#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)38  1.2  src/bos/usr/ccs/lib/libcurses/wclrtoeol.c, libcurses, bos411, 9428A410j 4/15/94 17:01:03";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: wclrtoeol
 *		
 *
 *   ORIGINS: 4
 *
 *                    SOURCE MATERIALS
 */
#endif /* _POWER_PROLOG_ */


/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)curses:screen/wclrtoeol.c	1.4"		*/



#include	"curses_inc.h"

/* This routine clears up to the end of line. */

wclrtoeol(win)
register	WINDOW	*win;
{
    register	int	y = win->_cury;
    register	int	x = win->_curx;
    register	int	maxx = win->_maxx;

    memSset(&win->_y[y][x], win->_bkgd, maxx - x);

#ifdef	_VR3_COMPAT_CODE
    if (_y16update)
	(*_y16update)(win, 1, maxx - x, y, x);
#endif	/* _VR3_COMPAT_CODE */

    /* if curscr, reset blank structure */
    if (win == curscr)
    {
	if (_BEGNS[y] >= x)
	    _BEGNS[y] = maxx;
	if (_ENDNS[y] >= x)
	    _ENDNS[y] = _BEGNS[y] > x ? -1 : x-1;

	_CURHASH[y] = x == 0 ? 0 : _NOHASH;

	if (_MARKS != NULL)
	{
	    register	char	*mkp = _MARKS[y];
	    register	int	endx = COLS / BITSPERBYTE + (COLS  %BITSPERBYTE ? 1 : 0);
	    register	int	m = x / BITSPERBYTE + 1;

	    for ( ; m < endx; ++m)
		mkp[m] = 0;
	    mkp += x / BITSPERBYTE;
	    if ((m = x % BITSPERBYTE) == 0)
		*mkp = 0;
	    else
		for (; m < BITSPERBYTE; ++m)
		    *mkp &= ~(1 << m);

	    /* if color terminal, do the same for color marks	*/

	    if (_COLOR_MARKS != NULL)
	    {
	        mkp = _COLOR_MARKS[y];

		m = x / BITSPERBYTE + 1;
		for (; m < endx; ++m)
		     mkp[m] = 0;
		mkp += x / BITSPERBYTE;
		if ((m = x % BITSPERBYTE) == 0)
		     *mkp = 0;
		else
		     for (; m < BITSPERBYTE; ++m)
			 *mkp &= ~(1 << m);
	    }
	}
	return (OK);
    }
    else
    {
	/* update firstch and lastch for the line. */
#ifdef	DEBUG
        /* Comment out -- this has problems */
       /*  if (outf)
	    fprintf(outf, "CLRTOEOL: line %d minx = %d, maxx = %d, firstch = %d, lastch = %d, next firstch %d\n", y, x, maxx - win->_y[y], win->_firstch[y], win->_lastch[y], win->_firstch[y+1]);  */
#endif	/* DEBUG */

	if (win->_firstch[y] > x)
	    win->_firstch[y] = x;
	win->_lastch[y] = maxx - 1;
	win->_flags |= _WINCHANGED;
	/* sync with ancestors structures */
	if (win->_sync)
	    wsyncup(win);

	return (win->_immed ? wrefresh(win) : OK);
    }
}
