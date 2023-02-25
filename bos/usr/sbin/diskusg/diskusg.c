static char sccsid [] = "@(#)61 1.15.1.8  src/bos/usr/sbin/diskusg/diskusg.c, cmdfs, bos411, 9428A410j 1/10/94 17:56:31";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: diskusg
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1992
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <jfs/ino.h>
#include <jfs/filsys.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <locale.h>
#include <nl_types.h>
#include "diskusg_msg.h"
#include <libfs/libfs.h>


#define  MSGSTR(Num,Str) catgets (catd, MS_DISKUSG, Num,Str)
#define  BLOCK		512	/* Block size for reporting 		*/
#define	 MAXIGN		10	/* max # of fs's to ignore 		*/
#define	 UNUSED		-1	/* flag for uid field in hash table 	*/
#define	 FAIL		-1	/* failure return code 			*/
#define	 MAXNAME	100	/* length of a user name 		*/
#define	 SUCCEED	0	/* success return code 			*/
#define	 TRUE		1	/* truth 				*/
#define	 FALSE		0	/* false 				*/
#define	 PFILE		"/etc/passwd"   /* system passwd file 		*/

#define  DEF_USAGE "Usage:\tdiskusg [ -U NumberUsers ] [  -i  FileListName ] [  -p File ]\n\t[  -u File |  -v ] { -s [ File ... ] | FileSystem ... }\n"

int  	VERBOSE = 0;		/* option flag 				*/
int 	MAXUSERS = 5000; 	/* default # of users 			*/
int  	etc_passwd = FALSE;  	/* flag for /etc/passwd being used or not */
FILE  	*ufd;			/* FILE * for inode/user output data 	*/
int  	index;			/* handy global variable 		*/
nl_catd catd;

/*
 * Passwd file junk
 */
static FILE  *pwf = NULL;
static char  line[BUFSIZ+1];
static struct passwd  passwd;

/* 
 * struct acct holds the diskusg information for a specific user.
 * the array userlist is malloc'ed with MAXUSERS elements in hashinit().
 */
struct acct
{
	int	uid;			/* user id 			*/
	long	usage;			/* # of UBSIZE byte blocks used */
	char	name [MAXNAME+1];	/* user name (null terminated) 	*/
} *userlist;

char	*ignlist[MAXIGN];	/* list of filesystems to ignore	*/
int	igncnt = 0;		/* # of filesystems in ignore list 	*/


static int	adduser	(FILE*);
static int	ilist (char*, int);
static int	ignore (char*);
static int	output (void);
static unsigned	hash (unsigned);
static int	hashinit (void);
static int	setup (char*);
static int	todo (char*);
static char	*skip (char*);
static int 	setpwent2 (char*);
static char 	*pwskip (char*);
struct passwd 	*getpwent2 (void);
static void	endpwent2 (void);
static int	count (struct dinode*, int);
static void	process_super (int, struct superblock*, char*);

/* 
 * since the passwd(3) functions don't allow the specification of an
 * alternate password file, we have our own password file processor.
 * this would be unportable if we ever thought the username and uid fields 
 * would ever be anyplace other than where they are. 
 */

/*
 * Defect 29769:
 * we will use these functions whenever passwd file is other than
 * /etc/passwd however if it is /etc/passwd we will use passwd(3)
 * functions to take care of YP stuff for us automatically
 * have renamed function to function2, so that I can use passwd(3)
 * functions from the program
 */


/*
 *
 * main (argc,argv) -- entry point: process arguments, main loop.
 *
 */
