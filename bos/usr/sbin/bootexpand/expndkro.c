static char sccsid[] = "@(#)65	1.9  src/bos/usr/sbin/bootexpand/expndkro.c, bosboot, bos411, 9431A411a 8/1/94 17:08:28";
/*
 *   COMPONENT_NAME: BOSBOOT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <sys/iplcb.h>
#include <sys/ramdd.h>
#include <sys/systemcfg.h>
#include <sys/types.h>


/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
typedef long int	code_int;
typedef long int	count_int;
typedef	unsigned char	char_type;

int decompress(void);
code_int getcode(void);
int fileread(char *ptr, int nbytes);
int putbyte(char c);
int getbyte(void);
char * bump(char *ptr, int bit_size);
char * find_hole(char *ptr, int bit_size, int target);

/*
 * decompression part of compress.c tailored for use in expanding
 * compressed kernel + boot filesystem.
 */

#define BITS		16	/* number of bits to use for compression */
#define HSIZE		69001			/* size of hash table */


#define	MAX_KER_SIZE	2*1024*1024


char_type magic_header[] = {'\037', '\235'};	/* 1F 9D */

					/* Defines for third byte of header */
#define BIT_MASK	0x1f
#define BLOCK_MASK	0x80		/* Masks 0x40 and 0x20 are free. */
#define INIT_BITS 9			/* number of bits/code */
#define STARTMASK	0x80000000

int n_bits;					       /* number of bits/code */
int maxbits = BITS;			/* user settable max # bits/code */
code_int maxcode;				/* maximum code, given n_bits */
code_int maxmaxcode = 1 << BITS;	/* should NEVER generate this code */
#define MAXCODE(n_bits)	((1 << (n_bits)) - 1)

count_int *htab ;					/* hash table */
unsigned short *codetab;					/* code table */

#define tab_suffixof(i)	((char_type *)(htab))[i]
#define de_stack		((char_type *)&tab_suffixof(1<<BITS))

code_int free_ent = 0;					/* first unused entry */


/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
int block_compress = BLOCK_MASK;
int clear_flg = 0;

/*
 * the next two codes should not be changed lightly, as they must not
 * lie within the contiguous general code space.
 */
#define FIRST	257					/* first free entry */
#define	CLEAR	256				/* table clear output code */


/*
 * stuff added to compress.c for this program follows.
 */

int	beginboot;
int	endboot;
int	startbase ;	/* start of savebase area */
int	endbase ;	/* end of savebase area */
int	endimage;	/* end of complete boot image */
char *	inptr;
char *	outptr;
int	noffset;
int	nsize;

int header_count;
int *ram_disk_point;
int *ram_end_point;
int *sbase_point;
int beginout;
int kernel_size;
int *map_ptr;
int bitsize;

#define PAGESIZE 4096
#define MAX_HEADER 1000			/* maximium size of kernel header */
#define RAM_DISK_START	0xc		/* offset to start of ram disk	*/
#define RAM_DISK_END	0x10		/* offset to end of ramdisk	*/
#define	EXPAND_MEMORY	0x00010000	/* bit indicating bad memory has been
					fixed */
#define	EXPAND_BCFG	0x00020000	/* bit indicating base config has been
					fixed */
char	ram_magic[] = RAM_MAGIC;
/*
 * xtoc_header declaration taken from mk3.5.c.
 */
struct xtoc_header {	/* see filehdr.h */
		unsigned short f_magic;
		unsigned short f_nscns;
		long	f_timdat;
		long	f_symptr;
		long	f_nsyms;
		unsigned short f_opthdr;
		unsigned short f_flags;
			/*see aouthdr.h*/
		short	o_mflag;
		short	o_vstamp;
		long	o_tsize;
		long	o_dsize;
		long	o_bsize;
		long	o_entry;
		long	skip3[3];
		short	sskip1;
		short	o_sntext;
		short	sskip6[6];
		unsigned long skip6[6];
		struct {
			long skip5[5];
			long s_scnptr;
			long skip4[4];
		} scnhdr[1];
};

/* The following is a global variable used by hacked-up
 * versions of the integer millicode used only by
 * bootexpand.  It has the same meaning as
 * _system_configuration.architecture.
 */

int system_architecture;

