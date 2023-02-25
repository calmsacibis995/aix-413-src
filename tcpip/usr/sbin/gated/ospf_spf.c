static char sccsid[] = "@(#)07	1.1  src/tcpip/usr/sbin/gated/ospf_spf.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:44";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF7
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
 * ospf_spf.c,v 1.21.2.1 1993/04/30 18:00:34 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


static block_t nh_block_index;

/*
 *	Bump the refcount
 */
struct NH_BLOCK *
alloc_nh_entry __PF1(np, register struct NH_BLOCK *)
{
    if (np) {
	np->nh_refcount++;

#ifdef	DEBUG
	trace(TR_OSPF, 0, "alloc_nh_entry: ALLOC %A type %d interface %A (%s) refcount %d",
	      sockbuild_in(0, np->nh_addr),
	      np->nh_type,
	      np->nh_ifap->ifa_addr,
	      np->nh_ifap->ifa_name,
	      np->nh_refcount);
#endif	/* DEBUG */
    }

    return np;
}


/*
 * Add next hop entry to nh_block
 */
struct NH_BLOCK *
add_nh_entry __PF3(ifap, if_addr *,
		   nh_addr, u_int32,
		   type, int)
{
    register struct NH_BLOCK *np;

    GNTOHL(nh_addr);

    NH_LIST(np) {
	if (nh_addr < ntohl(np->nh_addr)) {
	    /* They are in order so we must have passed it */

	    break;
	} else if (nh_addr == ntohl(np->nh_addr) &&
	    type == np->nh_type &&
	    ifap == np->nh_ifap) {
	    /* We have a match */

	    goto Return;
	}
    } NH_LIST_END(np) ;

    /* Allocate a new one */
    if (!nh_block_index) {
	nh_block_index = task_block_init(sizeof (*np));
    }
    /* Insert before the current element */
    insque((struct qelem *) task_block_alloc(nh_block_index),
	   (struct qelem *) np->nh_back);
    np = np->nh_back;
    assert(!np->nh_addr);

    /* Fill it in */
    np->nh_addr = htonl(nh_addr);
    np->nh_type = type;
    np->nh_ifap = ifa_alloc(ifap);

#ifdef	DEBUG
    trace(TR_OSPF, 0, "add_nh_entry: ADD %A type %d interface %A (%s)",
	  sockbuild_in(0, np->nh_addr),
	  np->nh_type,
	  np->nh_ifap->ifa_addr,
	  np->nh_ifap->ifa_name);
#endif	/* DEBUG */

 Return:
    return alloc_nh_entry(np);
}


/*
 *	Remove a NH entry
 */
void
free_nh_entry __PF1(npp, struct NH_BLOCK **)
{
    register struct NH_BLOCK *np = *npp;

    assert(!np || np->nh_back);

    if (np) {
#ifdef	DEBUG
	trace(TR_OSPF, 0, "free_nh_entry: FREE %A type %d interface %A (%s) refcount %d",
	      sockbuild_in(0, np->nh_addr),
	      np->nh_type,
	      np->nh_ifap->ifa_addr,
	      np->nh_ifap->ifa_name,
	      np->nh_refcount-1);
#endif	/* DEBUG */

	if (!--np->nh_refcount) {

	    trace(TR_OSPF, 0, "free_nh_entry: DELETE %A type %d interface %A (%s)",
		  sockbuild_in(0, np->nh_addr),
		  np->nh_type,
		  np->nh_ifap->ifa_addr,
		  np->nh_ifap->ifa_name);

	    ifa_free(np->nh_ifap);

	    remque((struct qelem *) np);
	
	    task_block_free(nh_block_index, (void_t) np);
	}
    }

    *npp = (struct NH_BLOCK *) 0;
}


/*
 * Find appropriate non-virtual nh_block entry
 */
static struct NH_BLOCK *
find_nh_entry __PF2(v, struct LSDB *,
		    if_ip_addr, u_int32)
{
    register struct NH_BLOCK *np = &ospf.nh_list;

    switch (LS_TYPE(v)) {
    case LS_STUB:
	/* either host or point-to-point interface */
	if (DB_NET(v)->net_mask == HOST_NET_MASK) {
	    NH_LIST(np) {
		if (np->nh_type != NH_VIRTUAL &&
		    BIT_TEST(np->nh_ifap->ifa_state, IFS_POINTOPOINT|IFS_LOOPBACK) &&
		    np->nh_addr == LS_ID(v))
		    break;
	    } NH_LIST_END(np) ;
	} else {
	    NH_LIST(np) {
		if (np->nh_type != NH_VIRTUAL &&
		    !BIT_TEST(np->nh_ifap->ifa_state, IFS_POINTOPOINT) &&
		    NH_NET(np) == DB_NETNUM(v))
		    break;
	    } NH_LIST_END(np) ;
	}
#ifdef NOTDEF
	/* go through host list, if should be configured */
	/* need to look up IF_NDX of host??? */
	for (host = ospf.area[i].hosts.ptr[NEXT],
	     host != HOSTNULL;
	     host = host->ptr[NEXT])
	    if (host->ipaddr == LS_ID(v))
		break;
#endif
	break;

    case LS_NET:
	NH_LIST(np) {
	    if (np->nh_type != NH_VIRTUAL &&
		!BIT_TEST(np->nh_ifap->ifa_state, IFS_POINTOPOINT) &&
		NH_NET(np) == DB_NETNUM(v))
		break;
	} NH_LIST_END(np) ;
	break;

    case LS_RTR:
	/* Find adjacent neighbor */
	NH_LIST(np) {
	    if (np->nh_type != NH_VIRTUAL &&
		np->nh_addr == if_ip_addr)
		break;
	} NH_LIST_END(np) ;
	break;
    }

    if (np == &ospf.nh_list) {
	trace(TR_ALL, LOG_ERR, "find_nh_entry: FAILED!  LinkStateType %s LinkStateID %A AdvRtr %A NextHop %A",
	      trace_state(ospf_ls_type_bits, LS_TYPE(v)),
	      sockbuild_in(0, LS_ID(v)),
	      sockbuild_in(0, ADV_RTR(v)),
	      sockbuild_in(0, if_ip_addr));

	assert(FALSE);
    }

    return alloc_nh_entry(np);
}


