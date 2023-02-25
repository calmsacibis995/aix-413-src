static char sccsid[] = "@(#)04	1.1  src/tcpip/usr/sbin/gated/ospf_rxlinkup.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:34";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF2
 *		__PF3
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
 * ospf_rxlinkup.c,v 1.15.2.1 1993/07/21 13:39:38 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


static int force_flood;

/*
 * Information about a specific received LSA
 */
struct DB_INFO {
    int mc;			/* TRUE if RX by multicast */
    int len; 			/* Length of new adv */
    int foundlsa;		/* If found in lsdb or not */
    struct LSDB *db; 		/* Current or to be installed db struct */
    struct AREA *area;	 	/* Associated area */
    struct INTF *intf; 		/* Interface received on */
    struct NBR *nbr;		/* Neighbor received on */
    union LSA_PTR adv; 		/* New advertisement */
    time_t diff;		/* Difference between sec and db orig rx time */
};

/* Direct ack queue */
static struct LS_HDRQ acks;

/*
 * Compare LSAs for changes
 * - schedule partial update
 * - return spfsched
 */
static int
lsacmp  __PF3(area, struct AREA *,
	      adv, union LSA_PTR,
	      db, struct LSDB *)
{
    int spfsched = 0;

    if ((ADV_AGE(db) >= MaxAge && adv.rtr->ls_hdr.ls_age < MaxAge) ||
	(ADV_AGE(db) < MaxAge && adv.rtr->ls_hdr.ls_age >= MaxAge)) {
	if (LS_TYPE(db) > LS_NET) {
	    DB_RERUN(area->htbl[LS_TYPE(db)][DB_MYHASH(db)]) = TRUE;
	    /*
	    * If virtual link transit area rerun backbone
	     */
	    if ((LS_TYPE(db) != LS_ASE) && BIT_TEST(area->area_flags, OSPF_AREAF_VIRTUAL_UP)) {
		ospf.backbone.spfsched |= SCHED_BIT(LS_NET);
	    }
	}
	return SCHED_BIT(LS_TYPE(db));
    }

    switch (adv.rtr->ls_hdr.ls_type) {
    case LS_RTR:
	if ((adv.rtr->ls_hdr.length != LS_LEN(db)) ||
	    (adv.rtr->lnk_cnt != DB_RTR(db)->lnk_cnt) ||
	    (adv.rtr->E_B != DB_RTR(db)->E_B) ||
	    (RTR_LINK_CMP(adv.rtr, DB_RTR(db),
			  (ntohs(adv.rtr->ls_hdr.length) - RTR_LA_HDR_SIZE))))
	    spfsched = SCHED_BIT(LS_RTR);
	break;

    case LS_NET:
	if ((adv.net->ls_hdr.length != LS_LEN(db)) ||
	    (NET_ATTRTR_CMP(adv.net, DB_NET(db),
			    ntohs(adv.net->ls_hdr.length) - (NET_LA_HDR_SIZE))))
	    spfsched = SCHED_BIT(LS_NET);
	break;

    case LS_SUM_NET:
	/*
	 * Just check metric
	 */
	if (adv.sum->tos0.tos_metric != (DB_SUM(db)->tos0.tos_metric)) {
	    /*
	     * If there is a virtual link schedule backbone spf
	     */
	    if (BIT_TEST(area->area_flags, OSPF_AREAF_VIRTUAL_UP))
		ospf.backbone.spfsched |= SCHED_BIT(LS_NET);

	    /*
	     * schedule netsum if not bdr rtr or bdr rtr and
	     * area is backbone
	     */
	    if ((IAmBorderRtr && area->area_id == OSPF_BACKBONE) ||
		(!(IAmBorderRtr))) {
		spfsched = SCHED_BIT(LS_SUM_NET);
		DB_RERUN(area->htbl[LS_SUM_NET][DB_MYHASH(db)]) = TRUE;
	    }
	}
	break;

    case LS_SUM_ASB:
	if (adv.sum->tos0.tos_metric != (DB_SUM(db)->tos0.tos_metric)) {
	    /*
	     * If there is a virtual link schedule backbone spf
	     */
	    if (BIT_TEST(area->area_flags, OSPF_AREAF_VIRTUAL_UP)) {
		ospf.backbone.spfsched |= SCHED_BIT(LS_NET);
	    }
	    /*
	     * schedule asbrsum if not bdr rtr or bdr rtr and
	     * area is backbone - will have to rerun sum and ase
	     */
	    if ((IAmBorderRtr && area->area_id == OSPF_BACKBONE) ||
		(!(IAmBorderRtr))) {
		spfsched = SCHED_BIT(LS_SUM_ASB);
		DB_RERUN(area->htbl[LS_SUM_ASB][DB_MYHASH(db)]) = TRUE;
	    }
	}
	break;

    case LS_ASE:
	if (ASE_TOS_CMP(&(DB_ASE(db)->tos0), &(adv.ase->tos0))) {
	    spfsched = SCHED_BIT(LS_ASE);
	    DB_RERUN(area->htbl[LS_ASE][DB_MYHASH(db)]) = TRUE;
	}
	break;
    }

    return spfsched;
}

