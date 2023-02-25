/* @(#)77	1.1  src/nfs/usr/include/nfs/pacl.h, sysxnfs, nfs411, GOLD410 12/3/93 16:54:45 */
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
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* This file was generated by rpcget on the pacl.x spec */

#ifndef __PACL_HEADER__
#define __PACL_HEADER__

enum access_id_type {
	ACCESS_USER_XDR = 1,
	ACCESS_GROUP_XDR = 2,
};
typedef enum access_id_type access_id_type;
bool_t xdr_access_id_type();


enum access_type {
	ACC_PERMIT_XDR = 1,
	ACC_DENY_XDR = 2,
	ACC_SPECIFY_XDR = 3,
};
typedef enum access_type access_type;
bool_t xdr_access_type();


enum access_permissions {
	ACL_EXECUTE = 0x01,
	ACL_WRITE = 0x02,
	ACL_READ = 0x04,
};
typedef enum access_permissions access_permissions;
bool_t xdr_access_permissions();


typedef struct {
	u_int acc_permlist_len;
	access_permissions *acc_permlist_val;
} acc_permlist;
bool_t xdr_acc_permlist();


enum acl_mode_type {
	ACL_IACL = 0100000000,
	ACL_ISUID = 0000004000,
	ACL_ISGID = 0000002000,
	ACL_ISVTX = 0000001000,
};
typedef enum acl_mode_type acl_mode_type;
bool_t xdr_acl_mode_type();


struct access_identifier {
	access_id_type id_type;
	union {
		struct {
			u_int uids_len;
			u_long *uids_val;
		} uids;
		struct {
			u_int gids_len;
			u_long *gids_val;
		} gids;
	} access_identifier_u;
};
typedef struct access_identifier access_identifier;
bool_t xdr_access_identifier();


struct acl_entry_xdr {
	access_type ace_type;
	acc_permlist ace_access;
	struct {
		u_int ace_entries_len;
		access_identifier *ace_entries_val;
	} ace_entries;
};
typedef struct acl_entry_xdr acl_entry_xdr;
bool_t xdr_acl_entry_xdr();


struct acl_xdr {
	struct {
		u_int acl_mode_len;
		acl_mode_type *acl_mode_val;
	} acl_mode;
	acc_permlist u_access;
	acc_permlist g_access;
	acc_permlist o_access;
	struct {
		u_int acl_entry_len;
		acl_entry_xdr *acl_entry_val;
	} acl_entry;
};
typedef struct acl_xdr acl_xdr;
bool_t xdr_acl_xdr();


enum pcl_privileges {
	PCL_BYPASS_DAC_WRITE=1,	/* write all objects */
	PCL_BYPASS_DAC_READ=2,	/* read all objects (inc dir search */
	PCL_BYPASS_DAC_EXEC=3,	/* execute all programs */
	PCL_BYPASS_DAC_KILL=4,	/* signal all processes */
	PCL_BYPASS_RAC=5,	/* consume all resources */
	PCL_BYPASS_MAC_WRITE=6,	/* read all objects */
	PCL_BYPASS_MAC_READ=7,	/* write all objects */
	PCL_BYPASS_TPATH=8,	/* do actions where tpath is required */
	PCL_BYPASS_DAC=9,

	PCL_SET_OBJ_DAC=10,	/* setting object owner, group, mode, ACL */
	PCL_SET_OBJ_RAC=11,	/* not used */
	PCL_SET_OBJ_MAC=12,	/* setting object MAC sensitivity label */
	PCL_SET_OBJ_INFO=13,	/* setting object MAC information label */
	PCL_SET_OBJ_STAT=14,	/* setting misc. attributes */
	PCL_SET_OBJ_PRIV=15,	/* setting object PCL and TP, TCB attributes */

	PCL_SET_PROC_DAC=20,	/* setting procs real uid, gid and group set */
	PCL_SET_PROC_RAC=21,	/* setting procs resource limits, quotas */
	PCL_SET_PROC_MAC=22,	/* setting procs MAC sensitivity label */
	PCL_SET_PROC_INFO=23,	/* setting procs MAC information label */
	PCL_SET_PROC_ENV=24,	/* setting procs protected environment */
	PCL_SET_PROC_ACCT=25,	/* not used */
	PCL_SET_PROC_AUDIT=26,	/* setting procs audit classes and ID */

