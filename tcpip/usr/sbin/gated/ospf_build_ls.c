static char sccsid[] = "@(#)85	1.1  src/tcpip/usr/sbin/gated/ospf_build_ls.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:46";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF3
 *		__PF4
 *		__PF5
 *		__PF6
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
 * ospf_build_ls.c,v 1.16.2.2 1993/08/27 22:43:37 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


/*
 * add db to list to be flooded by calling routine
 */
static void
txadd __PF5(db, struct LSDB *,
	    txq, struct LSDB_LIST **,
	    len, size_t,
	    age, u_int16,
	    area, struct AREA *)
{
    struct LSDB_LIST *ll = LLNULL;

    /*
     * if there is a retrans list, free it
     */
    if (DB_RETRANS(db) != NLNULL)
	rem_db_retrans(db);
    LS_AGE(db) = 0;
    LL_ALLOC(ll);
    if (ll == LLNULL)
	return;
    ll->lsdb = db;
    ll->flood = FLOOD;
    ospf_checksum_sum(DB_RTR(db), len, area->db_chksumsum);
    LS_AGE(db) = age;
    /* 
     * put lsa on queue to be flooded 
     */
    EN_Q((*txq), ll);

    /* 
     * Increase count of self originated LSAs for MIBness 
     */
    ospf.orig_new_lsa++;
}


/*
 * build ls updates - three cases where these are called
 *		   1) tq has fired
 *		   2) nbr change
 *		   3) received self orig with earlier seq number
 */
