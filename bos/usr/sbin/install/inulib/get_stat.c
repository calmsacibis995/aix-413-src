static char sccsid[] = "@(#)49  1.10  src/bos/usr/sbin/install/inulib/get_stat.c, cmdinstl, bos411, 9428A410j 5/31/94 08:17:03";
/*
 * COMPONENT_NAME: (CMDINSTL) LPP Install
 *
 * FUNCTIONS: get_stat (inulib)
 *            get_vmount
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/mntctl.h>
#include <sys/vmount.h>

/*
 * NAME: get_stat
 *                                                                    
 * FUNCTION: 
 *	gather mount status for all virtual mounts for this host.
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *                                                                    
 * ON ENTRY:
 *	vmountpp	pointer to a buffer in which to put mount info
 *
 * NOTES:
 *	get_stat() was lifted and slightly modified from
 *	AIX Version 3 df.c.
 *
 * RETURNS:
 *	< 0 	error in mntctl()
 *	> 0	for number of struct vmounts (mount points) in
 *		buffer which is pointed to by pointer left at
 *		*vmountpp.
 */
int get_stat(
struct vmount	**vmountpp)	/* place to tell where buffer is */
{
  size_t		size;
  register struct vmount	*vm;
  int			nmounts;
  int			count;

  size = BUFSIZ;		/* initial default size	*/
  count = 10;			/* don't try forever	*/

  while (count--) {		/* don't try it forever	*/
	if ((vm = (struct vmount *)malloc(size)) == NULL) {
		return(-1);
	}

/*
 * perform the QUERY mntctl - if it returns > 0, that is the
 * number of vmount structures in the buffer.  If it returns
 * -1, an error occured.  If it returned 0, then look in
 * first word of buffer for needed size.
 */
	if ((nmounts = mntctl(MCTL_QUERY, size, (caddr_t)vm)) > 0) {
		*vmountpp = vm;		/* OK, got it, now return */
		return(nmounts);
	} else {
		if (nmounts == 0) {
			size = *(int *)vm; /* the buffer wasn't big enough */
			free((void *)vm);   /* get required buffer size     */
		} else {
			free((void *)vm);/* some other kind of error occurred*/
			return(-1);
		}
	}
  }
  return(-1);
}

/*
 * NAME: get_vmount
 *                                                                    
 * FUNCTION: 
 *        Determines, via the filesystems vmount structures,
 *        the vmount id of the the filesystem id provided as 
 *        an argument (enables ultimate access to the actual 
 *        name of the filesystem).
 *
 * EXECUTION ENVIRONMENT: Part of user command.
 *                                                                    
 * RETURNS:
 *	ptr to structure with vmount id of filesystem or NULL
 */
struct vmount *get_vmount(fsid_t *fsid)
{
    extern struct vmount *inu_vmount_p;
    extern int inu_vmount_num;
    register struct vmount *vm;
    int nmount;

    /* make sure we have all the virtual mount status of this host */
    if(inu_vmount_p == NULL)
        inu_vmount_num = get_stat(&inu_vmount_p);

    /* get the number of struct vmount in the vmount buffer */
    nmount = inu_vmount_num;

    /* go thru all the structures in the vmount buffer */
    for (vm = inu_vmount_p; nmount; nmount--,
            vm = (struct vmount *)((char *)vm + vm->vmt_length)) {
         if(( vm->vmt_fsid.fsid_dev == fsid->fsid_dev ) &&
            ( vm->vmt_fsid.fsid_type == fsid->fsid_type ))
             return(vm);
    }
    return((struct vmount *)NULL);
}
