static char sccsid[] = "@(#)24  1.3  src/bos/usr/sbin/mkram/mkram.c, sysio, bos411, 9428A410j 4/12/93 19:08:04";
/*
 * COMPONENT_NAME: SYSIO
 *
 * FUNCTIONS: Read and write blocks to compressed ram image
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * NAME:	mkram.c
 *
 * FUNCTION: 	This low level command creates a compressed RAM 
 *		file system. The given file is compressed at  
 *		4K bytes granularity.	
 *
 * EXECUTION ENVIRONMENT:
 *
 *	The mkram program is a command that can be executed from
 *	a shell environment. The syntax of the command is
 *
 *		mkram file1 [ file2 ]
 *	
 *	It takes file1 as an input file and creates a compressed image 
 *	in file2.  If only file1 is given, then file1 is compressed
 *	in place,i.e., the compressed file system  image will be in file1.  
 *	A map of the compressed blocks is placed at the beginning of the
 *	output file followed by compressed data blocks.	
 *
 *	Logic:
 *		Determine file1 and/or file2 names. 
 *		Open file(s).
 *		Determine size of file1 and allocate buffer 
 *		for compressed data.
 *		While end of file1 is not reached
 *		     read 4K block
 *		     compress it
 *		     setup pointers in ram map
 *		     if different input/output files or
 *			  enough blocks has been read from input
 *		       write the compressed block in buffer
 *		     (otherwise leave compressed data in buffer)
 *		end While
 *		Truncate the output file	
 *		Update the head/tail pointers in the map
 *		Write the map at the beginning of the output file.
 *		Close file(s).
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syspest.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <sys/ramdd.h>

char inbuff[PSIZE];
char *outbuff;
char *fname;

int outf = -1;			/* output file			*/

int
main(argc, argv)
int argc;
char **argv;
{
	int inf;		/* input file			*/
	int rc;
	struct stat statbuf;	/* for file size		*/
	struct ramimage *rmap;	/* ram image being created	*/
	int hsize;		/* header size			*/
	int blocks;		/* blocks in ram image		*/
	int offset;		/* offset in ramdisk		*/
	int bufsize;		/* size of compressed block	*/
	int i;
	char *buff;
	void intr();
	char *outbuffp;  /* pointer into output buffer */
	int outbuffsize; /*  */
	int diff_files;  /* flag for different input and output files */

	if (argc > 3) {
		fprintf(stderr, "invalid argument\n");
		abort(EINVAL);
	}

	if( argc > 2 ) {
	        fname = argv[2];
		diff_files = strcmp(argv[1], argv[2]);  /* files different ? */
	}
	else {
		fname = argv[1];
		diff_files = 0;  /* input and output files are same */
	}

	if (ram_comp_init())	/* Initialize ram compression */
		abort(ENOMEM);

	signal(SIGINT, intr);

	inf = open(argv[1], O_RDONLY, 0);
	if (inf < 0)
		abort(errno);
	if (fstat(inf, &statbuf) < 0)
		abort(errno);

	if ((statbuf.st_size % PSIZE) || (statbuf.st_size == 0)) {
		fprintf(stderr, "mkram: %s invalid input size\n", argv[1]);
		abort(EINVAL);
	}

	blocks = (statbuf.st_size / PSIZE);	

	hsize = blocks * sizeof(struct ramblock) + RAM_HEAD_SIZE;

	/* Note: Following malloc allocates a a fairly large chunk of 
	   memory. However, until the pages are touched, there is no 
	   real allocation. */

	buff = (char *) malloc( hsize + blocks * PSIZE );
	if (buff == NULL) {
		fprintf(stderr, "malloc failed\n");
		abort(ENOMEM);
	}
	rmap = (struct ramimage *) buff;

	strncpy(rmap->r_magic, RAM_MAGIC, sizeof(rmap->r_magic));
	rmap->r_blocks = blocks;

	outf = open(fname, O_RDWR|O_CREAT, 0644); 
	if (outf < 0)
		abort(errno);

	rc = lseek(outf, hsize, SEEK_SET);
	if (rc < 0)
		abort(errno);

	offset = hsize;			/* reserved space for ram blocks */
	outbuff = buff + offset;	/* start of compressed blocks    */
	outbuffp = outbuff;		 
	outbuffsize = 0;
	for (i = 0 ; i < blocks ; i++ ) {
		if (read(inf, inbuff, PSIZE) != PSIZE)
			abort(EIO);
		ram_compress(inbuff, outbuffp, &bufsize); 
		rmap->r_block[i].rb_addr = offset;
		rmap->r_block[i].rb_size = bufsize;
		rmap->r_block[i].rb_prev = i - 1;
		rmap->r_block[i].rb_next = i + 1;
		outbuffp += bufsize;	
		outbuffsize += bufsize;
		offset += bufsize;

		/* optimization to conserve memory (and therefore page space):
		 * When we get to a certain point in the file, we can start
		 * updating it in place.
		 */
		if (diff_files || (((i+1) * PSIZE) - offset > PSIZE)) {
			if (write(outf, outbuff, outbuffsize) != outbuffsize)
				abort(ENOSPC);
			outbuffp = outbuff;
			outbuffsize = 0;
	        }
	}

	ftruncate(outf, offset);  /* reduce the file size */

	rmap->r_first = 0;		
	rmap->r_last = blocks - 1;
	rmap->r_block[0].rb_prev = LAST_BLOCK;
	rmap->r_block[blocks - 1].rb_next = LAST_BLOCK;
	rmap->r_alloc = offset;
	rmap->r_high = offset;
	rmap->r_segsize = 0;
	rmap->r_flag = RAM_WPACK;

	if (lseek(outf, 0, SEEK_SET) < 0)
		abort(errno);
	if (write(outf, rmap, hsize) != hsize)		/* update ram map */
		abort(ENOSPC);
	close(inf);
	close(outf);
	exit(0);
}

int
abort(ec)
int ec;
{
	if (outf != -1  && fname != NULL )
		unlink(fname);
	exit(ec);
}

void
intr()
{
	abort(EINTR);
}

#ifdef DEBUG
panic(s)
char *s;
{
	printf("%s\n", s);
	exit(-1);
}
#endif /* DEBUG */

