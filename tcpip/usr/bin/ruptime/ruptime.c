static char sccsid[] = "@(#)24	1.14  src/tcpip/usr/bin/ruptime/ruptime.c, tcpadmin, tcpip411, GOLD410 2/15/94 19:08:39";
/* 
 * COMPONENT_NAME: TCPIP ruptime.c
 * 
 * FUNCTIONS: MSGSTR, Mruptime, down, hscmp, interval, lcmp, tcmp, 
 *            ucmp 
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
static char sccsid[] = "ruptime.c	5.5 (Berkeley) 8/25/88";
#endif  not lint */

#include <sys/param.h>
#include <sys/dir.h>
#include <sys/file.h>
#include <protocols/rwhod.h>
#include <stdio.h>

#include <nl_types.h>
#include "ruptime_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_RUPTIME,n,s) 

#define	NHOSTS	100
int	nhosts;
struct hs {
	struct	whod *hs_wd;
	int	hs_nusers;
} hs[NHOSTS];
struct	whod awhod;

#define	WHDRSIZE	(sizeof (awhod) - sizeof (awhod.wd_we))
#define	RWHODIR		"/usr/spool/rwho"
#define	down(h)		(now - (h)->hs_wd->wd_recvtime > 11 * 60)

time_t	now;
int	rflg = 1;
int	hscmp(), ucmp(), lcmp(), tcmp();

#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	register struct hs *hsp = hs;
	register struct whod *wd;
	register struct whoent *we;
	register DIR *dirp;
	struct direct *dp;
	int aflg, cc, ch, f, i, maxloadav;
	char buf[sizeof(struct whod)];
	int (*cmp)() = hscmp;
	time_t time();
	char *interval(), *malloc();
	char *upmsg, *dnmsg;

	char  *msg;   /* added to pass msg from msg catalog */

	setlocale(LC_ALL,"");
	catd = catopen(MF_RUPTIME,NL_CAT_LOCALE);

	aflg = 0;
	maxloadav = -1;
	while ((ch = getopt(argc, argv, "alrut")) != EOF)
		switch((char)ch) {
		case 'a':
			aflg = 1;
			break;
		case 'l':
			cmp = lcmp;
			break;
		case 'r':
			rflg = -1;
			break;
		case 't':
			cmp = tcmp;
			break;
		case 'u':
			cmp = ucmp;
			break;
		default: 
			fprintf(stderr, MSGSTR(USEAGE, "Usage: ruptime [-arlut]\n")); /*MSG*/
			exit(1);
		}

	if (chdir(RWHODIR) || (dirp = opendir(".")) == NULL) {
		perror(MSGSTR(RWHODERR, RWHODIR)); /*MSG*/
		exit(1);
	}
	while (dp = readdir(dirp)) {
		if (dp->d_ino == 0 || strncmp(dp->d_name, "whod.", 5))
			continue;
		if (nhosts == NHOSTS) {
			fprintf(stderr, MSGSTR(TOOMANYHOSTS,"too many hosts\n")); /*MSG*/
			exit(1);
		}
		f = open(dp->d_name, O_RDONLY, 0);
		if (f > 0) {
			cc = read(f, buf, sizeof(struct whod));
			if (cc >= WHDRSIZE) {
				/* NOSTRICT */
				hsp->hs_wd = (struct whod *)malloc(WHDRSIZE);
				wd = (struct whod *)buf;
				bcopy(wd, hsp->hs_wd, WHDRSIZE);
				hsp->hs_nusers = 0;
				for (i = 0; i < 2; i++)
					if (wd->wd_loadav[i] > maxloadav)
						maxloadav = wd->wd_loadav[i];
				we = (struct whoent *)(buf+cc);
				while (--we >= wd->wd_we)
					if (aflg || we->we_idle < 3600)
						hsp->hs_nusers++;
				nhosts++; hsp++;
			}
			(void)close(f);
		}
	}
	if (!nhosts) {
		printf(MSGSTR(NOHOSTS, "ruptime: no hosts!?!\n")); /*MSG*/
		exit(1);
	}
	(void)time(&now);
	qsort((char *)hs, nhosts, sizeof (hs[0]), cmp);

	msg = catgets(catd,MS_RUPTIME,UPLAB,"  up");
	upmsg = malloc(strlen(msg)+1);		/* need to copy from catalog static */
	(void) strcpy(upmsg,msg);			/* string area to our own  */
	msg = catgets(catd,MS_RUPTIME,DNLAB,"down");
	dnmsg = malloc(strlen(msg)+1);
	(void) strcpy(dnmsg,msg);

	for (i = 0; i < nhosts; i++) {
		hsp = &hs[i];
		if (down(hsp)) {
			printf(MSGSTR(DNTIM, "%-12.12s%s\n"), hsp->hs_wd->wd_hostname,
		    	interval(now - hsp->hs_wd->wd_recvtime,dnmsg)); /*MSG*/
			continue;
		}
		printf(MSGSTR(UPTIME, "%-12.12s%s,  "),hsp->hs_wd->wd_hostname, 
		    interval(hsp->hs_wd->wd_sendtime -
				hsp->hs_wd->wd_boottime,upmsg)); /*MSG*/
		if(hsp->hs_nusers == 1)
			printf(MSGSTR(USER1, "%4d user, "),hsp->hs_nusers); /*MSG*/
		else
			printf(MSGSTR(USER2, "%4d users,"),hsp->hs_nusers);	     /*MSG*/
		printf(MSGSTR(LOAD, "  load %*.2f, %*.2f, %*.2f\n"),  /*MSG*/
		    maxloadav >= 1000 ? 5 : 4, hsp->hs_wd->wd_loadav[0] / 100.0,
		    	maxloadav >= 1000 ? 5 : 4, hsp->hs_wd->wd_loadav[1] / 100.0,
		    		maxloadav >= 1000 ? 5 : 4,hsp->hs_wd->wd_loadav[2] / 100.0);
		cfree(hsp->hs_wd);
	}
	exit(0);
}

