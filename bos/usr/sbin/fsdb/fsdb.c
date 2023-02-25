static char sccsid[] = "@(#)95	1.15.1.22  src/bos/usr/sbin/fsdb/fsdb.c, cmdfs, bos411, 9428A410j 3/5/94 17:54:58";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fsdb
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <setjmp.h>
#include <errno.h>
#include <unistd.h>
#include <nl_types.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/vmdisk.h>
#include <sys/sysmacros.h>
#include <jfs/filsys.h>
#include <jfs/inode.h>
#include <jfs/ino.h>
#define _KERNEL
#include <sys/dir.h>
#undef _KERNEL
#include <sys/vmount.h>

/* 
 * Before including fsdb.h, announce to the 
 * preprocessor this is the main module
 */
#define IN_FSDB.C	1	 
#include "fsdb.h"

/*
 *	1001 Globals (in true cmdfs fashion)
 *
 *	All comments in the following global declarations were
 *	added posthumously ipso facto they may be somewhat inaccurate.
 */

/* 
 * 	Flag for name changes to directory entries.
 */
static	int 		chname;

/*
 * 	Dynamicly allocated bitmaps that are '# of blocks in fs' elements long.
 * 	Each block in s_bitmap is 1 is the block is an inode block, 
 *	or any other special block.
 */
static	bitvec_t	*s_bitmap;

/*
 *	 A list of blocks of the inode maps and disk maps.
 */
static	daddr_t	 	*imblocks;
static	daddr_t 	*dmblocks;

/*
 * 	Current byte offset to filesystem objects.
 */
static	offset_t	addr;
static	offset_t 	value;
static	offset_t 	temp; 
static  int		num_frags;
static	offset_t	cur_ino 	= INODES_B * BLKSIZE;	
static	short		abs_addr;	/* absolute address flag    */
/*
 *	Map information
 */
static	offset_t 	cur_mp;		/* current map page address */
static	offset_t	cur_mpgno;	/* current map page number  */
static  struct dinode	*cur_mpino	= (struct dinode *) NULL;
static	int		map_ver		= -1;

/*
 * 	Characteristics of filesystem objects
 */
static	short		objsz 		= INUM;
static	short		type;

/*
 * 	Byte offset to filesystem objects to restore upon any (almost ?) error.
 */
static	offset_t 	erraddr;
static	int		err_num_frags;

/*
 *	Addresses to save/resotore via '>' or '<'
 */
static	offset_t 	oldaddr; 
static	offset_t 	oldvalue;
static	short		oldobjsz;
static	offset_t 	saveaddr;

/*
 *	Filesystem statistics
 */
static	daddr_t		fsize;
static	char    	*dskname 	= NULL;
static	char		filsysname[PATH_MAX];
static	short		mounted = 0;
static	int		fd;

/*
 *	Truly silly global generic variables
 */
static	short		c_count;	/* characters read		*/
static	short		i, j, k;
static	short		count;
static	short		retcode;
static	short		prt_flag;

/*
 * 	Someting to increment for any error
 */
static	short		error;

/*
 * 	Inode information
 */
static	ushort		nlinks;
static	mode_t		mode;
static	pid_t		pid;
static	pid_t		rpid;
static	jmp_buf 	env;

/*
 * 	Disk and inode map inodes
 */
static	struct dinode 	*imi 	= (struct dinode *) NULL;
static	struct dinode 	*dmi 	= (struct dinode *) NULL;

/*
 * Function prototypes
 */
static	void		reinitialize (int);
static	void		get_map_inodes (int, int);
static 	void		alloc_map_block_lists (void);
static	void		load_sizes (struct superblock*);
static  int		init_bitmaps (void);
static  int		mark_blocks (struct dinode*, char*, long*,
				     long*, daddr_t*);
static  int		mark_inodes (long*);
static  long		get (offset_t, short);
static	int		put (offset_t, int, long, short);
static  int		dir_align (offset_t);
static  int		align (short);
static  int		puta(offset_t, int);
static  int		fprnt(char, short, int);
static	int		icheck(offset_t);
static	void		err(void);
static  int		devcheck(short);
static  int		dircheck(offset_t, int);
static  int		map_pageno (int, daddr_t);
static  char 		*map_type (int);
static  int		ismounted (char*);
static	ino_t		calc_inode_num (offset_t);
static	off_t		print_chars (char*, offset_t, int);
static	void		print_time (time_t, char*);
static	offset_t	get_mapsum (offset_t, struct dinode*, int, int);
static	offset_t	get_bitmap (offset_t, struct dinode*, int, char, int);

/*
 * Interfaces to fsdb_read.c
 */
extern	int		read_obj (int,  offset_t, char*, int);
extern	void		process_super (int, struct superblock*, char*);
extern	void		obj_dirty(void);
extern	int		adjust_nfrags (frag_t*);
extern  int		bit_man (int,  bitvec_t*, frag_t);

/*
 * 	Interfaces to fsdb_dir.c
 */
extern	int		dir_add (int, offset_t*, int, char);
extern	int		dir_print (int, offset_t,  offset_t,  int);
extern	int		dir_entry (int, offset_t, offset_t, offset_t*,
				   ino_t*, int);
extern	int		dir_load (int, offset_t, offset_t);
extern	int		dir_check_entry (offset_t);
extern	int		dir_round_down (int, offset_t,offset_t*);
extern	void		dir_dirty (void);
extern	int		dir_expand (int, offset_t, offset_t*);
extern	void		dir_raw_print (int, offset_t*, int, int);

/*
 * NAME:	main
 *
 * FUNCTION:	main - continuously looping input scanner.  Lines are scanned
 * 		a character at a time and are processed as soon as
 * 		sufficient information has been collected. When an error
 * 		is detected, the remainder of the line is flushed.
 */

main (short	argc,
      char	**argv)
{
	char		*cptr;
	char 		c;
	short 		offset		= 0;
	offset_t	dir_offset	= 0;
	ino_t		i_num		= 0;
	fdaddr_t	frag_addr;
	struct dinode 	itmp;



        fd = (-1);
	d_level = 0;
	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_FSDB, NL_CAT_LOCALE);

#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv ("fsdb", &argv, 0);
#endif
        if (! strcmp(argv[1], "-d"))
	{
		int foo;
		argc--;
		argv++;
		foo = atoi(argv[1]);
		if (foo != 0)
		{
			argv++;
			argc--;
		}
		else
			foo = 99999;
		d_level = foo;
	}

	if (argc != 2 && argc != 3)
	{
		printf(MSGSTR(USAGE, "usage: fsdb filsys [-]\n"));
		goodbye(1);
	}

	dbprintf(1000, ("calling getdisk(%s)\n", argv[1]));

	dskname = argv[1];
	if ((fd = fsopen (dskname, O_RDWR)) < 0)
	{
		printf(MSGSTR(CANTACCESS,
    			"Unable to access file system %s\n"), dskname);
		goodbye (1);
	}

	printf ("%-40s%12s\n", MSGSTR (HEADERMSG, "\n\nFile System:"), 
				dskname);

	if (ismounted (dskname))
		mounted = 1;
	/*
	 * Initalize globals, load filesystem size, and load bitmaps
	 */
	reinitialize (1);

	if (argc == 3)
	{
		printf (MSGSTR (ERRCHKOFF, "error checking off\n"));
		override = 1;		
	}
	else
		override = 0;

#ifndef STANDALONE
	signal(2,err);
	setjmp(env);
