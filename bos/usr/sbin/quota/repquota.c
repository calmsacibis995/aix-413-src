static char sccsid[] = "@(#)32	1.7.1.6  src/bos/usr/sbin/quota/repquota.c, cmdquota, bos41J, 9516A_all 4/17/95 09:20:41";
/*
 * COMPONENT_NAME: CMDQUOTA
 *
 * FUNCTIONS: 	main, usage, repquota, lookup,
 *		addid, timeprt
 *
 * ORIGINS: 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980, 1990 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
 *
 * Berkeley sccsid = "@(#)repquota.c	5.10 (Berkeley) 6/1/90";
 *
 *
 * Quota report
 * 
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <jfs/quota.h>
#include "fstab.h"
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <ctype.h>

#include "repquota_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_REPQUOTA,n,s)


struct fileusage {
	struct	fileusage *fu_next;
	struct  fileusage *fu_sorted;
	struct	dqblk fu_dqblk;
	uid_t	fu_id;
	char	fu_name[1];	/* actually longer since this structure is   */
                                /* malloc'd storage, and this field is of    */
                                /* variable length                           */
};
#define FUHASH 4096	/* must be power of two */
struct fileusage *fuhead[MAXQUOTAS][FUHASH];
struct fileusage *fuSortedHead[MAXQUOTAS];

int	vflag;			/* verbose */
int	aflag;			/* all file systems */

/* 
 * Function prototypes
 */
void   usage	 ();
int    repquota  (register struct fstab *, int, char *);
struct fileusage *lookup(long, int);
struct fileusage *addid(ulong, int, char *);
char   *timeprt  (time_t);


