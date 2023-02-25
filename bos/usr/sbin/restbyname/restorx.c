static char sccsid[] = "@(#)60	1.54.1.13  src/bos/usr/sbin/restbyname/restorx.c, cmdarch, bos411, 9436D411a 9/8/94 15:09:49";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: restorx
 *
 * ORIGINS: 3, 27, 9
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define MAXINO  3000
#define MAXBLOCKS	10

char ROOT[] = "/";      /* name of root directory with inode backup */

#include <stdio.h>
#include <utime.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/dir.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <dumprestor.h>
#include <ar.h>
#include <sys/acl.h>
#include <sys/priv.h>
#include <fnmatch.h>

extern int	must_audit;
/*
 * the extraction list keeps track of the various I-nodes
 *     we are supposed to be interested in.
 * we use the clrimap to avoid running out of space.
 */
#define xtrmap clrimap

/*
 * the inode table is used to keep track of the inodes for each directory
 *     on the backup tape.  dirfile is where we keep the dir data
 *     so we can trace directory trees.
 */
struct inotab {
	ino_t   t_ino;
	off_t   t_seekp;
} inotab[MAXINO];
int ipos;                       /* next open slot */

char    dirfile[] = "/tmp/rstXXXXXX";
char    lname[] = "/tmp/rsuXXXXXX";
char	REPLACE_NAME[] = "RESTORE:ALREADY_EXTRACTED";
FILE    *tmpf;
char    armag_buf[SAIAMAG + 1];          /* buf for archive magic string */
char    syscmd[81];                     /* argument for system cmd      */

/* maps */
extern unsigned char clrimap[];         /* bit map of cleared files */
extern unsigned char dumpmap[];         /* bit map of backed up files */

#include "rbyname_msg.h"
extern nl_catd catd;
#define MSGSTR(N,S) catgets(catd,MS_RBYNAME,N,S)

#ifdef DEBUG
extern int debug;               /* flag requesting debugging output     */
#endif
extern int realuid;
extern ushort magic;            /* magic number to look for */
extern int med_fd;              /* input file descriptor for medium     */
       int ofile;               /* output file descriptor for xtr files */
extern int verbose;             /* flag requesting verbose output       */
extern int showbytes;           /* show bytes when listing files        */
extern int not_stdio;           /* flag - input from stdin        p26447*/
extern int volno;               /* tape volume number expected          */
extern unsigned files_restored; /* number of files restored             */
extern int dflag;               /* restore directory subtree?           */
extern int hflag;               /* modify file and archive timestamps?  */
       int needed;              /* number of files needed               */

static int XtrEmsg;             /* used to prevent multiple error messages */
static char *XtrName;           /* for communication with error routine */

extern union fs_rec *thishd;    /* pointer to last header read          */

extern int ExitCode;

char    *itoname(), *toname();
char    *trylink(), *mktemp();
void    exit();
long    lseek();
int     putdir(), null(), breakout(void);
int     xtrfile(), xtrskip();
static  XtrError();
ino_t   psearch(), search();

#define OLDDIRSIZ 14
/* The version 2 and previous inode backups have the
 * fixed length directory entries.
 */
struct	olddirect {
	ushort	d_ino;
	char	d_name[OLDDIRSIZ];
};

struct  olddirect nulldir =
{	0, "/"	};

struct packctl {
	int ptocfd[2];
	int ctopfd[2];
	int shmid;
	char *addr;
	int cpid;
} packctl;

struct r_comm                   /* Communications struct        */
{       int r_bsz;
	int r_eof;
};

int pinit = 1;

dirlist()
{
	int listfile();

	/* read in all of the directories       */
	pass1();

	/* list out all of the directories      */
	traverse( ROOTINO, NULL, listfile, NULL );

	/* get rid of our temp file */
	unlink( dirfile );
}

/* D49216 */
#define ERRNO_CHECK                           \
{                                            \
   if (errno == ENOSPC || errno == EDQUOT)    \
   {                                          \
       files_restored--;    /* don't count this one */ \
       perror("restbyname");  \
   }                              \
}

/*
 * extract the files
 * how is BYNAME or BYINODE (how files were backed up)
 * np is the list of files
 * if dflag set, it's a list of directories instead
 * np == NULL means give a table of contents only
 * np == "" means extract all files on tape (BYNAME only)
 */
