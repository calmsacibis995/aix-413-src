static char sccsid[] = "@(#)88	1.51  src/bos/usr/sbin/rdump/dumpmain.c, cmdarch, bos411, 9428A410j 3/14/94 10:42:39";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
static char sccsid[] = "(#)dumpmain.c	5.4 (Berkeley) 5/28/86";
#endif not lint
*/

#include <nl_types.h>
#include "dumpi_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_DUMP,N,S)
nl_catd catd;

#include "dump.h"
#include <sys/vmount.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/audit.h>
#include <sys/priv.h>
#include <sys/id.h>
#include <sys/errno.h>

#include <locale.h>
extern void (*dump_signal()) ();
struct  pfstab {
        struct  pfstab *pf_next;
        struct  fstab *pf_fstab;
};
extern struct pfstab *getfstab();

#define	EQ(a,b)		(strcmp(a,b) == 0)
extern	io_error;
int 	first_tape_fd;
int 	is_first_tape = 1;
int	ntrec = HIGHDENSITYTREC;	/* # tape blocks in each tape record */
				
int	cartridge = 0;	/* Assume non-cartridge tape */
struct devinfo devinfo;
extern int nogripe;

#ifndef RDUMP
extern int	range;
extern int	unit;
extern int	min_unit;
extern int	max_unit;
extern char	unit_prefix[];
char		tapenamebuf[PATH_MAX];
#endif

#ifdef RDUMP
char	*host;
char	*user;
#endif
int	anydskipped;	/* set true in mark() if any directories are skipped */
			/* this lets us avoid map pass 2 in some cases */

int	Batch_flag = 0;
daddr_t fs_size;	/* size (in 512 bytes) of entire filesystem */
ino_t	imax;		/* maximum number of inodes in the filesystem */

