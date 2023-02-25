static char sccsid[] = "@(#)28  1.13  src/bos/usr/sbin/catman/catman.c, cmdman, bos41B, 9506A 1/30/95 07:53:18";
/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: doit, whatis, usage
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
 * NAME: catman [ -n | -p | -w ] [-M path] [sections]
 *
 * FUNCTION: 
 *	The catman command creates the preformatted versions of the on-line
 *	manual from the nroff input files.  Each manual page is examined and
 * 	those whose preformatted versions are missing or out of date are 
 *	recreated. If any changes are made, catman will recreate the whatis
 *  	database.
 * FLAGS:
 *	-M	updates manual pages located in the set of directories
 *		specified by path.
 *	-n	prevents creations of the whatis database.
 *	-p 	prints what would be done instead of doing it.
 *	-w	causes only the whatis database to be created. No manual
 *		reformatting is done.
 */ 
#define _ILS_MACROS
#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/dir.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#include "catman_msg.h"
static nl_catd catd;
#define MSGSTR(n, s) catgets(catd, MS_CATMAN, n, s)
extern	mkwhatis_krs();
static char	buf[BUFSIZ];
static int	pflag = 0;
static int	nflag = 0;
static int	wflag = 0;
static int	mypid;
static char	man[MAXNAMLEN+6] = "manx/";
static int	exstat = 0;
static char	cat[MAXNAMLEN+6] = "catx/";
static char	lncat[MAXNAMLEN+9] = "../catx/";
char	*manpath = "/usr/share/man";
static char	*sections = "CLFnlpo12345678";
static char	*makewhatis = "/usr/lbin/mkwhatis";
char	*index(), *rindex();

main(int argc, char **argv)
{
	char *mp, *nextp;
	int c;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CATMAN, NL_CAT_LOCALE);

	if ((mp = getenv("MANPATH")) != NULL)
		manpath = mp;

	while((c = getopt(argc, argv, "npwM:")) != EOF)
		switch (c) {
		case 'n':
			nflag = (pflag || wflag) ? usage() : 1;
			break;
		case 'p':
			pflag = (nflag || wflag) ? usage() : 1;
			break;
		case 'w':
			wflag = (nflag || pflag) ? usage() : 1;
			break;
		case 'M':
			manpath = optarg;
			break;
		default:
			usage();
			break;
		}
	if ((c = argc - optind) > 1) 
		usage();
	if (c == 1)
		sections = *argv;
	for (mp = manpath; mp && ((nextp = index(mp, ':')), 1); mp = nextp) {
		if (nextp)
			*nextp++ = '\0';
		doit(mp);
	}
	exit(exstat);
}

/*
 * NAME: doit
 *
 * FUNCTION: for each man file in each manual section , performs a nroff 
 *	     fuuction on it.  The resulting files are cat files.
 *
 */
