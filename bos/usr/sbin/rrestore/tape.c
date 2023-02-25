static char sccsid[] = "@(#)03	1.25.1.8  src/bos/usr/sbin/rrestore/tape.c, cmdarch, bos41J, 9516A_all 4/5/95 13:14:44";
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

/*
#ifndef lint
static char sccsid[] = "(#)tape.c	5.6 (Berkeley) 5/2/86";
#endif not lint
*/

#define _ILS_MACROS
#include "restore.h"
#include <sys/tape.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/file.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/audit.h>
#include <sys/priv.h>
#include <sys/acl.h>
#include <sys/pcl.h>
#include <sys/id.h>
#include <ctype.h>

static long	fssize = PAGESIZE /*MAXBSIZE*/;
static int	mt = -1;
static int	pipein = 0;
static char	magtape[PATH_MAX];
static int	bct;
static char	*tbf;
static union	u_spcl endoftapemark;
static long	blksread;
static long	tapesread;
static jmp_buf	restart;
static int	gettingfile = 0;	/* restart has a valid frame */
static unsigned long lseek_counter = 0;
static struct devinfo devinfo;
#ifndef RRESTORE
static int	range = 0;
static int	unit;
static int	min_unit;
static int	max_unit;
static char	unit_prefix[PATH_MAX];
#endif RRESTORE

static int	ofile;
static char	*map;
static char	lnkbuf[PATH_MAX];
static int	pathlen;

int		Bcvt;		/* Swap Bytes (for CCI or sun) */
static int	Qcvt;		/* Swap quads (for sun) */
struct acl *getacl();
struct pcl *getpcl();
#ifdef RRESTORE
	char *user;
#endif RRESTORE
/*
 * Set up an input source
 */
setinput(source)
	char *source;
{
#ifdef RRESTORE
	char *host, *tape;
#endif RRESTORE

	flsht();
	if (bflag)
		newtapebuf(ntrec);
#ifndef RRESTORE
	else if (isdiskette(source))
		newtapebuf(2 * devinfo.un.dk.secptrk);
#endif
	else
		newtapebuf(NTREC > HIGHDENSITYTREC ? NTREC : HIGHDENSITYTREC);
	terminal = stdin;
#ifdef RRESTORE
	host = source;
	tape = index(host, ':');
	if (tape == 0) {
nohost:
		msg(MSGSTR(NEEDF, "need keyletter ``f'' and device ``host:tape''\n"));
		done(1);
	}
	*tape++ = '\0';
	/* parse optional user name */
	user = host;
	host = index(user, '@');
	if (host) *host++ = 0;
	if (host == 0) {
	      host = user;
	      user = 0;
	}
	(void) strcpy(magtape, tape);
	if (rmthost(host) == 0)
		done(1);
	setuid(getuid());	/* no longer need or want root privileges */
#else
	if (strcmp(source, "-") == 0) {
		/*
		 * Since input is coming from a pipe we must establish
		 * our own connection to the terminal.
		 */
		terminal = fopen("/dev/tty", "r");
		if (terminal == NULL) {
			perror(MSGSTR(CANTOPTTY, "Cannot open(\"/dev/tty\")"));
			terminal = fopen("/dev/null", "r");
			if (terminal == NULL) {
				perror(MSGSTR(CANTOPNUL, "Cannot open(\"/dev/null\")"));
				done(1);
			}
		}
		pipein++;
	}
	(void) strcpy(magtape, source);
#endif RRESTORE
}

newtapebuf(size)
	long size;
{
	static tbfsize = -1;

	ntrec = size;
	if (size <= tbfsize)
		return;
	if (tbf != NULL)
		free(tbf);
	tbf = (char *)malloc((size_t)(size * TP_BSIZE));
	if (tbf == NULL) {
		fprintf(stderr, MSGSTR(ALLOCF, "Cannot allocate space for archive buffer\n"));
		done(1);
	}
	tbfsize = size;
}

/*
 * Verify that the archive can be accessed and
 * that it actually is a dump archive.
 */
