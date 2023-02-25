static char sccsid[] = "@(#)38	1.4  src/bos/usr/sbin/install/bffcreate/inu_bff_eval_size.c, cmdinstl, bos411, 9428A410j 6/10/93 18:07:46";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_bff_eval_size
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
#include <inudef.h>
#include <inuerr.h>

/*
	topath is the destination for the file
	bffsize is the size of the bff
*/
extern char *inu_cmdname;

int inu_bff_eval_size(char *topath, int bffsize)
{
	int i;
	struct stat buf;
	int rc;

	if ((rc = mk_fs_list()) != INUGOOD)
		return(rc);
	bzero(fs_total,sizeof(size_t)*(nfs));
	bzero(al_total,sizeof(size_t)*(nfs));

	i=find_lfs(topath);

	/* get the size of the bff file */
	fs_total[i]= ((bffsize+511)/512);

	/* get the size of the destination file */
	if(stat(topath,&buf)==0 && access(topath,W_ACC)==0)
		al_total[i]= ((((buf.st_size + buf.st_blksize -1)/buf.st_blksize) * buf.st_blksize) + 511) / 512;

	return(enough_space(inu_cmdname));

}