/*****************************************************************
 *
 * main program for expanding compressed kernel + bootfs.
 * normally this program does NOT return to caller. after
 * successful expansion, control is passed to code which
 * moves kernel to zero and calls main program of si.
 *
 * Algorithm from "A Technique for High Performance Data Compression",
 * Terry A. Welch, IEEE Computer Vol 17, No 6 (June 1984), pp 8-19.
 *
 * Algorithm:
 *	Modified Lempel-Ziv method (LZW).  Basically finds common
 * substrings and replaces them with a variable size code.  This is
 * deterministic, and can be done on the fly.  Thus, the decompression
 * procedure needs no input table, but tracks the way the table was built.
 */

main(int iplcbptr, int *bit_point, int bit_size, int map_size, int aiplcbptr)
/*
 * int iplcbptr;	address of ipl control block
 * int *bit_point;	pointer to start of the ram bit map
 * int bit_size;	number of byte per word
 * int map_size;	size in words of ram bit map
 * int aiplcbptr;	address of auxiliary ipl control block, used for
 *				ROS Emulation boot, or 0 if normal boot
 */
{
	int *targ,*source;
	int entry_point, text_start;
	struct xtoc_header * ph;
	struct ipl_cb	*iplcb_ptr;		/* ros ipl cb pointer */
	struct ipl_info	*info_ptr;		/* info pointer */
	struct ipl_directory *dir_ptr;		/* ipl dir pointer */
	int mach_model;
	struct processor_info *proc_ptr;	/* processor info pointer */

	/* Get addressability to iplinfo structure, and store away the
	 * architecture information.  This is stolen from hardinit.c.
	 * system_architecture is used by the special version of the
	 * integer millicode used by the compiler for certain operations
	 * such as multiply and divide which are different on POWER and
	 * PowerPC.
	 */

	iplcb_ptr = (struct ipl_cb *)iplcbptr;
	dir_ptr = &(iplcb_ptr->s0);
	info_ptr = (struct ipl_info *)((unsigned int) iplcb_ptr +
		dir_ptr->ipl_info_offset);
	mach_model = info_ptr->model;

	if (mach_model & 0x08000000)
	{
		system_architecture = POWER_PC;
	}
	else
	{
		system_architecture = POWER_RS;
	}

	/* establish pointer to begin of the kernel and the
	 * bootfs (beginboot) and the end of the same thing
	 * (endboot. [beginboot,endboot] is what we want to
	 * expand. the code assumes mk3.5 put at the magic
	 * locations 0xc and 0x10 the addresses of a "bootfs"
	 * which in this case is the compressed kernel+ the
	 * real bootfs.
	 */

	beginboot = *(int *)(0xC);
	endboot = *(int *)(0x10);
	startbase = *(int *)(0x18);
	endbase = *(int *)(0x1C);
	inptr = (char *) beginboot;
	bitsize = bit_size;

	/*
	 * set endimage to point to the end of the boot image
	 */
	if( endbase == startbase )
		endimage = endboot;
	else
		endimage = endbase;

	/*
	 * Find enough good memory to place the hash table, the code
	 * table, and the kernel.  Start searching at the page alignment
	 * that follows the end of the boot image in memory.
	 */
	map_ptr = bit_point;

	beginout = ((endimage + PAGESIZE - 1) & (~(PAGESIZE-1)));
	outptr = (char *) beginout;

	/*
	 * need to find enough space for the maximum kernel size plus
	 * space for the two tables rounded up to the next page boundary
	 */
	outptr = find_hole(outptr,bit_size, (MAX_KER_SIZE +
		((((HSIZE*sizeof(count_int)) + 2*HSIZE)+ PAGESIZE - 1)
			& (~(PAGESIZE-1)))));
	htab = (count_int *) outptr;
	codetab = (unsigned short *) ((int)htab + HSIZE*sizeof(count_int));


	/*
	 * Find enough good memory to decompress the kernel.  Start
	 * searching at the page alignment that follows the end of
	 * the end of the code table.
	 */
	beginout = ((((int)codetab + 2*HSIZE) + PAGESIZE - 1) &
			(~(PAGESIZE-1)));
	outptr = (char *) beginout;

	header_count = MAX_HEADER;

	/*
	 * we will find out kernel size after the header has been decompressed
	 */
	kernel_size = 0;


	/* start of decompression code from compress.c
	 */
	if ((getbyte()!=(magic_header[0] & 0xFF))
		|| (getbyte()!=(magic_header[1] & 0xFF)))
			return (-1);

	maxbits = getbyte();	/* set -b from file */
	block_compress = maxbits & BLOCK_MASK;
	maxbits &= BIT_MASK;
	maxmaxcode = 1 << maxbits;
	if(maxbits > BITS) {
		return(-1);
	}

	/* decompress it.
	 */
	decompress();

	/* get address of address of text_start from xtoc header
	 */

	ph = (struct xtoc_header * )beginout;
	if (ph->f_magic != 0637 && ph->f_magic != 0737)
		return(-2);
	text_start = (int)ph->scnhdr[ph->o_sntext - 1].s_scnptr + beginout;

	/* indicate begin and end of bootfs in the kernel locations */

	*(int *) (text_start + 0xc) = (int)ram_disk_point; /* begin bootfs */
	*(int *) (text_start + 0x10) = (int)ram_end_point; /* end bootfs */
	*(int *) (text_start + 0x14) |= EXPAND_MEMORY; /* bad memory has
							* been accounted for */
	if( startbase != endbase ) {
	*(int *) (text_start + 0x18) = (int) sbase_point; /* begin savebase */
	*(int *) (text_start + 0x1C) = (int) outptr; /* end savebase */
	*(int *) (text_start + 0x14) |= EXPAND_BCFG; /* bad memory has
							* been accounted for */
	}
	else {
	*(int *) (text_start + 0x18) = (int) 0; /* begin savebase */
	*(int *) (text_start + 0x1C) = (int) 0; /* end savebase */
	}

	/* move the header to location 0. this overlays expand.s
	 * and start1.s and stack but it is ok since we don't return.
	 */
	source = (int *)beginout;
	for (targ = 0; source < (int *)text_start; targ++,source++)
	{
		*targ = *source;
	}

	/* flush caches - this is done because after expanding the kernel   */
	/*   we have kernel code in the data cache. On machines with        */
	/*   separate instruction and data caches, this can cause problems. */

	kernel_size = ram_disk_point - (int *)text_start;
	if (system_architecture == POWER_PC) {
		proc_ptr = (struct processor_info *)((char *) iplcb_ptr +
				dir_ptr->processor_info_offset);
		vm_cflush_ppc_splt(text_start, kernel_size,
				proc_ptr->icache_block, proc_ptr->dcache_block);
	}
	else
		vm_cflush_pwr(text_start, kernel_size);
	

	/* go to entry point which will move kernel to right place and
	 * transfer control to main system init code.
	 */

	entry_point = (int)ph->o_entry + text_start;
	if (aiplcbptr)		/* ROS Emulation boot */
		(* (void (*)()) (&entry_point) ) (-1,iplcbptr,aiplcbptr);
	else
		(* (void (*)()) (&entry_point) ) (iplcbptr,0,0);

}


