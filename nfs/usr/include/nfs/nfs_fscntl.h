/* @(#)78	1.6  src/nfs/usr/include/nfs/nfs_fscntl.h, sysxnfs, nfs411, GOLD410 5/13/94 17:25:28 */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: find_nfs_id
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifndef _NFS_FSCNTL_
#define _NFS_FSCNTL_

#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>		/* for typedef  t_rcstat */
#include <rpc/svc.h>		/* for typedef  t_rsstat */


/* 
 * struct of configurable nfs options.  used with cmd define
 * NFS_CNTL_PRT_OPTIONS		      
 */
typedef int	nfs_portmonitor;
typedef int	nfsudpcksum;
typedef int	nfs_socketsize;
typedef int	nfs_setattr_err_ignore;
typedef int	nfs_write_gather;
typedef int	nfs_repeat;

/*
 * struct of all configuable options,
 * struct is to be used with
 * NFS_CNTL_GET_OPTIONS in which the return stats are placed 
 * in var of this type, i.e.
 * t_nfs_options nfs_options
 * nfs_cntl(NFS_CNTL_GET_OPTIONS,&nfs_options,NFS_CNTL_OPTIONS_SIZE);
 */
typedef struct {
	nfs_portmonitor	portmon;
	nfsudpcksum	cksum;
	nfs_socketsize	socksize;
	nfs_setattr_err_ignore nfs_setattr_error;
	nfs_write_gather wr_gather;
	nfs_repeat	nfs_repeat_msg;
} t_nfs_options;
 
/* example of interface 
 * nfscntl(int, nfs_pormon *, int);
 * nfscntl(int, nfsudpcksum *, int);
 * nfscntl(int, nfs_socksize *, int);
 */

/*
 * server side nfs statistics
 * struct is to be used with 
 * NFS_CNTL_GET_SERV_STAT in which the return stats are placed 
 * in var of this type, i.e.
 * t_svstat svstat;
 * nfs_cntl(NFS_CNTL_GET_SERV_STAT,&svstat,NFS_CNTL_GET_SERV_STAT);
 */
typedef struct {
	u_int   ncalls;         /* number of calls received */
	u_int   nbadcalls;      /* calls that failed */
	u_int   reqs[32];       /* count for each request */
} t_svstat;

/*
 * client side nfs statistics
 * struct is to be used with 
 * NFS_CNTL_GET_CLNT_STAT in which the return stats are placed 
 * in var of this type, i.e.
 * t_clstat clstat;
 * nfs_cntl(NFS_CNTL_GET_CLNT_STAT,&clstat,NFS_CNTL_CLNT_SIZE);
 */
typedef struct {
	int     nclsleeps; /* client handle waits*/
	int     nclgets;   /* client handle gets */
	int     ncalls;    /* client requests */
	int     nbadcalls; /* rpc failures */
	int     reqs[32];  /* # of each request*/
} t_clstat ;

/*
 * struct of nfs_stats (nfs statistics) is used with define
 * NFS_CNTL_GET_ALL_STAT in which the return stats are placed 
 * in var of this type, i.e.
 * t_nfs_stats nfsstats;
 * nfs_cntl(NFS_CNTL_GET_ALL_STAT,&nfsstats,NFS_CNTL_ALL_SIZE);
 */
typedef struct {
	t_clstat clstat;
	t_svstat svstat;
	t_rcstat rcstat;
	t_rsstat rsstat;
} t_nfs_stats;
	
/*
 * struct of nfs_stats_pkts (nfs statistics pkts) is used with define
 * NFS_CNTL_GET_PKTS in which the return stats are placed 
 * in var of this type, i.e.
 * t_nfs_stats_pkts nfs_pkts;
 * nfs_cntl(NFS_CNTL_GET_PKTS_STAT,&nfs_pkts,NFS_CNTL_PKTS_SIZE);
 */
typedef struct {
	int nfs_good_sends;
	int nfs_send_tries;
} t_nfs_stats_pkts;

/*
 * data type for setting the size of the NFS server duplicate cache.
 */
typedef u_int	nfs_dupcachesize;

/*
 * data type for setting the base priority for the NFS server
 */
typedef u_int	nfs_basepriority;

/*
 * data type for setting the dynamic retransmission flag on the NFS client
 */
typedef u_int	nfs_dynamicretrans;

/*
 * data type for setting the NFS client iopacing number of pages
 */
typedef	u_int	nfs_clientiopace;

/*
 * commands recognized by nfs fscntl
 */
