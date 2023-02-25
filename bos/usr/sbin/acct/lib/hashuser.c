static char sccsid[] = "@(#)73	1.3  src/bos/usr/sbin/acct/lib/hashuser.c, cmdacct, bos411, 9428A410j 6/15/90 20:03:45";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: hashuser
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
#include "acctdef.h"

unsigned
hashuser(uid, name)
uid_t uid;
register char *name;
{       register unsigned uh;
	register int i = NSZ;

	for (uh = *name++; --i > 0; ) {
		uh *= 7;
		if (*name == 0) break;
		uh += *name++;
	}
	return((uh+uid) % UHASH);
}

