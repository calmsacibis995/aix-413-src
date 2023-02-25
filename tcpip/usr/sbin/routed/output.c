static char sccsid[] = "@(#)18	1.9  src/tcpip/usr/sbin/routed/output.c, tcprouting, tcpip411, GOLD410 10/8/93 10:29:18";
/* 
 * COMPONENT_NAME: TCPIP output.c
 * 
 * FUNCTIONS: sendmsg, supply, toall 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
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
 */
/*
#ifndef lint
static char sccsid[] = "@(#)output.c	5.14 (Berkeley) 6/1/90";
#endif  not lint */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>

/*
 * Apply the function "f" to all non-passive
 * interfaces.  If the interface supports the
 * use of broadcasting use it, otherwise address
 * the output to the known router.
 */
toall(f, rtstate, skipif)
	int (*f)();
	int rtstate;
	struct interface *skipif;
{
	register struct interface *ifp;
	register struct sockaddr *dst;
	register int flags;
	extern struct interface *ifnet;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_PASSIVE || ifp == skipif)
			continue;
		dst = ifp->int_flags & IFF_BROADCAST ? &ifp->int_broadaddr :
		      ifp->int_flags & IFF_POINTOPOINT ? &ifp->int_dstaddr :
		      &ifp->int_addr;
		flags = ifp->int_flags & IFF_INTERFACE ? MSG_DONTROUTE : 0;
		(*f)(dst, flags, ifp, rtstate);

		/*
		 * If NOECHO, make the route status follow
		 * the if status, and disallow timeouts.
		 */
		
		if (ifp->int_flags & IFF_NOECHO) {
			struct ifreq ifr;
			struct rt_entry *rt;
			static int removed = 0;

			rt = rtfind(dst);
			if (rt) rt->rt_timer = 0;

			memcpy(ifr.ifr_name, ifp->int_name, IFNAMSIZ);
			if (ioctl(s, SIOCGIFFLAGS, &ifr) < 0)
				continue;
			if (ifr.ifr_flags & IFF_UP) {
				if (removed) {
					if (rt) continue;
					addrouteforif(ifp);
					removed = 0;
				}
			} else {
				if (!removed) {
					if (!rt) continue;
					rtdelete(rt);
					removed = 1;
				}
			}
		}
	}
}

/*
 * Output a preformed packet.
 */
/*ARGSUSED*/
Sendmsg(dst, flags, ifp, rtstate)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
	int rtstate;
{

	(*afswitch[dst->sa_family].af_output)(s, flags,
		dst, sizeof (struct rip));
	TRACE_OUTPUT(ifp, dst, sizeof (struct rip));
}

/*
 * Supply dst with the contents of the routing tables.
 * If this won't fit in one packet, chop it up into several.
 */
supply(dst, flags, ifp, rtstate)
	struct sockaddr *dst;
	int flags;
	register struct interface *ifp;
	int rtstate;
{
	register struct rt_entry *rt;
	register struct netinfo *n = msg->rip_nets;
	register struct rthash *rh;
	struct rthash *base = hosthash;
	int doinghost = 1, size;
	int (*output)() = afswitch[dst->sa_family].af_output;
	int (*sendroute)() = afswitch[dst->sa_family].af_sendroute;
	int npackets = 0;

	msg->rip_cmd = RIPCMD_RESPONSE;
	msg->rip_vers = RIPVERSION;
	bzero(msg->rip_res1, sizeof(msg->rip_res1));
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		/*
		 * Don't resend the information on the network
		 * from which it was received (unless sending
		 * in response to a query).
		 */
		if (ifp && rt->rt_ifp == ifp &&
		    (rt->rt_state & RTS_INTERFACE) == 0)
			continue;
		if (rt->rt_state & RTS_EXTERNAL)
			continue;
		/*
		 * For dynamic updates, limit update to routes
		 * with the specified state.
		 */
		if (rtstate && (rt->rt_state & rtstate) == 0)
			continue;
		/*
		 * Limit the spread of subnet information
		 * to those who are interested.
		 */
		if (doinghost == 0 && rt->rt_state & RTS_SUBNET) {
			if (rt->rt_dst.sa_family != dst->sa_family)
				continue;
			if ((*sendroute)(rt, dst) == 0)
				continue;
		}
		size = (char *)n - packet;
		if (size > MAXPACKETSIZE - sizeof (struct netinfo)) {
			TRACE_OUTPUT(ifp, dst, size);
			(*output)(s, flags, dst, size);
			/*
			 * If only sending to ourselves,
			 * one packet is enough to monitor interface.
			 */
			if (ifp && (ifp->int_flags &
			   (IFF_BROADCAST | IFF_POINTOPOINT | IFF_REMOTE)) == 0)
				return;
			n = msg->rip_nets;
			npackets++;
		}
		n->rip_dst = rt->rt_dst;
#if 0	/* (XXX) was: #if _BSD < 198810 */
		if (sizeof(n->rip_dst.sa_family) > 1)	/* XXX */
		n->rip_dst.sa_family = htons(n->rip_dst.sa_family);
#else
#define osa(x) ((struct osockaddr *)(&(x)))
		osa(n->rip_dst)->sa_family = htons(n->rip_dst.sa_family);
#endif
		n->rip_metric = htonl(rt->rt_metric);
		n++;
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	if (n != msg->rip_nets || (npackets == 0 && rtstate == 0)) {
		size = (char *)n - packet;
		TRACE_OUTPUT(ifp, dst, size);
		(*output)(s, flags, dst, size);
	}
}