int
main (int argc,
      char **argv)
{
	extern int  optind;	/* for getopt(3)	*/
	extern char  *optarg;	/* for getopt(3) 	*/
	register  c;		/* return from getopt() 		*/
	register FILE  *fd;	/* for processing formatted input files */
	register  rfd;		/* for processing disks 		*/
	struct stat  sb;	/* to get info about a disk 		*/
	int  sflg = FALSE;	/* take-input-from-preformatted-file flag*/
	char *pfile = PFILE;    /* default password file	*/
	int  errfl = FALSE;	/* error-flag flag 		*/

    
	(void) setlocale (LC_ALL,"");
	catd = catopen (MF_DISKUSG, NL_CAT_LOCALE);

	ufd = 0;				/* initially null ufd */

	/*
	 * process arguments loop 
	 */
	while ((c = getopt(argc, argv, "vU:u:p:si:")) != EOF)
	{
		switch(c)
		{
			/*
			 * take input from pre-formatted
			 * file or stdio
			 */
			case 's': 
			sflg = TRUE;
			break;
			 /* 
			  * verbose mode to stderr
			  */
			 case 'v':
			 VERBOSE = 1;
			 break;
			 /*
			  * ignore certain file systems
			  */
			 case 'i':	
			 ignore (optarg);
			 break;
			 /*
			  * output user/inode data to a file
			  */
			 case 'u':	
			 ufd = fopen(optarg, "a");
			 break;
			 /*
			  * use optarg for a password file
			  */
			 case 'p':	
			 pfile = optarg;
			 break;
			 /*
			  * set max users (UNDOCUMENTED 3.2*)
			  */
			 case 'U':	
			 MAXUSERS = atoi(optarg);
			 if (MAXUSERS <= 0)
			 {
				 fprintf (stderr,
					  MSGSTR (BADUVALUE,
				  "diskusg: Value for -U must be > 0\n"));
				 errfl++;
			 }
			 break;
			 /*
			  * error return from getopt
			  */
			 case '?':
			 default:
			 errfl++;
			 break;
		 }
	}
	/*
	 * if the user gave us a bad argument...
	 */
	if(errfl)
	{
		fprintf (stderr, MSGSTR (USAGE, DEF_USAGE));
		exit(10);
	}

	/*
	 * are we using /etc/passwd as our file ??
	 */
	if (!strcmp (pfile, PFILE))
		etc_passwd = TRUE;
	/*
	 * initialize username hash table
	 */
	hashinit();			
	/*
	 * process input from stdin or formatted file
	 */
	if (sflg == TRUE)
	{		
		if (optind == argc)
		{
			/*
			 * no extra args?  use stdin
			 */
			adduser (stdin);	
		}
		else
		{
			for( ; optind < argc; optind++)
			{
				/*
				 * use extra args for fnames
				 */
				if ((fd = fopen (argv[optind], "r")) == NULL)
				{
					fprintf (stderr,
				 MSGSTR(CANTOPEN,"diskusg: Cannot open %s\n"),
						 argv[optind]);
					continue;
				}
				adduser(fd);
				fclose(fd);
			}
		}
	}
	else
	{
		/*
		 * initialize username list
		 */
		setup(pfile);		
		/*
		  * for each disk named on the command line..
		  */
		for ( ; optind < argc; optind++)
		{
			/*
			 * open file `argv[optind]': we need
			 * to make sure that it's a disk
			 */
			if ((rfd = open (argv[optind], O_RDONLY)) < 0)
			{
				fprintf(stderr,
					MSGSTR(CANTOPEN,
					       "diskusg: Cannot open %s\n"), 
					argv[optind]);
				exit(11);
			}
			/*
			 * stat the disk to get some
			 * info. (file type=disk?)
			 */
			if (stat (argv[optind],	&sb) < 0)
			{
				fprintf (stderr,
					 MSGSTR(STATFAIL,
						"diskusg: stat `%s' fails.\n"),
					 argv[optind]);
				exit(12);
			}
			if ((((sb.st_mode & IFMT) == IFBLK) |
			     ((sb.st_mode&IFMT) == IFCHR)) == 0)
			{
				fprintf (stderr,
					 MSGSTR (NOTBLOCK,
				 "diskusg: `%s' is not a block device.\n"),
					 argv[optind]);
			}
			/*
			 * read in inode list for this device
			 */
			(void) ilist (argv [optind], rfd);
			/*
			 * close disk
			 */
			(void) close(rfd);
		}
	}
	output();
	/*
	 * closing user database
	 */
	endpwent2();  
	exit(0);
}

/*
 *
 * FUNCTION:	adduser
 *
 * 	sum up users' disk usage, reading from a file (fd).
 *
 */

static int
adduser	(register FILE *fd)
{
	int  usrid;
	long  blcks;
	char  login[MAXNAME+10];

	while (fscanf (fd, "%d %s %ld\n", &usrid, login, &blcks) == 3)
	{
		if ((index = hash (usrid)) == FAIL)
			return (FAIL);
		if (userlist[index].uid == UNUSED)
		{
			userlist[index].uid = usrid;
			strncpy (userlist[index].name, login, MAXNAME);
		}
		userlist[index].usage += blcks;
	}
}

/*
 *
 * FUNCTION:
 *	main function to read devices' ilists to glean information 
 * 	about disk usage...
 *
 */

