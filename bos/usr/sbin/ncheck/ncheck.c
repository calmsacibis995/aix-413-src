static char sccsid[] = "@(#)53	1.9.2.6  src/bos/usr/sbin/ncheck/ncheck.c, cmdfs, bos411, 9428A410j 4/12/94 15:37:13";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: ncheck
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *      obtain file names from reading filesystem
 *
 *	Bogus note retained for historical (hysterical) reasons:
 *
 *      NOTE:  The HSIZE parameter below should be provided at compile
 *             time for all systems with more than 65 KB total address
 *             space.
 *
 *	HSIZE parameter is essentially the number of directory inodes
 *	in the filesystem.
 * 
 *	HSIZE and ISIZE parameters may be set at runtime by setting the
 *	HSIZE and ISIZE environment variables. This program is not compiled
 *	with the big data option thus HSIZE is limited to:
 *		(2^28 - ISIZE * sizeof(ino_t)) / (HSIZE * sizeof(struct htab))
 *	This approximates to a 1016200 directories per filesystem limit.
 *	The default is 50000.
 *
 *
 */

#include <nl_types.h>
#include <locale.h>
#include "ncheck_msg.h"

#define	MSGSTR(N,S)	catgets(catd,MS_NCHECK,N,S)
nl_catd catd;

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
#include <errno.h>
#include <jfs/filsys.h>
#include <jfs/ino.h>
#include <IN/AFdefs.h>
#include <IN/FSdefs.h>
#include <libfs/libfs.h>

#ifdef DEBUG
#define	Dprintf printf
#else
#define Dprintf
#endif


#define DEFAULT_USAGE "Usage:  ncheck [ [-a] [-i InodeNumbers ...] | [-s] ] [FileSystem]\n"


/*
 * Arbitrarily chosen table sizes; these may be altered at runtime by
 * setting environment variables ISIZE and HSIZE
 */
#define HSIZE		50000
int			hsize;
#define	ISIZE	512
int		isize;

ino_t	*ilist;		/* list of inodes specified on cmd line (-i)	*/
int	nxfile;		/* index for ilist 				*/

struct	htab		/* Hash table for directory inodes		*/
{
	ino_t	h_ino;			/* inode number 		*/
	ino_t	h_pino;			/* parent inode number of h_ino	*/
	char	h_name[MAXNAMLEN];	/* name of h_ino 		*/
};
struct htab	*htab;
int		nhent;		/* number hashed		*/


int	aflg 	= 0;	/* cmd line flag -a: include . and .. files	*/
int	iflg 	= 0;	/* cmd line flag -i: list only provided inodes	*/
int	sflg 	= 0;	/* cmd line flag -s: list only special files	*/
int	nerror 	= 0;	/* count of non-fatal errors 	*/
int	fd;		/* file reading from		*/

extern int	errno;

void		checkall (void);
int		check (char*);
void		pass1 (struct dinode*, ino_t);
void		pass2 (struct dinode*, ino_t, int);
void		pass3 (struct dinode*, ino_t, int);
int		dotname (struct direct*);
void		pname (ino_t, int);
int		dirblk_read (struct dinode*, int, char*, ino_t);
struct htab	*lookup (ino_t,	int);
struct dinode 	*ginode (int, ino_t, struct superblock*);
void		read_super (int, struct superblock*, char*);


/*
 * 
 *	MAIN
 *
 */

