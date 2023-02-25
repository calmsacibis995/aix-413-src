static char sccsid[] = "@(#)65	1.7  src/tcpip/usr/sbin/gated/rip.c, tcprouting, tcpip411, GOLD410 12/6/93 17:56:21";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF5
 *		__PF6
 *		__PF7
 *		__PF8
 *		rip_init
 *		rip_var_init
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  rip.c,v 1.47.2.5 1993/08/27 22:28:29 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_UDP

#include "include.h"
#include "inet.h"
#include "targets.h"
#define	RIPCMDS
#include "rip.h"
#include "krt.h"

#define	RIP_TIMER_UPDATE	0	/* For scheduled update */
#define	RIP_TIMER_FLASH		1	/* To maintain update spacing */
#define	RIP_TIMER_MAX		2


static u_short rip_port;
flag_t rip_flags = 0;			/* Options */
flag_t rip_trace_flags = 0;		/* Trace flags */
metric_t rip_default_metric = 0;	/* Default metric to use when propogating */
pref_t rip_preference = 0;		/* Preference for RIP routes */

static target *rip_targets;		/* Target list */

#ifdef	IP_MULTICAST
static sockaddr_un *rip_addr_mc;	/* Multicast address */
static int rip_mc_count;
#endif	/* IP_MULTICAST */

int rip_n_trusted = 0;			/* Number of trusted gateways */
int rip_n_source = 0;			/* Number of source gateways */
gw_entry *rip_gw_list = 0;		/* List of RIP gateways */
adv_entry *rip_import_list = 0;		/* List of nets to import from RIP */
adv_entry *rip_export_list = 0;		/* List of sources to exports routes to RIP */
adv_entry *rip_int_policy = 0;		/* List of interface policy */

#if	defined(PROTO_SNMP) && defined(MIB_RIP)
u_int rip_global_changes;
u_int rip_global_responses;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */

static const bits rip_flag_bits[] = {
{ RIPF_ON,		"ON" },
{ RIPF_BROADCAST,	"Broadcast" },
{ RIPF_SOURCE,		"Source" },
{ RIPF_CHOOSE,		"Choose" },
{ RIPF_NOCHECK,		"NoCheck" },
{ RIPF_FLASHDUE,	"FlashDue" },
{ RIPF_NOFLASH,		"NoFlash" },
{ RIPF_RECONFIG,	"ReConfig" },
{ 0 }
} ;

static const bits rip_target_bits[] = {
    { RIPTF_POLL,	"Poll" },
    { RIPTF_V2MC,	"V2Multicast" },
    { RIPTF_V2BC,	"V2Broadcast" },
    { RIPTF_MCSET,	"MCEnabled" },
    { 0 }
} ;

static const bits rip_if_bits[] = {
    { RIP_IFPS_V2MC,	"V2Multicast" },
    { RIP_IFPS_V2BC,	"V2Broadcast" },
    { 0 }
} ;

/*
 *	Trace RIP packets
 */
static void
rip_trace __PF5(dir, int,
    		ifap, if_addr *,
		who, sockaddr_un *,
		rpmsg, register struct rip *,
		size, register size_t)
{
    int limit;
    int check_zero;
    register struct netinfo *n = (struct netinfo *) ((void_t) (rpmsg + 1));
    register const char *cmd = "Invalid";

    if (rpmsg->rip_cmd && rpmsg->rip_cmd < RIPCMD_MAX) {
	cmd = trace_state(rip_cmd_bits, rpmsg->rip_cmd);
    }

    switch (rpmsg->rip_vers) {
    case RIP_VERSION_0:
	limit = 4;
	check_zero = FALSE;
	break;

    case RIP_VERSION_1:
	limit = 4;
	check_zero = BIT_TEST(rip_flags, RIPF_NOCHECK) ? FALSE : TRUE;
	break;

    default:
	limit = 1;
	check_zero = FALSE;
    }

    if (dir) {
	/* Trace packet transmission */

	tracef("RIP %sSENT %A -> %#A ",
	       dir > 0 ? "" : "*NOT* ",
	       ifap ? ifap->ifa_addr_local : sockbuild_str("Response"),
	       who);
    } else {
	/* Trace packet reception */
	tracef("RIP RECV %#A ",
	       who);
	if (task_recv_dstaddr) {
	    /* Some systems report the destination address */
	    tracef("-> %A ",
		   task_recv_dstaddr);
	}
    }
    tracef("vers %d, cmd %s, length %d",
	   rpmsg->rip_vers,
	   cmd, size);
    if (rpmsg->rip_vers > RIP_VERSION_1) {
	tracef(" domain %d",
	       ntohs(rpmsg->rip_domain));
    } else if (check_zero) {
	if (rpmsg->rip_domain) {
	    tracef(" reserved fields not zero");
	}
    }
    switch (rpmsg->rip_cmd) {
    case RIPCMD_POLL:
    case RIPCMD_REQUEST:
    case RIPCMD_RESPONSE:
	trace(TR_RIP, 0, NULL);
	if (BIT_TEST(trace_flags, TR_UPDATE)) {
	    int n_routes = 0;

	    for (size -= (sizeof (struct rip));
		 size >= sizeof(struct netinfo);
		 n++, size -= sizeof(struct netinfo)) {
		char zero = ' ';
		int family = ntohs(n->rip_family);
		metric_t metric = ntohl(n->rip_metric);

		/* Verify that all reserved fields are zero */
		if (check_zero
		    && (n->rip_tag
			|| n->rip_dest_mask
			|| n->rip_router)) {
		    zero = '?';
		}

		switch (family) {
		case RIP_AF_INET:
		    if (rpmsg->rip_vers < RIP_VERSION_2) {
			tracef("\t%15A%c%2d",
			       sockbuild_in(0, n->rip_dest),
			       zero,
			       metric);
		    } else {
			tracef("\t%15A/%-15A router %-15A metric %2d tag %#04X",
			       sockbuild_in(0, n->rip_dest),
			       sockbuild_in(0, n->rip_dest_mask),
			       sockbuild_in(0, n->rip_router),
			       metric,
			       ntohs(n->rip_tag));
		    }
		    break;

		case RIP_AF_UNSPEC:
		    if (metric == RIP_METRIC_UNREACHABLE) {
			tracef("\trouting table request%c",
			       zero);
			break;
		    }
		    goto bogus;

		case RIP_AF_AUTH:
		    if (rpmsg->rip_vers > RIP_VERSION_1
			&& n == (struct netinfo *) ((void_t) (rpmsg + 1))) {
			struct authinfo *auth = (struct authinfo *) n;
			int auth_type = ntohs(auth->auth_type);

			switch (auth_type) {
			case RIP_AUTH_NONE:
			    tracef("\tAuthentication: None");
			    break;

			case RIP_AUTH_SIMPLE:
			    tracef("\tAuthentication: %.*s",
				   sizeof auth->auth_data,
				   (char *) auth->auth_data);
			    break;

			default:
			    tracef("\tInvalid auth type: %d",
				   auth_type);
			}
			break;
		    }
		    /* Fall through */

		default:
		bogus:
		    tracef("\tInvalid family: %d",
			   family);
		}
		if (++n_routes == limit) {
		    n_routes = 0;
		    trace(TR_RIP|TR_NOSTAMP, 0, NULL);
		}
	    }
	    if (n_routes) {
		trace(TR_RIP|TR_NOSTAMP, 0, NULL);
	    }
	    tracef("RIP %s end of packet",
		   dir ? "SENT" : "RECV");
	    if (size) {
		tracef(" %d residual bytes",
		       size);
	    }
	    trace(TR_RIP, 0, NULL);
	}
	break;

    case RIPCMD_TRACEON:
	trace(TR_RIP, 0, ", file %*s",
	      size,
	      (char *) (rpmsg + 1));
	break;

    case RIPCMD_POLLENTRY:
	trace(TR_RIP, 0, ", net %A",
	      sockbuild_in(0, n->rip_dest));
	break;

    default:
	trace(TR_RIP, 0, NULL);
	break;
    }
    trace(TR_RIP, 0, NULL);
}


#ifdef	IP_MULTICAST
/* Remove ourselves from the MC group on this interface if necessary */
static void
rip_mc_reset __PF2(tp, task *,
		   tlp, target *)
{
    int *count = (int *) &tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_data;

    if (BIT_TEST(tlp->target_flags, RIPTF_MCSET)
	&& !--(*count)) {
	(void) task_set_option(tp,
			       TASKOPTION_GROUP_DROP,
			       tlp->target_ifap,
			       rip_addr_mc);
	BIT_RESET(tlp->target_flags, RIPTF_MCSET);
	if (!--rip_mc_count) {
	    krt_multicast_delete(rip_addr_mc);
	}
    }
}


/* Add ourselves to the MC group on this interface */
static int
rip_mc_set __PF2(tp, task *,
		 tlp, target *)
{
    flag_t ifps_state = tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_state;
    int *count = (int *) &tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_data;
    int rc = FALSE;

    if (BIT_TEST(tlp->target_flags, RIPTF_MCSET)) {
	/* Indicate MC is already set */

	rc = TRUE;
    } else if (!BIT_TEST(ifps_state, RIP_IFPS_NOMC)
	       && !(*count)++) {
	/* Try to join the MC group on this interface */

	if (!rip_addr_mc) {
	    /* Initialize address constant */

	    rip_addr_mc = sockdup(sockbuild_in(0, htonl(RIP_ADDR_MC)));
	}

	if (!BIT_TEST(tlp->target_ifap->ifa_state, IFS_MULTICAST)
	    || ((task_set_option(tp,
				 TASKOPTION_GROUP_ADD,
				 tlp->target_ifap,
				 rip_addr_mc) < 0)
		&& (errno != EADDRNOTAVAIL)
		&& (errno != EADDRINUSE))) {
	    /* Indicate that this interface is not capable of MC */

	    BIT_SET(ifps_state, RIP_IFPS_NOMC);

	    if (BIT_TEST(ifps_state, RIP_IFPS_V2MC)) {
		/* If V2 was explicitly enabled, complain */

		trace(TR_ALL, LOG_WARNING,
		      "rip_mc_set: Multicast not available on %A (%s); reverting to RIP V1 compatability",
		      tlp->target_ifap->ifa_addr,
		      tlp->target_ifap->ifa_name);
	    }
	} else {
	    /* Indicate that we successfully enabled the MC address */

	    BIT_SET(tlp->target_flags, RIPTF_MCSET);
	    tlp->target_reset = rip_mc_reset;
	    if (!rip_mc_count++) {
		krt_multicast_add(rip_addr_mc);
	    }
	    rc = TRUE;
	}
    }

    return rc;
}
#endif	/* IP_MULTICAST */


