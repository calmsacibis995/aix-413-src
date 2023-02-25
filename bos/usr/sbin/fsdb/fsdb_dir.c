static char sccsid[] = "@(#)10	1.3  src/bos/usr/sbin/fsdb/fsdb_dir.c, cmdfs, bos411, 9428A410j 1/6/94 11:42:41";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS:	dir_add, dir_print, dir_entry, dir_load, dir_check_entry,
 *		dir_round_down, dir_dirty, dir_expand, dir_raw_print,
 *
 * ORIGINS: 27
 *
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
#include <strings.h>
#include "fsdb.h"

/*
 * Module global data:
 *
 * Maintain information about the current data block that is being
 * interpreted as directory entries. Can/should only be updated/invalidated
 * via a call to dir_load() or dir_dirty().
 */

/*
 * 	Inode referencing the current data block.
 */
static	struct	dinode	cur_inode;

/*
 * 	Byte off set to cur_inode
 */
static	offset_t	cur_ino_off;

/*
 *	Logical block number of the current data block.
 */
static	daddr_t		cur_lbno;

/*
 *	Block address of the current data block
 */
static	fdaddr_t	cur_block;

/*
 *	Contents of current block
 */
static	char		cur_data[BLKSIZE];

/*
 *	Map of directory entries in the current data block.
 *	cur_slot_map[N] = the absolute byte offset to the Nth
 *	directory entry in the current block
 */
static	offset_t	cur_slot_map[BLKSIZE];

/*
 *	Total number of entries in cur_slot_map
 */
static	int		cur_tot_slot;

/*
 *	Sum total of all d_reclens in current block.
 */
static	int		cur_len;



/*
 *	Function Prototypes
 */
int		dir_add (int, offset_t*, int, char);
int		dir_print (int, offset_t,  offset_t,  int);
int		dir_entry (int, offset_t,  offset_t, offset_t*, ino_t*, int);
int		dir_load (int, offset_t, offset_t);
int		dir_check_entry (offset_t);
int		dir_round_down (int, offset_t, offset_t*);
void		dir_dirty (void);
int		dir_expand (int, offset_t, offset_t*);
void		dir_raw_print (int, offset_t*, int, int);
static	int	dir_dump (char*, off_t, int,  int);
static 	int	dir_inode (int, offset_t);
static 	int	dir_blkno (int, offset_t, daddr_t);
static	int	dir_block_read (int, fdaddr_t);
static 	void	dir_load_dmaps (void);


/*
 * NAME:	dir_dirty
 *
 * FUNCTION:	A write has taken place in the main module, ipso facto
 *		the state information maintained herein may no longer
 *		be valid. Invalidate everything so that the next
 *		operation will reread everything.
 *
 * RETURN:	void
 *
 */

void
dir_dirty (void)
{
	memset (&cur_inode, '\0', sizeof(struct dinode));
	memset (cur_slot_map, '\0', BLKSIZE);
	cur_tot_slot 	= 0;
	cur_lbno 	= 0;
	*cur_data	= '\0';	
	cur_ino_off 	= 0;
	cur_block.d 	= 0;
	cur_len		= 0;
}	

/*
 * NAME:	dir_add
 *
 * FUNCTION:	Add or subtract count d_reclen's. to/from the current address.
 *		If the operation falls out side the current block, do nothing.
 *
 *		This function is generally called via then macros
 *		dir_increment and dir_decrement as defined in fsdb.h.
 *
 * RETURN:	0  for any error
 *		else return 1 
 *
 */

int
dir_add	 (int		fd,
	  offset_t	*dir_off,	/* Byte offset to dir entry	*/
	  int		count,		/* Number of d_reclen's to inc.	*/
	  char		op)
{
	int	lsn;		/* Logical slot number of *dir_off	*/
	int	i;


	if ((lsn = dir_check_entry (*dir_off)) < 0)
	{
		printf (MSGSTR(DIRSLOTRANGE,
			       "directory slot %d out of range\n"), lsn);
		return (0);
	}

	switch (op)
	{
		case '+':
		lsn = lsn + count;
		break;

		case '-':
		lsn = lsn - count;
		break;

		default:
			printf(MSGSTR(BADDIROFF, "bad directory offset\n"));
			return (0);
	}
	
	if (lsn >= cur_tot_slot || lsn < 0)
	{
		printf (MSGSTR (DIRSLOTRANGE,
				"directory slot %d out of range\n"), lsn);
		return (0);
	}	

	*dir_off = cur_slot_map[lsn];
	return (1);
}

