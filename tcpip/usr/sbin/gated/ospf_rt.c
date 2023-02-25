static char sccsid[] = "@(#)01	1.1  src/tcpip/usr/sbin/gated/ospf_rt.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:26";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: OSPF_EXPORT_DEQUEUE
 *		OSPF_EXPORT_Q_ADD
 *		OSPF_EXPORT_Q_CHANGE
 *		OSPF_EXPORT_Q_DELETE
 *		OSPF_TSI_GET
 *		OSPF_TSI_PUT
 *		__PF1
 *		__PF2
 *		__PF3
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
 * ospf_rt.c,v 1.27.2.4 1993/08/27 10:14:02 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


static void
ospf_rt_dump  __PF2(fp, FILE *,
		    rt, rt_entry *)
{
    struct LSDB *db = ORT_V(rt);
    
    fprintf(fp,
	    "\t\t\tCost: %d\tArea: %A\tType: %s\tAdvRouter: %A\n",
	    ORT_COST(rt),
	    sockbuild_in(0, ORT_AREA(rt)->area_id),
	    trace_state(ospf_ls_type_bits, ORT_PTYPE(rt)),
	    sockbuild_in(0, ORT_ADVRTR(rt)));
    if (LS_TYPE(db) == LS_ASE) {
	fprintf(fp,
		"\t\t\tType: %d\t\tForward: %A\tTag: %A",
		ASE_TYPE1(db) ? 1 : 2,
		sockbuild_in(0, DB_ASE_FORWARD(db)),
		ospf_path_tag_dump(inet_autonomous_system, rt->rt_tag));
	if (ASE_TYPE2(db)) {
	    fprintf(fp,
		    "\t\t\tExternal Cost: %d",
		    ORT_TYPE2COST(rt));
	}
	fprintf(fp, "\n");
    }
}


static void
ospf_rt_free __PF1(rtd, rt_data *)
{
    OSPF_RT_INFO *rtinfo = (OSPF_RT_INFO *) rtd->rtd_data;
    int i = rtinfo->nh_cnt;

    while (i--) {
	free_nh_entry(&rtinfo->nh_ndx[i]);
    }

    rtinfo->nh_cnt = 0;
}


void
ospf_route_update __PF3(o_rt, rt_entry *,
			area, struct AREA *,
			level, int)
{
    struct NET_RANGE *foundnr = NRNULL, *nr;
    struct LSDB *db = ORT_V(o_rt);
    int should_run_build_inter = FALSE;
    int build_flag = 0;
    int rt_change_flag = FALSE;

    /* A few checks - some paranoid */
    if (!ORT_INFO(o_rt) || 
	!db || 
	NO_GUTS(db) || 
	DB_WHERE(db) == ON_ASE_LIST ||
	!(PTYPE_BIT(ORT_PTYPE(o_rt)) & level) ||
	((PTYPE_BIT(ORT_PTYPE(o_rt)) & PTYPE_INTRA) && area != ORT_AREA(o_rt))) {

	return;
    }

    /*
     * if internal to this area and this is a border rtr,
     * set cost - build_ls will figure out what to do
     */
    if (IAmBorderRtr) {
	if (PTYPE_BIT(ORT_PTYPE(o_rt)) & PTYPE_INTRA ||
	    ORT_CHANGE(o_rt) == E_WAS_INTRA_NOW_ASE || 
	    ORT_CHANGE(o_rt) == E_WAS_INTER_NOW_INTRA) {
	    /* Should only summarize if no virtual links */

	    RANGE_LIST(nr, area) {
	    	/* check for net to be summarized */
	    	if ((nr->mask & RT_DEST(o_rt)) == nr->net) {
		    foundnr = nr;
		    break;
		}
	    } RANGE_LIST_END(nr, area) ;
	    if (!foundnr) {
	    	should_run_build_inter = TRUE;
	    }
    	} else if ((PTYPE_BIT(ORT_PTYPE(o_rt)) & PTYPE_INTER) && 
		   area->area_id == OSPF_BACKBONE) {
	    should_run_build_inter = TRUE;
	}
    }

    /*
     * Check for invalid route:
     * - Revision != current revision &&
     * - Intra route && area that is currently running dijkstra == rt's area ||
     * - Inter or Ase route - correct level (inter or ase) is checked above
     */
    if ((ORT_REV(o_rt) != RTAB_REV) &&
	(((PTYPE_BIT(ORT_PTYPE(o_rt)) & PTYPE_INTRA) && area == ORT_AREA(o_rt)) ||
	 (PTYPE_BIT(ORT_PTYPE(o_rt)) & PTYPE_LEAVES))) {
	if (should_run_build_inter) {
	    /* delete inter-area route from other areas */
	    /* May be out of memory */
	    if (build_inter(db, area, E_DELETE)) {
		return;
	    }
	}

	/* remove route from routing table */
	/* free parent list */
	freeplist(db);
	DB_ROUTE(db) = (rt_entry *) 0;
	rt_delete(o_rt);
	return;
    }

    /* Set low cost for net range */
    if ((foundnr && ORT_COST(o_rt) < foundnr->cost) &&
	ORT_CHANGE(o_rt) != E_WAS_INTRA_NOW_ASE) {
	foundnr->cost = ORT_COST(o_rt);
    }

    /* Handle rtab changes */
    switch (ORT_CHANGE(o_rt)) {
    case E_NEXTHOP:
	rt_change_flag = TRUE;
	break;

    case E_NEW:
	/* if border rtr and bb, add intra routes to all areas */
	if (ORT_AREA(o_rt) == area && should_run_build_inter) {
	    build_flag = E_NEW;
	}
	break;

    case E_WAS_INTER_NOW_INTRA:
	if (should_run_build_inter) {
	    build_flag = E_NEW;
	} else if (IAmBorderRtr && 
		   foundnr && 
		   foundnr->net != DB_NETNUM(db)) {
	    /* Have to delete more specific inter-area route */
	    build_flag = E_DELETE;
	}
	rt_change_flag = TRUE;
	break;

    case E_WAS_INTRA_NOW_ASE:
    case E_WAS_INTER_NOW_ASE:
	if (should_run_build_inter) {
	    build_flag = E_DELETE;
	}
	break;

    case E_WAS_ASE:
	if (ORT_AREA(o_rt) == area && should_run_build_inter) {
	    build_flag = E_WAS_ASE;
	}
	break;

    case E_WAS_INTRA_NOW_INTER:
	if (ORT_AREA(o_rt) == area && should_run_build_inter) {
	    build_flag = ORT_CHANGE(o_rt);
	}

    case E_ASE_METRIC:
    case E_ASE_TYPE:
    case E_ASE_TAG:
	/* External changes require a new route because of policy */
	ORT_CHANGE(o_rt) = E_UNCHANGE;
	build_route(area, ORT_V(o_rt), o_rt->rt_data);
	rt_delete(o_rt);
	return;

    case E_METRIC:
	rt_change_flag = TRUE;
	break;

    case E_UNCHANGE:
	/* Nothing to do */
	return;
	
    default:
	assert(FALSE);
    }

    if (build_flag)  {
	build_inter(db, area, build_flag);
    }

    if (rt_change_flag) {
	register int i = ORT_NH_CNT(o_rt);
	sockaddr_un *routers[RT_N_MULTIPATH];
	pref_t preference = o_rt->rt_preference;

	/* If this is route to a local network we must prevent it from */
	/* becoming the active route */
	if (DB_DIRECT(db)) {
	    if (preference > 0) {
		preference = -preference;
		BIT_SET(o_rt->rt_state, RTS_NOADVISE|RTS_NOTINSTALL);
	    }
	} else {
	    if (preference < 0) {
		preference = -preference;
		BIT_RESET(o_rt->rt_state, RTS_NOADVISE|RTS_NOTINSTALL);
	    }
	}
	
    	while (i--) {
	    routers[i] = sockbuild_in(0, ORT_NH(o_rt, i));
    	}
    	(void) rt_change(o_rt,
			 DB_DIST(db),
			 o_rt->rt_tag,
			 preference,
			 (int) ORT_NH_CNT(o_rt), routers);
    }

    ORT_CHANGE(o_rt) = E_UNCHANGE;
    return;
}


