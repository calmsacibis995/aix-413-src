static char sccsid[] = "@(#)25	1.34.2.12  src/bos/usr/sbin/logredo/logredo.c, cmdfs, bos411, 9431A411a 8/1/94 19:30:55";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: logredo
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <jfs/log.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <jfs/inode.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <jfs/fsdefs.h>
#include <sys/vmount.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/vmdisk.h>
#include <sys/errno.h>
#include <stdlib.h>
#include <sys/lvdd.h>

#ifdef	_IBMRT
#define	LINESIZE	256
#endif	/* _IBMRT */
#ifdef	_POWER
#define	LINESIZE	128
#endif  /* _POWER */

#define	UZBIT	((uint) (1 << 31 ))
#define ONES    ((uint)0xffffffff)
#define min(A,B) ((A) < (B) ? (A) : (B))

#define COMSIZE   512
#define	BHASHSIZE 1024		/* must be a power of two	*/
#define NUMMINOR  256
#define NBUFPOOL  128
#define B_READ    0
#define B_UPDATE  1	

/* defines for hanfs duplicate cache records */

#define        MAXDUPSAVE        400	/* only save MRU 400 hanfs records    */
#define        DUPSAVECNT        8	/* each record is 8 ulongs wide       */
#define        DUPSAVESIZ        (DUPSAVECNT*sizeof(ulong))

/* error types
 */
#define MAPERR 	  0	
#define DBTYPE    1	
#define INOTYPE   2
#define READERR   3
#define UNKNOWNR  4
#define LOGEND    5
#define SERIALNO  6
#define OPENERR	  7	
#define IOERROR	  8	
#define LOGWRAP	  9

#define MINOR_ERROR -1		/* error that will not abort the program */
#define MAJOR_ERROR -2		/* return code to force abort of logredo */
#define REFORMAT_ERROR -3	/* return code to force logform of log	 */

/* committed transactions. entry stays in com table until first 
 * record of transaction is found (it has backchain = 0).
 */
int comhmask = 63; 		/* hash mask for comhash */  
int comhash[64];  		/* head of hash chains */
int comfree; 			/* index of a free com structure */
struct com
{	
	int tid;  		/* committed tid.  */
	int next; 		/* next on free list or hash chain */
} com[COMSIZE];

/* disk pages that have redo records have entry in doblk.       
 * the summary bit vector is set to the "OR" of all lines 
 * of redo log records encountered as log is read in backwards
 * direction. a redo log record is only processed for the
 * lines corresponding to summary bits which are not yet set,
 * so that only the last update to a line is applied.
 */

struct doblk
{
	int volid;  		/* volume id */
	int disk;   		/* page number (PAGESIZE UNITS) */
	uint  summary;  	/* bit vector of changed lines	*/ 
	struct doblk *next;	/* next entry on hash chain	*/
};

int blkhmask = (BHASHSIZE-1);	/* hash mask for blkhash */
struct doblk *blkhash[BHASHSIZE]; /* head of doblk hash chains */
int numdoblk;			/* number of do blocks used	*/

/* no redo array. journalled segments which were deleted.
 */
struct nodo
{
	int  volid;   		/* volume id			*/
	ino_t  inode;		/* inode number			*/
	struct nodo *next;	/* next entry on no do list	*/
}; 

struct nodo *nodolist;		/* allocate nodo blocks		*/
int numnodo;			/* number of nodo blocks used	*/	

/* open volume array. this program processes a single log. the 
 * logical volumes (i.e. filesystems) it pertains to are all in
 * same logical volume group, so volumes are uniquely specified
 * by their minor number, and the maximum number of them is
 * NUMMINOR (256). the log and all the filesystems have the same
 * major number.
 */
struct vopen
{
	int  fd;     		/* file descriptor */
	int  status; 		/* status of volume */
	int  fragsize; 		/* fragment size in bytes */
	int  fperpage; 		/* fragments per page */
	int  agsize; 		/* fragments per allocation group */
	int  iagsize; 		/* inodes per allocation group */
	struct vmdmap * imap;   /* pointer to inode map */
	struct vmdmap * dmap;   /* pointer to disk map  */
} vopen[NUMMINOR];

union frag_daddr
{
        frag_t  f;
        ulong   d;
};
typedef union frag_daddr fdaddr_t;      /* trendy underscore t name     */


/* buffer pool for file system disk blocks (but not log).
 * for k > 0 bufhdr[k] describes contents of buffer[k-1].
 * bufhdr[0] is used as anchor for free/lru list. when a
 * buffer is needed, bufhdr[0].prev is the buffer selected.
 * bread of a buffer causes bufhdr[0].next to be set to the
 * buffer's index.
 */
int bhmask = (NBUFPOOL - 1); 	/* hash mask for bhash */
short bhash[NBUFPOOL]; 		/* head of buffer hash chains */
struct bufhdr
{
	short   	next;		/* next on free/lru list */
	short   	prev;		/* previous on free/lru list */
	short		hnext;		/* next on hash chain */
	short   	hprev;		/* previous on hash chain */
	char		modify;		/* buffer was modified */
	char		inuse;		/* buffer on hash chain */
	short		vol;		/* volume minor number */
	fdaddr_t	bn;		/* disk address */
} bufhdr[NBUFPOOL];

struct bufpool
{
	char	bytes[PAGESIZE];
} buffer[NBUFPOOL - 1];

/* things for the log. 
 * log has its own 4 page buffer pool. 
 */
int    logmajor;		/* major number of log device */	
int    logminor = 0;		/* minor number of log device */
int    logserial;		/* log serial number in super block */	
int    logend;			/* address of last  log record */
int    loglastp;		/* last page of log read       */	
int    lognumread; 		/* number of log pages read    */
int    logfd;			/* file descriptor for log */
int    logsize;			/* size of log in pages */
int    nextrep;			/* next log buffer pool slot to replace */
int    logptr[4];		/* log pages currently held in logp */
int    afterdata[PAGESIZE/4];	/* buffer to read in after data */
struct logpage logp[4];		/* log page buffer pool  */ 
struct logsuper sup;		/* log super block */ 

/* maptable. the kth bit of dmaptab[x] is a one if the byte x
 * contains a  sequence of k consecutive zeros, k = 1,2,..8.
 * leftmost bit of char is bit 1.
 */
unsigned char  dmaptab[256];

/* Misc
 */
caddr_t prog;			/* Program name				*/
int mntcnt;
char *mntinfo;

#define	FATAL(str)	fatal(str, __LINE__)
#define numeric(c)	(c >= '0' && c <= '9')
#define DBLPERPAGE	(PAGESIZE/sizeof(ulong))
#define INDIRPERPAGE	(PAGESIZE/sizeof(struct idblock))
#define	NOINDIRECT(nb)	(nb <= NDADDR)
#define	DOUBLEI(nb)	(((unsigned)nb) > DBLPERPAGE)
#define	DIDNDX(pg)	(((ulong) pg)/DBLPERPAGE)
#define	SIDNDX(pg)	(((ulong) pg) & (DBLPERPAGE - 1))

/* Global variables
 */
char *prog;				/* program name			*/
int dflag;				/* Debug on/off			*/
int retcode;				/* return code from logredo    */

char	loglockpath[MAXPATHLEN + 1];	/* log lock file		*/
void locklog(), unlocklog();

int nfsstrange = 0; /* strange length commit records found during logredo   */
int nfsdupcall = 0; /* exact length commit records passed to nfs_dupsave    */
int nfsdupacc  = 0; /* exact length commit records accepted by nfs_dupsave  */
int nfscommit  = 0; /* exact length commit records found during logredo     */
ulong nfsduplist[MAXDUPSAVE][DUPSAVECNT]; /* list of exact commit records   */

extern	char *optarg;
extern	int optind;
extern errno;

char *logpath(), *findpath(), *makedevice();
extern char *strcpy();

main(argc,argv)
int argc;
char **argv;
{
	int inc;
	char *log, logdev[MAXPATHLEN + 1];
	int bufsize;

	prog = argv[0];
	dflag = 0;

	while ((inc = getopt (argc, argv, "n")) != EOF)
		switch (inc) {
		case 'n':
			dflag++;
			break;
		default:
			usage ();
		}

	/*
	 * reposition argc & argv 
	 */
	argc -= optind - 1;
	argv += optind - 1;

	if (argc < 2)
	{	usage ();
		return -1;
	}

	/* loop tell we enough memory to read vmount struct
	 */
	mntinfo = (char *)&bufsize;
	bufsize = sizeof(int);
	while ((mntcnt = mntctl(MCTL_QUERY, bufsize, mntinfo)) <= 0) {

		if (mntcnt < 0) {
			printf ("Memory error\n");
			exit(-1);
		}

		bufsize = *(int *)mntinfo;
		mntinfo = (mntinfo == (char *)&bufsize) ? malloc(bufsize) :
					realloc(mntinfo, bufsize);

		if (mntinfo == NULL) {
			printf ("Memory error\n");
			exit(-1);
		}
	}

	/*
	 * logredo it 
	 */
	if (logredo (*++argv))
	{	fprintf (stderr, "Failure replaying log: %d\n",retcode);
		return -1;
	}

        /* handle hanfs commit records found in the log */

        if (nfsdupcall && nfsisloaded()) { /* found records && nfs is loaded  */
		/* pass commit records to nfs, to be saved in duplicate cache.*/
		/* records are in MRU order in nfsduplist; only nfsdupcall    */
		/* are passed in, to avoid flooding the duplicate cache.      */
		/* nfsdupacc contains number accepted by nfs_dupsave.         */
		nfsdupacc = nfs_dupsave(DUPSAVESIZ*nfsdupcall, nfsduplist);
		if (nfsdupacc == -1)
			perror(prog);
		/* if any nfs commit recs found, print information about them.*/
		if (nfscommit) {
			fprintf(stdout, "%d hanfs commit records found.\n",
				nfscommit);
			fprintf(stdout, "%d hanfs state records provided.\n",
				nfsdupcall);
			fprintf(stdout, "%d hanfs state records accepted.\n",
				nfsdupacc);
		}
		/* if starnge length commit found, print count of them.       */
		if (nfsstrange)
			fprintf(stderr, "%d strange commit records found.\n",
				nfsstrange);
	}

	return 0;
}

/* 
 * logredo processing.
 *
 * algorithm is one-pass over the log reading it  backwards from
 * logend to the value specified by the first log syncpt record 
 * encountered.
 *
 * committed after records which are not the subject of noredo records
 * are applied to the disk blocks. this handles .inodes, indirect 
 * blocks, and directories. for each disk block, a summary bit vector
 * is maintained that represents which lines have been updated by log
 * records encountered in the process. in processing a log record only
 * the lines which correspond to unset bits are updated, so that the
 * only the last update to a line is applied.
 *
 * allocation maps are reconstructed from the last permanent maps
 * written to disk as follows. committed after records to inodes with
 * positive link count imply allocation of the related disk blocks and
 * the inode itself. committed after records for pages in .indirect
 * imply allocation of the disk blocks they point to and themselves.
 * committed dfree records imply free of the blocks they specify. at
 * the beginning of the process the allocation status of every block
 * is in doubt. as log records are processed (reading log backwards
 * means the latest in time records are processed first) the allocation
 * state is determined and the fact that it is determined is recorded
 * in the corresponding bits in the work map, so that earlier in time
 * log records which apply to blocks already encountered are NOT applied
 * (incorrectly).
 *
 * the method for handling crashes in the middle of extending a file
 * system is as follows. the size of a filesystem is established from
 * the s_fsize field in the super-block (i.e the sizes in the diskmap
 * and inodemaps are ignored). in extendfs (xix_cntl.c) the superblock
 * is not updated before the maps have been extended and the new inodes
 * formatted to zeros. no allocations in the new part of the filesystem
 * occur prior to the change in map sizes. if a crash occurs just
 * before updating the superblock, the map sizes will be their old
 * values. in this case the maps as files may be bigger than necessary.
 * if the crash occurs just after writing the super block, the map sizes
 * are fixed up here.
 *
 * the status field s_fmod in the superblock of each file-system is
 * set to FM_CLEAN provided the initial status was either FM_CLEAN
 * or FM_MOUNT and logredo processing was successful. if an error
 * is detected in logredo the status is set to FM_LOGREDO. the status
 * is not changed if its initial value was FM_MDIRTY. fsck should be
 * run to clean-up the probable damage if the status after logredo
 * is either FM_LOGREDO or FM_MDIRTY.
 */

