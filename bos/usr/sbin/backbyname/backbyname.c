static char sccsid[] = "@(#)89	1.56.1.18  src/bos/usr/sbin/backbyname/backbyname.c, cmdarch, bos41J, 9512A_all 3/15/95 12:31:01";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: backbyname 
 *
 * ORIGINS: 3, 27, 9 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#define _ILS_MACROS

#include <nl_types.h>
#include "bbyname_msg.h"
#include <unistd.h>
#include <stdio.h>
#include <values.h>
#include <varargs.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/lockf.h>
#include <sys/devinfo.h>
#include <fcntl.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <dumprestor.h>
#include <sys/audit.h>
#include <sys/limits.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/acl.h>
#include <sys/priv.h>
#include <sys/id.h>
#include <regex.h>
#include <locale.h>
#include <ctype.h>

#define MSGSTR(N,S) catgets(catd,MS_BBYNAME,N,S)
#define NOT_PACKED      0
#define PACKED          1
#ifdef DEBUG
#define dbg1(a) if (debug) fprintf(stderr,a)
#define dbg2(a,b) if (debug) fprintf(stderr,a,b)
#define dbg3(a,b,c) if (debug) fprintf(stderr,a,b,c)
#define dbg4(a,b,c,d) if (debug) fprintf(stderr,a,b,c,d)
#else
#define dbg1(a)
#define dbg2(a,b)
#define dbg3(a,b,c)
#define dbg4(a,b,c,d)
#endif

#define RCFLAGS REG_EXTENDED|REG_NOSUB
#define NILPTR(x) (x == 0 || *x == 0)
#define SNGL_VOL_MNT "Please mount volume %d on %s\n...\
 and press Enter to continue\n"
#define MULT_VOL_MNT "Please mount volumes %d-%d on %s-%d\n...\
 and press Enter to continue\n"

nl_catd catd;
char    filsys[NAMESZ];         /* file system (e.g. /usr) */
char    special[NAMESZ];        /* file system special file name */
char    medium[NAMESZ];         /* output medium name */
char    *ttydev = "/dev/tty";   /* need to play with tty */
int     med_fd;                 /* output file descriptor for medium */
int     tty;                    /* input fd for terminal, used for responses */
int	must_audit = 0;
daddr_t esize;                  /* estimated backup length (# wds) */
regex_t	pexcl;                  /* Compiled exclude pack regular expr */

/* flags */
unsigned char debug = 0;        /* request debugging information */
unsigned char rflag = 0;        /* assume volume 1 is ready */
unsigned char lflag = 0;        /* block limit for disks specified by user */
unsigned char oflag = 0;        /* use version 2 headers if specified by user */
unsigned char pflag = 0;        /* pack files being backed up */
unsigned char no_op = 0;        /* no operator flag */
unsigned char verbose;          /* request lots of conversation */
int novalidRE = 1;              /* set to zero if we were able to compile RE */
int not_stdio = 0;              /* medium isn't stdout(=0), is stdout(=1) */
off_t cursize;                  /* Size of current archive entry. */
off_t totsize = 0;              /* Total size of archive entries seen. */
int ExitCode;                   /* Exit value */

/* buffers */
union  fs_rec vol_rec;          /* volume header and file index */
union  fs_rec hdr_rec;          /* other headers */

/*
 * the v_* items we must keep around to put on each volume
 * header.  we can't save the info in vol_rec, because it's
 * reused as the file index.  even if there's no index,
 * vol_rec's bytes get rearranged whenever it is written out.
 */
time_t  v_date;
time_t  v_dumpdate;
short   v_incno;
ushort  v_volnum;

#define BR_BACKUP

#include "util.h"

/* length of headers */
unsigned char hdrlen[] = {
	btow(sizeof(hdr_rec.v)),                /* FS_VOLUME    */
	btow(sizeof(hdr_rec.x)),                /* FS_VOLINDEX  */
	btow(sizeof(hdr_rec.b)),                /* FS_CLRI      */
	btow(sizeof(hdr_rec.b)),                /* FS_BITS      */
	btow(sizeof(hdr_rec.oi)),               /* FS_OINODE    */
	btow(sizeof(hdr_rec.on)-DUMNAME),       /* FS_ONAME     */
	btow(sizeof(hdr_rec.e)),                /* FS_VOLEND    */
	btow(sizeof(hdr_rec.e)),                /* FS_END       */
	btow(sizeof(hdr_rec.i)),                /* FS_DINODE    */
	btow(sizeof(hdr_rec.n)-DUMNAME),        /* FS_NAME      */
	btow(sizeof(hdr_rec.ds)),               /* FS_DS        */
	btow(sizeof(hdr_rec.nx)-DUMNAME),       /* FS_NAME_X    */
};

/* function type declarations */
char *getenv();
char *strncpy();
long time();
void puthdr();
void volfail();
void buf_reset();
void buf_oseek();
void buf_volinit();
void infomedium();
void init_diskette(void);
void get_unit();
void prompt();

main(int argc, char *argv[])
{
	register c;
	time_t now;
	char iflag = 1;
	static char tmbuf [64];
	struct tm * ltm;

	(void) setlocale(LC_ALL,"");
	catd = catopen("bbyname.cat", NL_CAT_LOCALE);

	time(&v_date);
	while ((c = getopt(argc,argv,"iDf:vl:rqb:C:ope:S")) != EOF) {
		switch( c ) {
#ifdef DEBUG
		    case 'D':     /* Debugging mode */
			  debug++;
			  verbose++;
			  break;
#endif
		    case 'v':     /* Verbose mode */
			  verbose++;
			  break;

		    case 'o':     /* backup with Version 2 headers */
			  oflag++;
			  break;

		    case 'l':     /* length of disk in blocks */
			  blks2use = atoi(optarg);
			  lflag++;
			  blk_limit++;
			  break;

		    case 'b':
		    case 'C':     /* specify cluster size */
			  cluster = atoi(optarg);
			  /*
			   * Check for max cluster size.
			   * Actually, this check is overkill
			   * since MAXBLK applies only to
			   * transfers to raw devices.
			   */
			  if(cluster * 512 > ctob(MAXBLK)) {
				int xsclus = cluster;
				cluster = (ctob(MAXBLK))/512;

				fprintf(stderr, MSGSTR(REDUCED, 
				"Warning: cluster size %d too large, reduced to %d\n"), xsclus, cluster);
			  }
			  break;

		    case 'f':     /* specify medium file */
			  strncpy(medium,optarg,strlen(optarg));
			  break;

		    case 'i':     /* backup by name */
			  /* no need to turn this flag on since
			   * it is on by default.
			   */
			  break;

		    case 'r':     /* assume ready */
		    case 'q':     /* for restore compatibility */
			  ++rflag;
			  break;

		    case 'p':
			  ++pflag;
			  break;

		    case 'e':     /* Exclude pack regular expression */
			  if (*optarg == '\'') {
			      optarg++;
			      optarg[strlen(optarg)-1] = '\0';
			  }
			  if (novalidRE = regcomp(&pexcl, optarg, RCFLAGS)) {
			      fprintf(stderr, MSGSTR(BADREGX,
				  "invalid regular expression: %s\n"), optarg);
			      exit(2);
			  }
			  break;
		    case 'S':
			  rflag++; /* Assume that device is ready */
			  no_op++;
			  break;

		    case '?':
		    default:
			  exit(2);
		}
	}

	if ((tty = open(ttydev,O_RDONLY)) < 0) 
		tty = -1;

	strncpy(filsys, "by name", NAMESZ);
	strncpy(special, filsys, NAMESZ);

	if NILPTR(medium)
		strncpy(medium, DEF_MEDIUM, NAMESZ);
	byname(); 

	/* might want to write several copies here */
	hdr_rec.h.type = FS_END;
	puthdr(&hdr_rec, NOT_PACKED);
	findex(TEND);
	flushmedium();
	if (close(med_fd) < 0) {
                fprintf(stderr, MSGSTR(ERRCLOSE, "failure closing "));
                perror(medium);
                exit(2);
	}

	if (not_stdio && verbose) {
		time(&now);
		ltm = localtime(&now);
		strftime(tmbuf,64,"%c",ltm);
		printf(MSGSTR(DONETIM,"Done at %s; "), tmbuf);
		printf(MSGSTR(BLKSVOL,
			"%ld blocks on %d volume(s)\n"), blocks_tot, v_volnum);
	}
	exit(ExitCode);
}


