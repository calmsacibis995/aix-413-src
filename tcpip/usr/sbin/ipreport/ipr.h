/* @(#)87	1.7  src/tcpip/usr/sbin/ipreport/ipr.h, tcpras, tcpip411, GOLD410 4/8/94 09:50:42 */
/*
 * COMPONENT_NAME: TCPIP ipr.h
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
 * This is all new code
 *
 *
 */


/* call info struct :
   	we keep track of rpc calls since rpc replys don't say which prog and
	proc they r a reply to
*/
struct cinfo {
	int	ci_reqid;	/* rpc transaction id */
	int	ci_program;	/* program and procedure we are asking the */
	int	ci_procedure;	/* question of */
	u_long	ci_orighost;	/* in_addr of client in this call */
	struct 	cinfo *ci_next;
};

bool_t c_create_dump();
bool_t c_getattr_dump();
bool_t c_link_dump();
bool_t c_lookup_dump();
bool_t c_mkdir_dump();
bool_t c_null_dump();
bool_t c_read_dump();
bool_t c_readdir_dump();
bool_t c_readlink_dump();
bool_t c_remove_dump();
bool_t c_rename_dump();
bool_t c_rmdir_dump();
bool_t c_setattr_dump();
bool_t c_statfs_dump();
bool_t c_symlink_dump();
bool_t c_write_dump();
bool_t hc_hexdump();
bool_t myxdr_void();
bool_t prt_attrstat();
bool_t prt_auth_args();
bool_t prt_auth_results();
bool_t prt_diropres();
bool_t prt_fhandle();
bool_t prt_getrddirres();
bool_t prt_long();
bool_t prt_nfsstat();
bool_t prt_nlm_cancargs();
bool_t prt_nlm_holder();
bool_t prt_nlm_lock();
bool_t prt_nlm_lockargs();
bool_t prt_nlm_notify();
bool_t prt_nlm_res();
bool_t prt_nlm_share();
bool_t prt_nlm_shareargs();
bool_t prt_nlm_shareres();
bool_t prt_nlm_stat();
bool_t prt_nlm_stats();
bool_t prt_nlm_testargs();
bool_t prt_nlm_testres();
bool_t prt_nlm_testrply();
bool_t prt_nlm_unlockargs();
bool_t prt_pmap();
bool_t prt_pmaplist();
bool_t prt_pr_init_args();
bool_t prt_pr_init_results();
bool_t prt_pr_start_args();
bool_t prt_pr_start_results();
bool_t prt_rdlnres();
bool_t prt_rdresult();
bool_t prt_rmtcall_args();
bool_t prt_rmtcall_result();
bool_t prt_star_nfsstat();
bool_t prt_statfs();
bool_t prt_void();
bool_t prt_ypall();
bool_t prt_ypdomain_wrap_string();
bool_t prt_ypproc_domain_noack_res();
bool_t prt_ypreq_key();
bool_t prt_ypreq_nokey();
bool_t prt_ypreq_xfr();
bool_t prt_ypresp_key_val();
bool_t prt_ypresp_maplist();
bool_t prt_ypresp_master();
bool_t prt_ypresp_order();
bool_t prt_ypresp_val();
bool_t xdr_attrstat();
bool_t xdr_auth_args();
bool_t xdr_auth_results();
bool_t xdr_creatargs();
bool_t xdr_createargs();
bool_t xdr_diropargs();
bool_t xdr_diropres();
bool_t xdr_enum_nfsstat();
bool_t xdr_fhandle();
bool_t xdr_fsh_access();
bool_t xdr_fsh_mode();
bool_t xdr_getrddirres();
bool_t xdr_hexcopy();
bool_t xdr_hexdump();
bool_t xdr_int();
bool_t xdr_linkargs();
bool_t xdr_long();
bool_t xdr_nlm_cancargs();
bool_t xdr_nlm_holder();
bool_t xdr_nlm_lock();
bool_t xdr_nlm_lockargs();
bool_t xdr_nlm_notify();
bool_t xdr_nlm_res();
bool_t xdr_nlm_share();
bool_t xdr_nlm_shareargs();
bool_t xdr_nlm_shareres();
bool_t xdr_nlm_stat();
bool_t xdr_nlm_stats();
bool_t xdr_nlm_testargs();
bool_t xdr_nlm_testres();
bool_t xdr_nlm_testrply();
bool_t xdr_nlm_unlockargs();
bool_t xdr_pmap();
bool_t xdr_pmaplist();
bool_t xdr_pr_init_args();
bool_t xdr_pr_init_results();
bool_t xdr_pr_start_args();
bool_t xdr_pr_start_results();
bool_t xdr_rddirargs();
bool_t xdr_rdlnres();
bool_t xdr_rdresult();
bool_t xdr_readargs();
bool_t xdr_rmtcall_args();
bool_t xdr_rmtcall_result();
bool_t xdr_rnmargs();
bool_t xdr_saargs();
bool_t xdr_slargs();
bool_t xdr_statfs();
bool_t xdr_writeargs();
bool_t prt_acl_pcl_block();
bool_t prt_acl_pcl_reply();
bool_t prt_acl_pcl_req_block();
bool_t prt_fh_cookie();
bool_t prt_rep_acl_pcl_cookie();
bool_t xdr_acl_pcl_block();
bool_t xdr_acl_pcl_reply();
bool_t xdr_acl_pcl_req_block();
bool_t xdr_fh_cookie();
bool_t xdr_rep_acl_pcl_cookie();
bool_t xdr_yppasswd();
bool_t prt_yppasswd();
bool_t prt_yppasswd_response();
 
#define	MAXPACKET	4096	/* max packet size */
#define XDR_REPLY	1
#define XDR_CALL	2
#define DUMP_REPLY	3
#define DUMP_CALL	4
extern char *beg_line;
extern char *lok_beg;
extern char *pm_beg;
extern char *nfs_beg;
extern char *acl_beg;
#define abs(x) (((x) >= 0) ? (x) : (-x))
#define PCNFSDPROG      (long)150001
#define PCNFSDVERS      (long)1
#define PCNFSD_AUTH     (long)1
#define PCNFSD_PR_INIT  (long)2
#define PCNFSD_PR_START (long)3

