static char sccsid[] = "@(#)30	1.5.1.2  src/bos/usr/sbin/quota/quotaon.c, cmdquota, bos411, 9428A410j 12/23/93 12:12:55";
/*
 * COMPONENT_NAME: CMDQUOTA
 *
 * FUNCTIONS: 	main, usage, quotaonoff, get_vmount, readonly
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
 * Berkeley sccsid = "@(#)quotaon.c   5.11 (Berkeley) 6/1/90"
 *
 * Turn quotas on/off for a filesystem.
 */


#include <sys/param.h>
#include <sys/file.h>
#include <jfs/quota.h>
#include <stdio.h>
#include <sys/vmount.h>
#include <errno.h>
#include "fstab.h"
#include <locale.h>
#include <ctype.h>

#include "quotaon_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOTAON,n,s)


#define NILPTR(type)    	((type *) 0)
#define COMPARE(v, i, s)        (strcmp(vmt2dataptr((v), (i)), (s)) == 0)

int	aflag;		/* all file systems */
int	gflag;		/* operate on group quotas */
int	uflag;		/* operate on user quotas */
int	vflag;		/* verbose */

/*
 * Function Prototypes
 */
static int get_vmount (struct vmount **);
int  quotaonoff (register struct fstab *, int, int, char *);
int  readonly   (register struct fstab *);
void usage      (char *);


main(int argc, char **argv)
{
	register struct fstab *fs;
	char ch, *qfnp, *whoami, *rindex();
	long argnum, done = 0;
	int i, offmode = 0, errs = 0;
	extern char *optarg;
	extern int optind;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_QUOTAON,NL_CAT_LOCALE);

	whoami = rindex(*argv, '/') + 1;
	if (whoami == (char *)1)
		whoami = *argv;
	if (strcmp(whoami, "quotaoff") == 0)
		offmode++;
	else if (strcmp(whoami, "quotaon") != 0) {
		fprintf(stderr,
		        MSGSTR(NAME,
			"Name must be quotaon or quotaoff\n"));
		exit(1);
	}
	while ((ch = getopt(argc, argv, "avug")) != (char) EOF) {
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
			usage(whoami);
		}
	}
	argc -= optind;
	argv += optind;
	if (argc <= 0 && !aflag)
		usage(whoami);
	if (!gflag && !uflag) {
		gflag++;
		uflag++;
	}
	setfsent();
	while ((fs = getfsent()) != NULL) {
		if (strcmp(fs->fs_vfstype, "jfs") ||
		    strcmp(fs->fs_type, FSTAB_RQ))
			continue;
		if (aflag) {
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, USRQUOTA, qfnp);

			continue;
		}
		if ((argnum = oneof(fs->fs_file, argv, argc)) >= 0 ||
		    (argnum = oneof(fs->fs_spec, argv, argc)) >= 0) {
			done |= 1 << argnum;
			if (gflag && hasquota(fs, GRPQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, GRPQUOTA, qfnp);
			if (uflag && hasquota(fs, USRQUOTA, &qfnp))
				errs += quotaonoff(fs, offmode, USRQUOTA, qfnp);
		}
	}
	endfsent();
	for (i = 0; i < argc; i++)
		if ((done & (1 << i)) == 0)
			fprintf(stderr, MSGSTR(NOFSTAB,
				"%s: not configured in /etc/filesystems\n"),
				argv[i]);
	exit(errs);
}


void
usage(char *whoami)
{

	fprintf(stderr,MSGSTR(USE1,"Usage:\n\t%s [-g] [-u] [-v] -a\n"),whoami);
	fprintf(stderr,MSGSTR(USE2,"\t%s [-g] [-u] [-v] filesystem\n"),whoami);
	exit(1);
}

int
quotaonoff(register struct fstab *fs, int offmode, int type, char *qfpathname)
{
  
	if (strcmp(fs->fs_file, "/") && readonly(fs))
		return (1);
	if (offmode) {
		if (quotactl(fs->fs_file, QCMD(Q_QUOTAOFF, type), 0, 0) < 0) {
			fprintf(stderr, "quotaoff: ");
			perror(fs->fs_file);
			return (1);
		}
		if (vflag)
			printf(	MSGSTR(QTOFF, "%s: quotas turned off\n"),
				fs->fs_file); return (0);
	}
	if (quotactl(fs->fs_file, QCMD(Q_QUOTAON, type), 0, qfpathname) < 0) {
		fprintf(stderr,MSGSTR(QON, "quotaon: using %s on %s\n"),
			qfpathname, fs->fs_file);
		perror((char *) NULL);
		return (1);
	}
	if (vflag)
		printf(MSGSTR(QTON, "%s: %s quotas turned on\n"), fs->fs_file,
		    qfextension[type]);
	return (0);
}

/*
 * Function : get_vmount()
 *
 */
static int
get_vmount(struct vmount **vmountpp)
{
	struct vmount   *vmountp;
        int              size;
        int              nmounts;

        size = BUFSIZ;                  /* Initial buffer size */

        while (1) 
        {
          if ((vmountp = (struct vmount *) malloc (size)) == NULL) 
          {
            printf(MSGSTR (NOMEM, "Out of memory\n"));
            return(-errno);
          }

          nmounts = mntctl(MCTL_QUERY, size, vmountp);
          if (nmounts > 0 && vmountp->vmt_revision != VMT_REVISION)
          {
            printf(MSGSTR (STALEFS, "Filesystem error: revision %d != %d\n"),
                    vmountp->vmt_revision, VMT_REVISION);
            return(-ESTALE);
          }

          if (nmounts > 0)
          {
            *vmountpp = vmountp;
            return(nmounts);
          } 
          else if (nmounts == 0)  /* the buffer wasn't big enough */
          {
            size = vmountp->vmt_revision;
            free((void *)vmountp);
          } 
          else /* some other kind of error */
          {
	    perror("mntctl");
            free((void *)vmountp);
            return(-errno);
          }
        }
}

/*
 * Verify file system is mounted and not readonly.
 */
int
readonly(register struct fstab *fs)
{
	register struct vmount *vmt;
 	struct vmount *bufp = NILPTR (struct vmount);   /* ptr to 1st vmount */
	int nmount  = 0,
	    mounts  = 0,
	    mounted = FALSE;
	
	if ((nmount = get_vmount(&bufp)) <= 0)
		return(nmount);

        for (mounts = nmount, vmt = bufp;
             (mounts-- && (mounted == FALSE)); 
             vmt = (struct vmount *)((int)vmt + vmt->vmt_length))  
        {
           if (COMPARE(vmt, VMT_OBJECT, fs->fs_spec) &&
               COMPARE(vmt, VMT_STUB, fs->fs_file))
	   {
               	mounted = TRUE;
		break;
 	   }
	}

	if (!mounted)
        {
		printf(MSGSTR(NOMNT, "%s: not mounted\n"), fs->fs_file);
		return (1);
	}

	if (vmt->vmt_flags & MNT_READONLY)
        {
		printf(MSGSTR(FSRDONLY, "%s: mounted read-only\n"),fs->fs_file);
		return (1);
	}
	return (0);
}
