static char sccsid[] = "@(#)09	1.3  src/bos/usr/sbin/XNSrouted/startup.c, cmdnet, bos411, 9428A410j 4/17/91 14:51:18";
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
 * Copyright (c) 1985 The Regents of the University of California.
 * Copyright (c) 1985 The Regents of the University of California.
 * All rights reserved.
 *
 * This file includes significant work done at Cornell University by
 * Bill Nesheim.  That work included by permission.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * 		 "@(#)startup.c	5.9 (Berkeley) 6/1/90";
 *
 */


/*
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <nlist.h>
#include <syslog.h>

struct	interface *ifnet;
int	lookforinterfaces = 1;
int	performnlist = 1;
int	gateway = 0;
int	externalinterfaces = 0;		/* # of remote and local interfaces */
char ether_broadcast_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


/*
 * Find the network interfaces which have configured themselves.
 * If the interface is present but not yet up (for example an
 * ARPANET IMP), set the lookforinterfaces flag so we'll
 * come back later and look again.
 */
ifinit()
{
	struct interface ifs, *ifp, *ifpa = 0;
	int s;
	struct sockaddr *dst;
	struct rt_entry *rt;
        struct ifconf ifc;
	char buf[BUFSIZ], *cp, *cplim;
        struct ifreq ifreq, *ifr;
	u_long i;

	if ((s = socket(AF_NS, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, "socket: %m");
		exit(1);
	}
        ifc.ifc_len = sizeof (buf);
        ifc.ifc_buf = buf;
        if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
                syslog(LOG_ERR, "ioctl (get interface configuration)");
		close(s);
                exit(1);
        }
        ifr = ifc.ifc_req;
	lookforinterfaces = 0;
#ifdef  _BSD
#define max(a, b) (a > b ? a : b)
#define size(p)	(max((p).sa_len, sizeof(p)) - sizeof(p))
#else
#define size(p) (sizeof (p))
#endif
	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim;
			cp += sizeof (*ifr) + size(ifr->ifr_addr)) {
		ifr = (struct ifreq *) cp;
		bzero((char *)&ifs, sizeof(ifs));
		ifs.int_addr = ifr->ifr_addr;
		ifreq = *ifr;

		/* no one cares about software loopback interfaces */
		if (strncmp(ifr->ifr_name,"lo", 2)==0)
			continue;

                if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
                        syslog(LOG_ERR, "ioctl (get interface flags)");
                        continue;
                }
		ifs.int_flags = ifreq.ifr_flags | IFF_INTERFACE;
		if ((ifs.int_flags & IFF_UP) == 0 ||
		    ifr->ifr_addr.sa_family == AF_UNSPEC) {
		    /* 
		     * already known to us? 
		     * what makes a POINTOPOINT if unique is its dst addr,
		     * NOT its source address 
		     */
		    if ( ((ifs.int_flags & IFF_POINTOPOINT) &&
			( ifpa = if_ifwithdstaddr(&ifs.int_dstaddr)) ) ||
			   (((ifs.int_flags & IFF_POINTOPOINT) == 0) &&
			   (ifpa = if_ifwithaddr(&ifs.int_addr)) )) {
				/* if the hardware interface changes state
				 * update the routing table
				 */
				if ( ifpa && (ifpa->int_flags & IFF_UP)) {
				     ifpa->int_flags &= ~IFF_UP;
				     if (ifp->int_flags & IFF_POINTOPOINT) 
					dst = &ifp->int_dstaddr;
				     else
					dst = &ifpa->int_broadaddr;
				     rt = rtlookup(dst);
				     if (rt)
				     rtdelete(rt);
				}
		    }
		    lookforinterfaces = 1;
		    continue;
		}
		if (ifs.int_addr.sa_family != AF_NS)
			continue;
                if (ifs.int_flags & IFF_POINTOPOINT) {
                        if (ioctl(s, SIOCGIFDSTADDR, (char *)&ifreq) < 0) {
                                syslog(LOG_ERR, "ioctl (get dstaddr): %m");
                                continue;
			}
			ifs.int_dstaddr = ifreq.ifr_dstaddr;
		}
                if (ifs.int_flags & IFF_BROADCAST) {
                        if (ioctl(s, SIOCGIFBRDADDR, (char *)&ifreq) < 0) {
                                syslog(LOG_ERR, "ioctl (get broadaddr: %m");
                                continue;
                        }
			ifs.int_broadaddr = ifreq.ifr_broadaddr;
		}
		/* 
		 * already known to us? 
		 * what makes a POINTOPOINT if unique is its dst addr,
		 * NOT its source address 
		 */
		if ( ((ifs.int_flags & IFF_POINTOPOINT) &&
			( ifpa = if_ifwithdstaddr(&ifs.int_dstaddr)) ) ||
			   (((ifs.int_flags & IFF_POINTOPOINT) == 0) &&
			   (ifpa = if_ifwithaddr(&ifs.int_addr)) )) {
				/* if the hardware interface changes state
				 * update the routing table
				 */
				if ( ifpa && !(ifpa->int_flags & IFF_UP)) {
				     ifpa->int_flags |= IFF_UP;
				     addrouteforif(ifpa);
				}
			 	continue;
		}
		ifp = (struct interface *)malloc(sizeof (struct interface));
		if (ifp == 0) {
			syslog(LOG_ERR,"XNSrouted: out of memory\n");
			break;
		}
		*ifp = ifs;
		/*
		 * Count the # of directly connected networks
		 * and point to point links which aren't looped
		 * back to ourself.  This is used below to
		 * decide if we should be a routing ``supplier''.
		 */
		if ((ifs.int_flags & IFF_POINTOPOINT) == 0 ||
		    if_ifwithaddr(&ifs.int_dstaddr) == 0)
			externalinterfaces++;
		/*
		 * If we have a point-to-point link, we want to act
		 * as a supplier even if it's our only interface,
		 * as that's the only way our peer on the other end
		 * can tell that the link is up.
		 */
		if ((ifs.int_flags & IFF_POINTOPOINT) && supplier < 0)
			supplier = 1;
		ifp->int_name = malloc(strlen(ifr->ifr_name) + 1);
		if (ifp->int_name == 0) {
			syslog(LOG_ERR,"XNSrouted: out of memory\n");
			exit(1);
		}
		strcpy(ifp->int_name, ifr->ifr_name);
		ifp->int_metric = 0;
		ifp->int_next = ifnet;
		ifnet = ifp;
		traceinit(ifp);
		addrouteforif(ifp);
	}
	if (externalinterfaces > 1 && supplier < 0)
		supplier = 1;
	close(s);
}

