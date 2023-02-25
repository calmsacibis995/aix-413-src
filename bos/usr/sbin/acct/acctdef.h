/* @(#)09       1.4.1.2  src/bos/usr/sbin/acct/acctdef.h, cmdacct, bos411, 9428A410j 3/30/94 15:49:22 */
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	defines, typedefs, etc. used by acct programs
 */

/*
 *	acct only typedefs
 */

/*typedef	unsigned short	uid_t; */

/* #define HZ	60 */
#include <sys/param.h>  /* has system defined HZ, which is changing from 60 to 100 tics */

#define LSZ	12	/* sizeof line name */
#define NSZ	8	/* sizeof login name */
#define P	0	/* prime time */
#define NP	1	/* nonprime time */

/*
 *	limits which may have to be increased if systems get larger
 */
#define A_SSIZE	1000	/* max number of sessions in 1 acct run */
#define A_TSIZE	100	/* max number of line names in 1 acct run */
#define A_USIZE	500	/* max number of distinct login names in 1 acct run */

#define UHASH   401     /* User hash size < USIZE, works best if prime */
#define THASH   79      /* Terminal hash size < TSIZE, works best if prime */

#define EQN(s1, s2)	(strncmp(s1, s2, sizeof(s1)) == 0)
#define CPYN(s1, s2)	strncpy(s1, s2, sizeof(s1))

#define SECSINDAY	86400L
#define SECS(tics)	((double) tics)/HZ
#define MINS(secs)	((double) secs)/60
#define MINT(tics)	((double) tics)/(60*HZ)

#define KCORE(kcore)	((double)(kcore))
#define	MEM(acmem)	((long) expand_int ((acmem)))

#define LOGGED_ON       0
#define LOGGED_OFF      1
