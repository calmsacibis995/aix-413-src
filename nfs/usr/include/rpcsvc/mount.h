/* @(#)87	1.9.1.4  src/nfs/usr/include/rpcsvc/mount.h, cmdnfs, nfs411, GOLD410 1/17/94 14:31:16  */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: svc_getcaller
 *
 *   ORIGINS: 24,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1988,1990 by Sun Microsystems, Inc.
 */


#ifndef _H_rpcsvc_mount
#define _H_rpcsvc_mount

#define MOUNTPROG 100005
#define MOUNTPROC_MNT		1
#define MOUNTPROC_DUMP		2
#define MOUNTPROC_UMNT		3
#define MOUNTPROC_UMNTALL	4
#define MOUNTPROC_EXPORT	5
#define MOUNTPROC_EXPORTALL	6
#define MOUNTPROC_PATHCONF	7
#define MOUNTVERS		1
#define MOUNTVERS_POSIX		2

#ifndef svc_getcaller
#define svc_getcaller(x) (&(x)->xp_raddr)
#endif


bool_t xdr_fhstatus();
#ifndef KERNEL
bool_t xdr_path();
bool_t xdr_fhandle();
bool_t xdr_mountlist();
bool_t xdr_exports();
bool_t xdr_pathcnf();
#endif /* ndef KERNEL */

struct mountlist {		/* what is mounted */
	char *ml_name;
	char *ml_path;
	struct mountlist *ml_nxt;
};

struct fhstatus {
	int fhs_status;
	fhandle_t fhs_fh;
};

/*
 * List of exported directories
 * An export entry with ex_groups
 * NULL indicates an entry which is exported to the world.
 */
struct exports {
	dev_t		  ex_dev;	/* dev of directory */
	char		 *ex_name;	/* name of directory */
	struct groups	 *ex_groups;	/* groups allowed to mount this entry */
	struct exports	 *ex_next;
	short		  ex_rootmap;	/* id to map root requests to */
	short		  ex_flags;	/* bits to mask off file mode */
};

struct groups {
	char		*g_name;
	struct groups	*g_next;
};

struct nfs_args {
	struct sockaddr_in	addr;		/* file server address */
	fhandle_t		fh;		/* File handle to be mounted */
	int			flags;		/* flags */
	int			wsize;		/* write size in bytes */
	int			rsize;		/* read size in bytes */
	int			timeo;		/* initial timeout in .1 secs */
	int			retrans;	/* times to retry send */
	char			*hostname;	/* server's hostname */
	int			acregmin;	/* attr cache file min secs */
	int			acregmax;	/* attr cache file max secs */
	int			acdirmin;	/* attr cache dir min secs */
	int			acdirmax;	/* attr cache dir max secs */
	char			*netname;	/* server's netname */
	struct pathcnf		*pathconf;	/* static pathconf kludge */
        int 			biods;		/* number of BIODS */
};

/*
 * NFS mount option flags
 */
#define MNTOPT_ACDIRMAX "acdirmax" /* max ac timeout for dirs (sec) */
#define MNTOPT_ACDIRMIN "acdirmin" /* min ac timeout for dirs (sec) */
#define MNTOPT_ACREGMAX "acregmax" /* max ac timeout for reg files (sec) */
#define MNTOPT_ACREGMIN "acregmin" /* min ac timeout for reg files (sec) */
#define MNTOPT_ACTIMEO  "actimeo" /* attr cache timeout (sec) */
#define MNTOPT_BG       "bg" /* mount op in background if 1st attempt fails */
#define MNTOPT_FG       "fg" /* mount op in fg if 1st attempt fails, default */
#define MNTOPT_GRPID    "grpid" /* SysV-compatible group-id on create */
#define MNTOPT_HARD     "hard"  /* hard mount (default) */
#define MNTOPT_INTR     "intr"  /* allow interrupts on hard mount */
#define MNTOPT_NOAC     "noac"  /* don't cache file attributes */
#define MNTOPT_NOACL	"noacl"  /* don't read acl's from server - default */
#define MNTOPT_ACL	"acl"  /* read acl's from server means we load ksec */
#define MNTOPT_NOAUTO   "noauto"/* hide entry from mount -a */
#define MNTOPT_NOCTO    "nocto" /* no "close to open" attr consistency */
#define MNTOPT_NODEV    "nodev" /*don't allow opening of devices accross mount*/
#define MNTOPT_NOQUOTA  "noquota" /* don't check quotas */
#define MNTOPT_NOSUID   "nosuid" /* no set uid allowed */
#define MNTOPT_BSY	"bsy"
#define MNTOPT_PORT     "port"  /* server IP port number */
#define MNTOPT_POSIX    "posix" /* ask for static pathconf values from mountd */
#define MNTOPT_QUOTA    "quota" /* quotas */
#define MNTOPT_RETRANS  "retrans" /* set number of request retries */
#define MNTOPT_RETRYS   "retry" /* # of times mount is attempted, def=10000*/
#define MNTOPT_RMNT     "remount" /* remount to rw if mode ro */
#define MNTOPT_RO       "ro"    /* read only */
#define MNTOPT_RSIZE    "rsize" /* set read size (bytes) */
#define MNTOPT_RW       "rw"    /* read/write */
#define MNTOPT_SECURE   "secure"/* use secure RPC for NFS */
#define MNTOPT_SHORTDEV "shortdev" /* server dev */
#define MNTOPT_SOFT     "soft"  /* soft mount */
#define MNTOPT_TIMEO    "timeo" /* set initial timeout (1/10 sec) */
#define MNTOPT_WSIZE    "wsize" /* set write size (bytes) */


#define NFSMNT_SOFT	0x001	/* soft mount (hard is default) */
#define NFSMNT_WSIZE	0x002	/* set write size */
#define NFSMNT_RSIZE	0x004	/* set read size */
#define NFSMNT_TIMEO	0x008	/* set initial timeout */
#define NFSMNT_RETRANS	0x010	/* set number of request retrys */
#define NFSMNT_HOSTNAME	0x020	/* set hostname for error printf */
#define NFSMNT_INT	0x040	/* allow interrupts on hard mount */
#define	NFSMNT_NOAC	0x080	/* don't cache attributes */
#define	NFSMNT_ACREGMIN	0x0100	/* set min secs for file attr cache */
#define	NFSMNT_ACREGMAX	0x0200	/* set max secs for file attr cache */
#define	NFSMNT_ACDIRMIN	0x0400	/* set min secs for dir attr cache */
#define	NFSMNT_ACDIRMAX	0x0800	/* set max secs for dir attr cache */
#define NFSMNT_SECURE	0x1000	/* secure mount */
#define NFSMNT_NOCTO	0x2000	/* no close-to-open consistency */
#define	NFSMNT_POSIX	0x4000	/* static pathconf kludge info */
#define NFSMNT_NOACL	0x8000	/* turn off acl support */
#define NFSMNT_SHORTDEV	0x10000	/* server does not support 32-bit device no. */
#define NFSMNT_BIODS    0x20000 /* Number of biods for the file system */
#define NFSMNT_GRPID    0x40000 /* SysV-compatible group-id on create */
#define NFSMNT_ACL	0x80000	/* turn on acl support */

#define DEF_BIODS       6

#endif /*_H_rpcsvc_mount*/
