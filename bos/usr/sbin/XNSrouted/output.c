static char sccsid[] = "@(#)07	1.2  src/bos/usr/sbin/XNSrouted/output.c, cmdnet, bos411, 9428A410j 9/30/93 14:11:47";
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
 * 		 "@(#)output.c	5.7 (Berkeley) 6/1/90";
 *
 */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"

/*
 * Apply the function "f" to all non-passive
 * interfaces.  If the interface supports the
 * use of broadcasting use it, otherwise address
 * the output to the known router.
 */
toall(f)
	int (*f)();
{
	register struct interface *ifp;
	register struct sockaddr *dst;
	register int flags;
	extern struct interface *ifnet;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_PASSIVE)
			continue;
		dst = ifp->int_flags & IFF_BROADCAST ? &ifp->int_broadaddr :
		      ifp->int_flags & IFF_POINTOPOINT ? &ifp->int_dstaddr :
		      &ifp->int_addr;
		flags = ifp->int_flags & IFF_INTERFACE ? MSG_DONTROUTE : 0;
		(*f)(dst, flags, ifp);
	}
}

/*
 * Output a preformed packet.
 */
/*ARGSUSED*/
Sendmsg(dst, flags, ifp)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
{

	(*afswitch[dst->sa_family].af_output)
		(flags, dst, sizeof (struct rip));
	TRACE_OUTPUT(ifp, dst, sizeof (struct rip));
}

/*
 * Supply dst with the contents of the routing tables.
 * If this won't fit in one packet, chop it up into several.
 */
supply(dst, flags, ifp)
	struct sockaddr *dst;
	int flags;
	struct interface *ifp;
{
	register struct rt_entry *rt;
	register struct rthash *rh;
	register struct netinfo *nn;
	register struct netinfo *n = msg->rip_nets;
	struct rthash *base = hosthash;
	struct sockaddr_ns *sns =  (struct sockaddr_ns *) dst;
	int (*output)() = afswitch[dst->sa_family].af_output;
	int doinghost = 1, size, metric;
	union ns_net net;

	msg->rip_cmd = ntohs(RIPCMD_RESPONSE);
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		size = (char *)n - (char *)msg;
		if (size > MAXPACKETSIZE - sizeof (struct netinfo)) {
			(*output)(flags, dst, size);
			TRACE_OUTPUT(ifp, dst, size);
			n = msg->rip_nets;
		}
		sns = (struct sockaddr_ns *)&rt->rt_dst;
	        if ((rt->rt_flags & (RTF_HOST|RTF_GATEWAY)) == RTF_HOST)
			sns = (struct sockaddr_ns *)&rt->rt_router;
		metric = min(rt->rt_metric + 1, HOPCNT_INFINITY);
		net = sns->sns_addr.x_net;
		/*
		 * Make sure that we don't put out a two net entries
		 * for a pt to pt link (one for the G route, one for the if)
		 * This is a kludge, and won't work if there are lots of nets.
		 */
		for (nn = msg->rip_nets; nn < n; nn++) {
			if (ns_neteqnn(net, nn->rip_dst)) {
				if (metric < ntohs(nn->rip_metric))
					nn->rip_metric = htons(metric);
				goto next;
			}
		}
		n->rip_dst = net;
		n->rip_metric = htons(metric);
		n++;
	next:;
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	if (n != msg->rip_nets) {
		size = (char *)n - (char *)msg;
		(*output)(flags, dst, size);
		TRACE_OUTPUT(ifp, dst, size);
	}
}
