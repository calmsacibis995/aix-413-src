static char sccsid[] = "@(#)52	1.5  src/tcpip/usr/sbin/gated/if_rt.c, tcprouting, tcpip411, GOLD410 12/6/93 17:48:42";
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
 *  if_rt.c,v 1.19 1993/04/05 04:28:51 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

/*
 *  Routines for handling routes to interfaces
 */

#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"

/*
 *	Add routes for an interface.
 */
static void
if_rtadd __PF1(ifap, if_addr *)
{
#ifndef	ROUTE_AGGREGATION
    rt_entry *rt;
#endif	/* ROUTE_AGGREGATION */

    switch (socktype(ifap->ifa_addr)) {
#ifdef	PROTO_ISO
    case AF_ISO:
	return;
#endif	/* PROTO_ISO */

#ifdef	PROTO_INET
    case AF_INET:
#endif	/* PROTO_INET */
    default:
	assert(!ifap->ifa_rt);
	break;
    }

    trace(TR_INT, 0, "if_rtadd: ADD route for interface %s %A/%A",
	  ifap->ifa_name,
	  ifap->ifa_addr,
	  ifap->ifa_subnetmask);

    int_rtparms.rtp_router = ifap->ifa_addr;
    int_rtparms.rtp_state = RTS_NOAGE|RTS_RETAIN;
    int_rtparms.rtp_preference = ifap->ifa_preference;

    switch (BIT_TEST(ifap->ifa_state, IFS_LOOPBACK|IFS_POINTOPOINT)) {
    case IFS_LOOPBACK:
	/* Add a host route to the loopback interface */

	BIT_SET(int_rtparms.rtp_state, RTS_NOADVISE);
	int_rtparms.rtp_dest = ifap->ifa_addr;
	int_rtparms.rtp_dest_mask = sockhostmask(ifap->ifa_addr);

	ifap->ifa_rt = rt_add(&int_rtparms);
#ifndef	ROUTE_AGGREGATION
	if (ifap->ifa_subnet) {

	    /* Add a aggregate reject route to 127/255 */
	    BIT_SET(int_rtparms.rtp_state, RTS_REJECT);
	    goto common;
	}
#endif	/* ROUTE_AGGREGATION */
	break;

    case IFS_POINTOPOINT:
	/*
	 * Add a route to the interface.
	 * 4.2 based systems need the router to be the destination.
	 * 4.3 (and later) based systems like it to be the local address
	 */
	int_rtparms.rtp_dest = ifap->ifa_addr;
	int_rtparms.rtp_dest_mask = sockhostmask(ifap->ifa_addr);
#ifndef	P2P_RT_REMOTE
	/* Interface routes need to point at the local address */
	int_rtparms.rtp_router = ifap->ifa_addr_local;
#endif	/* P2P_RT_REMOTE */

	ifap->ifa_rt = rt_add(&int_rtparms);
	break;

#ifndef	ROUTE_AGGREGATION
    common:
	/* Fall through */
#endif	/* ROUTE_AGGREGATION */

    default:
	/*  Delete any routes to this subnet and add an interface route to  */
	/*  it if we are the most attractive.                               */

	BIT_SET(int_rtparms.rtp_state, RTS_INTERIOR);

	int_rtparms.rtp_dest = ifap->ifa_subnet;
	int_rtparms.rtp_dest_mask = ifap->ifa_subnetmask;
#ifdef	ROUTE_AGGREGATION
	ifap->ifa_rt = rt_add(&int_rtparms);
#else	/* ROUTE_AGGREGATION */
	rt = rt_add(&int_rtparms);
	if (ifap->ifa_rt) {
	    ifap->ifa_rt->rt_aggregate = rt;
	} else {
	    ifap->ifa_rt = rt;
	}
	BIT_SET(int_rtparms.rtp_state, RTS_NOTINSTALL);

	if (rt
	    && BIT_TEST(ifap->ifa_state, IFS_SUBNET)) {
	    /*  Interface is to a subnet.  Must update route to the main net  */
	    /*  if this is the most attractive interface to it.               */

	    int_rtparms.rtp_dest = ifap->ifa_net;
	    int_rtparms.rtp_dest_mask = ifap->ifa_netmask;
	    BIT_RESET(int_rtparms.rtp_state, RTS_RETAIN);
	    rt->rt_aggregate = rt_add(&int_rtparms);
	}
#endif	/* ROUTE_AGGREGATION */
	break;
    } /* switch */
}


