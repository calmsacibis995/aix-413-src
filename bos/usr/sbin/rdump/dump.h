/* @(#)81	1.17  src/bos/usr/sbin/rdump/dump.h, cmdarch, bos411, 9428A410j 4/8/94 12:12:46 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
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
 * (#)dump.h	5.3 (Berkeley) 5/23/86
 */

#define DEV_BSIZE	512
#define FSBSIZE		BSIZE	/* set FSBSIZE to 4096 in jfs/fsparam.h */

/* just temporary defines */
#define howmany(x, y)	(((x)+((y)-1))/(y))
#define roundup(x, y)	((((x)+((y)-1))/(y))*(y))

#define _ILS_MACROS
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <jfs/filsys.h>
#include <jfs/inode.h>
#include <jfs/ino.h>
#include <jfs/fsparam.h>
#define  _KERNEL
#include <sys/dir.h>
#undef   _KERNEL
#include <utmp.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/statfs.h>
#include <sys/vmount.h>
#include <fcntl.h>
#include <fstab.h>
#include <dumprestor.h>
#include <libfs/libfs.h>
#include <ctype.h>

int	msiz;
char	*clrmap;
char	*dirmap;
char	*nodmap;

/*
 *	All calculations done in 0.1" units!
 */

char	*disk;		/* name of the disk file */
char	*tape;		/* name of the tape file */
char	*increm;	/* name of the file containing incremental information*/
char	*temp;		/* name of the file for doing rewrite of increm */
char	lastincno;	/* increment number of previous dump */
char	incno;		/* increment number */
int	uflag;		/* update flag */
int	lflag;		/* blocks limit flag */
int	fi;		/* disk file descriptor */
int	to;		/* tape file descriptor */
int	pipeout;	/* true => output to standard output */
ino_t	ino;		/* current inumber; used globally */
int	nsubdir;
int	newtape;	/* new tape flag */
int	nadded;		/* number of added sub directories */
int	dadded;		/* directory added flag */
int	density;	/* density in 0.1" units */
long	tsize;		/* tape size in 0.1" units */
long	esize;		/* estimated tape size, blocks */
long	asize;		/* number of 0.1" units written on current tape */
int	etapes;		/* estimated number of tapes */

int	maxblock;	/* maximum number of blocks allowed per tape or diskette */
int	blockswritten;	/* number of blocks written on current tape */
int	tapeno;		/* current tape number */
time_t	tstart_writing;	/* when started writing the first tape block */
int	diskette;	/* if media is a diskette */
struct statfs fs_statbuf;	/* buffer for file system stat */
char	buf[FSBSIZE];

char	*prdate();
long	atol();
int	mark();
int	add();
int	dirdump();
int	dump();
int	dsrch();
char	*rawname();

int	interrupt(void);		/* in case operator bangs on console */

#define	HOUR	(60L*60L)
#define	DAY	(24L*HOUR)
#define	YEAR	(365L*DAY)

/*
 *	Exit status codes
 */
#define	X_FINOK		0	/* normal exit */
#define	X_REWRITE	2	/* restart writing from the check point */
#define	X_ABORT		3	/* abort all of dump; don't attempt checkpointing*/
#define X_IOERR		4	/* pass the I/O error status back up to parent */

/*
 *	Default responeses for query()
 */
#define	YES	1
#define NO	0

#define	NINCREM	"/etc/dumpdates"	/*new format incremental info*/
#define	TEMP	"/etc/dtmp"		/*output temp file*/

#define	TAPE	"/dev/rfd0"		/* default tape device */
#define	DISK	"/dev/rhd4"		/* default disk */

struct	fstab	*fstabsearch();	/* search in fs_file and fs_spec */

/*
 *	The contents of the file NINCREM is maintained both on
 *	a linked list, and then (eventually) arrayified.
 *	struct idates is now declared in dumprestor.h
 */

struct	itime{
	struct	idates	it_value;
	struct	itime	*it_next;
};
struct	itime	*ithead;	/* head of the list version */
int	nidates;		/* number of records (might be zero) */
int	idates_in;		/* we have read the increment file */
struct	idates	**idatev;	/* the arrayfied version */
#define	ITITERATE(i, ip) for (i = 0,ip = idatev[0]; i < nidates; i++, ip = idatev[i])

/*
 *	We catch these interrupts
 */
int	sighup(void);
int	sigquit(void);
int	sigill(void);
int	sigtrap(void);
int	sigfpe(void);
int	sigkill(void);
int	sigbus(void);
int	sigsegv(void);
int	sigsys(void);
int	sigalrm(void);
int	sigterm(void);