/*
 * Remove request from nbr request list
 */
int
nbr_rem_req __PF2(nbr, struct NBR *,
		  adv, union LSA_PTR)
{
    struct LS_REQ *ls_req;

    for (ls_req = nbr->ls_req[adv.rtr->ls_hdr.ls_type];
	 ls_req != LS_REQ_NULL; ls_req = ls_req->ptr[NEXT])
	if ((adv.rtr->ls_hdr.ls_id == ls_req->ls_id) &&
	    (adv.rtr->ls_hdr.adv_rtr == ls_req->adv_rtr)) {
	    int ret = REQ_LESS_RECENT;
	    
	    if (MORE_RECENT(ls_req, &(adv.rtr->ls_hdr), 0)) {
		return REQ_MORE_RECENT;
	    }
	    nbr->reqcnt--;
	    if (SAME_INSTANCE(ls_req, &(adv.rtr->ls_hdr), 0))
		ret =  REQ_SAME_INSTANCE;
	    REM_Q((nbr->ls_req[adv.rtr->ls_hdr.ls_type]),
		  ls_req,
		  TRUE,
		  OMEM_LS_REQ);
	    return ret;
	}
    return REQ_NOT_FOUND;
}

/*
 * Handle self-originated LSA
 */
static int
rx_self_orig  __PF3(dbi, struct DB_INFO *,
		    trans, struct LSDB_LIST **,
		    asetrans, struct LSDB_LIST **)
{
    int spfsched = 0;
    int implied_ack = FALSE;
    struct LSDB_LIST *ll;
    struct INTF *intf, *db_intf = INTFNULL;
    u_int16 age = 0;
    u_int32 new_seq;
    int flushit = FALSE;


    if ((!dbi->foundlsa) ||
	MORE_RECENT(&(dbi->adv.rtr->ls_hdr),
		    &(DB_RTR(dbi->db)->ls_hdr),
		    dbi->diff)) {
	/*
	 * newer one has come in, we have an old one floating around
	 */

	/* 
	 * if there is a retrans list  free it 
	 */
	if ((dbi->foundlsa) && (DB_RETRANS(dbi->db) != NLNULL))
	    rem_db_retrans(dbi->db);

	/*
	 * build new LSA and send
	 */
	switch (dbi->adv.rtr->ls_hdr.ls_type) {
	case LS_RTR:
	    age = LS_AGE(dbi->db);
	    LS_AGE(dbi->db) = 0;
	    LS_SEQ(dbi->db) = dbi->adv.rtr->ls_hdr.ls_seq;
	    dbi->area->db_chksumsum -= LS_CKS(dbi->db);
	    ospf_checksum_sum(DB_RTR(dbi->db), dbi->len, dbi->area->db_chksumsum);
	    LS_AGE(dbi->db) = age;
	    force_flood |= PTYPE_BIT(LS_RTR);
	    dbi->area->build_rtr = TRUE;
	    break;

	case LS_NET:
	    /*
	     * Find interface associated with this net lsa
	     */
	    INTF_LIST(intf, dbi->area) {
		if (intf->type != POINT_TO_POINT
		    && ADV_NETNUM(dbi->adv.net) == INTF_NET(intf)) {
		    db_intf = intf;
		    break;
		}
	    } INTF_LIST_END(intf, dbi->area) ;
	    if ((!dbi->foundlsa) ||
		(db_intf == INTFNULL) ||
		(db_intf->state != IDr) ||
		((db_intf->state == IDr) && (!db_intf->nbrFcnt))) {
		/*
		 * nbr has an out of date net lsa around
		 */
		if (!dbi->foundlsa) {
		    ADV_ALLOC(dbi->db,
			      LS_NET,
			      dbi->len);
		    if (!DB_NET(dbi->db)) {
	    		ospf.db_cnt--;
			dbi->area->db_cnts[LS_NET]--;
			dbi->area->db_int_cnt--;
			db_free(dbi->db);
			return FLAG_NO_BUFS;
		    }
		    ADV_COPY(dbi->adv.net, DB_NET(dbi->db), (size_t) dbi->len);
		} else {
		    dbi->area->db_chksumsum -= LS_CKS(dbi->db);
		}
		/*
		 * MaxAge so everyone deletes it
		 */
		age = MaxAge;
		flushit = TRUE;
		LL_ALLOC(ll);
		ll->lsdb = dbi->db;
		EN_Q((*trans), ll);
	    } else {
		/*
		 * have one in lsdb and this rtr is Dr
		 */
		dbi->area->db_chksumsum -= LS_CKS(dbi->db);
		LS_SEQ(dbi->db) = dbi->adv.net->ls_hdr.ls_seq;
		age = LS_AGE(dbi->db);
		if (db_intf)
		    BIT_SET(db_intf->flags, OSPF_INTFF_BUILDNET);
	    }
	    DB_TIME(dbi->db) = time_sec;
	    LS_AGE(dbi->db) = 0;
	    ospf_checksum_sum(DB_NET(dbi->db), dbi->len, dbi->area->db_chksumsum);
	    LS_AGE(dbi->db) = age;
	    force_flood |= SCHED_BIT(LS_NET);
	    break;

	case LS_SUM_ASB:
	case LS_SUM_NET:
	    LL_ALLOC(ll);
	    /*
	     * An old sum hangin' around
	     */
	    if (!dbi->foundlsa) {
		ADV_ALLOC(dbi->db, dbi->adv.rtr->ls_hdr.ls_type, dbi->len);
		if (!DB_SUM(dbi->db)) {
	    	    ospf.db_cnt--;
		    dbi->area->db_cnts[dbi->adv.rtr->ls_hdr.ls_type]--;
		    dbi->area->db_int_cnt--;
		    db_free(dbi->db);
		    return FLAG_NO_BUFS;
		}
		ADV_COPY(dbi->adv.sum, DB_SUM(dbi->db), dbi->len);
		DB_SUM(dbi->db)->tos0.tos_metric = htonl(SUMLSInfinity);
	    	/* MODIFIED 5/1/92 */
		DB_WHERE(dbi->db) = ON_SUM_INFINITY;
		flushit = TRUE;
		age = MaxAge;	/* let everyone know to free it */
	    	/* MODIFIED 5/1/92 */
		new_seq = NEXTNSEQ(dbi->adv.sum->ls_hdr.ls_seq);
	    } else if ((!(DB_SEQ_MAX(dbi->db))) && 
		       LS_SEQ(dbi->db) == MaxSeqNum) {
		/*
		 * Flush from everyone's db
		 */
		age = MaxAge;
		new_seq = MaxSeqNum;
		DB_SEQ_MAX(dbi->db) = TRUE;
		dbi->area->db_chksumsum -= LS_CKS(dbi->db);
	    } else {
		if (DB_WHERE(dbi->db) == ON_SUM_INFINITY) {
		    flushit = TRUE;
		    age = MaxAge;
		} else {
		    age = 0;
		}
		new_seq = NEXTNSEQ(dbi->adv.sum->ls_hdr.ls_seq);
		dbi->area->db_chksumsum -= LS_CKS(dbi->db);
	    }

	    LS_AGE(dbi->db) = 0;
	    LS_SEQ(dbi->db) = new_seq;
	    DB_TIME(dbi->db) = time_sec;
	    /*
	     * add new chksum
	     */
	    ospf_checksum_sum(DB_SUM(dbi->db), dbi->len, dbi->area->db_chksumsum);
	    LS_AGE(dbi->db) = age;
	    ll->lsdb = dbi->db;
	    EN_Q((*trans), ll);
	    break;

	case LS_ASE:
	    LL_ALLOC(ll);
	    /*
	     * An old ase hangin' around
	     */
	    if (!dbi->foundlsa) {
		/*
		 * Save ASE type bit
		 */
		u_int32 bitE = htonl(ASE_bit_E);

		ADV_ALLOC(dbi->db, LS_ASE, dbi->len);
		if (!DB_ASE(dbi->db)) {
	    	    ospf.db_cnt--;
		    ospf.db_ase_cnt--;
		    db_free(dbi->db);
		    return FLAG_NO_BUFS;
		}
		ADV_COPY(dbi->adv.ase, DB_ASE(dbi->db), dbi->len);
		age = MaxAge;
	    	DB_WHERE(dbi->db) = ON_ASE_INFINITY;
		flushit = TRUE;
	    	/* MODIFIED 5/1/92 */
		if (!(bitE & DB_ASE(dbi->db)->tos0.tos_metric))
		    bitE = 0;
		DB_ASE(dbi->db)->tos0.tos_metric = htonl(ASELSInfinity);
		DB_ASE(dbi->db)->tos0.tos_metric |= htonl(bitE);
		new_seq = NEXTNSEQ(dbi->adv.ase->ls_hdr.ls_seq);
	    } else if ((!(DB_SEQ_MAX(dbi->db))) && 
		       LS_SEQ(dbi->db) == MaxSeqNum) {
		/*
		 * Flush from everyone's db
		 */
		age = MaxAge;
		new_seq = MaxSeqNum;
		DB_SEQ_MAX(dbi->db) = TRUE;
		ospf.db_chksumsum -= LS_CKS(dbi->db);
	    } else {
		if (DB_WHERE(dbi->db) != ON_ASE_LIST) {
		    flushit = TRUE;
		    age = MaxAge;
		} else {
		    age = 0;
		}
		new_seq = NEXTNSEQ(dbi->adv.sum->ls_hdr.ls_seq);
		ospf.db_chksumsum -= LS_CKS(dbi->db);
	    }
	    LS_SEQ(dbi->db) = new_seq;
	    DB_TIME(dbi->db) = time_sec;
	    LS_AGE(dbi->db) = 0;
	    /*
	     * add new chksum
	     */
	    ospf_checksum_sum(DB_ASE(dbi->db), dbi->len, ospf.db_chksumsum);
	    LS_AGE(dbi->db) = age;
	    ll->lsdb = dbi->db;
	    EN_Q((*asetrans), ll);
	    break;
	}

	/*
	 * If it is to be flushed, put on free list
	 */
	if (flushit) {
	    DEL_DBQ(dbi->db);
	    DB_FREEME(dbi->db) = TRUE;
	    ADD_DBQ(&ospf.db_free_list, dbi->db);
	}
    } else if (SAME_INSTANCE(&(dbi->adv.rtr->ls_hdr),
		&(DB_RTR(dbi->db)->ls_hdr),
		dbi->diff))
    {
	if (dbi->nbr->state < NFULL) {
	    /*
	     * Out of sync with nbr's state machine
	     */
	    if (nbr_rem_req(dbi->nbr, dbi->adv)) {
		(*(nbr_trans[BAD_LS_REQ][dbi->nbr->state]))(dbi->intf,dbi->nbr);
		return FLAG_BAD_REQ;
	    }
	}
	/*
         * remove from retransmit lists
         */
	rem_nbr_ptr(dbi->db, dbi->nbr);
	implied_ack |= rem_db_ptr(dbi->nbr, dbi->db);
	/*
         * in this case, implied ack means that the lsa wasn't
         * on the retrans list so send direct ack
         */
	if (!implied_ack) {
	    ADD_ACK(&acks, dbi->db);
	}

	/*
         * If seqnum is MaxSeq and this is last ack, generate a new one
         */
	if ((DB_SEQ_MAX(dbi->db) == TRUE) &&
	    (DB_RETRANS(dbi->db) == NLNULL) &&
	    (DB_FREEME(dbi->db) != TRUE))
	    beyond_max_seq(dbi->area,dbi->intf,dbi->db,trans,asetrans,TRUE);
    } else {
	/*
	 * Less recent
	*/
	if (dbi->nbr->state < NFULL) {
	    /*
	     * Out of sync with nbr's state machine
	     */
	    if (nbr_rem_req(dbi->nbr, dbi->adv)) {
		(*(nbr_trans[BAD_LS_REQ][dbi->nbr->state]))(dbi->intf,dbi->nbr);
		return FLAG_BAD_REQ;
	    }
	}
	ospf_log_rx(UNUSUAL_INSTANCE,
		    dbi->intf->ifap->ifa_addr_local,
		    dbi->nbr->nbr_addr);
    }
    return spfsched;
}


