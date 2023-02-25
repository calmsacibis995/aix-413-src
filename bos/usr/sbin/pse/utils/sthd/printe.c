static char sccsid[] = "@(#)69	1.1  src/bos/usr/sbin/pse/utils/sthd/printe.c, cmdpse, bos411, 9428A410j 5/7/91 13:52:52";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/** Copyright (c) 1990  Mentat Inc.
 ** printe.c 2.2, last change 4/9/91
 **/


#include <stdio.h>
#include <pse/common.h>
#ifdef	USE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

extern	int	noshare	errno;

#ifdef	USE_STDARG
printe (boolean print_errno, ...    )
#else
printe (print_errno, va_alist)
	boolean	print_errno;
	va_dcl
#endif
{
	va_list	ap;
	char	* fmt;

#ifdef	USE_STDARG
	va_start(ap, print_errno);
#else
	va_start(ap);
#endif
	printf("*ERROR* ");
	if (fmt = va_arg(ap, char *))
		vprintf(fmt, ap);
	va_end(ap);
	if (print_errno)
		printf(", %s\n", errmsg(0));
	else
		printf("\n");
	errno = 0;
	return false;
}

#ifdef	USE_STDARG
printok (char * fmt, ...    )
#else
printok (fmt, va_alist)
	char	* fmt;
	va_dcl
#endif
{
	va_list	ap;

#ifdef	USE_STDARG
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	return true;
}