char *
interval(tval, updown)
	time_t tval;
	char *updown;
{
	static char resbuf[256];
	int days, hours, minutes;

	if (tval < 0 || tval > 365*24*60*60) {
		(void) sprintf(resbuf,MSGSTR(RAWTIM, "   %s ??:??"), updown); /*MSG*/
		return(resbuf);
	}
	minutes = (tval + 59) / 60;		/* round to minutes */
	hours = minutes / 60; minutes %= 60;
	days = hours / 24; hours %= 24;
	if (days)
		(void) sprintf(resbuf, MSGSTR(DYHRMN, "%s %2d+%02d:%02d"),
		    updown, days, hours, minutes);
	else
		(void) sprintf(resbuf, MSGSTR(HRMIN, "%s    %2d:%02d"), 
		    updown, hours, minutes);
	return(resbuf);
}

hscmp(h1, h2)
	struct hs *h1, *h2;
{
	return(rflg * strcmp(h1->hs_wd->wd_hostname, h2->hs_wd->wd_hostname));
}

/*
 * Compare according to load average.
 */
lcmp(h1, h2)
	struct hs *h1, *h2;
{
	if (down(h1))
		if (down(h2))
			return(tcmp(h1, h2));
		else
			return(rflg);
	else if (down(h2))
		return(-rflg);
	else
		return(rflg *
			(h2->hs_wd->wd_loadav[0] - h1->hs_wd->wd_loadav[0]));
}

/*
 * Compare according to number of users.
 */
ucmp(h1, h2)
	struct hs *h1, *h2;
{
	if (down(h1))
		if (down(h2))
			return(tcmp(h1, h2));
		else
			return(rflg);
	else if (down(h2))
		return(-rflg);
	else
		return(rflg * (h2->hs_nusers - h1->hs_nusers));
}

/*
 * Compare according to uptime.
 */
tcmp(h1, h2)
	struct hs *h1, *h2;
{
	return(rflg * (
		(down(h2) ? h2->hs_wd->wd_recvtime - now
			  : h2->hs_wd->wd_sendtime - h2->hs_wd->wd_boottime)
		-
		(down(h1) ? h1->hs_wd->wd_recvtime - now
			  : h1->hs_wd->wd_sendtime - h1->hs_wd->wd_boottime)
	));
}