int
main (int argc, 
      char **argv)
{
	int	i;
	long 	n;
	char	*end;
	char	*ptr;

	(void) setlocale (LC_ALL, "");
	catd = catopen (MF_NCHECK, NL_CAT_LOCALE);

	/*
	 * Get the size of the 2 required tables and malloc them.
	 */
	isize 	= (ptr = getenv("ISIZE")) ? strtol(ptr, &end, 0) : ISIZE;
	hsize 	=  (ptr = getenv("HSIZE")) ? strtol(ptr, &end, 0) : HSIZE;
	
	if ((ilist =
		(ino_t  *) malloc ((isize + 1) * sizeof(ino_t))) == NULL)
	{
		perror (*argv);
		exit (ENOMEM);
	}
	
	if ((htab =
	     (struct htab *) malloc (hsize * sizeof(struct htab))) == NULL)
	{
		perror (*argv);
		exit (ENOMEM);
	}

	while (--argc)
	{
		argv++;
		if (**argv == '-')
		{
			switch ((*argv)[1])
			{
			/*
			 * List . and ..
			 */
			case 'a':
				if (sflg)
				{
					fprintf (stderr,
						 MSGSTR(USAGE, DEFAULT_USAGE));
					exit(1);
				}
				aflg++;
				continue;

			/*
			 * List only the given inodes
			 */
			case 'i':
				if (sflg)
				{
					fprintf (stderr,
						 MSGSTR(USAGE, DEFAULT_USAGE));
					exit (2);
				}
				
				for (i = 0; i < isize; i++)
				{
					n = atol(argv[1]);
					if (n == 0)
						break;
					ilist[i] = n;
					argv++;
					argc--;
				}
				ilist[i] = 0;
				nxfile = i;
				iflg++;
				continue;
			/*
			 * List special and set UID files only
			 */
			case 's':
				if (aflg || iflg)
				{
					fprintf (stderr,
						 MSGSTR(USAGE, DEFAULT_USAGE));
					exit (3);
				}
				sflg++;
				continue;
			
				default:
				fprintf (stderr,
					 MSGSTR (BADFLAG,
					 "ncheck: bad flag %c\n"), (*argv)[1]);
				fprintf (stderr,MSGSTR (USAGE, DEFAULT_USAGE));
				exit (4);
			}
		}
		else 
			break;
	}

	/*
	 * arg list has file names
	 */
	if (argc)
		while (argc-- > 0)
			check (*argv++);
	else
		checkall ();
	
	exit (nerror);
}

/*
 *
 *	NAME:	checkall
 *
 *	FUNCTION:
 *		Use /etc/filesystems to get the filesystems to check
 */

void
checkall (void)
{
	register char *c;
	ATTR_t  a;
	AFILE_t portf;

	/*
	 * open the filesystem information file so we can look things up
	 */
	if ((portf = AFopen(FSYSname,MAXREC,MAXATR)) == NULL)
	{
		printf (MSGSTR (CANTOPEN, "ncheck: cannot open %s\n"), 
			FSYSname );
		exit (5);
	}

	while ((a = AFnxtrec(portf)) != NULL)
	{
		c = AFgetatr( a, "mount" );
		if (c == NULL)
			continue;

		if (strcmp( c, "false" ) == 0)
			continue;

		c = AFgetatr( a, "dev" );

		if (c)
			check( c );
	}

	AFclose(portf);
}

/*
 *
 *	NAME:	check
 *
 *	FUNCTION:
 *		ncheck one filesystem
 *
 */