logredo (pathname)
caddr_t pathname;
{
	int  rc,k,logaddr,nextaddr,lastaddr, nlogrecords;
	int  syncrecord, word, bit;
	struct logrdesc ld;
	struct stat sb;
	int logformit;

	fprintf( stdout, "log redo processing for %-32s \n", pathname);

	/* Check to see if device is currently in use as a log 
	 */
	if (rc = islogging (pathname, mntinfo, mntcnt)) 
		return rc;
	free(mntinfo);

	/* open the log
	 */
	logfd = logopen(pathname);
	if (logfd < 0)
	{
		fprintf(stderr, "couldn't open log device\n");
		goto error_out;
	}

	/* read superblock and make sure its ok. get size
	 */
	lseek(logfd, (off_t)ctob(LOGSUPER_B), SEEK_SET); 
	if ( read(logfd, (char *)&sup, (unsigned)sizeof(sup)) < 0)
	{
		fprintf(stderr, "couldn't read log superblock  \n");
		goto error_out;
	}

	if (sup.magic != LOGMAGIC && sup.magic != LOGMAGICV4) {
		fprintf(stderr, "%s: not a log file %s %-32s\n",prog ,pathname);
		return(-1);
	}
	/* Check versioning info */ 
	if (sup.version != LOGVERSION)
	{
		fprintf(stderr, "%s and log file %s version mismatch\n", prog, 
			pathname);
		return(-1);
	}

	if (sup.redone == LOGREDONE)
		return 0;

	logsize = sup.size;
	logserial = sup.serial;

	/* find the end of the log
	 */
	logend = findend();
	if (logend  < 0) {
		logerror(LOGEND,0);
		lseek(logfd, (off_t)ctob(LOGSUPER_B), SEEK_SET); 
		write(logfd, (char *)&sup, (unsigned)sizeof(sup));
		rc = logend;
		goto error_out;
	}

	/* init nodo list, and counts
	 */
	nodolist = NULL;
	numnodo = 0;
	numdoblk = 0;

	/* init block hash chains
	 */
	for ( k = 0 ; k < BHASHSIZE ; k++ )
		blkhash[k] = NULL;

	/* init free list for com. index 0 is not used.
	 */
	comfree = 1;
	for (k = 1; k < COMSIZE; k++)
		com[k].next = k + 1;
 
	/* init buffer pool
	 */
	for (k = 0; k < NBUFPOOL; k++)
	{
		bufhdr[k].next = k + 1;
		bufhdr[k].prev = k - 1;
	}
	bufhdr[0].prev = NBUFPOOL - 1;
	bufhdr[NBUFPOOL - 1]. next = 0;

	/* open all volumes which were in the active list
	 */
	for ( k = 0; k < NUMMINOR ; k++)
	{
		word = k/32;
		bit = k - 32*word;
		if (sup.active[word] & (UZBIT >> bit))
		{
			openvol(k);
		}
	}

	/* read log backwards and process records as we go.
	 * reading stops at place specified by first SYNCPT we
	 * encounter. 
	 */
	nlogrecords = lastaddr = 0;
	nextaddr = logend;

	do
	{
		logaddr = nextaddr;
		nextaddr = logread(logaddr, &ld, afterdata);
		nlogrecords += 1;
		if (nextaddr < 0) {
			logerror(READERR,logaddr);
			if (nextaddr == REFORMAT_ERROR) {
				rc = nextaddr;
				goto loopexit;
				}
			break;
		}
		switch(ld.type) {
		case AFTER:
			rc = doafter(&ld);
			BUGLPR(-1 , rc, ("Bad After record\n"));
			break; 

		case DFREE:
			rc = dofree(&ld);
			BUGLPR(-1, rc, ("Bad Dfree record\n"));
			break; 

		case COMMIT:
			rc = docommit(&ld);
			BUGLPR(-1, rc, ("Bad Commit record\n"));
			break; 

		case NEWPAGE:
			rc = doafter(&ld); /* same as after */
			BUGLPR(-1, rc, ("Bad Newpage record\n"));
			break;

		case NOREDO:
			rc = donoredo(&ld);
			BUGLPR(-1, rc, ("Bad Noredo record\n"));
			break; 

		case NODREDO:
			rc = doafter(&ld); /* same as after */
			BUGLPR(-1, rc, ("Bad Nodredo record\n"));
			break; 

		case MOUNT:
			rc = domount(&ld);
			BUGLPR(-1, rc, ("Bad Mount record\n"));
			break; 

		case DUM:
			rc = 0;
			break; 

		case SYNCPT:
			rc = 0;
			fprintf (stdout, "syncpt record at %x\n",logaddr);
			if (lastaddr == 0)
			{
				syncrecord = logaddr;
				lastaddr = (ld.log.sync == 0)
					? logaddr
					: ld.log.sync;
			}
			break; 
		default:
			logerror(UNKNOWNR, logaddr);
			rc = REFORMAT_ERROR;
			break;
		}
	
		if (rc == REFORMAT_ERROR) {
			BUGPR(("format error. exiting\n"));
			goto loopexit;
			}

		if (rc == MAJOR_ERROR) {
			BUGPR(("fatal error. Exiting\n"));
			goto loopexit;
		}


	} while (logaddr != lastaddr);

	loopexit:

	/*
	 * run logform?
	 */
	logformit = (rc == REFORMAT_ERROR);

	/* flush buffer pool
	 */
	for (k = 0; k < NBUFPOOL - 1; k ++)
	{
		if (rc = bflush(&buffer[k]))
			break;
	}

	/* write out bit maps.
	 * update superblocks of volumes which are open if they were
	 * modified here. i.e. if they were not previously unmounted
	 * cleanly.
	 */
	for(k = 0; k < NUMMINOR; k++)
	{
		if (!vopen[k].fd) 
			continue;

		if (rc = updatemaps(k))
		{
			fprintf(stderr,"update of maps failed \n");
		}

		if (rc = updatesuper(k))
		{
			fprintf(stderr,"update of superblock failed \n");
		}
	}

	/* update superblock of log. clear active list.
	 * If this is a fully replayed log then it can be moved to earlier
	 * versions of the operating system.  Therefore switch the magic
	 * number to the earliest level.
	 */
	if (sup.redone != LOGREADERR)
	{
		for (k = 0; k < NUMMINOR/32; k++)
			sup.active[k] = 0;
		sup.logend = logend;
		sup.redone = LOGREDONE;
		sup.magic = LOGMAGIC;
	}

	lseek(logfd, (off_t)ctob(LOGSUPER_B), SEEK_SET);
	rc = write(logfd, (char *)&sup, (unsigned)sizeof(sup));
	if (rc != sizeof(sup) && errno == EIO)
		rc = REFORMAT_ERROR;
	close(logfd);

	fprintf (stdout, "end of log %x\n",logend);
	fprintf (stdout, "syncpt record at %x\n" , syncrecord);
	fprintf (stdout, "syncpt address %x\n" , lastaddr);
	fprintf (stdout, "number of log records  = %d\n",nlogrecords);
	fprintf (stdout, "number of do blocks = %d\n", numdoblk);
	fprintf (stdout, "number of nodo blocks = %d\n", numnodo);

	sync();

	if (logformit) {
		logform();
		return(0);
		}

	return (rc < 0) ? -1 : 0;

error_out:
		close(logfd);

	if (rc == REFORMAT_ERROR) {
		logform();
		return(0);
		}

		perror(prog);
		return(-1);
}

/*
 * logread(logaddr , ld, dataptr)
 * reads the log record addressed by logaddr and 
 * returns the address of the preceding log record.   
 * returns  -1 if there is an i/o error in reading.
 */

logread(logaddr, ld, dataptr)
int    logaddr;       /* address of log record to read */
struct logrdesc *ld;  /* pointer to a log record descriptor */
char	* dataptr;    /* pointer to buffer.  PAGESIZE long */ 
{
	int buf,off,rc,nwords, pno;

	/* get page containing logaddr into log buffer pool
	 */
	pno = btoct(logaddr);
	if (pno != loglastp)
	{
		loglastp = pno;
		lognumread += 1;
		if (lognumread > logsize - 2)
		{
			BUGPR(("lograp\n"));
			logerror(LOGWRAP,0);
			return(MAJOR_ERROR);
		}
	}
	buf = getpage (pno);
	if (buf < 0) {
		BUGPR(("getpage failed\n"));
		return(buf);
	}

	/* read the descriptor */
	off = logaddr & (PAGESIZE - 1) ;  /* offset just past desc. */
	rc = movewords(8, ld, &buf, &off);
	if (rc < 0) {
		BUGPR(("movewords failed\n"));
		return(rc);
	}

	/* read the data if there is any */
	if (ld->length > 0) {
		nwords = (ld->length + 3)/4;
		rc = movewords(nwords,dataptr,&buf,&off);
		if (rc < 0) {
			BUGPR(("movewords failed\n"));
			return(rc);
		}
	}

	return(ctob(logptr[buf]) + off);
}

/*
 * domount(ld)
 * a log mount record is the first-in-time record which is
 * put in the log so it is the last we want to process in
 * logredo. so we mark volume as cleanly unmounted in vopen
 * array. the mount record is imperative when the volume
 * is a newly made filesystem.
 */ 
int
domount(ld)
struct logrdesc * ld;  	/* pointer to record descriptor */
{
	int vol, status;

	vol = minor(ld->log.mnt.volid);
	status = vopen[vol].status;
	if (status != FM_LOGREDO && status != FM_MDIRTY)
		vopen[vol].status = FM_CLEAN;
	return 0;
}

/*
 * doafter(ld)
 * processing for AFTER, NEWPAGE, and NODREDO record types.
 * if after record is for a page in .inodes or .indirect,
 * the inode and disk maps are also updated.
 */
doafter(ld)
struct logrdesc * ld;  	/* pointer to record descriptor */
{
	 
	int n,k,vol, rc;
	struct nodo *nodoptr;

	/* is it committed ? */
	if ( !findcom(ld->transid))
		return(0);

	/* delete entry from com if no more records for tid
	 */
	if (ld->backchain == 0)
		deletecom(ld->transid);

	/* if cleanly unmounted nothing to do.
	 */
	vol = minor(ld->log.aft.volid);
	if ( vopen[vol].status == FM_CLEAN || vopen[vol].status == FM_LOGREDO)
		return 0;

	/* is it on noredo list. 
	 */
	for (nodoptr = nodolist ; nodoptr != NULL ; nodoptr = nodoptr->next)
	{	if ( ld->log.aft.volid  == nodoptr->volid &&
			ld->log.aft.inode == nodoptr->inode )
			return(0);
	}

	/* apply updates to the page 
	 */
	if (rc = updatep(ld)) {
		BUGPR(("updatep failed rc=%d\n", rc));
		return(rc);
	}

	/* if it isn't an AFTER record return
	 */
	if (ld->type != AFTER)
		return 0;

	/* if it is in .inodes update disk and inode allocation maps
	 */
	if (ld->log.aft.inode == INODES_I) {
		rc = updateinode(vol,ld);
		BUGLPR(-1, rc, ("updateinode failed rc=%d\n", rc));
		return(rc);
	}

	/* if it is in .indirect update disk allocation map.
	 */
	if (ld->log.aft.inode == INDIR_I) {
		rc = updateind(vol,ld);
		BUGLPR(-1, rc, ("updateind falied rc=%d\n", rc));
		return(rc);
	}

	return 0;

}

/*
 * dofree(ld)
 * processing for a dfree log record.
 */

dofree(ld)
struct logrdesc * ld;  	/* pointer to record descriptor */
{
	 
	int n,k,vol, rc, nblocks;

	/* is it committed ? */
	if (!findcom(ld->transid))
		return(0);

	/* delete entry from com if no more records for tid
	 */
	if (ld->backchain == 0)
		deletecom(ld->transid);

	/* if cleanly unmounted or logredo fail nothing to do.
	 */
	vol = minor(ld->log.aft.volid);
	if (vopen[vol].status == FM_CLEAN || vopen[vol].status == FM_LOGREDO)
		return 0;