static void
rip_send  __PF6(tp, task *,
		ifap, if_addr *,
		flags, flag_t,
		addr, sockaddr_un *,
		packet, void_t,
		size, size_t)
{
    u_short port = sock2port(addr);
    int rc;

    if (!port) {
	sock2port(addr) = rip_port;
    }

#ifdef	IP_MULTICAST
    if (inet_class_of(addr) == INET_CLASSC_MULTICAST) {
	static if_addr *last_ifap;

	/* Multicast sends fail if MSG_DONTROUTE is set */
	BIT_RESET(flags, MSG_DONTROUTE);
	
	if (last_ifap != ifap) {
	    (void) task_set_option(tp,
				   TASKOPTION_MULTI_IF,
				   last_ifap = ifap);
	}
    }
#endif	/* IP_MULTICAST */

    (void) task_set_option(tp,
			   TASKOPTION_TTL,
			   BIT_TEST(flags, MSG_DONTROUTE) ? 1 : MAXTTL);

    rc = task_send_packet(tp, packet, size, flags, addr);

    if (BIT_TEST(trace_flags, TR_RIP)) {
	rip_trace(rc, ifap, addr, (struct rip *) packet, size);
    }

    sock2port(addr) = port;
}


/* Send RIP updates to all targets on the list */
/*ARGSUSED*/
static int
rip_supply __PF5(tp, task *,
		 targets, target *,
		 flash_update, int,
		 reply, int,
		 terminate, int)
{
    int count = 0;
    size_t size;
    void_t packet = task_get_send_buffer(void_t);
    rt_list *rtl = rthlist_active(AF_INET);
    register rt_head *rth;
    target *tlp;

    /* Initialize some fields in all the packets */
    TARGET_LIST(tlp, targets) {
	if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
	    struct rip *ripmsg = (struct rip *) packet;

	    bzero((caddr_t) ripmsg, sizeof *ripmsg);
	    
	    ripmsg->rip_cmd = RIPCMD_RESPONSE;
	    ripmsg->rip_vers = BIT_TEST(tlp->target_flags, RIPTF_V2) ? RIP_VERSION_2 : RIP_VERSION_1;
	    ripmsg->rip_domain = htons(0);	/* XXX */

	    tlp->target_packet = (void_t) ripmsg;
	    if (0 /* XXX */) {
		struct authinfo *auth = (struct authinfo *) ((void_t) (ripmsg + 1));

		auth->auth_family = htons(RIP_AF_AUTH);
		
		tlp->target_fillp = (void_t) (auth + 1);
	    } else {
		tlp->target_fillp = (void_t) (ripmsg + 1);
	    }

	    packet = (void_t) ((byte *) packet + tlp->target_maxsize);
	}
    } TARGET_LIST_END(tlp, targets) ;

    rt_open(tp);

    RT_LIST(rth, rtl, rt_head) {
	TARGET_LIST(tlp, targets) {
	    byte tsi, new_tsi;
	    rt_entry *release_rt = (rt_entry *) 0;

	    if (!tlp->target_packet) {
		continue;
	    }

	    rttsi_get(rth, tlp->target_rtbit, (byte *) &tsi);
	    if (!tsi) {
		/* This route is not valid for this target */
		continue;
	    }
	    new_tsi = tsi;

	    if (flash_update) {
		if (!RIP_TSI_CHANGED(tsi)) {
		    /* Route has not changed, do not flash it */
		    continue;
		}
	    } else {
		/* Check for termination of holddown */
		if (RIP_TSI_HOLDDOWN(tsi) && !terminate) {
		    if (new_tsi = RIP_TSI_METRIC(tsi) - 1) {
			BIT_SET(new_tsi, RIP_TSIF_HOLDDOWN);
		    } else {
			/* Holddown is over */
			rt_entry *rt1;
		    
			RT_ALLRT(rt1, rth) {
			    if (rtbit_isset(rt1, tlp->target_rtbit)) {
				/* We were announcing this route */

				release_rt = rt1;
				break;
			    }
			} RT_ALLRT_END(rt1, rth) ;

			/* Reset the tsi, the flash code will find us a new route to announce */
			new_tsi = 0;
		    }
		}
	    }

	    if (RIP_TSI_CHANGED(tsi)) {
		/* Reset the changed field */
		BIT_RESET(new_tsi, RIP_TSIF_CHANGED);
	    }

	    if (new_tsi != tsi) {
		/* Update the tsi field */
		rttsi_set(rth, tlp->target_rtbit, (byte *) &new_tsi);
	    }

	    size = (byte *) tlp->target_fillp - (byte *) tlp->target_packet;
	    if (size > (tlp->target_maxsize - sizeof(struct netinfo))) {
		rip_send(tp,
			 tlp->target_ifap,
			 reply ? (flag_t) 0 : MSG_DONTROUTE,
			 *tlp->target_dst,
			 (void_t) tlp->target_packet,
			 size);

		count++;

		/* Reset the fill pointer for the next time */
		tlp->target_fillp = (void_t) ((struct rip *) tlp->target_packet + 1);
	    }

	    /* Put this entry in the packet */
 	    {
		register struct netinfo *n = (struct netinfo *) ((void_t) tlp->target_fillp);

		n->rip_family = htons(socktype(rth->rth_dest));
		n->rip_tag = htons(0);	/* XXX - need to pick out of per entry structure */
		n->rip_dest = sock2ip(rth->rth_dest);
		if (BIT_TEST(tlp->target_flags, RIPTF_V2)) {
		    rt_entry *rt = rth->rth_active;

		    if (!rt || !rtbit_isset(rt, tlp->target_rtbit)) {
			rt = rth->rth_holddown;
		    }

		    n->rip_dest_mask = sock2ip(rth->rth_dest_mask);

		    if (rt->rt_ifap == tlp->target_ifap) {
			n->rip_router = sock2ip(rt->rt_router);
		    } else {
			n->rip_router = sock2ip(*tlp->target_src);
		    }
		} else {
		    n->rip_dest_mask = 0;
		    n->rip_router = 0;
		}

		if (RIP_TSI_HOLDDOWN(tsi)) {
		    n->rip_metric = ntohl(RIP_METRIC_UNREACHABLE);
		} else {
		    if (terminate) {
			n->rip_metric = htonl(RIP_METRIC_SHUTDOWN);
		    } else {
			n->rip_metric = htonl(RIP_TSI_METRIC(tsi));
		    }
		}

		/* Update the fill pointer */
		tlp->target_fillp = (void_t) (n + 1);
	    }

	    /* Release this route if necessary */
	    if (release_rt && !rtbit_reset(release_rt, tlp->target_rtbit)) {
		/* No more routes for this rt_head */
		goto Released;
	    }
	} TARGET_LIST_END(tlp, targets);

    Released: ;
    } RT_LIST_END(rth, rtl, rt_head);

    RTLIST_RESET(rtl) ;
    
    /* Send any packets with data remaining in them */
    TARGET_LIST(tlp, targets) {
	if (tlp->target_packet) {
	    if ((byte *) tlp->target_fillp > (byte *) ((struct rip *) tlp->target_packet + 1)) {
		/* OK to reply to a RIPQUERY with an empty packet */
		size = (byte *) tlp->target_fillp - (byte *) tlp->target_packet;
		rip_send(tp,
			 tlp->target_ifap,
			 reply ? (flag_t) 0 : MSG_DONTROUTE,
			 *tlp->target_dst,
			 tlp->target_packet,
			 size);
		count++;
	    }
	    tlp->target_packet = tlp->target_fillp = (void_t) 0;
	}
    } TARGET_LIST_END(tlp, targets) ;

    rt_close(tp, (gw_entry *) 0, 0, NULL);

    return count;
}


/*
 *	Process a valid response
 */
