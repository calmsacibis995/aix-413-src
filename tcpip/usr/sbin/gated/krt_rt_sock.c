static char sccsid[] = "@(#)78	1.2  src/tcpip/usr/sbin/gated/krt_rt_sock.c, tcprouting, tcpip411, GOLD410 3/31/94 12:21:19";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: KRT_BUMP
 *		KRT_MSG_FIRST
 *		KRT_RESET
 *		KRT_TIMER
 *		RTM_LIST
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF6
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
 *  krt_rt_sock.c,v 1.6.2.1 1993/09/24 02:18:49 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

#define	INCLUDE_ROUTE
#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"
#include "krt_var.h"


const bits rtm_type_bits[] =
{
    {RTM_ADD, "ADD"},
    {RTM_DELETE, "DELETE"},
    {RTM_CHANGE, "CHANGE"},
    {RTM_GET, "GET"},
    {RTM_LOSING, "LOSING"},
    {RTM_REDIRECT, "REDIRECT"},
    {RTM_MISS, "MISS"},
    {RTM_LOCK, "LOCK"},
    {RTM_OLDADD, "OLDADD"},
    {RTM_OLDDEL, "OLDDEL"},
    {RTM_RESOLVE, "RESOLVE"},
#ifdef	RTM_NEWADDR
    {RTM_NEWADDR,	"NEWADDR"},
#endif	/* RTM_NEWADDR */
#ifdef	RTM_DELADDR
    {RTM_DELADDR,	"DELADDR"},
#endif	/* RTM_DELADDR */
#ifdef	RTM_IFINFO
    {RTM_IFINFO,	"IFINFO"},
#endif	/* RTM_IFINFO */
    {0, NULL}
};

static const bits rtm_lock_bits[] =
{
    {RTV_MTU, "MTU"},
    {RTV_HOPCOUNT, "HOPCOUNT"},
    {RTV_EXPIRE, "EXPIRE"},
    {RTV_RPIPE, "RPIPE"},
    {RTV_SPIPE, "SPIPE"},
    {RTV_SSTHRESH, "SSTHRESH"},
    {RTV_RTT, "RTT"},
    {RTV_RTTVAR, "RTTVAR"},
    {0, NULL}
};

static const bits rtm_sock_bits[] =
{
    {RTA_DST,		"Dest"},
    {RTA_GATEWAY,	"Gateway"},
    {RTA_NETMASK,	"NetMask"},
    {RTA_GENMASK,	"GenMask"},
    {RTA_IFP,		"IFP"},
    {RTA_IFA,		"IFA"},
    {RTA_AUTHOR,	"Author"},
#ifdef	RTA_BRD
    {RTA_BRD,		"Broadcast"},
#endif	/* RTA_BRD */
    {0, NULL}
};


#ifdef	KRT_IFREAD_KINFO
static const bits krt_if_flag_bits[] =
{
    {IFF_UP,		"UP" },
    {IFF_BROADCAST,	"BROADCAST" },
    {IFF_DEBUG,		"DEBUG" },
    {IFF_LOOPBACK,	"LOOPBACK" },
    {IFF_POINTOPOINT,	"POINTOPOINT" },
#ifdef	IFF_NOTRAILERS
    {IFF_NOTRAILERS,	"NOTRAILERS" },
#endif	/* IFF_NOTRAILERS */
    {IFF_RUNNING,	"RUNNING" },
#ifdef	IFF_NOARP
    {IFF_NOARP,		"NOARP" },
#endif	/* IFF_NOARP */
    {IFF_PROMISC,	"PROMISC" },
    {IFF_ALLMULTI,	"ALLMULTI" },
    {IFF_OACTIVE,	"OACTIVE" },
    {IFF_SIMPLEX,	"SIMPLEX" },
#ifdef	IFF_LINK0
    {IFF_LINK0,		"LINK0" },
    {IFF_LINK1,		"LINK1" },
    {IFF_LINK2,		"LINK2" },
#endif	/* IFF_LINK0 */
    {0, NULL}
};
#endif	/* KRT_IFREAD_KINFO */


/*
 * Pickup all the address from the message
 */
krt_addrinfo *
krt_xaddrs __PF2(rtp, register struct rt_msghdr *,
		 len, size_t)
{
    register int i;
    register int family = AF_UNSPEC;
    register struct sockaddr *ap;
    caddr_t limit = (caddr_t) rtp + len;
    static krt_addrinfo addrinfo;

    if (rtp->rtm_msglen != len) {
	trace(TR_ALL, LOG_ERR, "krt_xaddrs: length mismatch!  Reported: %d, actual %d",
	      rtp->rtm_msglen,
	      len);
	return (krt_addrinfo *) 0;
    }
    
    if (rtp->rtm_version != RTM_VERSION) {
	trace(TR_ALL, LOG_ERR, "krt_xaddrs: version mismatch!  Expected %d, received %d",
	      RTM_VERSION,
	      rtp->rtm_version);
	return (krt_addrinfo *) 0;
    }

    /* Locate address bits and start of addresses */
    switch (rtp->rtm_type) {
    case RTM_ADD:
    case RTM_DELETE:
    case RTM_CHANGE:
    case RTM_GET:
    case RTM_LOSING:
    case RTM_REDIRECT:
    case RTM_MISS:
    case RTM_LOCK:
    case RTM_OLDADD:
    case RTM_OLDDEL:
    case RTM_RESOLVE:
	ap = (struct sockaddr *) (rtp + 1);
	addrinfo.rti_addrs = rtp->rtm_addrs;
	break;

#ifdef	KRT_IFREAD_KINFO
    case RTM_IFINFO:
	ap = (struct sockaddr *) (((struct if_msghdr *) rtp) + 1);
	addrinfo.rti_addrs = ((struct if_msghdr *) rtp)->ifm_addrs;
	break;

    case RTM_NEWADDR:
    case RTM_DELADDR:
	ap = (struct sockaddr *) (((struct ifa_msghdr *) rtp) + 1);
	addrinfo.rti_addrs = ((struct ifa_msghdr *) rtp)->ifam_addrs;
	break;
#endif	/* KRT_IFREAD_KINFO */

    default:
	return (krt_addrinfo *) 0;
    }


    /* Grab all the addresses */
    RTAX_LIST(i) {
	if (BIT_TEST(addrinfo.rti_addrs, 1 << i)) {
	    if ((caddr_t) ap >= limit) {
		trace(TR_ALL, LOG_ERR, "krt_xaddrs: out of data fetching %s address",
		      trace_state(rtm_sock_bits, i));
		return (krt_addrinfo *) 0;
	    }
	    
	    switch (i) {
	    case RTAX_DST:
	    case RTAX_IFA:
		family = ap->sa_family;
		/* Fall through */
		
	    case RTAX_GATEWAY:
	    case RTAX_IFP:
	    case RTAX_AUTHOR:
	    case RTAX_BRD:
		if (ap->sa_len) {
		    addrinfo.rti_info[i] = sock2gated(ap, ap->sa_len);
		}
		break;

	    case RTAX_NETMASK:
	    case RTAX_GENMASK:
		addrinfo.rti_info[i] = (sockaddr_un *) ap;
		break;
	    }

	    RTM_ADDR(ap);
	} else {
	    addrinfo.rti_info[i] = (sockaddr_un *) 0;
	}
    } RTAX_LIST_END(i) ;

    /* Now that we have the family, convert the masks */
    for (i = RTAX_NETMASK; i <= RTAX_GENMASK; i++) {
	if (ap = (struct sockaddr *) addrinfo.rti_info[i]) {
	    if (ap->sa_len < 2) {
		/* Zero mask, fudge it so we can translate it */

		*((u_long *) ap) = 0;
		ap->sa_len = 2;	/* Minimum length */
	    }

	    /* Set the family and translate */
	    ap->sa_family = family;
	    addrinfo.rti_info[i] = sock2gated(ap, ap->sa_len);
	}
    }

    return &addrinfo;
}


