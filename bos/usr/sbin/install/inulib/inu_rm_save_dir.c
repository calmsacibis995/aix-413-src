static char sccsid[] = "@(#)49	1.7  src/bos/usr/sbin/install/inulib/inu_rm_save_dir.c, cmdinstl, bos411, 9428A410j 6/17/93 16:15:43";
/*
 * COMPONENT_NAME: (CMDINSTL) command install
 *
 * FUNCTIONS: inu_rm_save_dir
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
#include <sys/limits.h>
#include <sys/stat.h>
*/

#include <inulib.h>

/*
 * NAME: rm_save_dir
 *
 * FUNCTION: Removes save directory and everything in it.
 *
 * NOTES: If the save directory is a symbolic link, rm_save_dir removes the
 *	  directory pointed to by the symbolic link.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */
void inu_rm_save_dir(char *path_name)/* path_name to save directory	*/
{
	struct stat buf;	/* structure returned from lstat	    */
	int len;		/* length of real_path returned by readlink */

	/* pathname of where save directory is actually located. */
	char real_path[PATH_MAX+1]; 


	/* if no directory or link exists, return	*/
	if (lstat(path_name, &buf) == -1)
		return;

	/* is save directory is a symbolic link? */
	if ((buf.st_mode & S_IFMT) == S_IFLNK) {

		/* find and delete the real path to the save directory */
		if ((len = readlink(path_name, real_path, PATH_MAX+1)) > 0) {

			/* readlink doesn't NULL-terminate the string	*/
			real_path[len] = NULL;

			/* remove the "real" directory & everything in it   */
			inu_rm(real_path);

		}
		/* delete the symbolic link	*/
		unlink(path_name);
	}
	else
		/* The save directory is not a symbolic link.	*/
		inu_rm(path_name);

} /* End rm_save_dir	*/