static void
rip_recv_response  __PF7(tp, task *,
			 gwp, gw_entry *,
			 src_addr, sockaddr_un *,
			 ifap, if_addr *,
			 msg, struct rip *,
			 n, register struct netinfo *,
			 last, struct netinfo *)
{
    int routes = 0;
    rt_parms rtparms;
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];

    bzero((caddr_t) &rtparms, sizeof (rtparms));

    rtparms.rtp_n_gw = 1;
    rtparms.rtp_gwp = gwp;

    rt_open(tp);

    for (; n <= last; n++) {
	rt_entry *rt;

	routes++;
		
	if (ntohs(n->rip_family) != RIP_AF_INET) {
	    /* Only interested in inet routes */
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	    gwp->gw_bad_routes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	    continue;
	}

	rtparms.rtp_dest = sockbuild_in(0, n->rip_dest);
	rtparms.rtp_preference = rip_preference;

	if (ifap == if_withmyaddr(rtparms.rtp_dest)) {
	    /* Ignore route to interface */
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	    gwp->gw_bad_routes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	    continue;
	}
		
	/* Convert metric to host byte order */
	rtparms.rtp_metric = ntohl(n->rip_metric);

	/* Verify that metric is valid */
	if (!rtparms.rtp_metric || rtparms.rtp_metric > RIP_METRIC_UNREACHABLE) {
	    trace(TR_RIP, LOG_NOTICE, "rip_recv_response: bad metric (%d) for net %A from %#A",
		  rtparms.rtp_metric,
		  rtparms.rtp_dest,
		  src_addr);
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	    gwp->gw_bad_routes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	    continue;
	}
		
	/* Now add hop count to metric */
	rtparms.rtp_metric += ips->ips_metric_in;

	rtparms.rtp_state = RTS_INTERIOR;

	/* Determine the mask and router */
	switch (msg->rip_vers) {
	default:
	    /* Mask */
	    if (n->rip_dest_mask) {
		/* Mask is supplied */

		rtparms.rtp_dest_mask = inet_mask_locate(n->rip_dest_mask);

		if (!rtparms.rtp_dest_mask) {
		    trace(TR_RIP, LOG_NOTICE, "rip_recv_response: bad mask (%A) for net %A from %#A",
			  rtparms.rtp_dest_mask,
			  rtparms.rtp_dest,
			  src_addr);
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
		    gwp->gw_bad_routes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
		    continue;
		}
	    } else if (sock2ip(rtparms.rtp_dest) != INADDR_DEFAULT) {
		/* Lookup the mask the old fashioned way */

		rtparms.rtp_dest_mask = inet_mask_withif(rtparms.rtp_dest, ifap, &rtparms.rtp_state);
	    }

	    /* Router */
	    if (n->rip_router) {
		/* Router was supplied */

		rtparms.rtp_router = sockbuild_in(0, n->rip_router);
	    
		if (if_withdst(rtparms.rtp_router) != ifap) {
		    /* Supplied router is invalid */
		    trace(TR_RIP, LOG_NOTICE, "rip_recv_response: bad router (%A) for net %A from %#A",
			  rtparms.rtp_router,
			  rtparms.rtp_dest,
			  src_addr);
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
		    gwp->gw_bad_routes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
		    continue;
		}

		/* Router is OK */
		break;
	    } else {
		/* Router is source address */

		rtparms.rtp_router = gwp->gw_addr;
	    }
	    break;
	    
	case RIP_VERSION_0:
	case RIP_VERSION_1:
	    /* Derive mask and router the old fashioned way */
	    rtparms.rtp_dest_mask = inet_mask_withif(rtparms.rtp_dest, ifap, &rtparms.rtp_state);
	    rtparms.rtp_router = gwp->gw_addr;
	}

	rtparms.rtp_tag = ntohs(n->rip_tag);
		    
	rt = rt_locate(rtparms.rtp_state,
		       rtparms.rtp_dest,
		       rtparms.rtp_dest_mask,
		       RTPROTO_BIT(tp->task_rtproto));
	if (!rt) {
	    rt_head *rth;
	    
	    /* No route installed.  See if we are announcing another route */
	    rt = rt_locate(RTS_NETROUTE,
			   rtparms.rtp_dest,
			   rtparms.rtp_dest_mask,
			   RTPROTO_BIT_ANY);
	    if (rt &&
		(rth = rt->rt_head) &&
		rth->rth_n_announce &&
		(rt == rth->rth_active || rt == rth->rth_holddown)) {
		/* We are announcing this route */
		register target *tlp;

		/* RIP won't announce an active route if one in holddown */
		/* so check the holddown route first */
		rt = rth->rth_holddown;
		if (!rt) {
		    rt = rth->rth_active;
		}

		TARGET_LIST(tlp, rip_targets) {
		    if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY) &&
			rtbit_isset(rt, tlp->target_rtbit)) {
			/* We are sending to this target */
			byte tsi;

			rttsi_get(rth, tlp->target_rtbit, &tsi);

			if (RIP_TSI_HOLDDOWN(tsi) ||
			    RIP_TSI_METRIC(tsi) < rtparms.rtp_metric) {
			    /* We are announcing this route from another protocol */

			    break;
			}
		    }
		} TARGET_LIST_END(tlp, rip_targets) ;

		if (tlp) {
		    /* Announced via another protocol, ignore this route */
		    continue;
		}
	    }
	    
	    /* New route */
	    if (rtparms.rtp_metric < RIP_METRIC_UNREACHABLE &&
		import(rtparms.rtp_dest,
		       rip_import_list,
		       ips->ips_import,
		       rtparms.rtp_gwp->gw_import,
		       &rtparms.rtp_preference,
		       ifap,
		       (void_t) 0)) {
		/* Add new route */

		rt = rt_add(&rtparms);
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
		rip_global_changes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	    } else {
		BIT_SET(rtparms.rtp_gwp->gw_flags, GWF_IMPORT);
	    }
	} else if (rt->rt_gwp == gwp) {
	    /* same route */
	    if (rtparms.rtp_metric > RIP_METRIC_UNREACHABLE) {
		if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		    rt_delete(rt);
		}
	    } else if (BIT_TEST(rt->rt_state, RTS_DELETE)
		       || rtparms.rtp_metric != rt->rt_metric
		       || rtparms.rtp_router != rt->rt_router
		       || rtparms.rtp_tag != rt->rt_tag) {
		(void) rt_change(rt,
				 rtparms.rtp_metric,
				 rtparms.rtp_tag,
				 rt->rt_preference,
				 1, &rtparms.rtp_router);
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
		rip_global_changes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	    } else {
		rt_refresh(rt);
	    }
	} else if ((rtparms.rtp_metric < rt->rt_metric
		    || (rt->rt_timer > (rt->rt_gwp->gw_timer_max / 2)
			&& rt->rt_metric == rtparms.rtp_metric
			&& !BIT_TEST(rt->rt_state, RTS_REFRESH)))
		   && import(rtparms.rtp_dest,
			     rip_import_list,
			     ips->ips_import,
			     rtparms.rtp_gwp->gw_import,
			     &rtparms.rtp_preference,
			     ifap,
			     (void_t) 0)) {
	    /* Better metric or same metric and old route has */
	    /* not been refreshed. */

	    (void) rt_change(rt,
			     rtparms.rtp_metric,
			     rtparms.rtp_tag,
			     rt->rt_preference,
			     1, &rtparms.rtp_router);
	    rt->rt_gwp = gwp;
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	    rip_global_changes++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	}
    }				/*  for each route */

    rt_close(tp, rtparms.rtp_gwp, routes, NULL);
}


/*
 *	Process a valid request
 */
static void
rip_recv_request __PF8(tp, task *,
		       src_addr, sockaddr_un *,
		       ifap, if_addr *,
		       tlp, target *,
		       msg, struct rip *,
		       n, register struct netinfo *,
		       last, struct netinfo *,
		       poll, int)
{
    struct netinfo *first = n;
    rt_entry *rt;
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];

    msg->rip_cmd = RIPCMD_RESPONSE;

    if (n == last
	&& n->rip_family == ntohs(RIP_AF_UNSPEC)
	&& n->rip_metric == ntohl(RIP_METRIC_UNREACHABLE)) {
	/* A routing table request */

	if (poll) {
	    /* Dump all the rip routes */

	    int count = 0;
	    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	    /* We can fill the whole packet */
	    last = (struct netinfo *) ((void_t) ((caddr_t) msg + RIP_MAXSIZE(ifap) - 1));

	    RT_LIST(rt, rtl, rt_entry) {
		if (n > last) {
		    rip_send(tp,
			     (if_addr *) 0,
			     0,
			     src_addr,
			     (void_t) msg,
			     (size_t) ((caddr_t) n - (caddr_t) msg));
		    n = first;
		    count++;
		}

		n->rip_family = htons(socktype(rt->rt_dest));
		n->rip_tag = htons(0);	/* XXX - need to pick out of per entry structure */
		n->rip_dest = sock2ip(rt->rt_dest);
		if (msg->rip_vers > RIP_VERSION_1) {
		    n->rip_dest_mask = sock2ip(rt->rt_dest_mask ? rt->rt_dest_mask : inet_mask_host);
		    if (rt->rt_ifap == ifap) {
			n->rip_router = sock2ip(rt->rt_router);
		    } else {
			n->rip_router = sock2ip(ifap->ifa_addr_local);
		    }
		} else {
		    n->rip_dest_mask = 0;
		    n->rip_router = 0;
		}
		n->rip_metric = ntohl(BIT_TEST(rt->rt_state, RTS_HOLDDOWN) ? RIP_METRIC_UNREACHABLE : rt->rt_metric);

		n++;
	    } RT_LIST_END(rt, rtl, rt_entry) ;

	    RTLIST_RESET(rtl);

	    if (!count || n > first) {
		/* Send an empty packet or remaining data */
		rip_send(tp,
			 (if_addr *) 0,
			 0,
			 src_addr,
			 (void_t) msg,
			 (size_t) ((caddr_t) n - (caddr_t) msg));
	    }
	} else {
	    /* Request - dump full routing table */

	    
	    if (tlp) {
		target stp;

		/* Fix some target fields */
		stp = *tlp;	/* struct copy */
		stp.target_dst = &src_addr;
		stp.target_next = (target *) 0;
		if (msg->rip_vers < RIP_VERSION_2) {
		    /* Do not respond to version 1 requests with version 2 packets */

		    BIT_RESET(stp.target_flags, RIPTF_V2);
		} else if (sock2port(task_recv_srcaddr) != rip_port
			   && !BIT_TEST(stp.target_flags, RIPTF_V2)) {
		    /* Answer V2 queries with V2 information */

		    BIT_SET(stp.target_flags, RIPTF_V2BC);
		}

		(void) rip_supply(tp, &stp, FALSE, TRUE, FALSE);
	    } else {
		/* If nothing to send him, provide an empty packet */
		rip_send(tp,
			 (if_addr *) 0,
			 0,
			 src_addr,
			 (void_t) msg,
			 (size_t) ((caddr_t) n - (caddr_t) msg));
	    }
	}
    } else {

	/* Specific request */

	while (n <= last) {
	    GNTOHS(n->rip_family);
	    GNTOHL(n->rip_metric);

	    n->rip_metric = RIP_METRIC_UNREACHABLE;
	    
	    if (n->rip_family == RIP_AF_INET) {
		sockaddr_un *addr = sockbuild_in(0, n->rip_dest);
		sockaddr_un *mask = inet_mask_withif(addr, ifap, (flag_t *) 0);
		flag_t table = RTS_INTERIOR;
		    
		rt = rt_locate(table, addr, mask, poll ? RTPROTO_BIT(tp->task_rtproto) : RTPROTO_BIT_ANY);
		if (rt && !BIT_TEST(rt->rt_state, RTS_DELETE|RTS_HIDDEN)) {
		    n->rip_metric = rt->rt_metric;
		    n->rip_metric += ips->ips_metric_out;
		    if (n->rip_metric > RIP_METRIC_UNREACHABLE) {
			n->rip_metric = RIP_METRIC_UNREACHABLE;
		    }
		}
	    }
	    GHTONL(n->rip_metric);
	    n++;
	}

	rip_send(tp,
		 (if_addr *) 0,
		 0,
		 src_addr,
		 (void_t) msg,
		 (size_t) ((caddr_t) n - (caddr_t) msg));
    }

