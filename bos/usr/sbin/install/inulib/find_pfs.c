static char sccsid[] = "@(#)47  1.11  src/bos/usr/sbin/install/inulib/find_pfs.c, cmdinstl, bos411, 9428A410j 6/23/94 19:04:22";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: find_pfs
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/mntctl.h>
#include <inuerr.h>
#include <inu_eval_size.h>
#include <inudef.h>

/*
 * NAME: find_pfs
 *
 * FUNCTION: Find name of physical filesystem containing a specific file.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * RETURNS: Pointer to string containing the name of the mount point
 *	describing the filesystem containing the specified file
 *	(or NULL if file does not exist).
 */

extern struct vmount *inu_vmount_p;
extern int inu_vmount_num;

int find_pfs(char *file)
{
	int	i,j;				/* loop counter			*/
	struct vmount	*p;			/* pointer into buf[] structs	*/
	struct stat	statbuf;		/* structure for stat()		*/

	if (mk_fs_list() != INUGOOD)
		return(-1);

	if(stat(file,&statbuf)==-1)
		return(-1);

	for(i=0,p=inu_vmount_p;i<=inu_vmount_num;i++,p=(struct vmount *)((char *)p + p->vmt_length)) {

		/* if the filesystem is a physical device and it's ID
		 * is the same as that of the file in question, then
		 * this is the filesystem containing the file
		 */

		if (p->vmt_flags & (MNT_DEVICE | MNT_REMOTE)) {
			if (p->vmt_fsid.val[0] == statbuf.st_dev) {
			/* In case of diskless, do not call find_lfs(). This is
			because chroot is called and so find_lfs() may not 
			find the path if it cannot see that path. 
			Example is suppose /export/share is a file system and 
			dir /export/share/AIX/usr/share is mounted over 
			/home/maintclients/usr/share and chroot is done to 
			/home/maintclients. Now find_pfs is called for 
			"/home/maintclients/usr/share". This would match to 
			"/export/share". Now if we call find_lfs() for this, 
			it is going to look for a file/directory under its 
			root. Since that is not existing it will return root, 
			which will be /home/maintclients filesystem!
			*/
			if (getenv ("INUCLIENTS") != NIL(char)) {
				for (j =  FIRST_REAL_FS; j <= nfs ; j++) 
					if (strcmp (vmt2dataptr(p,VMT_STUB), fs[j] ) == 0 )  
						return(j); 
			} else {
				return(find_lfs(vmt2dataptr(p,VMT_STUB)));
			}
		}

		}
	}
	return(-1);
}
