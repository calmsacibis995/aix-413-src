static char sccsid[] = "@(#)55  1.5  src/bos/usr/sbin/defragfs/defragfs.c, cmdfs, bos411, 9428A410j 5/20/94 03:53:18";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: defragfs
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <locale.h>
#include <sys/types.h>
#include <sys/param.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <jfs/inode.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <jfs/fsdefs.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/vmdisk.h>
#include <sys/errno.h>
#include <sys/vmker.h>
#include <nlist.h>
#include <fstab.h>
#include <errno.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/fscntl.h>
#include <nl_types.h>
#include "defragfs_msg.h"

#define MSGSTR(N,S)  catgets(catd,MS_DEFRAGFS,N,S)
nl_catd catd;

const char USAGE_STR[] = "Usage : defragfs [-q | -r] {device | mount-path} \n";
const char NOTJFS_STR[] = "%s is not recognized as a JFS file system. \n";
const char VERSION_STR[] = "%s JFS version number not supported. \n";
const char BITMAP_STR[] = "error reading bit map %d \n";
const char MALLOC_STR[] = "malloc failed \n";
const char IOERR_STR[] = "i/o read error \n";
const char ALLOCBIT_STR[]  = "bit allocator internal error \n";
const char SUPBLK_STR[]  = "i/o error reading superblock \n";
const char MAPINC_STR[]  = "bad bit map size: size =%d, mapsize = %d\n";
const char OPENFAIL_STR[] = "failure in opening device %s\n";
const char READONLY_STR[] = "%s must be mounted read-write \n";
const char GETVFS_STR[] = "getvfsnumber internal error\n";
const char MNTERR_STR[] = "mount of %s failed \n";
const char UMNTERR_STR[] = "device %s got unmounted \n";
const char STANZA_STR[] = "no stanza for %s in /etc/filesystems \n";
const char BEFORE_STR[] = "statistics before running defragfs:\n";
const char AFTER_STR[] = "statistics after running defragfs:\n";
const char OTHER_STR[] = "other statistics:\n";
const char NFMOVED_STR[] = "number of fragments moved %d\n";
const char NBMOVED_STR[] = "number of logical blocks moved %d\n";
const char NALLOC_STR[] = "number of allocation attempts %d\n";
const char NEXACT_STR[] = "number of exact matches %d\n";
const char NSHORT_STR[]  = "number of free spaces shorter than a block %d\n";
const char NSFRAG_STR[]  = "number of free fragments in short free spaces %d\n";
const char NFREE_STR[]  = "number of free fragments %d\n";
const char NINUSE_STR[]  = "number of allocated fragments %d\n";

#define UZBIT  (0x80000000)
#define ONES  (0xffffffff)

#define DBLPERPAGE	(PAGESIZE/sizeof(ulong))
#define INDIRPERPAGE	(PAGESIZE/sizeof(struct idblock))
#define	NOINDIRECT(nb)	(nb <= NDADDR)
#define	DOUBLEI(nb)	(((unsigned)nb) > DBLPERPAGE)
#define	DIDNDX(pg)	(((ulong) pg)/DBLPERPAGE)
#define	SIDNDX(pg)	(((ulong) pg) & (DBLPERPAGE - 1))

int fd;		/* file descriptor for device */
int debug;	/* debug flag */
int query;	/* query option chosen */
int report;	/* report option chosen */
int vfsid;	/* vfs id of mounted file system */
int didmount;	/* set to 1 if device was mounted by this program */
int version;	/* file system version number */
int fssize;	/* size of file system expressed in fragments */
int agsize;	/* disk allocation group size in fragments */
int iagsize;	/* number of inodes per disk ag */
int fragsize;	/* fragment size in bytes */
int fperpage;	/* fragments per 4k page */
int nags;	/* number of allocation groups in fs */
int threshold;	/* set to fperpage..related to what is considered short */
int nextag;	/* next ag not covered by fraginode map */
int firstag;	/* first ag covered by fraginode map */
int reorgfirst; /* first 64 bit interval to work on in reorg */
int reorgnext;  /* next 64 bit interval to work on in next call to reorg */
int exactmatch; /* number of exact matches in allocator */
int numallocs;  /* number of calls to allocator */
int nfragsmoved; /* total number of fragments moved */
int nblocksmoved; /* total number of logical blocks moved */
int realmemsize;  /* real memory size in pages */
int unprofitable; /* calls to movefrag() which didn't meet profitability test */
int ncallmovefrag; /* num calls to movefrag */
int nrunbits;	 /* num fragments is short free spaces */
int firstprint;  /* flag to control printing */


/* raw and block device names.
 */
char rdevice[MAXPATHLEN];
char bdevice[MAXPATHLEN];

/* fragment - inode map .
 */
int htabsize; 	/* size of  hash table */
int ftabsize;	/* size of fraginode table */
int nfragtable; /* number of valid entries in table */
struct fraginode {
	frag_t frag;
	uint	ino;
	uint	gen;
	uint	bno;
	struct  fraginode * next;
};
struct fraginode * ftab;      /* pointer to array (table) of fraginode */	
struct fraginode * freefptr;  /* head of free list */
struct fraginode * nextfptr;  /* next never used struct fraginode in table */
struct fraginode ** htab;     /* hash anchor table for fraginode table */

/* array of logical blocks to move.
 */
#define MAXMOVE 64
int nmovelist;
struct fraginode * moveptr[MAXMOVE];

/* diskmaps. 
 */
int treesize;	/* size of freemap tree */
unsigned int *diskmap; /* disk allocation bit map */
unsigned int * freemap; /* freemap tree over diskmap */
char * nruns;  /* number of short free sequences per 64-bit interval */
char * nalloc; /* number of 1's per 64-bit interval of diskmap */

/* inode and indirect block buffers
 * a four page buffer is kept for inodes. ibfirst and iblast define
 * first and last inodes in the buffer. there is a one page buffer
 * for a single indirect block (contents = sgldisk) and a one page
 * buffer for a double indirect block (contents = dbldisk).
 */
int ibfirst, iblast;
#define IBUFSIZE (4*PAGESIZE/sizeof (struct dinode))
struct dinode ibuffer[IBUFSIZE];
int dbldisk, sgldisk;
struct idblock dblind[PAGESIZE/sizeof(struct idblock)];
unsigned int sglind[PAGESIZE/4];
char buffer[PAGESIZE];

struct fraginode * allocfrag();
struct fraginode * findfrag();

