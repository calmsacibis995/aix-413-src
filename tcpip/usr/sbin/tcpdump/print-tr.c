static char sccsid[] = "@(#)63	1.4  src/tcpip/usr/sbin/tcpdump/print-tr.c, tcpip, tcpip411, GOLD410 3/25/94 08:15:01";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: tr_if_print
 *		tr_print
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
#include <netinet/if_802_5.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include "interface.h"
#include "addrtoname.h"
#include "extract.h"

u_char *packetp;
u_char *snapend;

static inline void
tr_print(tr, length, mac_llc_len)
	register u_char *tr;
	int length;
	int mac_llc_len;
{
	struct ie2_llc_snaphdr *llcp;

	llcp = tr + (mac_llc_len - sizeof(struct ie2_llc_snaphdr));
	if (qflag)
		(void)printf("%s %s %d: ",
			     traddr_string(tr+8),
			     traddr_string(tr+2),
			     length);
	else
		(void)printf("%s %s %s %d: ",
			     traddr_string(tr+8),
			     traddr_string(tr+2),
			     etherproto_string(llcp->type),
			     length);
}

/*
 * This is the top level routine of the printer.  'p' is the points
 * to the tokenring mac header of the packet, 'tvp' is the timestamp, 
 * 'length' is the length of the packet off the wire, and 'caplen'
 * is the number of bytes actually captured.
 */
void
tr_if_print(p, tvp, length, caplen)
	u_char *p;
	struct timeval *tvp;
	int length;
	int caplen;
{
	register u_char *tr;
	register int i;
	struct ie2_llc_snaphdr *llcp;
	int macsize, mac_llc_len;

	macsize = mac_size((struct ie5_mac_hdr *)p);
	mac_llc_len = (macsize + sizeof(struct ie2_llc_snaphdr));

	llcp = (struct ie2_llc_snaphdr *) ((caddr_t)p + macsize);

	ts_print(tvp);

	if (caplen < mac_llc_len) {
		printf("[|tokenring]");
		goto out;
	}

	if (eflag)
		tr_print(p, length, mac_llc_len);

	/*
	 * Some printers want to get back at the tokenring addresses,
	 * and/or check that they're not walking off the end of the packet.
	 * Rather than pass them all the way down, we set these globals.
	 */
	packetp = p;
	snapend = (p + caplen);
	
	tr = p;
	p = ((caddr_t)p + (mac_llc_len));
	length -= mac_llc_len; 

	switch (ntohs(llcp->type)) {

	case _802_5_TYPE_IP:
		ip_print((struct ip *)p, length);
		break;

	case _802_5_TYPE_ARP:
		arp_print((struct ie5_arp *)p, length, caplen - mac_llc_len);
		break;
	default:
		if (!eflag)
			tr_print(tr, length, mac_llc_len);
		if (!xflag && !qflag)
			default_print((u_short *)p, caplen - mac_llc_len);
		break;
	}
	if (xflag)
		default_print((u_short *)p, caplen - mac_llc_len);
 out:
	putchar('\n');
}