static int
ilist (char  *file,
       register fd)
{
	int  			rc;
	ino_t  			ino, imax;
	fdaddr_t		tot_frags;
	struct dinode  		dip;
	struct superblock  	sblock;

	/*
	 * check to make sure that we were passed a valid fd...
	 */
	if (fd < 0)
		return (FAIL);
	/*
	 * Check for filesystem names to ignore
	 */
	if (! todo (file))
		return (0);
	/*
	 * Read in the superblock and calculate the max number of inodes
	 */
	process_super (fd, &sblock, file);
	fsmax (&imax, &tot_frags);

	for (ino = 0; ino <= imax; ino++)
	{
		if (ino <= LAST_RSVD_I && ino != ROOTDIR_I)
			continue;
		if ((get_inode (fd, &dip, ino)) != 0)
			continue;
		if (dip.di_nlink > 0)
		{
			/*
			 * count the blocks taken up by this inode
			 */
			if (count (&dip, sblock.s_fragsize) == FAIL)
			{
				if (VERBOSE)
					fprintf (stderr, MSGSTR(BADUID,	"BAD UID: file system = %s, inode = %u, uid = %u\n"), file, ino, dip.di_uid);

				if (ufd)
				{
					fprintf (ufd,"%s %u %u\n",
						 file, ino, dip.di_uid);
				}
			}
		}
	}
	return(SUCCEED);
}

/*
 *
 * FUNCTION:	ignore
 *
 */

static int
ignore (register char  *str)
{
	char  *skip ();

	
	for ( ; *str && igncnt < MAXIGN; str = skip(str), igncnt++)
		ignlist[igncnt] = str;

	if(igncnt == MAXIGN)
	{
		fprintf (stderr,
			 MSGSTR(IGLISTOFL,
		"diskusg: Specify no more than 10 filesystems to ignore.\n"));
		exit(12);
	}
}

/*
 *
 * FUNCTION:	count
 *
 */

static int
count (struct dinode  *di,
       int blk_size)
{
	/*
	 * if #links == 0 or mode == 0 return SUCCEED
	 */
	if (di->di_nlink == 0 || di->di_mode == 0)
		return (SUCCEED);

	if ((index = hash (di->di_uid)) == FAIL ||
	    userlist[index].uid == UNUSED)
	{
		return (FAIL);
	}
	/*
	 * calc usage in BLOCK size blocks.
	 */
	userlist[index].usage += (di->di_nblocks * blk_size) / BLOCK; 
	return (SUCCEED);
}

/*
 *
 * FUNCTION:	output
 *
 */

static int
output()
{
	/*
	 * for each user in turn...
	 */
	for (index = 0; index < MAXUSERS ; index++)
	{
		if (userlist[index].uid != UNUSED &&
		    userlist[index].usage != 0 )
		{
			printf("%4u\t%s\t%ld\n",
			       userlist[index].uid,
			       userlist[index].name, userlist[index].usage);
		}
	}
}

/*
 *
 * FUNCTION:	hash
 *
 *	simple circle hash function
 *
 */

static unsigned
hash (register unsigned  j)
{
	register unsigned start;
	register unsigned circle;
	circle = start = j % MAXUSERS;
	do
	{
		if (userlist[circle].uid == j ||
		    userlist[circle].uid == UNUSED)
		{
			return (circle);
		}
		circle = (circle + 1) % MAXUSERS;

	} while (circle != start);
	return (FAIL);
}

/*
 *
 * FUNCTION:	hashinit
 *
 *	initialize hash table
 *
 */

static int
hashinit () 
{
	if ((userlist = (struct acct *)
	     malloc ((size_t)(sizeof(struct acct) * MAXUSERS))) == NULL)
	{
		exit (13);
	}
	for (index = 0; index < MAXUSERS; index++)
	{
		userlist[index].uid = UNUSED;
		userlist[index].usage = 0;
		userlist[index].name[0] = '\0';
	}
}

/*
 *
 * FUNCTION:	setup
 *
 *	read in password file pfile
 *
 */

