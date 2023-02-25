static char sccsid[] = "@(#)50	1.8  src/tcpip/usr/sbin/gated/if.c, tcprouting, tcpip411, GOLD410 5/20/94 11:23:22";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF8
 *		ifae_decr
 *		ifae_incr
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
 *  if.c,v 1.48.2.5 1993/09/24 02:53:18 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

/*
 * if.c
 *
 */

#define	INCLUDE_IOCTL
#define	INCLUDE_CTYPE
#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */

#define	IF_TIMER_AGE	0	/* To age interface routes */
#define	IF_TIMER_MAX	1

static task *if_task = (task *) 0;

if_link if_plist = { &if_plist, &if_plist };	/* List of physical interfaces */
if_count if_n_link = { 0 };			/* Number of physical interfaces */

if_addr if_list = { {(if_info *) &if_list, (if_info *) &if_list} };	/* List of active interfaces */
if_count if_n_addr[AF_MAX] = { { 0 } };		/* Number of protocol addresses */

if_addr_entry if_local_list = { &if_local_list, &if_local_list };	/* List of unique local addresses of up interfaces */
if_addr_entry if_addr_list = { &if_addr_list, &if_addr_list };		/* List of addresses */
if_addr_entry if_name_list = { &if_name_list, &if_name_list };		/* List of interface names */

if_info if_config = { &if_config, &if_config };	/* List of configured interfaces */

static block_t int_link_block_index;		/* Allocation index for	if_link */
adv_entry *int_import[RTPROTO_MAX] = { 0 };	/* Import clauses for various protocols */
adv_entry *int_export[RTPROTO_MAX] = { 0 };	/* Export clauses for various protocols */
bits const *int_ps_bits[RTPROTO_MAX] = { 0 };	/* Bit definitions for protocols */
adv_entry *int_policy = 0;			/* Interface control info */
static block_t int_block_index;			/* Allocation index for if_addr */
static block_t int_info_block_index;		/* Allocation index for if_info */
static block_t int_entry_block_index;		/* Allocation index for if_addr_entry */
rt_parms int_rtparms = RTPARMS_INIT(1,
				    (metric_t) 0,
				    (flag_t) 0,
				    (pref_t) 0);
const bits if_state_bits[] =
{
    {IFS_UP,		"Up"},
    {IFS_BROADCAST,	"Broadcast"},
    {IFS_POINTOPOINT,	"PointToPoint"},
    {IFS_LOOPBACK,	"Loopback"},
    {IFS_MULTICAST,	"Multicast"},
    {IFS_SIMPLEX,	"Simplex"},

    {IFS_SUBNET,	"Subnet"},
    {IFS_NOAGE,		"NoAge"},
    {IFS_DELETE,	"Delete"},
    {0}
};

const bits if_change_bits[] =
{
/*  {IFC_NOCHANGE,	"NoChange"}, */
    {IFC_REFRESH,	"Refresh"},
    {IFC_ADD,		"Add"},
    {IFC_DELETE,	"Delete"},

    {IFC_UPDOWN,	"UpDown"},
    {IFC_NETMASK,	"Netmask"},
    {IFC_METRIC,	"Metric"},
    {IFC_BROADCAST,	"Broadcast"},
    {IFC_MTU,		"MTU"},
    {IFC_ADDR,		"Address"},
    {0}
};

const bits if_proto_bits[] = {
    {IFPS_METRICIN,	"MetricIn"},
    {IFPS_METRICOUT,	"MetricOut"},
    {IFPS_NOIN,		"NoIn"},
    {IFPS_NOOUT,	"NoOut"},

    {0}
};


/*
 * Find the interface with specified address.  Matches the destination address
 * of P2P interfaces.
 */
if_info *
ifi_withaddr __PF3(addr, sockaddr_un *,
		   broad_ok, int,
		   list, if_info *)
{
    register if_info *ifi;

    IF_INFO(ifi, list) {
	if (socktype(ifi->ifi_addr) == socktype(addr) &&
	    BIT_TEST(ifi->ifi_state, IFS_UP)) {
	    if (sockaddrcmp(ifi->ifi_addr, addr)) {
		break;
	    }
	    if (broad_ok
		&& BIT_TEST(ifi->ifi_state, IFS_BROADCAST)
		&& ifi->ifi_addr_broadcast
		&& sockaddrcmp(ifi->ifi_addr_broadcast, addr)) {
		break;
	    }
	}
    } IF_INFO_END(ifi, list) ;

    return ifi;
}

/*
 * Find the interface with the specified address.  Matches the local address
 * of P2P interfaces
 */
if_info *
ifi_withlcladdr __PF3(addr, sockaddr_un *,
		      broad_ok, int,
		      list, if_info *)
{
    register if_info *ifi;

    IF_INFO(ifi, list) {
	if (socktype(ifi->ifi_addr) == socktype(addr) &&
	    BIT_TEST(ifi->ifi_state, IFS_UP)) {
	    if (sockaddrcmp(ifi->ifi_addr_local, addr)) {
		break;
	    }
	    if (broad_ok
		&& BIT_TEST(ifi->ifi_state, IFS_BROADCAST)
		&& ifi->ifi_addr_broadcast
		&& sockaddrcmp(ifi->ifi_addr_broadcast, addr)) {
		break;
	    }
	}
    } IF_INFO_END(ifi, list) ;

    return ifi;
}


/*
 * Find the POINTOPOINT or LOOPBACK interface with the specified address.
 */
if_info *
ifi_withdstaddr __PF2(addr, sockaddr_un *,
		      list, if_info *)
{
    register if_info *ifi;

    IF_INFO(ifi, list) {
	if (socktype(ifi->ifi_addr) == socktype(addr)  &&
	    BIT_TEST(ifi->ifi_state, IFS_UP) &&
	    BIT_TEST(ifi->ifi_state, IFS_POINTOPOINT|IFS_LOOPBACK) &&
	    sockaddrcmp(ifi->ifi_addr, addr)) {
	    /* Found it */
	    break;
	}
    } IF_INFO_END(ifi, list) ;

    return ifi;
}


/*
 * Find interface on whole network of a possibly subnetted address.
 */
if_addr *
if_withnet __PF1(dstaddr, sockaddr_un *)
{
    int af = socktype(dstaddr);
    register if_addr *ifap = (if_addr *) 0;

    switch (socktype(dstaddr)) {
#ifdef	AF_APPLETALK
    case AF_APPLETALK:
	/* XXX - do range comparison */
	break;
#endif	/* AF_APPLETALK */

#ifdef	notdef
    case AF_INET:
	/* Optimize the IP case */
	IF_ADDR(ifap) {
	    if (af == socktype(ifap->ifa_addr) &&
		BIT_TEST(ifap->ifa_state, IFS_UP) &&
		!BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT) &&
		!((sock2ip(dstaddr) ^ sock2ip(ifap->ifa_addr)) & sock2ip(ifap->ifa_netmask))) {
		return ifap;
	    }
	} IF_ADDR_END(ifap) ;
	break;
#endif	/* notdef */
	    
    default:
	/* General netmask case */
	IF_ADDR(ifap) {
	    if (af == socktype(ifap->ifa_addr)
		&& ifap->ifa_net
		&& BIT_TEST(ifap->ifa_state, IFS_UP)
		&& !BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
		register byte *dp = (byte *) dstaddr->a.sa_data;
		register byte *ap = (byte *) ifap->ifa_addr->a.sa_data;
		register byte *mp = (byte *) ifap->ifa_netmask->a.sa_data;
		register byte *lp = (byte *) ifap->ifa_netmask + socksize(ifap->ifa_netmask);

		for (; mp < lp; mp++) {
		    if ((*dp++ ^ *ap++) & *mp) {
			/* Match failure */
			goto Continue;
		    }
		}
		/* Success */
		break;
	    }
	Continue:
	    ;
	} IF_ADDR_END(ifap) ;
	break;
    }

    return ifap;
}


/*
 * Find interface on a specific subnet of a possibly subnetted network
 */
if_info *
ifi_withsubnet __PF2(dstaddr, sockaddr_un *,
		     list, if_info *)
{
    int af = socktype(dstaddr);
    register if_info *ifi, *ifi_maybe = (if_info *) 0;

    switch (socktype(dstaddr)) {
#ifdef	AF_APPLETALK
    case AF_APPLETALK:
	break;
#endif	/* AF_APPLETALK */

    default:
	/* General netmask case */

	IF_INFO(ifi, list) {
	    if (socktype(ifi->ifi_addr) == af &&
		BIT_TEST(ifi->ifi_state, IFS_UP) &&
		!BIT_TEST(ifi->ifi_state, IFS_POINTOPOINT|IFS_LOOPBACK)) {
		register byte *dp = (byte *) dstaddr->a.sa_data;
		register byte *ap = (byte *) ifi->ifi_addr->a.sa_data;
		register byte *mp = (byte *) ifi->ifi_subnetmask->a.sa_data;
		register byte *lp = (byte *) ifi->ifi_subnetmask + socksize(ifi->ifi_subnetmask);

		for (; mp < lp; mp++) {
		    if ((*dp++ ^ *ap++) & *mp) {
			/* Match failure */
			goto Continue;
		    }
		}

		if (!ifi_maybe ||
		    mask_refines(ifi->ifi_subnetmask, ifi_maybe->ifi_subnetmask)) {
		    /* This is the only mask or is more specific than the last one */
		    ifi_maybe = ifi;
		}
	    }
	Continue:
	    ;
	} IF_INFO_END(ifi, list);
	break;
    }

    return ifi_maybe;
}


/*
 * Find the interface for the specified gateway.  First try to find a P2P interface
 * with the specified address, then find out if we are on the attached network of
 * any multi-access interfaces.
 */
if_info *
ifi_withdst __PF2(dstaddr, sockaddr_un *,
		  list, if_info *)
{
    register if_info *ifi = 0;

    ifi = ifi_withdstaddr(dstaddr, (if_info *) &if_list);
    if (!ifi) {
	ifi = ifi_withsubnet(dstaddr, (if_info *) &if_list);
    }

    return ifi;
}