extract(how, np)
 char **np;
 {
	char gotfile;           /* did we get this file? */
	char getit;             /* do we want this file? */
	char *name;
        char savname[NAMESZ];   /* D73127 increase size of savname to
                                   accommodate pathnames of length NAMESZ */
	off_t cursize;          /* size of file being extracted. */
	off_t totsize=0;        /* Total of all files extracted. */
	int verb;		/* previous value of verbose flag */
	extern int rflag;	/* set when -q was given */
/* Security */
	struct acl *tmpaclp;
	struct acl *aclp = NULL;
	struct pcl *tmppclp;
	struct pcl *pclp = NULL;

/* AUDIT */
	char	*name_on_tape;


	/*
	 * if BYINODE, find directories and names, then announce
	 * the correspondence.  rewind tape, and start over.
	 *
     * This is for the version 2 byinode backups not version 3
	 * dumps.
	 */
	if (how == BYINODE) {
		pass1();                /* get dirs */
		findi(np);             /* get inumbers, set needed */
		if( needed )		/* do we need any files? */
		{
			/*
			* If the first volume is still loaded,
			* (ie. volno == 1), then call newmedium
			* with an arg of 0.  This causes get_unit
			* not to prompt to insert the tape/diskette.
			* Else, pass newmedium an argument of 1
			* to cause the user to be prompted to
			* re-insert the first volume.
			*/
			if( volno == 1 )
			{
				/*
				* By temporarily clearing the verbose
				* flag, the user avoids seeing the
				* "New volume" messages in newmedium().
				*/
				verb = verbose;
				verbose = 0;
				newmedium(0);
				verbose = verb;
			}
			else
			{
				/*
				* The -q flag on the command line
				* has no meaning now that a second
				* volume has been examined; clear it.
				*/
				rflag = 0;
				volno = 1;
				newmedium(1);
			}
		}
		if (not_stdio && verbose)
		   printf(MSGSTR(FTOBE,"Files to be extracted: %d\n"), needed);
	}
	else  /* BYNAME */
		needed = countfiles(np);

	/*
	 * go through the tape, extracting needed files
	 */

	gethead(0);

	while( needed )
	{
		checkhead();

		/*
		 * we now have the next header off of the tape
		 */

		switch( thishd->h.type )
		{ 
		  case FS_CLRI:
			readbits( NULL );
			break;

		  case FS_FINDEX:
			gethead(0);
			break;

		  case FS_BITS:
			readbits( dumpmap );
			break;

		  case FS_END:
			needed = 0;
			break;

		  case FS_NAME_X:
		  {
			/* Security header information is not written for symbolic link */
			if ((thishd->nx.mode & IFMT) != S_IFLNK) {
				struct sac_rec *sp;
				ulong aclsize, pclsize;

				sp = (struct sac_rec *)readmedium(btow(sizeof(struct sac_rec)));
				aclsize = rlong(sp->aclsize);
				pclsize = rlong(sp->pclsize);

				tmpaclp = (struct acl *)readmedium(aclsize);
				reorderacl(tmpaclp);
				aclp = malloc (tmpaclp->acl_len);
				memcpy (aclp, tmpaclp, ((struct acl *)tmpaclp)->acl_len);

				tmppclp = (struct pcl *)readmedium(pclsize);
				reorderpcl(tmppclp);
				pclp = malloc (tmppclp->pcl_len);
				memcpy (pclp, tmppclp, ((struct pcl *)tmppclp)->pcl_len);

			}
		  }	
		  case FS_ONAME:
		  case FS_OINODE:
		  case FS_NAME:
		  case FS_DINODE:
			gotfile = 0;
			getit = 1;
			if (how == BYINODE) {
				name = itoname((ino_t)thishd->i.ino);
				cursize = thishd->i.size;
				if (name == NULL)       /* not wanted */
					getit = 0;
			}
			else {
				if (thishd->h.type == FS_NAME_X) {
					name = thishd->nx.name;
					cursize = thishd->nx.size;
				} else if (thishd->h.type == FS_NAME) {
					name = thishd->n.name;
					cursize = thishd->n.size;
				} else {
					name = thishd->on.name;
					cursize = thishd->on.size;
				}

				if (!wanted(name, np)) {
					/* table of contents? */
					getit = 0;
					if (np == NULL) {
						if (showbytes)
							printf("%12d %s\n", cursize, name);
						else
							printf("%s\n", name);
						totsize += cursize;
						++files_restored;
					}
				}
			}
			if (getit) {
				char *lname;
				if (lname = trylink(thishd)) {
					if (showbytes)
						printf("x %12d ", cursize);
					else
						printf("x ");
					printf(MSGSTR(LNKTO, 
						"%s\nlinked to %s\n"), name, lname);
					totsize += cursize;
					gotfile = 0;
					++files_restored;
				} 
				else {
					/*
					 * AUDIT RECORD ("import")
					 * not link ...
					 * name_on_tape = ((how == BYINODE && !dflag) ? toname(inum) : name)
					 * new = "name"
					 */
					if (must_audit) {
						if ((how == BYINODE) && !dflag)
		   					name_on_tape = toname((ino_t)thishd->i.ino);
						else
		   					name_on_tape = name;
					}
					strcpy(savname, name);
					gotfile = xtr1(thishd, name, aclp, pclp);
					if (!gotfile)
						ExitCode=1;
					else {
						if (showbytes)
							printf("x %12d %s\n", cursize, savname);
						else
							printf("x %s\n", savname);
						totsize += cursize;
					}
				}
				if (needed != DONTCOUNT)
					--needed;
			}
			if (gotfile)
				++files_restored;
			else
				getfile(thishd, null, null);
			break;

		  default:
			errmsg(MSGSTR(UNKNHD,
				"Unknown header type %d - ignored\n"), thishd->h.type );
			gethead(0);
			break;
		}
	}
	if (how == BYINODE)
		unlink(dirfile);
	else    unlink(lname);
	if (pinit == 0)
		packquit();
	if (showbytes && not_stdio)
		printf(MSGSTR(TOTSIZ, "    total size: %u\n"), totsize);
}


/*
 * given an inumber, return its name if we are supposed to extract
 * it.  if no -d, the list of files in xtrmap tells which we
 * want, and the name is just the inumber.  if -d ("directory subtree"),
 * we translate the inumber to a pathname (SLOW!).
 */
char *
itoname(n)
	register ino_t n; {

	if (BIT(n, xtrmap)) {

		if (dflag)
			return toname(n);
		else {
			static char name[20];

			sprintf(name, "%u", n);
			return name;
		}
	}
	return NULL;
}

/*
 * extract the single file whose header is pointed to by hp,
 * using name.  return success.
 */

