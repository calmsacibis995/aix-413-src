/* static char sccsid[] = "@(#)13	1.7  src/bos/usr/sbin/lsfs/lsfs.h, cmdfs, bos411, 9428A410j 1/6/94 15:58:58"; */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: crfs, lsfs, chfs, rmfs, imfs
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 /*
  *  Includes, defines, and globals for lsfs.c and jfs.c
  */
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <sys/vfs.h>
#include <sys/vmount.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/limits.h>
#include <fshelp.h>
#include <IN/standard.h>
#include <IN/FSdefs.h>
#include <IN/AFdefs.h>
#include <IN/CSdefs.h>
#include <math.h>
#include <sys/statfs.h>
#include <jfs/filsys.h>
#include <jfs/log.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <ctype.h>
#include <sys/types.h>
#include <locale.h>
#include <nl_types.h>
#include "lsfs_msg.h"
#include <libfs/libfs.h>

/*
 *	If this is not the main module (lsfs.c) then
 *	most all variables and and function prototypes are
 *	externs. Define a macro to add the "extern"
 *	keyword if needed. The first statement in
 *	the main module must be to define IN_LSFS.C
 */

#ifdef IN_LSFS.C

#define _EXTERN_

#else

#define _EXTERN_	extern

#endif

/*
 * getopt stuff (extern to all modules)
 */
extern int optind;
extern char *optarg;

/*
 * external functions (external to all modules)
 */
extern char *FSdskname();

/*
 * Message cat. jnk
 */
_EXTERN_	nl_catd 	catd;
#define MSGSTR(Num,Str) catgets(catd,MS_LSFS,Num,Str)

/*
 * various macros
 */
#define EQ(a,b) (!strcmp(a,b))			/* compare two strings */
#define NELEM(a) (sizeof(a)/sizeof(a[0]))	/* # elements in an array */
#undef 	MAXREC 
#define MAXREC 500			/* max # of records to retrieve */ 
#define MAXATTR 100			/* max # of attribs per record */
#define FILESYSTEMS "/etc/filesystems"	/* file to operate on */
#define LINE 80
#define MATCH_MOUNTPT 1
#define MATCH_DEVICE 2
#define MATCH_EITHER (MATCH_MOUNTPT | MATCH_DEVICE)

#ifndef MNT_JFS
#define	MNT_JFS		MNT_AIX3
#endif

#define NILPTR(type)	((type *) 0)
#define	NULLPTR		NILPTR(char)

/*
 * biggest attr lv will store
 */
#define	LVATTRLEN	127	

/*
 * Max length of options passed to mkfs	currently calculated to be:
 * strlen ("-o nbpi=16384,frag=4096") => 24
 */
#define OPT_LEN		64	/* 24 + large fudge factor */

/*
 * Length of argv[0] - the program name
 */
#define PROG_NAME_LEN 1024
	
/*
 * an attribute in a stanza
 */
struct attribute
{
	char		*name;
	char		*value;
	int		 flag;	/* flags for various routines (see below) */
};
 
/*
 * flags for attribute.flag
 */
#define	MARKIT		01

/*
 * a list of attributes
 */
struct attrlist
{
	struct attribute	*attr;
	int			 count;
};

/*
 * stanzas for & from /etc/filesystems
 */
struct fs_stanza
{
	struct fs_stanza	*next;		/* next stanza in list	*/
	char			*name;		/* stanza name		*/
	struct attrlist		 list;		/* list of attributes	*/
};

/*
 * stanzas read from /etc/filesystems
 */
struct fs_stanza *stanzalist;

/*
 * lv type for jfs log device
 */
#define	Logtype	"jfslog"

/*
 * What type an lvm id or name is (currently for
 * process_lvs but also for future use I guess)
 */
typedef enum {IS_PV, IS_VG} lvmid_t;

/*
 * Changable/configurable filesystem options
 */
typedef enum {LV_SIZE, LOG_NAME, FRAG_SIZE, NBPI, COMP, OTHER} fsattr_t;

#define	SIZE_str	"size"
#define LOG_str		"logname"
#define FRAG_str	"frag"
#define NBPI_str	"nbpi"
#define COMPRESS_str	"compress"

/*
 * Information unique to the creation of a jfs
 */
struct	jfs_attrs
{
	long  	size;
	int  	fragsize;
	int  	nbpi;
	int  	logsiz;
	char  	*lvsize;
	char  	*logdev;
	char  	*vgrp;
	char	*compress;
};

_EXTERN_	int nfs;
_EXTERN_	int rflg;

/*
 * default logical partition to user block size factor (initialized in main)
 */
_EXTERN_	long LPTOUB;
_EXTERN_	char *progname;	

/*
 * debugging (initilaized in main)
 */