setup()
{
	int i, j, *ip;
	struct stat stbuf;
	extern char *ctime();
	extern int xtrmap(), xtrmapskip();
	struct tm *ltm;
    static char tmbuf[64];

	vprintf(stdout, MSGSTR(VERIFYA, "Verify archive and initialize maps\n"));
#ifdef RRESTORE
	if ((mt = rmtopen(magtape, 0)) < 0)
	{
		perror(magtape);
		done(1);
	}
#else
	if (pipein)
		mt = 0;
	else {
		char *medium, tapenamebuf[PATH_MAX];

		strcpy(tapenamebuf, magtape);
		medium = tapenamebuf + strlen(tapenamebuf) - 1;
		if (isdigit((int)*medium)) {
			while (isdigit((int)*medium))
				medium--;
			max_unit = atoi (medium+1);
			if (*medium == '-' ) {
				*medium-- = NULL;
				if (isdigit((int)*medium)) {
					while ( isdigit((int)*medium) )
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
				sprintf(magtape, "%s%d", unit_prefix, unit);
			} else
				range = 0;
		}
		if ((mt = open(magtape, 0)) < 0) {
			perror(magtape);
			done(1);
		}
	}
#endif
	volno = 1;
	setdumpnum();
	flsht();
	if (!pipein && !bflag)
		findtapeblksize();
	if (gethead(&spcl) == FAIL) {
		bct--; /* push back this block */
		cvtflag++;
		if (gethead(&spcl) == FAIL) {
			fprintf(stderr, MSGSTR(NOTDUMPM, "Media is not in dump format\n"));
			done(1);
		}
		fprintf(stderr, MSGSTR(CONVNEWF, "Converting to new file system format.\n"));
	}
	if (pipein) {
		endoftapemark.s_spcl.c_magic = cvtflag ? OFS_MAGIC : NFS_MAGIC;
		endoftapemark.s_spcl.c_type = TS_END;
		ip = (int *)&endoftapemark;
		j = sizeof(union u_spcl) / sizeof(int);
		i = 0;
		do
			i += *ip++;
		while (--j);
		endoftapemark.s_spcl.c_checksum = CHECKSUM - i;
	}
	if (vflag || command == 't') {
		ltm = localtime(&spcl.c_date);
		strftime(tmbuf,64,"%c",ltm);
		fprintf(stdout, MSGSTR(DUMPDATE, "Dump   date: %s.\n"), tmbuf);
		ltm = localtime(&spcl.c_ddate);
		strftime(tmbuf,64,"%c",ltm);
		fprintf(stdout, MSGSTR(DUMPFROM, "Dumped from: %s.\n"), tmbuf);
	}
	dumptime = spcl.c_ddate;
	dumpdate = spcl.c_date;
	if (stat(".", &stbuf) < 0) {
		perror(MSGSTR(CANTSTAT, "cannot stat ."));
		done(1);
	}

	if (stbuf.st_blksize > 0 && stbuf.st_blksize <= PAGESIZE /*MAXBSIZE*/)
		fssize = stbuf.st_blksize;
	if (((fssize - 1) & fssize) != 0) {
		fprintf(stderr, MSGSTR(BADBLKS, "bad block size %d\n"), fssize);
		done(1);
	}
	if (checkvol(&spcl, (long)1) == FAIL) {
		fprintf(stderr, MSGSTR(NOTVOL1, "Media is not volume 1 of the dump\n"));
		done(1);
	}
	if (readhdr(&spcl) == FAIL)
		panic(MSGSTR(NOHEAD, "no header after volume mark!\n"));
	findinode(&spcl, 1);
	if (checktype(&spcl, TS_CLRI) == FAIL) {
		fprintf(stderr, MSGSTR(CANTFIND, "Cannot find file removal list\n"));
		done(1);
	}
	maxino = (spcl.c_count * TP_BSIZE * NBBY) + 1;
	dprintf(stdout, "maxino = %d\n", maxino);
	map = calloc((size_t)1, (size_t)howmany(maxino, NBBY));
	if (map == (char *)NIL)
		panic(MSGSTR(NOMEMREV, "no memory for file removal list\n"));
	clrimap = map;
	curfile.action = USING;
	getfile(xtrmap, xtrmapskip, 1);
	if (checktype(&spcl, TS_BITS) == FAIL) {
		fprintf(stderr, MSGSTR(CANTFINL, "Cannot find file dump list\n"));
		done(1);
	}
	map = calloc((size_t)1, (size_t)howmany(maxino, NBBY));
	if (map == (char *)NULL)
		panic(MSGSTR(NOMEMDUMP, "no memory for file dump list\n"));
	dumpmap = map;
	curfile.action = USING;
	getfile(xtrmap, xtrmapskip, 1);
}

#ifndef RRESTORE

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
			perror("restore");
			done(1);
		}
	}
	if (!S_ISCHR(stbuf.st_mode)) {
		fprintf(stderr, MSGSTR(BADRANGE, "Device range not valid\n"));
		done(1);
	}
	for(i=min+1; i <= max; i++) {
		sprintf(devname, "%s%d", prefix, i);
		if ((stat(devname, &stbuf) < 0) || (!S_ISCHR(stbuf.st_mode))) {
			fprintf(stderr, MSGSTR(BADRANGE, "Device range not valid\n"));
			done(1);
		}
	}

	return 1;
}

#endif

/*
 * Prompt user to load a new dump volume.
 * "Nextvol" is the next suggested volume to use.
 * This suggested volume is enforced when doing full
 * or incremental restores, but can be overrridden by
 * the user when only extracting a subset of the files.
 */