/*
 * Handle non self-originated LSA
 */
static int
not_my_lsa  __PF3(dbi, struct DB_INFO *,
		  trans, struct LSDB_LIST **,
		  asetrans, struct LSDB_LIST **)
{
    int spfsched = 0;
    int implied_ack = FALSE;
    struct LSDB_LIST *ll;

    /*
     * if found and not the same or if not found
     */
    if (!dbi->foundlsa ||
	MORE_RECENT(&(dbi->adv.rtr->ls_hdr),
		    &(DB_RTR(dbi->db)->ls_hdr),
		    dbi->diff)) {
	if (dbi->foundlsa && dbi->diff < MinLSInterval) {
	    return FLAG_NO_PROBLEM;		/* The Tsuchiya fix */
	}

	/*
	 * Keep track of received new instances for MIBness
	 */
	ospf.rx_new_lsa++;

	/*
         * install in lsdb - may have to recalculate routing table
         */
	if (!dbi->foundlsa) {
	    /*
	     * allocate a new one and assign the lsa info to it
	     */
	    ADV_ALLOC(dbi->db,
		      dbi->adv.rtr->ls_hdr.ls_type,
		      dbi->len);

	    if (!DB_RTR(dbi->db)) {
		/* No memory - undo a couple of things... */
		ospf.rx_new_lsa--;
	    	ospf.db_cnt--;
		if (dbi->adv.rtr->ls_hdr.ls_type == LS_ASE)
		    ospf.db_ase_cnt--;
		else {
		    dbi->area->db_cnts[dbi->adv.rtr->ls_hdr.ls_type]--;
    		    dbi->area->db_int_cnt--;
		}
		db_free(dbi->db);
		return FLAG_NO_BUFS;
	    }

	    ADV_COPY(dbi->adv.rtr, DB_NET(dbi->db), dbi->len);
	    switch (LS_TYPE(dbi->db)) {
	    case LS_RTR:
	    case LS_NET:
		/*
		 * will rerun intra()
		 */
		spfsched |= (SCHED_BIT(LS_TYPE(dbi->db)));
		break;

	    case LS_SUM_NET:
		/* 
		 * add net 
		 */
		if ((IAmBorderRtr && dbi->area->area_id == OSPF_BACKBONE) ||
		    (!(IAmBorderRtr))) {
		    spfsched = SCHED_BIT(LS_SUM_NET);
		    DB_RERUN(dbi->area->htbl[LS_SUM_NET][DB_MYHASH(dbi->db)]) = TRUE;
		}
		if (BIT_TEST(dbi->area->area_flags, OSPF_AREAF_VIRTUAL_UP))
		    ospf.backbone.spfsched |= SCHED_BIT(LS_NET);
		break;

	    case LS_SUM_ASB:
		/* add as border rtr and schedule ase -
		   ls_ase may have become reachable */
		if ((IAmBorderRtr && dbi->area->area_id == OSPF_BACKBONE) ||
		    (!(IAmBorderRtr))) {
		    spfsched = SCHED_BIT(LS_SUM_ASB);
		    DB_RERUN(dbi->area->htbl[LS_SUM_ASB][DB_MYHASH(dbi->db)]) = TRUE;
		}
		if (BIT_TEST(dbi->area->area_flags, OSPF_AREAF_VIRTUAL_UP))
		    ospf.backbone.spfsched |= SCHED_BIT(LS_RTR);
		break;

	    case LS_ASE:
		/* new ase has come in */
		spfsched = SCHED_BIT(LS_ASE);
		DB_RERUN(dbi->area->htbl[LS_ASE][DB_MYHASH(dbi->db)]) = TRUE;
		break;
	    }
	} else {
	    u_int16 old_age;
	    u_int32  old_cks;
    	    struct RTR_LA_HDR *r;

	    old_age = ADV_AGE(dbi->db);
	    old_cks = LS_CKS(dbi->db);

	    /*
	     * found lsa
	     */

	    /*
	     * see if they're the same - if not schedule spf
	     */
	    spfsched |= lsacmp(dbi->area, dbi->adv, dbi->db);

	    /*
	     * save an alloc
     	     */
	    if ((dbi->len) > ntohs(LS_LEN(dbi->db))) {
    		RTR_HDR_ALLOC(r, dbi->len);
		if (!r)
		    return FLAG_NO_BUFS;
		DBADV_FREE((dbi->db),LS_TYPE(dbi->db));
		DB_RTR(dbi->db) = r;
	    }
	    ADV_COPY(dbi->adv.rtr, DB_RTR(dbi->db), dbi->len);


	    /*
	     * if there is a retrans list free it
	     */
	    if (DB_RETRANS(dbi->db) != NLNULL)
		rem_db_retrans(dbi->db);

	    /*
	     * if new one is no longer maxage, remove from free list
	     */
	    if ((old_age >= MaxAge) &&
		(LS_AGE(dbi->db) < MaxAge)) {
		if (DB_FREEME(dbi->db)) {
		    DEL_DBQ(dbi->db);
		    DB_FREEME(dbi->db) = FALSE;
		}
	    } else if ((LS_AGE(dbi->db) >= MaxAge) &&
		       (DB_FREEME(dbi->db) == FALSE)) {
		/*
		 * set it up so when retrans list is empty, free
		 */

		DEL_DBQ(dbi->db);
		DB_FREEME(dbi->db) = TRUE;
		ADD_DBQ(&ospf.db_free_list, dbi->db);
	    }

	    /*
	     * Subtract chksum for MIBness
	     */
	    if (LS_TYPE(dbi->db) < LS_ASE)
	    	dbi->area->db_chksumsum -= old_cks;
	    else 
	    	ospf.db_chksumsum -= old_cks;

	}

	LL_ALLOC(ll);
	ll->lsdb = dbi->db;
	/*
         * put on appropriate list for flooding and update new checksum sum
         */
	if (LS_TYPE(dbi->db) < LS_ASE) {
	    EN_Q((*trans), ll)
	    /*
	     * Update area chksumsum for MIB
	     */
	    dbi->area->db_chksumsum += LS_CKS(dbi->db);
	} else {
	    EN_Q((*asetrans), ll);
	    ospf.db_chksumsum += LS_CKS(dbi->db);
	}

	/*
         * note when it came it
         */
	DB_TIME(dbi->db) = time_sec;

	/*
         * will add non-direct acks in flood()
         */
    } else if (SAME_INSTANCE(&(dbi->adv.rtr->ls_hdr),
			     &(DB_RTR(dbi->db)->ls_hdr),
			     dbi->diff)) {
	if (dbi->nbr->state < NFULL) {	/* remove from req list */
	    /* Out of sync with nbr's state machine */
	    if (nbr_rem_req(dbi->nbr, dbi->adv)) {
		(*(nbr_trans[BAD_LS_REQ][dbi->nbr->state]))(dbi->intf,dbi->nbr);
		return FLAG_BAD_REQ;
	    }
	}
	/* count same instance as ack, remove from nbrs list */
	/* remove from db */
	rem_nbr_ptr(dbi->db, dbi->nbr);
	/* remove from nbr */
	implied_ack = rem_db_ptr(dbi->nbr, dbi->db);
	if (implied_ack &&
	    (dbi->intf->state == IBACKUP &&
	     NBR_ADDR(dbi->nbr) == NBR_ADDR(dbi->intf->dr))) {
	    ADD_ACK_INTF(dbi->intf, dbi->db);
	} else if (implied_ack &&
		 DB_CAN_BE_FREED(dbi->db) &&
		 ADV_AGE(dbi->db) >= MaxAge) {
	    /*
	     * if MaxAge and not retrans queue, remove from lsdb
	     */
	    db_free(dbi->db);
	} else if (!implied_ack) {
	    /* send direct ack */
	    ADD_ACK(&acks, dbi->db);
	}
    } else {				/* Less recent LSA */
	if (dbi->nbr->state < NFULL) {	/* remove from req list */
	    /* Out of sync with nbr's state machine */
	    if (nbr_rem_req(dbi->nbr, dbi->adv)) {
		(*(nbr_trans[BAD_LS_REQ][dbi->nbr->state]))(dbi->intf,dbi->nbr);
		return FLAG_BAD_REQ;
	    }
	}
	ospf_log_rx(UNUSUAL_INSTANCE,
		    dbi->intf->ifap->ifa_addr_local,
		    dbi->nbr->nbr_addr);
    }
    return spfsched;
}


