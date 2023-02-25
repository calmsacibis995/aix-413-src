/* @(#)25	1.4  src/nfs/usr/sbin/rpc.lockd/prot_time.h, cmdnfs, nfs411, GOLD410 3/11/94 18:36:27 */
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

/*
 * (#)prot_time.h	1.1 88/08/05 NFSSRC4.0 1.6 88/02/07
 *
 * This file consists of all timeout definition used by rpc.lockd
 */

#define MAX_LM_TIMEOUT_COUNT	50		/* max timeout period is 100* default */
#define OLDMSG			500		/* counter to throw away old msg */
#define LM_TIMEOUT_DEFAULT 	60	/* 60 millisecond default timeout */
#define LM_GRACE_DEFAULT 	500	/* 500 timeouts for the grace period */
extern int 	LM_TIMEOUT;
extern int 	LM_GRACE;
extern int	grace_period;