/*
 * Combine the next hop lists of parents in sorted order
 */
static void
nh_list_add __PF2(v, struct LSDB *,
		  parent, struct LSDB *)
{
    struct NH_BLOCK *p[MAXNH * 2];
    int vndx, pndx, i, tot = DB_NH_CNT(v) + DB_NH_CNT(parent);

    vndx = pndx = 0;
    for (i = 0; i < (MAXNH * 2); i++)
	p[i] = 0;
    /* merge lists into p based on nh_addr */
    for (i = 0; i < (tot); i++) {
	if (vndx >= DB_NH_CNT(v)) {
	    p[i] = DB_NH_NDX(parent, pndx);
	    pndx++;
	} else if (pndx >= DB_NH_CNT(parent)) {
	    p[i] = DB_NH_NDX(v, vndx);
	    vndx++;
	} else if (DB_NH_NDX(v, vndx) == DB_NH_NDX(parent, pndx)) {
	    /* same next hop */
	    p[i] = DB_NH_NDX(v, vndx);
	    vndx++;
	    pndx++;
	    tot--;
	} else if (ntohl(DB_NH_NDX(v, vndx)->nh_addr) <=
		   ntohl(DB_NH_NDX(parent, pndx)->nh_addr)) {
	    p[i] = DB_NH_NDX(v, vndx);
	    vndx++;
	} else {
	    p[i] = DB_NH_NDX(parent, pndx);
	    pndx++;
	}
    }

    DB_NH_CNT(v) = 0;
    for (i = 0; i < (MAXNH); i++) {
	if (!p[i]) {
	    return;
	}
	DB_NH_NDX(v, i) = alloc_nh_entry(p[i]);
	DB_NH_CNT(v)++;
    }
}


/*
 *	free db's parent list
 */
void
freeplist __PF1(db, struct LSDB *)
{
    int i;

    if (!DB_NH_CNT(db))
	return;

    DB_DIRECT(db) = FALSE;
    for (i = 0; i < DB_NH_CNT(db); i++) {
	if (DB_NH_NDX(db, i)) {
	    free_nh_entry(&DB_NH_NDX(db, i));
	}
    }
    DB_NH_CNT(db) = 0;
}


/*
 *      Spf(a) has just been run - cleanup unused stubs
 */
static void
stub_cleanup __PF1(a, struct AREA *)
{
    struct LSDB *db, *hp;

    for (hp = a->htbl[LS_STUB]; hp < &(a->htbl[LS_STUB][HTBLSIZE]); hp++) {
	for (db = DB_NEXT(hp); db; db = DB_NEXT(db)) {
	    if (NO_GUTS(db))
		continue;

	    if (!DB_ROUTE(db)) {
		freeplist(db);
		db_free(db);
	    }
	}
    }
}


/*
 *  spfinit()    free parent list and reset vertex variables
 */
static void
spfinit __PF1(a, struct AREA *)
{
    int type;
    struct LSDB *hp, *db;

    for (type = LS_STUB; type <= LS_NET; type++)
	for (hp = a->htbl[type]; hp < &(a->htbl[type][HTBLSIZE]); hp++) {
	    for (db = DB_NEXT(hp); db; db = DB_NEXT(db)) {
		if (NO_GUTS(db) ||
		    DB_FREEME(db))
		    continue;
		DB_WHERE(db) = UNINITIALIZED;
		DB_VIRTUAL(db) = FALSE;
		DB_PTR(db)[LAST] = DB_PTR(db)[NEXT] = LSDBNULL;
		if (type == LS_NET || type == LS_STUB) {
		    if (DB_ROUTE(db))
			ORT_CHANGE(DB_ROUTE(db)) = E_UNCHANGE;
		} else {
		    if (DB_AB_RTR(db))
			RRT_CHANGE(DB_AB_RTR(db)) = E_UNCHANGE;
		    if (DB_ASB_RTR(db))
			RRT_CHANGE(DB_ASB_RTR(db)) = E_UNCHANGE;
		}
		DB_DIST(db) = RTRLSInfinity;
		freeplist(db);
	    }
	}
}

/*
 * enq v in order on candidate list
 */
