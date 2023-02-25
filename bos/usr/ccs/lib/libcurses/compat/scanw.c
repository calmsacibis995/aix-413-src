static char sccsid[] = "@(#)89  1.1  src/bos/usr/ccs/lib/libcurses/compat/scanw.c, libcurses, bos411, 9428A410j 9/2/93 13:51:05";
/*
 * COMPONENT_NAME: (LIBCURSES) Curses Library
 *
 * FUNCTIONS: scanw
 *
 * ORIGINS: 3, 10, 27
 *
 *   This module contains IBM CONFIDENTIAL code. -- (IBM
 *   Confidential Restricted when combined with the aggregated
 *   modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"
# include	<varargs.h>

/*
 * NAME:        scanw
 *
 * FUNCTION:
 *
 *      This routine implements a scanf on the standard screen.
 */

scanw(fmt, va_alist)
char	*fmt;
va_dcl
{
	va_list	ap;

	va_start(ap);
	return __sscans(stdscr, fmt, ap);
}
