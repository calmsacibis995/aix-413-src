/* @(#)00	1.20  src/nfs/usr/include/nfs/sunglue.h, sysxnfs, nfs411, GOLD410 3/11/94 11:16:28 */
/* 
 * COMPONENT_NAME: (SYSXNFS) Network File System Kernel
 * 
 * FUNCTIONS: SYSENT, dbtob, freefid, min 
 *
 * ORIGINS: 24 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 Sun Microsystems, Inc.
 */

#ifndef _SUNGLUE_H
#define _SUNGLUE_H
#define VREAD	R_ACC
#define VEXEC	X_ACC
#define VWRITE	W_ACC
#define VSGID	S_ISGID
#define VNOCACHE 0x40

#define min(a, b)	(((a) < (b)) ? (a) : (b))

#define BLKDEV_IOSIZE	BSIZE
#define MAXBSIZE	BSIZE
#define NFS_CLBYTES	BSIZE
#define VROOT		V_ROOT
#define VTEXT		V_TEXT
#define IO_SYNC		FSYNC
#define IO_APPEND	FAPPEND
#define IO_NDELAY	FNDELAY
#define MOUNT_NFS	MNT_NFS
#define v_pdata		v_data
#define v_op		v_gnode->gn_ops
#define VISSWAP         0x8000  /* vnode is part of virtual swap device */

#define VFS_RESERVED1	0x00200000	/* should be defined in vfs.h	*/
#define VFS_RESERVED2	0x00400000	/* should be defined in vfs.h	*/
#define VFS_GRPID	VFS_RESERVED1	/* Old BSD group-id on create */
#define VFS_RDONLY      VFS_READONLY	/* file system mounted rdonly */
#define VFS_REMOUNT     VFS_RESERVED2	/* modify mount options only */

#define	freefid(fidp) kmem_free((caddr_t)(fidp) , sizeof(struct fid) )

struct buf	*getblk(), *geteblk(), *breada(), *bread();

/* globals extracted from suns kernel.h */

char hostname[MAXHOSTNAMELEN];
int  hostnamelen;
char domainname[MAXHOSTNAMELEN];
int  domainnamelen;

/* pulled from suns systm.h */

long dumplo;

/* pulled from suns param.h */

#define	dbtob(db)			/* calculates (db * 512) */ \
	((unsigned)(db) << 9)

#define b_actf	av_forw

/* for nattr_to_vattr */
#define	va_nodeid	va_serialno

/*
 * the following macros are defined here instead of functions in nfs_sun2gn.c
 * in order to allow the client kernext (clnt.ext) load without the server
 * kernext (nfs.ext).
 */
#define VN_HOLD(vp)	(VNOP_HOLD(vp))

#define VN_RELE(vp)	(VNOP_RELE(vp))

#define VOP_ACCESS(vp,mode,cred)	(VNOP_ACCESS(vp,mode,ACC_SELF,cred))

#define VOP_GETATTR(vp,vap,cred)	VNOP_GETATTR(vp,vap,cred)
/*
 * Set vattr structure to a null value.
 * Boy is this machine dependent!
 */
                                       
#define vattr_null(vap)			\
{					\
        register int n;			\
        register char *cp;		\
                                        \
        n = sizeof(struct vattr);	\
        cp = (char *)vap;		\
        while (n--) {			\
                *cp++ = -1;		\
        }				\
}					\

#endif