int
check (char *file)
{
	ino_t 			imax, ino;
	fdaddr_t		fmax;
	struct dinode 		dp;
	struct superblock 	super;

	
	if ((fd = fsopen (file, O_RDONLY)) < 0)
	{
		fprintf (stderr, MSGSTR (CANTOPEN,
					 "ncheck: cannot open %s\n"), file);
		return (++nerror);
	}
	read_super (fd, &super, file);	

	/*
	 * Get the max number of inodes and blocks
	 */	
	fsmax (&imax, &fmax);
	if ((imax == 0) || (fmax.f.addr <= 0))
	{
		fprintf(stderr, MSGSTR (BADFSIZE,
			"super block has corrupted filesystem size\n"));
		exit(6);
	}

	nhent = 0;
	if (sflg)
	{
		nxfile = 0;
		memset (ilist, 0 , sizeof(ino_t) * (isize + 1));
	}
	memset (htab, 0 , sizeof(struct htab) * hsize);

	printf("%s:\n", file);	

	/*
	 * Pass1:
	 *	Save all directory inode numbers
	 */	
	for (ino = 0; ino < imax; ino++)
	{
		if (ino == INODES_I)
			continue;
		if ((get_inode (fd,  &dp, ino )) != LIBFS_SUCCESS)
			continue;
		if (dp.di_nlink > 0)
			pass1 (&dp, ino);
	}

	/*
	 * Pass2:
	 *	Save the name and parent inode number
	 *	of all hashed directory inodes found in pass1.
	 */
	for (ino = 0; ino < imax; ino++)
	{
		if (ino == INODES_I)
			continue;
		if ((get_inode (fd, &dp, ino )) != LIBFS_SUCCESS)
			continue;
		if (dp.di_nlink > 0)
			pass2(&dp, ino, super.s_bsize);
	}

	/*
	 * Pass3:
	 *	Print the path to each allocated inode.
	 */
	for (ino = 0; ino < imax; ino++)
	{
		if (ino == INODES_I)
			continue;
		if ((get_inode (fd, &dp, ino)) != LIBFS_SUCCESS)
			continue;
		if (dp.di_nlink > 0)
			pass3(&dp, ino, super.s_bsize);
	}
}

/*
 *
 *	NAME:	pass1
 *
 *	FUNCTION:
 *		1.)	If -s flag provided:
 *				Store all special and set [U|G]ID inodes
 *				in ilist.
 *
 *		2.)	Always store all directory inodes in the
 *			hash table
 *
 */

void
pass1 (struct dinode *ip,
       ino_t  inum)
{

	if ((ip->di_mode & IFMT) != IFDIR)
	{
		if (sflg != 1)
			return;
		
		if (nxfile >= isize)
		{
			++sflg;
			fprintf (stderr,
				 MSGSTR (TOOSPEC,
			"Too many special files (increase ilist array)\n"));
			return;
		}
		if ((ip->di_mode & IFMT) == IFBLK ||
		    (ip->di_mode & IFMT) == IFCHR ||
		    (ip->di_mode & (ISUID | ISGID)))
		{
			ilist [nxfile++] = inum;
		}
		return;
	}
	/*
	 * Else hash this directory inode number.
	 */
	lookup (inum, 1);

}

/*
 *
 *	NAME:	pass2
 *
 *	FUNCTION:
 *		Process all directory inodes and their contents.
 *
 *		Store dir name and inode number
 *		in htab for all directories hashed in pass1.
 *
 */

void
pass2 (struct dinode 	*ip,
       ino_t		inode_num,
       int  		sb_bsize)
{

	char 		dbuf[BLKSIZE];
	struct direct 	*dp;
	register 	i, j, l;
	int 		numb, frg_len;
	struct htab 	*hp;
	long  		dirsize;

	if ((ip->di_mode & IFMT) != IFDIR)
		return;
	/*
	 * Get the directory size and the number of blocks.
	 */
	dirsize = ip->di_size;
	numb = BYTE2BLK (ip->di_size + sb_bsize - 1);

	/*
	 * go through all the blocks of this inode.
	 */
	for (i = 0; i < numb; i++)
	{
		/*
		 * Read a directory frag and go through all its
		 * directory chunks.
		 */
		frg_len = dirblk_read (ip, i, &dbuf, inode_num);
		
		for (j = 0; j < frg_len / DIRBLKSIZ; j++)
		{
			/*
			 * pick the next  directory chunk
			 */
			dp = (struct direct *) &dbuf[j * DIRBLKSIZ];

			/*
			 * For each entry in the chunk save the
			 * parent inode number and name of all allocated,
			 * directory, non-dot, inodes in this chunk (dp).
			 */
			for (l = 0; l < DIRBLKSIZ; l += dp->d_reclen,
			     dp = (struct direct *) ((char *) dp +
						     dp->d_reclen))
			{
				if (dp->d_ino == 0)
					continue;
				/*
				 * look for the inode - only directory
				 * inodes are hashed.
				 */
				if ((hp = lookup(dp->d_ino, 0)) ==
				    (struct htab*) NULL)
				{
					continue;
				}
				/*
				 * forget . and ..
				 */
				if (dotname(dp))
					continue;

				/*
				 * Save the name and parent ino_t
				 */
				hp->h_pino = inode_num;
				strcpy (hp->h_name, dp->d_name);
			}
			if (dirsize <= ((j+1) * DIRBLKSIZ))
				break;
		}
		dirsize -= frg_len;		
	}
}