/* It is improper to use -l with a non-disk medium.
 * If this occurs, we warn the user of the problem 
 * and if possible ask if the user wants to go on anyway.
 */
void
flagwarning(void)
{
	unsigned char confirm = 0;

	/* Bad -l usage */
	if (lflag && devinfo.devtype != DD_DISK) {
		fprintf(stderr, MSGSTR(IGNDISK,
			"Warning: -l will be ignored (medium not a disk)\n"));
		confirm++;
	}

	/* If the user has made a flag error, we will prompt only if
	   the -r flag is not used and if the user is at a tty.
	   Otherwise, we just continue. */

	if (confirm && !rflag && tty != -1) {
		char ans[BUFSIZ];
		int i;
		char *index();
		char * catmsg_str; 
		
		/* This determines if the catalog is there. */
		catmsg_str = MSGSTR(ANSYORN,""); 
		do {
			if ( *catmsg_str == '\0' )
			  fprintf(stderr, "Do you wish to continue? (yes or no) ");
			else  
			  fprintf(stderr,"%s",catmsg_str);
			/*
			 * we doctor up the answer, remove the return character
			 * and make sure it is null terminated
			 */
			bzero(ans, sizeof(ans));
			read(tty, ans, sizeof(ans));
			ans[sizeof(ans) - 1] = '\0';
			if (index(ans, '\n') != NULL)
				*(index(ans, '\n')) = '\0';
		    
			/* no message file - assume English */ 	
			if ( *catmsg_str == '\0' ) {		 
				if (strcmp(ans,"yes") == 0 )
		    	  		i = 1;
                		else 
					if (strcmp(ans,"no") == 0 )
			   			i = 0;
                  			else
		            			i = -1;
              		}
			else
              			i = rpmatch(ans);

			fprintf(stderr, "%s\n", ans);
			if (!i)
				exit(2);
		} while (i == -1);
	}
}


/*
 * backup files by names that come from stdin
 */
byname()
{
	char fname[NAMESZ];	/* file name as read from stdin	*/
	char filemsg[NAMESZ+16];
	void flagwarning();

	v_dumpdate = v_date;
	v_incno = BYNAME;
	omedium(0);
	flagwarning();

	if (oflag && not_stdio && verbose)
		printf(MSGSTR(VERS2, "Version 2 backup in progress.\n"));
	/*
	 * Basic loop: read a name from standard input,
	 * then call dfile to put the file on the medium.
	 */
	errno = 0;

	while (gets(fname) != NULL) {
		/* call dfile to back up the file */
		if (fname[0] != 0 && dfile(fname))
			if (verbose)
				sprintf(filemsg, "a %12d %s\n", cursize, fname);

			if(not_stdio) {
				printf(filemsg); /* Print to stdout if it is */
				fflush(stdout);  /* not the archive medium.  */
			}
			else
				fprintf(stderr, filemsg);
			totsize += cursize;
		errno = 0;
	}
	if (errno)
		quit(MSGSTR(INPUTERR, "error reading from stdin\n"));
	if (verbose)
		printf(MSGSTR(TOTSIZ,"    total size: %d\n"), totsize);
}


/*
 * backup this file by name
 */
