static char sccsid[] = "@(#)04	1.2  src/bos/usr/sbin/XNSrouted/input.c, cmdnet, bos411, 9428A410j 4/18/91 10:49:34";
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
 * 	 "@(#)input.c	5.9 (Berkeley) 6/1/90";
 *
 */


/*
 * XNS Routing Table Management Daemon
 */
#include "defs.h"

struct sockaddr *
xns_nettosa(net)
union ns_net net;
{
	static struct sockaddr_ns sxn;
	extern char ether_broadcast_addr[6];
	
	bzero(&sxn, sizeof (struct sockaddr_ns));
	sxn.sns_family = AF_NS;
	sxn.sns_len = sizeof (sxn);
	sxn.sns_addr.x_net = net;
	sxn.sns_addr.x_host = *(union ns_host *)ether_broadcast_addr;
	return( (struct sockaddr *)&sxn);
	
}

/*
 * Process a newly received packet.
 */
rip_input(from, size)
	struct sockaddr *from;
	int size;
{
	struct rt_entry *rt;
	struct netinfo *n;
	struct interface *ifp;
	int newsize;
	struct afswitch *afp;

	
	ifp = 0;
	TRACE_INPUT(ifp, from, size);
	if (from->sa_family >= AF_MAX)
		return;
	afp = &afswitch[from->sa_family];
	
	size -= sizeof (u_short)	/* command */;
	n = msg->rip_nets;

	switch (ntohs(msg->rip_cmd)) {

	case RIPCMD_REQUEST:
		newsize = 0;
		while (size > 0) {
			if (size < sizeof (struct netinfo))
				break;
			size -= sizeof (struct netinfo);

			/* 
			 * A single entry with rip_dst == DSTNETS_ALL and
			 * metric ``infinity'' means ``all routes''.
			 */
			if (ns_neteqnn(n->rip_dst, ns_anynet) &&
		            ntohs(n->rip_metric) == HOPCNT_INFINITY &&
			    size == 0) {
				ifp = if_ifwithnet(from);
				supply(from, 0, ifp);
				return;
			}
			/*
			 * request for specific nets
			 */
			rt = rtlookup(xns_nettosa(n->rip_dst));
			if (ftrace) {
				fprintf(ftrace,
					"specific request for %s",
					xns_nettoa(n->rip_dst));
				fprintf(ftrace,
					" yields route %x\n",
					rt);
			}
			n->rip_metric = htons( rt == 0 ? HOPCNT_INFINITY :
				min(rt->rt_metric+1, HOPCNT_INFINITY));
			n++;
		        newsize += sizeof (struct netinfo);
		}
		if (newsize > 0) {
			msg->rip_cmd = htons(RIPCMD_RESPONSE);
			newsize += sizeof (u_short);
			/* should check for if with dstaddr(from) first */
			(*afp->af_output)(0, from, newsize);
			ifp = if_ifwithnet(from);
			TRACE_OUTPUT(ifp, from, newsize);
			if (ftrace) {
				fprintf(ftrace,
					"request arrived on interface %s\n",
					ifp->int_name);
			}
		}
		return;

	case RIPCMD_RESPONSE:
		/* verify message came from a router */
		if ((*afp->af_portmatch)(from) == 0)
			return;
		(*afp->af_canon)(from);
		/* are we talking to ourselves? */
		if (ifp = if_ifwithaddr(from)) {
/*		XXX  - changes to not adding route to interface
			rt = rtfind(from);
			if (rt == 0 || (rt->rt_state & RTS_INTERFACE) == 0)
				addrouteforif(ifp);
			else
				rt->rt_timer = 0;
*/
			return;
		}
		/* Update timer for interface on which the packet arrived.
		 * If from other end of a point-to-point link that isn't
		 * in the routing tables, (re-)add the route.
		 */
		if ((rt = rtfind(from)) && (rt->rt_state & RTS_INTERFACE)) {
			if(ftrace) fprintf(ftrace, "Got route\n");
			rt->rt_timer = 0;
		} else if (ifp = if_ifwithdstaddr(from)) {
			if(ftrace) fprintf(ftrace, "Got partner\n");
			addrouteforif(ifp);
		}
		for (; size > 0; size -= sizeof (struct netinfo), n++) {
			struct sockaddr *sa;
			if (size < sizeof (struct netinfo))
				break;
			if ((unsigned) ntohs(n->rip_metric) >= HOPCNT_INFINITY)
				continue;
			sa = xns_nettosa(n->rip_dst);
			/* check the hardware interface whether the route is
				already there */
			if (if_ifwithnet (sa)) continue;
			rt = rtfind(sa);
			if (rt == 0) {
				rtadd(sa, from, ntohs(n->rip_metric), 0);
				continue;
			}

			/*
			 * Update if from gateway and different,
			 * from anywhere and shorter, or getting stale and equivalent.
			 */
			if ((equal(from, &rt->rt_router) &&
			    ntohs(n->rip_metric) != rt->rt_metric ) ||
			    (unsigned) ntohs(n->rip_metric) < rt->rt_metric ||
			    (rt->rt_timer > (EXPIRE_TIME/2) &&
			    rt->rt_metric == ntohs(n->rip_metric))) {
				rtchange(rt, from, ntohs(n->rip_metric));
				rt->rt_timer = 0;
			}
		}
		return;
	}
}