static void
clenq __PF2(a, struct AREA *,
	    v, struct LSDB *)
{
    struct LSDB *nextv;

    /* Remove from previous queue */
    DEL_DBQ(v);
    nextv = DB_PTR(&(a->candidates))[NEXT];
    if ((nextv == LSDBNULL) || (DB_DIST(nextv) > DB_DIST(v))) {
	ADD_DBQ(&(a->candidates), v);
    } else {				/* install in order */
	while(DB_PTR(nextv)[NEXT] && DB_DIST(DB_PTR(nextv)[NEXT]) < DB_DIST(v))
	     nextv = DB_PTR(nextv)[NEXT];
	/* MODIFIED 7/24/92 */
	while(DB_PTR(nextv)[NEXT] && 
	      DB_DIST(DB_PTR(nextv)[NEXT]) == DB_DIST(v) &&
	      LS_TYPE(DB_PTR(nextv)[NEXT]) == LS_NET)
	     nextv = DB_PTR(nextv)[NEXT];
	ADD_DBQ(nextv, v);
    }
}


/*
 * Resolve virtual next hops
 */
static int
resolve_virtual __PF2(v, struct LSDB *,
		      parent, struct LSDB *)
{
    int hash;
    struct LSDB *db;
    struct AREA *transarea = DB_TRANS_AREA(v);
    struct OSPF_ROUTE *rr;
    u_int32 newmetric = SUMLSInfinity;
   
    switch (LS_TYPE(v)) {
    case LS_NET:
    case LS_STUB:
	hash = XHASH(DB_NETNUM(v), 0);
	/* Zip through sum nets for transarea */
	db = &(transarea->htbl[LS_SUM_NET][hash]);
	{
	    for (db = DB_NEXT(db); db; db = DB_NEXT(db)) {
		/* or mania */
		if ((NO_GUTS(db)) ||
		    (BIG_METRIC(db) == SUMLSInfinity) ||
		    (DB_FREEME(db)) ||
		    (LS_AGE(db) >= MaxAge) ||
		    (ADV_RTR(db) == MY_ID) ||
		    (DB_NETNUM(v) != DB_NETNUM(db))) {
		    continue;
		}
		if (!(rr = rtr_findroute(transarea,
					ADV_RTR(db),
					DTYPE_ABR,
					PTYPE_INTRA))) {
		    continue;
		}
		if ((RRT_COST(rr) + BIG_METRIC(db)) < newmetric) {
    		    /* MODIFIED 8/19/92 - check for DB_NH_CNT */
		    if (DB_NH_CNT(parent)) {
		    	DB_NH_CNT(v) = 1;
		    	DB_DIST(v) = (newmetric = RRT_COST(rr) + BIG_METRIC(db));
		    	DB_NH_NDX(v, 0) = alloc_nh_entry(RRT_NH_NDX(rr));
		    }
		}
	    }
	}
	break;
	
    case LS_RTR:
	hash = XHASH(LS_ID(v), 0);
	/*
	 * resolve only if AS Border route
	 */
	if ((ntohs(DB_RTR(v)->E_B) & bit_E)) {
	    /*
	     * First check for intra-area route
	     */
	    rr = rtr_findroute(transarea,
			      LS_ID(v),
			      DTYPE_ASBR,
			      PTYPE_INTRA);
	    if (rr) {
		DB_DIST(v) = RRT_COST(rr);
		DB_NH_NDX(v, 0) = alloc_nh_entry(RRT_NH_NDX(rr));
		DB_NH_CNT(v) = 1;
		return TRUE;
	    }
	    /*
	     * Zip through sum ASBs for transarea
	     */
	    db = &(transarea->htbl[LS_SUM_ASB][hash]);
	    {
		for (db = DB_NEXT(db); db; db = DB_NEXT(db)) {
		    /* or mania */
		    if ((NO_GUTS(db)) ||
			(BIG_METRIC(db) == SUMLSInfinity) ||
			(DB_FREEME(db)) ||
			(LS_AGE(db) >= MaxAge) ||
			(ADV_RTR(db) == MY_ID) ||
			(LS_ID(db) != LS_ID(v))) {
			continue;
		    }
		    if (!(rr = rtr_findroute(transarea,
					     ADV_RTR(db),
					     DTYPE_ABR,
					     PTYPE_INTRA))) {
			continue;
		    }
		    if ((RRT_COST(rr) + BIG_METRIC(db)) < newmetric) {
			DB_DIST(v) =
			    (newmetric = RRT_COST(rr) + BIG_METRIC(db));
			DB_NH_NDX(v, 0) = alloc_nh_entry(RRT_NH_NDX(rr));
			DB_NH_CNT(v) = 1;
		    }
		}
	    }
	} else {
	    /*
	     * Non-asb router link
	     */
	    DB_VIRTUAL(v) = TRUE;
	    DB_TRANS_AREA(v) = transarea;
	    nh_list_add(v, parent);
	}
	break;

    default:
	break;
    }

    /* MODIFIED 8/19/92 - check for DB_NH_CNT */
    if (DB_NH_CNT(v)) {
	return TRUE;
    }

    return FALSE;
}


/*
 *	Add parent to vertex
 *	- used by all entries except for ase() who rolls its own
 */
