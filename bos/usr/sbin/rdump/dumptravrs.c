static char sccsid[] = "@(#)93	1.40  src/bos/usr/sbin/rdump/dumptravrs.c, cmdarch, bos41J, 9518A_all 4/27/95 18:20:44";
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
static char sccsid[] = "(#)dumptraverse.c      5.3 (Berkeley) 1/9/86";
#endif not lint
*/

#include <nl_types.h>
#include "dumpi_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_DUMP,N,S)
nl_catd catd;

#include "dump.h"
#include <unistd.h>
#include <sys/acl.h>
#include <sys/pcl.h>
#include <sys/types.h>

#define NINDIRECT (FSBSIZE/sizeof(daddr_t))

static int bmap();
void get_total_ino();
void ginode();
void read_superblk();
struct acl *getacl();
struct pcl *getpcl();
extern struct dinode *inoext; /* Ptr to INODEX_I which contains security data */
extern daddr_t fs_size;
extern ino_t imax;	/* equal to last allocated inode in the filesystem */
static int fscompress;  /* equal to s_compress field in super block */
int compressed;  /* set in dump(ip) if ip is compressed. (used by dmpblk()) */
#ifndef RDUMP
extern ino_t last_ino_dumped;   /* the last inode that actually got dumped */
#endif

checkinode(ip)
struct dinode *ip;
{
	int i;
	printf("INO number %d\n", ino);
	printf("di_nlink=%d\n", ip->di_nlink);
	printf("di_mode=%o\n", ip->di_mode);
	printf("di_uid=%d\n", ip->di_uid);
	printf("di_gid=%d\n", ip->di_gid);
	printf("di_size=%d\n", ip->di_size);
	printf("di_nblocks=%d\n", ip->di_nblocks);
	printf("di_mtime=%d\n", ip->di_mtime);
	printf("di_atime=%d\n", ip->di_atime);
	printf("di_ctime=%d\n", ip->di_ctime);
	for (i = 0; i < NDADDR; i++)
		printf("di_rdaddr[%d]=%d\n", i, ip->di_rdaddr[i]);
	printf("di_rindirect=%d\n", ip->di_rindirect);
}

/*
 * get_total_ino() sets imax equal to the last allocated inode in 
 * the filesystem and sets fs_size equal to the number of 512
 * byte blocks in the filesystem.
 */
void
get_total_ino()
{
	fdaddr_t fmax;
	struct superblock sb;

	/* Read & validate super block */
	read_superblk(&sb);

	/* set global file size (512 byte blocks) */
	fs_size = sb.s_fsize;

	/* Get maximum number of inodes */
	fsmax(&imax, &fmax);
	if (imax ==0 || fmax.f.addr == 0) {
		fprintf(stderr, MSGSTR (BADFSIZE,
			"super block has corrupted filesystem size\n"));
		exit(1);
	}
	imax--;

        fscompress = sb.s_compress;
        if (valid_algorithm(fscompress) != 0)
        {
                fprintf(stderr, MSGSTR(BADCOMPRESS,
                        "unknown compression algorithm\n"));
                exit(1);
        }
}


/*
 *
 * NAME:	
 *	read_superblk
 *
 * FUNCTION:	
 *	Read the super block using the libfs routine, get_super(),
 *	and process the return codes to validate the filesystem.
 *
 * RETURN:	
 *	If able to read super block then return otherwise exit with
 *	a non-zero value and an appropriate error message if bad filesystem 
 *	or unable to read the superblock.
 *
 */