/*
 * Find the interface with the specified local address or net/subnet address
 */
if_addr *
if_withmyaddr __PF1(addr, sockaddr_un *)
{
    register if_addr *ifap;
    int af = socktype(addr);
    
    IF_ADDR(ifap) {
	if (socktype(ifap->ifa_addr) == af &&
	    BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    if (sockaddrcmp(ifap->ifa_addr, addr)) {
		/* My address */
		break;
	    }
	    if (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
		if (sockaddrcmp(ifap->ifa_addr_local, addr)) {
		    /* My local address */
		    break;
		}
	    } else {
		if (BIT_TEST(ifap->ifa_state, IFS_BROADCAST)
		    && ifap->ifa_addr_broadcast) {
		    if (sockaddrcmp(ifap->ifa_addr_broadcast, addr)) {
			/* My broadcast address */
			break;
		    }
		}
		if ((BIT_TEST(ifap->ifa_state, IFS_SUBNET)
		     && sockaddrcmp(ifap->ifa_subnet, addr))
		    || (ifap->ifa_net
			&& sockaddrcmp(ifap->ifa_net, addr))) {
		    /* My network address */
		    break;
		}
	    }
	}
    } IF_ADDR_END(ifap) ;

    return ifap;
}


/* Lookup the interface the way the kernel does */
if_addr *
if_withroute __PF3(iwr_dest, sockaddr_un *,
		   iwr_router, sockaddr_un *,
		   iwr_state, flag_t)
{
    register if_addr *iwr_ifap;
    
    if (BIT_TEST(iwr_state, RTS_GATEWAY)) {
	/* Remote net or host.  On the other end of a p2p link? */
	iwr_ifap = if_withdstaddr(iwr_router);
	if (!iwr_ifap) {
	    iwr_ifap = if_withsubnet(iwr_router);
	}
    } else {
	/* Route to an interface. */
	iwr_ifap = if_withdstaddr(iwr_dest);
	if (!iwr_ifap) {
	    iwr_ifap = if_withaddr(iwr_router, FALSE);
	}
    }

    return iwr_ifap;
}


/*
 *		Log the configuration of the interface
 */
static void
ifa_display __PF4(ifap, if_addr *,
		  name, const char *,
		  name1, const char *,
		  pri, int)
{
    int proto;

    if (pri) {
	/* Log some info in the syslog */

	tracef("EVENT %s %s ",
	       trace_bits(if_change_bits, ifap->ifa_change),
	       ifap->ifa_name);
	       
	switch (BIT_TEST(ifap->ifa_state, IFS_BROADCAST|IFS_LOOPBACK|IFS_POINTOPOINT)) {
	case IFS_POINTOPOINT:
	    tracef("%A -> %A",
		   ifap->ifa_addr_local,
		   ifap->ifa_addr);
	    break;

	case IFS_BROADCAST:
	    tracef("%A/%A -> %A",
		   ifap->ifa_addr,
		   ifap->ifa_subnetmask,
		   ifap->ifa_addr_broadcast);
	    break;

	case IFS_LOOPBACK:
	    tracef("%A",
		   ifap->ifa_addr);
	    break;

	default:
	    /* NBMA */
	    tracef("%A/%A",
		   ifap->ifa_addr,
		   ifap->ifa_subnetmask);
	    break;
	}
	trace(0, LOG_INFO, " <%s>",
	      trace_bits(if_state_bits, ifap->ifa_state));
    }
    
    trace(TR_INT, 0, "%s%s\t%A",
	  name,
	  name1,
	  ifap->ifa_addr);

    trace(TR_INT, 0, "%s%s\t\tindex: %u  name: %s  state: <%s>",
	  name,
	  name1,
	  ifap->ifa_index,
	  ifap->ifa_name,
	  trace_bits(if_state_bits, ifap->ifa_state));

    trace(TR_INT, 0, "%s%s\t\tchange: <%s>  metric: %u  route: %sinstalled",
	  name,
	  name1,
	  trace_bits(if_change_bits, ifap->ifa_change),
	  ifap->ifa_metric,
	  ifap->ifa_rt ? "" : "not ");

    trace(TR_INT, 0, "%s%s\t\tpreference: %d  down: %d  refcount: %d  mtu: %d",
	  name,
	  name1,
	  ifap->ifa_preference,
	  ifap->ifa_preference_down,
	  ifap->ifa_refcount,
	  ifap->ifa_mtu);

    if (ifap->ifa_addr_broadcast
	|| BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
	if (ifap->ifa_addr_broadcast) {
	    tracef("%s%s\t\tbroadaddr: %A",
		   name,
		   name1,
		   ifap->ifa_addr_broadcast);
	} else if (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
	    tracef("%s%s\t\tlcladdr: %A",
		   name,
		   name1,
		   ifap->ifa_addr_local);
	}
	trace(TR_INT, 0, NULL);
    }
    if (ifap->ifa_net) {
	trace(TR_INT, 0, "%s%s\t\tnet: %A  netmask: %A",
	      name,
	      name1,
	      ifap->ifa_net,
	      ifap->ifa_netmask);
    }
		    
    if (BIT_TEST(ifap->ifa_state, IFS_SUBNET)) {
	trace(TR_INT, 0, "%s%s\t\tsubnet: %A  subnetmask: %A",
	      name,
	      name1,
	      ifap->ifa_subnet,
	      ifap->ifa_subnetmask);
    } else if (ifap->ifa_subnetmask) {
	trace(TR_INT, 0, "%s%s\t\tsubnetmask: %A",
	      name,
	      name1,
	      ifap->ifa_subnetmask);
    }
		    
    for (proto = 0; proto < RTPROTO_MAX; proto++) {
	if (ifap->ifa_ps[proto].ips_state) {
	    tracef("%s%s\t\t\tproto %s state: <%s>",
		   name,
		   name1,
		   trace_state(rt_proto_bits, proto),
		   trace_bits(if_proto_bits, ifap->ifa_ps[proto].ips_state));
	    if (BIT_TEST(ifap->ifa_ps[proto].ips_state, IFPS_METRICIN)) {
		tracef("  metricin: %u",
		       ifap->ifa_ps[proto].ips_metric_in);
	    }
	    if (BIT_TEST(ifap->ifa_ps[proto].ips_state, IFPS_METRICOUT)) {
		tracef("  metricout: %u",
		       ifap->ifa_ps[proto].ips_metric_out);
	    }
	    trace(TR_INT, 0, NULL);
	}
    }
    trace(TR_INT, 0, NULL);
}

#ifdef	notdef
static void
if_display __PF3(target_ifap, if_addr *,
		 name, const char *,
		 pri, int)
{
    if_link *ifl;
    
    IF_LINK(ifl) {
	if_addr *ifap;

	if (!target_ifap || target_ifap->ifa_link == ifl) {
	    trace(TR_INT, 0, "%s\t%s\tIndex %u\tAddress %A",
		  name,
		  ifl->ifl_name,
		  ifl->ifl_index,
		  ifl->ifl_addr);
	    trace(TR_INT, 0, "%s\t\tchange <%s>\tstate <%s>",
		  name,
		  trace_bits(if_change_bits, ifl->ifl_change),
		  trace_bits(if_state_bits, ifl->ifl_state));
	    trace(TR_INT, 0, "%s\t\trefcount %d\tmetric %d\tmtu %d\tup->downs %u",
		  name,
		  ifl->ifl_refcount,
		  ifl->ifl_metric,
		  ifl->ifl_mtu,
		  ifl->ifl_transitions);
	    trace(TR_INT, 0, NULL);

	    IF_ADDR(ifap) {
		if (target_ifap == ifap
		    || (!target_ifap
			&& ifap->ifa_link == ifl)) {

		    ifa_display(ifap, name, "\t", pri);
		}
	    } IF_ADDR_END(ifap) ;
	}
    } IF_LINK_END(ifl) ;
    trace(TR_INT, pri, NULL);
}
#endif	/* notdef */


/* Verify that no two non-POINTOPOINT interfaces have the same address and */
/* that no two interfaces have the same destination route */
static void
if_dupcheck __PF0(void)
{
    if_addr *ifap, *ifap2;

    IF_ADDR(ifap) {
	switch (socktype(ifap->ifa_addr)) {
	case AF_UNSPEC:	/* Place holder */
#ifdef	SOCKADDR_DL
	case AF_LINK:
#endif	/* SOCKADDR_DL */
#ifdef	PROTO_ISO
	case AF_ISO:
#endif	/* PROTO_ISO */
	    continue;

	default:
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		IF_ADDR(ifap2) {
		    if (ifap != ifap2 &&
			socktype(ifap->ifa_addr) == socktype(ifap2->ifa_addr) &&
			BIT_TEST(ifap2->ifa_state, IFS_UP)) {
			if (sockaddrcmp(ifap->ifa_addr, ifap2->ifa_addr)) {
			    /* Make sure the addresses of these interfaces are unique */
			    break;
			}
			/* Make sure we only have one interface for the given net/subnet */
			if (!BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT|IFS_LOOPBACK) &&
			    !BIT_TEST(ifap2->ifa_state, IFS_POINTOPOINT|IFS_LOOPBACK) &&
			    sockaddrcmp(ifap->ifa_subnet, ifap2->ifa_subnet)) {
#ifdef _POWER
			    /* allow network aliases that point to */
			    /* the same network interface */
			    if (strcmp(ifap->ifa_name, ifap2->ifa_name))
			        /* XXX - This is supposedly legal */
			        break;
#else
			    /* XXX - This is supposedly legal */
			    break;
#endif _POWER
			}
		    }
		} IF_ADDR_END(ifap2) ;
		if (ifap2) {
		    tracef("if_dupcheck: address/destination conflicts between %s %A",
			   ifap->ifa_name,
			   ifap->ifa_addr);
		    if (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
			tracef(" lcladdr %A",
			       ifap->ifa_addr_local);
		    }
		    tracef(" and %s %A",
			   ifap2->ifa_name,
			   ifap2->ifa_addr);
		    if (BIT_TEST(ifap2->ifa_state, IFS_POINTOPOINT)) {
			tracef(" lcladdr %A",
			       ifap->ifa_addr_local);
		    }
		    trace(TR_ALL, LOG_CRIT, NULL);
		    ifa_display(ifap, "if_dupcheck:", "", LOG_CRIT);
		    ifa_display(ifap2, "if_dupcheck:", "", LOG_CRIT);
		}
	    }
	    break;
	}
    } IF_ADDR_END(ifap) ;
}


