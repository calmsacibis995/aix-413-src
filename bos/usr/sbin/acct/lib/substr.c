static char sccsid[] = "@(#)86	1.2  src/bos/usr/sbin/acct/lib/substr.c, cmdacct, bos411, 9428A410j 6/15/90 20:04:09";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: substr
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
	Place the `len' length substring of `as' starting at `as[origin]'
	in `aresult'.
	Return `aresult'.
  Note: The copying of as to aresult stops if either the
	specified number (len) characters have been copied,
	or if the end of as is found.
	A negative len generally guarantees that everything gets copied.
*/

char *substr(as, aresult, origin, len)
char *as, *aresult;
int origin;
register unsigned len;
{
	register char *s, *result;

	s = as + origin;
	result = aresult;
	++len;
	while (--len && (*result++ = *s++)) ;
	if (len == 0)
		*result = 0;
	return(aresult);
}
