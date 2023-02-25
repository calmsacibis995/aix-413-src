static char sccsid[] = "@(#)62  1.6  src/bos/usr/sbin/install/inulib/is_file.c, cmdinstl, bos411, 9428A410j 6/17/93 16:16:30";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: is_file
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <sys/stat.h>

int is_file(device)
/*
 * NAME: is_file
 *
 * FUNCTION: Returns true if the device specified is a file.
 *
 * RETURNS: TRUE -- device is a file
 *	    FALSE - device is not file
 */
char *device;			/* device name	*/
{
	struct devinfo buf;	/* buffer to hold devinfo structure returned
				   by ioctl()				    */
	struct stat statbuf;	/* buffer to hold fstat information */
	int fdesc;		/* file descriptor of device		    */
	int rc;			/* return code from ioctl()/fstat()	    */


	if ((fdesc = open(device, O_RDONLY)) == -1)
		/* return if it can't be opened		*/
		return(FALSE);

	rc = ioctl(fdesc, IOCINFO, &buf);

	if (rc >= 0) {		/* If this is true device is a device */
		close(fdesc);
		return(FALSE);
	}

	rc = fstat(fdesc, &statbuf);	/* Attempt to stat */
	close(fdesc);

	if (rc < 0) {		/* If we can't stat it can't be a file */
		return(FALSE);
	}

	if (statbuf.st_mode & (S_IFREG|S_IFLNK)) {  /* It really is a file */
		return(TRUE);
	}

	return(FALSE);
	
} /* End is_file */
