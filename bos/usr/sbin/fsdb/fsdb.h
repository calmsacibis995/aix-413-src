/*  static char sccsid[] = "@(#)22	1.7  src/bos/usr/sbin/fsdb/fsdb.h, cmdfs, bos411, 9428A410j 1/6/94 11:44:48"; */
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fsdb header file
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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/vmdisk.h>
#include <libfs/libfs.h>
#include "fsdb_msg.h"

/*
 * 	Define a macro to handle external variable declarations
 * 	for the various modules. The first #define in fsdb.c before
 * 	#include "fsdb.h" must  be #define IN_FSDB.C
 */

#ifdef	IN_FSDB.C
#define	_EXTERN_
#else
#define	_EXTERN_	extern
#endif


/*
 * 	Define macro and variable for messages
 */
_EXTERN_	nl_catd 	catd;
#define MSGSTR(Num,Str) catgets(catd,MS_FSDB,Num,Str)

/*
 * 	Functions shared between modules
 */
_EXTERN_	void		goodbye (int);
_EXTERN_	offset_t	getnumb (void);
_EXTERN_	void		dirty_buffers(void);

/*
 * 	Override error checking
 */
_EXTERN_	short		override;

/*
 * 	Vital filesystem info
 */
_EXTERN_	long		dsk_agsize;
_EXTERN_	long		ino_per_ag;
_EXTERN_	ino_t		imax;
_EXTERN_	frag_t		fmax;

/*
 * 	Directory addition and subtraction functions (see fsdb_dir.c)
 */
#define	dir_increment(num1, num2, sum)	dir_add(num1, num2, sum, '+')
#define	dir_decrement(num1, num2, diff)	dir_add(num1, num2, diff, '-')

/* 
 * 	debugging macros 
 */
#define dbprintf(lev,stuff)\
	if (d_level >= (lev)) printf("line %5d: ",__LINE__), printf stuff;
#define debug_level(lev)  (d_level > (lev))
_EXTERN_	int d_level;

/* 
 * 	macro to give the offset of a structure member within that structure 
 */
#define OFFSET(structname,member)     (long)(&(((structname *)0)->member))

#define INODE_OFFSET(inodeaddr,member)	\
	((inodeaddr)+OFFSET(struct dinode,member))

#define	HIBIT   0100000

/*
 *	pages summarized per v4 map summary block 
 */
#define	PGPER_SUMBLK	8
#define SUMBLK_MASK	0x7

/*
 * 	pages per v4 map group
 */
#define PGPER_SUMGP	9

/*
 *	size of summary information
 *	2**9 = 512 -> size of struct vmdmap summary information
 */
#define	VMD_SUMSHIFT	9

/*
 *	Given a logical page number locate the corresponding version 4
 *	summary page number.
 */
#define	SUM_PAGE(lpgno)		(((lpgno) / PGPER_SUMBLK) * PGPER_SUMGP)

/*
 *	Given a logical page number calculate the byte offfset to the
 *	corresponding summary information w/in the v4 summary map page.
 */
#define	SUM_OFF(lpgno)		(((lpgno) & SUMBLK_MASK) << VMD_SUMSHIFT)

/*
 *	Given a logical map page number calculate the corresponding v4
 *	physical map page number.
 */
#define	LM2PM(lpgno)	((lpgno >> 3) + 1 + lpgno)

/*
 *	Number of words in a bitmap by map version number
 */
#define WPER_PAGE(ver)	(ver ? WPERPAGEV4 : WPERPAGE)

/*
 *	Define the byte offset to each of the 4 bitmaps
 */
#define BPERWRD		4
#define	WOFFV4		0			/* Working map	*/
#define WOFFV3		LMAPCTL
#define POFFV4		WPERPAGEV4 * BPERWRD	/* Permanent map	*/
#define POFFV3		LMAPCTL + WPERPAGE * BPERWRD

/*
 *	Return the correct inode pointer given the DISKMAP or INOMAP
 *	types constants defined below.
 */
#define	MAP_INO(type)	(type == DISKMAP ? dmi : imi)

/* 
 * 	type constants     
 */
#define OTHER   	1
#define NUMB    	2
#define INODE		sizeof(struct dinode)
#define DIRECTORY 	16
#define DISKMAP		LMAPCTL
#define INOMAP		(-LMAPCTL)
#define LONG		sizeof(long)
#define LONG_NO_ALIGN	(-sizeof(long))
#define SHORT   	sizeof(short)
#define SHORT_NO_ALIGN  (-sizeof(short))
#define CHAR		sizeof(char)
#define	GID		sizeof(gid_t)
#define	UID		sizeof(uid_t)
#define	INUM		sizeof(ino_t)
#define	MODE		sizeof(mode_t)

/* 
 *  	Given a byte offset calculate the 4k block address and the byte offset
 *	into the 4k block.
 */
#define BLOCK_ADDR(a)    ((a) & (~BLKMASK))	/* addr of the start of blk a */
#define BLOCK_OFFSET(a)  ((a) & (BLKMASK))	/* offset of a into block     */

/*
 * 	Sets of valid digits see function getnumb()
 */
#define HEX_CHARS "0123456789abcdefABCDEF"
#define OCT_CHARS "01234567"
#define DEC_CHARS "0123456789"

/*
 * Mask to round up to 4 byte boundry
 */
#define	ROUND_UP4(x)	((x + 3) & ~0x3)

/*
 * 	Define the length of a buffer for reading numebers
 */
#define	NUM_LEN		128

/*
 * 	Get the byte offset form the nearest frag boundry.
 */
#define FRAG_OFFSET(bytes)	((bytes) & (FragMask))

/*
 * 	Define a macro to reconstruct a frag_t from its parts.
 * 	This is needed because fsdb keeps all addresses in bytes.
 * 	This is also bogus because it will not always reconstruct the
 * 	true frag, but rather a frag_t that starts at the nearest frag boundry
 * 	and extends for n_frags. Thus it only reconstructs the original
 *	frag if the byte offset is in the first fragment of the fraged-block.
 */
#define	RECONSTITUTE_FRAG(byte_addr, n_frags, result_frag)	\
	result_frag.f.addr 	= BYTE2FRAG(byte_addr);\
	result_frag.f.nfrags 	= n_frags;\
	result_frag.f.new	= 0;

/*
 *	Bit vector jnk
 */

typedef unsigned long bitvec_t;

#define BIT_MASK (0x0000001f)		/* mask to set off bits in a long */

#define BIT_SHIFT 5			/* to divide by 32 */

#define bit_set(vec,bit) ((vec)[(bit)>>BIT_SHIFT] |= (1<<((bit)&BIT_MASK)))
#define bit_clr(vec,bit) ((vec)[(bit)>>BIT_SHIFT] &= (!(1<<((bit)&BIT_MASK))))
#define bit_not(vec,bit) ((vec)[(bit)>>BIT_SHIFT] ^= (1<<((bit)&BIT_MASK)))
#define bit_tst(vec,bit) ((vec)[(bit)>>BIT_SHIFT]&(1<<((bit)&BIT_MASK)))

#define	SET	1
#define TEST	2
#define	DUMP	3

/*
 *	Define the max number of bytes in a multibyte character
 */
#define	MAX_MBYTE	4

/*
 *	Define the max length of a date string
 */
#define	MAX_DATELEN	128
