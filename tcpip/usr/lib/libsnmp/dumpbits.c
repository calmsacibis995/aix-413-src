static char sccsid[] = "@(#)52	1.2  src/tcpip/usr/lib/libsnmp/dumpbits.c, snmp, tcpip411, GOLD410 3/2/93 18:22:48";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: dump_bfr()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/dumpbits.c
 */

#include <stdio.h>

/* dump bytes in a buffer (to stderr) */
void
dump_bfr (buf, len)
char *buf;
int len;
{
    register int i;

    for (i = 0; i < len; i++)
	fprintf (stderr, "%2.2x%c", *buf++, ((i&15) == 15) ? '\n' : ' ');
    fprintf (stderr, "\n");
}