int
ospf_rx_lsupdate  __PF5(lsup, struct LS_UPDATE_HDR *,
			intf, struct INTF *,
			src, sockaddr_un *,
			router_id, sockaddr_un *,
			olen, size_t)
{
    struct DB_INFO dbi;
    int i;			/* length of adv */
    int spfsched = 0;
    int reqcnt;			/* current number of requests on nbr->ls_req */
    struct NBR *n = NBRNULL;

    /* list of lsdbs to be sent */
    struct LSDB_LIST *trans = LLNULL, *asetrans = LLNULL;
    struct LSDB_LIST *src_trans = LLNULL, *src_asetrans = LLNULL;

    struct AREA *a;
    struct INTF *intf1;
    u_int16 newage;

    dbi.intf = intf;
    dbi.foundlsa = TRUE;
    dbi.area = intf->area; /* area received from */
    dbi.nbr = NBRNULL;

    /* Locate nbr */
    if (intf->type > NONBROADCAST) {
	n = FirstNbr(intf);
	if (n->nbr_id
	    && sockaddrcmp_in(n->nbr_id, router_id)) {
	    dbi.nbr = n;
	}
    } else {
	NBRS_LIST(n, intf) {
	    if (sockaddrcmp_in(src, n->nbr_addr)) {
		dbi.nbr = n;
		break;
	    }
	} NBRS_LIST_END(n, intf) ;
    }

    if (dbi.nbr == NBRNULL)
	return CANT_FIND_NBR2;
    if (dbi.nbr->state < NEXCHANGE)
	return LOW_NBR_STATE2;

    /* log count of ls_reqests */
    reqcnt = dbi.nbr->reqcnt;
    force_flood = 0;		/* use if self_orig and build rtr or net */

    GNTOHL(lsup->adv_cnt);

    for (i = 0, dbi.adv.rtr = (struct RTR_LA_HDR *) & (lsup->adv.rtr);
	 i < lsup->adv_cnt && olen;
	 i++,
	 olen -= dbi.len,
	 dbi.adv.rtr = (struct RTR_LA_HDR *) ((long) dbi.adv.rtr + dbi.len)) {
	dbi.len = ntohs(dbi.adv.rtr->ls_hdr.length);
	newage = dbi.adv.rtr->ls_hdr.ls_age;
	dbi.adv.rtr->ls_hdr.ls_age = 0;

	if (ospf_checksum_bad(dbi.adv.rtr, dbi.len)) {
	    ospf_log_rx(BAD_LS_CHKSUM,
			intf->ifap->ifa_addr_local,
			dbi.nbr->nbr_addr);
	    continue;
	}

	switch (dbi.adv.rtr->ls_hdr.ls_type) {
	case LS_RTR:
	case LS_NET:
	case LS_SUM_NET:
	case LS_SUM_ASB:
	case LS_ASE:
	    /* OK */
	    break;

	case LS_GM:
	case LS_NSSA:
	    /* Ignore */
	    continue;

	default:
	    ospf_log_rx(BAD_LSA_TYPE,
			intf->ifap->ifa_addr_local,
			dbi.nbr->nbr_addr);
	    continue;
	}

	/* If stub area and type is ASE continue */
	if (BIT_TEST(dbi.area->area_flags, OSPF_AREAF_STUB) && 
	    dbi.adv.rtr->ls_hdr.ls_type == LS_ASE)
	    continue;

	/* put back age in host order */
	dbi.adv.rtr->ls_hdr.ls_age = ntohs(newage);

	if (!(XAddLSA(&dbi.db, dbi.area, dbi.adv.rtr, 0))) {
	    dbi.foundlsa = FALSE;
	    dbi.diff = 0;
	    /* 
	     * not found, if MaxAge remove from req lists and drop 
	     */
	    if (dbi.adv.rtr->ls_hdr.ls_age >= MaxAge) {
		if (!dbi.db)
	    	    goto nobufs;

		ADV_ALLOC(dbi.db,
			  dbi.adv.rtr->ls_hdr.ls_type,
			  dbi.len);

		/* 
		 * No memory - undo a couple of things... 
		 */
		if (!DB_NET(dbi.db)) {
	    	    ospf.db_cnt--;
		    if (dbi.adv.rtr->ls_hdr.ls_type == LS_ASE)
			ospf.db_ase_cnt--;
		    else {
			dbi.area->db_cnts[dbi.adv.rtr->ls_hdr.ls_type]--;
    			dbi.area->db_int_cnt--;
		    }
		    db_free(dbi.db);
		    goto nobufs;
		}

		ADV_COPY(dbi.adv.net, DB_NET(dbi.db), dbi.len);
		ADD_ACK(&acks, dbi.db);

		/*
		 * put on appropriate list for flooding and update new checksum sum
		 */
		if (LS_TYPE(dbi.db) < LS_ASE) {
		    /*
		     * Update area chksumsum for MIB
		     */
		    dbi.area->db_chksumsum += LS_CKS(dbi.db);
		} else {
		    ospf.db_chksumsum += LS_CKS(dbi.db);
		}

		if (dbi.nbr->state < NFULL) {	/* remove from req list */

		    /* Add to the free list */
		    DB_FREEME(dbi.db) = TRUE;
		    ADD_DBQ(&ospf.db_free_list, dbi.db);

		    nbr_rem_req(dbi.nbr, dbi.adv);
		    if (NO_REQ(dbi.nbr))
		  	(*(nbr_trans[LOAD_DONE][dbi.nbr->state]))(intf,dbi.nbr);
		} else {
		    db_free(dbi.db);
		}
		continue;
	    }
	} else {
	    dbi.foundlsa = TRUE;
	    dbi.diff = time_sec - DB_TIME(dbi.db);	/* For the NEW AGE */
	}
	/* check for self originated lsa */
	if (dbi.adv.rtr->ls_hdr.adv_rtr == MY_ID) {
	    spfsched |= rx_self_orig(&dbi, &src_trans, &src_asetrans);
	} else {
	    spfsched |= not_my_lsa(&dbi, &trans, &asetrans);
	}

	if (spfsched & (FLAG_NO_BUFS | FLAG_BAD_REQ)) {
	    break;
	}
    }

  nobufs:				/* have run out of buffers... */

    /*
     * Flood new self originated ls_ase
     */
    if (src_asetrans != LLNULL) {
	AREA_LIST(a) {
	    if (!BIT_TEST(a->area_flags, OSPF_AREAF_STUB)) {
		self_orig_area_flood(a, src_asetrans, LS_ASE);
	    }
	} AREA_LIST_END(a) ;

	ospf_freeq((struct Q **) &src_asetrans, OMEM_LL);
    }
    /*
     * Flood local to this area
     */
    if (trans != LLNULL) {
	area_flood(dbi.area,
		   trans,
		   intf,
		   dbi.nbr,
		   LS_RTR);
	ospf_freeq((struct Q **) &trans, OMEM_LL);
    }
    /*
     * Flood other than this RTR's LS_ASEs
     */
    if (asetrans != LLNULL) {
	AREA_LIST(a) {
	    if (!BIT_TEST(a->area_flags, OSPF_AREAF_STUB)) {
		area_flood(a,
			   asetrans,
			   intf,
			   dbi.nbr,
			   LS_ASE);
	    }
	} AREA_LIST_END(a) ;

	ospf_freeq((struct Q **) &asetrans, OMEM_LL);
    }
    /*
     * load done event has occurred, build_rtr LSA and net LSA
     */
    if (dbi.area->build_rtr) {
	dbi.area->build_rtr = FALSE;
	spfsched |= build_rtr_lsa(dbi.area,
				  &src_trans,
				  (PTYPE_BIT(LS_RTR) & force_flood));
    }
    /*
     * Build net LSAs for intfs scheduled to do so while parsing this pkt
     */
    INTF_LIST(intf1, dbi.area) {
	if (BIT_TEST(intf1->flags, OSPF_INTFF_BUILDNET))
	    spfsched |= build_net_lsa(intf1,
				      &src_trans,
				      (PTYPE_BIT(LS_NET) & force_flood));
    } INTF_LIST_END(intf1, dbi.area) ;

    /*
     * flood new self-originated LSAs
     */
    if (src_trans != LLNULL) {
	self_orig_area_flood(dbi.area, src_trans, LS_NET);
	ospf_freeq((struct Q **) &src_trans, OMEM_LL);
    }

    /* Send direct ack */
    if (acks.ptr[NEXT]) {
	(void) send_ack(intf, dbi.nbr, &acks);
    }

    /* Send an ack packet if we have enough for a full one */
    if (!ACK_QUEUE_FULL(intf) || send_ack(intf, NBRNULL, &intf->acks)) {
	/* There is less than one full packet left, make sure the Ack timer is running */

	timer_interval(TASK_TIMER(ospf.task, SYS_TIMER_ACK), OSPF_T_ACK);
    }
    
    /*
     * Topology change in area or load done has occurred
     */
    dbi.area->spfsched |= (spfsched & ALLSCHED);

    /*
     * if any requests were acked, send new request pkt
     */
    if (dbi.nbr->state > NEXSTART && dbi.nbr->reqcnt < reqcnt)
	send_req(intf, dbi.nbr, 0);

#ifdef	notdef
    struct LSDB *dbnext = LSDBNULL;
    /*
     *  Check to-be-free list
     */
    for (dbi.db = DB_PTR(&(ospf.db_free_list))[NEXT]; dbi.db; dbi.db = dbnext) {
	dbnext = DB_PTR(dbi.db)[NEXT];
	if (DB_CAN_BE_FREED(dbi.db))
	    db_free(dbi.db);
    }
#endif	/* notdef */

    return GOOD_RX;
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
