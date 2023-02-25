static char sccsid[] = "@(#)94	1.10.1.2  src/bos/usr/sbin/crash/prvnode.c, cmdcrash, bos411, 9428A410j 3/28/94 17:06:42";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prvnode,dumpvnode,vfslot
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */


#include "sys/inode.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/gfs.h"
#include "sys/dir.h"
#include "sys/stat.h"
#include "crash.h"
#include "sys/gpai.h"
#include <sys/errno.h>

/* Note: The following code is liberally copied from src/bos/kernel/lfs/gpai.c.
 * It had been modified to work with a file image of memory instead of memory.
 * The gpai_getlist routine is modeled upon gpai_srch.  An important difference
 * is that the sp pointer is kept as the address of the current pool, instead
 * of being the address of the address of the pool (i.e. a level of indirection
 * has been removed).  This appeared to be unnecessary in gpai_srch, and it 
 * causes additional problems here (since we must use readmem to handle 
 * dereferences), so I have updated the use of sp, including the SPTOBJ macro
 * below.
 */

/* The number of list entries to allocate space for at initial malloc and at 
 * each realloc.
 */
#define ENTRIES_TO_ALLOC 4

/* Flag value for gpai pool structure indicating that this slot in the pool has
 * been allocated.
 */
#define OBJ_ALLOCATED 0xffffffff

/* Macro to obtain ptr to actual structure (e.g. vnode) being kept in the pool 
 * slot pointed to by sp.
 */
#define SPTOBJ(sp)			((caddr_t)(sp) + sizeof(sp))

/* Macro to determine the highest possible address for a slot within the current
 * subpool.  sp points to the current subpool, and ap points to the pool anchor.
 */
#define	HIGHBOUND(sp,ap)		(sp + ap->a_pas)

/* The next macro takes a pointer within the file being crashed and converts it to
 * a pointer that we can actually dereference.  It depends upon the following
 * variables:
 *
 * sp - the dumpfile's pointer to the base of the current pool.
 * currpool - the address of the working storage that holds our copy of the 
 *            current pool.
 */
#define MAP_PTR(p)			(((char *)(p)) + (currpool - (char *)sp))

/* Perform a sequential search of the galloc array for all objects that
 * match the specified type.  Return a pointer to a dynamically-
 * allocated array of pointers to the objects that match.
 */
char **
gpai_getlist( struct galloc *ap,       /* pointer to pool anchor (in real memory) */
              int            type,     /* specific type for object in list */
              int            typeoff)  /* offset in bytes to type in object */
{
        caddr_t  sp;
        char    *optr;
        caddr_t  ha;
        int      rc = -1;       /* return -1 if nothing found */
        int      ocnt;
	char   **retlist;	/* Pointer to pointer array to be returned */
	int      curritem = 0;	/* Current item in list */
				/* Current size of list */
	int      currsize = ENTRIES_TO_ALLOC;
	const int entrysize = sizeof(char *);
	char 	*currpool;	/* Address of current pool we are scanning */

				/* Allocate initial return list */
	retlist = (char **)malloc(entrysize * currsize);
				/* If no memory, get out */
	if (!retlist) {
		fprintf(stderr, NLcatgets(scmc_catd, MS_crsh, DLA_MSG_1,
			"0452-1051: Out of memory\n"));
        	exit(ENOMEM);
	}
				/* Clear allocated memory */
	memset(retlist,0,entrysize * currsize);

	/* Allocate memory for current subpool */
	currpool = malloc(ap->a_pas);

        /* set pointer to start of object's allocation */
        sp = ap->a_sat;

        /* get count of active objects */

        ocnt = ap->a_inuse;

        /* Search the object pool, comparing types for each active object.  */
        while (sp)
        {
		if (readmem(currpool,sp,ap->a_pas,CURPROC) != ap->a_pas) {
			/* If we can't read the subpool, tell the user we can't read the */
			/* first vnode within the subpool (at offset 8 within the subpool) */
			printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_1, "0452-287: Cannot read vnode at address 0x%8x.\n") ,((char *)sp) + 8);
			return retlist;
		}
                optr = SPTOBJ(sp);		/* get first item in pool */
                ha = (caddr_t)HIGHBOUND(sp, ap);/* get addr of last item */
                while (optr < ha)
                {
			/* If we have an item to look at... */
                        if (*(long *)MAP_PTR(optr) == OBJ_ALLOCATED)
                        {
				/* Get item address (in file) */
                                caddr_t obj = optr + sizeof(caddr_t);

				/* If type matches, add ptr to object to return list */
                                if ((type == 0) || (type == *((int *)MAP_PTR(&obj[typeoff]))))
                                {
					retlist[curritem++] = obj;
                                }

                                /* if all active objects searched, we're done */
                                if (--ocnt == 0)
                                        goto done;
                        }
			/* If necessary, resize the list */
			if (curritem >= currsize-1) {  /* Within one of the end? */
				currsize += ENTRIES_TO_ALLOC;  /* Change to new size */
				retlist = realloc(retlist,currsize*entrysize);
				if (!retlist) {
					fprintf(stderr, NLcatgets(scmc_catd, MS_crsh, DLA_MSG_1,
						"0452-1051: Out of memory\n"));
			        	exit(ENOMEM);
				}

				/* Zero out to end of new storage */
				memset(retlist+((currsize-ENTRIES_TO_ALLOC)),0,
					entrysize*ENTRIES_TO_ALLOC);
			}
                        /* go to next node */
                        optr += objsize(ap);
                }
                /* go to next allocation */
                sp = (caddr_t)(*MAP_PTR(sp));
        }

