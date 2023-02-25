static char sccsid[] = "@(#)09	1.6  src/bos/usr/sbin/fsdb/fsdb_read.c, cmdfs, bos411, 9428A410j 1/6/94 11:42:38";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS:  read_obj, process_super, obj_dirty, bit_man
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

/*
 *	Functions added to fsdb to help deal with fragmented
 *	JFS file systems and to replace the old cache.c
 *
 *	Functions:
 *		int		read_obj (int,  off_t, char*, int);
 *		void		obj_dirty(void);
 *		int		bit_man (int,  bitvec_t*, frag_t);
 *		void		process_super (int, struct superblock*, char*);
 *		static 	void	warn_super (int, char*, int);
 *
 *
 */

#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
#include "fsdb.h"


int		read_obj (int,  offset_t, char*, int);
int		adjust_nfrags (frag_t*);
void		obj_dirty(void);
int		bit_man (int,  bitvec_t*, frag_t);
void		process_super (int, struct superblock*, char*);
static 	void	warn_super (int, char*, int);


/*
 * Global data
 */
static	int		dirty = 1;	/* data in frags[] is invalid	*/
static	char		frags[BLKSIZE];	/* buffer for read of fs frags	*/
static	fdaddr_t	frags_addr;	/* address of frags in frags[]	*/


/*
 * NAME:	read_obj
 *
 * FUNCTION:	Read some number of bytes from the filesystem. Use fslib to
 *		perform the actual read.
 *
 *		Begin the read from the nearest frag boundry.
 *		If possible read 1 full block from the starting point
 *		else adjust the read length by seting nfrags to an appropriate
 *		value. This latter case only occurs when attempting to read
 *		an object that falls in the last block of the filesystem.
 *
 *		Copy requested data into provided buffer. Object may
 *		span block boundry. 
 *
 * RETURN:	length of object read or a negative number if error.
 *
 */

int
read_obj (int		fd,
	  offset_t	abs_offset,	/* absolute byte offset to object */
	  char		*obj_buf,	/* buffer to copy object into	*/
	  int		obj_len)	/* length of object		*/

{
	fdaddr_t  faddr;	/* fragment address for object		*/
	off_t	cur_offset;	/* current offset of object in frag	*/
	off_t	cpy_len;	/* amt. to copy out of current frag	*/
	off_t	rem_len;	/* remaining amount to copy		*/
	off_t	frag_len;	/* length of valid data in global buf  	*/
	char	*obj_ptr;	/* pointer into destination buffer 	*/
	char	*src_ptr;	/* pointer into global buffer frags[]	*/

	/*
	 * Does the entire object fall within the filesystem ?
	 */
	if (abs_offset + obj_len > FRAG2BYTE(fmax.addr))
		return (-1);

	rem_len = obj_len;
	obj_ptr = obj_buf;
	src_ptr = frags;
	
	while (rem_len)
	{
		/*
		 * Get begining address of a frag containing the current
		 * piece of the desired object, and calculate the objects
		 * offset within the frag.
		 */
		faddr.f.addr 	= BYTE2FRAG(abs_offset);
		faddr.f.new 	= 0;
		if (! adjust_nfrags (&faddr.f))
			return (-1);

		frag_len = FRAGLEN(faddr.f);
		cur_offset = abs_offset & FragMask;

		/*
		 * Read the block in if necessary, mark it clean,
		 * and save its address.
		 */
		if (faddr.d != frags_addr.d || dirty)
		{
			dirty = 0;
			frags_addr.d = faddr.d;

			if (bread (fd, src_ptr, frags_addr.f) != frag_len)
				return(-1);
		}

		/*
		 * If the object spans this frag  then
		 * copy from cur_offset to the end of the frag.
		 */
		if ((rem_len + cur_offset) > frag_len)
		{
			cpy_len = frag_len - cur_offset;
			abs_offset += cpy_len;
		}
		else
			cpy_len = rem_len;
		
		bcopy ((src_ptr + cur_offset), obj_ptr, cpy_len);
		rem_len -= cpy_len;
		obj_ptr += cpy_len;
	}
	
	return (obj_len);
}
/*
 * NAME:	adjust_nfrags
 *
 * FUNCTION:	Adjust the nfrags field of the frag_t to ensure that it
 *		does not reference frags outside the filesystem.
 *
 *
 * RETURN:	return 0 if it fails
 *		return 1 otherwise.
 */

int
adjust_nfrags (frag_t	*f)
{
	daddr_t	diff;

	if (f->addr + FragPerBlk > fmax.addr)
	{
		if ((diff = fmax.addr - f->addr) < 0)
			return (0);
		f->nfrags = FragPerBlk - diff;
	}
	else
		f->nfrags = 0;

	return(1);
}

/*
 * NAME:	obj_dirty
 *
 * FUNCTION:	After any write to the filesystems the read_obj buffer
 *		should be marked dirty. 
 *
 * RETURN:	void
 *
 */