static int
setup (char  *pfile)
{
	register struct passwd *pw;

	if (! setpwent2 (pfile))
	{
		fprintf (stderr, MSGSTR(CANTOPEN,
					"diskusg: Cannot open %s\n"), pfile);
		exit(14);
	}

	/*
	 * Since setpwent() is type void it could have failed in setpwent2()
	 * and we'd never know it. Thus, before entring the loop try
	 * getpwent2() once; if it succeeds then setpwent() must have
	 * succeeded.
	 */
	if ((pw = (struct passwd *)getpwent2()) == NULL)
	{
		fprintf (stderr, MSGSTR(CANTOPEN,
				"diskusg: Cannot open %s\n"), pfile);
		exit(14);
	}
		
	do 
	{
		if ((index = hash(pw->pw_uid)) == FAIL )
		{
			fprintf(stderr,
				MSGSTR(USERLIM,
	       "diskusg: User limit reached, use -U # flag > %d\n"),MAXUSERS);
			return (FAIL);
		}
		if (userlist[index].uid == UNUSED)
		{
			userlist[index].uid = pw->pw_uid;
			strncpy (userlist[index].name, pw->pw_name, MAXNAME);
		}

	} while ((pw = getpwent2 ()) != NULL);
}

/*
 *
 * FUNCTION:	todo
 *
 *	check whether or not we really have to do this file system
 *
 */

static int
todo (register char  *fname)
{
	register  i;

	for (i = 0; i < igncnt; i++)
	{
		if (strcmp (fname, ignlist[i]) == 0)
			return (FALSE);
	}
	return (TRUE);
}

/*
 *
 * FUNCTION:	skip
 *
 */

static char
*skip (register char  *str)
{
    while (*str)
    {
	    if (*str == ' ' || *str == ',')
	    {
		    *str = '\0';
		    str++;
		    break;
	    }
	    str++;
    }
    return (str);
}

/*
 *
 * FUNCTION:	setpwent2
 *
 */

static int
setpwent2 (register char *pfile)
{ 
	/*
	 * using passwd(3) function for /etc/passwd to 
	 * take care of YP stuff automatically  
         */
	if (etc_passwd)
	{
		setpwent();
		return (1);
	}

	if (pwf == NULL)
		pwf = fopen (pfile, "r");
	else
		rewind (pwf);
	
	return (pwf != NULL);
}

static void
endpwent2 ()
{
	/*
	 * use passwd(3) for /etc/passwd
	 */
	if (etc_passwd)
		endpwent();
	else
	{
		if (pwf != NULL)
		{
			(void) fclose (pwf);
			pwf = NULL;
		}
	}
}

/*
 *
 * FUNCTION:	pwskip
 *
 */

static char
*pwskip (register char *p)
{
	while (*p && *p != ':' && *p != '\n')
		++p;
	
	if (*p == '\n')
		*p = '\0';
	else if (*p)
		*p++ = '\0';
	return (p);
}

/*
 *
 * FUNCTION:	getpwent2
 *
 */

struct passwd
*getpwent2()
{
	register char *p;
	long	x;

	/*
	 * if we are using /etc/passwd then we better use getpwent()
	 * call, as it will automatically take care of the YP Stuff
	 */
	if (etc_passwd)
		return(getpwent());

	if(pwf == NULL)
		return(0);

	p = fgets (line, BUFSIZ, pwf);
	if (p == NULL)
		return(0);
	
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	x = atol(p);	
	passwd.pw_uid = (x < 0 || x > UID_MAX)? (UID_MAX+1): x;
	p = pwskip(p);
	x = atol(p);
	passwd.pw_gid = (x < 0 || x > UID_MAX)? (UID_MAX+1): x;
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	(void) pwskip(p);

	return(&passwd);
}

static void
process_super (int	fd,
	       struct superblock  *sb,
	       char	*fsname)
{
	int	ec;
	
	switch (get_super (fd, sb))
	{
		case LIBFS_SUCCESS:
		return;
				
		case LIBFS_BADMAGIC:
		fprintf(stderr, MSGSTR (NOTJFS,
		"diskusg:  %s is not recognized as a JFS filesystem.\n"),
			fsname);
		ec = 15;
		break;

		case LIBFS_BADVERSION:
		fprintf(stderr, MSGSTR (NOTSUPJFS,
		"diskusg:  %s is not a supported JFS filesystem version.\n"),
			fsname);
		ec = 16;
		break;		

		case LIBFS_CORRUPTSUPER:
		fprintf(stderr, MSGSTR (CORRUPTJFS,
		"diskusg:  The %s JFS filesystem super block is corrupted.\n"),
			fsname);
		ec = 17;
		break;		

		default:
		fprintf(stderr, MSGSTR(CANTRESB, 
			"diskusg:  Cannot read super block on %s.\n"),
			fsname);
		ec = 18;
		break;
	}
	close (fd);
	exit (ec);
}