#if	defined(PROTO_SNMP) && defined(MIB_RIP)
    rip_global_responses++;
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
}


/*
 *	Process a valid poll-entry packet
 */
static void
rip_recv_pollentry __PF4(tp, task *,
			 src_addr, sockaddr_un *,
			 ifap, if_addr *,
			 msg, struct rip *)
{
    rt_entry *rt;
    struct netinfo *n = (struct netinfo *) ((void_t) (msg + 1));

    if (n->rip_family == RIP_AF_INET) {
	sockaddr_un *addr = sockbuild_in(0, n->rip_dest);
	sockaddr_un *mask = inet_mask_withif(addr, ifap, (flag_t *) 0);
	flag_t table = RTS_INTERIOR;
		    
	rt = rt_locate(table, addr, mask, RTPROTO_BIT_ANY);
	if (BIT_TEST(rt->rt_state, RTS_DELETE)) {
	    rt = (rt_entry *) 0;
	}
    } else {
	rt = (rt_entry *) 0;
    }

    if (rt) {
	/* don't bother to check rip_vers */

	struct entryinfo *e = (struct entryinfo *) n;

	bzero((caddr_t) &e->rtu_dst, sizeof (e->rtu_dst));
	e->rtu_dst.rip_family = htons(socktype(rt->rt_dest));
	e->rtu_dst.rip_addr = sock2ip(rt->rt_dest);		/* struct copy */
	bzero((caddr_t) &e->rtu_router, sizeof (e->rtu_router));
	e->rtu_router.rip_family = htons(socktype(rt->rt_router));
	e->rtu_router.rip_addr = sock2ip(rt->rt_router);	/* struct copy */
	e->rtu_flags = htons((unsigned short) krt_state_to_flags(rt->rt_state));
	e->rtu_state = htons((unsigned short) rt->rt_state);
	e->rtu_timer = htonl((unsigned long) rt->rt_timer);
	e->rtu_metric = htonl(rt->rt_metric);
	if (rt->rt_ifap) {
	    e->rtu_int_flags = htonl((unsigned long) rt->rt_ifap->ifa_state);
	    (void) strncpy(e->rtu_int_name, rt->rt_ifap->ifa_name, IFNAMSIZ);
	} else {
	    e->rtu_int_flags = 0;
	    (void) strncpy(e->rtu_int_name, "(none)", IFNAMSIZ);
	}
    } else {
	bzero((char *) n, sizeof (struct entryinfo));
    }

    rip_send(tp,
	     (if_addr *) 0,
	     0,
	     src_addr,
	     (void_t) msg,
	     sizeof (struct rip) + sizeof (struct entryinfo));
}


/*
 * 	Check out a newly received RIP packet.
 */
static void
rip_recv __PF1(tp, task *)
{
    size_t size;
    int n_packets = TASK_PACKET_LIMIT;

#define	REJECT(p, m)	{ reject_msg = (m); pri = (p); goto Reject; }

    while (n_packets-- && !task_receive_packet(tp, &size)) {
	register if_addr *ifap = (if_addr *) 0;
	gw_entry *gwp = (gw_entry *) 0;
	register int OK = TRUE;
	struct rip *inripmsg = task_get_recv_buffer(struct rip *);
	int pri = 0;
	const char *reject_msg = (char *) 0;
	int check_zero = FALSE;
	int poll = FALSE;
	struct netinfo *nets = (struct netinfo *) ((void_t) (inripmsg + 1));
	struct netinfo *last = (struct netinfo *) ((void_t) ((caddr_t) inripmsg + size)) - 1;

	if (socktype(task_recv_srcaddr) != RIP_AF_INET) {
	    REJECT(0, "protocol not INET");
	}

	/* Locate or create a gateway structure for this gateway */
	gwp = gw_timestamp(&rip_gw_list,
			   tp->task_rtproto,
			   tp,
			   (as_t) 0,
			   (as_t) 0,
			   RIP_T_EXPIRE,
			   task_recv_srcaddr,
			   GWF_NEEDHOLD);

	switch (inripmsg->rip_vers) {
	case RIP_VERSION_0:
	    REJECT(LOG_NOTICE, "ignoring version 0 packets");

	case RIP_VERSION_1:
	    check_zero = BIT_TEST(rip_flags, RIPF_NOCHECK) ? FALSE : TRUE;
	    break;

	default:
	    check_zero = FALSE;
	}

	/* If we have a list of trusted gateways, verify that this gateway is trusted */
	if (rip_n_trusted && !BIT_TEST(gwp->gw_flags, GWF_TRUSTED)) {
	    OK = FALSE;
	}
	if (BIT_TEST(trace_flags, TR_RIP)) {
	    rip_trace(0, ifap, gwp->gw_addr, inripmsg, size);
	}

	if (check_zero) {
	    /* Verify that all reserved fields are zero */
	    caddr_t limit = (caddr_t) inripmsg + size;
	    register struct netinfo *n;

	    if (inripmsg->rip_domain) {
		goto not_zero;
	    }

	    switch (inripmsg->rip_cmd) {
	    case RIPCMD_REQUEST:
	    case RIPCMD_RESPONSE:
		/* Check the fields in the entries */

		for (n = (struct netinfo *) ((void_t) (inripmsg + 1));
		     (caddr_t) n < limit;
		     n++) {
		    if (n->rip_tag ||
			n->rip_dest_mask ||
			n->rip_router) {
		not_zero:
			pri = BIT_TEST(gwp->gw_flags, GWF_FORMAT) ? 0 : LOG_WARNING;
			BIT_SET(gwp->gw_flags, GWF_FORMAT);
			REJECT(pri, "reserved field not zero");
		    }
		}
		break;
	    }
	}

	/* Verify authentication */
	if (inripmsg->rip_vers > RIP_VERSION_1) {

	    if (htons(inripmsg->rip_domain)) {
		REJECT(0, "unknown domain");
	    }

	    switch (inripmsg->rip_cmd) {
	    case RIPCMD_REQUEST:
	    case RIPCMD_POLL:
	    case RIPCMD_RESPONSE:
		if (inripmsg->rip_vers > RIP_VERSION_1) {
		    struct authinfo *auth = (struct authinfo *) nets;
		    int auth_type;
		    byte *auth_data;

		    if (nets <= last &&
			ntohs(auth->auth_family) == RIP_AF_AUTH) {
			/* Verify authentication */

			auth_type = auth->auth_type;
			auth_data = auth->auth_data;
			nets++;
		    } else {
			auth_type = 0;
			auth_data = (byte *) 0;
		    }

		    if (auth_type != RIP_AUTH_NONE /* XXX */) {
			pri = BIT_TEST(gwp->gw_flags, GWF_AUTHFAIL) ? 0 : LOG_WARNING;
			BIT_SET(gwp->gw_flags, GWF_AUTHFAIL);
			REJECT(pri, "authentication failure");
		    }
		}

		break;
	    }
	}

	/* Process packet */
	switch (inripmsg->rip_cmd) {
	case RIPCMD_POLL:
	    poll = TRUE;
	    /* Fall through */

	case RIPCMD_REQUEST:
	    {
		target *tlp = (target *) 0;

		if (poll || sock2port(task_recv_srcaddr) != rip_port) {
		    /* We should answer this request */

		    /* BSD 4.3 Reno has a bug causing the address to be zero */
		    if (task_recv_dstaddr
			&& sock2ip(task_recv_dstaddr)
#ifdef	IP_MULTICAST
			&& !sockaddrcmp(task_recv_dstaddr, rip_addr_mc)
#endif	/* IP_MULTICAST */
			) {
			/* On some systems we can find the destination address of the packet */
			ifap = if_withlcladdr(task_recv_dstaddr, TRUE);
			if (!ifap) {
			    REJECT(0, "can not find interface for source address");
			}
		    } else {
			ifap = if_withdst(gwp->gw_addr);
			if (!ifap) {
			    /* This host does not share a network */
			    rt_entry *rt;
			    
			    rt = rt_lookup(RTS_NETROUTE,
					   RTS_HIDDEN|RTS_DELETE,
					   gwp->gw_addr,
					   RTPROTO_BIT_ANY);

			    if (!rt) {
				REJECT(0, "can not find interface for route");
			    }
			    ifap = rt->rt_ifap;
			}
		    }
		}

		if (!poll) {
		    target *tlp2;

		    if (sock2port(task_recv_srcaddr) == rip_port) {
			/* This is a request from a router, make sure we should */
			/* and are allowed to reply */

			if (if_withlcladdr(gwp->gw_addr, FALSE)) {
			    /* Ignore my own requests */
			    continue;
			}
			if (!OK) {
			    REJECT(0, "not on trustedgateways list");
			}
			if (!(ifap = if_withdst(gwp->gw_addr))) {
			    REJECT(0, "not on attached network");
			}
			if (BIT_TEST(ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN|IFPS_NOOUT)) {
			    REJECT(0, "interface marked for no RIP in/out");
			}
		    }

		    /* Find the Target for this host */
		    TARGET_LIST(tlp2, rip_targets) {
			/* Look for target for this interface */

			if (BIT_TEST(tlp2->target_flags, TARGETF_SUPPLY)) {
			    if (tlp2->target_gwp == gwp) {
				/* Found a target for this gateway! */

				tlp = tlp2;
				break;
			    } else if (tlp2->target_ifap == ifap) {
				/* Found a target for this interface */
				/* remember it, but keep looking in case */
				/* there is one for the gateway */

				tlp = tlp2;
			    }
			}
		    } TARGET_LIST_END(tlp2, rip_targets) ;

		    if (sock2port(task_recv_srcaddr) == rip_port) {
			/* More sanity check on a request from a router */

			if (!tlp) {
			    REJECT(0, "not supplying RIP");
			}

#ifdef	IP_MULTICAST
			/* If we are configured to send version 2 multicast packet */
			/* to this target, do not reply to queries from sources */
			/* that would not normally hear our multicasts */

			/* Caveat: */
			/* Systems earlier than BSD 4.3 Reno (and Reno itself without */
			/* a bug fix) do not report the destination address of UDP */
			/* packets.  In this case it is not possible to determine if */
			/* a V2 requests was multicast, so to be safe we will not */
			/* even reply to version 2 requests unless we can determine */
			/* that it was multicast */

			if (BIT_TEST(tlp->target_flags, RIPTF_V2MC)
			    && (inripmsg->rip_vers < RIP_VERSION_2
				|| (!task_recv_dstaddr
				    || !sock2ip(task_recv_dstaddr)
				    || !sockaddrcmp(task_recv_dstaddr, rip_addr_mc)))) {
			    REJECT(0, "not supplying broadcast RIP");
			}
#endif	/* IP_MULTICAST */
		    }
		}

		BIT_SET(gwp->gw_flags, GWF_QUERY | GWF_ACCEPT);

		rip_recv_request(tp, task_recv_srcaddr, ifap, tlp, inripmsg, nets, last, poll);
	    }
	    continue;

	case RIPCMD_TRACEON:
	case RIPCMD_TRACEOFF:
	    if (!OK) {
		REJECT(0, "not on trustedgateways list");
	    }
	    if (ntohs(sock2port(task_recv_srcaddr)) > IPPORT_RESERVED) {
		REJECT(0, "not from a trusted port");
	    }
	    if (!(ifap = if_withdst(gwp->gw_addr))) {
		REJECT(0, "not on same net");
	    }
	    if (BIT_TEST(ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN)) {
		REJECT(0, "not listening to RIP on this interface");
	    }
	    REJECT(LOG_NOTICE, "TRACE packets not supported");

	case RIPCMD_POLLENTRY:
	    BIT_SET(gwp->gw_flags, GWF_QUERY | GWF_ACCEPT);
	    rip_recv_pollentry(tp, task_recv_srcaddr, ifap, inripmsg);
	    continue;

	case RIPCMD_RESPONSE:
	    /* If this packet is addressed from us, flag the interface as up and ignore the packet */
	    ifap = if_withlcladdr(gwp->gw_addr, FALSE);
	    if (ifap) {
		if_rtupdate(ifap);
		continue;
	    }

	    if (!OK) {
#ifndef	notdef
		REJECT(0, "not on trustedgateways list");
#else	/* notdef */
		continue;
#endif	/* notdef */
	    }
	    if (sock2port(task_recv_srcaddr) != rip_port) {
		REJECT(0, "not from a trusted port");
	    }
	    if (!(ifap = if_withdst(gwp->gw_addr))) {
		REJECT(0, "not on same net");
	    }

	    /* update interface timer on interface that packet came in on. */
	    if_rtupdate(ifap);

	    if (BIT_TEST(ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN)) {
		REJECT(0, "interface marked for no RIP in");
	    }
	    BIT_SET(gwp->gw_flags, GWF_ACCEPT);

	    rip_recv_response(tp,
			      gwp,
			      task_recv_srcaddr,
			      ifap,
			      inripmsg,
			      nets,
			      last);
	    break;

	default:
	    pri = BIT_TEST(gwp->gw_flags, GWF_FORMAT) ? 0 : LOG_WARNING;
	    BIT_SET(gwp->gw_flags, GWF_FORMAT);
	    REJECT(0, "invalid or not implemented command");
	}
	continue;

    Reject:
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	if (gwp) {
	    gwp->gw_bad_packets++;
	}
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	tracef("rip_recv: ignoring RIP ");
	if (inripmsg->rip_cmd < RIPCMD_MAX) {
	    tracef("%s",
		   trace_state(rip_cmd_bits, inripmsg->rip_cmd));
	} else {
	    tracef("#%d",
		   inripmsg->rip_cmd);
	}
	trace(TR_RIP, pri, " packet from %#A - %s",
	      task_recv_srcaddr,
	      reject_msg);
	trace(TR_RIP, 0, NULL);
	if (gwp) {
	    BIT_SET(gwp->gw_flags, GWF_REJECT);
	}
    }
}