int
build_rtr_lsa __PF3(a, struct AREA *,
		    txq, struct LSDB_LIST **,
		    force_flood, int)	/* if true always flood */
{
    int foundlsa;			/* true if lsa was found when adding */
    int spfsched = 0;
    int addnet = 0;
    struct RTR_LA_HDR *r = (struct RTR_LA_HDR*) 0;
    struct RTR_LA_PIECES *linkp;
    struct OSPF_HOSTS *host;
    struct INTF *intf;
    struct LSDB *db;
    size_t len;
    u_int32 new_seq;

    if (RTR_LSA_LOCK(a)) {
	/* schedule one to send */
	set_rtr_sched(a);
	return FLAG_NO_PROBLEM;
    }

    foundlsa = AddLSA(&db, a, MY_ID, MY_ID, 0, LS_RTR);


    /* Check seq number for max */
    if (foundlsa) {
	if ((LS_SEQ(db) == MaxSeqNum)) {
	    if (DB_SEQ_MAX(db) == TRUE && DB_RETRANS(db) != NLNULL) {
		/*
		 * Will reoriginate when the acks come home
		 */
		return FLAG_NO_PROBLEM;
		/* If no nbrs >= exchange there will be no acks */
	    } else if (DB_SEQ_MAX(db) == FALSE && a->nbrEcnt) {
		/*
		 * Not already noted - flush from everyone's LSDB
		 */
		DB_SEQ_MAX(db) = TRUE;
		txadd(db, txq, (size_t) LS_LEN(db), MaxAge, a);
		rtr_lsa_lockout(a);
		DB_TIME(db) = time_sec;
		return FLAG_NO_PROBLEM;
	    }
	    /* have receive acks */
	}
	DB_SEQ_MAX(db) = FALSE;
	new_seq = NEXTNSEQ(LS_SEQ(db));
    } else {
        /*
         * Enough mem for DB?
         */
        if (db == LSDBNULL) {
	    set_rtr_sched(a);
	    return FLAG_NO_PROBLEM;
        }
	new_seq = FirstSeq;
	spfsched = RTRSCHED;
    }

    /*
     * Allocate enough for all links
     */
    len = MY_RTR_ADV_SIZE(a);
    RTR_HDR_ALLOC(r, len);
    if (!r) {
	set_rtr_sched(a);
	return FLAG_NO_PROBLEM;
    }

    r->ls_hdr.ls_type = LS_RTR;
    r->ls_hdr.ls_id = MY_ID;
    r->ls_hdr.adv_rtr = MY_ID;
    if (IAmBorderRtr)
	r->E_B = bit_B;
    if (IAmASBorderRtr)
	r->E_B |= bit_E;
    GHTONS(r->E_B);
    len = RTR_LA_HDR_SIZE;
    linkp = &(r->link);

    INTF_LIST(intf, a) {
	if (intf->state == IDOWN)
	    continue;
	addnet = 0;

	switch (intf->state) {
	case IPOINT_TO_POINT:
	    if (intf->nbr.state == NFULL) {
		r->lnk_cnt++;
		/* len += RTR_LA_METRIC_SIZE - add metric lengths */
		len += RTR_LA_PIECES_SIZE;
		/* add for TOS here */
		linkp->con_type = RTR_IF_TYPE_RTR;
		linkp->lnk_id = NBR_ID(&intf->nbr);
		linkp->lnk_data = INTF_LCLADDR(intf);
		linkp->metric_cnt = 0;
		linkp->tos0_metric = htons(intf->cost);
		/* add metric lengths here if necessary */
		linkp = (struct RTR_LA_PIECES *) ((long) linkp +
						 RTR_LA_PIECES_SIZE +
						 ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));

		/* Add a stub link to the remote address */
		r->lnk_cnt++;
		len += RTR_LA_PIECES_SIZE;
		/* add for TOS here */
		linkp->con_type = RTR_IF_TYPE_HOST;
		linkp->lnk_data = HOST_NET_MASK;
		linkp->lnk_id = NBR_ADDR(&intf->nbr);
		linkp->metric_cnt = 0;
		linkp->tos0_metric = htons(intf->cost);
		linkp = (struct RTR_LA_PIECES *) ((long) linkp +
						 RTR_LA_PIECES_SIZE +
						 ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));
	    }
	    break;
	    
	case ILOOPBACK:
	    /*
	     * Need ip address for this part...
	     */
	    if (intf->ifap->ifa_addr) {
		r->lnk_cnt++;
		len += RTR_LA_PIECES_SIZE;
		/* add for TOS here */
		linkp->con_type = RTR_IF_TYPE_HOST;
		linkp->lnk_data = HOST_NET_MASK;
		linkp->lnk_id = INTF_ADDR(intf);
		linkp->metric_cnt = 0;
		linkp->tos0_metric = 0;
		linkp = (struct RTR_LA_PIECES *) ((long) linkp +
						 RTR_LA_PIECES_SIZE +
						 ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));
	    }
	    break;

	case IDr:			/* have DR selected cases */
	case IBACKUP:
	case IDrOTHER:
	    /*
	     * If there are full nbrs and we are adjacent or are DR
	     * it is OK to add a network link
	     */
	    if (intf->nbrFcnt) {
		if ((intf->state == IDr) ||
		    ((intf->dr) && (intf->dr->state == NFULL)))
		    addnet++;
	    }

	    if (addnet) {
		r->lnk_cnt++;
		len += RTR_LA_PIECES_SIZE;
		/* add for TOS here */
		linkp->con_type = RTR_IF_TYPE_TRANS_NET;
		linkp->lnk_id = NBR_ADDR(intf->dr);
		linkp->lnk_data = INTF_ADDR(intf);
		linkp->metric_cnt = 0;
		linkp->tos0_metric = htons(intf->cost);
		linkp = (struct RTR_LA_PIECES *) ((long) linkp +
						 RTR_LA_PIECES_SIZE +
						 ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));
		break;
	    }
	    /*
	     * else fall through
	     */
	case IWAITING:
	    /*
	     * still determining DR
	     */
	    r->lnk_cnt++;
	    len += RTR_LA_PIECES_SIZE;
	    /* add for TOS here */
	    linkp->con_type = RTR_IF_TYPE_STUB_NET;
	    linkp->lnk_data = INTF_MASK(intf);
	    linkp->lnk_id = INTF_NET(intf);
	    linkp->metric_cnt = 0;
	    linkp->tos0_metric = htons(intf->cost);
	    linkp = (struct RTR_LA_PIECES *) ((long) linkp +
					     RTR_LA_PIECES_SIZE +
					     ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));
	    break;
	}
    } INTF_LIST_END(intf, a) ;
    
    /*
     * add virtual links
     */
    if (a->area_id == OSPF_BACKBONE && ospf.vcnt) {
	struct INTF *vintf;

	VINTF_LIST(vintf) {
	    if ((vintf->state == IPOINT_TO_POINT) &&
		(vintf->nbr.state == NFULL)) {
		r->lnk_cnt++;
		len += RTR_LA_PIECES_SIZE;
		/* add for TOS here */
		linkp->con_type = RTR_IF_TYPE_VIRTUAL;
		linkp->lnk_id = NBR_ID(&vintf->nbr);
		linkp->lnk_data = INTF_LCLADDR(vintf);
		linkp->metric_cnt = 0;
		linkp->tos0_metric = htons(vintf->cost);
		linkp = (struct RTR_LA_PIECES *) ((long) linkp +
						 RTR_LA_PIECES_SIZE +
			      ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));
	    }
	} VINTF_LIST_END(vintf) ;
    }
    /*
     * add host links
     */
    for (host = a->hosts.ptr[NEXT]; host != HOSTSNULL; host = host->ptr[NEXT]) {
	if_addr *ifap = if_withdst(sockbuild_in(0, host->if_addr));

	if (!ifap) {
	    /* No interface for this host */
	    
	    continue;
	}
	switch (ifap->ifa_state & (IFS_POINTOPOINT|IFS_LOOPBACK)) {
	    struct NH_BLOCK *np;
	    
	case IFS_POINTOPOINT:
	    if ((np = (struct NH_BLOCK *) ifap->ifa_ospf_nh)
		&& np->nh_addr == host->if_addr) {
		/* Matches on remote address */

		break;
	    }
	    /* Fall through */

	case IFS_LOOPBACK:
	default:
	    if (!(np = (struct NH_BLOCK *) ifap->ifa_ospf_nh_lcl)
		|| np->nh_addr != host->if_addr) {
		/* No next hop block or no match */

		continue;
	    }
	    break;
	}
	
	r->lnk_cnt++;
	len += RTR_LA_PIECES_SIZE;
	/* add for TOS here */
	linkp->con_type = RTR_IF_TYPE_HOST;
	linkp->lnk_data = HOST_NET_MASK;
	linkp->lnk_id = host->if_addr;
	linkp->metric_cnt = 0;
	linkp->tos0_metric = host->cost;
	linkp = (struct RTR_LA_PIECES *) ((long) linkp +
					  RTR_LA_PIECES_SIZE +
					  ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE));
    }

    r->ls_hdr.length = htons(len);

    if (foundlsa) {
	/*
	 * compare to see if it's the same, if not rerun spf
	 */
	if ((r->ls_hdr.length != LS_LEN(db)) ||
	    (r->E_B != DB_RTR(db)->E_B) ||
	    (LS_AGE(db) == MaxAge) ||
	    (r->lnk_cnt != ntohs(DB_RTR(db)->lnk_cnt)) ||
	    RTR_LINK_CMP(r, DB_RTR(db),
			 (size_t) (ntohs(r->ls_hdr.length) - RTR_LA_HDR_SIZE)))
	    spfsched = RTRSCHED;
	if (DB_FREEME(db) == TRUE) {
	    DB_FREEME(db) = FALSE;
	    DEL_DBQ(db);		/* remove from db_free_list */
	}
    }

    /*
     * if found or changed, replace with new one and flood
     */
    if (force_flood ||
	spfsched ||
	new_seq == FirstSeq ||
	ospf.nbrEcnt != ospf.nbrFcnt) {

	if (foundlsa)
	    a->db_chksumsum -= LS_CKS(db);

	if (DB_RTR(db))
	    DBADV_FREE(db, LS_RTR);
	DB_RTR(db) = r;
	LS_SEQ(db) = new_seq;

	GHTONS(r->lnk_cnt);

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	    ospf_trace_build(a, a, DB_ADV(db), FALSE);
	}

	/*
	 * lock out new instansiation for MinLSInterval
	 */
	rtr_lsa_lockout(a);
	DB_TIME(db) = time_sec;
	/*
	 * put lsa on queue to be flooded
	 */
	txadd(db, txq, len, 0, a);
    } else
	ADV_FREE(r, OMEM_RTR);

    /*
     * let calling routine know to rerun spf or not
     */
    return spfsched;
}

