static char sccsid[] = "@(#)36	1.58.1.11  src/bos/usr/sbin/restbyname/restbyname.c, cmdarch, bos41J, 9524C_all 6/13/95 09:24:16";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: restbyname
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

/*
 *      This is a utility to restore files and file systems from tape
 *      backups.  It is a counterpart to the "backup" program.  The format
 *      of the tape is described in <dumprestor.h>
 *
 * Note:
 *      There are many WARNING messages spread throughout this program.
 *      for efficiency's sake, there is very little rebuffering of data.
 *      This means that everyone shares common buffers and must be careful.
 *      You would be well advised to appreciate the WARNING messages before
 *      trying to make changes to this program.
 *
 */

#define _ILS_MACROS
#include <nl_types.h>
#include "rbyname_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_RBYNAME,N,S)
nl_catd catd;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <values.h>
#include <varargs.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/id.h>
#include <sys/fblk.h>
#include <sys/filsys.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/dir.h>
#include <dumprestor.h>
#include <sys/audit.h>
#include <sys/tape.h>
#include <locale.h>
#include <ctype.h>
#include <time.h>

/* file info */
char    medium[PATH_MAX+1];     /* input medium name                */
char    *ttydev  = "/dev/tty";  /* need to play with tty            */
int     med_fd;                 /* file descriptor for backup input */
int     tty = 0;                /* and for terminal                 */
int	input_type;             /* input type (ie. S_IFIFO)         */


/* flags */
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0
#ifdef DEBUG
int     debug = 0;      /* user requested debugging output      */
#endif
int     verbose;        /* user wants us to be talkative        */
int     not_stdio = 0;  /* restore medium is not stdin (=0)     */
int	must_audit = 0; /* does this get audited? */

#define STDIN   0       /*  Stdin as per stdio.h                */
#define STDOUT  1       /*  Stdout as per stdio.h               */
#define STDERR  2       /*  Stderr as per stdio.h               */

#define NILPTR(x) (x == 0 || *x == 0)

struct	nplinklist {
	char *name;
	struct nplinklist *next;
};

int	Aflag=FALSE;	/* retore busy files			*/
int     dflag;          /* restore entire dir trees             */
int	Zflag=FALSE;
FILE    *FileP;
char	FileName[PATH_MAX + 2]; /* allows to check size of file */
int	NumberOfFiles;
char	**np;
struct	nplinklist *npll;
struct	nplinklist *npllp;
struct	nplinklist *lastnp;

int     rflag;          /* used for the -q (quiet) flag         */
int	pflag = 0;	/* indicate tape positioning.		*/
int	Yflag = 0;
int	hitfilemark = 0;/* we have read the file mark		*/
int     hflag = 0;      /* modify file and archive timestamps   */
int	Xflag = 0;	/* Restore from arbitrary volume  	*/
int	Tflag = 0;      /* full table of contents specified     */
int	showbytes = 0;	/* Show bytes when listing files.	*/
int	gotavol = 0;	/* A valid volume has been seen         */
long    FSblocks;       /* file system size in blocks           */

/* status */
int     files_on_medium; /* number of files on backup medium       */
unsigned files_restored; /* number of files retrieved from medium  */
int     volno = 1;       /* expected backup medium volume number   */
ino_t   curino;          /* inode number of file being restored    */
daddr_t curbno;          /* logical block number being restored    */
unsigned char eovol;     /* end of volume encountered?             */
int	ExitCode = 0;    /* maintain exit status                   */

/* bit maps */
unsigned char   clrimap[MSIZ];  /* bit map for unallocated files   */
unsigned char   dumpmap[MSIZ];  /* bit map for backed up files     */

/* misc */
ushort  magic;          /* magic number to look for              */
ushort  pmagic;         /* packed magic number to look for       */
union fs_rec *thishd;   /* pointer to current header             */
union fs_rec *hbase;    /* pointer to last header read from tape */
union fs_rec *hdbuf;    /* buffer to safely hold current header  */

/* length of headers */
unsigned char hdrlen[] = {
	btow(sizeof(thishd->v)),         	 /* FS_VOLUME    */
	btow(sizeof(thishd->x)),        	 /* FS_FINDEX    */
	btow(sizeof(thishd->b)),         	 /* FS_CLRI      */
	btow(sizeof(thishd->b)),        	 /* FS_BITS      */
	btow(sizeof(thishd->oi)),       	 /* FS_OINODE    */
	btow(sizeof(thishd->on)-DUMNAME),	 /* FS_ONAME     */
	btow(sizeof(thishd->e)),         	 /* FS_VOLEND    */
	btow(sizeof(thishd->e)),        	 /* FS_END       */
	btow(sizeof(thishd->i)),        	 /* FS_DINODE    */
	btow(sizeof(thishd->n)-DUMNAME),	 /* FS_NAME      */
	btow(sizeof(thishd->ds)),       	 /* FS_DS        */
	btow(sizeof(thishd->nx)-DUMNAME), 	 /* FS_NAME_X    */
};

#define FS_MAX  (sizeof(hdrlen)/sizeof(hdrlen[0]))

int     no_op = 0;      /* set when no operator interaction permitted */

/* functions */
long    time();
union   fs_rec *readmedium();
dword *buf_read();

/* dummy routine */
null() {}

#define BR_RESTORE
#include "util.h"

#define OLDDIRSIZ 14
/* The version 2 and previous inode backups have the
 * fixed length directory entries.
 */
struct	olddirect {
	ushort	d_ino;
	char	d_name[OLDDIRSIZ];
};

void make_filelist(char *);
void reverse_tapehead(int);
void beep(int);

/*
 * main
 *      this routine collects arguments and then invokes the appropriate
 *      functions to do the real work.
 */