/*
 * NAME:	dir_print
 *
 * FUNCTION:	Print directory entries from the current address to end of
 *		the containing block or for "count"  entries which ever comes
 *		first. A count of 0 results in printing to the end of
 *		the block.
 *
 *
 * RETURN:	if error
 *			return -1
 *
 *		if no error
 *			return sum of d_reclen's transversed during the
 *			print process.
 *			
 */

int
dir_print (int		fd,
	   offset_t  	ino_offset, /* byte offset to an inode	*/
	   offset_t  	addr,	    /* address w/in a data blk of the inode */
	   int		count)      /* Number of entries to print */
{
	off_t	dir_offset;	/* Offset w/in block to the dir entry	*/
	off_t	blk_off;	/* Num bytes transversed by dir_dump()	*/
	int	lsn;		/* Log. slot num: offset w/in cur_slot_map */

	if (! dir_load (fd, ino_offset, addr))
			return (-1);

	if ((lsn = dir_check_entry (addr)) < 0)
		return (-1);

	if ((dir_offset = cur_slot_map[lsn] - FRAG2BYTE(cur_block.f.addr)) < 0)
		return (-1);
	
	if ((blk_off = dir_dump (cur_data, dir_offset,
				 count, FRAGLEN(cur_block.f))) <= 0)
	{
		return (-1);
	}
	/*
	 * Return the number of bytes past dir_offset that was printed.
	 */
	return (abs(blk_off - dir_offset));
}

/*
 * NAME:	dir_entry
 *
 * FUNCTION:	Get the byte offset to a directory entry (direct_t);
 *		also get the inode number (d_ino) recorded in the direct_t.
 *
 *		The directory entry in question is defined by the following:
 *			- a byte offset to a data block.
 *			- a byte offset to an inode.
 *			- a logical directory entry number.
 *
 *			- The data block must be referenced by the inode.
 *			- The logical directory entry must fall within
 *			the data block.
 *
 * RETURN:	
 *		if failure
 *			return 0;
 *		
 *		else
 *			Set off_t *byte to the byte address of
 *				the Nth	directory entry.
 *			Set ino_t *dir_ino to the inode number of the Nth
 *				directory entry.
 *			return 1
 */

int
dir_entry (int	fd,
	   offset_t	inode_off,	/* byte address of inode	*/
	   offset_t	addr,		/* byte offset of block to load */
	   offset_t	*offset,	/* byte address to return	*/
	   ino_t	*dir_ino,	/* inode num to return	  	*/
	   int	entry)			/* logical dir entry number	*/
{
	int		blk_rel_off;
	int		frag_len = FRAGLEN(cur_block.f);
	offset_t	off;
	direct_t  	*dp;

	if (! dir_load (fd, inode_off, addr))
		return (0);	
	
	if (entry >= cur_tot_slot)
		return(0);

	/*
	 * Get the offset w/in the current block to the directory entry,
	 * point a direct_t to it, and set the return information.
	 */
	off = cur_slot_map[entry];
	if ((blk_rel_off = off -  FRAG2BYTE(cur_block.f.addr)) >= frag_len)
	{
		return (0);
	}
	dp = (direct_t *)((char *) cur_data + blk_rel_off);
	if (dp->d_reclen > DIRBLKSIZ || dp->d_reclen + blk_rel_off > frag_len)
		return (0);

	*dir_ino =  dp->d_ino;
	*offset = off;
	return (1);
}


/*
 * NAME:	dir_round_down
 *
 * FUNCTION:	Given an address that lies within an inode's data blocks
 *		find the begining of the nearest directory entry.
 *
 * RETURN:	return 0 if failure
 *		return 1 if successful
 * 	
 */

