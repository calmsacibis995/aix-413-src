static char sccsid[] = "@(#)99	1.1  src/tcpip/usr/sbin/tcpdump/print-iso88023.c, tcpip, tcpip411, GOLD410 4/9/94 15:00:27";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: iso88023_if_print
 *		iso88023_print
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
iso88023_print(ep, length)
	register u_char *ep;
	int length;
{
	struct ie2_llc_snaphdr *llcp;

	llcp = (struct ie2_llc_snaphdr *)(ep + sizeof(struct ie3_mac_hdr));

	if(qflag)
		(void)printf("%s %s %d: ",
			     etheraddr_string(ESRC((struct ether_header *)ep)),
			     etheraddr_string(EDST((struct ether_header *)ep)),
			     length);
	else
		(void)printf("%s %s %s %d: ",
			     etheraddr_string(ESRC((struct ether_header *)ep)),
			     etheraddr_string(EDST((struct ether_header *)ep)),
			     etherproto_string(llcp->type),
			     length);
}

/*
 * This is the top level routine of the printer.  'p' is the points
 * to the ether header of the packet, 'tvp' is the timestamp, 
 * 'length' is the length of the packet off the wire, and 'caplen'
 * is the number of bytes actually captured.
 */
void
iso88023_if_print(p, tvp, length, caplen)
	u_char *p;
	struct timeval *tvp;
	int length;
	int caplen;
{
	struct ie3_hdr *ep;
	struct ie2_llc_snaphdr *llcp;
	register int i;

	llcp = (struct ie2_llc_snaphdr *)
			((caddr_t)p + sizeof(struct ie3_mac_hdr));

	ts_print(tvp);

	if (caplen < sizeof(struct ie3_hdr)) {
		printf("[|iso88023]");
		goto out;
	}

	if (eflag)
		iso88023_print((struct ie3_hdr *)p, length);

	/*
	 * Some printers want to get back at the ethernet addresses,
	 * and/or check that they're not walking off the end of the packet.
	 * Rather than pass them all the way down, we set these globals.
	 */
	packetp = p;
	snapend = p + caplen;
	
	length -= sizeof(struct ie3_hdr);
	ep = (struct ie3_hdr *)p;
	p += sizeof(struct ie3_hdr);
	switch (ntohs(llcp->type)) {

	case _802_3_TYPE_IP:
		ip_print((struct ip *)p, length);
		break;

	case _802_3_TYPE_ARP:
		arp_print((struct ether_arp *)p, length, caplen - sizeof(*ep));
		break;

	default:
		if (!eflag)
			iso88023_print(ep, length);
		if (!xflag && !qflag)
			default_print((u_short *)p, caplen - sizeof(*ep));
		break;
	}
	if (xflag)
		default_print((u_short *)p, caplen - sizeof(*ep));
 out:
	putchar('\n');
}