/*
 *
 *	NAME:	pass3
 *
 *	FUNCTION:
 *		Print the path to each inode
 *
 */

void
pass3 (struct dinode	*ip,
       ino_t		inode_num,
       int		 sb_bsize)
{
	char 		dbuf [BLKSIZE];
	struct direct 	*dp;
	register 	i,j, k, l;
	int 		numb, frg_len;
	ino_t 		kno;
	long  		dirsize;

	
	if ((ip->di_mode & IFMT) != IFDIR)
		return;

	/*
	 * Get the directory size and its number of blocks
	 */
	dirsize = ip->di_size;
	numb = BYTE2BLK(ip->di_size + sb_bsize - 1);

	/*
	 * go through all the blocks
	 */
	for (i = 0; i < numb; i++)
	{
		/*
		 * Read a directory frag and go through all its
		 * directory chunks.
		 */
		frg_len = dirblk_read(ip, i, dbuf, inode_num);

		for (j = 0; j < frg_len / DIRBLKSIZ; j++)
		{
			/*
			 * pick the next  directory chunk
			 */
			dp = (struct direct *) &dbuf[j * DIRBLKSIZ];

			/*
			 * For each entry in the chunk print the
			 * full path to each allocated inode.
			 */			
			for (l = 0; l < DIRBLKSIZ; l += dp->d_reclen,
			     dp = (struct direct *) ((char *) dp +
						     dp->d_reclen))
			{
				if (dp->d_ino == 0)
					continue;
				kno = dp->d_ino;

				/*
				 * Skip '.' and '..' if -a not used
				 */
				if (aflg == 0 && dotname(dp))
					continue;
				/*
				 * Skip special files if -i or -s
				 * not provided
				 */
				if (ilist[0] == 0 && sflg == 0)
					goto pr;
				/*
				 * Else see if the current inode (kno)
				 * is in ilist - if so print it
				 */
				for (k = 0; ilist[k] != 0; k++)
				{
					if (ilist[k] == kno)
						goto pr;
				}
				continue;
			
			pr:
				/*
				 * print the current inode number
				 */
				printf ("%u	", kno);
				/*
				 * print the path to kno
				 */
				pname (inode_num, 0);
				/*
				 * print the current name and end it
				 * with a '/' if its a directory
				 */
				printf ("/%s", dp->d_name);
				if (lookup (kno, 0))
					printf("/.");
				printf("\n");
			}
			if (dirsize <= ((j+1) * DIRBLKSIZ))
				break;	
		}
		dirsize -= frg_len;
	}
}		

/*
 *
 *	NAME:	dirblk_read
 *
 *	FUNCTION:
 *		Read directory block corresponding to the logical block
 *		(lbno) for inode *d_inode.
 *
 *	RETURN:
 *		return the size of the frag in bytes if successful
 *		return 0 if failure.
 */

int
dirblk_read (struct dinode	*d_inode,
	     int  		lbno,
	     char  		*bp,
	     ino_t		inode_num)		
{

	fdaddr_t  frg;

	if (ltop (fd, &frg.f, d_inode, lbno) != LIBFS_SUCCESS)
	{
		fprintf(stderr, MSGSTR(SKIPPING, "skipping...\n"));
		return (0);
	}
	if (frg.f.addr == 0)
		return (0);

	if (bread (fd, bp, frg.f) < 0)
	{
		fprintf (stderr, MSGSTR(CANTRDIR,
				"Can't read directory block for inode %d\n"),
			 inode_num);
		fprintf (stderr, MSGSTR(SKIPPING, "skipping...\n"));
		return (0);
	}
	return (FRAGLEN(frg.f));
}

