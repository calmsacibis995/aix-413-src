static char sccsid[] = "@(#)01	1.7.1.4  src/bos/usr/sbin/halt/halt.c, cmdoper, bos411, 9428A410j 11/22/93 12:31:25";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: halt
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include "halt_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_HALT,n,s) 

/*
 * NAME: Halt
 * FUNCTION: stops the processor.
 *   -n  Prevents the sync before stopping
 *   -q  Causes a quick halt, no graceful stop in attempted.
 *   -y  Halts the system from a dialup.
 *   -l  Does not log the halt in the system accounting records.
 */
#include <stdio.h>
#include <locale.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/syslog.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <utmp.h>
#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
char	wtmpf[]	= "/var/adm/wtmp";
struct utmp wtmp;

main(int argc, char **argv)
{
	int nosync = 0;
	char *ttyn = (char *)ttyname(2);
	register int qflag = 0;
	int needlog = 1;
	char *user, *getlogin();
	struct passwd *pw;
	int opt;

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_HALT, NL_CAT_LOCALE);
	while((opt = getopt(argc,argv,"lnqy")) != EOF) {
		switch (opt) {
		case 'l':		/* do not log halt */
			needlog = 0;
			break;
		case 'n':		/* do not sync or log */
			needlog = 0;
			nosync++;
			break;
		case 'q':		/* halt quickly, do not log */
			qflag++;
			needlog = 0;
			break;
		case 'y':		/* halting from a dailup */
			ttyn = 0;
			break;
		default:
			fprintf(stderr, MSGSTR(USAGE, "usage: halt [-y][-q][-l][-n]\n"));
			exit(1);
			break;
		}
	}

	if (ttyn && *(ttyn+strlen("/dev/tty")) == 'd') {
		fprintf(stderr, MSGSTR(DIALUP, "halt: dangerous on a dialup; use ``halt -y'' if you are really sure\n"));
		exit(1);
	}

	if (needlog) {
		openlog("halt", 0, LOG_AUTH);
		user = getlogin();
		if (user == (char *)0 && (pw = getpwuid((uid_t)getuid())))
			user = pw->pw_name;
		if (user == (char *)0)
			user = "root";
		syslog(LOG_CRIT, MSGSTR(LOGHLT, "halted by %s"), user);
		markdown();
	}

	signal(SIGHUP, SIG_IGN);		/* for network connections */

	if (!qflag) {
		if (!nosync)
			sync();
		killall();
		setalarm(5);
		pause();
		if (!nosync)
			sync();
	}
	printf(MSGSTR(HALTOVER, "....Halt completed....\n") );
	fflush(stdout);
	sleep(3);
	reboot(RB_HALT,(time_t *)0);
	perror("reboot");
	exit(1);
}

/*
 * NAME: sigalrm
 * FUNCTION: catch SIGALRM
 */
sigalrm(void)
{
}

/*
 * NAME: setalarm
 * FUNCTION:  set alarm
 */
setalarm(n)
{
	signal(SIGALRM, (void (*)(int))sigalrm);
	alarm(n);
}

/*
 * NAME: markdown
 * FUNCTION: log shutdown procedure
 */
markdown()
{
	char *line;
	char *tmps;

	register f = open(wtmpf, 1);
	if (f >= 0) {
		lseek(f, 0L, 2);
		if ((line = (char *)ttyname(1)) == NULL)	
			SCPYN(wtmp.ut_line, "~");
		else {
			tmps = (char *) strchr(line,'/');
			tmps++;
			line = (char *) strchr(tmps,'/');
			line++;
			SCPYN(wtmp.ut_line,line);
		}
		SCPYN(wtmp.ut_name, "shutdown");
		SCPYN(wtmp.ut_host, "");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		fsync(f);
		close(f);
	}
}

/*
 * NAME: killall
 * FUNCTION: kill any processes that are running except this one
 */
killall()
{
	if (fork() == 0) {     /* child */
		execl("/usr/sbin/killall","killall","-",0);
		fprintf(stderr,MSGSTR(CAUTION,"CAUTION: some process(es) wouldn't die\n"));
	}
	else
		wait((int *)0);
}