main(argc, argv)
int argc;
char ** argv;
{
	int c;
	int rc, k, nwords;
	extern int optind;

	(void) setlocale (LC_ALL,"");
	catd = catopen (MF_DEFRAGFS, NL_CAT_LOCALE);

	if (argc < 2)
	{
		usage();
		return 99;
	}

	/* parse options
	 */
	while ((c = getopt(argc, argv, "qrD")) != EOF)
		switch(c)
		{
			case 'q':
			query = 1;
			break;

			case 'r':
			report = 1;
			break;

			case 'D':
			debug = 1;
			break;

			default:
			usage();
			exit(1);
		}

	if (report && query)
	{
		usage();
		return 99;
	}

	/* determine device names. open device and check super block.
	 * mount the device if necessary. determine vfsid of mounted
	 * file system.
	 */
	if (rc = defragsetup(argv[optind]))
		goto out;

	if (debug) printf("vfsid %d \n", vfsid);

	/* read allocation bit map into memory.
	 */
	if (rc = readmap())
	{
		printf(MSGSTR(BITMAP, BITMAP_STR), rc);
		goto out;
	}

	if (debug) printmap();

	/* threshold = number of fragments in a PAGE
	 */
	threshold = (PAGESIZE/fragsize);

	if (rc = initvals()) 
	{
		goto out;
	}

	/* output runs etc.
	 */
	printf(MSGSTR(BEFORE, BEFORE_STR));
	printvals();

	if (query) goto out;

	realmemsize = getrealmem();

	while(firstag < nags)
	{
		/* compute frag-inode map starting from firstag.
		 * the map covers [firstag, nextag - 1].
		 */
		if (rc = fraginodemap())
		{
			if (debug) printf("fragment-inode-map failed %d\n", rc);
			goto out;
		}

		/* reorganize the allocation groups covered by the
		 * fraginodemap.
		 */
		for (k = firstag; k < nextag; k++)
		{
			if (rc = reorg(k,1))
			{
				if (debug) printf("reorg failed \n", rc);
				goto out;
			}
		}

		firstag = nextag;
	}

	/* recompute runs etc.
	 */
	initvals();
	if (debug) printmap();

	/* output runs etc.
	 */
	printf(MSGSTR(AFTER, AFTER_STR));
	printvals();

	/* output other statistics
	 */
	printf(MSGSTR(OTHER, OTHER_STR));
	printf(MSGSTR(NFMOVED, NFMOVED_STR), nfragsmoved);
	printf(MSGSTR(NBMOVED, NBMOVED_STR), nblocksmoved);
	printf(MSGSTR(NALLOC, NALLOC_STR) , numallocs);
	printf(MSGSTR(NEXACT, NEXACT_STR), exactmatch);

	out:
	if (didmount) umount(bdevice);
	if (rc == ENOMEM)
		printf(MSGSTR(MALLOC, MALLOC_STR));
	if (rc == EIO)
		printf(MSGSTR(IOERR, IOERR_STR));
	return rc;
}

/* computes the fragment to inode map for as much of the disk
 * map as possible starting from firstag. sets nextag to the
 * number of the next allocation group which is not covered
 * by the map. the amount of the map that can be covered depends
 * on the amount of real memory which is available and on the
 * number of map entries which are needed to cover the interval.
 * the latter is determined as the computation proceeds (it depends
 * on the average number of fragments per logical block of files
 * for those fragments in 64-bit intervals of the diskmap which
 * are not fully allocated).
 */

int 
fraginodemap()
{
	int k, rc, count, nbytes, ino, first, maxpages;
	struct dinode * dp;
	struct fraginode * fptr, * next;

	if (ftab == NULL)
	{
		count = 0;
		first = firstag*(agsize/64);
		for (k = first; k < fssize/64; k++)
		{
			/* fully allocated intervals can be ignored */
			if (nalloc[k] != 64) count += nalloc[k];
		}

		/* count is an upper bound on the number of
		 * fraginodes structures that will be needed.
		 * maxpages is an upper bound on the storage
	 	 * we will use.
		 */
		nbytes = count*(sizeof (struct fraginode));
		maxpages = MIN(realmemsize/3, 16*1024);
		nbytes = MIN(nbytes, 4096*maxpages);
		if ((ftab = malloc(nbytes)) == NULL)
		{
			return ENOMEM;
		}
		ftabsize = nbytes/(sizeof (struct fraginode));
	}

	nextfptr = ftab;
	freefptr = NULL;
	nfragtable = 0;

	/* assume that we will have enuff memory for all of the
	 * allocation groups. if not, nextag will be reduced in
	 * the computation.
	 */
	nextag = nags;

	/* read all inodes
	 */
	for (ino = 0; ino < nags*iagsize; ino++)
	{
		if (ino <= SPECIAL_I && ino != ROOTDIR_I)
			continue;

		if (rc = getinode(ino,&dp))
		{
			return rc;
		}

		if (dp->di_nlink == 0) continue;
		switch(dp->di_mode & IFMT)
		{
		case IFLNK:
			if(dp->di_size <= D_PRIVATE) continue;
		case IFREG:
		case IFDIR:
			addfrags(dp, ino);
			break;
		default:
			continue;
		}
	}

	if (debug) printfraginode();

	/* now allocate the htab for the map and put everything 
	 * in hash table.
	 */ 
	if (htab != NULL) 
	{
		free(htab);
		htab = NULL;
	}

	htabsize = 2;
	while(htabsize < nfragtable/4)
	{
		htabsize = 2*htabsize;
	}

	if ((htab = malloc(htabsize*4)) == NULL)
		return ENOMEM;

	for (k = 0; k < htabsize; k++)
		htab[k] = NULL;

	for (fptr = ftab; fptr < nextfptr; fptr += 1)
	{
		if (fptr->frag.addr >= nextag*agsize)
			continue;
		k = (fptr->frag.addr)&(htabsize - 1);
		fptr->next = htab[k];
		htab[k] = fptr;
	}
	return 0;
}

/* allocate a free fraginode structure. if none available reduce
 * value of nextag and replenish free list with the entries which
 * refer to fragments past beginning of nextag.
 */
