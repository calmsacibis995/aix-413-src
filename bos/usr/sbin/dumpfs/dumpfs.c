static char sccsid[] = "@(#)55  1.13.2.9  src/bos/usr/sbin/dumpfs/dumpfs.c, cmdfs, bos411, 9428A410j 5/20/94 03:53:21";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: dumpfs
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

#include <locale.h>
#include <ctype.h>
#include <sys/types.h>
#include <jfs/filsys.h>
#include <sys/vmdisk.h>
#include <jfs/ino.h>
#include <stdio.h>
#include <fstab.h>
#include <nl_types.h>
#include "dumpfs_msg.h"
#include <libfs/libfs.h>

#define	MSGSTR(N,S)	catgets(catd,MS_DUMPFS,N,S)
nl_catd catd;

#define	TIME_SIZE   100
#define BUF	   64	/* tmp buffer length 				*/
#define	MAP_INDEX  12	/* number of spaces for printing map indicies	*/
#define NUM_COLS    3	/* number of columns 			*/

/*
 * Function proto types
 */

int  	dumpfs (char*, char*);
int  	dumpsuper (int, char*);
int  	dumpmap (int, char*, ino_t);
void  	show_vmdmapv3 (register struct vmdmap*, int);
void  	show_vmdmapv4 (register struct vmdmap*, uint *, int);
void	print_uint_array (char*, uint*, uint);
void	print_short_array (char*, short*, uint);
void	build_index (char*, int, int);
void	print_word (int, int, int, uint);
void	print_short (int, int, int, short);
void	read_super (int, struct superblock*, char*);
static char  	*get_time (time_t*);

/*
 * dumpfs - currently dumps superblock, disk map and inode map
 */


/*
 *	MAIN
 *
 *	- processes command line args...
 */
int
main (int argc,
      char **argv)
{
	struct fstab *fs;
	int rc = 0;

	(void) setlocale (LC_ALL,"");

	catd = catopen (MF_DUMPFS, NL_CAT_LOCALE);

	argc--, argv++;
	if (argc < 1)
	{
		fprintf (stderr, MSGSTR (USAGE, "usage: dumpfs fs ...\n"));
		exit(1);
	}

	for (; argc > 0; argv++, argc--)
	{
		fs = getfsfile(*argv);
		if (fs == 0)
			rc += dumpfs(*argv, *argv);
		else
			rc += dumpfs(*argv, fs->fs_spec);
	}

	return rc ? 1 : 0;
}
/*
 *	dumpfs
 *
 *	- dump a filesystem
 *
 *	- `name' is the command line name
 *	- `device' is the device containing the filesystem
 */
int
dumpfs (char *name,
	char *device)
{
	int fd, rc;

	if ((fd = open (device, 0)) < 0)
	{
		perror(device);
		return (-1);
	}

        if ((rc = dumpsuper (fd, name)) == 0)
	{
		rc = dumpmap (fd, name, INOMAP_I);
		rc += dumpmap (fd, name, DISKMAP_I);
		putchar ('\n');
	}
	close (fd);
	return (rc);
}

/*
 *	dumpsuper
 *
 *	- dump the super block
 *
 *	- Exit if this is not a valid JFS; this is accoplished in read_super.
 */

int
dumpsuper (int fd,
	   char *name)
{
	struct superblock sb;
	static char *get_time (time_t *thetime);
	/*
	 *  read superblock and dump it
	 */
	read_super (fd, &sb, name);

	printf("%s:\n\n", name);
	
	printf (MSGSTR (MAGICN, "magic\t\t\t0x%x%x%x%x\tcpu type\t\t0x%x\n"),
	       sb.s_magic[0], sb.s_magic[1], sb.s_magic[2],
	       sb.s_magic[3], sb.s_cpu);
	
	printf (MSGSTR (FILSYSTY,
		       "file system type\t%d\t\tfile system version\t%d\n"),
		sb.s_type, sb.s_version);

	printf (MSGSTR (FILSYSSZ,
			"file system size\t%-15.d fragment size\t\t%d\n"),
		sb.s_fsize, sb.s_fragsize);
	
	printf (MSGSTR (FILSYSSI,
		"block size\t\t%d\t\tallocation group size\t%d (frags)\n"),
		sb.s_bsize, sb.s_agsize);

	printf (MSGSTR (IAGSIZE,
			"inodes/allocation grp\t%d\t\tcompress\t\t%d\n"),
		sb.s_iagsize,  sb.s_compress);
	
	printf (MSGSTR (FILSYSNA,
		       "file system name\t%.*s\t\tvolume name\t\t%.*s\n"),
		sizeof(sb.s_fname), sb.s_fname, sizeof(sb.s_fpack),
		sb.s_fpack);
	
	printf (MSGSTR (LOGDEVC,
		       "log device\t\t0x%-13.x log serial number\t0x%x\n"),
		sb.s_logdev, sb.s_logserial);
	
	printf (MSGSTR (FILSYSSTA,
		       "file system state\t%d\t\tread only\t\t%d\n"),
		sb.s_fmod, sb.s_ronly);

	printf (MSGSTR (LASTUP,
		       "last update\t\t%s"), get_time(&sb.s_time));

	return (0);
}