#define NFS_CNTL_SET_CHKSUM		1  /* set krpc udp checksum opt      */
#define NFS_CNTL_SET_SOCKS		2  /* set nfs socket size def=60000  */
#define NFS_CNTL_SET_PORTCK   		3  /* set portcheck opt   	     */
#define NFS_CNTL_GET_OPTIONS		4  /* get all configurable options   */
#define NFS_CNTL_GET_CLNT_STAT		5  /* get client nfs stats - t_clstat*/
#define NFS_CNTL_CLR_CLNT_STAT		6  /* clear client nfs statistics    */
#define NFS_CNTL_GET_SERV_STAT		7  /* get server rpc stats - t_rsstat*/
#define NFS_CNTL_CLR_SERV_STAT		8  /* clear server rpc statistics    */
#define NFS_CNTL_GET_CLKRPC_STAT	9  /* get client rpc stats - t_rcstat*/
#define NFS_CNTL_CLR_CLKRPC_STAT	10 /* clear client rpc statistics    */
#define NFS_CNTL_GET_SVKRPC_STAT	11 /* get server rpc stats - t_rcstat*/
#define NFS_CNTL_CLR_SVKRPC_STAT	12 /* clear server rpc statistics    */
#define NFS_CNTL_GET_ALL_STAT		13 /* get all stats - t_nfs_stats    */
#define NFS_CNTL_CLR_ALL_STAT		14 /* clear all statistics           */
#define NFS_CNTL_SET_SETATTR		15 /* ignore setattr error	     */
#define NFS_CNTL_SET_WRGATHER		16 /* set write gather size	     */
#define NFS_CNTL_SET_REPEATMSG		17 /* set nfs repeat msg behavior    */
#define NFS_CNTL_GET_PKTS_STAT 		18 /* get outgoing pkts statistics   */
#define NFS_CNTL_CLR_PKTS_STAT 		19 /* zero outgoing pkts statistics  */
#define NFS_CNTL_SET_NFSDUPCACHESZ	20 /* set size of nfs dup cache      */
#define NFS_CNTL_GET_NFSDUPCACHESZ	21 /* get size of nfs dup cache      */
#define NFS_CNTL_SET_NFSBASEPRIORITY	22 /* set base priority - nfs server */
#define NFS_CNTL_GET_NFSBASEPRIORITY	23 /* get base priority - nfs server */
#define NFS_CNTL_SET_NFSDYNAMICRETRANS	24 /* set client dynamic retrans     */
#define NFS_CNTL_GET_NFSDYNAMICRETRANS	25 /* get client dynamic retrans     */
#define NFS_CNTL_SET_NFSCLIENTIOPACE	26 /* set client iopace pages        */
#define NFS_CNTL_GET_NFSCLIENTIOPACE	27 /* get client iopace pages        */

/* size to be used with NFS_CNTL_SET_CHKSUM */
#define NFS_CNTL_CHKSUM_SIZE sizeof(nfsudpcksum)

/* size to be used with NFS_CNTL_SET_SOCKS */
#define NFS_CNTL_SOCKS_SIZE	sizeof(nfs_socketsize)

/* size to be used with NFS_CNTL_SET_PORTCK */
#define NFS_CNTL_PORTCK_SIZE sizeof(nfs_portmonitor)

/* size to be used with NFS_CNTL_GET_OPTIONS */
#define NFS_CNTL_OPTIONS_SIZE	sizeof(t_nfs_options)

/* size to be used with NFS_CNTL_GET_CLNT_STAT	*/
#define NFS_CNTL_CLNT_SIZE	sizeof(t_clstat)

/* size to be used with NFS_CNTL_GET_SERV_STAT	*/
#define NFS_CNTL_SERV_SIZE	sizeof(t_svstat)

/* size to be used with NFS_CNTL_GET_CLKRPC_STAT	*/
#define NFS_CNTL_CLKRPC_SIZE	sizeof(t_rcstat)

/* size to be used with NFS_CNTL_GET_SVKRPC_STAT	*/
#define NFS_CNTL_SVKRPC_SIZE	sizeof(t_rsstat)

/* size to be used with NFS_CNTL_GET_ALL_STAT	*/
#define NFS_CNTL_ALL_SIZE	sizeof(t_nfs_stats)

/* size to be used with NFS_CNTL_SET_SETATTR */
#define NFS_CNTL_SETATTR_SIZE	sizeof(nfs_setattr_err_ignore)

/* size to be used with NFS_CNTL_SET_WRGATHER */
#define NFS_CNTL_WRGATHER_SIZE	sizeof(nfs_write_gather)

/* size to be used with NFS_CNTL_SET_REPEATMSG */
#define NFS_CNTL_REPEATMSG_SIZE	sizeof(nfs_repeat)

/* size to be used with NFS_CNTL_GET_PKTS_STAT 	*/
#define NFS_CNTL_PKTS_SIZE	sizeof(t_nfs_stats_pkts)

#define NFS_CNTL_DUPCACHE_SIZE	sizeof(nfs_dupcachesize)

#define NFS_CNTL_BASEPRIORITY_SIZE	sizeof(nfs_basepriority)

#define NFS_CNTL_DYNAMICRETRANS_SIZE	sizeof(nfs_dynamicretrans)

#define NFS_CNTL_CLIENTIOPACE_SIZE	sizeof(nfs_clientiopace)

#endif /* _NFS_FSCNTL_ */