void
read_superblk(struct superblock *sb)
{

	switch (get_super(fi, sb)) 
	{
		case LIBFS_SUCCESS:
		return;
		
		case LIBFS_BADMAGIC:
		fprintf(stderr, MSGSTR (NOTJFS,
			"%s is not recognized as a JFS filesystem.\n"),disk);
		break;

		case LIBFS_BADVERSION:
		fprintf(stderr, MSGSTR (NOTSUPJFS,
			"%s is not a supported JFS filesystem version.\n"),
			disk);
		break;

		case LIBFS_CORRUPTSUPER:
		fprintf(stderr, MSGSTR (CORRUPTJFS,
			"The %s JFS filesystem superblock is corrupted.\n"),
			disk);
		break;

		default:
		fprintf(stderr, MSGSTR (CANTRESB,
			"Cannot read superblock on %s.\n"),disk);
		break;
	}
	exit(1);
}

pass(fn, map)
	register int (*fn)();
	register char *map;
{
	unsigned char bits = 0;
	struct dinode dp;

	for (ino = 0; ino < imax; ) {
		if ((ino % NBBY) == 0) {
			bits = ~0;
			if (map != NULL)
				bits = *map++;
		}
		ino++;
		if ((bits & 1) && (ino > LAST_RSVD_I || ino == ROOTDIR_I)) {
			ginode(ino, &dp);
			(*fn)(&dp);
		}
		bits >>= 1;
	}
}

void
ginode(inum, dp)
ino_t inum;
struct dinode *dp;
{
	if (get_inode(fi, dp, inum) < 0) {
		fprintf(stderr, MSGSTR (CANTGINO,
			"Cannot get block for inode %d\n"),inum);
		bzero(dp, sizeof(struct dinode));
	}
}

/*
 * Block map.  Given page offset ie block # bn, map that to a real 
 * block # in the disk inode given in dp and return actual block number
 */
static int
bmap(dp, bn, return_blk)
struct dinode	*dp;		/* Disk inode to read		*/
daddr_t		 bn;		/* Page offset in file		*/
daddr_t		*return_blk;
{
        daddr_t nb;                     /* number of blocks             */
        daddr_t indblk[DADDRPERBLK];

        /*
         * get number of blocks in file & check against requested block number
         */
        if ((nb = howmany(dp->di_size, FSBSIZE)) <= bn)
                return (-1);

        /*
         * No indirect blocks
         */
        if (NOINDIRECT(nb))
        {
                *return_blk = dp->di_rdaddr[bn];
                return (0);
        }

        /*
         * Get the single or double indirect block
         */

        blkread(dp->di_rindirect * (FragSize / DEV_BSIZE), (char *)indblk, FSBSIZE);

        if (ISDOUBLEIND(nb))
        {
                struct idblock *iddp;
                int didndx;

                didndx = DIDNDX(bn);
                iddp = (struct idblock *) indblk + didndx;

                /*
                 * Overlay block with single indirect table
                 */

                blkread(iddp->id_raddr * (FragSize / DEV_BSIZE), (char *)indblk, FSBSIZE);

                /*
                 * Fall through to common code.
                 */
        }

        *return_blk = indblk[SIDNDX(bn)];

        return (0);
}


mark(ip)
	struct dinode *ip;
{
	register int f;
	extern int anydskipped;

	f = ip->di_mode & IFMT;
	if ((f == 0) || (ip->di_nlink == 0) || (f == IFSOCK))
		return;
	BIS(ino, clrmap);
	if (f == IFDIR) 
		BIS(ino, dirmap);
	if ((ip->di_mtime >= spcl.c_ddate || ip->di_ctime >= spcl.c_ddate) &&
	    !BIT(ino, nodmap)) {
		BIS(ino, nodmap);
		if (f != IFREG && f != IFDIR && f != IFLNK) {
			esize += 1;
			return;
		}
		est(ip);
	} else if (f == IFDIR)
		anydskipped = 1;
}