	/* mark the disk blocks free.
	 */
	nblocks = ld->log.dfree.nblocks;
	for (k = 0; k < nblocks; k ++) {
		if (rc = markit(vopen[vol].dmap,afterdata[k],0,DBTYPE,vol)) {
			BUGLPR(-1, rc, ("markit failed rc=%d\n", rc));
		}
	}

	return 0;

}

/*
 * docommit(ld)
 * insert a commit record in com for transaction
 */
docommit(ld)
struct logrdesc * ld;  /* pointer to record descriptor */
{
	int  k,hash;

	if (comfree == 0)
		return(MAJOR_ERROR);
	k = comfree;
	comfree = com[k].next;
	hash = ld->transid & comhmask;
	com[k].next = comhash[hash];
	com[k].tid = ld->transid;
	comhash[hash] = k;

        /* if this is an hanfs commit record, record it for later */

        if(DUPSAVESIZ == ld->length) { /* correct length */
          nfscommit++; /* found another... */
          if(nfsdupcall < MAXDUPSAVE) /* only save n most recent records. */
            (void)bcopy(afterdata,nfsduplist[nfsdupcall++],DUPSAVESIZ);
	    /* extra data in record copied into nfsduplist array for later. */
          }
        else if(ld->length) /* strange length; probably a bug somewhere */
          nfsstrange++;

	return(0);
}

/* 
 * donoredo()
 * put a record in nodo array. delete tid from com if first
 * record of transaction.
 */
donoredo(ld)
struct logrdesc * ld;  /* pointer to record descriptor */
{
	struct nodo *ndptr;

	/* is it committed ? */
	if ( !findcom(ld->transid))
		return(0);

	/* insert in nodo list
	 */
	ndptr = malloc((size_t)sizeof(struct nodo));
	if (ndptr == NULL) {
		perror("malloc");
		return(MAJOR_ERROR);
		}
	numnodo++;
	ndptr->volid = ld->log.nodo.volid;
	ndptr->inode = ld->log.nodo.inode;
	ndptr->next = nodolist;
	nodolist = ndptr;

	/* delete entry from com if no more records for tid
	 */
	if (ld->backchain == 0)
		deletecom(ld->transid);

	return 0;
}

/*
 * findcom(tid)
 * search for tid in com. return index if found
 * and 0 if not found. 
 */

findcom(tid) 
int tid;  /* transaction id */
{

	int k,hash;
	hash = tid & comhmask;  /* hash class */
	for ( k = comhash[hash]; k != 0; k = com[k].next)
		if (com[k].tid == tid)
			return(k);
	
	return(0);   /* not found */
}

/*
 * deletecom(tid)
 * delete the entry in com for tid.
 */
deletecom(tid)
int tid;  /* transaction id to be deleted */
{

	int k,n,hash;
	hash = tid & comhmask;  	/* hash class */
	n = 0;  			/* previous entry on hash chain  */

	for (k = comhash[hash]; com[k].tid != tid ; k = com[k].next)
			n = k;
	
	/* remove k from hash chain and put it on free list
	 * Special case when 1st on the hash list
	 */
	if ( n == 0)
		comhash[hash] = com[k].next;
	else 
		com[n].next = com[k].next;

	com[k].next = comfree;
	comfree = k;
	return(0);
}

/*
 * updatep(ld)
 * apply changes to disk location specified by ld.  update summary
 * bit vector in doblks for blocks containing the disk location. afterdata
 * contains value of after record for type of AFTER.
 *
 * the changes are applied one block at a time (i.e. if the disk location
 * described by ld spans a disk block boundry, the changes are applied to
 * the applicable fragments of the first block followed by updates to the
 * applicable fragments of the second block).
 */
updatep(ld)
struct logrdesc *ld;
{
	int fperpage, nfrags, vol, nbytes, fbytes, rc;
	int adoff, n, k, first, len, bno, length, fragsize;
	int firstline, lastline, *ptr, *buf;
	offset_t fsoff, blkoff, fragoff, psoff;
	struct doblk *db;
	fdaddr_t disk;
	uint mask, psaddr;

	/* pick up the vol and the fragment size and fragments 
	 * per page values.
	 */
	vol = minor(ld->log.aft.volid);
	fperpage = vopen[vol].fperpage;
	fragsize = vopen[vol].fragsize;

	/* get the disk location (address) and determine the number
	 * of bytes described by this address and the starting byte
	 * offset within the file system of the first fragment.
	 */
	disk.d = ld->log.aft.disk;
	nbytes = (fperpage - disk.f.nfrags) * fragsize;
	fsoff = (offset_t)disk.f.addr * fragsize;

	/* if this is an AFTER record, pickup the starting offset
	 * and length of the changes to be applied to the disk
	 * location.  for NEWPAGE and NODREDO records the changes
	 * apply to the entire disk location.
	 */
	if (ld->type == AFTER) 
	{
		psaddr = ld->log.aft.psaddr & (PAGESIZE - 1);
		length = ld->length;
	}

	/* apply the changes one block at a time.
	 */
	for (fragoff = 0; fragoff < nbytes; fragoff += fbytes, fsoff += fbytes)
	{
		/* determine the starting byte offset of the disk
		 * location within the block and the number of
		 * bytes of the disk location that are contained
		 * within the block.
		 */
		blkoff = fsoff & (PAGESIZE - 1);
		fbytes = MIN(nbytes - fragoff,PAGESIZE - blkoff);

		/* determine which portion of the block will be
		 * effected.
		 */
		if (ld->type == AFTER) 
		{
			/* check if all changes have been applied for 
			 * this record or if the changes do not apply
			 * to this block.
			 */
			if (length == 0 || psaddr >= fragoff + fbytes)
				continue;

			/* get the starting offset within afterdata
			 * for this block.
			 */
			adoff = ld->length - length;

			/* determine the starting offset within the
			 * block the at which the changes should be
			 * applied and the length of the changes.
			 */
			psoff = blkoff + psaddr - fragoff;
			len = MIN(length,PAGESIZE - psoff);

			/* determine the first and last lines effected
			 * within the block.
			 */
			firstline = psoff / LINESIZE;
			lastline = (psoff + len) / LINESIZE - 1;

			/* update length and starting offset to
			 * indicate the changes applied to this
			 * block. 
			 */
			length -= len;
			psaddr += len;
		}
		else 
		{
			/* NEWPAGE or NODREDO.  determine the first
			 * and last lines effected within this block.
			 */
			firstline = blkoff / LINESIZE;
			lastline = (blkoff + fbytes) / LINESIZE - 1;
		}

		/* get the block number and the doblk for this
		 * block.
		 */
		bno = fsoff / PAGESIZE;
		if (rc = insertdo(ld->log.aft.volid,bno,&db))
			return(rc);

		/* check if there is anything to do for this block.
		 */
		if (db->summary == ONES)
			continue;

		/* apply updates to lines not updated previously.
		 */
		buf = NULL;
		for (k = firstline; k <= lastline; k++)
		{
			/* check if the line is already updated.
			 */
			mask = UZBIT >> k;
			if (db->summary & mask)
				continue;

			/* if NODREDO, simply update the summary for
			 * this line and move on to the next line.
			 */
			if (ld->type == NODREDO)
			{
				db->summary |= mask;
				continue;
			}

			/* read the disk block into buffer pool.
	 		 */
			if (buf == NULL)
			{
				if (bread(vol,bno * fperpage,&buf,B_UPDATE))
				{
					BUGPR(("bread failed\n"));
					return(MINOR_ERROR);
				}
			}

			/* update the line in buffer pool page.
			 */
			ptr = buf + k * (LINESIZE/4);
			switch(ld->type)
			{
			case NEWPAGE:
				for (n = 0 ; n < (LINESIZE/4); n++, ptr++)
					*ptr = 0;
				break;

			case AFTER:
				first = (k - firstline) * (LINESIZE/4) +
								(adoff / 4);
				for (n = 0 ; n < (LINESIZE/4); n++, ptr++)
					*ptr = afterdata[n + first];
				break;

			default:
				fprintf (stderr,"wrong kind of record updatep");
				return(MAJOR_ERROR);
			}

			/* update summary bit vector.
			 */
			db->summary |= mask;
		} 
	}

	return 0;
}

/*
 * insertdo(volid, blkno, doptr)
 * finds or inserts an entry in doblk for the disk block
 * associated with volid. returns the index of the entry.
 */
insertdo(volid,blkno,doptr)
int volid;
int blkno;
struct doblk **doptr;
{
	int hash;
	struct doblk *dp;

	/* is it already in doblk ?
	 */
	hash = (volid + blkno) & blkhmask;
	for (dp = blkhash[hash];  dp != NULL; dp = dp->next)
		if (dp->volid == volid && dp->disk == blkno) {
			*doptr = dp;
			return(0);
		}

	/* allocate memory for a new doblk.
	 */
	if ((dp = malloc((size_t)sizeof(struct doblk))) == NULL)
	{
		BUGPR(("malloc failed\n"));
		perror("malloc");
		*doptr = NULL;
		return(MAJOR_ERROR);
	}

	/* insert entry in doblk for it
	 */
	numdoblk++;
	dp->next = blkhash[hash];
	blkhash[hash] = dp;
	dp->volid = volid;
	dp->disk = blkno;
	dp->summary = 0;
	*doptr = dp;
	return(0);
}

/*
 * updateinode(vol,ld)
 * update allocation map for an inode after record. the after
 * record may refer to one or more inode, but if more than 
 * one they are consecutively numbered. each inode is processed
 * as follows. if its link count is zero the inode is freed
 * in the inode map. otherwise, it and the disk blocks it points
 * to directly (exclude indirect block) are allocated. afterdata
 * contains the inode data.
 */

updateinode(vol,ld)
int  vol;	/* index in vopen */
struct logrdesc *ld;  
{
	struct dinode * dp;
	int k, n, numinodes, allocate, rc;
	fdaddr_t disk, ino;

	/* calculate first inode number, number of inodes 
	 * and pointer dp to first disk inode.
	 */
	ino.d = ld->log.aft.psaddr/sizeof(struct dinode);
	numinodes = ld->length/sizeof(struct dinode);
	dp = (struct dinode *) &afterdata[0];

	for (n = 0; n < numinodes ; n ++, ino.d++, dp++)
	{
		/* update inode allocation map
	 	 */
		allocate = (dp->di_nlink != 0);
		if (rc = markit(vopen[vol].imap,ino,allocate,INOTYPE,vol)) {
			BUGPR(("markit failed rc=%d\n"));
		}

		if (!allocate)
			continue;

		/* only handle regular files, directories and
		 * big symbolic links
		 */

		switch(dp->di_mode & IFMT) {
		case IFLNK:
			if (dp->di_size <= D_PRIVATE)
				continue;
		case IFREG: 
		case IFDIR:
			break;

		default:
			continue;
		}

		/* mark disk blocks directly pointed to as allocated
	 	 */
		for (k = 0; k < NDADDR; k++)
		{
			if (disk.d = dp->di_rdaddr[k])
				if (rc = markit(vopen[vol].dmap, disk,
						 1,DBTYPE,vol)) {
					BUGPR(("markit failed rc=%d\n", rc));
				}
		}

	}

	return 0;
}
/*
 * updateind(vol,ld)
 * update allocation map for a .indirect after record.
 * the disk block underlying the page in .indirect is
 * marked as allocated. then all the blocks it (directly)
 * points to are marked as allocated.
 */
updateind(vol,ld)
int  vol;	/* index in vopen */
struct logrdesc *ld;  
{
	int nb, k;
	fdaddr_t disk;
	struct idblock * idp;
	struct vmdmap * mapptr;
	int rc;

	/* get disk block of indirect block. markit allocated.
	 */
	disk.d = ld->log.aft.disk;
	mapptr = vopen[vol].dmap;
	markit(mapptr, disk, 1,DBTYPE,vol);

	/* mark disk blocks directly pointed to as allocated.
	 */
	if (ld->log.aft.indtype == SINGLEIND)
	{
		nb = ld->length/4;
		for (k = 0; k < nb; k++)
		{
			if(disk.d = afterdata[k])
				if (rc = markit(mapptr, disk, 1,DBTYPE,vol)) {
					BUGPR(("markit falied rc=%d\n", rc));
				}
		}
		return 0;
	}