/*
 * ntab_update()
 *	- called by routine who knows which levels have been run before
 * 	- process all routes from this level up
 *      - notify changed or new net routes to real routing table
 *	- notify real routing table of deleted net routes
 *        routes are deletable if:
 *		intra && area == r->area && r->spfcnt != RTAB_REV
 *		inter && r->spfcnt != RTAB_REV
 *		ase && r->spfcnt != RTAB_REV
 */
void
ntab_update __PF2(a, struct AREA *,
		  lvl, int)
{
    struct NET_RANGE *nr1;
    rt_entry *rt;
    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(RTPROTO_OSPF) | RTPROTO_BIT(RTPROTO_OSPF_ASE));

    /* MODIFIED 5/1/92 */
    if (IAmBorderRtr && (lvl & PTYPE_INTRA))		/* set up for net_sum */
	RANGE_LIST(nr1, a) {
	    nr1->cost = SUMLSInfinity;
	} RANGE_LIST_END(nr1, a) ;

    RT_LIST(rt, rtl, rt_entry) {
	if (!BIT_TEST(rt->rt_state, RTS_DELETE|RTS_REJECT)) {
	    ospf_route_update(rt, a, lvl);
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;

    RTLIST_RESET(rtl);
}


/*
 * Allocate route and install it in the routing table
 */
int
build_route __PF3(a, struct AREA *,
		  v, struct LSDB *,
		  rtd, rt_data *)
{
    int i;
    rt_entry *rt;
    OSPF_RT_INFO *rtinfo;
    rt_parms rtparms;

    bzero((caddr_t) &rtparms, sizeof (rtparms));

    rtparms.rtp_dest = sockbuild_in(0, DB_NETNUM(v));
    rtparms.rtp_dest_mask = inet_mask_locate(DB_MASK(v));
    rtparms.rtp_n_gw = DB_NH_CNT(v);
    /* Build the next-hop list */
    i = DB_NH_CNT(v);
    assert(i);
    while (i--) {
	register u_int32 addr = DB_NH_NDX(v, i)->nh_addr;

	assert(addr);

	rtparms.rtp_routers[i] = sockbuild_in(0, addr);
    }
    rtparms.rtp_gwp = ospf.gwp;
    rtparms.rtp_metric = DB_DIST(v);
    rtparms.rtp_state = RTS_NOAGE;
    rtparms.rtp_preference = ospf.preference;
    rtparms.rtp_rtd = rtd;

    switch (LS_TYPE(v)) {
    case LS_SUM_NET:
	/* Summary Networks (Inter-Area routes) */
	BIT_SET(rtparms.rtp_state, RTS_EXTERIOR);
	break;

    case LS_STUB:
    case LS_NET:
	BIT_SET(rtparms.rtp_state, RTS_INTERIOR);
	/* Intra-Area routes */
	break;
	
    case LS_ASE:
	/* AS External routes */

	rtparms.rtp_preference = ospf.preference_ase;
	rtparms.rtp_gwp = ospf.gwp_ase;
	if (IAmASBorderRtr) {
	    (void) import(rtparms.rtp_dest,
			  ospf.import_list,
			  (adv_entry *) 0,
			  (adv_entry *) 0,
			  &rtparms.rtp_preference,
			  (if_addr *) 0,
			  (void_t) v);
	}

	if (ASE_TYPE2(v)) {
	    BIT_SET(rtparms.rtp_state, RTS_EXTERIOR);
	    rtparms.rtp_metric = BIG_METRIC(v);
	} else {
	    BIT_SET(rtparms.rtp_state, RTS_INTERIOR);
	}
	break;
    }

    if (DB_DIRECT(v)) {
	/* We need this route, but gated shouldn't see or use it */

	rtparms.rtp_preference = -rtparms.rtp_preference;
	BIT_SET(rtparms.rtp_state, RTS_NOADVISE|RTS_NOTINSTALL);
    }

    /* Make sure we have a data area */
    if (!rtparms.rtp_rtd) {
	rtparms.rtp_rtd = rtd_alloc(sizeof (OSPF_RT_INFO));
	rtparms.rtp_rtd->rtd_dump = ospf_rt_dump;
	rtparms.rtp_rtd->rtd_free = ospf_rt_free;
    }

    /* Build the OSPF specific extension */
    rtinfo = (OSPF_RT_INFO *) rtparms.rtp_rtd->rtd_data;

    rtinfo->change = E_NEW;
    rtinfo->v = v;
    rtinfo->revision = RTAB_REV;
    rtinfo->area = a;
    rtinfo->cost = DB_DIST(v);
    rtinfo->ptype = LS_TYPE(v);
    rtinfo->advrtr = ADV_RTR(v);
    ospf_rt_free(rtparms.rtp_rtd);
    i = rtinfo->nh_cnt = DB_NH_CNT(v); 
    while (i--) {
	rtinfo->nh_ndx[i] = alloc_nh_entry(DB_NH_NDX(v, i));
    }
    rtinfo->etype = FALSE;
    if (LS_TYPE(v) == LS_ASE) {
	if (ASE_TYPE2(v)) {
	    rtinfo->etype = TRUE;	/* XXX - INTERIOR/EXTERIOR */
	    rtinfo->type2cost = BIG_METRIC(v);
	}
	rtparms.rtp_tag = ntohl(LS_ASE_TAG(v));
    }

    rt = rt_add(&rtparms);
    if (!rt) {
	rtd_unlink(rtparms.rtp_rtd);
	/* XXX - Couldn't add the route, but if I return FALSE the SPF will not finish */
	return TRUE;
    }

    DB_ROUTE(v) = rt;

    return TRUE;
}



/*
 * Bind this route to this vertex
 */
void
rvbind __PF3(rt, rt_entry *,
	     v, struct LSDB *,
	     a, struct AREA *)
{
#define TYPE_CHANGE(old, new)	((old << 8) | new)
    int change = TYPE_CHANGE(ORT_PTYPE(rt), LS_TYPE(v));
    time_t timestamp = (time_t) 0;
    int nh_is_diff = FALSE;
    int i;

    if (ORT_NH_CNT(rt) != DB_NH_CNT(v)) {
	nh_is_diff = TRUE;
    } else {
    	for (i = 0; i < ORT_NH_CNT(rt); i++)
    	    if (ORT_NH_NDX(rt, i) != DB_NH_NDX(v, i)) {
	    	nh_is_diff = TRUE;
		break;
	    }
    }

    /* break link from LSDB to routing table */
    if (ORT_V(rt)) {
	timestamp = DB_TIME(ORT_V(rt));
	DB_ROUTE(ORT_V(rt)) = (rt_entry *) 0;
    }

    /* Lets assume no change */
    ORT_CHANGE(rt) = E_UNCHANGE;

    /* Figure out what (if anything) has changed */
    switch (change) {
    case TYPE_CHANGE(LS_STUB, LS_NET):
    case TYPE_CHANGE(LS_NET, LS_STUB):
    case TYPE_CHANGE(LS_NET, LS_NET):
    case TYPE_CHANGE(LS_STUB, LS_STUB):
    case TYPE_CHANGE(LS_SUM_NET, LS_SUM_NET):
	if (nh_is_diff || timestamp == DB_TIME(v)) {
	    ORT_CHANGE(rt) = E_NEXTHOP;
	} else if (DB_DIST(v) != ORT_COST(rt)) {
	    ORT_CHANGE(rt) = E_METRIC;
	}
	break;
	
    /*
     * The next two cases are a bit weird since they imply that
     * there is an inter-area net with the same net number as an intra-area net 
     */
    case TYPE_CHANGE(LS_STUB, LS_SUM_NET):
    case TYPE_CHANGE(LS_NET, LS_SUM_NET):
	ORT_CHANGE(rt) = E_WAS_INTRA_NOW_INTER;
	break;

    case TYPE_CHANGE(LS_SUM_NET, LS_STUB):
    case TYPE_CHANGE(LS_SUM_NET, LS_NET):
	ORT_CHANGE(rt) = E_WAS_INTER_NOW_INTRA;
	break;
	
    case TYPE_CHANGE(LS_STUB, LS_ASE):
    case TYPE_CHANGE(LS_NET, LS_ASE):
	build_route(a, v, rt->rt_data);
	ORT_CHANGE(rt) = E_WAS_INTRA_NOW_ASE;
	rt_delete(rt);
	return;

    case TYPE_CHANGE(LS_SUM_NET, LS_ASE):
	build_route(a, v, rt->rt_data);
	ORT_CHANGE(rt) = E_WAS_INTER_NOW_ASE;
	rt_delete(rt);
	return;

    case TYPE_CHANGE(LS_ASE, LS_STUB):
    case TYPE_CHANGE(LS_ASE, LS_NET):
    case TYPE_CHANGE(LS_ASE, LS_SUM_NET):
	build_route(a, v, rt->rt_data);
	ORT_CHANGE(rt) = E_WAS_ASE;
	rt_delete(rt);
	return;

    case TYPE_CHANGE(LS_ASE, LS_ASE):
	if (nh_is_diff) {
	    /* Next hop change */
	    ORT_CHANGE(rt) = E_NEXTHOP;
	} else if (ASE_TYPE2(v) != ORT_ETYPE(rt)) {
	    ORT_CHANGE(rt) = E_ASE_TYPE;
	} else if (ntohl(LS_ASE_TAG(v)) != rt->rt_tag) {
	    ORT_CHANGE(rt) = E_ASE_TAG;
	} else if (DB_DIST(v) != ORT_COST(rt)
		   || (ASE_TYPE2(v)
		       &&BIG_METRIC(v) != ORT_TYPE2COST(rt))) {
	    /* Metric change */
	    ORT_CHANGE(rt) = E_ASE_METRIC;
        }
	break;
    }

    ORT_REV(rt) = RTAB_REV;

    /* Link routing table entry to LSDB */
    ORT_V(rt) = v;
    DB_ROUTE(v) = rt;

    /* Make sure info is up to date */
    ORT_AREA(rt) = a;
    ORT_COST(rt) = DB_DIST(v);
    ORT_PTYPE(rt) = LS_TYPE(v);
    ORT_ADVRTR(rt) = ADV_RTR(v);
    /* Free old pointers */
    for (i = 0; i < ORT_NH_CNT(rt); i++) {
	free_nh_entry(&ORT_NH_NDX(rt, i));
    }
    for (i = 0; i < DB_NH_CNT(v); i++) {
    	ORT_NH_NDX(rt, i) = alloc_nh_entry(DB_NH_NDX(v, i));
    }
    ORT_NH_CNT(rt) = DB_NH_CNT(v);

    ORT_ETYPE(rt) = FALSE;
    if (LS_TYPE(v) == LS_ASE) {
	if (ASE_TYPE2(v)) {
	    ORT_ETYPE(rt) = TRUE;	/* XXX - INTERIOR/EXTERIOR */
	    ORT_TYPE2COST(rt) = BIG_METRIC(v);
	}
	rt->rt_tag = ntohl(LS_ASE_TAG(v));
    }
#undef	TYPE_CHANGE
}


/**/
/*
 *  Exportation of routes into OSPF.
 */

/*
 *  For exporting gated routes to OSPF.
 */
static block_t ospf_export_block_index;

/*
 * Macros to dig change and LSDB pointers out of the tsi field, and stuff
 * them back in the same way.
 */
#define	OSPF_TSI_NONE	0
#define	OSPF_TSI_LSDB	1
#define	OSPF_TSI_CE	2

#define	OSPF_TSI_DATA_SIZE	(sizeof(void_t) + sizeof(byte))

struct ospf_tsi_data {
    void_t ptr;
    byte type;		/* XXX assume no hole */
};

#define	OSPF_TSI_GET(rth, rtbit, cept, lsdbpt) \
    do { \
	struct ospf_tsi_data Xtsi; \
	rttsi_get((rth), (rtbit), (byte *)&Xtsi); \
	switch (Xtsi.type) { \
	case OSPF_TSI_LSDB: \
	    (cept) = NULL; \
	    (lsdbpt) = (struct LSDB *)Xtsi.ptr; \
	    break; \
	case OSPF_TSI_CE: \
	    (cept) = (ospf_export_entry *)Xtsi.ptr; \
	    (lsdbpt) = ((ospf_export_entry *)Xtsi.ptr)->db; \
	    break; \
	default: \
	    (cept) = NULL; \
	    (lsdbpt) = NULL; \
	    break; \
	} \
    } while (0)

#define	OSPF_TSI_PUT(rth, rtbit, cept, lsdbpt) \
    do { \
	struct ospf_tsi_data Xtsi; \
	if ((Xtsi.ptr = (void_t)(cept)) != NULL) { \
	    Xtsi.type = OSPF_TSI_CE; \
	    ((ospf_export_entry *)Xtsi.ptr)->db = (lsdbpt); \
	} else if ((Xtsi.ptr = (void_t)(lsdbpt)) != NULL) { \
	    Xtsi.type = OSPF_TSI_LSDB; \
	} else { \
	    Xtsi.type = OSPF_TSI_NONE; \
	} \
	rttsi_set((rth), (rtbit), (byte *)&Xtsi); \
    } while (0)

/*
 * Macros to enqueue/dequeue export entries.  The problem with delaying
 * ASE exports is that, if you are a transit AS and a delete is long
 * delayed, you can potentially be black-holing a neighbour's traffic
 * for as long as it takes to get the delete to them during a time when
 * they could be using alternate routes.  Because of this it is slightly
 * better to get deletes out faster.  Also, during changes, BGP may
 * be telling lies to your neighbours, so it is desireable to minimize
 * this as well.
 *
 * These macros maintain the export queue such that deletes are queued
 * before changes, which are queued before adds.  This is probably slightly
 * less efficient from OSPF's point-of-view, but is somewhat more satisfying
 * from the point-of-view of the transient.
 */
#define	OSPF_EXPORT_Q_DELETE(ospf, export_ent) \
    do { \
	register struct OSPF *Xospf = (ospf); \
	register ospf_export_entry *Xce = (export_ent); \
	if (Xospf->export_queue_change == Xospf->export_queue_delete) { \
	    Xospf->export_queue_change = Xce; \
	} \
	insque((struct qelem *) Xce, \
	  (struct qelem *) Xospf->export_queue_delete); \
	Xospf->export_queue_delete = Xce; \
	Xospf->export_queue_size++; \
    } while (0)

#define	OSPF_EXPORT_Q_CHANGE(ospf, export_ent) \
    do { \
	register struct OSPF *Xospf = (ospf); \
	register ospf_export_entry *Xce = (export_ent); \
	insque((struct qelem *) Xce, \
	  (struct qelem *) Xospf->export_queue_change); \
	Xospf->export_queue_change = Xce; \
	Xospf->export_queue_size++; \
    } while (0)

#define	OSPF_EXPORT_Q_ADD(ospf, export_ent) \
    do { \
	register struct OSPF *Xospf = (ospf); \
	insque((struct qelem *)(export_ent), \
	  (struct qelem *) Xospf->export_queue.back); \
	Xospf->export_queue_size++; \
    } while (0)

#define	OSPF_EXPORT_DEQUEUE(ospf, export_ent) \
    do { \
	register struct OSPF *Xospf = (ospf); \
	register ospf_export_entry *Xce = (export_ent); \
	if (Xospf->export_queue_delete == Xce) { \
	    Xospf->export_queue_delete = Xce->back; \
	} \
	if (Xospf->export_queue_change == Xce) { \
	    Xospf->export_queue_change = Xce->back; \
	} \
	remque((struct qelem *) Xce); \
	Xospf->export_queue_size--; \
    } while (0)


static void
ospf_tsi_dump __PF3(rth, rt_head *,
		    data, void_t,
		    buf, char *)
{
    task *tp = (task *)data;
    ospf_export_entry *ce;
    struct LSDB *db;

    OSPF_TSI_GET(rth, tp->task_rtbit, ce, db);
    if (ce) {
	(void) sprintf(buf, "%s export entry",
		       tp->task_name);
    } else if (db) {
	(void) sprintf(buf, "%s LSDB seq %8x",
		       tp->task_name,
		       ntohl(LS_SEQ(db)));
    }
}


static void
ospf_ase_export __PF2(tip, timer *,
		      interval, time_t)
{
    struct LSDB_LIST *txq = LLNULL;
    int i = ospf.export_limit;
    register ospf_export_entry *ce;
    task *tp;
    u_int rtbit;

    if (ospf.export_queue.forw == &ospf.export_queue) {
	/* No work to do, reset timer and quit */
	timer_reset(tip);
	return;
    }

    tp = tip->timer_task;
    rtbit = tp->task_rtbit;
    rt_open(tp);

    while ((ce = ospf.export_queue.forw) != &ospf.export_queue &&
	   i--) {
        struct LSDB *db = ce->db;
	struct LSDB_LIST *ll;

	/*
	 * If we're in here we're going to do something to this
	 * route.  Remove any old db pointer from all retransmission
	 * lists if there are any.
	 */
	if (ce->old_rt) {
	    assert(db &&
		   DB_GUTS(db) &&
		   DB_WHERE(db) == ON_ASE_LIST);

	    if (DB_RETRANS(db)) {
		rem_db_retrans(db);
	    }
	} else {
	    assert(db == NULL);
	}

	if (ce->old_rt && !ce->new_rt) {
	    /* Here because we are deleting one we originated */

	    /* Timestamp new origination */
	    DB_TIME(db) = time_sec;

	    /* adjust update for tqLsaAse */
	    if (ospf.cur_ase == db) {
		ospf.cur_ase = DB_PTR(db)[NEXT];
		if (!ospf.cur_ase) {
		    /* removed last on lst */
		    ospf.cur_ase = DB_PTR(&ospf.my_ase_list)[NEXT];
		}
	    }

	    /* Calculate checksum and update checksumsum */
	    LS_AGE(db) = 0;
	    ospf.db_chksumsum -= LS_CKS(db);
	    ospf_checksum_sum(DB_ASE(db), ASE_LA_HDR_SIZE, ospf.db_chksumsum);
	    LS_AGE(db) = MaxAge;

	    /* Log this new ASE */
	    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		ospf_trace_build((struct AREA *) 0, (struct AREA *) 0, DB_ADV(db), FALSE);
	    }

	    /* Remove from ASE list */
	    DEL_DBQ(db);

	    /* Add to free list */
	    DB_WHERE(db) = ON_ASE_INFINITY;
	    DB_FREEME(db) = TRUE;
	    ADD_DBQ(&ospf.db_free_list, db);

	    /* Add to retransmit queue */
	    LL_ALLOC(ll);
	    ll->lsdb = db;
	    EN_Q(txq, ll);

	    /* Set TSI to null and reset bit on old route */
	    rttsi_reset(ce->old_rt->rt_head, rtbit);
	    (void) rtbit_reset(ce->old_rt, rtbit);
	} else {
	    struct ASE_LA_HDR *as_hdr;
	    u_int32 age = 0;
	    int newseq = FALSE;
	    const char *action;

	    /* Either a route change or a new route */
	    if (ce->old_rt) {
		/* Make a change to the existing ASE */

		action = "Change";
	    
		as_hdr = DB_ASE(db);
		if (ce->new_rt != ce->old_rt) {
	    	    (void) rtbit_reset(ce->old_rt, rtbit);
		}

		/* Indicate we need to bump the sequence number */
		newseq++;
	    } else {
		/* New route.  Create an LS db entry for it */
		int foundlsa = AddLSA(&db,
				      ospf.area,
				      RT_DEST(ce->new_rt),
				      MY_ID,
				      0,
				      LS_ASE);

		if (foundlsa) {
		    /* Previous one was in the process of being flushed */

		    action = "Existing";

		    as_hdr = DB_ASE(db);

		    if (DB_RETRANS(db)) {
			rem_db_retrans(db);
		    }

		    /* Remove from the free queue */
		    assert(DB_FREEME(db));
		    DEL_DBQ(db);
		    DB_FREEME(db) = FALSE;

		    /* Indicate we need to bump the sequence number */
		    newseq++;
		} else {
		    action = "Add";

		    ASE_HDR_ALLOC(as_hdr, ASE_LA_HDR_SIZE);
		    DB_ASE(db) = as_hdr;
		    as_hdr->ls_hdr.ls_seq = FirstSeq;
		}

		/* Add to the ASE list */
		ADD_DBQ(&ospf.my_ase_list, db);
		DB_WHERE(db) = ON_ASE_LIST;

		/* Init ASE fields */
		as_hdr->ls_hdr.ls_type = LS_ASE;
		as_hdr->ls_hdr.adv_rtr = MY_ID;
		as_hdr->ls_hdr.length = htons(ASE_LA_HDR_SIZE);
		as_hdr->ls_hdr.ls_id = RT_DEST(ce->new_rt);
		as_hdr->net_mask = RT_MASK(ce->new_rt);
	    }

	    if (newseq) {
		/* We need to bump the sequence number */

		if (!DB_SEQ_MAX(db) &&
		    as_hdr->ls_hdr.ls_seq == MaxSeqNum) {
		    /* Maximum sequence number - Flush from everyone's db */

		    age = MaxAge;
		    as_hdr->ls_hdr.ls_seq = MaxSeqNum;
		    DB_SEQ_MAX(db) = TRUE;
		} else {
		    /* Increment sequence number */
		
		    as_hdr->ls_hdr.ls_seq = NEXTNSEQ(as_hdr->ls_hdr.ls_seq);
		}
	    }

	    /* Fill in the fields we calculated in ospf_policy */
	    as_hdr->tos0.ExtRtTag = ce->tag;
	    as_hdr->tos0.tos_metric = ce->metric;
	    as_hdr->tos0.ForwardAddr = ce->forward.s_addr;

	    /* Recalculate checksum and update checksumsum */
	    if (as_hdr->ls_hdr.ls_chksum) {
		ospf.db_chksumsum -= as_hdr->ls_hdr.ls_chksum;
	    }
	    as_hdr->ls_hdr.ls_age = 0;
	    ospf_checksum_sum(as_hdr, ASE_LA_HDR_SIZE, ospf.db_chksumsum);
	    as_hdr->ls_hdr.ls_age = age;

	    /* Add to transmit queue */
	    DB_TIME(db) = time_sec;
	    LL_ALLOC(ll);
	    ll->lsdb = db;
	    EN_Q(txq, ll);

	    /* Log this change */
	    /* Log this new ASE */
	    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		ospf_trace_build((struct AREA *) 0, (struct AREA *) 0, DB_ADV(db), FALSE);
	    }

	    /* Store the db pointer in the TSI field */
	    OSPF_TSI_PUT(ce->new_rt->rt_head,
			 rtbit,
			 (ospf_export_entry *)0,
			 db);
	}

	/* Free the change entry */
	OSPF_EXPORT_DEQUEUE(&ospf, ce);
	task_block_free(ospf_export_block_index, (void_t) ce);
    }

    if (i != ospf.export_limit && !interval) {
	timer_set(tip, ospf.export_interval, 0);
    }

    if (txq) {
	/* Flood all the changes I've made */

	struct AREA *a;

	AREA_LIST(a) {
	    if (!BIT_TEST(a->area_flags, OSPF_AREAF_STUB)) {
		self_orig_area_flood(a, txq, LS_ASE);
	    }
	} AREA_LIST_END(a) ;

	ospf_freeq((struct Q **)&txq, OMEM_LL);
    }

    rt_close(tp, (gw_entry *) 0, ospf.export_limit - i, NULL);

    trace(TR_OSPF, 0, "ospf_ase_export: processed %d ASEs %d remaining",
	  ospf.export_limit - i,
	  ospf.export_queue_size);
}


