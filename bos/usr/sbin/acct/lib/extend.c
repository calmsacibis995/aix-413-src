static char sccsid[] = "@(#)71	1.2.1.1  src/bos/usr/sbin/acct/lib/extend.c, cmdacct, bos411, 9428A410j 4/24/92 13:19:14";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: extend
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

/* Extend table pointed to by tb.  Returns NULL if unsuccessful. */

#include <IN/standard.h>
#include <sys/types.h>
#include "table.h"
char *realloc(), *malloc();

char *
extend(tb)
register struct table *tb;
{       register unsigned sz = (unsigned)tb->tb_nel * tb->tb_elsize;
	register char *space = *tb->tb_base;

	/* Check for overflow */
	if ((unsigned) tb->tb_nel != sz/tb->tb_elsize)
		space = NULL;
	else if (space) {
		space = realloc(space,sz);
	} else
		space = malloc(sz);
	return(*tb->tb_base = space);
}