xtr1(hp, name, aclp, pclp)
    register union fs_rec *hp;
    char *name;
    struct acl *aclp;
    struct pcl *pclp;
    {
	register success;
	register i, t;
	char buff[BSIZE];
	dev_t device;
	struct utimbuf times;
	char nmbuf[MAXPATH];
	union fs_rec hbuf;
	extern int Aflag;
	struct stat Symstat;

	/* getfile() can garbage out header *hp
	 * so repoint hp at a safe copy.
	 * a safe copy of name is made also.
	 *
	 * reason: (1) hp (always) and name (usually) point
	 *             within the i/p buffer.
	 *         (2) getfile calls readtape which can shift
	 *             down or overwrite the i/p buffer, causing
	 *             pointers to there to point to garbage.
	 */
	hbuf = *hp;
	hp = &hbuf;

	nmbuf[0] = '\0';

	strcat(nmbuf, name);

#ifdef  DEBUG
	if( getenv("XTR1") )
		fprintf(stderr,"xtr1: %s -> %s\n", name, nmbuf );
#endif  DEBUG

	name = nmbuf;
	t = hp->h.type;
	if (t == FS_NAME_X)
		i = (hp->nx.mode & IFMT);
	else 
		i = (hp->i.mode & IFMT);
	success = 0;

	switch(i) {

	  /* regular file -- just write data */
	  case S_IFREG:
	  regular:

		if ( Aflag == TRUE ) {
			char *ptr, path[MAXPATHLEN], file[_D_NAME_MAX];

			strcpy(file,"TMP_XXXXXX");
			mktemp(file);

			strcpy(path,name);
			if ( (ptr = strrchr(path,'/')) != NULL )
				*++ptr = NULL;
			else
				*path = NULL;

			strcat(path,file);

			if ( (ofile=open(path, O_CREAT|O_WRONLY|O_TRUNC, 0644)) < 0 ) {
				if (!checkdir(path))
					break;
				if ( (ofile=open(path, O_CREAT|O_WRONLY|O_TRUNC, 0644)) < 0 ) {
					fprintf(stderr, MSGSTR(TMPCREAT, 
						"Unable to open or create a temporary file to\n\t restore "),path);
					perror(name);
					break;
				}
			}
			if ( rename(path,name) < 0 ) {
				fprintf(stderr, MSGSTR(RENAME,
					"Unable to restore the file "));
				perror(name);
				unlink(path);
				break;
			}

		} else {
			ofile = open (name, O_CREAT|O_WRONLY|O_TRUNC, 0644);
			if (ofile < 0) {
				if ((t == FS_OINODE || t == FS_DINODE) && !dflag) {
					fprintf(stderr, MSGSTR(CREATE, 
						"Unable to open or create the file "));
					perror(name);
					break;
				}
				if (!checkdir(name))
					break;
				ofile = open (name, O_CREAT|O_WRONLY|O_TRUNC, 0644);
				if (ofile < 0) {
					fprintf(stderr, MSGSTR(CREATE, 
						"Unable to open or create the file "));
					perror(name);
					break;
				}
			}
		}

		if (t == FS_NAME_X) {
			if (aclp->acl_len)
				if ((fchacl(ofile, aclp, aclp->acl_len) == -1) && (errno != EPERM)){
					fprintf(stderr, MSGSTR(EACLIMP, "Import of ACLs failed\n"));
					perror("");
				}
			free(aclp);
			if (pclp->pcl_len) /* we basically ignore this failure as pcls are not implemented */
				if ((fchpriv(ofile, pclp, pclp->pcl_len) == -1) && (errno != EPERM) && (errno != EINVAL)) {
						fprintf(stderr, MSGSTR(EPCLIMP, "Import of PCLs failed\n"));
						perror("");
				}
			free(pclp);
			fchown(ofile, hp->nx.uid, hp->nx.gid);/*p26595*/
		}
		else {
			fchmod(ofile, (mode_t)hp->n.mode);
			fchown(ofile, (uid_t)hp->n.uid, (gid_t)hp->n.gid);    /*p26595*/
		}


		XtrEmsg=0;              /* no error reported on this file */
		XtrName=name;           /* name of file for error report */
		if (hp->h.magic == magic)
			getfile(hp, xtrfile, xtrskip);
		else
			getpfile(hp, name);
		errno = 0;
		close( ofile );
		ERRNO_CHECK

		/* if h option specified then determine if file is an   */
		/* archive, if it is, call the ar command to change     */
		/* the timestamps of all members.                       */

		if (hflag) {
			ofile = open(name,0);
			if ( read(ofile,armag_buf,SAIAMAG) == SAIAMAG )
			{
				close(ofile);
				if ( strncmp(armag_buf,AIAMAG,SAIAMAG) == 0 )
				{
					strcpy(syscmd,"ar h ");
					strcat(syscmd,name);
					strcat(syscmd," 1>&- 2>&- ");
					if ( system(syscmd) != 0 )
						perror(syscmd);
					}
				}
			else
				close( ofile );
		}

		/* if h option not specified, use timestamp from     */
		/* backup media, otherwise, keep timestamp from creat*/

		if (!hflag)                                   /*p26595*/
			if (t == FS_NAME_X) {
		   		times.actime = hp->nx.atime;
		   		times.modtime = hp->nx.mtime;
		   		utime(name, (struct utimbuf *)&times);
		   }
		   	else {
			   	times.actime = hp->n.atime;
		   		times.modtime = hp->n.mtime;
		  		utime(name, (struct utimbuf *)&times);
		   }                                            /*p26595*/
		
		++success;
		break;

	  /*
	   * directory -- extract as regular file if by INODE
	   * otherwise, make directory (no contents present)
	   */
	  case S_IFDIR:
		if ((t == FS_OINODE || t == FS_DINODE) && !dflag)
			goto regular;

		if (!checkdir(name) || !rest_mkdir(name))
			break;

		if (t == FS_NAME_X) {
			if (aclp->acl_len)
				if ((chacl(name, aclp, aclp->acl_len) == -1) && (errno != EPERM)) {
					fprintf(stderr, MSGSTR(EACLIMP, "Import of ACLs failed\n"));
					perror("");
				}
			free(aclp);
			if (pclp->pcl_len)
				if ((chpriv(name, pclp, pclp->pcl_len) == -1) && (errno != EPERM) && (errno != EINVAL)) {
					fprintf(stderr, MSGSTR(EPCLIMP, "Import of PCLs failed\n"));
					perror("");
				}
			free(pclp);
			chown(name, hp->nx.uid, hp->nx.gid);
		}
		else {
			chmod(name, (mode_t)hp->n.mode);
			chown(name, (uid_t)hp->n.uid, (gid_t)hp->n.gid);
		}
	
		/* if INODE, skip the contents */
		if (t == FS_OINODE || t == FS_DINODE)
			getfile(hp, null, null);
		else    gethead(0);
		++success;
		break;

	  case S_IFLNK:
	  sym_link:
		/* Check for a existance of "name" and try to remove it */
		if (lstat(name, &Symstat)<0) {
		    if (errno != ENOENT) {
			perror(name);
			break;
		    }
		} else {
		    /* Dont unlink a directory, use "rmdir" instead. */
		    /* Unlinking a directory will corrupt the filesystem. */
		    if ((Symstat.st_mode & S_IFMT)==S_IFDIR) { 
			if (rmdir(name)!=0) {
			    perror(name);
			    break;
			}
		    } else
			unlink(name);
		}
		ofile = creat(name, (mode_t)0644);
		if (ofile < 0) {
			if ((t == FS_OINODE || t == FS_DINODE) && !dflag) {
				perror(name);
				break;
			}
			if (!checkdir(name))
				break;
			ofile = creat(name, (mode_t)0644);
			if (ofile < 0) {
				perror(name);
				break;
			}
		}
		XtrEmsg=0;              /* no error reported on this file */
		XtrName=name;           /* name of file for error report */
		if (hp->h.magic == magic)
			getfile(hp, xtrfile, xtrskip);
		else
			getpfile(hp, name);
		errno = 0;
		close(ofile);
		ERRNO_CHECK
		if ((ofile = open(name,0)) < 0)
			fprintf(stderr, MSGSTR(BADOPEN, "Bad open \n"));
		if (t == FS_NAME_X) {
			if (read(ofile,buff,hp->nx.size) < 0) {
				fprintf(stderr, MSGSTR(BADREAD, "Bad read \n"));
				perror(name);
			}
			buff[hp->nx.size] = '\0';
		}
		else {
			if (read(ofile,buff,hp->n.size) < 0) {
				fprintf(stderr, MSGSTR(BADREAD, "Bad read \n"));
				perror(name);
			}
			buff[hp->n.size] = '\0';
		}
		close( ofile );

		unlink(name);
		if (symlink(buff,name) < 0) {
			fprintf(stderr, MSGSTR(SYLNFL, " Symbolic link failed \n"));
			perror("");
			break;
		}

		if (t == FS_NAME_X) {
			lchown(name, hp->nx.uid, hp->nx.gid);
		} else {
			lchown(name, (uid_t)hp->n.uid, (gid_t)hp->n.gid);
		}
		++success;
		break;

	  /*
	   * special file.
	   */
	  case S_IFCHR:
	  case S_IFBLK:
	  case S_IFIFO:
		if (t == FS_OINODE || t == FS_ONAME)
			device = hp->on.rdev;
		else {
			if (t == FS_NAME_X) 
 				device = makedev(hp->nx.rdevmaj, hp->nx.rdevmin);
			else
				device = makedev(hp->n.rdevmaj, hp->n.rdevmin);
		     }

		if (t == FS_NAME_X) {
			if (mknod(name, hp->nx.mode, device) == -1) {
				if ((t == FS_DINODE || t == FS_OINODE) && !dflag)
					break;
				if (!checkdir(name))
					break;
				if (mknod(name, hp->nx.mode, device) == -1) {
					perror(name);
					break;
				}
			}
		}
		else {
			if (mknod(name, hp->n.mode, device) == -1) {
				if ((t == FS_DINODE || t == FS_OINODE) && !dflag)
					break;
				if (!checkdir(name))
					break;
				if (mknod(name, hp->n.mode, device) == -1) {
					perror(name);
					break;
				}
			}	
		}

		if (t == FS_NAME_X) {
			if (aclp->acl_len)
				if ((chacl(name, aclp, aclp->acl_len) == -1) && (errno != EPERM)) {
					fprintf(stderr, MSGSTR(EACLIMP, "Import of ACLs failed\n"));
					perror("");
				}
			free(aclp);
			if (pclp->pcl_len)
				if ((chpriv(name, pclp, pclp->pcl_len) == -1) && (errno != EPERM) && (errno != EINVAL)) {
					fprintf(stderr, MSGSTR(EPCLIMP, "Import of PCLs failed\n"));
					perror("");
				}
			free(pclp);
			chown(name, hp->nx.uid, hp->nx.gid);
		}
		else {
			chmod(name, (mode_t)hp->n.mode);
			chown(name, (uid_t)hp->n.uid, (gid_t)hp->n.gid);
		}

		gethead(0);
		++success;
		break;

	  default:
		errmsg(MSGSTR(UNKIFMT,"unknown IFMT type %o\n"), i);
		break;
	}
	return success;

}