	/* double indirect. 
	 */
	nb = ld->length/sizeof(struct idblock);
	idp = (struct idblock *) &afterdata[0];
	for (k = 0; k < nb; k++, idp ++)
	{
		if (disk.d = idp->id_raddr)
			if (rc = markit(mapptr, disk, 1,DBTYPE,vol)) {
				BUGPR(("markit failed rc=%d\n", rc));
			}
	}

	return 0;

}

/*
 * open the volume specified . check if it is a filesystem
 * and check if it was cleanly unmounted. also check log
 * serial number. initialize disk and inode mpas.
 */
int
openvol(vol)
int	vol;	/* minor number */
{
	int k, fd, rc, fssize;
	struct superblock sb;

	fd = makeopen(vol);
	if (fd < 0)
	{	
		BUGPR(("makeopen failed\n"));
		fserror(OPENERR,vol,0);
		vopen[vol].fd = 0;
		return(MINOR_ERROR);
	}

	vopen[vol].fd = fd;

	/* read in superblock of the volume and
	 * check it out.
	 */
	if (rc = rdwrsuper(fd, &sb, B_READ))
	{
		BUGPR(("rdwrsuper failed\n"));
		fserror(READERR,vol, SUPER_B);
		close(fd);
		vopen[vol].fd = 0;
		return(MINOR_ERROR);
	}

	/* check magic number and initialize version specific
	 * values in the vopen struct for this vol.
	 */
	if (strncmp(sb.s_magic,fsv3pmagic,4) == 0)
	{
		if (sb.s_version != fsv3pvers)
		{
			fprintf(stderr, "magic number not right %d\n",vol);
			close(fd);
			vopen[vol].fd = 0;
			return(MINOR_ERROR);
		}
		vopen[vol].fragsize = sb.s_fragsize;
		vopen[vol].agsize = sb.s_agsize;
		vopen[vol].iagsize = sb.s_iagsize;
	}
	else if (strncmp(sb.s_magic,fsv3magic,4) == 0)
	{
		vopen[vol].fragsize = PAGESIZE;
		vopen[vol].agsize = vopen[vol].iagsize = sb.s_agsize;
	}
	else
	{
		fprintf(stderr, "magic number not right %d\n",vol);
		close(fd);
		vopen[vol].fd = 0;
		return(MINOR_ERROR);
	}

	/* set fperpage in vopen.
	 */
	vopen[vol].fperpage = PAGESIZE / vopen[vol].fragsize;

	/* was it cleanly umounted ?
	 */
	if (sb.s_fmod == FM_CLEAN)
	{
		vopen[vol].status  = FM_CLEAN;
		close(fd);
		vopen[vol].fd = 0;
		return(0);
	}

	/* else get status of volume 
	 */
	vopen[vol].status = sb.s_fmod;

	/* check log serial number
	 */
	if (sb.s_logserial != logserial)
	{
		BUGPR(("bad serial number\n"));
		close(fd);
		vopen[vol].fd = 0;
		fserror(SERIALNO, vol, SUPER_B);
		return(MINOR_ERROR);
	}

	/* initialize the disk and inode maps.
	 */
	fssize = sb.s_fsize/(vopen[vol].fragsize/512);
	if (rc = initmaps(vol,fssize))
	{
		BUGPR(("initmaps failed rc=%d\n", rc));
		fserror(MAPERR,vol,0);
	}
	return rc;
}

/*
 * read or write the superblock for the file system described
 * by vol.
 */

int
rdwrsuper(fd, sb, rwflag)
int fd;
struct superblock *sb;
int rwflag;
{
	union {
		struct superblock super;
		char block[PAGESIZE];
		} super;

	/* seek to the postion of the superblock.
	 */
	lseek(fd, (off_t)ctob(SUPER_B), SEEK_SET);

	/* if the request is read, read the block containing the superblock
	 * from disk and set the returned superblock structure.  otherwise,
	 * clear the local block buffer, copy the passed superblock, and
	 * write the buffer to disk.
	 */
	if (rwflag == B_READ)
	{
		if (read(fd, super.block, (unsigned)PAGESIZE) != PAGESIZE)
		{
			BUGPR(("read failed errno=%d\n", errno));
			return(MINOR_ERROR);
		}
		*sb = super.super;
	}
	else
	{
		memset(super.block, 0, PAGESIZE);
		super.super = *sb;
		if (write(fd, super.block, (unsigned)PAGESIZE) != PAGESIZE)
		{
			BUGPR(("write failed errno=%d\n", errno));
			return(MINOR_ERROR);
		}
	}
	return 0;
}


/* 
 * initmaps(vol, fssize)
 * allocate storage for inode and disk maps and read them
 * into memory. initialize workmaps to zeros. initialize
 * maps according to fssize parameter.
 */

initmaps(vol,fssize)
int vol;  /* index in vopen array = minor(volid) */
int fssize;  /* fs size in fragment size blocks */
{
	int rc;
	struct dinode * dp;

	/* read first page of disk inodes into buffer pool
	 * this page has the disk inodes for both maps.
	 */
	if (rc = bread(vol, INODES_B * vopen[vol].fperpage, &dp , B_READ))
	{
		BUGPR(("bread failed errno=%d\n", errno));
		return(MINOR_ERROR);
	}

	/* read inode map into memory and init workmap to zeros.
	 */
	if (rc = readmap(vol,&vopen[vol].imap,dp + INOMAP_I, fssize, INODEMAP))
	{
		BUGPR(("readmap failed rc=%d\n", rc));
		return rc;
	}

	/* read disk map into memory and init workmap to zeros.
	 */
	rc = readmap(vol,&vopen[vol].dmap,dp + DISKMAP_I, fssize, FSDISKMAP);
	BUGLPR(-1, rc, ("readmap failed rc=%d\n", rc));
	return(rc);
}

/*
 * readmap(vol,mapptr,dp, fssize)
 * allocate storage for a map, read it into memory, and initialize
 * workmap to zeros. recompute on the basis of the file system size
 * and allocation group sizes in the filesystem superblock the number
 * of allocation groups and the size of map. (ie the size info in the
 * map is not trusted). this is done to recover from a crash in the middle
 * of extending a filesystem. depending on where the crash occurred, the
 * extendfs will be completed here, or backed-out. in backing out, the map
 * considered as files may be bigger than neceassry, but logically
 * this causes no problem: extendfs can be re-run, and the extra data
 * not accessed before then.
 */

readmap(vol,mapptr,dp,fssize,type)
int	vol;		  /* index in vopen array */
struct vmdmap ** mapptr;  /* set to where map is allocated */
struct dinode * dp;       /* disk inode of map */
int 	fssize;	  	  /* file system size in fragment size chuncks */
int 	type;  	  	  /* type of allocation map */
{
	int k, npages, fd, w;
	int nag, agsize, iagsize, p, rem, mapsize, agperpage;
	int bit, firstbit, lastbit, minfrags;
	struct vmdmap *ptr, *ptr0;
	fdaddr_t disk;
	uint dbperpage, version, wperpage, *wmap, *pmap, np, totalpages;

	
	/* get allocation group sizes.
	 */
	agsize = vopen[vol].agsize;
	iagsize = vopen[vol].iagsize;

	/* determine the number of allocation groups in the map.
	 * minfrag is the minimum nuber of fragment required to
	 * cover an allocation group's disk inodes.
	 */
	nag = fssize/agsize;
	rem = fssize - nag * agsize;
	minfrags = iagsize / (vopen[vol].fragsize / sizeof(struct dinode));
        nag = (rem >= minfrags) ? nag + 1: nag;

	/* allocate storage for map. the size is always a multiple
	 * of PAGESIZE so we don't have to round it up. 
	 */
	if((ptr0 = (struct vmdmap *) malloc((size_t)dp->di_size)) == NULL)
	{
		fprintf(stderr,"malloc for map failed %d\n",vol);
		return(MINOR_ERROR);
	}

	/* read first page of map into memory.
	 */
	fd = vopen[vol].fd;
	bmap(vol, dp , 0 , &disk);
	llseek(fd, (offset_t)disk.f.addr * vopen[vol].fragsize, SEEK_SET);
	if (read(fd, (char *)ptr0, (unsigned)PAGESIZE) != PAGESIZE)
	{
		fprintf(stderr,"read of map failed %d\n",vol);
		return(MINOR_ERROR);
	}

	/* set map version. and dbperpage. */
	version = ptr0->version;
	dbperpage = WBITSPERPAGE(version);

	/* get the number of bits covered by the map and the number
	 * of allocation groups per page.
	 */
	if (type == INODEMAP)
	{
		mapsize = iagsize * nag;
		agperpage = dbperpage / iagsize;
	}
	else
	{
		mapsize = fssize;
		agperpage = dbperpage / agsize;
	}

	/* consistency check on file-system size and size of map
	 * as a file. 
	 */
	totalpages = npages = (mapsize + dbperpage - 1)/dbperpage;
	if (version == ALLOCMAPV4)
	{
		totalpages += (npages + 7)/8;  /* add in control pages */
	}
	if (totalpages  > btoc(dp->di_size))
	{
		fprintf(stderr, "mapsize inconsistent \n");
		return(MINOR_ERROR);
	}

	/* read the map into memory and init work map to zeros.
	 */
	ptr = ptr0;
	for (k = 0; k < totalpages; k++, ptr += 1)
	{
		bmap(vol, dp , k , &disk);
		llseek(fd, (offset_t)disk.f.addr * vopen[vol].fragsize, SEEK_SET);
		if (read(fd, (char *)ptr, (unsigned)PAGESIZE) != PAGESIZE)
		{
			fprintf(stderr,"read of map failed %d\n",vol);
			return(MINOR_ERROR);
		}
		if (version == ALLOCMAPV3)
		{
			wmap = (uint *)(ptr) + LMAPCTL/4;
			wperpage = WPERPAGE;
		}
		else 
		{
			if ( k % 9 == 0) continue;
			wmap = (uint *) (ptr);
			wperpage = WPERPAGEV4;
		}
		/* init work bit map to zeros.
	 	 */
		for (w = 0; w < wperpage; w++)
			wmap[w] = 0;

	}

	/* set mapsize and totalags.
	 */
	ptr = ptr0;
	ptr->mapsize = mapsize;
	ptr->totalags = nag;

	/* initialize the number of allocation groups in each 
	 * page.
	 */
	for (k = 0; k < npages; k++)
	{
		if (version == ALLOCMAPV3)
		{
			ptr = ptr0 + k;
		}
		else
		{
			ptr = ptr0 + 9*(k >> 3);
			ptr = (struct vmdmap *)((char *)ptr + ((k & 0x7)<<9));
		}
		ptr->agcnt = MIN(agperpage, nag);
		nag -= agperpage;
	}

	/* initialize the bits in the permanent map for all bits
	 * beyond mapsize to allocated. in the case of a disk map
	 * with a remainder too small to contain the inodes required
	 * for an allocation group, the remainder is also marked as
	 * as allocated. 
	 */
	firstbit = mapsize;
	if (type == FSDISKMAP && rem < minfrags)
		firstbit -= rem;
	lastbit = npages * dbperpage - 1;

	for (k = firstbit; k <= lastbit; k++)
	{
		p = k / dbperpage;
		rem = k - p * dbperpage;
		w = rem >> L2DBWORD;
		bit = (rem - (w << L2DBWORD));
		if (version == ALLOCMAPV3)
		{
			ptr = ptr0 + p;
			pmap = (uint *)(ptr) + LMAPCTL/4 + WPERPAGE;
		}
		else
		{
			ptr = ptr0 + 9*(p >> 3) + 1 + (p & 0x7);
			pmap = (uint *)(ptr) + WPERPAGEV4;
		}
		pmap[w] |= (UZBIT >> bit);
	}

	*mapptr = ptr0;

	return 0;
}

/*
 * updatesuper(vol)
 * updates superblock state and writes it out.
 */
int 
updatesuper(vol)
int vol;   /* index in vopen array */
{
	int rc, status;
	struct superblock sb;

	/* read in superblock of the volume 
	 */
	if (rc = rdwrsuper(vopen[vol].fd, &sb, B_READ)) {
		BUGPR(("rdwrsuper failed rv=%d\n", errno));
		return(MINOR_ERROR);
	}

	/* mark superblock state. write it out.
	 */
	status = vopen[vol].status;
	if (status != FM_LOGREDO && status != FM_MDIRTY)
		sb.s_fmod = FM_CLEAN;
	else
		sb.s_fmod = status;

	if (rc = rdwrsuper(vopen[vol].fd, &sb, B_UPDATE))
	{
		BUGLPR(-1, rc, ("rdwrsuper failed\n"));
		fprintf (stderr, "error writing %d,%d\n", vol, SUPER_B);
	}
	return(rc);
}

