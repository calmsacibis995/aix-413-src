static char sccsid[] = "@(#)83  1.10.1.3  src/bos/usr/sbin/crash/prvfs.c, cmdcrash, bos411, 9428A410j 3/28/94 17:06:30";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prvfs,dumpvfs,dumpcounts,prvnode2,printvfsflags
 *
 * ORIGINS: 27,3,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */


#include "sys/inode.h"
#include "sys/vnode.h"
#include "sys/vfs.h"
#include "sys/dir.h"
#include "sys/stat.h"
#include "crash.h"


prvfs(c, till_end, extended)
int	c;
int	till_end;
int	extended;
{
	struct vfs	vfs;
	struct gfs	gfs;
	struct vnode	rootvnode;
	struct vnode	stubvnode;
	struct vfs	*vfsp;
	struct vmount	vmount;
	int		i, print;


	if (c == -1)
		return;
	
	if(readmem(&vfsp, SYM_VALUE(Rootvfs), sizeof(vfsp), CURPROC) 
	   != sizeof(vfsp)) {
		printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_1, "0452-258: Cannot read rootvfs structure at address 0x%8x.\n") ,SYM_VALUE(Rootvfs));
		return;
	}
	for (i = 0; i <= c || till_end; i++, vfsp =  vfs.vfs_next)
	{
		if (vfsp == (struct vfs *)NULL)
			break;
		if (readmem(&vfs, vfsp, sizeof(vfs), CURPROC) != sizeof(vfs)) {
			printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_2, "0452-259: Cannot read vfs list from address 0x%8x.\n") ,vfsp);
			return;
		}
		if (readmem(&vmount, vfs.vfs_mdata, sizeof(vmount), CURPROC)
				!= sizeof(vmount)) {
			printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_3, "0452-260: Cannot read vfs vmount data at address 0x%8x.\n") ,vfs.vfs_mdata);
		}
		if (readmem(&gfs, vfs.vfs_gfs, sizeof(gfs), CURPROC) 
		    != sizeof(gfs)) {
			printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_4, "0452-261: Cannot read vfs gfs data at address 0x%8x.\n") ,vfs.vfs_gfs);
		}
		if ((i == c) || till_end)
			print = 1;
		else
			print = 0;

		if (print) {
			printf("%3d %7x %6s ", i, vfsp, gfs.gfs_name);
			dumpvfs(&vfs, &vmount);
			if (extended)
			{
				printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_5, "ADDRESS VFS MVFS VNTYPE FSTYPE COUNT ISLOT    INODE FLAGS\n") );
				prvnode(vfsp);
				printf("\n");
			}
		}
		if ((i==c) && !till_end)
			break;
	} 
	if ((i != c) && !till_end) 
		printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_6, "0452-263: VFS %4d not currently on vfs list.\n") ,c);
}

dumpvfs(vfsp, vmount)
struct vfs	*vfsp;
struct vmount	*vmount;
{
	char	stubname[80], objectname[80];
	char	*data_string;


	printf("%6x %7x %6d ", vfsp->vfs_mntd, vfsp->vfs_mntdover,
		vfsp->vfs_number);
	if (vmount->vmt_flags)
	{
		if (vmount->vmt_flags & VFS_READONLY) printf("R");
		if (vmount->vmt_flags & VFS_REMOVABLE) printf("P");
		if (vmount->vmt_flags & VFS_DEVMOUNT) printf("D");
		if (vmount->vmt_flags & VFS_DISCONNECTED) printf("C");
		if (vmount->vmt_flags & VFS_UNMOUNTING) printf("U");
		if (vmount->vmt_flags & VFS_SHUTDOWN) printf("S");
	}

	data_string = (char *)((int)vfsp->vfs_mdata + 
		(int)vmount->vmt_data[VMT_STUB].vmt_off);
	if(readmem(stubname, data_string, vmt2datasize(vmount, VMT_STUB),
	    CURPROC) != vmt2datasize(vmount, VMT_STUB)) {
		stubname[0] = '?';
		stubname[1] = '\0';
	}
	data_string = (char *)((int)vfsp->vfs_mdata + 
		(int)vmount->vmt_data[VMT_OBJECT].vmt_off);
	if(readmem(objectname, data_string, vmt2datasize(vmount, VMT_OBJECT),
	   CURPROC) != vmt2datasize(vmount, VMT_OBJECT)){
		objectname[0] = '?';
		objectname[1] = '\0';
		}
	printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_7, "\t%s mounted over %s\n") , objectname, stubname);
}