/*
 * build net link state advertisement - this rtr is DR
 */
int
build_net_lsa __PF3(intf, struct INTF *,
		    txq, struct LSDB_LIST **,
		    force_flood, int)	/* if true always flood */
{
    int foundlsa;			/* true if lsa was found when adding */
    int spfsched = 0;
    struct AREA *a = intf->area;
    struct NET_LA_HDR *nh;
    struct NET_LA_PIECES *att_rtr;
    struct NBR *nbr;
    struct LSDB *db;
    u_int32 new_seq;
    size_t len;

    if (intf->state != IDr)
	return FALSE;

    if (NET_LSA_LOCK(intf)) {
	/* schedule one to send */
	set_net_sched(intf);
	return FLAG_NO_PROBLEM;
    }
    /*
     * Are we adjacent to any neighbors?
     */
    if ((!intf->nbrFcnt))
	return FALSE;

    foundlsa =
	AddLSA(&db, a, INTF_ADDR(intf), MY_ID, 0, LS_NET);
    /*
     * Check seq number for max
     */
    if (foundlsa) {
	if (DB_FREEME(db)) {
	    DB_FREEME(db) = FALSE;
	    DEL_DBQ(db);
	}
	if ((LS_SEQ(db) == MaxSeqNum)) {
	    if (DB_SEQ_MAX(db) == TRUE && DB_RETRANS(db) != NLNULL) {
		/* Will re originate when the acks come home */
		return FALSE;
		/* If no nbrs >= exchange there will be no acks */
	    } else if (DB_SEQ_MAX(db) == FALSE && a->nbrEcnt) {
		/* Not already noted - flush from everyone's LSDB */
		DB_SEQ_MAX(db) = TRUE;
		txadd(db, txq, (size_t) LS_LEN(db), MaxAge, a);
		net_lsa_lockout(intf);
		DB_TIME(db) = time_sec;
		return FALSE;
	    }
	}
	/*
         * have received acks
         */
	DB_SEQ_MAX(db) = FALSE;
	new_seq = NEXTNSEQ(LS_SEQ(db));
    } else {
	if (!db) {
	    set_net_sched(intf);
	    return FLAG_NO_PROBLEM;
	}
	new_seq = FirstSeq;
	spfsched = NETSCHED;
    }


    /*
     * Allocate enough for all links
     */
    len = MY_NET_ADV_SIZE(intf);
    NET_HDR_ALLOC(nh, len);
    if (!nh) {
    	set_net_sched(intf);
	if (!foundlsa) {
	    ospf.db_cnt--;
	    a->db_cnts[LS_NET]--;
    	    a->db_int_cnt--;
	    db_free(db);
	}
	return FLAG_NO_PROBLEM;
    }
    len = NET_LA_HDR_SIZE + NET_LA_PIECES_SIZE;	/* one for this rtr */
    nh->ls_hdr.ls_type = LS_NET;
    nh->ls_hdr.ls_id = INTF_ADDR(intf);
    nh->ls_hdr.adv_rtr = MY_ID;
    nh->net_mask = INTF_MASK(intf);
    att_rtr = &(nh->att_rtr);
    /*
     * set the first one to this rtr
     */
    att_rtr->lnk_id = MY_ID;
    att_rtr++;
    for (nbr = intf->nbr.next; nbr != NBRNULL; nbr = nbr->next) {
	if (nbr->state == NFULL) {
	    att_rtr->lnk_id = NBR_ID(nbr);
	    len += NET_LA_PIECES_SIZE;
	    att_rtr++;
	}
    }
    nh->ls_hdr.length = htons(len);
    if (foundlsa) {
	/*
  	 * compare to see if it's the same, if not rerun spf
	 */
	if ((nh->ls_hdr.length != LS_LEN(db)) ||
	    (LS_AGE(db) == MaxAge) ||
	    NET_ATTRTR_CMP(nh, DB_NET(db), (len - NET_LA_HDR_SIZE)) ||
	    nh->net_mask != DB_MASK(db))
	    spfsched = NETSCHED;
	if (DB_FREEME(db) == TRUE) {
	    DB_FREEME(db) = FALSE;
	    DEL_DBQ(db);		/* remove from db_free_list */
	}
    }
    /*
     * if !found, not the same or force flood, replace with new one
     */
    if (force_flood ||
	spfsched ||
	new_seq == FirstSeq ||
	ospf.nbrEcnt != ospf.nbrFcnt) {
	if (foundlsa)
	    a->db_chksumsum -= LS_CKS(db);
	if (DB_NET(db))
	    DBADV_FREE(db, LS_NET);
	DB_NET(db) = nh;
	LS_SEQ(db) = new_seq;
	/*
	 * lock out new instansiation for MinLSInterval
	 */
	net_lsa_lockout(intf);
	DB_TIME(db) = time_sec;
	txadd(db, txq, len, 0, a);

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	    ospf_trace_build(a, a, DB_ADV(db), FALSE);
	}
    } else {
	ADV_FREE(nh, OMEM_NETHDR);
    }

    /* A done deal */
    BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);

    /* rerun spf? */
    return spfsched;
}