char_type lmask[] = {0xff, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};
char_type rmask[] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};


/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

int decompress(void) {
    char_type *stackp;
    int finchar;
    code_int code, oldcode, incode;

    /*
     * As above, initialize the first 256 entries in the table.
     */
    maxcode = MAXCODE(n_bits = INIT_BITS);
    for ( code = 255; code >= 0; code-- ) {
	codetab[code] = 0;
	tab_suffixof(code) = (char_type)code;
    } /* end for */
    free_ent = ((block_compress) ? FIRST : 256 );

    finchar = oldcode = getcode();
    if(oldcode == -1)					/* EOF already? */
	return(0);					/* Get out of here */
    putbyte( (char)finchar );		/* first code must be 8 bits = char */
    stackp = de_stack;

    while ( (code = getcode()) > -1 ) {

	if ( (code == CLEAR) && block_compress ) {
		for ( code = 255; code >= 0; code-- )
			codetab[code] = 0;
		clear_flg = 1;
		free_ent = FIRST - 1;
		if ( (code = getcode ()) == -1 )	/* O, untimely death! */
		break;
	} /* end if */
	incode = code;
	if ( code >= free_ent ) {	/* Special case for KwKwK string. */
		*stackp++ = finchar;
		code = oldcode;
	} /* end if */

	while(code >= 256) { /* Generate output characters in reverse order */
		*stackp++ = tab_suffixof(code);
		code = codetab[code];
	} /* end while */
	*stackp++ = finchar = tab_suffixof(code);

	do			/* And put them out in forward order */
		putbyte( *--stackp );
	while ( stackp > de_stack );
	if( !kernel_size && !header_count ) break;	/* End of kernel - do
						not decompress ram disk*/

	if ( (code=free_ent) < maxmaxcode ) {	/* Generate the new entry. */
		codetab[code] = (unsigned short)oldcode;
		tab_suffixof(code) = finchar;
		free_ent = code+1;
	} /* end if */
	oldcode = incode;			/* Remember previous code. */
    } /* end while */
	/* copy ram disk as is */
	while(((long) outptr & 0xfff) != 0)
		outptr=bump(outptr, bitsize);
	ram_disk_point = (int *)outptr; /* set address to start of RAM disk */

/*
	Following is the adjustment to inptr to determine the actual end of the
	kernel and the beginning of the ram file system. Decompress reads
	"nsize" number of bytes at a time from input, so depending on where
	the kernel ends, give back the remaining bytes which belongs to the
	ram file system, and and align it on a 4 K boundary.
*/
	inptr = inptr - nsize + (noffset >> 3) +
		((noffset -((noffset>>3)<<3) == 0 ) ? 0:1);

	/* Search for ram magic number to ensure inptr points to
		start of compressed RAM disk */
	while(1) {
		if( *inptr == ram_magic[0] ) {
			if( *(inptr+1) == ram_magic[1] &&
				*(inptr+2) == ram_magic[2] &&
				*(inptr+3) == ram_magic[3] )
					break;
		}
		inptr++;
	}

	while( inptr <= (char *) endboot ) {
		*outptr= *inptr++;
		outptr=bump(outptr,bitsize);
	}

	ram_end_point = (int *)outptr;
	if( ! endbase ) return(0); /* no savebase area */

	while(((long) outptr & 0xfff) != 0)	/* ensure page alignment */
		outptr=bump(outptr, bitsize);
	sbase_point = (int *) outptr; /* set address to start of savebase */
	inptr = (char *) startbase;	/* switch input to start of savebase */

	while((finchar=getbyte()) > -1 ) { /* copy data */
		*outptr=finchar;
		outptr=bump(outptr,bitsize);
	}

} /* end decompress */

