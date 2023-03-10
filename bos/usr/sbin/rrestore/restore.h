/* @(#)00	1.11  src/bos/usr/sbin/rrestore/restore.h, cmdarch, bos411, 9428A410j 5/14/91 10:55:26 */
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
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 *	(#)restore.h	5.1 (Berkeley) 5/28/85
 */

#include <stdio.h>
#include <sys/param.h>
#include <jfs/inode.h>
#include <jfs/ino.h>
#include <jfs/filsys.h>
#define  _KERNEL
#include <sys/dir.h>
#undef   _KERNEL
#include <dumprestor.h>

/*
 * Flags
 */
extern int	cvtflag;	/* convert from old to new tape format */
extern int	bflag;		/* set input block size */
extern int	dflag;		/* print out debugging info */
extern int	hflag;		/* restore heirarchies */
extern int	mflag;		/* restore by name instead of inode number */
extern int	vflag;		/* print out actions taken */
extern int	yflag;		/* always try to recover from tape errors */
/*
 * Global variables
 */
extern char	*dumpmap; 	/* map of inodes on this dump tape */
extern char	*clrimap; 	/* map of inodes to be deleted */
extern ino_t	maxino;		/* highest numbered inode in this file system */
extern long	dumpnum;	/* location of the dump on this tape */
extern long	volno;		/* current volume being read */
extern long	ntrec;		/* number of TP_BSIZE records per tape block */
extern time_t	dumptime;	/* time that this dump begins */
extern time_t	dumpdate;	/* time that this dump was made */
extern char	command;	/* opration being performed */
extern FILE	*terminal;	/* file descriptor for the terminal input */

/*
 * Each file in the file system is described by one of these entries
 */
struct entry {
	char	*e_name;		/* the current name of this entry */
	u_char	e_namlen;		/* length of this name */
	char	e_type;			/* type of this entry, see below */
	short	e_flags;		/* status flags, see below */
	ino_t	e_ino;			/* inode number in previous file sys */
	long	e_index;		/* unique index (for dumpped table) */
	struct	entry *e_parent;	/* pointer to parent directory (..) */
	struct	entry *e_sibling;	/* next element in this directory (.) */
	struct	entry *e_links;		/* hard links to this inode */
	struct	entry *e_entries;	/* for directories, their entries */
	struct	entry *e_next;		/* hash chain list */
};

typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	dd_buf[512];
} DIR;

/* types */
#define	LEAF 1			/* non-directory entry */
#define NODE 2			/* directory entry */
#define LINK 4			/* synthesized type, stripped by addentry */
/* flags */
#define EXTRACT		0x0001	/* entry is to be replaced from the tape */
#define NEW		0x0002	/* a new entry to be extracted */
#define KEEP		0x0004	/* entry is not to change */
#define REMOVED		0x0010	/* entry has been removed */
#define TMPNAME		0x0020	/* entry has been given a temporary name */
#define EXISTED		0x0040	/* directory already existed during extract */
/*
 * functions defined on entry structs
 */
extern struct entry *lookupino();
extern struct entry *lookupname();
extern struct entry *lookupparent();
extern struct entry *addentry();
extern char *myname();
extern char *savename();
extern char *gentempname();
extern char *flagvalues();
extern ino_t lowerbnd();
extern ino_t upperbnd();
extern DIR *rst_opendir();
extern struct direct *rst_readdir();
#define NIL ((struct entry *)(0))
/*
 * Constants associated with entry structs
 */
#define HARDLINK	1
#define SYMLINK		2
#define TMPHDR		"RSTTMP"

/*
 * The entry describes the next file available on the tape
 */
struct context {
	char	*name;		/* name of file */
	ino_t	ino;		/* inumber of file */
	struct	bsd_dinode *dip;	/* pointer to inode */
	char	action;		/* action being taken on this file */
} curfile;
/* actions */
#define	USING	1	/* extracting from the tape */
#define	SKIP	2	/* skipping */
#define UNKNOWN 3	/* disposition or starting point is unknown */

/*
 * Other exported routines
 */
extern ino_t psearch();
extern ino_t dirlookup();
extern long listfile();
extern long deletefile();
extern long addfile();
extern long nodeupdates();
extern long verifyfile();
extern char *rindex();
extern char *index();
extern char *mktemp();

#ifdef _NO_PROTO
extern char *strcat();
extern char *strncat();
extern char *strcpy();
extern char *strncpy();
extern char *malloc();
extern char *calloc();
extern char *fgets();
extern char *realloc();
extern long lseek();
#else /* ~ _NO_PROTO */
extern char  *strcat(char *s1, const char *s2);
extern char  *strncat(char *s1, const char *s2, size_t n);
extern char  *strcpy(char *s1, const char *s2);
extern char  *strncpy(char *s1, const char *s2, size_t n);
extern void  *malloc(size_t size);
extern void  *calloc(size_t nmemb, size_t size);
extern char  *fgets(char *s, int n, FILE *stream);
extern void  *realloc(void *ptr, size_t size);
extern off_t lseek(int fildes, off_t offset, int whence);
#endif /* _NO_PROTO */

/*
 * Useful macros
 */
#define	MWORD(m,i) (m[(unsigned)(i-1)/NBBY])
#define	MBIT(i)	(1<<((unsigned)(i-1)%NBBY))
#define	BIS(i,w)	(MWORD(w,i) |=  MBIT(i))
#define	BIC(i,w)	(MWORD(w,i) &= ~MBIT(i))
#define	BIT(i,w)	(MWORD(w,i) & MBIT(i))

#undef DIRSIZ
#define DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

#define dprintf		if (dflag) fprintf
#define vprintf		if (vflag) fprintf

#ifdef GOOD
#undef GOOD
#endif

#define GOOD 1
#define FAIL 0

#include <nl_types.h>
#include "restore2_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_RESTORE,N,S)
nl_catd catd;