main(argc, argv)
	int	argc;
	char	*argv[];
{
#ifdef RDUMP
	char 		*arg;
#endif
	int		bflag = 0, i;
	float		fetapes;
	register	struct	fstab	*dt;
	int 		c;
	extern int	optind;
	extern char	*optarg;
        char            *tmp_disk;
	char 		dev[PATH_MAX+1];
	char		fspath[PATH_MAX+1];
	char 		fsdev[PATH_MAX+1];
	int		got_fstab_info, mounted;
        register struct pfstab *pf;
        struct superblock *Super;
        char superbuf[FSBSIZE];
        static struct pfstab *temp_table = NULL;

	(void) setlocale(LC_ALL,"");
	catd = catopen("dumpi.cat", NL_CAT_LOCALE);

	/* suspend implicit auditing */
/**************************************************
	auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0);
***************************************************/

	time(&(spcl.c_date));

	tsize = 0;	/* Default later, based on 'c' option for cart tapes */
	diskette = 0;
	tape = TAPE;
	disk = DISK;
	increm = NINCREM;
	temp = TEMP;
	if (TP_BSIZE / DEV_BSIZE == 0 || TP_BSIZE % DEV_BSIZE != 0) {
		msg(MSGSTR(TPBSIZE, "TP_BSIZE must be a multiple of DEV_BSIZE\n"));
		dumpabort();
	}
	incno = '9';
	uflag = 0;
#ifdef RDUMP
	if (argc > 1 && argv[1][0] != '-') {
		arg = "u";
		if(argc > 1) {
			argv++;
			argc--;
			arg = *argv;
			if (*arg == '-')
				argc++;
		}
		while(*arg)
		switch (*arg++) {
		case 'w':
/*			lastdump('w');  comment out until fs_freq has a good value - dcr for 3.3  */ 
                           /* tell us only what has to be done */
			exit(0);
			break;
		case 'W':			/* what to do */
			lastdump('W');		/* tell us the current state of what has been done */
			exit(0);		/* do nothing else */
			break;
	
		case 'f':			/* output file */
			if(argc > 1) {
				argv++;
				argc--;
				tape = *argv;
			}
			break;
	
		case 'd':			/* density, in bits per inch */
			if (argc > 1) {
				argv++;
				argc--;
				density = atoi(*argv) / 10;
				if (density >= 625 && !bflag)
					ntrec = HIGHDENSITYTREC;
			}
			break;
	
		case 's':			/* tape size, feet */
			if(argc > 1) {
				argv++;
				argc--;
				tsize = atol(*argv);
				tsize *= 12L*10L;
			}
			break;
	
		case 'b':			/* blocks per tape write */
			if(argc > 1) {
				argv++;
				argc--;
				bflag++;
				ntrec = atol(*argv);
			}
			break;
	
		case 'B':			/* Batch : non-interactive */
			Batch_flag = 1;
			break;
	
		case 'c':			/* Tape is cart. not 9-track */
			cartridge++;
			break;
	
		case '0':			/* dump level */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			incno = arg[-1];
			break;
	
		case 'u':			/* update /etc/dumpdates */
			uflag++;
			break;
	
		default:
		        fprintf(stderr, MSGSTR(RBADKEY1, "rdump: %c is not a recognized flag\n"), arg[-1]);
			Exit(X_ABORT);
		}
		if(argc > 1) {
			argv++;
			argc--;
			disk = *argv;
		}
	}
	else {
#endif
/* if no argument is given, then 9u is implied */
	if (argc == 1) uflag++;
#ifdef RDUMP
	while ((c = getopt(argc, argv, "?wWf:d:s:b:Bc0123456789u")) != EOF)
#else
	while ((c = getopt(argc, argv, "?wWf:d:s:b:l:Bc0123456789u")) != EOF)
#endif
		switch (c) {
        	case 'w':
/*			lastdump('w');  comment out until fs_freq has a good value - dcr for 3.3  */ 
                           /* tell us only what has to be done */
			exit(0);
			break;
		case 'W':			/* what to do */
			lastdump('W');		/* tell us the current state of what has been done */
			exit(0);		/* do nothing else */
			break;

		case 'f':			/* output file */
			tape = optarg;
			break;

		case 'd':			/* density, in bits per inch */
			density = atoi(optarg) / 10;
			if (density >= 625 && !bflag)
				ntrec = HIGHDENSITYTREC;
			break;

		case 's':			/* tape size, feet */
			tsize = atol(optarg);
			tsize *= 12L*10L;
			break;

		case 'b':			/* blocks per tape write */
			bflag++;
			ntrec = atol(optarg);
			break;

		case 'B':			/* Batch : non-interactive */
			Batch_flag = 1;
			break;
#ifndef RDUMP
		case 'l':
			lflag++;
			maxblock = atol(optarg);
			break;
#endif

		case 'c':			/* Tape is cart. not 9-track */
			cartridge++;
			break;

		case '0':			/* dump level */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			incno = c;
			break;

		case 'u':			/* update /etc/dumpdates */
			uflag++;
			break;

		case '?':
#ifdef RDUMP
	        	fprintf(stderr, MSGSTR(RUSAGE, "Usage: rdump [-b Number1] [-d Density] -f Machine:Device [-sSize]\n\t [-u] [-c] [w|W] [-Level] [Filesystem]\n"));
#else
	        	fprintf(stderr, MSGSTR(USAGE, "Usage: backup [-b Number1] [-f Device] [-l Number2] \n\t      [-u] [-c] [w|W] [-Level] [Filesystem]\n"));
#endif
		        Exit(X_ABORT);
        	default:
#ifdef RDUMP
			fprintf(stderr, MSGSTR(RBADKEY1, "rdump: %c is not a recognized flag\n"), arg[-1]);
#else
			fprintf(stderr, MSGSTR(BADKEY1, "backup: %c is not a recognized flag\n"), c);
#endif
		        Exit(X_ABORT);
	}
	if(argc > optind)
		disk = argv[optind];
#ifdef RDUMP
	}
#endif
	if (strcmp(tape, "-") == 0) {
		pipeout++;
		tape = "standard output";
#ifndef RDUMP
	} else {
		char *medium;

		strcpy(tapenamebuf, tape);
		medium = tapenamebuf + strlen(tapenamebuf) - 1;
		if (isdigit((int)*medium)) {
			while (isdigit((int)*medium))
				medium--;
			max_unit = atoi (medium+1);
			if (*medium == '-') {
				*medium-- = NULL;
				if (isdigit((int)*medium)) {
					while (isdigit((int)*medium))
						medium--;
					min_unit = atoi(medium+1);
					*++medium = NULL;
					range = 1;
				}
			}
		}
		if (range) {
			strcpy(unit_prefix, tapenamebuf);
			if (valid_range(unit_prefix, min_unit, max_unit)) {
				unit = min_unit;
				tape = tapenamebuf;
				sprintf(tape, "%s%d", unit_prefix, unit);
			} else
				range = 0;
		}
#endif
	}

#ifndef RDUMP
	if (!pipeout && isdiskette(tape)) {
		if (!lflag) {
			lflag++;
			maxblock = ( devinfo.un.dk.numblks / 2 );
		}
		ntrec = 2 * devinfo.un.dk.secptrk;
	}
#endif

	/*
	 * Determine how to default tape size and density
	 *
	 *         	density				tape size
	 * 9-track	1600 bpi (160 bytes/.1")	2300 ft.
	 * 9-track	6250 bpi (625 bytes/.1")	2300 ft.
 	 * cartridge	8000 bpi (100 bytes/.1")	1700 ft. (450*4 - slop)
	 */
	if (density == 0)
		density = cartridge ? 100 : 160;
	if (tsize == 0)
 		tsize = cartridge ? 1700L*120L : 2300L*120L;

#ifdef RDUMP
	{ char *index();
	  host = tape;
	  tape = index(host, ':');
	  if (tape == 0) {
		msg(MSGSTR(NEEDKEY, "need keyletter ``f'' and device ``host:tape''\n"));
		exit(1);
	  }
	  *tape++ = 0;
	  /* parse optional user name */
	  user = host;
	  host = index(user, '@');
	  if (host) *host++ = 0;
	  if (host == 0) {
	      host = user;
	      user = 0;
	  }
	  if (rmthost(host) == 0)
		exit(X_ABORT);
	}
	setuid(getuid());	/* rmthost() is the only reason to be setuid */
#endif
	if (dump_signal(SIGHUP, (void (*)(int))sighup) == SIG_IGN)
		dump_signal(SIGHUP, SIG_IGN);
	if (dump_signal(SIGTRAP, (void (*)(int))sigtrap) == SIG_IGN)
		dump_signal(SIGTRAP, SIG_IGN);
	if (dump_signal(SIGFPE, (void (*)(int))sigfpe) == SIG_IGN)
		dump_signal(SIGFPE, SIG_IGN);
	if (dump_signal(SIGBUS, (void (*)(int))sigbus) == SIG_IGN)
		dump_signal(SIGBUS, SIG_IGN);
	if (dump_signal(SIGSEGV, (void (*)(int))sigsegv) == SIG_IGN)
		dump_signal(SIGSEGV, SIG_IGN);
	if (dump_signal(SIGTERM, (void (*)(int))sigterm) == SIG_IGN)
		dump_signal(SIGTERM, SIG_IGN);
	

	if (dump_signal(SIGINT, (void (*)(int))interrupt) == SIG_IGN)
		dump_signal(SIGINT, SIG_IGN);

	/* get a list of jfs filesystems from /etc/filesystems */
	temp_table = getfstab();

	dt = NULL;
	got_fstab_info = 0;
			/* Look for "disk" in /etc/filesystems */
	for (pf = temp_table; pf; pf = pf->pf_next) {
		if (filesys_match(pf->pf_fstab, disk)) {
			dt = pf->pf_fstab;
			got_fstab_info = 1;
			break;
		}
	}
	tmp_disk = disk;
	if (got_fstab_info)
		strcpy(dev,dt->fs_spec);	/* block special device name */
	else
		strcpy(dev,disk);

	/* If the device is mounted then set 'disk' equal to the  */
	/* character device name and stat the mounted filesystem. */
	/* fspath gets set to the mount point, and fsdev gets set */
	/* to the block special device name.                      */
	if (getmountinfo(dev, fspath, fsdev)) {
		mounted = 1;
		disk = rawname(fsdev);
		if (statfs(fspath, &fs_statbuf, sizeof(struct statfs)) < 0) {
			msg(MSGSTR(BADFS, "bad file system: could not statfs\n"));
			dumpabort();
		}
		/* Check vfs type in case the specified device was not found */
		/* in /etc/filesystems.                                      */
		if (fs_statbuf.f_vfstype != MNT_JFS) {
                        msg(MSGSTR(NOTNATIV, "Not native file system\n"));
                        dumpabort();
		}
	} else {
		mounted = 0;
		if (got_fstab_info)
			disk = rawname(dt->fs_spec);
	}

	if (!mounted && !got_fstab_info) {
		struct stat sbuf;

		if (stat(disk, &sbuf) < 0) {
			msg("%s: ", disk);
			perror("");
			dumpabort();
		}
		/* Is it a block or character device? */
		if (S_ISBLK(sbuf.st_mode)) {
			disk = rawname(disk);
		} else if (!S_ISCHR(sbuf.st_mode)) {
			msg(MSGSTR(NOTFS, "%s is not a filesystem.\n"), disk);
			dumpabort();
		}
	}
	getitime();             /* get /etc/dumpdates info */
	msg(MSGSTR(DUMPD, "Date of this level %c backup: %s\n"), incno, prdate(spcl.c_date));
 	msg(MSGSTR(DUMPDL, "Date of last level %c backup: %s\n"),
		lastincno, prdate(spcl.c_ddate));
	msg(MSGSTR(DUMPING, "Backing up %s "), disk);
	if (got_fstab_info)
		msgtail("(%s) ", dt->fs_file);
#ifdef RDUMP
	msgtail(MSGSTR(TOHOST, "to %s on host %s\n"), tape, host);
#else
	msgtail(MSGSTR(TOLOCLA, "to %s\n"), tape);
#endif

	if (mounted)
		msg(MSGSTR(WARNING1, "File system still mounted, inconsistent data possible\n"));

	fi = open(disk, 0);
	if (fi < 0) {
		msg(MSGSTR(CANTODISK, "Cannot open %s\n"), disk);
		Exit(X_ABORT);
	}
	esize = 0;
	sync();
	get_total_ino();
	msiz = roundup(howmany(imax, NBBY), TP_BSIZE);
	clrmap = (char *)calloc(msiz, sizeof(char));
	dirmap = (char *)calloc(msiz, sizeof(char));
	nodmap = (char *)calloc(msiz, sizeof(char));

	anydskipped = 0;
	msg(MSGSTR(MAP1, "Mapping (Pass I) [regular files]\n"));
	pass(mark, (char *)NULL);		/* mark updates esize */

	if (anydskipped) {
		do {
			msg(MSGSTR(MAP2, "Mapping (Pass II) [directories]\n"));
			nadded = 0;
			pass(add, dirmap);
		} while(nadded);
	} else				/* keep the operators happy */
		msg(MSGSTR(MAP2, "Mapping (Pass II) [directories]\n"));

	bmapest(clrmap);
	bmapest(nodmap);

	if (cartridge) {
		/* Estimate number of tapes, assuming streaming stops at
		   the end of each block written, and not in mid-block.
		   Assume no erroneous blocks; this can be compensated for
		   with an artificially low tape size. */
		fetapes = 
		(	  esize		/* blocks */
			* TP_BSIZE	/* bytes/block */
			* (1.0/density)	/* 0.1" / byte */
		  +
			  esize		/* blocks */
			* (1.0/ntrec)	/* streaming-stops per block */
			* 15.48		/* 0.1" / streaming-stop */
		) * (1.0 / tsize );	/* tape / 0.1" */
	} else {
		/* Estimate number of tapes, for old fashioned 9-track tape */
		int tenthsperirg = (density == 625) ? 3 : 7;
		fetapes =
		(	  esize		/* blocks */
			* TP_BSIZE	/* bytes / block */
			* (1.0/density)	/* 0.1" / byte */
		  +
			  esize		/* blocks */
			* (1.0/ntrec)	/* IRG's / block */
			* tenthsperirg	/* 0.1" / IRG */
		) * (1.0 / tsize );	/* tape / 0.1" */
	}
	etapes = fetapes;		/* truncating assignment */
	etapes++;
	/* count the nodemap on each additional tape */
	for (i = 1; i < etapes; i++)
		bmapest(nodmap);
	esize += i + 10;	/* headers + 10 trailer blocks */
	if (!diskette)
		msg(MSGSTR(ESTBLK, "estimated %ld 1k blocks.\n"), esize);

	alloctape();			/* Allocate tape buffer */

	otape();			/* bitmap is the first to tape write */
	time(&(tstart_writing));
	bitmap(clrmap, TS_CLRI);

	msg(MSGSTR(MAP3, "Backing up (Pass III) [directories]\n"));
 	pass(dirdump, dirmap);

	msg(MSGSTR(MAP4, "Backing up (Pass IV) [regular files]\n"));
	pass(dump, nodmap);

	spcl.c_type = TS_END;
#ifndef RDUMP
	if (devinfo.devtype == DD_SCTAPE || devinfo.devtype == DD_TAPE ||
	    diskette) {
 	 	/* output ntrec X 5 (number of SLAVES) end of tape recs */
		for(i=0; i<ntrec*5; i++)
			spclrec();
		/* make sure that we don't need to start another volume */
		check_eot();
	} 
	else {
		for(i=0; i<ntrec; i++)
			spclrec();
	}
#endif
	if (!diskette)
		msg(MSGSTR(DUMPBLKS, "%ld 1k blocks on %d volume(s)\n"),spcl.c_tapea,spcl.c_volume);

	putitime();
#ifndef RDUMP
	if (!pipeout) {
		close(to);
	}
#else
	tflush(1);
#endif
	dump_rewind();				/* A12856 */
	msg(MSGSTR(DONE2, "Backup is complete\7\7\n"));
	if (io_error)
		Exit(X_IOERR);
	else
		Exit(X_FINOK);
}

	/* "disk" can be either the block or character special file name or */
	/* the pathname of the mount point.                                 */