/*
 * find the inumbers for these files and save them in
 * the xtrmap.  set "needed" to number needed.
 * if -d, we call traverse, since the args may be dir names
 */
findi(np)
	register char **np; {
	register n;
	int checkfile();

	if (dflag) {
		fprintf(stderr, MSGSTR(SDMTM,
		     "Searching directories (may take several minutes)...\n"));
		traverse(ROOTINO, NULL, checkfile, np);
		return;
	}

	while (*np) {

		fprintf(stderr, MSGSTR(EXTRC,"    extract %s ... "), *np );
		if ((n = psearch( *np )) == 0 || BIT( n, dumpmap ) == 0)
			errmsg(MSGSTR(NOTON,"file not on backup\n"));
		else
		{       fprintf(stderr, MSGSTR(INODEN,"inode #%u\n"), n );
			BIS(n, xtrmap);
			needed++;
		}
		++np;
	}
}

checkfile(inum, name, np)
	char *name;
	char **np; 
{
	if (name && wanted(name, np)) {
		BIS(inum, xtrmap);
		++needed;
	}
}


breakout(void)
{       fprintf(stderr, MSGSTR(SIGRE,"Signal received, aborting!\n"));
	if(pinit == 0)
		packquit();
	unlink( dirfile );
	unlink( lname );
	exit(1);
}

/*
 * pass1
 *
 *      make a first pass over the tape, finding the directories and
 *      inode numbers of all files.  The backup is supposed to begin with
 *      directories, so the first non-directory on the backup heralds
 *      the end of this pass.
 */
pass1()
{
	int     putdir(), null();
	char    mapread;

	/*
	 * Conjure up a name for the temporary file and arrange for
	 *      interrupts to trap to a routine that will delete it.
	 */
	mktemp( dirfile );
	if (signal( SIGINT, (void (*)(int))breakout ) == SIG_IGN)
		signal( SIGINT, SIG_IGN );
	if (signal( SIGTERM, (void (*)(int))breakout ) == SIG_IGN)
		signal( SIGTERM, SIG_IGN );

	tmpf = fopen( dirfile, "w" );
	if (tmpf == NULL)
		quit(MSGSTR(NOTEMP,					/*MSG*/
		"Unable to create temp file %s ... aborting\n"), dirfile);

	mapread = 0;
	gethead(0);
	for(;;)
	{       /*
		 * everything that returns to this loop is
		 *      supposed to flush input until the
		 *      next header has been reached.
		 */
		checkhead();

		switch(thishd->h.type)
		{ case FS_BITS:
			if (!mapread) {
				readbits( dumpmap );
				++mapread;
			}
			break;

		  case FS_FINDEX:
			gethead(0);
			break;

		  case FS_CLRI:
			readbits( NULL );
			break;

		  case FS_OINODE:
		  case FS_DINODE:
			if ((thishd->i.mode&IFMT) == IFDIR  &&  ipos < MAXINO)
			{       inotab[ipos].t_ino = thishd->i.ino;
				inotab[ipos].t_seekp = ftell( tmpf );
#ifdef DEBUG
				if (debug)
					fprintf(stderr, "Processing directory %d, offset %ld\n",
						inotab[ipos].t_ino, inotab[ipos].t_seekp );
#endif
				ipos++;
				getfile(thishd, putdir, null);
				fwriteOK( (char *) &nulldir, sizeof (struct olddirect), 1, tmpf );
				continue;
			}

			if (ipos == MAXINO)
				errmsg(MSGSTR(ONL1ST,
				"Only first %d directories will be searched\n"),
				MAXINO);

			/* if we find a non-directory, we are done with
			 * the directory building pass
			 */
			/* fall through */
		  case FS_END:
			fclose( tmpf );
			tmpf = fopen( dirfile, "r" );
			return;

		  default:
			errmsg(MSGSTR(UNKHDR,				/*MSG*/
			  "Unknown header type %d - ignored\n"),thishd->h.type);
			gethead(0);
		}
	}
}

/*
 * putdir
 *      this routine process a page of a directory by adding all of
 *      its entries into our directory file
 */
putdir(b,s)
char *b;
int s;
{
	register struct olddirect *dp = (struct olddirect *)b;
	register i;

	for(i = 0; i < s; i += sizeof (struct olddirect))
	{       if (dp->d_ino)
			fwriteOK( (char *)dp, sizeof (struct olddirect), 1, tmpf );
		dp++;
	}
}

/*
 * psearch
 *
 *      search the stored directories to find a particular file name
 *      and map it into its inode number
 */
ino_t psearch( n )
 register char *n;
{       register char *s;
	register char c;
	register ino_t ino;

	ino = ROOTINO;

	if (strcmp(n, ROOT) == 0)
		return ROOTINO;

	while( *n )
	{
		/* chop off the next element of the file name   */
		for( s = n; *s && *s != '/'; s++);
		c = *s;
		*s = 0;

		/* try to find that directory entry             */
		ino = search( ino, n );
		*s = c;
		/* remain pointing @ EOS, but past a slash      */
		n = s + (c == '/');

		if (ino == 0)
			return( 0 );
	}

	return( ino );
}

/*
 * search
 *      search a particular directory for a particular file name and
 *      return the inode number
 */
ino_t search( inum, n )
 register ino_t inum;
 register char *n;
{       register int i;
	struct olddirect olddirent;

	for (i = 0; i < MAXINO  &&  inotab[i].t_ino != inum; i++);

	if (inotab[i].t_ino != inum)
		return( 0 );

	fseek( tmpf, inotab[i].t_seekp, 0 );
	for (;;)
	{       fread( (void *) &olddirent, (size_t)sizeof (struct olddirect), (size_t)1, tmpf );
		if (feof(tmpf))
			break;
		if (olddirent.d_name[0] == '/')
			break;

		for( i = 0; i<OLDDIRSIZ && n[i] && n[i]==olddirent.d_name[i]; i++);

		if (i == OLDDIRSIZ  ||  n[i] == olddirent.d_name[i])
			return( (ino_t)olddirent.d_ino );
	}

	return( 0 );
}