void
obj_dirty (void)
{
	dirty = 1;
}

/*
 * NAME:	bit_man
 *
 * FUNCTION:	Manipulate bit maps. Thus far there are 3 operations. All
 *		operations begin at a fragment number and extend for a number
 *		of fragments.
 *
 *			TEST:	return the OR of num frags bits.
 *			SET:	set to 1 num frags bits.
 *			DUMP:	print  num frags bits
 *		
 *
 * RETURN:	depends on type of bit manipulation
 *		if test
 *			return the OR of frags-per-block - nfrags bits 
 *		if set || dump
 *			return 0 
 */

int
bit_man (int		op,		/* Type of bitwise manipulation	*/
	 bitvec_t	*vec,		/* bitwise operand		*/
	 frag_t		frg)		/* address mapped to a bit in *vec */
{
	int	dump		= 0;
	int	test		= 0;
	int	i;

	for (i = frg.addr; i < frg.addr + FRAGSIN(frg); i++)
	{
		switch (op)
		{
			case SET:
			bit_set(vec, i);
			break;
			
			case TEST:
			test |= bit_tst(vec, i);
			break;

			case DUMP:
			dump |= bit_tst(vec, i);
			break;

			default:
			return (0);
		}
	}

	if (op == DUMP)
		printf ("0x%02x: word = %8.8x\tdump = %8.8x\n",
			frg.addr, vec[frg.addr>>BIT_SHIFT], dump);

	return (test);
}

/*
 * NAME:	process_super
 *
 * FUNCTION:	Attempt to read the primary and possibly the secondary
 *		superblock.
 *			- If the primary is valid return.
 *			- If the primary is not valid print various
 *			warnings and read the secondary.
 *			- If the secondary is valid return.
 *			- If the secondary is invalid exit.
 *
 * RETURN:	void
 */

void
process_super (int	fd,
	       struct superblock  *sb,
	       char	*fsname)

{

	int	err;
	char	*primary;

	/*
	 * Attempt to get the primary superblock
	 */
	if ((err = get_super(fd, sb)) == LIBFS_SUCCESS)
			return;

	/*
	 * Warn that the primary superblock was somehow wrong
	 * and attempt to get the secondary superblock.
	 */
	warn_super(err, fsname, SUPER_B);
	if ((err = get_super2(fd, sb)) == LIBFS_SUCCESS)
	{
		fprintf (stderr,  MSGSTR(FIXPRIMARY,
		"\nWARNING:\n\tThe problems with the primary superblock\n\tmust be corrected prior to mounting %s.\n"), fsname);
		fprintf (stderr, "\n");
		return;
	}

	/*
	 * Both superblocks couldn't be read or are bad.
	 * Print the problem with the secondary superblock and exit.
	 */
	warn_super(err, fsname, SUPER_B1);

	fprintf (stderr,  MSGSTR(FIXPRIMARY,
		"\nWARNING:\n\tThe problems with the primary superblock\n\tmust be corrected prior to mounting %s.\n"),
		 fsname);

	fprintf(stderr, MSGSTR(FIXSECONDARY,
		"\tThe secondary superblock should also be revalidated.\n\n"));

	exit (1);
}

/*
 * NAME:	warn_super
 *
 * FUNCTION:	Warn the user about superblock problems.
 *
 * RETURN
 *	void
 *
 */

static void
warn_super (int		err,
	    char	*fsname,
	    int		sb_num)
{
	char	*problem;
			  
	switch (err)
	{

		case LIBFS_BADVERSION:
		problem = MSGSTR(JFSVERSION,
			"\n\thas an unrecognized JFS version number.");
		break;

		case LIBFS_BADMAGIC:
		problem = MSGSTR(MAGIC,
			"\n\thas an unrecognized JFS magic number.");
		break;

		case LIBFS_SEEKFAIL:
		case LIBFS_READFAIL:
		problem = MSGSTR(NOREADSUPER,
				  "\n\tcould not be read.");
		break;
		
		case LIBFS_CORRUPTSUPER:
		problem = MSGSTR(CORUPT,
    "\n\tis corrupted. One of s_fragsize, s_agsize, or s_iagsize is invalid.");
		break;
				
		default:
		problem = MSGSTR(BADSUPER, "\n\tis not recognized as AIX JFS.");
		break;
	}

	/*
	 * If its the primary super block thats defective then warn the
	 * user and go back and attempt to read the secondary superblock.
	 * Else the secondary superblock is bad - not much we can do - exit.
	 */
	if (sb_num == SUPER_B)
	{
		fprintf (stderr, MSGSTR(WARNING1,
					"\nWARNING:\n\tThe primary superblock for %s %s\n\tAttempting to read the secondary superblock...\n"),
					fsname, problem);
	}
	else
	{
		fprintf (stderr, MSGSTR(BADSECONDARY,
		"\nFATAL ERROR:\n\tThe secondary superblock for %s %s\n"),
			 fsname, problem);
	}
}
