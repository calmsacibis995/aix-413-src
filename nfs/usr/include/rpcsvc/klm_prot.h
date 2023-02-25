/* @(#)86     1.7  src/nfs/usr/include/rpcsvc/klm_prot.h, cmdnfs, nfs411, 9436E411a 9/9/94 09:59:36 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */
/* (#)klm_prot.h	1.1 12/13/88 17:43:37 */
/*	(#)klm_prot.h	1.2 88/07/11 4.0NFSSRC SMI	*/

/* 
 * (#) from SUN 1.6
 */
#ifndef _H_rpcsvc_klm_prot 
#define _H_rpcsvc_klm_prot

#define KLM_PROG 100020
#define KLM_VERS 1
#define KLM_TEST 1
#define KLM_LOCK 2
#define KLM_CANCEL 3
#define KLM_UNLOCK 4
#define KLM_GRANTED 5

#define LM_MAXSTRLEN 1024

enum klm_stats {
	klm_granted = 0,
	klm_denied = 1,
	klm_denied_nolocks = 2,
	klm_working = 3,
	klm_deadlck = 5,        /* lock not granted due to deadlock situation */
};
typedef enum klm_stats klm_stats;
bool_t xdr_klm_stats();


struct klm_lock {
	char *server_name;
	netobj fh;
	int pid;
	u_int l_offset;
	u_int l_len;
};
typedef struct klm_lock klm_lock;
bool_t xdr_klm_lock();


struct klm_holder {
	bool_t exclusive;
	int svid;
	u_int l_offset;
	u_int l_len;
};
typedef struct klm_holder klm_holder;
bool_t xdr_klm_holder();


struct klm_stat {
	klm_stats stat;
};
typedef struct klm_stat klm_stat;
bool_t xdr_klm_stat();


struct klm_testrply {
	klm_stats stat;
	union {
		struct klm_holder holder;
	} klm_testrply_u;
};
typedef struct klm_testrply klm_testrply;
bool_t xdr_klm_testrply();


struct klm_lockargs {
	bool_t block;
	bool_t exclusive;
	struct klm_lock alock;
};
typedef struct klm_lockargs klm_lockargs;
bool_t xdr_klm_lockargs();


struct klm_testargs {
	bool_t exclusive;
	struct klm_lock alock;
};
typedef struct klm_testargs klm_testargs;
bool_t xdr_klm_testargs();


struct klm_unlockargs {
	struct klm_lock alock;
};
typedef struct klm_unlockargs klm_unlockargs;
bool_t xdr_klm_unlockargs();

#endif /*_H_rpcsvc_klm_prot*/