static void
rip_exit __PF1(tp, task *)
{

    /* Release the target list, bit assignments, and buffers */
    rip_targets = target_free(tp, rip_targets);

    {
	rt_entry *rt = (rt_entry *) 0;
	rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	rt_open(tp);

	RT_LIST(rt, rtl, rt_entry) {
	    if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		rt_delete(rt);
	    }
	} RT_LIST_END(rt, rtl, rt_entry) ;

	RTLIST_RESET(rtl);

	rt_close(tp, (gw_entry *) 0, 0, NULL);
    }

    task_delete(tp);
}


/*
 *	Evaluate policy for changed routes.
 */
static int
rip_policy __PF3(tp, task *,
		 tlp, target *,
		 change_list, rt_list *)
{
    if_addr *ifap = tlp->target_ifap;
    int changes = 0;
    rt_head *rth;
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];

    RT_LIST(rth, change_list, rt_head) {
	register rt_entry *new_rt = rth->rth_active;
	register rt_entry *old_rt;
	adv_results result;

	/* Were we announcing an old route? */
	if (rth->rth_last_active
	    && rtbit_isset(rth->rth_last_active, tlp->target_rtbit)) {
	    /* We were announcing the last active route */
	    
	    old_rt = rth->rth_last_active;
	} else if (rth->rth_holddown
		   && rtbit_isset(rth->rth_holddown, tlp->target_rtbit)) {
	    /* We were doing a holddown on a route */

	    old_rt = rth->rth_holddown;

	} else if (rth->rth_n_announce) {
	    /* If we have a route in holddown, the above might not catch it */
 
	    RT_ALLRT(old_rt, rth) {
		if (rtbit_isset(old_rt, tlp->target_rtbit)) {
		    break;
		}
	    } RT_ALLRT_END(old_rt, rth) ;
	} else {
	    /* No old route */

	    old_rt = (rt_entry *) 0;
	}

	/* Can we announce the new route (if there is one)? */
	if (new_rt) {

	    if (socktype(new_rt->rt_dest) != AF_INET) {
		goto no_export;
	    }
	    
	    if (BIT_TEST(new_rt->rt_state, RTS_NOADVISE|RTS_GROUP)) {
		/* Absolutely not */

		goto no_export;
	    }

	    if (new_rt->rt_ifap == ifap &&
		new_rt->rt_gwp->gw_proto == RTPROTO_DIRECT) {
		/* Do not send interface routes back to the same interface */

		goto no_export;
	    }

	    if (new_rt->rt_ifap == ifap &&
		sockaddrcmp(new_rt->rt_router, *tlp->target_dst)) {
		/* Sending a route back to the router you are using could */
		/* cause a routing loop */

		goto no_export;
	    }
	    
	    /* Subnets and host routes do not go everywhere */
	    if (new_rt->rt_dest_mask == inet_mask_host) {
		if (new_rt->rt_gwp->gw_proto != RTPROTO_DIRECT &&
		    ((sock2ip(ifap->ifa_net) ^ sock2ip(rth->rth_dest)) & sock2ip(ifap->ifa_netmask)) &&
		    !((sock2ip(new_rt->rt_ifap->ifa_net) ^ sock2ip(rth->rth_dest)) & sock2ip(new_rt->rt_ifap->ifa_netmask))) {
		    /* Host is being sent to another network and we learned it via it's home network */

		    /* XXX - This assumes we are announcing the network route */
		    goto no_export;
		}
	    } else if (new_rt->rt_dest_mask != inet_mask_natural(new_rt->rt_dest)) {
		if ((sock2ip(rth->rth_dest) & sock2ip(ifap->ifa_netmask)) != sock2ip(ifap->ifa_net)) {
		    /* Only send subnets to interfaces of the same network */

		    /* XXX - v2? */
		    goto no_export;
		}
#ifdef	ROUTE_AGGREGATION
		if (ntohl(sock2ip(rth->rth_dest_mask)) > ntohl(sock2ip(inet_mask_natural(rth->rth_dest)))) {
		    /* Do not sent aggregate routes to RIP 1 */

		    break;
		}
#endif	/* ROUTE_AGGREGATION */
		if (!BIT_TEST(tlp->target_flags, RIPTF_V2MC)
		    && rth->rth_dest_mask != ifap->ifa_subnetmask) {
		    /* If not full v2 only send subnets that have the same mask */

		    goto no_export;
		}
	    } else {
		if (sockaddrcmp_in(rth->rth_dest, ifap->ifa_net)) {
		    /* Do not send the whole net to a subnet */

		    goto no_export;
		}
	    }

	    if ((new_rt->rt_gwp->gw_proto == tp->task_rtproto) && (ifap == new_rt->rt_ifap)) {
		/* Split horizon */
		goto no_export;
	    }

	    if (new_rt->rt_ifap
		&& !BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_UP)) {
		/* The interface is down */
		goto no_export;
	    }

	    /* Assign default metric */
	    if (!new_rt->rt_ifap
		|| BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_LOOPBACK)) {
		/* Routes via the loopback interface must have an explicit metric */ 

		result.res_metric = RIP_METRIC_UNREACHABLE;
	    } else if (new_rt->rt_gwp->gw_proto == RTPROTO_DIRECT) {
		/* Interface routes */

		if (BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_POINTOPOINT)) {
		    /* Add a hop for the P2P link */

		    result.res_metric = RIP_HOP * 2;
		} else {
		    /* Default to one hop */

		    result.res_metric = RIP_HOP;
		}
	    } else {
		/* Use configured default metric */

		result.res_metric = rip_default_metric;
	    }

		
	    if (!export(new_rt,
			tp->task_rtproto,
			rip_export_list,
			ips->ips_export,
			tlp->target_gwp ? tlp->target_gwp->gw_export : (adv_entry *) 0,
			&result)) {
		/* Policy prohibits announcement */

		goto no_export;
	    } else {
		/* Add the interface metric */
		    
		result.res_metric += ips->ips_metric_out;
	    }

	    if (result.res_metric >= RIP_METRIC_UNREACHABLE) {
	    no_export:
		new_rt = (rt_entry *) 0;
	    } else {
		/* Finally, we figured out that we can export it! */

		if (new_rt != old_rt) {
		    /* Not just a metric change, set new announce bit */
		    rtbit_set(new_rt, tlp->target_rtbit);
		    if (old_rt) {
			/* Reset announce bit on old route */
			(void) rtbit_reset(old_rt, tlp->target_rtbit);
		    }
		}
	    }
	}

	/* We've figured out what needs to be changed, now update the TSI */
	if (new_rt) {
	    /* New route or changed metric */
	    byte tsi;

	    rttsi_get(rth, tlp->target_rtbit, &tsi);

	    if (RIP_TSI_HOLDDOWN(tsi) ||
		RIP_TSI_METRIC(tsi) != result.res_metric) {
		/* Only flash if metric has changed */
		
		tsi = RIP_TSIF_CHANGED | result.res_metric;
		rttsi_set(rth, tlp->target_rtbit, &tsi);
		changes++;
	    }
	} else if (old_rt) {
	    /* No new route, just an old one */
	    byte tsi;

	    /* Look at how we were announcing it */
	    rttsi_get(rth, tlp->target_rtbit, &tsi);

	    if (tsi && !RIP_TSI_HOLDDOWN(tsi)) {
		/* Route has been announce and is not already in holddown */
		
		tsi = RIP_TSIF_CHANGED | RIP_TSIF_HOLDDOWN | RIP_TSI_HOLDCOUNT;
		rttsi_set(rth, tlp->target_rtbit, &tsi);
		changes++;
	    }
	}
    } RT_LIST_END(rth, change_list, rt_head) ;

    return changes;
}


