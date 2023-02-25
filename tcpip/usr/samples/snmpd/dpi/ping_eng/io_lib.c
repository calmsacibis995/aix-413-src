static char sccsid[] = "@(#)24	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/io_lib.c, snmp, tcpip411, GOLD410 9/13/93 15:08:37";
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
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/io_lib.c
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
 * This file contains procedures that handle reads and file descriptor details
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include "io_lib.h"

unsigned long select_mask;
static char     fd_tbl[MAX_FDS];

/* for debug
 * FUNCTION:	hex_dump
 *
 * INPUT:
 *	bfr	The data to dump
 *	len	The length of the data to dump
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	none
 *
 * NOTE:
 *	none
 *
 */
void hex_dump(bfr, len)
unsigned char *bfr;
int len;
{
	register int i;

	printf("\n dumping packet of %d bytes:\n ",len);
	for(i=0;i<len;i++) {
		printf("%2.2x%s",*bfr,((i & 15) == 15) ? "\n " : " ");
		bfr++;
	}
	if ((i & 15) != 15) printf("\n");
}

/*
 * FUNCTION:	must_read
 *
 * INPUT:
 *	fd		file descriptor to read
 *	required_len	The amount of the data to read
 *
 * OUTPUT:
 *	buffer		The data read from fd
 *
 * RETURN:
 *	<0		failure
 *	required_len	successful
 *
 */
static int must_read(fd, buffer, required_len)
int fd;
unsigned char *buffer;
int required_len;
{
	struct timeval timeout;
	register int bytes;
	register int bytes_to_read;
	register int bytes_read;
	unsigned long read_mask;

	timeout.tv_usec = 0;
	timeout.tv_sec = 3;
	bytes_to_read = required_len;
	bytes_read = 0;

	while (bytes_to_read > 0) {	/* while more to get */
		read_mask = 1 << fd;	/* check for data */
		bytes = select(fd+1,&read_mask,0,0,&timeout);
		if (bytes == 1) {	/* select returned OK */
			bytes = read(fd,buffer + bytes_read,bytes_to_read);
		}
		if (bytes <= 0) return(bytes);
		else {
			bytes_read += bytes;
			bytes_to_read -= bytes;
		}
	}
	return (required_len);
}

/*
 * FUNCTION:	read_packet
 *
 * INPUT:
 *	fd		file descriptor to read
 *
 * OUTPUT:
 *	buf		The packet read from fd
 *
 * RETURN:
 *	 0	successful
 *	-1	EOF
 *	-2	can't read data
 *
 */
int read_packet(fd, buf)
int fd;
unsigned char buf[];
{
	int len, bytes_to_read;

	len = read(fd,buf,2);
	if (len == 0) return(-1);	/* EOF */
	if (len < 0) {
		perror("get_packet::read");
		fprintf(stderr,"can't read response\n");
		close(fd);
		return(-2);
	}
	bytes_to_read = (buf[0] << 8) + buf[1];
	len = must_read(fd,&buf[2],bytes_to_read);
	if (len <= 0) {
		fprintf(stderr,"can't read remainder of response\n");
		close(fd);
		return(-2);
	}
	return(0);
}

/*
 * FUNCTION:	lookup_host
 *
 * INPUT:
 *	hostname	The name of the host to look up
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	address		The address of the host
 *
 */
unsigned long lookup_host(hostname)
char *hostname;
{
	register unsigned long ret_addr;

	if ((*hostname >= '0') && (*hostname <= '9'))
		ret_addr = inet_addr(hostname);
	else {
		struct hostent *host;
		struct in_addr *addr;

		host = gethostbyname(hostname);
		if (host == NULL) return(0);
		addr = (struct in_addr *) (host->h_addr_list[0]);
		ret_addr = addr->s_addr;
	}
	return(ret_addr);
}

/*
 * FUNCTION:	connect_to_agent
 *
 * INPUT:
 *	dpi_host	The host that the SMUX<->DPI agent is on
 *	dpi_port	The port to talk to the SMUX<->DPI agent 
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1		failure
 *	port		The new port for the sub-agent to use to talk to the
 *			SMUX<->DPI agent.
 */
int connect_to_agent(dpi_host, dpi_port)
char *dpi_host;
int dpi_port;
{
	struct	sockaddr_in sin;	/* socket address */
	int dpi_fd;
	register int ret_code;
	unsigned short port;

	port = dpi_port;
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET; 
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = lookup_host(dpi_host);
	if (sin.sin_addr.s_addr == 0) {
		fprintf(stderr,"unknown host:  %s\n",dpi_host);
		return(-1);
	}
	dpi_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (dpi_fd < 0) {
		perror("connect_to_server:  socket");
		return(-1);
	}
	ret_code = connect(dpi_fd,&sin,sizeof(sin));
	if (ret_code != 0) {
		perror("connect_to_server:  connect");
		close(dpi_fd);
		return(-1);
	}
	return(dpi_fd);
}

/*
 * FUNCTION:	add_to_mask
 *
 * INPUT:
 *	fd	The file descriptor to add to the global select mask
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	none
 */
static void     add_to_mask(fd)
int             fd;
{
    select_mask |= (1 << fd);
}

/*
 * FUNCTION:	del_from_mask
 *
 * INPUT:
 *	fd	file descriptor to remove from the select mask
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	none
 */
void            del_from_mask(fd)
int fd;
{
    select_mask &= ~(1 << fd);
}

/*
 * FUNCTION:	add_fd
 *
 * INPUT:
 *	fd	A file descriptor to add to the of monitored file descriptors
 *	type	The type of the file descriptor
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	none
 */
void            add_fd(fd, type)
int fd;
int type;
{
    fd_tbl[fd] = type;
    add_to_mask(fd);
}

/*
 * FUNCTION:	await_queries
 *
 * INPUT:
 *	none
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1		failure
 *	  		If successful, should never return.
 *
 * NOTE:
 *	An infinite loop that waits for packets from the SMUX<->DPI agent and
 *	ping responses.  It uses select to do timing for the ping packets.
 */
int await_queries()
{
    struct timeval  timeout;
    struct timeval *t_ptr;
    unsigned long   read_mask;
    register int    fd;
    register int    nfds;
    int rc;

    timeout.tv_usec = 0;

    while (1) {	/* do this forever */
	/*
	 * process event queues 
	 */
	timeout.tv_sec = next_timeout();
	/*
	 * if nothing scheduled, block forever 
	 */
	if (timeout.tv_sec == -1)
	    t_ptr = 0;
	else
	    t_ptr = &timeout;
	read_mask = select_mask;
	nfds = select(32, &read_mask, 0, 0, t_ptr);
	if (nfds <= 0)
	    continue;	/* next pass */
	/*
	 * there is something to process.. 
	 */
	fd = 0;
	while (read_mask != 0) {
	    if ((read_mask & 1) != 0) {
		/* allow user to process it */
		rc = data_available(fd,fd_tbl[fd]);
		if (rc < 0) return(rc);
	    }
	    fd++;
	    read_mask >>= 1;
	}
    }
    /* NOTREACHED */
}
