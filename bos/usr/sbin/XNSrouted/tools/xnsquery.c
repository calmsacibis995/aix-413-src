static char sccsid[] = "@(#)17	1.3  src/bos/usr/sbin/XNSrouted/tools/xnsquery.c, cmdnet, bos411, 9428A410j 9/18/93 15:33:33";
/*
 * COMPONENT_NAME: (CMDNET) Network commands. 
 *
 * FUNCTIONS: 
 *
 *
 * ORIGINS: 26 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * This file includes significant work done at Cornell University by
 * Bill Nesheim.  That work included by permission.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *  Copyright (c) 1983, 1986 The Regents of the University of California.
 *  All rights reserved.
 *
 *		 "query.c	5.6 (Berkeley) 9/19/88";
 *
 */


#include <sys/param.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netns/ns.h>
#include <netns/idp.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include "../protocol.h"
#define IDPPORT_RIF 1

#define	WTIME	5		/* Time to wait for responses */

int	s;
int	timedout, timeout();
char	packet[MAXPACKETSIZE];
extern int errno;
struct sockaddr_ns	myaddr ;
char *ns_ntoa();
struct ns_addr ns_addr();
main(argc, argv)
int argc;
char *argv[];
{
	int cc, count, bits;
	struct sockaddr from;
	int fromlen = sizeof(from);
	struct timeval notime;
	
	if (argc < 2) {
		printf("usage: query hosts...\n");
		exit(1);
	}
	s = getsocket(SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		exit(2);
	}

	argv++, argc--;
	query(argv,argc);

	/*
	 * Listen for returning packets;
	 * may be more than one packet per host.
	 */
	bits = 1 << s;
	bzero(&notime, sizeof(notime));
	signal(SIGALRM, timeout);
	alarm(WTIME);
	while (!timedout || 
	    select(20, &bits, 0, 0, &notime) > 0) {
		struct nspacket {
			struct idp hdr;
			char	data[512];
		} response;
		cc = recvfrom(s, &response, sizeof (response), 0,
		  &from, &fromlen);
		if (cc <= 0) {
			if (cc < 0) {
				if (errno == EINTR)
					continue;
				perror("recvfrom");
				(void) close(s);
				exit(1);
			}
			continue;
		}
		rip_input(&from, response.data, cc);
		count--;
	}
}
static struct sockaddr_ns router;
static struct ns_addr zero_addr;
static short allones[] = {-1, -1, -1};

query(argv,argc)
char **argv;
{
	register struct rip *msg = (struct rip *)packet;
	char *host = *argv;
	struct ns_addr specific;

	argv++; argc--;
	bzero ((char *) &router, sizeof (router));
	router.sns_family = AF_NS;
	router.sns_addr = ns_addr(host);
	router.sns_addr.x_port = htons(IDPPORT_RIF);
	if (ns_hosteq(zero_addr, router.sns_addr)) {
		router.sns_addr.x_host = *(union ns_host *) allones;
	}
	msg->rip_cmd = htons(RIPCMD_REQUEST);
	msg->rip_nets[0].rip_dst = *(union ns_net *) allones;
	msg->rip_nets[0].rip_metric = htons(HOPCNT_INFINITY);
	if (argc > 0) {
		specific = ns_addr(*argv);
		msg->rip_nets[0].rip_dst = specific.x_net;
		specific.x_host = zero_addr.x_host;
		specific.x_port = zero_addr.x_port;
		printf("Net asked for was %s\n", ns_ntoa(specific));
	}
	if (sendto(s, packet, sizeof (struct rip), 0,
	  &router, sizeof(router)) < 0)
		perror(host);
}

/*
 * Handle an incoming routing packet.
 */
rip_input(from, msg,  size)
	struct sockaddr_ns *from;
	register struct rip *msg;
	int size;
{
	struct netinfo *n;
	char *name;
	int lna, net, subnet;
	struct hostent *hp;
	struct netent *np;
	static struct ns_addr work;

	if (htons(msg->rip_cmd) != RIPCMD_RESPONSE)
		return;
	printf("from %s\n", ns_ntoa(from->sns_addr));
	size -= sizeof (struct idp);
	size -= sizeof (short);
	n = msg->rip_nets;
	while (size > 0) {
		union ns_net_u net;
		if (size < sizeof (struct netinfo))
			break;
		net.net_e = n->rip_dst;
		printf("\t%d, metric %d\n", ntohl(net.long_e),
			ntohs(n->rip_metric));
		size -= sizeof (struct netinfo), n++;
	}
}

timeout()
{
	timedout = 1;
}
getsocket(type, proto)
	int type, proto; 
{
	struct sockaddr_ns *sns = &myaddr;
	int domain;
	int retry, s, on = 1;

	bzero ((char *) &myaddr, sizeof (myaddr));
	domain= myaddr.sns_family = AF_NS;
	retry = 1;
	while ((s = socket(domain, type, proto)) < 0 && retry) {
		perror("socket");
		sleep(5 * retry);
		retry <<= 1;
	}
	if (retry == 0)
		return (-1);
	while (bind(s, sns, sizeof (*sns)) < 0 && retry) {
		perror("bind");
		sleep(5 * retry);
		retry <<= 1;
	}
	if (retry == 0)
		return (-1);
	if (domain==AF_NS) {
		struct idp idp;
		if (setsockopt(s, 0, SO_HEADERS_ON_INPUT, &on, sizeof(on))) {
			perror("setsockopt SEE HEADERS");
			exit(1);
		}
		idp.idp_pt = NSPROTO_RI;
		if (setsockopt(s, 0, SO_DEFAULT_HEADERS, &idp, sizeof(idp))) {
			perror("setsockopt SET HEADERS");
			exit(1);
		}
	}
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		perror("setsockopt SO_BROADCAST");
		exit(1);
	}
	return (s);
}