struct fraginode *
allocfrag()
{
	int nmore, lastfrag, k;
	struct fraginode * fptr;

	loop:

	if (freefptr != NULL)
	{
		fptr = freefptr;
		freefptr = freefptr->next;
		return fptr;
	}

	/* try to get 64 free structures from the initial allocation
	 * of table. nextfptr -> never used structures.
	 */
	if (nextfptr < ftab + ftabsize)
	{
		nmore = (ftab + ftabsize) - nextfptr;
		nmore = MIN(64,nmore);
		freefptr = nextfptr;
		for (fptr = nextfptr;fptr < nextfptr + nmore - 1; fptr += 1)
		{
			fptr->next = fptr + 1;
			fptr->frag.addr = nags*agsize;
		}
		fptr->next = NULL;
		nextfptr = fptr + 1;
		goto loop;
	}

	/* reduce the size of nextag and remove the entries in
	 * ftab that refer to  beyond new value of nextag.
	 */
	if (nextag == firstag + 1) return NULL;
	nextag = nextag - 1;
	lastfrag = nextag*agsize;
	for (k = 0; k < ftabsize; k++)
	{
		if(ftab[k].frag.addr < lastfrag) continue;
		nfragtable -= 1;
		ftab[k].next = freefptr;
		freefptr  = ftab + k;
	}
	goto loop;
}

/* inserts into the fragment-inode table entries for each logical
 * block of the file which correspond to disk fragments that are
 * in the interval [firstag, nextag - 1] but not in fully allocated
 * 64-bit intervals of the disk map.
 */
int
addfrags(dp, ino)
struct dinode * dp;
int ino;
{

	frag_t frag;
	struct fraginode * fptr;
	int k, nblocks, ag, dbl, rc, gen;

	/* invalidate sglind and dblind buffers
	 */
	sgldisk = dbldisk = 0;
	nblocks = (dp->di_size + PAGESIZE - 1)/PAGESIZE;
	gen = dp->di_gen;

	for (k = 0; k < nblocks; k++)
	{
		if (rc = bmap(dp,k,&frag))
		{
			if (debug) printf("bmap failed %d \n", rc);
			return rc;
		}

		/* test if fragment address is of interest
		 */
		if (frag.addr == 0) continue;
		dbl = frag.addr/64;
		if (nalloc[dbl] == 64) continue;
		ag = frag.addr/agsize;
		if (ag >= nextag || ag < firstag) continue;

		/* insert entry into fragment-inode table.
		 * (allocfrag could have resized nextag..ok).
		 */
		if ((fptr = allocfrag()) == NULL) continue;
		fptr->frag = frag;
		fptr->ino = ino;
		fptr->gen = gen;
		fptr->bno = k;
		nfragtable += 1;
	}
	return 0;
}

/* returns in *dpp a pointer to the disk inode specified by ino.
 */
int
getinode(ino, dpp)
int ino;
struct dinode **dpp;
{
	int nbytes, ag, rem, disk, iperpage;

	loop:

	if (ino >= ibfirst && ino < iblast)
	{
		*dpp = ibuffer + (ino - ibfirst);
		return 0;
	}

	/* determine the disk page number that contains ino.
	 */
	iperpage = PAGESIZE/(sizeof (struct dinode));
	ag = ino/iagsize;
	disk = (ag == 0) ? INODES_B : ag*agsize/(PAGESIZE/fragsize);
	rem = ino - ag*iagsize;
	disk += rem/iperpage;
	ibfirst = ag*iagsize + (rem/iperpage)*iperpage;
	iblast = MIN(ibfirst + IBUFSIZE, (ag + 1)*iagsize);

	/* read it and next pages in.
	 */
	llseek(fd, (offset_t)disk * PAGESIZE, SEEK_SET);
	nbytes = (iblast - ibfirst)*(sizeof (struct dinode));
	if (nbytes != read(fd,ibuffer,nbytes))
	{
		return EIO;
	} 

	goto loop;
}

/* reorganize the numags allocation groups starting with ag 
 */
int
reorg(ag,numags)
int ag;
int numags;
{
	int k, next, pos0, pos1, before, after, len, runs, rc;
	int zeros[33], ones[33];

	/* reorfirst and reorgnext are in units of 64-bit intervals
	 */
	reorgfirst = ag*agsize/64;
	reorgnext = reorgfirst + numags*agsize/64;
	reorgnext = MIN(reorgnext, fssize/64);

	if (rc = maketree())
	{
		return rc;
	}

	next = reorgnext;
	while(1)
	{
		/* pick "next" 64-bit interval to work on.
		 */
		if ((next = getinterval(next)) < 0)
			return 0;

		/* process the interval.
	 	 */
		runs = zeros[0] = pos0 = 0;
		while(pos0 < 64)
		{
			pos1 = nextbit(next,pos0,1);
			if (pos1 >= 64) break;
			ones[runs++] = pos1;
			pos0 = nextbit(next,pos1,0);
			zeros[runs] = pos0;
		}
		ones[runs] = 64;

		for (k = 0; k < runs; k++)
		{

			before = ones[k] - zeros[k];
			len = zeros[k + 1] - ones[k];
			after = ones[k + 1] - zeros[k + 1];
			movefrag(64*next+ones[k], len, before, after);
		}
			
		/* update freemap etc. 
	 	 */
		updatetree(next);
	}
	return 0;
}

/* moves if "profitable" the fragments starting at frag.
 */
int
movefrag(frag, nbits, before, after)
int frag;  /* bit number of first fragment */
int nbits;   /* number of consecutive bits */	
int before; /* length of run of preceding zeros */
int after;  /* length of run of following zeros */
{
	int k, n,lasti, lastb, nleft, nfrags, nf;
	unsigned int fit;
	struct fraginode * fptr;

	ncallmovefrag += 1;

	/* bits at beginning or end of 64-bit interval as opposed
	 * to interior have different thresholds for "profitability"
	 */
	nfrags = (before == 0 || after == 0) ? 32 : 64;
	if(nbits > nfrags)
	{
		unprofitable += 1;
		return 0;
	}
	
	/* build list of fptr's corrsponding to the interval of 
	 * bits [frag, frag + nbits - 1].
	 */
	nmovelist = 0;
	for(n = 0; n < nbits; n += fperpage - fptr->frag.nfrags)
	{
		if ((fptr = findfrag(frag + n)) == NULL) return 0;
		moveptr[nmovelist++] = fptr;
	}

	/* reallocate and move each logical block.
	 */
	for (k = 0; k < nmovelist; k += n)
	{
		fptr = moveptr[k];
		lasti = fptr->ino;
		lastb = fptr->bno;
		nfrags = fperpage - fptr->frag.nfrags;
		nleft = nmovelist - k - 1;
		/* adjacent logical blocks are moved together to maintain
		 * contiguity on disk.
		 */
		for(n = 1; n <= nleft; n++)
		{
			fptr = moveptr[k + n];
			if (fptr->ino != lasti || fptr->bno != lastb + n)
				break;
			nfrags += fperpage - fptr->frag.nfrags;
		}

		/* if there is at least one more frag, and there won't
		 * be an exact fit for nfrags, try for an exact-fit with
		 * the next frag included.
		 */
		if (n <= nleft)
		{
			fit = UZBIT >> (nfrags - 1);
			if (!(fit & freemap[0]))
			{
				/* try for exact-fit with next included */
				nf = nfrags + fperpage - fptr->frag.nfrags;
				fit = UZBIT >> (nf - 1);
				if (fit & freemap[0])
				{
					nfrags = nf;
					n += 1;
				}
			}
		}

		/* allocate and move the frags
		 */
		allocmove(k, n, nfrags);
	}
}

