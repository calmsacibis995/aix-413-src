static char sccsid[] = "@(#)08        1.4.1.3  src/bos/usr/sbin/strconf/strconf.c, cmdpse, bos411, 9428A410j 5/10/94 19:09:22";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   FUNCTIONS: MSGSTR
 *		die
 *		main
 *		usage
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * strconf - query stream configuration of stream on stdin
 *
 * from man page in SVR4 SPG
 *
 * 11/01/90 dgb
 *
 * SYNOPSIS:
 *	strconf [-t | -m <module>]
 *	<no args>	print names of all modules
 *	-t		print name of topmost module
 *	-m <module>	print "yes" if module <module> is on stream, else "no"
 *	-?		print a usage message
 *
 * HISTORY:
 *	11/01/90 dgb	original version
 *	11/12/90 dgb	determines if stdin is a stream before using
 *			lint considerations
 *			more intelligent error messages
 *	11/20/90 dgb	ported to AIX3
 */

#include <sys/types.h>
#include <sys/stropts.h>
#include <errno.h>
#include <stdio.h>

#include "strconf_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRCONF,n,s)

void die(), usage();
extern char *strrchr(), *memset(), *malloc();
extern void exit();

#define	Fprintf	(void)fprintf
#define	Sprintf	(void)sprintf
#define	Printf	(void)printf
#define	Memset	(void)memset

char *Prog;
extern int errno;

main(argc, argv)
	int argc;
	char **argv;
{
	int c;
	int fd;
	int toponly = 0;
	int retcode = 0;
	char *module = NULL;

	extern int optind;
	extern char *optarg;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRCONF, NL_CAT_LOCALE);

	if (Prog = strrchr(argv[0], '/'))
		argv[0] = ++Prog;	/* for getopt() */
	else
		Prog = argv[0];
	while ((c = getopt(argc, argv, "?tm:")) != EOF) {
		switch (c) {
		case 't':	toponly = 1; break;
		case 'm':	module = optarg; break;
		case '?':	/* fallthru */
		default:
			usage();
		}
	}
	if (toponly && module)
		die(MSGSTR(MSG1, "-t and -m are mutually exclusive"));
	argc -= optind;
	argv += optind;

	if (argc)
		usage();

	fd = fileno(stdin);

	switch (isastream(fd)) {
	case 0:		errno = ENOSTR; /* fallthrough */
	case -1:	die(MSGSTR(MSG2, "stdin"));
	}

	if (toponly) {
		char mname[FMNAMESZ+1];

		Memset(mname, '\0', sizeof(mname));
		if (ioctl(fd, I_LOOK, mname) < 0) {
			if (errno == EINVAL) {
				errno = 0;
				die(MSGSTR(MSG3, "no modules on stream"));
			}
			die(MSGSTR(MSG4, "ioctl I_LOOK"));
		}
		Printf("%s\n", mname);

	} else if (module) {
		int retcode = 0;

		if (ioctl(fd, I_FIND, module) == 1) {
			Printf(MSGSTR(MSG6, "yes\n"));
		} else {
			Printf(MSGSTR(MSG5, "no\n"));
			retcode = 1;
		}
		/* strconf -m x must exit after yes or no reply */
		exit(retcode);

	} else {
		int nmods;
		unsigned size;
		struct str_list strlist;
		struct str_mlist *p;

		if ((nmods = ioctl(fd, I_LIST, (char *)0)) < 0)
			die(MSGSTR(MSG10, "cannot determine number of modules"));
		size = nmods * sizeof(struct str_mlist);
		strlist.sl_nmods = nmods;
		strlist.sl_modlist = (struct str_mlist *)malloc(size);
		Memset(strlist.sl_modlist, '\0', (int)size);
		if ((nmods = ioctl(fd, I_LIST, (char *)&strlist)) < 0)
			die(MSGSTR(MSG11, "cannot get names of modules"));
		if (nmods != strlist.sl_nmods) {
			Fprintf(stderr,
				MSGSTR(MSG12, "%s: %d modules in stream, only %d reported"),
				Prog, strlist.sl_nmods, nmods);
			retcode = 1;
		}
		p = strlist.sl_modlist;
		while (nmods-- > 0)
			Printf("%s\n", (p++)->l_name);
	}
	exit(retcode);
	/* NOTREACHED */
}

void
usage()
{
	Fprintf(stderr, MSGSTR(USAGE11, "usage: %s [-t | -m <module>]\n"), Prog);
	exit(1);
}

/* VARARGS */
void
die(msg, a, b, c)
	char *msg;
	int a,b,c;
{
	char buf[512];

	Sprintf(buf, "%s: ", Prog);
	Sprintf(buf+strlen(buf), msg, a, b, c);
	if (errno != 0)
		perror(buf);
	else
		Fprintf(stderr, "%s\n", buf);
	exit(1);
}
