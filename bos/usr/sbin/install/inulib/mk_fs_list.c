static char sccsid[] = "@(#)79  1.11  src/bos/usr/sbin/install/inulib/mk_fs_list.c, cmdinstl, bos411, 9428A410j 5/31/94 08:16:02";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: mk_fs_list
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME: mk_fs_list
 *
 * FUNCTION:
 *	Build the list of filesystems physically on this system
 *	in fs[] and put the number of them in nfs.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *
 * NOTES:
 *	Calls get_stat which was borrowed from AIX V3 df.c.
 *
 * RETURNS:
 *  		INUGOOD:    if successful.
 *  		INUFS:      if mounted file systems are >= MAXFILESYSTEMS.
 *
 * Parameters:
 *		fs		contains list of pointers to names of
 *				filesystems attached to devices on the
 *				local system
 *		nfs		contains integer number of entries in fs[]
 */

#include <sys/types.h>
#include <sys/mntctl.h>
#include <inu_eval_size.h>
#include <inuerr.h>
#include <instlmsg.h>
#include <commonmsg.h>
#include <inudef.h>

struct vmount *inu_vmount_p = NULL;
int inu_vmount_num = 0;

extern char     *inu_cmdname;	/* Name of command calling these routines */
extern char    **fs_used;
extern ulong_t  *fs_vfsnum;

int mk_fs_list()
{
	int	count;				/* n of vmounts on sys (nfs,ds, etc.)*/
	struct vmount	*p;			/* pointer into buf vmount structs   */
	static int made_fs_list=0;

	if(made_fs_list)
		return(INUGOOD);

	made_fs_list=1;
	count = get_stat(&p);

        fs        = (char **)  calloc ((count + FIRST_REAL_FS),sizeof(char * ));
        fs_flags  = (int *)    calloc ((count + FIRST_REAL_FS),sizeof(int    ));
        fs_vfsnum = (ulong_t *)calloc ((count + FIRST_REAL_FS),sizeof(ulong_t));
        fs_total  = (size_t *) calloc ((count + FIRST_REAL_FS),sizeof(size_t ));
        al_total  = (size_t *) calloc ((count + FIRST_REAL_FS),sizeof(size_t ));
        work_max  = (size_t *) calloc ((count + FIRST_REAL_FS),sizeof(size_t ));
        fs_used   = (char **)  calloc ((count + FIRST_REAL_FS),sizeof(char * ));

	/* export the stats */
	inu_vmount_p=p;
	inu_vmount_num=count;

	fs[PAGESPACE]="PAGESPACE";
	fs[INSTWORK]="INSTWORK";
	fs[USRSAVESPACE]="/usr/lpp/SAVESPACE";
	fs[ROOTSAVESPACE]="/lpp/SAVESPACE";
	fs[SHARESAVESPACE]="/usr/share/lpp/SAVESPACE";
	nfs = FIRST_REAL_FS-1;

	while (count > 0) {			/* for each one returned	*/
		nfs++;
			
		fs[nfs] = vmt2dataptr(p,VMT_STUB);
		fs_flags[nfs] = p->vmt_flags;
		fs_vfsnum[nfs] = (ulong_t)p->vmt_vfsnumber;
		p = (struct vmount *)((char *)p + p->vmt_length);	/* p++	  */
		count--;
	}
	return(INUGOOD);
}