/*
 * Common logic for controling routes
 */
int
krt_rtaddrs __PF4(adip, krt_addrinfo *,
		  rtp, rt_parms *,
		  author, sockaddr_un **,
		  flags, flag_t)    
{
    int rc = KRT_ADDR_OK;
    
    rtp->rtp_state = krt_flags_to_state(flags);

    rtp->rtp_dest = adip->rti_info[RTAX_DST];
    if (!rtp->rtp_dest) {
	return KRT_ADDR_IGNORE;
    }

    switch (socktype(rtp->rtp_dest)) {
#ifdef	PROTO_INET
    case AF_INET:
	if (BIT_TEST(flags, RTF_LLINFO)
	    || BIT_MATCH(flags, RTF_CLONING|RTF_GATEWAY)) {
	    /* Skip ARP cache entries and cloning masks that are not interface routes */
	    return KRT_ADDR_IGNORE;
	}
	break;
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
    case AF_ISO:
        if (BIT_TEST(flags, RTF_DYNAMIC)) {
	    /* Delete redirects */
            rc = KRT_ADDR_BOGUS;
	    break;
        }
        if (BIT_TEST(flags, RTF_LLINFO|RTF_CLONING)) {
	    /* Skip ES-IS routes and cloning masks */
            return KRT_ADDR_IGNORE;
        }
        break;
#endif	/* PROTO_ISO */

    default:
	break;
    }

    *author = adip->rti_info[RTAX_AUTHOR];

    rtp->rtp_router = adip->rti_info[RTAX_GATEWAY];
    if (rtp->rtp_router) {
	/*
	 * Kernel gateway is possibly an AF_LINK.  If so, look up the interface
	 * with the destination address and figure out the gateway from there.
	 * When installing the interface route the kernel is supposed to put what
	 * it wants so we can just keep it as the gateway.
	 */
	switch (socktype(rtp->rtp_router)) {
	    register if_addr *ifap;

	case AF_LINK:
	    if (!BIT_TEST(flags, RTF_GATEWAY)) {
		/* Not an interface route */

		return KRT_ADDR_IGNORE;
	    }
	    
	    ifap = if_withsubnet(rtp->rtp_dest);

	    if (ifap) {
		if (ifap->ifa_index == rtp->rtp_router->dl.sdl_index) {
		    rtp->rtp_router = ifap->ifa_addr;
		}
	    } else {
		/* Bogus, delete it */

		rc = KRT_ADDR_BOGUS;
	    }
	    break;
		
	default:
	    break;
	}
    }

    rtp->rtp_dest_mask = adip->rti_info[RTAX_NETMASK];
    if (rtp->rtp_dest_mask) {
	/* Have a netmask */
	
	rtp->rtp_dest_mask = mask_locate(rtp->rtp_dest_mask);
    } else {
	/* Figure out a mask */
	    
	switch (socktype(rtp->rtp_dest)) {
#ifdef	PROTO_INET
	case AF_INET:
	    rtp->rtp_dest_mask = BIT_TEST(flags, RTF_HOST) ? inet_mask_host : inet_mask_natural(rtp->rtp_dest);
	    break;
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
	case AF_ISO:
	    rtp->rtp_dest_mask = iso_mask_natural(rtp->rtp_dest);
	    break;
#endif	/* PROTO_ISO */

	default:
	    assert(TRUE);
	}
    }

    /* If we have a mask, mask off potential garbage in the address */
    if (rtp->rtp_dest_mask) {
	sockmask(rtp->rtp_dest, rtp->rtp_dest_mask);
    }
	
    /* Check to see if the address is valid */
    switch (krt_addrcheck(rtp)) {
    case KRT_ADDR_OK:
	/* Address is OK */
	break;

    case KRT_ADDR_IGNORE:
	/* Ignore it */
	return KRT_ADDR_IGNORE;

    case KRT_ADDR_BOGUS:
	/* Bogus, delete it */
	rc = KRT_ADDR_BOGUS;
	break;

#ifdef	IP_MULTICAST
    case KRT_ADDR_MC:
	/* Multicast group */
	rc = KRT_ADDR_MC;
	break;
#endif	/* IP_MULTICAST */
    }

    if (if_withnet(rtp->rtp_dest)
	|| if_withdstaddr(rtp->rtp_dest)) {
	BIT_SET(rtp->rtp_state, RTS_INTERIOR);
    } else {
	BIT_SET(rtp->rtp_state, RTS_EXTERIOR);
    }

    return rc;
}

 
inline static rt_entry *
krt_not_installed __PF1(installed_rt, rt_entry *)
{
    sockaddr_un *router;
    
    /* Get the installed router address */
    rttsi_get(installed_rt->rt_head, krt_task->task_rtbit, (byte *) &router);
    sockfree(router);

    /* Free the route */
    if (!rtbit_reset(installed_rt, krt_task->task_rtbit)) {
	/* Route no longer exists */

	return (rt_entry *) 0;
    }
    
    /* Head still exists, reset router */
    rttsi_reset(installed_rt->rt_head, krt_task->task_rtbit);

    return installed_rt;
}


