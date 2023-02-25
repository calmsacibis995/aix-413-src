static char sccsid[] = "@(#)53	1.2  src/tcpip/usr/sbin/tcpdump/print-ether.c, tcpip, tcpip411, GOLD410 9/15/93 14:51:14";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: ether_if_print
 *		ether_print
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#ifdef _AIX
#include <sys/cdli.h>
#include <net/nd_lan.h>
#endif /* _AIX */
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include "interface.h"
#include "addrtoname.h"

u_char *packetp;
u_char *snapend;

static inline void
ether_print(ep, length)
	register struct ether_header *ep;
	int length;
{
	if (qflag)
		(void)printf("%s %s %d: ",
			     etheraddr_string(ESRC(ep)),
			     etheraddr_string(EDST(ep)),
			     length);
	else
		(void)printf("%s %s %s %d: ",
			     etheraddr_string(ESRC(ep)),
			     etheraddr_string(EDST(ep)),
			     etherproto_string(ep->ether_type), 
			     length);
}

/*
 * This is the top level routine of the printer.  'p' is the points
 * to the ether header of the packet, 'tvp' is the timestamp, 
 * 'length' is the length of the packet off the wire, and 'caplen'
 * is the number of bytes actually captured.
 */
void
ether_if_print(p, tvp, length, caplen)
	u_char *p;
	struct timeval *tvp;
	int length;
	int caplen;
{
	struct ether_header *ep;
	register int i;

	ts_print(tvp);

	if (caplen < sizeof(struct ether_header)) {
		printf("[|ether]");
		goto out;
	}

	if (eflag)
		ether_print((struct ether_header *)p, length);

	/*
	 * Some printers want to get back at the ethernet addresses,
	 * and/or check that they're not walking off the end of the packet.
	 * Rather than pass them all the way down, we set these globals.
	 */
	packetp = p;
	snapend = p + caplen;
	
	length -= sizeof(struct ether_header);
	ep = (struct ether_header *)p;
	p += sizeof(struct ether_header);
	switch (ntohs(ep->ether_type)) {

	case ETHERTYPE_IP:
		ip_print((struct ip *)p, length);
		break;

	case ETHERTYPE_ARP:
	case ETHERTYPE_REVARP:
		arp_print((struct ether_arp *)p, length, caplen - sizeof(*ep));
		break;

	default:
		if (!eflag)
			ether_print(ep, length);
		if (!xflag && !qflag)
			default_print((u_short *)p, caplen - sizeof(*ep));
		break;
	}
	if (xflag)
		default_print((u_short *)p, caplen - sizeof(*ep));
 out:
	putchar('\n');
}