/*
 * Update seq number
 * - if current seq number == MaxSeqNum set to max age and add to queue
 */
static void
sum_next_seq __PF4(db, struct LSDB *,
		   a, struct AREA *,
		   metric, u_int32,
		   sec, time_t)
{
    /* MODIFIED 5/1/92 */
    u_int16 age = (metric == ntohl(SUMLSInfinity)) ? MaxAge : 0;

    DB_SUM(db)->tos0.tos_metric = metric;
    if (LS_SEQ(db) == MaxSeqNum) {
	if (DB_SEQ_MAX(db) == TRUE && DB_RETRANS(db) != NLNULL)
	    return;
	if ((DB_SEQ_MAX(db) == FALSE) && (a->nbrEcnt)) {
	    /* Not noted - flush from everyone's LSDB */
	    DB_SEQ_MAX(db) = TRUE;
	    age = MaxAge;
	} else {
	    DB_SEQ_MAX(db) = FALSE;
	    LS_SEQ(db) = NEXTNSEQ(LS_SEQ(db));
	}
    } else {
	LS_SEQ(db) = NEXTNSEQ(LS_SEQ(db));
    }
    DB_TIME(db) = sec;
    a->db_chksumsum -= LS_CKS(db);
    txadd(db, &a->txq, (size_t) SUM_LA_HDR_SIZE, age, a);

}

/*
 * Build network summary link-state advertisements
 *    called by spf - just build the ones that have changed
 *    - Loop through all area and store in dst's lsdb where dst != orig area
 *    - Put on dst area's->sumnetlst
 */