int
dir_round_down (int		fd,
		offset_t	i_off,
		offset_t	*d_off)
{
	int		lsn;	/* Logical slot number of *dir_off	*/
	offset_t  	addr = *d_off;

	/*
	 * Don't run past the end or fall off the begining.
	 */
	if (addr < cur_slot_map[0] ||
	    addr > (FRAG2BYTE(cur_block.f.addr) + FRAGLEN(cur_block.f)))
	{
		return (0);
	}
	
	for (lsn = 0; lsn < cur_tot_slot && addr > cur_slot_map[lsn]; lsn++);

	/*
	 * If its not exactly the current slot then it falls in the previous.
	 */
	*d_off = (addr == cur_slot_map[lsn]) ? cur_slot_map[lsn] :
		cur_slot_map[lsn - 1];
	
	return (1);
}

/*
 * NAME:	dir_load
 *
 * FUNCTION:	Initailize all module global data.
 *
 * RETURN:	return 0 if failure
 *		return 1 if successful
 * 	
 */

int
dir_load (int		fd,
	  offset_t	i_off,
	  offset_t	d_off)
{
	daddr_t  	nblks;
	
	/*
	 * Read the inode and save some info about it.
	 */
	if (! dir_inode (fd, i_off))
		return (0);

	/*
	 * Set the logical block num and the block address.
	 */
	nblks = NUMDADDRS(cur_inode);
	if (! dir_blkno (fd, d_off, nblks))
	{
		printf (MSGSTR (BADDIROFF, "bad directory offset\n"));
		return (0);
	}
		
	/*
	 * Load the data block
	 */
	if (dir_block_read (fd, cur_block) < 0)
	{
		printf (MSGSTR (READERROR2, "read error : 0x%lx\n"),
			cur_block.d);
		return (0);
	}

	/*
	 * Walk the current block and record the begining
	 * of each directory entry.
	 */
	dir_load_dmaps ();
	return (1);
}


/*
 * NAME:	dir_check_entry
 *
 * FUNCTION:	Check to see if "entry" is in cur_slot_map.
 *
 * RETURN:	Return the offset into cur_slot_map[] its "entry"
 *		was located else return -1.
 *
 */

int
dir_check_entry (offset_t 	entry)
{

	int	i;
	
	for (i = 0; i < cur_tot_slot && entry != cur_slot_map[i]; i++);

	if (entry == cur_slot_map[i])
		return (i);

	return (-1);
}

/*
 * NAME:	dir_expand
 *
 * FUNCTION:	Expand a directory block by "len" bytes - sort of.
 *		The user supplied "len" is used to see if a new directory
 *		entry can be added to the last dir chunk or if
 *		a new dir chunk must be included. In either case d_reclen
 *		will point to the end of the dir chunk; this will be greater or
 *		equal to "len". If a new chunk is required but the there is
 *		no more space in the FRAG then error out.
 *
 *
 * RETURN:	return 0 for any error
 *		return 1 if sucessful
 *
 */