#endif

	for(;;)
	{
		if (error)
		{
			if(c != '\n')
				while (getc(stdin) != '\n');
			c_count = 0;
			error = 0;
			addr = erraddr;
			num_frags = err_num_frags;
			prt_flag = 0;			
			printf (MSGSTR (HOOKNL, "?\n"));
			/* type = -1; allows "d63 + +" to work */
		}

		c = getc (stdin);
		c_count++;
		dbprintf(1001,("got char %d (%c)\n",c,c<' '?' ':c));

		switch(c)
		{
			case 'x':  /* x-pand directory */
			if (mounted)
			{
				/* discard rest of line */
				while ((c = getc(stdin)) != '\n');
				
				printf(MSGSTR(CANTMODIFY,
				"cannot modify mounted file systems\n"));
				error++;
			}
			else if (! dir_expand(fd, cur_ino, &addr))
				error++;

			continue;

			/*
			 * debug level: Undocumented !
			 */
			case 'G':
			c = getc(stdin);
			if (isdigit((int)c))
			{
				ungetc((int)c,stdin);
				temp = getnumb();
				if (error)
					continue;
				d_level = temp;
				prt_flag = 1;				
				continue;
			}
			else if (c == '\n')
			{
				prt_flag = 1;				
				continue;
			} 
			error++; 
			continue;

			/*
			 * command end
			 */			
			case '\n':
			dbprintf(1001,("erraddr = addr (%lld)\n",addr));
			erraddr = addr;
			err_num_frags = num_frags;
			/*
			 * If count == 1 then we are steping through the
			 * filesystem on objsiz increments
			 */
			if (c_count == 1)
			{
				if (objsz != DIRECTORY) 
					addr = addr + abs(objsz);
				else if (! dir_increment (fd, &addr, 1))
				{
					error++;
					continue;
				}
			}
		
			c_count = 0;
			if (prt_flag)
			{
				prt_flag = 0;
				continue;
			}
			temp = get(addr, objsz);
			/*
			 * if an error has been flagged, it is probably
			 * due to alignment.  This will have set objsz
			 * to 1 hence the next get should work.
			 */
			dbprintf(1001,
				 ("calling get (addr=%lld,objsz=%d)\n",
				  addr,objsz));
			if (error)
				temp = get(addr,objsz);
			switch (objsz)
			{
				case CHAR:
				cptr = ".B";
				break;
				
				case SHORT_NO_ALIGN:
				case SHORT:
				cptr = "";
				break;
				
				case LONG_NO_ALIGN:
				case LONG:
				cptr = ".D";
				break;
				
				case DIRECTORY:
				fprnt('d', 1, 0);
				prt_flag = 0;
				continue;

				case INOMAP:
				fprnt('I',1, num_frags);
				cur_mp = addr;
				prt_flag = 0;
				continue;

				case DISKMAP:
				fprnt('M',1, num_frags);
				cur_mp = addr;
				prt_flag = 0;				
				continue;

				case INODE: 
				fprnt('i', 1, num_frags); 
				cur_ino = addr;
				prt_flag = 0;				
				continue;
			}
			printf(MSGSTR (OUTFORMAT,
			"0x%10.10llx%-2s:  0x%8.8x (%d)\n"),
	       			addr,cptr, (int)temp, (int)temp);
			/* 
			 * end case '\n'
			 */
			continue;	

			/*
			 * catch absolute addresses, b and i#'s
			 */
			default: 
			if (isdigit ((int)c))
			{
				ungetc ((int)c, stdin);
				addr = getnumb();
				frag_addr.d = (daddr_t) addr;
				num_frags = frag_addr.f.nfrags;
				objsz 	= INUM;
				value 	= addr;
				type 	= NUMB;
				abs_addr = 1;
				continue;
			}
			if (feof (stdin))
				goodbye(0);
			error++;
			continue;
			
			/*
			 * Inode map block # to map block conversion
			 */
			case 'I':
			/*
			 * Disk map block # to map block conversion
			 */
			case 'M':
			/*
			 * 'addr' contains a logical map block
			 * number. In this case, nfrags is
			 * irrelevant.
			 *
			 *  If this is a version 4 map the we are going to
			 *  locate the summary information associated w/
			 *  given map logical block and the appropriate
			 *  map inode.
			 *
			 *  If this is not version 4 map then we simply read
			 *  the given logical block number assoicated with
			 *  the appropriate map inode.
			 */
		       {
			    	int ty;			       

				if (c == 'I')
					ty = INOMAP;
				else
					ty = DISKMAP;

				cur_mpino = MAP_INO(ty);
				/*
				 * If the previous map type is different the
				 * the current type then this if statement
				 * is busted - so be it.
				 */
				if (c_count == 1)
				{
					addr = cur_mp;
					value = get(addr, ty);
					type = OTHER;
					continue;
				}

				if (type == NUMB)
					cur_mpgno = addr;

				/*
				 * Get the address of the map page
				 */
				if (ltop (fd, &frag_addr.f, cur_mpino,
					  cur_mpgno) != LIBFS_SUCCESS)
				{
					printf(MSGSTR(MAPBLOCKRANGE,
						"map block out of range\n"));
					continue;
				}				

				cur_mp = FRAG2BYTE(frag_addr.f.addr);
				erraddr = addr;
				
				if ( ! (addr = get_mapsum (cur_mp, cur_mpino,
					cur_mpgno, 0)))
				{
					error++;
				}
				else
				{
					num_frags = 0;
					value = get(addr, ty);
					type = OTHER;
					continue;
				}
				continue;
			}
				
			/*
			 * i# to inode conversion
			 */
			case 'i':
			abs_addr = 0;
			/*
			 * if only 'i' entered then use current inode
			 */
			if (c_count == 1)
			{
				addr = cur_ino;
				value = get(addr, INODE);
				type = OTHER;
				continue;
			}
			if (type == NUMB)
				value = addr;
			if (value > imax && ! override)
			{
				printf (MSGSTR (INODERANGE,
						"inode out of range\n"));
				error++;
				continue;
			}

			/*
			 * Value should now contain an inode number.
			 * Get the address of the block containing the
			 * inode. Then use the block number to calculate
			 * the byte address of the inode. Nfrags is irrelavent.
			 */
			addr = get_inode_blk ((ino_t) value);
			/*
			 * convert block number to the inode address
			 */

			addr = addr * BLKSIZE + 
			    ((ino_t) value % INOPERBLK) * INODE;

			if (icheck (addr))
				continue;
			cur_ino = addr;
			value = get (addr, INODE);
			type = OTHER;
			continue;

			/*
			 * block conversion
			 * For 4.1 block numbers will be interpreted 
			 * as frag numbers. The user may enter
			 * a frag_t by specifying an address that
			 * sets nfrags to an appropriate value.
			 */
			case 'b':
			if (type == NUMB)
			{
				value = frag_addr.f.addr;
				num_frags = frag_addr.f.nfrags;
			}

			addr = FRAG2BYTE(value);
			value = get(addr, SHORT);
			type = OTHER;
			continue;

			/*
			 * directory offsets
			 */
			case 'd': 
			c = getc(stdin);
			if (!isdigit((int)c))
			{
				error++;
				continue;
			}
			ungetc((int)c, stdin);
			value = getnumb();
			if (error)
				continue;

			if (dircheck (addr, num_frags))
				continue;

			/*
			 * Get the address of the "value"th directory entry
			 */

			if ( ! dir_entry (fd, cur_ino, addr,
					      &dir_offset, &i_num, value))
			{
				printf (MSGSTR(INVALID_DIRENT,
					       "Invalid directory entry\n"));
				error++;
				continue;
			}
			
			value = i_num;
			addr = dir_offset;
			num_frags = 0;
			type = OTHER;
			objsz = DIRECTORY;
			continue;

			
			/*
			 * White space 
			 */
			case '\t':
			case ' ':
			case '.':
			continue;

			/*
			 * address addition
			 */
			case '+':
			c = getc(stdin);
			ungetc ((int)c, stdin);
			if (! isdigit((int) c))
				temp = 1;
			else
				temp = getnumb();
			if (error)
			{
				printf(MSGSTR(BADNUMBER,
					      "bad number\n")); continue;
			}
			if (objsz == DIRECTORY)
			{
				if (! dir_increment (fd, &addr, temp))
				{
					error++;
					c = '+';
					continue;
				}
				
			}
			else
				addr = addr + (temp * abs (objsz));

			value = get(addr, objsz); 
			continue;

			/*
			 * address subtraction
			 */
			case '-': 
			c = getc (stdin);
			ungetc((int)c, stdin);
			if (! isdigit ((int)c))
				temp = 1;
			else
				temp = getnumb();
			if (error)
			{
				printf(MSGSTR(BADNUMBER,
					      "bad number\n")); continue;
			}

			if (objsz == DIRECTORY)
			{
				if (! dir_decrement (fd, &addr, temp))
				{
					error++;
					c = '-';
					continue;
				}
			}
			else
				addr = addr - temp * abs(objsz);
			
			value = get(addr, objsz);
			continue;

			case '*': 
			temp = getnumb();
			if(error)
			{
				printf(MSGSTR(BADNUMBER,
					      "bad number\n")); continue;
			}
			addr = addr * temp;
			value = get(addr,objsz); 
			continue;

			case '/': 
			temp = getnumb();
			if (temp == 0 )
			{
			    error++;
			    printf(MSGSTR(DIVIDEBY,
					  "divide by 0\n"));
			    continue; 
			}
			if (error)
			{
				printf (MSGSTR(BADNUMBER,
					       "bad number\n"));
				continue;
			}
			addr = addr / temp;
			value = get(addr, objsz);
			continue;

			case 'q': /* quit */
			if (c_count != 1 || (c = getc(stdin)) != '\n')
			{
				error++;
				continue;
			}
			goodbye(0);


			/*
			 * save current address
			 */
			case '>':
			oldaddr = addr;
			oldvalue = value;
			oldobjsz = objsz;
			continue;

			/*
			 * restore saved address
			 */			
			case '<': 
			addr = oldaddr;
			value = oldvalue;
			objsz = oldobjsz;
			continue;

			/*
			 * access time, data block addresses
			 */
			case 'a': 
			saveaddr = addr;
			if((c = getc (stdin)) == 't')
			{
				addr = INODE_OFFSET (cur_ino, di_atime);
				type = OTHER;
				value = get (addr, LONG);
				continue;
			}
			/*
			 * Undocumented !	- get the agsize field
			 * from the the current map page.
			 *
			 */
			if (c == 'g')
			{
				erraddr = addr;
				if ( ! (addr = get_mapsum (cur_mp, cur_mpino,
					cur_mpgno,
					OFFSET(struct vmdmap, agsize))))
				{
					error++;
				}
				else
				{
					type = OTHER;
					value = get(addr, LONG);
				}
				continue;
			}

			/*
			 * data block addresses
			 */
			ungetc ((int)c, stdin);
			value = getnumb ();
			/*
			 * get inode from disk
			 */
			if (read_obj (fd, cur_ino, &itmp, INODE) < INODE)
			{
				printf (MSGSTR(READERROR1, "read error.\n"));
				error++;
				continue;
			}

			if ((value >= NDADDR && ! override) || 
			    value > NUMDADDRS(itmp))
			{
				printf (MSGSTR (DATABLOCKRANGE,
				"data block out of range: 0 <= %d < %d\n"),
				       value, NUMDADDRS(itmp));
				error++;
				continue;
			}
			addr = INODE_OFFSET(cur_ino, di_rdaddr[value]);
			value = get(addr, LONG);
			type = OTHER;
			continue;

			/*
			 * mt, md, maj, min, mf, ms, mw, mp
			 */
		case 'm': 
			saveaddr = addr;
			addr = INODE_OFFSET(cur_ino, di_mode);
			mode = get(addr, MODE);
			addr = cur_ino;

			if (error)
				continue;

			switch(c = getc(stdin))
			{
				/*
				 * map free count
				 * 
				 */
				case 'f':
				erraddr = addr;
				if ( ! (addr = get_mapsum (cur_mp, cur_mpino,
					cur_mpgno,
  				   OFFSET(struct vmdmap, freecnt))))
				{
					error++;
				}
				else
				{
					type = OTHER;
					value = get(addr, LONG);
				}
				continue;

				/*
				 * number of blocks covered by map
				 */
				case 's':
				erraddr = addr;
				if ( ! (addr = get_mapsum (cur_mp, cur_mpino,
					   cur_mpgno,
 				   OFFSET(struct vmdmap, mapsize))))
				{
					error++;
				}
				else
				{
					type = OTHER;
					value = get(addr, LONG);
				}
				continue;

				/*
				 * modification time
				 */
				case 't': 
				addr = INODE_OFFSET(cur_ino, di_mtime);
				type = OTHER;
				value = get(addr,LONG);
				continue;

				/*
				 * mode
				 */
				case 'd': 
				addr = INODE_OFFSET(cur_ino,di_mode);
				type = OTHER;
				value = get(addr,MODE);
				continue;

				/*
				 * major device number
				 */
				case 'a': 
				if((c = getc(stdin)) != 'j')
				{
					error++;
					continue;
				}
				if (devcheck(mode))
					continue;
				addr = INODE_OFFSET(cur_ino,di_rdev);
				value = major(get(addr,SHORT));
				type = OTHER;
				continue;

				/*
				 * minor device number
				 */
				case 'i': 
				if((c = getc(stdin)) != 'n')
				{
					error++;
					continue;
				}
				if(devcheck(mode))
					continue;
				addr = INODE_OFFSET(cur_ino,di_rdev);
				value = minor(get(addr,SHORT));
				type = OTHER;
				continue;

				/*
				 * mw - wmap, mp - pmap -- actual map data
				 */
				case 'p':
				case 'w':  
			    	{
					int word, ch, off2bitmap;
				    
					ch = c;
 					if (ch != 'w' && ch != 'p')
					{
						printf(MSGSTR(BADMAPTYPE1,
						   "bad map type {p,m}\n"));
						error++;
						continue;
					}
					c = getc(stdin);
					if (!isdigit((int)c))
					{
						printf(MSGSTR(WORDIDXREQ,
						  "word index required\n"));
						error++;
						continue;
					}
					ungetc((int)c,stdin);
					word = (int)getnumb();
					if (error)
						continue;
					
					if (word >= WPER_PAGE(map_ver))
					{
						printf(MSGSTR (BADWORDIDX,
						  "bad word index, 0<=n<%d\n"),
						       WPER_PAGE(map_ver));
						error++;
						continue;
					}

					erraddr = addr;
					if ( ! (addr =
						get_bitmap (cur_mp, cur_mpino,
							    cur_mpgno, ch,
							    word)))
					{
						error++;
					}
					else
					{
						value = get(addr, LONG);
						type = OTHER;
					}
					continue;
				}
			}
			error++;
			continue;

			/*
			 * file size
			 */
			case 's': 
			saveaddr = addr;
			switch(c = getc(stdin))
			{
				case 'z':
				addr = INODE_OFFSET (cur_ino, di_size);
				value = get (addr,LONG);
				type = OTHER;
				continue;

				default:
				error++;
				continue;
			}

			/*
			 * link count
			 */
			case 'l':
			saveaddr = addr;
			switch(c = getc(stdin))
			{
				/*
				 * link count
				 */
				case 'n':	
				addr = INODE_OFFSET(cur_ino,di_nlink);
				break;

				default:
				error++;
				continue;
			}
			value = get(addr, SHORT);
			type = OTHER;
			continue;

			/*
			 * group id
			 */
			case 'g': 
			saveaddr = addr;
			if ((c = getc(stdin))!= 'i' || (c=getc(stdin)) != 'd')
			{
				error++;
				continue;
			}
			addr = INODE_OFFSET(cur_ino, di_gid);
			value = get(addr, GID);
			type = OTHER;
			continue;

			/*
			 * user id
			 */
			case 'u':
			saveaddr = addr;
			if ((c = getc(stdin)) != 'i' || (c=getc(stdin)) != 'd')
			{
				error++;
				continue;
			}
			addr = INODE_OFFSET(cur_ino,di_uid);
			value = get(addr,UID);
			type = OTHER;
			continue;

			case 'r': /* directory record length */
			case 'n': /* directory name, name length */
			saveaddr = addr;
			if (c == 'r')
			{
				switch(c = getc (stdin))
				{
					/* rl = reclen */
					case 'l':	
					offset = OFFSET(direct_t, d_reclen);
					objsz = SHORT_NO_ALIGN;
					type = OTHER;
					break;

					default:
					error++;
					continue;
				}
			}
			else
			{
				switch(c = getc(stdin))
				{
					/* nm = name */
					case 'm':
					offset = OFFSET (direct_t, d_name[0]);
					objsz = DIRECTORY;
					type = OTHER;
					chname++;
					break;

					/* nl = name length */
					case 'l':   
					offset = OFFSET(direct_t, d_namlen);
					objsz = SHORT_NO_ALIGN;
					type = OTHER;
					break;

					default:
					error++;
					continue;
				}
			}

			/*
			 * check to make sure this block contains dir info
			 */
			if (dircheck (addr, num_frags))
				continue;

			/*
			 * round address to nearest dir-ent
			 */
			if (! dir_round_down (fd, cur_ino, &addr))
			{
				error++;
				continue;
			}
			addr += offset;
			continue;

			/*
			 * assignment operation
			 */
			case '=': 
			if (mounted)
			{
				/*
				 * discard all assignments
				 */
				while ((c = getc(stdin)) != '\n');
				addr = saveaddr;
				printf(MSGSTR(CANTMODIFY,
				"cannot modify mounted file systems\n"));
				error++;
				continue;
			}
			switch (c = getc(stdin))
			{
				/* 
				 * character string
				 */
				case '"': 
				if (objsz == DIRECTORY)
				{
					if (! dir_round_down (fd, cur_ino,
							      &addr))
					{
						error++;
						continue;
					}
					    
					addr += OFFSET(direct_t, d_name[0]);
				}
				puta (addr, num_frags);
				prt_flag = 1;				
				chname = 0;
				continue;

				/*
				 * =+ operator
				 */
				case '+': 
				temp = getnumb();
				value = get(addr, objsz);
				if (! error)
					put(addr, num_frags,
					    value + temp, objsz);
				continue;

				/*
				 * =- operator
				 */
				case '-': 
 				temp = getnumb();
				value = get(addr, objsz);
				if (! error)
					put(addr, num_frags,
					    value - temp, objsz);
				continue;

				/*
				 * nm and regular assignment
				 */
				default: 
				ungetc ((int)c, stdin);
				if (type == DIRECTORY && (! isdigit ((int)c)))
				{
					if (objsz == DIRECTORY)
					{
						/*
						 * make addr offset of name
						 */
						if (! dir_round_down (fd,
								      cur_ino,
								      &addr))
						{
							error++;
							continue;
						}
						addr +=
						OFFSET(direct_t, d_name[0]);
					}
					dbprintf(7, ("calling puta\n"));
					puta(addr, num_frags);
					prt_flag = 1;
					
					/*
					 * fix address to point to
					 * beginning of directory entry
					 */
					if (! dir_round_down (fd, cur_ino,
							      &addr))
					{
						error++;
					}
					continue;
				}

				/*
				 * Not a directory or it is a number
				 */
				value = getnumb();
				if (! error)
					put(addr, num_frags, value, objsz);
				continue;
			}

			/*
			 * shell command
			 */
			case '!': 
#ifdef STANDALONE
			printf (MSGSTR (SHELLNOTAVAIL,
					"Shell not available"));
			continue;
#else
			if (c_count != 1)
			{
				printf(MSGSTR(INVALIDSHELL,
					      "Invalid shell command.\n"));
				error++;
				continue;
			}
			if ((pid = fork()) == 0)
			{
				execl("/usr/bin/sh", "sh", "-t", 0);
				error++;
				continue;
			}
			while ((rpid = wait (&retcode)) != pid && rpid != -1);

			printf (MSGSTR (BANGNL,"!\n"));
			c_count = 0;
			prt_flag = 1;			
			continue;
#endif
			/*
			 * file print facility
			 */
			case 'f': 
			c = getc (stdin);
			/*
			 * Get the block number to print.
			 * Default to block 0. 
			 */
			if (isdigit ((int)c))
			{
				ungetc((int)c, stdin);
				temp = getnumb();
				if (error)
				{
					printf(MSGSTR (BADBLOCKNO,
						       "bad block number\n"));
					continue;
				}
				c = getc(stdin);
			}
			else 
				temp = 0;
			count = 0;
			addr = cur_ino;
			mode = get (INODE_OFFSET(cur_ino, di_mode), MODE);
			nlinks = get(INODE_OFFSET(cur_ino, di_nlink), SHORT);

			if (! override)
			{
				if (nlinks <= 0)
				{
					printf(MSGSTR(INODENOTALLOC,
					"warning: inode not allocated\n"));
				}
			}	

			if ((mode & IFMT) == IFCHR || (mode & IFMT) == IFBLK)
			{
				printf(MSGSTR(SPECIALDEV,"special device\n"));
				error++;
				continue;
			}
			/*
			 * get the inode from the disk for the ltop call
			 */
			if (read_obj (fd, addr, &itmp, INODE) < INODE)
			{
				printf(MSGSTR (READERROR1,
					       "read error.\n"));
				error++;
				continue;
			}
			
			if (ltop (fd, &frag_addr.f, &itmp, temp) != 0)
			{
				error++;
				continue;
			}
			num_frags = frag_addr.f.nfrags;
			addr = FRAG2BYTE(frag_addr.f.addr);
			/*
			 * Print the info (c holds the print mode)
			 */
			fprnt(c, 0, num_frags);
			continue;

			/*
			 * override flip flop
			 */
			case 'O': 
			if (override = !override)
			{
				printf (MSGSTR (ERRCHKOFF,
						"error checking off\n"));
			}
			else
			{
				printf(MSGSTR(ERRCHKON,"error checking on\n"));
				reinitialize(0);
			}
			prt_flag++;			
			continue;

			/*
			 * byte offsets
			 */
			case 'B':
			objsz = CHAR;
			continue;

		 	/*
			 * word offsets
			 */
			case 'W':
			objsz = SHORT;
			addr = addr & ~01;
			continue;

			/*
			 * double word offsets
			 */
			case 'D':
			objsz = LONG;
			addr = addr & ~01;
			continue;

			case 'A':
#ifdef STANDALONE
			goodbye(1);
#else
			abort();
#endif
			/*
			 * general print facilities
			 */
			case ',': 	
			case 'p':
			c = getc (stdin);
			if (isdigit ((int)c))
			{
				ungetc((int)c, stdin);
				count = getnumb();
				if (error)
					continue;
				if ((count < 0) || (count > BLKSIZE))
					count = 1;
				c = getc(stdin);
			}
			else
			{
				switch(c)
				{
					/* print to end of block */
					case '*':
					count = 0;	
					c = getc(stdin);
					break;

					default:
					count = 1;
					break;
				}
			}
			if (! abs_addr && (c == 'S' || c == 'D'))
			{
				addr = INODE_OFFSET (cur_ino, di_rindirect);
				value = get(addr, LONG);
				addr = BLK2BYTE(value);
				num_frags = 0;
			}
			else if (c == 'd')
				dir_raw_print (fd, &addr, num_frags, count);
			else
			{
				abs_addr = 1;
				fprnt(c, count, num_frags);
				abs_addr = 0;
			}
		}
	}
}