/*
 *	Scan the supplied list for any matches with this interface
 */
adv_entry *
if_policy_match __PF2(ifap, if_addr *,
		      list, adv_entry *)
{
    register adv_entry *adv;

    ADV_LIST(list, adv) {
	switch (adv->adv_flag & ADVF_TYPE) {
		
	case ADVFT_ANY:
	    return adv;

	case ADVFT_IFN:
	    if (ifap->ifa_nameent == adv->adv_ifn
		|| ifap->ifa_nameent_wild == adv->adv_ifn) {
		return adv;
	    }
	    break;

	case ADVFT_IFAE:
	    if (ifap->ifa_addrent == adv->adv_ifae) {
		return adv;
	    }
	    break;

	default:
	    assert(FALSE);
	}
    } ADV_LIST_END(list, adv) ;

    return (adv_entry *) 0;
}


/*
 * Process any control info about this interface
 */
static void
if_control_reset __PF1(ifap, if_addr *)
{
    ifap->ifa_preference = RTPREF_DIRECT;
    ifap->ifa_preference_down = RTPREF_DIRECT_DOWN;
    BIT_RESET(ifap->ifa_state, ifap->ifa_state_policy);
    ifap->ifa_state_policy = 0;
}


static void
if_control_set __PF2(ifap, if_addr *,
		     whom, const char *)
{
    int change = 0;
    config_entry **list = config_resolv(int_policy,
					ifap,
					IF_CONFIG_MAX);

    /* Reset old policy */
    if_control_reset(ifap);

    if (list) {
	int type = IF_CONFIG_MAX;
	config_entry *cp;

	/* Fill in the parameters */
	while (--type) {
	    if (cp = list[type]) {
		switch (type) {
		case IF_CONFIG_PREFERENCE_UP:
		    if (ifap->ifa_preference != (pref_t) cp->config_data) {
			ifap->ifa_preference = (pref_t) cp->config_data;
			change++;
		    }
		    break;
			
		case IF_CONFIG_PREFERENCE_DOWN:
		    if (ifap->ifa_preference_down != (pref_t) cp->config_data) {
			ifap->ifa_preference_down = (pref_t) cp->config_data;
			change++;
		    }
		    break;
			
		case IF_CONFIG_PASSIVE:
		    if (!BIT_TEST(ifap->ifa_state, IFS_NOAGE)) {
			BIT_SET(ifap->ifa_state_policy, (ifap->ifa_state ^ IFS_NOAGE) & IFS_NOAGE);
			BIT_SET(ifap->ifa_state, IFS_NOAGE);
			change++;
		    }
		    break;
			
		case IF_CONFIG_SIMPLEX:
		    if (!BIT_TEST(ifap->ifa_state, IFS_SIMPLEX)) {
			BIT_SET(ifap->ifa_state_policy, (ifap->ifa_state ^ IFS_SIMPLEX) & IFS_SIMPLEX);
			BIT_SET(ifap->ifa_state, IFS_SIMPLEX);
			change++;
		    }
		    break;

		case IF_CONFIG_REJECT:
#ifdef	PROTO_INET
		    if (socktype(ifap->ifa_addr) == AF_INET
			&& BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)
			&& (!inet_addr_reject
			    || !sockaddrcmp(ifap->ifa_addr, inet_addr_reject))
			&& !sockaddrcmp(ifap->ifa_addr, inet_addr_loopback)) {
			if (inet_addr_reject) {
			    sockfree(inet_addr_reject);
			}
			inet_addr_reject = sockdup(ifap->ifa_addr);
			trace(TR_ALL, 0, "%s: Reject address set to %A",
			      whom,
			      inet_addr_reject);
		    }
#endif	/* PROTO_INET */
		    break;

		case IF_CONFIG_BLACKHOLE:
#ifdef	PROTO_INET
		    if (socktype(ifap->ifa_addr) == AF_INET
			&& BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)
			&& (!inet_addr_blackhole
			    || !sockaddrcmp(ifap->ifa_addr, inet_addr_blackhole))
			&& !sockaddrcmp(ifap->ifa_addr, inet_addr_loopback)) {
			if (inet_addr_blackhole) {
			    sockfree(inet_addr_blackhole);
			}
			inet_addr_blackhole = sockdup(ifap->ifa_addr);
			trace(TR_ALL, 0, "%s: Reject address set to %A",
			      whom,
			      inet_addr_blackhole);
		    }
#endif	/* PROTO_INET */
		    break;

		default:
		    assert(FALSE);
		    break;
		}
	    }
	}

	config_resolv_free(list, IF_CONFIG_MAX);
    }
    
    if (change) {
	ifa_display(ifap, whom, "", 0);
    }
}


/**/

/*
 *	Maintain list of local addresses
 */
if_addr_entry *
ifae_alloc __PF1(ifae, register if_addr_entry *)
{
    ifae->ifae_refcount++;

    return ifae;
}


if_addr_entry *
ifae_locate __PF2(addr, sockaddr_un *,
		  list, if_addr_entry *)
{
    register if_addr_entry *ifae;

    IF_ADDR_LIST(ifae, list) {
	if (sockaddrcmp(addr, ifae->ifae_addr)) {
	    /* Found it! */

	    goto Found;
	}
    } IF_ADDR_LIST_END(ifae, list) ;

    /* XXX - Sorted order */

    ifae = (if_addr_entry *) task_block_alloc(int_entry_block_index);
    ifae->ifae_addr = sockdup(addr);

    insque((struct qelem *) ifae,
	   (struct qelem *) list->ifae_back);

 Found:
    return ifae_alloc(ifae);
}


if_addr_entry *
ifae_lookup __PF2(addr, sockaddr_un *,
		  list, if_addr_entry *)
{
    register if_addr_entry *ifae;

    IF_ADDR_LIST(ifae, list) {
	if (sockaddrcmp(addr, ifae->ifae_addr)) {
	    /* Found it! */

	    return ifae_alloc(ifae);
	}
    } IF_ADDR_LIST_END(ifae, list) ;

    return ifae;
}

#define	ifae_incr(ifae, p2p)	{ (ifae)->ifae_n_if++; if (p2p) (ifae)->ifae_n_p2p++; }
#define	ifae_decr(ifae, p2p)	{ (ifae)->ifae_n_if--; if (p2p) (ifae)->ifae_n_p2p--; }

static void
ifae_ifa_alloc __PF1(ifap, if_addr *)
{
    /* Local address */
    ifap->ifa_addrent_local = ifae_locate(ifap->ifa_addr_local,
					  &if_local_list);
    /* Remote address */
    ifap->ifa_addrent = ifae_locate(ifap->ifa_addr,
				    &if_addr_list);

    /* Wildcard name */
    {
	char name[IFNAMSIZ];
	register char *sp = ifap->ifa_name;
	register char *dp = name;

	while (isalpha(*sp)) {
	    *dp++ = *sp++;
	}
	*dp = (char) 0;

	ifap->ifa_nameent_wild = ifae_locate(sockbuild_str(name),
					     &if_name_list);
    }

    /* Interface name */
    ifap->ifa_nameent = ifae_locate(sockbuild_str(ifap->ifa_name),
				    &if_name_list);
}


void
ifae_free __PF1(ifae, if_addr_entry *)
{
    if (!--ifae->ifae_refcount) {
	/* Address no longer referenced, delete entry */

	remque((struct qelem *) ifae);

	/* Free the address */
	sockfree(ifae->ifae_addr);

	if (ifae->ifae_rt) {
	    rt_open(if_task);
	    rt_delete(ifae->ifae_rt);
	    rt_close(if_task, (gw_entry *) 0, 1, NULL);
	}

	task_block_free(int_entry_block_index, (void_t) ifae);
    }
}


/**/

/*
 *	Insert on list in ifa_index order
 */
static void
ifa_insert __PF1(new_ifap, if_addr *)
{
    register if_addr *ifap = (if_addr *) if_list.ifa_forw;

    if (ifap == &if_list) {
	/* First interface */

	ifap = (if_addr *) if_list.ifa_back;
    } else {
	/* Insert in order by index, address family, and finally protocol address */

	do {
	    if (new_ifap->ifa_index > ifap->ifa_index) {
		continue;
	    }
	    if (new_ifap->ifa_index == ifap->ifa_index &&
		 sockaddrcmp2(new_ifap->ifa_addr, ifap->ifa_addr) < 0) {
		/* Insert before this one */

		break;
	    }
	} while ((ifap = (if_addr *) ifap->ifa_forw) != &if_list) ;

	ifap = (if_addr *) ifap->ifa_back;
    }

    insque((struct qelem *) new_ifap,
	   (struct qelem *) ifap);

    /* Count this protocol address */
    if (!BIT_TEST(new_ifap->ifa_state, IFS_LOOPBACK)) {
	if_n_addr[socktype(new_ifap->ifa_addr)].all++;
	if (BIT_TEST(new_ifap->ifa_state, IFS_UP)) {
	    if_n_addr[socktype(new_ifap->ifa_addr)].up++;
	}
    }
}


/**/