/*
 *	send RIP packets
 */
/*ARGSUSED*/
static void
rip_job __PF2(tip, timer *,
	      interval, time_t)
{
    task *tp = tip->timer_task;
    
    (void) rip_supply(tp, rip_targets, FALSE, FALSE, FALSE);

    /* Indicate that flash updates are possible as soon as the timer fires */
    BIT_RESET(rip_flags, RIPF_NOFLASH|RIPF_FLASHDUE);
    timer_set(TASK_TIMER(tp, RIP_TIMER_FLASH), RIP_T_FLASH, (time_t) 0);
}


/*
 *	send a flash update packet
 */
/*ARGSUSED*/
static void
rip_do_flash __PF2(tip, timer *,
		   interval, time_t)
{
    static int term_updates = 4;
    int terminate = BIT_TEST(task_state, TASKS_TERMINATE);
    task *tp = tip->timer_task;

    if (BIT_TEST(rip_flags, RIPF_FLASHDUE) || terminate) {
	int count;

	trace(TR_TASK, 0, "rip_do_flash: Doing flash update for RIP");
	count = rip_supply(tp, rip_targets, !terminate, FALSE, terminate);
	trace(TR_TASK, 0, "rip_do_flash: Flash update done");

	if (terminate && (!count || !--term_updates)) {
	    /* Sent the requisite number of updates or nothing to send */
	    rip_exit(tp);
	    return;
	}

	/* Indicate no flash update is due */
	BIT_RESET(rip_flags, RIPF_FLASHDUE);

	/* Schedule the next flash update */
	if (terminate ||
	    time_sec + RIP_T_MIN + RIP_T_MAX < TASK_TIMER(tp, RIP_TIMER_UPDATE)->timer_next_time) {
	    /* We can squeeze another flash update in before the full update */

	    timer_set(tip, RIP_T_FLASH, (time_t) 0);
	} else {
	    /* The next flash update will be scheduled after the next full update */

	    timer_reset(tip);
	    BIT_SET(rip_flags, RIPF_NOFLASH);
	}
    } else {
	timer_reset(tip);
    }
}


/*
 *	Do or schedule a flash update
 */
static void
rip_need_flash __PF1(tp, task *)
{
    timer *tip = TASK_TIMER(tp, RIP_TIMER_FLASH);

    if (!tip) {
	BIT_RESET(rip_flags, RIPF_FLASHDUE);
	return;
    }
    
    /* Indicate we need a flash update */
    BIT_SET(rip_flags, RIPF_FLASHDUE);

    /* And see if we can do it now */
    if (!tip->timer_next_time
	&& !BIT_TEST(rip_flags, RIPF_NOFLASH)) {
	/* Do it now */

	rip_do_flash(tip, (time_t) 0);
    }
}


/*
 *	Process changes in the routing table.
 */
static void
rip_flash __PF2(tp, task *,
		change_list, rt_list *)
{
    int changes = 0;
    target *tlp;
    
    /* Re-evaluate policy */
    rt_open(tp);

    TARGET_LIST(tlp, rip_targets) {
	if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
	    changes += rip_policy(tp, tlp, change_list);
	}
    } TARGET_LIST_END(tlp, rip_targets) ;
    
    /* Close the table */
    rt_close(tp, (gw_entry *) 0, 0, NULL);

    if (changes) {
	/* Schedule a flash update */

	rip_need_flash(tp);
    }
}


/*
 *	Re-evaluate routing table
 */
static void
rip_newpolicy __PF2(tp, task *,
		    change_list, rt_list *)
{
    /* Indicate reconfig done */
    BIT_RESET(rip_flags, RIPF_RECONFIG);

    /* And evaluate policy */
    rip_flash(tp, change_list);
}


/*
 *	Initialize static variables
 */
void
rip_var_init()
{
    rip_flags = RIPF_ON|RIPF_CHOOSE;

    rip_default_metric = RIP_METRIC_UNREACHABLE;
    rip_preference = RTPREF_RIP;

    /* Set up interface bits to be printed */
    int_ps_bits[RTPROTO_RIP] = rip_if_bits;
}


/*
 *	Cleanup before re-init
 */
/*ARGSUSED*/
static void
rip_cleanup __PF1(tp, task *)
{
    adv_cleanup(RTPROTO_RIP,
		&rip_n_trusted,
		&rip_n_source,
		rip_gw_list,
		&rip_int_policy,
		&rip_import_list,
		&rip_export_list);
}


static void
rip_tsi_dump __PF3(rth, rt_head *,
		   data, void_t,
		   buf, char *)
{
    target *tlp = (target *) data;
    byte tsi;

    rttsi_get(rth, tlp->target_rtbit, (byte *) &tsi);

    if (tsi) {
	const char *changed = RIP_TSI_CHANGED(tsi) ? "changed " : "";
	const char *holddown = RIP_TSI_HOLDDOWN(tsi) ? "holddown " : "";
    
	(void) sprintf(buf, "RIP %-15A %s%s %d",
		       *tlp->target_dst,
		       changed,
		       holddown,
		       RIP_TSI_METRIC(tsi) * (*holddown ? RIP_T_UPDATE : 1));
    }
    
    return;
}


/*
 *	Update the target list
 */
