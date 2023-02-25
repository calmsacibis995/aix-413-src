static char sccsid[] = "@(#)49	1.2  src/tcpip/usr/lib/libsnmp/advise.c, snmp, tcpip411, GOLD410 3/2/93 18:21:33";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: advise()
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/advise.c
 */

#include <stdio.h>
#include <varargs.h>

/* dummy advise routine.  Just prints to stderr */
void
advise (va_alist)
va_dcl
{
    int		event;
    char	*format;
    va_list	ap;

    va_start (ap);

    /* advise is always called like: 
     * advise (EVENT, printf_format, args)
     */
    event = va_arg (ap, int);
    format = va_arg (ap, char *);

#ifndef VSPRINTF
    _doprnt (format, ap, stderr);
#else
    vfprintf (stderr, format, ap);
#endif

    fputc ('\n', stderr);	/* logging advise calls don't have new-lines */

    va_end (ap);
}
