static char sccsid[] = "@(#)55	1.2  src/tcpip/usr/lib/libsnmp/lookup_addr.c, snmp, tcpip411, GOLD410 3/2/93 18:23:32";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: lookup_addr()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/lookup_addr.c
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

char *
lookup_addr (addr)
unsigned long *addr;
{
    register struct hostent *hp;
    static char hostname[80];

    hp = gethostbyaddr (addr, sizeof (struct in_addr), AF_INET);
    if (hp != 0) {
	strcpy (hostname, hp -> h_name);
    } 
    else 
	strcpy (hostname, inet_ntoa (*addr));
    return (hostname);
}
