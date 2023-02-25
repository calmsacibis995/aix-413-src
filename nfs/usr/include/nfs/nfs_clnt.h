/* @(#)98	1.8.3.5  src/nfs/usr/include/nfs/nfs_clnt.h, sysxnfs, nfs411, GOLD410 6/15/94 15:51:19 */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: CACHE_VALID
 *		INVAL_ATTRCACHE
 *		PURGE_ATTRCACHE
 *		PURGE_STALE_FH
 *		vftomi
 *		vtoblksz
 *		vtomi
 *		
 *
 *   ORIGINS: 24,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*      @(#)nfs_clnt.h  1.6 91/03/08 NFSSRC4.1 from 2.32 90/07/31 SMI   */


#ifndef __NFS_CLNT_HEADER__
#define __NFS_CLNT_HEADER__

/*
 * vfs pointer to mount info
 */
#define	vftomi(vfsp)	((struct mntinfo *)((vfsp)->vfs_data))

/*
 * vnode pointer to mount info
 */
#define	vtomi(vp)	((struct mntinfo *)(((vp)->v_vfsp)->vfs_data))

/*
 * NFS vnode to server's block size
 */
#define	vtoblksz(vp)	(vtomi(vp)->mi_bsize)


#define	HOSTNAMESZ	32
#define	ACREGMIN	3	/* min secs to hold cached file attr */
#define	ACREGMAX	60	/* max secs to hold cached file attr */
#define	ACDIRMIN	30	/* min secs to hold cached dir attr */
#define	ACDIRMAX	60	/* max secs to hold cached dir attr */
#define ACMINMAX        3600    /* 1 hr is longest min timeout */
#define ACMAXMAX        36000   /* 10 hr is longest max timeout */

#define	NFS_CALLTYPES	3	/* Lookups, Reads, Writes */

/*
 * Fake errno passed back from rfscall to indicate transfer size adjustment
 */
#define	ENFS_TRYAGAIN	999

/*
 * NFS private data per mounted file system
 */
struct mntinfo {
	struct sockaddr_in mi_addr;	/* server's address */
	struct vnode	*mi_rootvp;	/* root vnode */
	u_int		 mi_hard : 1;	/* hard or soft mount */
	u_int		 mi_int : 1;	/* interrupts allowed on hard mount */
	u_int		 mi_noac : 1;	/* don't cache attributes */
	u_int		 mi_nocto : 1;	/* no close-to-open consistency */
	u_int		 mi_dynamic : 1; /* dynamic transfer size adjustment */
	u_int		 mi_sec_na : 1;	/* Security is not available */
	u_int		 mi_shortdev : 1; /*32bit device numbers not supported*/
	u_int		 mi_grpid : 1;	/* use grpid of directory if true */
	u_int		 mi_printed;	/* not responding console msg printed */
	u_int		 mi_down;	/* server is down */
	u_int		 mi_usertold;	/* not responding user msg printed */

	int		 mi_refct;	/* active vnodes for this vfs */
	long		 mi_tsize;	/* transfer size (bytes) */
					/* really read size */
	long		 mi_stsize;	/* server's max transfer size (bytes) */
					/* really write size */
	long		 mi_bsize;	/* server's disk block size */
	int		 mi_mntno;	/* kludge to set client rdev for stat*/
	int		 mi_timeo;	/* inital timeout in 10th sec */
	int		 mi_retrans;	/* times to retry request */
	char		 mi_hostname[HOSTNAMESZ];	/* server's hostname */
	char		*mi_netname;	/* server's netname */
	int		 mi_netnamelen;	/* length of netname */
	int		 mi_authflavor;	/* authentication type */
	u_int		 mi_acregmin;	/* min secs to hold cached file attr */
	u_int		 mi_acregmax;	/* max secs to hold cached file attr */
	u_int		 mi_acdirmin;	/* min secs to hold cached dir attr */
	u_int		 mi_acdirmax;	/* max secs to hold cached dir attr */
	int		*mi_biods;	/* max number of biods on this fs */
	/*
	 * Extra fields for congestion control, one per NFS call type,
	 * plus one global one.
	 */
	struct rpc_timers mi_timers[NFS_CALLTYPES+1];
	long		mi_curread;	/* current read size */
	long		mi_curwrite;	/* current write size */
	long		mi_lastmod;	/* last time rw size modified (hz) */
	struct pathcnf *mi_pathconf;	/* static pathconf kludge */
};

/*
 * Mark cached attributes as timed out
 */
#define	PURGE_ATTRCACHE(vp)	\
{ \
	  curtime(&vtor(vp)->r_attrtime); \
	  vtor(vp)->r_attrtime.tv_usec /= NS_PER_uS; \
	  vtor(vp)->r_flags |= (RACLINVALID | RPCLINVALID); \
}

/*
 * Mark cached attributes as uninitialized (must purge all caches first)
 */
#define	INVAL_ATTRCACHE(vp)	\
{ \
	  vtor(vp)->r_attrtime.tv_sec = 0; \
	  vtor(vp)->r_flags |= (RACLINVALID | RPCLINVALID); \
}



/*
 * If returned error is ESTALE flush all caches.
 */
#define PURGE_STALE_FH(errno, vp) if ((errno) == ESTALE) {btrash(vp); nfs_purge_caches(vp);}

/*
 * Is cache valid?
 * Swap is always valid, if no attributes (attrtime == 0) or
 * if mtime matches cached mtime it is valid
 * If server thinks that filesize has changed then caches aren't valid.
 */
#define	CACHE_VALID(rp, mtime, fsize) \
	((rtov(rp)->v_flag & VISSWAP) == VISSWAP || \
	    (rp)->r_attrtime.tv_sec == 0 || \
	    (((mtime).tv_sec == (rp)->r_attr.va_mtime.tv_sec && \
	    (mtime).tv_usec == (rp)->r_attr.va_mtime.tv_nsec) && \
	    ((fsize) == (rp)->r_attr.va_size)))

/*
 * Stole this macro from rpc/clnt_kudp.c to calculate current time in hz
 */
#define	time_in_hz(time)	(time.tv_sec*hz + time.tv_usec/(1000000/hz))

#define SQUISH_DEV(devno) (((devno & 0x00ff0000) >> 8) | (devno & 0x000000ff))
#define EXPAND_DEV(devno) (((devno & 0x0000ff00) << 8) | (devno & 0x000000ff))
#define MUNGDEVMAJOR(devno)	(major(devno) | 0x00008000)
#define MUNGDEVMINOR(devno)	(minor(devno))
#define UNMUNGEDEV(major,minor) (((major & 0x00007fff) << 16) | (minor))
#define IS_BIG_DEV(devno) (devno & 0xff80ff00)
#define IS_NFSDEV(na_type)	((na_type) & (VCHR | VBLK | VMPC))
#define IS_MUNGED_DEV(devno) (devno & 0x00008000)

#define CLIENTHANDLES	4	/* number of client handles to cache at client */
#define AUTHTABLESIZEDES	8	/* size of des authentication table */
#define AUTHTABLESIZEUNIX	8	/* size of unix authentication table */

#ifdef _KERNEL

extern	Complex_lock	nfs_client_lock;

#define	NFS_CLIENT_LOCK()	lock_write(&nfs_client_lock)
#define NFS_CLIENT_UNLOCK()	lock_done(&nfs_client_lock)
#endif

#endif !__NFS_CLNT_HEADER__
