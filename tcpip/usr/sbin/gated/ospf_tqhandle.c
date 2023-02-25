static char sccsid[] = "@(#)11	1.1  src/tcpip/usr/sbin/gated/ospf_tqhandle.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:55";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF2
 *		__PF5
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
 * ospf_tqhandle.c,v 1.13 1993/03/22 02:40:35 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


/*
 * Call down event on virtual interfaces associated with intf
 */
static void
virtual_intf_down  __PF1(intf, struct INTF *)
{

    /* If virtual links exist that use this interface, kill it */
    if (IAmBorderRtr &&
	(intf->area->area_id) != OSPF_BACKBONE &&
	ospf.vcnt) {
	struct INTF *vintf;

	VINTF_LIST(vintf) {
	    if (vintf->state > IDOWN &&
		vintf->ifap == intf->ifap) {
		(*if_trans[INTF_DOWN][vintf->state]) (vintf);
		/* schedule a build_rtr for the backbone */
		set_rtr_sched(&ospf.backbone);
	    }
	    /* set_rtr_sched will delay building */
	    ospf.backbone.build_rtr = FALSE;
	} VINTF_LIST_END(vintf) ;
    }
}


/*
 *	Interface is down
 */
void
ospf_ifdown __PF1(intf, struct INTF *)
{
    struct AREA *area = intf->area;
    struct LSDB_LIST *txq = LLNULL;

    trace(TR_OSPF, 0, "ospf_ifdown: Interface %s (%s) DOWN",
	  intf->ifap->ifa_name,
	  intf->ifap->ifa_name);

    (*if_trans[INTF_DOWN][intf->state]) (intf);
    virtual_intf_down(intf);

    if (BIT_TEST(intf->flags, OSPF_INTFF_NBR_CHANGE)) {
	(*(if_trans[NBR_CHANGE][intf->state])) (intf);
    }

    /* Received from outside source */
    if (area->build_rtr) {
	area->spfsched |= build_rtr_lsa(area, &txq, 0);
	area->build_rtr = FALSE;
    }
    /* build net and rtr lsa if necessary */
    if (BIT_TEST(intf->flags, OSPF_INTFF_BUILDNET)) {
	area->spfsched |= build_net_lsa(intf, &txq, 0);
	BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);
    }
    if (txq != LLNULL) {	/* may be locked out */
	self_orig_area_flood(area, txq, LS_RTR);
	ospf_freeq((struct Q **)&txq, OMEM_LL);
    }
    if (area->spfsched)
	run_spf(area, 0);
}


/*
 *	Interface is up
 */
void
ospf_ifup __PF1(intf, struct INTF *)
{
    struct LSDB_LIST *txq = LLNULL;
    struct AREA *area = intf->area;

    (*(if_trans[INTF_UP][IDOWN])) (intf);

    /* Received from outside source (GATED) */
    if (area->build_rtr) {
	area->spfsched |= build_rtr_lsa(area, &txq, 0);
	area->build_rtr = FALSE;
    }
    /* build net and rtr lsa if necessary */
    if (BIT_TEST(intf->flags, OSPF_INTFF_BUILDNET)) {
	area->spfsched |= build_net_lsa(intf, &txq, 0);
	BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);
    }
    if (txq != LLNULL) {	/* may be locked out */
	self_orig_area_flood(area, txq, LS_RTR);
	ospf_freeq((struct Q **)&txq, OMEM_LL);
    }
    if (area->spfsched)
	run_spf(area, 0);
}


