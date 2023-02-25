/* @(#)22	1.1  src/bos/usr/sbin/quota/fstab.h, cmdquota, bos411, 9428A410j 2/15/91 13:16:24 */
#ifndef _H_FSTAB
#define _H_FSTAB 
/*
 * COMPONENT_NAME: 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>

/*
 * File system (filesystems) 
 *
 * The fs_spec field is the block special name.  Programs
 * that want to use the character special name must create
 * that name by prepending a 'r' after the right most slash.
 * Quota files are always named "quotas", so if type is "rq",
 * then use concatenation of fs_file and "quotas" to locate
 * quota file.
 */

#define	FSTAB_RW	"rw"	/* read/write device */
#define	FSTAB_RQ	"rq"	/* read/write with quotas */
#define	FSTAB_RO	"ro"	/* read-only device */
#define	FSTAB_SW	"sw"	/* swap device */
#define	FSTAB_XX	"xx"	/* ignore totally */

struct	fstab{
	char	*fs_spec;		/* block special device name */
	char	*fs_file;		/* file system path prefix */
	char	*fs_type;		/* read/write, etc see above defines */
	int	fs_check;		/* true=0, false=-1, else "check" val */
	int	fs_freq;		/* not used */
	int	fs_passno;		/* not used */
        char	*fs_vfstype;
	char	*fs_mntops;
};

extern  struct	fstab *getfsent();
extern  struct	fstab *getfsspec();
extern  struct	fstab *getfsfile();
extern  struct	fstab *getfstype();
extern  int	setfsent();
extern  int	endfsent();

#endif /* _H_FSTAB */
