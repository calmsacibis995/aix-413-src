/* @(#)94	1.5  src/nfs/usr/include/rpcsvc/sm_inter.h, cmdnfs, nfs411, GOLD410 1/17/94 14:33:01 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * (#) from SUN 1.5
 * 
 */

/* (#)sm_inter.h	1.1 12/13/88 17:46:48 */
/*	(#)sm_inter.h	1.2 88/07/11 4.0NFSSRC SMI	*/

#ifndef _H_rpcsvc_sm_inter
#define _H_rpcsvc_sm_inter

#define SM_PROG 100024
#define SM_VERS 1
#define SM_STAT 1
#define SM_MON 2
#define SM_UNMON 3
#define SM_UNMON_ALL 4
#define SM_SIMU_CRASH 5

#define SM_MAXSTRLEN 1024

struct sm_name {
	char *mon_name;
};
typedef struct sm_name sm_name;
bool_t xdr_sm_name();


struct my_id {
	char *my_name;
	int my_prog;
	int my_vers;
	int my_proc;
};
typedef struct my_id my_id;
bool_t xdr_my_id();


struct mon_id {
	char *mon_name;
	struct my_id my_id;
};
typedef struct mon_id mon_id;
bool_t xdr_mon_id();


struct mon {
	struct mon_id mon_id;
	char priv[16];
};
typedef struct mon mon;
bool_t xdr_mon();


struct sm_stat {
	int state;
};
typedef struct sm_stat sm_stat;
bool_t xdr_sm_stat();


enum res {
	stat_succ = 0,
	stat_fail = 1,
};
typedef enum res res;
bool_t xdr_res();


struct sm_stat_res {
	res res_stat;
	int state;
};
typedef struct sm_stat_res sm_stat_res;
bool_t xdr_sm_stat_res();


struct status {
	char *mon_name;
	int state;
	char priv[16];
};
typedef struct status status;
bool_t xdr_status();

#endif /*_H_rpcsvc_sm_inter*/
