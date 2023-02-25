static char sccsid[] = "@(#)08  1.14.1.2  src/bos/usr/sbin/crash/inode.c, cmdcrash, bos411, 9438B411a 9/20/94 11:57:25";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prinode,prhinode
 *
 * ORIGINS: 27,3
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
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include	<sys/inode.h>
#include	<sys/sysmacros.h>
#include	"crash.h"

prhinode(iblocks, args)
	register int iblocks;
	char **args;
{
	int maj, mindev, ino;
	int i, nhino, hinode; 
	int found = FALSE;
	dev_t dev;
	long hip;
	struct inode ibuf,*ip;

	
	if (args) {
		maj = atoi(args[0]);
		mindev = atoi(args[1]);
		ino = atoi(args[2]);
		dev = makedev(maj, mindev);
	}
	
	readmem(&nhino, (long)SYM_VALUE(Nhino), sizeof(nhino));
	readmem(&hinode, (long)SYM_VALUE(Hinode), sizeof(struct hinode *));
	for (i = 0; i < nhino && !found; i++) {
		hip = hinode + (i * sizeof(struct hinode));
		if(readmem(&ibuf, hip, sizeof(struct hinode), CURPROC) 
		   != sizeof(struct hinode)) {
			printf( catgets(scmc_catd, MS_crsh, INODE_MSG_2, "0452-087: Cannot read inode hash table entry %d.\n") ,i);
			return;
		}
		while ((long)ibuf.i_forw != hip) {
			ip = ibuf.i_forw;
			if(readmem(&ibuf, ibuf.i_forw, sizeof(struct inode),
			   CURPROC) != sizeof(struct inode)) {
				printf( catgets(scmc_catd, MS_crsh, INODE_MSG_3, "0452-088: Cannot read inode at address 0x%8x.\n") ,ibuf.i_forw);
				return;
			}
			if (!args || 
			    (ibuf.i_dev == dev && ibuf.i_number == ino)) {
				prinode(&ibuf,ip,iblocks);
				if (args) {
					found = TRUE;
					break;
				}
			}
		}
	}
}


prinode(ibuf,ip, md)
	register struct inode *ibuf;
	register struct inode *ip;
	register int	md;
{
	register char	ch;
	register int	i;
	daddr_t		daddr;

	printf("0x%08x %3.3d %4.4d %5u %3d %4d %4d %4d %6ld", ip,
		bmajor(ibuf->i_dev), minor(ibuf->i_dev), ibuf->i_number,
		ibuf->i_count,
		ibuf->i_nlink, ibuf->i_uid, ibuf->i_gid, ibuf->i_size);
	switch(ibuf->i_mode & IFMT) {
	case IFDIR: ch = 'd'; break;
	case IFCHR: ch = 'c'; break;
	case IFBLK: ch = 'b'; break;
	case IFREG: ch = 'f'; break;
	case IFIFO: ch = 'p'; break;
	default:    ch = '-'; break;
	}
	printf(" %c", ch);
	printf("%s%s%s%3o",
		ibuf->i_mode & ISUID ? "u" : "-",
		ibuf->i_mode & ISGID ? "g" : "-",
		ibuf->i_mode & ISVTX ? "v" : "-",
		ibuf->i_mode & 0777);
	printf(((ibuf->i_mode & IFMT) == IFCHR) ||
		((ibuf->i_mode & IFMT) == IFBLK) ||
		((ibuf->i_mode & IFMT) == IFBLK) ?
		" %4.4o %4.4o" : "    -    -",
		major(ibuf->i_rdev), minor(ibuf->i_rdev));

		if (ibuf->i_locks & IXLOCK ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_5, " lck" ) );
		if (ibuf->i_locks & IXWANT ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_6, " wnt" ) );
		if (ibuf->i_flag & IACC ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_8, " acc" ) );
		if (ibuf->i_flag & ICHG ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_9, " chg" ) );
		if (ibuf->i_flag & IUPD ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_10, " upd" ) );
		if (ibuf->i_flag & IFSYNC ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_11, " fsyn" ) );
		if (ibuf->i_flag & IDEFER ) 	printf(  catgets(scmc_catd, MS_crsh, INODE_MSG_13, " def" ) );
		printf("\n");

	if(md == TRUE && (((ibuf->i_mode & IFMT) == IFREG) ||
		((ibuf->i_mode & IFMT) == IFDIR))) {
		printf("     addr:");
			for(i = 0; i < NDADDR; i++)
				printf(" %3ld", ibuf->i_rdaddr[i]);
		printf("\n");
	}
}