int
ospf_add_parent __PF7(v, struct LSDB *,
		      parent, struct LSDB *,
		      newdist, u_int32,
		      a, struct AREA *,
		      nh, u_int32,
		      vnh_ndx, struct NH_BLOCK *,
		      transarea, struct AREA *)
{
    int ret = TRUE;
    
    if (v == DB_PTR(&(a->spf))[NEXT])
	return ret;

    /*
     * If there are no parents or this is an Intra area route
     * place on candidate list
     */
    /* MODIFIED 7/24/92 */
    DB_DIST(v) = newdist;
    if (!DB_NH_CNT(v) && LS_TYPE(v) < LS_SUM_NET) {
	/*
	 * put it in order on the candidate list
	 */
	if (LS_TYPE(v) != LS_STUB)
	    clenq(a, v);
	DB_WHERE(v) = ON_CLIST;
    }

    /*
     * set rtr if addr for virtual link dest ip addr
     */
    if (transarea) {
	/*
	 * virtual link
	 */
	DB_NH_NDX(v, 0) = alloc_nh_entry(vnh_ndx);
	DB_NH_CNT(v) = 1;
	DB_VIRTUAL(v) = TRUE;
	DB_TRANS_AREA(v) = transarea;
    } else if ((DB_VIRTUAL(parent) == TRUE) && (LS_TYPE(v) < LS_SUM_NET)) {
	/*
	 * Flag for children of this virtual vertex
	 */
	DB_VIRTUAL(v) = TRUE;
	DB_TRANS_AREA(v) = DB_TRANS_AREA(parent);
	/*
	 * If transit area hasn't come around yet
	 * remove from candidate list
	 */
	/* MODIFIED 8/19/92  */
	if (!resolve_virtual(v, parent)) {
	    if (LS_TYPE(v) != LS_STUB) {
		DEL_DBQ(v);
	    }
	    ret = FALSE;
	}
    } else if (ADV_RTR(parent) == MY_ID) {
	/*
	 * if adjacent to this rtr, first hop is direct
	 */
	DB_NH_NDX(v, 0) = find_nh_entry(v, nh);
	DB_NH_CNT(v) = 1;
	DB_DIRECT(v) = TRUE;
    } else if (DB_DIRECT(parent) && LS_TYPE(parent) == LS_NET) {
	/*
	 * if this rtr is DR (parent) - is also direct
	 */
	if (ADV_RTR(v) == MY_ID) {
	    DB_NH_NDX(v, 0) = find_nh_entry(v, nh);
	} else {
	    DB_NH_NDX(v, 0) = add_nh_entry(DB_NH_NDX(parent, 0)->nh_ifap,
					   nh,
					   NH_NBR);
	}
	DB_NH_CNT(v) = 1;
    } else {
	/*
	 * bind to parent's next hop interface addr
	 */
	nh_list_add(v, parent);
	if (LS_TYPE(v) == LS_STUB
	    && DB_NH_NDX(v, 0)->nh_type == NH_LOCAL) {
	    /* This is local end of P2P interface */
	    DB_DIRECT(v) = TRUE;
	}
    }

    return ret;
}


/*
 *	Get next candidate (closest to the root)
 *	  and install it on the spftree
 */
static void
getnextcandidate __PF2(a, struct AREA *,
		       v, struct LSDB **)
{

    if (!((*v) = DB_PTR(&(a->candidates))[NEXT]))
	return;
    DEL_DBQ(*v);
}

/*
 *  	Check rtr vertex to see if there is a link back to id
 *		Add vertex's interface address to vertex
 */
static int
rtr_backlink __PF4(nextv, struct LSDB *,
		   v, struct LSDB *,
		   type, int,
		   if_ip_addr, u_int32 *)
{
    int i;
    struct RTR_LA_PIECES *linkp;
    int cnt = ntohs(DB_RTR(nextv)->lnk_cnt);
    U_INT32 metric = RTRLSInfinity;

    *if_ip_addr = 0;
    if (cnt == 0) {
        return FALSE;
    }

    for (i = 0, linkp = &(DB_RTR(nextv)->link);
         i < cnt;
         i++, linkp = (struct RTR_LA_PIECES *) ((long) linkp +
						RTR_LA_PIECES_SIZE +
						((linkp->metric_cnt) * RTR_LA_METRIC_SIZE))) {
        if (linkp->lnk_id == LS_ID(v) && linkp->con_type == type) {
            if (linkp->tos0_metric < metric) {
                *if_ip_addr = linkp->lnk_data;
                metric = linkp->tos0_metric;
            }
        }
    }

    return (*if_ip_addr) ? TRUE : FALSE;
}


/*
 *  	Check net vertex to see if there is a link back to id
 */
static int
net_backlink __PF2(id, u_int32,
		   db, struct LSDB *)
{
    int		i;
    int		cnt = (ntohs(LS_LEN(db)) - NET_LA_HDR_SIZE) / 4;
    struct	NET_LA_PIECES *att_rtr;

    for (i = 0, att_rtr = &(DB_NET(db)->att_rtr); i < cnt; i++, att_rtr++) {
	if (att_rtr->lnk_id == id)
	    return TRUE;
    }

    return FALSE;
}