main(int argc, char **argv)
{
	register int c;
	short toc = 0;
	char command = '\0';

	(void) setlocale(LC_ALL,"");
	catd = catopen("rbyname.cat", NL_CAT_LOCALE);

	/* D73127 allocate enough space for hdbuf to hold information 
	 * containing file names that are NAMESZ in length. 
	 */

	hdbuf=malloc(sizeof(union fs_rec)+NAMESZ);

	while ((c = getopt(argc,argv,"AdDvpqYZ:f:TtX:rxhcb:C:S")) != EOF) {
		switch(c) {
#ifdef DEBUG
		  case 'D':     /* debug        */
			debug++;
#endif
		  case 'A':	/* restore busy files	*/
			Aflag = TRUE;
			break;
		  case 'v':     /* verbose      */
			verbose++;
			showbytes++;
			break;

		  case 'd':
			++dflag;
			break;

		  case 'h':
			++hflag;
			break;

		  case 'f':
			movestr(medium,optarg);
			break;

		  case 'Z':
			Zflag = TRUE;
			make_filelist(optarg);
			break;

		  case 'b':
		  case 'C':   /* specify cluster size */
			cluster = atoi( optarg );
			/*
			 * Check for max cluster size.
			 * Actually, this check is overkill
			 * since MAXBLK applies only to
			 * transfers to raw devices.
			 */
			if( cluster * 512 > ctob(MAXBLK) )
			{
				int xclus = cluster;
				cluster = (ctob(MAXBLK))/512;
				fprintf(stderr, MSGSTR(CLTOOLRG,
				"Warning: cluster size %d too large, reduced to %d\n"),
				xclus, cluster);
			}
			if( cluster < (BSIZE / 512))
			{
				int xclus = cluster;
				cluster = (BSIZE / 512);
				fprintf(stderr, MSGSTR(CLTOOSML,
				"Warning: cluster size %d too small, enlarged to %d\n"),
				xclus, cluster);
			}
			break;

		  case 'X':
			volno = atoi(optarg);
			++Xflag;
			goto check;

		  case 'T':
			++Tflag;
		  case 't':
			++toc;
			++verbose;
			/* fall through */

		  case 'r':             /* restore by name, whole volume */
		  case 'x':             /* restore by name */
		  check:

			if( command != '\0' && command != c )
			{
				fprintf(stderr, MSGSTR(OPTNC,
				"Option -%c invalid, -%c already given\n"), c, command);
				usage();
			}
			command = c;
			break;

		  case 'q':
			++rflag; /* -q is same as -r from backup */
			break;

		  case 'Y':
			Yflag++;
			break;

		  case 'p':
			pflag++;
			break;

		  /* If S flag is specified, no operator prompting permitted */
		  case 'S':		/* d23060 using defect p29160 */
			no_op++;
			rflag++; /* Implied by -S */
			break;

		  case '?':
		  default:
			usage();
		}
	}

	if( command == '\0' )
		command = 'r';

	if ((hflag) && (command != 'x' && command != 'X' && command != 'r'))
	    usage(); 

	/*
	 * "magic" is the magic number in the byte order
	 * that it will be on medium.
	 */
	magic = MAGIC;
	wshort(magic);
	pmagic = PACKED_MAGIC;
	wshort(pmagic);

	if NILPTR(medium) 
		movestr(medium,DEF_MEDIUM);

	switch( command ) {
	  case 'X': /* recover starting in mid backup     */
		switch (newmedium(0)) {
		  case BYNAME:
		       if (Zflag)
		       		extract(BYNAME, &np[0]);
		       else
		       		extract(BYNAME, &argv[optind]);
		       break;
		  case BYMD: {
		       fprintf(stderr, MSGSTR(MINFM,
		       "%s is in minidisk (-m) format.. no longer supported.\n"), medium);
		       exit(1);
		       }
	          case BYINODE:
		       /* not available for inode backups */
		       fprintf(stderr, MSGSTR(OPTNX,
		       "%s is in by-inode (-r) format.. -X option not available.\n"), medium);
		       exit(1);
	          default:
		       fprintf(stderr, MSGSTR(NOFORM,"Unknown format.\n"));
		       exit(1);
		}
		break;

	  case 'r':
		    ++dflag;
	  case 'x': /* extract files by name, -r is handled the same way. */
		switch (newmedium(0)) {
		  case BYNAME:
		       if (Zflag)
		       		extract(BYNAME, &np[0]);
		       else
		       		extract(BYNAME, &argv[optind]);
		       break;
		  case BYMD: {
		       fprintf(stderr, MSGSTR(MINFM,
		       "%s is in minidisk (-m) format.. no longer supported.\n"),medium);
		       exit(1);
		       }
		  case BYINODE:
		       /* all inode backups will be extracted through
			* the by name method
			*/
		       if (Zflag)
		       		extract(BYINODE, &np[0]);
		       else
		       		extract(BYINODE, &argv[optind]);
		  default:
		       fprintf(stderr, MSGSTR(NOFORM, "Unrecognized archive format.\n"));
		       exit(1);
		}
		break;

	  case 't': /* tape header only */
		newmedium(0);
		break;

	  case 'T': /* table of contents           */
		switch (newmedium(0)) {
		  case BYNAME:
		       extract(BYNAME, NULL);
		       break;
		  case BYMD: {
		       fprintf(stderr, MSGSTR(MINFM,
		       "%s is in minidisk (-m) format.. no longer supported.\n"),medium);
		       exit(1);
		       }
		  case BYINODE:
		       dirlist();
			break;
		  default:
                       fprintf(stderr, MSGSTR(NOFORM, "Unrecognized archive format.\n"));
                       exit(1);
		  }
		break;

	  default:
		usage();
	}

	if (not_stdio && verbose) {
		if (Tflag)
			fprintf(stderr, MSGSTR(ARCIVD,
				"    files archived: %u\n"), files_restored);
		if (!toc)
			fprintf(stderr, MSGSTR(RESTRD,
				"    files restored: %u\n"), files_restored);
	}

	if (pflag || Yflag)
		reverse_tapehead(pflag);

	if (not_stdio) 
		close(med_fd);
		
	exit(ExitCode);
}

/* Function: void reverse_tapehead(int)
 *
 * Description: This function repositions the tapehead
 * based on whether the user specified one of the -p or
 * -Y options.  The -p option causes the tapehead to be
 * respositioned back at the beginning of the archive image
 * just restored.  The -Y option causes the tapehead to
 * be repositioned on the BOT(begin of tape) side of the
 * filemark that marks the EOF of the archive image just
 * restored.
 *
 * Return value: This function returns no explicit value.
 * The function will exit with a non-zero return value
 * on an error.
 * 
 * Side effects: None.
 */
