static char sccsid[] = "@(#)28	1.8.1.6  src/bos/usr/sbin/quota/quotacheck.c, cmdquota, bos41J, 9516A_all 4/17/95 09:20:26";
/*
 * COMPONENT_NAME: CMDQUOTA
 *
 * FUNCTIONS:   main, usage, needchk, chkquota, update,
 *		getquotagid, lookup, addid, checkall
 *		
 *
 * ORIGINS: 18, 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
 * Berkeley sccsid = "@(#)quotacheck.c	5.14 (Berkeley) 6/1/90";
 *
 * PURPOSE: Fix up / report on disk quotas & usage
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <sys/param.h>
#include <jfs/quota.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <errno.h>
#include "fstab.h"
#include <locale.h>
#include <ctype.h>
#include <libfs/libfs.h>

#include "quotacheck_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_QUOTACHECK,n,s)

/*
 * Block size recording from the filesystem arrives to me in 512 byte
 * blocks.  I will record this size in my curblocks usage up until the
 * point that the quota file is written.  At that point I will switch to
 * 1024 in order to maintain compatibility to AIX 3.1 and AIX 3.2
 * 
 * If I have a partial 1024 byte block after the switch, we will set the
 * top order bit of the dqblk.dqb_btime field.  This will signal that a 
 * partial block must be factored in the next time fragments are allocated 
 * or freed.  
 *
 * (See Feature 41055 for a rationale on this approach)
 */
#define FBLOCK	512 		/* Block size reported to me              */
#define QBLOCK	1024		/* Block size recorded in the quota files */

struct quotaname {
	long	flags;
	char	grpqfname[MAXPATHLEN + 1];
	char	usrqfname[MAXPATHLEN + 1];
};

#define	HASUSR	1
#define	HASGRP	2

struct fileusage {
	struct	fileusage *fu_next;
	struct  fileusage *fu_sorted;
	u_long	fu_curinodes;
	u_long	fu_curblocks;
	uid_t	fu_id;
	char	fu_name[1];
};

#define FUHASH 4096	/* must be power of two */
struct fileusage *fuhead[MAXQUOTAS][FUHASH];
struct fileusage *fuSortedHead[MAXQUOTAS];

int	aflag;			/* all file systems */
int	gflag;			/* check group quotas */
int	uflag;			/* check user quotas */
int	vflag;			/* verbose */

/* 
 * Function prototypes
 */
void   usage       ();
int    needchk     (register struct fstab *);
int    chkquota    (char *, char *, register struct quotaname *);
int    update      (char *, char *, register int);
int    getquotagid ();
int    checkall    ();
struct fileusage   *lookup (long, int);
struct fileusage   *addid  (ulong, int, char *);
char   *calloc 	   ();