/* 
 * updatemaps(vol)
 * copy permanent map to work map.
 */
updatemaps(vol)
int vol;	/* index in vopen array */
{
	int rc;
	struct	dinode *dp;		

	/* read first page of disk inodes into buffer pool
	 * this page has the disk inodes for both maps.
	 */
	if (rc = bread(vol, INODES_B * vopen[vol].fperpage, &dp , B_READ)) {
		BUGPR(("bread failed errno=%d\n", errno));
		return(MINOR_ERROR);
	}

	/* process inode map.  writemap() also will calculate
	 * various free counts.
	 */
	if (rc = writemap(vol,vopen[vol].imap,dp + INOMAP_I)) {
		BUGPR(("writemap failed rc=%d\n", rc));
		return rc;
	}

	/* process disk map.  writemap() also will calculate
	 * various free counts.
	 */
	rc = writemap(vol,vopen[vol].dmap,dp + DISKMAP_I);
	BUGLPR(-1, rc, ("writemap failed rc=%d\n", rc));
	return(rc);

}

/*
 * writemap(vol,p0,dp)
 * copy permanent map to work map. calculate free block data 
 * and initalize tree. then write the map to disk.
 */
writemap(vol,p0,dp)
int    vol;		/* index in vopen array */
struct vmdmap * p0;	/* pointer to first page of map */
struct dinode * dp;	/* disk inode of map */
{
	struct vmdmap *p1;
	int k, npages, fd;
	fdaddr_t disk;
	uint version, dbperpage, totalpages;

	if (vopen[vol].status == FM_LOGREDO)
		return(MINOR_ERROR);

	/* consistency check on file-system size and size of map
	 * as a file. 
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	totalpages = npages = (p0->mapsize + dbperpage - 1)/dbperpage;
	if (version == ALLOCMAPV4)
	{
		totalpages += (npages + 7)/8;
	}

	if (totalpages  > btoc(dp->di_size))
	{
		fprintf(stderr, "mapsize inconsistent \n");
		return(MINOR_ERROR);
	}

	/* process each page of the map.
	 */
	p0->freecnt = 0;
	for (k = 0; k < npages; k++)
	{
		updatepage(p0,k);
	}

	/* write out bit map to disk
	 */
	p1 = p0;
	fd = vopen[vol].fd;
	for (k = 0; k < totalpages; k++, p1 += 1)
	{
		bmap(vol, dp , k , &disk);
		llseek(fd, (offset_t)disk.f.addr * vopen[vol].fragsize , SEEK_SET);
		if (write(fd, (char *)p1, (unsigned)PAGESIZE) < 0)
		{
			BUGPR(("write failed errno=%d\n", errno));
			fprintf(stderr,"write of map failed %d\n",vol);
			return(MINOR_ERROR);
		}
	}

	return 0;
}

/*
 * updatepage(p0,p)
 * 
 * copies the perm-map to the work map. initializes the free
 * counts in allocation groups on the page and initializes
 * the tree to reflect allocation state of map. updates 
 * total free count in page 0.
 */

int
updatepage(p0,p)
struct vmdmap *p0;
int p;   /* page in bit-map excluding control pages */
{
	struct vmdmap * p1;
	uint ag, fw, w, k, agfree, word, mask, wperag, agperpage;
	uint version, dbperpage, *wmap, *pmap, wperpage;

	/* copy the perm map to the work map.
	 */
	version = p0->version;
	if (version == ALLOCMAPV3)
	{
		p1 = p0+p;
		wmap = (uint *)p1 + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
		wperpage = WPERPAGE;
		dbperpage = DBPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *) (p1 + 1 + (p & 0x7));
		p1 = VMDMAPPTR(p1, p);
		pmap = wmap + WPERPAGEV4;
		wperpage = WPERPAGEV4;
		dbperpage = DBPERPAGEV4;
	}
	for (w = 0; w < wperpage; w++)
		wmap[w] = pmap[w];

	/* calculate free block data for each ag of the page.
	 */
	wperag = p0->agsize >> L2DBWORD;
	agperpage = dbperpage/p0->agsize;
	fw = 0;
	for (ag = 0; ag < agperpage; ag++ , fw += wperag)
	{
		
		/* look at each map word of the ag.
		 */
		agfree = 0;
		for(w = 0; w < wperag; w++)
		{
			/* check each bit of the word.
			 */
			word = wmap[w+fw];
			for( mask = UZBIT; mask ; mask = mask >> 1)
			{
				/* check if the bit is free.
				 */
				if ((mask & word) == 0)
					agfree += 1;
			}	
		}
		p1->agfree[ag] = agfree;
		p0->freecnt += agfree;
	}

	/* initialize the tree for the page.  updtree is called
	 * for each pair of words in the map because each byte in
	 * leaf-words cover 2 words. initialize the dmaptab() if
	 * not already inited.
	 */
	if (dmaptab[0] == 0)
		idmaptab();

	/* clear the words of the tree.
	 */
	for (k = 0; k < TREESIZE; k++)
		p1->tree[k] = 0;

	/* recalculate the tree.
	 */
	for (w = 0; w < wperpage; w += 2)
		updtree(p0,p,w);
}

/* 
 * initializes dmaptab.  the kth bit of dmaptab[x] is a one if the
 * byte x contains a  sequence of k consecutive zeros, k = 1,2,..8.
 * leftmost bit of char is bit 1.
 */
idmaptab()
{
	unsigned int mask, tmask;
	int k,n,j,shift;

	dmaptab[255] = 0;
	dmaptab[0] = ONES;

	/* other than  0 and 255, all have sequences of zeros
	 * between 1 and 7.
	 */
	for (k = 1; k < 255; k++)
	{
		for (n = 7; n > 1; n--)
		{
			shift = 8 - n;
			mask = (ONES << shift) & 0xff;
			tmask = mask;
			for (j = 0; j <= shift; j++) 
			{
				if ((tmask & k) == 0)
					goto next;
				tmask = tmask >> 1;
			}
		}
		next : dmaptab[k] = (j <= shift) ? mask : 0x80;
	}

}

/*
 * update vmdmap tree.
 *
 * input parameters:
 *		p0  - pointer to page zero of map
 *		p - page number in bitmap.
 *		ind - index within page of word of a map that has changed.
 */

updtree(p0,p,ind)
struct vmdmap *p0;
int p;
int ind;
{
	struct vmdmap * p1;
	uint n, lp, k, index, maxc, max; 
	uchar *cp0, *cp1;

	/* get the index of the first word of the
	 * double word.
	 */
	ind = ind & (ONES << 1);

	/* calculate max for the two words of the section.  for
	 * v3 allocation maps, the max is determined as the max
	 * sequence of free bits found within a character of the
	 * double word.  for v3p maps, max is the max sequence of
	 * free bits found within the double word.
	 */
	if (p0->version == ALLOCMAPV3)
	{
		p1 = p0 + p;
		max = 0;
		maxc = p1->clmask;
		cp0 = (uchar *)(p1) + LMAPCTL + 4*ind; 

		/* examine each character of the double word. the
		 * max for each character is determine via table
		 * lookup.
		 */
		for(n = 0; n < 8; n++, cp0++)
		{
			max = MAX(max,dmaptab[*cp0]);
			if (max >= maxc)
			{
				max = maxc;
				break;
			}
		}
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		cp0 = (uchar *)(p1 + 1 + (p & 0x7)) + ind*4;
		p1 = VMDMAPPTR(p1, p);
		max = maxstring((int *)cp0);
	}

	/* calculate pointers to leaf word and to the character
	 * in it that corresponds to ind; leaf words cover 8
	 * sections (32 bytes of map) so the low order 3-bits 
	 * of ind are shifted right by one.
	 */
	lp = LEAFIND + (ind >> 3);
	cp0 = (uchar *) &p1->tree[lp];
	cp1 = cp0 + ((ind & 0x7) >> 1);

	/* there are at most four levels of the tree to process.
	 */
	for (k = 0; k < 4; k++)
	{
		/* if old max is same as max, nothing to do.
	 	 */
		if (*cp1 == max)
			return 0;

	 	/* set the value of the new maximum.
		 * get the maximum in the word after change
	 	 */
		*cp1 = max;
		max = treemax(cp0);

		/* get parent of lp.
		 * parent covers four words so division is by 4.
		 * calculate pointers to word and character.
		 */
		index = (lp - 1) & 0x3;
		lp = (lp - 1) >> 2;
		cp0 = (uchar *) &p1->tree[lp];
		cp1 = cp0 + index;
	}

	return 0;
}

/*
 * treemax(cp0)
 *
 * returns the maximum sequence in a tree word.
 * cp0 points to first character of the word.
 */
int
treemax(cp0)
uchar * cp0;
{
	uint max1, max2;
	max1 = MAX(*cp0, *(cp0+1));
	max2 = MAX(*(cp0+2), *(cp0+3));
	return (MAX(max1,max2));
}

/*
 * return longest string of zeros in 64-bit double word
 * addressed by ptr. used in find the maximum number of
 * contiguous free bits within a double word of an allocation
 * map.
 */
int
maxstring(ptr)
unsigned int *ptr;
{
	unsigned int max, m, mask, word;

	/* calculate longest string in first word.
	 */
	m = max = 0;
	mask = UZBIT;
	word = *ptr;
	while(mask)
	{
		if (word & mask)
		{
			max = MAX(max, m);
			m = 0;
		}
		else m = m + 1;
		mask = mask >> 1;
	}

	/* m is number of trailing zeros of first word.
	 * calculate longest string in second word,
	 * including trailing zeros of first word.
	 */
	mask = UZBIT;
	word = *(ptr + 1);
	while(mask)
	{
		if (word & mask)
		{
			max = MAX(max, m);
			m = 0;
		}
		else m = m + 1;
		mask = mask >> 1;
	}

	return (MAX(max,m));
}

/* 
 * markit(p0, bn, val, type, vol)
 *
 * sets the bits specified by bn in the permanent map to val
 * unless the bits were previously marked by this procedure. 
 * the work map is used to record previous calls to markit
 * for the same bits.
 */

markit(p0, bn, val, type, vol)
struct vmdmap * p0;		/* pointer to page zero of map  */
fdaddr_t bn;			/* disk address			*/
int	val;			/* 1 to allocate 0 to free      */
int 	type;
int	vol;
{
	int p, s, w, rem, n, m, bit, nbits, wordbits;
	struct vmdmap *p1;
	uint version, dbperpage, *wmap, *pmap;;

	nbits = (type == INOTYPE) ? 1 : vopen[vol].fperpage - bn.f.nfrags;
	if (bn.f.addr + nbits > p0->mapsize)
	{
		BUGPR(("block out of range\n"));
		fserror(type, vol, bn);
		return MINOR_ERROR;
	}

	/* calculate page, word, and bit number corresponding
	 * to starting bit of bn. 
	 */
	version = p0->version;
	dbperpage = WBITSPERPAGE(version);
	p = bn.f.addr / dbperpage;
	rem  = bn.f.addr - p * dbperpage;
	w =  rem >> L2DBWORD;
	bit = (rem - (w << L2DBWORD));

	if (version == ALLOCMAPV3)
	{
		p1 = p0+p;
		wmap = (uint *)p1 + LMAPCTL/4;
		pmap = wmap + WPERPAGE;
	}
	else
	{
		p1 = p0 + 9*(p >> 3);
		wmap = (uint *) (p1 + 1 + (p & 0x7));
		pmap = wmap + WPERPAGEV4;
	}

	/* the bits described by bn may span a word boundry, so we
	 * may have to set bits in two word of the map (the JFS
	 * allocation policies guarantee that the bits of a single
	 * disk address will be contained within a double).  first,
	 * we determine the number of bits that we must set in the
	 * word containing the starting bit.
	 */
	 wordbits = min(32 - bit,nbits);

	/* now we process the first word.
	 */
	for (n = 0; n < wordbits; n++, bit++)
	{
		/* if state is already determined continue;
		 */
		if (wmap[w] & (UZBIT >> bit))
			continue;

		/* update pmap according to val. set workmap to indicate
		 * state is determined.
		 */
		wmap[w] |= (UZBIT >> bit);
		if (val)
			pmap[w] |= (UZBIT >> bit);
		else
			pmap[w] &= ~(UZBIT >> bit);
	}

	/* now, we determine the number of bits to set in the
	 * second word.
	 */
	wordbits = nbits - wordbits;
	w = w + 1;
	bit = 0;

	/* process the second word.
	 */
	for (n = 0; n < wordbits; n++, bit++)
	{
		/* if state is already determined continue;
		 */
		if (wmap[w] & (UZBIT >> bit))
			continue;

		/* update pmap according to val. set workmap to indicate
		 * state is determined.
		 */
		wmap[w] |= (UZBIT >> bit);
		if (val)
			pmap[w] |= (UZBIT >> bit);
		else
			pmap[w] &= ~(UZBIT >> bit);
	}

	return 0;
}

