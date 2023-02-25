static char sccsid[] = "@(#)36	1.8  src/tcpip/usr/bin/ruser/ruser.c, tcprcmds, tcpip411, GOLD410 2/15/94 19:20:42";
/*
 * COMPONENT_NAME:  TCPIP
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 * ruser.c
 */                                                                   

/*
 * ruser manipulates entries in the following files:  /etc/hosts.equiv,
 * /etc/hosts.lpd, /etc/ftpusers.  it allows the user to add, delete, and 
 * show entries in these files.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "libffdb.h"

#define  EQVFILE  "/etc/hosts.equiv" /* name of the database file */
#define  LPDFILE  "/etc/hosts.lpd" /* name of the database file */
#define  FTPFILE  "/etc/ftpusers" /* name of the database file */

char progname[128];		/* = argv[0] at run time */

extern char *optarg;
extern int optind;

#include <nl_types.h>
#include "ruser_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_RUSER,n,s)

main(argc,argv)
	int argc;
	char **argv;
{
	char *phosts[128], **php = phosts;
	char *rhosts[128], **rhp = rhosts;
	char *fusers[128], **fup = fusers;
	char *rp, *ff_delete();
	int c, ret = 0;
	int aflag = 0, dflag = 0, sflag = 0, pflag = 0, rflag = 0;
	int fflag = 0, Fflag = 0, Pflag = 0, Rflag = 0, Xflag = 0;
	int Zflag = 0;
	char *incomp_msg = "incompatible command line switches";

	setlocale(LC_ALL, "");
	catd = catopen(MF_RUSER,NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, *argv);

	if (argc < 2)  usage(0);

	/* process command line args */
	while ((c = getopt(argc, argv, "adsf:p:r:FPRXZ")) != EOF) {
		switch (c) {
		case 'a':
			if (dflag || sflag || Xflag || Fflag || Pflag || Rflag || Zflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			aflag++;
			break;
		case 'd':
			if (aflag || sflag || Xflag || Fflag || Pflag || Rflag || Zflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			dflag++;
			break;
		case 's':
			if (aflag || dflag || Xflag || fflag || pflag || rflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			sflag++;
			break;
		case 'X':
			if (aflag || dflag || sflag || fflag || pflag || rflag || Zflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Xflag++;
			break;
		case 'f':
			if (!aflag && !dflag)
				usage(0);
			if (Xflag || sflag || Fflag || Pflag || Rflag || Zflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			PARSE_LIST(optarg, fup);
			fflag++;
			break;
		case 'p':
			if (!aflag && !dflag)
				usage(0);
			if (Xflag || Fflag || Pflag || Rflag || sflag || Zflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			PARSE_LIST(optarg, php);
			pflag++;
			break;
		case 'r':
			if (!aflag && !dflag)
				usage(0);
			if (Xflag || Fflag || Pflag || Rflag || sflag || Zflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			PARSE_LIST(optarg, rhp);
			rflag++;
			break;
		case 'F':
			if (!Xflag && !sflag)
				usage(0);
			if (aflag || dflag || fflag || pflag || rflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Fflag++;
			break;
		case 'P':
			if (!Xflag && !sflag)
				usage(0);
			if (aflag || dflag || fflag || pflag || rflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Pflag++;
			break;
		case 'R':
			if (!Xflag && !sflag)
				usage(0);
			if (aflag || dflag || fflag || pflag || rflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Rflag++;
			break;
		case 'Z':
			if (aflag || dflag || Xflag || fflag || pflag || rflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Zflag++;
			break;
		case '?':
			usage(0);
		default:
			usage(MSGSTR(UNK_SWITCH, "unknown switch: %c\n"), c);
		}
	}
	if (optind != argc)
		usage(MSGSTR(NOT_PARSED, "some parameters were not parsed.\n"));

	if (aflag) {
		c = 0;
		if (fflag) {
			for (fup = fusers; *fup; fup++)
				c = ff_add(FTPFILE, *fup);
		}
		if (pflag) {
			for (php = phosts; *php; php++)
				c = ff_add(LPDFILE, *php) || c;
		}
		if (rflag) {
			for (rhp = rhosts; *rhp; rhp++)
				c = ff_add(EQVFILE, *rhp) || c;
		}
		if (!fflag && !pflag && !rflag)
			usage(MSGSTR(MISSINGFLAGS, "missing flags"));
		exit(!c);
	}


	if (dflag) {
		if (fflag) {
			for (fup = fusers; *fup; fup++)
				rp = ff_delete(FTPFILE, 1, *fup, 0, 0);
			if (!rp)
				fprintf(stderr,
					MSGSTR(ERRDEL, "%s: Error deleting from %s.\n"),
					progname, FTPFILE);
		}
		if (pflag) {
			for (php = phosts; *php; php++)
				rp = ff_delete(LPDFILE, 1, *php, 0, 0) || rp;
			if (!rp)
				fprintf(stderr,
					MSGSTR(ERRDEL, "%s: Error deleting from %s.\n"),
					progname, LPDFILE);
		}
		if (rflag) {
			for (rhp = rhosts; *rhp; rhp++)
				rp = ff_delete(EQVFILE, 1, *rhp, 0, 0) || rp;
			if (!rp)
				fprintf(stderr,
					MSGSTR(ERRDEL, "%s: Error deleting from %s.\n"),
					progname, EQVFILE);
		}
		if (!fflag && !pflag && !rflag)
			usage(MSGSTR(MISSINGFLAGS, "missing flags"));
		exit((rp)?0:1);
	}


	if (Xflag) {
		rp = 0;
		if (Fflag) {	/* delete all records in FTPFILE */
			rp = ff_delete(FTPFILE, 0, 0, 0, 0);
		}
		if (Pflag) {	/* delete all records in LPDFILE */
			rp = ff_delete(LPDFILE, 0, 0, 0, 0) || rp;
		}
		if (Rflag) {	/* delete all records in EQVFILE */
			rp = ff_delete(EQVFILE, 0, 0, 0, 0) || rp;
		}
		if (!Fflag && !Pflag && !Rflag)
			usage(MSGSTR(MISSINGFLAGS, "missing flags"));
		exit((rp)?0:1);
	}


	if (sflag) {
		c = 0;
		if (Fflag) {
			if (Zflag)
				printf("#ftpusers\n");
			c = ff_show(FTPFILE, 0, 0, 0);
		} 
		if (Pflag) {
			if (Zflag)
				printf("#lpdhosts\n");
			c = ff_show(LPDFILE, 0, 0, 0) || c;
		} 
		if (Rflag) {
			if (Zflag)
				printf("#eqvhosts\n");
			c = ff_show(EQVFILE, 0, 0, 0) || c;
		}
		if (!Fflag && !Pflag && !Rflag)
			usage(MSGSTR(MISSINGFLAGS, "missing flags"));
		exit(!c);
	}

	exit(0);
}


/*
 * usage()
 *
 * prints the usage message, plus optional parameters, then exits.
 *
 */
usage(a, b, c, d)
    char *a, *b, *c, *d;
{
    if (a)
	fprintf(stderr, a, b, c, d);
    fprintf(stderr,
	    MSGSTR(USAGE1, "\nusage:	%s  -a  -f \"username ...\" | -p \"hostname ...\" | -r \"hostname ...\"\n"),
	    progname);
    fprintf(stderr,
	    MSGSTR(USAGE2, "	%s  -d  -f \"username ...\" | -p \"hostname ...\" | -r \"hostname ...\"\n"),
	    progname);
    fprintf(stderr,
	    MSGSTR(USAGE3, "	%s  -X  {-F | -P | -R }\n"), progname);
    fprintf(stderr,
	    MSGSTR(USAGE4, "	%s  -s  {-F | -P | -R } [-Z]\n"), progname);
    exit(2);
}