/*
 *	Delete all routes for an interface
 */
void
if_rtdelete __PF1(ifap, if_addr *)
{

    switch (socktype(ifap->ifa_addr)) {
#ifdef	PROTO_ISO
    case AF_ISO:
	return;
#endif	/* PROTO_ISO */

#ifdef	PROTO_INET
    case AF_INET:
#endif	/* PROTO_INET */
    default:
	assert(ifap->ifa_rt);
	break;
    }

    trace(TR_INT, 0, "if_rtdelete: DELETE route for interface %s %A/%A",
	  ifap->ifa_name,
	  ifap->ifa_addr,
	  ifap->ifa_subnetmask);

    if (ifap->ifa_rt) {
#ifndef	ROUTE_AGGREGATION
	if (ifap->ifa_rt->rt_aggregate) {
	    if (ifap->ifa_rt->rt_aggregate->rt_aggregate) {
		rt_delete(ifap->ifa_rt->rt_aggregate->rt_aggregate);
		ifap->ifa_rt->rt_aggregate->rt_aggregate = (rt_entry *) 0;
	    }
	    rt_delete(ifap->ifa_rt->rt_aggregate);
	    ifap->ifa_rt->rt_aggregate = (rt_entry *) 0;
	}
#endif	/* ROUTE_AGGREGATION */
	rt_delete(ifap->ifa_rt);
	ifap->ifa_rt = (rt_entry *) 0;
    }
}


/*
 *	Interface is up, make the preference attractive and reset the
 *	RTS_NOADVISE flag.  Also used to change the preference on an
 *	interface
 */
void
if_rtup __PF1(ifap, if_addr *)
{
    if (!ifap->ifa_rt) {
	/* Route not yet installed */

	if_rtadd(ifap);
    } else {
	int loopback = BIT_TEST(ifap->ifa_state, IFS_LOOPBACK);
#ifdef	ROUTE_AGGREGATION

	if (ifap->ifa_rt->rt_preference == ifap->ifa_preference &&
	    (loopback ||
	     !BIT_TEST(ifap->ifa_rt->rt_state, RTS_NOADVISE))) {
	    return;
	}

	/* XXX - only log message if it was down */
	trace(TR_ALL, LOG_WARNING, "if_rtup: UP route for interface %s %A/%A",
	      ifap->ifa_name,
	      ifap->ifa_addr,
	      ifap->ifa_subnetmask);

	    /* Make it advertisable */
	    if (!loopback) {
		BIT_RESET(ifap->ifa_rt->rt_state, RTS_NOADVISE);
	    }

	    /* And the preference normal */
	    (void) rt_change(ifap->ifa_rt,
			     ifap->ifa_rt->rt_metric,
			     (metric_t) 0,
			     ifap->ifa_preference,
			     0,
			     (sockaddr_un **) 0);
#else	 /* ROUTE_AGGREGATION */
	rt_entry *rt = ifap->ifa_rt;

	if (rt->rt_preference == ifap->ifa_preference &&
	    (loopback ||
	     !BIT_TEST(rt->rt_state, RTS_NOADVISE))) {
	    return;
	}

	/* XXX - only log message if it was down */
	trace(TR_ALL, LOG_WARNING, "if_rtup: UP route for interface %s %A/%A",
	      ifap->ifa_name,
	      ifap->ifa_addr,
	      ifap->ifa_subnetmask);

	do {
	    /* Make it advertisable */
	    if (!loopback) {
		BIT_RESET(rt->rt_state, RTS_NOADVISE);
	    }

	    /* And the preference normal */
	    (void) rt_change(rt,
			     rt->rt_metric,
			     (metric_t) 0,
			     ifap->ifa_preference,
			     0,
			     (sockaddr_un **) 0);
	} while (rt = rt->rt_aggregate) ;
#endif	/* ROUTE_AGGREGATION */
    }
}


/*
 *	Change routes to signify an interface is down.  Set the preference to
 *	be less attractive and set the RTS_NOADVISE bit.
 */
