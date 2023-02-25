static char sccsid[] = "@(#)62  1.11  src/bos/usr/sbin/install/inulib/inu_expand_ps.c, cmdinstl, bos411, 9428A410j 10/27/93 10:23:26";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: inu_expand_ps
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
#include <string.h>
#include <limits.h>
#include <sys/vminfo.h>
#include <sys/cfgodm.h>
#include <inudef.h>

/* Macros to determine values shown by lsps.
 * PAGES2MB converts from number of 4k pages to number of megabytes (MB).
 */

#define PAGES2MB(s)		((s) / 256)

#define ODMBUF	40		/* maximum odm character string		*/
#define	CMDBUF	256		/* maximum command line string		*/

#define DEV		"/dev/"

#define	TYPEA		"type"
#define PGTYPE		"paging"
#define SIZE		"size"
#define ODMPATH		"/etc/objrepos"



int inu_expand_ps(int needed_pagesize)
{
	struct CuAt *cuatp;		/* odm query struct		*/
	struct CuAt *ptr;		/* odm query struct		*/
	struct objlistinfo stat_info;	/* odm query status		*/
	struct objlistinfo stat_info2;
	char crit[CMDBUF];		/* odm search criteria		*/
	char devname[CMDBUF];	/* device special file name	*/
	int num;
	int pgsize=0;
	struct pginfo pginfo;

	odm_initialize();
	odm_set_path(ODMPATH);

	/* find our page spaces */
	sprintf(crit,"attribute = '%s' and value = '%s'", TYPEA, PGTYPE);
	cuatp = get_CuAt_list(CuAt_CLASS, crit, &stat_info, 16, 1);

	/* query failed hope we are ok with page space ? */
	if ((int)cuatp == -1)
	{
		odm_terminate();
		return(SUCCESS);
	}

	ptr=cuatp;

	/* how many meg of page space where requested */
	needed_pagesize = (((needed_pagesize + 1) / 2) + 1023) / 1024;

	/* add up the size of the active pagespaces */
	for (num=0; num<stat_info.num; num++)
	{
		strcpy(devname, DEV);
		strcat(devname, cuatp->name);
		
		/* is the page space ative? */
		if(swapqry(devname,&pginfo) == 0)
		{
			pgsize += PAGES2MB(pginfo.size);
			if(pgsize>=needed_pagesize)
			{
				odm_free_list(ptr,&stat_info);
				odm_terminate();
				return(SUCCESS);
			}
		}
		cuatp++;
	}
	
	/* if we can't determine the size of the page space go on */
	if(pgsize==0)
	{
		odm_free_list(ptr,&stat_info);
		odm_terminate();
		return(SUCCESS);
	}

	/* how many meg of page space do we need? */
	needed_pagesize = needed_pagesize - pgsize;

	cuatp=ptr;

	/* allocate more page space
	 * 	allocate to one pagespace only
	 *	will try inorder of pagespaces in odm
	 */
	for (num=0; num<stat_info.num; num++)
	{
		strcpy(devname, DEV);
		strcat(devname, cuatp->name);

		/* is the page space ative? */
		if(swapqry(devname, &pginfo) == 0)
		{
			struct CuAt *ptr;
			char command[CMDBUF];
			int num_pps, new_pps;
			int pp_size;

			/* get the number of pps for this page space */
			sprintf(crit,"name = '%s' and attribute = '%s'", cuatp->name, SIZE);
			ptr = odm_get_list(CuAt_CLASS, crit, &stat_info2, 16, 1);
			if(ptr== (struct CuAt *)-1)
				continue;

			/* get the number of new pp's to add */
			num_pps = atoi(ptr->value);
			odm_free_list(ptr,&stat_info2);
			pp_size = (PAGES2MB(pginfo.size)) / num_pps;
			num_pps = (needed_pagesize + pp_size - 1) / pp_size;

			/* minimum of one pp */
			new_pps = ((num_pps <= 0) ? 1 : num_pps);
			
			/* allocate new pp's for the page space */
			sprintf(command,"/usr/sbin/chps -s %d %s",new_pps,cuatp->name);
			if(inu_do_exec (command) == 0)
			{
				odm_free_list(ptr,&stat_info);
				odm_terminate();
				return(SUCCESS);
			}
		}
		cuatp++;
	}

	odm_free_list(ptr,&stat_info);
	odm_terminate();

	/* could not expand page space */
	return(needed_pagesize);
}