int
build_sum_net __PF1(area, struct AREA *)
{
    int foundlsa;			/* true if lsa was found when adding */
    struct AREA *dst;			/* injecting from a - into area dst  */
    struct LSDB *db;
    struct SUM_LA_HDR *s;
    struct NET_RANGE *nr;

    RANGE_LIST(nr, area) {
	/* is now unreachable */

	if (nr->cost == SUMLSInfinity) {
	    AREA_LIST(dst) {
		if (area == dst)
		    continue;
		db = FindLSA(dst, nr->net, MY_ID, LS_SUM_NET);
		if (db == LSDBNULL)
		    continue;		/* still down or unreach */
		else if (DB_WHERE(db) != ON_SUM_INFINITY) {
		    /* has become unreachable */
	    	    /* MODIFIED 5/1/92 */
		    DB_WHERE(db) = ON_SUM_INFINITY;
	    	    DEL_DBQ(db);
	    	    DB_FREEME(db) = TRUE;
	    	    ADD_DBQ(&ospf.db_free_list, db);
		    /* MODIFIED 5/1/92 */
#ifdef	notdef
		    ospf_discard_delete(nr);
#endif	/* notdef */
		    /* add to txq */
		    sum_next_seq(db, dst, htonl(SUMLSInfinity), time_sec);

		    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
			ospf_trace_build(area, dst, DB_ADV(db), FALSE);
		    }
		}
	    } AREA_LIST_END(dst) ;
	    continue;
	}

	/* 
	 * is not unreachable 
	 */
	AREA_LIST(dst) {
	    if (area == dst) {
		/* 
	 	 * Flush this route from this area if it existed 
		 */
		db = FindLSA(dst, nr->net, MY_ID, LS_SUM_NET);
		if ((db) && (DB_WHERE(db) == ON_SUMNET_LIST)) {
	    	    /* MODIFIED 5/1/92 */
		    DB_WHERE(db) = ON_SUM_INFINITY;
	    	    DEL_DBQ(db);
	    	    DB_FREEME(db) = TRUE;
	    	    ADD_DBQ(&ospf.db_free_list, db);
		    /* MODIFIED 5/1/92 */
#ifdef	notdef
		    ospf_discard_delete(nr);
#endif	/* notdef */
		    sum_next_seq(db, dst, htonl(SUMLSInfinity), time_sec);
		}
		continue;
	    }
	    foundlsa =
		AddLSA(&db, dst, nr->net, MY_ID, 0, LS_SUM_NET);
	    if (!foundlsa) {		/* net has come up */
		if (!db)
		    return FLAG_NO_BUFS;
		SUM_HDR_ALLOC(s, SUM_LA_HDR_SIZE);
		if (!s
#ifdef	notdef
		    || ospf_discard_add(nr) == FLAG_NO_BUFS
#endif	/* notdef */
		    ) {
		   ospf.db_cnt--;
    		   dst->db_cnts[LS_SUM_NET]--;
    		   dst->db_int_cnt--;
		   db_free(db);
		   return FLAG_NO_BUFS;
		}
		s->ls_hdr.ls_type = LS_SUM_NET;
		s->ls_hdr.ls_id = nr->net;
		s->ls_hdr.adv_rtr = MY_ID;
		s->ls_hdr.ls_seq = FirstSeq;
		s->ls_hdr.length = htons(SUM_LA_HDR_SIZE);
		s->net_mask = nr->mask;
		s->tos0.tos_metric = htonl(nr->cost);
		DB_SUM(db) = s;
		/* 
		 * add to this areas net sum list 
		 */
		DB_WHERE(db) = ON_SUMNET_LIST;
		ADD_DBQ(&dst->sumnetlst, db);
		DB_TIME(db) = time_sec;
		/* add to txq */
		txadd(db, &(dst->txq), SUM_LA_HDR_SIZE, 0, dst);
	    } else if (DB_WHERE(db) == ON_SUM_INFINITY) {
		/* is now reachable - let the world know */
#ifdef	notdef
		if (ospf_discard_add(nr) == FLAG_NO_BUFS)
		    return FLAG_NO_BUFS;
#endif	/* notdef */
		sum_next_seq(db, dst, htonl(nr->cost), time_sec);
		if (DB_FREEME(db) == TRUE) {
		    DB_FREEME(db) = FALSE;
		    DEL_DBQ(db);	/* remove from db_free_list */
		}
		/* 
		 * add to this area's net sum list 
		 */
		DB_WHERE(db) = ON_SUMNET_LIST;
		ADD_DBQ(&dst->sumnetlst, db);
	    } else {
		/* 
		 * had in lsdb and on netlst - if cost has changed send 
		 */
		if (DB_SUM(db)->tos0.tos_metric != htonl(nr->cost)) {
		    sum_next_seq(db, dst, htonl(nr->cost), time_sec);
		}			/* else no changes, continue */
	    }
	    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		ospf_trace_build(area, dst, DB_ADV(db), FALSE);
	    }
	} AREA_LIST_END(dst) ;

    } RANGE_LIST_END(nr, area) ;
    return FLAG_NO_PROBLEM;
}


/*
 * Build autonomous system border rtr summary link-state advertisements
 *    from area into dest
 *    - r is passed from spf()
 *    - Store all sum links in dst area's lsdb
 *    - Put on dst area's asb list for so it is easy to do periodic update
 *    - Will also add to this area but not transmit
 */