/* Trace a routing socket packet */
/*ARGSUSED*/
void
krt_trace __PF6(tp, task *,
		direction, const char *,
		rtp, struct rt_msghdr *,
		len, size_t,
		adip, krt_addrinfo *,
		pri, int)
{
    if (!adip) {
	adip = krt_xaddrs(rtp, len);
	if (!adip) {
	    /* Could not parse message */
	    return;
	}
    }

    TRACE_UPDATE(TR_KRT) {
	/* Long form */

	tracef("KRT %s len %d ver %d type %s(%d)",
	       direction,
	       rtp->rtm_msglen,
	       rtp->rtm_version,
	       trace_state(rtm_type_bits, rtp->rtm_type - 1),
	       rtp->rtm_type);
    } else {
	/* Short form */

	tracef("KRT %s type %s(%d)",
	       direction,
	       trace_state(rtm_type_bits, rtp->rtm_type - 1),
	       rtp->rtm_type);
    }

    switch (rtp->rtm_type) {
    case RTM_ADD:
    case RTM_DELETE:
    case RTM_CHANGE:
    case RTM_GET:
    case RTM_LOSING:
    case RTM_REDIRECT:
    case RTM_MISS:
    case RTM_LOCK:
    case RTM_OLDADD:
    case RTM_OLDDEL:
    case RTM_RESOLVE:
	TRACE_UPDATE(TR_KRT) {
	    /* Long form */
	    tracef(" addrs <%s>(%x) pid %d seq %d",
		   trace_bits(rtm_sock_bits, (flag_t) rtp->rtm_addrs),
		   rtp->rtm_addrs,
		   rtp->rtm_pid,
		   rtp->rtm_seq);
	    if (rtp->rtm_errno) {
		errno = rtp->rtm_errno;
		trace(TR_KRT, pri, " error %d: %m",
		      rtp->rtm_errno);
	    } else {
		trace(TR_KRT, pri, NULL);
	    }

	    tracef("KRT %s flags %s(%x)",
		   direction,
		   trace_bits(krt_flag_bits, (flag_t) rtp->rtm_flags),
		   rtp->rtm_flags & 0xffff);
	    if (rtp->rtm_rmx.rmx_locks) {
		tracef("  locks %s(%x)",
		       trace_bits(rtm_lock_bits, rtp->rtm_rmx.rmx_locks),
		       rtp->rtm_rmx.rmx_locks);
	    }
	    if (rtp->rtm_inits) {
		tracef(" inits %s(%x)",
		       trace_bits(rtm_lock_bits, rtp->rtm_inits),
		       rtp->rtm_inits);
	    }
	    trace(TR_KRT, pri, NULL);

	    /* Display metrics */
	    switch (rtp->rtm_type) {
	    case RTM_ADD:
	    case RTM_CHANGE:
	    case RTM_GET:
	    case RTM_LOCK:
		if (BIT_TEST(rtp->rtm_rmx.rmx_locks|rtp->rtm_inits, RTV_MTU|RTV_HOPCOUNT|RTV_EXPIRE|RTV_SSTHRESH)) {
		    trace(TR_KRT, pri, "KRT %s mtu %d hopcount %d expire %#T ssthresh %d",
			  direction,
			  rtp->rtm_rmx.rmx_mtu,
			  rtp->rtm_rmx.rmx_hopcount,
			  rtp->rtm_rmx.rmx_expire,
			  rtp->rtm_rmx.rmx_ssthresh);
		}
		if (BIT_TEST(rtp->rtm_rmx.rmx_locks|rtp->rtm_inits, RTV_RPIPE|RTV_SPIPE|RTV_RTT|RTV_RTTVAR)) {
		    trace(TR_KRT, pri, "KRT %s recvpipe %d sendpipe %d rtt %d rttvar %d",
			  direction,
			  rtp->rtm_rmx.rmx_recvpipe,
			  rtp->rtm_rmx.rmx_sendpipe,
			  rtp->rtm_rmx.rmx_rtt,
			  rtp->rtm_rmx.rmx_rttvar);
		}
		break;
	    }
	} else {
	    /* Short form */

	    tracef("flags %s(%x) error %d",
		   trace_bits(krt_flag_bits, (flag_t) rtp->rtm_flags),
		   rtp->rtm_flags & 0xffff,
		   rtp->rtm_errno);
	    if (rtp->rtm_errno) {
		errno = rtp->rtm_errno;
		trace(TR_KRT, pri, ": %m");
	    } else {
		trace(TR_KRT, pri, NULL);
	    }
	}
	break;

#ifdef	KRT_IFREAD_KINFO
    case RTM_IFINFO:
        {
	    struct if_msghdr *ifap = (struct if_msghdr *) rtp;
	    
	    trace(TR_KRT, pri, " index %d flags <%s>%x mtu %d metric %d",
		  ifap->ifm_index,
		  trace_bits(krt_if_flag_bits, ifap->ifm_flags),
		  ifap->ifm_flags,
		  ifap->ifm_data.ifi_mtu,
		  ifap->ifm_data.ifi_metric);
	}
	break;

    case RTM_NEWADDR:
    case RTM_DELADDR:
        {
	    struct ifa_msghdr *ifap = (struct ifa_msghdr *) rtp;
	    
	    trace(TR_KRT, pri, " index %d flags <%s>%x metric %d",
		  ifap->ifam_index,
		  trace_bits(krt_flag_bits, ifap->ifam_flags),
		  ifap->ifam_flags,
		  ifap->ifam_metric);
	}
	break;
#endif	/* KRT_IFREAD_KINFO */

    default:
	return;
    }
    
    /* Display addresses */
    if (adip->rti_addrs) {
	register int i;

	tracef("KRT %s", direction);

	RTAX_LIST(i) {
	    register sockaddr_un *ap = adip->rti_info[i];

	    if (ap) {
		tracef(" %s %A",
		       gd_lower(trace_state(rtm_sock_bits, i)),
		       ap);
	    }
	} RTAX_LIST_END(i);

	trace(TR_KRT, pri, NULL);
    }
}


void
krt_trace_disp __PF4(msg, const char *,
		     adip, krt_addrinfo *,
		     flags, flag_t,
		     pri, int)
{
    TRACE_ON(TR_KRT) {
	trace(TR_KRT, pri, "%s route",
	      msg);
    } else {
	trace(TR_KRT, pri, "%s route to %A/%A via %A <%s>",
	      msg,
	      adip->rti_info[RTAX_DST],
	      adip->rti_info[RTAX_NETMASK],
	      adip->rti_info[RTAX_GATEWAY],
	      trace_bits(krt_flag_bits, flags));
    }
}


/* Read interfaces from kernel */