/*
 * Update intra area routing table updates
 */
static int
rtr_update __PF1(area, struct AREA *)
{
    struct OSPF_ROUTE *rr, *next;
    area->asbr_cnt = 0;
    area->abr_cnt = 0;

    /*
     * Update as border rtrs
     */
    for (rr = area->asbrtab.ptr[NEXT];
	 rr;
	 rr = next) {
	next = RRT_NEXT(rr);
	/*
	 * Add Asb rtr summary info to other areas - prepare to flood
	 */
	if ((IAmBorderRtr) && (RRT_ADVRTR(rr) != MY_ID)) {
	    if (build_sum_asb(area, rr, area)) {
		area->spfsched = INTRASCHED;
		return FLAG_NO_BUFS;
	    }
	}
	if (RRT_REV(rr) != RTAB_REV) {   /* no longer valid route *//*
					 * If not marked invalid, flag build_sum_asb to remove
					 * for now  unbind from lsdb entry
					 */
	    if (RRT_V(rr)) {
		DB_ASB_RTR(RRT_V(rr)) = (struct OSPF_ROUTE *) 0;
		freeplist(RRT_V(rr));	/* free parent vertex list */
	    }
	    /*
	     * delete from routing table
	     */
	    DEL_Q(rr, TRUE, OMEM_ORT);
	} else
	    area->asbr_cnt++;
    }

    /*
     * Update area border rtrs
     * These are entered into the routing tble if this area isn't the
     * backbone for virtual endpoints to exchange routing info
     */
    for (rr = area->abrtab.ptr[NEXT];
	 rr;
	 rr = next) {
	next = RRT_NEXT(rr);
	if (RRT_REV(rr) != RTAB_REV) {
	    /*
	     * no longer valid route
	     * unbind from lsdb entry
	     */
	    if (RRT_V(rr)) {
		DB_AB_RTR(RRT_V(rr)) = (struct OSPF_ROUTE *) 0;
		/* free parent vertex list */
		freeplist(RRT_V(rr));
	    }
	    DEL_Q(rr, TRUE, OMEM_ORT);
	} else if (area->area_id != OSPF_BACKBONE && RRT_ADVRTR(rr) != MY_ID) {
	    RRT_CHANGE(rr) = E_UNCHANGE;
	    area->abr_cnt++;
	}
    }
    return FLAG_NO_PROBLEM;
}


/*
 * get_transarea() - return area if nbr_id of virtual link == nh_addr
 */
static struct AREA *
get_transarea __PF1(virt_id, u_int32)
{
    struct INTF *intf;

    VINTF_LIST(intf) {
	if (NBR_ID(&intf->nbr) == virt_id) {
	    return intf->transarea;
	}
    } VINTF_LIST_END(intf) ;
    return AREANULL;
}


/*
 *	set_virtual_addr() - called by intra when
 *		1) virtual links are configured,
 * 		2) v is a rtr adv with the ABR bit set
 *		3) area is transit area
 *	set up ifspfndx and virtual nbr's ip address
 */
static void
set_virtual_addr __PF3(v, struct LSDB *,
		       area, struct AREA *,
		       if_ip_addr, u_int32)
{
    struct INTF *intf;

    VINTF_LIST(intf) {
	if (ADV_RTR(v) == NBR_ID(&intf->nbr) &&
	    intf->transarea == area) {
	    if (intf->nbr.nbr_addr) {
		sockfree(intf->nbr.nbr_addr);
	    }
	    intf->nbr.nbr_addr = sockdup(sockbuild_in(0, if_ip_addr));
	    intf->ifap = ifa_alloc(DB_NH_NDX(v, 0)->nh_ifap);
	    intf->nbr.intf = intf;
	    break;
	}
    } VINTF_LIST_END(intf) ;
}


/*
 * First handle all intra-area routes
 */
