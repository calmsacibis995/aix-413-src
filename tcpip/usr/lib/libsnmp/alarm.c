static char sccsid[] = "@(#)50	1.2  src/tcpip/usr/lib/libsnmp/alarm.c, snmp, tcpip411, GOLD410 3/2/93 18:21:49";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: alarmtr()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/alarm.c
 */

#include <setjmp.h>

/* GLOBAL */
jmp_buf Sjbuf;

int
alarmtr (s)
int s;
{
    longjmp (Sjbuf, 1);
    return(0);
}