/*ARGSUSED*/
void
tq_hellotmr __PF2(t, OTIMER *,
		  interval, time_t)
{
    struct INTF *intf = INTF_FROM_TIMER(t);
    struct NBR *nbr;
    struct LSDB_LIST *txq = LLNULL;
    struct AREA *area = intf->area;
    int do_poll = 0;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    if (intf->state > IDOWN) {
	struct NBR *next_nbr;
 	if (intf->type == NONBROADCAST) {
 	    if ((intf->pollmod * intf->hello_timer) >= intf->poll_timer) {
 		do_poll = TRUE;
 		intf->pollmod = 1;
 	    } else {
 	    	intf->pollmod++;
 	    }
 	}
 	
	send_hello(intf, 0, FALSE);
	for (nbr = FirstNbr(intf); nbr != NBRNULL; nbr = next_nbr) {
	    next_nbr = nbr->next;
	    /* Inactivity timer */
	    if ((nbr->state >= NATTEMPT) &&
		nbr->last_hello &&
		(time_sec - nbr->last_hello >= intf->dead_timer)) {
		nbr->last_hello = 0;
		if (intf->type == VIRTUAL_LINK) {
		    (*if_trans[INTF_DOWN][intf->state]) (intf);
		    goto virtual_bail_out;
		} else
		    (*(nbr_trans[INACT_TIMER][nbr->state])) (intf, nbr);
	    }
	    /*
	     * Handle holding interval for Slave mode - nbr state is Loading
	     * or Full
	     */
	    if (nbr->mode == SLAVE_HOLD &&
		nbr->last_exch &&
		(time_sec - nbr->last_exch) > intf->dead_timer) {
		nbr->mode = SLAVE;
		if (!(nbr->dbsum == LSDB_SUM_NULL)) {
		    DB_FREE_PKT(nbr->dbsum);
		    FREE(nbr->dbsum, OMEM_DBSUM);
		    nbr->dbsum = LSDB_SUM_NULL;
		}
	    }
	    /* Poll timer */
 	    if (do_poll && nbr->state == NDOWN)
 		send_hello(intf, nbr, FALSE);
 	}
	/* Wait timer */
	if (intf->state == IWAITING &&
	    intf->wait_time &&
	    (time_sec - intf->wait_time) >= intf->dead_timer)
	    (*if_trans[WAIT_TIMER][intf->state]) (intf);

	if (BIT_TEST(intf->flags, OSPF_INTFF_NBR_CHANGE)) {
	    (*(if_trans[NBR_CHANGE][intf->state])) (intf);
	}

	/* build net and rtr lsa if necessary */
	if (BIT_TEST(intf->flags, OSPF_INTFF_BUILDNET)) {
	    area->spfsched |= build_net_lsa(intf, &txq, 0);
	    BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);
	}
      virtual_bail_out:
	if (area->build_rtr) {
	    area->spfsched |= build_rtr_lsa(area, &txq, 0);
	    area->build_rtr = FALSE;
	}
	if (txq != LLNULL) {	/* may be locked out */
	    self_orig_area_flood(area, txq, LS_RTR);
	    ospf_freeq((struct Q **)&txq, OMEM_LL);
	}
	if (area->spfsched)
	    run_spf(area, 0);
    }
}


/*
 *	LSA Lock one shot timer - value is MinLSInterval
 *	- can't generate an LSA within MinLSInterval
 */
/*ARGSUSED*/
void
tq_lsa_lock __PF2(t, OTIMER *,
		  interval, time_t)
{
    struct AREA *a = AREA_FROM_TIMER(t);
    struct INTF *intf;
    struct LSDB_LIST *txq = LLNULL;
    struct LSDB *db, *dbnext;

    TRAP_REF_UPDATE;	/* Update the trap event counter */
    /*
     * Check to-be-free list
     */
    for (db = DB_PTR(&(ospf.db_free_list))[NEXT]; db; db = dbnext) {
	dbnext = DB_PTR(db)[NEXT];
	if (DB_CAN_BE_FREED(db))
	    db_free(db);
    }

    /* MODIFIED 1/22/92 */
    /* If LSDB limit is exceeded send trap */
    if (ospf.lsdb_limit && 
	ospf.db_cnt > ospf.lsdb_limit &&
    	!ospf.lsdb_overflow) {
	ospf.lsdb_overflow++;
	ospf.lsdb_hiwater_exceeded++;
#ifdef NOTYETDEF
	ospf_lsdb_trap(ospfLSDBOverflowTrap);
#endif
    } else if (ospf.lsdb_hiwater && 
	ospf.db_cnt > ospf.lsdb_hiwater &&
    	!ospf.lsdb_hiwater_exceeded) {
	ospf.lsdb_hiwater_exceeded++;
#ifdef NOTYETDEF
	ospf_lsdb_trap(ospfLSDBHiWaterExceededTrap);
#endif
    }

    INTF_LIST(intf, a) {
	if (intf->lock_time && (time_sec - intf->lock_time >= MinLSInterval)) {
	    if (BIT_TEST(intf->flags, OSPF_INTFF_NETSCHED)) {
		reset_net_sched(intf);	/* turn off sched */
		reset_net_lock(intf);
		if (intf->state == IDr)
		    a->spfsched |= build_net_lsa(intf, &txq, TRUE);
	    } else
		reset_net_lock(intf);
	}
    } INTF_LIST_END(intf, a) ;
    if (a->lock_time && (time_sec - a->lock_time >= MinLSInterval)) {
	if ((a->lsalock & RTRSCHED)) {
	    reset_rtr_sched(a);	/* turn off sched */
	    reset_rtr_lock(a);
	    a->spfsched |= build_rtr_lsa(a, &txq, 1);
	} else
	    reset_rtr_lock(a);
    }
    if (txq != LLNULL) {	/* may not be adjacent to anyone */
	self_orig_area_flood(a, txq, LS_NET);
	ospf_freeq((struct Q **)txq, OMEM_LL);
    }
    /*
     * Check for a spf sched event having been run - if out of memory during
     * spf, spf may have scheduled
     */
    if (a->spfsched)
	run_spf(a, 0);
}