/*
 *	dumpmap
 *
 *	- dump the allocation map described by inode 'map'
 */

int
dumpmap (int fd,
	 char *name,
	 ino_t map)
{
	struct vmdmap   vmd, vmd1;
	struct dinode   dp;
	fdaddr_t	frag;
	int 		numpages, page, version;

	/*
	 * print greeting
	 */
	if (map == INOMAP_I)
		printf (MSGSTR(DINODEMAP, "\nInode Map:\n"));
	else
		printf (MSGSTR(DISKBLKMAP, "\nDisk Block Map:\n"));

	/*
	 * get the map inode
	 */
	if (get_inode(fd, &dp, map) != LIBFS_SUCCESS)
	{
		if (map == INOMAP_I)
			fprintf(stderr,	MSGSTR (CANTGI,
					"Cannot get inode for inode map\n"));
		else
			fprintf(stderr,	MSGSTR (CANTGB,
			      "Cannot get inode for disk block map\n"));
		return (-1);
	}


	/*
	 *  dump the pages of the map
	 */
	page = numpages = 0;
	do
	{
		if (ltop (fd, &frag.f, &dp, page) < 0)
		{
			fprintf(stderr,
				MSGSTR (CANTMAP,
				"Can't map logical to physical block\n"));
			return (-1);
		}
		if (bread (fd, &vmd, frag.f) < 0)
		{
			fprintf (stderr, MSGSTR (CANTLSEEK, "can't lseek.\n"));
			return -1;
		}

		/* determine version first time thru */
		if (page == 0)
		{
			version = vmd.version;
			numpages = (vmd.mapsize + WBITSPERPAGE(version) - 1) / 
				   WBITSPERPAGE(version);
			if (version == ALLOCMAPV4)
				numpages += (numpages + 7) / 8;
		}
		if (version == ALLOCMAPV3)
			show_vmdmapv3(&vmd, page);
		else
		{
			/* if it is a control page, save it in vmd1 */
			if ((page % 9) == 0) 
				vmd1 = vmd;
			else
				show_vmdmapv4(&vmd1, (uint *)&vmd, page);
		}
		page++;
	}
	while (page < numpages);
	return 0;
}

/*
 *	show_vmdmapv3
 *
 *	- show the map page 'vmd', page number 'page'
 */

#define	SZARRAY(map)		(sizeof(map) / sizeof(map[0]))

void
show_vmdmapv3 (register struct vmdmap *vmd,
	     int page)
{
	void show_short_array(), show_ushort_array(), show_uint_array();
	unsigned int *pmap, *wmap;

	printf (MSGSTR (PAGCNT,
			"PAGE %d: size=%d, freecnt=%d, agsiz=%d, agcnt=%d\n"),
		page, vmd->mapsize, vmd->freecnt, vmd->agsize, vmd->agcnt);

	printf (MSGSTR(AGSIZ,
	"totalags=%d, lastalloc=%d, clsize=%d, clmask=0x%x, version=%d\n"),
	vmd->totalags, vmd->lastalloc, vmd->clsize, vmd->clmask, vmd->version);


	print_short_array(MSGSTR(AGCNT, "ags:"), vmd->agfree, vmd->agcnt);
	print_uint_array (MSGSTR(TREE, "tree:"), vmd->tree,
			  SZARRAY(vmd->tree));

	wmap = (uint *)vmd + LMAPCTL/4;
	pmap = wmap + WPERPAGE;
	if (memcmp((void *) wmap,
		    (void *)pmap, WPERPAGE*4) == 0)
	{
		print_uint_array (MSGSTR(WPMAP, "maps:"), wmap,
				 WPERPAGE);
	}
	else
	{
		print_uint_array (MSGSTR (WMAP, "wmap:"), wmap,
				 WPERPAGE);
		print_uint_array (MSGSTR(PMAP, "pmap"), pmap,
				WPERPAGE);
	}
}

/* this imititates show_vmdmapv3 by writing out the control
 * information for a bitmap page followed by the bitmap.
 * page is the page in the file, vmd1 is a pointer to the control
 * page, and wmap points to the working copy of the bit map.
 */

