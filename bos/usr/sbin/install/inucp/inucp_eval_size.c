static char sccsid[] = "@(#)43  1.7  src/bos/usr/sbin/install/inucp/inucp_eval_size.c, cmdinstl, bos411, 9428A410j 6/19/93 11:53:23";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inucp_eval_size
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

#include <inu_eval_size.h>
#include <inulib.h>
#include <sys/stat.h>
#include <sys/access.h>

/*
#include <inuerr.h>
#include <stdio.h>
#include <inudef.h>
*/

int inucp_eval_size(char *oldroot,char *newroot,char *cplist)
{
	FILE	*fp;
	char	file[PATH_MAX];		/* dir name from size file	*/
	char	new_file[PATH_MAX];
	char	old_file[PATH_MAX];
	char	*file_ptr;
	char	*new_file_ptr;
	char	*old_file_ptr;
	int	j;			/* misc counter			*/
	struct stat buf;
	int rc;

	if ((rc = mk_fs_list()) != INUGOOD)
		return(rc);

	if((fp = fopen(cplist,"r")) == NULL) 
		return(SUCCESS);


	/* setup path to old root path for the file in question */
	if(oldroot==NULL || strcmp(oldroot,"/")==0)
		old_file_ptr=old_file;
	else
	{
		int len;
		strcpy(old_file,oldroot);
		len=strlen(old_file);
		if(old_file[len-1]=='/')
		{
			len--;
			old_file[len]='\0';
		}
		old_file_ptr=old_file + len;
	}

	/* setup path to new root path for the file in question */
	if(newroot==NULL || strcmp(newroot,"/")==0)
		new_file_ptr=new_file;
	else
	{
		int len;
		strcpy(new_file,newroot);
		len=strlen(new_file);
		if(new_file[len-1]=='/')
		{
			len--;
			new_file[len]='\0';
		}
		new_file_ptr=new_file + len;
	}

	/* read the apply list */
	while (fscanf(fp,"%s",file) != EOF) {

		/* remove . from path */
		file_ptr = file;
		if (file[0] == '.')
			file_ptr++;

		/* find it's file system */
		strcpy(new_file_ptr,file_ptr);
		j = find_lfs(new_file);
		if(j<0)
			continue;

		/* size of file already there */
		if(stat(new_file, &buf) == 0 && access(file_ptr,W_ACC) == 0)
			al_total[j]+= ((((buf.st_size + buf.st_blksize -1)/buf.st_blksize) * buf.st_blksize) + 511) / 512;

		/* size of file to be coppied there */
		strcpy(old_file_ptr,file_ptr);
		if(stat(old_file, &buf) == 0)
			fs_total[j]+= ((((buf.st_size + buf.st_blksize -1)/buf.st_blksize) * buf.st_blksize) + 511) / 512;

	}

	fclose(fp);
	return(enough_space("inucp"));
}
