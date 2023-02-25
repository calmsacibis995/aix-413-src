/* @(#)20 1.2.1.1  src/bos/usr/sbin/acct/ctmp.h, cmdacct, bos411, 9428A410j 5/6/93 16:39:23 */
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
 *	connect time record (various intermediate files)
 */
struct ctmp {
	dev_t	ct_tty;			/* major minor */
	uid_t	ct_uid;			/* userid */
	char	ct_name[8];		/* login name */
	long	ct_con[2];		/* connect time (p/np) secs */
	time_t	ct_start;		/* session start time */
};
