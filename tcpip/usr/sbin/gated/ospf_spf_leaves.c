static char sccsid[] = "@(#)08	1.1  src/tcpip/usr/sbin/gated/ospf_spf_leaves.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:47";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
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
 * ospf_spf_leaves.c,v 1.20.2.2 1993/07/08 21:01:30 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


/*
 * These routines deal with the SUMMARY and AS External part of the
 * 	Dijkstra algorithm
 */

struct NH_BLOCK nh_block[MAXNHCOUNT] = { { 0 } };
struct OSPF_ROUTE sum_asb_rtab = { { 0 } };


/* MODIFIED 5/2/92 - should make nr a hash table */
static int
area_nr_check __PF1(nr_net, u_int32)
{
    struct NET_RANGE *nr;
    struct AREA *dst;

    AREA_LIST(dst) {
	RANGE_LIST(nr, dst) {
	    if (nr->net == nr_net && nr->cost < SUMLSInfinity)
		return(TRUE);
	    if ( ntohl(nr_net) > ntohl(nr->net) )
		break;
	} RANGE_LIST_END(nr, dst) ;
    } AREA_LIST_END(dst) ;

    return FALSE;
}


/*
 * Handle network routes imported from another area
 * 	- called by spf() or rxlinkup()
 *   	- intra or sum may have been run before, but it ain't necessarily so
 *	- if called by rxlinkup can just do partial updates
 */
int
netsum __PF4(a, struct AREA *,
	     from, int,			/* level was run from */
	     from_area, struct AREA *,	/* area running the algorithm */
	     partial, int)		/* True if called from rxlinkup */
{
    struct LSDB *db, *hp, *low = LSDBNULL;
    struct OSPF_ROUTE *rr;
    rt_entry *old_route = (rt_entry *) 0;
    u_int32 lowcost = SUMLSInfinity, cost;

    if (!(from & INTRASCHED)) {
	/* intra hasn't been run */
	a->spfcnt++;
	RTAB_REV++;

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_SPF)) {
	    trace(TR_OSPF, 0, "OSPF RUN SPF\tSummary Nets  Area %A",
		  sockbuild_in(0, a->area_id));
	    trace(TR_OSPF, 0, NULL);
	}
    }
    for (hp = a->htbl[LS_SUM_NET];
	 hp < &(a->htbl[LS_SUM_NET][HTBLSIZE]);
	 hp++) {
	if (partial && !DB_RERUN(*hp)) {
	    /* routes not included in partial update */
	    for (db = DB_NEXT(hp); db; db = DB_NEXT(db)) {
		if (NO_GUTS(db))
		    continue;
		/* running partial update - just mark current */
		if (DB_ROUTE(db)) {
		    ORT_CHANGE(DB_ROUTE(db)) = E_UNCHANGE;
		    ORT_REV(DB_ROUTE(db)) = RTAB_REV;
		}
	    } 
	} else
	    if ((partial && DB_RERUN(*hp)) || !(partial)) {
	    /* non-partial update or partial update sched for this row */
	    DB_RERUN(*hp) = FALSE;
	    for (db = DB_NEXT(hp);
		 db;
		 db = DB_NEXT(db)) {
		if ((GOT_GUTS(db)) &&
		    (BIG_METRIC(db) != SUMLSInfinity) &&
		    (!DB_FREEME(db)) &&
		    (LS_AGE(db) < MaxAge) &&
		    (ADV_RTR(db) != MY_ID) && 
		    (!area_nr_check(LS_ID(db)))) {

		    rr = (struct OSPF_ROUTE *) 0;

		    /* check to see if we know about this ABR */
		    if (GOT_A_BDR(db)) {
			if (ABRTR_ACTIVE(db))
			    rr = DB_AB_RTR(DB_BDR(db));
		    } else {
			rr = rtr_findroute(a,
					   ADV_RTR(db),
					   DTYPE_ABR,
					   PTYPE_INTRA);
			if (rr)
			    DB_BDR(db) = RRT_V(rr);
		    }

		    /* Have a valid border router? */
		    if ((rr) &&
			DB_BDR(db) &&
			(GOT_GUTS(DB_BDR(db))) &&
			((cost = (RRT_COST(rr) + BIG_METRIC(db))) <
			 SUMLSInfinity)) {
			if (cost == lowcost) {
			    ospf_add_parent(low,
					    DB_BDR(db),
					    cost,
					    a,
					    RRT_NH(rr),
					    0,
					    0);
			} else if (cost < lowcost) {
			    /* free old parent list */
			    freeplist(db);
			    low = db;
			    lowcost = cost;
			    ospf_add_parent(low,
					    DB_BDR(low),
					    cost,
					    a,
					    RRT_NH(rr),
					    0,
					    0);
			}
		    }
		}
		/* Grab old route for deletion with partial update */
		if ((DB_ROUTE(db)))
		    old_route = DB_ROUTE(db);

		/* MODIFIED 5/1/92 - removed ase rerun flag */
		if (!DB_NEXT(db) ||
		    (low && (DB_NEXT(db)->key[0] != low->key[0])) ||
		    (old_route && (DB_NEXT(db)->key[0] != RT_DEST(old_route)))) {
		    /* Nexthops were added? */
		    if (low && DB_NH_CNT(low)) {
		    	if ( addroute(a, low, from, from_area) )
			    return FLAG_NO_BUFS;
		    }
		    /* Modify partial update */
		    if (partial) {
			if (low && DB_ROUTE(low))
			    old_route = DB_ROUTE(low);
			if (old_route)
			    ospf_route_update(old_route, a, SUMASESCHED);
		    }
		    low = LSDBNULL;
		    lowcost = SUMLSInfinity;
		    old_route = (rt_entry *) 0;
		}
	    }
	}
    }
    from_area->spfsched &= (~SUMNETSCHED);
    a->spfsched &= (~SUMNETSCHED);
    return 0;
}