/* Find all import or export policy that refers to this interface */
static inline void
if_policy_sub __PF3(ifap, if_addr *,
		    new, adv_entry **,
		    list, adv_entry *)
{
    adv_entry *adv, *last = (adv_entry *) 0;

    for (adv = list;
	 adv = if_policy_match(ifap, adv);
	 adv = adv->adv_next) {
	/* Allocate a new entry and add it to the list */

	adv_entry *new_adv = adv_alloc(ADVFT_ANY, adv->adv_proto);

	new_adv->adv_next = (adv_entry *) 0;
	new_adv->adv_flag = adv->adv_flag;
	new_adv->adv_ru = adv->adv_ru;
	if (new_adv->adv_list = adv->adv_list) {
	    register adv_entry *advp;

	    ADV_LIST(new_adv->adv_list, advp) {
		advp->adv_refcount++;
	    } ADV_LIST(new_adv->adv_list, advp) ;
	}
	switch (adv->adv_flag & ADVF_TYPE) {
	case ADVFT_ANY:
	    break;
	    
	case ADVFT_IFN:
	    new_adv->adv_ifn = ifae_alloc(adv->adv_ifn);
	    break;

	case ADVFT_IFAE:
	    new_adv->adv_ifae = ifae_alloc(adv->adv_ifae);
	    break;

	default:
	    assert(FALSE);
	    break;
	}
#ifdef	notdef
	BIT_RESET(new_adv->adv_flag, ADVF_TYPE);
	BIT_SET(new_adv->adv_flag, ADVFT_ANY);
#endif	/* notdef */

	/* Append to the list */
	if (last) {
	    last->adv_next = new_adv;
	    last = new_adv;
	} else {
	    *new = last = new_adv;
	}
    }
}


static void
if_policy_alloc __PF1(ifap, if_addr *)
{
    int proto = RTPROTO_MAX;

    while (proto--) {
	register struct ifa_ps *ips = &ifap->ifa_ps[proto];

	if (int_import[proto]) {
	    if_policy_sub(ifap, &ips->ips_import, int_import[proto]);
	}
	if (int_export[proto]) {
	    if_policy_sub(ifap, &ips->ips_export, int_export[proto]);
	}
    }
}


/* Free protocol policy lists for this interface */
static void
if_policy_free __PF1(ifap, if_addr *)
{
    int proto = RTPROTO_MAX;

    while (proto--) {
	register struct ifa_ps *ips = &ifap->ifa_ps[proto];

	if (ips->ips_import) {
	    adv_free_list(ips->ips_import);
	    ips->ips_import = (adv_entry *) 0;
	}
	if (ips->ips_export) {
	    adv_free_list(ips->ips_export);
	    ips->ips_export = (adv_entry *) 0;
	}
    }
}


/* Free protocol policy lists */
static void
if_policy_cleanup __PF0(void)
{
    register int proto = RTPROTO_MAX;
    register if_addr *ifap;

    IF_ADDR(ifap) {
	if_policy_free(ifap);
    } IF_ADDR_END(ifap) ;

    while (proto--) {
	if (int_import[proto]) {
	    adv_free_list(int_import[proto]);
	    int_import[proto] = (adv_entry *) 0;
	}
	if (int_export[proto]) {
	    adv_free_list(int_export[proto]);
	    int_export[proto] = (adv_entry *) 0;
	}
    }
}

/**/

static if_link *
ifl_alloc __PF1(ifl, if_link *)
{
    if (ifl) {
	ifl->ifl_refcount++;
	trace(TR_INT, 0, "ifl_alloc: interface %s index %u refcount %d",
	      ifl->ifl_name,
	      ifl->ifl_index,
	      ifl->ifl_refcount);
    }

    return ifl;
}


static if_link *
ifl_free __PF1(ifl, if_link *)
{
    if (ifl) {
	ifl->ifl_refcount--;

	trace(TR_INT, 0, "ifl_free: interface %s index %u refcount %d",
	      ifl->ifl_name,
	      ifl->ifl_index,
	      ifl->ifl_refcount);

	if (!ifl->ifl_refcount) {
	    if_link *prev_ifl = ifl->ifl_back;

	    /* Remove this from the count */
	    if (!BIT_TEST(ifl->ifl_state, IFS_LOOPBACK)) {
		if_n_link.all--;
	    }

	    sockfree(ifl->ifl_addr);
	    
	    remque((struct qelem *) ifl);
	
	    task_block_free(int_link_block_index, (void_t) ifl);

	    ifl = prev_ifl;
	}
    }

    return ifl;
}


if_addr *
ifa_alloc __PF1(ifap, if_addr *)
{
    if (ifap) {
	ifap->ifa_refcount++;
    }

    return ifap;
}


if_addr *
ifa_free __PF1(ifap, if_addr *)
{
    if (ifap) {
	ifap->ifa_refcount--;

	if (!ifap->ifa_refcount) {
	    if_addr *prev_ifap = (if_addr *) ifap->ifa_back;

	    /* Remove this from the count */
	    if (!BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)) {
		if_n_addr[socktype(ifap->ifa_addr)].all--;
	    }
    
	    /* Release the addresses */
	    ifi_addr_free(&ifap->ifa_info);
	    if (ifap->ifa_subnet
		&& ifap->ifa_subnet != ifap->ifa_net) {
		sockfree(ifap->ifa_subnet);
		ifap->ifa_subnet = (sockaddr_un *) 0;
	    }
	    if (ifap->ifa_net) {
		sockfree(ifap->ifa_net);
		ifap->ifa_net = (sockaddr_un *) 0;
	    }

	    /* Free the address entry pointers */
	    ifae_free(ifap->ifa_addrent);
	    ifae_free(ifap->ifa_addrent_local);
	    ifae_free(ifap->ifa_nameent);
	    ifae_free(ifap->ifa_nameent_wild);

	    /* Free the physical interface if necessary */
	    (void) ifl_free(ifap->ifa_link);

	    if_policy_free(ifap);
    
	    remque((struct qelem *) ifap);

	    task_block_free(int_block_index, (void_t) ifap);

	    ifap =  prev_ifap;
	}
    }

    return ifap;
}


/**/
static void
ifa_delete __PF1(ifap, if_addr *)
{
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_REFRESH:
	break;

    case IFC_DELETE:
    case IFC_DELETE|IFC_UPDOWN:
	return;

    default:
	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)
	    && !BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    /* Just transitioned to down */

	    break;
	}
	/* Fall through */

    case IFC_ADD:
	assert(FALSE);
	break;
    }

    if (!BIT_TEST(ifap->ifa_state, IFS_DELETE)) {
	if (BIT_COMPARE(ifap->ifa_state, IFS_UP|IFS_LOOPBACK, IFS_UP)) {
	    if_n_addr[socktype(ifap->ifa_addr)].up--;
	}
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    /* Was up */
	    
	    BIT_RESET(ifap->ifa_state, IFS_UP);
	    BIT_SET(ifap->ifa_state, IFS_DELETE);
	    ifap->ifa_change = IFC_DELETE|IFC_UPDOWN;
	} else {
	    /* Already down */

	    ifap->ifa_change = IFC_DELETE;
	}
    }
}


static void
ifa_down __PF1(ifap, if_addr *)
{
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_REFRESH:
	break;

    case IFC_DELETE:
    case IFC_DELETE|IFC_UPDOWN:
	return;

    default:
	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)
	    && !BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    /* Just transitioned to down */

	    return;
	}
	break;

    case IFC_ADD:
	assert(FALSE);
	break;
    }

    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	BIT_RESET(ifap->ifa_state, IFS_UP);
	ifap->ifa_change = IFC_UPDOWN;
	if (!BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)) {
	    if_n_addr[socktype(ifap->ifa_addr)].up--;
	}
    }
}


static void
ifa_up __PF1(ifap, if_addr *)
{
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_REFRESH:
	if (!BIT_TEST(ifap->ifa_state, IFS_UP)) {

	    BIT_SET(ifap->ifa_change, IFC_UPDOWN);
	    break;
	}
	return;

    case IFC_ADD:
    default:
	if (!BIT_TEST(ifap->ifa_state, IFS_UP)) {

	    break;
	}
	/* Fall through */

    case IFC_DELETE:
    case IFC_DELETE|IFC_UPDOWN:
	return;
    }

    BIT_SET(ifap->ifa_state, IFS_UP);
    if (!BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)) {
	if_n_addr[socktype(ifap->ifa_addr)].up++;
    }
}


/*
 * Notify the protocols of all the interfaces.
 * Called during initialization and reconfiguration.
 */
void
if_notify __PF0(void)
{
    if_addr *ifap;
    if_link *ifl;

    /* Link level interfaces */
    IF_LINK(ifl) {
	/* Notify the protocols */

	task_iflchange(ifl);
    } IF_LINK_END(ifl) ;

    /* Protocol addresses */
    IF_ADDR(ifap) {
	/* Notify the protocols */

	task_ifachange(ifap);
    } IF_ADDR_END(ifap) ;
}


/**/
static task *if_conf_task;	/* Task that is configuring the interfaces */
static int if_conf_all;		/* True if interfaces not referenced are going away */


void
if_conf_open __PF2(tp, task *,
		   all, int)
{

    assert(!if_conf_task);
    if_conf_task = tp;
    if_conf_all = all;

    if (if_conf_all) {
	register if_addr *ifap;
	register if_link *ifl;
	
	IF_ADDR(ifap) {
	    ifap->ifa_change = IFC_NOCHANGE;
	} IF_ADDR_END(ifap) ;

	IF_LINK(ifl) {
	    ifl->ifl_change = IFC_NOCHANGE;
	} IF_LINK_END(ifl);
    }
}