int
dir_expand (int		fd,
	    offset_t	i_off,
	    offset_t	*d_off)
{
	int	req_len;	/* Requested length of new dir entry	*/
	int	new_len;	/* Actual Length of new dir entry	*/
	int	last_len;	/* Length of the last dir entry current blk */
	int	new_name_len;
	int	excess;
	char	c;
	char	new_name[MAXNAMLEN + 1];
	direct_t	*last_dir;
	direct_t	*new_dir;

	
	/*
	 * Get the expansion length.
	 */
	c = getc(stdin);
	if (! isdigit ((int)c))
	{
		ungetc((int)c, stdin);
 		printf (MSGSTR (MUSTSPECRECLEN,
				"must specify record length\n"));
		return (0);
	}
	ungetc((int)c, stdin);
	if ((req_len = getnumb()) < 0)
		return(0);
	/*
	 * Round the length to 4 byte boundry.
	 */
	req_len = ROUND_UP4(req_len);

	/*
	 * Is the length reasonable
	 */
	if (req_len < LDIRSIZE(0))
	{
		printf (MSGSTR (MINDIRSIZE,"minimum directory size = %d\n"),
			LDIRSIZE(0));
		return (0);
	}
	if (req_len > DIRBLKSIZ)
	{
		printf(MSGSTR (MAXDIRSIZE,
			       "maximum directory size = %d\n"), DIRBLKSIZ);
		return (0);
	}
	
	
	/* 
	 * load directory block
	 */
	if (! dir_load (fd, i_off, *d_off))
		return(0);

	/*
	 * There should be at least one directory entry, but this fact
	 * needs to be confirmed else the following
	 * "cur_slot_map[cur_tot_slot - 1]" would  result in an invalid offset.
	 */
	if (cur_tot_slot <= 0)
		return (0);

	/*
	 * Get the current last entry and its required length.
	 */
	last_dir = (direct_t *) ((char *) cur_data +
				    (cur_slot_map[cur_tot_slot - 1] -
				     FRAG2BYTE(cur_block.f.addr)));

	last_len = LDIRSIZE(last_dir->d_namlen);

	/*
	 * Make up the name of the new entry: "d""slotnum".
	 * This is bogus because it may result in a d_reclen greater than
	 * the requested length which then may not fit where the user
	 * wants it to. Only way to fix this would be to require the user
	 * to enter a new name.
	 */
	*new_name = '\0';
	sprintf (new_name, "d%d", cur_tot_slot);
	new_name_len = strlen(new_name);
	new_len = LDIRSIZE(new_name_len);
	new_len = (new_len > req_len) ? new_len : req_len;

	/*
	 * See if the new entry will fit in the last DIRBLKSIZ.
	 * Else,  see if a new DIRBLKSIZ block can be added.
	 * Else,  no can do.
	 */
	excess = last_dir->d_reclen - last_len;
	if (excess >= new_len)
	{
		/*
		 * Add entry to current dir chunk.
		 */
		memset ((char *)last_dir + last_len, '\0', excess);
		last_dir->d_reclen = last_len;
		new_dir = (direct_t *) ((char *)last_dir + last_len);
		memcpy (new_dir->d_name, new_name, new_name_len);
		new_dir->d_namlen = new_name_len;
		new_dir->d_reclen = excess;
	}
	else if ((cur_len + DIRBLKSIZ) <= FRAGLEN(cur_block.f))
	{
		/*
		 * Add entry to a new dir chunk.
		 */
		memset ((char *)last_dir + last_dir->d_reclen,
			'\0', DIRBLKSIZ);
		new_dir = (direct_t *) ((char *)last_dir + last_dir->d_reclen);
		memcpy (new_dir->d_name, new_name, new_name_len);
		new_dir->d_namlen = new_name_len,
		new_dir->d_reclen = DIRBLKSIZ;
	}
	else
	{
		/*
		 * error out
		 */
		printf (MSGSTR (NOFREE,
				"There is not enough free space in block (0x%2x)\nfor a directory entry of %d bytes long.\n"),
			cur_block.d, new_len);
		return (0);
	}

	if (bwrite (fd, cur_data, cur_block.f) < FRAGLEN(cur_block.f))
	{
		dirty_buffers();
		return(0);
	}
	/*
	 * set the offset the address of the new entry
	 */
	*d_off = cur_slot_map[cur_tot_slot - 1] + last_dir->d_reclen;
	dirty_buffers();
	return (1);
}

/*
 * NAME:	dir_raw_print
 *
 * FUNCTION:	Print any part of the filesystem as directory entries.
 *		Does not use any of the globals maintained in this
 *		module; they are reserved for directory entries associated
 *		w/ and inode.
 *
 * RETURN:	void
 *
 */

void
dir_raw_print (int	fd,
	       offset_t	*addr,
	       int	nfrags,
	       int	count)
{
	char		buf[BLKSIZE];
	off_t		offset;
	off_t		new_offset;
	fdaddr_t	frg;
	
	RECONSTITUTE_FRAG(*addr, nfrags, frg);

	if (bread (fd, buf, frg.f) <  FRAGLEN(frg.f))
	{
		printf (MSGSTR (READERROR2, "read error : 0x%lx\n"),
			frg.d);
		return;
	}	
	offset = *addr - FRAG2BYTE(frg.f.addr);
	if (new_offset = dir_dump (buf, offset, count, FRAGLEN(frg.f)) <= 0)
		return;

	*addr += new_offset;
	return;
}


