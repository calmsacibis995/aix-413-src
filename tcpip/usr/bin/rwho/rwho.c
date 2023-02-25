static char sccsid[] = "@(#)25	1.13  src/tcpip/usr/bin/rwho/rwho.c, tcpadmin, tcpip41J, 9509A 2/23/95 09:26:03";
/* 
 * COMPONENT_NAME: TCPIP rwho.c
 * 
 * FUNCTIONS: MSGSTR, Mrwho, down, utmpcmp 
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
 * Copyright (c) 1983 The Regents of the University of California.
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
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "rwho.c	5.3 (Berkeley) 8/25/88";
#endif  not lint */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <protocols/rwhod.h>
#include <stdio.h>

DIR	*dirp;

struct	whod wd;
int	utmpcmp();
#define	NUSERS	1000
struct	myutmp {
	char	myhost[MAXHOSTNAMELEN];
	int	myidle;
	struct	outmp myutmp;
} myutmp[NUSERS];
int	nusers;

#include <nl_types.h>
#include "rwho_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_RWHO,n,s) 

#define	WHDRSIZE	(sizeof (wd) - sizeof (wd.wd_we))
#define	RWHODIR		"/usr/spool/rwho"
/* 
 * this macro should be shared with ruptime.
 */
#define	down(w,now)	((now) - (w)->wd_recvtime > 11 * 60)

char	*ctime(), *strcpy();
time_t	now;
int	aflg;

#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int ch;
	struct direct *dp;
	int cc, width;
	register struct whod *w = &wd;
	register struct whoent *we;
	register struct myutmp *mp;
	int f, n, i;
	time_t time();
	struct tm *tmptr;
	char temputline[10];

	setlocale(LC_ALL,"");
	catd = catopen(MF_RWHO,NL_CAT_LOCALE);
	while ((ch = getopt(argc, argv, "a")) != EOF)
		switch((char)ch) {
		case 'a':
			aflg = 1;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR(USAGE, "usage: rwho [-a]\n"));
			exit(1);
		}
	if (chdir(RWHODIR) || (dirp = opendir(".")) == NULL) {
		perror(MSGSTR(RWHOCD, RWHODIR)); /*MSG*/
		exit(1);
	}
	mp = myutmp;
	(void)time(&now);
	while (dp = readdir(dirp)) {
		if (dp->d_ino == 0 || strncmp(dp->d_name, "whod.", 5))
			continue;
		f = open(dp->d_name, O_RDONLY);
		if (f < 0)
			continue;
		cc = read(f, (char *)&wd, sizeof (struct whod));
		if (cc < WHDRSIZE) {
			(void) close(f);
			continue;
		}
		if (down(w,now)) {
			(void) close(f);
			continue;
		}
		cc -= WHDRSIZE;
		we = w->wd_we;
		for (n = cc / sizeof (struct whoent); n > 0; n--) {
			if (aflg == 0 && we->we_idle >= 60*60) {
				we++;
				continue;
			}
			if (nusers >= NUSERS) {
				printf(MSGSTR(TOOMANYUSERS, "too many users\n")); /*MSG*/
				exit(1);
			}
			mp->myutmp = we->we_utmp; mp->myidle = we->we_idle;
			(void) strcpy(mp->myhost, w->wd_hostname);
			nusers++; we++; mp++;
		}
		(void) close(f);
	}
	qsort((char *)myutmp, nusers, sizeof (struct myutmp), utmpcmp);
	mp = myutmp;
	width = 0;
	for (i = 0; i < nusers; i++) {
		int j = strlen(mp->myhost) + 1 + strlen(mp->myutmp.out_line);
		if (j > width)
			width = j;
		mp++;
	}
	mp = myutmp;
	for (i = 0; i < nusers; i++) {
		char buf[BUFSIZ];
		strncpy(temputline, mp->myutmp.out_line, 8);
		temputline[8] = '\0';
		(void)sprintf(buf, MSGSTR(HOSTNM, "%s:%s"), mp->myhost, temputline); /*MSG*/
		printf("%-8.8s %-*s", mp->myutmp.out_name, width, buf);
		tmptr = (struct tm *)localtime((time_t *)&mp->myutmp.out_time);
		if (strftime(buf, BUFSIZ, "%sD %H:%M", tmptr))
			printf(" %.12s", buf);
		else
			printf(" %.12s",
				ctime((time_t *)&mp->myutmp.out_time)+4);
		mp->myidle /= 60;
		if (mp->myidle) {
			if (aflg) {
				if (mp->myidle >= 100*60)
					mp->myidle = 100*60 - 1;
				if (mp->myidle >= 60)
					printf(MSGSTR(IDLE, " %2d"), mp->myidle / 60); /*MSG*/
				else
					printf(MSGSTR(SPC3, "   ")); /*MSG*/
			} else
				printf(MSGSTR(SPC1, " ")); /*MSG*/
			printf(MSGSTR(IDLE2, ":%02d"), mp->myidle % 60); /*MSG*/
		}
		printf(MSGSTR(CRLF, "\n")); /*MSG*/
		mp++;
	}
	exit(0);
}

utmpcmp(u1, u2)
	struct myutmp *u1, *u2;
{
	int rc;

	rc = strncmp(u1->myutmp.out_name, u2->myutmp.out_name, 8);
	if (rc)
		return (rc);
	rc = strncmp(u1->myhost, u2->myhost, 8);
	if (rc)
		return (rc);
	return (strncmp(u1->myutmp.out_line, u2->myutmp.out_line, 8));
}