/*
 * Handle all network as border rtrs imported from another area
 * 	- called by spf() or rxlinkup()
 *   	- intra or sum may have been run before, but it ain't necessarily so
 */
int
asbrsum __PF4(a, struct AREA *,
	      from, int,		/* 0 if called from spf() else 1 */
	      from_area, struct AREA *,
	      partial, int)
{
    struct LSDB *db, *hp, *low = LSDBNULL;
    struct OSPF_ROUTE *rr, *rrt_next;
    u_int32 lowcost = SUMLSInfinity, cost;

    /*
     * For now will not do partial updates for asbrsum because there probably
     * won't be more than a few asbr routes
     */

    /* if intra and netsum haven't been run incriment spfcnt */
    if (!(from & SUMNETSCHED)) {
	a->spfcnt++;
	RTAB_REV++;
    }
    
    if (!(from & INTRASCHED)) {
	if (BIT_TEST(ospf.trace_flags, TR_OSPF_SPF)) {
	    trace(TR_OSPF, 0, "OSPF RUN SPF\tSummary ASBR  Area %A",
		  sockbuild_in(0, a->area_id));
	    trace(TR_OSPF, 0, NULL);
	}
    }
    
    for (hp = a->htbl[LS_SUM_ASB];
	 hp < &(a->htbl[LS_SUM_ASB][HTBLSIZE]);
	 hp++) {
	for (db = DB_NEXT(hp); db; db = DB_NEXT(db)) {
	    if ((GOT_GUTS(db)) &&
		(BIG_METRIC(db) != SUMLSInfinity) &&
		(!DB_FREEME(db)) &&
		(LS_AGE(db) < MaxAge) &&
		(ADV_RTR(db) != MY_ID)) {
		/* First check for intra */
		if ((rr = rtr_findroute(0,
					LS_ID(db),
					DTYPE_ASBR,
					PTYPE_INTRA)))
		    goto next_db;

		/* check to see if we know about this ABR */
		if (GOT_A_BDR(db)) {
		    if (ABRTR_ACTIVE(db))
			rr = DB_AB_RTR(DB_BDR(db));
		} else {
		    rr = rtr_findroute(a,
				       ADV_RTR(db),
				       DTYPE_ABR,
				       PTYPE_INTRA);
		    if (rr)
			DB_BDR(db) = RRT_V(rr);
		}
		/* Have a valid border router? */
		if ((rr) &&
		    DB_BDR(db) &&
		    (GOT_GUTS(DB_BDR(db))) &&
		    ((cost = (RRT_COST(rr) + BIG_METRIC(db))) <
		     SUMLSInfinity)) {
		    if (cost == lowcost) {
			ospf_add_parent(low,
					DB_BDR(db),
					cost,
					a,
					RRT_NH(rr),
					0,
					0);
		    } else if (cost < lowcost) {
			/* free old parent list */
			freeplist(db);
			low = db;
			lowcost = cost;
			ospf_add_parent(low,
					DB_BDR(low),
					cost,
					a,
					RRT_NH(rr),
					0,
					0);
		    }
		}
	    }			/* Reasonable LS_SUM_ASB */
	  next_db:
	    if (low &&
		(!DB_NEXT(db) ||
		 (DB_NEXT(db)->key[0] != low->key[0]))) {
		/* Nexthops were added? */
		if (DB_NH_CNT(low)) {
		    /* add route to ospf's routing table */
		    if (addroute(a, low, from, from_area))
			return FLAG_NO_BUFS;
		}
		low = LSDBNULL;
		lowcost = SUMLSInfinity;
	    }
	}
    }

    /* modify asb routes */
    for (rr = sum_asb_rtab.ptr[NEXT];
	 rr;
	 rr = rrt_next) {
	rrt_next = RRT_NEXT(rr);
	
	if (RRT_REV(rr) != RTAB_REV) {
	    /*
	     * delete intra routes from all other areas - add to txq - - dist
	     * will = Infinity
	     */
 	    if (RRT_V(rr)) {
		if ((IAmBorderRtr) &&
		    (a->area_id == OSPF_BACKBONE) &&
		    (ADV_RTR(RRT_V(rr)) != MY_ID))
		    build_sum_asb(a, rr, from_area);

		DB_ASB_RTR(RRT_V(rr)) = (struct OSPF_ROUTE *) 0;
		freeplist(RRT_V(rr));
	    }
	    DEL_Q(rr, TRUE, OMEM_ORT);
	    continue;
	} else if (RRT_CHANGE(rr) != E_UNCHANGE) {
 	    /* 
 	     * add inter routes to all other areas txq
 	     * change in firsthop or newroute 
 	     */
	    if ((IAmBorderRtr) && (a->area_id == OSPF_BACKBONE) &&
		(ADV_RTR(RRT_V(rr)) != MY_ID))
		build_sum_asb(a, rr, from_area);
	    RRT_CHANGE(rr) = E_UNCHANGE;
	}
    }
    a->spfsched &= (~SUMASBSCHED);
    return FLAG_NO_PROBLEM;
}