_EXTERN_	int Debugflag;
#define	DPRINTF	if (Debugflag) printf

/*
 * do a perror but include the program name...
 */
#define	PERROR(file)	fprintf(stderr, "%s: ", progname), perror(file)
#define	PERROR2(file, other) \
	fprintf(stderr, "%s: %s: ", progname, other), perror(file)
/*
 * Prototypes
 */
_EXTERN_	int		makedirectory (char*);
_EXTERN_	void		make_file (char*);
_EXTERN_	int		scanopt (char*);
_EXTERN_	void		repl_opt (char*, char*);
_EXTERN_	char 		*process_lvs (char*, int(*func)(char*),
					      int*, lvmid_t);
_EXTERN_	char 		*querylog (char*);
_EXTERN_	char 		*getvgrp (char*);
_EXTERN_	char 		*getlvfld (char*, char*);
_EXTERN_	int		change_stanza (struct fs_stanza*, char*);
_EXTERN_	void		remove_stanza (char*);
_EXTERN_	int		isblank (char*);
_EXTERN_	void		copy_file (FILE*, FILE*, char*);
_EXTERN_	int		match (char*, char*);
_EXTERN_	void		append_stanza (struct fs_stanza*);
_EXTERN_	int		jfstype(char*);
_EXTERN_	char 		*getlvsiz (char*);
_EXTERN_	char 		*getlvid (char*);
_EXTERN_	void		lock_file (FILE*);
_EXTERN_	struct fs_stanza *stanza_lookup (char*, int);
_EXTERN_	char 		*do_list (char*, char*, int len);
_EXTERN_	AFILE_t		open_stanzafile (void);
_EXTERN_	int		extendfs (char*, char*, char*, char*, int);
_EXTERN_	char		*get_dev (char*);
_EXTERN_	int		ismounted (char*, int*, char*, int*);
_EXTERN_	char 		*getvfsname (char*);
_EXTERN_	char 		*strsave(register char*);
_EXTERN_	int		getvfstype (char*);
_EXTERN_	char  		*getjfsname (void);
_EXTERN_	size_t		getfssize (char*, char*, int);
_EXTERN_	fsattr_t	parse_attr (char*, char**, int);
_EXTERN_	int		run_cmd (char*, ...);
_EXTERN_	char 		*getdevname (char*);
_EXTERN_	char 		*makedevname (char*);
_EXTERN_	void		save_attribute (char*, char*, 
						struct attrlist*);
_EXTERN_	struct attribute *find_attribute (char*,  struct attrlist*);
_EXTERN_	void 		append_attribute(char*, char*,
						 struct attrlist*);
_EXTERN_	int		contains_attribute (struct fs_stanza*,
						    char*, char*);
_EXTERN_	char 		*strcatsave(char*, char*);
_EXTERN_	void		do_changeables (struct fs_stanza*, char*,
						char*, char*, char*, int);
_EXTERN_	mode_t		file_type (char*);
_EXTERN_	int		isa_jfslog (char*);
_EXTERN_	int		isa_jfs(char *dev);
_EXTERN_	char 		*read_cmd (char*, ...);
_EXTERN_	char 		*getlvlabel (char*);
_EXTERN_	int		putlvlabel (char*, char*);
_EXTERN_	char 		*getlvattrs (char*);
_EXTERN_	int		putlv (struct fs_stanza*);
_EXTERN_	struct attrlist lvattrs_to_attrlist (char*);
_EXTERN_	char 		*attrlist_to_lvattrs (struct attrlist*);
_EXTERN_	void		free_attrlist (struct attrlist);
_EXTERN_	char 		*absolute_and_canonize (char*);
_EXTERN_	int 		verify_jfsopt (fsattr_t, char*, int*);
_EXTERN_	int		being_used (char*);
_EXTERN_	char		*mklv (char*, long, char* , ...);


/*
 * Macro to sace attributes:
 *	don't save defaults:
 *		vfs if it's jfs
 *		check if it's false
 *		mount if it's false
 *		device
 *		size (never!)
 */

#define	SAVEATTR()							\
	if ((!EQ(a->name, Vfs_attr) || !EQ(a->value, "jfs")) &&		\
	    (!EQ(a->name, Check_attr) || !EQ(a->value, "false")) &&	\
	    (!EQ(a->name, Mount_attr) || !EQ(a->value, "false")) &&	\
	     !EQ(a->name, Dev_attr) &&					\
	     !EQ(a->name, Size_attr)) {					\
		sprintf(buf, "%s%s=%s", lvattrs[0] ? Attrsep : "",	\
				a->name, a->value);			\
		if (strlen(lvattrs) + strlen(buf) + 1 <= sizeof(lvattrs)) \
			strcat(lvattrs, buf);				\
			}



