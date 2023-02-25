static char sccsid[] = "@(#)09	1.4.1.2  src/bos/usr/sbin/lvm/intercmd/copyrawlv.c, cmdlvm, bos411, 9428A410j 6/17/94 09:03:21";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS:
 *	rawcpy,copy_lv,getflg,main
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
/*
 * Success 0
 * Failure 
 *	   1 Usage
 *	   2 Could not open source file
 *	   3 Could not open destination file
 *	   4 Read error on source file
 *	   5 Write error on destination file
 *	   6 Close error on destination file
 *	   7 Could not seek past the source lvcb 
 *	   8 Could not seek past the destination lvcb 
 */


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/param.h>

#define FAILURE (-1)

/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/*
 * GLOBAL VARIABLES
 */

/* external variables */
extern int optind;               /* index to next argv argument - incremented by getopt      */
extern char *optarg;             /* pointer to command line argument string - set by getopt  */

/*
 * void copy_lv (source, destination, blocks)
 *    	i char *source - source lv to be copied
 *    	i char *destination - destination lv to be copied to
 *    	i unsigned int blocks - number of BSIZE blocks to copy
 * copy_lv copies the source lv to the destination lv
 * NOTE: filesystems should be unmounted before copying
 */

#define BUFSIZE  131072

void copy_lv (source, destination, blocks)
char
	*source,
	*destination;
unsigned int
	blocks;

{
	int
		count,
		source_handle,
                bytes_read,
                bytes_written,
		destination_handle;
	char
		buffer[BUFSIZE];

	/* open the source file */
	source_handle = open (source, O_RDONLY  | O_EXCL );
	if (source_handle == FAILURE)
		exit(2);

	/* open the destination file */
	destination_handle = open (destination, O_WRONLY | O_EXCL );
	if (destination_handle == FAILURE)
		exit(3);

	/* skip past the source volumes lvcb */
	if (lseek(source_handle,UBSIZE,SEEK_CUR) < 0)
		exit(7);
		
	/* skip past the destination volumes lvcb */
	if (lseek(destination_handle,UBSIZE,SEEK_CUR) < 0)
		exit(8);

        /*
	 * copy all full blocks, except the last block (thus count starts at 1)
	 * because of the lvcb skip of 512 bytes.  The last block
	 * is short by 512 bytes, and we've got to give it a special
	 * read/write count to handle that case and make sure the
	 * return values match
	 */
	for(count = 1; count < blocks; count++)
	{
		/* read from the source lv */
		bytes_read = read (source_handle, buffer, BUFSIZE);
		if (bytes_read < 0)
		{ /* error has occured */
			perror("copyrawlv");
			exit(4);
		}
		if (bytes_read != EOF)
		{ /* did we get EOF back? */
		  /* write to the source lv */
			bytes_written = write(destination_handle, buffer,
						bytes_read);
			if (bytes_written != bytes_read)
			{
				perror("copyrawlv");
				exit(5);
			}
		}
	}

	/* Now read-write last block minus 512 bytes of lvcb */
	/* read from the source lv */
	bytes_read = read (source_handle, buffer, BUFSIZE - UBSIZE);
	if (bytes_read < 0)
	{ /* error has occured */
		perror("copyrawlv");
		exit(4);
	}
	if (bytes_read != EOF)
	{ /* did we get EOF back? */
	  /* write to the source lv */
		bytes_written = write(destination_handle, buffer, bytes_read);
		if (bytes_written != bytes_read)
		{
			perror("copyrawlv");
			exit(5);
		}
	}



	/* close our files */
	close(source_handle);  
	if (close(destination_handle) == FAILURE)
		exit(6);
}

getflg (){}

/*
 * NAME: main
 *
 * FUNCTION:
 *
 */
int main (argc, argv)
int argc;
char *argv[];
{
	long size;
	long atol();

	/* usage? */
	if (argc != 4)
		exit(1);

	/* how many 512 blocks do we wish to copy */
	size = atol (argv[3]);

	/* copy the source lv to the destination lv */
	copy_lv (argv[1], argv[2], size);

	exit(0);
}