add(ip)
	register struct	dinode	*ip;
{
	register int i;
	long filesize;
	int nblks;

	if(BIT(ino, nodmap))
		return;
	nsubdir = 0;
	dadded = 0;
	filesize = ip->di_size;
	nblks = btoc(filesize);
	if (NOINDIRECT(nblks)) {
		for (i = 0; i < NDADDR; i++) {
			if (ip->di_rdaddr[i] != 0)
				dsrch(ip->di_rdaddr[i], FSBSIZE, filesize);
			filesize -= FSBSIZE;
		}
	} else {
		if (ip->di_rindirect != 0) {
			if (ISDOUBLEIND(nblks))
				i = 1;
			else i = 0;
			indir(ip->di_rindirect, i, &filesize);
		}
	}
	if(dadded) {
		nadded++;
		if (!BIT(ino, nodmap)) {
			BIS(ino, nodmap);
			est(ip);
		}
	}
	if(nsubdir == 0)
		if(!BIT(ino, nodmap))
			BIC(ino, dirmap);
}

indir(d, n, filesize)
	daddr_t d;
	int n, *filesize;
{
	register i;
	daddr_t	idblk[FSBSIZE];

	blkread(d * (FragSize / DEV_BSIZE), (char *)idblk, FSBSIZE);
	if(n <= 0) {
		for(i=0; i < NINDIRECT; i++) {
			d = idblk[i];
			if(d != 0)
				dsrch(d, FSBSIZE, *filesize);
			*filesize -= FSBSIZE;
		}
	} else {	/* we are looking at double indirect blocks */
		n--;
		for(i=0; i < NINDIRECT/2; i++) {
			d = idblk[i*2 + 1];	/* second long word has the disk blk */
			if(d != 0)
				indir(d, n, filesize);
		}
	}
}

dirdump(ip)
	struct dinode *ip;
{
	/* watchout for dir inodes deleted and maybe reallocated */
	if ((ip->di_mode & IFMT) != IFDIR)
		return;
	dump(ip);
}

dump(ip)
	struct dinode *ip;
{
	register int i, j;
	long size;
	long aclsize;
	long pclsize;

	int nblks;
	char *symlnk;
	struct acl *aclbuf;
	struct pcl *pclbuf;

#ifndef RDUMP
	last_ino_dumped = ino;
#endif
	if(newtape) {
		newtape = 0;
		bitmap(nodmap, TS_BITS);
	}
	BIC(ino, nodmap);
	copydinode(ip);
	spcl.xix_dinode = *ip;
	spcl.c_type = TS_INODE;
	spcl.c_count = 0;
	i = ip->di_mode & IFMT;

	if ((ip->di_nlink == 0) || (i == IFSOCK)) /* free inode or socket */
		return;

	if (i == IFLNK && ip->di_size <= D_PRIVATE) {
		spcl.c_count = 1;
		spcl.c_addr[0] = 1;
		spclrec();
		if ((symlnk = malloc(TP_BSIZE)) == NULL)
			dumpabort();
		memcpy(symlnk, &ip->di_rdaddr[0], ip->di_size);
		*(symlnk + ip->di_size) = '\0';
		taprec(symlnk, FALSE);
		free(symlnk);
		return;
	}

	if ((i != IFDIR && i != IFREG &&  i != IFLNK) || ip->di_size == 0) {
		spclrec();
#ifndef RDUMP
		if (i != IFLNK) {
			aclbuf = getacl(ino);
			aclsize= aclbuf->acl_len;
			if (aclsize)
				putacl(aclbuf, aclsize);
			free(aclbuf);

			pclbuf = getpcl(ino);
			pclsize= pclbuf->pcl_len;
			if (pclsize)
				putpcl(pclbuf, pclsize);
			free(pclbuf);
		}
#endif
		return;
	}

        /* set compressed for use in dmpblk() */
        compressed = iscompressed(ip);