	PCL_AUDIT_CONFIG=40,	/* config auditing, bin and stream modes */
	PCL_ACCT_CONFIG=41,	/* enabling accounting */
	PCL_DEV_CONFIG=42,	/* configuring hardware */
	PCL_FS_CONFIG=43,	/* mounting filesystems, chroots */
	PCL_GSS_CONFIG=44,	/* configuring X, graphaics subsystems */
	PCL_LVM_CONFIG=45,	/* congiguring the Logical Volume Manager */
	PCL_NET_CONFIG=46,	/* network (SNA, TCP/IP, OSI) configuration */
	PCL_RAS_CONFIG=47,	/* configuring and writing RAS records */
	PCL_RAC_CONFIG=48,	/* not used */
	PCL_SYS_CONFIG=49,	/* adding/removing kernel extensions */
	PCL_SYS_OPER=50,	/* setting time, system naming info */
	PCL_TPATH_CONFIG=51,	/* setting terminal SAK, Trusted state */
	PCL_VMM_CONFIG=52,	/* defining paging space */
};

typedef enum pcl_privileges pcl_privileges;
bool_t xdr_pcl_privileges();


typedef struct {
	u_int pcl_privlist_len;
	pcl_privileges *pcl_privlist_val;
} pcl_privlist;
bool_t xdr_pcl_privlist();


struct pcl_entry_xdr {
	pcl_privlist pce_access;
	struct {
		u_int pce_entries_len;
		access_identifier *pce_entries_val;
	} pce_entries;
};
typedef struct pcl_entry_xdr pcl_entry_xdr;
bool_t xdr_pcl_entry_xdr();


enum pcl_mode_type {
	PRIV_S_ITP = 1,
	PRIV_S_ITCB = 2,
};
typedef enum pcl_mode_type pcl_mode_type;
bool_t xdr_pcl_mode_type();


struct pcl_xdr {
	struct {
		u_int pcl_mode_len;
		pcl_mode_type *pcl_mode_val;
	} pcl_mode;
	pcl_privlist pce_access_default;
	struct {
		u_int pcl_entry_len;
		pcl_entry_xdr *pcl_entry_val;
	} pcl_entry;
};
typedef struct pcl_xdr pcl_xdr;
bool_t xdr_pcl_xdr();


typedef struct {
	u_int fh_cookie_len;
	char *fh_cookie_val;
} fh_cookie;
bool_t xdr_fh_cookie();


struct acl_pcl_cookie {
	fh_cookie cookie;
	struct {
		u_int acl_pcl_id_len;
		char *acl_pcl_id_val;
	} acl_pcl_id;
	u_long size;
};
typedef struct acl_pcl_cookie acl_pcl_cookie;
bool_t xdr_acl_pcl_cookie();


struct rep_acl_pcl_cookie {
	int status;
	union {
		acl_pcl_cookie cookie;
		int error;
	} rep_acl_pcl_cookie_u;
};
typedef struct rep_acl_pcl_cookie rep_acl_pcl_cookie;
bool_t xdr_rep_acl_pcl_cookie();


struct acl_pcl_block {
	acl_pcl_cookie cookie;
	u_long offset;
	u_long count;
	struct {
		u_int acl_pcl_data_len;
		char *acl_pcl_data_val;
	} acl_pcl_data;
	bool_t eof;
};
typedef struct acl_pcl_block acl_pcl_block;
bool_t xdr_acl_pcl_block();


struct acl_pcl_req_block {
	acl_pcl_cookie cookie;
	u_long offset;
	u_long count;
};
typedef struct acl_pcl_req_block acl_pcl_req_block;
bool_t xdr_acl_pcl_req_block();

#define RESTART_TRANSACTION -1

struct acl_pcl_reply {
	int status;
	union {
		acl_pcl_block block_reply;
		int error;
	} acl_pcl_reply_u;
};
typedef struct acl_pcl_reply acl_pcl_reply;
bool_t xdr_acl_pcl_reply();


struct acl_pcl_inuse {
	int error;
	bool_t acl_inuse;
	bool_t pcl_inuse;
};
typedef struct acl_pcl_inuse acl_pcl_inuse;
bool_t xdr_acl_pcl_inuse();


#define ACL_PCL_PROG ((u_long)200006)
#define INITIAL ((u_long)1)
#define ACL_PCL_STAT ((u_long)1)
extern acl_pcl_inuse *acl_pcl_stat_1();
#define ACL_STAT_LONG ((u_long)2)
extern rep_acl_pcl_cookie *acl_stat_long_1();
#define PCL_STAT_LONG ((u_long)3)
extern rep_acl_pcl_cookie *pcl_stat_long_1();
#define ACL_READ ((u_long)4)
extern acl_pcl_reply *acl_read_1();
#define PCL_READ ((u_long)5)
extern acl_pcl_reply *pcl_read_1();
#define ACL_WRITE ((u_long)6)
extern int *acl_write_1();
#define PCL_WRITE ((u_long)7)
extern int *pcl_write_1();

#endif
