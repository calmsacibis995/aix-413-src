static char sccsid[] = "@(#)75	1.1  src/tcpip/usr/sbin/gated/krt_ifread_ioctl.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:21";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
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
 *  krt_ifread_ioctl.c,v 1.4.2.2 1993/08/27 22:28:19 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_IOCTL
#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"
#include "krt_var.h"


/* Make some assumptions about interface MTU */
static inline mtu_t
krt_mtu __PF1(state, flag_t)
{
    switch (BIT_TEST(state, IFS_POINTOPOINT|IFS_LOOPBACK|IFS_BROADCAST)) {
    case IFS_POINTOPOINT:
	/* Compressed SLIP interfaces use 256, it can't really hurt to specify it */
	/* too low. */
	return 256;

    case IFS_LOOPBACK:
	/* Loopback interfaces use more */
	return 1536;
	
    case IFS_BROADCAST:
	/* Assume an Ethernet */
	return 1500;
    }

    /* A safe assumption? */
    return 576;
}


int
krt_ifread __PF1(save_task_state, flag_t)
{
#ifndef	SOCKADDR_DL
    u_int indx = 0;
#endif	/* SOCKADDR_DL */
    caddr_t limit;
    struct ifreq *ifrp;
    if_link *ifl = (if_link *) 0;
#ifdef	PROTO_ISO
    static struct iso_ifreq iso_ifreq;
    static struct ifreq *ifr = (struct ifreq *) &iso_ifreq;
#else	/* PROTO_ISO */
    static struct ifreq ifreq;
    static struct ifreq *ifr = &ifreq;
#endif	/* PROTO_ISO */
    register task *tp = krt_task;
    static int s_in = -1;
#ifdef	ISOPROTO_RAW
    static int s_iso = -1;
#endif	/* ISOPROTO_RAW */

    if (tp->task_socket < 0) {
	return EBADF;
    }

    do {
	if (task_send_buffer) {
	    int rc;

#ifdef	USE_STREAMIO
	    struct strioctl si;
 
	    si.ic_cmd = SIOCGIFCONF;
	    si.ic_timout = 0;
	    si.ic_len = task_send_buffer_len;
	    si.ic_dp = (caddr_t) task_send_buffer;
    
	    NON_INTR(rc, ioctl(tp->task_socket, I_STR, &si));

	    limit = si.ic_dp + si.ic_len;

#else	/* USE_STREAMIO */
	    static struct ifconf ifconf_req;

	    ifconf_req.ifc_buf = (caddr_t) task_send_buffer;
	    ifconf_req.ifc_len = task_send_buffer_len;

	    rc = task_ioctl(tp->task_socket,
			    SIOCGIFCONF,
			    (void_t) &ifconf_req,
			    ifconf_req.ifc_len);

	    limit = ifconf_req.ifc_buf + ifconf_req.ifc_len;

#endif	/* USE_STREAMIO */
	    if (rc == 0) {
		break;
	    } else if (errno != EFAULT) {
		/* Somethings wrong */

		trace(TR_ALL, LOG_ERR, "krt_ifread: ioctl SIOCGIFCONF: %m");
		return errno;
	    }
	}

	/* Increase buffer size */
	task_alloc_send(krt_task, task_send_buffer_len ? task_send_buffer_len * 2 : task_pagesize);
    } while (TRUE) ;

    /* Tell the IF code that we are passing complete knowledge */
    if_conf_open(tp, TRUE);
    
#define	ifrpsize(x) ((unix_socksize(&(x)->ifr_addr) > sizeof((x)->ifr_addr)) ? \
    	sizeof(*(x)) + unix_socksize(&(x)->ifr_addr) - sizeof((x)->ifr_addr) : sizeof(*(x)))

    for (ifrp = (struct ifreq *) task_send_buffer;
	 (caddr_t) ifrp < limit;
	 ifrp = (struct ifreq *) ((void_t) ((caddr_t) ifrp + ifrpsize(ifrp)))) {
	if_info ifi;
	int s = -1;
	u_int siocgifdstaddr = 0;
#ifdef	SIOCGIFNETMASK
	u_int siocgifnetmask = 0;
#endif	/* SIOCGIFNETMASK */
	u_int siocgifbrdaddr = 0;
	char if_name[IFNAMSIZ+1];

	bzero((caddr_t) &ifi, sizeof (ifi));

	bcopy(ifrp->ifr_name, if_name, IFNAMSIZ);
	if_name[IFNAMSIZ] = (char) 0;
#ifdef	SUNOS5_0
        {
	    register char *cp = index(ifrp->ifr_name, ':');

	    /* Remove the :n extension from the name */
	    if (cp) {
		*cp = (char) 0;
	    }
	}
#endif	/* SUNOS5_0 */

	if (
#ifdef	SOCKADDR_DL
	    ifrp->ifr_addr.sa_family == AF_LINK
#else	/* SOCKADDR_DL */
	    !ifl || strncmp(if_name, ifl->ifl_name, IFNAMSIZ)
#endif	/* SOCKADDR_DL */
	    ) {
	    flag_t state;
	    metric_t metric;
	    mtu_t mtu;

	    /* And save for ioctls */
	    (void) strncpy(ifr->ifr_name, ifrp->ifr_name, IFNAMSIZ);

#ifdef	SUNOS5_0
	    /* State, MTU and metric are associated with addresses, not the interface */
	    state = mtu = metric = 0;

#else	/* SUNOS5_0 */
	    /* Get interface flags */
	    bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	    if (task_ioctl(tp->task_socket, SIOCGIFFLAGS, (char *) ifr, sizeof (*ifr)) < 0) {
		trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOCGIFFLAGS: %m",
		      ifr->ifr_name);
		goto Continue;
	    }
	    state = krt_if_flags(ifr->ifr_flags);

	    /* Get a resonable default */
	    mtu = krt_mtu(state);

#ifdef	SIOCGIFMTU
#ifndef	ifr_mtu
#define	ifr_mtu	ifr_metric
#endif
	    /* Get interface MTU */
	    bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	    if (task_ioctl(tp->task_socket, SIOCGIFMTU, (char *) ifr, sizeof (*ifr)) < 0) {
		trace(0, 0, "krt_ifread: %s: ioctl SIOCGIFMTU: %m",
		      ifr->ifr_name);
	    } else {
		mtu = ifr->ifr_mtu;
	    }
#endif	/* SIOCGIFMTU */

	    /* Get interface metric */
#if	defined(SIOCGIFMETRIC) && defined(ifr_metric)
	    bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	    if (task_ioctl(tp->task_socket, SIOCGIFMETRIC, (caddr_t) ifr, sizeof (*ifr)) < 0) {
		trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOCGIFMETRIC: %m",
		      ifr->ifr_name);
		goto Continue;
	    }
	    metric = (ifr->ifr_metric >= 0) ? ifr->ifr_metric : 0;