static void
rip_target_list __PF2(tp, task *,
		      ifap, if_addr *)
{
    int targets;
    target *tlp;
    flag_t target_flags = TARGETF_ALLINTF;
    static int n_targets, n_source;

    /* If broadcast/nobroadcast not specified, figure out if we */
    /* need to broadcast packets */
    if (BIT_TEST(rip_flags, RIPF_CHOOSE)) {
	if (if_n_addr[AF_INET].up > 1
	    && inet_ipforwarding) {

	    BIT_SET(rip_flags, RIPF_BROADCAST);
	} else {

	    BIT_RESET(rip_flags, RIPF_BROADCAST);
	}
    }

    if (BIT_TEST(rip_flags, RIPF_SOURCE|RIPF_BROADCAST)) {
	/* We are supplying updates */

	/* Gateways do not listen to redirects */
	redirect_disable(tp->task_rtproto);
	
	/* Make sure the timers are active */
	if (!TASK_TIMER(tp, RIP_TIMER_UPDATE)) {
	    /* Create the update timer */

	    (void) timer_create(tp,
				RIP_TIMER_UPDATE,
				"Update",
				0,
				(time_t) RIP_T_UPDATE,
				(time_t) RIP_T_MAX,
				rip_job,
				(void_t) 0);
	}

	if (!TASK_TIMER(tp, RIP_TIMER_FLASH)) {
	    /* Create flash update timer */

	    (void) timer_create(tp,
				RIP_TIMER_FLASH,
				"Flash",
				TIMERF_ABSOLUTE,
				(time_t) RIP_T_FLASH,
				(time_t) RIP_T_MAX,
				rip_do_flash,
				(void_t) 0);
	}
    } else {
	/* We are not supplying updates */
	
	/* Hosts do listen to redirects */
	redirect_enable(tp->task_rtproto);

	/* Make sure the timers do not exist */
	if (TASK_TIMER(tp, RIP_TIMER_UPDATE)) {
	    timer_delete(TASK_TIMER(tp, RIP_TIMER_UPDATE));
	}

	if (TASK_TIMER(tp, RIP_TIMER_FLASH)) {
	    timer_delete(TASK_TIMER(tp, RIP_TIMER_FLASH));
	}
    }

    
    /* Set flags for target list build */
    if (BIT_TEST(rip_flags, RIPF_BROADCAST)) {
	BIT_SET(target_flags, TARGETF_BROADCAST);
    }
    if (BIT_TEST(rip_flags, RIPF_SOURCE)) {
	BIT_SET(target_flags, TARGETF_SOURCE);
    }

    /* Build or update target list */
    targets = target_build(tp,
			   &rip_targets,
			   rip_gw_list,
			   target_flags,
			   sizeof (byte),
			   rip_tsi_dump);

    /* Allocate the output buffers */
    task_alloc_send(tp, (size_t) (targets * RIP_PKTSIZE));

    /* Calculate the maximum packet size for each target */
    TARGET_LIST(tlp, rip_targets) {
	tlp->target_maxsize = RIP_MAXSIZE(tlp->target_ifap);
    } TARGET_LIST_END(tlp, rip_targets) ;

#ifdef	IP_MULTICAST
    TARGET_LIST(tlp, rip_targets) {
	/* Enable or disable v1 compatible v2 extensions */
	if (BIT_TEST(tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_state, RIP_IFPS_V2BC)) {
	    BIT_SET(tlp->target_flags, RIPTF_V2BC);
	} else {
	    BIT_RESET(tlp->target_flags, RIPTF_V2BC);
	}

	if (BIT_TEST(tlp->target_flags, TARGETF_BROADCAST)
	    && BIT_TEST(tlp->target_ifap->ifa_state, IFS_MULTICAST)
	    && BIT_TEST(tlp->target_ifap->ifa_state, IFS_BROADCAST|IFS_POINTOPOINT)) {
	    /* Enable transmission and reception of packets to the RIP multicast group */

	    if (rip_mc_set(tp, tlp)
		&& BIT_TEST(tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_state, RIP_IFPS_V2MC)) {
		/* Interface is MC capable and sending of V2 packets requested */

		BIT_SET(tlp->target_flags, RIPTF_V2MC);
		tlp->target_dst = &rip_addr_mc;
	    } else {
		/* Can not send MC V2 packets */

		BIT_RESET(tlp->target_flags, RIPTF_V2MC);
	    }
	}
    } TARGET_LIST_END(tlp, rip_targets) ;
#endif	/* IP_MULTICAST */

    /* Send a RIP REQUEST for everyone's routing table */
    if (!BIT_TEST(task_state, TASKS_TEST)) {
	byte buffer[sizeof (struct rip) + 2 * sizeof (struct netinfo)];
	struct rip *ripmsg = (struct rip *) ((void_t) buffer);

	bzero((caddr_t) buffer, sizeof buffer);
	ripmsg->rip_cmd = RIPCMD_REQUEST;
	ripmsg->rip_domain = htons(0);	/* XXX */

	TARGET_LIST(tlp, rip_targets) {
	    struct netinfo *nets = (struct netinfo *) ((void_t) (ripmsg + 1));

	    if (ifap && ifap == tlp->target_ifap
		&& (BIT_TEST(rip_flags, RIPF_RECONFIG)
		    || !BIT_TEST(tlp->target_flags, RIPTF_POLL))
		&& BIT_TEST(tlp->target_flags, TARGETF_BROADCAST)
		&& !BIT_TEST(tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN)) {
		/* Do a poll if one has not been done on this interface, */
		/* or we are reconfiguring */

		/* Set the version */
		ripmsg->rip_vers = BIT_TEST(tlp->target_flags, RIPTF_V2) ? RIP_VERSION_2 : RIP_VERSION_1;

		if (0 /* XXX */) {
		    struct authinfo *auth = (struct authinfo *) nets++;

		    auth->auth_family = htons(RIP_AF_AUTH);
		    /* XXX - authentication */
		}

		bzero((caddr_t) nets, sizeof (*nets));
		nets->rip_family = htons(RIP_AF_UNSPEC);
		nets->rip_metric = htonl(RIP_METRIC_UNREACHABLE);

		rip_send(tp,
			 tlp->target_ifap,
			 MSG_DONTROUTE,
			 *tlp->target_dst,
			 (void_t) ripmsg,
			 (size_t) ((byte *) (nets + 1) - (byte *) buffer));
		BIT_SET(tlp->target_flags, RIPTF_POLL);
	    }
	} TARGET_LIST_END(tlp, rip_targets) ;
    }

    /* Evaluate policy for new targets */
    {
	int changes = 0;
	int opened = 0;
	rt_list *rtl = (rt_list *) 0;
	rt_list *rthl = (rt_list *) 0;

	TARGET_LIST(tlp, rip_targets) {
	    if (BIT_TEST(tlp->target_flags, TARGETF_BROADCAST)) {
		if (BIT_TEST(tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN)) {
		    register rt_entry *rt;

		    if (!rtl) {
			/* Get a list of all routes we installed */

			rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

			if (!opened) {
			    rt_open(tp);
			    opened++;
			}
		    }

		    RT_LIST(rt, rtl, rt_entry) {
			if (rt->rt_ifap == tlp->target_ifap
			    && !BIT_TEST(rt->rt_state, RTS_DELETE)) {
			    rt_delete(rt);
			    changes++;
			}			    
		    } RT_LIST_END(rt, rtl, rt_entry) ;
		}
	    }
	    switch (BIT_TEST(tlp->target_flags, TARGETF_POLICY|TARGETF_SUPPLY)) {
	    case TARGETF_SUPPLY:
		/* Need to run policy for this target */

		if (!rthl) {
		    /* Get target list */
		    rthl = rthlist_active(AF_INET);

		    if (!opened) {
			rt_open(tp);
			opened++;
		    }
		}

		/* and run policy */
		changes += rip_policy(tp, tlp, rthl);

		/* Indicate policy has been run */
		BIT_SET(tlp->target_flags, TARGETF_POLICY);
		break;

	    case TARGETF_POLICY:
		/* Indicate policy run on this target */

		BIT_RESET(tlp->target_flags, TARGETF_POLICY);
		break;

	    default:
		break;
	    }
	} TARGET_LIST_END(tlp, rip_targets) ;

	if (rtl) {
	    RTLIST_RESET(rtl);
	}
	if (rthl) {
	    RTLIST_RESET(rthl);
	}

	if (opened) {
	    rt_close(tp, (gw_entry *) 0, 0, NULL);
	}

	if (changes
	    && !BIT_TEST(rip_flags, RIPF_RECONFIG)) {
	    rip_need_flash(tp);
	}
    }

    if (targets != n_targets
	|| rip_n_source != n_source) {

	tracef("rip_target_list: ");
	if (targets) {
	    tracef("supplying updates to");
	    if (targets - rip_n_source) {
		tracef(" %d interface%s",
		       targets - rip_n_source,
		       (targets - rip_n_source) > 1 ? "s" : "");		
	    }
	    if (rip_n_source) {
		tracef(" %d gateways",
		       rip_n_source);
	    }
	} else {
	    tracef("just listening");
	}
	n_targets = targets;
	n_source = rip_n_source;
	trace(TR_RIP, LOG_INFO, NULL);
	trace(TR_RIP, 0, NULL);
    }

    /* XXX - Send a full update if anything has changed */
}


/*
 *	Reinit after parse
 */
/*ARGSUSED*/
static void
rip_reinit __PF1(tp, task *)
{
    int entries = 0;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

    /* Open the routing table */
    rt_open(tp);

    RT_LIST(rt, rtl, rt_entry) {
	pref_t preference = rip_preference;

	/* Calculate preference of this route */
	if (import(rt->rt_dest,
		   rip_import_list,
		   rt->rt_ifap->ifa_ps[tp->task_rtproto].ips_import,
		   rt->rt_gwp->gw_import,
		   &preference,
		   rt->rt_ifap,
		   (void_t) 0)) {
	    if (rt->rt_preference != preference) {
		/* The preference has changed, change the route */
		(void) rt_change(rt,
				 rt->rt_metric,
				 rt->rt_tag,
				 preference,
				 0, (sockaddr_un **) 0);
	    }
	    entries++;
	} else {
	    /* This route is now restricted */
	    rt_delete(rt);
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;

    RTLIST_RESET(rtl);
    
    /* Close the routing table */
    rt_close(tp, (gw_entry *) 0, entries, NULL);

    /* Indicate a reconfig in process */
    BIT_SET(rip_flags, RIPF_RECONFIG);
}


/*
 *	Terminating - clean up
 */
static void
rip_terminate __PF1(tp, task *)
{
    if (BIT_TEST(rip_flags, RIPF_SOURCE|RIPF_BROADCAST)) {
	/* Disable receive */
	tp->task_recv = 0;
	task_set_socket(tp, tp->task_socket);
	timer_reset(TASK_TIMER(tp, RIP_TIMER_UPDATE));

	/* Start shutdown procedure */
	rip_do_flash(TASK_TIMER(tp, RIP_TIMER_FLASH), (time_t) 0);

	/* Make sure we don't try this again */
	tp->task_terminate = 0;
    } else {
	/* Not supplying, go away now */
	rip_exit(tp);
    }
}


/*
 *	Dump info about RIP
 */
static void
rip_int_dump __PF2(fd, FILE *,
		   list, config_entry *)
{
    register config_entry *cp;

    CONFIG_LIST(cp, list) {
	switch (cp->config_type) {
	case RIP_CONFIG_NOIN:
	    (void) fprintf(fd, " noripin");
	    break;

	case RIP_CONFIG_NOOUT:
	    (void) fprintf(fd, " noripout");
	    break;

	case RIP_CONFIG_METRICIN:
	    (void) fprintf(fd, " metricin %u",
			   (metric_t) cp->config_data);
	    break;

	case RIP_CONFIG_METRICOUT:
	    (void) fprintf(fd, " metricout %u",
			   (metric_t) cp->config_data);
	    break;

	case RIP_CONFIG_FLAG:
	    (void) fprintf(fd, " <%s>",
			   trace_bits(rip_if_bits, (flag_t) cp->config_data));
	    break;

	default:
	    assert(FALSE);
	    break;
	}
    } CONFIG_LIST_END(cp, list) ;
}


static void
rip_dump __PF2(tp, task *,
	       fd, FILE *)
{
    (void) fprintf(fd, "\tFlags: %s\tDefault metric: %d\t\tDefault preference: %d\n",
		   trace_bits(rip_flag_bits, rip_flags),
		   rip_default_metric,
		   rip_preference);
    target_dump(fd, rip_targets, rip_target_bits);
    if (rip_gw_list) {
	(void) fprintf(fd, "\tActive gateways:\n");
	gw_dump(fd,
		"\t\t",
		rip_gw_list,
		tp->task_rtproto);
	(void) fprintf(fd, "\n");
    }
    if (rip_int_policy) {
	(void) fprintf(fd, "\tInterface policy:\n");
	control_interface_dump(fd, 2, rip_int_policy, rip_int_dump);
    }
    control_import_dump(fd, 1, RTPROTO_RIP, rip_import_list, rip_gw_list);
    control_export_dump(fd, 1, RTPROTO_RIP, rip_export_list, rip_gw_list);
    (void) fprintf(fd, "\n");
}


/*
 *	Deal with interface policy
 */
static void
rip_control_reset __PF2(tp, task *,
			ifap, if_addr *)
{
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];

    BIT_RESET(ips->ips_state, IFPS_RESET);
    ips->ips_metric_in = ifap->ifa_metric + RIP_HOP;
    ips->ips_metric_out = (metric_t) 0;
}


static void
rip_control_set __PF2(tp, task *,
		      ifap, if_addr *)
{
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];
    config_entry **list = config_resolv(rip_int_policy,
					ifap,
					RIP_CONFIG_MAX);

    /* Init */
    rip_control_reset(tp, ifap);

    if (list) {
	int type = RIP_CONFIG_MAX;
	config_entry *cp;

	/* Fill in the parameters */
	while (--type) {
	    if (cp = list[type]) {
		switch (type) {
		case RIP_CONFIG_NOIN:
		    BIT_SET(ips->ips_state, IFPS_NOIN);
		    break;

		case RIP_CONFIG_NOOUT:
		    BIT_SET(ips->ips_state, IFPS_NOOUT);
		    break;

		case RIP_CONFIG_METRICIN:
		    BIT_SET(ips->ips_state, IFPS_METRICIN);
		    ips->ips_metric_in = (metric_t) cp->config_data;
		    break;

		case RIP_CONFIG_METRICOUT:
		    BIT_SET(ips->ips_state, IFPS_METRICOUT);
		    ips->ips_metric_out = (metric_t) cp->config_data;
		    break;

		case RIP_CONFIG_FLAG:
		    BIT_SET(ips->ips_state, (flag_t) cp->config_data);
		    break;
		}
	    }
	}

	config_resolv_free(list, RIP_CONFIG_MAX);
    }
}