void
reverse_tapehead(int pflag)
{
	struct stop op;
	struct devinfo devinfo;
	int bot = 0;

	/* all of the reverse skips and forward skips are due to
	 * the way that our tape devices or device drivers work.  
	 * A reverse skip filemark (STRSF) will leave the tapehead 
	 * on the BOT side of a filemark.  If a read is done with 
	 * the tape head on the BOT side of the filemark then a 
	 * read() will fail. 
	 */
	if (ioctl(med_fd, IOCINFO, &devinfo) == -1) 
		devinfo.devtype = '#';

	/* Only done if archive media is tape */
	if ((devinfo.devtype == DD_TAPE) || (devinfo.devtype == DD_SCTAPE)) {

		/* for -p: if we have already read the end of archive 
		 * filemark then we will have to reverse skip over this 
		 * filemark as well as the filemark that was previous 
		 * to this archive image so set count to 2 otherwise 
		 * there is only 1 filemark to skip over the one preceding 
		 * the archive image.
		 *
		 * for -Y: if we have already read the end of archive 
		 * filemark then we will have to reverse skip over only 
		 * this filemark so set count to 1 otherwise since we
		 * haven't read the end of archive filemark yet and are
		 * therefore already where we want to be just set count
		 * to 0 
		 */
		if (hitfilemark)
			op.st_count = (pflag) ? 2 : 1;
		else
			op.st_count = (pflag) ? 1 : 0;

		op.st_op = STRSF;

		/* if the ioctl() to reverse the tapehead fails then
		 * we need to know if it failed because it hit the
		 * beginning of tape any other reason causes us to
		 * exit.  If we did hit BOT(begin of tape) then there
		 * won't be any preceding filemark that we need to
		 * forward skip over to be in position at the beginning
		 * of the archive image.
		 */
		if (ioctl(med_fd, STIOCTOP, &op) < 0) {
			/* EIO means that we got back to beginning of tape */
			if (errno != EIO) {
				fprintf(stderr, MSGSTR(IOCTLERR, 
					"Can't reverse tape record\n"));
				exit(1);
			} else
				/* set beginning of tape flag */
				bot++;
		}

		/* if we didn't hit BOT then forward skip to put the 
		 * tapehead at the beginning of the archive image and
		 * ready to read() valid data.
		 */
		if (pflag && !bot) {
			op.st_op = STFSF;
			op.st_count = 1;
			if (ioctl(med_fd, STIOCTOP, &op) < 0) {
				fprintf(stderr, MSGSTR(IOCTLT, 
					"Can't skip tape records\n"));
				exit(1);
			}
		}
	}
}


/* Function: void make_filelist(const char *)
 *
 * Description: This function will initializes the
 * global linked list of filenames, np.  The newline
 * seperated filenames will be read from either an 
 * input file or from stdin.  If no filenames
 * were in the file or on stdin np will be set to NULL.
 *
 * Return value: This function returns no explicit value.
 * The function will exit with a non-zero return value 
 * on an error.
 *
 * Side effects: The global filename list np will contain
 * the list of filenames to be restored.  If no filenames
 * were in the file or on stdin np will be NULL.
 */

void
make_filelist(const char *optarg)
{
	int i;

	if (strcmp(optarg, "-") == 0) {
		FileP = stdin;
		if ((tty = open(ttydev, O_RDWR)) < 0) {
			exit(1);
		}
	}
	else 
		if ((FileP = fopen(optarg, "r")) == NULL) {
			fprintf(stderr, MSGSTR(BADOPEN,"Cannot open file\n"));
			exit(1);
		}

	/* allocate the space for the structure */
	if ((npll = (struct nplinklist *) malloc(sizeof(struct nplinklist))) == NULL)
		quit(MSGSTR(QUIT7,"no space for internal buffer\n"));

	npll->next = (struct nplinklist *)NULL;
	npllp=npll;

	NumberOfFiles = 0;

	while (fgets(FileName, PATH_MAX + 3, FileP) != NULL) {
		if ((i = strlen(FileName)) > PATH_MAX + 1)
			quit(MSGSTR(NAMETOLONG,
			"%s exceeds the maximum path length: %d.\n"),FileName,PATH_MAX);
		else
			FileName[i-1]='\0'; /* remove the new line */

		NumberOfFiles++;
        	/* get the space for the file name and copy the file name */
        	if ((npllp->name = (char *)malloc(strlen(FileName)+1)) == NULL)
                	quit(MSGSTR(QUIT7,"no space for internal buffer\n"));

        	strcpy(npllp->name, FileName);

        	/* save current pointer get next pointer and set next pointer */
        	lastnp = npllp;
		if ((npllp=(struct nplinklist *)malloc(sizeof(struct nplinklist)))==NULL)
			quit(MSGSTR(QUIT7,"no space for internal buffer\n"));

		lastnp->next = npllp;
	}

	/* get the space for the list of pointers */
	if ((np = (char **) malloc(sizeof(char *) * (NumberOfFiles+1))) == NULL)
		quit(MSGSTR(QUIT7,"no space for internal buffer\n"));

	/* put the pointer to the file name in to the list of pointers */
	npllp=npll;
	for (i=0; i<NumberOfFiles; i++) {
		np[i]=npllp->name;
		npllp=npllp->next;
	}
	np[NumberOfFiles] = (char *) NULL;
}


/*
 * bitson
 *      count the number of bits on in the specified bit map
 */
bitson( mp )
 register unsigned char *mp;
{       register int i, k;
	register unsigned char j;

	k = 0;
	for( i = 0; i < MSIZ; i++)
	{       if ((j = *mp++) == 0)
			continue;

		while( j )
		{       if (j&1)
				k++;
			j >>= 1;
		}
	}

	return( k );
}

/*
 * checksum
 *      validate the checksum on a header block
 */
checksum(hp)
 union fs_rec *hp;
{
	register ushort sum;
	register unsigned char *cp;
	register c;
	register len;
	ushort sumthere;
	char ans;

	sumthere = hp->h.checksum;
	hp->h.checksum = 0;
	rshort(sumthere);

	cp = (unsigned char *) hp;
	len = wtob(hp->h.len);
	sum = 0;
	do {
		c = *cp++;
		sum += (c << (c&7));
	}
	while (--len);

	ans = (sumthere == sum);
	wshort(sumthere);
	hp->h.checksum = sumthere;

	if (ans == 0)
		errmsg(MSGSTR(CKSUMR,"Checksum error\n"));

	return ans;
}


/*
 * getfile
 *      extract a file from medium, using the supplied functions to write
 *      full and empty blocks to the file on disk.  rearrange inumbers
 *      if this is a directory.
 */