int
build_sum_asb __PF3(area, struct AREA *,
		    r, struct OSPF_ROUTE *,
		    from_area, struct AREA *)	/* area building from */
{
    int foundlsa;			/* true if lsa was found when adding */
    struct AREA *dst;			/* injecting into area */
    struct LSDB *db;
    struct SUM_LA_HDR *s;
    struct OSPF_ROUTE *rr;

    /*
     * first check known reachable ASBRs
     */
    AREA_LIST(dst) {
	/*
         * Don't use sum asbs to for flooding into backbone
         */
	if ((RRT_PTYPE(r) == LS_SUM_ASB) && (dst->area_id == OSPF_BACKBONE))
	    continue;

	rr = rtr_findroute( dst,
			    RRT_DEST(r),
			    DTYPE_ASBR,
			    PTYPE_INTRA );
	/*
         * If there is a currently valid intra-area route in dst
         * or the route passed in isn't valid, nuke the one in dst
         */
	if ( (rr) &&
	     ( ((RRT_PTYPE(r) == LS_RTR) &&
	        ( (dst != from_area) ||
	          ((dst == from_area) && (RRT_REV(rr) == RTAB_REV)) ) ) ||
	     (RRT_PTYPE(r) == LS_SUM_ASB)) ) 
	{
	    /*
     	     * If we injected one in the past, free it
	     */
	    db = FindLSA(dst, RRT_DEST(r), MY_ID, LS_SUM_ASB);
	    if (db != LSDBNULL) {
		/*
		 * Flush this asb route from this area if it existed
		 */
		if ((db) && (DB_WHERE(db) == ON_SUMASB_LIST)) {
	    	    /* MODIFIED 5/1/92 */
		    DB_WHERE(db) = ON_SUM_INFINITY;
	    	    DEL_DBQ(db);
	    	    DB_FREEME(db) = TRUE;
	    	    ADD_DBQ(&ospf.db_free_list, db);
		    sum_next_seq(db, dst, htonl(SUMLSInfinity), time_sec);
		}
	    }
	    continue;
	}
	/*
         * inter area routes have been added in spf by build_inter
         */
	foundlsa = AddLSA(&db, dst, RRT_DEST(r), MY_ID, 0, LS_SUM_ASB);

	/*
         * the ones on the routing table are reachable
         */
	if (!foundlsa) {
	    if (!db)
	    	return FLAG_NO_BUFS;
	    /*
	     * If not valid route, ignore
	     */
	    if ((RRT_REV(r) != RTAB_REV)) {
	    	ospf.db_cnt--;
		dst->db_cnts[LS_SUM_ASB]--;
		dst->db_int_cnt--;
		db_free(db);
		continue;
	    }
	    /*
	     * new asbr
	     */
	    SUM_HDR_ALLOC(s, SUM_LA_HDR_SIZE);
	    if (!s)
	    {
	    	ospf.db_cnt--;
		dst->db_cnts[LS_SUM_ASB]--;
		dst->db_int_cnt--;
		db_free(db);
		return FLAG_NO_BUFS;
	    }
	    s->ls_hdr.ls_type = LS_SUM_ASB;
	    s->ls_hdr.ls_id = RRT_DEST(r);
	    s->ls_hdr.adv_rtr = MY_ID;
	    s->ls_hdr.ls_seq = FirstSeq;
	    s->ls_hdr.length = htons(SUM_LA_HDR_SIZE);
	    s->tos0.tos_metric = htonl(RRT_COST(r));
	    DB_SUM(db) = s;
	    ADD_DBQ(&dst->asblst, db);
	    DB_WHERE(db) = ON_SUMASB_LIST;
	    DB_TIME(db) = time_sec;
	    txadd(db, &(dst->txq), SUM_LA_HDR_SIZE, 0, dst);
	} else if (DB_WHERE(db) == ON_SUMASB_LIST) {
	    /*
	     * may have changed
	     */
	    /* MODIFIED 5/1/92 */
	    DB_FREEME(db) = FALSE;
	    if (RRT_REV(r) != RTAB_REV) {
		/*
	         * no longer valid route
	         */
	    	/* MODIFIED 5/1/92 */
		DB_WHERE(db) = ON_SUM_INFINITY;
	    	DEL_DBQ(db);
	    	DB_FREEME(db) = TRUE;
	    	ADD_DBQ(&ospf.db_free_list, db);
		sum_next_seq(db, dst, htonl(SUMLSInfinity), time_sec);
	    } else if (DB_SUM(db)->tos0.tos_metric != htonl(RRT_COST(r))) {
		sum_next_seq(db, dst, htonl(RRT_COST(r)), time_sec);
	    }
	} else if (DB_WHERE(db) == ON_SUM_INFINITY) {
	    /*
	     * If not valid route, ignore
	     */
	    if ((RRT_REV(r) != RTAB_REV)) {
		continue;
	    }
	    /*
	     * was unreachable, now reachable
	     */
	    if (DB_FREEME(db) == TRUE) {
		DB_FREEME(db) = FALSE;
		DEL_DBQ(db);		/* remove from db_free_list */
	    }
	    DB_WHERE(db) = ON_SUMASB_LIST;
	    ADD_DBQ(&dst->asblst, db);
	    sum_next_seq(db, dst, htonl(RRT_COST(r)), time_sec);
	}
	/*
         * ones currently on infinity list will just age out
         */
	if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	    ospf_trace_build(area, dst, DB_ADV(db), FALSE);
	}
    } AREA_LIST_END(dst) ;

    return FLAG_NO_PROBLEM;
}