dfile(const char *name)
{
	struct stat statb;
        char buf[BSIZE];
	char linkname[NAMESZ];	/* symbolic link name  */
	off_t bytes;
	register fd, i;
	int aclsize, pclsize;
	struct acl  *aclbuf;
	struct pcl  *pclbuf;
	int response;
	char ans[20];
	int size;
	int (*readfunc)();
	int issymlink=0;

	/*
	 * Fetch the stat info and make sure that the file 
         * can be accessed.
	 */	

        if ((lstat(name, &statb) < 0) ||
            ( (!(issymlink = (statb.st_mode & S_IFMT)==S_IFLNK))
             && (access(name, R_OK) < 0)))
        {
                /*
                * If the file cannot be accessed, complain.
                */

		fprintf(stderr, MSGSTR(NOACCESS,"cannot access "));
                perror(name);
		ExitCode=1;
                return 0;
        }
	cursize = statb.st_size;

	if (!oflag) {
		/* Fetch acl information */

		aclsize = BUFSIZ;
		/* Loop till we've got a big enough buffer. */
		while (1) {
			int sz;
	
			sz = btow(aclsize)*8;
			if ((aclbuf = (struct acl *)malloc((size_t)sz)) == NULL) {
				fprintf(stderr, MSGSTR(NOABUF, 
					"backup: Cannot malloc buffer for ACL\n"));
				exit(2);
			}
			if (statacl(name, issymlink?STX_LINK:STX_NORMAL,
				    aclbuf, aclsize) != -1)
				break;
			if (errno != ENOSPC) {
				perror("backup");
				exit(2);
			}
			aclsize = aclbuf->acl_len;
			free((void *)aclbuf);
		}
	 	aclsize = aclbuf->acl_len;

		/* Fetch pcl information */

		pclsize = BUFSIZ;
		while (1) {
			int sz;

			sz = btow(pclsize)*8;
			if ((pclbuf = (struct pcl *)malloc((size_t)sz)) == NULL) {
				fprintf(stderr, MSGSTR(NOPBUF, 
					"backup: Cannot malloc buffer for PCL\n"));
				exit(2);
			}
			if (statpriv(name, issymlink?STX_LINK:STX_NORMAL,
				     pclbuf, pclsize) != -1)
				break;
			if (errno != ENOSPC) {
				perror("backup");
			 	exit(2);
			}
			pclsize = pclbuf->pcl_len;
			free((void *)pclbuf);
		}
		pclsize = pclbuf->pcl_len;
	} 

	if (oflag) {
		/* user specified version 2 headers so use old hdr definitions */
		/* ATTN: THESE ASSIGNMENTS SHOULD BE TYPECAST TO THE OLD TYPE DEFS */
		if (check_vers2(statb,name)) {
			hdr_rec.h.type    = FS_NAME;
			hdr_rec.n.ino     = (ushort)statb.st_ino;
			hdr_rec.n.uid     = (ushort)statb.st_uid;
			hdr_rec.n.gid     = (ushort)statb.st_gid;
			hdr_rec.n.mode    = (ushort)(statb.st_mode & ~IFJOURNAL);
			hdr_rec.n.nlink   = (short)statb.st_nlink;
			hdr_rec.n.size    = statb.st_size;
			hdr_rec.n.atime   = statb.st_atime;
			hdr_rec.n.mtime   = statb.st_mtime;
			hdr_rec.n.ctime   = statb.st_ctime;
			hdr_rec.n.devmaj  = (ushort)major(statb.st_dev);
			hdr_rec.n.devmin  = (ushort)minor(statb.st_dev);
			hdr_rec.n.rdevmaj = (ushort)major(statb.st_rdev);
			hdr_rec.n.rdevmin = (ushort)minor(statb.st_rdev);
			hdr_rec.n.dsize   = statb.st_size;
		}
		else
			return 0;
	}
	else {
		hdr_rec.h.type    = FS_NAME_X;
		hdr_rec.nx.ino     = statb.st_ino;
		hdr_rec.nx.uid     = statb.st_uid;
		hdr_rec.nx.gid     = statb.st_gid;
		hdr_rec.nx.mode    = statb.st_mode;
		hdr_rec.nx.nlink   = statb.st_nlink;
		hdr_rec.nx.size    = statb.st_size;
		hdr_rec.nx.atime   = statb.st_atime;
		hdr_rec.nx.mtime   = statb.st_mtime;
		hdr_rec.nx.ctime   = statb.st_ctime;
		hdr_rec.nx.devmaj  = major(statb.st_dev);
		hdr_rec.nx.devmin  = minor(statb.st_dev);
		hdr_rec.nx.rdevmaj = major(statb.st_rdev);
		hdr_rec.nx.rdevmin = minor(statb.st_rdev);
		hdr_rec.nx.dsize   = statb.st_size;
	}

	i = (statb.st_mode & S_IFMT);
	if ( i == S_IFSOCK ) {
		fprintf(stderr, MSGSTR(SOCK_TYPE,
		"backup: %s socket will not be backed up.\n"),name);
		if (!oflag) {
			free(aclbuf);
			free(pclbuf);
		}
		return(0);
	}
		
	if((i != S_IFREG) && (i != S_IFLNK)) {

		if (i == S_IFDIR) {
			if (oflag) 
				hdr_rec.n.size = 0;
			else 
				hdr_rec.nx.size = 0;
			cursize = 0;
		}

		/* write header */
		puthdr(&hdr_rec, NOT_PACKED, name);

		if (!oflag)  {
			/* write security header */
			putsechdr(aclsize, pclsize);
		
			/* write acl information */
			putaclhdr(aclbuf, aclsize);
			free(aclbuf);

			/* write pcl information */
			putpclhdr(pclbuf, pclsize);
			free(pclbuf);
		}
		return 1;
	}
	else
	if (i == S_IFLNK ) {
		if (strlen(name) >= NAMESZ) {
			fprintf(stderr, MSGSTR(FILTOL, 
			"backup: %s filename too long\n"), name);
			if (!oflag) {
				free(aclbuf);
				free(pclbuf);
			}
			return 0;
		}
		strcpy(linkname, name);
		if (statb.st_size + 1 > NAMESZ) {
			fprintf(stderr, MSGSTR(SYLNTO, 
			"backup: %s symbolic link too long\n"), name);
			if (!oflag) {
				free(aclbuf);
				free(pclbuf);
			}
			return 0;
		}
		i = readlink(name,linkname, NAMESZ -1);
		if (i < 0) {
			fprintf(stderr, MSGSTR(CANTSY, 
				"backup: can't read symbolic link:"));
			perror(name);
			ExitCode=1;
			if (!oflag) {
				free(aclbuf);
				free(pclbuf);
			}
			return 0;
		}
		linkname[i] = '\0';

		dbg3("a %s symbolic link to %s\n",name,linkname);

		if (oflag)
			hdr_rec.n.size = i;
		else {
			free(aclbuf);
			free(pclbuf);
			hdr_rec.nx.size = i;
		}
		cursize = i;
		puthdr(&hdr_rec, NOT_PACKED, name);
		wmedium((dword *)linkname, btow(i));
		return 1;
	}

	/* Open the file that we are going to back up */
	if ((fd = open(name, O_RDONLY))<0) {
		fprintf(stderr, MSGSTR(OPNER, "cannot open %s: "), name);
		perror(NULL);
		ExitCode=1;
		if (!oflag) {
			free(aclbuf);
			free(pclbuf);
		}
		return 0;
	}

	/* If initpack fails or decides packing is not worth
	 * the trouble then, backup the file in the normal
	 * fashion
	 * novalidRE is initialized to 1 and should only be zero when
	 * regcomp() successfully compiled the user specified a RE to 
	 * exclude files from packing.  Therefore we will always skip
	 * the regexec() portion of the conditional unless there is
	 * something to compare against.
	 */
	if (pflag && statb.st_size &&
	   (novalidRE || regexec(&pexcl,name,(size_t)0,NULL,0)) && 
	    initpack(fd, &size)) {
		int packread();

		readfunc = packread;
		bytes = size;
		if (oflag)
			hdr_rec.n.dsize = size;
		else
			hdr_rec.nx.dsize = size;
		i = PACKED;
	} else {
		extern read();

		readfunc = read;
		bytes = statb.st_size;
		i = NOT_PACKED;
	}

	/* write header */
	puthdr(&hdr_rec, i, name);

	if (!oflag) {
		/* write security header */
		putsechdr(aclsize, pclsize);

		/* write acl information */
		putaclhdr(aclbuf, aclsize);
		free(aclbuf);

		/* write pcl information */
		putpclhdr(pclbuf, pclsize);
		free(pclbuf);
	}

	while (bytes) {
		register unsigned nwant;
		register ngot;

		nwant = min(BSIZE, bytes);
		ngot = (*readfunc)(fd, buf, nwant);
		if (ngot < 0) {
			fprintf(stderr, MSGSTR(BADREAD,"Bad read?\n"));
			perror(name);
			ExitCode=1;
			ngot = 0;
		}
		if (ngot < nwant)       /* changed size? */
			bzero(buf+ngot, nwant-ngot);
		wmedium((dword *)buf, btow(nwant));
		bytes -= nwant;
	}

	if (i == PACKED) 
		endpack();
	close(fd);

	return 1;
}