/*
 *	LSA update interval timer for rtr and net LSAs
 *	- generate for all areas
 */
/*ARGSUSED*/
void
tq_IntLsa __PF2(t, OTIMER *,
		interval, time_t)
{
    struct AREA *a;
    struct INTF *intf;
    struct LSDB_LIST *txq = LLNULL;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    AREA_LIST(a) {
	a->spfsched |= build_rtr_lsa(a, &txq, 1);

	INTF_LIST(intf, a) {
	    if (intf->state == IDr)
		a->spfsched |= build_net_lsa(intf, &txq, TRUE);
	} INTF_LIST_END(intf, a) ;

	if (txq != LLNULL) {	/* may be locked out */
	    self_orig_area_flood(a, txq, LS_RTR);
	    ospf_freeq((struct Q **)&txq, OMEM_LL);
	}
	if (a->spfsched)
	    run_spf(a, 0);
    } AREA_LIST_END(a) ;
    timer_interval(t, LSRefreshTime);
}


/*
 *	LSA update interval timer for summary LSAs
 *	- generate for all areas
 */
/*ARGSUSED*/
void
tq_SumLsa __PF2(t, OTIMER *,
		interval, time_t)
{
    int reset = LSRefreshTime;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    /* will send to all areas */

    /* build_sum check buffers */
    if (build_sum())
	reset = 40;
    /* If ran out of buffers - try again in 40 seconds */

    timer_interval(t, reset);
}


/*
 *	LSA update interval timer for ase LSAs
 *	- Walk linked list of self-originated LSAs and generate new versions
 */