static int
intra __PF1(a, struct AREA *)
{
    int i, cnt;
    u_int32 newdist;
    int foundlsa, was_added;
    int nomem = 0;		/* if out of memory will run again in
				 * tq_lsa_lock */
    u_int lookup;			/* RTR or NET LSA lookup in LSDB */
    struct LSDB *v, *nextv;	/* current spfnode */
    struct NET_LA_PIECES *att_rtr;
    struct RTR_LA_PIECES *linkp;
    struct LSDB *stublist = LSDBNULL, *stub;
    struct AREA *transarea;	/* transit area for virtual route */
    struct OSPF_ROUTE *vr;	/* for virtual route */
    u_int32 nh;			/* for discovering next hop address */
    struct NH_BLOCK *vnh_ndx = (struct NH_BLOCK *) 0;		/* virtual next hop ndx */

    v = FindLSA(a, MY_ID, MY_ID, LS_RTR);
    assert(v);

    if (BIT_TEST(ospf.trace_flags, TR_OSPF_SPF)) {
	trace(TR_OSPF, 0, "OSPF RUN SPF\tIntra  Area %A",
	      sockbuild_in(0, a->area_id));
	trace(TR_OSPF, 0, NULL);
    }

    DB_DIST(v) = 0;
    DB_WHERE(v) = ON_RTAB;
    ADD_DBQ(&(a->spf), v);
    for (; v; getnextcandidate(a, &v)) {
	/* if no candidates left we are finished */
	if (LS_TYPE(v) != LS_STUB)
	    nomem = addroute(a, v, INTRASCHED, a);
	if (nomem) {
	    a->spfsched = INTRASCHED;
	    return FLAG_NO_BUFS;
	}
	transarea = AREANULL;
	was_added = 0;
	
	if ((ADV_AGE(v) >= MaxAge) || DB_FREEME(v))
	    continue;

	switch (LS_TYPE(v)) {
	case LS_RTR:
	    /* first handle ls rtr update */
	    /*
	     * check all links
	     */
	    cnt = ntohs(DB_RTR(v)->lnk_cnt);
	    for (i = 0, linkp = (struct RTR_LA_PIECES *) & (DB_RTR(v)->link);
		 i < cnt;
		 linkp = (struct RTR_LA_PIECES *) ((long) linkp + RTR_LA_PIECES_SIZE +
			  ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE)), i++) {

		newdist = ntohs(linkp->tos0_metric) + DB_DIST(v);

		/*
		 * if it is a transit vertex check other side
		 */
		switch (linkp->con_type) {
		case RTR_IF_TYPE_VIRTUAL:
		    if (linkp->tos0_metric == RTRLSInfinity)
			continue;
		    lookup = LS_RTR;
		    /*
		     * check for virtual link
		     */
		    if (a->area_id == OSPF_BACKBONE && ADV_RTR(v) == MY_ID) {
			transarea = get_transarea(linkp->lnk_id);
			if (transarea == AREANULL) {
			    continue;
			}
			/*
			 *lookup area bdr rtr for virtual link
			 */
			vr = rtr_findroute(transarea,
					   linkp->lnk_id,
					   DTYPE_ABR,
					   PTYPE_INTRA);
			if (!vr) {
			    continue;
			}
			/*
			 * add new virtual nh_entry
			 */
			vnh_ndx = add_nh_entry(RRT_IO_NDX(vr),
					       RRT_NH(vr),
					       NH_VIRTUAL);
		    }
		    goto common;
		    
		case RTR_IF_TYPE_RTR:
		    if (linkp->tos0_metric == RTRLSInfinity)
			continue;
		    lookup = LS_RTR;
		    goto common;

		case RTR_IF_TYPE_TRANS_NET:
		    lookup = LS_NET;
		    /* Fall through */

		common:
		    /*
		     * if not in lsdb, next vertex
		     */
		    if (!(nextv = (struct LSDB *) FindLSA(a, linkp->lnk_id, linkp->lnk_id, lookup))) {
			continue;
		    }
		    
		    /*
		     * if it is too old, or on spf tree get next vertex
		     */
		    if ((DB_WHERE(nextv) == ON_RTAB) ||
			(ADV_AGE(nextv) >= MaxAge) ||
			(DB_FREEME(nextv))) {
			continue;
		    }
		    /*
		     * if no backlink, next vertex
		     */
		    switch (linkp->con_type) {
		    case RTR_IF_TYPE_RTR:
		    case RTR_IF_TYPE_VIRTUAL:
			if (!rtr_backlink(nextv,
					  v,
					  (int) linkp->con_type,
					  &nh)) {
			    continue;
			}
			/*
			 * This rtrs link data holds nbr's ip address
			 */
			break;

		    case RTR_IF_TYPE_TRANS_NET:
			if (!net_backlink(LS_ID(v), nextv)) {
			    continue;
			}
			nh = linkp->lnk_data;
			break;

		    default:
			break;
		    }
		    /*
		     * if newdist is > than current, next vertex
		     */
		    if (newdist > DB_DIST(nextv)) {
			continue;
		    }
		    /*
		     * add equal cost route
		     */
		    if (newdist == DB_DIST(nextv)) {
			ospf_add_parent(nextv,
					v,
					newdist,
					a,
					nh,
					vnh_ndx,
					transarea);
			if (ntohs(DB_RTR(nextv)->E_B) & bit_B &&
			    BIT_TEST(a->area_flags, OSPF_AREAF_TRANSIT) &&
			    linkp->con_type == RTR_IF_TYPE_RTR) {
			    set_virtual_addr(nextv, a, nh);
			}
		    } else if (newdist < DB_DIST(nextv)) {
			freeplist(nextv);
			DB_VIRTUAL(nextv) = FALSE;
			ospf_add_parent(nextv,
					v,
					newdist,
					a,
					nh,
					vnh_ndx,
					transarea);
			/* Potential virtual neighbor */
			if (ntohs(DB_RTR(nextv)->E_B) & bit_B &&
			    BIT_TEST(a->area_flags, OSPF_AREAF_TRANSIT) &&
			    linkp->con_type == RTR_IF_TYPE_RTR) {
			    set_virtual_addr(nextv, a, nh);
			}
		    }
		    break;

		case RTR_IF_TYPE_STUB_NET:
		    /*
		     * stub -  will handle after spf algorithm
		     */
		    foundlsa = ospf_add_stub_lsa(&stub,
						 a,
						 RTR_ADV_NETNUM(linkp),
						 ADV_RTR(v),
						 linkp->lnk_data);
		    if (!foundlsa) {
			STUB_HDR_ALLOC(DB_NET(stub), NET_LA_HDR_SIZE);
			LS_ID(stub) = linkp->lnk_id;
			ADV_RTR(stub) = ADV_RTR(v);
			DB_MASK(stub) = linkp->lnk_data;
			LS_LEN(stub) = htons(NET_LA_HDR_SIZE);
			DB_TIME(stub) = DB_TIME(v);
			/* add ls checksum */
			ospf_checksum(DB_NET(stub), NET_LA_HDR_SIZE);
			was_added = ospf_add_parent(stub,
						    v,
						    newdist,
						    a,
						    linkp->lnk_id,
						    vnh_ndx,
						    transarea);

			/* Trace this build */
			if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
			    ospf_trace_build(a, a, DB_ADV(stub), FALSE);
			}
		    } else {
			DB_TIME(stub) = DB_TIME(v);
			if (newdist == DB_DIST(stub))
			    was_added = ospf_add_parent(stub,
							v,
							newdist,
							a,
							linkp->lnk_id,
							vnh_ndx,
							transarea);
			if (newdist < DB_DIST(stub)) {
			    freeplist(stub);
			    was_added = ospf_add_parent(stub,
							v,
							newdist,
							a,
							linkp->lnk_id,
							vnh_ndx,
							transarea);
			}
		    }
		    if (was_added) {
			/* Remove just in case... */
			REM_DBQ(stublist, stub);
                        EN_DBQ(stublist, stub);
		    }
		    continue;

		default:
		    break;
		}
	    }			/* end of router links loops */
	    break;

	case LS_NET:
	    /* now zip through the net links */
	    cnt = ntohs(LS_LEN(v)) - NET_LA_HDR_SIZE;
	    for (att_rtr = &(DB_NET(v)->att_rtr), i = 0;
		 i < cnt;
		 att_rtr++, i += 4) {
		/*
		 * check other side
		 */
		if (!(nextv = FindLSA(a, att_rtr->lnk_id, att_rtr->lnk_id, LS_RTR))) {
		    continue;
		}
		/*
		 * if it is on spftree, too old or no back link,
		 *      go to next vertex
		 */
		if (DB_WHERE(nextv) == ON_RTAB ||
		    ADV_AGE(nextv) >= MaxAge ||
		    DB_FREEME(nextv) ||
		    !rtr_backlink(nextv, v, RTR_IF_TYPE_TRANS_NET, &nh)) {
		    continue;
		}
		/*
		 * 0 cost on same net
		 */
		if (DB_DIST(v) > DB_DIST(nextv)) {
		    continue;
		} else if (DB_DIST(v) == DB_DIST(nextv)) {
		    ospf_add_parent(nextv,
				    v,
				    DB_DIST(v),
				    a,
				    nh,
				    (struct NH_BLOCK *) 0,
				    0);
		    /*
		     * Potential virtual neighbor
		     */
		    if (ntohs(DB_RTR(nextv)->E_B) & bit_B &&
			BIT_TEST(a->area_flags, OSPF_AREAF_TRANSIT)) {
			set_virtual_addr(nextv, a, nh);
		    }
		} else if (DB_DIST(v) < DB_DIST(nextv)) {
		    freeplist(nextv);
		    if (DB_WHERE(nextv) == ON_CLIST) {
			DEL_DBQ(nextv);
		    }
		    ospf_add_parent(nextv,
				    v,
				    DB_DIST(v),
				    a,
				    nh,
				    (struct NH_BLOCK *) 0,
				    0);
		    /*
		     * Potential virtual neighbor
		     */
		    if (ntohs(DB_RTR(nextv)->E_B) & bit_B &&
			BIT_TEST(a->area_flags, OSPF_AREAF_TRANSIT)) {
			set_virtual_addr(nextv, a, nh);
		    }
		}
	    }
	    break;

	default:
	    break;
	}
    }

    /* now do stub nets; at this point shortest intra routes are known */
    for (stub = stublist; stub;) {
	stublist = DB_PTR(stub)[NEXT];
	/*
	 * remove from stublist
	 */
	DB_PTR(stub)[LAST] = DB_PTR(stub)[NEXT] = LSDBNULL;
	/*
	 * check for an existing net - could have more than one entry
	 */
	if (addroute(a, stub, INTRASCHED, a)) {
	    a->spfsched = INTRASCHED;
	    return FLAG_NO_BUFS;
	    /* XXX - freeq? */
	}

	stub = stublist;
    }

    return 0;
}


