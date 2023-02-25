static char sccsid[] = "@(#)14	1.7  src/bos/usr/sbin/install/inurest/inurest_eval_size.c, cmdinstl, bos411, 9428A410j 6/21/93 17:27:40";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inurest_eval_size
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
#include <inu_eval_size.h>
#include <inudef.h>
#include <inuerr.h>

int inurest_eval_size(char *inutmpdir, char *inulibdir, char *apply_list)
{
	FILE	*fp;				/* size file pointer		*/
	FILE	*lppfp;				/* lpp_name file pointer	*/
	char filename[1024];
	char lppname[1024];
	int rc;
	int i;
	
	if ((rc = mk_fs_list()) != INUGOOD)
		return(rc);

	/*
	 * loop thru the size file, and add up all of the space needed in the fs
	 * if "size" exists, then there is only 1 size file
	 * otherwise, loop thru the options
	 */
	strcpy(filename,inulibdir);
	strcat(filename,"/size");
	if((fp = fopen(filename,"r")) != NULL) {		/* open size file */
		inu_fget_fs_sizes(fp,fs_total,work_max);
		(void) fclose(fp);
	} else {					/* open lpp-option.size files*/
		strcpy(filename,inutmpdir);
		strcat(filename,"/lpp.options");
		lppfp = fopen(filename,"r");
		if(lppfp != NULL)
		{
			while ((get_lppname(lppfp,lppname)) == 0) {	
				sprintf(filename,"%s/%s.size",inulibdir,lppname);
				if ((fp = fopen(filename,"r")) != NULL) {
					inu_fget_fs_sizes(fp,fs_total,work_max);
					(void) fclose(fp);
				}				
			}
		(void) fclose(lppfp);
		}
	}

	/*
	 * loop thru the apply list, and add up all of the space currently used
	 * if "al" exists, then there is only 1 apply list
	 * otherwise, loop thru the options
	 */
	if((fp = fopen(apply_list,"r")) != NULL) {
		inu_fget_file_sizes(fp,al_total,1);
		(void) fclose(fp);

	}

	/* Don't count savespace all over again */
	for (i = DEFSAVESPACE; i < FIRST_REAL_FS; i++) 
	{
	     fs_total[i] = work_max[i] = 0; 
	}

	return(enough_space("inurest"));
}