	nblks = btoc(ip->di_size);
	if (NOINDIRECT(nblks)) {
		j = howmany(ip->di_size, FSBSIZE);
		blksout(&ip->di_rdaddr[0], j);
#ifndef RDUMP
		if (i != IFLNK) {
			aclbuf = getacl(ino);
			aclsize= aclbuf->acl_len;
			if (aclsize)
				putacl(aclbuf, aclsize);
			free(aclbuf);

			pclbuf = getpcl(ino);
			pclsize= pclbuf->pcl_len;
			if (pclsize)
				putpcl(pclbuf, pclsize);
			free(pclbuf);
		}
#endif
		return;
	}
	if (ip->di_rindirect != 0) {
		size = ip->di_size;
		if (ISDOUBLEIND(nblks))
			j = 1;
		else j = 0;
		dmpindir(ip->di_rindirect, j, &size);
#ifndef RDUMP
		if (i != IFLNK) {
			aclbuf = getacl(ino);
			aclsize= aclbuf->acl_len;
			if (aclsize)
				putacl(aclbuf, aclsize);
			free(aclbuf);

			pclbuf = getpcl(ino);
			pclsize= pclbuf->pcl_len;
			if (pclsize)
				putpcl(pclbuf, pclsize);
			free(pclbuf);
		}
#endif
		if (size <= 0)
			return;
	}
}

dmpindir(blk, lvl, size)
	daddr_t blk;
	int lvl;
	long *size;
{
	int i, cnt;
	daddr_t idblk[FSBSIZE];

	if (blk != 0)
		blkread(blk * (FragSize / DEV_BSIZE), (char *)idblk, FSBSIZE);
	else
		bzero(idblk, FSBSIZE);
	if (lvl <= 0) {
		if (*size < NINDIRECT * FSBSIZE)
			cnt = howmany(*size, FSBSIZE);
		else
			cnt = NINDIRECT;
		*size -= NINDIRECT * FSBSIZE;
		blksout(&idblk[0], cnt);
		return;
	}
	lvl--;
	for (i = 0; i < NINDIRECT/2; i++) {
		dmpindir(idblk[i*2 + 1], lvl, size);
		if (*size <= 0)
			return;
	}
}

blksout(blkp, blocks)
	daddr_t *blkp;
	int blocks;
{
	int i,j,count; 
	int blks;	/* the number of 1k tape blocks in the file */
	int tbperdb;	/* the number of 1k tape blocks in a 4k disk block */

	blks = howmany(blocks * FSBSIZE, TP_BSIZE);
	tbperdb = FSBSIZE / TP_BSIZE;
	for (i = 0; i < blks; i += TP_NINDIR) {
		if (i + TP_NINDIR > blks)
			count = blks;
		else
			count = i + TP_NINDIR;
		for (j = i; j < count; j++)
				if (blkp[j / tbperdb] != 0)
					spcl.c_addr[j - i] = 1;
				else
					spcl.c_addr[j - i] = 0;
		spcl.c_count = count - i;
		spclrec();
		for (j = i; j < count; j += tbperdb)
			if (blkp[j / tbperdb] != 0)
				if (j + tbperdb <= count)
					dmpblk(blkp[j / tbperdb],
					    FSBSIZE);
				else
					dmpblk(blkp[j / tbperdb],
					    (count - j) * TP_BSIZE);
		spcl.c_type = TS_ADDR;
	}
}

bitmap(map, typ)
	char *map;
{
	register i;
	char *cp;

	spcl.c_type = typ;
	spcl.c_count = howmany(msiz * sizeof(map[0]), TP_BSIZE);
	spclrec();
	for (i = 0, cp = map; i < spcl.c_count; i++, cp += TP_BSIZE)
		taprec(cp, FALSE);
}

spclrec()
{
	register int s, i, *ip;

	spcl.c_inumber = ino;
	spcl.c_magic = NFS_MAGIC;
	spcl.xix_flag = XIX_MAGIC;
	spcl.c_checksum = 0;
	ip = (int *)&spcl;
	s = 0;
	i = sizeof(union u_spcl) / (4*sizeof(int));
	while (--i >= 0) {
		s += *ip++; s += *ip++;
		s += *ip++; s += *ip++;
	}
	spcl.c_checksum = CHECKSUM - s;
	taprec((char *)&spcl, TRUE);
}