filesys_match(dt, disk)
register struct fstab *dt;
char *disk;
{
	if (!dt)
		return 0;

	if (EQ(dt->fs_file, disk))		/* mount point */
		return 1;
	if (EQ(dt->fs_spec, disk))		/* block device */
		return 1;
	if (EQ(rawname(dt->fs_spec), disk))	/* character device */
		return 1;

	return 0;
}

getmountinfo(dev, fspath, fsdev)
char *dev, *fspath, *fsdev;
{
	int i, mntcnt, mntsiz, count;
	char *mntinfo, *device, *mntpoint;
	struct vmount *vmt;

	if (!dev)
	    return 0;

	/* Get mount information from mntctl() */

	mntinfo = (char *) &count;
	mntsiz = sizeof(count);
	while ((mntcnt = mntctl(MCTL_QUERY, mntsiz, mntinfo)) <= 0) {
		if (mntcnt < 0) {
			msg(MSGSTR(MNTCTLE, "Cannot get status of mounted file systems\n"));
			dumpabort();
		}
		mntsiz = *((int *) mntinfo);
		mntinfo = (mntinfo == (char *) &count) ? (char *) malloc(mntsiz) : (char *) realloc(mntinfo, mntsiz);
		if (mntinfo == NULL) {
			msg(MSGSTR(MNTCTLE, "Cannot get status of mounted file systems\n"));
			dumpabort();
		}
	}

	/* Search for dev in mount info */

	for (vmt = (struct vmount *)mntinfo, i = 0; i < mntcnt; i++) {
		char *obj, *stub;

		obj = vmt2dataptr(vmt, VMT_OBJECT);	/* device */
		stub = vmt2dataptr(vmt, VMT_STUB);	/* mount point */
		if (EQ(obj, dev) || EQ(rawname(obj), dev) || EQ(stub, dev)) {
			if (fspath)
				strcpy(fspath, stub);
			if (fsdev)
				strcpy(fsdev, obj);
			return 1;
		}
		vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length);
	}
	return 0;
}

