static char sccsid[] = "@(#)96	1.16.1.3  src/bos/usr/sbin/rrestore/dirs.c, cmdarch, bos411, 9428A410j 6/10/94 16:59:38";
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
static char sccsid[] = "(#)dirs.c	5.4 (Berkeley) 4/23/86";
#endif not lint
*/

#include "restore.h"
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/audit.h>
#include <sys/priv.h>

/*
 * Symbol table of directories read from tape.
 */
#define DIRBLKSIZ 512
#define HASHSIZE	1000
#define INOHASH(val) (val % HASHSIZE)
struct inotab {
	struct inotab *t_next;
	ino_t	t_ino;
	daddr_t	t_seekpt;
	long t_size;
};
static struct inotab *inotab[HASHSIZE];
extern struct inotab *inotablookup();
extern struct inotab *allocinotab();
extern struct acl *getacl();
extern struct pcl *getpcl();
struct acl *rdacl();
struct pcl *rdpcl();

/*
 * Information retained about directories.
 */
struct modeinfo {
	ino_t ino;
	time_t timep[2];
	ushort mode;
	ushort uid;
	ushort gid;
};

/*
 * Global variables for this file.
 */
static daddr_t	seekpt;
static FILE	*df, *mf;
static DIR	*dirp;
static char	dirfile[32] = "#";	/* No file */
static char	modefile[32] = "#";	/* No file */
extern ino_t	search();
struct direct	*rst_readdir();
DIR	*my_opendir();
long	my_telldir();
extern void 	rst_seekdir();

/*
 * Format of old style directories.
 */
#define ODIRSIZ 14
struct odirect {
	u_short	d_ino;
	char	d_name[ODIRSIZ];
};

/*
 *	Extract directory contents, building up a directory structure
 *	on disk for extraction by name.
 *	If genmode is requested, save mode, owner, and times for all
 *	directories on the tape.
 */
extractdirs(genmode)
	int genmode;
{
	register int i;
	register struct bsd_dinode *ip;
	struct inotab *itp;
	struct acl *aclp;
	struct pcl *pclp;
	struct direct nulldir;
	int putdir(), null();
	char dot[] = ".";

	vprintf(stdout, MSGSTR(EXTRDIR, "Extract directories from media\n"));
	(void) sprintf(dirfile, "/tmp/rstdir%d", dumpdate);
	df = fopen(dirfile, "w");
	if (df == 0) {
		fprintf(stderr,
		    MSGSTR(CNCRDIR, "restore: %s - cannot create directory temporary\n"),
		    dirfile);
		perror(MSGSTR(FOPENE, "fopen"));
		done(1);
	}
	if (genmode != 0) {
		(void) sprintf(modefile, "/tmp/rstmode%d", dumpdate);
		mf = fopen(modefile, "w");
		if (mf == 0) {
			fprintf(stderr,
			    MSGSTR(CNCRMO, "restore: %s - cannot create modefile \n"),
			    modefile);
			perror(MSGSTR(FOPENE, "fopen"));
			done(1);
		}
	}
	nulldir.d_ino = 0;
	nulldir.d_namlen = 1;
	(void) strcpy(nulldir.d_name, "/");
	nulldir.d_reclen = DIRSIZ(&nulldir);
	for (;;) {
		curfile.name = "<directory file - name unknown>";
		curfile.action = USING;
		ip = curfile.dip;
		if (ip == NULL || (ip->bsd_di_mode & IFMT) != IFDIR) {
			(void) fclose(df);
			dirp = my_opendir(dirfile);
			if (dirp == NULL)
				perror(MSGSTR(OPENDIR, "opendir"));
			if (mf != NULL)
				(void) fclose(mf);
			i = dirlookup(dot);
			if (i == 0)
				panic(MSGSTR(NOROOT, "Root directory is not on media\n"));
			return;
		}
		itp = allocinotab(curfile.ino, ip, seekpt);

		getfile(putdir, null, 0);

		if (checktype(&spcl, TS_ACL) == GOOD) 
			aclp = getacl(&spcl);
		else 
			aclp = (struct acl *)NULL;

		if (checktype(&spcl, TS_PCL) == GOOD) 
			pclp = getpcl(&spcl);
		else 
			pclp = (struct pcl *)NULL;

		if (genmode != 0) 
			putsec(aclp, pclp);

		findinode(&spcl, 1);
		putent(&nulldir);
		flushent();
		itp->t_size = seekpt - itp->t_seekpt;
	}
}