done:
	free(currpool);
        return retlist;  /* Return dynamically-allocated list of pointers to caller */
}


/* This routine prints out a single vnode.  The address passed is in relation to the
 * dumpfile - not the local storage used by crash.
 */
pr_one_vnode(addr)
struct vnode * addr;
{
	struct vnode vnode;

	if (readmem(&vnode, addr, sizeof(struct vnode), CURPROC)
	   != sizeof(struct vnode)) {
		printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_1, "0452-287: Cannot read vnode at address 0x%8x.\n") ,addr);
		return;
	}
	dumpvnode(&vnode, addr);
	return;
}

/* This routine prints out all the vnodes for a particular vfs, or all the vnodes in
 * the dumpfile.  If single_vfs is specified, then we're looking for all the vnodes for
 * the passed vfs address.  Note that single_vfs is in relation to the dumpfile - not
 * in the local storage used by crash.
 */
prvnode(single_vfs)
struct vfs * single_vfs;
{
	struct vfs *vfsaddr;
	struct vnode *vaddr;
	struct vfs vfs;
	struct vnode vnode;
	int count = 0, index = 0;
	struct vnode **vnode_array;
	struct galloc pool_base;


/*	vfsaddr = (struct vfs *)SYM_VALUE(Rootvfs); */

	/* If we're looking only for the vnodes for a particular vfs, set that up. */
	if (single_vfs)
		vfsaddr = single_vfs;
	/* Otherwise, get the vfs base from the dump file */
	else {
		/* Get address of first vfs struct out of dumpfile */
		if(readmem(&vfsaddr, SYM_VALUE(Rootvfs), sizeof(vfsaddr), CURPROC)
		    != sizeof(vfsaddr)) {
			printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_1, "0452-258 : Cannot read rootvfs structure at address 0x%8x.\n") ,SYM_VALUE(Rootvfs));
			return;
		}
	}

	for (; vfsaddr != (struct vfs *)NULL; vfsaddr = vfs.vfs_next)
	{
		if (readmem(&vfs, vfsaddr, sizeof(struct vfs), CURPROC)
		    != sizeof(struct vfs)) {
			printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_2, "0452-259: Cannot read vfs list from address 0x%8x.\n") ,vfsaddr);
			return;
		}
		/* If vfs_vnodes is set, use the linked list based on it to run the vnodes */
		if (vfs.vfs_vnodes) {
			for (vaddr = vfs.vfs_vnodes; vaddr != (struct vnode *)NULL; 
				vaddr = vnode.v_vfsnext)
			{
				if(readmem(&vnode, vaddr, sizeof(struct vnode), CURPROC)
				   != sizeof(struct vnode)) {
					printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_1, "0452-287: Cannot read vnode at address 0x%8x.\n") ,vaddr);
					break;  /* go on to next vfs */
				}
				dumpvnode(&vnode, vaddr);
				count++;   /* Bump counter for final total */
			}
		}
		/* Otherwise we must use the gpai mechanism to find the vnodes */
		else {
			/* Get base of vnode pool into working memory */
			if (readmem(&pool_base, SYM_VALUE(Gpa_vnode), sizeof(pool_base), CURPROC)
			    != sizeof(pool_base)) {
				printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_21, "0452-288: Cannot read vnode list anchor at address 0x%08x.\n"), SYM_VALUE(Gpa_vnode));
				return;
			}
			/* Now get a list of all the vnodes that belong to this vfs */
			vnode_array = (struct vnode **)gpai_getlist(&pool_base,(int)vfsaddr,
							(int)&(((struct vnode *)0)->v_vfsp));

			for (index = 0; vnode_array[index] != NULL; index++) {
				/* If readmem fails, print error, but go on to next vnode */
				if(readmem(&vnode, vnode_array[index], sizeof(struct vnode), 
				    CURPROC) != sizeof(struct vnode)) {
					printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_1, "0452-287: Cannot read vnode at address 0x%8x.\n") ,vnode_array[index]);
				}
				else {
					dumpvnode(&vnode, vnode_array[index]);
					count++;   /* Bump counter for final total */
				}
			}
		}
		/* If we only wanted one vfs's vnodes, we are done, so return. */
		if (single_vfs) return;
	}
	/* Only print count total if we were displaying all vnodes in the dumpfile */
	printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_2, " Total VNODES printed %d\n") ,count);
}