/*
 * NAME:	reinitialize
 *
 *
 * FUNCTION:	Reinitialize global data. If phase1 (ie this the
 *		first invocation) then allocate data structures
 *		and initialize bitmaps. 
 *
 * RETURN:	Exit if error else void
 *
 */

void
reinitialize (int phase1)
{
	struct superblock	sb;

	process_super(fd, &sb, dskname);
	get_map_inodes (phase1, sb.s_version);	
	load_sizes (&sb);
	
	if (phase1)
	{
		alloc_map_block_lists ();
		if (init_bitmaps() == -1)
			goodbye (1);
	}
}

/*
 * NAME:	get_map_inodes
 *
 * FUNCTION:	allocate and initialize inodes .INODES and .DISKMAP
 *
 * RETURN:	void
 *
 */

static void
get_map_inodes (int 	phase1,
		int	sb_vers)
{
	fdaddr_t	map_addr;
	struct vmdmap	vmd_map;

	/*
	 * Allocate space for  INOMAP_I and DISKMAP_I if this is the first call
	 */
	if (phase1)
	{
		dmi = (struct dinode *) malloc (INODE);
		imi = (struct dinode *) malloc (INODE);
	
		if (dmi == (struct dinode *) NULL ||
		    imi == (struct dinode *) NULL)
		{
			printf (MSGSTR (MALLOC,
				"malloc error. dmi=%x, imi=%x\n"), dmi, imi);
			goodbye(1);
		}

		cur_mpino = dmi;	
	}

	if (get_inode (fd, dmi, DISKMAP_I))
	{
		printf (MSGSTR(DISK_MAP,
		       "Cannot read disk map inode (inode number %d).\n"),
			DISKMAP_I);
		goodbye(1);
	}

	if (get_inode (fd, imi, INOMAP_I))
	{
		printf (MSGSTR(INODE_MAP,
		       "Cannot read inode map inode (inode number %d).\n"),
			INOMAP_I);
		goodbye(1);
	}

	/*
	 * Get the map version. Since the map manipulating mnemonics are
	 * of dubious value, it will not be consided a fatal error
	 * if the version number can not be attained; rather, the version
	 * will be assumed based on the superblock version.
	 */

	map_ver = sb_vers ? ALLOCMAPV4 : ALLOCMAPV3;

	if (ltop (fd, &map_addr.f, cur_mpino, 0) == LIBFS_SUCCESS)
	{
		if (bread (fd, &vmd_map, map_addr.f) == FRAGLEN(map_addr.f))
		{
			map_ver = vmd_map.version;
		}
	}
}

