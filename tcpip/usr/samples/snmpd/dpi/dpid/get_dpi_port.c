static char sccsid[] = "@(#)13	1.1  src/tcpip/usr/samples/snmpd/dpi/dpid/get_dpi_port.c, snmp, tcpip411, GOLD410 9/13/93 15:05:11";
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
 * FILE:        /usr/samples/snmpd/dpi/dpid/get_dpi_port.c
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
 * This file contains query_PID_port, _query_DPI_port, and extract_DPI_port.
 * These routines are defined and described in RFC 1228.  These are also part
 * of the DPI library.
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

static unsigned char asn1_hdr[] = {0x30};

/* insert length of remaining packet, not including this */
static unsigned char version[] = {0x02, 0x01, 0x00, 0x04};

/* integer, len=1, value=0, string */
/* insert community name length and community name */
static unsigned char request[] = {
	0xa0, 0x1b,		/* get request, len=0x1b */
	0x02, 0x01, 0x01,	/* integer, len=1,request_id = 1 */
	0x02, 0x01, 0x00,	/* integer, len=1, error_status = 0 */
	0x02, 0x01, 0x00,	/* integer, len=1, error_index = 0 */
	0x30, 0x10,		/* varbind list, len=0x10 */
	0x30, 0x0e,		/* varbind , len=0x0e */
	0x06, 0x0a,		/* object ID, len=0x0a */
	0x2b, 0x06, 0x01, 0x04, 0x01, 0x02, 0x02, 0x01, 0x01, 0x00,
	0x05, 0x00		/* value, len = 0 */
};

static          extract_DPI_port();

query_DPI_port(hostname, community_name)
char           *hostname;
char           *community_name;
{
    int             community_len;
    int             rc;

    community_len = strlen(community_name);

    rc = _query_DPI_port(hostname, community_name, community_len);
    return (rc);
}

/* use if community_name has embedded nulls */

_query_DPI_port(hostname, community_name, community_len)
char           *hostname;
char           *community_name;
int             community_len;
{
    unsigned char   packet[1024];
    int             packet_len;
    int             remaining_len;
    int             fd, rc, sock_len;
    struct sockaddr_in sock, dest_sock;
    struct timeval  timeout;
    unsigned long   host_addr, read_mask;
    int             tries;

    host_addr = lookup_host(hostname);
    packet_len = 0;
    bcopy(asn1_hdr, packet, sizeof(asn1_hdr));
    packet_len += sizeof(asn1_hdr);

    remaining_len = sizeof(version) + 1 +
      community_len + sizeof(request);

    packet[packet_len++] = remaining_len & 0xff;
    bcopy(version, packet + packet_len, sizeof(version));
    packet_len += sizeof(version);
    packet[packet_len++] = community_len & 0xff;
    bcopy(community_name, packet + packet_len, community_len);
    packet_len += community_len;
    bcopy(request, packet + packet_len, sizeof(request));
    packet_len += sizeof(request);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
	return (-1);
    }
    bzero(&sock, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_port = 0;
    sock.sin_addr.s_addr = 0;
    rc = bind(fd, &sock, sizeof(sock));
    if (rc < 0)
	return (-1);
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    bzero(&dest_sock, sizeof(dest_sock));
    dest_sock.sin_family = AF_INET;
    dest_sock.sin_port = htons(161);
    dest_sock.sin_addr.s_addr = host_addr;

    tries = 0;
    while (++tries < 4) {
	rc = sendto(fd, packet, packet_len, 0, &dest_sock,
		    sizeof(dest_sock));
	read_mask = 1 << fd;
	rc = select(read_mask + 1, &read_mask, 0, 0, &timeout);
	if (rc <= 0)
	    continue;
	sock_len = sizeof(dest_sock);
	packet_len = recvfrom(fd, packet, sizeof(packet), 0,
			      &dest_sock, &sock_len);
	if (packet_len <= 0) {
	    return (-1);
	}
	rc = extract_DPI_port(packet, packet_len, community_len);
	return (rc);
    }
    return (-1);
}

static          extract_DPI_port(packet, len, community_len)
unsigned char   packet[];
int             len;
int		community_len;
{
    int             offset;
    int             port;


    /* should do error checking (like for noSuchName) */
    if (38 + community_len != len) {
	/* not what we expected... */
	return (-1);
    }
    offset = len - 2;
    port = (packet[offset] << 8) + packet[offset + 1];
    return (port);
}