void
if_conf_close __PF1(tp, task *)
{
    register if_addr *ifap;
    register if_link *ifl;

    assert(tp == if_conf_task);
    
    if (if_conf_all) {
	/* Scan for down interfaces */

	/* Check for any physical interfaces that have gone away */
	IF_LINK(ifl) {
	    if (ifl->ifl_change == IFC_NOCHANGE
		&& !BIT_TEST(ifl->ifl_state, IFS_DELETE)) {
		/* No longer present, mark it down */

		if (BIT_TEST(ifl->ifl_state, IFS_UP)) {
		    /* Was up */

		    BIT_RESET(ifl->ifl_state, IFS_UP);
		    BIT_SET(ifl->ifl_state, IFS_DELETE);
		    ifl->ifl_change = IFC_DELETE|IFC_UPDOWN;
		} else {
		    /* Was already down */

		    ifl->ifl_change = IFC_DELETE;
		}
	    }
	} IF_LINK_END(ifl) ;

	/* Check for addresses that are no longer present */
	IF_ADDR(ifap) {
	    if (ifap->ifa_change == IFC_NOCHANGE) {
		/* No longer present - delete it */

		ifa_delete(ifap);
	    }
	} IF_ADDR_END(ifap) ;
    }

    if_dupcheck();

    /* Now scan any remaining physical interfaces to see if we have to change */
    /* state on any addresses */
    IF_LINK(ifl) {
	switch (ifl->ifl_change) {
	case IFC_NOCHANGE:
	case IFC_REFRESH:
	    break;

	case IFC_ADD:
	    if (!BIT_TEST(ifl->ifl_state, IFS_UP)) {
		break;
	    }
	    /* Fall through */

	Up:
	    IF_ADDR(ifap) {
		if (ifap->ifa_link == ifl) {
		    ifa_up(ifap);
		}
	    } IF_ADDR_END(ifap) ;
	    break;

	case IFC_DELETE:
	case IFC_DELETE|IFC_UPDOWN:
	    /* Going away */

	    IF_ADDR(ifap) {
		if (ifap->ifa_link == ifl) {
		    ifa_delete(ifap);
		}
	    } IF_ADDR_END(ifap) ;
	    break;

	default:
	    if (BIT_TEST(ifl->ifl_change, IFC_UPDOWN)) {
		/* Up or Down transition */

		if (BIT_TEST(ifl->ifl_state, IFS_UP)) {
		    /* Down to up */

		    goto Up;
		} else {
		    /* Up to down */

		    ifl->ifl_transitions++;

		    IF_ADDR(ifap) {
			if (ifap->ifa_link == ifl) {
			    ifa_down(ifap);
			}
		    } IF_ADDR_END(ifap) ;
		}
	    }
	}
    } IF_LINK_END(ifl) ;

    /* Do housekeeping based on addresses */
    IF_ADDR(ifap) {
	int p2p = BIT_MATCH(ifap->ifa_state, IFS_POINTOPOINT);

	switch (ifap->ifa_change) {
	case IFC_NOCHANGE:
	case IFC_REFRESH:
	    break;

	case IFC_ADD:
	    ifae_ifa_alloc(ifap);
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		ifae_incr(ifap->ifa_addrent_local, p2p);
		ifae_incr(ifap->ifa_addrent, p2p);
		ifae_incr(ifap->ifa_nameent, p2p);
		ifae_incr(ifap->ifa_nameent_wild, p2p);
	    }
	    break;

	case IFC_DELETE:
	    /* Already down */
	    break;
	    
	case IFC_DELETE|IFC_UPDOWN:
	    ifae_decr(ifap->ifa_addrent_local, p2p);
	    ifae_decr(ifap->ifa_addrent, p2p);
	    ifae_decr(ifap->ifa_nameent, p2p);
	    ifae_decr(ifap->ifa_nameent_wild, p2p);
	    break;

	default:
	    if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
		/* Up or Down transition */

		if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		    /* Down to up */

		    ifae_incr(ifap->ifa_addrent_local, p2p);
		    ifae_incr(ifap->ifa_addrent, p2p);
		    ifae_incr(ifap->ifa_nameent, p2p);
		    ifae_incr(ifap->ifa_nameent_wild, p2p);
		} else {
		    /* Up to down */

		    ifae_decr(ifap->ifa_addrent_local, p2p);
		    ifae_decr(ifap->ifa_addrent, p2p);
		    ifae_decr(ifap->ifa_nameent, p2p);
		    ifae_decr(ifap->ifa_nameent_wild, p2p);
		}
	    }
	    if (BIT_TEST(ifap->ifa_change, IFC_ADDR)) {
		/* Local address change */

		if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		    /* Change local address entry */

		    ifae_decr(ifap->ifa_addrent_local, p2p);
		    ifae_free(ifap->ifa_addrent_local);
		    ifap->ifa_addrent_local = ifae_locate(ifap->ifa_addr_local, &if_local_list);
		    ifae_incr(ifap->ifa_addrent_local, p2p);
		}
	    }
	}
    } IF_ADDR_END(ifap) ;

    /* Notify protocols of changes */
    IF_LINK(ifl) {

	/* Reset the refresh indication */
	BIT_RESET(ifl->ifl_change, IFC_REFRESH);

	/* Display a message and notify the protocols if something has changed */
	if (ifl->ifl_change != IFC_NOCHANGE) {
	    int pri = BIT_TEST(task_state, TASKS_INIT|TASKS_TEST) ? 0 : LOG_INFO;

	    if (ifl->ifl_change != IFC_NOCHANGE) {
		tracef("EVENT <%s> %s index %u <%s>",
		       trace_bits(if_change_bits, ifl->ifl_change),
		       ifl->ifl_name,
		       ifl->ifl_index,
		       trace_bits(if_state_bits, ifl->ifl_state));
		if (ifl->ifl_addr) {
		    tracef(" address %A",
			   ifl->ifl_addr);
		}
		trace(TR_INT, pri, NULL);
		trace(TR_INT, 0, NULL);
	    }
    
	    if (!BIT_TEST(task_state, TASKS_INIT)) {

		/* Notify the protocols */
		task_iflchange(ifl);
	    }

	    /* Reset the change indication */
	    ifl->ifl_change = IFC_NOCHANGE;
	}
    } IF_LINK_END(ifl) ;

    IF_ADDR(ifap) {
	/* Reset the refresh indication */
	BIT_RESET(ifap->ifa_change, IFC_REFRESH);

	/* Display a message and notify the protocols if something has changed */
	if (ifap->ifa_change != IFC_NOCHANGE) {
	    int pri = BIT_TEST(task_state, TASKS_INIT|TASKS_TEST) ? 0 : LOG_INFO;

	    if (ifap->ifa_change != IFC_NOCHANGE) {
		ifa_display(ifap, "EVENT", "", pri);
	    }
    
	    if (!BIT_TEST(task_state, TASKS_INIT)) {
		/* Notify the protocols */

		task_ifachange(ifap);
	    }

	    /* Reset the change indication */
	    ifap->ifa_change = IFC_NOCHANGE;
	}
    } IF_ADDR_END(ifap) ;

    /* Remove old interfaces */
    IF_ADDR(ifap) {
	if (BIT_TEST(ifap->ifa_state, IFS_DELETE)) {
	    ifap = ifa_free(ifap);
	}
    } IF_ADDR_END(ifap) ;

    IF_LINK(ifl) {
	if (BIT_TEST(ifl->ifl_change, IFC_DELETE)) {
	    ifl = ifl_free(ifl);
	}
    } IF_LINK_END(ifl) ;
    
    if_conf_all = FALSE;

    if_conf_task = (task *) 0;
}


static void
ifl_insert __PF2(new_ifl, if_link *,
		 indx, u_int)
{
    register if_link *ifl = if_plist.ifl_forw;

    if (ifl == &if_plist) {
	/* First interface */

	ifl = if_plist.ifl_back;
    } else {
	/* Insert in order by index */

	do {
	    if (indx < ifl->ifl_index) {
		break;
	    }
	} while ((ifl = ifl->ifl_forw) != &if_plist) ;

	ifl = ifl->ifl_back;
    }

    insque((struct qelem *) new_ifl,
	   (struct qelem *) ifl);
}

if_link *
ifl_locate_index __PF1(indx, u_int)
{
    register if_link *ifl;

    IF_LINK(ifl) {
	if (ifl->ifl_index == indx) {
	    return ifl;
	}
    } IF_LINK_END(ifl) ;

    return (if_link *) 0;
}


static inline if_link *
ifl_locate_name __PF2(name, char *,
		      nlen, size_t)
{
    register if_link *ifl;

    IF_LINK(ifl) {
	if (!strncmp(ifl->ifl_name, name, nlen)
	    && strlen(ifl->ifl_name) == nlen) {
	    return ifl;
	}
    } IF_LINK_END(ifl) ;

    return (if_link *) 0;
}


if_link *
ifl_addup __PF8(tp, task *,
		indx, u_int,
		state, flag_t,
		metric, metric_t,
		mtu, mtu_t,
		name, char *,
		nlen, size_t,
		addr, sockaddr_un *)
{
    if_link *ifl;

    assert(tp == if_conf_task);

    if (ifl = ifl_locate_name(name, nlen)) {
	/* Found this one */

	ifl->ifl_change = IFC_NOCHANGE;

	if (!BIT_MASK_MATCH(ifl->ifl_state, state, IFS_UP)) {
	    /* State change */

	    if (BIT_TEST(state, IFS_UP)) {
		/* Was down, now up */

		if (!BIT_TEST(state, IFS_LOOPBACK)) {
		    if_n_link.up++;
		}
	    } else {
		/* Was up, now down */

		if (!BIT_TEST(state, IFS_LOOPBACK)) {
		    if_n_link.up--;
		}
	    }
	    ifl->ifl_change = IFC_UPDOWN;
	}

#ifdef	notdef
	/* Look for state changes */
	if (mtu != ifl->ifl_mtu) {
	    BIT_SET(ifl->ifl_change, IFC_MTU);
	}

	if (metric != ifl->ifl_metric) {
	    BIT_SET(ifl->ifl_change, IFC_METRIC);
	}
#endif	/* notdef */

	if (addr) {
	    /* The Data Link sockaddr contains the name, */
	    /* link-level address and link-level selector */

	    /* Check for change */
	    if (!sockaddrcmp(addr, ifl->ifl_addr)) {
		/* Link level address has changed */

		BIT_SET(ifl->ifl_change, IFC_ADDR);

		sockfree(ifl->ifl_addr);
		ifl->ifl_addr = sockdup(addr);
	    }
	}

	if (ifl->ifl_change == IFC_NOCHANGE) {
	    /* No changes, just a refresh */

	    ifl->ifl_change = IFC_REFRESH;
	}

	if (addr) {
	    goto copy_addr;
	} else {
	    goto copy;
	}
    }

    /* Allocate a structure */
    ifl = (if_link *) task_block_alloc(int_link_block_index);

    /* Count this interface */
    if (!BIT_TEST(state, IFS_LOOPBACK)) {
	if_n_link.all++;
	if (BIT_TEST(state, IFS_UP)) {
	    if_n_link.up++;
	}
    }

    if (addr) {
	ifl->ifl_addr = sockdup(addr);
    }
    ifl_insert(ifl, indx);

    ifl->ifl_change = IFC_ADD;

 copy_addr:
    strncpy(ifl->ifl_name, name, nlen);
    ifl->ifl_name[IFNAMSIZ] = (char) 0;

 copy:
    ifl->ifl_index = indx;
    ifl->ifl_state = state;
    ifl->ifl_metric = metric;
    ifl->ifl_mtu = mtu;

    if (ifl->ifl_change == IFC_ADD) {
	(void) ifl_alloc(ifl);
    }
    
    return ifl;
}