/*
 * build_sum() builds all inter area routes
 * - changes have been sent when changes have been seen
 *    as a result of running spf or receiving a new sum pkt
 * - this routine just updates the sequence numbers and sends out new sum
 *    pkts called by tq_SumLsa
 */
int
build_sum __PF0(void)
{
    struct LSDB *db;
    struct AREA *a;
    char *timestr;
    int	ret_flag = FLAG_NO_PROBLEM;

    timestr = ospf_get_ctime();

    AREA_LIST(a) {
	/* new sequence number only */

	for (db = DB_PTR(&a->asblst)[NEXT];
	     db != LSDBNULL;
	     db = DB_PTR(db)[NEXT]) 
	{
	    if ((time_sec - DB_TIME(db)) < MinLSInterval)
		continue;
	    sum_next_seq(db, a, DB_SUM(db)->tos0.tos_metric, time_sec);

	    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		ospf_trace_build(a, a, DB_ADV(db), FALSE);
	    }

	}

	for (db = DB_PTR(&a->sumnetlst)[NEXT];
	     db != LSDBNULL;
	     db = DB_PTR(db)[NEXT])
	{
	    if ((time_sec - DB_TIME(db)) < MinLSInterval)
		continue;
	    sum_next_seq(db, a, DB_SUM(db)->tos0.tos_metric, time_sec);

	    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		ospf_trace_build(a, a, DB_ADV(db), FALSE);
	    }

	}

	if (BIT_TEST(a->area_flags, OSPF_AREAF_STUB_DEFAULT)) {
	    /* Update stub area default route */

	    db = DB_PTR(&a->dflt_sum)[NEXT];
	    if ((time_sec - DB_TIME(db)) >= MinLSInterval) {
	    	sum_next_seq(db, a, a->dflt_metric, time_sec);

		if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		    ospf_trace_build(a, a, DB_ADV(db), FALSE);
		}

	    }
	}

	/* 
	 * Inter area routes determined from routing table 
	 */
	for (db = DB_PTR(&a->interlst)[NEXT];
	     db != LSDBNULL;
	     db = DB_PTR(db)[NEXT]) 
	{
	    if ((time_sec - DB_TIME(db)) < MinLSInterval)
		continue;
	    sum_next_seq(db, a, DB_SUM(db)->tos0.tos_metric, time_sec);

	    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
		ospf_trace_build(a, a, DB_ADV(db), FALSE);
	    }
	}

	/* 
	 * Send the whole shebangee 
 	 */
	if (a->txq != LLNULL) {
	    self_orig_area_flood(a, a->txq, LS_SUM_NET);
	    ospf_freeq((struct Q **) &a->txq, OMEM_LL);
	}
	if (ret_flag)
	    return FLAG_NO_BUFS;
    } AREA_LIST_END(area) ;

    /* If they make it on the retrans list, that's good enough */
    return FLAG_NO_PROBLEM;
}

/*
 * Build inter-area route
 *   - db is an inter-area route belonging to the backbone area,
 *	but not originated by this router
 *   - add to every area except backbone area
 *   - called by spf and rxlinkup
 */
int
build_inter __PF3(v, struct LSDB *,
		  area, struct AREA *,	/* originating area */
		  what, int)
{
    struct LSDB *db;
    struct AREA *dst;			/* inject into this area */
    struct SUM_LA_HDR *s;
    int foundlsa = 0;
    int	ret_flag = FLAG_NO_PROBLEM;

    /* Backbone route exists */
    AREA_LIST(dst) {

	if (area == dst) {
	    /* Flush this route from this area if it existed */
	    db = FindLSA(dst, DB_NETNUM(v), MY_ID, LS_SUM_NET);
	    if ((db) && (DB_WHERE(db) == ON_SUMNET_LIST)) {
	    	/* MODIFIED 5/1/92 */
		DB_WHERE(db) = ON_SUM_INFINITY;
	    	DEL_DBQ(db);
	    	DB_FREEME(db) = TRUE;
	    	ADD_DBQ(&ospf.db_free_list, db);
		sum_next_seq(db, dst, htonl(SUMLSInfinity), time_sec);
	    }
	    continue;
	}

	foundlsa = AddLSA(&db, dst, DB_NETNUM(v), MY_ID, 0, LS_SUM_NET);
	if (!foundlsa) {
	    u_int16 age = 0;

	    if (db == LSDBNULL)
		return FLAG_NO_BUFS;
	    SUM_HDR_ALLOC(s, SUM_LA_HDR_SIZE);
	    if (!s) {
	    	ospf.db_cnt--;
    		dst->db_cnts[LS_SUM_NET]--;
		dst->db_int_cnt--;
		db_free(db);
		return FLAG_NO_BUFS;
	    }
	    s->ls_hdr.ls_type = LS_SUM_NET;
	    s->ls_hdr.ls_id = DB_NETNUM(v);
	    s->ls_hdr.adv_rtr = MY_ID;
	    s->ls_hdr.ls_seq = FirstSeq;
	    s->ls_hdr.length = htons(SUM_LA_HDR_SIZE);
	    s->net_mask = DB_MASK(v);
	    DB_SUM(db) = s;
	    if (DB_ROUTE(v) && what != E_DELETE) {
		s->tos0.tos_metric = htonl(ORT_COST(DB_ROUTE(v)));
		ADD_DBQ(&dst->interlst, db);
		DB_WHERE(db) = ON_INTER_LIST;
	    } else {
		DB_WHERE(db) = ON_SUM_INFINITY;
	    	/* MODIFIED 5/1/92 */
	    	DB_FREEME(db) = TRUE;
	    	ADD_DBQ(&ospf.db_free_list, db);
		s->tos0.tos_metric = htonl(SUMLSInfinity);
		age = MaxAge;
	    }
	    DB_TIME(db) = time_sec;
	    txadd(db, &(dst->txq), SUM_LA_HDR_SIZE, age, dst);
	} else {			/* had one */
	    u_int32 metric = htonl(SUMLSInfinity);

	    /* 
	     * set db->where and put on the inter list if needed
	     */
	    if (what == E_DELETE) {
	    	/* MODIFIED 5/1/92 */
	    	DEL_DBQ(db);
	    	DB_FREEME(db) = TRUE;
	    	ADD_DBQ(&ospf.db_free_list, db);
		DB_WHERE(db) = ON_SUM_INFINITY;
	    } else if (DB_ROUTE(v)) {
		metric = htonl(ORT_COST(DB_ROUTE(v)));
		DB_FREEME(db) = FALSE;
		DEL_DBQ(db);	/* remove from db_free_list */
		ADD_DBQ(&dst->interlst, db);
		DB_WHERE(db) = ON_INTER_LIST;
	    }

	    sum_next_seq(db, dst, metric, time_sec);
	}

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	    ospf_trace_build(&ospf.backbone, dst, DB_ADV(db), FALSE);
	}
    } AREA_LIST_END(dst) ;
    return ret_flag;
}