/*
 * traverse
 *      traverse the directory tree as read by pass1().
 *      on each file, call supplied function with name and
 *      inumber.
 */
traverse( inum, dirname, fn, np )
 register ino_t inum;
 register char *dirname;
 int (*fn)();
 char **np;
{       register int i, j;
	register char *p;
	off_t save;
	char namebuf[256];
	struct olddirect olddirent;

	/*
	 * see if the file is on the tape
	 */
	if (BIT( inum, dumpmap ) == 0)
		return;

	(*fn)(inum, dirname? dirname: ROOT, np);


	for( i = 0; i < MAXINO; i++)
	{       if (inum != inotab[i].t_ino)
			continue;

		save = ftell( tmpf );
		fseek( tmpf, inotab[i].t_seekp, 0 );
		if (dirname) {
			strcpy(namebuf, dirname);
			p = namebuf;
			p += strlen(namebuf);
			*p++ = '/';
		} else  p = namebuf;


		while(!feof( tmpf ))
		{       fread( (void *) &olddirent, (size_t)sizeof (struct olddirect), (size_t)1, tmpf );

			/* check for end of directory flag      */
			if (olddirent.d_name[0] == '/')
				break;

			/* ignore . and .. entries              */
			if (olddirent.d_name[0] == '.')
			{       if (olddirent.d_name[1] == 0)
					continue;
				if (olddirent.d_name[1] == '.' &&
				    olddirent.d_name[2] == 0)
					continue;
			}

			/* otherwise, push this name and recur  */
			for( j = 0; j<OLDDIRSIZ; j++)
				p[j] = olddirent.d_name[j];
			p[j] = 0;
			traverse( (ino_t)olddirent.d_ino, namebuf, fn, np );
		}
		fseek( tmpf, save, 0 );
		return;
	}
}



/*
 * xtrfile, xtrskip
 *      these routines are called by getfile when it is processing a
 *      file which is to be extracted by name.  The former routine
 *      is called to restore a page of the file.  The latter routine
 *      is called to skip a page of the file.
 */
xtrfile( b, size )
char *b;
int size;
{
	if (write( ofile, b, size ) != size) XtrError();
}


xtrskip( b, size )
char *b;
int size;
{
	if (lseek( ofile, (off_t) size, 1 ) == -1) XtrError();
}

/*
 * do we want to extract name?  yes if name is on nlist,
 * or if nlist is empty.
 */

wanted(name, nlist)
register char *name;
register char **nlist;
{
	static char dir_pat[PATH_MAX];
	static short wasindir=0; 

	if (!nlist)             /* nlist is NULL -- table of contents only */
		return 0;

	if (!*nlist)            /* nlist is empty -- extract everything */
		return 1;

	/* otherwise -- nlist contains list of files and/or directories 
	 * to be extracted if dflag is set we want to make sure that we 
	 * "match" any files that begin with a previously matched directory 
	 * name ( dir_pat ), without having to actually go through 
	 * and do the pattern matching  - we assume when dflag is set and 
	 * a match is found that we may be in a directory and therefore 
	 * want to check if the next file is a sub-child of that directory, 
	 * by checking the equality of the first 'n' characters that are 
	 * the length of the directory name previously encountered. 
	 */
	if (dflag && wasindir && (strlen(dir_pat) <= strlen(name)) &&
		(strncmp(dir_pat,name,strlen(dir_pat)) == 0))
			return 1;

	do{
		if (dflag){
			wasindir=0;
			strcpy(dir_pat,*nlist);
			strcat(dir_pat,"/");
			if ((strlen(dir_pat) <= strlen(name)) &&
				(strncmp(dir_pat,name,strlen(dir_pat)) == 0)) {
					wasindir++;
					return 1;
			}
		}
		/* call fnmatch with flags to enforce matching of slashes 
		 * in pathname, matching of the leading period, and the 
		 * use of the backslash within quotes.
		 * We will replace the name to be extracted on the list 
		 * to an unique name so that multiple file with the same 
		 * name in the archive and on the list will not confused 
		 * restore. Replace the name only if we are counting the
		 * exact number of files to be extracted.
		 */
		if (fnmatch(*nlist,name,(FNM_PATHNAME|FNM_PERIOD|FNM_QUOTE)) == 0) {
			if (needed != DONTCOUNT)
				*nlist = REPLACE_NAME;
			return 1;
		}
	}while (*++nlist);
	return 0;
}

/*
 * make the directory name
 */
rest_mkdir(name)
	register char *name; {
	struct stat statb;
	extern int errno;
	int status;

	/* do we need to make it, or is it there? */
	if (stat(name, &statb) == -1) {
		/* stat failed */
		if (errno != ENOENT) {
			fprintf(stderr, MSGSTR(NOMKDIR,
				"Cannot make directory '%s': "), name);
			perror("");
			return 0;
		}
	} else {
		/* stat succeeded */
		if ((statb.st_mode&S_IFMT) != S_IFDIR) {
			errno = EEXIST;
			fprintf(stderr, MSGSTR(NOMKDIR,
				"Cannot make directory '%s': "), name);
			perror("");
			return 0;
		}
		return 1;
	}

	if (mkdir(name, (mode_t)0777) < 0) {
		fprintf(stderr, MSGSTR(NOMKDIR,
			"Cannot make directory '%s': "), name);
		perror("");
		return 0;
	} else 
		return 1;
}


/* Given a pathname, be sure all the intermediate directories exist. */
checkdir(name)
register char *name;
{
	register char *cp, *cpend;
	struct stat sbuf;

	cpend = name + strlen(name);

	/* Scan back from end for first directory that exists */
	for (cp = cpend; --cp > name; ) {
		if (*cp == '/' && cp[-1] != '/') {
			*cp = '\0';
			if (stat(name, &sbuf) >= 0) {
				*cp++ = '/';
				break;
			}
		}
	}
	/* Any NULs before cpend are directories that have to be made */
	for ( ; cp < cpend; ++cp) {
		if (*cp == '\0') {
			if (!rest_mkdir(name))
				return 0;
			*cp = '/';
		}
	}
	return 1;
}

/*
 * count files requested.  return number in list.
 * special cases: empty or null list means all files,
 * dflag means entire subtrees, wild cards possible.
 * return DONTCOUNT if we can't tell exactly how many files
 * need extracting.
 */
countfiles(np)
	register char **np; {
	register i;

	if (dflag || !np || !*np)
		return DONTCOUNT;

	for (i=0; *np; np++,i++)
		if (wildcards(*np))
			return DONTCOUNT;

	return i;

}

