static char sccsid[] = "@(#)52        1.11  src/tcpip/usr/lbin/talkd/talkd.c, tcptalk, tcpip411, GOLD410 3/8/94 18:52:39";
/* 
 * COMPONENT_NAME: TCPIP talkd.c
 * 
 * FUNCTIONS: MSGSTR, Mtalkd, timeout, trace_handler
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
char copyright[] =
" Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif  

#ifndef lint
static char sccsid[] = "talkd.c	5.4 (Berkeley) 6/18/88";
#endif  not lint */

/*
 * The top level of the daemon, the format is heavily borrowed
 * from rwhod.c. Basically: find out who and where you are; 
 * disconnect all descriptors and ttys, and then endless
 * loop on waiting for and processing requests
 */
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/syslog.h>

#include <protocols/talkd.h>

#include <nl_types.h>
#include "talkd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALKD,n,s) 
CTL_MSG		request;
CTL_RESPONSE	response;

int	tracing = 0;
int	sockt;
int	debug = 0;
int	timeout();
long	lastmsgtime;

char	hostname[32];

#define NO_CHILD -1
pid_t child_pid = NO_CHILD;;


#define TIMEOUT 30
#define MAXIDLE 120

#include <locale.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	register CTL_MSG *mp = &request;
	int cc;
	int ch;
	int on = 1;
	struct sigvec sv;
	int trace_handler();

	setlocale(LC_ALL,"");
	catd = catopen(MF_TALKD,NL_CAT_LOCALE);
	if (getuid()) {
		fprintf(stderr, MSGSTR(ERR_GETUID, "%s: getuid: not super-user"), argv[0]); /*MSG*/
		exit(1);
	}
	openlog("talkd", LOG_PID, LOG_DAEMON);
	if (gethostname(hostname, sizeof (hostname) - 1) < 0) {
		syslog(LOG_ERR, MSGSTR(GTHSTNM_SYSLOG, "gethostname: %m")); /*MSG*/
		_exit(1);
	}
	if (chdir("/dev") < 0) {
		syslog(LOG_ERR, MSGSTR(CHDIR_SYSLOG, "chdir: /dev: %m")); /*MSG*/
		_exit(1);
	}
	while ((ch = getopt(argc, argv, "ds")) != EOF)
		switch (ch) {
		case 'd':
			debug = 1;
			break;
		case 's':
			tracing = 1;
			break;
		case '?':
		default:
			syslog(LOG_ERR, "usage: talkd [-d] [-s]");
			break;
		}
	signal(SIGALRM, timeout);
	alarm(TIMEOUT);
	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);
	for (;;) {
		extern int errno;

		cc = recv(0, (char *)mp, sizeof (*mp), 0);
		if (cc != sizeof (*mp)) {
			if (cc < 0 && errno != EINTR)
				syslog(LOG_WARNING, MSGSTR(RECV_SYSLOG, "recv: %m")); /*MSG*/
			continue;
		}
		lastmsgtime = time(0);
		process_request(mp, &response);
		/* can block here, is this what I want? */
		cc = sendto(sockt, (char *)&response,
		        sizeof (response), 0, (struct sockaddr *)&mp->ctl_addr, 
			sizeof (mp->ctl_addr));
		if (cc != sizeof (response))
			syslog(LOG_WARNING, MSGSTR(SENDTO_SYSLOG, "sendto: %m")); /*MSG*/
	}
}

timeout()
{
	if (time(0) - lastmsgtime >= MAXIDLE) {
		if (child_pid != NO_CHILD)
			kill(child_pid, SIGHUP);
		_exit(0);
	}
	alarm(TIMEOUT);
}

/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
trace_handler(sig)
	int	sig;
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/
}