#ifdef	KRT_IFREAD_KINFO
void
krt_ifaddr __PF4(tp, task *,
		 ifap, struct ifa_msghdr *,
		 adip, krt_addrinfo *,
		 ifl, if_link *)
{
    sockaddr_un *ap;
    if_info ifi;
    int addrs;
    int p2p = FALSE;

    if (!ifl) {
	/* No link level interface association */
    }

    bzero((caddr_t) &ifi, sizeof (ifi));

    ifi.ifi_link = ifl;
    ifi.ifi_rtflags = ifap->ifam_flags;
    ifi.ifi_state = ifl->ifl_state;
    /* If we have a metric, use it, otherwise get it from the cached link level info */
    ifi.ifi_metric = ifap->ifam_metric ? ifap->ifam_metric : ifl->ifl_metric;
    /* Pickup the cached MTU info */
    ifi.ifi_mtu = ifl->ifl_mtu;

    switch (BIT_TEST(ifl->ifl_state, IFS_POINTOPOINT|IFS_LOOPBACK|IFS_BROADCAST)) {
    case IFS_POINTOPOINT:
	addrs = 2;
	p2p = TRUE;
	break;

    case IFS_LOOPBACK:
	addrs = 1;
	break;

    case IFS_BROADCAST:
	addrs = 3;
	break;

    default:
	/* NBMA */
	addrs = 2;
	break;
    }
    
    if (ap = adip->rti_info[RTAX_IFA]) {
	/* Interace address */

	ifi.ifi_addr_local = sockdup(ap);
	if (!p2p) {
	    ifi.ifi_addr = ifi.ifi_addr_local;
	}
	addrs--;
    }
    if (ap = adip->rti_info[RTAX_BRD]) {
	/* Remote or broadcast address */

	if (p2p) {
	    ifi.ifi_addr = sockdup(ap);
	} else {
	    ifi.ifi_addr_broadcast = sockdup(ap);
	}
	addrs--;
    }
    if (ap = adip->rti_info[RTAX_NETMASK]) {
	/* Subnet mask */

	ifi.ifi_subnetmask = mask_locate(ap);
	addrs--;
    }

    if (addrs <= 0) {
	/* We have enough addresses */

	if (ifap->ifam_type == RTM_NEWADDR) {
	    if_conf_addaddr(tp, &ifi);
	} else {
	    if_conf_deladdr(tp, &ifi);
	}
    } else {
	/* Clean up addresses */

	ifi_addr_free(&ifi);
    }
}
#endif	/* KRT_IFREAD_KINFO */


/* Support for BSD4.4 route socket. */

#define	KRT_TIMEOUT	2				/* Default initial timeout on a failure */
#define	KRT_MAX_TIMEOUT	64				/* Maximum timeout */
#define	KRT_TIMER(tp)	TASK_TIMER(tp, KRT_TIMER_TIMEOUT)
/* Action to take on sucessive failures */
#define	KRT_BUMP(value)	\
  (((value) > KRT_MAX_TIMEOUT/2) ? ((value) = KRT_MAX_TIMEOUT) : ((value) *= 2))
#define	KRT_RESET(value)	value = KRT_TIMEOUT	/* Reset timer */
static time_t krt_waittime = KRT_TIMEOUT;		/* Length of time before a response is overdue */

static u_int rtm_seq;					/* Sequence number */

struct rtm_block {
    struct rtm_block *rtb_forw;
    struct rtm_msg *rtb_first;
    struct rtm_msg *rtb_free;
};

struct rtm_msg {
    struct rtm_block *rtm_block;
    size_t rtm_len;
    struct rt_msghdr msghdr;
};

static struct rtm_block *rtm_head;			/* Head of message queue */
static struct rtm_block *rtm_last;			/* Last page allocated */

#define	RTM_LIST(rtp)	\
{ \
    struct rtm_block *rtb; \
    for (rtb = rtm_head; rtb; rtb = rtb->rtb_forw) { \
	for (rtp = rtb->rtb_first; rtp < rtb->rtb_free; rtp = (struct rtm_msg *) ((caddr_t) rtp + rtp->rtm_len))

#define	RTM_LIST_END(rtp) \
    } \
}

/* Is this the first message */
#define	KRT_MSG_FIRST(rtp)	((rtp) == rtm_head->rtb_first)

			      
static inline struct rtm_msg *
krt_msg_alloc __PF1(size, size_t)
{
    struct rtm_block *rtb = rtm_last;
    struct rtm_msg *rtp;

    /* Add size of message and round up */
    size += sizeof (struct rtm_msg);
    size = ROUNDUP(size, sizeof (u_long));
    
    if (!rtb
	|| ((caddr_t) rtb->rtb_free + size) > (caddr_t) rtb + task_pagesize) {
	/* Need to allocate a new page */

	rtb = (struct rtm_block *) task_block_alloc(task_block_pagesize);

	rtb->rtb_first = rtb->rtb_free = (struct rtm_msg *) ((caddr_t) rtb + sizeof (*rtb));
	if (rtm_last) {
	    /* Append to end of chain */

	    rtm_last->rtb_forw = rtb;
	    rtm_last = rtb;
	} else {
	    /* Start of chain */

	    rtm_head = rtm_last = rtb;
	}
    }

    /* Get and clear block */
    rtp = rtb->rtb_free;
    bzero((caddr_t) rtp, size);

    /* Set some pointers */
    rtp->rtm_len = size;
    rtp->rtm_block = rtb;

    /* Set free pointer */
    rtb->rtb_free = (struct rtm_msg *) ((caddr_t) rtb->rtb_free + rtp->rtm_len);

    return rtp;
}


static inline void
krt_msg_free __PF1(rtp, struct rtm_msg *)
{
    struct rtm_block *rtb = rtp->rtm_block;

    assert(rtp == rtb->rtb_first);
    
    rtb->rtb_first = rtp = (struct rtm_msg *) ((caddr_t) rtp + rtp->rtm_len);
    if (rtb->rtb_first == rtb->rtb_free) {
	/* Free the page */

	assert(rtb == rtm_head);

	if (rtb->rtb_forw) {
	    /* Not the only block, free */
	    
	    /* Point at next page */
	    rtm_head = rtb->rtb_forw;

	    /* Free it */
	    task_block_free(task_block_pagesize, (void_t) rtb);
	
	    if (rtb == rtm_last) {
		/* Remove last page */

		rtm_last = rtm_head;
	    }
	} else {
	    /* Reset pointers */

	    rtm_head = rtm_last = rtb;
	    rtb->rtb_free = rtb->rtb_first = (struct rtm_msg *) ((caddr_t) rtb + sizeof (*rtb));
	}
    }
}