main(int argc, char **argv)
{
	register struct fstab *fs;
	register struct passwd *pw;
	register struct group *gr;
	int gflag = 0, uflag = 0, errs = 0;
	long i, argnum, done = 0;
	extern char *optarg;
	extern int optind;
	char ch, *qfnp;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_REPQUOTA,NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "aguv")) != (char) EOF) {
		switch(ch) {
		case 'a':
			aflag++;
			break;
		case 'g':
			gflag++;
			break;
		case 'u':
			uflag++;
			break;
		case 'v':
			vflag++;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0 && !aflag)
		usage();
	if (!gflag && !uflag) {
		if (aflag)
			gflag++;
		uflag++;
	}
	if (gflag) {
		setgrent();
		while ((gr = getgrent()) != 0)
			(void) addid(gr->gr_gid, GRPQUOTA, gr->gr_name);
		endgrent();
	}
	if (uflag) {
		setpwent();
		while ((pw = _getpwent()) != 0)
			(void) addid(pw->pw_uid, USRQUOTA, pw->pw_name);
		endpwent();
	}
	setfsent();
	while ((fs = getfsent()) != NULL) {
		if (strcmp(fs->fs_vfstype, "jfs"))
			continue;
		if (aflag) {
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += repquota(fs, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += repquota(fs, USRQUOTA, qfnp);
			continue;
		}
		if ((argnum = oneof(fs->fs_file, argv, argc)) >= 0 ||
		    (argnum = oneof(fs->fs_spec, argv, argc)) >= 0) {
			done |= 1 << argnum;
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += repquota(fs, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += repquota(fs, USRQUOTA, qfnp);
		}
	}
	endfsent();
	for (i = 0; i < argc; i++)
		if ((done & (1 << i)) == 0)
			fprintf(stderr, MSGSTR(NOFSTAB, "%s not found in /etc/filesystems\n"), argv[i]);
	exit(errs);
}

void
usage()
{
	fprintf(stderr, "Usage:\n\t%s\n\t%s\n",
		MSGSTR(USE1, "repquota [-v] [-g] [-u] -a"),
		MSGSTR(USE2, "repquota [-v] [-g] [-u] filesystem"));
	exit(1);
}

int
repquota(register struct fstab *fs, int type, char *qfpathname)
{
	register struct fileusage *fup;
	FILE *qf;
	ulong id;
	struct dqblk dqbuf;
	static struct dqblk zerodqblk;
	static int warned = 0;
	static int multiple = 0;
	extern int errno;

	if (quotactl(fs->fs_file, QCMD(Q_SYNC, type), 0, 0) < 0 &&
	    errno == EOPNOTSUPP && !warned && vflag) {
		warned++;
		fprintf(stdout,
		    MSGSTR(NOQUOTA, "*** Warning: Quotas are not enabled on this machine\n"));
	}
	if (multiple++)
		printf("\n");
	if (vflag)
		fprintf(stdout, MSGSTR(REPORT, "*** Report for %s quotas on %s (%s)\n"),
		    qfextension[type], fs->fs_file, fs->fs_spec);

	qf = fopen(qfpathname, "r");
	if (qf == NULL) {
		perror(qfpathname);
		return (1);
	}
	for (id = 0; ; id++) {
		fread(&dqbuf, sizeof(struct dqblk), 1, qf);
		if (feof(qf))
			break;
		if (dqbuf.dqb_ihardlimit == 0 && dqbuf.dqb_bhardlimit == 0 &&
                    dqbuf.dqb_isoftlimit == 0 && dqbuf.dqb_bsoftlimit == 0 &&
                    dqbuf.dqb_curinodes == 0 && dqbuf.dqb_curblocks == 0)
			continue;
                /*
                 * quotactl() will round up my block count to the next
                 * 1024 block if I have a partial block.  However the
                 * quota files have the true number of 1024 blocks plus
                 * an overloaded top order bit of dqb_btime if there is
                 * a fragment.  I want to give the exact same number of
                 * blocks no matter where I get the info from.
                 */
                if (DQCARRY(dqbuf.dqb_btime))
                        dqbuf.dqb_curblocks++;

		if ((fup = lookup(id, type)) == 0)
			fup = addid(id, type, (char *)0);
		fup->fu_dqblk = dqbuf;
	}
	fclose(qf);
	printf(MSGSTR(LIMITS, "                        Block limits               File limits\n"));
	printf(MSGSTR(MSGHD, "User            used    soft    hard  grace    used  soft  hard  grace\n"));


	for (fup = fuSortedHead[type] ; fup ; fup = fup->fu_sorted)
	{
		if (fup->fu_dqblk.dqb_ihardlimit == 0 &&
		    fup->fu_dqblk.dqb_bhardlimit == 0 &&
                    fup->fu_dqblk.dqb_isoftlimit == 0 &&
                    fup->fu_dqblk.dqb_bsoftlimit == 0 &&
                    fup->fu_dqblk.dqb_curinodes == 0 &&
                    fup->fu_dqblk.dqb_curblocks == 0)
			continue;
		printf("%-10s", fup->fu_name);
		printf("%c%c%8d%8d%8d%7s",
			fup->fu_dqblk.dqb_bsoftlimit && 
			    fup->fu_dqblk.dqb_curblocks >= 
			    fup->fu_dqblk.dqb_bsoftlimit ? '+' : '-',
			fup->fu_dqblk.dqb_isoftlimit &&
			    fup->fu_dqblk.dqb_curinodes >=
			    fup->fu_dqblk.dqb_isoftlimit ? '+' : '-',
			fup->fu_dqblk.dqb_curblocks,
			fup->fu_dqblk.dqb_bsoftlimit,
			fup->fu_dqblk.dqb_bhardlimit,
			fup->fu_dqblk.dqb_bsoftlimit && 
			    fup->fu_dqblk.dqb_curblocks >= 
			    fup->fu_dqblk.dqb_bsoftlimit ?
			    timeprt(DQBTIME(fup->fu_dqblk.dqb_btime)) : "");
		printf("  %6d%6d%6d%7s\n",
			fup->fu_dqblk.dqb_curinodes,
			fup->fu_dqblk.dqb_isoftlimit,
			fup->fu_dqblk.dqb_ihardlimit,
			fup->fu_dqblk.dqb_isoftlimit &&
			    fup->fu_dqblk.dqb_curinodes >=
			    fup->fu_dqblk.dqb_isoftlimit ?
			    timeprt(fup->fu_dqblk.dqb_itime) : "");
		fup->fu_dqblk = zerodqblk;
	}
	return (0);
}

/*
 * Routines to manage the file usage table.
 *
 * Lookup an id of a specific type.
 */
struct fileusage *
lookup(long id, int type)
{
	register struct fileusage *fup;

	for (fup = fuhead[type][id & (FUHASH-1)]; fup != 0; fup = fup->fu_next)
		if (fup->fu_id == id)
			return (fup);
	return ((struct fileusage *)0);
}

/*
 * Add a new file usage id if it does not already exist.
 */
struct fileusage *
addid(ulong id, int type, char *name)
{
	struct fileusage *fup, **fhp, *fupSort, *fupSortBK;
	int len;

	if (fup = lookup(id, type))
		return (fup);

	if (name)
		len = strlen(name);
	else
		len = 10; /* maximum number of characters for displaying MAXulong */

	if (!(fup = (struct fileusage *)calloc(1,sizeof(struct fileusage) + len))) {
		fprintf(stderr, "out of memory for fileusage structures\n");
		exit(1);
	}
	fhp = &fuhead[type][id & (FUHASH - 1)];
	fup->fu_next = *fhp;
	*fhp = fup;
	fup->fu_id = id;


	if (name)
		bcopy(name, fup->fu_name, len + 1);
	else 
		sprintf(fup->fu_name, "%u", id);


        /*******************************************
         *        sorted LOCATOR
         *******************************************/
        for ( fupSort = fuSortedHead[type], fupSortBK = 0 ; fupSort ;
              fupSort = fupSort->fu_sorted )
        {
                if ( id <= fupSort->fu_id )
                        break ;
                fupSortBK = fupSort ;
        }

        /*******************************************
         *        sorted INSERTER
         *******************************************/
        if ( ! fuSortedHead[type] ) {
                /* head is empty */
                fuSortedHead[type] = fup ;
        } else if ( ! fupSortBK ) {
                /* new head */
                fup->fu_sorted = fuSortedHead[type] ;
                fuSortedHead[type]   = fup ;
        } else if ( ! fupSort ) {
                /* tail insert */
                fupSortBK->fu_sorted = fup ;
        } else {
                /* chain insert */
                fup->fu_sorted       = fupSortBK->fu_sorted ;
                fupSortBK->fu_sorted = fup ;
        }

	return (fup);
}

/*
 * Calculate the grace period and return a printable string for it.
 */
char *
timeprt(time_t seconds)
{
	time_t hours, minutes;
	static char buf[20];
	static time_t now;

	if (now == 0)
		time(&now);
	if (now > seconds)
		return ("none");
	seconds -= now;
	minutes = (seconds + 30) / 60;
	hours = (minutes + 30) / 60;
	if (hours >= 36) {
		sprintf(buf, MSGSTR(DAYS, "%ddays"), (hours + 12) / 24);
		return (buf);
	}
	if (minutes >= 60) {
		sprintf(buf, "%2d:%d", minutes / 60, minutes % 60);
		return (buf);
	}
	sprintf(buf, "%2d", minutes);
	return (buf);
}