/*
 * ospf_policy - flash/newpolicy update routine for
 */
static void
ospf_policy __PF2(tp, task *,
		  change_list, rt_list *)
{
    int changes = 0;
    rt_head *rth;
    u_int rtbit = tp->task_rtbit;

    rt_open(tp);

    RT_LIST(rth, change_list, rt_head) {
	register rt_entry *new_rt;
	register rt_entry *old_rt;
	register ospf_export_entry *ce;
	struct LSDB *db;
	adv_results result;
	int had_export_entry;

	if (socktype(rth->rth_dest) != AF_INET) {
	    continue;
	}

	/* Read the tsi field to see what the state of this route is */
	OSPF_TSI_GET(rth, rtbit, ce, db);
#ifdef	notdef
	trace(TR_ALL, 0, "ce = %x, db = %x", (u_int)ce, (u_int)db);
#endif	/* notdef */
	new_rt = rth->rth_active;

	/* If this is one which has already changed, use old_rt from there */
	if (ce) {
	    old_rt = ce->old_rt;
	    if (ce->new_rt &&
		ce->new_rt != new_rt &&
		ce->new_rt != old_rt) {
		/* Reset routing bit on previous new route */
		(void) rtbit_reset(ce->new_rt, rtbit);
	    }

	    /*
	     * Remove it from the linked list.  It'll get put back on
	     * at the end later.
	     */
	    OSPF_EXPORT_DEQUEUE(&ospf, ce);
	    had_export_entry = 1;
	} else {
	    /*
	     * See if we were announcing another route.  The only route
	     * this could be is the last_active route.  If there is a bit
	     * set on the last_active route the db pointer had better
	     * be non-zero, otherwise it had better be zero.
	     */
	    if (rth->rth_last_active &&
		rtbit_isset(rth->rth_last_active, rtbit)) {
		assert(db);
		old_rt = rth->rth_last_active;
	    } else {
		assert(db == NULL);
		old_rt = (rt_entry *) 0;
	    }
	    had_export_entry = 0;
	}

	/* See if we can announce the new route */
	if (new_rt) {
	    if (BIT_TEST(new_rt->rt_state, RTS_NOADVISE)) {
		new_rt = (rt_entry *) 0;
	    } else {
		switch (new_rt->rt_gwp->gw_proto) {
		case RTPROTO_OSPF:
		case RTPROTO_OSPF_ASE:
		    /* Our own route - ignore */
		    new_rt = (rt_entry *) 0;
		    break;

		case RTPROTO_DIRECT:
		    /* Ignore pseudo interface routes and interfaces running OSPF */
		    if (BIT_TEST(new_rt->rt_state, RTS_NOTINSTALL) ||
			IF_INTF(new_rt->rt_ifap) ||
			BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_LOOPBACK)) {
			new_rt = (rt_entry *) 0;
		    }
		    break;

		default:
#ifdef	notdef
		    if (BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_LOOPBACK)) {
			/* Route is via the loopback interface */
			new_rt = (rt_entry *) 0;
		    }
#endif	/* notdef */
		    break;
		}
	    }

	    if (new_rt) {
		/* See if policy will allow us to export this route */
		result.res_metric = ospf.export_metric;
		result.res_metric2 = ospf.export_tag;
		result.res_flag = ospf.export_type;
		if (!export(new_rt,
			    (proto_t) 0,
			    ospf.export_list,
			    (adv_entry *) 0,
			    (adv_entry *) 0,
			    &result)) {
		    new_rt = (rt_entry *) 0;
		}
	    }
	}

	/*
	 * So far, so good.  If we haven't got either a new route
	 * or an old route at this point, continue.
	 */
	if (!new_rt && !old_rt) {
	    if (ce) {
		task_block_free(ospf_export_block_index, (void_t) ce);
		OSPF_TSI_PUT(rth,
			     rtbit,
			     (ospf_export_entry *) 0,
			     (struct LSDB *) 0);
	    }
	    continue;
	}

	/*
	 * If we haven't got an export entry we'll probably need one
	 */
	if (!ce) {
	    ce = (ospf_export_entry *) task_block_alloc(ospf_export_block_index);
	    ce->old_rt = old_rt;
	}

	/*
	 * If we haven't got a new route just queue the delete.
	 * If we've got a new route, record the export data for comparison.
	 * If we've also got an old route, check whether the old route is
	 * the same.  If so we can forget about this altogether.
	 */
	if (!new_rt) {
	    OSPF_EXPORT_Q_DELETE(&ospf, ce);
	} else {
	    /*
	     * Set the bit on the new route if it isn't there already
	     */
	    if (new_rt != old_rt &&
		new_rt != ce->new_rt) {
		rtbit_set(new_rt, rtbit);
	    }

	    ce->metric = htonl((BIT_TEST(result.res_flag, OSPF_EXPORT_TYPE2) ? ASE_bit_E : 0) | result.res_metric);
	    ce->tag =
#ifdef	PROTO_ASPATHS
		BIT_TEST(result.res_metric2, PATH_OSPF_TAG_TRUSTED) ?
		    aspath_tag_ospf(inet_autonomous_system, new_rt, result.res_metric2) :
#endif	/* PROTO_ASPATHS */
			htonl(result.res_metric2);

	    /* Calculate forwarding address */
	    if (IF_INTF(new_rt->rt_ifap)) {
		/*
		 * Only set forwarding address if OSPF is running on
		 * said interface.  Should not point to a router that
		 * runs OSPF, but that is too difficult to calculate
		 */
		ce->forward.s_addr = RT_NEXTHOP(new_rt);
	    } else {
		ce->forward.s_addr = 0;
	    }

	    /*
	     * Compare what we've got now to the old route.  If they're
	     * the same, forget about this.  In any case, add this to
	     * the appropriate part of the list
	     */
	    if (old_rt) {
	        struct ASE_LA_HDR *as_hdr = DB_ASE(db);

		if (ce->forward.s_addr == as_hdr->tos0.ForwardAddr &&
		    ce->metric == as_hdr->tos0.tos_metric &&
		    ce->tag == as_hdr->tos0.ExtRtTag) {
		    if (old_rt != new_rt) {
			rtbit_reset(old_rt, rtbit);
		    }
		    task_block_free(ospf_export_block_index, (void_t) ce);
		    OSPF_TSI_PUT(rth, rtbit, (ospf_export_entry *) 0, db);
		    continue;
		}
		OSPF_EXPORT_Q_CHANGE(&ospf, ce);
	    } else {
		OSPF_EXPORT_Q_ADD(&ospf, ce);
	    }
	}

	/*
	 * Okdk.  By the time we're here we've got an exportable new
	 * route and/or an old route we're not using any more.  Queue it
	 * at the end of the list.
	 */
	ce->new_rt = new_rt;
	OSPF_TSI_PUT(rth, rtbit, ce, db);
	changes++;
    } RT_LIST_END(rth, change_list, rt_head) ;

    rt_close(tp, (gw_entry *) 0, 0, NULL);

    if (changes &&
	!BIT_TEST(task_state, TASKS_TEST) &&
	!tp->task_timer[SYS_TIMER_ASE_QUEUE]->timer_interval) {
	/* We added something to the queue and the timer is not running, start running the queue */

	ospf_ase_export(tp->task_timer[SYS_TIMER_ASE_QUEUE], (time_t) 0);
    }
}


