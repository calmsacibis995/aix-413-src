/* @(#)99	1.11.3.5  src/nfs/usr/include/nfs/rnode.h, sysxnfs, nfs412, 9444b412 10/28/94 08:02:14 */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: RLOCK
 *		RUNLOCK
 *		rtofh
 *		rtov
 *		vtofh
 *		vtor
 *		
 *
 *   ORIGINS: 24,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	Copyright (C) 1988, Sun Microsystems Inc.
 */

/*      @(#)rnode.h     1.7 91/03/08 NFSSRC4.1 from 1.29 90/07/04 SMI   */


#ifndef _NFS_RNODE_H
#define _NFS_RNODE_H

/*
 * Remote file information structure.
 * The rnode is the "inode" for remote files.  It contains all the
 * information necessary to handle remote file on the client side.
 *
 * Note on file sizes:  we keep two file sizes in the rnode: the size
 * according to the client (r_size) and the size according to the server
 * (r_attr.va_size).   They can differ because we modify r_size during a
 * write system call (nfs_rdwr), before the write request goes over the
 * wire (before the file is actually modified on the server).  If an OTW
 * request occurs before the cached data is written to the server the file
 * size returned from the server (r_attr.va_size) may not match r_size. 
 * r_size is the one we use, in general.  r_attr->va_size is only used to
 * determine whether or not our cached data is valid.
 */
struct rnode {
	struct rnode	*r_freef;	/* free list forward pointer */
	struct rnode	*r_freeb;	/* free list back pointer */
	struct rnode	*r_hash;	/* rnode hash chain */
	struct vnode	r_vnode;	/* vnode for remote file */
	struct gnode	r_gnode;	/* gnode for remote file */
	fhandle_t	r_fh;		/* file handle */
	u_short		r_flags;	/* flags, see below */
	short		r_error;	/* async write error */
	daddr_t		r_lastr;	/* last block read - readdir */
	struct ucred	*r_cred;	/* current credentials */
	struct ucred	*r_altcred;	/* saved credentials */
	struct ucred	*r_unlcred;	/* unlinked credentials */
	char		*r_unlname;	/* unlinked file name */
	struct vnode	*r_unldvp;	/* parent dir of unlinked file */
	off_t		r_size;		/* client's view of file size (long)*/
	struct vattr	r_attr;		/* cached vnode attributes */
	struct timeval	r_attrtime;	/* time attributes become invalid */
	char		*r_sdname;	/* short dev. twin name */
	struct vnode	*r_sdvp;	/* short dev. twin file */
	vmhandle_t	r_vh;		/* Handle for the VM system */
	int		r_sid;		/* Segment ID for VMM */
	struct acl	*r_acl;		/* the access control list */
	u_long		r_aclsz;	/* the size of the acl */
	struct pcl	*r_pcl;		/* the permission control list */
	u_long		r_pclsz;	/* the size of the pcl */
	Complex_lock	r_lock;		/* used for rnode locking */
	int		r_rmevent;	/* rnode has been removed event */
};

#define MAXRNODES	200

/*
 * Flags
 */
#define	RLOCKED		0x01		/* rnode is in use */
#define	RWANT		0x02		/* someone wants a wakeup */
#define	RATTRVALID	0x04		/* Attributes in the rnode are valid */
#define	REOF		0x08		/* EOF encountered on readdir */
#define	RDIRTY		0x10		/* dirty pages from write operation */
#define	RRWVP		0x20		/* rnode is being used for rw op */
#define DOING_CHECK	0x40		/* A biod is doing a check on this */
#define RACLINVALID	0x80		/* Acl for this rnode has become */
					/* invalid because of attr update */
#define RPCLINVALID    0x100		/* Pcl for this rnode has become */
					/* invalid because of attr update */

/*
 * Convert between vnode and rnode
 */
#define	rtov(rp)	(&(rp)->r_vnode)
#define	vtor(vp)	((struct rnode *)((vp)->v_data))
#define	vtofh(vp)	(&(vtor(vp)->r_fh))
#define	rtofh(rp)	(&(rp)->r_fh)

/*
 * Lock and unlock rnodes.
 */
#define RLOCK(rp)	rlock(rp);
#define RUNLOCK(rp)	runlock(rp);

void	rlock(struct rnode *rp);
void	runlock(struct rnode *rp);

#endif /* !_nfs_rnode_h */
