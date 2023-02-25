static char sccsid[] = "@(#)48	1.2  src/tcpip/usr/lib/libsnmp/adios.c, snmp, tcpip411, GOLD410 3/2/93 18:21:19";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: adios()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/adios.c
 */

#include <stdio.h>
#include <varargs.h>

/* dummy adios routine.  Just prints to stderr, then exits */
void
adios (va_alist)
va_dcl
{
    char	*format;
    va_list	ap;

    va_start (ap);

    /* adios is always called like: 
     * adios (printf_format, args)
     */
    format = va_arg (ap, char *);

#ifndef VSPRINTF
    _doprnt (format, ap, stderr);
#else
    vfprintf (stderr, format, ap);
#endif

    fputc ('\n', stderr);	/* logging advise calls don't have new-lines */

    va_end (ap);

    exit (1);
}