/* Issue a request */
static inline int
krt_send __PF2(ki_tp, task *,
	       ki_rtp, struct rtm_msg *)
{
    int retry_type = 0;

    if (!ki_rtp->msghdr.rtm_seq) {
	ki_rtp->msghdr.rtm_seq = ++rtm_seq;
    }
    ki_rtp->msghdr.rtm_pid = task_pid;
    ki_rtp->msghdr.rtm_errno = 0;


    while (TRUE) {
	int pri = 0;
	const char *sent = "SENT";

	if (!BIT_TEST(task_state, TASKS_TEST) && krt_install) {
	    errno = 0;
	    if (write(ki_tp->task_socket,
		      (caddr_t) &ki_rtp->msghdr,
		      (size_t) ki_rtp->msghdr.rtm_msglen) < 0) {
		ki_rtp->msghdr.rtm_errno = errno;
		trace(TR_ALL, LOG_ERR, "krt_send: write: %m");
		sent = "*NOT SENT*";
		pri = LOG_ERR;
	    }
	}

	if (pri || BIT_TEST(trace_flags, TR_KRT)) {
	    krt_trace(ki_tp,
		      sent,
		      &ki_rtp->msghdr,
		      (size_t) ki_rtp->msghdr.rtm_msglen,
		      (krt_addrinfo *) 0,
		      pri);
	    trace(TR_KRT, 0, NULL);
	}

	switch (ki_rtp->msghdr.rtm_errno) {
	case ENOBUFS:
	    /* Indicate request should be retried if the error is not fatal */
	    ki_rtp->msghdr.rtm_pid = 0;
	    break;

	case EEXIST:
	    /* What!  We must have missed a routing message. */
	    switch (ki_rtp->msghdr.rtm_type) {
	    case RTM_ADD:
		/* Try to make it look the way we want it to */
		ki_rtp->msghdr.rtm_type = RTM_CHANGE;
		continue;
	    }
	    ki_rtp->msghdr.rtm_errno = 0;
	    break;

	case ESRCH:
	    /* Asked to process a route that does not exist */
	    switch (ki_rtp->msghdr.rtm_type) {
	    case RTM_CHANGE:
		/* Change a change into an add */
		ki_rtp->msghdr.rtm_type = RTM_ADD;
		continue;

	    case RTM_DELETE:
		/* Same result, do we really care how it happened? */
		break;
	    }
	    ki_rtp->msghdr.rtm_errno = 0;
	    break;
	    
	case EDQUOT:
	    /* The new gateway address is longer than the old one. This */
	    /* can only happen when we changed an RTM_ADD into an RTM_CHANGE */
	    switch (ki_rtp->msghdr.rtm_type) {
	    case RTM_CHANGE:
		/* Do an ADD later */
		retry_type = RTM_ADD;
	    
		/* But a delete now */
		ki_rtp->msghdr.rtm_type = RTM_DELETE;
		ki_rtp->msghdr.rtm_errno = 0;
		continue;
	    }
	    /* Fall-Through */

	default:
	    /* Unknown error, just forget it (don't quit) */
	    ki_rtp->msghdr.rtm_errno = 0;
	    break;

	case ETOOMANYREFS:
	    break;
		
	case 0:
	    break;
	}

	if (!ki_rtp->msghdr.rtm_errno && retry_type) {
	    ki_rtp->msghdr.rtm_type = retry_type;
	    retry_type = 0;
	    continue;
	}

	break;
    }
    
    return ki_rtp->msghdr.rtm_errno;
}


/* Process as many requests on the queue as possible */
static inline int
krt_runqueue __PF1(tp, task *)
{
    struct rtm_block *rtb;
    struct rtm_msg *rtp;

    while ((rtb = rtm_head)
	   && ((rtp = rtb->rtb_first) < rtb->rtb_free)) {
	int error = krt_send(tp, rtp);

	if (error) {
	    timer_set(KRT_TIMER(tp),
		      krt_waittime,
		      (time_t) 0);
	    KRT_BUMP(krt_waittime);		/* Increase timeout value if it fails again */
	    return error;
	}

	/* Remove this element from the queue and free it */
	krt_msg_free(rtp);

	timer_reset(KRT_TIMER(tp));		
	KRT_RESET(krt_waittime);		/* Reset delay timer */
    }

    return 0;
}


/* Fill in a request and enqueue it */
int
krt_request __PF3(type, u_int,
		  rt, rt_entry *,
		  router, sockaddr_un *)
{
    int error = 0;
    struct rtm_msg *rtp;
    struct sockaddr *ap;
    struct sockaddr *s_dest = sock2unix(rt->rt_dest, (int *) 0);
    struct sockaddr *s_mask = (struct sockaddr *) 0;
    struct sockaddr *s_router = sock2unix(router, (int *) 0);

    if (BIT_TEST(trace_flags, TR_KRT)) {
	tracef("KERNEL %-6s %-15A",
	       trace_state(rtm_type_bits, type - 1),
	       rt->rt_dest);

	if (rt->rt_dest_mask) {
	    tracef(" mask %-15A",
		   rt->rt_dest_mask);
	}

	tracef(" gateway %-15A %s <%s>",
	       router,
	       trace_state(rt_proto_bits, rt->rt_gwp->gw_proto),
	       trace_bits(rt_state_bits, rt->rt_state));
    }

    if (krt_install) {
	flag_t flags = 0;
	size_t size = s_dest->sa_len;

	if (sockishost(rt->rt_dest, rt->rt_dest_mask)) {
	    BIT_SET(flags, RTF_HOST);
	} else if (rt->rt_dest_mask) {
	    s_mask = sock2unix(rt->rt_dest_mask, (int *) 0);
	    size += s_mask->sa_len;
	}
	
	switch (type) {
	case RTM_ADD:
	case RTM_CHANGE:
	    size += s_router->sa_len;
	    break;

	default:
	    s_router = (struct sockaddr *) 0;
	}

	/* Allocate a block and clear it */
	rtp = krt_msg_alloc(size);

	rtp->msghdr.rtm_type = type;
	rtp->msghdr.rtm_version = RTM_VERSION;
	rtp->msghdr.rtm_flags = flags | krt_state_to_flags(rt->rt_state);
	if (rt->rt_ifap
	    && BIT_TEST(rt->rt_ifap->ifa_state, IFS_UP)) {
	    BIT_SET(rtp->msghdr.rtm_flags, RTF_UP);
	}
#ifdef	KRT_IFREAD_KINFO
	if (rt->rt_gwp->gw_proto == RTPROTO_DIRECT
	    && rt->rt_ifap
	    && !BIT_TEST(rt->rt_state, RTS_REJECT)) {
	    /* Set interface specific flags */
	    rtp->msghdr.rtm_flags |= rt->rt_ifap->ifa_info.ifi_rtflags;
	}
#endif	/* KRT_IFREAD_KINFO */
#ifdef	RTF_DYNAMIC
	if (rt->rt_gwp->gw_proto == RTPROTO_REDIRECT) {
	    BIT_SET(rtp->msghdr.rtm_flags, RTF_DYNAMIC);
	}
#endif	/* RTF_DYNAMIC */

	rtp->msghdr.rtm_msglen = size;

	/* XXX - set metrics */

	ap = (struct sockaddr *) (rtp + 1);

	bcopy((caddr_t) s_dest, (caddr_t) ap, (size_t) s_dest->sa_len);
	RTM_ADDR(ap);
	BIT_SET(rtp->msghdr.rtm_addrs, RTA_DST);

	if (s_router) {
	    bcopy((caddr_t) s_router, (caddr_t) ap, (size_t) s_router->sa_len);
	    RTM_ADDR(ap);
	    BIT_SET(rtp->msghdr.rtm_addrs, RTA_GATEWAY);
	}

	/* Provide a mask if this is not a host route */
	if (s_mask) {
	    /* Convert our netmask format into the kernel's netmask format. */
	    /* The kernel does not want the address family nor trailing zeros. */
	    register byte *sp = (byte *) s_mask;
	    register byte *lp = (byte *) s_mask + s_mask->sa_len;
	    register byte *dp = (byte *) ap;
	    register byte *cp = (byte *) 0;

	    s_mask->sa_len = s_mask->sa_family = 0;	/* OK to write here, it's a scratch buffer */

	    /* Copy mask and keep track of last non-zero byte */
	    while (sp < lp) {
		if (*dp++ = *sp++) {
		    /* We actually point to the first byte after last zero byte */
		    cp = dp;
		}
	    }

	    if (cp <= (byte *) &ap->sa_family) {
		/* If the netmask is zero length, make sure there is at least a */
		/* long word of zeros present */
		*((u_long *) ap) = 0;
	    } else {
		ap->sa_len = cp - (byte *) ap;
	    }
		
	    RTM_ADDR(ap);
	    BIT_SET(rtp->msghdr.rtm_addrs, RTA_NETMASK);
	}

	rtp->msghdr.rtm_msglen = (caddr_t)ap - (caddr_t) &rtp->msghdr;

	/* If this is the first entry on the queue, run the queue */
	if (KRT_MSG_FIRST(rtp)) {
	    trace(TR_KRT, 0, NULL);
	    error = krt_runqueue(krt_task);
	} else {
	    trace(TR_KRT, 0, ": deferred");
	}
    } else {
	trace(TR_KRT, 0, NULL);
    }

    return error;
}


