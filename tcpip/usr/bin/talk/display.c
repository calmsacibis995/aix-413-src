static char sccsid[] = "@(#)34	1.5  src/tcpip/usr/bin/talk/display.c, tcptalk, tcpip411, GOLD410 9/6/91 17:15:09";
/* 
 * COMPONENT_NAME: TCPIP display.c
 * 
 * FUNCTIONS: display, max, readwin, xscroll 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
static char sccsid[] = "display.c	5.3 (Berkeley) 6/29/88";
#endif  not lint */

/*
 * The window 'manager', initializes curses and handles the actual
 * displaying of text
 */
#include "talk.h"
#include <stdlib.h>

xwin_t	my_win;
xwin_t	his_win;
WINDOW	*line_win;

int	curses_initialized = 0;

/*
 * max HAS to be a function, it is called with
 * a argument of the form --foo at least once.
 */
max(a,b)
	int a, b;
{

	return (a > b ? a : b);
}

/*
 * Display some text on somebody's window, processing some control
 * characters while we are at it.
 */
display(win, text, size)
	register xwin_t *win;
	register char *text;
	int size;
{
	register int i;
	char cch;
	int count = 1;

	for (i = 0; i < size; i += count) {
		if (MB_CUR_MAX > 1) {		/* multibyte code */
			wchar_t	wc_text, wc_win, wc_cch;

			count = mbtowc(&wc_text, text, MB_CUR_MAX);
			if (count < 0) {  
				text++;
				count = 1;
				continue;
			} 
                	if (wc_text == '\n') {
                        	xscroll(win, 0);
                        	text += count;
                        	continue;
                	}
                /* erase character */
                	if (wc_text == win->cerase) {
                           wmove(win->x_win, win->x_line, max(--win->x_col, 0));
                           getyx(win->x_win, win->x_line, win->x_col);
			   waddch(win->x_win, ' ');
                           wmove(win->x_win, win->x_line, win->x_col);
                           getyx(win->x_win, win->x_line, win->x_col);
                           text += count;
                           continue;
                	}
                /*
                 * On word erase search backwards until we find
                 * the beginning of a word or the beginning of
                 * the line.
                 */
                	if (wc_text == win->_werase) {
                        	int endcol, xcol, i, c;

         		        endcol = win->x_col;
                        	xcol = endcol - 1;
                        	while (xcol >= 0) {
                                   c = readwin(win->x_win, win->x_line, xcol);
                                   if (c != ' ')
                                        break;
                                   xcol--;
                        	}
                	        while (xcol >= 0) {
                                   c = readwin(win->x_win, win->x_line, xcol);
                                   if (c == ' ')
                                        break;
                                   xcol--;
                        	}
                        	wmove(win->x_win, win->x_line, xcol + 1);
                        	for (i = xcol + 1; i < endcol; i++)
                               	    waddch(win->x_win, ' ');
                        	wmove(win->x_win, win->x_line, xcol + 1);
                        	getyx(win->x_win, win->x_line, win->x_col);
                        	continue;
                	}
                /* line kill */
                	if (wc_text == win->kill) {
                        wmove(win->x_win, win->x_line, 0);
                        wclrtoeol(win->x_win);
                        getyx(win->x_win, win->x_line, win->x_col);
                        text += count;
                        continue;
                	}
                	if (wc_text == '\f') {
                        	if (win == &my_win)
                                	wrefresh(curscr);
                        	text += count;
                        	continue;
                	}
                	if (win->x_col == COLS-1) {
                	        /* check for wraparound */
                	        xscroll(win, 0);
                	}
                	if (wc_text < ' ' && wc_text != '\t') {
                        	waddch(win->x_win, '^');
                        	getyx(win->x_win, win->x_line, win->x_col);
				/* check for wraparound */
                        	if (win->x_col == COLS-1) 
                                	xscroll(win, 0);
                        	wc_cch = (wc_text & 63) + 64;
                        	waddch(win->x_win, wc_cch);
                	} else
                        	waddch(win->x_win, wc_text);
                	getyx(win->x_win, win->x_line, win->x_col);
                	text += count;
        	}
		else {			/* single byte code */
			if (*text == '\n') {
				xscroll(win, 0);
				text++;
				continue;
			}
		/* erase character */
			if (*text == win->cerase) {
			   wmove(win->x_win, win->x_line, max(--win->x_col, 0));
			   getyx(win->x_win, win->x_line, win->x_col);
			   waddch(win->x_win, ' ');
			   wmove(win->x_win, win->x_line, win->x_col);
			   getyx(win->x_win, win->x_line, win->x_col);
			   text++;
			   continue;
			}
		/*
		 * On word erase search backwards until we find
		 * the beginning of a word or the beginning of
		 * the line.
		 */
			if (*text == win->_werase) {
				int endcol, xcol, i, c;

				endcol = win->x_col;
				xcol = endcol - 1;
				while (xcol >= 0) {
				   c = readwin(win->x_win, win->x_line, xcol);
				   if (c != ' ')
				   	break;
				    xcol--;
				}
				while (xcol >= 0) {
				   c = readwin(win->x_win, win->x_line, xcol);
				   if (c == ' ')
					break;
				   xcol--;
				}
				wmove(win->x_win, win->x_line, xcol + 1);
				for (i = xcol + 1; i < endcol; i++)
					   waddch(win->x_win, ' ');
				wmove(win->x_win, win->x_line, xcol + 1);
				getyx(win->x_win, win->x_line, win->x_col);
				continue;
			}
		/* line kill */
			if (*text == win->kill) {
				wmove(win->x_win, win->x_line, 0);
				wclrtoeol(win->x_win);
				getyx(win->x_win, win->x_line, win->x_col);
				text++;
				continue;
			}
			if (*text == '\f') {
				if (win == &my_win)
					wrefresh(curscr);
				text++;
				continue;
			}
			if (win->x_col == COLS-1) {
			/* check for wraparound */
				xscroll(win, 0);
			}
			if (*text < ' ' && *text != '\t') {
				waddch(win->x_win, '^');
				getyx(win->x_win, win->x_line, win->x_col);
				/* check for wraparound */
				if (win->x_col == COLS-1) 
					xscroll(win, 0);
				cch = (*text & 63) + 64;
				waddch(win->x_win, cch);
			} else
				waddch(win->x_win, *text);
			getyx(win->x_win, win->x_line, win->x_col);
			text++;
		}
	}
	wrefresh(win->x_win);
}

/*
 * Read the character at the indicated position in win
 */
readwin(win, line, col)
	WINDOW *win;
{
	int oldline, oldcol;
	register int c;

	getyx(win, oldline, oldcol);
	wmove(win, line, col);
	c = winch(win);
	wmove(win, oldline, oldcol);
	return (c);
}

/*
 * Scroll a window, blanking out the line following the current line
 * so that the current position is obvious
 */
xscroll(win, flag)
	register xwin_t *win;
	int flag;
{

	if (flag == -1) {
		wmove(win->x_win, 0, 0);
		win->x_line = 0;
		win->x_col = 0;
		return;
	}
	win->x_line = (win->x_line + 1) % win->x_nlines;
	win->x_col = 0;
	wmove(win->x_win, win->x_line, win->x_col);
	wclrtoeol(win->x_win);
	wmove(win->x_win, (win->x_line + 1) % win->x_nlines, win->x_col);
	wclrtoeol(win->x_win);
	wmove(win->x_win, win->x_line, win->x_col);
}
