static char sccsid[] = "@(#)97  1.2  src/bos/usr/sbin/trace/mk_sp_file.c, cmdtrace, bos411, 9428A410j 5/25/93 15:37:21";
/*
 * COMPONENT_NAME: CMDTRACE
 * 
 * FUNCTIONS:
 * 
 * ORIGINS: 27 83
 * 
 *  This module contains IBM CONFIDENTIAL code. -- (IBM
 *  Confidential Restricted when combined with the aggregated
 *  modules for this product)
 *                   SOURCE MATERIALS
 *  (C) COPYRIGHT International Business Machines Corp. 1993,1993
 *  All Rights Reserved
 * 
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *
 * LEVEL 1, 12 Years Bull Confidential and Proprietary Information
 */
/* NAME: mk_sp_file
 *
 * FUNCTION: Creates, or alters a special file as required
 *
 * EXECUTION ENVIRONMENT:
 *
 *	The device-specific portions of the config methods call this
 *	routine to generate the special files the device requires.
 *
 * NOTES:
 *	mk_sp_file(devno,suffix,cflags)
 *
 *	suffix	= suffix part of the special file name.  For most cases
 *		this will be the device logical name.  For devices with
 *		more than one special file, this routine will be called
 *		one time for each special file needed, passing the file
 *		name required for the special file.
 *	cflags	= create flags for determining the type and mode of the
 *		special file.
 *
 *	If the special file already exists, then the major/minor numbers
 *	are checked. If they are incorrect, the old file is deleted, and
 *	a new one created. If the numbers were correct, no action is
 *	taken, and 0 is returned.
 *
 * RETURNS: 0 For success, errno for failure.
 */
/* add include files from bull */
#include <sys/types.h>
#include <sys/stat.h>
#include <cf.h>
#include <errno.h>
/* -------------------------- */
extern	int	errno;

int mk_sp_file(devno,suffix,cflags)
dev_t	devno;			/* major and minor numbers */
char	*suffix;		/* suffix for special file name */
long	cflags;			/* create flag / mode & type indicator */
{
	struct	stat	buf;
	char	spfilename[128];
	int	rc;
	long	filetype;	/* character or block device */

	filetype=cflags&(S_IFBLK|S_IFCHR);

	if(devno<0 | *suffix=='\0' | !filetype)
		return(E_MKSPECIAL);	/* error in parameters */

	sprintf(spfilename,"/dev/%s",suffix);	/* file name =/dev/[suffix] */


	if(stat(spfilename,&buf)) {

		/* stat failed, check that reason is ok */

		if( errno != ENOENT ) {
			/* DEBUG_0("stat failed\n"); */
			return(E_MKSPECIAL);
		}

		/* file does not exist, so make it */

		if(mknod(spfilename,filetype,devno)) {
			/* DEBUG_0("mknod failed\n"); */
			return(E_MKSPECIAL);
		}

	} else {

		/* stat succeeded, so file already exists */

		if(buf.st_rdev==devno)		/* major/minor #s are same, */
			return(0);		/* leave special file alone */
		if(unlink(spfilename)) {	/* unlink special file name */
			/* DEBUG_0("unlink failed\n"); */
			return(E_MKSPECIAL);
		}
		if(mknod(spfilename,filetype,devno)) {
			/* create special file */
			/* DEBUG_0("mknod failed\n"); */
			return(E_MKSPECIAL);
		}
	}

	/* change mode of special file.  This is not in the same step as   */
	/* creating the special file because of an error in mknod().	   */
	if(chmod(spfilename, (cflags&(~filetype)))) {
		/* DEBUG_0("chmod failed\n"); */
		return (E_MKSPECIAL);
	}

	return (0);
}

