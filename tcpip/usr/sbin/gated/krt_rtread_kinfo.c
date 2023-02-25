static char sccsid[] = "@(#)79	1.1  src/tcpip/usr/sbin/gated/krt_rtread_kinfo.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:29";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
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
 *  krt_rtread_kinfo.c,v 1.2 1993/04/05 04:14:05 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_KINFO
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


/* Use the getkinfo() system call to read the routing table(s) */
/*ARGSUSED*/
int
krt_rtread __PF0(void)
{
    size_t size, alloc_size;
    caddr_t kbuf, cp, limit;
    rt_parms rtparms;
    struct rt_msghdr *rtp;
#ifdef	USE_SYSCTL
    static int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_DUMP, 0 };
#endif	/* USE_SYSCTL */

    trace(TR_KRT, 0, NULL);
    trace(TR_KRT, 0, "krt_rtread: Initial routes read from kernel (getkerninfo):");

    bzero((caddr_t) &rtparms, sizeof (rtparms));
    rtparms.rtp_n_gw = 1;

    if (
#ifdef	USE_SYSCTL
	sysctl(mib, sizeof mib / sizeof *mib, (caddr_t) 0, &size, NULL, 0)
#else	/* USE_SYSCTL */
	(int) (alloc_size = getkerninfo(KINFO_RT_DUMP, (caddr_t) 0, (int *) 0, 0))
#endif	/* USE_SYSCTL */
	< 0) {
	trace(TR_ALL, LOG_ERR, "krt_rtread: getkerninfo/sysctl routing table estimate: %m");
	return errno;
    }
    if (BIT_TEST(trace_flags, TR_PROTOCOL)) {
	trace(TR_KRT, 0, "krt_rtread: getkerninfo/sysctl estimates %d bytes needed",
	      alloc_size);
    }
    size = alloc_size = ROUNDUP(alloc_size, task_pagesize);
    kbuf = (caddr_t) task_block_malloc(alloc_size);
    if (
#ifdef	USE_SYSCTL
	sysctl(mib, sizeof mib / sizeof *mib, kbuf, &size, NULL, 0)
#else	/* USE_SYSCTL */
	getkerninfo(KINFO_RT_DUMP, kbuf, (int *) &size, 0)
#endif	/* USE_SYSCTL */
	< 0) {
	trace(TR_ALL, LOG_ERR, "krt_rtread: getkerninfo/sysctl routing table retrieve: %m");
	return errno;
    }
    limit = kbuf + size;

    for (cp = kbuf; cp < limit; cp += rtp->rtm_msglen) {
	sockaddr_un *author;
	krt_addrinfo *adip;

	rtp = (struct rt_msghdr *) ((void_t) cp);

	adip = krt_xaddrs(rtp,
			  (size_t) rtp->rtm_msglen);
	if (!adip) {
	    continue;
	}

	TRACE_ON(TR_KRT) {
	    krt_trace(krt_task,
		      "RTINFO",
		      rtp,
		      (size_t) rtp->rtm_msglen,
		      adip,
		      0);
	}

	switch (krt_rtaddrs(adip, &rtparms, &author, (flag_t) rtp->rtm_flags)) {
	case KRT_ADDR_OK:
	    break;

	case KRT_ADDR_IGNORE:
	    krt_trace_disp("krt_rtread: ignoring",
			   adip,
			   (flag_t) rtp->rtm_flags,
			   0);
	    goto Continue;

	case KRT_ADDR_BOGUS:
	    krt_trace_disp("krt_rtread: deleting bogus",
			   adip,
			   (flag_t) rtp->rtm_flags,
			   0);
	    goto Delete;

#ifdef	IP_MULTICAST
	case KRT_ADDR_MC:
	    if (krt_multicast_install(rtparms.rtp_dest, rtparms.rtp_router)) {
		krt_trace_disp("krt_rtread: deleting multicast",
			       adip,
			       (flag_t) rtp->rtm_flags,
			       0);
		goto Delete;
	    }
	    krt_trace_disp("krt_rtread: ignoring multicast",
			   adip,
			   (flag_t) rtp->rtm_flags,
			   0);
	    goto Continue;
#endif	/* IP_MULTICAST */
	}

	if (!krt_rtadd(&rtparms, rtp->rtm_flags)) {
	    /* We don't want it around */

	    krt_trace_disp("krt_rtread: deleting unusable",
			   adip,
			   (flag_t) rtp->rtm_flags,
			   0);

	Delete:
	    krt_delete_dst(krt_task,
			   (rt_entry *) 0,
			   &rtparms,
			   author,
			   RTPROTO_KERNEL,
			   &krt_gw_list);
	}

    Continue:
	trace(TR_KRT, 0, NULL);
    }

    task_block_reclaim(alloc_size, kbuf);

    return 0;
}