main(int argc, char **argv)
{
	register struct fstab *fs;
	register struct passwd *pw;
	register struct group *gr;
	int i, argnum, maxrun, errs = 0;
	struct quotaname *auxdata;
	long done = 0;
	char ch;
	extern char *optarg;
	extern int optind;

        setlocale( LC_ALL, "" );
        catd = catopen(MF_QUOTACHECK,NL_CAT_LOCALE);

	while ((ch = getopt(argc, argv, "aguvl:")) != (char) EOF) {
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
		case 'l':
			maxrun = atoi(optarg);
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if ((argc == 0 && !aflag) || (argc > 0 && aflag))
		usage();
	if (!gflag && !uflag) {
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
	if (aflag)
		exit(checkall());
	if (setfsent() == 0) {
		fprintf(stderr, MSGSTR(NOOPEN, "Can't open "));
		perror(FSYSname);
		exit(8);
	}
	while ((fs = getfsent()) != NULL) {
		if (((argnum = oneof(fs->fs_file, argv, argc)) >= 0 ||
		    (argnum = oneof(fs->fs_spec, argv, argc)) >= 0) &&
		    (auxdata = (struct quotaname *) needchk(fs))) {
			done |= 1 << argnum;
			errs += chkquota(fs->fs_spec, fs->fs_file, auxdata);
		}
	}
	endfsent();
	for (i = 0; i < argc; i++)
		if ((done & (1 << i)) == 0)
			fprintf(stderr,MSGSTR(NOTFOUND,
				"%s must be configured in %s\n"),
				argv[i], FSYSname);
	exit(errs);
}

void 
usage()
{

	fprintf(stderr, MSGSTR(USE1, "Usage:\n\t%s\n\t%s\n"),
		MSGSTR(USE2, "quotacheck [-g] [-u] [-v] -a"),
		MSGSTR(USE3, "quotacheck [-g] [-u] [-v] filesystem"));
	exit(1);
}

/*
 *  Function :	checkall()
 *
 *  Purpose  :	This function has been added to cycle through the filesystems
 *		found in /etc/filesystems.  In the OSF version, this code
 *		called an alternate checkfstab function which had a
 *		large amount of extraneous things that were incompatible.
 *		The pertinent code of checking the filesystems file has
 *		been inlined and other functions such as checking the
 *		disk status and checking multiple filesystems at once
 *		have been dropped.
 */
int
checkall()
{
	struct fstab	*fs	= (struct fstab *)NULL;
	struct quotaname *auxdata = (struct quotaname *)NULL;
	int  errs	= 0;

	if (setfsent() == 0) {
		fprintf(stderr, MSGSTR(NOOPEN, "Can't open "));
		perror(FSYSname);
		exit(8);
	}
	while ((fs = getfsent()) != NULL) {
		if (auxdata = (struct quotaname *) needchk(fs))
			errs += chkquota(fs->fs_spec, fs->fs_file, auxdata);
	}
	endfsent();

	return(errs);
}

int 
needchk(register struct fstab *fs)
{
	register struct quotaname *qnp;
	char *qfnp;

	if (strcmp(fs->fs_vfstype, "jfs") ||
	    strcmp(fs->fs_type, FSTAB_RQ))
		return (0);
	if ((qnp = (struct quotaname *)malloc(sizeof *qnp)) == 0) {
		fprintf(stderr,
			MSGSTR(NOMEM, "Out of memory for quota structures\n"));
		exit(1);
	}
	qnp->flags = 0;
	if (gflag && hasquota(fs, GRPQUOTA, &qfnp)) {
		strcpy(qnp->grpqfname, qfnp);
		qnp->flags |= HASGRP;
	}
	if (uflag && hasquota(fs, USRQUOTA, &qfnp)) {
		strcpy(qnp->usrqfname, qfnp);
		qnp->flags |= HASUSR;
	}
	if (qnp->flags)
		return ((int)qnp);
	free((char *)qnp);
	return (0);
}

/*
 * Function : chkquota
 *
 * Purpose  : Scan the specified filesystem for per user block and 
 *	      inode usage 
 */
int 
chkquota(char *fsname, char *mntpt, register struct quotaname *qnp)
{
	ino_t			imax, ino;
        fdaddr_t		fmax;
        struct dinode		dp;
	struct superblock 	sblock;  /* superblock for this filesystem */
	int			fd;      /* open disk file descriptor      */
	struct fileusage 	*fup;
	int  			mode, errs = 0;

	if ((fd = fsopen(fsname, O_RDONLY)) < 0) {
		perror(fsname);
		return (1);
	}
	if (vflag) {
		fprintf(stdout, MSGSTR(CHK, "*** Checking "));
		if (qnp->flags & HASUSR)
			fprintf(stdout, "%s%s", qfextension[USRQUOTA],
			    (qnp->flags & HASGRP) ? MSGSTR(AND, " and ") : "");
		if (qnp->flags & HASGRP)
			fprintf(stdout, "%s", qfextension[GRPQUOTA]);
		fprintf(stdout, 
			MSGSTR(QUOTAS, " quotas for %s (%s)\n"), fsname, mntpt);
	}

	if (get_super(fd, &sblock)) {
		fprintf(stderr, MSGSTR(CANTREAD, 
			"Cannot read super block on filesystem %s\n"), fsname);
		return(1);
	}

        /*
         * Get the max number of inodes and blocks
         */
        fsmax (&imax, &fmax);
        if ((imax == 0) || (fmax.f.addr <= 0)) {
                fprintf(stderr, MSGSTR(BADFSIZE,
                        "super block has corrupted filesystem size\n"));
                return(1);
        }

	sync();	/* Lessens the chance of counting released inodes */

        for (ino = 0; ino < imax; ino++) {
		/*
		 * We are going to skip the filesystem overhead inodes
		 * and process only those that exist in the filesystem
		 * namespace.
		 */
		if ((ino <= SPECIAL_I) && (ino != ROOTDIR_I))
			continue;

                if ((get_inode (fd,  &dp, ino)) != LIBFS_SUCCESS)
                        continue;
                if (dp.di_nlink != 0) {
			/* 
			 * If the mode masks off and doesn't give me a 
			 * standard file type (like IFDIR, IFREG, IFBLK...), 
			 * then ignore it.
			 */
			if ((mode = dp.di_mode & IFMT) == 0)
                                continue;

			if (qnp->flags & HASGRP) {
				fup = addid(dp.di_gid, GRPQUOTA, (char *)0);
				fup->fu_curinodes++;

				if (mode == IFREG || mode == IFDIR ||
				    mode == IFLNK)
	fup->fu_curblocks += (dp.di_nblocks * sblock.s_fragsize) / FBLOCK;

			}	
			if (qnp->flags & HASUSR) {
				fup = addid(dp.di_uid, USRQUOTA, (char *)0);
				fup->fu_curinodes++;

				if (mode == IFREG || mode == IFDIR ||
				    mode == IFLNK)
	fup->fu_curblocks += (dp.di_nblocks * sblock.s_fragsize) / FBLOCK;

			}
		}
   	}
	if (qnp->flags & HASUSR)
		errs += update(mntpt, qnp->usrqfname, USRQUOTA);
	if (qnp->flags & HASGRP)
		errs += update(mntpt, qnp->grpqfname, GRPQUOTA);
	close(fd);
	return (errs);
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
		fprintf(stderr, 
		     MSGSTR(NOMEMU,"Out of memory for fileusage structures\n"));
		exit(1);
	}
	fhp = &fuhead[type][id & (FUHASH - 1)];
	fup->fu_next = *fhp;
	*fhp = fup;
	fup->fu_id = id;

        if (name)
                bcopy(name, fup->fu_name, len);
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
 * Function : update
 *
 * Purpose  : Update the specified quota file with new usage statistics.
 */
int 
update(char *fsname, char *quotafile, register int type)
{
	register struct fileusage *fup, *fupSort, *fupSortBK;
	register FILE 	*qfi, *qfo;
	register ulong 	id, lastid;
	struct   dqblk 	dqbuf;
	extern   int    errno;
	static   int    warned = 0;
	static   struct dqblk zerodqbuf;
	static   struct fileusage zerofileusage;
	int	 quota_blocks;		/* number of whole 1K blocks used   */
	int	 partial_blocks; 	/* are there partial 1K blocks      */ 
	int	 rounded_quota;		/* quota blocks rounded to the next */
					/* 1K block                         */
	int	 rounded_fs;		/* filesystem blocks rounded to the  */
					/* next 1K blocks                   */

	if ((qfo = fopen(quotafile, "r+")) == NULL) {
		if (errno != ENOENT) {
			perror(quotafile);
			return (1);
		}
		if ((qfo = fopen(quotafile, "w+")) == NULL) {
			perror(quotafile);
			return (1);
		}
		fprintf(stderr, 
			MSGSTR(CREATE, "Creating quota file %s\n"), quotafile);
		(void) fchown(fileno(qfo), getuid(), getquotagid());
		(void) fchmod(fileno(qfo), 0640);
	}
	if ((qfi = fopen(quotafile, "r")) == NULL) {
		perror(quotafile);
		fclose(qfo);
		return (1);
	}
	if (quotactl(fsname, QCMD(Q_SYNC, type), (u_long)0, (caddr_t)0) < 0 &&
	    errno == EOPNOTSUPP && !warned && vflag) {
		warned++;
		fprintf(stdout, MSGSTR(WARN1, "*** Warning: %s\n"),
		    MSGSTR(WARN2, "Quotas are not enabled on this machine."));
	}
        /*
         *  (1) Go sequential from 0 .. USHRT_MAX (65535)
         *  (2) for ids higher than USHRT_MAX, ONLY check
         *      known userids.  Doing otherwise will cause
         *      this loop to go forever.
         */
        for ( id = 0, fupSort = fuSortedHead[type] ; fupSort ; id++ )
        {
                if ( fupSort->fu_id < id )
                        fupSort = fupSort->fu_sorted ;
                if ( id > USHRT_MAX )
                        id = fupSort->fu_id ;
                if ( id > MAXDQID )
                {
                        if (vflag)
                        {
                                if (aflag)
                                        printf("%s: ", fsname);
                                printf(MSGSTR(QLARGEID,
                                       "User id %u (%s) is too large, quotas are not maintained\n"),
                                       fupSort->fu_id, fupSort->fu_name);
                        }
                        if ( id == -1 )
                                break ;
                        continue ;
                }
                if ( id > USHRT_MAX )
                {
                        fseek(qfi, (long) (id * sizeof(struct dqblk)), SEEK_SET)
;
                        fseek(qfo, (long) (id * sizeof(struct dqblk)), SEEK_SET)
;
                }

		if (fread((char *)&dqbuf, sizeof(struct dqblk), 1, qfi) == 0) {
			dqbuf = zerodqbuf;
			/* 
			 * Position input pointer to next record.  This is
			 * necessary in the first pass during file creation. 
			 */
			fseek(qfi, (long)sizeof(struct dqblk), 1);
		}
		if ((fup = lookup(id, type)) == 0)
			fup = &zerofileusage;

		/* 
		 * Compute if I have a partial block (less than 1024).  This
		 * will be recorded in the dqb_btime high order bit.
		 */
		partial_blocks = fup->fu_curblocks * FBLOCK % QBLOCK;

		/*
		 * Compute the number of 1024 blocks that I have. This will
		 * be the blocks recorded in the quota files. 
		 */
		quota_blocks = fup->fu_curblocks * FBLOCK / QBLOCK;
		
		/*
		 * If we have a partial block in the filesystem or a partial
		 * block in the quota file, then these will be rounded to the
		 * next whole 1K block for display to the user.
		 */ 
		rounded_fs = (partial_blocks ? quota_blocks + 1 : quota_blocks);
		rounded_quota = (DQCARRY(dqbuf.dqb_btime) ? 
				dqbuf.dqb_curblocks + 1 : dqbuf.dqb_curblocks);
		/*
		 * First check to see if my current calculations on usage
		 * equal what the quota files say.  This is done by checking
		 * the number of inodes, followed by checking the current 
		 * blocks with my recorded usage.
		 *
		 * Lastly I check if I have any partial 1024 blocks and see 
		 * if the dqb_btime field has been overloaded to signify
		 * fragment allocation.  (If my top order bit is set AND
		 * I have a partial block) || (if my top order bit is zero 
		 * AND I don't have a partial block) then I require no changes.
		 */
		if (dqbuf.dqb_curinodes == fup->fu_curinodes &&
		    dqbuf.dqb_curblocks == quota_blocks &&  
		     ((DQCARRY(dqbuf.dqb_btime) && partial_blocks) ||
		     (!DQCARRY(dqbuf.dqb_btime) && !partial_blocks))) {
			fup->fu_curinodes = 0;
			fup->fu_curblocks = 0;
			fseek(qfo, (long)sizeof(struct dqblk), SEEK_CUR);
			continue;
		}

		/*
		 * If verbose and either my current inodes are wrong or
		 * the number of whole quota blocks is wrong then display
		 * this to the user.
		 */ 
		if (vflag && ((dqbuf.dqb_curinodes != fup->fu_curinodes) ||
		    (rounded_fs != rounded_quota))) {
			if (aflag)
				printf("%s: ", fsname);
			printf(MSGSTR(FIX, "%-8s fixed:"), fup->fu_name);

			if (dqbuf.dqb_curinodes != fup->fu_curinodes)
				fprintf(stdout,MSGSTR(INO, "\tinodes %d -> %d"),
					dqbuf.dqb_curinodes, fup->fu_curinodes);

			if (rounded_fs != rounded_quota)
				fprintf(stdout,MSGSTR(BLK, "\tblocks %d -> %d"),
				       rounded_quota, rounded_fs);
			fprintf(stdout, "\n");
		}
		/*
		 * Reset time limit if have a soft limit and were
		 * previously under it, but are now over it.
		 */
		if (dqbuf.dqb_bsoftlimit &&
		    dqbuf.dqb_curblocks < dqbuf.dqb_bsoftlimit &&
		    quota_blocks >= dqbuf.dqb_bsoftlimit)
			dqbuf.dqb_btime = 0;
		if (dqbuf.dqb_isoftlimit &&
		    dqbuf.dqb_curinodes < dqbuf.dqb_isoftlimit &&
		    fup->fu_curinodes >= dqbuf.dqb_isoftlimit)
			dqbuf.dqb_itime = 0;

		/*
		 * Update the current number of inodes and disk blocks.
		 * If I have a partial block I need to set the high order 
	         * bit of the dqb_btime field.
		 */
		dqbuf.dqb_curinodes = fup->fu_curinodes;
		dqbuf.dqb_curblocks = quota_blocks;

		if (partial_blocks)
			dqbuf.dqb_btime = DQSBTIME(1, DQBTIME(dqbuf.dqb_btime));
		else
			dqbuf.dqb_btime = DQBTIME(dqbuf.dqb_btime);

		/*
		 * Finally update the quota file and the tell the kernel
	 	 * about it.
		 */
		fwrite((char *)&dqbuf, sizeof(struct dqblk), 1, qfo);
		(void) quotactl(fsname, QCMD(Q_SETUSE, type), id,
		    (caddr_t)&dqbuf);
		fup->fu_curinodes = 0;
		fup->fu_curblocks = 0;
	}
	fclose(qfi);
	fflush(qfo);

        for ( fup = fuSortedHead[type], fupSortBK = 0 ; fup ;
              fupSortBK = fup, fup = fup->fu_sorted )
                if ( (unsigned)(fup->fu_id) > MAXDQID )
                        break ;

        if ( fupSortBK )
		ftruncate(fileno(qfo),
			(off_t)((fupSortBK->fu_id + 1) * sizeof(struct dqblk)));
	fclose(qfo);
	return (0);
}


/*
 * Determine the group identifier for quota files.
 */
int 
getquotagid()
{
	struct group *gr;

	if (gr = getgrnam(quotagroup))
		return (gr->gr_gid);
	return (-1);
}

