/* @(#)80	1.1  src/nfs/usr/include/nfs/nfs_trc.h, sysxnfs, nfs411, GOLD410 12/3/93 16:55:40 */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef __NFS_TRC_HEADER__
#define __NFS_TRC_HEADER__

#include <sys/trcmacros.h>

/*
 * Client side VNOP routines
 */
#define HKWD_NFS_VOPSRW    0x21100000
#define hkwd_NFS_READ             1
#define hkwd_NFS_WRITE            2

/*
 * Client side VNOP routines
 */
#define HKWD_NFS_VOPS      0x21200000
#define hkwd_NFS_SELECT       1
#define hkwd_NFS_LOOKUP       2
#define hkwd_NFS_OPEN         3
#define hkwd_NFS_CLOSE        4
#define hkwd_NFS_IOCTL        5
#define hkwd_NFS_GETATTR      6
#define hkwd_NFS_SETATTR      7
#define hkwd_NFS_ACCESS       8
#define hkwd_NFS_CREATE       9
#define hkwd_NFS_REMOVE       10
#define hkwd_NFS_LINK         11
#define hkwd_NFS_RENAME       12
#define hkwd_NFS_MKDIR        13
#define hkwd_NFS_RMDIR        14
#define hkwd_NFS_READDIR      15
#define hkwd_NFS_SYMLINK      16
#define hkwd_NFS_READLINK     17
#define hkwd_NFS_FSYNC        18
#define hkwd_NFS_INACTIVE     19
#define hkwd_NFS_BMAP         20
#define hkwd_NFS_BADOP        21
#define hkwd_NFS_STRATEGY     22
#define hkwd_NFS_LOCKCTL      23
#define hkwd_NFS_NOOP         24
#define hkwd_NFS_CMP          26


/*
 * Server services
 */

#define HKWD_NFS_RFSRW     0x21300000
#define hkwd_RFS_READ        1
#define hkwd_RFS_WRITE       2

#define HKWD_NFS_RFS       0x21400000
#define hkwd_RFS_NULL          1
#define hkwd_RFS_GETATTR       2
#define hkwd_RFS_SETATTR       3
#define hkwd_RFS_ERROR         4
#define hkwd_RFS_LOOKUP        5
#define hkwd_RFS_READLINK      6
#define hkwd_RFS_CREATE        8
#define hkwd_RFS_REMOVE        9
#define hkwd_RFS_RENAME        10
#define hkwd_RFS_LINK          11
#define hkwd_RFS_SYMLINK       12
#define hkwd_RFS_MKDIR         13
#define hkwd_RFS_RMDIR         14
#define hkwd_RFS_READDIR       15
#define hkwd_RFS_STATFS        16
#define hkwd_NFS_LOCKFH        30

/*
 * Server dispatch routine
 */
#define HKWD_NFS_DISPATCH  0x21500000
#define hkwd_NFS_DISP_ENTRY    1
#define hkwd_NFS_DISP_EXIT     2

/*
 * Client call routine
 */
#define HKWD_NFS_CALL      0x21600000
#define hkwd_NFS_CALL_ENTRY    1
#define hkwd_NFS_CALL_EXIT     2

/*
 * NFS/RPC Problem determination hooks
 */
#define HKWD_NFS_RPC_DEBUG 0x21700000
#define hkwd_KDES_CREATE_FAIL_1		0x01
#define hkwd_KDES_CREATE_FAIL_2		0x02
#define hkwd_KDES_CREATE_FAIL_3		0x03
#define hkwd_KDES_MARSHAL_FAIL		0x04
#define hkwd_KDES_VALIDATE_FAIL_1	0x05
#define hkwd_KDES_VALIDATE_FAIL_2	0x06
#define hkwd_KDES_REFRESH_FAIL_1	0x07
#define hkwd_KDES_REFRESH_FAIL_2	0x08
#define hkwd_KDES_RTIME_FAIL_SEND	0x09
#define hkwd_KDES_RTIME_FAIL_RECV	0x0A
#define hkwd_KRPC_UDPCREATE_FAIL_1	0x0B
#define hkwd_KRPC_UDPCREATE_FAIL_2	0x0C
#define hkwd_KRPC_UDPCREATE_FAIL_3	0x0D
#define hkwd_KRPC_KURECVFROM_1		0x0E
#define hkwd_KRPC_KURECVFROM_2		0x0F
#define hkwd_KRPC_SVCAUTH_U		0x10
#define hkwd_KRPC_SVCKUDP_SEND		0x11
#define hkwd_KRPC_SVCKUDP_DUPMEM	0x12
#define hkwd_KRPC_SVCKUDP_DUPNUMBER	0x13
#define hkwd_KRPC_XDRMBUF_1		0x14
#define hkwd_KRPC_XDRMBUF_2		0x15
#define hkwd_KRPC_XDRMBUF_3		0x16
#define hkwd_KRPC_XDRMBUF_4		0x17
#define hkwd_NFSSRV_SETSOCK		0x18
#define hkwd_NFSSRV_BADPROC		0x19
#define hkwd_NFSSRV_BADVERS		0x1A
#define hkwd_NFSSRV_BADGETARGS		0x1B
#define hkwd_NFSSRV_WEAKAUTH		0x1C
#define hkwd_NFSSRV_BADFREEARGS		0x1D
#define hkwd_NFSSRV_BADSENDREPLY	0x1E
#define hkwd_NFSSRV_UNPRIVPORT		0x1F
#define hkwd_KRPC_MBUF_GET_FAIL		0x20
#define hkwd_NFSCLNT_MNT_REGMIN		0x21
#define hkwd_NFSCLNT_MNT_REGMAX		0x22
#define hkwd_NFSCLNT_MNT_DIRMIN		0x23
#define hkwd_NFSCLNT_MNT_DIRMAX		0x24
#define hkwd_NFSCLNT_KLMLCK_1		0x25
#define hkwd_NFSCLNT_KLMLCK_2		0x26
#define hkwd_NFSCLNT_KLMLCK_3		0x27
#define hkwd_NFSCLNT_KLMLCK_4		0x28
#define hkwd_NFSCLNT_KLMLCK_5		0x29
#define hkwd_NFSCLNT_AUTHGET_DESFAIL	0x2A
#define hkwd_NFSCLNT_AUTHGET_UNKNOWN	0x2B
#define hkwd_NFSCLNT_AUTHFREE_UNKNOWN	0x2C
#define hkwd_NFSSRVACL_BADSENDREPLY	0x2D
#define hkwd_NFSSRVACL_BADFREEARGS	0x2E

/*
 * Rpc lock deamon (NFS) 
 */
#define HKWD_RPC_LOCKD	   0x21800000
#define hkwd_LOCKD_KLM_PROG	1
#define hkwd_LOCKD_NLM_REQ	2
#define hkwd_LOCKD_NLM_RES	3
#define hkwd_LOCKD_KLM_REPLY	4
#define hkwd_LOCKD_NLM_REPLY	5
#define hkwd_LOCKD_NLM_CALL	6
#define hkwd_LOCKD_NLM_CALL_UDP	7

#endif /* __NFS_TRC_HEADER__ */