/*
 * NAME:	dir_dump
 *
 * FUNCTION:	Dump the given buffer as directory entries. Print from the
 *		given starting location to the end of block or for the
 *		given number of entries. Logical slot numbers are calculated by
 *		begining at the front of the buffer and walking the chain
 *		of direct_t's.
 *
 *		if (count <= 0)
 *			dump to end of block;
 *
 * RETURN:
 *		return the ending offset; to put it another way, return
 *		number of bytes transversed into the dir block. This
 *		number is from the begining of the block.
 *
 */

static int
dir_dump (char	*buf,
	  off_t	start,		/* Start printing from this offset w/in *buf */
	  int	count,		/* Number of entries to print		*/
	  int	len)		/* Length of *buf			*/
{
	int	buf_offset = 0;		/* Current offset w/in *buf	*/
	int	chunk;			/* DIRBLKSIZE piece of *buf	*/
	int	sln = 0;		/* slot number			*/
	int	dir_ent;		/* offset within a chunk	*/
	direct_t	*dp;


	/*
	 * A count == 0 is an instruction to dump the entire block.
	 * If count <= 0 then set count to -1 so that it won't
	 * ever be 0 in the following loops, thus the other checks
	 * control the loop iteration. Else, count is decremnted
	 * to 0 when printing out "count" entries.
	 */
	if (count <= 0)
		count = -1;
	
	for (chunk = 0; chunk < len / DIRBLKSIZ && count; chunk++)
	{
		/*
		 * pick the next directory chunk
		 */
		dp = (direct_t *) &buf[chunk * DIRBLKSIZ];

		/*
		 * Go through each chunk examining each dir entry.
		 * Only start printing  when we have found the entry
		 * corresponding to the "start" offset.
		 */			
		for (dir_ent = 0; dir_ent < DIRBLKSIZ && count;
		     dir_ent += dp->d_reclen,
		     dp = (direct_t *) ((char *)dp + dp->d_reclen))
		{
			if (buf_offset >= start)
			{
				count--;
				printf (MSGSTR(DIRFORMAT1,
					       "d%d (slot=%u): %4d  "),
					sln, buf_offset, dp->d_ino);

				printf ("%s", dp->d_name);
				printf (MSGSTR (DIRFORMAT2,
				      " (d_reclen/d_namlen = %d/%d)\n"),
					dp->d_reclen, dp->d_namlen);
			}

			/*
			 * If d_reclen is 0 then we'll loop for quite
			 * some time and then crash and burn. The valid
			 * test "&& dp->d_ino != 0" cannot be added to the
			 * loop conditional because we may be walking a block
			 * that is not valid directory block. One way such
			 * an invalid block could be created is by the
			 * dir_expand() function which sets d_ino = 0.
			 *
			 * To avoid the loop and crash scenario and to
			 * allow the one to see the changes made by
			 * dir_expand(), call it quits for this chunk
			 * when  d_reclen == 0 not when d_ino == 0;
			 */			 
			sln++;
			if (dp->d_reclen <= 0)
			{
				buf_offset += DIRBLKSIZ;
				break;
			}
			buf_offset += dp->d_reclen;
		}
	}
	return (buf_offset);
}	
	
/*
 * NAME:	dir_inode
 *
 * FUNCTION:	Read an inode if its different from the last inode read.
 *		If an new inode then invalidate global data.
 *
 * RETURN:	0 if fail
 *		1 otherwise
 */

static int
dir_inode (int  	fd,
	   offset_t	ino_offset)
{
	struct dinode	inode;
	fdaddr_t	frg;

	
	if ( cur_ino_off == ino_offset)
		return (1);

	if (read_obj (fd, ino_offset, &inode,
		      sizeof(struct dinode)) !=   sizeof(struct dinode))
	{
		printf (MSGSTR (READERROR2, "read error : 0x%lx\n"),
			ino_offset);
		return (0);
	}

	if ((inode.di_mode & IFMT) != IFDIR && ! override)
	{
		printf (MSGSTR (NOTDIRINO,
			"Not a directory inode: inode address = 0x%lx\n"),
			ino_offset);
		return (0);		
	}

	if (ltop (fd, &frg, &inode, 0) != LIBFS_SUCCESS)
			return (0);	
	
	memcpy (&cur_inode, &inode, sizeof (struct dinode));
	memset (cur_slot_map, '\0', BLKSIZE);
	cur_tot_slot 	= 0;
	cur_lbno 	= 0;
	*cur_data	= '\0';	
	cur_ino_off 	= ino_offset;
	cur_block.d 	= frg.d;
	cur_len		= 0;
	return (1);
}