/*****************************************************************
 * TAG( getcode )
 *
 * Read one code from the standard input.  If EOF, return -1.
 * Inputs:
 *	stdin
 * Outputs:
 *	code or -1 is returned.
 */

code_int
getcode(void) {
    code_int code;
    static int offset = 0, size = 0;
    static char_type buf[BITS];
    int r_off, bits;
    char_type *bp = buf;

    if ( clear_flg > 0 || offset >= size || free_ent > maxcode ) {
	/*
	 * If the next entry will be too big for the current code
	 * size, then we must increase the size.  This implies reading
	 * a new buffer full, too.
	 */
	if ( free_ent > maxcode ) {
		n_bits++;
		if ( n_bits == maxbits )
			maxcode = maxmaxcode;	/* won't get any bigger now */
		else
			maxcode = MAXCODE(n_bits);
	} /* end if */
	if ( clear_flg > 0) {
		maxcode = MAXCODE (n_bits = INIT_BITS);
		clear_flg = 0;
	} /* end if */
	size = fileread( buf,n_bits );
	nsize = size;				/* remember size */
	if ( size <= 0 )
		return -1;				/* end of file */
	offset = 0;
			/* Round size down to integral number of codes */
	size = (size << 3) - (n_bits - 1);
    } /* end if */
    r_off = offset;
    bits = n_bits;
    bp += (r_off >> 3);			/* Get to the first byte. */
    r_off &= 7;
    code = (*bp++ >> r_off);		/* Get first part (low order bits) */
    bits -= (8 - r_off);
    r_off = 8 - r_off;				/* now, offset into code word */
		/* Get any 8 bit parts in the middle (<=1 for up to 16 bits). */
    if ( bits >= 8 ) {
	code |= *bp++ << r_off;
	r_off += 8;
	bits -= 8;
    } /* end if */
    code |= (*bp & rmask[bits]) << r_off;		/* high order bits. */
    offset += n_bits;
    noffset = offset;		/* remember offset */
    return code;
} /* end of getcode */

