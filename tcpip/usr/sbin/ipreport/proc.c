static char sccsid[] = "@(#)96	1.8  src/tcpip/usr/sbin/ipreport/proc.c, tcpras, tcpip411, GOLD410 4/8/94 09:50:51";
/*
 * COMPONENT_NAME: TCPIP proc.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * This file contains routines for the proclist, which is a linked list
 * of function pointers. Given any rpc program, and rpc procedure within
 * that program, (i.e. NFSPROGRAM,NFSSTATFS,) we should be able to return
 * a pointer to the xdr routine that decodes that result, (and to the
 * dump/print routine that prints it.)
 * 
 * Created Sat Aug 10 1991
*/

#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/x25user.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/devinfo.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <aixif/net_if.h>
#include "klm_prot.h"
#include "mount.h"
#define NFSSERVER 3
#include "nfs.h"
/*#include <rpcsvc/nfs_prot.h>*/
#include "nlm_prot.h"
#include "rex.h"
#include "rstat.h"
#include "rusers.h"
#include "rwall.h"
#include "sm_inter.h"
#include "spray.h"
#include <rpcsvc/yp_prot.h>
#include <pwd.h>
#include "yppasswd.h"
#include <rpcsvc/ypup_prot.h>
#include "ipr.h"
#include "pacl.h"

static int init_yet=0;	/* have we initialized the linked list yet? */

#define NAMELEN 50 

/*	if you have a zero for a function pointer in this data structure
	some routines will be used which just do hexdumps.  So it is still
	valid to include d_prog, d_proc, d_progname, d_procname, alone in
	the structure with only null function pointers.  This will at least
	print out the prog and proc names, which is still useful.
*/

struct data {
	int 	d_prog;			/* program number */
	int 	d_proc;			/* procedure number w/in that program*/
	char	*d_progname;	 	/* program name */
	char	*d_procname;	 	/* proc name */
	int	(*d_xdrcall)();		/* xdr call routine */
	int	(*d_dumpcall)();	/* dump call routine */
	int	(*d_xdrrep)();		/* xdr reply routine */
	int	(*d_dumprep)();		/* dump reply routine */
};
	