/**/

/*
 *	Flash update when aux protocol is enabled
 */
static void
ospf_aux_flash __PF2(tp, task *,
		     change_list, rt_list *)
{
    register aux_proto *auxp = tp->task_aux;
    
    ospf_policy(tp, change_list);

    task_aux_flash(auxp, change_list);
}


/*
 *	New policy when aux protocol is enabled
 */
static void
ospf_aux_newpolicy __PF2(tp, task *,
			 change_list, rt_list *)
{
    register aux_proto *auxp = tp->task_aux;
    
    ospf_policy(tp, change_list);

    task_aux_newpolicy(auxp, change_list);
}


/*
 *	Initiate Aux connection
 */
static void
ospf_aux_register __PF2(tp, task *,
			auxp, register aux_proto *)
{
    if (auxp) {
	/* Register */

	if (IAmASBorderRtr) {
	    /* Set intercept routines */

	    tp->task_flash = ospf_aux_flash;
	    tp->task_newpolicy = ospf_aux_newpolicy;
	}
    
	task_aux_initiate(auxp, tp->task_rtbit);
    } else {
	/* Un-Register */

	if (tp->task_flash == ospf_aux_flash) {
	    /* Reset intercept routines */

	    tp->task_newpolicy = tp->task_flash = ospf_policy;
	}
    }
}