/*ARGSUSED*/
void
tq_AseLsa __PF2(t, OTIMER *,
		interval, time_t)
{
    int i;
    struct LSDB *db, *last = LSDBNULL;
    time_t reset = ospf.export_interval;  /* reset value for this timer */
    u_int16 age;
#ifdef NOTYETDEF
    struct LSDB *start_flush, *end_flush;  /* For flushing when lsdb overflow */
#endif

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    /* If no externals just return */
    if (!DB_PTR(&ospf.my_ase_list)[NEXT]) {
    	    timer_interval(t, reset);
	    return;
    }

    /* note start time of this period to know when to resched */
    if (ospf.cur_ase == LSDBNULL) {
	ospf.ase_start = time_sec;
	ospf.cur_ase = DB_PTR(&ospf.my_ase_list)[NEXT];
    }

#ifdef NOTYETDEF
    start_flush = ospf.cur_ase;		/* Used if db overflow */
#endif

    /* 
     * set new seq and newly generated time 
     */
    for (i = 0, db = ospf.cur_ase;
	 db != LSDBNULL && i < ASEGenLimit;
	 db = DB_PTR(db)[NEXT], i++) {
	if (NO_GUTS(db))
	    continue;
	if ((time_sec - ADV_AGE(db)) < MinLSInterval)
	    continue;

	if (DB_RETRANS(db) != NLNULL)
	    rem_db_retrans(db);

	LS_AGE(db) = 0;
#ifdef NOTYETDEF
	/* MODIFIED 1/22/92 */
	if (ospf.db_overflow) {
	    age = MaxAge;
	} else
#endif
	if ((LS_SEQ(db) == MaxSeqNum) &&
	    (DB_SEQ_MAX(db) == FALSE) &&
	    (ospf.nbrEcnt)) {
	    /* Not noted - flush from everyone's LSDB */
	    DB_SEQ_MAX(db) = TRUE;
	    age = MaxAge;
	} else {
	    DB_SEQ_MAX(db) = FALSE;
	    LS_SEQ(db) = NEXTNSEQ(LS_SEQ(db));
	    age = 0;
	}
    	ospf.db_chksumsum -= LS_CKS(db);
	ospf_checksum_sum(DB_ASE(db), ASE_LA_HDR_SIZE, ospf.db_chksumsum);
    	ospf.orig_new_lsa++;
	DB_TIME(db) = time_sec;
	LS_AGE(db) = age;

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	    ospf_trace_build((struct AREA *) 0, (struct AREA *) 0, DB_ADV(db), FALSE);
	}
	last = db;
    }

#ifdef NOTYETDEF
    end_flush = last;
#endif
    /* 
     * send LS_ASEs from current to last
     */
    if (ospf.cur_ase != LSDBNULL && last)
	send_ase(ospf.cur_ase, last);
 
    /* 
     * If more to send, set reset time to be MinAseInterval, else
     * set it to be LSRefreshTime - elapsed time 
     */
    if ((!last) || (last && (DB_PTR(last)[NEXT] == LSDBNULL))) {
	last = LSDBNULL;
	reset = (LSRefreshTime - (time_sec - ospf.ase_start));
	if (reset <= 0)
    	    reset = ospf.export_interval;  /* reset value for this timer */
    } else {
    	last = DB_PTR(last)[NEXT];
    }

    ospf.cur_ase = last;

#ifdef NOTYETDEF
    for (db = start_flush; 
	 db != LSDBNULL;
	 db = DB_PTR(db)[NEXT]) 
    {
	if (NO_GUTS(db))
	    continue;
	if ((time_sec - ADV_AGE(db)) < MinLSInterval)
	    continue;
	DEL_DBQ(db);
	DB_WHERE(db) = ON_ASE_INFINITY;
	DB_FREEME(db) = TRUE;
	ADD_DBQ(&ospf.db_free_list, db);
	if (db == end_flush) break;
    }
#endif

    timer_interval(t, reset);
}

/*
 *	retransmit timer for this interface
 *		what to retransmit is based on state
 */
/*ARGSUSED*/
void
tq_retrans __PF2(t, OTIMER *,
		 interval, time_t)
{
    struct INTF *intf = INTF_FROM_TIMER(t);
    struct NBR *n;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    NBRS_LIST(n, intf) {
	if (n->state < NEXSTART)
	    continue;

	if (n->state == NEXSTART) {
	    /* retransmit dbsum if any on queue */
	    if (n->dbsum != LSDB_SUM_NULL)
		send_dbsum(intf, n, IS_RETRANS);
	    else
		send_dbsum(intf, n, NOT_RETRANS);
	    continue;
	}
	if ((n->state == NEXCHANGE) &&
	    (n->mode == MASTER) &&
	    (n->dbsum != LSDB_SUM_NULL))
	    send_dbsum(intf, n, IS_RETRANS);

	if (n->state >= NEXCHANGE) {
	    if (!(NO_REQ(n)))
		send_req(intf, n, IS_RETRANS);

	    if (n->rtcnt) {
		send_lsu(n->retrans, OSPF_HASH_QUEUE, n, intf, IS_RETRANS);
	    }
	}
    } NBRS_LIST_END(n, intf) ;
}


/*
 *	Ack timer for this interface
 *		Send delayed ack if there area any
 */