dsrch(d, size, filesize)
	daddr_t d;
	int size, filesize;
{
	register struct direct *dp;
	long loc;
	char dblk[FSBSIZE];
	fdaddr_t blkaddr;

	if(dadded)
		return;
	if (filesize > size)
		filesize = size;
	blkaddr.d = d;
	d = blkaddr.f.addr;

	blkread(d * (FragSize / DEV_BSIZE), dblk, filesize);
	for (loc = 0; loc < filesize; ) {
		dp = (struct direct *)(dblk + loc);
		if (dp->d_reclen == 0) {
			msg(MSGSTR(CORDIR, "corrupted directory, inumber %d\n"), ino);
			break;
		}
		loc += dp->d_reclen;
		if(dp->d_ino == 0)
			continue;
		if(dp->d_name[0] == '.') {
			if(dp->d_name[1] == '\0')
				continue;
			if(dp->d_name[1] == '.' && dp->d_name[2] == '\0')
				continue;
		}
		if(BIT(dp->d_ino, nodmap)) {
			dadded++;
			return;
		}
		if(BIT(dp->d_ino, dirmap))
			nsubdir++;
	}
}

int	breaderrors = 0;		
int	io_error = 0;		
#define	BREADEMAX 32

blkread(da, ba, cnt)
daddr_t da;
char *ba;
int cnt;
{
	int n;

	if (da >= fs_size) {
		bzero(ba, cnt);
		return 0;
	}
	if (llseek(fi, (offset_t)da * DEV_BSIZE, 0) < 0) {
		msg(MSGSTR(LSEEKF, "lseek failed\n"));
	}
	n = read(fi, ba, cnt);
	if (n == cnt)
		return 0;
	if (da + (n / DEV_BSIZE) == fs_size) {
		bzero(ba + n, cnt - n);
		return 0;
	}
	
	io_error++;
	msg(MSGSTR(IMPOSS, 
		"bread from %s [block %d]: count=%d, got=%d\n\t"),disk,da,cnt,n);
	if (++breaderrors > BREADEMAX){
		msg(MSGSTR(BREADERR, 
			"More than %d block read errors from %s\n"), 
				BREADEMAX, disk);
		msg(MSGSTR(UNRECERR, "This is an unrecoverable error.\n"));
		dumpabort();
	}
	return(-1);
}

copydinode(ip)
struct dinode *ip;
{
	spcl.bsd_c_dinode.bsd_di_mode = ip->di_mode;
	spcl.bsd_c_dinode.bsd_di_nlink = ip->di_nlink;
	spcl.bsd_c_dinode.bsd_di_uid = ip->di_uid;
	spcl.bsd_c_dinode.bsd_di_gid = ip->di_gid;
	spcl.bsd_c_dinode.bsd_di_size = ip->di_size;
	if ( ip->di_mode & (IFCHR|IFBLK))
		spcl.bsd_c_dinode.bsd_di_rdev = ip->di_rdev;
	else
		spcl.bsd_c_dinode.bsd_di_rdev = ip->di_rdaddr[0];
	spcl.bsd_c_dinode.bsd_di_atime = ip->di_atime;
	spcl.bsd_c_dinode.bsd_di_mtime = ip->di_mtime;
	spcl.bsd_c_dinode.bsd_di_ctime = ip->di_ctime;
}

putacl(aclbuf, size)
struct acl *aclbuf;
size_t size;
{
	int i;

	spcl.c_type = TS_ACL;
	spcl.c_count = howmany(size, TP_BSIZE);
	spclrec();
	dmpsec(aclbuf, spcl.c_count, TS_ACL);

}