check_vers2(struct stat statb,char *name)
{
	int badvalue=0;

	if (statb.st_ino > USHRT_MAX) {
		fprintf(stderr, MSGSTR(INO,
			"%s inode too large for Version 2: %ld\n"),
			name, statb.st_ino);
		badvalue++;
	}
	if (statb.st_nlink > SHRT_MAX) {
		fprintf(stderr, MSGSTR(NLINK,
			"%s nlink too large for Version 2: %ld\n"),
			name,statb.st_nlink);
		badvalue++;
	}
	if (statb.st_uid > USHRT_MAX) {
		fprintf(stderr, MSGSTR(UID,
			"%s uid too large for Version 2: %ld\n"),
			name,statb.st_uid);
		badvalue++;
	}
	if (statb.st_gid > USHRT_MAX) {
		fprintf(stderr, MSGSTR(GID,
			"%s gid too large for Version 2: %ld\n"),
			name,statb.st_gid);
		badvalue++;
	}
	if ((major(statb.st_dev)) > USHRT_MAX) {
		fprintf(stderr, MSGSTR(MAJDEV,
		"%s major device number too large for Version 2: %ld\n"),
			name, major(statb.st_dev));
		badvalue++;
	}
	if ((minor(statb.st_dev)) > USHRT_MAX) {
		fprintf(stderr, MSGSTR(MINDEV,
		"%s minor device number too large for Version 2: %ld\n"),
			name, minor(statb.st_dev));
		badvalue++;
	}
	if ((major(statb.st_rdev)) > USHRT_MAX) {
		fprintf(stderr, MSGSTR(MAJRDEV,
		"%s remote major device number too large for Version 2: %ld\n"), 
			name, major(statb.st_rdev));
		badvalue++;
	}
	if ((minor(statb.st_rdev)) > USHRT_MAX) {
		fprintf(stderr, MSGSTR(MINRDEV,
		"%s remote minor device number too large for Version 2: %ld\n"), 
			name, minor(statb.st_rdev));
		badvalue++;
	}

	if (badvalue) {
		fprintf(stderr, MSGSTR(DONTBKUP, 
			"backup: %s will not be backed up.\n"),name);
		return 0;
	}
	return 1;
}



omedium(int restart)
{
	static beenhere=0;

	if (beenhere++ && not_stdio && (close(med_fd) < 0)) {
		fprintf(stderr, MSGSTR(ERRCLOSE, "close failed "));
		perror(medium);
                exit(2);
	}
	v_volnum++;
	/* prompt will come from get_unit */
	get_unit(medium,restart,v_volnum);

	med_fd = open_medium(medium,O_RDWR|O_CREAT|O_TRUNC, 0666,v_volnum); 

	dbg3("Backup to Medium %s, FD = %d\n", medium, med_fd);

	buf_volinit(med_fd);
	if (verbose && not_stdio) {
		printf(MSGSTR(BAUPTO,
			"Backing up to %s\n"), medium);
		printf(MSGSTR(CLUST, "Cluster %d bytes (%d blocks).\n"),
			wtob(buf_len),cluster);
	}
	/* Here special and filsys are the same bogus value.
	 * Furthermore, the vol_rec fields v.disk and v.fsname are
	 * not used by restbyname for restoring byname archives but
	 * are used to report archive information when restoring RT
	 * style inode backups, which are restored with restbyname
	 * and not restbyinode.
	 */
	strncpy(vol_rec.v.disk, special, SIZSTR-1);
	strncpy(vol_rec.v.fsname, filsys, SIZSTR-1);
	strncpy(vol_rec.v.user, getuinfo("NAME"), SIZSTR-1);
	vol_rec.h.type   = FS_VOLUME;
	vol_rec.v.volnum = v_volnum;
	vol_rec.v.date   = v_date;
	vol_rec.v.dumpdate = v_dumpdate;
	vol_rec.v.numwds = (diskette) ? wrds_vol : MAXINT;
	vol_rec.v.incno  = v_incno;
	puthdr(&vol_rec, NOT_PACKED);
	if (verbose && not_stdio)
		printf(MSGSTR(VOLUME,"Volume %d on %s\n"), v_volnum, medium);
}


/*
 * write out a header
 */
void
puthdr(hp,pack,name)
register union fs_rec *hp;
int pack;
char *name;
{
	register sum, typ;
	register hlen, nlen;
	int i;

	hlen = hp->h.len = hdrlen[hp->h.type];
	typ = hp->h.type;
	if ((typ == FS_NAME_X) || (typ == FS_NAME)) {
		nlen = btow(strlen(name)+1);
		hp->h.len += nlen;
	}

	if (pack == PACKED)
		hp->h.magic = (ushort)PACKED_MAGIC;
	else
		hp->h.magic = (ushort)MAGIC;

	hp->h.checksum = 0;
	sum = csum(hp, hlen);
	if ((typ == FS_NAME_X) || (typ == FS_NAME)) 
		sum += csum(name, nlen);
	hp->h.checksum = sum;

	/*
	 * reorder the bytes to be machine-independent
	 */
	wshort(hp->h.magic);
	wshort(hp->h.checksum);

	switch (typ) {

	  case FS_VOLUME:
		wshort(hp->v.volnum);
		wlong(hp->v.date);
		wlong(hp->v.dumpdate);
		wlong(hp->v.numwds);
		wshort(hp->v.incno);
		break;
    
	  case FS_NAME:
		wshort(hp->i.ino);
		wshort(hp->i.mode);
		wshort(hp->i.nlink);
		wshort(hp->i.uid);
		wshort(hp->i.gid);
		wlong(hp->i.size);
		wlong(hp->i.atime);
		wlong(hp->i.mtime);
		wlong(hp->i.ctime);
		wshort(hp->i.devmaj);
		wshort(hp->i.devmin);
		wshort(hp->i.rdevmaj);
		wshort(hp->i.rdevmin);
		wlong(hp->i.dsize);
		break;

	  case FS_NAME_X:
		wshort(hp->nx.nlink);
		wlong(hp->nx.ino);
		wlong(hp->nx.mode);
		wlong(hp->nx.uid);
		wlong(hp->nx.gid);
		wlong(hp->nx.size);
		wlong(hp->nx.atime);
		wlong(hp->nx.mtime);
		wlong(hp->nx.ctime);
		wlong(hp->nx.devmaj);
		wlong(hp->nx.devmin);
		wlong(hp->nx.rdevmaj);
		wlong(hp->nx.rdevmin);
		wlong(hp->nx.dsize);
		break;


	  case FS_END:
	  case FS_VOLEND:
		/* no fields to reorder */
		break;

	  default:
		fprintf(stderr, MSGSTR(BACKERR,
			"backup error: reorder type %d\n"), typ);

	}

	wmedium((dword *)hp, hlen);
	if ((typ == FS_NAME_X) || (typ == FS_NAME)) 
		wmedium((dword *)name, nlen);
}

/*
 * return the checksum of area p, len words long
 * this must be independent of byte order, but should
 * at the same time give numbers with a reasonable
 * spread.
 */
csum(p, len)
register unsigned char *p;
register len;
{
	register c,sum;

	sum = 0;
	len = wtob(len);
	do {
		c = *p++;
		sum += (c << (c&7));
	} while (--len);

	return sum;
}

/* 
 * outfrag writes that part of the buffer that didn't
 * make it to the previous volume.
 */