getfile(hp, f1, f2)
	union fs_rec *hp;
	int (*f1)(), (*f2)(); {

	register bsz;
	register char *p;
	register struct olddirect *dp;
	register typ;
	off_t size;

	if (hp->h.type == FS_NAME_X) {
		curino = hp->nx.ino;
		size = (hp->h.magic == magic) ? hp->nx.size : hp->nx.dsize;
		typ = (hp->nx.mode & IFMT);
	}
	else {
		curino = hp->i.ino;
		size = (hp->h.magic == magic) ? hp->i.size : hp->i.dsize;
		typ = (hp->i.mode & IFMT);
	}
	curbno = 0;

	while (size) {
		bsz = min(size, BSIZE);
		p = (char *) readmedium(btow(bsz));

		/* reorder inumbers if directory */
		if (typ == IFDIR)
			for (dp = (struct olddirect *)(p + bsz);
			     --dp >= (struct olddirect *)p;  )
				rshort(dp->d_ino);

		if (bsz == size || !zero(p, bsz))
			(*f1)(p, bsz);
		else    (*f2)(p, bsz);
		size -= bsz;
	}

	gethead(0);
	curino = 0;
}

/*
 * does p contain s zeroes?
 */

zero(p, s)
register char *p;
register int s;
{
	do {
		if (*p++)
			return 0;
	} while (--s);
	return 1;
}

/*
 * does this header look reasonable?
 * be paranoid, since
 * if we think we have a header, but don't, the byte
 * reversals can destroy useful data
 */
goodhdr(hp)
	register union fs_rec *hp; {
	register typ;

	if ((hp->h.magic != magic && hp->h.magic != pmagic)
	    || (typ = hp->h.type) > FS_MAX)
		return 0;
	if (typ == FS_NAME || typ == FS_NAME_X || typ == FS_ONAME || typ == FS_DS) {
		if (hp->h.len < hdrlen[typ])
			return 0;
	} else  if (hp->h.len != hdrlen[typ])
			return 0;

	return 1;
}

/*
 * read the next medium record and see whether or not it is a header block
 */
gethead(sync)
{       register union fs_rec *hp;
	int thisvol;

	hp = readmedium(SIZHDR);

	 /*
	 If the Xflag is set and this is the first time we've 
         inspected the backup, verify it is a valid backup image
         */
	if (Xflag && !gotavol) {
		thisvol = hp->v.volnum;
		rshort(thisvol);
		if (thisvol == volno)
			gotavol++;
	}

	if (sync || (Xflag && gotavol)) {
		/* If we have a valid backup image, 
                   get to a header, no matter what.  */
		for (;;) {
			register lastrd;

			lastrd = SIZHDR;
			if (goodhdr(hp)) {
				buf_iseek(-SIZHDR);
				hp = readmedium(lastrd = hp->h.len);
				if (checksum(hp))
					break;
			}
			buf_iseek(1 - lastrd);
			hp = readmedium(SIZHDR);
		}
	} else {
		/* dont sync in case of error */
		if (!goodhdr(hp)) {thishd=0; return(0);}
		buf_iseek(-SIZHDR);
		hp = readmedium(hp->h.len);
		if (!checksum(hp)) {thishd=0; return(0);}
	}

        /* Save away a copy of this pointer and header.  hbase will be used
           later to peek at the next header.  The copy of the header is saved
           so if the buffer is refilled, the header information will not 
           be lost.
        */
	/* D73127 use memcpy to fill hdbuf to avoid truncation of filenames
	 * NAMESZ in length 
	 */
	hbase = hp;
        memcpy(hdbuf,hp,hp->h.len*sizeof(dword));
	hp = hdbuf;



	/*
	 * reorder the bytes so they make sense
	 */
	switch (hp->h.type) {
	  register i;

	  case FS_VOLUME:
		rshort(hp->v.volnum);
		rlong(hp->v.date);
		rlong(hp->v.dumpdate);
		rlong(hp->v.numwds);
		rshort(hp->v.incno);
#ifdef DEBUG
		if (debug)
			fprintf(stderr, " FS_VOLUME, volume %d\n", hp->v.volnum );
#endif
		break;

	  case FS_BITS:
	  case FS_CLRI:
		rshort(hp->b.nwds);
#ifdef DEBUG
		if (debug)
			fprintf(stderr, " FS_BITS/FS_CLRI, %d dwords\n", hp->b.nwds );
#endif
		break;

	  case FS_OINODE:
	  case FS_ONAME:
		rshort(hp->oi.ino);
		rshort(hp->oi.mode);
		rshort(hp->oi.nlink);
		rshort(hp->oi.uid);
		rshort(hp->oi.gid);
		rlong(hp->oi.size);
		rlong(hp->oi.atime);
		rlong(hp->oi.mtime);
		rlong(hp->oi.ctime);
		rshort(hp->oi.dev);
		rshort(hp->oi.rdev);
		rlong(hp->oi.dsize);
#ifdef DEBUG
		if (debug) {
			if (hp->h.type == FS_ONAME)
				fprintf(stderr, " FS_ONAME, name %s, size %ld\n",
					hp->on.name, hp->on.size );
			else    fprintf(stderr, " FS_OINODE, inum %d, size %ld, loc %ld\n",
					hp->oi.ino, hp->oi.size,
					buf_tell() - hp->h.len );
		}
#endif
		break;

	  case FS_NAME:
	  case FS_DINODE:
		rshort(hp->i.ino);
		rshort(hp->i.mode);
		rshort(hp->i.nlink);
		rshort(hp->i.uid);
		rshort(hp->i.gid);
		rlong(hp->i.size);
		rlong(hp->i.atime);
		rlong(hp->i.mtime);
		rlong(hp->i.ctime);
		rshort(hp->i.devmaj);
		rshort(hp->i.devmin);
		rshort(hp->i.rdevmaj);
		rshort(hp->i.rdevmin);
		rlong(hp->i.dsize);
#ifdef DEBUG
		if (debug) {
			if (hp->h.type == FS_NAME) 
				fprintf(stderr, " FS_NAME, name %s, size %ld\n",
					hp->n.name, hp->n.size );
			else    fprintf(stderr, " FS_DINODE, inum %d, size %ld, loc %ld\n",
					hp->i.ino, hp->i.size,
					buf_tell() - hp->h.len );
		}
#endif
		break;

	  case FS_NAME_X:
		rshort(hp->nx.nlink);
		rlong(hp->nx.ino);
		rlong(hp->nx.mode);
		rlong(hp->nx.uid);
		rlong(hp->nx.gid);
		rlong(hp->nx.size);
		rlong(hp->nx.atime);
		rlong(hp->nx.mtime);
		rlong(hp->nx.ctime);
		rlong(hp->nx.devmaj);
		rlong(hp->nx.devmin);
		rlong(hp->nx.rdevmaj);
		rlong(hp->nx.rdevmin);
		rlong(hp->nx.dsize);
#ifdef DEBUG
		if (debug) {
			if (hp->h.type == FS_NAME_X) 
				fprintf(stderr, " FS_NAME_X, name %s, size %ld\n",
					hp->nx.name, hp->nx.size );
		}
#endif
		break;

	  case FS_FINDEX:
		for (i=0; i<FXLEN; ++i) {
			rshort(hp->x.ino[i]);
			rlong(hp->x.addr[i]);
		}
		rlong(hp->x.link);
		break;

	  case FS_VOLEND:
#ifdef DEBUG
		if (debug)
			fprintf(stderr, " FS_VOLEND\n");
#endif
		++eovol;
		return gethead(sync);

	  case FS_END:
		/* no fields to reorder */
#ifdef DEBUG
		if (debug)
			fprintf(stderr, " FS_END\n");
#endif
		break;

	  case FS_DS:
		/* no fields to reorder */
#ifdef DEBUG
		if (debug)
		{
			fprintf(stderr, " FS_DS:\n");
			fprintf(stderr, " node %s, qdir %s\n",
			hp->ds.nid, &hp->ds.qdir[2]);
		}
#endif
		break;

	  default:
		errmsg(MSGSTR(PRREORD,
			"PROGRAM ERROR: reorder type %d\n"), hp->h.type);

	}

	thishd = hp;
	return(1);
}

