/* @(#)98	1.2  src/nfs/usr/sbin/automount/mntent.h, cmdnfs, nfs411, GOLD410 2/3/94 12:47:33 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */
/* static char sccsid[] = 	"(#)auto_main.c	1.3 88/07/27 4.0NFSSRC Copyr 1988 Sun Micro"; */


/*	@(#)mntent.h	1.5 90/07/23 4.1NFSSRC SMI;	from SMI 1.13 99/01/06	*/

/*
 * File system table, see mntent (5)
 *
 * Used by dump, mount, umount, swapon, fsck, df, ...
 *
 * Quota files are always named "quotas", so if type is "rq",
 * then use concatenation of mnt_dir and "quotas" to locate
 * quota file.
 */

#ifndef _mntent_h
#define _mntent_h

#define	MNTTAB		"/etc/fstab"
#define	MOUNTED		"/etc/mtab"

#define	MNTMAXSTR	128

#define	MNTTYPE_43	"4.3"	/* 4.3 file system */
#define	MNTTYPE_42	"4.2"	/* 4.2 file system */
#define	MNTTYPE_NFS	"nfs"	/* network file system */
#define	MNTTYPE_PC	"pc"	/* IBM PC (MSDOS) file system */
#define	MNTTYPE_SWAP	"swap"	/* swap file system */
#define	MNTTYPE_IGNORE	"ignore"/* No type specified, ignore this entry */
#define MNTTYPE_LO      "lo"    /* Loop back File system */

/*  mount options  */
#define	MNTOPT_RO	"ro"	/* read only */
#define	MNTOPT_RW	"rw"	/* read/write */
#define MNTOPT_GRPID 	"grpid"	/* SysV-compatible group-id on create */
#define MNTOPT_REMOUNT	"remount"/* change options on previous mount */
#define	MNTOPT_NOAUTO	"noauto"/* hide entry from mount -a */
#define MNTOPT_NOSUB	"nosub"  /* disallow mounts beneath this one */

/*  4.2 specific options  */
#define	MNTOPT_QUOTA	"quota"	/* quotas */
#define	MNTOPT_NOQUOTA	"noquota"/* no quotas */

/*  NFS specific options  */
#define	MNTOPT_SOFT	"soft"	/* soft mount */
#define	MNTOPT_HARD	"hard"	/* hard mount (default) */
#define	MNTOPT_NOSUID	"nosuid"/* no set uid allowed */
#define	MNTOPT_INTR	"intr"	/* allow interrupts on hard mount */
#define MNTOPT_SECURE 	"secure"/* use secure RPC for NFS */
#define MNTOPT_NOAC 	"noac"	/* don't cache file attributes */
#define MNTOPT_NOCTO 	"nocto"	/* no "close to open" attr consistency */
#define MNTOPT_PORT	"port"	/* server IP port number */
#define MNTOPT_RETRANS 	"retrans" /* set number of request retries */
#define MNTOPT_RSIZE 	"rsize" /* set read size (bytes) */
#define MNTOPT_WSIZE 	"wsize" /* set write size (bytes) */
#define MNTOPT_TIMEO 	"timeo"	/* set initial timeout (1/10 sec) */
#define MNTOPT_ACTIMEO 	"actimeo" /* attr cache timeout (sec) */
#define MNTOPT_ACREGMIN "acregmin" /* min ac timeout for reg files (sec) */
#define MNTOPT_ACREGMAX "acregmax" /* max ac timeout for reg files (sec) */
#define MNTOPT_ACDIRMIN "acdirmin" /* min ac timeout for dirs (sec) */
#define MNTOPT_ACDIRMAX "acdirmax" /* max ac timeout for dirs (sec) */
#define	MNTOPT_POSIX	"posix"	/* ask for static pathconf values from mountd */

/* Information about the mount entry */
#define MNTINFO_DEV	"dev"	/* device number of the mounted file system */

struct	mntent {
	char	*mnt_fsname;		/* name of mounted file system */
	char	*mnt_dir;		/* file system path prefix */
	char	*mnt_type;		/* MNTTYPE_* */
	char	*mnt_opts;		/* MNTOPT* */
	int	mnt_freq;		/* dump frequency, in days */
	int	mnt_passno;		/* pass number on parallel fsck */
};

struct	mntent *getmntent();
char	*hasmntopt();
FILE	*setmntent();
int	endmntent();

#endif /*!_mntent_h*/