/*
 *	Used to explicitly delete addresses, normally they are
 *	deleted when they are not added during a complete configuration.
 */
void
if_conf_deladdr __PF2(tp, task *,
		      ifi, if_info *)
{
    register if_addr *ifap;

    assert(tp == if_conf_task);

    IF_ADDR(ifap) {
	if (sockaddrcmp(ifap->ifa_addr, ifi->ifi_addr)
	    && ifi->ifi_link == ifap->ifa_link
	    && BIT_MASK_MATCH(ifap->ifa_state, ifi->ifi_state, IFS_POINTOPOINT|IFS_LOOPBACK|IFS_BROADCAST)) {
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		/* Up - mark it down */

		ifa_delete(ifap);
	    } else {
		/* Already marked down - indicate we saw it */

		ifap->ifa_change = IFC_REFRESH;
	    }
	    break;
	}
    } IF_ADDR_END(ifap) ;

    /* Release the addresses */
    ifi_addr_free(ifi);
}


void
if_conf_addaddr __PF2(tp, task *,
		      ifi, if_info *)
{
    register if_addr *ifap;
    sockaddr_un *net, *netmask, *subnet;

    assert(tp == if_conf_task);

    /* If the local and remote addresses of a P2P interface are the same */
    /* assume that it is in testing mode and ignore it */
    if (!ifi->ifi_addr || !ifi->ifi_addr_local
	|| (BIT_TEST(ifi->ifi_state, IFS_POINTOPOINT)
	    && (sockaddrcmp(ifi->ifi_addr, ifi->ifi_addr_local)))) {
	/* Flag it as down */

    Delete:
	tracef("if_conf_addaddr: ignoring %s %A/%A",
	       ifi->ifi_link->ifl_name,
	       ifi->ifi_addr_local,
	       ifi->ifi_subnetmask);
	
	if (BIT_TEST(ifi->ifi_state, IFS_POINTOPOINT)) {
	    tracef(" -> %A",
		   ifi->ifi_addr);
	} else if (BIT_TEST(ifi->ifi_state, IFS_BROADCAST)) {
	    tracef(" -> %A",
		   ifi->ifi_addr_broadcast);
	}
	trace(TR_ALL, 0, NULL);
	if_conf_deladdr(tp, ifi);
	return;
    }

    /* Calculate masks and stuff */
    switch (socktype(ifi->ifi_addr)) {
#ifdef	PROTO_INET
    case AF_INET:
	/* Verify the addresses */
	if (sock2ip(ifi->ifi_addr) == INADDR_ANY
	    || sock2ip(ifi->ifi_addr) == INADDR_BROADCAST
	    || sock2ip(ifi->ifi_addr_local) == INADDR_ANY
	    || sock2ip(ifi->ifi_addr_local) == INADDR_BROADCAST) {
	    /* Bogus addresses */

	    goto Delete;
	}
	
	/* Calculate the natural netmask */
	net = sockdup(ifi->ifi_addr);
	netmask = inet_mask_natural(net);
	if (!netmask) {
	    trace(TR_ALL, LOG_ERR, "if_conf_addaddr: Unknown class for interface %A(%s)",
		  ifi->ifi_addr,
		  ifi->ifi_link->ifl_name);
	    task_quit(EINVAL);
	}
	sockaddrmask_in(net, netmask);

	/* Calculate the subnet */
	if (BIT_TEST(ifi->ifi_state, IFS_LOOPBACK)) {
	    /* Loopback host */

	    if (sockaddrcmp(ifi->ifi_addr, inet_addr_loopback)) {
		/* The 127.0.0.1 host needs to generate the 127/255 reject network */

		subnet = net;
		ifi->ifi_subnetmask = netmask;
	    } else {
		/* Just a host route */
		
		subnet = (sockaddr_un *) 0;
		ifi->ifi_subnetmask = inet_mask_host;
	    }
	} else if (BIT_TEST(ifi->ifi_state, IFS_POINTOPOINT)) {
	    /* Check the mask to see if it looks valid */

	    if (ifi->ifi_subnetmask == netmask
		|| ifi->ifi_subnetmask == inet_mask_host
		|| ((sock2ip(ifi->ifi_addr) ^ sock2ip(ifi->ifi_addr_local)) & sock2ip(netmask))) {
		/* Mask is not valid, assume host mask */

		ifi->ifi_subnetmask = inet_mask_host;
	    }
	    subnet = (sockaddr_un *) 0;
	} else if (mask_refines(ifi->ifi_subnetmask, netmask)) {
	    /* This net is subnetted */

	    if (socktype(ifi->ifi_subnetmask) != AF_INET
		|| sock2ip(ifi->ifi_subnetmask) == INADDR_ANY) {
		/* Bogus subnet mask */

		goto Delete;
	    }

	    if (BIT_TEST(ifi->ifi_state, IFS_BROADCAST)
		&& (socktype(ifi->ifi_addr_broadcast) != AF_INET
		    || sock2ip(ifi->ifi_addr_broadcast) == INADDR_ANY)) {
		/* Bogus broadcast address */

		goto Delete;
	    }
	    
	    /* XXX - This needs thought for supernetting */
	    BIT_SET(ifi->ifi_state, IFS_SUBNET);
	    subnet = sockdup(ifi->ifi_addr);
	    sockmask(subnet, ifi->ifi_subnetmask);
	} else {
	    /* This net is not subnetted */
	    
	    subnet = net;
	}

	/* Adjust the MTU */
	ifi->ifi_mtu -= IP_MAXHDRLEN;
#endif	/* PROTO_INET */
	break;

#ifdef	PROTO_ISO
    case AF_ISO:
	if (!ifi->ifi_subnetmask
	    && BIT_TEST(ifi->ifi_state, IFS_LOOPBACK|IFS_POINTOPOINT)) {
	    /* No mask provided */

	    net = netmask = subnet = (sockaddr_un *) 0;
	} else {
	    /* Mask provided */

	    net = subnet = sockdup(ifi->ifi_addr);
	    sockmask(subnet, netmask = ifi->ifi_subnetmask);
	}
	break;
#endif	/* PROTO_ISO */

    default:
	/* Unknown address family - ignore it */
	goto Delete;
	break;
    }

    IF_ADDR(ifap) {
	if (sockaddrcmp(ifap->ifa_addr, ifi->ifi_addr)
	    && ifi->ifi_link == ifap->ifa_link
	    && BIT_MASK_MATCH(ifap->ifa_state, ifi->ifi_state, IFS_POINTOPOINT|IFS_LOOPBACK|IFS_BROADCAST)) {
	    /* Old address */

	    ifap->ifa_change = IFC_NOCHANGE;

	    /* MTU */
	    if (ifap->ifa_mtu != ifi->ifi_mtu) {
		/* The MTU has changed */

		ifap->ifa_mtu = ifi->ifi_mtu;
		BIT_SET(ifap->ifa_change, IFC_MTU);
	    }

	    /* Metric */
	    if (ifap->ifa_metric != ifi->ifi_metric) {
		/* The metric has changed */

		ifap->ifa_metric = ifi->ifi_metric;
		BIT_SET(ifap->ifa_change, IFC_METRIC);
	    }

	    /* Subnet mask */
	    if (ifap->ifa_subnetmask != ifi->ifi_subnetmask) {
		/* The subnet mask has changed */

 		/* Get the new mask */
  		ifap->ifa_subnetmask = ifi->ifi_subnetmask;

		/* Free the old net and subnet mask */
		if (ifap->ifa_subnet
		    && ifap->ifa_subnet != ifap->ifa_net) {
		    sockfree(ifap->ifa_subnet);
		}
		if (ifap->ifa_net) {
		    sockfree(ifap->ifa_net);
		}

		/* Assign new net and mask */
		ifap->ifa_net = net;
		ifap->ifa_subnet = subnet;
		net = subnet = (sockaddr_un *) 0;
 
		/* Fix the flags */
		BIT_RESET(ifap->ifa_state, IFS_SUBNET);
		BIT_SET(ifap->ifa_state, ifi->ifi_state & IFS_SUBNET);
		BIT_SET(ifap->ifa_change, IFC_NETMASK);
	    }

	    /* Broadcast address */
	    if (!sockaddrcmp(ifap->ifa_addr_broadcast, ifi->ifi_addr_broadcast)) {
		/* The broadcast address has changed */

		sockfree(ifap->ifa_addr_broadcast);
		ifap->ifa_addr_broadcast = ifi->ifi_addr_broadcast;
		BIT_SET(ifap->ifa_change, IFC_BROADCAST);
	    } else if (ifi->ifi_addr_broadcast) {
		/* No change, just free the address */

		sockfree(ifi->ifi_addr_broadcast);
	    }

	    /* Local address (for P2P links) */
	    if (!sockaddrcmp(ifap->ifa_addr_local, ifi->ifi_addr_local)) {
		/* The local address has changed */

		sockfree(ifap->ifa_addr_local);
		ifap->ifa_addr_local = ifi->ifi_addr_local;
		if (ifi->ifi_addr != ifi->ifi_addr_local) {
		    sockfree(ifi->ifi_addr);
		}
		BIT_SET(ifap->ifa_change, IFC_ADDR);
	    } else {
		/* No changes, just free the addresses */
		
		sockfree(ifi->ifi_addr_local);
		if (ifi->ifi_addr != ifi->ifi_addr_local) {
		    sockfree(ifi->ifi_addr);
		}
	    }

	    if (!ifap->ifa_change) {
		/* No changes, just a refresh */

		ifap->ifa_change = IFC_REFRESH;
	    }

	    /* XXX - Error if configured twice */

	    goto Return;
	}
    } IF_ADDR_END(ifap) ;

    /* New address */

    ifap = (if_addr *) task_block_alloc(int_block_index);

    ifap->ifa_info = *ifi;	/* struct copy */

    /* Bump the reference counts */
    (void) ifl_alloc(ifa_alloc(ifap)->ifa_link);

    /* Flag it as changed */
    ifap->ifa_change = IFC_ADD;

    /* Set default preferences */
    ifap->ifa_preference = RTPREF_DIRECT;
    ifap->ifa_preference_down = RTPREF_DIRECT_DOWN;

#ifdef	PROTO_INET
    if (BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)
	|| sockaddrcmp(ifap->ifa_addr, inet_addr_loopback)
	) {
	/* Make sure loopback bit is set */

	BIT_SET(ifap->ifa_state, IFS_LOOPBACK);
    }