/*
 * NAME:	alloc_map_block_lists
 *
 *
 * FUNCTION:	Allocate space for a list of all blocks belonging to
 *		the inode map and disk map. *
 *
 * RETURN:	exit if error
 *
 */

static void
alloc_map_block_lists (void)
{
	imblocks = (daddr_t *) malloc((NUMDADDRS(*imi) + 1) * sizeof(daddr_t));
	dmblocks = (daddr_t *) malloc((NUMDADDRS(*dmi) + 1) * sizeof(daddr_t));

	if (imblocks == (daddr_t *) NULL ||
	    dmblocks == (daddr_t *) NULL)
	{
		printf (MSGSTR (MALLOC, "malloc error\n"));
		goodbye(1);
	}
}

/*
 * NAME:	load_sizes
 *
 *
 * FUNCTION:	Load various filesystem sizes. These are the
 *		basis for most error checking.
 *
 *
 * RETURN:	void
 *
 */

static void
load_sizes (struct superblock	*sb)
{
	fsize 	= 	sb->s_fsize;
	dsk_agsize = 	sb->s_agsize;
	ino_per_ag = 	sb->s_iagsize;	
	fsmax (&imax, &fmax);

	printf ("%-40s%12ld  %s", MSGSTR(FSIZEMSG, "\nFile System Size:"),
		fsize, MSGSTR(BLOCK512,"(512 byte blocks)"));
		
	printf ("%-40s%12u  %s", MSGSTR(DMSIZE, "\nDisk Map Size:"),
		dmi->di_size / BLKSIZE, MSGSTR(BLOCK4K, "(4K blocks)"));
		
	printf ("%-40s%12u  %s", MSGSTR(IMSIZE, "\nInode Map Size:"),
		imi->di_size / BLKSIZE, MSGSTR(BLOCK4K, "(4K blocks)"));	
		
	printf ("%-40s%12d  %s", MSGSTR (FRAGINFO, "\nFragment Size:"),
		sb->s_fragsize, MSGSTR(BYTES, "(bytes)"));

	printf ("%-40s%12d  %s", MSGSTR(AGINFO, "\nAllocation Group Size:"),
		dsk_agsize, MSGSTR(FRAGS, "(fragments)"));

	printf ("%-40s%12d",
		MSGSTR(INOINFO, "\nInodes per Allocation Group:"),
		ino_per_ag);	

	printf ("%-40s%12d", MSGSTR(TOTALINODE, "\nTotal Inodes:"), imax);	
	printf ("%-40s%12d\n\n", MSGSTR(TOTALFRAG,"\nTotal Fragments:"), fmax);	

}

/*
 * FUNCTION:	get_inode_blk
 *
 *	Return the disk block number containing the given inode.
 */

static	daddr_t
get_inode_blk (ino_t	inum)
{
	int ag;

	ag = inum / ino_per_ag;
	return ((ag ? FRAG2BLK(ag * dsk_agsize) : INODES_B) +
                INO2BLK(inum % ino_per_ag));
}

/* 
 * init_bitmaps:
 *
 *	initializes the bitmaps i_bitmap and s_bitmap 
 * 	called once at the beginning of the program.
 *
 *	RETURN:
 *		-1 for any failure
 *		0  for success
 */

static	int
init_bitmaps (void)
{
	int		maplen;
	fdaddr_t	frg;

	/*
	 * One bit for each frag in the filesystem
	 */
	maplen = (DEVBLK2FRAG(fsize) + sizeof(char) - 1) / sizeof(char);
	
	s_bitmap = (bitvec_t *) malloc (maplen);

	if (s_bitmap == (bitvec_t *) NULL)
	{
		printf(MSGSTR(MALLOC, "fsdb: malloc failed\n"));
		goodbye(1);
	}

	bzero (s_bitmap, maplen);
	/*
	 * mark the ipl block and the superblock as special
	 */
	dbprintf (1000, ("setting IPL_B and SUPER_B bits\n"));
	frg.d = BLK2FRAG(IPL_B);
	bit_man (SET, s_bitmap, frg.f);
	frg.d = BLK2FRAG(SUPER_B);	
	bit_man (SET, s_bitmap, frg.f);

	/*
	 * mark blocks that are inodes in i_bitmap and s_bitmap
	 */
	dbprintf (1000, ("marking .inodes blocks\n"));
	if (mark_inodes (s_bitmap) == -1)
		return(-1);

	dbprintf (1000, ("marking .diskmap blocks\n"));
	if (mark_blocks (dmi, ".diskmap", 0, 0, dmblocks) == -1)
		return (-1);

	dbprintf (1000,("marking .inomap blocks\n"));
	if (mark_blocks (imi, ".inomap", 0, 0, imblocks) == -1)
		return(-1);
	/*
	 * don't mark the blocks in the .indirect file as special:
	 * they could be directories...
	 */

	/* mark the blocks in the inode map as special.. */
	dbprintf(1000,("marking inode map blocks \n"));
	if (mark_blocks (imi, ".inodemap", s_bitmap, 0, 0) == -1)
		return(-1);

	/* mark the blocks in the diskmap as special */
	dbprintf (1000, ("marking disk map blocks\n"));
	if (mark_blocks (dmi, ".diskmap", s_bitmap, 0, 0) == -1)
		return(-1);

	return(0);
}

/*
 * mark_blocks():  read in the file represented by inode *ip, (it's filename
 * is 'fn' for error messages.  if any of bm1, bm2 or blist are set,
 * fill them in. bm1 and bm2 are bitmaps.  one bit for each block in the
 * filesystem, blist is a list of blocks of this file. assume that it's big
 * enough.  0 terminates the blist array (it's not a valid block for most
 * applications)
 *
 */

static	int
mark_blocks (struct dinode *ip,
	     char *fn,
	     long *bm1,
	     long *bm2,
	     daddr_t *blist)
{
	int 		i;
	long 		b;
	daddr_t		*lp;
	fdaddr_t	frag_addr;

	if (blist)
		lp = blist;
	else
		lp = 0;


	for (i = 0; i < NUMDADDRS(*ip); i++)
	{
		dbprintf (2000, ("\t\tcalling ltop(fd=%d,ip,i=%d,BLKSIZE)\n",
				fd, i));

		if (ltop(fd, &frag_addr.f, ip, i) != 0)
		{
			printf(MSGSTR (LTOPFAIL,
    	     "fsdb: mark_blocks: ltop failed for i-block #%d of %s file.\n"),
			       i, fn);
			return(-1);
		}

		dbprintf(2001,
		("\t\tltop returns phys block %x for logical block %d\n",
		 frag_addr.d, i));
		dbprintf(1000,("mark_blocks: setting bit %x\n", frag_addr.d));

		if (bm1)
			bit_man(SET, bm1, frag_addr.f);
		if (bm2)
			bit_man(SET, bm2, frag_addr.f);
		if (blist)
			*lp++ = frag_addr.d;
	}
	if (blist)
		*lp = 0;

	return(0);
}