void
show_vmdmapv4 (register struct vmdmap *vmd1,
	     unsigned int * wmap,
	     int page)
{
	void show_short_array(), show_ushort_array(), show_uint_array();
	int mpage, ncpage;
	struct vmdmap * vmd;
	unsigned int * pmap;

	/* ncpage = number of control pages pages preceding page.
	 * mpage = page number excluding control pages. page is not
	 * a multiple of 9.
	 */
	ncpage = (page/9) + 1;
	mpage = page - ncpage;
	vmd = VMDMAPPTR(vmd1, mpage);

	printf (MSGSTR (PAGCNT,
			"PAGE %d: size=%d, freecnt=%d, agsiz=%d, agcnt=%d\n"),
		mpage, vmd->mapsize, vmd->freecnt, vmd->agsize, vmd->agcnt);

	printf (MSGSTR(AGSIZ,
	"totalags=%d, lastalloc=%d, clsize=%d, clmask=0x%x, version=%d\n"),
	vmd->totalags, vmd->lastalloc, vmd->clsize, vmd->clmask, vmd->version);


	print_short_array(MSGSTR(AGCNT, "ags:"), vmd->agfree, vmd->agcnt);
	print_uint_array (MSGSTR(TREE, "tree:"), vmd->tree,
			  SZARRAY(vmd->tree));

	pmap = wmap + WPERPAGEV4;
	if (memcmp((void *) wmap,
		    (void *)pmap, 4*WPERPAGEV4) == 0)
	{
		print_uint_array (MSGSTR(WPMAP, "maps:"), wmap,
				  WPERPAGEV4);
	}
	else
	{
		print_uint_array (MSGSTR (WMAP, "wmap:"), wmap,
				 WPERPAGEV4);
		print_uint_array (MSGSTR(PMAP, "pmap"), pmap,
				WPERPAGEV4);
	}
}

/*
 * print_uint_array
 *
 */

void
print_uint_array (char  *name,
	     uint  *map,
	     uint  len)
{
	uint	old_value = *map;
	uint	old_index = 0;
	int	cols = 1;
	int	index_len = strlen (name);
	int	i;

	
	printf ("%s", name);

	index_len = MAP_INDEX - index_len;
	
	for (i = 0; i < len; i++)
	{
		if (map[i] != old_value)
		{
			print_word (i, old_index, index_len, old_value);
			old_index = i;
			old_value = map[i];
			index_len = MAP_INDEX;

			if  (cols++ % NUM_COLS == 0)
			{
				printf ("\n");
				cols = 1;
			}
		}
	}
	print_word (i, old_index, index_len, old_value);
	printf ("\n");
}

/*
 * print_short_array
 *
 */
void
print_short_array (char  *name,
	     short  *map,
	     uint  len)
{
	short	old_value = *map;
	uint	old_index = 0;
	int	cols = 1;
	int	index_len = strlen (name);
	int	i;

	
	printf ( "%s", name);

	index_len = MAP_INDEX - index_len;
	
	for (i = 0; i < len; i++)
	{
		if (map[i] != old_value)
		{
			print_short (i, old_index, index_len, old_value);
			old_index = i;
			old_value = map[i];
			index_len = MAP_INDEX;

			if  (cols++ % NUM_COLS == 0)
			{
				printf ("\n");
				cols = 1;
			}
		}
	}
	print_short (i, old_index, index_len, old_value);
	printf ("\n");
}

void
print_word (int  new,
	    int  old,
	    int	 space,
	    uint value)
{
	char  index[BUF];

	build_index (index, new, old);
	printf ("%*s  0x%08x",	space, index, value);
}
/*
 * print_short
 *
 */

void
print_short (int  new,
	    int  old,
	    int	 space,
	    short value)
{
	char  index[BUF];

	build_index (index, new, old);
	printf ("%*s  %-10d", space, index, value);
}

/*
 * build_index
 *
 *
 */
void
build_index (char  *index,
	     int  new,
	     int  old)
{
	
	if (new  == old + 1)
		sprintf (index, "[%d]:", old);
	else
		sprintf (index, "[%d-%d]:",  old, new - 1);
}

/*
 * NAME: get_time
 *
 * FUNCTION: This is to replace ctime() for international support language
 *
 * RETURNS: string of characters indicates the time
 */

static char
*get_time (time_t * thetime)
{
	struct tm *tmp;
	static char buf[TIME_SIZE];
	
	if ((tmp = localtime(thetime)) == NULL)
		return((char *) NULL);
	/*
	 *strftime does the time in a NLS manner
	 */
	strftime(buf, TIME_SIZE, "%c\n", tmp);
	return(buf);
}

/*
 * NAME:	read_super
 *
 * FUNCTION:	Read the super block and process the return codes
 *		from get_super.
 *
 * RETURN:	Print an error, close file, and exit if
 *		get_super() returns any error.
 *
 */

void
read_super (int		fd,
	    struct superblock  *sb,
	    char	*fsname)
{

	switch (get_super (fd, sb))
	{

		case LIBFS_SUCCESS:
		return;

		case LIBFS_BADMAGIC:
		fprintf(stderr, MSGSTR (NOTJFS,
		"dumpfs:  %s is not recognized as a JFS filesystem.\n"),
			fsname);
		break;

		case LIBFS_BADVERSION:
		fprintf(stderr, MSGSTR (NOTSUPJFS,
		"dumpfs:  %s is not a supported JFS filesystem version.\n"),
			fsname);
		break;

		case LIBFS_CORRUPTSUPER:
		fprintf(stderr, MSGSTR (CORRUPTJFS,
	       "dumpfs:  The %s JFS filesystem superblock is corrupted.\n"),
			fsname);
		break;

		default:
		fprintf(stderr, MSGSTR(CANTRESB, 
			"dumpfs:  Cannot read superblock on %s.\n"),
			fsname);
		break;
	}
	fprintf(stderr, "\n");
	close (fd);
	exit (2);
}