addrouteforif(ifp)
	struct interface *ifp;
{
	struct sockaddr_ns net;
	struct sockaddr *dst;
	int state, metric;
	struct rt_entry *rt;

	if (ifp->int_flags & IFF_POINTOPOINT) {
		int (*match)();
		register struct interface *ifp2 = ifnet;
		register struct interface *ifprev = ifnet;
		
		dst = &ifp->int_dstaddr;

		/* Search for interfaces with the same net */
		ifp->int_sq.n = ifp->int_sq.p = &(ifp->int_sq);
		match = afswitch[dst->sa_family].af_netmatch;
		if (match)
		for (ifp2 = ifnet; ifp2; ifp2 =ifp2->int_next) {
			if (ifp->int_flags & IFF_POINTOPOINT == 0)
				continue;
			if ((*match)(&ifp2->int_dstaddr,&ifp->int_dstaddr)) {
				insque(&ifp2->int_sq,&ifp->int_sq);
				break;
			}
		}
	} else {
		dst = &ifp->int_broadaddr;
	}
	rt = rtlookup(dst);
	if (rt)
		rtdelete(rt);
	if (tracing)
		fprintf(stderr, "Adding route to interface %s\n", ifp->int_name);
	if (ifp->int_transitions++ > 0)
		syslog(LOG_ERR, "re-installing interface %s", ifp->int_name);
	rtadd(dst, &ifp->int_addr, ifp->int_metric,
		ifp->int_flags & (IFF_INTERFACE|IFF_PASSIVE|IFF_REMOTE));
}

