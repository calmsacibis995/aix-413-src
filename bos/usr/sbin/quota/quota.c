static char sccsid[] = "@(#)26	1.12  src/bos/usr/sbin/quota/quota.c, cmdquota, bos411, 9434B411a 8/23/94 09:36:48";
/*
 * COMPONENT_NAME: CMDQUOTA
 *
 * FUNCTIONS: 	main, usage, showuid, showusrname, showgid, showgrpname,
 *		showquotas, heading, timeprt, getprivs
 *
 * ORIGINS: 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
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
 * Berkeley sccsid = "@(#)quota.c	5.11 (Berkeley) 6/1/90";
 *
 * Disk quota reporting program.
 */


#include "sys/param.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <jfs/quota.h>
#include <stdio.h>
#include "fstab.h"
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <locale.h>
#include <ctype.h>

#include "quota_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOTA,n,s) 

struct quotause {
	struct	quotause *next;
	long	flags;
	struct	dqblk dqblk;
	char	fsname[MAXPATHLEN + 1];
};

int	qflag;
int	vflag;

/*
 * Function prototypes
 */
struct quotause *getprivs(register long, int);
void   heading (int, u_long, char *, char *);
void   showgid (gid_t);
void   showgrpname (char *);
void   showusrname (char *);
void   showquotas (int, u_long, char *);
void   showuid (uid_t);
char   *timeprt	(time_t);
void   usage ();


