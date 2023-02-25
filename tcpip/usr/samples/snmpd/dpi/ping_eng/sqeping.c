static char sccsid[] = "@(#)37	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/sqeping.c, snmp, tcpip411, GOLD410 9/13/93 15:09:34";
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
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/sqeping.c
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
 * This file contains the routines to do the ping functionality.
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>


#define PACKET_SIZE 64

static int      ping_port;
int             ping_pid;
static struct sockaddr_in ping_sock;

static int      num_packets = 0;

static int      send_PING_packet();
static          in_cksum();

/* PING routines */

/*
 * FUNCTION:	mkPINGport
 *
 * INPUT:
 *	none
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1	unsuccessful
 *	port 	successful, port number of new ICMP socket
 *
 */
int             mkPINGport()
{
    /* 1 == ICMP */
    if              ((ping_port = socket(AF_INET, SOCK_RAW, 1)) < 0) {
	                return (-1);
    }
    ping_pid = getpid() & 0xFFFF;	/* get pid into 16 bits */
    bzero((char *) &ping_sock, sizeof(ping_sock));
    ping_sock.sin_family = AF_INET;
    return (ping_port);
}


/*
 * FUNCTION:	parsePING
 *
 * INPUT:
 *	buf	data buf read from ping fd
 *	len	the len of the buffer received
 *
 * OUTPUT:
 *	from	the address of the sender (struct sockaddr_in *)
 *
 * RETURN:
 *	-1	unsuccessful
 *	 0 	successful
 *
 */
int             parsePING(buf, len, from)
char           *buf;
int             len;
struct sockaddr_in *from;
{
    struct ip      *ip;
    register struct icmp *icp;
    int             hlen;
    struct timeval  tv;
    struct timeval *tvp;
    unsigned long   delta;
    int             id;

    ip = (struct ip *) buf;
#ifndef AIXPS2
    hlen = ip->ip_hl * 4;	/* 4 bytes per word */
#else
    hlen = (ip->ip_hl_v & IP_HL_MASK) * 4;	/* 4 bytes per word */
#endif
    len -= hlen;/* bytes of data passed */
    icp = (struct icmp *) (buf + hlen);
    if (icp->icmp_type != ICMP_ECHOREPLY) {
	/* this is not an error... different kinds can arrive */
	return (0);
    }
    if (icp->icmp_id != ping_pid) {
	return (0);	/* could have been another ping */
    }
    tvp = (struct timeval *) (icp->icmp_data);
    gettimeofday(&tv, 0);
    delta = (tv.tv_sec - tvp->tv_sec) * 1000 +
      (tv.tv_usec - tvp->tv_usec) / 1000;
    id = icp->icmp_seq;
    retPINGdata(id, delta, from->sin_addr.s_addr);
    return (0);
}

/* send an ICMP echo request to the designated host */
/*
 * FUNCTION:	mkICMPreq
 *
 * INPUT:
 *	dest_addr	the address to send ping to
 *	pid		the identifier for the ping packet
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1	unsuccessful
 *	 0 	successful
 *
 */
int             mkICMPreq(dest_addr, pid)
unsigned long   dest_addr;
int             pid;
{
    static unsigned char bfr[PACKET_SIZE];
    register struct icmp *icp;
    register int    i;
    register unsigned char *datap;
    int             len, rc;

    icp = (struct icmp *) bfr;
    /* type(1) + code(1) + cksum(2) + seq(2) + id(2) == 8 */
    datap = bfr + 8 + sizeof(struct timeval);


    icp->icmp_type = ICMP_ECHO;
    icp->icmp_code = 0;
    icp->icmp_cksum = 0;
    /* increment packet count, wrap if needed */
    if ((++num_packets & 0xffff) == 0) {
	num_packets = 1;
    }
    icp->icmp_seq = pid;
    icp->icmp_id = ping_pid;

    len = PACKET_SIZE;

    gettimeofday(bfr + 8, NULL);	/* pop in time */
    /* fill in rest of packet with nonsense data */
    for (i = 8 + sizeof(struct timeval); i < len; i++);
    *datap++ = i;

    /* Compute ICMP checksum here */
    icp->icmp_cksum = in_cksum(icp, len);
    rc = send_PING_packet(dest_addr, bfr, len);
    return (rc);
}

/*
 * FUNCTION:	send_PING_packet
 *
 * INPUT:
 *	dest_addr	the address to send ping to
 *	packet		the data to send
 *	len		the length of the data to send
 *
 * OUTPUT:
 *	none
 *
 * RETURN:
 *	-1	unsuccessful
 *	 0 	successful
 *
 */
static int      send_PING_packet(dest_addr, packet, len)
unsigned long   dest_addr;
unsigned char  *packet;
int             len;
{
    int             rc;

    ping_sock.sin_addr.s_addr = dest_addr;
    rc = sendto(ping_port, packet, len, 0, &ping_sock,
		sizeof(ping_sock));
    if (rc < 0) {
	return (-1);
    } else
	return (0);
}


/*
 * From ping.c.... I N _ C K S U M 
 *
 * Checksum routine for Internet Protocol family headers (C Version) 
 *
 */
static          in_cksum(addr, len)
u_short        *addr;
int             len;
{
    register int    nleft = len;
    register u_short *w = addr;
    register u_short answer;
    register int    sum = 0;

    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we
     * add sequential 16 bit words to it, and at the end, fold back
     * all the carry bits from the top 16 bits into the lower 16
     * bits. 
     */
    while (nleft > 1) {
	sum += *w++;
	nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1)
	sum += *(u_char *) w;

    /*
     * add back carry outs from top 16 bits to low 16 bits 
     */
    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);	/* add carry */
    answer = ~sum;	/* truncate to 16 bits */
    return (answer);
}

