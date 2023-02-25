static char sccsid[] = "@(#)22	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/init.c, snmp, tcpip411, GOLD410 9/13/93 15:08:17";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/init.c
 */

/*
 * INTERNATIONAL BUSINESS MACHINES CORP. PROVIDES THIS SOURCE CODE
 * EXAMPLE, ASSOCIATED LIBRARIES AND FILES "AS IS," WITHOUT WARRANTY 
 * OF ANY KIND EITHER EXPRESSED OR IMPLIED INCLUDING BUT NOT LIMITED 
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  The entire risk as to the quality and performance 
 * of this example is with you.  Should any part of this source code 
 * example prove defective you (and not IBM or an IBM authorized 
 * dealer) assume the entire cost of all necessary servicing, repair, 
 * or correction.
 */

/*
 * This file contains the function, register_oids.  This function is used
 * to register oid trees with the SMUX-DPI adent that has attached itself to the
 * SNMP daemon. 
 */

#include <stdio.h>

#include "io_lib.h"
#include "init.h"

#include "snmp_dpi.h"

#define PACKET_LEN(packet) (*(packet) * 256 + *((packet) + 1) + 2)

/*
 * FUNCTION: register_oids
 *
 * INPUTS: 
 *	agent_fd	The file descriptor of the SMUX<->DPI agent.
 *
 * OUTPUT:
 *	none
 *
 * RETURNS:
 *	 0 if successful
 *	-1 if failure (no address for hostname, cannot talk to SMUX<->DPI agent)
 * 
 * GLOBALS:
 *	hostname	A string containing the hostname on which to find the
 *                      SMUX<->DPI agent.
 */
int register_oids(agent_fd)
int agent_fd;
{
	char hostname[80];
	char object_id[512];
	unsigned long addr;
	int i, len, rc;
	unsigned char *packet;
	
	gethostname(hostname,sizeof(hostname));
	addr = lookup_host(hostname);
	if (addr == 0) {
		fprintf(stderr,"can't get address for %s\n",hostname);
		return(-1);
	}
	/* register the instances of the RTT variables */
	packet = mkDPIregister("1.3.6.1.4.1.2.2.1.2.");
	len = PACKET_LEN(packet);
	rc = write(agent_fd, packet, len);
	if (rc <= 0) {
		perror("register_oids:  write");
		return (-1);
	}
	return (0);
}