/*
 * Build default route for area border router
 * - called by init
 */
void
build_sum_dflt __PF1(a, struct AREA *)
{
    struct SUM_LA_HDR *s;
    struct LSDB *db;

    AddLSA(&db, a, 0, MY_ID, 0, LS_SUM_NET);
    SUM_HDR_ALLOC(s, SUM_LA_HDR_SIZE);
    s->ls_hdr.ls_type = LS_SUM_NET;
    s->ls_hdr.ls_id = 0;
    s->ls_hdr.adv_rtr = MY_ID;
    s->ls_hdr.ls_seq = FirstSeq;
    s->ls_hdr.length = htons(SUM_LA_HDR_SIZE);
    s->net_mask = 0;
    s->tos0.tos_metric = a->dflt_metric;
    DB_SUM(db) = s;
    /* 
     * add to this area's net sum list 
     */
    DB_WHERE(db) = ON_SUMNET_LIST;
    ADD_DBQ(&a->dflt_sum, db);
    DB_TIME(db) = time_sec;
    ospf_checksum_sum(DB_SUM(db), SUM_LA_HDR_SIZE, a->db_chksumsum);
    a->db_cnts[LS_SUM_NET]++;

    if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	ospf_trace_build(a, a, DB_ADV(db), FALSE);
    }
}

/*
 * Acks have come home, generate new LSA with seq FirstSeq, put on txq
 */
int
beyond_max_seq __PF6(a, struct AREA *,
		     intf, struct INTF *,
		     db, struct LSDB *,
		     txq, struct LSDB_LIST **,
		     asetxq, struct LSDB_LIST **,
		     from_rxlinkup, int)
{

    if (LS_TYPE(db) == LS_RTR) {
	if (from_rxlinkup) {
	    a->build_rtr = TRUE;
	    return FLAG_NO_PROBLEM;
	}
	return build_rtr_lsa(a, txq, TRUE);
    }

    if (LS_TYPE(db) == LS_NET) {
	if (intf == INTFNULL) {
	    INTF_LIST(intf, a) {
		if (INTF_LCLADDR(intf) == LS_ID(db)) {
		    break;
		}
	    } INTF_LIST_END(intf, a) ;
	}
	if (intf == INTFNULL) {
	    return FLAG_NO_PROBLEM;		/* something has gone weird */
	}
	if (from_rxlinkup && intf->state == IDr) {
	    BIT_SET(intf->flags, OSPF_INTFF_BUILDNET);
	    return FLAG_NO_PROBLEM;
	}
	return build_net_lsa(intf, txq, TRUE);
    }


    DB_SEQ_MAX(db) = FALSE;
    LS_SEQ(db) = NEXTNSEQ(LS_SEQ(db));
    if (LS_TYPE(db) == LS_ASE) {
    	ospf.db_chksumsum -= LS_CKS(db);
	txadd(db, asetxq, (size_t) ntohs(LS_LEN(db)), 0, a);
    } else {
    	a->db_chksumsum -= LS_CKS(db);
	txadd(db, txq, (size_t) ntohs(LS_LEN(db)), 0, a);
    }
    return FLAG_NO_PROBLEM;
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