/*
shoindex(hp)
	register union fs_rec *hp; {
	register i;

	fprintf(stderr, MSGSTR(INDEX,"INDEX, loc %ld, link %ld\n"),
		ttell() - hp->h.len, hp->x.link );

	for (i=0; i < FXLEN && hp->x.ino[i]; ++i)
		fprintf(stderr, MSGSTR(INOLO,"  ino %u at loc %ld\n"),
			hp->x.ino[i], hp->x.addr[i]);
}

*/


/*
 * does the string np contain (shell-style) wild cards?
 * (Check each char in np against 'wildcard' list.)
 */
wildcards(np)
register char *np;
{
	while(*np)
		if (strchr("*?[", *np++) != NULL)
			return 1;
	return 0;
}


/*
 * given an inumber, turn it into a name
 * not terribly fast, but only called for files that we know we
 * want to extract.
 */
#define NBUFSIZ 200
char *
toname(ino)
	ino_t ino; {
	static char name[NBUFSIZ];
	register char *np, *dp;
	struct olddirect olddirent;
	ino_t curdir, pardir;
	register i;

	np = name + NBUFSIZ;

	*--np = 0;              /* null byte */
	if (ino == ROOTINO)
		return ROOT;

	/* find the entry for ino in tmpf */
	rewind(tmpf);
	for(;;) {
		fread((void *)&olddirent, (size_t)sizeof(struct olddirect), (size_t)1, tmpf);
		if (feof(tmpf)) {
			return np;
		}

		dp = olddirent.d_name;
		if (dp[0] == '/')
			continue;
		if (dp[0] == '.' && (!dp[1] || (dp[1]=='.' && !dp[2])))
			continue;
		if ((ino_t)olddirent.d_ino == ino)
			break;
	}

	/* crawl up directory tree */
	for (;;) {

		off_t here;

		/* add name to what we have so far */
		while (*dp && dp < &olddirent.d_name[OLDDIRSIZ])
			++dp;
		if (*np)
			*--np = '/';
		do
			*--np = *--dp;
		while (dp > olddirent.d_name);

		/* find . and .. in this dir */
		here = ftell(tmpf);
		for (i=0; i<ipos; i++)
			if (inotab[i].t_seekp < here
			&& (i+1 == ipos || inotab[i+1].t_seekp > here))
				break;
		fseek(tmpf, inotab[i].t_seekp, 0);
		fread((void *)&olddirent, (size_t)sizeof(struct olddirect), (size_t)1, tmpf);
		curdir = (ino_t)olddirent.d_ino;          /* . */
		if (curdir == ROOTINO)
			return np;
		fread((void *)&olddirent, (size_t)sizeof(struct olddirect), (size_t)1, tmpf);
		pardir = (ino_t)olddirent.d_ino;          /* .. */

		/* look through .. for the named entry for . */
		for (i=0; i<ipos; i++)
			if (inotab[i].t_ino == pardir)
				break;

		fseek(tmpf, (long int)(inotab[i].t_seekp+2*sizeof(struct olddirect)), 0);
		for (;;) {
			fread((void *)&olddirent, (size_t)sizeof(struct olddirect), (size_t)1, tmpf);
			if ((ino_t)olddirent.d_ino == curdir)
				break;
		}
	}

}

struct linfo {
	ino_t  l_ino;           /* i number */
	ulong l_devmaj;        /* maj/min device number */
	ulong l_devmin;
	ushort l_namelen;       /* length of name, not counting null */
};

/*
 * try to link this file to another one:  If link count == 1,
 * return failure.  If on link list, return the name that it was
 * restored under, after doing the actual link.
 * Otherwise, add it to the link list for future
 * reference, and return failure (NULL).
 */
char *
trylink(hp)
	register union fs_rec *hp;
{
	static FILE *ilfile, *olfile;
	struct linfo linfo;
	static char nambuf[NAMESZ];
	ulong majdev, mindev;          /* no. of dev that contains file */
	ino_t inonum;
	char *name;

	/* Only link ordinary files with link count > 1 */
	if (hp->h.type == FS_NAME_X) {
		if (hp->nx.nlink == 1 || (hp->nx.mode & IFMT) == S_IFDIR)
			return NULL;
	}
	else
		if (hp->n.nlink == 1 || (hp->n.mode & IFMT) == S_IFDIR)
			return NULL;

	/* May need to initialize link file */
	if (ilfile == NULL) {
		mktemp(lname);
		olfile = fopen(lname, "w");
		ilfile = fopen(lname, "r");
		if (ilfile == NULL || olfile == NULL)
			return NULL;    /* forget it */
	}

	if (hp->h.type == FS_ONAME) {
		inonum = (ino_t)hp->on.ino;
		majdev = (ulong)major(hp->on.dev);
		mindev = (ulong)minor(hp->on.dev);
		name   = hp->on.name;
	} 
	else 
		if (hp->h.type == FS_NAME_X) {
			inonum = hp->nx.ino;
			majdev = hp->nx.devmaj;
			mindev = hp->nx.devmin;
			name   = hp->nx.name;
		}
		else {
			inonum = (ino_t)hp->n.ino;
			majdev = (ulong)hp->n.devmaj;
			mindev = (ulong)hp->n.devmin;
			name   = hp->n.name;
		};

	/* see if file is in the list so far */
	/* always rewind ilfile */
	/* olfile always kept at end */
	rewind(ilfile);
	for (;;) {
		fread((void *)&linfo, (size_t)sizeof(linfo), (size_t)1, ilfile);

		/* end of list -- return failure */
		/* but first, add info on current file to list */
		if (ferror(ilfile))
			return NULL;
		if (feof(ilfile)) {
			int nlen;

			/* could we read it back in? */
			nlen = strlen(name);
			if (nlen >= NAMESZ)
				return NULL;

			linfo.l_ino = inonum;
			linfo.l_devmaj = majdev;
			linfo.l_devmin = mindev;
			linfo.l_namelen = nlen;
			fwriteOK(&linfo, sizeof(linfo), 1, olfile);
			fwriteOK(name, sizeof(char), nlen, olfile);
			if (fflush(olfile)<0) fwriteerr();

			return NULL;
		}

		/* read in name */
		fread((void *)nambuf, (size_t)sizeof(char), (size_t)linfo.l_namelen, ilfile);
		nambuf[linfo.l_namelen] = 0;    /* null byte */

		/* see if this entry matches */
		if (linfo.l_ino == inonum
		    && linfo.l_devmaj == majdev
		    && linfo.l_devmin == mindev)
			break;
	}

	/* here we have a match, so we can try the link */
	if (link(nambuf, name) < 0)
		return NULL;
	else   
	{

	return nambuf;
	}

}


listfile(inum,dirname) 
char *dirname; 
{
	printf("     %6u\t%s\n", inum, dirname );
}


/* do an fwrite, and check the result; on an error, exit */
fwriteOK(b,s,c,f) 
char *b; 
FILE *f; 
{
	if (fwrite((void *)b,(size_t)s,(size_t)c,f)!=c) 
		fwriteerr();
}

/* report an fwrite error on the internal files */
fwriteerr() 
{
	quit(MSGSTR(FWRERR,"write error on temporary file\n"));
}

