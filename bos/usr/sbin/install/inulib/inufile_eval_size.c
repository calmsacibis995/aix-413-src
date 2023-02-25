static char sccsid[] = "@(#)87	1.12  src/bos/usr/sbin/install/inulib/inufile_eval_size.c, cmdinstl, bos411, 9428A410j 6/21/93 12:37:43";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inufile_eval_size
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <sys/access.h>
#include <inu_eval_size.h>
#include <inulib.h>

/*
#include <inudef.h>
#include <inuerr.h>
*/

/*
	frompath is the source file to be moved
	topath is the destination for the file
	active must be 1 for the frompath to beable to reuse the
		destination fs size.
*/
extern char *inu_cmdname;

int inu_file_eval_size(char *topath, char *frompath, int active)
{
	int i;
	struct stat buf;
	int rc;

	if ((rc = mk_fs_list()) != INUGOOD)
		return(rc);
	bzero(fs_total,sizeof(size_t)*(nfs));
	bzero(al_total,sizeof(size_t)*(nfs));

	i=find_lfs(topath);

	/* get the size of the source file */
	if(stat(frompath,&buf)==0)
		fs_total[i]= ((((buf.st_size + buf.st_blksize -1)/buf.st_blksize) * buf.st_blksize) + 511) / 512;


	/* get the size of the destination file */
	if(active && stat(topath,&buf)==0 && access(topath,W_ACC)==0)
		al_total[i]= ((((buf.st_size + buf.st_blksize -1)/buf.st_blksize) * buf.st_blksize) + 511) / 512;

	return(enough_space(inu_cmdname));

}
