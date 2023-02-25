static char sccsid[] = "@(#)77	1.3  src/bos/usr/sbin/acct/lib/namtouid.c, cmdacct, bos411, 9428A410j 6/15/90 20:03:53";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: namtouid
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
 *	namtouid converts login names to uids
 *	maintains ulist for speed only
 */
#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <pwd.h>
static	usize;
static	struct ulist {
	char	uname[NSZ];
	uid_t	uuid;
} ul[A_USIZE];
char	ntmp[NSZ+1];

uid_t
namtouid(name)
char	name[NSZ];
{
	register struct ulist *up;
	register uid_t tuid;

	register struct passwd *pp;

	for (up = ul; up < &ul[usize]; up++)
		if (strncmp(name, up->uname, NSZ) == 0)
			return(up->uuid);
	strncpy(ntmp, name, NSZ);
	setpwent();
	if ((pp = getpwnam(ntmp)) == NULL)
		tuid = -1;
	else {
		tuid = pp->pw_uid;
		if (usize < A_USIZE) {
			CPYN(up->uname, name);
			up->uuid = tuid;
			usize++;
		}
	}
	return(tuid);
}
