static char sccsid[] = "@(#)60	1.2  src/bos/usr/sbin/acct/lib/cat.c, cmdacct, bos411, 9428A410j 6/15/90 20:03:24";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: cat
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

/*      Concatenate strings, returns destination. */
/*      cat(destination,source1,source2,...,sourcen,0); */

char *cat(dest,source)
char *dest, *source;
{
	register char *d, *s, **sp;

	d = dest;
	for (sp = &source; s = *sp; sp++) {
		while (*d++ = *s++) ;
		d--;
	}
	return(dest);
}