getvol(nextvol)
	long nextvol;
{
	long newvol;
	long savecnt, i;
	union u_spcl tmpspcl;
#	define tmpbuf tmpspcl.s_spcl
	struct tm *ltm;
	static char tmbuf[64];
#ifndef RRESTORE
	int do_again = 0;
#endif

	if (nextvol == 1) {
		tapesread = 0;
		gettingfile = 0;
	}
	if (pipein) {
		if (nextvol != 1)
			panic(MSGSTR(CANPIPE, "Changing volumes on pipe input?\n"));
		if (volno == 1)
			return;
		goto gethdr;
	}
	savecnt = blksread;
again:
	if (pipein)
		done(1); /* pipes do not get a second chance */
#ifdef RRESTORE
	if (command == 'R' || command == 'r' || curfile.action != SKIP)
#else
	if (command == 'R' || command == 'r' || curfile.action != SKIP || range)
#endif
		newvol = nextvol;
	else 
		newvol = 0;
	while (newvol <= 0) {
		if (tapesread == 0) {
			fprintf(stderr, "%s%s%s%s%s",
			    MSGSTR(MEDIAMES1, "You have not read any media yet.\n"),
			    MSGSTR(MEDIAMES2, "Unless you know which volume your"),
			    MSGSTR(MEDIAMES3, " file(s) are on you should start\n"),
			    MSGSTR(MEDIAMES4, "with the last volume and work"),
			    MSGSTR(MEDIAMES5, " towards the first.\n"));
		} else {
			fprintf(stderr, MSGSTR(YOUHAVE, "You have read volumes"));
			strcpy(tbf, ": ");
			for (i = 1; i < 32; i++)
				if (tapesread & (1 << i)) {
					fprintf(stderr, "%s%d", tbf, i);
					strcpy(tbf, ", ");
				}
			fprintf(stderr, "\n");
		}
		do	{
			fprintf(stderr, MSGSTR(NEXTVOL, "Specify next volume #: "));
			(void) fflush(stderr);
			(void) fgets(tbf, BUFSIZ, terminal);
		} while (!feof(terminal) && tbf[0] == '\n');
		if (feof(terminal))
			done(1);
		newvol = atoi(tbf);
		if (newvol <= 0) {
			fprintf(stderr,
			    MSGSTR(VOLPOS, "Volume numbers are positive numerics\n"));
		}
	}
	if (newvol == volno) {
		tapesread |= 1 << volno;
		return;
	}
	closemt();
#ifndef RRESTORE
	if (range) {
		if (do_again) {
			fprintf(stderr, MSGSTR(MOUNTV,
"Mount volume %d on %s and press the Enter key to continue.\n"),
				newvol, magtape);
			(void) fgets(tbf, BUFSIZ, terminal);
			if (feof(terminal))
				done(1);
			do_again = 0;
		} else {
			unit++;
			if (unit > max_unit) {
				unit = min_unit;
				fprintf(stderr, MSGSTR(MOUNTV2,
"Mount volumes %d-%d and press the Enter key to continue.\n"),
					newvol, newvol+(max_unit-min_unit));
				(void) fgets(tbf, BUFSIZ, terminal);
				if (feof(terminal))
					done(1);
			}
			sprintf(magtape, "%s%d", unit_prefix, unit);
		}
	} else
#endif
	{
		fprintf(stderr, MSGSTR(MOUNT, "Mount volume %d\n"), newvol);
		fprintf(stderr, MSGSTR(ENTERD, "then enter device name (default: %s) "), magtape);
		(void) fflush(stderr);
		(void) fgets(tbf, BUFSIZ, terminal);
		if (feof(terminal))
			done(1);
		if (tbf[0] != '\n') {
			(void) strcpy(magtape, tbf);
			magtape[strlen(magtape) - 1] = '\0';
		}
	}
#ifdef RRESTORE
	if ((mt = rmtopen(magtape, 0)) == -1)
#else
	if ((mt = open(magtape, 0)) == -1)
#endif
	{
		fprintf(stderr, MSGSTR(CANTOPEN, "Cannot open %s\n"), magtape);
#ifndef RRESTORE
		if (range)
			do_again = 1;
#endif
		volno = -1;
		goto again;
	}
gethdr:
	volno = newvol;
	setdumpnum();
	flsht();
	if (readhdr(&tmpbuf) == FAIL) {
		fprintf(stderr, MSGSTR(NOTDUMPM, "media is not in dump format\n"));
#ifndef RRESTORE
		if (range)
			do_again = 1;
#endif
		volno = 0;
		goto again;
	}
	if (checkvol(&tmpbuf, volno) == FAIL) {
		fprintf(stderr, MSGSTR(WRONGVOL, "Wrong volume (%d)\n"), tmpbuf.c_volume);
#ifndef RRESTORE
		if (range)
			do_again = 1;
#endif
		volno = 0;
		goto again;
	}
	if (tmpbuf.c_date != dumpdate || tmpbuf.c_ddate != dumptime) {
		ltm = localtime(&tmpbuf.c_date);
		strftime(tmbuf,64,"%c",ltm);
		strcat(tmbuf,"\n");
		fprintf(stderr, MSGSTR(WRONGDATE, "Wrong dump date\n\tgot: %s"), tmbuf);
		ltm = localtime(&dumpdate);
		strftime(tmbuf,64,"%c",ltm);
		strcat(tmbuf,"\n");
		fprintf(stderr, MSGSTR(WANTED, "\twanted: %s"), tmbuf);
#ifndef RRESTORE
		if (range)
			do_again = 1;
#endif
		volno = 0;
		goto again;
	}
	tapesread |= 1 << volno;
	blksread = savecnt;
	if (curfile.action == USING) {
		if (volno == 1)
			panic(MSGSTR(ACTIVEF, "active file into volume 1\n"));
		return;
	}
	(void) gethead(&spcl);
	findinode(&spcl, curfile.action == UNKNOWN ? 1 : 0);
	if (gettingfile) {
		gettingfile = 0;
		longjmp(restart, 1);
	}
}

/*
 * handle multiple dumps per tape by skipping forward to the
 * appropriate one.
 */
setdumpnum()
{
	struct stop tcom;

	if (dumpnum == 1 || volno != 1)
		return;
	if (pipein) {
		fprintf(stderr, MSGSTR(MULTIDP, "Cannot have multiple dumps on pipe input\n"));
		done(1);
	}
	tcom.st_op = STFSF;
	tcom.st_count = dumpnum - 1;
#ifdef RRESTORE
	rmtioctl(STFSF, dumpnum - 1);
#else
	if (ioctl(mt, (int)STIOCTOP, (char *)&tcom) < 0)
		perror(MSGSTR(IOCTLE, "ioctl STFSF"));
#endif
}

