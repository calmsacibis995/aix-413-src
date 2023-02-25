/* @(#)36	1.4  src/nfs/usr/sbin/rpc.statd/sm_statd.h, cmdnfs, nfs411, GOLD410 3/3/94 17:45:42 */
/* 
 * COMPONENT_NAME: (CMDNFS) Network File System Commands
 * 
 * FUNCTIONS: 
 *
 * ORIGINS: 24 
 *
 *                  SOURCE MATERIALS
 *
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 
 */

/*	(#)sm_statd.h	1.1 88/04/20 4.0NFSSRC SMI	*/


struct stat_chge {
	char *name;
	int state;
};
typedef struct stat_chge stat_chge;

#define SM_NOTIFY 6
#define STATD_MAXHOSTNAMELEN MAXNAMLEN