#endif	/* PROTO_INET */

    /* Assign the net and netmask */
    ifap->ifa_net = net;
    ifap->ifa_netmask = netmask;
    ifap->ifa_subnet = subnet;
    net = subnet = (sockaddr_un *) 0;

    /* Insert this interface into the list */
    ifa_insert(ifap);
	
 Return:
    if (net) {
	sockfree(net);
    }
    if (subnet && subnet != net) {
	sockfree(subnet);
    }
    net = subnet = (sockaddr_un *) 0;
    ifi->ifi_addr = ifi->ifi_addr_local = ifi->ifi_addr_broadcast = ifi->ifi_subnetmask = (sockaddr_un *) 0;
}


/**/
/* Support for pre-configured interfaces */

int
if_parse_add __PF2(ifi2, if_info *,
		   err_msg, char *)
{
    if_info *ifi;

    /* Scan for duplicates */
    IF_INFO(ifi, &if_config) {
	if (sockaddrcmp(ifi->ifi_addr, ifi2->ifi_addr) &&
	    BIT_MASK_MATCH(ifi->ifi_state, ifi2->ifi_state, IFS_POINTOPOINT|IFS_LOOPBACK|IFS_BROADCAST)) {
	    /* Duplicate address */

	    (void) sprintf(err_msg, "if_parse_add: Duplicate address: %A <%s>",
			   ifi->ifi_addr,
			   trace_bits(if_state_bits, BIT_TEST(ifi->ifi_state, IFS_POINTOPOINT|IFS_LOOPBACK|IFS_BROADCAST)));
	    return TRUE;
	}
    } IF_INFO_END(ifi, &if_config) ;

    ifi = (if_info *) task_block_alloc(int_info_block_index);
    *ifi = *ifi2;	/* struct copy */

    /* Mark it up so ifi_with*() routines don't ignore it */
    BIT_SET(ifi->ifi_state, IFS_UP);

    insque((struct qelem *) ifi,
	   (struct qelem *) if_config.ifi_back);

    return FALSE;
}


/* Remove all preconfigured interfaces */
static void
if_parse_clear __PF0(void)
{
    if_info *ifi;
    
    IF_INFO(ifi, &if_config) {
	if_info *ifi2 = ifi->ifi_back;

	remque((struct qelem *) ifi);

	task_block_free(int_info_block_index, (void_t) ifi);

	ifi = ifi2;
    } IF_INFO_END(ifi, &if_config) ;
}


adv_entry *
if_parse_address __PF1(addr, sockaddr_un *)
{
    register adv_entry *adv = (adv_entry *) 0;
    
    if (!addr
	|| (BIT_TEST(task_state, TASKS_STRICTIFS)
	    && !if_withaddr(addr, FALSE)
	    && !ifi_withaddr(addr, FALSE, &if_config))) {
	return adv;
    }
    
    adv = adv_alloc(ADVFT_IFAE, (proto_t) 0);
    adv->adv_ifae = ifae_locate(addr, &if_addr_list);

    return adv;
}


adv_entry *
if_parse_name __PF2(name, char *,
		    create, int)
{
    adv_entry *adv;
    if_addr_entry *ifae;
    
    if (create) {
	/* Lookup and create if necessary */

	ifae = ifae_locate(sockbuild_str(name),
			   &if_name_list);
    } else {
	/* Lookup but don't create */

	ifae = ifae_lookup(sockbuild_str(name),
			   &if_name_list);
    }

    if (ifae) {
	/* Build a policy entry */

	adv = adv_alloc(ADVFT_IFN, (proto_t) 0);
	adv->adv_ifn = ifae;
    } else {
	/* Indicate failure */

	adv = (adv_entry *) 0;
    }

    return adv;
}


static void
if_int_dump __PF2(fd, FILE *,
		  list, config_entry *)
{
    register config_entry *cp;

    CONFIG_LIST(cp, list) {
	switch (cp->config_type) {
	case IF_CONFIG_PREFERENCE_UP:
	    (void) fprintf(fd, " preference up %d",
			   (pref_t) cp->config_data);
	    break;
			
	case IF_CONFIG_PREFERENCE_DOWN:
	    (void) fprintf(fd, " preference down %d",
			   (pref_t) cp->config_data);
	    break;
			
	case IF_CONFIG_PASSIVE:
	    (void) fprintf(fd, " passive");
	    break;
			
	case IF_CONFIG_SIMPLEX:
	    (void) fprintf(fd, " simplex");
	    break;
			
	case IF_CONFIG_REJECT:
	    (void) fprintf(fd, " reject");
	    break;
			
	case IF_CONFIG_BLACKHOLE:
	    (void) fprintf(fd, " blackhole");
	    break;
			
	default:
	    assert(FALSE);
	    break;
	}
    } CONFIG_LIST_END(cp, list) ;
}


/*
 *	Dump the interface list
 */