/*
 * skip over all the directories on the tape
 */
skipdirs()
{

	while ((curfile.dip->bsd_di_mode & IFMT) == IFDIR) {
		skipfile(1);
	}
}

/*
 *	Recursively find names and inumbers of all files in subtree 
 *	pname and pass them off to be processed.
 */
treescan(pname, ino, todo)
	char *pname;
	ino_t ino;
	long (*todo)();
{
	register struct inotab *itp;
	register struct direct *dp;
	register struct entry *np;
	int namelen;
	daddr_t bpt;
	char locname[MAXPATHLEN + 1];

	itp = inotablookup(ino);
	if (itp == NULL) {
		/*
		 * Pname is name of a simple file or an unchanged directory.
		 */
		(void) (*todo)(pname, ino, LEAF);
		return;
	}
	/*
	 * Pname is a dumped directory name.
	 */
	if ((*todo)(pname, ino, NODE) == FAIL)
		return;
	/*
	 * begin search through the directory
	 * skipping over "." and ".."
	 */
	(void) strncpy(locname, pname, (size_t)MAXPATHLEN);
	(void) strncat(locname, "/", (size_t)MAXPATHLEN);
	namelen = strlen(locname);
	rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
	dp = rst_readdir(dirp); /* "." */
	if (dp != NULL && strcmp(dp->d_name, ".") == 0)
		dp = rst_readdir(dirp); /* ".." */
	else
		fprintf(stderr, MSGSTR(MISSDOT, "Warning: `.' missing from directory %s\n"),
			pname);
	if (dp != NULL && strcmp(dp->d_name, "..") == 0)
		dp = rst_readdir(dirp); /* first real entry */
	else
		fprintf(stderr, MSGSTR(MDOTDOT, "Warning: `..' missing from directory %s\n"),
			pname);
	bpt = my_telldir(dirp);
	/*
	 * a zero inode signals end of directory
	 */
	while (dp != NULL && dp->d_ino != 0) {
		locname[namelen] = '\0';
		if (namelen + dp->d_namlen >= MAXPATHLEN) {
			fprintf(stderr, MSGSTR(NAMLONG, "%s%s: name exceeds %d char\n"),
				locname, dp->d_name, MAXPATHLEN);
		} else {
			(void) strncat(locname, dp->d_name, (size_t)dp->d_namlen);
			treescan(locname, dp->d_ino, todo);
			rst_seekdir(dirp, bpt, itp->t_seekpt);
		}
		dp = rst_readdir(dirp);
		bpt = my_telldir(dirp);
	}
	if (dp == NULL)
		fprintf(stderr, MSGSTR(CORRDIR, "corrupted directory: %s.\n"), locname);
}

/*
 * Search the directory tree rooted at inode ROOTINO
 * for the path pointed at by n
 */
ino_t
psearch(n)
	char	*n;
{
	register char *cp, *cp1;
	ino_t ino;
	char c;

	ino = ROOTINO;
	if (*(cp = n) == '/')
		cp++;
next:
	cp1 = cp + 1;
	while (*cp1 != '/' && *cp1)
		cp1++;
	c = *cp1;
	*cp1 = 0;
	ino = search(ino, cp);
	if (ino == 0) {
		*cp1 = c;
		return(0);
	}
	*cp1 = c;
	if (c == '/') {
		cp = cp1+1;
		goto next;
	}
	return(ino);
}

/*
 * search the directory inode ino
 * looking for entry cp
 */
ino_t
search(inum, cp)
	ino_t	inum;
	char	*cp;
{
	register struct direct *dp;
	register struct inotab *itp;
	int len;

	itp = inotablookup(inum);
	if (itp == NULL)
		return(0);
	rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
	len = strlen(cp);
	do {
		dp = rst_readdir(dirp);
		if (dp == NULL || dp->d_ino == 0)
			return (0);
	} while (dp->d_namlen != len || strncmp(dp->d_name, cp, len) != 0);
	return(dp->d_ino);
}

/*
 * Put the directory entries in the directory file
 */
