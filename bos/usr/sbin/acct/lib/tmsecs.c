static char sccsid[] = "@(#)90	1.2  src/bos/usr/sbin/acct/lib/tmsecs.c, cmdacct, bos411, 9428A410j 6/15/90 20:04:16";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: tmsecs
 *
 * ORIGINS: 3, 9, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	tmsecs returns number of seconds from t1 to t2,
 *	times expressed in localtime format.
 *	assumed that t1 <= t2, and are in same day.
 */

#include <time.h>
long
tmsecs(t1, t2)
register struct tm *t1, *t2;
{
	return((t2->tm_sec - t1->tm_sec) +
		60*(t2->tm_min - t1->tm_min) +
		3600L*(t2->tm_hour - t1->tm_hour));
}
