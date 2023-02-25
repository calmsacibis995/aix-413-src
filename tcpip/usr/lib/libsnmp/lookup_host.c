static char sccsid[] = "@(#)56	1.2  src/tcpip/usr/lib/libsnmp/lookup_host.c, snmp, tcpip411, GOLD410 3/2/93 18:23:50";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: lookup_host()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/lookup_host.c
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

/* look up a host name (either name or in "dot" notation) and
	return the address */

unsigned long 
lookup_host(hostname)
char *hostname;
{
    register unsigned long ret_addr;

    if ((*hostname >= '0') && (*hostname <= '9'))
	    ret_addr = inet_addr(hostname);
    else {
	struct hostent *host;
	struct in_addr *addr;

	if ((host = gethostbyname(hostname)) == 0)
	    return 0;
	addr = (struct in_addr *) (host->h_addr_list[0]);
	ret_addr = addr->s_addr;
    }
    return(ret_addr);
}