dumpvnode(vp, vaddr)
struct vnode	*vp;
struct vnode	*vaddr;
{
	struct vfs	vfs;
	struct gfs	gfs;
	struct gnode	gnode;
	char		*vntype, *v_fstype, *vfs_slot_str, *mvfs_slot_str;
	char		vfs_slot_buf[8], mvfs_slot_buf[8];
	int		vfs_slot, mvfs_slot;

	readmem(&vfs, vp->v_vfsp, sizeof(struct vfs), CURPROC);
	readmem(&gfs, vfs.vfs_gfs, sizeof(struct gfs), CURPROC);

	v_fstype = gfs.gfs_name;
	vfs_slot = vfslot(vp->v_vfsp);
	sprintf(vfs_slot_buf, "%d", vfs_slot);
	vfs_slot_str = (vfs_slot < 0) ? "-" : vfs_slot_buf;

	mvfs_slot = vfslot(vp->v_mvfsp);
	sprintf(mvfs_slot_buf, "%d", mvfs_slot);
	mvfs_slot_str = (mvfs_slot < 0) ? "-" : mvfs_slot_buf;

	readmem(&gnode, vp->v_gnode, sizeof (struct gnode), CURPROC);
	switch(gnode.gn_type)
	{
	case VREG:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_3, "vreg") ;	break;
	case VDIR:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_4, "vdir") ;	break;
	case VBLK:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_5, "vblk") ;	break;
	case VCHR:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_6, "vchr") ;	break;
	case VLNK:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_7, "vlnk") ;	break;
	case VSOCK:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_8, "vsock") ;	break;
	case VFIFO:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_9, "vfifo") ;	break;
	case VMPC:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_10, "vmpc") ;	break;
	case VBAD:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_11, "vbad") ;	break;
	case VUNDEF:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_12, "vundef") ;	break;
	default:	vntype =  catgets(scmc_catd, MS_crsh, PRVNODE_MSG_13, "weird") ;	break;
	}
	if (gfs.gfs_type == MNT_AIX) {
		printf("%7x %3s %4s %6s %6s %5d %5d  %7x ",
			vaddr,
			vfs_slot_str, mvfs_slot_str,
			vntype, v_fstype,
			vp->v_count,
			(struct inode *)gnode.gn_data -
				(struct inode *) SYM_VALUE(Inode),
			gnode.gn_data);
	} else {
		printf("%7x %3s %4s %6s %6s %5d %5s  %7x ",
			vaddr,
			vfs_slot_str, mvfs_slot_str,
			vntype, v_fstype,
			vp->v_count,
			"-",
			gnode.gn_data);
	}
	if (vp->v_flag)
	{
		if (vp->v_flag & V_ROOT) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_14, "vfs_root ") );
		if (vp->v_flag & VFS_UNMOUNTED) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_15, "vfs_unmounted ") );
		if (vp->v_flag & V_INTRANSIT) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_16, "remote ") );
		if (vp->v_flag & V_DMA) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_17, "dma ") );
		if (vp->v_flag & V_TEXT) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_18, "text ") );
		if (vp->v_flag & V_RMDC) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_19, "rmdc ") );
		if (vp->v_flag & V_RENAME) printf( catgets(scmc_catd, MS_crsh, PRVNODE_MSG_20, "rename") );
	}
	printf("\n");
}

vfslot(vfsp)
register struct vfs	*vfsp;
{
	struct vfs		vfs;
	struct vfs		*tvfsp;
	register int		fslot = 0;

	if (vfsp == NULL)
		return(-1);

	readmem(&tvfsp, SYM_VALUE(Rootvfs), sizeof(struct vfs *), CURPROC);
	for (;tvfsp != (struct vfs *)NULL; tvfsp = vfs.vfs_next) {
		if (tvfsp == vfsp)
			return(fslot);
		readmem(&vfs, tvfsp, sizeof(struct vfs), CURPROC);
		fslot++;
	}
	return(-1);
}
