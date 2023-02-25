/* @(#)56	1.5  src/nfs/usr/include/nfs/nfs_swap.h, sysxnfs, nfs411, GOLD410 12/3/93 16:51:35  */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: NFS_VFSP_SWAP_DEV_LOCK
 *		NFS_VFSP_SWAP_DEV_UNLOCK
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Specifics for NFS paging functionality 
 */
#ifndef _NFS_SWAP_H

#include <sys/lock_def.h>


#define SWAP_KPROC_NAME		"sbiod"
#define BIOD_KPROC_NAME		"kbiod"

/* Max number of kprocs that can be created for a configure NFS swap device.
 */
#define MAX_NFS_SWAP_PROCS	6
#define MIN_NFS_SWAP_PROCS	1

/* Number of buf structs to be used when reading or writing from the
   NFS swap device.
   */
#define NFS_PAGING_BUFCNT	6

/* max size of stack for biod and biodswap processes 
 */
#define ASYNC_STACK_SIZE	(PAGESIZE * 3)  

/* This is where we will store the list of vnodes that correspond to 
 * paging devices.
 */
extern struct vfs *vfsp_swap_devices;
extern int nfs_gn_swap_strategy();

/* Used to define what type of daemon is working on a read/write request 
 */
#define ASYNC_DAEMON	1
#define NFS_PAGING_DAEMON	2


struct bufwork {
	struct buf *bufp;
	int	async_daemon_wait;
};

extern struct bufwork async_bufwork;
extern struct bufwork async_bufwork_swap;

/* Macros used to serialize access to the list of paging data structures
 */
extern Simple_lock nfs_vfsp_swap_dev_lock;

#define NFS_VFSP_SWAP_DEV_LOCK() disable_lock(INTPAGER, &nfs_vfsp_swap_dev_lock)
#define NFS_VFSP_SWAP_DEV_UNLOCK(ipri) unlock_enable(ipri, &nfs_vfsp_swap_dev_lock)

#endif
