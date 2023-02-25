static char sccsid[] = "@(#)92	1.3  src/bos/usr/sbin/acct/lib/uidtonam.c, cmdacct, bos411, 9428A410j 6/15/90 20:04:20";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: uidtonam
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
 * convert uid to login name; interface to getpwuid that keeps up to USIZE1
 * names to avoid unnecessary accesses to passwd file
 * returns ptr to NSZ-byte name (not necessarily null-terminated)
 * returns ptr to "?" if cannot convert
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <pwd.h>

#define USIZE1	50
static	usize1;
static struct ulist {
	char	uname[NSZ];
	uid_t	uuid;
} ul[USIZE1];

char *
uidtonam(uid)
uid_t	uid;
{
	register struct ulist *up;
	register struct passwd *pp;

	for (up = ul; up < &ul[usize1]; up++)
		if (uid == up->uuid)
			return(up->uname);
	setpwent();
	if ((pp = getpwuid(uid)) == NULL)
		return("?");
	else {
		if (usize1 < USIZE1) {
			up->uuid = uid;
			CPYN(up->uname, pp->pw_name);
			usize1++;
		}
		return(pp->pw_name);
	}
}