/* attempts to move the logical blocks specified. nfrags specifies
 * the number of fragments which must be allocated to hold the 
 * the logical blocks.
 */
int
allocmove(mptr, nmove, nfrags)
int mptr; /* index of first in moveptr array */
int nmove;  /* number to move */
int nfrags; /* number of fragments to allocate */
{
	int nf, newfrag, hash, m, frag, nbits, rc, mprev, mnext;
	struct fraginode ** prev, *fptr;
	
	/* allocator avoids the same 64-bit interval we are 
	 * moving from. allocator is exact-fit if possible
 	 * and first-fit otherwise.
	 */
	if ((newfrag = allocbits(nfrags)) == -1) return 0;

	/* try to move the logical blocks one at a time.
	 */
	nf = 0;
	for (m = mptr; m < mptr + nmove; m++)
	{
		fptr = moveptr[m];

		/* do it unless report option chosen
		 */
		if (report == 0)
		{
			mprev = (m > mptr) ? m - 1 : -1;
			mnext = (m < mptr + nmove - 1) ? m + 1 : -1;
			rc = moveit(fptr, newfrag + nf, mprev, mnext);
			if (rc != 0) return 0;
		}

		/* free old bits. 
		 */
		frag = fptr->frag.addr;
		nbits = fperpage - fptr->frag.nfrags;
		markit(frag,nbits,0);
		nfragsmoved += nbits;
		nblocksmoved += 1;

		/* remove from hash chain 
		 */
		prev = &htab[frag & (htabsize - 1)];
		while(*prev != fptr)
		{
			prev = &((*prev)->next);
		}
		*prev = fptr->next;

		/* re-insert in hash chain
		 */
		fptr->frag.addr = newfrag + nf;
		nf += nbits;
		hash = fptr->frag.addr & (htabsize - 1);
		fptr->next = htab[hash];
		htab[hash] = fptr;
	}

	return 0;
}

/* attempts to move the logical block specified by fptr to the
 * fragments starting at daddr. prev is the index in moveptr[]
 * of the previous call to this procedure, and next the index
 * of the next (anticipated) call. the value of next and prev
 * are used to determine when the moves are committed.
 */
int
moveit(fptr, daddr, prev, next)
struct fraginode * fptr;
int daddr;
int prev;
int next;
{
	int rc, nextino, previno;
	struct plist {
		uint ino;
		uint gen;
		uint bno;
		frag_t oldf;
		frag_t newf;
		int commit;
	} p;

	/* set up plist for call to fscntl()
	 */
	p.ino = fptr->ino;
	p.gen = fptr->gen;
	p.bno = fptr->bno;
	p.oldf = fptr->frag;
	p.newf = p.oldf;
	p.newf.addr = daddr;

	/* commit if next inode is different
	 */
	nextino = (next >= 0) ? moveptr[next]->ino : -1;
	p.commit = (nextino == p.ino) ? 0 : 1;

	/* try to move it
	 */
	if ((rc = fscntl(vfsid, FS_MOVEBLKS, &p, sizeof (struct plist))) == 0)
		return 0;

	/* if the previous inode is the same as the current inode
	 * we want to commit the changes to it.
	 */
	previno = (prev >= 0) ? moveptr[prev]->ino : -1;
	if (previno == p.ino)
	{
		p.oldf = p.newf;
		p.commit = 1;
		fscntl(vfsid, FS_MOVEBLKS, &p, sizeof (struct plist));
	}

	return rc;
}

/* allocates nbits in the disk map and returns the number
 * of the first bit allocated. returns -1 if request could
 * not be satisfied. search for free bits is over the interval 
 * [reorgfirst, reornext-1]. algorithm is exact fit and then
 * first fit. 
 */
int 
allocbits(nbits)
int nbits;
{
	int k, n, dbl, one, zeros, frag, exact;
	unsigned int fit, w1, w2, string, mask;

	/* maximum number bits is 32 */
	if (nbits > 32) return -1;

	/* try for exact-fit first.
	 */
	fit = UZBIT >> (nbits - 1);
	if (fit & freemap[0])
	{
		exact = (nbits < 32);
		exactmatch += exact;
	}
	else
	{
		fit = ONES >> (nbits - 1);
		if ((fit & freemap[0]) == 0) return -1;
		exact = 0;
	}
	numallocs += 1;

	/* descend tree trying left child first.
	 */
	k = 0;
	while(k < treesize/2)
	{
		k = 2*k + 1;
		if (fit & freemap[k]) continue;
		k = k + 1;
	}

	/* dbl is the number of the 64-bit interval of diskmap
	 * which should satisfy request.
	 */
	dbl = reorgfirst + k - treesize/2;
	w1 = diskmap[2*dbl];
	w2 = diskmap[2*dbl + 1];

	zeros = 0;
	for (n = 0; n < 64; n++)
	{
		one = (w1 & UZBIT);
		/* shift (w1,w2) as a double word. or in a one
		 * in w2 for exact match at the end
		 */
		w1 = (w1 << 1) | (w2 >> 31);
		w2 = (w2 << 1) | 1;
		zeros = (one) ? 0 : zeros + 1;
		if (zeros == nbits)
		{
			if (exact == 0 || (w1 & UZBIT)) goto found;
		}
	}

	printf(MSGSTR(ALLOCBIT, ALLOCBIT_STR));
	if (didmount) umount(bdevice);
	exit(1);

	/* mark bits as allocated and update freemap.
	 */
	found:
	frag = 64*dbl + n - nbits + 1;
	markit(frag, nbits, 1);
	updatetree(dbl);

	return (frag);
}

/* sets the bits in the diskmap to the value specified. 
 */