void
if_rtdown __PF1(ifap, if_addr *)
{
#ifndef	ROUTE_AGGREGATION
    rt_entry *rt = ifap->ifa_rt;
#endif	/* ROUTE_AGGREGATION */

    switch (socktype(ifap->ifa_addr)) {
#ifdef	PROTO_ISO
    case AF_ISO:
	return;
#endif	/* PROTO_ISO */

#ifdef	PROTO_INET
    case AF_INET:
#endif	/* PROTO_INET */
    default:
	assert(ifap->ifa_rt);
	break;
    }
    
    trace(TR_ALL, LOG_WARNING, "if_rtdown: DOWN route for interface %s %A/%A",
	  ifap->ifa_name,
	  ifap->ifa_addr,
	  ifap->ifa_subnetmask);

#ifdef	ROUTE_AGGREGATION
	/* Make it not advertisable */
	BIT_SET(ifap->ifa_rt->rt_state, RTS_NOADVISE);

	/* And the preference normal */
	(void) rt_change(ifap->ifa_rt,
			 ifap->ifa_rt->rt_metric,
			 (metric_t) 0,
			 ifap->ifa_preference_down,
			 0,
			 (sockaddr_un **) 0);
#else	/* ROUTE_AGGREGATION */
    do {
	/* Make it not advertisable */
	BIT_SET(rt->rt_state, RTS_NOADVISE);

	/* And the preference normal */
	(void) rt_change(rt,
			 rt->rt_metric,
			 (metric_t) 0,
			 ifap->ifa_preference_down,
			 0,
			 (sockaddr_un **) 0);
    } while (rt = rt->rt_aggregate) ;
#endif	/* ROUTE_AGGREGATION */
}


/*
 *	We just received a routing packet via an interface.  Make sure
 *	we consider the interface up and reset the timer.
 */
void
if_rtupdate __PF1(ifap, if_addr *)
{
#ifdef	ROUTE_AGGREGATION
    if (ifap->ifa_rt->rt_preference != ifap->ifa_preference) {
	/* We consider it down */

	trace(TR_INT, 0, "if_rtupdate: UPDATE route for interface %s %A/%A",
	      ifap->ifa_name,
	      ifap->ifa_addr,
	      ifap->ifa_subnetmask);

	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    /* Kernel thinks it's up */
	    if_rtup(ifap);
	} else {
	    /* Check what kernel thinks about it */
	    if_check();
	}
    } else {
	/* We consider it up, refresh it */
	
	rt_refresh(ifap->ifa_rt);
    }
#else	/* ROUTE_AGGREGATION */
    rt_entry *rt = ifap->ifa_rt;
    
    if (rt->rt_preference != ifap->ifa_preference) {
	/* We consider it down */

	trace(TR_INT, 0, "if_rtupdate: UPDATE route for interface %s %A/%A",
	      ifap->ifa_name,
	      ifap->ifa_addr,
	      ifap->ifa_subnetmask);

	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    /* Kernel thinks it's up */
	    if_rtup(ifap);
	} else {
	    /* Check what kernel thinks about it */
	    if_check();
	}
    } else {
	/* We consider it up, refresh it */
	
	do {
	    rt_refresh(rt);
	} while (rt = rt->rt_aggregate) ;
    }
#endif	/* ROUTE_AGGREGATION */
}


/* Check for changes needed on interface routes */
void
if_rtifachange __PF1(ifap, if_addr *)
{
    register if_addr_entry *ifae = ifap->ifa_addrent_local;

    if (ifae) {
	switch (socktype(ifap->ifa_addr)) {
#ifdef	PROTO_INET
	case AF_INET:
	    if (ifae->ifae_n_p2p) {
		
		int_rtparms.rtp_state = RTS_NOAGE|RTS_RETAIN|RTS_GATEWAY|RTS_NOADVISE;

		if (ifae->ifae_n_p2p == ifae->ifae_n_if) {
		    /* Need a loopback route for a P2P interface */

		    int_rtparms.rtp_preference = RTPREF_DIRECT_AGGREGATE;
		} else {
		    /* Need a dummy route to prevent bogus routing */

		    int_rtparms.rtp_preference = RTPREF_DIRECT;
		    BIT_SET(int_rtparms.rtp_state, RTS_NOTINSTALL);
		}
		
		if (ifae->ifae_rt
		    && ifae->ifae_rt->rt_preference != int_rtparms.rtp_preference) {
		    /* Wrong type, delete it */

		    rt_delete(ifae->ifae_rt);
		    ifae->ifae_rt = (rt_entry *) 0;
		}

		if (!ifae->ifae_rt) {
		    int_rtparms.rtp_dest = ifap->ifa_addr_local;
		    int_rtparms.rtp_dest_mask = inet_mask_host;
		    int_rtparms.rtp_router = inet_addr_loopback;
		    ifae->ifae_rt = rt_add(&int_rtparms);
		}
	    }
	    break;
#endif	/* PROTO_INET */

	default:
	    break;
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
