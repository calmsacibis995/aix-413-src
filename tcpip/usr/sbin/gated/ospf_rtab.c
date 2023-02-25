static char sccsid[] = "@(#)02	1.1  src/tcpip/usr/sbin/gated/ospf_rtab.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:28";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF2
 *		__PF3
 *		__PF4
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
 * ospf_rtab.c,v 1.12 1993/03/22 02:40:20 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


/*
 * Is this route an active intra or inter area route?
 */
int
ospf_int_active __PF3(from, int,
		      area, struct AREA *,
		      r, rt_entry *)
{
    int ptype = PTYPE_BIT(ORT_PTYPE(r));

    if (ptype & PTYPE_INTER) {
        if (!(from & ptype)) {
            return TRUE;
	}
        return ORT_REV(r) == RTAB_REV;
    }

    if (ptype & PTYPE_INTRA) {
        if (!(from & ptype))
            return TRUE;
        if (ORT_AREA(r) == area) {
             return ORT_REV(r) == RTAB_REV;
        }
        return TRUE;
    }

    return FALSE;
}


/*
 * Is this route an active intra area route?
 */
static inline int
ospf_intra_active __PF3(from, int,
			area, struct AREA *,
			r, rt_entry *)
{
    int ptype = PTYPE_BIT(ORT_PTYPE(r));

    if (ptype & PTYPE_INTRA) {
        if (!(from & ptype)) {
            return TRUE;
	}
        if (ORT_AREA(r) == area) {
             return ORT_REV(r) == RTAB_REV;
        }
        return TRUE;
    }

    return FALSE;
}


/*
 * Enqueue r in order in this queue
 */
static void
enq_rtab __PF2(qhp, struct OSPF_ROUTE *,
	       r, struct OSPF_ROUTE *)
{
    struct OSPF_ROUTE *o;
    u_int32 id = RRT_DEST(r);

    for (o = qhp;
	 RRT_NEXT(o) && id < RRT_DEST(RRT_NEXT(o));
	 o = RRT_NEXT(o));

    /* insert r between o and o->next */
    ADD_Q(o, r);
}


/*
 * Search for id in this linked list
 */
static struct OSPF_ROUTE *
routescan __PF2(head, struct OSPF_ROUTE *,
		id, u_int32)
{
    struct OSPF_ROUTE *r = head;

    for (; (r && id < RRT_DEST(r)); r = RRT_NEXT(r)) ;

    if (r && id == RRT_DEST(r))
	return r;
    return (struct OSPF_ROUTE *) 0;
}


/*
 * Allocate route and install it in the routing table
 */
static int
rtr_build_route __PF3(a, struct AREA *,
		      v, struct LSDB *,
		      dtype, int)
{
    struct OSPF_ROUTE *r = (struct OSPF_ROUTE *) 0;

    RRT_ALLOC(r);
    if (!r)
	return FALSE;

    switch (dtype) {
    case DTYPE_ABR:
	DB_AB_RTR(v) = r;
	break;

    case DTYPE_ASBR:
	DB_ASB_RTR(v) = r;
	break;
    }
    RRT_REV(r) = RTAB_REV;
    RRT_AREA(r) = a;
    RRT_DTYPE(r) = dtype;
    RRT_COST(r) = DB_DIST(v);
    RRT_PTYPE(r) = LS_TYPE(v);
    RRT_NH_NDX(r) = alloc_nh_entry(DB_NH_NDX(v, 0));
    RRT_ADVRTR(r) = ADV_RTR(v);
    /* set up back link */
    RRT_V(r) = v;
    RRT_DEST(r) = LS_ID(v);
    RRT_CHANGE(r) = E_NEW;

    /* Install on routing table */
    switch (LS_TYPE(v)) {
    case LS_SUM_ASB:
	enq_rtab(&sum_asb_rtab, r);
	break;
    case LS_RTR:
	if (dtype == DTYPE_ABR)
	    enq_rtab(&a->abrtab, r);
	if (dtype == DTYPE_ASBR)
	    enq_rtab(&a->asbrtab, r);
	break;
    }

    return TRUE;
}


/*
 * add v to the routing table
 */
