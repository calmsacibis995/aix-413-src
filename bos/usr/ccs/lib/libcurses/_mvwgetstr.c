#ifdef _POWER_PROLOG_
static char sccsid[] = "@(#)49  1.1  src/bos/usr/ccs/lib/libcurses/_mvwgetstr.c, libcurses, bos411, 9428A410j 9/3/93 15:03:59";
/*
 *   COMPONENT_NAME: LIBCURSES
 *
 *   FUNCTIONS: mvwgetstr
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

/* #ident	"@(#)curses:screen/_mvwgetstr.c	1.1"		*/



#define		NOMACROS
#include	"curses_inc.h"

mvwgetstr(win, y, x, str)
WINDOW	*win;
int	y, x;
char	*str;
{
    return (wmove(win, y, x)==ERR?ERR:wgetstr(win, str));
}