/*
 * Check virtual links to see if they have come up
 *	- if we have an intra area route to the virtual nbr, bring it up
 */
static void
virtual_check __PF1(area, struct AREA *)
{
    int match = FALSE;
    struct OSPF_ROUTE *rr;
    struct INTF *intf;

    VINTF_LIST(intf) {
	if (intf->transarea == area) {
	    /* if we have route, bring link up -
	     * we will wait for hello protocol to discover that
	     * link is down or that other parameters have changed
	     * so virtual links works like everyone else
	     */
	    if (intf->state == IDOWN) {
		match++;
		if (!(rr = rtr_findroute(area,
					 NBR_ID(&intf->nbr),
					 DTYPE_ABR,
					 PTYPE_INTRA))) {
		    continue;
		}
		intf->cost = RRT_COST(rr);
		(*(if_trans[INTF_UP][IDOWN])) (intf);
	    }
	}
    } VINTF_LIST_END(intf) ;
    
    /*
     * have to schedule backbone to rerun if there is a virtual link
     * if there was a match, building the rtr lsa will rerun the dijkstra
     */
    if (!match && BIT_TEST(area->area_flags, OSPF_AREAF_VIRTUAL_UP))
	ospf.backbone.spfsched |= SCHED_BIT(LS_RTR);
}