int
addroute __PF4(a, struct AREA *,
	       v, struct LSDB *,
	       from, int,
	       from_area, struct AREA *)	/* associatiated area */
{
    rt_entry *rt;
    struct OSPF_ROUTE *rr = (struct OSPF_ROUTE *) 0;

    assert(DB_RTR(v));

    /*
     * The from parameter is the starting level of spf algorithm,
     * and is used to check for valid entry
     */

    switch (LS_TYPE(v)) {
	struct NH_BLOCK * nhndx;

    case LS_STUB:
    case LS_NET:
	/* if attached to this net, interface is the first hop */
	if (DB_ROUTE(v)) {
	    /* had route, check for change */

	    rvbind(DB_ROUTE(v), v, a);
	    DB_WHERE(v) = ON_RTAB;
	} else {
	    /* check to see if anyone else was advertising this net */
	    /* This will also handle the case of inter becoming intra */

	    findroute(rt, DB_NETNUM(v), DB_MASK(v));
	    if (rt) {
		/*
		 * Bind route to this vertex
		 * There could be stub routes and net routes until
		 * things converge, so use the best route
		 */

		if (ORT_PTYPE(rt) > PTYPE_INTRA ||
		    ORT_REV(rt) != RTAB_REV ||
		    (ORT_REV(rt) == RTAB_REV &&
		     ORT_COST(rt) > DB_DIST(v))) {
		    rvbind(rt, v, a);
		    DB_WHERE(v) = ON_RTAB;
		}
	    } else {
		/* no previous net adv */

		if (!build_route(a, v, (rt_data *) 0))
		    return FLAG_NO_BUFS;
		DB_WHERE(v) = ON_RTAB;
	    }
	}
	break;

    case LS_RTR:
	/* only asbr and abr are added to rtab */
	/* AREA Border */
	nhndx = DB_NH_NDX(v, 0);

	if (ntohs(DB_RTR(v)->E_B) & bit_B) {
	    /* if this is this rtr, set accordingly */
	    if (v == DB_PTR(&a->spf)[NEXT])
		DB_DIST(v) = 0;
	    if (DB_AB_RTR(v)) {
		rr = DB_AB_RTR(v);
		RRT_REV(rr) = RTAB_REV;
		if ((RRT_COST(rr) != DB_DIST(v)) ||
		    (RRT_NH_NDX(rr) != nhndx)) {
		    RRT_COST(rr) = DB_DIST(v);
		    RRT_NH_NDX(rr) = alloc_nh_entry(nhndx);
		    RRT_CHANGE(rr) = E_NEXTHOP;
		} else
		    RRT_CHANGE(rr) = E_UNCHANGE;
	    } else {
		if (!rtr_build_route(a, v, DTYPE_ABR))
		    return FLAG_NO_BUFS;
	    }
	    DB_WHERE(v) = ON_RTAB;
	}
	/* AS Border router */
	if (ntohs(DB_RTR(v)->E_B) & bit_E) {
	    /* if this is this rtr, set accordingly */
	    if (v == DB_PTR(&a->spf)[NEXT])
		DB_DIST(v) = 0;
	    if (DB_ASB_RTR(v)) {
		rr = DB_ASB_RTR(v);
		RRT_REV(rr) = RTAB_REV;
		if ((RRT_COST(rr) != DB_DIST(v)) ||
		    (RRT_NH_NDX(rr) != nhndx) ) {
		    RRT_COST(rr) = DB_DIST(v);
 		    RRT_NH_NDX(rr) = alloc_nh_entry(nhndx);
		    RRT_CHANGE(rr) = E_NEXTHOP;
		} else
		    RRT_CHANGE(rr) = E_UNCHANGE;
	    } else {
		/* this node is ASBR */
		if (!rtr_build_route(a, v, DTYPE_ASBR))
		    return FLAG_NO_BUFS;
	    }
	    DB_WHERE(v) = ON_RTAB;
	}
	break;

    case LS_SUM_NET:
	if (!DB_ROUTE(v)) {
	    /* see if one exists on current routing table */
	    findroute(rt, DB_NETNUM(v), DB_MASK(v));
	    if (rt) {
		/* check for possible intra-area route */
		trace(TR_OSPF, 0,
		      "Sum: route found from area %A rt area %A from %x ptype %d",
		      sockbuild_in(0, from_area->area_id),
		      sockbuild_in(0, ORT_AREA(rt)->area_id),
		      from,
		      ORT_PTYPE(rt));
		if (ospf_intra_active(from, from_area, rt)) {
		    return FLAG_NO_PROBLEM;
		} else {
		    /* old or ase changing to inter so bind r to this v */

		    rvbind(rt, v, a);
		    return FLAG_NO_PROBLEM;
		}
	    } else {		/* route doesn't exist */
		if (!build_route(a, v, (rt_data *) 0))
		    return FLAG_NO_BUFS;
	    }
	} else {		/* v->route != ROUTENULL */
	    /*
	     * v->route != ROUTENULL
	     * area border checked in netsum
	     */
	    rvbind(DB_ROUTE(v), v, a);
	}
	break;

    case LS_SUM_ASB:
	/* have route to as bdr rtr */
	nhndx = DB_NH_NDX(v, 0);
	if (!DB_ASB_RTR(v)) {
	    /* put on inter-area asb route tab */
	    if (!rtr_build_route(a, v, DTYPE_ASBR))
		return FLAG_NO_BUFS;
	} else {
	    rr = DB_ASB_RTR(v);
	    RRT_REV(rr) = RTAB_REV;
	    if (DB_DIST(v) != RRT_COST(rr))
		RRT_COST(rr) = DB_DIST(v);
	    if (RRT_NH_NDX(rr) != nhndx) {
		RRT_NH_NDX(rr) = alloc_nh_entry(nhndx);
		RRT_CHANGE(rr) = E_NEXTHOP;
	    } else
		RRT_CHANGE(rr) = E_UNCHANGE;
	}
	break;

    case LS_ASE:
	if (!DB_ROUTE(v)) {
	    /* see if one exists on current routing table */
	    findroute(rt, DB_NETNUM(v), DB_MASK(v));
	    if (rt) {
		/* may have already run intra and sum for single area */
		if (ospf_int_active(from, from_area, rt)) {
		    return FLAG_NO_PROBLEM;
		} else {
		    /* current route is invalid - bind route to this v */
		    rvbind(rt, v, a);
		    return FLAG_NO_PROBLEM;
		}
	    } else {		/* no existing route */
		if (!build_route(a, v, (rt_data *) 0))
		    return FLAG_NO_BUFS;
	    }
	} else {		/* v->route != ROUTENULL */
	    /* GOT_A_BDR && BORDER_ACTIVE are true */
	    rvbind(DB_ROUTE(v), v, a);
	}
	break;
    }
    return FLAG_NO_PROBLEM;
}


