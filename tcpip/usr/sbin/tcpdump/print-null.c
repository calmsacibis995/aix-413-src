static char sccsid[] = "@(#)55	1.2  src/tcpip/usr/sbin/tcpdump/print-null.c, tcpip, tcpip411, GOLD410 9/15/93 14:52:23";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: null_if_print
 *		null_print
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
 * Copyright (c) 1990 The Regents of the University of California.
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#ifdef _AIX
#include <sys/cdli.h>
#include <net/nd_lan.h>
#endif /* _AIX */
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include <net/bpf.h>

#include "interface.h"
#include "addrtoname.h"

#define	NULL_HDRLEN 4

static void
null_print(p, ip, length)
	u_char *p;
	struct ip *ip;
	int length;
{
  	u_int family;

	bcopy(p, &family, sizeof(family));

	if (nflag) {
		/* XXX just dump the header */
		return;
	}
	switch (family) {

	case AF_INET:
		printf("ip: ");
		break;

	case AF_NS:
		printf("ns: ");
		break;

	default:
		printf("AF %d: ", family);
		break;
	}
}

void
null_if_print(p, tvp, length, caplen)
	u_char *p;
	struct timeval *tvp;
	int length;
	int caplen;
{
	struct ip *ip;

	ts_print(tvp);

	/*
	 * Some printers want to get back at the link level addresses,
	 * and/or check that they're not walking off the end of the packet.
	 * Rather than pass them all the way down, we set these globals.
	 */
	packetp = (u_char *)p;
	snapend = (u_char *)p + caplen;

	length -= NULL_HDRLEN;

	ip = (struct ip *)(p + NULL_HDRLEN);

	if (eflag)
		null_print(p, ip, length);

	ip_print(ip, length);

	if (xflag)
		default_print((u_short *)ip, caplen - NULL_HDRLEN);
	putchar('\n');
}