#else	/* defined(SIOCGIFMETRIC) && defined(ifr_metric) */
	    metric = 0;
#endif	/* defined(SIOCGIFMETRIC) && defined(ifr_metric) */
#endif	/* SUNOS5_0 */


#ifdef	SOCKADDR_DL
	    {
		struct sockaddr_dl *sdl = (struct sockaddr_dl *) &ifrp->ifr_addr;
		sockaddr_un *lladdr;

		if (sdl->sdl_alen) {
		    /* We have an address */

		    lladdr = sockbuild_ll(krt_type_to_ll(sdl->sdl_type),
					  (byte *) (sdl->sdl_data + sdl->sdl_nlen),
					  (size_t) sdl->sdl_alen);
		} else {
		    /* This system may not supply link-level  addresses, */
		    /* or this interface may not have one.  Just to be sure */
		    /* try to look one up */

		    lladdr = krt_lladdr(ifr);
		}

		ifl = ifl_addup(krt_task,
				sdl->sdl_index,
				state,
				metric,
				mtu,
				sdl->sdl_data,
				(size_t) sdl->sdl_nlen,
				lladdr);
	    }
#else	/* SOCKADDR_DL */
	    ifl = ifl_addup(krt_task,
			    ++indx,
			    state,
			    metric,
			    mtu,
			    if_name,
			    (size_t) strlen(if_name),
			    krt_lladdr(ifr));
