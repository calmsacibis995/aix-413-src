/* @(#)17     1.14  src/nfs/usr/sbin/rpc.lockd/prot_lock.h, cmdnfs, nfs411, 9436E411a 9/9/94 09:46:19 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 27
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
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 * 
 */

/*
 * prot_lock.h	1.1 88/08/05 NFSSRC4.0 1.8 87/03/09
 *
 * This file consists of all structure information used by lock manager 
 */

#include <rpc/rpc.h>
#include <rpcsvc/nlm_prot.h>
#include <rpcsvc/klm_prot.h>

typedef struct nlm_testres remote_result;
#define lstat stat.stat
#define lholder stat.nlm_testrply_u.holder

#define LOCKD_MAXHOSTNAMELEN	512
#define NLM_LOCK_RECLAIM	16
#undef MSG
#define MSG 	0		/* choices of comm to remote svr */
#define RPC 	1		/* choices of comm to remote svr */

#define MAXLEN		017777777777
#define lck 		alock
#define svr		server_name
#define caller		caller_name
#define clnt		clnt_name
#undef fh_len
#define fh_len		fh.n_len
#define fh_bytes	fh.n_bytes
#define oh_len		oh.n_len
#define oh_bytes	oh.n_bytes
#define cookie_len	cookie.n_len
#define cookie_bytes	cookie.n_bytes

#define granted		nlm_granted
#define denied		nlm_denied
#define nolocks 	nlm_denied_nolocks
#define blocking	nlm_blocked
#define grace		nlm_denied_grace_period
#define deadlck		nlm_deadlck
#define rpc_error	6

/*
 * warning:  struct alock consists of klm_lock and nlm_lock,
 * it has to be modified if either structure has been modified!!!
 */
struct alock {
	/* from klm_prot.h */
	char *server_name;
	netobj fh;
	int pid;
	u_int l_offset;
	u_int l_len;

	/* addition from nlm_prot.h */
	char *caller_name;
	netobj oh;
	int svid;

	/* addition from lock manager */
	char *clnt_name;
	u_int ub;
	int op;
	int msg;
};


struct reclock {
	netobj cookie;
	bool_t block;
	bool_t exclusive;
	struct alock alock;
	bool_t reclaim;
	int state;

	/* auxiliary structure */
	int rel;			/* rel =1, to release this lock when appropriate; otherwise, rel = 0; */
	int w_flag;			/* w_falg =1, lock in wait queue; otherwise, w_flag = 0; */
	int a_id;			/* alarm id for grace period usage*/
	SVCXPRT *transp;		/* transport handle for delayed response due to blocking or grace period*/
	/* various links */
	struct reclock *prev;		/* backward ptr to other reclock in the same file system */
	struct reclock *nxt;		/* forward prt to other reclock in the same file system */
	struct reclock *wait_prev;	/* backward ptr to other reclock of the same process */
	struct reclock *wait_nxt;		/* forward ptr to other reclock of the same process */
	struct reclock *pre_le;		/* ptr to preallocated le entry */
	char *pre_fe;			/* ptr to preallocated fe entry; cannot usr struct fs_rlck because prob of recurrsive def */
	ulong retry_id;			/* retry id assigned by kernal for sleeping lock */
	ulong sysid;			/* sysid for lock (internet address of requesting client)*/
};
typedef struct reclock reclock;

typedef struct LLNode {
  void *data;
  struct LLNode *mnt_nxt;
  struct LLNode *mnt_prev;
} LLNode;

/*
 * struct fs_rlck consists of a file_id (svrname, file handle)
 * or a monitor_id (svrname, procedure name) and ptr to a list
of
 * record locks on the same file system or same monitor requirement
 */

struct fs_rlck {
	char *svr;
	union {
		netobj fh;			/* file handle */
		int procedure;			/* procedure name */
	} fs;
	reclock *rlckp;
	LLNode *mntlist;
	struct fs_rlck *prev;
	struct fs_rlck *nxt;
};


struct timer {
	/* timer goes off when exp == curr */
	int exp;
	int curr;
};
typedef struct timer timer;

/*
 * msg passing structure
 */
struct msg_entry {
	reclock *req;
	remote_result *reply;
	timer t;
	int proc;		/* procedure name that req is sent to; needed for reply purpose */
	SVCXPRT *transp;
	struct msg_entry *prev;
	struct msg_entry *nxt;
};
typedef struct msg_entry msg_entry;

struct priv_struct {
	int pid;
	int *priv_ptr;
};
