static char sccsid[] = "@(#)94	1.16  src/bos/usr/sbin/flcopy/flcopy.c, cmdarch, bos411, 9428A410j 6/27/94 15:58:45";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: flcopy
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
char copyright[] =
"(#) Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "(#)flcopy.c	5.1 (Berkeley) 6/6/86";
#endif not lint
*/

#include <locale.h>
#include <nl_types.h>
#include "flcopy_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_FLCOPY,N,S)
nl_catd catd;

#include <sys/file.h>
#include <fcntl.h>

#define EXITOK  0
#define EXITBAD 1

int	floppydes;
char	*flopname = "/dev/rfd0";
/* These calculations are based on the high density diskettes. */
#ifdef _POWER
long	dsize = 160 * 18 * 512;
long	tracksize = 18 * 512;
#endif
#ifdef _IBMRT
long	dsize = 160 * 15 * 512;
long	tracksize = 15 * 512;
#endif
int	hflag;
int	rflag;
int	tflag;

main(argc, argv)
	register char **argv;
{
	static char buff[18 * 512];
	register long count;
	register int n, file;
	register char *cp;

	(void) setlocale(LC_ALL,"");
	catd = catopen("flcopy.cat", NL_CAT_LOCALE);
	while ((cp = *++argv), --argc > 0) {
		while (*cp) {
			switch(*cp++) {

			case '-':
				continue;

			case 'h':
				hflag++;
				continue;

			case 'f':
				if (argc <= 1) {
					printf(MSGSTR(MISSF, "flcopy: -f: missing file name\n"));
					usage(EXITBAD);
				}
				flopname = *++argv;
				argc--;
				break;

			case 't':
				tflag++;
				if (*cp >= '0' && *cp <= '9')
					dsize = atoi(cp);
				else if (argc > 1) {
					dsize = atoi(*++argv);
					argc--;
				} else
					dsize = 160;
				if (dsize <= 0 || dsize > 160) {
					printf(MSGSTR(BADT, "Bad number of tracks\n"));
					exit(2);
				}
#ifdef _POWER
				dsize *= 18 * 512;
#endif
#ifdef _IBMRT
				dsize *= 15 * 512;
#endif
				break;

			case 'r':
				rflag++;
				continue;

			case '?':
				usage(EXITOK);

			default :
				usage(EXITBAD);
			}
			break;
		}
	}
	if (hflag && rflag)
		usage(EXITBAD);
	if (!hflag) {
		file = open("floppy", O_RDWR|O_CREAT|O_TRUNC, 0666);
		if (file < 0) {
			printf(MSGSTR(CANTOP, "can't open \"floppy\"\n"));
			exit(1);
		}
		open_flop(O_RDONLY); /* read the diskette */
		if (tflag) {
			for (count = dsize; count > 0 ; count -= tracksize) {
				n = count > tracksize ? tracksize : count;
				if (read(floppydes, buff, n) < 0) {
					perror("read");
					exit(1);
				}
				if (write(file, buff, n) < 0) {
					perror("write");
					exit(1);
				}
			}
		} else {
			while ((n = read(floppydes, buff, tracksize)) > 0)
				if (write(file, buff, n) < 0) {
					perror("write");
					exit(1);
				}
			if (n < 0) {
				perror("read");
				exit(1);
			}
		}
	} else{
		printf(MSGSTR(HALFT, "Halftime!\n"));
		if ((file = open("floppy", O_RDONLY)) < 0) {
			printf(MSGSTR(CANTOP, "can't open \"floppy\"\n"));
			exit(1);
		}
	}
	if (rflag)
		exit(0);
	if (!hflag)
		printf(MSGSTR(CHANGEF, "Change Floppy, Hit return when done.\n"));
	close(floppydes);
	gets(buff);
	lseek(file, 0L, 0);
	count = dsize;
	open_flop(O_RDWR);	/* read/write to the diskette */
	if (tflag) {
		for (count = dsize; count > 0 ; count -= tracksize) {
			n = count > tracksize ? tracksize : count;
			if (read(file, buff, n) < 0) {
				perror("read");
				exit(1);
			}
			if (write(floppydes, buff, n) < 0) {
				perror("write");
				exit(1);
			}
		}
	} else {
		while ((n = read(file, buff, tracksize)) > 0)
			if (write(floppydes, buff, n) < 0) {
				perror("write");
				exit(1);
			}
		if (n < 0) {
			perror("read");
			exit(1);
		}
	}
	catclose(catd);
	exit(0);
}

/*
 * open floppy for either read or read/write access
 */

open_flop(mode)
int mode;
{

	if (rflag)
		mode = O_RDONLY;
	if ((floppydes = open(flopname, mode)) < 0) {
		printf(MSGSTR(FLPPYO, "Floppy open failed\n"));
		exit(1);
	}
}

usage(exitvalue)
int exitvalue;
{
	printf(MSGSTR(USAGE,"Usage: flcopy [-h|-r] [-t Tracks] [-f Filename]\n"));
	exit(exitvalue);
}