int
markit(firstbit, nbits, val)
int firstbit;
int nbits;
int val;
{
	int word, n, bit, nfirst;

	/* process the bits in first word.
	 */
	bit = firstbit & 0x1f;
	nfirst = MIN(32 - bit,nbits);
	word = firstbit/32;
	for (n = 0; n < nfirst ; n++, bit += 1)
	{
		if (val)
			diskmap[word] |= (UZBIT >> bit);
		else
			diskmap[word] &= ~(UZBIT >> bit);
	}

	/* go on to next word 
	 */
	bit = 0;
	word += 1;
	for (n = 0; n < nbits - nfirst; n++, bit += 1)
	{
		if (val)
			diskmap[word] |= (UZBIT >> bit);
		else
			diskmap[word] &= ~(UZBIT >> bit);
	}
}

struct fraginode *
findfrag(frag)
int frag;
{
	struct fraginode * fptr;

	fptr = htab[frag & (htabsize - 1)];

	for ( ; fptr != NULL; fptr = fptr->next)
	{
		if (fptr->frag.addr == frag) return fptr;
	}

	return NULL;
}

/* searches the 64-bit interval of the map stating at pos for the
 * next bit = type specified. returns its position or 64 if none
 * is found.
 */
int
nextbit(dbl,pos,type)
int dbl;
int pos;
int type;
{
	int start, k;
	unsigned int word, bitmask, w1;

	/* determine the word of the diskmap containing pos.
	 */
	word = 2*dbl;
	if (pos >= 32) word += 1;

	/* search the word for first bit = type
	 */
	start = pos & (0x1f);  
	bitmask = UZBIT >> start;
	w1 = (type) ? diskmap[word] : ~diskmap[word];
	for (k = 0; k < 32 - start; k++)
	{
		if (bitmask & w1) return (pos + k);
		bitmask = bitmask >> 1;
	}

	/* return 64 if we started with 2nd word. otherwise search 2nd word.
	 */
	if (pos >= 32) return 64;
	bitmask = UZBIT;
	w1 = (type) ? diskmap[word + 1] : ~diskmap[word + 1];
	for (k = 32; k < 64; k++)
	{
		if (bitmask & w1) return k;
		bitmask = bitmask >> 1;
	}

	return 64;
}

/* find the next interval which is worth processing.
 * return -1 if the none is worthwhile. also adjusts
 * freemap so that no-allocations will be made from the 
 * interval returned.
 */
int
getinterval(next)
{
	int  best, k, even, odd;

	best = -1;
	for (k = next - 1; k >= reorgfirst; k--)
	{
		if (nalloc[k] < 64 && nalloc[k] > 0)
		{
			best = k;
			break;
		}
	}

	if (best < 0) return -1;

	/* make it so that allocator avoids "best".
	 */
	k = treesize/2 + (best - reorgfirst);
	freemap[k] = 0;
	while(1)
	{
		even = ((k + 1)/2)*2;
		odd = even - 1;
		k = odd/2;
		freemap[k] = freemap[even] | freemap[odd];
		if (k == 0) break;
	}


	return best;
}

/* computes nruns, and nalloc for diskmap. kth element of them
 * refer to the kth 64-bit interval in the diskmap. nruns[k] is the
 * number of runs of 0's which have length < threshold. nalloc[k]
 * is the total number of 1's. 
 */
int
initvals()
{
	int nbytes, k;

	/* allocate storage. agsize is a multiple of 64.
	 * for nruns and nalloc we need one byte per 64 bits.
	 */
	if (nruns == NULL)
	{
		nbytes = nags*agsize/64;
		if ((nruns = malloc(nbytes)) == NULL)
			return ENOMEM;
		if ((nalloc = malloc(nbytes)) == NULL)
			return ENOMEM;
	}

	/* calculate nruns and nalloc for the 64-bit intervals.
	 */
	nrunbits = 0;
	for(k = 0; k < nags*agsize/64; k++)
		evalruns(k);

	return 0;
}

/*
 * read superblock. check if it is a filesystem
 * save the values of agsize, iagsize, fssize, etc.
 */
int
checksuper(device)
char * device;
{
	int rc, rem, iperfrag;
	struct superblock * sb;

	/* read in superblock of the volume into bufpool
	 * and check it out. 
	 */
	lseek(fd, SUPER_B*PAGESIZE, SEEK_SET);
	if ((rc = read(fd,buffer,PAGESIZE)) != PAGESIZE)
	{
		printf(MSGSTR(SUPBLK, SUPBLK_STR));
		return EIO;
	}

	sb = (struct superblock *) buffer;

	/* check magic number.
	 * fssize, agsize are in number of fragments.
	 * iagsize is in number of inodes.
	 */
	if (strncmp(sb->s_magic,fsv3magic,4) == 0)
	{
		if (sb->s_version != fsv3vers)
		{
			printf(MSGSTR(VERSION, VERSION_STR));
			return 99;
		}
		version = ALLOCMAPV3;
		fssize = sb->s_fsize/(PAGESIZE/512);
		iagsize = agsize = sb->s_agsize;
		fragsize = PAGESIZE;
	}
	else
	if (strncmp(sb->s_magic,fsv3pmagic,4)==0 && sb->s_version==fsv3pvers)
	{
		/*
		 * in wacky case where v3 fs grown > 2 gig,
		 * magic and version changed to v4, but maps stay v3
		 */
		if (sb->s_fragsize == sb->s_bsize &&
		    sb->s_iagsize == sb->s_agsize)
			version = ALLOCMAPV3;
		else
			version = ALLOCMAPV4;
		fssize = sb->s_fsize/(sb->s_fragsize/512);
		agsize = sb->s_agsize;
		iagsize = sb->s_iagsize;
		fragsize = sb->s_fragsize;
	}
	else
	{
		printf(MSGSTR(NOTJFS, NOTJFS_STR),device);
		return 99;
	}

	nags = fssize/agsize;
	rem = fssize - nags*agsize;
	iperfrag = fragsize/sizeof(struct dinode);
	nags = ( rem >= iagsize/iperfrag) ? nags + 1: nags;
	fperpage = PAGESIZE/fragsize;

	return 0;
}

/* 
 * readmap()
 * allocate storage for the bitmap and initialize it to the
 * working copy of the disk map on disk. any errors here are
 * fatal.
 */