/*
 *
 *	NAME:	dotname
 *
 *	FUNCTION:
 *		check for names '.' or '..'
 *
 *	RETURN:
 *		return 1 if d_name == '.' or '..'
 */

int
dotname (register struct direct *dp)
{

	if (dp->d_name[0]=='.')
	{
		if (dp->d_name[1] == 0 || (dp->d_name[1] == '.' &&
					   dp->d_name[2] == 0))
		{
			return (1);
		}
	}
	return(0);
}

/*
 *
 *	NAME:	pname
 *
 *	FUNCTION:
 *		print file names
 *
 */

void
pname (ino_t i,
      int lev)
{
	register struct htab *hp;

	if (i == ROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0)
	{
		printf("???");
		return;
	}
	if (lev > 10)
	{
		printf("...");
		return;
	}
	pname(hp->h_pino, ++lev);
	printf("/%s", hp->h_name);
}

/*
 *
 *	NAME:	lookup
 *
 *	FUNCTION:
 *		Search htab for the given inode number (i).
 *		Save the inode number if not found and ef != 0.
 *		Handle collisions by moving to next slot
 *
 *	RETURN:
 *		Return a pointer into htab if the inode number was found
 *		or it was not found but was addedd to htab (ie ef != 0).
 *		Return Null if inode number was not found and ef == 0.
 *		Exit with error if HSIZE exceeded (bogus).
 */

struct htab
*lookup (ino_t	i,	/* inode number to to locate	*/
	int 	ef)	/* Side effect			*/
{
	register struct htab *hp;

	/*
	 * search for ino_t i
	 */
	for (hp = &htab[i % hsize]; hp->h_ino;)
	{
		if (hp->h_ino == i)
			return (hp);
		if (++hp >= &htab[hsize])
			hp = htab;
	}
	Dprintf ("i = %d nhent = %d \n", i, nhent);
	/*
	 * Inode not found - process the side effect
	 * (that is: save the inode number and return its
	 * corresponding struct htab)
	 */
	if (ef == 0)
		return (struct htab *) NULL;

	/*
	 * Only print the error message once.
	 */
	if (nhent == hsize - 1)
	{
		fprintf (stderr, MSGSTR (OUTCORE,
			"ncheck: out of core-- increase HSIZE\n"));
		nerror++;
		nhent++;		
		return (struct htab *) NULL;
	}
	else if (nhent >= hsize)
		return (struct htab *) NULL;

	nhent++;
	hp->h_ino = i;
	return hp;
}

/*
 *
 *	NAME:	read_super
 *
 *	FUNCTION:
 *		Read the super block using the fslib routines.
 *		Process the return codes of get_super.
 *
 *	RETURN:
 *		Exit if couldn't read the super block
 *
 */

void
read_super (int fd,
	    struct superblock *sb,
	    char	*fsname)
{

	switch (get_super (fd, sb))
	{
		case LIBFS_SUCCESS:
		return;

		case LIBFS_BADMAGIC:
		fprintf(stderr, MSGSTR (NOTJFS,
	     	"ncheck:  %s is not recognized as a JFS filesystem.\n"),
			fsname);
		break;

		case LIBFS_BADVERSION:
		fprintf(stderr, MSGSTR (NOTSUPJFS,
	  	"ncheck:  %s is not a supported JFS filesystem version.\n"),
			fsname);
		break;

		case LIBFS_CORRUPTSUPER:
		fprintf(stderr, MSGSTR (CORRUPTJFS,
	        "ncheck:  The %s JFS filesystem superblock is corrupted.\n"),
			fsname);
		break;		

		
		default:
		fprintf(stderr, MSGSTR(CANTRESB, 
			"ncheck:  Cannot read superblock on %s.\n"),
			fsname);
		break;
	}
	fprintf (stderr, "\n");
	close (fd);
	exit(8);
}