static struct data data[]= 
       { 

{ACL_PCL_PROG, ACL_PCL_STAT,  "ACL_PCL_PROG", "ACL_PCL_STAT",  0,0,0,0}, 
{ACL_PCL_PROG, ACL_STAT_LONG, "ACL_PCL_PROG", "ACL_STAT_LONG",
				xdr_fh_cookie, prt_fh_cookie, 
				xdr_rep_acl_pcl_cookie, prt_rep_acl_pcl_cookie},
{ACL_PCL_PROG, PCL_STAT_LONG, "ACL_PCL_PROG", "PCL_STAT_LONG", 0,0,0,0}, 
{ACL_PCL_PROG, ACL_READ,      "ACL_PCL_PROG", "ACL_READ", 
				xdr_acl_pcl_req_block, prt_acl_pcl_req_block,
				xdr_acl_pcl_reply, prt_acl_pcl_reply},
{ACL_PCL_PROG, PCL_READ,      "ACL_PCL_PROG", "PCL_READ",      0,0,0,0}, 
{ACL_PCL_PROG, ACL_WRITE,     "ACL_PCL_PROG", "ACL_WRITE",     
				xdr_acl_pcl_block, prt_acl_pcl_block,
				xdr_int, 0},
{ACL_PCL_PROG, PCL_WRITE,     "ACL_PCL_PROG", "PCL_WRITE",     0,0,0,0}, 

{ NFS_PROGRAM, RFS_ROOT,       "NFS_PROGRAM", "RFS_ROOT (Not Supported)",
    0,0,0,0}, 
{ NFS_PROGRAM, RFS_WRITECACHE, "NFS_PROGRAM", "RFS_WRITECACHE (Not Supported)",
0,0,0,0}, 

{NFS_PROGRAM, RFS_NULL, "NFS_PROGRAM", "RFS_NULL",
				0, c_null_dump, 
				myxdr_void, prt_void },
{NFS_PROGRAM, RFS_GETATTR, "NFS_PROGRAM", "RFS_GETATTR",
				xdr_fhandle, prt_fhandle,
				xdr_attrstat, prt_attrstat },
{NFS_PROGRAM, RFS_SETATTR, "NFS_PROGRAM", "RFS_SETATTR",
				xdr_saargs, c_setattr_dump,
				xdr_attrstat, prt_attrstat },
{NFS_PROGRAM, RFS_LOOKUP, "NFS_PROGRAM", "RFS_LOOKUP",
				xdr_diropargs, c_lookup_dump,
				xdr_diropres, prt_diropres } ,
{NFS_PROGRAM, RFS_READLINK, "NFS_PROGRAM", "RFS_READLINK",
				xdr_fhandle, c_readlink_dump,
				xdr_rdlnres, prt_rdlnres },
{NFS_PROGRAM, RFS_READ, "NFS_PROGRAM", "RFS_READ",
				xdr_readargs, c_read_dump,
				xdr_rdresult, prt_rdresult },
{NFS_PROGRAM, RFS_WRITE, "NFS_PROGRAM", "RFS_WRITE",
				xdr_writeargs, c_write_dump,
				xdr_attrstat, prt_attrstat },
{NFS_PROGRAM, RFS_CREATE, "NFS_PROGRAM", "RFS_CREATE",
				xdr_creatargs, c_create_dump,
				xdr_diropres, prt_diropres } ,
{NFS_PROGRAM, RFS_REMOVE, "NFS_PROGRAM", "RFS_REMOVE",
				xdr_diropargs, c_remove_dump,
				xdr_enum_nfsstat, prt_star_nfsstat },
{NFS_PROGRAM, RFS_RENAME, "NFS_PROGRAM", "RFS_RENAME",
				xdr_rnmargs, c_rename_dump,
				xdr_enum_nfsstat, prt_star_nfsstat },
{NFS_PROGRAM, RFS_LINK, "NFS_PROGRAM", "RFS_LINK",
				xdr_linkargs, c_link_dump,
				xdr_enum_nfsstat, prt_star_nfsstat },
{NFS_PROGRAM, RFS_SYMLINK, "NFS_PROGRAM", "RFS_SYMLINK",
				xdr_slargs, c_symlink_dump,
				xdr_enum_nfsstat, prt_star_nfsstat },
{NFS_PROGRAM, RFS_MKDIR, "NFS_PROGRAM", "RFS_MKDIR",
				xdr_creatargs, c_mkdir_dump,
				xdr_diropres, prt_diropres } ,
{NFS_PROGRAM, RFS_RMDIR, "NFS_PROGRAM", "RFS_RMDIR",
				xdr_diropargs, c_rmdir_dump,
				xdr_enum_nfsstat, prt_star_nfsstat },
{NFS_PROGRAM, RFS_READDIR, "NFS_PROGRAM", "RFS_READDIR",
				xdr_rddirargs, c_readdir_dump,
				xdr_getrddirres, prt_getrddirres },
{NFS_PROGRAM, RFS_STATFS, "NFS_PROGRAM", "RFS_STATFS",
				xdr_fhandle, prt_fhandle,
				xdr_statfs, prt_statfs },


{KLM_PROG, KLM_TEST,        "KLM_PROG", "KLM_TEST",        0,0,0,0}, 
{KLM_PROG, KLM_LOCK,        "KLM_PROG", "KLM_LOCK",        0,0,0,0}, 
{KLM_PROG, KLM_CANCEL,      "KLM_PROG", "KLM_CANCEL",      0,0,0,0}, 
{KLM_PROG, KLM_UNLOCK,      "KLM_PROG", "KLM_UNLOCK",      0,0,0,0}, 
{KLM_PROG, KLM_GRANTED,     "KLM_PROG", "KLM_GRANTED",     0,0,0,0}, 


{NLM_PROG, NLM_TEST,        "NLM_PROG", "NLM_TEST",
				xdr_nlm_testargs, prt_nlm_testargs,
				xdr_nlm_testres, prt_nlm_testres},
{NLM_PROG, NLM_LOCK,        "NLM_PROG", "NLM_LOCK",        
				xdr_nlm_lockargs, prt_nlm_lockargs,
				xdr_nlm_res, prt_nlm_res},
{NLM_PROG, NLM_CANCEL,      "NLM_PROG", "NLM_CANCEL",     
				xdr_nlm_cancargs, prt_nlm_cancargs,
				xdr_nlm_res, prt_nlm_res},
{NLM_PROG, NLM_UNLOCK,      "NLM_PROG", "NLM_UNLOCK",    
				xdr_nlm_unlockargs, prt_nlm_unlockargs,
				xdr_nlm_res, prt_nlm_res},
{NLM_PROG, NLM_GRANTED,     "NLM_PROG", "NLM_GRANTED",  
				xdr_nlm_testargs, prt_nlm_testargs,
				xdr_nlm_res, prt_nlm_res},
{NLM_PROG, NLM_TEST_MSG,    "NLM_PROG", "NLM_TEST_MSG",
				xdr_nlm_testargs, prt_nlm_testargs,
				xdr_nlm_testres, prt_nlm_testres},
{NLM_PROG, NLM_LOCK_MSG,    "NLM_PROG", "NLM_LOCK_MSG", 
				xdr_nlm_lockargs, prt_nlm_lockargs, 0,0},
{NLM_PROG, NLM_CANCEL_MSG,  "NLM_PROG", "NLM_CANCEL_MSG",
				xdr_nlm_cancargs, prt_nlm_cancargs, 0,0},
{NLM_PROG, NLM_UNLOCK_MSG,  "NLM_PROG", "NLM_UNLOCK_MSG", 
				xdr_nlm_unlockargs, prt_nlm_unlockargs, 0,0},
{NLM_PROG, NLM_GRANTED_MSG, "NLM_PROG", "NLM_GRANTED_MSG", 
				xdr_nlm_testargs, prt_nlm_testargs, 0,0},
{NLM_PROG, NLM_TEST_RES,    "NLM_PROG", "NLM_TEST_RES",    
				xdr_nlm_testres, prt_nlm_testres,0,0},
{NLM_PROG, NLM_LOCK_RES,    "NLM_PROG", "NLM_LOCK_RES",   
				xdr_nlm_res, prt_nlm_res,0,0},
{NLM_PROG, NLM_CANCEL_RES,  "NLM_PROG", "NLM_CANCEL_RES",
				xdr_nlm_res, prt_nlm_res,0,0},
{NLM_PROG, NLM_UNLOCK_RES,  "NLM_PROG", "NLM_UNLOCK_RES",
				xdr_nlm_res, prt_nlm_res,0,0},
{NLM_PROG, NLM_GRANTED_RES,  "NLM_PROG", "NLM_GRANTED_RES",
				xdr_nlm_res, prt_nlm_res,0,0},
{NLM_PROG, NLM_NM_LOCK,     "NLM_PROG", "NLM_NM_LOCK",     
				xdr_nlm_lockargs, prt_nlm_lockargs, 
				xdr_nlm_res, prt_nlm_res},
{NLM_PROG, NLM_SHARE,       "NLM_PROG", "NLM_SHARE",       
				xdr_nlm_shareargs,prt_nlm_shareargs,
				xdr_nlm_shareres,prt_nlm_shareres},
{NLM_PROG, NLM_UNSHARE,     "NLM_PROG", "NLM_UNSHARE",     
				xdr_nlm_shareargs,prt_nlm_shareargs,
				xdr_nlm_shareres,prt_nlm_shareres},
{NLM_PROG, NLM_FREE_ALL,    "NLM_PROG", "NLM_FREE_ALL",    
				xdr_nlm_notify, prt_nlm_notify,
				xdr_void,	prt_void},
{YPPROG, YPPROC_NULL,          "YPPROG", "YPPROC_NULL"          ,0,0,0,0}, 
{YPPROG, YPPROC_DOMAIN,        "YPPROG", "YPPROC_DOMAIN"        ,0,0,0,0}, 
{YPPROG, YPPROC_DOMAIN_NONACK, "YPPROG", "YPPROC_DOMAIN_NONACK",
                            xdr_ypdomain_wrap_string, prt_ypdomain_wrap_string,
                            xdr_int, prt_ypproc_domain_noack_res},


{YPPROG, YPPROC_MATCH,         "YPPROG", "YPPROC_MATCH",
					xdr_ypreq_key, prt_ypreq_key, 
					xdr_ypresp_val, prt_ypresp_val}, 
{YPPROG, YPPROC_FIRST,         "YPPROG", "YPPROC_FIRST",
					xdr_ypreq_nokey, prt_ypreq_nokey,
					xdr_ypresp_key_val, prt_ypresp_key_val},
{YPPROG, YPPROC_NEXT,          "YPPROG", "YPPROC_NEXT",
					xdr_ypreq_key, prt_ypreq_key, 
					xdr_ypresp_key_val, prt_ypresp_key_val},
/* xfr has no reply */
{YPPROG, YPPROC_XFR,           "YPPROG", "YPPROC_XFR",
					xdr_ypreq_xfr, prt_ypreq_xfr,
					0,0},
/* clear has no args or reply */
{YPPROG, YPPROC_CLEAR,         "YPPROG", "YPPROC_CLEAR"         ,0,0,0,0}, 
{YPPROG, YPPROC_ALL,           "YPPROG", "YPPROC_ALL",
					xdr_ypreq_nokey, prt_ypreq_nokey,
					0, 0},
{YPPROG, YPPROC_MASTER,        "YPPROG", "YPPROC_MASTER",
					xdr_ypreq_nokey, prt_ypreq_nokey,
					xdr_ypresp_master, prt_ypresp_master},
{YPPROG, YPPROC_ORDER,         "YPPROG", "YPPROC_ORDER",
					xdr_ypreq_nokey, prt_ypreq_nokey,
					xdr_ypresp_order, prt_ypresp_order},
{YPPROG, YPPROC_MAPLIST,       "YPPROG", "YPPROC_MAPLIST",
			xdr_ypdomain_wrap_string, prt_ypdomain_wrap_string,
			xdr_ypresp_maplist, prt_ypresp_maplist},

{YPU_PROG,  YPU_CHANGE,          "YPU_PROG",  "YPU_CHANGE",          0,0,0,0}, 
{YPU_PROG,  YPU_INSERT,          "YPU_PROG",  "YPU_INSERT",          0,0,0,0}, 
{YPU_PROG,  YPU_DELETE,          "YPU_PROG",  "YPU_DELETE",          0,0,0,0}, 
{YPU_PROG,  YPU_STORE,           "YPU_PROG",  "YPU_STORE",           0,0,0,0}, 

{MOUNTPROG, MOUNTPROC_MNT,       "MOUNTPROG", "MOUNTPROC_MNT",       0,0,0,0}, 
{MOUNTPROG, MOUNTPROC_DUMP,      "MOUNTPROG", "MOUNTPROC_DUMP",      0,0,0,0}, 
{MOUNTPROG, MOUNTPROC_UMNT,      "MOUNTPROG", "MOUNTPROC_UMNT",      0,0,0,0}, 
{MOUNTPROG, MOUNTPROC_UMNTALL,   "MOUNTPROG", "MOUNTPROC_UMNTALL",   0,0,0,0}, 
{MOUNTPROG, MOUNTPROC_EXPORT,    "MOUNTPROG", "MOUNTPROC_EXPORT",    0,0,0,0}, 
{MOUNTPROG, MOUNTPROC_EXPORTALL, "MOUNTPROG", "MOUNTPROC_EXPORTALL", 0,0,0,0}, 

{PMAPPROG,  PMAPPROC_NULL,       "PMAPPROG",  "PMAPPROC_NULL",       0,0,0,0}, 
{PMAPPROG,  PMAPPROC_SET,        "PMAPPROG",  "PMAPPROC_SET",        
				xdr_pmap, prt_pmap,
				xdr_long, prt_long},
{PMAPPROG,  PMAPPROC_UNSET,      "PMAPPROG",  "PMAPPROC_UNSET",      
				xdr_pmap, prt_pmap,
				xdr_long, prt_long},
{PMAPPROG,  PMAPPROC_GETPORT,    "PMAPPROG",  "PMAPPROC_GETPORT",   
				xdr_pmap, prt_pmap,
				xdr_long, prt_long},
{PMAPPROG,  PMAPPROC_DUMP,       "PMAPPROG",  "PMAPPROC_DUMP",     
				xdr_pmap, prt_pmap,
				xdr_pmaplist, prt_pmaplist},
{PMAPPROG,  PMAPPROC_CALLIT,     "PMAPPROG",  "PMAPPROC_CALLIT",  
				xdr_rmtcall_args, prt_rmtcall_args,
				xdr_rmtcall_result, prt_rmtcall_result},

{SM_PROG,     SM_STAT,       "SM_PROG",     "SM_STAT",       0,0,0,0}, 
{SM_PROG,     SM_MON,        "SM_PROG",     "SM_MON",        0,0,0,0}, 
{SM_PROG,     SM_UNMON,      "SM_PROG",     "SM_UNMON",      0,0,0,0}, 
{SM_PROG,     SM_UNMON_ALL,  "SM_PROG",     "SM_UNMON_ALL",  0,0,0,0}, 
{SM_PROG,     SM_SIMU_CRASH, "SM_PROG",     "SM_SIMU_CRASH", 0,0,0,0}, 

{REXPROG      ,-1,           "REXPROG"      ,""              ,0,0,0,0}, 
{RSTATPROG    ,-1,           "RSTATPROG"    ,""              ,0,0,0,0}, 
{RUSERSPROG   ,-1,           "RUSERSPROG"   ,""              ,0,0,0,0}, 
{SPRAYPROG    ,-1,           "SPRAYPROG"    ,""              ,0,0,0,0}, 
{WALLPROG     ,-1,           "WALLPROG"     ,""              ,0,0,0,0}, 
{YPBINDPROG   ,-1,           "YPBINDPROG"   ,""              ,0,0,0,0}, 
{YPPASSWDPROG ,YPPASSWDPROC_UPDATE,"YPPASSWDPROG","YPPASSWDPROC_UPDATE",
             xdr_yppasswd, prt_yppasswd, xdr_int, prt_yppasswd_response }, 

{PCNFSDPROG, PCNFSD_AUTH,     "PCNFSDPROG", "PCNFSD_AUTH",     
				xdr_auth_args, prt_auth_args,
				xdr_auth_results, prt_auth_results},
{PCNFSDPROG, PCNFSD_PR_INIT,  "PCNFSDPROG", "PCNFSD_PR_INIT",  
				xdr_pr_init_args, prt_pr_init_args,
				xdr_pr_init_results, prt_pr_init_results},
{PCNFSDPROG, PCNFSD_PR_START, "PCNFSDPROG", "PCNFSD_PR_START", 
				xdr_pr_start_args, prt_pr_start_args,
				xdr_pr_start_results, prt_pr_start_results},



	{ 0, 0, "", "", 0, 0, 0, 0 }
       };