static	int
mark_inodes (long *bm)
{
	int 		i;
	fdaddr_t	b;
	
    
	for (i = 0; i <= imax; i += INOPERBLK)
	{
		b.d = get_inode_blk(i);
		b.d = BLK2FRAG(b.d);
		bit_man(SET, bm, b.f);
	}
	return(0);
}

/*
 * NAME:	getnumb
 *
 * FUNCTION:	Read positive decimal, octal or hexidecimal numbers from
 *		stdin. Read until the first non base numeric character is
 *		located; push this character back. All numbers are assumed 
 *		to be positive. Any sign chraracters are to be read before
 *		calling this function.
 *		
 *		This getnumb() will return 0 for 09 because 9 is not a 
 *		digit of the octal base. 
 *		
 *
 * RETURN: 	if error
 *			-1
 *		else
 *			converted number 
 *
 */

offset_t
getnumb (void)
{

	int	base;
	int	offset = 0;
	char	num_buf[NUM_LEN];
	char	*char_set;
	char	*end;
	char	c;
	offset_t  number;	


	num_buf[0] = '\0';
		
	c = getc (stdin);
	if (! isdigit ((int)c))
	{
		error++;
		ungetc ((int)c, stdin);
		return (-1);
	}
	
	/*
	 * Get the number base and character set and
	 * read the first digit of the number.
	 */
	base = 10;
	char_set = DEC_CHARS;
	num_buf[offset++] = c;

	/*
	 * Check for ocatal or hex
	 */
	if (c == '0')
	{
		base = 8;
		char_set = OCT_CHARS;

		switch (c = getc (stdin))
		{
			case '\n':
			ungetc ((int)c, stdin);
			return (0);

			case 'x':
			case 'X':
			base = 16;
			char_set = HEX_CHARS;
			num_buf[offset++] = c;
			c = getc(stdin);
			/*
			 * 0x followed by any seperator 
			 * (ie '.', '\n', ' ', '\t') is interpreted as a
			 * zero followed by an 'x'. The seperator is ignored
			 * and the 'x' pushed back.
			 * This particular hack is constructed to handle the
			 * special case of the valid fsdb command 
			 * "f0x" which says print block 0 of the current inode
			 * as hex words. This results in 0x<non-hex-digit> 
			 * being interpeted as 0 and the <non-hex-digit> is
			 * pushed back. A more elegant solution to the
			 * the general class of problems that "f0x" falls
			 * into requires a rewrite of fsdb's lame 
			 * getc/ungetc input processing.
			 *
			 */
			switch (c)
			{
				case '\t':
				case ' ':
				case '.':
				case '\n':	
				ungetc ((int)'x', stdin);
				return (0);
			}
			break;

			default:
			break;
		}
	}
	else
		c = getc(stdin);

	/*
	 * Get one character at a time until an non char_set 
	 * character is read.
	 */
	do
	{
		if (index(char_set, c) != 0)
			num_buf[offset++] = c;
		else
		{
			ungetc ((int)c, stdin);
			break;
		}
	}
	while ((c = getc(stdin)) && offset < NUM_LEN - 1);

	num_buf[offset] = '\0';
	errno = 0;
	number = strtoll (num_buf, &end, base);

	return (errno ? -1 : number);
}

/*
 * get - read a byte, word or double word from the file system.
 * Inode and directory size requests result in 'long'
 * fetches.
 *
 *
 *	Side effects:
 *		- The global variable 'objsz' get set to the parameter
 *		'lngth'.
 *
 */

static	long
get (offset_t 	addr,
     short 	lngth)
{
	char	buf [BLKSIZE];
	char 	*bptr;
	off_t	offset;
	long 	vtemp;

	
	bptr = buf;
	/*
	 * Alter the global object size
	 */
	objsz = lngth;
	dbprintf (999, ("in get, calling align (%d)\n", lngth));
	
	if (align(lngth))
		return (-1);

	offset = BLOCK_OFFSET(addr);

	if (offset + abs(lngth) > BLKSIZE)
	{
		error++;
		printf (MSGSTR (GETTOOLONG,
				"get(addr=%llx, lngth=%d) - %lld too long\n"),
			addr, abs(lngth), offset + abs(lngth));
		return(0);
	}

	
	if (read_obj(fd, addr, buf, abs(lngth)) != abs(lngth))
	{
		printf(MSGSTR(READERROR2,"read error : 0x%2llx\n"), addr);
		error++;
		return(0);
	}

	dbprintf (1001,("bptr[0-4] = { %x %x %x %x } dec = { %d %d %d %d }\n",
	    bptr[0], bptr[1], bptr[2], bptr[3],
	    bptr[0], bptr[1], bptr[2], bptr[3]));

	switch(lngth)
	{
		case INOMAP:
		case DISKMAP:
		case DIRECTORY:
		case LONG_NO_ALIGN:
		case LONG:
		vtemp = *((long *)bptr);
		dbprintf(1000,("in get, returning %d\n", vtemp));
		return(vtemp);
		
		case INODE:
		case SHORT_NO_ALIGN:
		case SHORT: 
		vtemp = *((short *)bptr);
		vtemp = vtemp & 0xffff;
		dbprintf (1000, ("in get, returning %d\n", vtemp));
		return (vtemp);

		case CHAR: 
		vtemp = *bptr;
		vtemp = vtemp & 0xff;
		dbprintf(1000, ("in get, returning %d\n", vtemp));
		return(vtemp);
	}
	error++;
	printf(MSGSTR (GETINVALLEN,
 		       "get(addr=%llx, lngth=%d) - invalid length\n"),
	       addr, lngth);
	return(0);
}

/*
 * put - Write an item into the buffer for the current address
 * block.  The value is checked to make sure that it will
 * fit in the size given without truncation.  If successful,
 * the entire block is written back to the file system.
 */

static	int
put (offset_t	addr,
     int	nfrags,
     long	item,
     short	lngth)
{
	char		*sbptr;		/* start of block		*/
	char		*bptr;		/* start + offset to  object	*/
	char		*vptr;
	char		sblk[BLKSIZE];
	short 		offset;
	long		nbytes;
	fdaddr_t	frag_addr;


	/*
	 * Alter the global objsiz
	 */
	objsz = lngth;
	dbprintf(999, ("in put, objsz = %d, item = %d, lngth = %d\n",
		       objsz, item, lngth));

	RECONSTITUTE_FRAG(addr, nfrags, frag_addr);	

	if (align(objsz) || addr + lngth > FRAG2BYTE(fmax.addr) ||
	    ! adjust_nfrags (&frag_addr.f))
	{
		error++;
		return;
	}

	offset = FRAG_OFFSET(addr);
	if (bread (fd, sblk, frag_addr.f) < FRAGLEN(frag_addr.f))
	{
		error++;
		return;
	}

	sbptr  = (char *)sblk;
	if (offset + lngth > FRAGLEN(frag_addr.f))
	{
		error++;
		printf(MSGSTR(BLOCKOVERFLOW, "block overflow\n"));
		return(0);
	}

	bptr = sbptr + offset;
	switch (objsz)
	{
		case DIRECTORY:
		case DISKMAP:
		case INOMAP:
		case LONG_NO_ALIGN:
		case LONG: 
		*((long *)bptr) = item;
		goto rite;
		
		case INODE:
		case SHORT_NO_ALIGN:
		case SHORT: 
		if (item & ~0177777L)
			break;
		*(short *)bptr = item;
		goto rite;
		
		case CHAR: 
		if (item & ~0377)
			break;
		*bptr = lobyte(loword(item));

	rite:	;
		if (bwrite(fd, sbptr, frag_addr.f) != FRAGLEN(frag_addr.f))
		{
			error++;
			printf(MSGSTR(WRITEERROR1,
			      "write error : addr   = 0x%10.10llx\n"), addr);
			return(0);
		}
		/*
		 * Any read buffers (no write buffers are maintained) may
		 * now be invalid
		 */
		dirty_buffers();
		return;

		default: 
		error++;
		return;
	}
	printf (MSGSTR (TRUNCERROR, "truncation error\n"));
	error++;
}

/* 
 * dir_align(ment): check the alignment of an address to make sure it's on
 * directory-slot entry boundaries
 */
static	int
dir_align (offset_t	ment)
{
	
	if (! dir_load (fd, cur_ino, ment))
	{
		error++;
		return(0);
	}

	if (dir_check_entry (ment) < 0)
	{
		error++;		
		return (0);
	}

	return (1);
}

/*
 * align - before a get or put operation check the 
 * current address for a boundary corresponding to the 
 * size of the object.
 */
static	int
align (short 	ment)
{
	switch (ment)
	{
		case LONG: 
		if (addr & 03)
			break;
		return(0);
		/* 
		 * check alignment of directories
		 * depends on which dirslot, size, etc.
		 * tricky.
		 */
		case DIRECTORY:
		if (! dir_align (addr))
			break;
		return(0);

		case INODE:
		case SHORT: 
		if (addr & 01L)
			break;

		case DISKMAP:
		case INOMAP:
		if (addr == BLOCK_ADDR(addr))
			return(0);

		case CHAR: 
		case LONG_NO_ALIGN:
		case SHORT_NO_ALIGN:
		return(0);
	}
	error++;
	objsz = CHAR;
	printf (MSGSTR(ALIGNMENT, "alignment\n"));
	return (1);
}