/*
 *	Deal with an interface status change
 */
static void
rip_ifachange __PF2(tp, task *,
		    ifap, if_addr *)
{
    int changes = 0;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = (rt_list *) 0;

    if (socktype(ifap->ifa_addr) != AF_INET) {
	return;
    }
    
    rt_open(tp);
    
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    rip_control_set(tp, ifap);
	}
	break;
	
    case IFC_DELETE:
    case IFC_DELETE|IFC_UPDOWN:
    Down:
	rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	RT_LIST(rt, rtl, rt_entry) {
	    if (rt->rt_ifap == ifap &&
		!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		rt_delete(rt);
		changes++;
	    }
	} RT_LIST_END(rt, rtl, rt_entry) ;
	rip_control_reset(tp, ifap);
	break;

    default:
	/* Something has changed */

	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		rip_control_set(tp, ifap);
	    } else {
		goto Down;
	    }
	}
	if (BIT_TEST(ifap->ifa_change, IFC_METRIC)) {
	    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];

	    /* The metric has changed, reset the POLL bit */
	    /* on any targets using this interface so we'll */
	    /* send another POLL */

	    if (!BIT_TEST(ips->ips_state, IFPS_METRICIN)) {
		target *tlp;

		ips->ips_metric_in = ifap->ifa_metric + RIP_HOP;

		TARGET_LIST(tlp, rip_targets) {
		    if (tlp->target_ifap == ifap) {
			BIT_RESET(tlp->target_flags, RIPTF_POLL);
		    }
		} TARGET_LIST_END(tlp, rip_targets) ;
	    }
	}
	if (BIT_TEST(ifap->ifa_change, IFC_NETMASK)) {
	    /* The netmask has changed, delete any routes that */
	    /* point at gateways that are no longer reachable */

	    target *tlp;
	    
	    rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	    RT_LIST(rt, rtl, rt_entry) {
		if (rt->rt_ifap == ifap
		    && !BIT_TEST(rt->rt_state, RTS_DELETE)
		    && (if_withdstaddr(rt->rt_router) != ifap
			|| BIT_TEST(rt->rt_state, RTS_IFSUBNETMASK))) {
		    /* Interface for this route has changed or we derived the subnet mask */
		    
		    rt_delete(rt);
		    changes++;
		}
	    } RT_LIST_END(rt, rtl, rt_entry) ;

	    TARGET_LIST(tlp, rip_targets) {
		if (tlp->target_ifap == ifap
		    && BIT_COMPARE(tlp->target_flags, TARGETF_SUPPLY|RIPTF_V2MC, TARGETF_SUPPLY)) {
		    /* Some subnet masks may have been derrived, indicate that policy needs to be rerun */

		    BIT_RESET(tlp->target_flags, TARGETF_POLICY);
		}
	    } TARGET_LIST_END(tlp, rip_targets) ;
	}
	if (BIT_TEST(ifap->ifa_change, IFC_BROADCAST)) {
	    /* The broadcast address has changed.  Since target_dst */
	    /* Is a pointer to the pointer to the broadcast address */
	    /* the change is automatic.  But we should reset the POLL */
	    /* bit so we'll POLL this new address in case there are */
	    /* routers we did not yet hear from */

	    target *tlp = target_locate(rip_targets, ifap, (gw_entry *) 0);

	    if (tlp
		&& !BIT_TEST(tlp->target_flags, RIPTF_V2MC)) {
		/* Using the broadcast address on this interface */

		BIT_RESET(tlp->target_flags, RIPTF_POLL);
	    }
	}
	    
	/* A LOCALADDR change will take effect when the peers notice that the */
	/* 	old address is no longer sending */
	/* An MTU change will take effece on output */
	/* A SEL change is not possible in IP */
	break;
    }

    rt_close(tp, (gw_entry *) 0, changes, NULL);

    if (rtl) {
	RTLIST_RESET(rtl);
    }

    /* Update target list */
    rip_target_list(tp, ifap);
}


/*
 * initialize RIP socket and RIP task
 */

/*ARGSUSED*/
void
rip_init()
{
    static struct servent *sp;
    static task *rip_task;
    _PROTOTYPE(flash,
	       void,
	       (task *,
		rt_list *)) = rip_flash;	/* Hack for UTX/32 and Ultrix */
    _PROTOTYPE(newpolicy,
	       void,
	       (task *,
		rt_list *)) = rip_newpolicy;	/* ditto */

    if (BIT_TEST(rip_flags, RIPF_ON)) {
	if (!inet_udpcksum) {
	    trace(TR_ALL, LOG_ERR, "rip_init: UDP checksums *DISABLED* in kernel; RIP disabled");
	    return;
	}
	if (!rip_task) {
	    if (!(sp = getservbyname("route", "udp"))) {
		trace(TR_ALL, LOG_ERR, "rip_init: no service for route available, using %d",
		      RIP_PORT);
		rip_port = htons(RIP_PORT);
	    } else {
		rip_port = sp->s_port;
	    }

	    rip_task = task_alloc("RIP", TASKPRI_PROTO);
	    rip_task->task_addr = sockdup(inet_addr_any);
	    sock2port(rip_task->task_addr) = rip_port;
	    rip_task->task_rtproto = RTPROTO_RIP;
	    rip_task->task_trace_flags = rip_trace_flags;
	    rip_task->task_recv = rip_recv;
	    rip_task->task_cleanup = rip_cleanup;
	    rip_task->task_reinit = rip_reinit;
	    rip_task->task_dump = rip_dump;
	    rip_task->task_terminate = rip_terminate;
	    rip_task->task_ifachange = rip_ifachange;
	    rip_task->task_flash = flash;
	    rip_task->task_newpolicy = newpolicy;
	    rip_task->task_n_timers = RIP_TIMER_MAX;

	    if ((rip_task->task_socket = task_get_socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		task_quit(errno);
	    }
	    if (!task_create(rip_task)) {
		task_quit(EINVAL);
	    }

	    if (task_set_option(rip_task,
				TASKOPTION_BROADCAST,
				TRUE) < 0) {
		task_quit(errno);
	    }
	    if (task_set_option(rip_task,
				TASKOPTION_RECVBUF,
				task_maxpacket) < 0) {
		task_quit(errno);
	    }
	    (void) task_set_option(rip_task,
				   TASKOPTION_RCVDSTADDR,
				   TRUE);
	    if (task_set_option(rip_task,
				TASKOPTION_NONBLOCKING,
				(caddr_t) TRUE) < 0) {
		task_quit(errno);
	    }
	    if (task_addr_local(rip_task, rip_task->task_addr)) {
		trace(TR_ALL, LOG_ERR, "rip_init: is routed or an old copy of gated running?");
		task_quit(errno);
	    }

	    task_alloc_recv(rip_task, RIP_PKTSIZE);

	    if (RIP_T_UPDATE < rt_timer->timer_interval) {
		timer_interval(rt_timer, (time_t) RIP_T_UPDATE);
	    }
#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	    rip_init_mib(TRUE);
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	} else {
	    rip_task->task_trace_flags = rip_trace_flags;
	}
    } else {
	rip_cleanup((task *) 0);

#if	defined(PROTO_SNMP) && defined(MIB_RIP)
	rip_init_mib(FALSE);
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */
	if (rip_task) {
	    rip_terminate(rip_task);

	    rip_task = (task *) 0;
	}
    }
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
