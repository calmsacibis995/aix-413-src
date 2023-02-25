/* @(#)08	1.5  src/tcpip/usr/sbin/snmpd/routes.h, snmp, tcpip411, GOLD410 2/22/94 11:29:05 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/routes.h
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* routes.h - support for MIB support of the routing tables */

#ifndef _ROUTES_H_
#define _ROUTES_H_

#include <sys/mbuf.h>
#include <net/route.h>

#ifdef RTM_ADD

/* use bsd4.3 compatible ioctl's for sets */
#define RTENTRY ortentry
#else
#define RTENTRY rtentry
#endif

/*  */

#define	METRIC_NONE	(-1)			/* ipRouteMetric[1234] */

#define	RTYPE_OTHER	1			/* ipRouteType */
#define RTYPE_INVALID	2
#define	RTYPE_DIRECT	3
#define	RTYPE_REMOTE	4
#define RTYPE_MIN	1
#define RTYPE_MAX	4

#define	PROTO_OTHER	1			/* ipRouteProto */
#define	PROTO_ICMP	4
#define	PROTO_ESIS	10

/*  */

struct rtetab {
#define	RT_SIZE		20			/* object instance */
    unsigned int   rt_instance[RT_SIZE + 1];
    int	    rt_insize;

    int     rt_touched;
    int     rt_type;
    int     rt_host;

    int	    rt_magic;				/* for multiple routes to the
						   same destination */

    struct rtentry rt_rt;			/* from kernel */

    union sockaddr_un rt_dst;			/* key */
    union sockaddr_un rt_gateway;		/* value */
#ifdef BSD44
    struct sockaddr *rt_rmsa;			/* route mask */
#endif

    struct RTENTRY rt_srt;			/* set support */
    int rt_sflags;				/*     ...     */
    struct settab rt_stab;			/* set support */
    struct rtetab *rt_snext;			/*     ...     */

    struct rtetab *rt_next;
};

extern struct rtetab *rts_inet;
#ifdef	BSD44
extern struct rtetab *rts_iso;
#endif


int	get_routes ();
struct rtetab *get_rtent ();

#endif /* _ROUTES_H_ */