/* report an extracted file write error, and die */
static XtrError()
{
	extern errno,special;
	if (errno != 0)
    	switch (errno) {
      		case EFBIG: {
			fprintf(stderr, MSGSTR(RHRUL,
"Restore has reached user file size limit of %d blocks\nwhile trying to restore file '%s'\n"),ulimit(1,0),XtrName);
			exit(1);
		}
      		case ENXIO: {
			fprintf(stderr, MSGSTR(RHRZS,
"Restore has reached zero free space on current logical volume\nwhile trying to restore file '%s'\n"),XtrName);
			exit(1);
		}
      		default: {
			fprintf(stderr, MSGSTR(RUTWF,"Restore unable to write to file '%s' .. Aborting!\n"),XtrName);
			perror("Restore ");
			exit(1);
		}
	}
}

/*
 * getpfile
 *      extract a file from medium, and unpack the file.
 */

getpfile(hp, name)
register union fs_rec *hp;
char *name;
{
	register char *p;
	register off_t size, off = 0;
	register int bsz;               /* Bytes to read        */
	int tmpshmid;                   /* shm id */
	char *tmpaddr;
	int status,exit_status; 	/* wait status          */
	char *s = NULL;                 /* Error string */
					/* Communications struct        */
	struct r_comm r_comm, *rcp = &r_comm;
	int numusedpages;
	char dummyc;

	register struct packctl *pkp = &packctl;

	if (hp->h.type == FS_NAME_X)
		size = hp->nx.dsize;
	else
		size = hp->i.dsize;

/* If the first time through here, call packinit() */

	if (pinit)
		if (packinit())
			goto out;

/* Create the control pipes,
 * we are using two pipes between the parent
 * process (the reader of the medium) and
 * the child (the unpacker).
 * The pipes are created each time a new
 * unpacker is created becouse keeping them
 * around for the next child would need
 * that the pipes be empty, currently the
 * ctopfd pipe might not empty in all cases
 * when the unpacking finishes. This would
 * confuse the next unpack ...
 */

	if (pipe(packctl.ptocfd) == -1 ||
	    pipe(packctl.ctopfd) == -1)
	{
		s = MSGSTR(PIPEF, "Restore: pipe(2) failed");
		goto out;
	}

/* Fork a child process */

	if ((pkp->cpid = fork()) == 0) {
		if (close(pkp->ptocfd[1]) == -1 || close(pkp->ctopfd[0]) == -1){
			perror(MSGSTR(INUNP,"Unpack: internal unpacking error\n"));
			exit(1);
		}
		_exit(unpack(hp->h.type == FS_NAME_X ? hp->nx.size : hp->i.size,
			pkp->addr, pkp->ptocfd[0], pkp->ctopfd[1]));
	} else if (pkp->cpid == -1) {
		perror(MSGSTR(INUNP,"Unpack: internal unpacking error\n"));
		exit (1);
	}
/* Close parents unneeded pipe fds */
	if (close(pkp->ptocfd[0]) == -1 || close(pkp->ctopfd[1]) == -1){
		perror(MSGSTR(INUNP,"Unpack: internal unpacking error\n"));
		exit(1);
	}


/* Read packed file into shared segment */

	rcp ->r_eof = 0;

	numusedpages = 0;
	while (size) {
		bsz = min(size, BSIZE);
		p = (char *) readmedium(btow(bsz));
		memcpy(pkp->addr + off, p, bsz);
		off += bsz;
		size -= bsz;
		rcp->r_bsz = bsz;
		if(!size)
			rcp->r_eof = EOF ;
		write(pkp->ptocfd[1], rcp, sizeof(struct r_comm));
		if (++numusedpages == MAXBLOCKS) { 
			/* we have read MAXBLOCKS into the shared memory
			 * segment, so we must wait for the child to use
			 * one before we continue reading blocks into it,
			 * the child will write one byte to wake us up
			 * when he is done with its current block
			 */
			if (read (pkp->ctopfd[0], &dummyc, 1) != 1) {
				perror(MSGSTR(BRDOP,"Unpack: bad read of pipe\n"));	
				exit(1);
			}
			--numusedpages;
		}
		if (off >= MAXBLOCKS * BSIZE)
			off = 0;
	}
/* Wait on the unpack process */

	waitpid(pkp->cpid,&status,0);
	if (close(pkp->ptocfd[1]) == -1 || close(pkp->ctopfd[0]) == -1){
		perror(MSGSTR(INUNP,"Unpack: internal unpacking error\n"));
		exit(1);
	}
	exit_status = status >> 8;
	if(exit_status == -1)
	{	fprintf(stderr, MSGSTR(EDUNP,
			"Restore: error during unpack of %s...\n"),name);
	}
	else
	if(exit_status & 0x1c){
		fprintf(stderr, MSGSTR(NOSPC, "Restore: Out Of Space!! \n"));
		exit(1);
	}

/* Get the next header and return */

	gethead(0);
	return;

out:
	if (s) perror(s);
	getfile(hp, null, null);

/* Get the next header */

	gethead(0);
}

packinit()
{
	char *s = NULL;                 /* Error string */
	extern errno;

/* Set up the signal handlers */

	if (signal( SIGINT, (void (*)(int))breakout ) == SIG_IGN)
		signal( SIGINT, SIG_IGN );
	if (signal( SIGTERM, (void (*)(int))breakout ) == SIG_IGN)
		signal( SIGTERM, SIG_IGN );

/* Initialize shared segment and attach it */

	if ((packctl.shmid = shmget(IPC_PRIVATE, MAXBLOCKS * BSIZE,
	    S_IRUSR|S_IWUSR)) < 0)
	{
		s = MSGSTR(SHMGETF,"Restore: shmget(2) failed");
		goto out;
	}

	if((packctl.addr = shmat(packctl.shmid, (char *)0, 0)) == (char *)-1)
	{
		s = MSGSTR(SHMATF,"Restore: shmat(2) failed");
		goto out2;
	}

	pinit = 0;
	return(0);

/* Detach the shared memory segment and destroy it */

out3:
	shmdt((char *)packctl.addr);
out2:
	shmctl(packctl.shmid, IPC_RMID, (struct shmid_ds *)0);
out:
	perror(s);
	return(1);
}

packquit()
{
	shmdt((char *)packctl.addr);
	shmctl(packctl.shmid, IPC_RMID, (struct shmid_ds *)0);
	kill(packctl.cpid, SIGTERM);
}

/*
 *      Huffman decompressor
 */

/* the dictionary */

/*
 * A worst-case tree would consist of one leaf per level.  Since leaves
 * coorespond to characters, the maximum would be 257 (256 chars + EOF code).
 */
static char     *tree[257];
static char     *eof;
static short    maxlev;
static short    intnodes[257];
static char     characters[256];

/* read in the dictionary portion and build decoding structures */
/* return 0 if successful, -1 otherwise */

