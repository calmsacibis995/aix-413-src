/* @(#)67	1.1  src/nfs/usr/sbin/yp/dbm.h, cmdnfs, nfs411, GOLD410 4/24/90 15:22:52 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 26 
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	(#)dbm.h	4.1 (Berkeley) 6/27/83";
 */

# ifndef PBLKSIZ
# define PBLKSIZ	1024		/* data file block size */
# endif
# ifndef DBLKSIZ
# define DBLKSIZ	2048		/* directory file block size */
# endif

#define	BYTESIZ		8

# ifndef NULL
# define NULL		0
# endif

long	bitno;
long	maxbno;
long	blkno;
long	hmask;

char	pagbuf[PBLKSIZ];
char	dirbuf[DBLKSIZ];

int	dirf;
int	pagf;
int	dbrdonly;

typedef struct {
	char	*dptr;
	int	dsize;
} datum;

datum	fetch();
datum	makdatum();
datum	firstkey();
datum	nextkey();
datum	firsthash();
long	calchash();
long	hashinc();
