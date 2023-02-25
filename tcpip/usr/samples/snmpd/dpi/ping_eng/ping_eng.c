static char sccsid[] = "@(#)35	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/ping_eng.c, snmp, tcpip411, GOLD410 9/13/93 15:09:11";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: main, data_available
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
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/ping_eng.c
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
 * This file contains the main function as well as data_available.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>

#include "io_lib.h"
#include "init.h"

int agent_fd;

#define FDT_AGENT_QUERY 	1
#define FDT_PING_RESPONSE	2

main(argc, argv)
int argc;
char *argv[];
{
	int snmp_port;
	int rc;

	/* Handle arguments and usage */
	if (argc != 3) {
		fprintf(stderr,"usage:  %s agent_hostname community\n",argv[0]);
		exit(1);
	}

	/*
	 * Must get the port that the SMUX<->DPI agent is listening.  This
	 * is done by sending a get-request packet to the snmp daemon and 
	 * getting the port number from its response.  This port will be 
	 * used to register the new sub-agent port.
	 */
	snmp_port = query_DPI_port(argv[1],argv[2]);

	if (snmp_port == -1) {
		fprintf(stderr,"can't contact agent at %s(%s)\n",argv[1],
			argv[2]);
		exit(1);
	}

	/*
	 * Create a port to receive ping over.
	 */
	rc = mkPINGport();
	if (rc < 0) {
		fprintf(stderr,"can't create RAW socket\n");
		exit(1);
	}
	add_fd(rc, FDT_PING_RESPONSE);

	/*
	 * This is a loop that will connect, register, and await queries.
         */
	do {
		/* 
		 * Connect to the SMUX<->DPI agent.  If failure, exit.
		 * This creates a new file descriptor to talk to the 
		 * SMUX<->DPI agent so that the registration port is not
		 * tied up handling requests.
		 */
		agent_fd = connect_to_agent(argv[1],snmp_port);
		if (agent_fd == -1) {
			fprintf(stderr,"can't connect to agent at %s(%d)\n",
				argv[0], snmp_port);
			exit(1);
		}
		/*
		 * Try to register oid trees to manage.  On failure, exit.
		 * The new specific port between this sub-agent and the 
		 * SMUX<->DPI agent.
		 */
		rc = register_oids(agent_fd);
		if (rc != 0) {
			fprintf(stderr,"unsuccessful register attempt\n");
			exit(1);
		}
		add_fd(agent_fd, FDT_AGENT_QUERY);
		/* 
		 * Wait for queries to process
		 */
		rc = await_queries();
	} while (rc == -1);
	exit(2);
}

/*
 * FUNCTION:	data_available
 *
 * INPUT:
 *	fd	the file descriptor of the incoming data
 *	type	the type of file descriptor
 *		FDT_AGENT_QUERY - messages from the SMUX<->DPI agent
 *		FDT_PING_RESPONSE - returning ping replies
 *			
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	 0	successful
 *	-1	connection to agent was not active
 *	-2	invalid type passed in
 */
int data_available(fd, type)
int fd;
int type;
{
	unsigned char buffer[1024];
	int len, fromlen, rc;
	struct sockaddr_in from;

	switch (type) {
	case FDT_AGENT_QUERY:
		/*
		 * Read packet from fd and process it if successful.
		 */
		rc = read_packet(fd, buffer);
		if (rc == -1) {	/* EOF */
			fprintf(stderr,"lost connection with AGENT\n");
			del_from_mask(fd);
			return(-1);
		}
                fprintf(stderr, "SNMP REQ RECEIVED\7\n");
		process_request(fd, buffer);
		break;
	case FDT_PING_RESPONSE:
		/*
		 * Read packet from fd and mark ping as received
		 */
		fromlen = sizeof(from);
		len = sizeof(buffer);
		rc = recvfrom(fd, buffer, len, 0, &from, &fromlen);
		if (rc < 0) break;
		parsePING(buffer, rc, &from);
		break;
	default:
		fprintf(stderr,"unknown fd type!\n");
		return(-2);
	}
	return(0);
}
