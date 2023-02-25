/* @(#)33 1.4  src/bos/usr/sbin/acct/ptmp.h, cmdacct, bos411, 9428A410j 5/6/93 16:39:06 */
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3,9,27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	per process temporary data
 */
struct ptmp {
	uid_t	pt_uid;			/* userid */
	char	pt_name[8];		/* login name */
	float   pt_cpu[2];		/* CPU (sys+usr) P/NP time tics */
	long    pt_io;                  /* chars transferred P/NP */
	long    pt_rw;                  /* blocks read/written P/NP */
	unsigned pt_mem;		/* avg. memory size (64byte clicks) */
};	