/*ARGSUSED*/
void
tq_ack __PF2(t, OTIMER *,
	     interval, time_t)
{
    int acks = 0;
    struct AREA *area;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    /* Send acks for all interfaces */
    AREA_LIST(area) {
	struct INTF *intf;

	INTF_LIST(intf, area) {
	    if (intf->acks.ptr[NEXT]) {
		acks += send_ack(intf, NBRNULL, &intf->acks);	/* non-direct ack */
	    }
	} INTF_LIST_END(intf, area) ;
    } AREA_LIST_END(area) ;

    /* Send acks for all virtual interfaces */
    if (IAmBorderRtr && ospf.vcnt) {
	struct INTF *intf;

	VINTF_LIST(intf) {
	    if (intf->acks.ptr[NEXT]) {
		acks += send_ack(intf, NBRNULL, &intf->acks);	/* non-direct ack */
	    }
	} VINTF_LIST_END(intf) ;
    }

    if (!acks) {
	/* No acks left, stop timer */

	timer_reset(t);
    }
}


static int
dbage  __PF5(a, struct AREA *,
	     type, int,
	     trans, struct LSDB_LIST **,
	     start, int,
	     stop, int)
{
    struct LSDB *hp, *db, *dblast;
    struct LSDB_LIST *ll;
    u_int16 age;
    int spfsched = 0;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    /*
     * Age DB from start to stop
     * - Ase can age pieces at a time to avoid major floods
     */
    for (hp = &(a->htbl[type][start]);
	 hp < &(a->htbl[type][stop]);
	 hp++) {
	for (dblast = hp, db = DB_NEXT(hp);
	     db;
	     dblast = db, db = DB_NEXT(db)) {
	    if (NO_GUTS(db)) {
		if (type == LS_ASE) {
		    /* garbage collection, remove all traces of */
		    DB_NEXT(dblast) = DB_NEXT(db);
		    FREE(db, OMEM_LSDB);
		    db = dblast;
		}
		continue;	/* was freed */
	    }
	    /* MaxSeq and no acks, generate a new one */
	    if (DB_SEQ_MAX(db) == TRUE &&
		DB_RETRANS(db) == NLNULL &&
		DB_FREEME(db) != TRUE) {
		spfsched |= beyond_max_seq(a, INTFNULL, db, trans, trans, TRUE);
	    } else if (DB_WHERE(db) != ON_ASE_LIST &&
		     ADV_AGE(db) >= MaxAge &&
		     !(DB_SEQ_MAX(db))) {
		/* Currently active route? */
		if (DB_ROUTE(db) || DB_ASB_RTR(db)) {
		    if ((LS_TYPE(db) == LS_SUM_NET) ||
			(LS_TYPE(db) == LS_SUM_ASB)) {
			if ((IAmBorderRtr && a->area_id == OSPF_BACKBONE) ||
			    (!(IAmBorderRtr)))
			    spfsched |= SCHED_BIT(type);
		    } else
			spfsched |= SCHED_BIT(type);

		    LS_AGE(db) = MaxAge;
		    if (DB_FREEME(db) != TRUE) {
			DEL_DBQ(db);
			DB_FREEME(db) = TRUE;
			ADD_DBQ(&ospf.db_free_list, db);
		    }
		    LL_ALLOC(ll);
		    ll->lsdb = db;
		    EN_Q((*trans), ll);
		} else if (DB_FREEME(db)) {	/* was set to be freed */
		    /* free if conditions are met */
		    if (DB_CAN_BE_FREED(db)) {
			db_free(db);
			/*
			 * Asbr rtr entries are pointed to by externals so
			 * they shouldn't be freed
			 */
			if ((NO_GUTS(db)) && (type == LS_ASE)) {
			    /* garbage collection, remove all traces of */
			    DB_NEXT(dblast) = DB_NEXT(db);
			    FREE(db, OMEM_LSDB);
			    db = dblast;
			}
		    }
		} else {	/* db->freeme hasn't been set yet */
		    LS_AGE(db) = MaxAge;
		    DEL_DBQ(db);
		    DB_FREEME(db) = TRUE;
		    ADD_DBQ(&ospf.db_free_list, db);
		    LL_ALLOC(ll);
		    ll->lsdb = db;
		    EN_Q((*trans), ll);
		}
		continue;
	    }
	    /* Rerun LS checksum */
	    age = LS_AGE(db);
	    LS_AGE(db) = 0;
	    if (ospf_checksum_bad(DB_RTR(db), ntohs(LS_LEN(db)))) {
		trace(TR_OSPF, 0, "Chksum failed: area %A type %d id %s adv %s",
		      sockbuild_in(0, a->area_id),
		      type,
		      LS_ID(db),
		      ADV_RTR(db));
		assert(FALSE);
	    }
	    LS_AGE(db) = age;
	}
    }
    return spfsched;
}