#endif	/* SOCKADDR_DL */
	}


	bzero((caddr_t) &ifi, sizeof (ifi));
	ifi.ifi_link = ifl;

	/* Reject unknown types */
	switch (ifrp->ifr_addr.sa_family) {
#ifdef	PROTO_INET
	case AF_INET:
	    break;
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
	case AF_ISO:
	    break;
#endif	/* PROTO_ISO */

	default:
	    /* Bogus address */
	    goto Continue;
	}
	    
	/* Copy the interface address */
	ifi.ifi_addr_local = ifi.ifi_addr = sockdup(sock2gated(&ifrp->ifr_addr, unix_socksize(&ifrp->ifr_addr)));

#ifdef	SUNOS5_0
	/* Get interface flags */
	bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	if (task_ioctl(tp->task_socket, SIOCGIFFLAGS, (char *) ifr, sizeof (*ifr)) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOCGIFFLAGS: %m",
		  ifr->ifr_name);
	    goto Continue;
	}
	ifi.ifi_state = krt_if_flags(ifr->ifr_flags);
	if (BIT_TEST(ifi.ifi_state, IFS_LOOPBACK)) {
	    /* Indicate this is a loopback interface */

	    BIT_SET(ifl->ifl_state, IFS_LOOPBACK);
	}

	/* Get interface MTU */
#ifndef	ifr_mtu
#define	ifr_mtu	ifr_metric
#endif
	bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	if (task_ioctl(tp->task_socket, SIOCGIFMTU, (char *) ifr, sizeof (*ifr)) < 0) {
	    trace(0, 0, "krt_ifread: %s: ioctl SIOCGIFMTU: %m",
		  ifr->ifr_name);

	    /* Figure out a default */
	    ifi.ifi_mtu = krt_mtu(ifi.ifi_state);
	} else {
	    ifi.ifi_mtu = ifr->ifr_mtu;
	}

	/* Get interface metric */
	bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	if (task_ioctl(tp->task_socket, SIOCGIFMETRIC, (caddr_t) ifr, sizeof (*ifr)) < 0) {
	    trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOCGIFMETRIC: %m",
		  ifr->ifr_name);
	    goto Continue;
	}
	ifi.ifi_metric = (ifr->ifr_metric >= 0) ? ifr->ifr_metric : 0;
#else	/* SUNOS5_0 */
	/* Inherit parameters from physical interface */

	ifi.ifi_state = ifl->ifl_state;
	ifi.ifi_metric = ifl->ifl_metric;
	ifi.ifi_mtu = ifl->ifl_mtu;
#endif	/* SUNOS5_0 */

	/* What we do next depends on the family */
	switch (ifrp->ifr_addr.sa_family) {
#ifdef	PROTO_INET
	case AF_INET:
	    /* Specify the right socket for this family */
	    if (s_in < 0) {
		BIT_RESET(task_state, TASKS_INIT|TASKS_TEST);
		s_in = task_get_socket(PF_INET, SOCK_DGRAM, 0);
		task_state = save_task_state;
	    }
	    s = s_in;
	    if (BIT_TEST(ifi.ifi_state, IFS_POINTOPOINT)) {
		siocgifdstaddr = SIOCGIFDSTADDR;
	    }
#ifdef	SIOCGIFNETMASK
	    if (!BIT_TEST(ifi.ifi_state, IFS_LOOPBACK)) {
		siocgifnetmask = SIOCGIFNETMASK;
#endif	/* SIOCGIFNETMASK */
	    }
	    if (BIT_TEST(ifi.ifi_state, IFS_BROADCAST)) {
		siocgifbrdaddr = SIOCGIFBRDADDR;
	    }
	    break;
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
	case AF_ISO:
	    /* Specify the right socket for this family */
	    if (s_iso < 0) {
		BIT_RESET(task_state, TASKS_INIT|TASKS_TEST);
		s_iso = task_get_socket(PF_ISO, SOCK_DGRAM, 0);
		task_state = save_task_state;
	    }
	    s = s_iso;
	    if (BIT_TEST(ifi.ifi_state, IFS_POINTOPOINT)) {
		siocgifdstaddr = SIOCGIFDSTADDR_ISO;
	    } else if (!BIT_TEST(ifi.ifi_state, IFS_LOOPBACK)) {
		siocgifnetmask = SIOCGIFNETMASK_ISO;
	    }
	    break;
#endif	/* PROTO_ISO */

	default:
	    goto Continue;
	}

	if (siocgifdstaddr) {
	    /* Get the destination address for point-to-point interfaces */

	    bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	    if (task_ioctl(s, siocgifdstaddr, (caddr_t) ifr, sizeof (*ifr)) < 0) {
		trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOCGIFDSTADDR: %m",
		      ifr->ifr_name);
		goto Continue;
	    }
	    if (ifr->ifr_dstaddr.sa_family == AF_UNSPEC
		|| !unix_socksize(&ifr->ifr_dstaddr)
		|| !(ifi.ifi_addr = sock2gated(&ifr->ifr_dstaddr, unix_socksize(&ifr->ifr_dstaddr)))) {
		trace(TR_ALL, 0, "krt_ifread: no dest address for %A (%s)",
		      ifi.ifi_addr_local,
		      ifrp->ifr_name);
		goto Continue;
	    } else {
		ifi.ifi_addr = sockdup(ifi.ifi_addr);
	    }
	}