static void
if_dump __PF2(tp, task *,
	      fd, FILE *)
{
    int proto;
    if_link *ifl;
    if_addr *ifap;
    if_addr_entry *ifae;

    /*
     * Print out all of the interface stuff.
     */

    (void) fprintf(fd,
		   "\t\tPhysical interfaces: %u\tUp: %u\n",
		   if_n_link.all,
		   if_n_link.up);

    for (proto = AF_UNSPEC; proto < AF_MAX; proto++) {
	if (if_n_addr[proto].all) {
	    (void) fprintf(fd,
			   "\t\t%s protocol addresses: %u\tUp: %u\n",
			   trace_value(task_domain_bits, proto),
			   if_n_addr[proto].all,
			   if_n_addr[proto].up);
	}
    }
    
    (void) fprintf(fd,
		   "\n");

    /* Print out address lists */
    (void) fprintf(fd,
		   "\tAddresses:\n");

    IF_ADDR_LIST(ifae, &if_addr_list) {
	(void) fprintf(fd,
		       "\t\t%A\n\t\t\tP2P %u\tTotal %u\tRefcount %u\tRoute: %sinstalled\n",
		       ifae->ifae_addr,
		       ifae->ifae_n_p2p,
		       ifae->ifae_n_if,
		       ifae->ifae_refcount,
		       ifae->ifae_rt ? "" : "not ");
    } IF_ADDR_LIST_END(ifae, &if_addr_list) ;

    (void) fprintf(fd,
		   "\n\tLocal addresses:\n");

    IF_ADDR_LIST(ifae, &if_local_list) {
	(void) fprintf(fd,
		       "\t\t%A\n\t\t\tP2P %u\tTotal %u\tRefcount %u\tRoute: %sinstalled\n",
		       ifae->ifae_addr,
		       ifae->ifae_n_p2p,
		       ifae->ifae_n_if,
		       ifae->ifae_refcount,
		       ifae->ifae_rt ? "" : "not ");
    } IF_ADDR_LIST_END(ifae, &if_local_list) ;

    (void) fprintf(fd,
		   "\n\tNames:\n");

    IF_ADDR_LIST(ifae, &if_name_list) {
	(void) fprintf(fd,
		       "\t\t%A\n\t\t\tP2P %u\tTotal %u\tRefcount %u\tRoute: %sinstalled\n",
		       ifae->ifae_addr,
		       ifae->ifae_n_p2p,
		       ifae->ifae_n_if,
		       ifae->ifae_refcount,
		       ifae->ifae_rt ? "" : "not ");
    } IF_ADDR_LIST_END(ifae, &if_name_list) ;

    (void) fprintf(fd,
		   "\n\n\tInterfaces:\n\n");

    IF_LINK(ifl) {
	(void) fprintf(fd, "\t%s\tIndex %u%s%A\tChange: <%s>\tState: <%s>\n",
		       ifl->ifl_name,
		       ifl->ifl_index,
		       ifl->ifl_addr ? " Address " : " ",
		       ifl->ifl_addr ? ifl->ifl_addr : sockbuild_str(""),
		       trace_bits(if_change_bits, ifl->ifl_change),
		       trace_bits(if_state_bits, ifl->ifl_state));
	(void) fprintf(fd, "\t\tRefcount: %d\tUp-down transitions: %u\n",
		       ifl->ifl_refcount,
		       ifl->ifl_transitions);
	(void) fprintf(fd, "\n");

	IF_ADDR(ifap) {
	    if (ifap->ifa_link == ifl) {
		(void) fprintf(fd, "\t\t%A\n\t\t\tMetric: %d\tMTU: %d\n",
			       ifap->ifa_addr,
			       ifap->ifa_metric,
			       ifap->ifa_mtu);
		(void) fprintf(fd, "\t\t\tRefcount: %d\tPreference: %d\tDown: %d\n",
			       ifap->ifa_refcount,
			       ifap->ifa_preference,
			       ifap->ifa_preference_down);
		(void) fprintf(fd, "\t\t\tChange: <%s>\tState: <%s>\n",
			       trace_bits(if_change_bits, ifap->ifa_change),
			       trace_bits(if_state_bits, ifap->ifa_state));
		if (ifap->ifa_addr_broadcast) {
		    (void) fprintf(fd, "\t\t\tBroadcast Address:   %A\n",
				   ifap->ifa_addr_broadcast);
		}
		if (ifap->ifa_addr_local
		    && BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
		    (void) fprintf(fd, "\t\t\tLocal Address: %A\n",
				   ifap->ifa_addr_local);
		}
		if (ifap->ifa_net) {
		    (void) fprintf(fd, "\t\t\tNet Number:    %A\n",
				   ifap->ifa_net);
		    (void) fprintf(fd, "\t\t\tNet Mask:    %A\n",
				   ifap->ifa_netmask);
		}

		if (BIT_TEST(ifap->ifa_state, IFS_SUBNET)) {
		    (void) fprintf(fd, "\t\t\tSubnet Number: %A\t\tSubnet Mask: %A\n",
				   ifap->ifa_subnet,
				   ifap->ifa_subnetmask);
		} else if (ifap->ifa_subnetmask) {
		    (void) fprintf(fd, "\t\t\tSubnet Mask: %A\n",
				   ifap->ifa_subnetmask);
		}
		if (ifap->ifa_rtactive) {
		    int i = RTPROTO_MAX;

		    (void) fprintf(fd,
				   "\t\t\tRouting protocols active:");
		    while (i--) {
			if (BIT_TEST(ifap->ifa_rtactive, RTPROTO_BIT(i))) {
			    (void) fprintf(fd,
					   " %s",
					   trace_state(rt_proto_bits, i));
			}
		    } ;
		    (void) fprintf(fd,
				   "\n");
		}
		for (proto = 0; proto < RTPROTO_MAX; proto++) {
		    struct ifa_ps *ips = &ifap->ifa_ps[proto];
		    
		    if (ips->ips_state || ips->ips_import || ips->ips_export) {
			(void) fprintf(fd, "\t\t\tproto:  %s\tState: <%s>",
				       trace_state(rt_proto_bits, proto),
				       trace_bits2(if_proto_bits, int_ps_bits[proto], ips->ips_state));
			if (BIT_TEST(ips->ips_state, IFPS_METRICIN)) {
			    (void) fprintf(fd, "\tMetricin: %u",
				       ips->ips_metric_in);
			}
			if (BIT_TEST(ips->ips_state, IFPS_METRICOUT)) {
			    (void) fprintf(fd, "\tMetricout: %u",
				       ips->ips_metric_out);
			}
			(void) fprintf(fd, "\n");
			if (ips->ips_import) {
			    (void) fprintf(fd, "\t\t\t\tImport policy:\n");
			    control_interface_import_dump(fd, 5, ips->ips_import);
			}
			if (ips->ips_export) {
			    (void) fprintf(fd, "\t\t\t\tExport policy:\n");
			    control_interface_export_dump(fd, 5, ips->ips_export);
			}
		    }
		}
		(void) fprintf(fd,
			       "\n");
	    }
	} IF_ADDR_END(ifap) ;
    } IF_LINK_END(ifl) ;

    (void) fprintf(fd,
		   "\n");

    /* Dump policy */
    if (int_policy) {
	(void) fprintf(fd,
		       "\tInterface policy:\n");
	control_interface_dump(fd, 2, int_policy, if_int_dump);
    }
    for (proto = 0; proto < RTPROTO_MAX; proto++) {
	if (int_import[proto] || int_export[proto]) {
	    if (int_import[proto]) {
		(void) fprintf(fd, "\t\t%s Import policy:\n",
			       trace_state(rt_proto_bits, proto));
		control_interface_import_dump(fd, 2, int_import[proto]);
	    }
	    if (int_export[proto]) {
		(void) fprintf(fd, "\t\t%s Export policy:\n",
			       trace_state(rt_proto_bits, proto));
		control_interface_export_dump(fd, 2, int_export[proto]);
	    }
	}
    }
}


static void
if_cleanup __PF1(tp, task *)
{
    /* Remove the preconfigured interfaces */
    if_parse_clear();

    /* Release policy list */
    adv_free_list(int_policy);
    int_policy = (adv_entry *) 0;

    if_policy_cleanup();
}


static void
if_check_age __PF0(void)
{
    if_addr *ifap;

    /* XXX - Need to sync timer to RT timer? */

    IF_ADDR(ifap) {
	rt_entry *rt = ifap->ifa_rt;

	if (rt) {
	    /* Interface is up and Address family has interface routes */

	    if (!BIT_TEST(ifap->ifa_state, IFS_NOAGE|IFS_SIMPLEX|IFS_LOOPBACK)
		&& ifap->ifa_rtactive
		&& if_n_addr[socktype(ifap->ifa_addr)].up > 1) {
		/* Interface is elligible for timeout */

		if (rt->rt_preference != ifap->ifa_preference_down
		    && rt->rt_timer >= rt->rt_gwp->gw_timer_max) {
		    /* Interface has timed out */
		    
		    if_rtdown(ifap);
		}
	    } else {

		if (rt->rt_preference != ifap->ifa_preference) {
		    /* Interface should be up */

		    if_rtup(ifap);
		}
	    }
	}
    } IF_ADDR_END(ifap) ;
}


static void
if_ifachange __PF2(tp, task *,
		   ifap, if_addr *)
{
    rt_open(tp);

    switch (ifap->ifa_change) {
    case IFC_REFRESH:
	assert(FALSE);
	break;

    case IFC_NOCHANGE:
    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	Up:
	    if_control_set(ifap, "if_ifachange:");
	    if_rtup(ifap);
	    if_policy_alloc(ifap);
	}
	break;
	
    case IFC_DELETE:
	break;
	
    case IFC_DELETE|IFC_UPDOWN:
    Down:
	if_control_reset(ifap);
	if (ifap->ifa_rt) {
	    if_rtdelete(ifap);
	}
	if_policy_free(ifap);
	break;

    default:
	/* Something has changed */

	if (!BIT_TEST(ifap->ifa_change, IFC_CCHANGE)) {
	    /* Should not happen */

	    assert(FALSE);
	} else if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		/* Transition to UP - install the routes */

		goto Up;
	    } else {
		/* Transition to DOWN - delete the routes */

		goto Down;
	    }
	}
	/* BROADCAST - no change to route */
	/* MTU - no change to route */
	/* METRIC - no change to route */
	if (BIT_TEST(ifap->ifa_change, IFC_ADDR|IFC_NETMASK)) {
	    /* The route needs to be changed */
	    if (ifap->ifa_rt) {
		if_rtdelete(ifap);

		if_rtup(ifap);
	    }
	}
	break;
    }

    if_rtifachange(ifap);

    rt_close(tp, (gw_entry *) 0, 0, NULL);

    /* Check for timed out interfaces */
    if_check_age();
}


static void
if_iflchange __PF2(tp, task *,
		   ifl, if_link *)
{

    /* Check for unusual conditions */

    switch (ifl->ifl_change) {
    case IFC_NOCHANGE:
	/* Assume we are reinitting */
	break;

    case IFC_REFRESH:
    default:
	if (!BIT_TEST(ifl->ifl_change, IFC_CCHANGE)) {
	    /* Should not happen */

	    assert(FALSE);
	}
	break;

    case IFC_DELETE:
    case IFC_DELETE|IFC_UPDOWN:
	assert(!BIT_TEST(ifl->ifl_state, IFS_UP));
	break;

    case IFC_ADD:
	break;

    }
}


static void
if_age __PF2(tip, timer *,
	      interval, time_t)
{
    if_check_age();
}


static void
if_terminate __PF1(tp, task *)
{
    if_addr *ifap;

    /* Reinstall routes for any interfaces that timed out */
    IF_ADDR(ifap) {
	rt_entry *rt = ifap->ifa_rt;

	if (rt
	    && rt->rt_preference != ifap->ifa_preference) {
	    /* Interface should be up */
	    
	    if_rtup(ifap);
	}
    } IF_ADDR_END(ifap) ;

    task_delete(tp);
}


/*
 *	Initialize after configuration before the protocols
 */
void
if_init __PF0(void)
{
    register if_addr *ifap;

    IF_ADDR(ifap) {
	if_ifachange(if_task, ifap);
    } IF_ADDR_END(ifap);

    if (BIT_TEST(task_state, TASKS_NOFLASH)) {
	/* We are reconfiguring, cause the kernel to be updated with any */
	/* routes we have changed */

	rt_flash_update(TRUE);
    }
}


/*
 *	Initialize task for interface check
 */
void
if_family_init __PF0(void)
{
    if_task = task_alloc("IF", TASKPRI_INTERFACE);
    if_task->task_rtproto = RTPROTO_DIRECT;
    if_task->task_dump = if_dump;
    if_task->task_cleanup = if_cleanup;
    if_task->task_ifachange = if_ifachange;
    if_task->task_iflchange = if_iflchange;
    if_task->task_n_timers = IF_TIMER_MAX;
    if_task->task_terminate = if_terminate;

    if (!task_create(if_task)) {
	task_quit(EINVAL);
    }

    int_rtparms.rtp_gwp = gw_init((gw_entry *) 0,
				  if_task->task_rtproto,
				  if_task,
				  (as_t) 0,
				  (as_t) 0,
				  IF_T_TIMEOUT,
				  (sockaddr_un *) 0,
				  (flag_t) 0);

    (void) timer_create(if_task,
			IF_TIMER_AGE,
			"AGE",
			0,
			(time_t) RT_T_AGE,
			(time_t) 0,
			if_age,
			(void_t) 0);

    int_block_index = task_block_init(sizeof (if_addr));
    int_info_block_index = task_block_init(sizeof (if_info));
    int_link_block_index = task_block_init(sizeof (if_link));
    int_entry_block_index = task_block_init(sizeof (if_addr_entry));
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