putpcl(pclbuf, size)
struct pcl *pclbuf;
size_t size;
{
	int i;

	spcl.c_type = TS_PCL;
	spcl.c_count = howmany(size, TP_BSIZE);
	spclrec();
	dmpsec(pclbuf, spcl.c_count, TS_PCL);

}

char acltbuf[FSBSIZE];

struct acl *
getacl(ino)
ino_t ino;
{
	static struct dinode *inoext = NULL; /* Ptr to INODEX_I  (ACL data) */
	struct dinode dp, *ip;
	int blkoffset;
	struct acl *aclbuf, *aclp;
	static struct acl aclhead;
	daddr_t aclblkp; /* ACL block pointer */
	char *cp;
	long acl_bcnt;


	if (!inoext)
	{
		/* Retrieve inode extensions inode */
		ginode(INODEX_I, &dp);
		ip = &dp;

		if((inoext = (struct dinode *)malloc(sizeof(struct dinode))) == NULL) 
		{
			msg(MSGSTR(EEXACL, "Export of ACLs failed\n"));
			exit(1);
		}
		bcopy(ip, inoext, sizeof(struct dinode));
	}


	ginode(ino, &dp);
	ip = &dp;

	if ((ip->di_acl & ~ACL_INCORE) == 0)
	{
		aclhead.acl_len = 0;
		return (&aclhead);
	} else {
		/* Fetch block where acl is found */
		bmap(inoext, btoct(ip->di_acl), &aclblkp);
		blkread(aclblkp * (FragSize / DEV_BSIZE), acltbuf, FSBSIZE);

		/* offset into block where info begins */
		blkoffset = ip->di_acl % FSBSIZE;
		aclbuf = (struct acl *)(acltbuf + blkoffset);

		acl_bcnt = FSBSIZE - blkoffset;

		if( aclbuf->acl_len > acl_bcnt )
		{
			/* acl straddles more than one block */

			int j, k;

			j = howmany(aclbuf->acl_len, FSBSIZE);
			if((aclp = (struct acl *)malloc(j * FSBSIZE)) == NULL)
			{
				msg(MSGSTR(EEXACL, "Export of ACLs failed\n"));
				exit(1);
			}
			cp = (char *)aclp;
			bcopy(cp, (char *)(acltbuf + blkoffset), acl_bcnt);
			cp += acl_bcnt;
			acl_bcnt += ip->di_acl;
			k = howmany(aclbuf->acl_len + blkoffset, FSBSIZE);
			for(; --k; cp += FSBSIZE, acl_bcnt += FSBSIZE)
			{
				bmap(inoext, btoct(acl_bcnt), &aclblkp);
				blkread(aclblkp * (FragSize / DEV_BSIZE), acltbuf, FSBSIZE);
				bcopy(cp, (char *)acltbuf, FSBSIZE);
			}
		}
		return(aclbuf);
	} 
}

char pcltbuf[FSBSIZE];

