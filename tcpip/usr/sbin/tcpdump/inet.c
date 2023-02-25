static char sccsid[] = "@(#)30	1.3  src/tcpip/usr/sbin/tcpdump/inet.c, tcpip, tcpip411, GOLD410 11/8/93 15:46:14";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: ISLOOPBACK
 *		ipaddrtonetmask
 *		lookup_device
 *		lookup_net
 *		is_printer_supported
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
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>

#include "interface.h"

#include <locale.h>
#include <nl_types.h>
#include "tcpdump_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TCPDUMP,n,s)

/* Not all systems have IFF_LOOPBACK */
#ifdef IFF_LOOPBACK
#define ISLOOPBACK(p) ((p)->ifr_flags & IFF_LOOPBACK)
#else
#define ISLOOPBACK(p) (strcmp((p)->ifr_name, "lo0") == 0)
#endif

/*
 * Return the name of a network interface attached to the system, or 0
 * if none can be found.  The interface must be configured up; the
 * lowest unit number is preferred; loopback is ignored.
 */
char *
lookup_device()
{
	struct ifreq ibuf[16], *ifrp, *ifend, *mp;
	struct ifconf ifc;
	int fd;
	int minunit, n;
	char *cp;
	static char device[sizeof(ifrp->ifr_name)];

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("tcpdump: socket");
		exit(1);
	}
	ifc.ifc_len = sizeof ibuf;
	ifc.ifc_buf = (caddr_t)ibuf;

	if (ioctl(fd, SIOCGIFCONF, (char *)&ifc) < 0 ||
	    ifc.ifc_len < sizeof(struct ifreq)) {
		perror("tcpdump: SIOCGIFCONF: ");
		exit(1);
	}
	ifrp = ibuf;
	ifend = (struct ifreq *)((char *)ibuf + ifc.ifc_len);
	
#define size(p)	MAX((p).sa_len, sizeof(p))
	mp = 0;
	minunit = 666;
	while (ifrp < ifend) {
		struct ifreq ifr;
		/*
		 * Need a template to preserve address info that is
		 * used below to locate the next entry.  (Otherwise,
		 * SIOCGIFFLAGS stomps over it because the requests
		 * are returned in a union.)
		 */
		bcopy(ifrp->ifr_name, ifr.ifr_name, sizeof(ifr.ifr_name));
		if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) < 0) {
			fprintf(stderr, "tcpdump: SIOCGIFFLAGS: ");
			perror(ifrp->ifr_name);
			exit(1);
		}
#ifdef DEBUG
		printf("iname = %s flags =%x\n", ifrp->ifr_name, ifr.ifr_flags );
#endif
		if ((ifr.ifr_flags & IFF_UP) && !ISLOOPBACK(&ifr) &&
			is_printer_supported(ifrp->ifr_name)) {
			for (cp = ifrp->ifr_name; !isdigit(*cp); ++cp)
				;
			n = atoi(cp);
			if (n < minunit) {
				minunit = n;
				mp = ifrp;
			}
		}
		ifrp = (struct ifreq *) ((char *) ifrp + sizeof(ifrp->ifr_name)
			 + size(ifrp->ifr_addr));
	}
	close(fd);
	if (mp == 0)
		return (0);
	
	(void)strcpy(device, mp->ifr_name);
	return (device);
}

/*
 * Get the netmask of an IP address.  This routine is used if
 * SIOCGIFNETMASK doesn't work.
 */
static u_long
ipaddrtonetmask(addr)
	u_long addr;
{
	if (IN_CLASSA(addr))
		return (IN_CLASSA_NET);
	if (IN_CLASSB(addr))
		return (IN_CLASSB_NET);
	if (IN_CLASSC(addr))
		return (IN_CLASSC_NET);
	error(MSGSTR(IPCLASS, "unknown IP address class: %08X"), addr);
	/* NOTREACHED */
}

void
lookup_net(device, netp, maskp)
	char *device;
	u_long *netp;
	u_long *maskp;
{
	int fd;
	struct ifreq ifr;
	struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;

	/* Use data gram socket to get IP address. */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("tcpdump: socket");
		exit(1);
	}
	(void)strncpy(ifr.ifr_name, device, sizeof ifr.ifr_name);
	if (ioctl(fd, SIOCGIFADDR, (char *)&ifr) < 0) {
		/*
		 * This will fail if an IP address hasn't been assigned.
		 */
		*netp = 0;
		*maskp = 0;
		return;
	}
	*netp = sin->sin_addr.s_addr;
	if (ioctl(fd, SIOCGIFNETMASK, (char *)&ifr) < 0)
		*maskp = 0;
	else
		*maskp = sin->sin_addr.s_addr;
	if (*maskp == 0)
		*maskp = ipaddrtonetmask(*netp);
	*netp &= *maskp;
	(void)close(fd);
}

extern struct printer printers[];

int
is_printer_supported(name)
	char *name;
{
	struct printer *p; 
	int i;

	i=0;
	p=printers;
	while (p[i].ifname) {
		if (!strncmp(name, p[i].ifname, 2))
			return((int)name);
		i++;
	}
	return(0);
}