unpack(osize, in, ip, op)
size_t osize;
char *in;
int ip;
int op;
{

	register int c, i, nchildren, rc = -1;
	register char *inp = in;
	struct r_comm r_comm, *rcp = &r_comm;   /* Communications struct */

	/* Add a close of the file descriptor for the diskette drive since
	   we don't use it and having it open by us causes problems if
	   the packed file spans two diskettes
	*/

	close(med_fd);

	/*
	 * check two-byte header
	 * get size of original file,
	 * get number of levels in maxlev,
	 * get number of leaves on level i in intnodes[i],
	 * set tree[i] to point to leaves for level i
	 */

	signal( SIGINT, SIG_DFL );
	signal( SIGTERM, SIG_DFL );

	eof = &characters[0];
	if(read(ip, rcp, sizeof(struct r_comm)) != sizeof(struct r_comm))
	{	fprintf(stderr, MSGSTR(BRDOP,"Unpack: bad read of pipe\n"));	
		return(-1);
	}

	/* Assumes the inital amount of data in the shm segment is >= to
	 * the size of the dictionary
	 */

	maxlev = *inp++ & 0377;

	for (i=1; i<=maxlev; i++)
		intnodes[i] = *inp++ & 0377;

	for (i=1; i<=maxlev; i++) {
		tree[i] = eof;
		for (c=intnodes[i]; c>0; c--) {
			if (eof >= &characters[255])
				goto eprint;
			*eof++ = *inp++;
		}
	}
	*eof++ = *inp++;
	intnodes[maxlev] += 2;

	if((rcp->r_bsz -= (inp - in)) < 0)
		goto eprint;

	/*
	 * convert intnodes[i] to be number of
	 * internal nodes possessed by level i
	 */

	nchildren = 0;
	for (i=maxlev; i>=1; i--) {
		c = intnodes[i];
		intnodes[i] = nchildren /= 2;
		nchildren += c;
	}

	rc = decode(inp, ip, op, osize, rcp, in);

	if(rc == -1)
eprint:		fprintf(stderr, MSGSTR(INUNP,"Unpack: internal unpacking error\n"));

	return (rc);
}

/* unpack the file */
/* return 0 if successful, -1 otherwise */

char outbuff[BSIZE];

decode (inp, ip, op, origsize, rcp, begshmbuf)
register char   *inp;
int ip;
int op;
register size_t origsize;
register struct r_comm *rcp;
char		*begshmbuf;
{
	register int bitsleft, c, j;
	register char *p, *outp = outbuff;
	char *eobuff = &outbuff[BSIZE];
	register int i = 0;
	register int lev = 1;
	size_t os = origsize;
	char *eoshmbuf = begshmbuf + MAXBLOCKS * BSIZE;

	for(;;) {
		if (--rcp->r_bsz < 0)
		{       if(rcp->r_eof == EOF)
			{	fprintf(stderr, MSGSTR(FOUTP,"Unpack: file out of phase\n"));
				return(-1);
			}
			if (write(op, "g", 1) != 1) {
				perror(MSGSTR(INUNP,"Unpack: internal unpacking error\n"));
				exit(1);
			}
			if(read(ip, rcp, sizeof(*rcp)) != sizeof(*rcp))
			{	fprintf(stderr, MSGSTR(BRDOP,"Unpack: bad read of pipe\n"));	
				return(-1);
			}
			--rcp->r_bsz;
			if (inp >= eoshmbuf)
				inp = begshmbuf;
		}
		c = *inp++;
		bitsleft = 8;
		while (--bitsleft >= 0) {
			i *= 2;
			if (c & 0200)
				i++;
			c <<= 1;
			if ((j = i - intnodes[lev]) >= 0) {
				p = &tree[lev][j];
				if (p == eof) {
					c = outp - outbuff;
					xtrfile(outbuff, c);
					origsize -= c;
					if (origsize != 0)
					{	fprintf(stderr, MSGSTR(UPCSZ,"Unpack: file size error..\n"));
						fprintf(stderr, MSGSTR(FORGZ,"File %s original size %d,  restored size %d\n"), XtrName, os, os - origsize);
						return(-1);
					}
					return (0);
				}
				*outp++ = *p;
				if(outp == eobuff) {
					if(!zero(outbuff, BSIZE) || origsize <= BSIZE)
						xtrfile(outbuff, BSIZE);
					else
						xtrskip(outbuff, BSIZE);

					outp = outbuff;
					origsize -= BSIZE;
				}
				lev = 1;
				i = 0;
			} else
				lev++;
		}
	}
}

reorderacl(ap)
struct acl *ap;
{
	struct acl_entry *acep,  *ace_end;

	rlong(ap->acl_len);
	rlong(ap->acl_mode);
	rshort(ap->acl_rsvd);
	rshort(ap->u_access);
	rshort(ap->g_access);
	rshort(ap->o_access);

#define ace_last acl_last   /* Due to an unfortunate choice of mnuemonics   */
#define ace_nxt acl_nxt     /* the last ace is referred to in acl.h as the  */
			    /* last acl                 */

	acep = ap->acl_ext; /* acep points to first ace entry   */
	ace_end = ace_last(ap); /* aclend points to last ace entry  */

	if (acep >= ace_end)
		return;

	/* reorder fields in the aces */
	for ( ; acep < ace_end; acep = ace_nxt(acep)) {
		struct ace_id   *idp;
		struct ace_id   *idend;
		unsigned short tmp;
			struct dummy {
            unsigned short  ace_len;
			unsigned short  ace_bits;
			struct  ace_id  ace_id[1];
		} *bp = (struct dummy *)acep;

		rshort(acep->ace_len);
		rshort(bp->ace_bits);

		idend = id_last(acep);

		/* reorder fields in the ace ids */
		for (idp=acep->ace_id; idp<idend; idp=id_nxt(idp)) {
			rshort(idp->id_len);
			rshort(idp->id_type);
		}
	}
}

reorderpcl(pp)
struct pcl *pp;
{
	struct pcl_entry *pcep, *pce_end;

	rlong(pp->pcl_len);
	rlong(pp->pcl_mode);
	rlong(pp->pcl_default.pv_priv[0]);
	rlong(pp->pcl_default.pv_priv[1]);

#define pce_last pcl_last   /* Due to an unfortunate choice of mnuemonics   */
#define pce_nxt pcl_nxt     /* the last pce is referred to in priv.h as the */
			    /* last pcl                 */

	pcep = pp->pcl_ext;         /* pcep points to first ace entry   */
	pce_end = pce_last(pp);         /* pce_end points to last ace entry */

	if (pcep >= pce_end)
		return;

	/* reorder the priv entries */
	for ( ; pcep < pce_end; pcep = pce_nxt(pcep)) {
		struct pce_id   *idp;
		struct pce_id   *idend;

		rlong(pcep->pce_len);
		rlong(pcep->pce_privs.pv_priv[0]);
		rlong(pcep->pce_privs.pv_priv[1]);

		idend = pcl_id_last(pcep);

		/* reorder the pc ids */
		for (idp = pcep->pce_id; idp < idend; idp = pcl_id_nxt(idp)) {
			rshort(idp->id_len);
			rshort(idp->id_type);
		}
	}
}
