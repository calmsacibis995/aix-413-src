static char sccsid[] = "@(#)42	1.3  src/tcpip/usr/sbin/tcpdump/print-arp.c, tcpip, tcpip41J, 9510A 3/2/95 09:13:12";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: arp_print
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

#include "interface.h"
#include "addrtoname.h"

void
arp_print(ap, length, caplen)
	register struct ether_arp *ap;
	int length;
	int caplen;
{
	if ((u_char *)(ap + 1) > snapend) {
		printf("[|arp]");
		return;
	}
	if (length < sizeof(struct ether_arp)) {
		(void)printf("truncated-arp");
		default_print((u_short *)ap, length);
		return;
	}

	NTOHS(ap->arp_hrd);
	NTOHS(ap->arp_pro);
	NTOHS(ap->arp_op);

	if ((ap->arp_pro != ETHERTYPE_IP
		&& ap->arp_pro != ETHERTYPE_TRAIL)
	    || ap->arp_hln != sizeof(SHA(ap))
	    || ap->arp_pln != sizeof(SPA(ap))) {
		(void)printf("arp-req #%d for proto #%d (%d) hardware %d (%d)",
				ap->arp_op, ap->arp_pro, ap->arp_pln,
				ap->arp_hrd, ap->arp_hln);
		return;
	}
	if (ap->arp_pro == ETHERTYPE_TRAIL)
		(void)printf("trailer");
	switch (ap->arp_op) {

	case ARPOP_REQUEST:
		(void)printf("arp who-has %s tell %s",
			ipaddr_string(TPA(ap)),
			ipaddr_string(SPA(ap)));
		break;

	case ARPOP_REPLY:
		(void)printf("arp reply %s is-at %s",
			ipaddr_string(SPA(ap)),
			etheraddr_string(SHA(ap)));
		break;

/* XXX - someday
	case REVARP_REQUEST:
		(void)printf("rarp who-is %s tell %s",
			etheraddr_string(THA(ap)),
			etheraddr_string(SHA(ap)));
		break;

	case REVARP_REPLY:
		(void)printf("rarp reply %s at %s",
			etheraddr_string(THA(ap)),
			ipaddr_string(TPA(ap)));
		break;
*/

	default:
		(void)printf("arp-%d", ap->arp_op);
		default_print((u_short *)ap, caplen);
		break;
	}
}
