static char sccsid[] = "@(#)98	1.18.1.6  src/bos/usr/sbin/rrestore/main.c, cmdarch, bos411, 9428A410j 2/23/94 09:59:51";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

/*
#ifndef lint
static char sccsid[] = "(#)main.c	5.3 (Berkeley) 4/23/86";
#endif not lint
*/

/*
 *	Modified to recursively extract all files within a subtree
 *	(supressed by the h option) and recreate the heirarchical
 *	structure of that subtree and move extracted files to their
 *	proper homes (supressed by the m option).
 *	Includes the s (skip files) option for use with multiple
 *	dumps on a single tape.
 *	8/29/80		by Mike Litzkow
 *
 *	Modified to work on the new file system and to recover from
 *	tape read errors.
 *	1/19/82		by Kirk McKusick
 *
 *	Full incremental restore running entirely in user code and
 *	interactive tape browser.
 *	1/19/83		by Kirk McKusick
 */

#include "restore.h"
#include <signal.h>
#include <sys/audit.h>
#include <sys/priv.h>
#include <locale.h>

int	bflag = 0, cvtflag = 0, dflag = 0, vflag = 0, yflag = 0;
int	hflag = 1, mflag = 1;
int	context_saved = 0;
char	command = '\0';
long	dumpnum = 1;
long	volno = 0;
long	ntrec;
char	*dumpmap;
char	*clrimap;
ino_t	maxino;
time_t	dumptime;
time_t	dumpdate;
FILE 	*terminal;

/* Defined in interactiv.c : does wildcard processing */
extern void process_fnames();