putdir(buf, size)
	char *buf;
	int size;
{
	struct direct cvtbuf;
	register struct odirect *odp;
	struct odirect *eodp;
	register struct direct *dp;
	long loc, i;
	extern int Bcvt;

	if (cvtflag) {
		eodp = (struct odirect *)&buf[size];
		for (odp = (struct odirect *)buf; odp < eodp; odp++)
			if (odp->d_ino != 0) {
				dcvt(odp, &cvtbuf);
				putent(&cvtbuf);
			}
	} else {
		for (loc = 0; loc < size; ) {
			dp = (struct direct *)(buf + loc);
			if (Bcvt) {
				swabst("l2s", (char *) dp);
			}
			i = DIRBLKSIZ - (loc & (DIRBLKSIZ - 1));
			if (dp->d_reclen == 0 || dp->d_reclen > i) {
				loc += i;
				continue;
			}
			loc += dp->d_reclen;
			if (dp->d_ino != 0) {
				putent(dp);
			}
		}
	}
}

/*
 * These variables are "local" to the following two functions.
 */
char dirbuf[DIRBLKSIZ];
long dirloc = 0;
long prev = 0;

/*
 * add a new directory entry to a file.
 */
putent(dp)
	struct direct *dp;
{
	dp->d_reclen = DIRSIZ(dp);
	if (dirloc + dp->d_reclen > DIRBLKSIZ) {
		((struct direct *)(dirbuf + prev))->d_reclen =
		    DIRBLKSIZ - prev;
		(void) fwrite((void *)dirbuf, (size_t)1, (size_t)DIRBLKSIZ, df);
		dirloc = 0;
	}
	bcopy((char *)dp, dirbuf + dirloc, (long)dp->d_reclen);
	prev = dirloc;
	dirloc += dp->d_reclen;
}

/*
 * flush out a directory that is finished.
 */
flushent()
{

	((struct direct *)(dirbuf + prev))->d_reclen = DIRBLKSIZ - prev;
	(void) fwrite((void *)dirbuf, (size_t)dirloc, (size_t)1, df);
	seekpt = ftell(df);
	dirloc = 0;
}

dcvt(odp, ndp)
	register struct odirect *odp;
	register struct direct *ndp;
{

	bzero((char *)ndp, (long)(sizeof *ndp));
	ndp->d_ino =  odp->d_ino;
	(void) strncpy(ndp->d_name, odp->d_name, (size_t)ODIRSIZ);
	ndp->d_namlen = strlen(ndp->d_name);
	ndp->d_reclen = DIRSIZ(ndp);
}

/*
 * Seek to an entry in a directory.
 * Only values returned by ``telldir'' should be passed to rst_seekdir.
 * This routine handles many directories in a single file.
 * It takes the base of the directory in the file, plus
 * the desired seek offset into it.
 */
void
rst_seekdir(dirp, loc, base)
	register DIR *dirp;
	daddr_t loc, base;
{

	if (loc == my_telldir(dirp))
		return;
	loc -= base;
	if (loc < 0)
		fprintf(stderr, MSGSTR(BADSEEKP, "bad seek pointer to rst_seekdir %d\n"), loc);
	(void) lseek(dirp->dd_fd, base + (loc & ~(DIRBLKSIZ - 1)), 0);
	dirp->dd_loc = loc & (DIRBLKSIZ - 1);
	if (dirp->dd_loc != 0)
		dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, DIRBLKSIZ);
}

/*
 * get next entry in a directory.
 */
struct direct *
rst_readdir(dirp)
	register DIR *dirp;
{
	register struct direct *dp;

	for (;;) {
		if (dirp->dd_loc == 0) {
			dirp->dd_size = read(dirp->dd_fd, dirp->dd_buf, 
			    DIRBLKSIZ);
			if (dirp->dd_size <= 0) {
				dprintf(stderr, "error reading directory\n");
				return NULL;
			}
		}
		if (dirp->dd_loc >= dirp->dd_size) {
			dirp->dd_loc = 0;
			continue;
		}
		dp = (struct direct *)(dirp->dd_buf + dirp->dd_loc);
		if (dp->d_reclen == 0 ||
		    dp->d_reclen > DIRBLKSIZ + 1 - dirp->dd_loc) {
			dprintf(stderr, "corrupted directory: bad reclen %d\n",
				dp->d_reclen);
			return NULL;
		}
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0 && strcmp(dp->d_name, "/") != 0)
			continue;
		if (dp->d_ino >= maxino) {
			dprintf(stderr, "corrupted directory: bad inum %d\n",
				dp->d_ino);
			continue;
		}
		return (dp);
	}
}