/*
 * tq_int_dbage
 *   	- Run on LS_RTR and LS_NET for all areas
 * 	- Recalculate DB checksum
 * 	- Check for MaxAge
 */
/*ARGSUSED*/
void
tq_int_age __PF2(t, OTIMER *,
		 interval, time_t)
{
    struct AREA *a;
    /* list of lsdbs to be sent */
    struct LSDB_LIST *trans = LLNULL;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    AREA_LIST(a) {
	a->spfsched |= dbage(a, LS_RTR, &trans, 0, HTBLSIZE);
	a->spfsched |= dbage(a, LS_NET, &trans, 0, HTBLSIZE);

	/* something with a valid route has aged out */
	/*
	 * this should only happen when a router goes brain dead, shouldn't
	 * even happen
	 */
	/* if a db entry was newly discovered to be MaxAge, flood it */
	if (trans != LLNULL) {
	    self_orig_area_flood(a, trans, LS_RTR);
	    ospf_freeq((struct Q **)&trans, OMEM_LL);
	}
	if (a->spfsched)
	    run_spf(a, 0);
    } AREA_LIST_END(a) ;
}

/*
 * tq_sum_age
 *   	- Run on LS_SUM_NET and LS_SUM_ASB for all areas
 * 	- Recalculate DB checksum
 * 	- Check for MaxAge
 */
/*ARGSUSED*/
void
tq_sum_age __PF2(t, OTIMER *,
		 interval, time_t)
{
    struct AREA *a;
    struct LSDB_LIST *trans = LLNULL;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    AREA_LIST(a) {
	a->spfsched |= dbage(a, LS_SUM_NET, &trans, 0, HTBLSIZE);
	a->spfsched |= dbage(a, LS_SUM_ASB, &trans, 0, HTBLSIZE);
	/*
	 * if a db entry was newly discovered to be MaxAge, flood it
	 */
	if (trans != LLNULL) {
	    self_orig_area_flood(a, trans, LS_SUM_NET);
	    ospf_freeq((struct Q **)&trans, OMEM_LL);
	}
	if (a->spfsched)
	    run_spf(a, 0);
    } AREA_LIST_END(a) ;
}


/*
 * tq_ase_age
 * 	- Age ase LSAs
 */
/*ARGSUSED*/
void
tq_ase_age __PF2(t, OTIMER *,
		 interval, time_t)
{
    struct AREA *area = ospf.area;
    struct LSDB_LIST *trans = LLNULL;
    int ase_age_end = ospf.ase_age_ndx + ASE_AGE_NDX_ADD;
    int ase_age_start = ospf.ase_age_ndx;

    TRAP_REF_UPDATE;	/* Update the trap event counter */

    if (ase_age_end >= HTBLSIZE) {
        ase_age_end = HTBLSIZE;
        ospf.ase_age_ndx = 0;
    } else {
        ospf.ase_age_ndx = ase_age_end;
    }

    area->spfsched |= dbage(area,
			    LS_ASE,
			    &trans,
			    ase_age_start,
			    ase_age_end);

    /*
     * if a db entry was newly discovered to be MaxAge, flood it
     */
    if (trans != LLNULL) {
	struct AREA *a;

	AREA_LIST(a) {
	    if (!BIT_TEST(a->area_flags, OSPF_AREAF_STUB)) {
                self_orig_area_flood(a, trans, LS_ASE);
	    }
	} AREA_LIST_END(a) ;

	ospf_freeq((struct Q **) &trans, OMEM_LL);
    }
    /*
     * something with a valid route has aged out
     */
    if (area->spfsched)
	run_spf(area, 0);
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