#ifndef RDUMP

valid_range(prefix, min, max)
char *prefix;
int min, max;
{
	int i;
	char devname[PATH_MAX];
	struct stat stbuf;

	if (max < min)
		return 0;

	sprintf(devname, "%s%d", prefix, min);

	if (stat(devname, &stbuf) < 0) {
		if (errno == ENOENT)
			return 0;
		else {
			perror("backup");
			Exit(X_ABORT);
		}
	}
	if (!S_ISCHR(stbuf.st_mode)) {
		msg(MSGSTR(BADRANGE, "Device range not valid\n"));
		Exit(X_ABORT);
	}

	for(i=min+1; i <= max; i++) {
		sprintf(devname, "%s%d", prefix, i);
		if ((stat(devname, &stbuf) < 0) || (!S_ISCHR(stbuf.st_mode))) {
			msg(MSGSTR(BADRANGE, "Device range not valid\n"));
			Exit(X_ABORT);
		}
	}

	return 1;
}

#endif

int	sighup(void){	msg(MSGSTR(TRYREW, "SIGHUP()  try rewriting\n")); sigAbort();}
int	sigtrap(void){	msg(MSGSTR(TRYREW1, "SIGTRAP()  try rewriting\n")); sigAbort();}
int	sigfpe(void){	msg(MSGSTR(TRYREW2, "SIGFPE()  try rewriting\n")); sigAbort();}
int	sigbus(void){	msg(MSGSTR(TRYREW3, "SIGBUS()  try rewriting\n")); sigAbort();}
int	sigsegv(void){	msg(MSGSTR(ABORTING, "SIGSEGV()  ABORTING!\n")); abort();}
int	sigalrm(void){	msg(MSGSTR(TRYREW4, "SIGALRM()  try rewriting\n")); sigAbort();}
int	sigterm(void){	msg(MSGSTR(TRYREW5, "SIGTERM()  try rewriting\n")); sigAbort();}