/*
 * Simulate the opening of a directory
 */
DIR *
rst_opendir(name)
	char *name;
{
	struct inotab *itp;
	ino_t ino;

	if ((ino = dirlookup(name)) > 0 &&
	    (itp = inotablookup(ino)) != NULL) {
		rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
		return (dirp);
	}
	return (0);
}

/*
 * Set the mode, owner, and times for all new or changed directories
 */
setdirmodes()
{
	FILE *mdfile;
	struct modeinfo node;
	struct entry *ep;
	char *cp;
	struct sac_rec sechdr;
	struct acl *aclp;
	struct pcl *pclp;

	
	vprintf(stdout, MSGSTR(SETMOT, "Set directory mode, owner, and times.\n"));
	(void) sprintf(modefile, "/tmp/rstmode%d", dumpdate);
	mdfile = fopen(modefile, "r");
	if (mdfile == NULL) {
		perror(MSGSTR(FOPENE, "fopen"));
		fprintf(stderr, MSGSTR(NOMODEFI, "cannot open mode file %s\n"), modefile);
		fprintf(stderr, MSGSTR(MODENOTS, "directory mode, owner, and times not set\n"));
		return;
	}
	clearerr(mdfile);
	for (;;) {
		(void) fread((void *)&node, (size_t)1, (size_t)sizeof(struct modeinfo), mdfile);
		if (feof(mdfile))
			break;


		fread(&sechdr, sizeof(sechdr), 1, mdfile);
		if (sechdr.aclsize != 0) 
			aclp = rdacl(mdfile, sechdr.aclsize);
		else
			aclp = (struct acl *)NULL;
		if (sechdr.pclsize != 0) 
			pclp = rdpcl(mdfile, sechdr.pclsize);
		else
			pclp = (struct pcl *)NULL;


		ep = lookupino(node.ino);
		if (command == 'i' || command == 'x') {
			if (ep == NIL)
				continue;
			if (ep->e_flags & EXISTED) {
				ep->e_flags &= ~NEW;
				continue;
			}
			if (node.ino == ROOTINO &&
		   	    reply(MSGSTR(SETDOT, "set owner/mode for '.'")) == FAIL)
				continue;
		}
		if (ep == NIL)
			panic(MSGSTR(NODIRI, "cannot find directory inode %d\n"), node.ino);
		cp = myname(ep);
		(void) chown(cp, node.uid, node.gid);

		if (aclp != (struct acl *)NULL) {
			if ((chacl(cp, aclp, aclp->acl_len) == -1) && (errno != EPERM)) {
				fprintf(stderr, MSGSTR(EACLIMP, "Import of ACLs failed\n"));
				perror("");
			free(aclp);
			} 
		}
		else
			(void) chmod(cp, (mode_t)node.mode);

		if (pclp != (struct pcl *)NULL) {
			if ((chpriv(cp, pclp, pclp->pcl_len) == -1) && (errno != EPERM)) {

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
					cp);
				if ((errno = save_errno) != EBUSY)
					perror("");
			} 
			free(pclp);
		}

		utime(cp, node.timep);
		ep->e_flags &= ~NEW;
	}
	if (ferror(mdfile))
		panic(MSGSTR(ESETDIR, "error setting directory modes\n"));
	(void) fclose(mdfile);
}

/*
 * Generate a literal copy of a directory.
 */
genliteraldir(name, ino)
	char *name;
	ino_t ino;
{
	register struct inotab *itp;
	int ofile, dp, i, size;
	char buf[BUFSIZ];

	itp = inotablookup(ino);
	if (itp == NULL)
		panic(MSGSTR(NODIRINO, "Cannot find directory inode %d named %s\n"), ino, name);
	if ((ofile = creat(name, (mode_t)0666)) < 0) {
		fprintf(stderr, "%s: ", name);
		(void) fflush(stderr);
		perror(MSGSTR(CANTCRF, "cannot create file"));
		return (FAIL);
	}
	rst_seekdir(dirp, itp->t_seekpt, itp->t_seekpt);
	dp = dup(dirp->dd_fd);
	for (i = itp->t_size; i > 0; i -= BUFSIZ) {
		size = i < BUFSIZ ? i : BUFSIZ;
		if (read(dp, buf, (int) size) == -1) {
			fprintf(stderr,
				MSGSTR(EWRITINO, "write error extracting inode %d, name %s\n"),
				curfile.ino, curfile.name);
			perror(MSGSTR(READ, "read"));
			done(1);
		}
		if (write(ofile, buf, (int) size) == -1) {
			fprintf(stderr,
				MSGSTR(EWRITINO, "write error extracting inode %d, name %s\n"),
				curfile.ino, curfile.name);
			perror(MSGSTR(WRITE, "write"));
			done(1);
		}
	}
	(void) close(dp);
	(void) close(ofile);
	return (GOOD);
}