extractfile(name)
	char *name;
{
	int mode;
	time_t timep[2];
	struct entry *ep;
	extern int xtrlnkfile(), xtrlnkskip();
	extern int xtrfile(), xtrskip();
	struct acl *aclp;
	struct pcl *pclp;

	curfile.name = name;
	curfile.action = USING;
	timep[0] = curfile.dip->bsd_di_atime;
	timep[1] = curfile.dip->bsd_di_mtime;
	mode = curfile.dip->bsd_di_mode;
	switch (mode & IFMT) {

	default:
		fprintf(stderr, MSGSTR(UNKFMODE, "%s: unknown file mode 0%o\n"), name, mode);
		skipfile(1);
		return (FAIL);

	case IFDIR:
		if (mflag) {
			ep = lookupname(name);
			if (ep == NIL || ep->e_flags & EXTRACT)
				panic(MSGSTR(UNEXTDIR, "unextracted directory %s\n"), name);
			skipfile(1);
			return (GOOD);
		}
		vprintf(stdout, MSGSTR(EXTRFILE, "extract file %s\n"), name);
		return (genliteraldir(name, curfile.ino));

	case IFLNK:
		lnkbuf[0] = '\0';
		pathlen = 0;

		getfile(xtrlnkfile, xtrlnkskip, 1);

		if (pathlen == 0) {
			vprintf(stdout,
			    MSGSTR(ZEROLEN, "%s: zero length symbolic link (ignored)\n"), name);
			return (GOOD);
		}
		return (linkit(lnkbuf, name, SYMLINK));

	case IFCHR:
	case IFBLK:
	case IFIFO:
		vprintf(stdout, MSGSTR(EXTSFILE, "extract special file %s\n"), name);
		if (mknod(name, mode, (int)curfile.dip->bsd_di_rdev) < 0) {
			fprintf(stderr, "%s: ", name);
			(void) fflush(stderr);
			perror(MSGSTR(CANTCRS, "cannot create special file"));
			skipfile(1);
			return (FAIL);
		}
		skipfile(0);
		(void) chown(name, curfile.dip->bsd_di_uid, curfile.dip->bsd_di_gid);
		if (checktype(&spcl, TS_ACL) == GOOD) {
			aclp =getacl(&spcl);
			if ((chacl(name, aclp, aclp->acl_len) == -1) && (errno != EPERM)) {
				fprintf(stderr, MSGSTR(EACLIMP, "Import of ACLs failed\n"));
				perror("");
			} 
		}
                else 
			(void) chmod(name, (mode_t)mode);

		if (checktype(&spcl, TS_PCL) == GOOD) {
			pclp =getpcl(&spcl);
			if (pclp != (struct pcl *)NULL) {
				if ((chpriv(name, pclp, pclp->pcl_len) == -1)
						&& (errno != EPERM)) {

					/*
					 * The PCL could not be set on the
					 * file for some reason other than
					 * permissions.  The 2nd most likely
					 * cause is the file being open for
					 * writing.  If there is any other
					 * reason, give the "perror" message
					 * as well as the warning below.
					 */

					int	save_errno = errno;

					fprintf(stderr, MSGSTR(EPCLIMP,
						"Import of PCL for file %s failed.\n"),
						name);
					if ((errno = save_errno) != EBUSY)
						perror("");
				} 
			}
		}

		findinode(&spcl, 1);
		utime(name, timep);
		return (GOOD);

	case IFREG:
		vprintf(stdout, MSGSTR(EXTRFILE, "extract file %s\n"), name);
		if ((ofile = creat(name, (mode_t)0666)) < 0) {
			fprintf(stderr, "%s: ", name);
			(void) fflush(stderr);
			perror(MSGSTR(CANTCRF, "cannot create file"));
			skipfile(1);
			return (FAIL);
		}
		(void) fchown(ofile, curfile.dip->bsd_di_uid, curfile.dip->bsd_di_gid);
		getfile(xtrfile, xtrskip, 0);
		if (checktype(&spcl, TS_ACL) == GOOD) {
			aclp =getacl(&spcl);
			if ((fchacl(ofile, aclp, aclp->acl_len) == -1 ) && (errno != EPERM)) {
			
				fprintf(stderr, MSGSTR(EACLIMP, "Import of ACLs failed\n"));
				perror("");
			}
		}
                else 
			(void) fchmod(ofile, mode);

		if (checktype(&spcl, TS_PCL) == GOOD) {
			pclp =getpcl(&spcl);
			if ((fchpriv(ofile, pclp, pclp->pcl_len) == -1) && (errno != EPERM)) {

				/*
				 * The PCL could not be set on the file for
				 * some reason other than permissions.  The
				 * 2nd most likely cause is the file being
				 * open for writing.  If there is any other
				 * reason, give the "perror" message as well
				 * as the warning below.
				 */

				int	save_errno = errno;

				fprintf(stderr, MSGSTR(EPCLIMP,
					"Import of PCL for file %s failed.\n"),
					name);
				if ((errno = save_errno) != EBUSY)
					perror("");
			} 
		}

		findinode(&spcl, 1);
                /*
                 * If the lseek counter is set at this point, just prior to
                 * a close, means that a hole needs to be created at the
                 * end of the file.  Since lseek does not actually cause
                 * the file size to change if close is done now, we seek
                 * back to the beginning of this sequence and do a fclear
                 * on the whole section.
                 */
                if (lseek_counter) {
                        if (lseek(ofile, -lseek_counter, 1) == (long)-1) {
                                fprintf(stderr, MSGSTR(SEEKERR, "fclear error extracting inode %d, name %s\n"), curfile.ino, curfile.name);
                                perror("fclear");
                                done(1);
                        }
                        if (fclear(ofile, lseek_counter) == (long)-1) {
                                fprintf(stderr, MSGSTR(SEEKERR, "fclear error extracting inode %d, name %s\n"), curfile.ino, curfile.name);
                                perror("fclear");
                                done(1);
                        }
                        lseek_counter = 0;
                }
		(void) close(ofile);
		utime(name, timep);
		return (GOOD);
	}
	/* NOTREACHED */
}