/**/


void
ospf_policy_init __PF1(tp, task *)
{
    tp->task_aux_register = ospf_aux_register;
    tp->task_aux_proto = RTPROTO_OSPF_ASE;

    if (IAmASBorderRtr) {
	tp->task_flash = ospf_policy;
	tp->task_newpolicy = ospf_policy;
	tp->task_rtbit = rtbit_alloc(tp,
				     OSPF_TSI_DATA_SIZE,
				     (void_t) tp,
				     ospf_tsi_dump);

	/* Create ASE timers */
	(void) timer_create(ospf.task,
			    SYS_TIMER_ASE_QUEUE,
			    "AseQueue",
			    0,
			    0,
			    0,
			    ospf_ase_export,
			    (void_t) 0);

	(void) timer_create(ospf.task,	
			    SYS_TIMER_ASE_LSA,
			    "LSAGenAse",
			    0,
			    LSRefreshTime,
			    OFF3,
			    tq_AseLsa,
			    (void_t) 0);
    }	

    /* Initiate protocol interaction */
    task_aux_lookup(tp);

    ospf_export_block_index = task_block_init(sizeof (ospf_export_entry));

}


void
ospf_policy_cleanup __PF1(tp, task *)
{
    IAmASBorderRtr = FALSE;

    if (tp) {

	/* Terminate Aux connection */
	task_aux_terminate(tp);
	tp->task_aux_register = 0;
	tp->task_aux_proto = (proto_t) 0;

	/* Free our bits */
	rtbit_reset_all(tp, tp->task_rtbit, (gw_entry *) 0);

	/* Delete any export queue entries */
	if (ospf.export_queue.forw != &ospf.export_queue) {
	    register ospf_export_entry *ce;

	    while ((ce = ospf.export_queue.forw) != &ospf.export_queue) {
		remque((struct qelem *) ce);
		task_block_free(ospf_export_block_index, (void_t) ce);
	    }
	}
    }
}


#ifdef	notdef
void
ospf_discard_delete __PF1(nr, struct NET_RANGE *)
{
    if (nr->rt) {
	rt_delete(nr->rt);
    }
    nr->rt = (rt_entry *) 0;
}


int
ospf_discard_add __PF1(nr, struct NET_RANGE *)
{
    rt_parms rtp;

    bzero((caddr_t) &rtp, sizeof (rtp));

    rtp.rtp_dest = sockbuild_in(0, nr->net);
    rtp.rtp_dest_mask = inet_mask_locate(nr->mask);
    rtp.rtp_n_gw = 1;
    rtp.rtp_router = inet_addr_loopback;
    rtp.rtp_gwp = ospf.gwp;
    rtp.rtp_metric = SUMLSInfinity;
    rtp.rtp_state = RTS_NOAGE|RTS_REJECT|RTS_EXTERIOR;
    rtp.rtp_preference = ospf.preference;

    nr->rt = rt_add(&rtp);

    return FLAG_NO_PROBLEM;
}
#endif	/* notdef */


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