void
outfrag(void)
{
	register dword *buffer;

	buffer = buf_save;
	if (fragleft >= 1)
		while(fragleft--) {
			if (buf_full()) {
				flushmedium();
				buf_reset(med_fd);
			}
			buf_append(buffer++);
		}
}


/*
 * wmedium writes the buffer b, len "words".
 */
wmedium(b,len)
register dword *b;
register len;
{
	if (len == 0) {
		if (flushmedium() == 1) {
			omedium(0);
			outfrag();
		}
		else 
			omedium(0);
		return;
	}

	while (len) {
		if (buf_full()) {
			if (flushmedium() == 1) {
				omedium(0);
				outfrag();
			}	
			else
				buf_reset(med_fd);
		}
		/* if diskette then we should never have any fragment
		 * that needs to be written out as the first data
		 */
		if (diskette &&
		   ((wtob(buf_1stwd) + (buf_ptr - buf_start)) >= wrds_vol)) {
				findex(TEND);
				flushmedium();
				omedium(0);
		}
		buf_append(b++); 
		len--;
	}
}

/*
 * flush the current output buffer
 */
flushmedium(void)
{
	if (!buf_empty())
		return(buf_write(med_fd));
}

/*
 * findex maintains the volume index in vol_rec.
 * commands are:
 *      TBEG            start a track
 *      TINS            install an inumber/adress pair in index
 *      TEND            about to finish with this track
 *
 * we write an empty index at the beginning of each volume.  when
 * we have installed enough inode-address pairs, we go back and
 * overwrite the empty index, then write another empty index at
 * the current location.  sometimes, there isn't enough room to
 * write an empty index, but there IS enough room for a file or
 * two.  in this case, "noroom" is set, and the last files don't
 * get indexed.  at the end of the volume, we go back and fill
 * out the most recent empty index, IF we have something to
 * add to it at all.  return success or failure.
 */
findex(cmd, ino)
register cmd;
int ino;
{
	daddr_t here, link;
	static numi;            /* number of valid entries in index */
	static daddr_t lastx;   /* where is last index */
	static char noroom;     /* no room for more indexing */
	char wdummy;            /* write a dummy header? */
	char wreal;             /* write a real header? */
	char success;

	if (!diskette)
		return 1;
	success = 1;

	wdummy = wreal = 0;
	here = buf_tell();
	switch (cmd) {

	/*
	 * beginning of track: write out empty index, to be filled
	 * out later.
	 */
	  case TBEG:
		noroom = 0;
		++wdummy;
		break;

	/*
	 * end of track: write out what we have so far
	 * then set noroom, in case next call is also TEND
	 * (with no index between the two TEND calls)
	 */
	  case TEND:
		if (noroom || numi == 0)
			break;
		link = 0;
		++wreal;
		++noroom;
		break;

	/*
	 * install an inode/address pair in the index.
	 * if the previous index is full, and there's not enough
	 * space for another index, noroom is set.
	 * the inumber has already been reordered, so we have to
	 * undo it.
	 */
	  case TINS:
		if (noroom) {
			success = 0;
			break;
		}
		vol_rec.x.ino[numi] = ino;
		vol_rec.x.addr[numi] = here;
		++numi;
		if (numi == FXLEN) {
			/* room for future indexing? */
			if (!buf_fits(hdrlen[FS_FINDEX]+1)) {
				link = 0;
				noroom = 1;
			} 
			else {
				link = here;
				vol_rec.x.addr[numi-1] += hdrlen[FS_FINDEX];
				++wdummy;
			}
			++wreal;
		}
		break;

	}

	/* write a real index? */
	if (wreal) {
		vol_rec.x.link = link;
		vol_rec.h.type = FS_FINDEX;
		buf_oseek(med_fd,lastx);
		puthdr(&vol_rec, NOT_PACKED);
		buf_oseek(med_fd,here);
	}

	/* write an empty index? */
	if (wdummy) {
		bzero((char *)vol_rec.x.ino, FXLEN*sizeof(ino_t));
		vol_rec.x.link = 0;
		vol_rec.h.type = FS_FINDEX;
		puthdr(&vol_rec, NOT_PACKED);
		lastx = here;
		numi = 0;
	}
	return success;
}

/*
 * recover from a tape/disk/floppy failure by restarting from the
 * beginning of the current volume
 */
void
volfail(s)
char *s;
{
	dbg2("ERRNO = %d .\n", errno);
	fprintf(stderr, "%s: %s\n", s, strerror(errno));
	quit(MSGSTR(MEDIAERR,"Check backup media and rerun the backup\n"));
}


/*
 *      Huffman encoding program
 *      Adapted April 1979, from program by T.G. Szymanski, March 1978
 */

#define END     256
#define BLKSIZE BUFSIZ
#define HPACKED 017436 /* <US><RS> - Unlikely value */
#define BYTE	8
#define MASK	0377
/*
 * SHIFT and MAXLEV both depend on the size of an unsigned long.  This
 * determines the maximum length of an encoding string.
 */
#define SHIFT	32
#define MAXLEV	32
unsigned long    bits [END+1];
unsigned long    mask;
unsigned long    inc;

/* union for overlaying a long int with a set of four characters */

/* character counters */
size_t    count [END+1];
size_t    insize;
size_t    filesz;
size_t    outsize;
int     diffbytes;

/* i/o stuff */
char    *infile;                /* unpacked file */

/* variables associated with the tree */
int     maxlev;
int     levcount [MAXLEV+1];
int     lastnode;
int     parent [2*END+1];

/* variables associated with the encoding process */
char    length [END+1];

/* the heap */
int     n;
struct  heap {
	long int count;
	int node;
} heap [END+2];
#define hmove(a,b) {(b).count = (a).count; (b).node = (a).node;}

/* gather character frequency statistics */
input ()
{
	register int i;
	register char *cp = infile;
	for (i=0; i<END; i++)
		count[i] = 0;

	for(i = 0 ; i < filesz; i++)
		count[*cp++ & MASK]++;
}


/* Function: void packerror(void)
 *
 * Description: The packerror() function prints out
 * a message about the occurance of an internal packing
 * error and exits with a non-zero return code.
 * 
 * Return value: The function does not return.
 *
 * Side effects: None.
 */
void
packerror(void)
{
	fprintf(stderr, MSGSTR(INTERN, 
		"An internal packing error occurred.\n"));
	exit (2);
}


/*
 * This function is coded as a co-routine of dfile.
 * The state of this thread is saved in a static variable.
 * The first time the function is called the thread produces
 * a first block of output.  The encoding tables must
 * fit in this first block, this assumption is also made
 * by restorx.c in its unpacking code.
 */