/*
 * newmedium
 *      open a new medium for use
 *      return BYNAME, BYINODE, or BYMD depending on volume header
 *      Note that readmedium "knows" that wrds_vol is cleared
 *      by buf_volinit before gethead is called.
 */
newmedium(restart)
int restart;
{
	static int beenhere=0;
	int sav_rflag = rflag;  /* save the -q flag setting       p26930*/
	register firstfor=0;
	union fs_rec *hp_save;
	struct stat statbuf;
	union fs_rec save_header;
	int rc;
	struct tm *ltm;
	static char tmbuf[64];
	char * catmsg_str;   
	char * p;  
	char *index();

	save_header = *thishd;


	if (beenhere++ && not_stdio) close(med_fd);
	for(;;) {
		/* get volume mounted and verified */
		get_unit(medium, restart || firstfor++, volno);
		rflag = sav_rflag;       /* restore -q flag setting */
		/* now check out the tape to be sure it is OK */
		med_fd = open_medium(medium,O_RDONLY,0,volno);

		/* we need to know if input is from a pipe */
		if (fstat(med_fd, &statbuf) != 0) {
			fprintf(stderr, MSGSTR(STATFAIL, "Can't get info on %s.\n"), medium);
			exit(1);
		}
		input_type=statbuf.st_mode & S_IFMT;

#ifdef DEBUG
		if (debug)
	  		fprintf(stderr, "Restore from Medium %s, FD = %d\n", medium, med_fd);
#endif
		/* initialize input buffering */
		buf_volinit(med_fd);
		if (gethead(0) == 0)
			fprintf(stderr, MSGSTR(VOLBU,
			 "Volume on %s is not in backup format\n"),medium);
		else if (thishd->h.type != FS_VOLUME)
			fprintf(stderr, MSGSTR(BDNBW,
			 "Backup does not begin with FS_VOLUME record\n"));
		else if (thishd->v.volnum != volno)
			fprintf(stderr, MSGSTR(WRONG,
			 "Wrong volume - expecting volume %d\n"), volno );
		else break; /* for loop */
		if (not_stdio && (no_op == 0))
			close(med_fd);
		else
			exit(1);    /* dont loop when stdio */
		rflag = 0;          /* force user intervention */
	}

	wrds_vol = thishd->v.numwds;

	/*
	* Look for FS_DS header on the first volume.
	*/
	if( thishd->v.incno == BYNAME && thishd->v.volnum == 1 )
	{
		union fs_rec *nxthd;

		nxthd = (union fs_rec *)
			( (int)hbase + wtob(hdrlen[FS_VOLUME]) );

		/* Peek at the next header to see if it's an FS_DS. */

		if( nxthd->h.type == FS_DS )
		{
			char inbuf[NAMESZ];
#ifdef  DEBUG
			if( getenv("FS_DS") )
				fprintf(stderr,"Found FS_DS hdr\n");
#endif
			hp_save = thishd;
			if( gethead(0) == 0 )
			        quit(MSGSTR(FSDSER,"Can't read FS_DS header\n"));

			/* this will indicate if cat. msg. avail */
			catmsg_str = MSGSTR(OBSODSN,""); 
			if (*catmsg_str == '\0' )
				fprintf(stderr,
				"Warning: Obsolete Distributed Services remote backup format found.\n"
				"Enter <y> to continue : ");
			else
				fprintf(stderr,"%s", catmsg_str);
 
			bzero(inbuf,sizeof inbuf);
			read(tty, inbuf, sizeof inbuf ); 
			p=  index(inbuf,'\n');
			if ( p != NULL )
				*p = '\0';
			/* no message file - assume English */
			if ( *catmsg_str == '\0' ) {
				/* quit if other than yes */
				if (!(strcmp(inbuf,"y") == 0) )
					quit(MSGSTR(ABORT,"Restoration aborted\n"));
			} else 
				/* cat. present, use int'l resp. for 'yes' */
				if (!(rpmatch(inbuf) == 1)) 
					quit(MSGSTR(ABORT,"Restoration aborted\n"));
			thishd = hp_save;
		}
	}

	if (verbose && not_stdio) {       
		fprintf(stderr, MSGSTR(NEWVL,"New volume on %s:\n"), medium);
		fprintf(stderr, MSGSTR(CLUST,
			" Cluster %d bytes (%d blocks).\n"), wtob(buf_len), cluster );
		fprintf(stderr, MSGSTR(VOLNM,"    Volume number %d\n"), volno );
		ltm = localtime(&thishd->v.date);
		strftime(tmbuf,64,"%c",ltm);
		strcat(tmbuf,"\n");      
		fprintf(stderr, MSGSTR(DATEB,"    Date of backup: %s"), tmbuf);

		switch(thishd->v.incno) {
		case BYNAME:
			fprintf(stderr, MSGSTR(FBKNM,"    Files backed up by name\n"));
			fprintf(stderr, MSGSTR(USERA,"    User %s\n"), thishd->v.user);
			break;
		case BYINODE:
                        fprintf(stderr, MSGSTR(FBKIN,"    Files backed up by inode\n"));
                        ltm = localtime(&thishd->v.dumpdate);
                        strftime(tmbuf,64,"%c",ltm);
                        strcat(tmbuf,"\n");
                        fprintf(stderr, MSGSTR(FBFRM,"    Backed up from:  %s"), tmbuf );
                        fprintf(stderr, MSGSTR(DFSYS,"    Disk %s, Fsys %s, User %s\n"),
                                thishd->v.disk, thishd->v.fsname, thishd->v.user);
			break;
		case BYMD:
			fprintf(stderr, MSGSTR(MINFM,
				"%s is in minidisk format...no longer supported.\n:"),medium);
			exit(1);
		default:
			fprintf(stderr, MSGSTR(NOFORM, "Unrecognized archive format\n"));
			exit(1);
		}
	}

	rc = (int)thishd->v.incno;
	*thishd = save_header;

	return rc;
}