/*
 * NAME:	dir_blkno
 *
 * FUNCTION:	Given an inode and a byte address find the block
 *		containing the address and  set the globals cur_block and
 *		cur_lbno.
 *
 * RETURN:	0 if the address in not referenced by the current inode.
 *		Otherwise, return 1.
 *
 */

static int
dir_blkno (int  		fd,
	   offset_t		addr,
	   daddr_t		nblks)
{

	int		lbno;
	daddr_t		cur_addr = FRAG2BYTE(cur_block.f.addr);
	fdaddr_t	frg;	


	if (addr >= cur_addr && addr < cur_addr + FRAGLEN(cur_block.f))
	{
		return (1);
	}
	 
	for (lbno = 0; lbno <  nblks; lbno++)
	{
		if (ltop (fd, &frg, &cur_inode, lbno) != LIBFS_SUCCESS)
			return (0);

		if (addr >= FRAG2BYTE(frg.f.addr) &&
		    addr < FRAG2BYTE(frg.f.addr) + FRAGLEN(frg.f))
		{
			cur_lbno = lbno;
			cur_block.d = frg.d;
			return (1);
		}
	}
	return (0);
}

/*
 * NAME:	dir_block_read
 *
 * FUNCTION:	Read in a new block only if its different from the current
 *		block or the current block is invalid.
 *
 * RETURN:	0 if fail
 *		1 otherwise
 *
 */

static	int
dir_block_read (int  		fd,
		fdaddr_t	frg)
{
	if (frg.d == cur_block.d && *cur_data != '\0')
		return (1);

	if (bread (fd, cur_data, frg.f) <  FRAGLEN(frg.f))
	{
		*cur_data = '\0';
		return (0);
	}

	cur_block.d = frg.d;
	return (1);
}


/*
 * NAME:	dir_load_dmaps
 *
 * FUNCTION:	Walk the current block as a directory block storing the
 *		absolute byte offset to each directory entry in cur_slot_map.
 *
 * RETURN:	Void
 *
 */

static void
dir_load_dmaps ()
{
	int	chunk;		/* Current dir chunk w/in current block	*/
	int	dir_ent;	/* Current dir entry w/in current chunk	*/
	off_t	   frag_len =	FRAGLEN(cur_block.f);
	offset_t   offset =	FRAG2BYTE(cur_block.f.addr);
	long  	dirsize;	
	direct_t 	*dp;


	cur_tot_slot = 0;
	cur_len = 0;
	
	for (chunk = 0; chunk < frag_len / DIRBLKSIZ; chunk++)
	{
		/*
		 * pick the next directory chunk
		 */
		dp = (direct_t *) &cur_data[chunk * DIRBLKSIZ];

		/*
		 * Go through each chunk examining each dir entry
		 */			
		for (dir_ent = 0;
		     dir_ent < DIRBLKSIZ;
		     dir_ent += dp->d_reclen,
		     dp = (direct_t *) ((char *) dp + dp->d_reclen))
		{
			
			/*
			 * If d_reclen is 0 then we'll loop for quite
			 * some time and then crash and burn. The valid
			 * test "&& dp->d_ino != 0" cannot be added to the
			 * loop conditional because we may be walking a block
			 * that is not valid directory block. One way such
			 * an invalid block could be created is by the
			 * dir_expand() function which sets d_ino = 0.
			 *
			 * To avoid the loop and crash scenario and to
			 * allow the one to see the changes made by
			 * dir_expand(), call it quits for this chunk
			 * when  d_reclen == 0 not when d_ino == 0;
			 */
			if (dp->d_reclen <= 0)
				break;			
			
			cur_slot_map[cur_tot_slot++] = offset;
			cur_len += dp->d_reclen;
			offset += dp->d_reclen;
		}
	}
}