/*
 * bmap( vol, dp, bn, bnp)
 * Block map.  Given block # bn in a file , find its disk address
 * and return it in bnp.
 */
bmap(vol,dp, bn, bnp)
int    vol;			/* index in vopen              */
struct dinode *dp;		/* Disk inode of file           */ 
ulong  bn;			/* page number in file		*/
fdaddr_t *bnp;			/* Returned disk address	*/
{

	ulong *block;			/* pointer to indirect		*/
	ulong nb;			/* number blocks 		*/
	int rc;				/* Return value bread		*/

	nb = btoc(dp->di_size);
	
	if (bn >= nb)
		FATAL("block number out of range");

	/* No indirect blocks ?
	 */
	if (NOINDIRECT(nb))
	{
		bnp->d = dp->di_rdaddr[bn];
		return 0;
	}

	/* Single or double indirect
	 */
	if (rc = bread(vol, dp->di_rindirect, &block, B_READ)) {
		BUGPR(("bread failed rc=%d\n", rc));
		return rc;
	}

	/* if double indirect read in the direct block
	 */
	if (DOUBLEI(nb))
	{
		struct idblock *idp;
		int didndx;

		didndx = DIDNDX(bn);
		idp = (struct idblock *)block + didndx;
		if(rc = bread(vol,idp->id_raddr, &block, B_READ)) {
			BUGPR(("bread failed\n"));
			return rc;
		}
	}

	bnp->d = block[SIDNDX(bn)];

	return 0;

}
	
/*
 * bread (vol , bn, buf, update)
 * return with buf set to pointer of page in buffer pool 
 * containing disk fragments specified. the parameter update 
 * specifies the caller's intentions.
 */

int
bread(vol, bn, buf, update)
int      vol;		/* index in vopen (minor of volid)    	*/ 
fdaddr_t bn;		/* disk fragments		 	*/ 
void     **buf;		/* set to point to buffer pool page    	*/
int      update;	/* true if buffer will be modified     	*/
{
	int k, hash, oldhash, nxt, prv, found, head, fd, rc;
	int nfrags, nbytes;
	struct vmdmap * p0;

	/* test that bn is ok
	 */
	p0 = vopen[vol].dmap;
	nfrags = vopen[vol].fperpage - bn.f.nfrags;
	if (p0 != NULL && bn.f.addr + nfrags > p0->mapsize)
	{
		BUGPR(("bad block\n"));
		fserror(DBTYPE,vol,bn);
		return(MINOR_ERROR);
	}


	/* hash it and search buffer pool for it.
	 */
	hash = (bn.d ^ vol) & bhmask;
	for (k = bhash[hash]; k != 0; k = bufhdr[k].hnext)
	{
		if (bufhdr[k].bn.d == bn.d && bufhdr[k].vol == vol)
			break;
	}

	/* was it in buffer pool ?
	 */
	found =  (k != 0);
	k = ( found) ? k : bufhdr[0].prev;

	/* remove k from current position in lru list
	 */
	nxt = bufhdr[k].next;
	prv = bufhdr[k].prev;
	bufhdr[nxt].prev = prv;
	bufhdr[prv].next = nxt;

	/* move k to head of lru list
	 */
	head = bufhdr[0].next;
	bufhdr[k].next = head;
	bufhdr[head].prev = k;
	bufhdr[k].prev = 0;
	bufhdr[0].next = k;

	/* bufhdr[k] describes buffer[k-1]
	 */
	*buf = &buffer[k-1];

	/* if found set modify bit and return.
	 */
	if (found)
	{
		bufhdr[k].modify |= update;
		return 0;
	}

	/* wasn't found. write it out if it was modified.
	 */
	if (bufhdr[k].inuse & bufhdr[k].modify)
	{
		if (rc = bflush(&buffer[k - 1]))
			return (rc);
	}		

	/* remove it from hash chain if necessary.
	 * hprev is 0 if it is at head of hash chain.
	 */
	if (bufhdr[k].inuse)
	{
		nxt = bufhdr[k].hnext;
		prv = bufhdr[k].hprev;
		if (prv == 0)
		{
			oldhash = (bufhdr[k].vol ^ bufhdr[k].bn.d) & bhmask;
			bhash[oldhash] = nxt;
		}		
		else
		{
			bufhdr[prv].hnext = nxt;
		}

		/* next assign is ok even if nxt is 0
		 */
		bufhdr[nxt].hprev  = prv;

	}

	/* insert k at head of new hash chain
	 */
	head = bhash[hash];
	bufhdr[k].hnext = head;
	bufhdr[k].hprev = 0;
	bufhdr[head].hprev = k;   /* ok even if head = 0 */
	bhash[hash] = k;

	/* fill in bufhdr with new data and read the page in
	 */
	bufhdr[k].vol = vol;
	bufhdr[k].bn.d  = bn.d;
	bufhdr[k].inuse = 1;
	bufhdr[k].modify = update;
	fd = vopen[vol].fd;
	llseek(fd, (offset_t)bn.f.addr * vopen[vol].fragsize, SEEK_SET);
	nbytes = nfrags * vopen[vol].fragsize;
	if (read (fd , (char *)&buffer[k-1], (unsigned)nbytes) != nbytes)
	{
		BUGPR(("read failed errno=%d\n", errno));
		fserror(IOERROR, vol, bn);
		return(MINOR_ERROR);
	}
	return 0;
}

/*
 * bflush(buf)
 * write out appropriate portion of buffer page if its modified.
 */
bflush(buf)
struct buffer * buf;  /* pointer to buffer pool page */
{

	int k, fd, vol, nbytes;
	fdaddr_t bn;

	/* convert buf to index in bufhdr.
	 */
	k = ((uint)buf - (uint)(&buffer[0]))/PAGESIZE + 1;

	/* nothing to do ?
	 */
	if(bufhdr[k].modify == 0)
		return 0;

	/* write it out
	 */
	vol = bufhdr[k].vol; 
	bn.d  = bufhdr[k].bn.d;
	fd = vopen[vol].fd;
	llseek(fd, (offset_t)bn.f.addr * vopen[vol].fragsize, SEEK_SET);
	nbytes = (vopen[vol].fperpage - bn.f.nfrags) * vopen[vol].fragsize;
	if (write(fd, (char *)buf, (unsigned)nbytes) != nbytes)
	{
		BUGPR(("write failed errno=%d\n", errno));
		fprintf (stderr, "error writing %d,%d\n", vol, bn);
		return(MINOR_ERROR);
	}
	return 0;
}

/*
 * getpage(pno)
 * gets the page pno into log buffer pool and returns its
 * index. returns -1 if i/o error.
 */

getpage(pno)
int pno;  /* page of log */                
{

	int k;

	/* is it in buffer pool ?
	 */
	for (k = 0; k <= 3; k++)
		if (logptr[k] == pno) return(k);

	/* read page into buffer pool into next slot
	 * don't have to use llseek() here.  log dev will never be > 2 gig
	 */
	nextrep = (nextrep + 1) % 4;
	lseek(logfd, (off_t)ctob(pno), SEEK_SET);

	if (read(logfd, (char *)&logp[nextrep], (unsigned)PAGESIZE) < 0)
	{
		BUGPR(("read failed errno =%d\n", errno));
		logerror(IOERROR, pno);
		return(errno == EIO ? REFORMAT_ERROR : MAJOR_ERROR);
	}
	logptr[nextrep] = pno;
	return(nextrep);
}

/*
 * movewords (nwords, target, buf,offset)
 * moves nwords from buffer pool to target. data 
 * is moved in backwards direction starting at offset.
 * if necessary, the previous page is read into the
 * buffer pool and on exit buf will point to this 
 * page in the buffer pool and offset to where the
 * move stopped. the previous page is fetched whenever
 * the current page is exhausted (all bytes were read)
 * even if all the words required to satisfy this move
 * are on the current page. returns 0 ok, -1 i/o error.
 */

movewords(nwords, target, buf, offset)
int  nwords;  /* number of 4-byte words to move */
int *target;  /* address of target (begin address) */
int *buf;     /* index in buffer pool of current page */
int *offset;  /* initial offset in buffer pool page */                   
{

	int n,j,words,pno;
	int * ptr;

	j = (*offset - 8)/4 - 1; /* index in log page data area   
	  			     of first word to move      */  
	words  = min(nwords,j + 1);  /* words on this page to move */
	ptr = target + nwords - 1; /* last word of target */
	for (n = 0; n < words; n++) 
	{
		*ptr = logp[*buf].ldata[j];
		j = j - 1;
		ptr = ptr - 1;
	}	
	*offset = *offset - 4*words;

	/* did we get less than nwords or exhaust the page ? 
	 */
	if ( words != nwords || j < 0) {
		/* get previous page */
		pno = logptr[*buf];
		pno = pno - 1;
		if (pno == 1) pno = logsize - 1;  
		*buf  = getpage(pno);
		if (*buf < 0) {
			BUGPR(("getpage failed\n"));
			return(*buf);
		}
		*offset = PAGESIZE - 8;
		j = PAGESIZE/4 - 4 - 1; /* index last word of data area */
		/* move rest of nwords if any. this will never 
		exhaust the page.                          */ 
		for (n = 0; n < nwords - words ; n++) {
			*ptr = logp[*buf].ldata[j];
			j = j - 1;
			ptr = ptr - 1;
		}
		*offset = *offset - 4*(nwords - words);
	}
	return(0);
}

/* 
 * NAME:	findend()
 * 
 * FUNCTION:	Returns the address of the last record in the log.
 * 		(i.e. the address of the byte following its descriptor).
 * 		Log errors are acceptable on the last page written to the 
 *		log since the last log page is written in a ping pong manner.
 *
 * 		The end of the log is found by finding the page with the 
 *		highest page number and for which h.eor == t.eor,  
 *		h.eor > 8 (i.e. a record ends on the page), and the redundancy
 *		check value in the page is correct.  Page numbers are compared 
 *		by comparing their difference with zero (necessary because 
 *		page numbers are allowed to wrap.)
 *
 * RETURNS:	>0		- byte offset of last record
 *		REFORMAT_ERROR	- reformat the log
 *		MAJOR_ERROR	- other major errors other than EIO.
 */
int
findend()
{
	 
	int	left, right, pmax, pval, eormax, eorval, k, rc;

	/* binary search for logend
	 */
	left = 2; 		/* first page containing log records */
	right = logsize - 1;	/* last page containing log records */

	if ((rc = pageval(left, &eormax, &pmax)) < 0)
	{
		BUGPR(("pageval failed\n"));
		return(rc);
	}

	while ((right - left) > 1)
	{	
		k = (left + right )/2;
		if ((rc = pageval(k, &eorval, &pval)) < 0)
		{
			BUGPR(("pageval failed\n"));
			return(rc);
		}

		if (pval - pmax > 0 )
		{	left = k;
			pmax = pval;
			eormax = eorval;
		}
	        else
			right = k;
	}
	if ((rc = pageval(right, &eorval, &pval)) < 0)
	{
		BUGPR(("pageval failed\n"));
		return(rc);
	}

	if (pval - pmax > 0)
	{
		if ((rc = findppong(&right, &eorval, &pval)) < 0)
			return(rc);
	}
	else
	{
		if ((rc = findppong(&left, &eormax, &pmax)) < 0)
			return(rc);
	}

	return (pval - pmax > 0)
		? (ctob(right) + eorval)
		: (ctob(left) + eormax);
}