/*
 * skip over bit maps on the tape
 */
skipmaps()
{

	while (checktype(&spcl, TS_CLRI) == GOOD ||
	       checktype(&spcl, TS_BITS) == GOOD)
		skipfile(1);
}

/*
 * skip over a file on the tape
 */
skipfile(nxtino)
int nxtino;
{
	extern int null();

	curfile.action = SKIP;
	getfile(null, null, nxtino);
}

/*
 * Do the file extraction, calling the supplied functions
 * with the blocks
 */
getfile(f1, f2, nxtino)
	int	(*f2)(), (*f1)();
	int	nxtino;
{
	register int i;
	int curblk = 0;
	off_t size = spcl.bsd_c_dinode.bsd_di_size;
	static char clearedbuf[PAGESIZE/*MAXBSIZE*/];
	char buf[PAGESIZE/*MAXBSIZE*/ / TP_BSIZE][TP_BSIZE];
	char junk[TP_BSIZE];

	if (checktype(&spcl, TS_END) == GOOD)
		panic(MSGSTR(ENDRUN, "ran off end of archive\n"));
	if (ishead(&spcl) == FAIL)
		panic(MSGSTR(NOTATB, "not at beginning of a file\n"));
	if (!gettingfile && setjmp(restart) != 0)
		return;
	gettingfile++;
loop:
	for (i = 0; i < spcl.c_count; i++) {
		if (spcl.c_addr[i]) {
			readtape(&buf[curblk++][0]);
			if (curblk == fssize / TP_BSIZE) {
				(*f1)(buf, size > TP_BSIZE ?
				     (long) (fssize) :
				     (curblk - 1) * TP_BSIZE + size);
				curblk = 0;
			}
		} else {
			if (curblk > 0) {
				(*f1)(buf, size > TP_BSIZE ?
				     (long) (curblk * TP_BSIZE) :
				     (curblk - 1) * TP_BSIZE + size);
				curblk = 0;
			}
			(*f2)(clearedbuf, size > TP_BSIZE ?
				(long) TP_BSIZE : size);
		}
		if ((size -= TP_BSIZE) <= 0) {
			for (i++; i < spcl.c_count; i++)
				if (spcl.c_addr[i])
					readtape(junk);
			break;
		}
	}
	if (readhdr(&spcl) == GOOD && size > 0) {
		if (checktype(&spcl, TS_ADDR) == GOOD)
			goto loop;
		dprintf(stdout, "Missing address (header) block for %s\n", curfile.name);
	}
	if (curblk > 0)
		(*f1)(buf, (curblk * TP_BSIZE) + size);

	if (nxtino)
		findinode(&spcl, 1);
	gettingfile = 0;
}

/*
 * The next routines are called during file extraction to
 * put the data into the right form and place.
 */
xtrfile(buf, size)
	char	*buf;
	long	size;
{

	if (write(ofile, buf, (int) size) == -1) {
		fprintf(stderr, MSGSTR(WRITEEE, "write error extracting inode %d, name %s\n"),
			curfile.ino, curfile.name);
		perror(MSGSTR(WRITE, "write"));
		done(1);
	}
        /*
         * reset the lseek counter since we have a successful write
         * which means that we don't have a hole at the end of the file.
         */
        lseek_counter = 0;
}

xtrskip(buf, size)
	char *buf;
	long size;
{

#ifdef lint
	buf = buf;
#endif
	if (lseek(ofile, size, 1) == (long)-1) {
		fprintf(stderr, MSGSTR(SEEKERR, "fclear error extracting inode %d, name %s\n"),
			curfile.ino, curfile.name);
		perror("fclear");
		done(1);
	}
        /*
         * add the lseek value to counter in the event we need to
         * backtrack and fclear for a file with hole at the end.
         */
        lseek_counter += size;
}


xtrlnkfile(buf, size)
	char	*buf;
	long	size;
{

	pathlen += size;
	if (pathlen > MAXPATHLEN) {
		fprintf(stderr, MSGSTR(SYMLONG, "symbolic link name: %s->%s%s; too long %d\n"),
		    curfile.name, lnkbuf, buf, pathlen);
		done(1);
	}
	(void) strcat(lnkbuf, buf);
}

xtrlnkskip(buf, size)
	char *buf;
	long size;
{

#ifdef lint
	buf = buf, size = size;
#endif
	fprintf(stderr, MSGSTR(UNALLOCSY, "unallocated block in symbolic link %s\n"),
		curfile.name);
	done(1);
}

xtrmap(buf, size)
	char	*buf;
	long	size;
{

	bcopy(buf, map, size);
	map += size;
}

xtrmapskip(buf, size)
	char *buf;
	long size;
{

#ifdef lint
	buf = buf;
#endif
	panic(MSGSTR(HOLE, "hole in map\n"));
	map += size;
}

null() {;}

/*
 * Do the tape i/o, dealing with volume changes
 * etc..
 */