#ifdef	SIOCGIFNETMASK
	if (siocgifnetmask) {
	    bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	    if (task_ioctl(s, siocgifnetmask, (caddr_t) ifr, sizeof (*ifr)) < 0) {
		trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOCGIFNETMASK: %m",
		      ifr->ifr_name);
		goto Continue;
	    }
#ifdef	SOCKET_LENGTHS
	    /* A zero mask would have a length of zero */
	    if (ifr->ifr_addr.sa_len < 2) {
		ifr->ifr_addr.sa_len = 2;	/* Enough for address and family */
	    }
	    /* Masks don't have an address family specified */
	    if (ifr->ifr_addr.sa_family == AF_UNSPEC) {
		ifr->ifr_addr.sa_family = socktype(ifi.ifi_addr);
	    }
#endif	/* SOCKET_LENGTHS */
	    /* Convert the mask */
	    ifi.ifi_subnetmask = sock2gated(&ifr->ifr_addr, unix_socksize(&ifr->ifr_addr));

	    if (ifi.ifi_subnetmask) {
		/* We have a mask, get pointer to right one */
		ifi.ifi_subnetmask = mask_locate(ifi.ifi_subnetmask);
	    }
	}
#else	/* SIOCGIFNETMASK */
	ifi.ifi_subnetmask = (sockaddr_un *) 0;
#endif	/* SIOCGIFNETMASK */

	/* Get the broadcast address for broadcast interfaces */
	if (siocgifbrdaddr) {
#ifdef SIOCGIFBRDADDR
/* Some systems (SunOS 3.x where x > 2) do not define ifr_broadaddr */
#ifndef	ifr_broadaddr
#define	ifr_broadaddr	ifr_addr
#endif	/* ifr_broadaddr */
	    bzero ((caddr_t) &ifr->ifr_ifru, sizeof (ifr->ifr_ifru));
	    if (task_ioctl(s, siocgifbrdaddr, (caddr_t) ifr, sizeof (*ifr)) < 0) {
		trace(TR_ALL, LOG_ERR, "krt_ifread: %s: ioctl SIOGIFBRDADDR: %m",
		      ifr->ifr_name);
		goto Continue;
	    }
	    ifi.ifi_addr_broadcast = sock2gated(&ifr->ifr_broadaddr, unix_socksize(&ifr->ifr_broadaddr));
	    if (ifi.ifi_addr_broadcast) {
		ifi.ifi_addr_broadcast = sockdup(ifi.ifi_addr_broadcast);
	    } else {
		trace(TR_ALL, LOG_ERR, "krt_ifread: no broadcast address for %A (%s)",
		      ifi.ifi_addr_local,
		      ifrp->ifr_name);
		goto Continue;
	    }
#else	/* !SIOCGIFBRDADDR */
	    /* Assume a 4.2 based system with a zeros broadcast */
	    ifi.ifi_addr_broadcast = (sockaddr_un *) 0;
#endif	/* SIOCGIFBRDADDR */
	}

	/* And add the interface */
	if_conf_addaddr(tp, &ifi);
	continue;

    Continue:
	/* Free any allocated addresses */
	ifi_addr_free(&ifi);
    }

    if_conf_close(tp);

    return 0;
}