/*
 * NAME:	findppong
 *
 * FUNCTION: 	Examines the last page found with the binary search in
 *		findend().  This binary search may not have produced the last
 *		page since the last page of the log (i) is written 
 *		to alternate locations (i+1 and i+2).  This is done since 
 *		the page is rewritten multiple times and each alternate 
 *		location will hold the last committed record.  If the 
 *		machine went down while writing the last page then we 
 *		can be assured that the last committed transaction appears 
 *		on a previous page or a ping pong page.  The findend() 
 *		algorithm may converge on one of several positions
 *		in a ping pong section.  
 *
 * 		(lastpage) may be (i-1) the last page before an unwritten 
 *		ping pong page (i).  In this situation (i) may have ping 
 *		pong pages written to (i+1) and/or (i+2) which would need 
 *		to be written to (i).
 *
 *		(lastpage) may be (i+1) or (i+2). (i+1) or (i+2) may need 
 *		to be written to (i). 
 *
 *		Or (lastpage) may be (i) on a ping pong section that has
 *		been completed.  (i+1) or (i+2) may need to be written to
 *		(i) if (i) is invalid.
 *
 *		The algorithm is to first search for a higher log page
 *		than the one found by findend().  If we find a higher page
 *		or a page that is the same page number but with more valid
 *		data, then record that as our last page.  Then search 
 *		backward.  If we find a previous log page, set the target 
 *		page to this previous page plus one.  After this if the 
 *		target page and lastpage don't match then a copy needs to 
 *		be performed.
 *
 * RETURNS:	0		- ok
 *		REFORMAT_ERROR	- reformat the log
 *		MAJOR_ERROR	- other major errors other than EIO.
 */
int
findppong(int *lastpage,  /* last page found by findend() */
	  int *lasteval,  /* last page's eor		  */
	  int *lastpval)  /* last page's log page value   */
{
	int i, 		/* for loop counter */
	    rc,		/* return code */
	    buf,	/* logp[] element corresponding to page */
	    eorval, 	/* End Of Record value in log page */
	    pval,	/* Page value in log page */
	    maxpval,	/* largest pval possible in a ping pong group */
	    searchpage,	/* current page we're looking at */
	    targetpage;	/* target of a ping/pong copy */ 

	searchpage = *lastpage;

	/* findend() may have converged on one of three possible ping 
	 * pong locations within a ping pong section.  Ensure that we 
	 * start our search from the first one written.
	 */
	for (i = 1; i < 3; i++)
	{
		if (--searchpage < 2)
			searchpage = logsize - 1;
		
		if ((rc = pageval(searchpage, &eorval, &pval)) < 0)
		{
			BUGPR(("pageval failed\n"));
			return(rc);
		}
		if (pval == *lastpval)
		{
			*lastpage = searchpage;
			*lasteval = eorval;
		}
	}

	/* Initially the page we will be starting from is the lastpage 
	 * found in findend().  Given a pval from findend() we should
	 * not find a second pval that is greater than lastpval plus one
	 * (maxpval).  If we do then we encountered an I/O error in the 
	 * middle of the log and ended up in findppong() due to the 
	 * binary search converging on too low a page.
	 */
	maxpval    = *lastpval + 1;
	searchpage = *lastpage;
	targetpage = *lastpage;

		
	/* Redundancy check the page given to us.  This is important
	 * in the case where findend() found (i) but (i) is invalid and
	 * must have (i+1) or (i+2) written to it.
	 */
	if (xorcheck(*lastpage))
		*lasteval = 8;

	/* We need to search up to three pages past the current last page
	 * to determine if we are indeed on the last page.  
	 */
	for (i = 1; i <= 3; i++)
	{
		if (++searchpage > logsize - 1)
			searchpage = 2;

		if ((rc = pageval(searchpage, &eorval, &pval)) < 0)
		{
			BUGPR(("pageval failed\n"));
			return(rc);
		}

		/* (maxpval != 0) handles the specific case where the first
		 * page of the log has been written to the first ping pong
		 * page and no page has been written to the origin.  In
		 * this instance the binary search in findend() will have
		 * not found any pages in the log and will have converged
		 * on the last page of the log.  These loops will find the
		 * ping pong page.
		 */
		if ((pval > maxpval) && (maxpval != 0)) 
			return REFORMAT_ERROR;

		/* If we found a pval that is greater than our last recorded
		 * pval, or if we got a pval that was the same and the page
		 * had more records in it, then do the redundancy check and
		 * record this latter page.
		 */
		if ((pval > *lastpval || 
		    (pval == *lastpval && eorval > *lasteval)) &&
		    !xorcheck(searchpage))
		{
			*lastpage = searchpage;
			*lastpval = pval;
			*lasteval = eorval;
		}
	}

	/* Now that we have found the last possible page in the log we
	 * need to check the pages previous to it to see if we were in
	 * a ping pong section. 
	 */
	for (i = 1; i <= 6; i++)
	{
		if (--searchpage < 2)
			searchpage = logsize - 1;

		if ((rc = pageval(searchpage, &eorval, &pval)) < 0)
		{
			BUGPR(("pageval failed\n"));
			return(rc);
		}

		/* If we have found a pval that is one less than our last
		 * page's pval then record the final target page.  Note: we
		 * can't just quit this loop once we find the first lower page
		 * because our last page was ping pong'd as well and one of 
		 * its pages may appear in our origin location.
		 */
		if (pval == (*lastpval - 1))
		{
			if ((targetpage = searchpage + 1) > logsize - 1)
				targetpage = 2;
		}
	}
	
	/* If we discovered either a new last page or we discovered that
	 * the last page was in a ping pong section and needed to be moved
	 * to a previous target page then perform the copy.
	 */
	if (*lastpage != targetpage)
	{
		if ((buf = getpage(*lastpage)) < 0)
		{
			BUGPR(("getpage failed\n"));
			return(buf);
		}
		*lastpage = targetpage;

		lseek(logfd, (off_t)ctob(targetpage), SEEK_SET);
		if (write(logfd,(char *)&logp[buf],(unsigned)PAGESIZE) < 0)
               	{
                       	BUGPR(("write failed errno = %d\n", errno));
                       	return(errno==EIO ? REFORMAT_ERROR:MAJOR_ERROR);
               	}

		/* Invalidate the cache since we rewrote over target page
		 */
        	for (i = 0; i <= 3; i++)
                	if (logptr[i] == targetpage) 
				logptr[i] = 0;
	}

	/* If we entered with a lastpage that failed the redundancy check
	 * and there were no ping pong pages then move back one page.
	 */
	if (*lasteval == 8)
	{
		if (--(*lastpage) < 2)
			*lastpage = logsize - 1;
		if ((rc = pageval(*lastpage, lasteval, lastpval)) < 0)
		{
			BUGPR(("pageval failed\n"));
			return(rc);
		}
	}

	return 0;
}

/*
 * NAME: 	xorcheck(pno)
 *
 * FUNCTION: 	Calculates the redundancy check of log data up to the end
 *	     	of the last record in the page (eor).  The header and 
 *		trailer are excluded.  The check consists of XOR'ing the 
 *		data.  
 *
 * RETURNS:	0	Redundancy check is ok
 *		1 	Check mismatch
 */
int
xorcheck(int pno)	/* page number in log */
{
	int i, nbytes, pagexor, buf, xor = 0;
	int *xorptr;

	/* Logs previous to version 4.1.A do not have redundancy checks
	 */
	if (sup.magic != LOGMAGICV4)
		return 0;

	if ((buf = getpage(pno)) < 0)
	{
		BUGPR(("getpage failed\n"));
		return 1;
	}
	
	xorptr = (int *)logp[buf].ldata;
	nbytes = logp[buf].h.eor - 8;

	for (i = 0; i < (nbytes / sizeof(int)); i++, xorptr++)
		xor ^= *xorptr;

	pagexor = logp[buf].h.xor;
	pagexor = (pagexor << 16) | (logp[buf].t.xor & 0xFFFF);

	return ((xor == pagexor) ? 0 : 1 );
}

/* 
 * NAME:	pageval(pno, eor, pmax)
 *
 * RETURNS:	0		- ok
 *		REFORMAT_ERROR	- reformat the log
 *		MAJOR_ERROR	- other major errors other than EIO.
 */
pageval(int pno, 	/* page number in log 		*/
	int *eor, 	/* corresponding eor value 	*/
	int *pmax) 	/* pointer to returned page number */ 
{
	int  	rc,	
		buf0,		/* logp[] buffer element number		*/
		buf1,	
		prevpno,	/* On error previous log page 		*/
		preveor,	/* previous log page eor		*/
		prevpval;	/* previous log page header page value 	*/
	
	/* Read the page into the log buffer pool.  If we encounter an 
	 * I/O error then build a valid page. 
	 */
	if ((buf0 = getpage(pno)) < 0)
	{
		/* Get previous page before i/o error page.   If head of log, 
		 * then previous page is at the end of log
		 */
		prevpno = (pno == 2) ? logsize - 1 : pno - 1;

		if ((buf1 = getpage(prevpno)) < 0)
		{
			BUGPR(("getpage failed\n"));
			return(buf1);
		}
		if ((rc = setpage(prevpno, &preveor, &prevpval, buf1)) < 0)
		{
			BUGPR(("setpage failed\n"));
			return(rc);
		}

		/* Since we received an I/O error on page (pno) we will
		 * have to correct this page for a subsequent ping pong
		 * search.  The page number is set to one greater than
		 * the previous page number.  (pmax) is returned using the
		 * same semantics as it is in setpage() (ie. the page number
		 * of the log page the last time thru the log).  The eor is 
		 * set to eight to signify no records on this page.  
		 * findppong() will subsequently find any valid ping pong 
		 * pages that may exist for this page.
		 */
		*eor  = 8;
		*pmax = (logp[buf1].h.page + 1) - (logsize + 2);

        	nextrep = (nextrep + 1) % 4;
		logp[nextrep].h.eor  = logp[nextrep].t.eor  = 8;
		logp[nextrep].h.page = logp[nextrep].t.page = 
							logp[buf1].h.page + 1;
		logptr[nextrep] = pno;

		lseek(logfd, (off_t)ctob(pno), SEEK_SET);
		if (writex(logfd, (char *)&logp[nextrep], PAGESIZE, WRITEV) < 0)
		{
			BUGPR(("write failed errno = %d\n", errno));
			return(errno == EIO ? REFORMAT_ERROR : MAJOR_ERROR);
		}
        	return 0;
	}
	return(setpage(pno, eor, pmax, buf0));
}
	

/* 
 * NAME:	setpage(pno, eor, pmax, buf)
 *
 * FUNCTION: 	Forms consistent log page and returns eor and pmax values.
 *		If the header and trailer in the page are not equal they 
 *		are reconciled as follows:  if the page values are 
 *		different the smaller value is taken and the eor fields 
 *		set to 8 (i.e the page is considered not written). if 
 *		the eor fields are different the smaller of their value 
 *		is taken.  if no record ends on the page (eor = 8), the 
 *		value returned is h.page - logsize + 2 (i.e. we  treat it
 * 		as not written for the purpose of finding the end of
 * 		the log).
 *
 * RETURNS:	0		- ok
 *		REFORMAT_ERROR	- reformat log
 *		MAJOR_ERROR	- other major error
 */
setpage(int pno, 	/* page number of log 		*/
	int *eor,	/* log header eor to return 	*/ 
	int *pmax,	/* log header page number to return */
	int buf) 	/* logp[] index number for page */
{
	int diff1, diff2;

	/* check that header and trailer are the same 
	 */
	diff1 = logp[buf].h.page - logp[buf].t.page;
	if (diff1 != 0)
	{	if (diff1 > 0) 
			logp[buf].h.page = logp[buf].t.page;
		else
			logp[buf].t.page = logp[buf].h.page;

		logp[buf].h.eor = logp[buf].t.eor = 8;  /* empty page */
		logp[buf].h.xor = logp[buf].t.xor = 0;
	}

	diff2 = logp[buf].h.eor - logp[buf].t.eor;
	if (diff2 != 0)
	{	if (diff2 > 0)
			logp[buf].h.eor = logp[buf].t.eor;
		else
			logp[buf].t.eor = logp[buf].h.eor;

		/* Version 4.1.A logs can disregard pages where eor does
		 * not match since this happens during a rewrite of log
		 * pages.  We can disregard these pages since rewrites
		 * occur to differing ping pong pages.
		 */
		logp[buf].h.xor = logp[buf].t.xor = 0;
	}
	
	/* if any difference write the page out
	 * don't need llseek here...  never > 2 gig
	 */
	if (diff1 || diff2)
	{	
		lseek(logfd, (off_t)ctob(pno), SEEK_SET);
		if (write(logfd, (char *)&logp[buf], (unsigned)PAGESIZE) < 0)
		{
			BUGPR(("write failed errno = %d\n", errno));
			return(errno == EIO ? REFORMAT_ERROR : MAJOR_ERROR);
		}
	}

	*eor = logp[buf].h.eor;
	*pmax = (logp[buf].h.eor > 8)
		? logp[buf].h.page
		: (logp[buf].h.page - logsize + 2); /* last time around */

	return 0;
}