readtape(b)
	char *b;
{
	register long i;
	long rd, newvol;
	int cnt;

partial:
	if (bct < ntrec) {
		bcopy(&tbf[(bct++*TP_BSIZE)], b, (long)TP_BSIZE);
		blksread++;
		return;
	}
	for (i = 0; i < ntrec; i++)
		((struct s_spcl *)&tbf[i*TP_BSIZE])->c_magic = 0;
	bct = 0;
	cnt = ntrec*TP_BSIZE;
	rd = 0;
getmore:
#ifdef RRESTORE
	i = rmtread(&tbf[rd], cnt);
#else
	i = read(mt, &tbf[rd], cnt);
#endif
	if (i > 0 && i != ntrec*TP_BSIZE) {
		if (pipein) {
			rd += i;
			cnt -= i;
			if (cnt > 0)
				goto getmore;
			i = rd;
		} else {
#ifdef notdef
			if (i % TP_BSIZE != 0)
				panic(MSGSTR(PARTIALB, "partial block read: %d should be %d\n"),
					i, ntrec * TP_BSIZE);
			bcopy((char *)&endoftapemark, &tbf[i],
				(long)TP_BSIZE);
#endif
			/*
			 * Since we now write to EOT we might have a blocking
			 * factor on the tape smaller than a TP_BSIZE block
			 * Round down to TP_BSIZE block
			 */
			if (i % TP_BSIZE != 0)
				i = (i/TP_BSIZE) * TP_BSIZE;
			/*
			 * Now move the blocks down to the end of the buffer
			 * and set up as though we are in the middle of a
			 * block
			 */
			bcopy(tbf, &tbf[ntrec*TP_BSIZE - i], i);
			bct = ntrec - i/TP_BSIZE;
			goto partial;
		}
	}
	if (i < 0) {
		switch (curfile.action) {
		default:
			fprintf(stderr, MSGSTR(SETUPA,
			    "Media read error while trying to set up archive\n"));
			break;
		case UNKNOWN:
			fprintf(stderr, MSGSTR(RESYNC,
			    "Media read error while trying to resyncronize\n"));
			break;
		case USING:
			fprintf(stderr, MSGSTR(RESTOR,
			    "Media read error while restoring %s\n"), curfile.name);
			break;
		case SKIP:
			fprintf(stderr, MSGSTR(SKIPI,
			    "Media read error while skipping over inode %d\n"),
			    curfile.ino);
			break;
		}
		if (!yflag && !reply(MSGSTR(CONTIN, "continue")))
			done(1);
		i = ntrec*TP_BSIZE;
		bzero(tbf, i);
#ifdef RRESTORE
		if (rmtseek(i, 1) < 0)
#else
		if (lseek(mt, i, 1) == (long)-1)
#endif
		{
			perror(MSGSTR(CONTFAIL, "continuation failed"));
			done(1);
		}
	}
	if (i == 0) {
		if (!pipein) {
			newvol = volno + 1;
			volno = 0;
			getvol(newvol);
			readtape(b);
			return;
		}
		if (rd % TP_BSIZE != 0)
			panic(MSGSTR(PARTIALB, "partial block read: %d should be %d\n"),
				rd, ntrec * TP_BSIZE);
		bcopy((char *)&endoftapemark, &tbf[rd], (long)TP_BSIZE);
	}
	bcopy(&tbf[(bct++*TP_BSIZE)], b, (long)TP_BSIZE);
	blksread++;
}

findtapeblksize()
{
	register long i;

	for (i = 0; i < ntrec; i++)
		((struct s_spcl *)&tbf[i * TP_BSIZE])->c_magic = 0;
	bct = 0;
#ifdef RRESTORE
	i = rmtread(tbf, ntrec * TP_BSIZE);
#else
	i = read(mt, tbf, ntrec * TP_BSIZE);
#endif
	if (i <= 0) {
		perror(MSGSTR(MEDIAREADE, "Media read error"));
		done(1);
	}
	if (i % TP_BSIZE != 0) {
		fprintf(stderr, MSGSTR(MEDIABNOT, "Media block size (%d) is not a multiple of dump block size (%d)\n"),
			i, TP_BSIZE);
		done(1);
	}
	ntrec = i / TP_BSIZE;
	vprintf(stdout, MSGSTR(MEDIABI, "Media block size is %d\n"), ntrec);
}

flsht()
{

	bct = ntrec+1;
}

closemt()
{
	if (mt < 0)
		return;
#ifdef RRESTORE
	rmtclose();
#else
	(void) close(mt);
#endif
}

checkvol(b, t)
	struct s_spcl *b;
	long t;
{

	if (b->c_volume != t)
		return(FAIL);
	return(GOOD);
}

readhdr(b)
	struct s_spcl *b;
{

	if (gethead(b) == FAIL) {
		dprintf(stdout, "readhdr fails at %d blocks\n", blksread);
		return(FAIL);
	}
	return(GOOD);
}

/*
 * read the tape into buf, then return whether or
 * or not it is a header block.
 */
gethead(buf)
	struct s_spcl *buf;
{
	long i, *j;
	union u_ospcl {
		char dummy[TP_BSIZE];
		struct	s_ospcl {
			long	c_type;
			long	c_date;
			long	c_ddate;
			long	c_volume;
			long	c_tapea;
			u_short	c_inumber;
			long	c_magic;
			long	c_checksum;
			struct odinode {
				unsigned short odi_mode;
				u_short	odi_nlink;
				u_short	odi_uid;
				u_short	odi_gid;
				long	odi_size;
				long	odi_rdev;
				char	odi_addr[36];
				long	odi_atime;
				long	odi_mtime;
				long	odi_ctime;
			} c_dinode;
			long	c_count;
			char	c_addr[256];
		} s_ospcl;
	} u_ospcl;

