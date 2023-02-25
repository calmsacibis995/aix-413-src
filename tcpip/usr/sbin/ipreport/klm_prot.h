/* @(#)74	1.1  src/tcpip/usr/sbin/ipreport/klm_prot.h, tcpras, tcpip411, GOLD410 10/24/91 18:13:30 */
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
/* (#)86	1.4  klm_prot.h, cmdnfs, nfs320 9/21/89 13:35:26 */

/* 
 * (#) from SUN 1.6
 */

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