struct procst {
	struct procst *p_next;
	struct data    p_d;
};


static struct procst *proctology= (struct procst *) 0;


init_procst()
{
	struct procst *p;
	int i;

	init_yet=1;

	/* we stop when the struct is zero. 
	*/
	for (i=0; 	data[i].d_prog || data[i].d_proc; 	i++) {
		p = mymalloc(sizeof(struct procst));
		p->p_d = data[i];		/* copy the struct */
		p->p_next=proctology;		
		proctology=p;			/* reset the head */
	}
}
		

/* Search thru linked list of procst's
   and return a pointer to a function that returns an int.
*/
int (*get_proc(int which, int prog, int proc))()
{
	struct procst *p;
	int	(*rv)()=NULL;

	if (!init_yet)
		init_procst();
	
	for (p=proctology; p; p=p->p_next)
		if ((prog == p->p_d.d_prog) && (proc == p->p_d.d_proc)) 
			switch (which) {
				case XDR_REPLY:  rv = p->p_d.d_xdrrep;   break; 
				case XDR_CALL:   rv = p->p_d.d_xdrcall;  break; 
				case DUMP_REPLY: rv = p->p_d.d_dumprep;  break; 
				case DUMP_CALL:  rv = p->p_d.d_dumpcall; break; 
				default:	
					perror("BUG IN GET_PROC!\n");
					exit(1);
			}

	if (!rv)
		switch (which) {
			case XDR_REPLY:  rv = xdr_hexcopy; break; 
			case XDR_CALL:   rv = xdr_hexcopy; break; 
			case DUMP_REPLY: rv = hc_hexdump;  break; 
			case DUMP_CALL:  rv = hc_hexdump;  break; 
			default:	
				perror("BUG IN GET_PROC!\n");
				exit(1);
		}

	return(rv);
}

/* Search thru linked list of procst's
   and return a pointer to the program name;
*/
char *get_prog_name(int prog)
{
	struct procst *p;
	static char   *u_pg="UNKNOWN_PROG";

	if (!init_yet)
		init_procst();
	
	for (p=proctology; p; p=p->p_next)
		if (prog == p->p_d.d_prog) 
				return(p->p_d.d_progname);
	return(u_pg);
}

/* Search thru linked list of procst's
   and return a pointer to procedure name;
*/
char *get_proc_name(int prog, int proc)
{
	struct procst *p;
	static char   *u_pc="UNKNOWN_PROC";

	if (!init_yet)
		init_procst();
	
	for (p=proctology; p; p=p->p_next)
		if ((prog == p->p_d.d_prog) && (proc == p->p_d.d_proc)) 
				return(p->p_d.d_procname);
	return(u_pc);
}