/*
 * NAME:	puta 
 *
 * FUNCTION:	Put ascii characters into a buffer.  The string
 * 		terminates with a quote or newline.  The leading quote,
 * 		which is optional for directory names, was stripped off
 * 		by the assignment case in the main loop.  If the type
 * 		indicates a directory name, the entry is null padded to
 * 		d_reclen bytes.  If more than d_reclen characters have
 *		been given* with this type or, in any case, if a block overflow
 * 		occurs, the current block is not written out.
 *
 * RETURN:
 */

static  int
puta	 (offset_t	byte_addr,
	  int		nfrags)
{
	char		*bptr, c;
	char		*sbptr;
	char 		buf[BLKSIZE];
	int		cnt;	
	off_t		offset;
	unsigned	block;
	long		nbytes;
	daddr_t 	maxaddr;
	fdaddr_t	frag_addr;


	RECONSTITUTE_FRAG(addr, nfrags, frag_addr);	
	if (byte_addr > FRAG2BYTE(fmax.addr) || ! adjust_nfrags (&frag_addr.f))
	{
		error++;
		return;
	}

	/*
	 * Alter the global objsiz
	 */
	objsz = CHAR;

	dbprintf(6, ("Entering puta().. reading frag #0x%x\n", frag_addr.d));

	if (bread(fd, buf, frag_addr.f) != FRAGLEN(frag_addr.f))
	{
		error++;
		return;
	}

	sbptr = buf;
	cnt = 0;

	/*
	 * Get the offset from the begining of buf[] to the object
	 */
	if ((offset =
	     byte_addr - FRAG2BYTE(frag_addr.f.addr)) > FRAGLEN(frag_addr.f))
	{
		error++;
		return;
	}

	bptr = sbptr + offset;
	if (type == DIRECTORY || chname)
	{

		offset_t	tmp = byte_addr;
		direct_t 	*p;		
		
		if (! dir_round_down (fd, cur_ino, &tmp))
		{
			error++;
			return;
		}
		/*
		 * tmp is now the byte offset to the direct_t of interest.
		 * Let p point to the direct_t of interest inside the buffer.
		 */
		p = (direct_t *)((char *)sbptr +
				 (tmp - FRAG2BYTE(frag_addr.f.addr)));
		
		if (byte_addr != tmp + OFFSET(direct_t,d_name[0]))
		{
			printf(MSGSTR(ASCIIASSIGN,
				 "ascii assign in dirent's only to d_name\n"));
			error++;
		}
		maxaddr = FRAG_OFFSET (tmp + (p->d_reclen -
				(sizeof(direct_t) - (MAXNAMLEN + 1))));
	}
	else
		maxaddr = FRAGLEN(frag_addr.f);
	

	dbprintf(5,("maxaddr = %d (block offset)\n",maxaddr));

	while ((c = getc(stdin)) != '"')
	{
		if (offset++ == maxaddr)
		{
			error++;
			printf (MSGSTR (BLOCKOVERFLOW, "block overflow\n"));
			return;
		}
		if(c == '\n')
		{
			ungetc((int)c,stdin);
			break;
		}
		if(c == '\\')
		{
			switch(c = getc(stdin))
			{
				case 't': 	cnt++, *bptr++ = '\t'; 	break;
				case 'n': 	cnt++, *bptr++ = '\n'; 	break;
				case '0': 	cnt++, *bptr++ = '\0'; 	break;
				case '"': 	cnt++, *bptr++ = '"'; 	break;
				case '\'': 	cnt++, *bptr++ = '\''; 	break;
				case 'r': 	cnt++, *bptr++ = '\r'; 	break;
				case '?': 	cnt++, *bptr++ = '\?'; 	break;
				default: 	cnt++, *bptr++ = c; 	break;
			}
		}
		/*
		 * handle ^a -> ^z and ^@-^Z and ^?
		 */
		if (c == '^')
		{
			int b = getc(stdin);
			/* ^A -> ^Z */			
			if (isalpha(b))
			{
				b = (islower(b)?toupper(b):b);
				b = b - '@';
				cnt++, *bptr++ = b;
				break;
			}
			if (b == '?')
			{
				cnt++, *bptr++ = 127; 	/* ^? - del */
			}
			if (b == '@')
			{
				*bptr == 0; cnt++;	/* ^@ - nul */
			}
			else
			{
				cnt++,
				*bptr='^';
				ungetc((int)b, stdin);
			} /* just a circumflex */
		}
		else cnt++, *bptr++ = c;
	}

	dbprintf(5,("cnt = %d\n",cnt));
	if(chname)
		*bptr++ = '\0';

	if (type == DIRECTORY || chname)
	{
		for (; offset < maxaddr; *bptr++ = 0, offset++);
		printf(MSGSTR(REMEMBERTOFIX,
			      "remember to fix d_namlen (nl) to = %d\n"), cnt);
	}

	if (bwrite (fd, sbptr, frag_addr.f) != FRAGLEN(frag_addr.f))
	{
		error++;
		printf(MSGSTR(WRITEERROR1,
			      "write error : addr   = 0x%10.10llx\n"), addr);
		return;
	}
	dirty_buffers();
}

/*
 * fprnt - print data as characters, octal or decimal words, octal
 * bytes, directories or inodes	A total of "count" entries
 * are printed. A zero count will print all entries to the 
 * end of the current block.  If the printing would cross a
 * block boundary, the attempt is aborted and an error returned.
 * This is done since logically sequential blocks are generally
 * not physically sequential.  The error address is set
 * after each entry is printed.  Upon completion, the current
 * address is set to that of the last entry printed.
 */

static	int
fprnt (char	style,
       short	count,
       int	nfrags)
{
	struct vmdmap 	*vm;
	int 		page;
	char 		*cptr, *cp, buf[4096], *p;
	short 		*iptr, *tptr, offset;
	long  		*lptr;
	fdaddr_t	frag_addr;



	prt_flag++;	
	RECONSTITUTE_FRAG(addr, nfrags, frag_addr);
 	offset = FRAG_OFFSET(addr);	
	cptr = buf;

	if (abs_addr)
		nfrags = FragPerBlk - 1;
	
	if (bread(fd, cptr, frag_addr.f) != FRAGLEN(frag_addr.f))
	{
		printf(MSGSTR(READERROR2, "read error : 0x%2llx\n"), addr);
 	        error++;
		return;
	}

	erraddr = addr;
	err_num_frags = nfrags;

	switch (style)
	{
		/*
		 * print as inodes
		 */
		case 'i':
		if (! print_inode (addr))
			addr = erraddr;
		return;

		/*
		 * print as directories
		 */
		case 'd':
		if (dir_print (fd, cur_ino, addr, count) < 0)
			error++;
		else
		{
			type = OTHER;
			objsz = DIRECTORY;
		}
		return;

		/*
		 * print as characters
		 */
		case 'c':
		if (count == 0)
			count = FRAGLEN(frag_addr.f);
		if (offset + count > FRAGLEN(frag_addr.f))
			break;
		
		objsz	= CHAR;
		addr = print_chars (cptr + offset, addr, count);
		return;

		/*
		 * or as octal bytes
		 * or print as hex bytes
		 */
		case 'b':
		case 'y':
		if (count == 0)
			count = FRAGLEN(frag_addr.f);
		if (offset + count > FRAGLEN(frag_addr.f))
			break;
		objsz	= CHAR;
		cptr	= cptr + offset;
		for (i = 0; count--; i++)
		{
			if (i % 16 == 0)
			    printf(MSGSTR(ADDRFORM, "\n0x%10.10llx:  "),
				       addr);

			if (style == 'y')
				printf (MSGSTR (PCT22X, "%02.2x "),
					*cptr++ & 0377);
			else
				printf(MSGSTR(PCT44O, "%4.4o "),
				       *cptr++ & 0377);
			erraddr = addr;
			addr++;
		}
		addr--;
		putc('\n', stdout);
		return;

		/*
		 * print as octal words,
		 * print as hex words, or
		 * print as decimal words
		 */
		case 'o':
		case 'x':
		case 'e':
		addr =  addr & ~01;
		offset = offset >> 1;
		iptr = ((short *)cptr) + offset;

		if (count == 0)
			count = FRAGLEN(frag_addr.f) / 2 - offset;

		if (offset + count > FRAGLEN(frag_addr.f))
			break;

		objsz = SHORT;
		for(i = 0; count--; i++)
		{
			if (i % 8 == 0)
			    printf(MSGSTR(ADDRFORM, "\n0x%10.10llx:  "),
				       addr);

			if (style == 'o')
				printf (MSGSTR(PCT66O,
					       "  %6.6o"),*iptr++ & 0177777);
			else if (style == 'x')
			     printf(MSGSTR(PCT44X,
					   "  %4.4x"),*iptr++ & 0177777);
			else
			     printf(MSGSTR(PCT6D,
					   "  %6d"),*iptr++);
			erraddr = addr;
			addr = addr + 2;
		}
		addr = erraddr;
		putc('\n', stdout);
		/*
		 * end case o || x || e
		 */
		return;

		/*
		 * Print as Single or Double indirect
		 */
		case 'S':
		case 'D':
		addr = addr & ~03;
		offset = offset >> 2;
		lptr = ((long *)cptr) + offset;

		if (count == 0)
			count = (BLKSIZE / 4) - offset;

		if (offset + count > (BLKSIZE / 4))
			break;

		objsz = LONG;
		for (i = 0; count--; i++)
		{
			if (i % 4 == 0)
			    printf(MSGSTR(ADDRFORM, "\n0x%10.10llx:  "),
				       addr);

			printf("0x%-12.8lx", *lptr++);
			erraddr = addr;
			addr += 4;
		}
		addr = erraddr;
		putc ('\n',stdout);
		return;


		/*
		 * print as a disk map entry or
		 * print as an inode map entry
		 */
		case 'M':
		case 'I':
		vm = (struct vmdmap *) (cptr + offset);
		page = cur_mpgno;
		if (page == -1)
		{
			if (override)
			{
				printf(MSGSTR(MAPPAGEOFF,
					      "%s map page @ 0x%10.10llx\n"),
				       map_type(style), addr);
			}
			else
			{
				printf(MSGSTR (NOTAMAPPAGE,
				"not a %s map page\n"),map_type(style));

				error++;
				return;
			}
		}
		else
		{
			printf(MSGSTR (MAPPAGENUM,
				       "%s map page # %d (0x%10.10llx)\n"),
			       map_type(style), page, addr);
		}
		printf (MSGSTR(MAPFMT1,
		"mapsize (ms) = %ld   freecnt (mf) = %ld   agsize(ag) = %d\n"),
			vm->mapsize, vm->freecnt, vm->agsize);
		return;

		default:
		error++;
		printf(MSGSTR(NOSUCHPRINT,"no such print option\n"));
		return;

	}
	error++;
	printf(MSGSTR(BLOCKOVERFLOW,"block overflow\n"));
}