/* Process an information routing socket request */
static void
krt_recv_rtinfo __PF3(tp, task *,
		      rtp, struct rt_msghdr *,
		      adip, krt_addrinfo *)
{
    switch (rtp->rtm_type) {
    case RTM_LOSING:
	trace(TR_KRT, 0, "krt_recv: kernel reports TCP lossage on route to %A/%A via %A",
	      adip->rti_info[RTAX_DST],
	      adip->rti_info[RTAX_NETMASK],
	      adip->rti_info[RTAX_GATEWAY]);
	break;

    case RTM_MISS:
	trace(TR_KRT, 0, "krt_recv: kernel can not find route to %A/%A",
	       adip->rti_info[RTAX_DST],
	       adip->rti_info[RTAX_NETMASK]);
	break;
	
    case RTM_RESOLVE:
	trace(TR_KRT, 0, "krt_recv: kernel resolution request for %A/%A",
	       adip->rti_info[RTAX_DST],
	       adip->rti_info[RTAX_NETMASK]);
	break;

    default:
	assert(FALSE);
    }
}


/* Process a route related routing socket request */
static void
krt_recv_route __PF3(tp, task *,
		     rtp, struct rt_msghdr *,
		     adip, krt_addrinfo *)
{
#ifdef	IP_MULTICAST
    int mc = FALSE;
#endif	/* IP_MULTICAST */
    int delete = FALSE;
    sockaddr_un *author = (sockaddr_un *) 0;
    rt_entry *rt;
    rt_entry *prev_rt = (rt_entry *) 0;
    rt_parms rtparms;

    bzero((caddr_t) &rtparms, sizeof (rtparms));
    rtparms.rtp_n_gw = 1;

    switch (krt_rtaddrs(adip, &rtparms, &author, (flag_t) rtp->rtm_flags)) {
    case KRT_ADDR_OK:
	break;

    case KRT_ADDR_IGNORE:
	krt_trace_disp("krt_recv_route: ignoring",
		       adip,
		       (flag_t) rtp->rtm_flags,
		       0);
	return;

    case KRT_ADDR_BOGUS:
	krt_trace_disp("krt_recv_route: ignoring bogus",
		       adip,
		       (flag_t) rtp->rtm_flags,
		       0);
	return; /* XXX - goto Delete */;

#ifdef	IP_MULTICAST
    case KRT_ADDR_MC:
	mc = TRUE;
	break;
#endif	/* IP_MULTICAST */
    }

    if (rtp->rtm_pid == task_pid) {
	trace(TR_ALL, LOG_ERR, "krt_recv_route: received a response to my own request");
	return;
    }
    
    /* Ignore incomplete messages and responses in error */
    if (rtp->rtm_errno) {
	return;
    }

    switch (socktype(rtparms.rtp_dest)) {
#ifdef	PROTO_INET
    case AF_INET:
	BIT_SET(rtparms.rtp_state, RTS_NOAGE | RTS_RETAIN);
	    
	if (BIT_TEST(rtparms.rtp_state, RTS_GATEWAY)) {
	    rtparms.rtp_gwp = krt_gwp;
	    rtparms.rtp_preference = RTPREF_KERNEL;
	} else {
	    rtparms.rtp_gwp = int_rtparms.rtp_gwp;
	    rtparms.rtp_preference = RTPREF_DIRECT;
	}
	break;
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
    case AF_ISO:
	BIT_SET(rtparms.rtp_state, RTS_NOAGE | RTS_RETAIN);
	rtparms.rtp_preference = RTPREF_KERNEL;
	break;
#endif	/* PROTO_ISO */
    }

    /* XXX - This is a bloody mess!  REWRITE IT! */

#ifdef	IP_MULTICAST
    if (mc) {
	krt_multicast_change(rtp->rtm_type, &rtparms);
	return;
    }
#endif	/* IP_MULTICAST */
    
    switch (rtp->rtm_type) {
    case RTM_CHANGE:
	if (rtparms.rtp_dest_mask) {
	    /* Reno interface routes may come through with host bits set */
	    sockmask(rtparms.rtp_dest, rtparms.rtp_dest_mask);
	}

	rt_open(tp);

	if (rt = rt_locate(RTS_NETROUTE,
			   rtparms.rtp_dest,
			   rtparms.rtp_dest_mask,
			   RTPROTO_BIT_ANY)) {
	    /* There is a route */
	    
	    if (rt->rt_gwp == krt_gwp) {
		/* Assume it is intended as a static route now, not a remnant */

		BIT_RESET(rt->rt_state, RTS_NOADVISE|RTS_REJECT);
		BIT_SET(rt->rt_state, RTS_NOAGE|RTS_RETAIN|RTS_STATIC);
		rt->rt_state |= BIT_TEST(rtparms.rtp_state, RTS_REJECT);
		rt = rt_change(rt,
			       (metric_t) 0,
			       (metric_t) 0,
			       RTPREF_KERNEL,
			       1, &rtparms.rtp_router);
		if (!rt) {
		    trace(TR_ALL, LOG_ERR, "krt_recv_route: error changing route to %A/%A via %A",
			  rtparms.rtp_dest,
			  rtparms.rtp_dest_mask,
			  rtparms.rtp_router);
		}
		rt_close(tp, (gw_entry *) 0, 1, NULL);

		break;
	    } else if (rt == rt->rt_active) {
		/* Not ours, remember to flag the old one as not installed */
		prev_rt = rt;
	    }
	}
	/* Add a new route */
	goto add;

    case RTM_DELETE:
	if (!BIT_TEST(rtparms.rtp_state, RTS_GATEWAY)
	    && (rt = rt_locate(rtparms.rtp_state & RTS_NETROUTE,
			       rtparms.rtp_dest,
			       rtparms.rtp_dest_mask,
			       RTPROTO_BIT(RTPROTO_DIRECT)))
	    && rtbit_isset(rt, tp->task_rtbit)) {
	    /* Interface route being deleted */

	    rt_open(tp);
	    krt_not_installed(rt);
	    rt_close(tp, (gw_entry *) 0, 1, NULL);
	    break;
	}
	/* Fall thru */

    case RTM_OLDDEL:
	delete = TRUE;
	goto common;

    case RTM_ADD:
	if (!BIT_TEST(rtparms.rtp_state, RTS_GATEWAY)
	    && (rt = rt_locate(rtparms.rtp_state & RTS_NETROUTE,
			       rtparms.rtp_dest,
			       rtparms.rtp_dest_mask,
			       RTPROTO_BIT(RTPROTO_DIRECT)))
	    && !rtbit_isset(rt, tp->task_rtbit)) {
	    /* Interface route being added */

	    rt_open(tp);
	    krt_installed(rt);
	    rt_close(tp, (gw_entry *) 0, 1, NULL);
	    break;
	}
	/* Fall thru */

    case RTM_OLDADD:
	/* Fall thru */
	    
    common:
	/* See if we have any new interfaces that we do not know about */
	/* This should not be necessary on systems with RTM_{NEWADDR,DELADDR,IFINFO}, */
	/* but due to a bug in BSD 4.4 Alpha we may not see all changes in interface */
	/* state. */
	if_check();

	if (rtparms.rtp_dest_mask) {
	    /* Reno interface routes may come through with host bits set */
	    sockmask(rtparms.rtp_dest, rtparms.rtp_dest_mask);
	}

#ifdef	notdef
	/* Verify that the interface exists */
	if (!if_withroute(rtparms.rtp_dest,
			  rtparms.rtp_router,
			  rtparms.rtp_state)) {
	    /* Can not find interface, scan the interface */
	    /* list to see if there is something we don't */
	    /* know about */
	    
	    if_check();
	    
	    /* The rt_add() will fail if the interface is still unknown */
	}
#endif	/* notdef */

	rt_open(tp);
	    
	/* Delete existing route */
	rt = rt_locate(RTS_NETROUTE,
		       rtparms.rtp_dest,
		       rtparms.rtp_dest_mask,
		       RTPROTO_BIT_ANY);
	if (rt && !BIT_TEST(rt->rt_state, RTS_DELETE)) {
	    switch (rt->rt_gwp->gw_proto) {
	    case RTPROTO_DIRECT:
		break;
		
	    case RTPROTO_REDIRECT:
	    case RTPROTO_KERNEL:
		if (rt == rt->rt_active) {
		    /* Indicate it is not installed in the kernel */
		    rt = krt_not_installed(rt);
		}
		if (rt) {
		    rt_delete(rt);
		}
		break;

	    default:
		if (rt == rt->rt_active && !BIT_TEST(rt->rt_state, RTS_NOTINSTALL)) {
		    /* Someone is fucking with an installed active route, put it back */
		    krt_change((rt_entry *) 0, (sockaddr_un *) 0, rt);
		}
	    }
	}

	if (!delete) {
	add:
	    /* Add new route */
	    rt = rt_add(&rtparms);
	    if (rt) {
		if (prev_rt && prev_rt != rt->rt_active) {
		    /* Mark previous route as not installed */

		    krt_not_installed(prev_rt);
		}
		if (rt == rt->rt_active) {
		    krt_installed(rt);
		    goto close;
		} else {
		    trace(TR_ALL, LOG_ERR, "krt_recv_route: %s route not active %A/%A via %A flags %s",
			  trace_state(rt_proto_bits, rtparms.rtp_gwp->gw_proto),
			  rtparms.rtp_dest,
			  rtparms.rtp_dest_mask,
			  rtparms.rtp_router,
			  trace_bits(krt_flag_bits, (flag_t) rtp->rtm_flags));
		}	
	    } else {
		/* Could not install route */
		trace(TR_ALL, LOG_ERR, "krt_recv_route: error adding route to %A/%A via %A - deleting",
		      rtparms.rtp_dest,
		      rtparms.rtp_dest_mask,
		      rtparms.rtp_router);
	    }

	    /* Delete bogus route */
	    krt_delete_dst(tp,
			   rt ? rt->rt_active : (rt_entry *) 0,
			   &rtparms,
			   author,
			   RTPROTO_KERNEL,
			   &krt_gw_list);
	}

    close:
	rt_close(tp, (gw_entry *) 0, 1, NULL);
	break;

    case RTM_REDIRECT:
	tracef("krt_recv_route: redirect to %A",
	       rtparms.rtp_dest);
	if (rtparms.rtp_dest_mask) {
	    tracef("/%A ",
		   rtparms.rtp_dest_mask);
	}
	trace(TR_KRT, 0, "via %A from %A",
	      rtparms.rtp_router,
	      author);
	redirect(tp,
		 rtparms.rtp_dest,
		 rtparms.rtp_dest_mask,
		 rtparms.rtp_router,
		 author,
		 BIT_TEST(rtp->rtm_flags, RTF_HOST) ? TRUE : FALSE);
	break;

    default:
	assert(FALSE);
    }
}