/*
 * read a bit map from the medium into m.
 * if m is NULL, we're just skipping the bits
 */
readbits(m)
 register unsigned char *m;
{
	register int i, n;
	register unsigned char *p;

	i = thishd->b.nwds;
	while (i) {
		n = min(i, btow(BSIZE));
		p = (unsigned char *) readmedium(n);
		i -= n;
		n = wtob(n);
		if (m)
			while (n--)
				*m++ = *p++;
	}
	gethead(0);                /* caller takes care of error */
}


/*
 * be sure we're at a header.  if not, seek to next one, and warn
 * user of possibly lost data.
 */

checkhead() {

	if (!thishd)
	{       errmsg(MSGSTR(MISSHDR,"Missing header block\n"));

	/*
	 * When ever an error is detected, we flush out the
	 *      buffers (to minimize possibility for damage)
	 *      and then try to resync by finding a new inode
	 *      record.
	 */
		errmsg(MSGSTR(RSYNC,"Attempting to resync after error ... "));

		for(;;)
		{       gethead(1);
			switch (thishd->h.type) {
			  case FS_END:
			    errmsg(MSGSTR(ENDOD,"END OF DUMP\n"));
			    return;
			  case FS_FINDEX:
			    errmsg(MSGSTR(RESYNC,"resync at file index\n"));
			    return;
			  case FS_ONAME:
			    errmsg(MSGSTR(RESNCF,
				   "resync at file %s\n"), thishd->on.name);
			    return;
			  case FS_NAME:
			    errmsg(MSGSTR(RESNCF,
				   "resync at file %s\n"), thishd->n.name);
			    return;
			  case FS_NAME_X:
			    errmsg(MSGSTR(RESNCF,
				   "resync at file %s\n"), thishd->nx.name);
			    return;
			  case FS_OINODE:
			  case FS_DINODE:
			    errmsg(MSGSTR(RESNCI,
				   "resync at inode %d\n"), thishd->i.ino);
			    return;
			  case FS_BITS:
			    errmsg(MSGSTR(RESNCBM,"resync at bit map\n"));
			    return;
			  case FS_CLRI:
			    errmsg(MSGSTR(RESNCCL,"resync at clri map\n"));
			    return;
			}
		}
	}

}

/*
 * input medium data buffering
 * eovol means don't read any more of this volume
 * watch out for the one-level recursion that is invoked by newmedium
 * also watch out for the interaction between prepare, fixup, and newmedium
 * If we are invoked by newmedium, and we at the beginning of
 * the tape, then we can fake out gethdr/goodhdr by returning
 * all zeros.
 * Here is the story: buf_prepare returns true if the data is available
 * to be picked up by buf_read.  It will have had side-effects on the
 * buffer if a block-cluster boundary is reached.  In the special case
 * where the volume ends in the middle of a request, the buf_fixup
 * routine has to adjust for the presence of a volume header.
 */
union fs_rec *
readmedium(len) 
{
	register was_eovol=eovol;
	static dword nullhdr;       /* init. to 0, guaranteed bad header */

	if (len > btow(RDTMAX)) 
		quit(MSGSTR(INTERR, "internal error: read request too large\n"));
	if (!eovol) 
		eovol = !buf_prepare(med_fd,len);
	if (eovol) {
		eovol=0;

		/* this condition only occurs reasonably during newmedium()
		 * and reflects the use of an empty medium 
		 */
		if (wrds_vol==0) 
			return((union fs_rec *)&nullhdr);
		++volno;
		newmedium(0);
		if (!was_eovol) 
			buf_fixup();
		if (!buf_prepare(med_fd,len)) 
			quit(MSGSTR(INTERRM, "internal error (read medium)\n"));
	}
	return ((union fs_rec *)buf_read(len));
}

