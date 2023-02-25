static char sccsid[] = "@(#)69	1.3.1.1  src/bos/usr/sbin/acct/lib/expand.c, cmdacct, bos411, 9428A410j 2/17/92 16:51:51";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: expand, expand_int
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/acct.h>

#define AHZ     64              /* 1/AHZ is the granularity of the data
				 * encoded in the various comp_t fields.
				 * This is not equal to hz....
				 */

float
expand(t)
	unsigned t;
{
	register long nt;
	float e;

	nt = t&017777;
	t >>= 13;
	while (t!=0) {
		t--;
		nt <<= 3;
	}
	e = (float) (nt/AHZ) + (((float)(nt%AHZ))/AHZ);
	return(e);
}

long
expand_int(t)
	unsigned t;
{
	register long nt;

	nt = t&017777;
	t >>= 13;
	while (t!=0) {
		t--;
		nt <<= 3;
	}
	return(nt);
}