packread (fd, buf, nbytes)
int	fd;
char	*buf;
int	nbytes;
{
	static int	firsttime = 1;
	static int	c, i, inleft;
	static char	*inp;
	static char	**q, *outp;
	static int	bitsleft;
	static int	j;
	static size_t	chkoutsize;

	char		*endbuf = buf + nbytes;

	outp = buf;

	if (!firsttime) 
		goto resume;
	
	firsttime = 0;
	chkoutsize = 0;

	*outp++ = maxlev;
	for (i=1; i<maxlev; i++) {
		if (outp >= endbuf)
			packerror();
		*outp++ = levcount[i];
	}
	if (outp >= endbuf)
		packerror();
	*outp++ = levcount[maxlev]-2;
	for (i=1; i<=maxlev; i++)
		for (c=0; c<END; c++)
			if (length[c] == i) {
				if (outp >= endbuf)
					packerror();
				*outp++ = c;
			}

	/* output the text */

	bitsleft = BYTE;
	inleft = 0;

	inp = infile;

	for(i = 0; i <= filesz; i++)
	{
		c = (i == filesz)? END : *inp++ & MASK;

		mask = bits[c];
		if (bitsleft == BYTE)
			*outp = 0;
		*outp |= (mask>>(SHIFT-bitsleft))&MASK;
		mask <<= bitsleft;
		bitsleft -= length[c];
		while (bitsleft < 0) {
			if (++outp >= endbuf) {
				chkoutsize += outp - buf;
				return nbytes;
			}
resume:
			*outp = (mask>>(SHIFT-BYTE))&MASK;
			bitsleft += BYTE;
			mask <<= BYTE;
		}

	}
	if (bitsleft < BYTE)
		outp++;

	firsttime = 1;
	chkoutsize += outp - buf;
	if (chkoutsize != outsize)
		packerror();

	return nbytes;
}

/* makes a heap out of heap[i],...,heap[n] */
heapify (i)
{
	register int k;
	int lastparent;
	struct heap heapsubi;
	hmove (heap[i], heapsubi);
	lastparent = n/2;
	while (i <= lastparent) {
		k = 2*i;
		if (heap[k].count > heap[k+1].count && k < n)
			k++;
		if (heapsubi.count < heap[k].count)
			break;
		hmove (heap[k], heap[i]);
		i = k;
	}
	hmove (heapsubi, heap[i]);
}

/* return 1 after successful creation of the pack code, 0 otherwise */
int mkpackcode ()
{
	register int c, i, p;
	size_t bitsout;

	/* gather frequency statistics */
	input();

	/* put occurring chars in heap with their counts */
	diffbytes = 0;
	insize = 0;
	/* Add the 'END' character to the heap with weight 0. */
	count[END] = 0;
	parent[END] = 0;
	heap[1].count = 0;
	heap[1].node = END;
	/* Add remaining characters to the heap. */
	n = 1;
	for (i=0; i<END; i++) {
		parent[i] = 0;
		if (count[i] > 0) {
			diffbytes++;
			insize += count[i];
			heap[++n].count = count[i];
			heap[n].node = i;
		}
	}
	if (diffbytes == 0) {
		return (0);
	}
	for (i=n/2; i>=1; i--)
		heapify(i);

	/* build Huffman tree */
	lastnode = END;
	while (n > 1) {
		parent[heap[1].node] = ++lastnode;
		inc = heap[1].count;
		hmove (heap[n], heap[1]);
		n--;
		heapify(1);
		parent[heap[1].node] = lastnode;
		heap[1].node = lastnode;
		heap[1].count += inc;
		heapify(1);
	}
	parent[lastnode] = 0;

	/* assign lengths to encoding for each character */
	bitsout = maxlev = 0;
	for (i=1; i<=MAXLEV; i++)
		levcount[i] = 0;
	for (i=0; i<=END; i++) {
		c = 0;
		for (p=parent[i]; p!=0; p=parent[p])
			c++;
		levcount[c]++;
		length[i] = c;
		if (c > maxlev)
			maxlev = c;
		bitsout += c*count[i];
	}
	/*
	 * There is actually 1 'END' character, but it is stored with a
	 * weight of 0 so that it will sink to the bottom of the tree.
	 */
	bitsout += length[END];
	if (maxlev > MAXLEV) {
		/* can't occur unless insize >= 2**32 */
		return(0);
	}

	/* don't bother if no compression results */
	outsize = ((bitsout+7)>>3)+1+maxlev+diffbytes;
	if ((insize+BLKSIZE-1)/BLKSIZE <= (outsize+BLKSIZE-1)/BLKSIZE) {
		return(0);
	}

	/* compute bit patterns for each character */
	inc = 1L << (SHIFT-maxlev);
	mask = 0;
	for (i=maxlev; i>0; i--) {
		for (c=0; c<=END; c++)
			if (length[c] == i) {
				bits[c] = mask;
				mask += inc;
			}
		mask &= ~inc;
		inc <<= 1;
	}

	return 1;
}

initpack(fd, size)
int fd;
int *size;
{
	struct stat sb;

	dbg1("Entering initpack\n");
	*size = 0;
	if ((fstat(fd, &sb) < 0) ||
		(int)(infile = shmat(fd, (char *)0, SHM_MAP|SHM_RDONLY)) < 0 ||
		*(short *)infile == HPACKED)
	{
		dbg1("Already packed? or shmget failed\n");
		return 0;
	}

	dbg1("All open initialization ok\n");

	filesz = sb.st_size;

	if(mkpackcode()) {
		*size = outsize;
		return 1;
	}

	return 0;
}

endpack()
{
	if (shmdt (infile) == -1) {
		perror(MSGSTR(SHMDT,"backup: shmdt failed\n"));
		exit (2);
	}
}

int check_medium(int fd)
{
	static int sector = 0;
        int not_device;

        /*
         * Don't complain if not a special file, just set
         * a flag so we can deal with it later
         */
        not_device = 0;

        if (ioctl(fd, IOCINFO, &devinfo) == -1) {
                not_device = 1;
		return(1);
	}

	if (devinfo.devtype == DD_SCTAPE || devinfo.devtype == DD_TAPE)
		return(1);

	if (sector && (sector != devinfo.un.dk.secptrk)) {
		fprintf(stderr, MSGSTR(CAPAC,
			"Diskette capacity must be consistent for all drives\n"));
		return(0);
        } else {
                /* only need to set the sector size */
                /* when it is a special file */
                if (!not_device)
                        sector = devinfo.un.dk.secptrk;
                return(1);
        }

}


/* write security header to medium */
putsechdr(aclsize, pclsize)
int aclsize;
int pclsize;
{
	struct sac_rec s;
	s.aclsize = btow(aclsize);
	wlong(s.aclsize);
	s.pclsize = btow(pclsize);
	wlong(s.pclsize);
	wmedium((dword *)&s, btow(sizeof(s)));
}

/* write acl header to medium */
putaclhdr(ap, aclsize)
struct acl *ap;
int aclsize;
{
	reorderacl(ap);
	wmedium((dword *)ap, btow(aclsize));
}

/* write pcl header to medium */
putpclhdr(pp, pclsize)
struct priv *pp;
int pclsize;
{
	reorderpcl(pp);
	wmedium((dword *)pp, btow(pclsize));
}

reorderacl(ap)
struct acl *ap;
{
	struct acl_entry *acep, *nxt_acep, *ace_end;