/* Process a route socket response from the kernel */
void
krt_recv __PF1(tp, task *)
{
    int n_packets = TASK_PACKET_LIMIT;
    size_t size;

    while (n_packets-- && !task_receive_packet(tp, &size)) {
	struct rt_msghdr *rtp = task_get_recv_buffer(struct rt_msghdr *);
	krt_addrinfo *adip;

	adip = krt_xaddrs(rtp, size);
	if (!adip) {
	    continue;
	}

	TRACE_ON(TR_KRT) {
	    krt_trace(tp,
		      "RECV",
		      rtp,
		      size,
		      adip,
		      LOG_DEBUG);
	}

	switch (rtp->rtm_type) {
	case RTM_ADD:
	case RTM_DELETE:
	case RTM_CHANGE:
	case RTM_OLDADD:
	case RTM_OLDDEL:
	    krt_recv_route(tp, rtp, adip);
	    break;

	case RTM_LOSING:
	case RTM_MISS:
	case RTM_RESOLVE:
	    krt_recv_rtinfo(tp, rtp, adip);
	    break;

	case RTM_GET:
	case RTM_LOCK:
	    break;
	    
#ifdef	KRT_IFREAD_KINFO
	case RTM_NEWADDR:
	case RTM_DELADDR:
	    {
		struct ifa_msghdr *ifap = (struct ifa_msghdr *) rtp;
		if_link *ifl = ifl_locate_index(ifap->ifam_index);

		if_conf_open(tp, FALSE);
		krt_ifaddr(tp,
			   (struct ifa_msghdr *) rtp,
			   adip,
			   ifl);
		if_conf_close(tp);
	    }
	    break;
	    
	case RTM_IFINFO:
            {
		struct if_msghdr *ifap = (struct if_msghdr *) rtp;
		sockaddr_un *ap = adip->rti_info[RTAX_IFP];

		if_conf_open(tp, FALSE);
		(void) ifl_addup(tp,
				 ifap->ifm_index,
				 krt_if_flags(ifap->ifm_flags),
				 ifap->ifm_data.ifi_metric,
				 ifap->ifm_data.ifi_mtu,
				 ap->dl.sdl_data,
				 ap->dl.sdl_nlen,
				 sockbuild_ll(krt_type_to_ll(ap->dl.sdl_type),
					      (byte *) (ap->dl.sdl_data + ap->dl.sdl_nlen),
					      ap->dl.sdl_alen));
		if_conf_close(tp);
	    }
	    break;
#endif	/* KRT_IFREAD_KINFO */

	default:
	    break;
	}

	trace(TR_KRT, 0, NULL);
    }
}


