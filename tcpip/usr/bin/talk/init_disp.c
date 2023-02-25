static char sccsid[] = "@(#)38	1.11  src/tcpip/usr/bin/talk/init_disp.c, tcptalk, tcpip41J, 9509A 2/24/95 10:57:23";
/* 
 * COMPONENT_NAME: TCPIP init_disp.c
 * 
 * FUNCTIONS: MSGSTR, init_display, quit, set_edit_chars, sig_sent 
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
static char sccsid[] = "init_disp.c	5.3 (Berkeley) 6/29/88";
#endif  not lint */

/*
 * Initialization code for the display package,
 * as well as the signal handling routines.
 */

#include "talk.h"
#include <signal.h>

#include <nl_types.h>
#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 
/* 
 * Set up curses, catch the appropriate signals,
 * and build the various windows.
 */
init_display()
{
	void sig_sent();
	struct sigaction sigact;

	initscr();
	(void) sigaction(SIGTSTP, (struct sigaction *)0, &sigact);
	SIGADDSET(sigact.sa_mask, SIGALRM);
	(void) sigaction(SIGTSTP, &sigact, (struct sigaction *)0);
	curses_initialized = 1;
	clear();
	refresh();
	noecho();
	crmode();
	signal(SIGINT, sig_sent);
	signal(SIGPIPE, sig_sent);
	/* curses takes care of ^Z */
	my_win.x_nlines = LINES / 2;
	my_win.x_ncols = COLS;
	my_win.x_win = newwin(my_win.x_nlines, my_win.x_ncols, 0, 0);
	scrollok(my_win.x_win, FALSE);
	wclear(my_win.x_win);

	his_win.x_nlines = LINES / 2 - 1;
	his_win.x_ncols = COLS;
	his_win.x_win = newwin(his_win.x_nlines, his_win.x_ncols,
	    my_win.x_nlines+1, 0);
	scrollok(his_win.x_win, FALSE);
	wclear(his_win.x_win);

	line_win = newwin(1, COLS, my_win.x_nlines, 0);
	box(line_win, '-', '-');
	wrefresh(line_win);
	/* let them know we are working on it */
	current_state = MSGSTR(NO_CONN_YET, "No connection yet");
}

/*
 * Trade edit characters with the other talk. By agreement
 * the first three characters each talk transmits after
 * connection are the three edit characters.
 */
#ifdef _AIX
#include <termio.h>
char sbld[TTNAMEMAX];
#define LINEDISP "posix"
#endif /* _AIX */

set_edit_chars()
{
	char buf[3];
	int cc;
#ifdef _AIX
	struct termio tio;
#else
	struct sgttyb tty;
#endif /* _AIX */
	struct ltchars ltc;
	
#ifdef _AIX
#ifdef TXGETLD
	ioctl(0, TXGETLD, sbld);
#endif /* TXGETLD */
#ifdef TXSETLD
	ioctl(0, TXSETLD, LINEDISP);
#endif /* TXSETLD */
	ioctl(0, TCGETA, &tio);
	tio.c_lflag &= ~ICANON;
	tio.c_lflag &= ~ECHO;
	tio.c_cc[4] = 1;             /* VMIN */
	tio.c_cc[5] = 1;             /* VTIME */
	ioctl(0, TCSETA, &tio);
	my_win.cerase = tio.c_cc[VERASE];
	my_win.kill = tio.c_cc[VKILL];
        my_win._werase = -1;
#else
	ioctl(0, TIOCGETP, &tty);
	ioctl(0, TIOCGLTC, (struct sgttyb *)&ltc);
	my_win.cerase = tty.sg_erase;
	my_win.kill = tty.sg_kill;
	if (ltc.t_werasc == (char) -1)
		my_win._werase = '\027';	 /* control W */
	else
		my_win._werase = ltc.t_werasc;
#endif /* _AIX */

	buf[0] = my_win.cerase;
	buf[1] = my_win.kill;
	buf[2] = my_win._werase;
	cc = write(sockt, buf, sizeof(buf));
	if (cc != sizeof(buf) )
		p_error(MSGSTR(LOST_CONN, "Lost the connection")); /*MSG*/
	cc = read(sockt, buf, sizeof(buf));
	if (cc != sizeof(buf) )
		p_error(MSGSTR(LOST_CONN, "Lost the connection")); /*MSG*/
	his_win.cerase = buf[0];
	his_win.kill = buf[1];
	his_win._werase = buf[2];
}

void
sig_sent()
{

	message(MSGSTR(CONN_CLOSE, "Connection closing. Exiting")); /*MSG*/
	quit(0);
}

/*
 * All done talking...hang up the phone and reset terminal thingy's
 */
quit(rc)
int rc;
{

	if (curses_initialized) {
#ifdef _AIX
#ifdef TXSETLD
	        ioctl(0, TXSETLD, sbld);
#endif /* TXSETLD */
#endif /* _AIX */
		wmove(his_win.x_win, his_win.x_nlines-1, 0);
		wclrtoeol(his_win.x_win);
		wrefresh(his_win.x_win);
		endwin();
	}
	if (invitation_waiting)
		send_delete();
	exit(rc);
}
