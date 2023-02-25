static char sccsid[] = "@(#)94	1.1  src/tcpip/usr/sbin/ipreport/p-rip.c, tcpras, tcpip411, GOLD410 12/8/92 10:53:21";
/*
 * Copyright (c) 1988-1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
static char rcsid[] =
    "$Header: print-rip.c,v 1.12 91/04/19 10:46:46 mccanne Exp $ (LBL)";
*/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <protocols/routed.h>

#include <errno.h>

/* newish RIP commands */
#ifndef RIPCMD_POLL
#define RIPCMD_POLL 5
#endif
#ifndef RIPCMD_POLLENTRY
#define RIPCMD_POLLENTRY 6
#endif

static void
rip_entry_print(ni)
	register struct netinfo *ni;
{
	int m;

	if (ntohs(ni->rip_dst.sa_family) != AF_INET) {
		register int i;

		printf("\t[family %d:", ntohs(ni->rip_dst.sa_family));
		for (i = 0; i < 14; i += 2)
			printf(" %02x%02x", ni->rip_dst.sa_data[i],
				ni->rip_dst.sa_data[i+1]);
		printf("]");
	} else {
		register struct sockaddr_in *sin = 
				(struct sockaddr_in *)&ni->rip_dst;
		printf("\tdst %s", inet_ntoa(sin->sin_addr));
		if (sin->sin_port)
			printf(" [port %d]", sin->sin_port);
	}
	m = ntohl (ni -> rip_metric);
	printf(" metric %d%s\n", m, m == HOPCNT_INFINITY ? " (infinity)" : "");
}

void
rip_dump(dat, length)
	u_char *dat;
	int length;
{
	register struct rip *rp = (struct rip *)dat;
	register struct netinfo *ni;
	register int i = MIN(length, length) -
			 (sizeof(struct rip) - sizeof(struct netinfo));
	int j;
	int trunc;
	
	if (i < 0)
		return;

	printf("RIP Packet breakdown:\n");
	if (rp->rip_vers != RIPVERSION)
		printf("    RIPVERSION = %d\n", rp->rip_vers);
	printf("    RIPCMD = %d ", rp -> rip_cmd);
	switch (rp->rip_cmd) {

	case RIPCMD_REQUEST:
		j = length / sizeof(*ni);
		trunc = ((i / sizeof(*ni)) * sizeof(*ni) != i);
		if (j * sizeof(*ni) != length - 4)
		    printf("(REQUEST%s) %d[%d]:\n", 
			    trunc ? "-truncated" : "", j, length);
		else
		    printf("(REQUEST%s) %d:\n", 
			    trunc ? "-truncated" : "", j);
		for (ni = rp->rip_nets, j = 0; (i -= sizeof(*ni)) >= 0;
			++ni, j++) {

		    /*
		     * A single entry with sa_family == AF_UNSPEC and
		     * metric ``infinity'' means ``all routes''.
		     */
		    if (ntohs(ni->rip_dst.sa_family) == AF_UNSPEC &&
			ntohl (ni->rip_metric) == HOPCNT_INFINITY && j == 0) 
			printf ("\tALL-ROUTES\n");
		    else
			rip_entry_print(ni);
		}
		break;
	case RIPCMD_RESPONSE:
		j = length / sizeof(*ni);
		trunc = ((i / sizeof(*ni)) * sizeof(*ni) != i);
		if (j * sizeof(*ni) != length - 4)
			printf("(RESPONSE%s) %d[%d]:\n", 
				trunc ? "-truncated" : "", j, length);
		else
			printf("(RESPONSE%s) %d:\n", 
				trunc ? "-truncated" : "", j);
		for (ni = rp->rip_nets; (i -= sizeof(*ni)) >= 0; ++ni)
			rip_entry_print(ni);
		break;
	case RIPCMD_TRACEON:
		printf("(TRACEON) len=%d: tracefile=\"%s\"\n", 
			length, rp->rip_tracefile);
		break;
	case RIPCMD_TRACEOFF:
		printf("(TRACEOFF) len=%d\n", length);
		break;
	case RIPCMD_POLL:
		printf("(POLL) len=%d\n", length);
		break;
	case RIPCMD_POLLENTRY:
		printf("(POLLENTRY) len=%d\n", length);
		break;
	default:
		printf("(unknown) len=%d\n", length);
		break;
	}
}
