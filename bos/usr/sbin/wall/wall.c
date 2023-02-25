static char sccsid[] = "@(#)11 1.16.1.5  src/bos/usr/sbin/wall/wall.c, cmdcomm, bos41B, 9504A 12/23/94 12:51:31";
/*
 * COMPONENT_NAME: (CMDCOMM) user to user communication
 *
 * FUNCTIONS: wall
 *
 * ORIGINS: 3 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
/*
 * wall.c - Broadcast a message to all users.
 *
 * This program is not related to David Wall, whose Stanford Ph.D. thesis
 * is entitled "Mechanisms for Broadcast and Selective Broadcast".
 * It was taken from the 4.5 berkeley wall and had the features of the
 * IS/1 "chicken little" program folded in.  With no arguments it acts
 * as wall.  If arguments are specified, they are the message to be
 * broadcast.
 */

#include <stdio.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <time.h>
#include <signal.h>
#include <IN/ERdefs.h>
#include <nl_types.h>
#include "wall_msg.h"
#include <libIN_msg.h>
static nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_WALL,Num,Str)

#define MAXMSG  3000

char    *progname;

static int     msize;
static char    hostname[32];
static char    mesg[MAXMSG];
static struct	utmp *utmp;
extern char   *LGname(), *ttyname(), *CSsname();
extern char   *CSskpa(), *malloc();
static int	fn(void);

main(argc, argv)
char *argv[];
{
	register i, entries;
	register char *s;
	register struct utmp *p;
	char *n, *t;
	FILE *f;
	char line_name[12];
	char user_name[8];
	struct stat statb;
	long timenow;
	char when[NLTBMAX];

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_WALL, NL_CAT_LOCALE);
	progname = argv[0];

        (void) gethostname(hostname, sizeof (hostname));
	/*
	 * read in the utmp structure to find out which lines are in use
	 */
	if((f = fopen("/etc/utmp", "r")) == NULL)
	{       ERcmderr(-1, ER_OPEN("/etc/utmp"));
		exit(1);
	} else
	{	
	    	(void)fstat(fileno(f), &statb);
		if ((utmp = (struct utmp *)malloc(statb.st_size)) == NULL) {
		    fprintf(stderr, MSGSTR(NOMEMORY,"wall: Cannot allocate enough memory.\n"));
		    exit(1);
		}
		bzero(utmp, statb.st_size);
		entries = statb.st_size / sizeof (struct utmp);
		fread((void *)utmp,(size_t)sizeof(struct utmp),(size_t)entries,f);
		fclose(f);
	}

	/*
	 * figure out who is sending the message
	 */
	n = LGname();
	if(!(t=ttyname(0)))
		t=ctermid(0);
	s = CSsname(t);
	/* HFT hack:  If last link in device name is all numeric, take it
	 * as a channel number, and back up tail pointer to include previous
	 * link, as well.
	 */
	if (!*CSskpa(s, "0123456789") && s != t)
		for (--s; t < s && *(s - 1) != '/'; )
			--s;
	time(&timenow);
	strftime(when, NLTBMAX, "%X", localtime(&timenow));
	sprintf(mesg, MSGSTR(BRODCAST,
		"\r\n\n\007Broadcast message from %s@%s (%s) at %s ... \r\n\n"), n, hostname, s, when);
	msize = strlen(mesg);

	/*
	 * if no arguments passed, message will come from standard input
	 */
	if (argc == 1)
	{	while( (i = getc(stdin)) != EOF )
		{	if (msize >= MAXMSG)
			{   ERcmderr(0, MSGSTR(TOOLONG,
				"Message too long (max %d chars)."), MAXMSG);
			    exit(1);
			}
			mesg[msize++] = i;
		}
	} else
	{	for(i = 1; i < argc; i++)
		{	for(s = argv[i]; *s; s++)
				mesg[msize++] = *s;
			mesg[msize++] = ' ';
		}
		mesg[msize - 1] = '\n';
	}
	
	mesg[msize++] = '\n';

	/*
	 * now, loop through all logged in ports and send the message
	 */
	line_name[0] = (char)NULL; user_name[0] = (char)NULL;
	/* Set the process group id to the same value as the parent pid. */
	setpgrp();
	for(i = 0; i < entries; i++)
	{	p = &utmp[i];
		strncpy ( line_name, p->ut_line, 11 );
		line_name[11] = (char)NULL;
		strncpy ( user_name, p->ut_user, 7 );
		user_name[7] = (char)NULL;
		if ( (p->ut_type == USER_PROCESS) && 	/* p38921 */ 
				( line_name[0] != (char)NULL ) &&
				( user_name[0] != (char)NULL )  )
			sendmes(line_name);
	}

	/*
	 * pick up all the bodies
	 */
        /* This wall(parent) process and its child processes will be terminated
after 30s. */
        signal(SIGALRM, (void (*)(int)) fn);
        alarm(30);
	while(wait((int *)0) != -1)
		continue;
	exit(0);
}

static sendmes(tty)
char *tty;
{

	register i;
	char t[50], buf[BUFSIZ];
	register char *cp;
	register int c, ch;
	FILE *f;

	while ((i = fork()) == -1)
		if (wait((int *)0) == -1)
		{	fprintf(stderr, MSGSTR(TRYAGAIN,"wall: Cannot create another process at this time.\n"));
			return;
		}

/*
 * the actual open and write are done in a separate process, to isolate
 *	the parent from hanging opens
 */
	if (i)
		return;
	strcpy(t, "/dev/");
	strcat(t, tty );

	signal(SIGALRM, SIG_DFL);	/* blow away if open hangs */
	alarm(20);

	if((f = fopen(t, "w")) == NULL)
	{       ERcmderr(-1, ER_OPEN(t));
		exit(1);
	}
	setbuf(f, buf);

	/*
	 * you never know whether or not someone has newline
	 *	mapping enabled - so do our own crlf just for
	 *	yucks.
	 */
	for (cp = mesg, c = msize; c-- > 0; cp++) {
		ch = *cp;
		if (ch == '\n')
			putc('\r', f);
		putc(ch, f);
	}

	fclose(f);
	exit(0);
}

static fn(void)
{
	kill(0,SIGKILL);
}