/*
 * NAME:	print_inode
 *
 *
 * FUNCTION:	Print the contents of the inode addressed by the byte
 *		offset inode_addr.
 *
 * RETURN:	- Return 0 if any errors
 *		- Return 1 if successful
 */

static int
print_inode (offset_t	inode_addr)
{
	struct dinode	inode;
	ino_t		inode_num;
	mode_t		mode;
	char		*p;
	long 		num_blks;
	int 		noindir;


	if (icheck(inode_addr))
		return(0);
	
	if (read_obj (fd, inode_addr, &inode, INODE) != INODE)
		return (0);


	/*
	 * Attempt to calculate the inode number. If this fails
	 * Print the inode address else print the calculated inode number.
	 */	
	if ((inode_num = calc_inode_num (inode_addr)) < 0)
	{
		printf(MSGSTR(INOFMT1,
			      "inode at 0x%.2llx\n\tmd: "), inode_addr);
	}
	else
	{
		printf (MSGSTR (INOFMT2,
				"i#: %6ld  md: "), inode_num);
	}	

	p = " ugtrwxrwxrwx";
	mode = inode.di_mode;

	switch (mode & IFMT)
	{
		case IFDIR:
		putc ('d',stdout);
		break;

		case IFCHR:
		putc ('c',stdout);
		break;

		case IFBLK:
		putc ('b',stdout);
		break;

		case IFREG:
		putc ('f',stdout);
		break;

		case IFIFO:
		putc ('p',stdout);
		break;

		case IFLNK:
		putc ('l',stdout);
		break;

		default:
		putc ('-',stdout);
		break;
	}

	for (mode = mode << 4; *++p; mode = mode << 1)
	{
		if(mode & HIBIT)
			putc (*p, stdout);
		else
			putc('-', stdout);
	}

	printf(MSGSTR(INOFMT3,"  ln:%5d  uid:%5d  gid:%5d"),
	       inode.di_nlink, inode.di_uid, inode.di_gid);

	printf(MSGSTR(INOFMT4,"  sz: %8lu\n"), inode.di_size);

	switch (inode.di_mode & IFMT)
	{
		case IFCHR:
		case IFBLK:
		printf(MSGSTR(INOFMT5, "maj:%3.3o  min:%3.3o\n"), 
			major(inode.di_rdev), minor(inode.di_rdev));
		break;

		case IFIFO:
		printf (MSGSTR(FIFO, "fifo\n"));
		break;

		default:
		if ((inode.di_mode & IFMT) == IFLNK &&
		    inode.di_size <= D_PRIVATE)
		{
			printf(MSGSTR (LINKEDTO,
				       "Linked to\t-> %s\n"), inode.di_rdaddr);
		}
		else
		{
			if (NOINDIRECT(NUMDADDRS(inode)))
			{
				for(i = 0; i < NDADDR; i++)
				{
					printf (MSGSTR(INOFMT6,
				     "a%d: 0x%-10.2x"),i,inode.di_rdaddr[i]);
					if (i == 3)
						putc('\n', stdout);
				}
			}
			else if (ISDOUBLEIND(NUMDADDRS(inode)))
			{
				printf(MSGSTR (BLKISDOUBLE,
				       "Double indirect block: 0x%-10.2x"),
				       inode.di_rindirect);
			}
			else
			{
				printf (MSGSTR (BLKISSINGLE,
					"Single indirect block: 0x%-10.2x"),
					inode.di_rindirect);
			}
			putc('\n',stdout);
		}
		break;
	}

	print_time (inode.di_atime, MSGSTR (INOFMT7, "at: "));
	print_time (inode.di_mtime, MSGSTR (INOFMT8, "mt: "));
	print_time (inode.di_ctime, MSGSTR (INOFMT9, "ct: "));
	putc('\n',stdout);

	return (1);
}

/*
 * NAME:	print_time
 *
 *
 * FUNCTION:	if possible print formatted string else print the time
 *		integer.
 *
 *
 * RETURN:	void
 *
 */

static void
print_time (time_t	thetime,
	   char		*header)
{
	struct tm	*time;
	char		tbuf [MAX_DATELEN];	


	printf ("%s", header);
	
	if ((time = localtime(&thetime)) == NULL)
	{
		printf ("%ul\n", thetime);
		return;
	}

	if (strftime (tbuf, MAX_DATELEN - 1,
		      MSGSTR(DATE_FMT, "%a %b %d %T %Y"), time))
	{
		printf ("%s\n", tbuf);
	}
	else
		printf ("%ul\n", thetime);		
}

/*
 * NAME:	print_chars
 *
 *
 * FUNCTION:	Make an attempt to interpret the given buffer as characters.
 *		If any the character is not printable or unrecognized as
 *		character then print \?.
 *
 *		This function replaces putf()
 *
 * RETURN:	- Return the number of bytes transversed.
 *
 */

static off_t
print_chars (char	*c,		/* pointer to start offset in  buffer */
	     offset_t	address,	/* byte offset in filesystem	*/
	     int	max_len)	/* length to print		*/
{
	int	char_len;	/* Length of the current character 	*/
	int	max_wchr_len;	/* Max length of the current char  	*/
	int	tot_len = 0;	/* Total lenth tranversed		*/
	wchar_t	wchr;


	while (tot_len < max_len)
	{
		if (tot_len % 16 == 0)
			    printf(MSGSTR(ADDRFORM, "\n0x%10.10llx:  "),
				   address);
       		/*
		 * Get the largest number of bytes that will have the
		 * the following property:
		 *	(<num-bytes> + <currrent-position>) < <max-length>
		 * This allows us to interpret the max number of bytes as
		 * multibyte characters without running past the end of
		 * the buffer.
		 */
		wchr = '\0';		
		max_wchr_len = max_len - tot_len;
		max_wchr_len =
			(max_wchr_len < MAX_MBYTE)  ?  max_wchr_len : MAX_MBYTE;

		/*
		 * Use a library function in to convert multibytes 
		 * to wide characters.
		 *
		 * 	If mbtowc returns < 0 then
		 *		this is not a character,
		 *		print \?, and
		 *		move 1 byte forward,
		 *
		 *	If mbtowc returns 0 then
 		 *		this is colud be a null character,
		 *		print \0 if it is null, and
		 *		move one byte formard,
		 *
		 *	Else
		 *		Handle some well known escape ASCII characters,
		 *		print printable characters,
		 *		and print all others as \?
		 */
		if ((char_len =
		     mbtowc (&wchr, (c + tot_len), max_wchr_len)) <= 0)
		{
			char_len = 1;
			if (wchr == '\0')
				fprintf (stdout, " \\0");			
			else
				fprintf (stdout, " \\?");
		}
		else
		{
			switch (wchr)
			{
				case '\\':
				fprintf (stdout, " \\");
				break;	

				/*
				 * BELL
				 */
				case '\007':
				fprintf (stdout, " \\a");
				break;
			
				case '\t':
				fprintf (stdout, " \\t");
				break;
				
				case '\n':
				fprintf (stdout, " \\n");
				break;

				case '\b':
				fprintf (stdout, " \\b");
				break;

				/*
				 * DEL
				 */
				case '\0177':
				fprintf (stdout, " \\?");

				/*
				 * ds, gs, rs, us
				 */
				case 0x1c:
				case 0x1d:
				case 0x1e:
				case 0x1f:
				fprintf (stdout, " \\s");
				break;
				
				default:
				if (iswcntrl(wchr))
					fprintf (stdout, " \\?");
				else
					fprintf (stdout, " %c ", wchr);
				break;
			}
		}
		tot_len += char_len;
		address += char_len;
	}
	putc ('\n', stdout);
	return (address - 1);
}		

/*
 * NAME:	calc_inode_num
 *
 * FUNCTION:	Given a byte offset calculate an inode number.
 *		The offset is assumed to be within an inode block. 
 *
 * RETURN:	If the calculated number appears to be valid return that
 *		number else return -1.
 *
 */

static	ino_t
calc_inode_num (offset_t	ino_addr)
{
	int		agnum;
	ino_t		base_inum;
	ino_t		offset_inum;
	ino_t		inum;
	offset_t	begin_ag;
	offset_t	offset2ino;

	/*
	 * Get the AG and its byte offset
	 */
	agnum	= ino_addr / (dsk_agsize * FragSize);
	begin_ag  = (agnum == 0) ? (offset_t) INODES_B * BLKSIZE :
		(offset_t) agnum * dsk_agsize * FragSize;
	/*
	 * Get the inode number of the first inode in the AG,
	 * and  get the offset w/in the AG to the inode.
	 */
	base_inum = agnum * ino_per_ag;
	offset2ino = ino_addr - begin_ag;
	/*
	 * Get the inode number w/in the AG
	 */
	offset_inum = (ino_t) offset2ino / INODE;

	inum = base_inum + offset_inum;
	return ((inum > imax || inum < 0) ? -1 : inum);
}

/*
 * NAME:	icheck
 *
 * FUNCTION:	Check to see if the given address falls in an inode
 *		allocation group.
 *
 * RETURN:	return 0 if the address is within and inode allocation group
 *		otherwise set error, print a message, and return 1.
 */