main(argc, argv)
	int argc;
	char *argv[];
{
#ifdef RRESTORE
	register char *cp;
	int berkeley;
	char *inputdev = "";
#else
	char *inputdev = "/dev/rfd0";
#endif
	ino_t ino;
	char *symtbl = "./restoresymtable";
	char name[MAXPATHLEN];
	extern int onintr(void);
	int 		c;
	extern int	optind;
	extern char	*optarg;

	(void) setlocale(LC_ALL,"");
	catd = catopen("restore2.cat", NL_CAT_LOCALE);
	/* suspend implicit auditing */
/************************************************************
	auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
*************************************************************/

	if (signal(SIGINT, (void (*)(int))onintr) == SIG_IGN)
		(void) signal(SIGINT, SIG_IGN);
	if (signal(SIGTERM, (void (*)(int))onintr) == SIG_IGN)
		(void) signal(SIGTERM, SIG_IGN);
	setlinebuf(stderr);
	if (argc < 2) {
usage:
		fprintf(stderr, MSGSTR(USAGE, "Usage:\n"));
#ifdef RRESTORE
		fprintf(stderr, "\trrestore ");
		fprintf(stderr, MSGSTR(RRUSAGE1, "-t[Dhvy] -f Host:Device [-s Number] [-b Number] [File ...]\n"));
		fprintf(stderr, "\trrestore ");
		fprintf(stderr, MSGSTR(RRUSAGE2, "-x[Dhmvy] -f Host:Device [-s Number] [-b Number] [File ...]\n"));
		fprintf(stderr, "\trrestore ");
		fprintf(stderr, MSGSTR(RRUSAGE3, "-i[Dhmvy] -f Host:Device [-s Number] [-b Number]\n"));
		fprintf(stderr, "\trrestore ");
		fprintf(stderr, MSGSTR(RRUSAGE4, "-r[Dvy] -f Host:Device [-s Number] [-b Number]\n"));
		fprintf(stderr, "\trrestore ");
		fprintf(stderr, MSGSTR(RRUSAGE5, "-R[Dvy] -f Host:Device [-s Number] [-b Number]\n"));
#else
		fprintf(stderr, "\trestore ");
		fprintf(stderr, MSGSTR(USAGE1, "-t[Dhvy] [-f Device] [-s Number] [-b Number] [File ...]\n"));
		fprintf(stderr, "\trestore ");
		fprintf(stderr, MSGSTR(USAGE2, "-x[Dhmvy] [-f Device] [-s Number] [-b Number] [File ...]\n"));
		fprintf(stderr, "\trestore ");
		fprintf(stderr, MSGSTR(USAGE3, "-i[Dhmvy] [-f Device] [-s Number] [-b Number]\n"));
		fprintf(stderr, "\trestore ");
		fprintf(stderr, MSGSTR(USAGE4, "-r[Dvy] [-f Device] [-s Number] [-b Number]\n"));
		fprintf(stderr, "\trestore ");
		fprintf(stderr, MSGSTR(USAGE5, "-R[Dvy] [-f Device] [-s Number] [-b Number]\n"));
#endif
		done(1);
	}
#ifdef RRESTORE
	berkeley = 0;
	if (argv[1][0] != '-') {
		berkeley = 1;
		argv++;
		argc -= 2;
		command = '\0';
		for (cp = *argv++; *cp; cp++) {
			switch (*cp) {
			case '-':
				break;
			case 'D':
				dflag++;
				break;
			case 'h':
				hflag = 0;
				break;
			case 'm':
				mflag = 0;
				break;
			case 'v':
				vflag++;
				break;
			case 'y':
				yflag++;
				break;
			case 'f':
				if (argc < 1) {
					fprintf(stderr, MSGSTR(MISSDEV, "missing device specifier\n"));
					done(1);
				}
				inputdev = *argv++;
				argc--;
				break;
			case 'b':
				/*
				 * change default tape blocksize
				 */
				bflag++;
				if (argc < 1) {
					fprintf(stderr, MSGSTR(MISSBLK, "missing block size\n"));
					done(1);
				}
				ntrec = atoi(*argv++);
				if (ntrec <= 0) {
					fprintf(stderr, MSGSTR(BLKPOS, "Block size must be a positive integer\n"));
					done(1);
				}
				argc--;
				break;
			case 's':
				/*
				 * dumpnum (skip to) for multifile dump tapes
				 */
				if (argc < 1) {
					fprintf(stderr, MSGSTR(MISSDUMP, "missing dump number\n"));
					done(1);
				}
				dumpnum = atoi(*argv++);
				if (dumpnum <= 0) {
					fprintf(stderr, MSGSTR(DUMPPOS, "Dump number must be a positive integer\n"));
					done(1);
				}
				argc--;
				break;
			case 't':
			case 'R':
			case 'r':
			case 'x':
			case 'i':
				if (command != '\0' && command != *cp) {
					fprintf(stderr,
						MSGSTR(BADOPTM, "%c and %c are mutually exclusive\n"),
						*cp, command);
					goto usage;
				}
				command = *cp;
				break;
			default:
				fprintf(stderr, MSGSTR(BADKEY, "Bad key character %c\n"), *cp);
				goto usage;
			}
		}
		if (command == '\0') {
			fprintf(stderr, MSGSTR(MUSTITR, "must specify i, t, r, R, or x\n"));
			goto usage;
		}
		setinput(inputdev);
		if (argc == 0) {
			argc = 1;
			*--argv = ".";
		}
	}
	else {
#endif
	command = '\0';
	while ((c = getopt(argc, argv, "?dDhmyvf:b:s:trRxi")) != EOF)
		switch (c) {
		case 'D':
			dflag++;
			break;
		case 'h':
			hflag = 0;
			break;
		case 'm':
			mflag = 0;
			break;
		case 'v':
			vflag++;
			break;
		case 'y':
			yflag++;
			break;
		case 'f':
			if (argc < 1) {
				fprintf(stderr, MSGSTR(MISSDEV, "missing device specifier\n"));
				done(1);
			}
			inputdev = optarg;
			break;
		case 'b':
			/*
			 * change default tape blocksize
			 */
			bflag++;
			if (argc < 1) {
				fprintf(stderr, MSGSTR(MISSBLK, "missing block size\n"));
				done(1);
			}
			ntrec = atoi(optarg);
			if (ntrec <= 0) {
				fprintf(stderr, MSGSTR(BLKPOS, "Block size must be a positive integer\n"));
				done(1);
			}
			break;
		case 's':
			/*
			 * dumpnum (skip to) for multifile dump tapes
			 */
			if (argc < 1) {
				fprintf(stderr, MSGSTR(MISSDUMP, "missing dump number\n"));
				done(1);
			}
			dumpnum = atoi(optarg);
			if (dumpnum <= 0) {
				fprintf(stderr, MSGSTR(DUMPPOS, "Dump number must be a positive integer\n"));
				done(1);
			}
			break;
		case 't':
		case 'R':
		case 'r':
		case 'x':
		case 'i':
			if (command != '\0' && command != c) {
				fprintf(stderr,
					MSGSTR(BADOPTM, "%c and %c are mutually exclusive\n"),
					c, command);
				goto usage;
			}
			command = c;
			break;
		case '?':
			goto usage;
		default:
			fprintf(stderr, MSGSTR(BADKEY, "Bad key character %c\n"), c);
			goto usage;
		}
	if (command == '\0') {
		fprintf(stderr, MSGSTR(MUSTITR, "must specify i, t, r, R, or x\n"));
		goto usage;
	}
	setinput(inputdev);
	if (argv[optind] == NULL)
		argv[--optind] = ".";
#ifdef RRESTORE
	}
#endif
	switch (command) {
	/*
	 * Interactive mode.
	 */
	case 'i':
		setup();
		extractdirs(1);
		initsymtable((char *)0);
		runcmdshell();
		done(0);
	/*
	 * Incremental restoration of a file system.
	 */
	case 'r':
		setup();
		if (dumptime > 0) {
			/*
			 * This is an incremental dump tape.
			 */
			vprintf(stdout, MSGSTR(BEGINIC, "Begin incremental restore\n"));
			initsymtable(symtbl);
			extractdirs(1);
			removeoldleaves();
			vprintf(stdout, MSGSTR(CALNODE, "Calculate node updates.\n"));
			treescan(".", ROOTINO, nodeupdates);
			findunreflinks();
			removeoldnodes();
		} else {
			/*
			 * This is a level zero dump tape.
			 */
			vprintf(stdout, MSGSTR(BEGINLE0, "Begin level 0 restore\n"));
			initsymtable((char *)0);
			extractdirs(1);
			vprintf(stdout, MSGSTR(CALEXTL, "Calculate extraction list.\n"));
			treescan(".", ROOTINO, nodeupdates);
		}
		createleaves(symtbl);
		createlinks();
		setdirmodes();
		checkrestore();
		if (dflag) {
			vprintf(stdout, MSGSTR(VERIFY, "Verify the directory structure\n"));
			treescan(".", ROOTINO, verifyfile);
		}
		dumpsymtable(symtbl, (long)1);
		done(0);
	/*
	 * Resume an incremental file system restoration.
	 */
	case 'R':
		initsymtable(symtbl);
		skipmaps();
		skipdirs();
		createleaves(symtbl);
		createlinks();
		setdirmodes();
		checkrestore();
		dumpsymtable(symtbl, (long)1);
		done(0);
	/*
	 * List contents of tape.
	 */
	case 't':
		setup();
		extractdirs(0);
		initsymtable((char *)0);
#ifdef RRESTORE
		if (berkeley) {
			while (argc--) 
				(void) process_fnames(*argv++, name, 0, listfile);
		
		} else {
#endif
		while (argv[optind] != NULL) 
				(void) process_fnames(argv[optind++], name, 0, listfile);
#ifdef RRESTORE
		}
#endif
		done(0);
	/*
	 * Batch extraction of tape contents.
	 */
	case 'x':
		setup();
		extractdirs(1);
		initsymtable((char *)0);
#ifdef RRESTORE
		if (berkeley) {
			while (argc--) 
			(void) process_fnames(*argv++, name, mflag, addfile);
		} else {
#endif
		while (argv[optind] != NULL) 
			(void) process_fnames(argv[optind++], name, mflag, addfile);
#ifdef RRESTORE
		}
#endif
		createfiles();
		createlinks();
		setdirmodes();
		if (dflag)
			checkrestore();
		done(0);
	}
}