/*
 * Determine the type of an inode
 */
inodetype(ino)
	ino_t ino;
{
	struct inotab *itp;

	itp = inotablookup(ino);
	if (itp == NULL)
		return (LEAF);
	return (NODE);
}

/*
 * Allocate and initialize a directory inode entry.
 * If requested, save its pertinent mode, owner, and time info.
 */
struct inotab *
allocinotab(ino, dip, seekpt)
	ino_t ino;
	struct bsd_dinode *dip;
	daddr_t seekpt;
{
	register struct inotab	*itp;
	struct modeinfo node;

	itp = (struct inotab *)calloc((size_t)1, (size_t)sizeof(struct inotab));
	if (itp == 0)
		panic(MSGSTR(NOMEMTAB, "no memory directory table\n"));
	itp->t_next = inotab[INOHASH(ino)];
	inotab[INOHASH(ino)] = itp;
	itp->t_ino = ino;
	itp->t_seekpt = seekpt;
	if (mf == NULL)
		return(itp);
	node.ino = ino;
	node.timep[0] = dip->bsd_di_atime;
	node.timep[1] = dip->bsd_di_mtime;
	node.mode = dip->bsd_di_mode;
	node.uid = dip->bsd_di_uid;
	node.gid = dip->bsd_di_gid;
	(void) fwrite((void *)&node, (size_t)1, (size_t)sizeof(struct modeinfo), mf);
	return(itp);
}

/*
 * Look up an inode in the table of directories
 */
struct inotab *
inotablookup(ino)
	ino_t	ino;
{
	register struct inotab *itp;

	for (itp = inotab[INOHASH(ino)]; itp != NULL; itp = itp->t_next)
		if (itp->t_ino == ino)
			return(itp);
	return ((struct inotab *)0);
}

/*
 * Clean up and exit
 */
done(exitcode)
	int exitcode;
{

	closemt();
	if (modefile[0] != '#')
		(void) unlink(modefile);
	if (dirfile[0] != '#')
		(void) unlink(dirfile);
	exit(exitcode);
}

/*
 * open a directory.
 */
DIR *
my_opendir(name)
	char *name;
{
	register DIR *dirp;
	register int fd;

	if ((fd = open(name, 0)) == -1)
		return NULL;
	if ((dirp = (DIR *)malloc((size_t)sizeof(DIR))) == NULL) {
		close (fd);
		return NULL;
	}
	dirp->dd_fd = fd;
	dirp->dd_loc = 0;
	return dirp;
}

long
my_telldir(dirp)
	DIR *dirp;
{
	return (lseek(dirp->dd_fd, 0L, 1) - dirp->dd_size + dirp->dd_loc);
}


putsec(aclp, pclp)
struct acl *aclp;
struct pcl *pclp;
{
	struct sac_rec sechdr;

	sechdr.aclsize = aclp->acl_len;
	sechdr.pclsize = pclp->pcl_len;
	fwrite(&sechdr, sizeof(sechdr), 1, mf);
	fwrite(aclp, aclp->acl_len, 1, mf); 
	fwrite(pclp, pclp->pcl_len, 1, mf); 
}

struct acl *
rdacl(mdfile, aclsize)
FILE *mdfile;
ulong aclsize;
{
	struct acl *aclp;

	aclp = (struct acl *)malloc(aclsize);
	fread(aclp, aclsize, 1, mdfile);
	return(aclp);
}

struct pcl *
rdpcl(mdfile, pclsize)
FILE *mdfile;
ulong pclsize;
{
	struct pcl *pclp;

	pclp = (struct pcl *)malloc(pclsize);
	fread(pclp, pclsize, 1, mdfile);
	return(pclp);
}

