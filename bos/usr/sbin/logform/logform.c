static char sccsid[] = "@(#)36	1.5  src/bos/usr/sbin/logform/logform.c, cmdfs, bos411, 9428A410j 6/15/90 21:21:48";
/*
 * COMPONENT_NAME: (CMDFS) commands that manipulate files
 *
 * FUNCTIONS: logform
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
#include <jfs/fsdefs.h>
#include <jfs/log.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#define MIN(a,b) ((a) < (b)) ? (a) : (b)

main(argc,argv)
int argc;
char *argv[];
{
	/* prelimary  version of log format program for XIX
	file system. command is

		logform device-name < blocks >

	where device_name is the name of a special device
	and the optional paramter blocks is the size of the
	log in 512 byte units.                        */
	
	/* the following disk-blocks are formatted:

		block 0 - not changed              
		block 1 - log superblock
		blocks 2-N  set to empty log pages
	 */

	char *pathname;
	int oflag,fd,npages,rc,k,dblks;
	struct stat st;
	struct devinfo dev;
	static struct logpage logp;
	static struct logsuper sup;

	/* was blocks specified as parameter ? */
	dblks = (argc >= 3) ? atoi(argv[2]) : 0;

	/* check that the thing is a block device */
	pathname = argv[1];
	rc = stat(pathname, &st);
	if (rc < 0) 
		goto error_out;
	if (!(st.st_mode & S_IFBLK)) {
		printf("device specified is not a block device \n");
		return(-1);
	}

	/* open the device */
	oflag = O_RDWR;
	fd = open(pathname,oflag);
	if (fd < 0)
		goto error_out;

	/*
	 * find out size. for bringup allow ioctl to fail or to
	 * report zero size.
	 */
	rc = ioctl(fd,IOCINFO,(char *) &dev);
	if (rc == 0)
	{
		if (dev.un.dk.numblks > 0)
		{
			dblks = (dblks == 0) ? dev.un.dk.numblks :
				MIN(dblks,dev.un.dk.numblks);
		}
	}

	if ((npages = dblks/(PAGESIZE/512)) == 0)
	{
		printf("ioctl failed \n");
		printf("try again but specify number of blocks \n");
		return;
	}	

	/* before we destroy anything try to ask the user if
	 *	they are sure they want to do this.
	 */
	if(isatty((int)fileno(stdin))) {
		char answer;

		while(TRUE) {
			printf("logform: destroy %s (y)?",pathname);
			answer = getchar();
			if(answer == 'n' || answer == 'N') exit(1);
			else if(answer == 'y' || answer == 'Y' ||
							answer == '\n') break;
			while((answer = getchar()) != '\n');
		}
	}
	
	/* init log superblock */
	sup.magic = LOGMAGIC;
	sup.version = LOGVERSION;
	sup.redone = 1;
	sup.size = npages;
	sup.logend = 2*PAGESIZE + 8;
	rc = lseek(fd,(off_t)PAGESIZE,SEEK_SET); /* seek to block 1 */
	rc = write(fd,(char *)&sup,(unsigned)PAGESIZE);
	if (rc < 0) 
		goto error_out;

	/* init  disk blocks 2 to npages-1  */
	rc = lseek(fd,(off_t)(2*PAGESIZE),SEEK_SET); /* seek to block 2 */
	for(k = 2; k < npages ;++k) {
		logp.h.eor = logp.t.eor = 8;
		logp.h.page = logp.t.page = k - 2 ;
		rc = write(fd,(char *)&logp,(unsigned)PAGESIZE);
		if (rc < 0) 
			goto error_out;
	}

	return(0);

error_out:
	perror(argv[0]);
	return(-1);

}

