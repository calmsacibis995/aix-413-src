static char sccsid[] = "@(#)48	1.5  src/tcpip/usr/sbin/tcpdump/print-fddi.c, tcpip, tcpip411, GOLD410 3/25/94 08:14:50";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: fddi_if_print
 *		fddi_print
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
#include <locale.h>
#include <nl_types.h>
#include "tcpdump_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TCPDUMP,n,s)

#include <stdio.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/mbuf.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#ifdef _AIX
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <netinet/if_802_5.h>
#include <netinet/if_fddi.h>
#endif /* _AIX */
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>

#include <net/bpf.h>

#include "interface.h"
#include "addrtoname.h"


static inline void
fddi_print(fddi, length, mac_llc_len)
	register u_char *fddi;
	int length;
	int mac_llc_len;
{
	struct ie2_llc_snaphdr *llcp;

	llcp = fddi + (mac_llc_len - sizeof(struct ie2_llc_snaphdr));
	if (qflag)
		(void)printf("%s %s %d: ",
			     traddr_string(fddi+8),
			     traddr_string(fddi+2),
			     length);
	else
		(void)printf("%s %s %s %d: ",
			     traddr_string(fddi+8),
			     traddr_string(fddi+2),
			     etherproto_string(llcp->type),
			     length);
}

void
fddi_if_print(p, tvp, length, caplen)
	u_char *p;
	struct timeval *tvp;
	int length;
	int caplen;
{
	register u_char *fd;
	struct ip *ip;
	u_short type;
	int maclen;
	int mac_llc_len;
	struct ie2_llc_snaphdr *llc;

	ts_print(tvp);

	maclen = mac_size_f((struct fddi_mac_hdr *)p);
	mac_llc_len = maclen + sizeof(struct ie2_llc_snaphdr);
	llc = (struct ie2_llc_snaphdr *)
		((caddr_t)p + mac_size_f((struct fddi_mac_hdr *)p));

	if (caplen < maclen) {
		printf("[|fddi]");
		goto out;
	}

	/*
	 * Some printers want to get back at the link level addresses,
	 * and/or check that they're not walking off the end of the packet.
	 * Rather than pass them all the way down, we set these globals.
	 */
	packetp = (u_char *)p;
	snapend = (u_char *)p + caplen;

	if (eflag)
		fddi_print(p, length, mac_llc_len);

	fd = p;
	p += mac_llc_len;
	length -= mac_llc_len;

	switch (ntohs(llc->type)) {

	case ETHERTYPE_IP:
		ip_print((struct ip *)p, length);
		break;

	case ETHERTYPE_ARP:
	case ETHERTYPE_REVARP:
		arp_print((struct ether_arp *)p, length, caplen - mac_llc_len);
		break;

	default:
		if (!eflag)
			fddi_print(fd, length, mac_llc_len);
		if (!xflag && !qflag)
			default_print((u_short *)p, caplen - mac_llc_len);
		break;
	}
	if (xflag)
		default_print((u_short *)p, caplen - mac_llc_len);

out:
	putchar('\n');
}