struct pcl *
getpcl(ino)
ino_t ino;
{
	static struct dinode *inoext = NULL; /* Ptr to INODEX_I  (PCL data) */
	struct dinode dp, *ip;
	int blkoffset;
	struct pcl *pclbuf, *pclp;
	static struct pcl pclhead;
	daddr_t pclblkp; /* PCL block pointer */
	char *cp;
	long pcl_bcnt;


	if (!inoext)
	{
		/* Retrieve inode extensions inode */
		ginode(INODEX_I, &dp);
		ip = &dp;

		if((inoext = (struct dinode *)malloc(sizeof(struct dinode))) == NULL) 
		{
			msg(MSGSTR(EEXPCL, "Export of PCLs failed\n"));
			exit(1);
		}
		bcopy(ip, inoext, sizeof(struct dinode));
	}

	ginode(ino, &dp);
	ip = &dp;

	if (((ip->di_privflags & PCL_EXTENDED) == 0) && ((ip->di_priv.pv_priv[0] != 0) || (ip->di_priv.pv_priv[1] != 0)))
	{
		pclhead.pcl_len = PCL_SIZ;
		pclhead.pcl_mode = ip->di_mode;
		pclhead.pcl_default = ip->di_priv;
		return (&pclhead);
	}

	if (((ip->di_privflags & PCL_EXTENDED) == 0) || (ip->di_privoffset ==0))
	{
		pclhead.pcl_len = 0;
		return (&pclhead);
	} else {
		/* Fetch block where pcl is found */
		bmap(inoext, btoct(ip->di_privoffset), &pclblkp);
		blkread(pclblkp * (FragSize / DEV_BSIZE), pcltbuf, FSBSIZE);

		/* offset into block where info begins */
		blkoffset = ip->di_privoffset % FSBSIZE;
		pclbuf = (struct pcl *)(pcltbuf + blkoffset);

		pcl_bcnt = FSBSIZE - blkoffset;

		if( pclbuf->pcl_len > pcl_bcnt )
		{
			/* pcl straddles more than one block */

			int j, k;

			j = howmany(pclbuf->pcl_len, FSBSIZE);
			if((pclp = (struct pcl *)malloc(j * FSBSIZE)) == NULL)
			{
				msg(MSGSTR(EEXPCL, "Export of PCLs failed\n"));
				exit(1);
			}
			cp = (char *)pclp;
			bcopy(cp, (char *)(pcltbuf + blkoffset), pcl_bcnt);
			cp += pcl_bcnt;
			pcl_bcnt += ip->di_privoffset;
			k = howmany(pclbuf->pcl_len + blkoffset, FSBSIZE);
			for(; --k; cp += FSBSIZE, pcl_bcnt += FSBSIZE)
			{
				bmap(inoext, btoct(pcl_bcnt), &pclblkp);
				blkread(pclblkp * (FragSize / DEV_BSIZE), pcltbuf, FSBSIZE);
				bcopy(cp, (char *)pcltbuf, FSBSIZE);
			}
		}
		return(pclbuf);
	} 
}

/* returns 0 if ip is not compressed; otherwise fscompress.
 * ip is assumed to not refer to a reserved inode (see pass()).
 */
int
iscompressed(ip)
struct dinode *ip;
{
        /* if journalled it isn't compressed. */
        if (ip->di_mode & IFJOURNAL)
                return 0;
        return fscompress;
}

/* read and decompress data, putting result into buffer.
 */
int
cblkread(da, ba, cnt, origin, length)
daddr_t da;  /* uncompressed disk address */
char *ba;    /* addr of tape buffer */
int cnt;     /* length of uncompressed data */
daddr_t origin; /* compressed disk address */
int length;     /* length of compressed data */
{
        char mybuf[PAGESIZE], *bptr, readbuf[PAGESIZE];

        /* read into our buffer */
        blkread(origin, readbuf, length);

        /* decompress into tape buffer or our buffer according to cnt */
        bptr = (cnt < PAGESIZE) ? mybuf : ba;

        /* if decode fails clear tape buffer */
        if (PAGESIZE != decompress(fscompress, readbuf, bptr, length))
        {
                msg(MSGSTR(UNCOMPRESS, "Uncompress failed \n"));
                memset(ba, 0, cnt);
                return 0;
        }

        /* did we uncompress into tape buffer ? */
        if (cnt == PAGESIZE)
                return 0;

        /* copy portion of mybuf to tape buffer corresponding
         * to da and cnt.
         */
        if (cnt < PAGESIZE)
        {
                /* copy part of mybuf to tape buffer */
                memcpy(ba, mybuf + ((da - origin)*DEV_BSIZE), cnt);
                return 0;
        }

        /* cnt should not be bigger than PAGESIZE unless there were
         * duplicate blkno's passed to dmpblk() (i.e. nonsense data).
         * if so clear buffer past PAGESIZE.
         */
        memset(ba + PAGESIZE, 0, cnt - PAGESIZE);
        return 0;
}