readmap()
{
	struct dinode * dp;
	struct vmdmap * ptr;
	int rc,nleft,pos,nwords,k, w,mapsize, npages, disk, rem;
	uint *wmap, dbperpage, wperpage;

	if (rc = getinode(DISKMAP_I, &dp))
	{
		return rc;
	}

	/* consistency check on file-system size and size of map
	 * as a file. 
	 */
	dbperpage = WBITSPERPAGE(version);
	wperpage = dbperpage/DBWORD;
	mapsize = fssize;
	npages = (mapsize + dbperpage - 1)/dbperpage;
	if (version == ALLOCMAPV4)
		npages += (npages + 7)/8;
	if (npages > btoc(dp->di_size))	
	{
		printf(MSGSTR(MAPINC, MAPINC_STR), btoc(dp->di_size), npages);
		return(99);
	}

	/* allocate storage for map. the amount is enuff to include
	 * the last allocation group.
	 */
	nwords = agsize*nags/32;
	if((diskmap = malloc(nwords*4)) == NULL)
	{
		return ENOMEM;
	}

	/* read map in a page at a time
	 */
	pos = 0;
	nleft = nwords;
	wmap = (version == ALLOCMAPV4) ?(uint *) buffer: (uint *) (buffer + 512);
	sgldisk = dbldisk = -1;

	for (k = 0; k < npages; k++)
	{
		/* skip control pages for version V4
		 */
		if (version == ALLOCMAPV4  && k % 9 == 0)
			continue;
		/* read next page of diskmap into buffer.
		 */
		if (rc = bmap(dp,k,&disk))
		{
			return rc;
		}
		llseek(fd, (offset_t)disk * fragsize, SEEK_SET);
		if ((read(fd, buffer, PAGESIZE)) != PAGESIZE)
		{
			return EIO;
		}

		/* copy workmap to bitmap a word at a time.
		 */
		nwords = MIN(wperpage, nleft);
		nleft -= nwords;
		for (w = 0; w < nwords; w++)
		{
			diskmap[pos++] = wmap[w];
		}
	}

	/* set the bits in the map beyond mapsize to allocated.
	 */
	for (k = mapsize; k < nags*agsize; k++)
	{
		w = k >> 5;
		rem = k & (0x1f);
		diskmap[w] |= (UZBIT >> rem);
	}
	return 0;
}

/*
 * bmap(dp, bn, bnp)
 * Block map.  Given block # bn in a file , find its disk
 * fragment pointer , return it in bnp. returns 0 if ok,
 * EIO if i/o error, and -1 if single or double indirect 
 * block are not valid (note that file-system is mounted
 * while this program is running so they can be garbage).
 */
bmap(dp, bn, bnp)
struct dinode *dp;		/* Disk inode of file           */ 
ulong  bn;			/* page number in file		*/
ulong *bnp;			/* Returned disk fragment pointer*/
{

	ulong *block;			/* pointer to indirect		*/
	ulong nb;			/* number blocks 		*/
	int rc;				/* Return value bread		*/
	struct idblock *idp;

	nb = btoc(dp->di_size);
	
	if (bn >= nb)
	{
		return -1;
	}

	/* No indirect blocks ?
	 */
	if (NOINDIRECT(nb))
	{
		*bnp = dp->di_rdaddr[bn];
		return 0;
	}

	/* Single indirect ?
	 */
	if (!DOUBLEI(nb))
	{
		if (sgldisk != dp->di_rindirect)
		{
			sgldisk = -1;
			llseek(fd, (offset_t)dp->di_rindirect * fragsize, SEEK_SET);
			if((rc = read(fd, sglind, PAGESIZE)) != PAGESIZE)
				return EIO;
			/* check that its a plausible single indirect block.
			 */
			if (chksingle()) return -1;
			sgldisk = dp->di_rindirect;
		}
		*bnp = sglind[bn & (DBLPERPAGE - 1)];
		return 0;
	}

	/* double indirect
	 */
	if (dbldisk != dp->di_rindirect)
	{
		dbldisk = -1;
		llseek(fd, (offset_t)dp->di_rindirect * fragsize, SEEK_SET);
		if((rc = read(fd, dblind, PAGESIZE)) != PAGESIZE)
			return EIO;
		/* check that its a plausible double indirect block.
		 */
		if (chkdouble()) return -1;
		dbldisk = dp->di_rindirect;
	}

	idp = dblind + DIDNDX(bn);

	if (idp->id_raddr == 0)
	{
		*bnp = 0;
		 return 0;
	}

	if (sgldisk != idp->id_raddr)
	{
		llseek(fd,(offset_t)idp->id_raddr * fragsize, SEEK_SET);
		if((rc = read(fd, sglind, PAGESIZE)) != PAGESIZE)
			return EIO;
		/* check that its a plausible single indirect block.
		 */
		if (chksingle()) return -1;
		sgldisk = idp->id_raddr;
	}

	*bnp = sglind[bn & (DBLPERPAGE - 1)];
	return 0;

}

/* checks that the data in sglind[] is a plausible single indirect
 * block. 
 */
int
chksingle()
{
	int k;
	frag_t *ptr;

	ptr = (frag_t *)sglind;
	for (k = 0; k < DBLPERPAGE; k++)
	{
		if (ptr[k].addr == 0) continue;
		if (ptr[k].addr >= fssize || ptr[k].nfrags >= fperpage)
		{
			sgldisk = -1; 
			return -1;
		}
	}
	return 0;
}

/* checks that the data in dblind[] is a plausible double indirect
 * block. 
 */
int
chkdouble()
{
	int k;

	for (k = 0; k < INDIRPERPAGE; k++)
	{
		if (dblind[k].id_raddr >= fssize)
		{
			dbldisk = -1;
			return -1;
		}
	}
	return 0;
}

int
printmap()
{
	int k, ndbls, first, last, n;

	printf("mapsize %d \n", fssize);

	ndbls = agsize/64;
	for (k = 0; k < nags; k++)
	{
		printf("allocation group %d \n", k);
		first = k*ndbls;
		last = MIN(first + ndbls, fssize/64);
		for (n = first; n < last; n++)
			printf("%8d %0.8x %0.8x \n",
				n, diskmap[2*n], diskmap[2*n+1]);
	}
}

