static char sccsid[] = "@(#)02	1.5.1.1  src/bos/usr/sbin/strace/strace.c, cmdpse, bos411, 9428A410j 11/16/93 08:57:05";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stropts.h>
#include <sys/strlog.h>

#include "strace_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRACE,n,s)

#define	MAXTIDS	10
#define	LOG	"/dev/slog"

#define	NLOGARGS 4

extern char *ctime();

main(ac, av)
	int ac;
	char **av;
{
	int fd;
	int flags;
	char buf[LOGMSGSZ+(WORDLEN*NLOGARGS)];
	struct strioctl stri;
	struct strbuf ctl, data;
	struct trace_ids ti[MAXTIDS], *tip;
	struct log_ctl logctl;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRACE, NL_CAT_LOCALE);

	if ((fd = open(LOG, 2)) == -1) {
		perror(MSGSTR(MSG1, "strace: cannot open log device"));
		exit(1);
	}

	stri.ic_cmd = I_TRCLOG;
	stri.ic_timout = -1;
	stri.ic_dp = (char *)ti;
	stri.ic_len = 0;

	if (--ac) {
		if (ac % 3) {
			fprintf(stderr, MSGSTR(USAGE1, "usage: strace [mid sid level] ...\n"));
			exit(1);
		}
		for (tip = ti; ac && tip < &ti[MAXTIDS]; ++tip) {
			tip->ti_mid =   !strcmp(av[1], "all") ?-1:atoi(av[1]);
			tip->ti_sid =   !strcmp(av[2], "all") ?-1:atoi(av[2]);
			tip->ti_level = !strcmp(av[3], "all") ?-1:atoi(av[3]);
			stri.ic_len += sizeof(*tip);
			av += 3;
			ac -= 3;
		}
	} else {
		tip = ti;
		tip->ti_mid = -1;
		tip->ti_sid = -1;
		tip->ti_level = -1;
		stri.ic_len = sizeof(ti[0]);
	}
	if (ioctl(fd, I_STR, (char *)&stri)) {
		/* always ENXIO */
		fprintf(stderr, MSGSTR(MSG2, "strace: already registered\n"));
		exit(1);
	}

	ctl.buf = (char *)&logctl;
	ctl.maxlen = sizeof logctl;
	data.buf = buf;
	data.maxlen = sizeof buf;
	flags = 0;

	while (getmsg(fd, &ctl, &data, &flags) != -1) {
		char *cp;
		int *ap;
		int i;

		cp = ctime(&logctl.ttime) + sizeof("Mon Day DD ") - 1;
		cp[8] = 0;
		printf("%06d %s %08x %2d ", logctl.seq_no, cp,
			logctl.ltime, (signed char)logctl.level);
		putchar(logctl.flags & SL_FATAL  ? 'F' : '.');
		putchar(logctl.flags & SL_NOTIFY ? 'N' : '.');
		putchar(logctl.flags & SL_ERROR  ? 'E' : '.');
		printf(" %d %d ", logctl.mid, logctl.sid);
		i = (int)buf + strlen(buf);
		ap = (int *)(i + (WORDLEN - i % WORDLEN));
		printf(buf, ap[0], ap[1], ap[2], ap[3]);
		putchar('\n');
		fflush(stdout);
	}
	exit(1);
}