	if (!cvtflag) {
		readtape((char *)buf);
		if (buf->c_magic != NFS_MAGIC) {
			if (swabl(buf->c_magic) != NFS_MAGIC)
				return (FAIL);
			if (!Bcvt) {
				vprintf(stdout, MSGSTR(SWAP, "Note: Doing Byte swapping\n"));
				Bcvt = 1;
			}
		}
		if (checksum((int *)buf) == FAIL)
			return (FAIL);
		if (Bcvt)
			swabst("8l4s31l", (char *)buf);
		goto good;
	}
	readtape((char *)(&u_ospcl.s_ospcl));
	bzero((char *)buf, (long)TP_BSIZE);
	buf->c_type = u_ospcl.s_ospcl.c_type;
	buf->c_date = u_ospcl.s_ospcl.c_date;
	buf->c_ddate = u_ospcl.s_ospcl.c_ddate;
	buf->c_volume = u_ospcl.s_ospcl.c_volume;
	buf->c_tapea = u_ospcl.s_ospcl.c_tapea;
	buf->c_inumber = u_ospcl.s_ospcl.c_inumber;
	buf->c_checksum = u_ospcl.s_ospcl.c_checksum;
	buf->c_magic = u_ospcl.s_ospcl.c_magic;
	buf->bsd_c_dinode.bsd_di_mode = u_ospcl.s_ospcl.c_dinode.odi_mode;
	buf->bsd_c_dinode.bsd_di_nlink = u_ospcl.s_ospcl.c_dinode.odi_nlink;
	buf->bsd_c_dinode.bsd_di_uid = u_ospcl.s_ospcl.c_dinode.odi_uid;
	buf->bsd_c_dinode.bsd_di_gid = u_ospcl.s_ospcl.c_dinode.odi_gid;
	buf->bsd_c_dinode.bsd_di_size = u_ospcl.s_ospcl.c_dinode.odi_size;
	buf->bsd_c_dinode.bsd_di_rdev = u_ospcl.s_ospcl.c_dinode.odi_rdev;
	buf->bsd_c_dinode.bsd_di_atime = u_ospcl.s_ospcl.c_dinode.odi_atime;
	buf->bsd_c_dinode.bsd_di_mtime = u_ospcl.s_ospcl.c_dinode.odi_mtime;
	buf->bsd_c_dinode.bsd_di_ctime = u_ospcl.s_ospcl.c_dinode.odi_ctime;
	buf->c_count = u_ospcl.s_ospcl.c_count;
	bcopy(u_ospcl.s_ospcl.c_addr, buf->c_addr, (long)256);
	if (u_ospcl.s_ospcl.c_magic != OFS_MAGIC ||
	    checksum((int *)(&u_ospcl.s_ospcl)) == FAIL)
		return(FAIL);
	buf->c_magic = NFS_MAGIC;

good:
	j = buf->bsd_c_dinode.bsd_di_ic.ic_size.val;
	i = j[1];
	if (buf->bsd_c_dinode.bsd_di_size == 0 &&
	    (buf->bsd_c_dinode.bsd_di_mode & IFMT) == IFDIR && Qcvt==0) {
		if (*j || i) {
			printf(MSGSTR(SWAPQ, "Note: Doing Quad swapping\n"));
			Qcvt = 1;
		}
	}
	if (Qcvt) {
		j[1] = *j; *j = i;
	}

	switch (buf->c_type) {

	case TS_CLRI:
	case TS_BITS:
		/*
		 * Have to patch up missing information in bit map headers
		 */
		buf->c_inumber = 0;
		buf->bsd_c_dinode.bsd_di_size = buf->c_count * TP_BSIZE;
		for (i = 0; i < buf->c_count; i++)
			buf->c_addr[i]++;
		break;

	case TS_TAPE:
	case TS_END:
		buf->c_inumber = 0;
		break;

	case TS_INODE:
	case TS_ACL:
	case TS_PCL:
	case TS_ADDR:
		break;

	default:
		panic(MSGSTR(GETHEAD, "gethead: unknown inode type %d\n"), buf->c_type);
		break;
	}
	if (dflag)
		accthdr(buf);
	return(GOOD);
}

/*
 * Check that a header is where it belongs and predict the next header
 */
accthdr(header)
	struct s_spcl *header;
{
	static ino_t previno = 0x7fffffff;
	static int prevtype;
	static long predict;
	long blks, i;

	if (header->c_type == TS_TAPE) {
		fprintf(stderr, MSGSTR(VOLHEAD, "Volume header\n"));
		return;
	}
	if (previno == 0x7fffffff)
		goto newcalc;
	switch (prevtype) {
	case TS_BITS:
		fprintf(stderr, MSGSTR(MASKH, "Dump mask header"));
		break;
	case TS_CLRI:
		fprintf(stderr, MSGSTR(REMMASK,"Remove mask header"));
		break;
	case TS_INODE:
		fprintf(stderr, MSGSTR(FILEHE, "File header, ino %d"), previno);
		break;
	case TS_ADDR:
		fprintf(stderr, MSGSTR(FILECON,"File continuation header, ino %d"), previno);
		break;
	case TS_END:
		fprintf(stderr, MSGSTR(EOAHE, "End of archive header"));
		break;
	}
	if (predict != blksread - 1)
		fprintf(stderr, MSGSTR(PREDICT,
			"; predicted %d blocks, got %d blocks"), predict, blksread - 1);
	fprintf(stderr, MSGSTR(PERIOD, "\n"));
newcalc:
	blks = 0;
	if (header->c_type != TS_END)
		for (i = 0; i < header->c_count; i++)
			if (header->c_addr[i] != 0)
				blks++;
	predict = blks;
	blksread = 0;
	prevtype = header->c_type;
	previno = header->c_inumber;
}