/* Retry a kernel socket request, it failed the first time */
/*ARGSUSED*/
void
krt_timeout __PF2(tip, timer *,
		  interval, time_t)
{
    krt_runqueue(tip->timer_task);
}


void
krt_rtsock_dump __PF2(tp, task *,
		      fd, FILE *)
{
    struct rtm_msg *rtp;

    (void) fprintf(fd, "\tRouting socket queue:\n");

    (void) fprintf(fd, "\t\tSequence:\t%d\n",
		   rtm_seq);

    RTM_LIST(rtp) {
	krt_addrinfo *adip = krt_xaddrs(&rtp->msghdr,
					(size_t) rtp->msghdr.rtm_msglen);

	if (!adip) {
	    /* Not parsable */
	    continue;
	}
	
	(void) fprintf(fd, "\t\t\tlength %u  version %u  type %s(%u)  addrs %s(%x)\n",
		       rtp->msghdr.rtm_msglen,
		       rtp->msghdr.rtm_version,
		       trace_state(rtm_type_bits, rtp->msghdr.rtm_type - 1),
		       rtp->msghdr.rtm_type,
		       trace_bits(rtm_sock_bits, (flag_t) rtp->msghdr.rtm_addrs),
		       rtp->msghdr.rtm_addrs);
	(void) fprintf(fd, "\t\t\tpid %d  seq %d  error %d",
		       rtp->msghdr.rtm_pid,
		       rtp->msghdr.rtm_seq,
		       rtp->msghdr.rtm_errno);
	if (rtp->msghdr.rtm_errno) {
	    errno = rtp->msghdr.rtm_errno;
	    (void) fprintf(fd, ": %m\n");
	} else {
	    (void) fprintf(fd, "\n");
	}

	(void) fprintf(fd, "\t\t\tflags %s(%x)",
		       trace_bits(krt_flag_bits, (flag_t) rtp->msghdr.rtm_flags),
		       rtp->msghdr.rtm_flags & 0xffff);
	if (rtp->msghdr.rtm_rmx.rmx_locks) {
	    (void) fprintf(fd, "  locks %s(%x)",
			   trace_bits(rtm_lock_bits, rtp->msghdr.rtm_rmx.rmx_locks),
			   rtp->msghdr.rtm_rmx.rmx_locks);
	}
	if (rtp->msghdr.rtm_inits) {
	    (void) fprintf(fd, "  inits <%s>(%x)",
			   trace_bits(rtm_lock_bits, rtp->msghdr.rtm_inits),
			   rtp->msghdr.rtm_inits);
	}
	(void) fprintf(fd, "\n");	

	/* Display metrics */
	switch (rtp->msghdr.rtm_type) {
	case RTM_ADD:
	case RTM_CHANGE:
	case RTM_GET:
	case RTM_LOCK:
	    if (BIT_TEST(rtp->msghdr.rtm_rmx.rmx_locks|rtp->msghdr.rtm_inits, RTV_MTU|RTV_HOPCOUNT|RTV_EXPIRE|RTV_SSTHRESH)) {
		(void) fprintf(fd, "\t\t\tmtu %u  hopcount %u  expire %u  ssthresh %u\n",
			       rtp->msghdr.rtm_rmx.rmx_mtu,
			       rtp->msghdr.rtm_rmx.rmx_hopcount,
			       rtp->msghdr.rtm_rmx.rmx_expire,
			       rtp->msghdr.rtm_rmx.rmx_ssthresh);
	    }
	    if (BIT_TEST(rtp->msghdr.rtm_rmx.rmx_locks|rtp->msghdr.rtm_inits, RTV_RPIPE|RTV_SPIPE|RTV_RTT|RTV_RTTVAR)) {
		(void) fprintf(fd, "\t\t\trecvpipe %u  sendpipe %u  rtt %u  rttvar %u\n",
			       rtp->msghdr.rtm_rmx.rmx_recvpipe,
			       rtp->msghdr.rtm_rmx.rmx_sendpipe,
			       rtp->msghdr.rtm_rmx.rmx_rtt,
			       rtp->msghdr.rtm_rmx.rmx_rttvar);
	    }
	    break;
	}

	/* Display addresses */
	if (adip->rti_addrs) {
	    register int i;

	    fprintf(fd, "\t\t\t");

	    RTAX_LIST(i) {
		register sockaddr_un *ap = adip->rti_info[i];

		if (ap) {
		    fprintf(fd, " %s %A",
			    gd_lower(trace_state(rtm_sock_bits, i)),
			    ap);
		}
	    } RTAX_LIST_END(i);
	}	

	(void) fprintf(fd, "\n\n");
    } RTM_LIST_END(rtp) ;
}
