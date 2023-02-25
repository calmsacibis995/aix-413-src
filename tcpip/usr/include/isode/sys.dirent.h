/* @(#)43	1.3  src/tcpip/usr/include/isode/sys.dirent.h, isodelib7, tcpip411, GOLD410 3/3/93 11:38:08 */
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: DIRENTSIZ S_ISDIR
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/include/isode/sys.dirent.h
 */

/*
	<sys/dirent.h> -- file system independent directory entry (SVR3)

	last edit:	25-Apr-1987	D A Gwyn

	prerequisite:	<sys/types.h>
*/

#ifndef _sysDirent_
#define _sysDirent_

/* The following nonportable ugliness could have been avoided by defining
   DIRENTSIZ and DIRENTBASESIZ to also have (struct dirent *) arguments. */
#define	DIRENTBASESIZ		(((struct dirent *)0)->d_name \
				- (char *)&((struct dirent *)0)->d_ino)
#define	DIRENTSIZ( namlen )	((DIRENTBASESIZ + sizeof(long) + (namlen)) \
				/ sizeof(long) * sizeof(long))

/* DAG -- the following was moved from <dirent.h>, which was the wrong place */
#define	MAXNAMLEN	512		/* maximum filename length */

#ifndef NAME_MAX
#define	NAME_MAX	(MAXNAMLEN - 1)	/* DAG -- added for POSIX */
#endif


#ifdef	dirent
#undef	dirent
#endif
struct dirent				/* data from getdents()/readdir() */
	{
#ifdef apollo
	long	d_ino;			/* inode number of entry */
	unsigned short	d_reclen;	/* length of this record */
	unsigned short	d_namlen;	/* length of string in d_name */
	off_t	d_off;			/* offset of disk directory entry */
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
#else
	long		d_ino;		/* inode number of entry */
	off_t		d_off;		/* offset of disk directory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */	/* non-POSIX */
#endif
	};

#ifdef XOS_2				
#define	S_ISDIR( mode )		(((mode) & S_IFMT) == S_IFDIR)
#endif

#endif /* _sysDirent_ */