static  int
icheck (offset_t address)
{
	int		agnum;
	offset_t	ag_off;
	
	if (override)
		return(0);

	agnum	= address / (dsk_agsize * FragSize);
	ag_off  = (agnum == 0) ? (offset_t) INODES_B * BLKSIZE :
		(offset_t) agnum * dsk_agsize * FragSize;

	if (address < ag_off  + (offset_t)ino_per_ag * INODE)
		return (0);

    printf(MSGSTR (NOTANINODE, "not an inode\n"));
    error++;
    return(1);
}

/*
 * err - called on interrupts.  Set the current address
 * back to the last address stored in erraddr. Reset all
 * appropriate flags.  
 * These are "erased" by invalidating the buffer, causing
 * the entire block to be re-read upon the next reference.
 * A reset call is made to return to the main loop;
 */

static void
err(void)
{
#ifndef STANDALONE
	signal(2,err);
	addr = erraddr;
	error = 0;
	c_count = 0;

	prt_flag = 0;
	printf(MSGSTR(NLHOOKNL,"\n?\n"));
	fseek(stdin, 0L, 2);
	longjmp(env,0);
#endif
}

/*
 * devcheck - check that the given mode represents a 
 * special device. The IFCHR bit is on for both
 * character and block devices.
 */

static   int
devcheck (register short md)
{
	if(override)
		return(0);
	if((md & IFMT) == IFCHR || (md & IFMT) == IFBLK)
		return(0);

	printf(MSGSTR(NOTCHARORBLOCK,"not char or block device\n"));
	error++;
	return(1);
}

/*
 * NAME:	dircheck
 *
 * FUNCTION:	
 * v2 method is just check to see if the address is in range 
 * in v3, since the inodes and maps and stuff are scattered around the
 * disk, we must consult our little bitmap s_bitmap which has bits
 * set to 1 corresponding to disk blocks that are 'special' (i.e. not 
 * simple data blocks.
 * ---------------- old method ----------------------------- 
 * This means it is not in the I-list, block 0 or the super block.
 */

static  int
dircheck (offset_t	address,
	  int		numfrgs)	
	
{
	fdaddr_t	frag_addr;

	if (override)
		return (0);

	RECONSTITUTE_FRAG(addr, numfrgs, frag_addr);
	if (! bit_man (TEST, s_bitmap, frag_addr.f))
		return(0);

	error++;
	printf(MSGSTR(BLOCKINILIST,"block in I-list or diskmaps\n"));
	return(1);
}

/*
 * goodbye(): exit cleanly with 'code' as an exit status 
 */
void
goodbye (int code)
{
	if (fd != (-1))
		close(fd);
	exit(code);
}

/*
 * map_pageno(): Search the list of inode map blocks or the list of disk
 *	map blocks for the given block.
 *
 */

static int
map_pageno (int style,
	    daddr_t block)
{
	daddr_t *p, i;

	if (style == 'I' || style == INOMAP)
		p = imblocks;
	else
		p = dmblocks;
	for (i = 0; *p ; p++, i++)
	{
		if (block == *p)
		{
			return (i);
		}
	}
	return(-1);
}

/* 
 * map_type(): return a string describing what type of map: "disk" or "inode"
 *
 */

static  char *
map_type (int typ)
{
	switch(typ)
	{
		case 'M':
		return ("disk");

		case 'I':
		default:
		return ("inode");
	}
}

/*
 * NAME:	ismounted
 *
 * FUNCTION:	ismounted - see if a device is mounted
 *
 * NOTES:	Ismounted looks at the currently mounted filesystems
 *		and sees if the device in question is mounted.
 *
 * RETURN VALUE:	1 if the device is mounted, else 0
 */

static  int
ismounted (char *dev)
{
	int count, i, ct;
	struct vmount *vmt;
	static int mntsiz = 0;
	static char *mnttab = NULL;

	/*
	 * first time in, point to count
	 */
	if (mnttab == NULL)
	{
		mnttab = (char *) &count;
		mntsiz = sizeof(count);
	}

	/*
	 * loop till we have enuf mem to read it in ...
	 */
	while ((ct = mntctl(MCTL_QUERY, mntsiz, mnttab)) <= 0)
	{
		/*
		 * error?
		 */
		if (ct < 0)
		{
			perror(MSGSTR(MNTCTLFAIL, "mntctl failed"));
			return (0);
		}

		/*
		 * get the current size and either malloc or realloc
		 */
		mntsiz = *((int *) mnttab);
		mnttab = (mnttab == (char *) &count) ? malloc(mntsiz) :
			realloc(mnttab, mntsiz);
		if (mnttab == NULL)
			fprintf(stderr, MSGSTR(NOMEMORY, "Out of memory.\n"));
	}
	for (vmt = (struct vmount *) mnttab;
	     ct > 0 && strcmp(vmt2dataptr(vmt, VMT_OBJECT), dev) != 0 &&
	     ct > 0 && strcmp(vmt2dataptr(vmt, VMT_STUB), dev) != 0;
	     ct--, vmt = (struct vmount *) ((char *) vmt + vmt->vmt_length))
		;
	/*
	 * find it?
	 */
	if (ct > 0)
		return (1);

	return (0);
}

void
dirty_buffers ()
{
	obj_dirty();
	dir_dirty();

}

/*
 * NAME:	get_mapsum
 *
 * FUNCTION:	Return the disk byte address of a struct vmdmap element.
 *
 *		If it is a version 4 map then the summary map page is
 *		located for the given map logical block. The correct
 *		offset into mapsorsummary[] is calculated.
 *
 *		If it is a version 3 map then the variable 'mp_off' is the
 *		disk address of the map page of interest.
 *
 *
 * PARAMETERS:
 *	offset_t	mp_off		If map version 3 then this is the byte
 *					offset to map page of interest.
 *	struct dinode	*mpino		Disk inode of either inode or disk map.
 *	int		lpgno 		Logical map page number
 *	int		ctl_off		The offset from the begining of a struct
 *					vmdmap to the control information
 *					of interest.
 *
 * RETURNS:	Return the disk address of the field of interest.
 *		Return 0 for any error.
 *
 */

static offset_t
get_mapsum (offset_t		mp_off,
	    struct dinode	*mpino,
	    int			lpgno,
	    int			ctl_off)
{
	int		sum_page;	/* V4 map summary page for lpgno */
	off_t		vmd_off;
	fdaddr_t	map_addr;

	/*
	 * If this is a version 4 map then ignore the mp_off variable and
	 * calculate the actual summary page number based on the logical page
	 * number. Use ltop to get the address of this summary page.
	 *
	 * If this is version 3 map then the mp_off variable is the byte
	 * offset to the page of interest. In this case just add the
	 * given offset (ctl_off).
	 */

	if (map_ver == ALLOCMAPV4)
	{
		/*
		 * Get the summary page number for the given logical map page
		 * number. Translate it into a disk address.
		 */
		sum_page = SUM_PAGE(lpgno);
		if (ltop (fd, &map_addr.f, mpino, sum_page) != LIBFS_SUCCESS)
		{
			printf(MSGSTR(MAPBLOCKRANGE,
				      "map block out of range\n"));
			return (0);
		}
		
		/*
		 * Calculate the byte offset to summary information
		 * corresponding to the logical page number
		 */
		vmd_off = SUM_OFF(lpgno) + FRAG2BYTE(map_addr.f.addr);
	}		
	else
		vmd_off = mp_off;

	if (! override  &&  ((vmd_off & DEVBLKMASK) || ctl_off >= 512))
		return (0);

	 return (vmd_off + ctl_off);
}

/*
 * NAME:	get_bitmap
 *
 * FUNCTION:	Return the disk byte address of a bitmap word.
 *
 *		If it is a version 4 map then the actual map page number
 *		is calculated based on the logical page number entered by the
 *		user.
 *
 *		If itis version 3 map then 'mp_off' is the disk byte address
 *		to the begininig of the bit map of interest.
 *
 * PARAMETERS:
 *	offset_t	mp_off		If map version 3 then this is the byte
 *					offset to map page of interest. 
 *	struct dinode	*mpino		Disk inode of either inode or disk map.
 *	int		lpgno 		Logical map page number
 *	char		map_type	'w' if working map and 'p' if permanent
 *	int		word		The word number of the map.
 *
 * RETURNS:	Return the disk address of the word of interest.
 *		Return 0 for any error.
 *
 */

static offset_t
get_bitmap (offset_t		mp_off,
	    struct dinode	*mpino,
	    int			lpgno,
	    char		map_type,
	    int			word)
{
	int		phy_page;	/* V4 map page for lpgno 	*/
	fdaddr_t	map_addr;	/* disk address of map page	*/
	off_t		off2page;	/* byte offset to map page 	*/
	off_t		off2map;	/* byte offset to map w/in page	*/
	off_t		off2wrd;	/* byte offset to word w/in map	*/	
	
	/*
	 * If this is a version 4 map then ignore the mp_off variable and
	 * calculate the actual map page number based on the logical page
	 * number. Use ltop to get the address of this bit map page.
	 *
	 * Else this is a version 3 map thus the mp_off variable is the byte
	 * offset to the page of interest. 
	 */

	if (map_ver == ALLOCMAPV4)
	{
		/*
		 * Get the bit map page number for the given logical map page
		 * number. Translate it into a byte address.
		 */
		phy_page = LM2PM(lpgno);
		if (ltop (fd, &map_addr.f, mpino, phy_page) != LIBFS_SUCCESS)
		{
			printf(MSGSTR(MAPBLOCKRANGE,
				      "map block out of range\n"));
			return (0);
		}
		off2page =  FRAG2BYTE(map_addr.f.addr);

		/*
		 * Get the offset to the correct bit map.
		 */
		off2map = (map_type == 'w') ? WOFFV4 : POFFV4;
	}		
	else
	{
		off2page = mp_off;
		off2map = (map_type == 'w') ? WOFFV3 : POFFV3;
	}

	return (off2page + off2map + ((char) word  * BPERWRD));
}
