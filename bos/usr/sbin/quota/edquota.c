static char sccsid[] = "@(#)19	1.9.1.5  src/bos/usr/sbin/quota/edquota.c, cmdquota, bos411, 9428A410j 12/23/93 12:11:42";
/*
 * COMPONENT_NAME: CMDQUOTA
 *
 * FUNCTIONS: 	main, usage, getentry, getprivs, putprivs, writeprivs,
 *		readprivs, freeprivs, editit, readtimes, writetimes, 
 *		cvtstoa, cvtatos
 *
 * ORIGINS: 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
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
 * Berkeley sccsid = "@(#)edquota.c   5.14 (Berkeley) 6/19/90";
 *
 * Edit filesystem quotas
 * Disk quota editor.
 */


#include <sys/param.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <jfs/quota.h>
#include <sys/signal.h>
#include <errno.h>
#include "fstab.h"
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <nl_types.h>
#include <ctype.h>

#include "edquota_msg.h"
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_EDQUOTA,n,s)


#define _PATH_VI        "/usr/bin/vi"
#define _PATH_TMP       "/tmp/EdP.aXXXXXX"

char tmpfil[] = _PATH_TMP;

struct quotause {
	struct	quotause *next;
	long	flags;
	struct	dqblk dqblk;
	char	fsname[MAXPATHLEN + 1];
	char	qfname[1];	/* actually longer since this structure is   */
				/* malloc'd storage, and this field is of    */
			  	/* variable length		             */		
};


#define	FOUND	0x01
  
/* 
 * Function prototypes
 */
int  cvtatos    (time_t, char *, time_t *);
char *cvtstoa   (time_t);
int  editit     (char *);
void freeprivs  (struct quotause *);
ulong getentry   (char *, int);
void putprivs   (uid_t, int, struct quotause *);
int  readprivs  (struct quotause *, char *);
int  readtimes  (struct quotause *, char *);
int  writeprivs (struct quotause *, int, char *, int);
int  writetimes (struct quotause *, int, int);
void usage();
struct quotause *getprivs(register ulong, int);