int
fileread(char *ptr, int nbytes)
{
	char * outp;
	int	nb;

	/*
	 * moves up to nbytes to buffer from input. returns
	 * number of bytes moved. updates inptr by number
	 * of bytes moved.
	 */

	/* Need code to ensure *inptr++ is in good memory */

	nb = 0;
	for (outp = ptr; ((int)inptr < endimage) && (nb < nbytes);
		outp++,inptr++)
	{
		nb += 1;
		*outp = *inptr;
	}

	return (nb);
}

int putbyte(char c)
{
	static int text_start;
	struct xtoc_header *ph;


	/*
	 * check if we are expanding the kernel header
	 */
	if( header_count ){
		/*
		 * when header has been decompressed, get the size of the
		 * kernel by looking at the ram disk start address in
		 * the kernel low memory
		 */

		/* write byte to output buffer */
		*outptr++ = c;

		header_count--;
		if( !header_count ){
		/* get address of address of text_start from xtoc header */
			ph = (struct xtoc_header * )beginout;
			text_start = (int)ph->scnhdr[ph->o_sntext - 1].s_scnptr
				+ beginout;
			kernel_size = *(int *)( text_start +
						RAM_DISK_START) - MAX_HEADER;
		}
	}
	else {
		/*
		 * check if we are expanding the kernel
		 */
		if( kernel_size ){

			/* write byte to output buffer */
			*outptr++ = c;
			kernel_size--;
		}
		else {
			/*
			 * writes byte to output buffer.
			 */
			*outptr = c;


			/*
			 * ensure *ptr++ is in good memory
			 * we only need to do this when kernel is completed
			 * since we already found contiguous good memory
			 * for kernel
			 */
			outptr=bump(outptr,bitsize);
		}

	}
	return(0);
}

int
getbyte(void)
{

	/*
	 * gets next byte from input and increments inptr
	 * returns -1 if past end of input.
	 */

	if ((int)inptr <= endimage)
		return(*inptr++);

	return(-1);
}

char * bump(char *ptr, int bit_size)
{
	int	bit_pos;
	int	*mptr;

	/* ensure *ptr++ is in good memory */

	/* do we need to look at next bit in the bit map? */
	if (((int)(ptr + 1) % bit_size) == 0)
	{

		/* find the position in the bit map that corresponds to what
			ptr points to */
		/* starting with appropriate word in bit map */
			/* check whether bit in that word says good or not */
			/* check next bit if not */

		bit_pos = (int)(ptr+1)/bit_size;
		mptr = map_ptr + bit_pos/32;
		bit_pos %= 32;
		for (;(*mptr & (STARTMASK>>bit_pos));) {
			ptr += bit_size;
			bit_pos = (int)(ptr+1)/bit_size;
			mptr = map_ptr + bit_pos/32;
			bit_pos %= 32;
		}

	}

	return(ptr+1);
}

char * find_hole(char *ptr, int bit_size, int target)
{
	int	bit_pos;
	int	hole_size;
	int	*mptr;

	/* find a hole in memory to expand the kernel */


	/* look at the bit corresponding to what ptr points to */
	bit_pos = (int)ptr/bit_size;
	mptr = map_ptr + bit_pos/32;
	bit_pos %= 32;

	/* repeat until we have a big enough hole */
	for (hole_size=0;hole_size < target;) {

		/* if next bit is bad, start over */
		if (*mptr & (STARTMASK>>bit_pos)) {

			ptr += hole_size + bit_size;

			/* must start on page boundary */
			ptr = (char *)(((int)ptr+PAGESIZE - 1) &
				(~(PAGESIZE-1)));
			bit_pos = (int)ptr/bit_size;
			mptr = map_ptr + bit_pos/32;
			bit_pos %= 32;
			hole_size=0;
		}

		/* otherwise, let the hole grow */
		else {
			hole_size += bit_size;
			bit_pos++;
			if (bit_pos/32) {
				bit_pos %= 32;
				mptr++;
			}
		}
	}
	return(ptr);
}