struct OSPF_ROUTE *
rtr_findroute __PF4(area, struct AREA *,
		    id, u_int32,
		    dtype, int,
		    ptype, int)
{
    struct OSPF_ROUTE *r;
    struct AREA *a;


    if (ptype & PTYPE_INTRA) {
	/* search intra area tables */
	switch (dtype) {

	case DTYPE_ASBR:
	    if (area != AREANULL) {
		r = routescan(area->asbrtab.ptr[NEXT], id);
		if (ptype == PTYPE_INTRA)
		    return r;
		else
		    break;
	    }
	    AREA_LIST(a) {
		if (r = routescan(a->asbrtab.ptr[NEXT], id))
		    return r;
	    } AREA_LIST_END(a) ;
	    break;

	case DTYPE_ABR:
	    return routescan(area->abrtab.ptr[NEXT], id);

	default:
	    return (struct OSPF_ROUTE *) 0;
	}
    }

    if (ptype & PTYPE_INTER) {
	/* search inter area routing tables */
	return routescan(sum_asb_rtab.ptr[NEXT], id);
    }

    return (struct OSPF_ROUTE *) 0;
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
 * ------------------------------------------------------------------------
 * 
 *                 U   U M   M DDDD     OOOOO SSSSS PPPPP FFFFF
 *                 U   U MM MM D   D    O   O S     P   P F
 *                 U   U M M M D   D    O   O  SSS  PPPPP FFFF
 *                 U   U M M M D   D    O   O     S P     F
 *                  UUU  M M M DDDD     OOOOO SSSSS P     F
 * 
 *     		          Copyright 1989, 1990, 1991
 *     	       The University of Maryland, College Park, Maryland.
 * 
 * 			    All Rights Reserved
 * 
 *      The University of Maryland College Park ("UMCP") is the owner of all
 *      right, title and interest in and to UMD OSPF (the "Software").
 *      Permission to use, copy and modify the Software and its documentation
 *      solely for non-commercial purposes is granted subject to the following
 *      terms and conditions:
 * 
 *      1. This copyright notice and these terms shall appear in all copies
 * 	 of the Software and its supporting documentation.
 * 
 *      2. The Software shall not be distributed, sold or used in any way in
 * 	 a commercial product, without UMCP's prior written consent.
 * 
 *      3. The origin of this software may not be misrepresented, either by
 *         explicit claim or by omission.
 * 
 *      4. Modified or altered versions must be plainly marked as such, and
 * 	 must not be misrepresented as being the original software.
 * 
 *      5. The Software is provided "AS IS". User acknowledges that the
 *         Software has been developed for research purposes only. User
 * 	 agrees that use of the Software is at user's own risk. UMCP
 * 	 disclaims all warrenties, express and implied, including but
 * 	 not limited to, the implied warranties of merchantability, and
 * 	 fitness for a particular purpose.
 * 
 *     Royalty-free licenses to redistribute UMD OSPF are available from
 *     The University Of Maryland, College Park.
 *       For details contact:
 * 	        Office of Technology Liaison
 * 		4312 Knox Road
 * 		University Of Maryland
 * 		College Park, Maryland 20742
 * 		     (301) 405-4209
 * 		FAX: (301) 314-9871
 * 
 *     This software was written by Rob Coltun
 *      rcoltun@ni.umd.edu
 */