int 
printvals()
{
	int k, ones, n , first, last, runs, ndbls;

	ones = runs = 0;
	ndbls = agsize/64;
	for (k = 0; k < nags; k++)
	{
		if (debug) 
			printf("allocation group %d \n", k);
		first = k*ndbls;
		last = MIN(first + ndbls, fssize/64);
		for (n = first; n < last; n++)
		{
			runs += nruns[n];
			ones += nalloc[n];
			if (debug)
				printf("%8d %8d %8d \n", n, nruns[n], nalloc[n]);
		}
	}

	if (firstprint == 0)
	{
		printf(MSGSTR(NFREE, NFREE_STR), fssize - ones);
		printf(MSGSTR(NINUSE, NINUSE_STR), ones);
	}
	firstprint = 1;

	printf(MSGSTR(NSHORT, NSHORT_STR), runs);
	printf(MSGSTR(NSFRAG, NSFRAG_STR), nrunbits);
	printf("\n");
}

int
printfraginode()
{
	struct fraginode * fptr;

	printf("inode    block    fragment \n");
	for (fptr = ftab; fptr < nextfptr; fptr += 1)
	{
		printf("%8x  %8x  %8x \n", fptr->ino, fptr->bno, fptr->frag);
	}
}

/* calculates nruns[dbl], nalloc[dbl] for the 64-bit interval of
 * the diskmap specified by dbl. nruns is the number of sequences
 * of free bits of length < threshold. also adds to nrunbits the
 * number of bits in such sequences.
 */
int
evalruns(dbl)
int dbl;
{
	int nbytes, zeros, n, ones;
	unsigned int w1, w2, sign;

	w1 = diskmap[2*dbl];
	w2 = diskmap[2*dbl + 1];
	nruns[dbl] = 0;
	sign = w1 & UZBIT;
	nalloc[dbl] = ones = (sign) ? 1 : 0;
	zeros = (sign) ? 0 : 1;

	/* left-shift by one (w1,w2) as a double word.
	 * process the leftmost bit of w1.
	 */
	for (n = 0; n < 63; n ++)
	{
		w1 = (w1 << 1) | (w2 >> 31);
		w2 = w2 << 1;
		if (w1 & UZBIT)
		{
			nalloc[dbl] += 1;
			if (ones) 
			{
				ones += 1;
				continue;
			}
			if (zeros < threshold)
			{
				nruns[dbl] += 1;
				nrunbits += zeros;
			}
			zeros = 0;
			ones = 1;
		}
		else
		{
			if (zeros)
			{
				zeros += 1;
				continue;
			}
			ones = 0;
			zeros = 1;
		}
	}

	if (zeros)
	{
		if (zeros < threshold) 
		{
			nrunbits += zeros;
			nruns[dbl] += 1;
		}
	}

	return 0;
}

/* makes a binary tree that represents the free-bits in the
 * diskmap for the interval [reorgfirst, reornext-1]. the
 * leaves of the tree correspond to 64-bit intervals of the
 * map and encode the free-bits in the interval as follows:
 * for  0 <= n < 31, bit n is a 1 if there is a run of 
 * exactly n+1 0's, and bit 31 is a one it there is a run
 * of 32 or more zeros. the parent of 2-nodes in the tree
 * is the OR of the 2-nodes.
 */
int
maketree()
{
	int k;

	if (freemap != NULL) free(freemap);

	/* calculate the number of words to hold tree. to simplify
	 * addressing we use  2**n - 1 words. the root of the tree is
	 * freemap[0]. left child of freemap[k] is freemap[2*k+1]
	 * and right child is freemap[2*k+2]. the leftmost leaf of 
	 * tree is freemap[treesize/2].
	 */
	treesize = 1;
	while (treesize < reorgnext - reorgfirst) treesize = 2*treesize;
	treesize = 2*treesize - 1;

	/* allocate and init tree. */
	if ((freemap = malloc(4*treesize)) == NULL) return ENOMEM;
	for (k = 0; k < treesize; k++)
		freemap[k] = 0;

	/* compute tree */
	for (k = reorgfirst; k < reorgnext; k++)
		updatetree(k);

	return 0;
}
/* re-evaluates the tree as if the diskmap for 64-bit interval 
 * specified by dbl was changed. dbl is assumed to be in the
 * interval [reorgfirst, reornext-1].
 */
int
updatetree(dbl)
int dbl;
{

	int nbytes, zeros, n, ones, ind, even, odd;
	unsigned int w1, w2;


	ind = treesize/2 + (dbl - reorgfirst);

	freemap[ind] = 0;
	w1 = diskmap[2*dbl];
	w2 = diskmap[2*dbl + 1];
	ones = w1 & UZBIT;
	zeros = (ones) ? 0 : 1;

	/* left-shift by one (w1,w2) as a double word.
	 * process the leftmost bit of w1.
	 */
	for (n = 0; n < 63; n ++)
	{
		w1 = (w1 << 1) | (w2 >> 31);
		w2 = w2 << 1;
		if (w1 & UZBIT)
		{
			if (ones) continue;
			ones = 1;
			zeros = MIN(32,zeros);
			freemap[ind] |= UZBIT >> (zeros - 1);
			zeros = 0;
		}
		else
		{
			if (zeros)
			{
				zeros += 1;
				continue;
			}
			ones = 0;
			zeros = 1;
		}
	}

	if (zeros)
	{
		zeros = MIN(32,zeros);
		freemap[ind] |= UZBIT >> (zeros - 1);
	}

	/* now recompute rest of tree that is affected.
	 */
	while(1)
	{
		even = ((ind + 1)/2)*2;
		odd = even - 1;
		ind =  odd/2;
		freemap[ind] = freemap[even] | freemap[odd];
		if (ind == 0) break;
	}

	return 0;
}

#define MINREALMEM  2048

/* returns the size of real-memory in bytes.
 */
int
getrealmem()  
{
        int  fd, nbytes;
	struct nlist nltbl;
	struct vmkerdata vmkerdata;

        if ((fd = open("/dev/kmem",O_RDONLY)) < 0)
	{
           	if (debug) printf("open /dev/kmem failed \n");
		return 0;
	}
 
	/* find address of vmker
	 */
        nltbl.n_name = "vmker";           
        if (knlist(&nltbl, 1, sizeof(struct nlist)))
	{
           	if (debug) printf("knlist failed \n");
		return MINREALMEM;
	}

	/* read vmker structure
	 */
	lseek(fd, nltbl.n_value, SEEK_SET);
	nbytes = sizeof(struct vmkerdata);
        if (read(fd, &vmkerdata,nbytes) != nbytes)
	{
           	if (debug) printf("read of vmker failed \n");
		return MINREALMEM;
	}
		
	return MAX(MINREALMEM, vmkerdata.nrpages - vmkerdata.badpages);
}
 
/* determines device names. opens raw device. checks super-block.
 * mounts if necessary the device. determines vfsid of mounted
 * filesystem.
 */ 