/* The following routine is not called from anywhere within crash.  Since crash is a
 * standalone module, there is no need for this routine, so it is defined out.
 */
#ifdef NOT_USED
dumpcounts(vfs, fstype)
struct vfs	*vfs;
int		fstype;
{
	unsigned long	vaddr;
	struct vnode	vnode;
	struct gnode	gnode;
	struct inode	inode;
	struct gfs	gfs;
	int		islot, vslot = 0, idev, inum;
	char		*vntype, *v_fstype, *itype, *i_fstype;

	readmem(&gfs, vfs->vfs_gfs, sizeof(struct gfs), CURPROC);

	for (vaddr = (unsigned long)vfs->vfs_vnodes; 
			vaddr != (unsigned long)NULL; 
			vaddr = (unsigned long) vnode.v_vfsnext, vslot++) {

		readmem(&vnode, vaddr, sizeof(struct vnode), CURPROC);
		readmem(&gnode, vnode.v_gnode, sizeof(struct gnode), CURPROC);

		switch(gnode.gn_type) {
		case VDIR:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_8, "vdir") ;	break;
		case VCHR:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_9, "vchr") ;	break;
		case VBLK:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_10, "vblk") ;	break;
		case VREG:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_11, "vreg") ;	break;
		case VMPC:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_12, "vmpc") ;	break;
		case VFIFO:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_13, "vfifo") ;	break;
		case VBAD:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_14, "vbad") ;	break;
		case VNON:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_15, "vnon") ;	break;
		case VLNK:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_16, "vlnk") ;	break;
		case VSOCK:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_17, "vsock") ;	break;
		default:	vntype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_18, "weird") ;	break;
		}

		v_fstype = gfs.gfs_name;
		if (gfs.gfs_type == MNT_AIX) {
			islot =  (struct inode *) gnode.gn_data -
				(struct inode *) SYM_VALUE(Inode),
			readmem(&inode, gnode.gn_data,
				sizeof(struct inode), CURPROC);
			switch(inode.i_mode & IFMT)
			{
			case IFDIR:	itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_19, "dir") ;	break;
			case IFCHR:	if (inode.i_mode & ISVTX)
					 itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_20, "mpc")  ;
					else itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_21, "chr") ;
					break;
			case IFBLK:	itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_22, "blk") ;		break;
			case IFREG:	itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_23, "reg") ;		break;
			case IFIFO:	itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_24, "fifo") ;		break;
			default:	itype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_25, "weird") ;	break;
			}
			i_fstype =  catgets(scmc_catd, MS_crsh, PRVFS_MSG_26, "aix") ;
			idev = inode.i_dev;
			inum = inode.i_number;
			printf("\t\t%5d %5s %6s %5d %5d %5s %6s %5d %8.8x %4d\n",
				vslot, vntype, v_fstype, vnode.v_count,
				islot, itype, i_fstype, inode.i_count,
				idev, inum);
		} else {
			/* values are meaningless so set them to -1*/
			printf("\t\t%5d %5s %6s %5d %5d %5s %6s %5d %8.8x %4d\n",
				vslot, vntype, v_fstype, -1,
				-1, "-1", "-1",-1, -1, -1);
				
		}
		
	}
}
#endif

/* The following routine is no longer needed.  The prvnode routine in prvnode.c now is
 * generic-enough to handle the needs of both the vnode and vfs commands.
 */
#ifdef OBSOLETE
prvnode2(vfs, print, vslot)
struct vfs	*vfs;
int		print, vslot;
{
	struct vnode	*vaddr;
	struct vnode	vnode;

	for (vaddr = vfs->vfs_vnodes; vaddr != (struct vnode *)NULL; 
		vaddr = vnode.v_vfsnext, vslot++)
	{
		if(readmem(&vnode, vaddr, sizeof(struct vnode), CURPROC) 
			!= sizeof(struct vnode)) {
			printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_27, "0452-284: Cannot read vnode structure at address 0x%8x.\n") ,vaddr);
			return ;
		}
		if (print)
			dumpvnode(&vnode, vslot, vaddr);
	}
	return(vslot);
}
#endif

printvfsflags()
{
	printf("\n");
	printf( catgets(scmc_catd, MS_crsh, PRVFS_MSG_28, "\tflags:\tC=disconnected D=device   I=remote P=removable\n") );
	printf(       catgets(scmc_catd, MS_crsh, PRVFS_MSG_29, "\t\tR=readonly     S=shutdown U=unmounted Y=dummy\n") );

}