main(int argc, register char **argv)
{
	register struct quotause *qup, *protoprivs, *curprivs;
	extern char *optarg;
	extern int optind;
	register uid_t id, protoid;
	register int quotatype, tmpfd;
	char *protoname, ch;
	int tflag = 0, pflag = 0;

	(void) setlocale( LC_ALL, "" );
	catd = catopen(MF_EDQUOTA,NL_CAT_LOCALE);

	if (argc < 2)
		usage();

	quotatype = USRQUOTA;
	while ((ch = getopt(argc, argv, "ugtp:")) != (char) EOF) {
		switch(ch) {
		case 'p':
			protoname = optarg;
			pflag++;
			break;
		case 'g':
			quotatype = GRPQUOTA;
			break;
		case 'u':
			quotatype = USRQUOTA;
			break;
		case 't':
			tflag++;
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (pflag) {
		if ((protoid = getentry(protoname, quotatype)) == -1)
			exit(1);
		protoprivs = getprivs(protoid, quotatype);
		for (qup = protoprivs; qup; qup = qup->next) {
			qup->dqblk.dqb_btime = 0;
			qup->dqblk.dqb_itime = 0;
		}
		while (argc-- > 0) {
			if ((int)(id = getentry(*argv++, quotatype)) < 0)
				continue;
			putprivs(id, quotatype, protoprivs);
		}
		exit(0);
	}
	tmpfd = mkstemp(tmpfil);
	fchown(tmpfd, getuid(), getgid());
	if (tflag) {
		protoprivs = getprivs((ulong) 0, quotatype);
		if (writetimes(protoprivs, tmpfd, quotatype) == 0)
			exit(1);
		if (editit(tmpfil) && readtimes(protoprivs, tmpfil))
			putprivs(0, quotatype, protoprivs);
		freeprivs(protoprivs);
		close(tmpfd);
		unlink(tmpfil);
		exit(0);
	}
	for ( ; argc > 0; argc--, argv++) {
		if ((id = getentry(*argv, quotatype)) == -1)
			continue;
		curprivs = getprivs(id, quotatype);
		if (writeprivs(curprivs, tmpfd, *argv, quotatype) == 0)
			continue;
		if (editit(tmpfil) && readprivs(curprivs, tmpfil))
			putprivs(id, quotatype, curprivs);
		freeprivs(curprivs);
	}
	close(tmpfd);
	unlink(tmpfil);
	exit(0);
}

void
usage()
{
	fprintf(stderr, "%s%s%s",
		MSGSTR(USE1, 
			"Usage:  edquota [-u] [-p username] username ...\n"),
		MSGSTR(USE2, "\tedquota -g [-p groupname] groupname ...\n"),
		MSGSTR(USE3, "\tedquota [-u] -t\n\tedquota -g -t\n"));
	exit(1);
}

/*
 * This routine converts a name for a particular quota type to
 * an identifier. 
 */
uid_t
getentry(char *name, int quotatype)
{
	struct passwd *pw;
	struct group *gr;

	if (alldigits(name))
		return ((uid_t)atoi(name));
	switch(quotatype) {
	case USRQUOTA:
		if (pw = getpwnam(name))
			return ((uid_t)pw->pw_uid);
		fprintf(stderr, MSGSTR(NOUSER, "%s: no such user\n"), name);
		break;
	case GRPQUOTA:
		if (gr = getgrnam(name))
			return ((uid_t)gr->gr_gid);
		fprintf(stderr, MSGSTR(NOGRP, "%s: no such group\n"), name);
		break;
	default:
		fprintf(stderr, MSGSTR(UNKTYPE, "%d: unknown quota type\n"),
			quotatype);
		break;
	}
	sleep(1);
	return ((uid_t)-1);
}

/*
 * Collect the requested quota information.
 */
struct quotause *
getprivs(register ulong id, int quotatype)
{
	register struct fstab *fs;
	register struct quotause *qup, *quptail;
	struct quotause *quphead;
	int qcmd, qupsize, fd;
	char *qfpathname;
	static int warned = 0;
	extern int errno;

	setfsent();
	quphead = (struct quotause *)0;
	qcmd = QCMD(Q_GETQUOTA, quotatype);
	while (fs = getfsent()) {
		if (strcmp(fs->fs_vfstype, "jfs"))
			continue;
		if (!hasquota(fs, quotatype, &qfpathname))
			continue;
		qupsize = sizeof(*qup) + strlen(qfpathname);
		if ((qup = (struct quotause *)malloc(qupsize)) == NULL) {
			fprintf(stderr,
				MSGSTR(NOMEM,"edquota: out of memory\n"));
			exit(2);
		}
		if (quotactl(fs->fs_file, qcmd, id, &qup->dqblk) != 0) {
	    		if (errno == EOPNOTSUPP && !warned) {
				warned++;
				fprintf(stderr, MSGSTR(WARN, "Warning: %s\n"),
				    	MSGSTR(NOQUOTAS, 
				"Quotas are not enabled on this machine."));
				sleep(3);
			}
			if ((fd = open(qfpathname, O_RDONLY)) < 0) {
				fd = open(qfpathname, O_RDWR|O_CREAT, 0640);
				if (fd < 0 && errno != ENOENT) {
					perror(qfpathname);
					free(qup);
					continue;
				}
				fprintf(stderr, 
				MSGSTR(CREATEQ, "Creating quota file %s\n"),
				    qfpathname);
				sleep(3);
				(void) fchown(fd, getuid(),
				    getentry(quotagroup, GRPQUOTA));
				(void) fchmod(fd, 0640);
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
				MSGSTR(RDERR, "edquota: read error in "));
				perror(qfpathname);
				close(fd);
				free(qup);
				continue;
			}
			close(fd);
		}
		/*
		 * quotactl() will round up my block count to the next
		 * 1024 block if I have a partial block.  However the 
		 * quota files have the true number of 1024 blocks plus
		 * an overloaded top order bit of dqb_btime if there is
		 * a fragment.  I want to give the exact same number of
		 * blocks no matter where I get the info from. 
		 */
                if (DQCARRY(qup->dqblk.dqb_btime))
                        qup->dqblk.dqb_curblocks++;

		strcpy(qup->qfname, qfpathname);
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

/*
 * Store the requested quota information.
 */
void
putprivs(uid_t id, int quotatype, struct quotause *quplist)
{
	register struct quotause *qup;
	int qcmd, fd;

	qcmd = QCMD(Q_SETQUOTA, quotatype);
	for (qup = quplist; qup; qup = qup->next) {
		/*
		 * If my dqb_btime field signifies that I have a 512 byte
		 * fragment then I rounded up to the next 1K block when
		 * I read from the quota file.  Switch it back to the exact
		 * 1K block usage before the rewrite.
		 */
		if (DQCARRY(qup->dqblk.dqb_btime))
			qup->dqblk.dqb_curblocks--;

		if (quotactl(qup->fsname, qcmd, id, &qup->dqblk) == 0)
			continue;
		if ((fd = open(qup->qfname, O_WRONLY)) < 0) {
			perror(qup->qfname);
		} else {
			lseek(fd, id * (ulong)sizeof (struct dqblk), 0);
			if (write(fd, &qup->dqblk, sizeof (struct dqblk)) !=
			    sizeof (struct dqblk)) {
				fprintf(stderr, MSGSTR(EDQ, "edquota: "));
				perror(qup->qfname);
			}
			close(fd);
		}
	}
}

/*
 * Take a list of quota usages and edit it.
 */
int
editit(char *tmpfile)
{
	long omask;
	int pid, stat;
	extern char *getenv();

	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
 top:
	if ((pid = fork()) < 0) {
		extern errno;

		if (errno == EPROCLIM) {
			fprintf(stderr, 
				MSGSTR(PROC, "You have too many processes\n"));
			return(0);
		}
		if (errno == EAGAIN) {
			sleep(1);
			goto top;
		}
		perror(MSGSTR(FORK, "fork"));
		return (0);
	}
	if (pid == 0) {
		register char *ed;

		sigsetmask(omask);
		setgid(getgid());
		setuid(getuid());
		if ((ed = getenv("EDITOR")) == (char *)0)
			ed = _PATH_VI;
		execlp(ed, ed, tmpfile, 0);
		perror(ed);
		exit(1);
	}
	waitpid(pid, &stat, 0);
	sigsetmask(omask);
	if (!WIFEXITED(stat) || WEXITSTATUS(stat) != 0)
		return (0);
	return (1);
}

/*
 * Convert a quotause list to an ASCII file.
 */
int
writeprivs(struct quotause *quplist, int outfd, char *name, int quotatype)
{
	register struct quotause *qup;
	FILE *fd;

	ftruncate(outfd, 0);
	lseek(outfd, 0, L_SET);
	if ((fd = fdopen(dup(outfd), "w")) == NULL) {
		fprintf(stderr, MSGSTR(EDQ, "edquota: "));
		perror(tmpfil);
		exit(1);
	}
	fprintf(fd, MSGSTR(QUOTAS, "Quotas for %s %s:\n"),
		qfextension[quotatype], name);
	for (qup = quplist; qup; qup = qup->next) {
		fprintf(fd,
		MSGSTR(QLIM1, "%s: %s %d, limits (soft = %d, hard = %d)\n"),
		    qup->fsname, MSGSTR(BUSE, "blocks in use:"),
		    qup->dqblk.dqb_curblocks,
		    qup->dqblk.dqb_bsoftlimit,
		    qup->dqblk.dqb_bhardlimit);
		fprintf(fd, 
		MSGSTR(QLIM2, "%s %d, limits (soft = %d, hard = %d)\n"),
		    MSGSTR(IUSE, "\tinodes in use:"), qup->dqblk.dqb_curinodes,
		    qup->dqblk.dqb_isoftlimit, qup->dqblk.dqb_ihardlimit);
	}
	fclose(fd);
	return (1);
}

/*
 * Merge changes to an ASCII file into a quotause list.
 */
int
readprivs(struct quotause *quplist, char *infil)
{
	register struct quotause *qup;
	FILE *fd;
	int cnt;
	register char *cp;
	struct dqblk dqblk;
	char *fsp, line1[BUFSIZ], line2[BUFSIZ];

	fd = fopen ( infil , "r" );
	if (fd == NULL) {
		fprintf(stderr, MSGSTR(TMP, "Can't read temporary file again\n"));
		return (0);
	}
	/*
	 * Discard title line, then read pairs of lines to process.
	 */
	(void) fgets(line1, sizeof (line1), fd);
	while (fgets(line1, sizeof (line1), fd) != NULL &&
	       fgets(line2, sizeof (line2), fd) != NULL) {
		if ((fsp = strtok(line1, " \t:")) == NULL) {
			fprintf(stderr,  
				MSGSTR(FORM1, "%s: bad format\n"), line1);
			return (0);
		}
		if ((cp = strtok((char *)0, "\n")) == NULL) {
			fprintf(stderr, 
				MSGSTR(FORM2, "%s: %s: bad format\n"), fsp,
			    	&fsp[strlen(fsp) + 1]);
			return (0);
		}
		cnt = sscanf(cp,
		    	MSGSTR(BLKUSE,  
			" blocks in use: %d, limits (soft = %d, hard = %d)"),
		    	&dqblk.dqb_curblocks, &dqblk.dqb_bsoftlimit,
		    	&dqblk.dqb_bhardlimit);
		if (cnt != 3) {
			fprintf(stderr,
			MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, cp);
			return (0);
		}
		if ((cp = strtok(line2, "\n")) == NULL) {
			fprintf(stderr, 
			MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, line2);
			return (0);
		}
		cnt = sscanf(cp,
		    	MSGSTR(INOUSE,  
			"\tinodes in use: %d, limits (soft = %d, hard = %d)"),
		    	&dqblk.dqb_curinodes, &dqblk.dqb_isoftlimit,
		    	&dqblk.dqb_ihardlimit);
		if (cnt != 3) {
			fprintf(stderr, 
			MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, line2);
			return (0);
		}
		for (qup = quplist; qup; qup = qup->next) {
			if (strcmp(fsp, qup->fsname))
				continue;
			/*
			 * Cause time limit to be reset when the quota
			 * is next used if previously had no soft limit
			 * or were under it, but now have a soft limit
			 * and are over it.
			 *
			 * I don't want to lose the top order bit because
			 * it may signify a fragment block.
			 */
			if (dqblk.dqb_bsoftlimit &&
			    qup->dqblk.dqb_curblocks >= dqblk.dqb_bsoftlimit &&
			    (qup->dqblk.dqb_bsoftlimit == 0 ||
			     qup->dqblk.dqb_curblocks <
			     qup->dqblk.dqb_bsoftlimit))
				qup->dqblk.dqb_btime = DQSBTIME
					(DQCARRY(qup->dqblk.dqb_btime), 0);

			if (dqblk.dqb_isoftlimit &&
			    qup->dqblk.dqb_curinodes >= dqblk.dqb_isoftlimit &&
			    (qup->dqblk.dqb_isoftlimit == 0 ||
			     qup->dqblk.dqb_curinodes <
			     qup->dqblk.dqb_isoftlimit))
				qup->dqblk.dqb_itime = 0;
			qup->dqblk.dqb_bsoftlimit = dqblk.dqb_bsoftlimit;
			qup->dqblk.dqb_bhardlimit = dqblk.dqb_bhardlimit;
			qup->dqblk.dqb_isoftlimit = dqblk.dqb_isoftlimit;
			qup->dqblk.dqb_ihardlimit = dqblk.dqb_ihardlimit;
			qup->flags |= FOUND;
			if (dqblk.dqb_curblocks == qup->dqblk.dqb_curblocks &&
			    dqblk.dqb_curinodes == qup->dqblk.dqb_curinodes)
				break;
			fprintf(stderr,
			    	MSGSTR(ALLOC,
				"%s: cannot change current allocation\n"), fsp);
			break;
		}
	}
	fclose(fd);
	/*
	 * Disable quotas for any filesystems that have not been found.
	 */
	for (qup = quplist; qup; qup = qup->next) {
		if (qup->flags & FOUND) {
			qup->flags &= ~FOUND;
			continue;
		}
		qup->dqblk.dqb_bsoftlimit = 0;
		qup->dqblk.dqb_bhardlimit = 0;
		qup->dqblk.dqb_isoftlimit = 0;
		qup->dqblk.dqb_ihardlimit = 0;
	}
	return (1);
}

/*
 * Convert a quotause list to an ASCII file of grace times.
 */
int
writetimes(struct quotause *quplist, int outfd, int quotatype)
{
	register struct quotause *qup;
	FILE *fd;

	ftruncate(outfd, 0);
	lseek(outfd, 0, L_SET);
	if ((fd = fdopen(dup(outfd), "w")) == NULL) {
		fprintf(stderr, MSGSTR(EDQ, "edquota: "));
		perror(tmpfil);
		exit(1);
	}
	fprintf(fd, MSGSTR(TIME, 
		"Time units may be: days, hours, minutes, or seconds\n"));
	fprintf(fd, MSGSTR(GRACE,  
		"Grace period before enforcing soft limits for %ss:\n"),
	    	qfextension[quotatype]);
	for (qup = quplist; qup; qup = qup->next) {
		fprintf(fd, MSGSTR(BGRACE, "%s: block grace period: %s, "),
		    qup->fsname, cvtstoa(DQBTIME(qup->dqblk.dqb_btime)));
		fprintf(fd, MSGSTR(FGRACE, "file grace period: %s\n"),
		    cvtstoa(qup->dqblk.dqb_itime));
	}
	fclose(fd);
	return (1);
}

/*
 * Merge changes of grace times in an ASCII file into a quotause list.
 */
int
readtimes(struct quotause *quplist, char *infil)
{
	register struct quotause *qup;
	FILE *fd;
	int cnt;
	register char *cp;
	time_t itime, btime, iseconds, bseconds;
	char *fsp, bunits[10], iunits[10], line1[BUFSIZ];

	fd = fopen ( infil, "r" );
	if (fd == NULL) {
		fprintf(stderr, MSGSTR(TMP, "Can't read temporary file again\n"));
		return (0);
	}
	/*
	 * Discard two title lines, then read lines to process.
	 */
	(void) fgets(line1, sizeof (line1), fd);
	(void) fgets(line1, sizeof (line1), fd);
	while (fgets(line1, sizeof (line1), fd) != NULL) {
		if ((fsp = strtok(line1, " \t:")) == NULL) {
			fprintf(stderr,MSGSTR(FORM1,"%s: bad format\n"), line1);
			return (0);
		}
		if ((cp = strtok((char *)0, "\n")) == NULL) {
			fprintf(stderr, MSGSTR(FORM2, "%s: %s: bad format\n"),
				fsp, &fsp[strlen(fsp) + 1]);
			return (0);
		}
		cnt = sscanf(cp,
		    	MSGSTR(BFGRACE, 
			" block grace period: %d %s file grace period: %d %s"),
		    	&btime, bunits, &itime, iunits);
		if (cnt != 4) {
			fprintf(stderr, 
			MSGSTR(FORM2, "%s: %s: bad format\n"), fsp, cp);
			return (0);
		}
		if (cvtatos(btime, bunits, &bseconds) == 0)
			return (0);
		if (cvtatos(itime, iunits, &iseconds) == 0)
			return (0);
		for (qup = quplist; qup; qup = qup->next) {
			if (strcmp(fsp, qup->fsname))
				continue;
			qup->dqblk.dqb_btime = DQSBTIME
				(DQCARRY(qup->dqblk.dqb_btime), bseconds);
			qup->dqblk.dqb_itime = iseconds;
			qup->flags |= FOUND;
			break;
		}
	}
	fclose(fd);
	/*
	 * reset default grace periods for any filesystems
	 * that have not been found.
	 */
	for (qup = quplist; qup; qup = qup->next) {
		if (qup->flags & FOUND) {
			qup->flags &= ~FOUND;
			continue;
		}
		qup->dqblk.dqb_btime = 0;
		qup->dqblk.dqb_itime = 0;
	}
	return (1);
}

/*
 * Convert seconds to ASCII times.
 */
char *
cvtstoa(time_t time)
{
	static char buf[20];

	if (time % (24 * 60 * 60) == 0) {
		time /= 24 * 60 * 60;
		sprintf(buf,
			"%d %s", time, time == 1 ? MSGSTR(DAY,"day") :
						 MSGSTR(DAYS,"days"));
	} else if (time % (60 * 60) == 0) {
		time /= 60 * 60;
		sprintf(buf,  
			"%d %s", time, time == 1 ? MSGSTR(HR,"hour") : 
						MSGSTR(HOURS,"hours"));
	} else if (time % 60 == 0) {
		time /= 60;
		sprintf(buf, "%d %s", time, time == 1 ? MSGSTR(MINS,"minute") : 						MSGSTR(MINUTES,"minutes"));
	} else
		sprintf(buf, "%d %s", time, time == 1 ? MSGSTR(SEC,"second") :
						MSGSTR(SECONDS,"seconds"));
	return (buf);
}

/*
 * Convert ASCII input times to seconds.
 */
int
cvtatos(time_t time, char *units, time_t *seconds)
{

	if (bcmp(units, MSGSTR(SEC,"second"), 6) == 0)
		*seconds = time;
	else if (bcmp(units, MSGSTR(MINS,"minute"), 6) == 0)
		*seconds = time * 60;
	else if (bcmp(units, MSGSTR(HR,"hour"), 4) == 0)
		*seconds = time * 60 * 60;
	else if (bcmp(units, MSGSTR(DAY,"day"), 3) == 0)
		*seconds = time * 24 * 60 * 60;
	else {
		printf(MSGSTR(BADUNIT,"%s: bad units, specify %s\n"), units,
		    MSGSTR(DHMS, "days, hours, minutes, or seconds"));
		return (0);
	}
	return (1);
}

/*
 * Free a list of quotause structures.
 */
void
freeprivs(struct quotause *quplist)
{
	register struct quotause *qup, *nextqup;

	for (qup = quplist; qup; qup = nextqup) {
		nextqup = qup->next;
		free(qup);
	}
}