/*
 * Run Dijkstra on this area
 */
static void
spf __PF1(area, struct AREA *)
{
     struct AREA *sum_area = (IAmBorderRtr) ? &ospf.backbone : area;

    area->spfcnt++;
    RTAB_REV++;

    /*
     * run spf on this area
     */
    spfinit(area);		/* clear spf tree */
    DB_PTR(&(area->candidates))[NEXT] =
	DB_PTR(&(area->spf))[NEXT] = LSDBNULL;
    if (intra(area))
	return;			/* Out of memory */
    /*
     * update asbr and abr routing tables for this area
     */
    rtr_update(area);

    /*
     * Run sum on backbone if this rtr is area border router
     * else run on first (and only) area
     */
    if (netsum(sum_area, ALLSCHED, area, 0))
        return;                 /* Out of memory */
    if (asbrsum(sum_area, ALLSCHED, area, 0))
        return;                 /* Out of memory */

    /*
     * Run ASE on global exteral as LSAs
     */
    if (ase(area, ALLSCHED, 0))
	return;			/* Out o'memory */
    /*
     * Update network routing table
     */
    ntab_update(area, ALLSCHED);

    /*
     * if border rtr, intra routes may have changed so build summary
     * pkts that may have changed sum will inject them into other areas
     * - will build sum nets links that have changed
     */
    if (IAmBorderRtr)
	if (build_sum_net(area))
	    return;

    /*
     * cleanup no longer valid stub network lsdb entries
     */
    stub_cleanup(area);

    /*
     * check virtual links - bring up adjacencies if we have intra route
     */
    if (BIT_TEST(area->area_flags, OSPF_AREAF_TRANSIT)) {
	virtual_check(area);
    }
    area->spfsched = 0;		/* a done deal */
}


/*
 * A call to run the correct level of the spf algorithm
 * - called by Rx routines and tq_lsa_lock
 */
void
run_spf __PF2(area, struct AREA *,
	      partial, int)
{
    int bad_run = FALSE;
    int from = 0;
    struct AREA *a, *sum_area = (IAmBorderRtr) ? &ospf.backbone : area;

    /* Open the routing table */
    rt_open(ospf.task);

    /*
     * Check for a spf sched event having been run - if out of memory during
     * spf process may have scheduled
     */
    if (area->spfsched & (RTRSCHED | NETSCHED)) {
	area->spfsched = 0;
	spf(area);
    } else {
	/*
	 * Only will run sum if area border rtr and area is backbone or not
	 * area border rtr
	 */
	if (area->spfsched & SUMSCHED) {
	    /*
	     * Run either or both
	     */
	     if (!(area->spfsched & SCHED_BIT(LS_SUM_ASB))) {
		 area->spfsched = 0;
		 bad_run = netsum(sum_area, SUMNETSCHED, area, partial);
		 from = (ASESCHED | SUMNETSCHED);
	     } else if (!(area->spfsched & SCHED_BIT(LS_SUM_NET))) {
		 area->spfsched = 0;
		 bad_run = asbrsum(sum_area, SUMASBSCHED, area, partial);
		 from = (ASESCHED | SUMASBSCHED);
	     } else {
		 area->spfsched = 0;
		 bad_run = netsum(sum_area, SUMASESCHED, area, partial);
		 bad_run |= asbrsum(sum_area, SUMASESCHED, area, partial);
		 from = SUMASESCHED;
	     }
	     /* Turn partial off for ASE's */
	     partial = 0;
	} else if (area->spfsched & ASESCHED) {
	    from = ASESCHED;
	    area->spfsched = 0;
	}
	if (!bad_run && from && !BIT_TEST(area->area_flags, OSPF_AREAF_STUB)) {
	    bad_run = ase(area, from, partial);
	}

	/*
	 * Update the network routing table
	 */
	if (!bad_run && from) {
	    /*
	     * Partial deals with updating the routing table
	     */
	    if (!partial)
		ntab_update(area, from);
	    /*
	     * will build sum nets links that have changed
	     */
/* MODIFIED 5/1/92 what the fuck is this doint in here? */
#ifdef notdef
	    if (IAmBorderRtr && (from & SUMNETSCHED))
		build_sum_net(area);
#endif
	}
    }
    /*
     * new sum routes may be on txq or new inter routes may have become
     * available, inject into areas
     */
    AREA_LIST(a) {
	if (a->txq != LLNULL) {
	    self_orig_area_flood(a, a->txq, LS_SUM_NET);
	    ospf_freeq((struct Q **)&(a->txq), OMEM_LL);
	}
    } AREA_LIST_END(a) ;

    rt_close(ospf.task, (gw_entry *) 0, 0, NULL);
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