main(int argc, char **argv)
{
	int ngroups;
	gid_t gidset[NGROUPS];
	int i, gflag = 0, uflag = 0;
	char ch;
	extern char *optarg;
	extern int optind, errno;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_QUOTA,NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "ugvq")) != (char) EOF) {
		switch(ch) {
		case 'g':
			gflag++;
			break;
		case 'u':
			uflag++;
			break;
		case 'v':
			vflag++;
			break;
		case 'q':
			qflag++;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (!uflag && !gflag)
		uflag++;
	if (argc == 0) {
		if (uflag)
			showuid(getuid());
		if (gflag) {
			ngroups = getgroups(NGROUPS, gidset);
			if (ngroups < 0) {
				perror("quota: getgroups");
				exit(1);
			}
			for (i = 0; i < ngroups; i++)
				showgid(gidset[i]);
		}
		exit(0);
	}
	if (uflag && gflag)
		usage();
	if (uflag) {
		for (; argc > 0; argc--, argv++) {
			if (alldigits(*argv))
				showuid((uid_t)atoi(*argv));
			else
				showusrname(*argv);
		}
		exit(0);
	}
	if (gflag) {
		for (; argc > 0; argc--, argv++) {
			if (alldigits(*argv))
				showgid((gid_t)atoi(*argv));
			else
				showgrpname(*argv);
		}
		exit(0);
	}
}


void
usage()
{

	fprintf(stderr, "%s\n%s\n%s\n",
		MSGSTR(USE1, "Usage:  quota [-guqv]"),
		MSGSTR(USE2, "\tquota [-qv] -u username ..."),
		MSGSTR(USE3, "\tquota [-qv] -g groupname ..."));
	exit(1);
}


/*
 * Print out quotas for a specified user identifier.
 */
void
showuid(uid_t uid)
{
	struct passwd *pwd = getpwuid(uid);
	uid_t myuid;
	char *name;

	if (pwd == NULL)
		name = MSGSTR(NOACC, "(no account)");
	else
		name = pwd->pw_name;
	myuid = getuid();
	if (uid != myuid && myuid != 0) 
	{
		fprintf(stderr, MSGSTR(PERDENU,
		       "Quota: %s (uid %d): permission denied\n"), name, uid);
		return;
	}
	showquotas(USRQUOTA, uid, name);
}


/*
 * Print out quotas for a specifed user name.
 */
void
showusrname(char *name)
{
	struct passwd *pwd = getpwnam(name);
	uid_t myuid;

	if (pwd == NULL) {
		fprintf(stderr, MSGSTR(UNKUSR,
				       "Quota: %s: unknown user\n"), name);
		return;
	}
	myuid = getuid();
	if (pwd->pw_uid != myuid && myuid != 0) {
		fprintf(stderr, MSGSTR(PERDENU,
				"Quota: %s (uid %d): permission denied\n"),
		    	name, pwd->pw_uid);
		return;
	}
	showquotas(USRQUOTA, pwd->pw_uid, name);
}


/*
 * Print out quotas for a specified group identifier.
 */
void
showgid(gid_t gid)
{
	struct group *grp = getgrgid(gid);
	int ngroups;
	gid_t gidset[NGROUPS];
	register int i;
	char *name;

	if (grp == NULL)
		name = MSGSTR(NOENT, "(no entry)");
	else
		name = grp->gr_name;
	ngroups = getgroups(NGROUPS, gidset);
	if (ngroups < 0) {
		perror("quota: getgroups");
		return;
	}
	for (i = 0; i < ngroups; i++)
		if (gid == gidset[i])
			break;
	if (i >= ngroups && getuid() != 0) {
		fprintf(stderr, MSGSTR(PERDENG,
				"Quota: %s (gid %d): permission denied\n"),
		    	name, gid);
		return;
	}
	showquotas(GRPQUOTA, gid, name);
}


/*
 * Print out quotas for a specified group name.
 */
void
showgrpname(char *name)
{
	struct group *grp = getgrnam(name);
	int ngroups;
	gid_t gidset[NGROUPS];
	register int i;

	if (grp == NULL) {
		fprintf(stderr, MSGSTR(UNKGRP,
					"Quota: %s: unknown group\n"),
			name);
		return;
	}
	ngroups = getgroups(NGROUPS, gidset);
	if (ngroups < 0) {
		perror("quota: getgroups");
		return;
	}
	for (i = 0; i < ngroups; i++)
		if (grp->gr_gid == gidset[i])
			break;
	if (i >= ngroups && getuid() != 0) {
		fprintf(stderr, MSGSTR(PERDENG,
				"Quota: %s (gid %d): permission denied\n"),
		    name, grp->gr_gid);
		return;
	}
	showquotas(GRPQUOTA, grp->gr_gid, name);
}


void
showquotas(int type, u_long id, char *name)
{
	register struct quotause *qup;
	struct quotause *quplist;
	char *msgi, *msgb;
	uid_t myuid;
	int fd, lines = 0;
	static int first;
	static time_t now;

	if (now == 0)
		time(&now);
	quplist = getprivs(id, type);
	for (qup = quplist; qup; qup = qup->next) {
		if (!vflag &&
		    qup->dqblk.dqb_isoftlimit == 0 &&
		    qup->dqblk.dqb_ihardlimit == 0 &&
		    qup->dqblk.dqb_bsoftlimit == 0 &&
		    qup->dqblk.dqb_bhardlimit == 0)
			continue;
		msgi = (char *)0;
		if (qup->dqblk.dqb_ihardlimit &&
		    qup->dqblk.dqb_curinodes >= qup->dqblk.dqb_ihardlimit)
			msgi = MSGSTR(FLIMIT, "File limit reached on");
		else if (qup->dqblk.dqb_isoftlimit &&
		    qup->dqblk.dqb_curinodes >= qup->dqblk.dqb_isoftlimit)
			if (qup->dqblk.dqb_itime > now)
				msgi = MSGSTR(FGRACE,"In file grace period on");
			else
				msgi = MSGSTR(FQUOTA, "Over file quota on");
		msgb = (char *)0;
		if (qup->dqblk.dqb_bhardlimit &&
		    qup->dqblk.dqb_curblocks >= qup->dqblk.dqb_bhardlimit)
			msgb = MSGSTR(BLIMIT, "Block limit reached on");
		else if (qup->dqblk.dqb_bsoftlimit &&
		    qup->dqblk.dqb_curblocks >= qup->dqblk.dqb_bsoftlimit)
			if ((DQBTIME(qup->dqblk.dqb_btime)) > now)
				msgb =MSGSTR(BGRACE,"In block grace period on");
			else
				msgb = MSGSTR(BQUOTA, "Over block quota on");
		if (qflag) {
			if ((msgi != (char *)0 || msgb != (char *)0) &&
			    lines++ == 0)
				heading(type, id, name, "");
			if (msgi != (char *)0)
				printf("\t%s %s\n", msgi, qup->fsname);
			if (msgb != (char *)0)
				printf("\t%s %s\n", msgb, qup->fsname);
			continue;
		}
		if (vflag ||
		    qup->dqblk.dqb_curblocks ||
		    qup->dqblk.dqb_curinodes) {
			if (lines++ == 0)
				heading(type, id, name, "");
			printf("%15s%8d%c%7d%8d%8s"
				, qup->fsname
				, qup->dqblk.dqb_curblocks
				, (msgb == (char *)0) ? ' ' : '*'
				, qup->dqblk.dqb_bsoftlimit
				, qup->dqblk.dqb_bhardlimit
				, (msgb == (char *)0) ? ""
				    : timeprt(DQBTIME(qup->dqblk.dqb_btime)));
			printf("%8d%c%7d%8d%8s\n"
				, qup->dqblk.dqb_curinodes
				, (msgi == (char *)0) ? ' ' : '*'
				, qup->dqblk.dqb_isoftlimit
				, qup->dqblk.dqb_ihardlimit
				, (msgi == (char *)0) ? ""
				    : timeprt(qup->dqblk.dqb_itime)
			);
			continue;
		}
	}
	if (!qflag && lines == 0)
		heading(type, id, name, MSGSTR(NONE, "none"));
}

void
heading(int type, u_long id, char *name, char *tag)
{

	printf(MSGSTR(HEAD, "Disk quotas for %s %s (%cid %d): %s\n"),
		qfextension[type], name, *qfextension[type], id, tag);
	if (!qflag && tag[0] == '\0') {
		printf("%15s%8s %7s%8s%8s%8s %7s%8s%8s\n"
			, MSGSTR(MSGFS, "Filesystem")
			, MSGSTR(MSGB, "blocks")
			, MSGSTR(MSGQ, "quota")
			, MSGSTR(MSGL, "limit")
			, MSGSTR(MSGG, "grace")
			, MSGSTR(MSGF, "files")
			, MSGSTR(MSGQ, "quota")
			, MSGSTR(MSGL, "limit")
			, MSGSTR(MSGG, "grace")
		);
	}
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
		return (MSGSTR(NONE, "none"));
	seconds -= now;
	minutes = (seconds + 30) / 60;
	hours = (minutes + 30) / 60;
	if (hours >= 36) {
		sprintf(buf, "%d%s", (hours + 12) / 24, MSGSTR(DAYS,"days"));
		return (buf);
	}
	if (minutes >= 60) {
		sprintf(buf, "%2d:%d", minutes / 60, minutes % 60);
		return (buf);
	}
	sprintf(buf, "%2d", minutes);
	return (buf);
}

/*
 * Collect the requested quota information.
 */
struct quotause *
getprivs(register long id, int quotatype)
{
	register struct fstab *fs;
	register struct quotause *qup, *quptail;
	struct quotause *quphead;
	char *qfpathname;
	int qcmd, fd;

	setfsent();
	quphead = (struct quotause *)0;
	qcmd = QCMD(Q_GETQUOTA, quotatype);
	while (fs = getfsent()) {
		if (strcmp(fs->fs_vfstype, "jfs"))
			continue;
		if (!hasquota(fs, quotatype, &qfpathname))
			continue;
		if ((qup = (struct quotause *)malloc(sizeof *qup)) == NULL) {
			fprintf(stderr,MSGSTR(NOMEM, "Quota: out of memory\n"));
			exit(2);
		}
		if (quotactl(fs->fs_file, qcmd, id, &qup->dqblk) != 0) {
			if ((fd = open(qfpathname, O_RDONLY)) < 0) {
				perror(qfpathname);
				free(qup);
				continue;
			}
			lseek(fd, (long)(id * sizeof(struct dqblk)), L_SET);
			switch (read(fd, &qup->dqblk, sizeof(struct dqblk))) {
			case 0:			/* EOF */
				/*
				 * Convert implicit 0 quota (EOF)
				 * into an explicit one (zero'ed dqblk)
				 */
				bzero((caddr_t)&qup->dqblk,
				    sizeof(struct dqblk));
				break;

			case sizeof(struct dqblk):	/* OK */
				break;

			default:		/* ERROR */
				fprintf(stderr,
					MSGSTR(RDERR, "Quota: read error"));
				perror(qfpathname);
				close(fd);
				free(qup);
				continue;
			}
			close(fd);
		}
		/*
                 * quotactl() will round up my block count to the next
                 * 1024 block if I have a partial block (512).  However the
                 * quota files have the true number of 1024 blocks plus
                 * an overloaded top order bit of dqb_btime if there is
                 * a fragment.  I want to give the exact same number of
                 * blocks no matter where I get the info from.
                 */
                if (DQCARRY(qup->dqblk.dqb_btime))
                        qup->dqblk.dqb_curblocks++;

		strcpy(qup->fsname, fs->fs_file);
		if (quphead == NULL)
			quphead = qup;
		else
			quptail->next = qup;
		quptail = qup;
		qup->next = 0;
	}
	endfsent();
	return (quphead);
}