sigAbort()
{
	if (pipeout) {
		msg(MSGSTR(UNKSIG,"Unknown signal, cannot recover\n"));
		dumpabort();
	}
	msg(MSGSTR(REWRITE1, "Rewriting attempted as response to unknown signal.\n"));
	fflush(stderr);
	fflush(stdout);
	nogripe = 1;
	close_rewind();
	exit(X_REWRITE);
}

char *rawname(cp)
	char *cp;
{
	static char rawbuf[FILENAME_MAX+1];
	char *rindex();
	char *dp = rindex(cp, '/');

	if (dp == 0)
		return (0);
	*dp = 0;
	strcpy(rawbuf, cp);
	*dp = '/';
	strcat(rawbuf, "/r");
	strcat(rawbuf, dp+1);
	return (rawbuf);

}

isdiskette(media)
char *media;
{

	int fd;
	
	fd = creat(media, (mode_t)0666);
	first_tape_fd = fd;
	if (fd < 0) {
		msg(MSGSTR(MEDIAOPEN, "Can't open %s\n"), media);
        perror("");
		dumpabort();
	}
	if (ioctl(fd, IOCINFO, &devinfo) < 0) {
		struct stat statbuf;
		if (fstat(fd, &statbuf) < 0) {
			msg(MSGSTR(IOCFAIL, "Can't get info on %s\n"), media);
			dumpabort();
		}
/*		close(fd); */
		return(0);
	}
	if ((devinfo.devtype == DD_DISK) && ((devinfo.flags & DF_FIXED) == 0)) {
/*    		close(fd); */
		diskette++;
		return (1);
	}
/*	close(fd); */
	return (0);
}