static doit(char *mandir)
{
	register char *msp, *csp, *sp;
	int changed = 0;
	int status;

	if (wflag)
		goto whatis;

	if (chdir(mandir) < 0) {
		fprintf(stderr, "catman: %s: %s\n", mandir, strerror(errno));
		return;
	}
	if (pflag)
		printf("cd %s\n", mandir);
	msp = &man[5];
	csp = &cat[5];
	(void) umask((mode_t)0);

/* looks in a list of manual sections */
	for (sp = sections; *sp; sp++) {
		register DIR *mdir;
		register struct dirent *dir;
		struct stat sbuf;

		man[3] = cat[3] = *sp;
		*msp = *csp = '\0';

/* opens a man directory, makes a cat directory and checks its accessing modes */ 
		if ((mdir = opendir(man)) == NULL) {
			fprintf(stderr, "catman: %s: %s\n", man, strerror(errno));
			continue;
		}
		if (stat(cat, &sbuf) < 0) {
			register char *cp;

			(void) strcpy(buf, cat);
		        cp = rindex(buf, '/');
			if (cp && cp[1] == '\0')
				*cp = '\0';
			if (pflag)
				printf("mkdir %s\n", buf);
			else if (mkdir(buf, (mode_t)0777) < 0) {
				fprintf(stderr, "catman: mkdir: %s\n", cat, strerror(errno));
				exstat = 1;
				continue;
			}
			(void) stat(cat, &sbuf);
		}
		if (access(cat, R_OK|W_OK|X_OK) == -1) {
			fprintf(stderr, "catman: %s\n", cat, strerror(errno));
			exstat = 1;
			continue;
		}
		if ((sbuf.st_mode & S_IFMT) != S_IFDIR) {
			fprintf(stderr, MSGSTR(NOTDIR, "catman: %s: Not a directory\n"), cat);
			exstat = 1;
			continue;
		}

/* reads in each file in one manual section and creates the cat file for it */
		while ((dir = readdir(mdir)) != NULL) {
			time_t time;
			register char *tsp;
			FILE *inf;
			int  makelink;

			if ((dir->d_ino == 0) || (0 == strcmp(dir->d_name, ".")) || (0 == strcmp(dir->d_name, "..")))
				continue;
			/*
			 * Make sure this is a man file, i.e., that it
			 * ends in .[0-9] or .[0-9][a-z]
			 */
			tsp = rindex(dir->d_name, '.');
			if (tsp == NULL)
				continue;
			tsp++;
			if (!isdigit((int)*tsp) && *tsp != *sp)
				continue;
			if (*++tsp && !isalpha((int)*tsp))
				continue;
			if (*tsp && *++tsp)
				continue;
			(void) strcpy(msp, dir->d_name);
			if ((inf = fopen(man, "r")) == NULL) {
				sprintf(buf, "catman: %s", man);
				perror(buf);
				exstat = 1;
				continue;
			}
			makelink = 0;
			if (getc(inf) == '.' && getc(inf) == 's'
			    && getc(inf) == 'o') {
				if (getc(inf) != ' ' ||
				    fgets(lncat+3, (int)(sizeof(lncat)-3), inf)==NULL) {
					fclose(inf);
					continue;
				}
				if (lncat[strlen(lncat)-1] == '\n')
					lncat[strlen(lncat)-1] = '\0';
				if (strncmp(lncat+3, "man", 3) != 0) {
					fclose(inf);
					continue;
				}
				bcopy("../cat", lncat, sizeof("../cat")-1);
				makelink = 1;
			}
			fclose(inf);

			(void) strcpy(csp, dir->d_name);
			if (stat(cat, &sbuf) >= 0) {
				time = sbuf.st_mtime;
				(void) stat(man, &sbuf);
				if (time >= sbuf.st_mtime)
					continue;
				(void) unlink(cat);
			}
			if (makelink) {
				/*
				 * Don't unlink a directory by accident.
				 */
				if (stat(lncat+3, &sbuf) >= 0 &&
				    (((sbuf.st_mode&S_IFMT)==S_IFREG) ||
				     ((sbuf.st_mode&S_IFMT)==S_IFLNK)))
					(void) unlink(cat);
				if (pflag)
					printf("ln -s %s %s\n", lncat, cat);
				else
					if (symlink(lncat, cat) == -1) {
						fprintf(stderr, "catman: symlink: %s\n", cat, strerror(errno));
						exstat = 1;
						continue;
					}
			}
			else {
				sprintf(buf, "nroff -man %s > %s", man, cat);
				if (pflag)
					printf("%s\n", buf);
				else if ((status = system(buf)) != 0) {
					fprintf(stderr, MSGSTR(BADMAN, "catman: nroff: %s: exit status %d: Owooooo!\n")
					, cat, status);
					exstat = 1;
					continue;
				}
			}
			changed = 1;
		}
		closedir(mdir);
	}

/* updates the whatis database */
	if (changed && !nflag) {
whatis:
		sprintf(buf, "%s %s", makewhatis, mandir);
		if (pflag)
			printf("%s\n", buf);
		else if ((status = system(buf)) != 0) {
			fprintf(stderr, MSGSTR(STATUS, "catman: %s: exit status %d\n"),
			buf, status);
			exstat = 1;
			if (access(mandir, R_OK|W_OK|X_OK) == -1) {
				perror(mandir);
				exstat = 1;
			} else
				mkwhatis_krs();
                }
	}
	return;
}

static usage()
{
	fprintf(stderr,MSGSTR(USAGE,"usage: catman [-n | -p | -w] [-M Path] [Sections]\n"));
	exit(1);
}