islogging (logname, vmt, cnt)
caddr_t logname;
struct vmount *vmt;
int cnt;
{
	int i;
	struct stat lsb, sb;
	dev_t mountdev;
	int logmajor, logminor;

	if (stat (logname, &lsb) < 0)
	{
		perror ("Stat");
		return(MAJOR_ERROR);
	}
	logmajor = major(lsb.st_rdev);
	logminor = is_filesystem(logname);

	/*
	 * if logminor < 0, then logname is not the name of
	 * 	a JFS dev, it actually IS the log dev.
	 */
	if (logminor < 0)
		logminor = minor(lsb.st_rdev);
	
	for (i = 0; i < mntcnt; i++)
	{
		if (vmt->vmt_gfstype == MNT_JFS &&
			(vmt->vmt_flags & MNT_READONLY) == 0)
		{
			if (stat (vmt2dataptr (vmt, VMT_OBJECT), &sb) < 0)
			{
				perror ("Stat");
				return(MINOR_ERROR);
			}
			memcpy(&mountdev, vmt2dataptr(vmt, VMT_INFO),
			       sizeof(mountdev));
			if (major(mountdev) == logmajor &&
			    minor(mountdev) == logminor)
			{
				fprintf (stderr,
				"Devices already mounted that are using %s\n", 
					logname);
				return(MINOR_ERROR);
			}
		}
		vmt = (struct vmount *)((caddr_t)vmt + vmt->vmt_length);
	}
	return 0;
}

/* 
 * fserror(type,vol,bn)
 *
 * error handling code for single filesystem (volume).
 */
fserror(type, vol, bn)
int type;
int vol; 
int bn;
{
	int status;

	fprintf (stderr, "bad error in volume %d \n", vol);

	retcode = -1;
	vopen[vol].status = FM_LOGREDO;

	switch(type) {
	case OPENERR:
		fprintf (stderr, "Open failed \n");
		break;
	case MAPERR:
		fprintf (stderr, "can not initialize maps \n");
		break;
	case DBTYPE:
		fprintf (stderr, "bad disk block number %d\n", bn);
		break;
	case INOTYPE:
		fprintf (stderr, "bad inode number %d\n", bn);
		break;
	case READERR:
		fprintf (stderr, "can not read block number %d\n", bn);
		break;
	case SERIALNO:
		fprintf(stderr, "log serial number no good /n");
		break;
	case IOERROR:
		fprintf (stderr, "io error reading block number %d\n", bn);
		break;
	}

	return 0;
}

/*
 * logerror(type)
 * error handling for log read errors.
 */
logerror(type,logaddr)
int type;
int logaddr;
{
	int k;

	retcode = -1;
	sup.redone = LOGREADERR;

	switch(type) {
	case LOGEND:
		fprintf (stderr, "find end of log failed \n");
		break;
	case READERR:
		fprintf (stderr, "log read failed %x\n",logaddr);
		break;
	case UNKNOWNR:
		fprintf (stderr, "unknown log record type \n");
		fprintf (stderr, "log read failed %x\n",logaddr);
		break;
	case IOERROR:
		fprintf (stderr, "i/o error log reading page %x\n",logaddr );
		break;
	case LOGWRAP:
		fprintf (stderr, "log wrapped...\n");
	}
	

	/* mark all open volumes in error
	 */
	for(k = 0; k < NUMMINOR; k++)
	{
		if (vopen[k].fd && vopen[k].status != FM_CLEAN) 
			vopen[k].status = FM_LOGREDO;
	}
	
	return 0;
}

/*
 * logopen(log)
 * opens the log and returns its fd. 
 * sets logmajor to the major number of the device.
 */
int
logopen(log)
caddr_t log;
{
	int k;
	struct stat st;
	int fd;

	/* stat the device and set its major number in logmajor.
	 */
	if (stat(log, &st) < 0) 
	{
		BUGPR(("stat of log device failed\n"));
		return (-1);
	}
	logmajor = major(st.st_rdev);

	/*
	 * if the path given was a filesystem, get the minor device number
	 * of the log from the superblock of the filesystem.
	 */
	if ((logminor = is_filesystem(log)) < 0)
		/*
		 * else, assume the path given was a log and just use its minor
		 * device number
		 */
		logminor = minor(st.st_rdev);

	/*
	 * try to open the log
	 */
	if ((fd = makeopen(logminor)) >= 0)
		/*
		 * try to lock the log
		 */
		locklog();

	BUGLPR(-1, fd, ("makeopen failed\n"));

	return(fd);
}

extern void exit(int);

/*
 *	attempt to lock the lock
 */
void
locklog()
{
	int i, fd;

	sprintf(loglockpath, "/tmp/locklog.%d,%d", logmajor, logminor);

	for (fd = -1, i = 0; fd < 0 && i < 10; i++) {
		close(open(loglockpath, O_WRONLY | O_CREAT, 0666));
		fd = open(loglockpath, O_WRONLY | O_NSHARE | O_DELAY, 0);
		}

	if (fd >= 0)
		/*
		 * try to delete the lockfile upon exit
		 */
		atexit(unlocklog);
}

/*
 *	unlocklog
 *
 *	- unlock the log
 */
void
unlocklog()
{
	if (loglockpath[0])
		(void) unlink(loglockpath);
}

/*
 * makeopen(num)
 * opens the device as a character device.
 * returns the fd of the opened device.
 */

int
makeopen(num)
int num;
{
	int fd;
	char *name;

	if ((name = makedevice(num)) != NULL)
	{
		fd = open(name, O_RDWR);
		unlink(name);

		return (fd);
	}

	BUGPR(("unable to create special file\n"));
	return(MAJOR_ERROR);
}

/*
 * makedevice()
 * creates the device as a character device.
 * returns the path of the opened device.
 */

char *
makedevice(minordev)
int minordev;
{
	int k;
	dev_t	device;
	static char name[16];

	/* do a make node for the device
	 */
	device = makedev(logmajor, minordev);
	for (k = 0; k < 10000; k++)
	{
		sprintf(name,"/tmp/%d",k);
		if (mknod(name,S_IFCHR | S_IRUSR | S_IWUSR,device) == 0)
		{
			return(name);
		}
	}

	BUGPR(("unable to create file\n"));

	return(NULL);
}

/*
 *	is_filesystem
 *
 *	- peek at the filesystem and see if it's a AIXV3 filesystem.
 *
 *	- if so, return the minor device number of the log for this
 *	  filesystem (it's in the superblock...)
 *	- if not, return -1.
 *
 *	- note this is kinda goofy since we re-open the log above.
 *	  I wanted to change as little code as possible so I wouldn't
 *	  break anything that already worked!
 */
int
is_filesystem(log)
caddr_t log;
{
	int fd, rc;
	struct superblock sb;

	rc = MINOR_ERROR;

	if ((fd = open(log, O_RDONLY)) < 0) {
		perror(log);
		return(rc);
	}

	/* try to read the superblock and check if it has a valid
	 * file system magic number.
	 */
	if (rdwrsuper(fd, &sb, B_READ) == 0 && 
	    (strncmp(sb.s_magic,fsv3magic,(unsigned)strlen(fsv3magic)) == 0 ||
	     strncmp(sb.s_magic,fsv3pmagic,(unsigned)strlen(fsv3pmagic)) == 0))
	{
		/* if the minor dev is out of range, just exit quietly.
		 */
		if ((rc = minor(sb.s_logdev)) >= NUMMINOR)
			exit (0);
	}

	close(fd);
	return (rc);
}

/*
 *	logform
 *
 *	- run logform on our log
 */
int
logform()
{
	char *log;
	int pip[2], pid, k;

	printf("logredo: log i/o error...  reformatting....\n");

	/*
	 * get a log device
	 */
	if ((log = makedevice(logminor)) == NULL) {
		fprintf(stderr,
			"logredo: can't make a log device to logform\n");
		return (-1);
		}

	/*
	 * open a pipe
	 */
	if (pipe(pip) < 0) {
		perror("pipe");
		return (-1);
		}

	/*
	 * fork off a kid
	 */
	if ((pid = fork()) < 0) {
		perror("fork");
		return (-1);
		}

	if (pid == 0) {		/* kid */
		/*
		 * setup for stdin to come from the pipe
		 */
		close(0);
		dup(pip[0]);
		close(pip[1]);

		execl("/usr/sbin/logform", "logform", log, (char *) 0);
		perror("logform");

		exit(1);
		}

	/* parent */

	/*
	 * write y down the pipe (logform needs a y to continue)
	 */
	close(pip[0]);
	write(pip[1], "y\n", 2);

	wait((int *) 0);
	close(pip[1]);

	unlink(log);

	return (0);
}

/* check whether nfs is loaded */

static
int
nfsisloaded()
{
	int sav_errno;
	int (*entry)();

	if (entry = load("/usr/sbin/probe", 0, 0))
		return 1;

	if (errno == ENOEXEC) {
		fprintf(stderr, "%s: nfs is not loaded\n", prog);
		return 0;
	}

	sav_errno = errno;
	fprintf(stderr, "%s: ", prog);
	errno = sav_errno;
	perror("load");
	return 0;
}

usage ()
{
	fprintf (stderr, "Usage: %s [-n] filename\n", prog);
	exit (1);
}

#ifdef _DEBUG

/*
 * xdump -- hex dump routine to facilitate debugging.
 */
xdump (saddr, count)
char *saddr;
int count;
{

#define LINESZ     60
#define ASCIISTRT    40
#define HEXEND       36

    int i, j, k, hexdigit;
    register int c;
    char *hexchar;
    char linebuf[LINESZ+1];
    char prevbuf[LINESZ+1];
    char *linestart;
    int asciistart;
    char asterisk = ' ';
    void x_scpy ();
    int x_scmp ();


    hexchar = "0123456789ABCDEF";
    prevbuf[0] = '\0';
    i = (int) saddr % 4;
    if (i != 0)
	saddr = saddr - i;

    for (i = 0; i < count;) {
	for (j = 0; j < LINESZ; j++)
	    linebuf[j] = ' ';

	linestart = saddr;
	asciistart = ASCIISTRT;
	for (j = 0; j < HEXEND;) {
	    for (k = 0; k < 4; k++) {
		c = *(saddr++) & 0xFF;
		if ((c >= 0x20) && (c <= 0x7e))
		    linebuf[asciistart++] = (char) c;
		else
		    linebuf[asciistart++] = '.';
		hexdigit = c >> 4;
		linebuf[j++] = hexchar[hexdigit];
		hexdigit = c & 0x0f;
		linebuf[j++] = hexchar[hexdigit];
		i++;
	    }
	    if (i >= count)
		break;
	    linebuf[j++] = ' ';
	}
	linebuf[LINESZ] = '\0';
	if (((j = x_scmp (linebuf, prevbuf)) == 0) && (i < count)) {
	    if (asterisk == ' ') {
		asterisk = '*';
		(void) printf ("    *\n");
	    }
	}
	else {
	    (void) printf ("    %x  %s\n",linestart, linebuf);
	    asterisk = ' ';
	    x_scpy (prevbuf, linebuf);
	}
    }

    return;
}

int x_scmp(s1,s2)
register char *s1,*s2;
{
    while ((*s1) && (*s1 == *s2)) {
	s1++;
	s2++;
    }
    if (*s1 || *s2)
	return(-1);
    else
	return(0);
}

void x_scpy(s1,s2)
register char *s1,*s2;
{
    while ((*s1 = *s2) != '\0') {
	s1++;
	s2++;
    }
}


#endif /* _DEBUG */

