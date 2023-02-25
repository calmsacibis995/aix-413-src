/* @(#)40	1.8  src/nfs/usr/include/nfs/export.h, sysxnfs, nfs411, GOLD410 1/17/94 14:34:50 */
/* 
 * COMPONENT_NAME: (SYSXNFS) Network File System Kernel
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *	Copyright (C) 1988 Sun Microsystems Inc.
 */

/*      (#)export.h	2.1 88/05/20 NFSSRC4.0 from 1.4 88/02/08 SMI      */

/*
 * exported vfs flags.
 */

#ifndef _H_nfs_export
#define _H_nfs_export

#define EX_RDONLY     0x01		/* exported read only */
#define EX_RDMOSTLY   0x02              /* exported read mostly */

#define EXMAXADDRS 256			/* max number in address list */
struct exaddrlist {
	unsigned naddrs;		/* number of addresses */
	struct sockaddr *addrvec;	/* pointer to array of addresses */
};

/*
 * Associated with AUTH_UNIX is an array of internet addresses
 * to check root permission.
 */
#define EXMAXROOTADDRS	256		/* should be config option */
struct unixexport {
	struct exaddrlist rootaddrs;
};

/*
 * Associated with AUTH_DES is a list of network names to check
 * root permission, plus a time window to check for expired
 * credentials.
 */
#define EXMAXROOTNAMES 256	   	/* should be config option */
struct desexport {
	unsigned nnames;
	char **rootnames;
	int window;
};


/*
 * The export information passed to exportfs()
 */
struct export {
	int ex_flags;	/* flags */
	unsigned ex_anon;	/* uid for unauthenticated requests */
	int ex_auth;	/* switch */
	union {
		struct unixexport exunix;	/* case AUTH_UNIX */
		struct desexport exdes;		/* case AUTH_DES */
	} ex_u;
	struct exaddrlist ex_writeaddrs;
};
#define ex_des ex_u.exdes
#define ex_unix ex_u.exunix

#ifdef _KERNEL
/*
 * A node associated with an export entry on the list of exported
 * filesystems.
 */

#define EXI_WANTED      0x1

struct exportinfo {
        int exi_flags;
        int exi_refcnt;
	struct export exi_export;
	fsid_t exi_fsid;
	struct fid *exi_fid;
	struct exportinfo *exi_next;
	int	exi_waiter;
};
extern struct exportinfo *findexport();
extern int exportrele(struct exportinfo *exi);
#endif

#endif /*_H_nfs_export*/
