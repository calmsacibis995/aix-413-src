/* @(#)79	1.1  src/nfs/usr/include/nfs/nfs_load.h, sysxnfs, nfs411, GOLD410 12/3/93 16:55:23 */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: 
 *		
 *
 *   ORIGINS: 24,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1990,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*      @(#)nfs_misc.h  1.6 91/03/08 NFSSRC4.1 from 2.32 90/07/31 SMI   */


#ifndef __NFS_MISC_HEADER__
#define __NFS_MISC_HEADER__

#define NFS_KLOAD_SERV	5	/* server extension nfs.ext	*/
#define NFS_KLOAD_ACL	4	/* acl extension acl.ext	*/
#define NFS_KLOAD_CLNT	3	/* client extension clnt.ext	*/
#define NFS_KLOAD_KRPC	2	/* rpc extension krpc.ext	*/
#define NFS_KLOAD_KDES	1	/* des extenstion kdes.ext	*/
#define NFS_KLOAD_STAT	0	/* flag for laod_nfs to return stat on which
				 * extensions are loaded        */

#endif !__NFS_MISC_HEADER__
