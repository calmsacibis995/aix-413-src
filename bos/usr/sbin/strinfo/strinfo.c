static char sccsid[] = "@(#)16        1.2.1.4  src/bos/usr/sbin/strinfo/strinfo.c, cmdpse, bos411, 9434A411a 8/19/94 14:09:19";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * strinfo - get info from MPS
 *
 * synopsis:
 *	strinfo -m
 *	strinfo -q
 */

#include <stdio.h>
#include <sys/strinfo.h>

#include "strinfo_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRINFO,n,s)

char *Prog;

#define	Fprintf	(void)fprintf

void usage();
extern char *strrchr();

static	char	buf[1024];

main(ac, av)
	int ac;
	char **av;
{
	int c;
	int cmd = 0;
	int len = 0;

	extern int optind;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRINFO, NL_CAT_LOCALE);

	if (Prog = strrchr(av[0], '/'))
		++Prog;
	else
		Prog = av[0];

	while ((c = getopt(ac, av, "?mq")) != EOF) {
		switch (c) {
		case 'm':	cmd = STR_INFO_MODS; break;
		case 'q':	cmd = STR_INFO_RUNQ; break;
		case '?':	/* fallthrough */
		default:
			usage();
		}
	}
	if (cmd == 0 || ac - optind)
		usage();
	
	if (strinfo(cmd, buf, &len)) {
		(void)perror(Prog);
		exit(1);
	}
	if (len) {
		write(1, buf, len);
		printf("\n");
	}
#ifdef lint
	return 0;
#else
	exit(0);
#endif
}

void
usage()
{
	Fprintf(stderr, MSGSTR(USAGE1, "usage: %s -m\n"), Prog);
	Fprintf(stderr, MSGSTR(USAGE3, "       %s -q\n"), Prog);
	exit(1);
}