/**/

/* Support for ASEs */

struct FORWARD_CACHE {
    u_int32 fwd_addr;
    /* PORT */
    rt_entry *fwd_rt;
    struct NH_BLOCK *nhndx;
    u_int16 active;
    u_int16 use;
};

static struct FORWARD_CACHE forward_cache[FORWARD_CACHE_SIZE];
static u_int forward_cache_size;

/*
 * Clear the forwarding cache and release any references to
 * next hop entries
 */
static void
forward_cache_clear __PF0(void)
{
    register struct FORWARD_CACHE *fp = forward_cache + forward_cache_size;

    while (fp-- > forward_cache) {
	if (fp->nhndx) {
	    free_nh_entry(&fp->nhndx);
	}
    }

    forward_cache_size = 0;
}


/*
 * Look up ase forwarding address
 *  - return route
 *  - stuff correct nexthop address in nexthop
 */
static inline rt_entry *
findforward  __PF4(id, u_int32,
		   ff_from, int,
		   from_area, struct AREA *,
		   ff_nhndx, struct NH_BLOCK **)
{
    rt_entry *rt;
    register struct FORWARD_CACHE *fp;
    register struct FORWARD_CACHE *fplim = &forward_cache[forward_cache_size];

    /* See if this next hop is in our cache */
    for (fp = forward_cache;
	 fp < fplim;
	 fp++) {
	if (fp->fwd_addr == id) {
	    /* Found it */

	    fp->use++;
	    *ff_nhndx = fp->active ? alloc_nh_entry(fp->nhndx) : (struct NH_BLOCK *) 0;
	    return fp->fwd_rt;
	}
    }

    rt = rt_lookup(RTS_NETROUTE,
		   RTS_DELETE,
		   sockbuild_in(0, id),
		   RTPROTO_BIT(RTPROTO_OSPF));
    if (!rt
	|| rt->rt_dest_mask == inet_mask_default
	|| sock2ip(rt->rt_ifap->ifa_addr) == id) {
	/* No route, route to default, or route to my interface address */

	*ff_nhndx = (struct NH_BLOCK *) 0;
	return (rt_entry *) 0;
    }

    /* Add to forward cache */
    if (forward_cache_size < FORWARD_CACHE_SIZE) {
	/* Because of lookup above, fp points to next entry to use */

	forward_cache_size++;
    } else {
	register struct FORWARD_CACHE *fp1 = (fp = forward_cache);

	/* Reuse the least used entry */

	while (++fp1 < fplim) {
	    if (fp1->use < fp->use) {
		fp = fp1;
	    }
	};

	if (fp->nhndx) {
	    /* Free the NH entry */
	    free_nh_entry(&fp->nhndx);
	}
    }

    fp->use = 1;
    fp->fwd_addr = id;

    if (ff_from == PTYPE_EXT
	|| ospf_int_active(ff_from, from_area, rt)) {
	/* This is a positive cache entry */

	fp->active = TRUE;
	fp->fwd_rt = rt;

	if (DB_DIRECT(ORT_V(rt))) {
	    *ff_nhndx = add_nh_entry(ORT_IO_NDX(rt, 0),
				     id,
				     NH_DIRECT_FORWARD);
	} else {
	    *ff_nhndx = alloc_nh_entry(ORT_NH_NDX(rt, 0));
	}

	fp->nhndx = alloc_nh_entry(*ff_nhndx);
    } else {
	/* This is a negative cache entry */

	fp->active = FALSE;
	rt = fp->fwd_rt = (rt_entry *) 0;
	fp->nhndx = *ff_nhndx = (struct NH_BLOCK *) 0;
    }

    return rt;
}


