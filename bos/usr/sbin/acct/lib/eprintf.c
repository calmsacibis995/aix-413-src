static char sccsid[] = "@(#)66  1.3  src/bos/usr/sbin/acct/lib/eprintf.c, cmdacct, bos411, 9428A410j 5/7/91 10:42:05";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: eprintf
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
 * flush stdout and then print arguments on stderr
 */

#include <stdio.h>
#include <varargs.h>
#include <IN/standard.h>

eprintf(va_alist) va_dcl
{
	va_list arglist;
	register char *fmt;
	register int ret;

	va_start(arglist);
	fmt = va_arg(arglist, char *);
	fflush(stdout);
	ret = vfprintf(stderr, fmt, arglist);
	fflush(stderr);
	va_end(arglist);
	return ret;
}