	wlong(ap->acl_mode);
	wshort(ap->acl_rsvd);
	wshort(ap->u_access);
	wshort(ap->g_access);
	wshort(ap->o_access);

#define	ace_last acl_last	/* Due to an unfortunate choice of mnuemonics	*/
#define ace_nxt acl_nxt		/* the last ace is referred to in acl.h as the	*/
				/* last acl					*/
	acep = ap->acl_ext;	/* acep points to first acl entry	*/
	ace_end = ace_last(ap);	/* ace_end points to last acl entry	*/

	/* redundant check (with for() loop) */
	if (acep >= ace_end) {
		wlong(ap->acl_len);
		return;
	}
	for ( ; acep < ace_end; acep = nxt_acep)
	{
		struct ace_id	*idp, *nxt_idp;
		struct ace_id	*idend;
		unsigned short tmp;
		struct dummy {
			unsigned short	ace_len;
			unsigned short	ace_bits;
			struct	ace_id	ace_id[1];
		} *bp;
		
		nxt_acep = ace_nxt(acep);
		idend = id_last(acep);
		bp = (struct dummy *)acep;
		wshort(bp->ace_bits);
		for (idp=acep->ace_id; idp<idend; idp=nxt_idp)
		{

			nxt_idp = id_nxt(idp);
			wshort(idp->id_len);
			wshort(idp->id_type);
		}
		wshort(acep->ace_len);
	}
	wlong(ap->acl_len);
}

reorderpcl(pp)
struct pcl *pp;
{
	struct pcl_entry *pcep, *nxt_pcep, *pce_end;

	wlong(pp->pcl_mode);			/* swap flags 			*/
	wlong(pp->pcl_default.pv_priv[0]);	/* swap priviledge bits		*/
	wlong(pp->pcl_default.pv_priv[1]);

#define	pce_last pcl_last	/* Due to an unfortunate choice of mnuemonics	*/
#define pce_nxt pcl_nxt		/* the last pce is referred to in priv.h as the	*/
				/* last pcl					*/
	pcep = pp->pcl_ext;			/* pcep points to first pcl entry	*/
	pce_end = pce_last(pp);			/* pce_end points to last pcl entry	*/

	/* redundant check (with for() loop) */
	if (pcep >= pce_end) {
		wlong(pp->pcl_len);
		return;
	}

	for ( ; pcep < pce_end; pcep = nxt_pcep)
	{
		struct pce_id	*idp, *nxt_idp;
		struct pce_id	*idend;

		nxt_pcep = pce_nxt(pcep);
		idend = pcl_id_last(pcep);
		wlong(pcep->pce_privs.pv_priv[0]);	/* swap priviledge bits			*/
		wlong(pcep->pce_privs.pv_priv[1]);
		for (idp = pcep->pce_id; idp < idend; idp = nxt_idp )
		{
			nxt_idp = pcl_id_nxt(idp);
			wshort(idp->id_type);
			wshort(idp->id_len);
		}
		wlong(pcep->pce_len);
	}
	wlong(pp->pcl_len);
}

void
buf_reset(fd) {
	register nbytes=wtob(buf_len);
	buf_ptr = buf_start;
	/* For very large amount of data, buf_1stwd might become negative,
	 * since the comparison is relative, we will just count double
	 * double word as the base unit.  buf_1stwd also changed to be
	 * unsigned long in util.h.  This can occur on the 3490 tape drive.
	 */
	buf_1stwd += btow(buf_len);
	if (buf_1stwd < high_wd) {
		if (read(fd, (char *)buf_start, (unsigned)nbytes) != nbytes)
			volfail(MSGSTR(VFREAD, "backup medium read error"));
		if (lseek(fd, -(off_t)nbytes, 1) == -1)
			volfail(MSGSTR(VFSEEK, "backup medium seek error"));
	}
}

buf_write(fd) {
	register int written, left;
	register dword *save_ptr, *pos_ptr;
	register daddr_t this_high;
	register nbytes = wtob(buf_len);

	if ((written = write(fd, (char *)buf_start, (unsigned)nbytes)) != nbytes) {
		if ((written < 0) && (errno == ENXIO)) 
			written = 0;

		/* now try to write again, if end of media condition is encountered,
 		 * we should get errno set to ENXIO, else the previous write 
 		 * encountered a media error.
 		 */
		if (write(fd, (char *)buf_start, (unsigned)nbytes) < 0) {
			if (errno == ENXIO) {	/* handle eom */
				left = fragleft = (buf_ptr - buf_start) - btow(written);
				save_ptr = buf_save;
				pos_ptr = buf_start + btow(written);
				if (left > 0)
					while(left--)
						*save_ptr++ = *pos_ptr++;
				if (high_wd <= buf_1stwd) blocks_tot += cluster;
				return(1);
			}
		}
		volfail(MSGSTR(VFWRIT,"backup medium write error"));
	}
	if (high_wd <= buf_1stwd) blocks_tot += cluster;
	this_high = buf_1stwd + btow(buf_len);
	if (this_high > high_wd) high_wd = this_high;
	return(0);
}

/* seek to word "wd" in the output file */
void
buf_oseek(fd, wd)
daddr_t wd;
{
	daddr_t hdwd;
	register nbytes = wtob(buf_len);
	/* what word will be at beginning of buffer? */
	hdwd = wd - (wd % buf_len);
	if (hdwd != buf_1stwd) {
		if (!buf_empty()) buf_write(fd);
		lseek(fd, (off_t)wtob(hdwd), 0);
		buf_1stwd = hdwd;
		if (high_wd < buf_1stwd) high_wd = buf_1stwd;
		if (buf_1stwd < high_wd) {
			if (read(fd,(char *)buf_start,(unsigned)nbytes)!=nbytes)
				volfail(MSGSTR(VFREAD,"backup medium read error"));
			if (lseek(fd, -(off_t)nbytes, 1) == -1)
				volfail(MSGSTR(VFSEEK, "backup medium seek error"));
		}
	}
	buf_ptr = buf_start + wd - hdwd;
}


/*
 * initialize the input buffering info
 * force a read (record 0) on next call to readinput
 * buf_start has RDMAX bytes in front of it so we can read logical blocks
 * (file headers, data blocks) that cross record boundaries
 */
void
buf_volinit(register fd)
{
	register unsigned old_buf_len = buf_len;
	/* first time? */
	if (buf_start==NULL) infomedium(fd);
	/* recalculate track/volume size? */
	else if (diskette && ioctl(fd, IOCINFO, &devinfo)!=-1) {
		init_diskette();
		if (old_buf_len != buf_len) {
			free((void *)(buf_start - EXTRA));
			buf_start = NULL;
		}
	}
	if (buf_start==NULL) {
		buf_start = (dword *)malloc((size_t)(wtob(buf_len + EXTRA)));
		buf_save = (dword *)malloc((size_t)wtob(buf_len));
		if (buf_start==NULL)
			quit(MSGSTR(QUIT5, "no space for output buffer\n"));
		buf_start += EXTRA;
	}
	buf_ptr = buf_start;
	buf_1stwd = 0;
	high_wd = 0;
}

/*
 * find out whatever we can about the backup medium, which might be a tape,
 * a diskette, or even a disk file or printer (?)
 */