/*
 * Handle all routes imported from another AS
 * 	- called by spf() or rxlinkup()
 *   	- intra or sum may have been run before, but it ain't necessarily so
 *	- will just keep one equal cost route per border rtr/forwarding addr
 */
int
ase __PF3(area, struct AREA *,	/* area resulting in ase() being called */
	  from, int,		/* what was (intra or sum run before ase) flag */
	  partial, int)		/* True if called for rxlinkup */
{
    int rc = FLAG_NO_PROBLEM;
    struct LSDB *hp, *db, *par, *low = LSDBNULL, *my_version;
    struct OSPF_ROUTE *rr;
    rt_entry *old_route = (rt_entry *) 0;
    int k = 0;
    u_int32 low_cost = ASELSInfinity, cost;
    int etype, low_etype = 2;
    u_int32 type2cost, low_type2cost = ASELSInfinity;

    if (!(from & INTSCHED)) {
	area->spfcnt++;
	RTAB_REV++;		/* intra and sum haven't been run */

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_SPF)) {
	    trace(TR_OSPF, 0, "OSPF RUN SPF\tASE  Area %A",
		  sockbuild_in(0, area->area_id));
	    trace(TR_OSPF, 0, NULL);
	}
    }

    /* The fowarding cache should be clear */
    assert(!forward_cache_size);

    for (hp = area->htbl[LS_ASE];
	 hp < &(area->htbl[LS_ASE][HTBLSIZE]);
	 hp++, k++) {
	my_version = LSDBNULL;

#ifdef	notdef
	if (partial && !DB_RERUN(*hp)) {
	    /* routes not included in partial update */
	    for (db = DB_NEXT(hp); db; db = DB_NEXT(db)) {
		if (NO_GUTS(db))
		    continue;
		/* running partial update - just mark current */
		if (DB_ROUTE(db)) {
		    ORT_CHANGE(DB_ROUTE(db)) = E_UNCHANGE;
		    ORT_REV(DB_ROUTE(db)) = RTAB_REV;
		}
	    }
	} else
#endif	/* notdef */
	if ((partial && DB_RERUN(*hp)) || !(partial)) {
	    /* non-partial update or partial update sched for this row */
	    DB_RERUN(*hp) = FALSE;
	    for (db = DB_NEXT(hp); db; db = DB_NEXT(db)) {
		/* Reasonable looking DB? */
		if ((GOT_GUTS(db)) &&
		    (BIG_METRIC(db) != ASELSInfinity) &&
		    (!DB_FREEME(db)) &&
		    (LS_AGE(db) < MaxAge)) {
		    struct NH_BLOCK *nhndx = (struct NH_BLOCK *) 0;

		    if (ADV_RTR(db) == MY_ID) {
			/* May nuke my version of this - store info for now */
			my_version = db;
			goto next_db;
		    }
		    rr = (struct OSPF_ROUTE *) 0;
		    /* check to see if we know about this ASBR */
		    /* ASBR may have changed so if not active lookup.. */
		    if ((GOT_A_BDR(db)) && (ASBRTR_ACTIVE(db)))
			rr = DB_BDR_ASB(db);
		    else {
			rr = rtr_findroute(0,
					   ADV_RTR(db),
					   DTYPE_ASBR,
					   PTYPE_INT);
			if (rr)
			    DB_BDR(db) = RRT_V(rr);
		    }
		    /* Can't seem to get this AS border thing together... */
		    if (!rr ||
			(!DB_BDR(db) &&
			NO_GUTS(DB_BDR(db)))) {
			goto next_db;
		    }

		    /* Do that forwarding address thing... */
		    if (DB_ASE_FORWARD(db)) {
			rt_entry *f;

			f = findforward(DB_ASE_FORWARD(db),
					from,
					area,
					&nhndx);
			
			if (!f) {
			    trace(TR_OSPF, 0, "Findforward failed for ASE %A RTRID %A FORWARD %A",
				  sockbuild_in(0, LS_ID(db)),
				  sockbuild_in(0, ADV_RTR(db)),
				  sockbuild_in(0, DB_ASE_FORWARD(db)));
			    goto next_db;
			}
			par = ORT_V(f);
			cost = ORT_COST(f);
		    } else {
			par = RRT_V(rr);
			nhndx = alloc_nh_entry(RRT_NH_NDX(rr));
			cost = RRT_COST(rr);
		    }

		    /* Set metric for external type */
		    if (etype = ASE_TYPE2(db))
			type2cost = BIG_METRIC(db);
		    else {
			cost = BIG_METRIC(db) + cost;
			type2cost = 0;
		    }

		    /* Have a valid parent? */
		    if (par &&
			GOT_GUTS(par) &&
			nhndx &&
			type2cost < ASELSInfinity &&
			cost < ASELSInfinity) {
			/* Add equal cost route */

			if (ASE_COST_EQUAL(etype, cost, type2cost,
				      low_etype, low_cost, low_type2cost)) {
			    /* Same cost, try to add this next hop to list */

			    register struct NH_BLOCK **cp;
			    struct NH_BLOCK **lp = &DB_NH_NDX(low, DB_NH_CNT(low));

			    for (cp = &DB_NH_NDX(low, 0); cp < lp; cp++) {
				if (nhndx == *cp) {
				    /* Duplicate */

				    goto next_db;
				} else if (ntohl(nhndx->nh_addr) < ntohl((*cp)->nh_addr)) {
				    /* Insert here */

				    break;
				}
			    }

			    /* Insert here */
			    if (DB_NH_CNT(low) < MAXNH) {
				/* Room for one more */

				DB_NH_CNT(low)++;
				lp++;
			    } else if (cp == lp) {
				/* No room to append to list */

				goto next_db;
			    } else {
				/* Free the last entry */

				free_nh_entry(--lp);
			    }

			    /* Shift the list to make room */
			    if (cp + 1 < lp) {
				register struct NH_BLOCK **dp = lp;

				while (--dp > cp) {
				    *dp = dp[-1];
				}
			    }

			    *cp = nhndx;
			} else if (ASE_COST_LESS(etype,
						 cost,
						 type2cost,
						 low_etype,
						 low_cost,
						 low_type2cost)) {
			    /* free old parent list */
			    freeplist(db);
			    low = db;
			    low_cost = cost;
			    low_etype = etype;
			    low_type2cost = type2cost;
			    DB_DIST(low) = cost;
			    DB_NH_CNT(low) = 1;
			    DB_NH_NDX(low, 0) = nhndx;
			} else {
			next_db:
			    if (nhndx) {
				free_nh_entry(&nhndx);
			    }
			}
		    }
		}

		/* Grab old route for deletion with partial update */
		if ((DB_ROUTE(db))) {
		    old_route = DB_ROUTE(db);
		}
		if (!DB_NEXT(db) ||
		    (low && ((DB_NEXT(db)->key[0] != low->key[0]))) ||
 		    (old_route && (DB_NEXT(db)->key[0] != RT_DEST(old_route)))) {
		    /* Nexthops were added? */
		    if (low && DB_NH_CNT(low)) {
			if (addroute(area, low, from, area)) {
			    rc = FLAG_NO_BUFS;
			    goto Return;
			}
		    }
		    /* Modify partial update */
		    if (partial) {
			if ((low) && DB_ROUTE(low)) {
			    old_route = DB_ROUTE(low);
			}
			if (old_route) {
			    ospf_route_update(old_route, area, ASESCHED);
			}
		    }
		    low = LSDBNULL;
		    low_cost = ASELSInfinity;
		    low_type2cost = ASELSInfinity;
		    low_etype = 1;
		    my_version = LSDBNULL;
		    old_route = (rt_entry *) 0;
		}
	    }			/* db loop */
	}			/* Partial */
	area->spfsched &= (~ASESCHED);
    }

 Return:
    /* Clear the forwarding cache and free references to Next Hop entries */
    forward_cache_clear();
    return rc;
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