int
defragsetup(arg)
char *arg;
{
	int rc;
	struct fstab *fs;
	char * device;
	char * mountpath;
	char * emsg;
	char cmd[MAXPATHLEN + 128];
	
	/* determine if device or mount-path specified. getfsfile 
	 * searches /etc/filesystems for the mount-path.
	 */
	fs = getfsfile(arg);
	if (fs == 0)
	{
		/* device was specified.
		 */
		device = arg;

		/* getfsspec searches /etc/filesystem for block device.
		 */
		makebdevice(device, bdevice);
		fs = getfsspec(bdevice);
		if (fs == 0)
		{
			mountpath = NULL;
		}
		else
		{
			mountpath = fs->fs_file;
		}
	}
	else
	{
		/* mount path was specified
		 */
		device = fs->fs_spec;
		mountpath = arg;
	}

	/* convert device to the character and block devices
	 */
	makerdevice(device, rdevice);
	makebdevice(device, bdevice);

	if (debug)
	printf("rdev %s bdev %s mountpath %s \n", rdevice, bdevice, mountpath);

	/* open the raw device
	 */
	fd = open(rdevice, O_RDONLY, 0666);
	if (fd < 0)
	{
		printf(MSGSTR(OPENFAIL, OPENFAIL_STR),device);
		return 99;
	}

	/* check the super block.
	 */
	if (rc = checksuper(device))
	{
		return rc;
	}

	/* don't need to have device mounted if query or report.
	 */
	if (query || report) return 0;

	/* get vfsid if mounted.
	 */
	if((rc = getvfsnumber(bdevice)) == 0)
		return 0;

	switch (rc)
	{
		case -1:
			printf(MSGSTR(GETVFS, GETVFS_STR));
			return -1;
		case -2:
			break;

		case -3:
			printf(MSGSTR(READONLY, READONLY_STR),bdevice);
			return -1;

		case ENOMEM:
			return ENOMEM;
	}

	/* mount it provided mount-path is defined.
	 */
	if (mountpath == NULL) 
	{
		printf(MSGSTR(STANZA, STANZA_STR), bdevice);
		return -1;
	}

	sprintf(cmd, "mount %s %s", bdevice, mountpath);
	if (rc = system(cmd))
	{
		printf(MSGSTR(MNTERR, MNTERR_STR), bdevice);
		return rc;
	}

	/* now try to get vfsid again.
	 */
	didmount = 1;
	if((rc = getvfsnumber(bdevice)) == 0)
		return 0;

	switch (rc)
	{
		case -1:
			printf(MSGSTR(GETVFS, GETVFS_STR));
			return -1;
		case -2:
			printf(MSGSTR(UMNTERR, UMNTERR_STR), bdevice);
			return -2;
		case ENOMEM:
			return ENOMEM;
	}

	/* shouldn't get here
	 */
	return -1;
			
}

/* sets rdev to the raw device corresponding to dev.
 * rdev[0] is set to 0 if dev points to too long a string.
 * assumption is made that block device leaf names do not
 * start with "r".
 */
int
makerdevice(dev, rdev)
char *dev;
char rdev[MAXPATHLEN];
{
	int k, lastslash, n;
	
	lastslash = -1;
	for (k = 0; k < MAXPATHLEN; k++)
	{
		rdev[k] = dev[k];
		if (dev[k] == 0) break;
		if (dev[k] == '/') lastslash = k;
	}

	/* too long or no '\' ?
	 */
	if (lastslash < 0 || k == MAXPATHLEN)
	{
		rdev[0] = 0;
		return;
	}

	/* already raw device ?
	 */
	if (dev[lastslash + 1] == 'r') return;

	/* will have to insert character. test for too long.
	 */
	if (k == MAXPATHLEN - 1)
	{
		rdev[0] = 0;
		return;
	}

	rdev[lastslash + 1] = 'r';
	for (n = lastslash + 1; n <= k; n++)
	{
		rdev[n + 1] = dev[n];
	}
}

/* sets bdev to the block device corresponding to dev.
 * bdev[0] is set to 0 if dev points to too long a string.
 * assumption is made that block device leaf names do not
 * start with "r".
 */
int
makebdevice(dev, bdev)
char *dev;
char bdev[MAXPATHLEN];
{
	int k, lastslash, n;
	
	lastslash = -1;
	for (k = 0; k < MAXPATHLEN; k++)
	{
		bdev[k] = dev[k];
		if (dev[k] == 0) break;
		if (dev[k] == '/') lastslash = k;
	}

	/* too long or no '\' ?
	 */
	if (lastslash < 0 || k == MAXPATHLEN)
	{
		bdev[0] = 0;
		return;
	}

	/* already block  device ?
	 */
	if (dev[lastslash + 1] != 'r') return;

	/* will have to remove character 'r'.
	 */
	for (n = lastslash + 1; n <= k; n++)
	{
		bdev[n] = dev[n + 1];
	}

	return;
}

/*
 *	getvfsnumber(dev)
 *
 *	if successful, sets vfsid to the vfsnumber of the mounted
 *	filesystem and return 0.
 *	- returns -1 if an error occurs.
 *	- returns -2 if device is not mounted.
 *	- returns -3 if device is mounted read-only
 *	- returns ENOMEM if couldn't allocate space.
 */
int
getvfsnumber (dev)
char * dev;  /* block device name */
{
	int count, ct, mntsiz, rc, readonly;
	struct vmount *vmt;
	char *mnttab;

	mnttab = (char *) &count;
	mntsiz = sizeof(count);

	/* loop till we have enuf mem to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0)
	{
		/* error?
		 */
		if (ct < 0) return -1;

		/* get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ?
			(char *) malloc(mntsiz) :
				(char *) realloc(mnttab, mntsiz);

		if (mnttab == NULL)
		{
			return ENOMEM;
		}
	}

	/* walk thru the mount table, see if this device is mounted
	 */
	for (vmt = (struct vmount *) mnttab; ct > 0;
	     ct--, vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length))
	{
		if (strcmp(vmt2dataptr(vmt, VMT_OBJECT), dev) == 0)
		{
			vfsid = vmt->vmt_vfsnumber;
			readonly = vmt->vmt_flags & MNT_READONLY;
			free(mnttab);
			return (readonly) ? -3 : 0;
		}
	}

	/* not mounted 
	 */
	free(mnttab);
	return (-2);
}

usage()
{
	printf (MSGSTR (USAGE, USAGE_STR));
}