/*
 * Find an inode header.
 * Complain if had to skip, and complain is set.
 */
findinode(header, complain)
	struct s_spcl *header;
	int complain;
{
	static long skipcnt = 0;

	curfile.name = "<name unknown>";
	curfile.action = UNKNOWN;
	curfile.dip = (struct bsd_dinode *)NIL;
	curfile.ino = 0;
	if (ishead(header) == FAIL) {
		skipcnt++;
		while (gethead(header) == FAIL)
			skipcnt++;
	}
	for (;;) {
		if (checktype(header, TS_INODE) == GOOD) {
			curfile.dip = &header->bsd_c_dinode;
			curfile.ino = header->c_inumber;
			break;
		}
		if (checktype(header, TS_END) == GOOD) {
			curfile.ino = maxino;
			break;
		}
		if (checktype(header, TS_CLRI) == GOOD) {
			curfile.name = "<file removal list>";
			break;
		}
		if (checktype(header, TS_BITS) == GOOD) {
			curfile.name = "<file dump list>";
			break;
		}
		while (gethead(header) == FAIL)
			skipcnt++;
	}
	if (skipcnt > 0 && complain)
		fprintf(stderr, MSGSTR(RESYNCR, "resync restore, skipped %d blocks\n"), skipcnt);
	skipcnt = 0;
}

/*
 * return whether or not the buffer contains a header block
 */
ishead(buf)
	struct s_spcl *buf;
{

	if (buf->c_magic != NFS_MAGIC)
		return(FAIL);
	return(GOOD);
}

checktype(b, t)
	struct s_spcl *b;
	int	t;
{

	if (b->c_type != t)
		return(FAIL);
	return(GOOD);
}

checksum(b)
	register int *b;
{
	register int i, j;

	j = sizeof(union u_spcl) / sizeof(int);
	i = 0;
	if(!Bcvt) {
		do
			i += *b++;
		while (--j);
	} else {
		/* What happens if we want to read restore tapes
			for a 16bit int machine??? */
		do 
			i += swabl(*b++);
		while (--j);
	}
			
	if (i != CHECKSUM) {
		fprintf(stderr, MSGSTR(CHKSUM, "Checksum error %o, inode %d file %s\n"), i, curfile.ino, curfile.name);
		return(FAIL);
	}
	return(GOOD);
}

#ifdef RRESTORE
/* VARARGS1 */
msg(cp, a1, a2, a3)
	char *cp;
{

	fprintf(stderr, cp, a1, a2, a3);
}
#endif RRESTORE

swabst(cp, sp)
register char *cp, *sp;
{
	int n = 0;
	char c;
	while(*cp) {
		switch (*cp) {
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = (n * 10) + (*cp++ - '0');
			continue;
		
		case 's': case 'w': case 'h':
			c = sp[0]; sp[0] = sp[1]; sp[1] = c;
			sp++;
			break;

		case 'l':
			c = sp[0]; sp[0] = sp[3]; sp[3] = c;
			c = sp[2]; sp[2] = sp[1]; sp[1] = c;
			sp += 3;
		}
		sp++; /* Any other character, like 'b' counts as byte. */
		if (n <= 1) {
			n = 0; cp++;
		} else
			n--;
	}
}
swabl(x) { unsigned long l = x; swabst("l", (char *)&l); return l; }

isdiskette(media)
char *media;
{

	int fd;
	char *ptr;
	
	if (!strcmp(media, "-"))
		return(0);
	fd = open(media, 0);
#ifndef RRESTORE
						/* Could be a range */
	if ((fd < 0) && ((ptr = strrchr(media, '-')) != NULL)) {
		*ptr = '\0';
		fd = open(media, 0);
		*ptr = '-';
	}
#endif
	if (fd < 0) {
		fprintf(stderr, MSGSTR(MEDIAOPEN, "Can't open %s\n"), media);
		done(1);
	}
	if (ioctl(fd, IOCINFO, &devinfo) < 0) {
		struct stat statbuf;
		if(fstat(fd, &statbuf) < 0) {
			fprintf(stderr, MSGSTR(IOCFAIL, "Can't get info on %s\n"), media);
			close(fd);
			done(1);
		}
		return(0);	/* just a regular file */
	}
	if ((devinfo.devtype == DD_DISK) && ((devinfo.flags & DF_FIXED) == 0)) {
		close(fd);
		return (1);
	}
	close(fd);
	return (0);
}

struct acl *
getacl(buf)
	struct s_spcl *buf;
{
	static char aclbuf[NTREC][TP_BSIZE];
	int i, curblk;

	curblk=0;

	for (i = 0; i < buf->c_count; i++)
		readtape(&aclbuf[curblk++][0]);

	gethead(&spcl);
	return((struct acl *)&aclbuf[0][0]);
}

struct pcl *
getpcl(buf)
	struct s_spcl *buf;
{
	static char pclbuf[NTREC][TP_BSIZE];
	int i, curblk;

	curblk=0;

	for (i = 0; i < buf->c_count; i++)
		readtape(&pclbuf[curblk++][0]);

	gethead(&spcl);
	return((struct pcl *)&pclbuf[0][0]);
}