void
infomedium(int fd)
{
	if (ioctl(fd, IOCINFO, &devinfo) == -1) devinfo.devtype='#';
	if (cluster == 0) cluster = DEF_CLUSTER;
	buf_len = btow(cluster * 512);
	switch (devinfo.devtype) {
	case DD_TAPE: 
	case DD_SCTAPE:
		clus_vol = 0;
		wrds_vol = 0;
		break;
	case DD_DISK:
		if ((devinfo.flags & DF_FIXED)==0)
			init_diskette();
		else /* fixed disk */
		{
			clus_vol = devinfo.un.dk.numblks / cluster;
			wrds_vol = clus_vol * WRDS_CLUSTER;
		}
		break;

	default: /* paper tape punches, printers, etc. */
		wrds_vol = MAXLONG;
		clus_vol = MAXLONG;
		break;
	}
}

/* removeable disk, presume 512-byte-sector diskette */
/* init_diskette() gets called for each volume */
void
init_diskette(void) {
	diskette++;
	cluster = devinfo.un.dk.secptrk;
	buf_len = btow(cluster * 512);
	if (!blk_limit) blks2use = devinfo.un.dk.numblks;
	if ((blks2use % cluster != 0) || (blks2use == 0))
		quit(MSGSTR(BADLIM, "The -l option value must be a multiple of %d, the number of sectors per track.\n"),cluster);
	clus_vol = blks2use / cluster;
	wrds_vol = clus_vol * WRDS_CLUSTER;
}


void
get_unit(medium, restart, volnum)
char *medium;
int restart, volnum;
{
	int unit;
	char *save;
	static char *unit_pos;
	char unit_string[15];
	static int min_unit, max_unit;
	static int range = 0;
	static int sector;
	int i, fd;
	struct devinfo devnfo;
	char msgbuf[NL_TEXTMAX];

	/* if stdio specified do an immediate return */
	if (strcmp(medium, "-") == 0)
		return;                  

	/* the first time we are called, we should examine the
	 * medium name to see if it is a range of drives
	 * normal: /dev/rmt0       range: /dev/rmt0-3 
	 */

	if (range) 
		unit = atoi(unit_pos) + !restart;
	else {
		save = medium;
		/* Go to the end of the medium string and look
		 * backwards for nn-nn to find a range of devices.
		 */
		medium += (strlen(medium)  -1);
		while (isdigit((int)*medium)) 
			medium--;
		if (*medium != '-') {
			min_unit = max_unit = atoi (medium+1);
			unit_pos = medium+1;
			medium = save;
		} else {
			/* range of units */
			range = 1;
			max_unit = atoi(medium+1);
			*medium-- = NULL;
			while (isdigit((int)*medium)) 
				medium--;
			min_unit = atoi(medium+1);
			unit_pos = medium+1;
			medium = save;
			for (i=min_unit; i<=max_unit; i++) {
				sprintf(unit_string, "%d", i);
				strcpy(unit_pos, unit_string); 
				if ((fd = open(medium, O_RDONLY))<0) {
					fprintf(stderr, MSGSTR(OPNER,
						"cannot open %s: "), medium);
					perror(NULL);
					exit(2);
				}
				ioctl(fd, IOCINFO, &devnfo);
				close(fd);
				if (devnfo.devtype == DD_DISK) {
					if (i == min_unit)
						sector = devnfo.un.dk.secptrk;
					else
					if (devnfo.un.dk.secptrk != sector) {
						fprintf(stderr, MSGSTR(CAPAC, 
						"Diskette capacity must be consistent for all drives\n"));
						exit(2);
					}
				}
			}
		}
		unit = min_unit;
	}
	if (unit > max_unit)
		unit = min_unit;

	if (range) {
		sprintf(unit_string, "%d", unit);
		strcpy(unit_pos, unit_string); 
		}

	if ((!rflag || volnum > 1) && unit==min_unit) {
		if (min_unit == max_unit)
			sprintf(msgbuf, MSGSTR(MOUNT, SNGL_VOL_MNT),
				volnum,medium);
		else
			sprintf(msgbuf, MSGSTR(MNT, MULT_VOL_MNT), volnum,
				volnum+(max_unit-min_unit), medium, max_unit );
		prompt(msgbuf);
	}
}

open_medium(medium,flags,mode,volno)
char *medium;
{
	register fd;
	char msgbuf[NL_TEXTMAX];

	if (strcmp(medium, "-") == 0)
		return((int)STDOUT_FILENO);

	not_stdio++;    /* I/O is not from stdin/stdout */

	for (;;) {
		fd = aud_open(medium,flags,mode);
		dbg4("OPEN: %s, FD= %d, errno= %d.\n", medium,fd,errno);
		if (fd >= 0) {
			if(!check_medium(fd))
				close(fd);
			else 
				break;
		}
		else {
			fprintf(stderr, MSGSTR(OPNER,"cannot open %s: "), medium);
			perror(NULL);

			/* If access permission denied, a file or directory in 
			 * path name does not exist, a part of the path name 
			 * expected to be a directory is not one, or the backup 
			 * media specified is a directory, then exit with an
			 * non zero return code.
		 	 */
			if (errno == EACCES || errno == ENOENT ||
 			    errno == ENOTDIR || errno == EISDIR)
		  		exit(2);
		}

		sprintf(msgbuf, MSGSTR(MOUNT,SNGL_VOL_MNT), volno,medium);
		prompt(msgbuf);
	}
	return(fd);
}

/*
 * Display the appropriate prompt, then check whether
 * or not we can get to the tty.  If the tty is not
 * available then exit with a non-zero return code
 * else get the user's response.
 */
void
prompt(char *msg)
{
	char ans[20];

	if (no_op)
		exit(2);

	fprintf(stderr, "%s",msg);
	/*
	 * We need to take care of the case where there may not
	 * be any tty attached to the process, then we need to
	 * exit.
	 */
	if (tty == -1)
		exit(2);

	/* flush the tty input to clear out any type-ahead */
	if (ioctl(tty, TCFLSH, 0) < 0) {
		perror(ttydev);
		exit(2);
	}

	if (read(tty, ans, sizeof(ans)) < 0) {
		perror(ttydev);
		exit(2);	
	}
}


/* print error message with formatting, then punt */
/*VARARGS*/
quit(va_alist) va_dcl {
        va_list vp;
        char buf[255];
        register char *fmt;
        va_start(vp);
        fmt=va_arg(vp,char*);
        vsprintf(buf,fmt,vp);
        va_end(vp);
        fprintf(stderr,buf);
        exit(2);
}

/*
 * RULES ARE:
 *	1) if medium is stdout and user is privileged then audit
 *	2) if medium is a device then audit
 *	3) use acquired group rights to open medium only if necessary
 */
aud_open(medium, flags, mode)
char *medium;
{
	int	output;
	struct stat	stbuf;

	if ((output = open(medium, flags, mode)) < 0)
		return(-1);

	if (fstat(output, &stbuf)) {
		close(output);
		return(-1);
	}

	if (((stbuf.st_mode & S_IFMT) == S_IFBLK) || 
	    ((stbuf.st_mode & S_IFMT) == S_IFCHR))
		must_audit = 1;

	return(output);
}