usage()
{
	fprintf(stderr, MSGSTR(USAGE, "For by name backups:\n"));
	fprintf(stderr, MSGSTR(USAGE1, "\trestore -x[Mdqv] [-f device] [-s num] [-b num] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE2, "\trestore -T|-t [-qv] [-f device] [-s num] [-b num]\n"));
	fprintf(stderr, MSGSTR(USAGE3, "\trestore -X num [-Mdqv] [-f device] [-s num] [-b num] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE4, "For version 2 inode backups:\n"));
	fprintf(stderr, MSGSTR(USAGE5, "\trestore -r[d] [-f device] [file ...] \n"));
	fprintf(stderr, MSGSTR(USAGE6, "For inode backups:\n"));
	fprintf(stderr, MSGSTR(USAGE7, "\trestore -t[Bhvy] [-f device] [-s num] [-b num] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE8, "\trestore -x[Bhmvy] [-f device] [-s num] [-b num] [file ...]\n"));
	fprintf(stderr, MSGSTR(USAGE9, "\trestore -i[hmvy] [-f device] [-s num] [-b num]\n"));
	fprintf(stderr, MSGSTR(USAGE10, "\trestore -r[Bvy] [-f device] [-s num] [-b num]\n"));
	fprintf(stderr, MSGSTR(USAGE11, "\trestore -R[Bvy] [-f device] [-s num] [-b num]\n"));
	exit(1);
}

/* return 1 if buf_read OK, else 0 if new volume is needed;
 * len is in dwords 
 */ 
buf_prepare(fd,len)
register len;
{     
	register num, retval;

	/* read next buffer? */
	if (buf_ptr + len <= buf_start + buf_actual) 
		return(1);

	/* request goes over buffer boundary
	 * copy current data to area in front of buffer so
	 * can get entire block at once.  If we are processing a new
	 * volume, we temporarily supress the cluster-crossing logic 
	 */
	if (!buf_newvol) {
		num = buf_start + buf_actual - buf_ptr;      /* dwords left */
		memcpy((char *)(buf_back=buf_start-num),(char *)buf_ptr,wtob(num));
		buf_ptr = buf_back;
	}
	buf_1stwd += buf_actual;

	/* version 3 backbyname set the wrds_vol to MAXINT if the length and 
	 * density of tape are not specified.(this means backup wrote till end 
	 * of tape)  So here we only check to see if wrds_vol has been exceeded 
	 * when wrds_vol is not MAXINT.  Else we just read until end of tape.
 	 */
	if (wrds_vol!=0 && (wrds_vol != MAXINT && buf_1stwd >= wrds_vol))
		retval = 0;
	else
		retval = physread(fd);

	/* Even if physread returned ok, ie, 1, there may still not be enough
 	 * in the buffer to satisfy the request, must check against the length
 	 * of what we read and then readjust the buffer.
 	 */
	if ((buf_ptr + len > buf_start + buf_actual) && (buf_ptr <= buf_start)) {
		char *tmp;

		retval = 0;
		num = buf_actual + (buf_start - buf_ptr);

		/* new location to accommodate the fragment */
		buf_back -= buf_actual;	
		tmp = (char *)malloc(wtob(num));
		memcpy(tmp, (char *)buf_ptr, wtob(num));
		memcpy((char *)buf_back, tmp, wtob(num));
		buf_ptr = buf_back;
		(void)free(tmp);
	}
	if (retval==0 && wrds_vol!=0) 
		buf_newvol++;

	return(retval);
}

/* do physical read into buffer, and deal with errors return 0 if 
 * eof or a read error occurred.  In the case of an error the 
 * buffer is filled with zeroes and 0 is returned.
 */
physread(fd)
{
	register nbytes; 
	register read_cnt;
	char *wrptr;
	int total_read=0;

	read_cnt = wtob(buf_len);
	wrptr = (char *)buf_start;
	if(!not_stdio) {			/* read from stdio */
		while( read_cnt && ((nbytes = read(fd, wrptr, read_cnt)) > 0) )  {
			wrptr += nbytes;
			read_cnt -= nbytes;
		}
		/*
		 * D13590:
		 *  if bytes are read from a FIFO, use them.
		 *  otherwise, quit.
		 */
		if (input_type==S_IFIFO) {
			if (wrptr == (char *)buf_start)
				quit(MSGSTR(BADEOF,"No writer on input pipe.\n"));
			buf_actual = btow(wrptr - (char *)buf_start);
		} else {
			buf_actual = buf_len;
			if (wrptr == (char *)buf_start)
				return(0);
		}
		if(read_cnt && nbytes == 0)
			return(1);
	}
	else {				/* not stdio */
	/* 
 	 * we need to try to read up to buf_len, else buf_actual might
 	 * be smaller than the buf_read request.
 	 */
		while( read_cnt && ((nbytes = read(fd, wrptr, read_cnt)) > 0) )  {
			wrptr += nbytes;
			total_read += nbytes;
			read_cnt -= nbytes;
		}
		buf_actual = btow(total_read);
		if (nbytes == 0) hitfilemark++;
		if (buf_actual > 0) return(1);
	}
	if ((input_type==S_IFIFO) && (nbytes==0))
		quit(MSGSTR(BADEOF,"No writer on input pipe.\n"));
	if (nbytes==0) 
		return(0);
	if (nbytes==wtob(buf_len)) 
		return(1);
	if (diskette) {
		/* if reading a diskette, try one sector at a time. */
		register dword *p;        /* read to here */
		off_t loc = wtob(buf_1stwd) / 512;
		lseek(fd, loc*512, 0);
		for (p = buf_start; p < buf_start + buf_len; p += 512/sizeof(dword)) {
			loc++;
			if (read(fd, (char *)p, (unsigned)512) != 512) {
				errmsg(MSGSTR(RDER1,
				    "READ ERROR on sector %d, assuming zeros\n"),loc);
				bzero((char *)p, 512);
				lseek(fd, loc*512, 0);
			}
		}
		buf_actual = buf_len;
		return(1);
	}
	if (nbytes == -1) {
		perror(MSGSTR(RDER2,"READ ERROR"));
		if (!no_op) {
			errmsg(MSGSTR(ASS0CNT,"assuming zeros and continuing\n"));
			bzero((char *)buf_start, wtob(buf_len));
			buf_actual = buf_len;
		} else
			exit(1);
	}
	return(1);
}

/* reading the volume header has caused buf_volinit to be called again,
   so the header was read from "buf_start".  If we moved data to the area
   before "buf_start", it needs to be moved up a little, overlaying the now
   obsolete header so that the old and new data are contiguous */
buf_fixup() {
	register dword *p = buf_ptr, *q = buf_start;
	register cnt = buf_start - buf_back;
	while (cnt--) *--p = *--q;
	buf_back = buf_ptr = p;
	/* re-enable normal block crossing logic */
	buf_newvol = 0;
}

dword *buf_read(len) {  /* len is in dwords */
	register dword *ans = buf_ptr;
	buf_ptr += len;
	return(ans);
}

/* seek relative to current position, entirely within current buffer */
buf_iseek(wd) {
	if (buf_back <= buf_ptr+wd && buf_ptr+wd <= buf_start+buf_actual) {
		buf_ptr += wd;
		return;
	}
	errmsg(MSGSTR(SEEKERR,"SEEK ERROR: wd == %d\n"), wd);
}         /*MSG*/


/*
 * initialize the input buffering info
 * force a read (record 0) on next call to readinput
 * buf_start has RDMAX bytes in front of it so we can read logical blocks
 * (file headers, data blocks) that cross record boundaries
 */
buf_volinit(fd) register fd; 
{
	register unsigned old_buf_len = buf_len;
	/* first time? */
	if (buf_start==NULL) 
		infomedium(fd);
	else 
		if (diskette && ioctl(fd, IOCINFO, &devinfo)!=-1) {
			/* recalculate track/volume size? */
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
			quit(MSGSTR(QUIT5,"no space for output buffer\n"));
		buf_start += EXTRA;
	}
	buf_ptr = buf_start;
	buf_1stwd = 0;
	wrds_vol=0;
	buf_actual = 0;
}


/*
 * find out whatever we can about the backup medium, which might be a tape,
 * a diskette, or even a disk file or printer (?)
 */
infomedium(fd) {

	if (ioctl(fd, IOCINFO, &devinfo) == -1) devinfo.devtype='#';
	if (cluster == 0) cluster = DEF_CLUSTER;
	buf_len = btow(cluster * 512);
	switch (devinfo.devtype) {
	case DD_DISK:
		if ((devinfo.flags & DF_FIXED)==0)
			init_diskette();
		else {
		 	/* fixed disk */
			clus_vol = devinfo.un.dk.numblks / cluster;
			wrds_vol = clus_vol * WRDS_CLUSTER;
		}
		break;

	default: 
		/* for tape we just write to end of media
		 * so we don't care about words per volume
		 * or clusters per volume.
		 */
		clus_vol = 0;
		wrds_vol = 0;
		break;
	}
}

/* removeable disk, presume 512-byte-sector diskette */
/* init_diskette() gets called for each volume */
init_diskette() {
	diskette++;
	cluster = devinfo.un.dk.secptrk;
	buf_len = btow(cluster * 512);
	if (!blk_limit) blks2use = devinfo.un.dk.numblks;
	if (blks2use % cluster != 0)
		quit(MSGSTR(QUIT6, "sector limit %d not multiple of cluster %d\n"),
			blks2use,cluster);
	clus_vol = 0;
	wrds_vol = 0;
}

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
		while ( isdigit((int)*medium) ) 
			medium--;
		if ( *medium != '-' ) {
			min_unit = max_unit = atoi (medium+1);
			unit_pos = medium+1;
			medium = save;
		} else {
			/* range of units */
			range = 1;
			max_unit = atoi(medium+1);
			*medium-- = NULL;
			while ( isdigit((int)*medium) ) 
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
					perror("");
					exit(1);
				}
				ioctl(fd, IOCINFO, &devnfo);
				close(fd);
				if (devnfo.devtype == DD_DISK) {
					if (i == min_unit)
						sector = devnfo.un.dk.secptrk;
					else if (devnfo.un.dk.secptrk != sector) {
						fprintf(stderr, MSGSTR(CAPAC,
"Diskette capacity must be consistent for all drives\n"));
						exit(1);
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
			fprintf(stderr, MSGSTR(MOUNT,
"Please mount volume %d on %s\n... and press Enter to continue " ), volnum,medium);
		else
			fprintf(stderr, MSGSTR(MNT,
"Please mount volumes %d-%d on %s-%d\n...and press Enter to continue "), 
volnum, volnum+(max_unit-min_unit), medium, max_unit );

		beep(volnum);
		waitret();
	}
}

/* if restore -X, only beep if on subsequent volume. */
void
beep(int volume)
{
	if ( (volume > 1 ) && (!Xflag ||  (Xflag && gotavol)) )
		fprintf(stderr, "\7");
}


open_medium(medium,flags,mode,volno)
char *medium;
{
	int must_audit;
	register fd;

	must_audit = 0;
	if (strcmp(medium, "-") == 0)    /* std io specified*/
		return((int)STDIN);     /* return stdin    */

	not_stdio++;    /* I/O in not from stdin/stdout */

	for (;;) {
		/* EXPORT */
		fd = aud_open(medium,flags,mode);
#ifdef DEBUG
		if (debug) fprintf(stderr, "OPEN: %s, FD= %d, errno= %d.\n", medium,fd,errno);
#endif
		if (fd >= 0) break;
		fprintf(stderr, MSGSTR(OPNER, "cannot open %s: "), medium);
		perror("");

		fprintf(stderr, MSGSTR(MOUNT,
"Please mount volume %d on %s\n...and press Enter to continue "), volno,medium);

		beep(volno);
		waitret();
	}
	return(fd);
}

movestr(dst,src) char *dst,*src; 
{
	strncpy(dst,src,NAMESZ);
}

/* wait for "return" */

waitret() {
	char ans[20];

	/* if no operator intervention, then exit */
	if (no_op)  
		exit(1);
	/*
	 * Restore, for political and NOT technical reasons, must maintain
	 * "tty" as file descriptor 0. Thus, we check if it is possible to
	 * open a terminal at all.  If it is not possible, bail out here to
	 * avoid an infinite loop.
	 */
	if ((tty == 0 && open(ttydev, O_RDONLY) < 0) || tty == -1)
		exit(1);
	if (tty != -1) {
		ioctl(tty, TCFLSH, 0);
		if (read(tty, ans, sizeof(ans)) < 0)
			quit(MSGSTR(TTYERR,"error reading from the tty\n"));
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
        fprintf(stderr, buf);
        exit(1);
}

/* print error message with formatting, set non-zero return code */
/*VARARGS*/
errmsg(va_alist) va_dcl {
        va_list vp;
        char buf[255];
        register char *fmt;
        va_start(vp);
        fmt=va_arg(vp,char*);
        vsprintf(buf,fmt,vp);
        va_end(vp);
        fprintf(stderr, buf);
        ExitCode=1;
}

/*
 * RULES ARE:
 *	1) if medium is stdout and user is privileged then audit
 *	2) if medium is a device then audit
 *	3) use acquired group rights to open medium only if necessary
 */
aud_open(medium, flags, mode)
char	*medium;
{
	int	output;
	struct stat	stbuf;

	output = open(medium, flags, mode);
	if (output < 0)
		return(-1);

	if (fstat(output, &stbuf))
	{
		close(output);
		return(-1);
	}

	if (((stbuf.st_mode & S_IFMT) == S_IFBLK) || 
	    ((stbuf.st_mode & S_IFMT) == S_IFCHR))
		must_audit = 1;

	return(output);
}
