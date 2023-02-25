static char sccsid[] = "@(#)06	1.1  src/tcpip/usr/sbin/gated/ospf_rxpkt.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:39";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF4
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
 * ospf_rxpkt.c,v 1.21 1993/03/25 12:47:21 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


/* LS Ack packet */
static int
ospf_rx_lsack  __PF5(ack, struct LS_ACK_HDR *,
		     intf, struct INTF *,
		     src, sockaddr_un *,
		     router_id, sockaddr_un *,
		     olen, size_t)
{
    struct NBR *nbr = NBRNULL, *n;
    struct LS_HDR *ap = &(ack->ack_piece);
    struct LSDB *db = LSDBNULL;
    struct AREA *a, *area = intf->area;
    struct LSDB_LIST *trans = LLNULL, *asetrans = LLNULL;
    int	ret_flag;

    /* list of db entries to be freed */

    /* Locate nbr */
    if (intf->type > NONBROADCAST) {
	n = FirstNbr(intf);
	if (n->nbr_id &&
	    sockaddrcmp_in(n->nbr_id, router_id)) {
	    nbr = n;
	}
    } else
	NBRS_LIST(n, intf) {
	    if (sockaddrcmp_in(src, n->nbr_addr)) {
		nbr = n;
		break;
	    }
	} NBRS_LIST_END(n, intf) ;

    if (nbr == NBRNULL)
	return NO_ACK_NBR;
    if (nbr->state < NEXCHANGE)
	return LOW_NBR_STATE3;

    for (olen -= OSPF_HDR_SIZE;
	 olen > 0;
	 olen -= ACK_PIECE_SIZE) {
	ret_flag = 0;

	switch (ap->ls_type) {
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
	    ap++;
	    continue;

	default:
	    ospf_log_rx(BAD_LSA_TYPE,
			intf->ifap->ifa_addr_local,
			nbr->nbr_addr);
	    ap++;
	    continue;
	}
	if (!(db = FindLSA(area, ap->ls_id, ap->adv_rtr, ap->ls_type))) {
	    ap++;
	    continue;
	}
 	GNTOHS(ap->ls_age);
	if (SAME_INSTANCE(ap, &(DB_RTR(db)->ls_hdr), 0)) {
	    if (DB_RETRANS(db) != NLNULL) {	/* on retrans list */
		/* remove from lsdb */
		rem_nbr_ptr(db, nbr);
		/* remove from nbr */
		rem_db_ptr(nbr, db);

		/* If MaxSeq and this is last ack, generate a new one */
		if (DB_SEQ_MAX(db) == TRUE &&
		    DB_RETRANS(db) == NLNULL &&
		    DB_FREEME(db) != TRUE)
		    area->spfsched |= beyond_max_seq(area,
						     intf,
						     db,
						     &trans,
						     &asetrans,
						     0);

		/*
		 * if MaxAge we have flooded this one - check for empty
		 * retrans list - if empty, remove from lsdb
		 */
		if (DB_CAN_BE_FREED(db)) {
		    db_free(db);
		}
	    }
	}
	ap++;
    }

    if (asetrans != LLNULL) {	/* new self originated ls_ase */
	AREA_LIST(a) {
	    if (!BIT_TEST(a->area_flags, OSPF_AREAF_STUB)) {
		self_orig_area_flood(a, asetrans, LS_ASE);
	    }
	} AREA_LIST_END(a) ;
	ospf_freeq((struct Q **)&asetrans, OMEM_LL);
    }
    if (trans != LLNULL) {
	self_orig_area_flood(area, trans, LS_RTR);
	ospf_freeq((struct Q **)&trans, OMEM_LL);
    }

    return GOOD_RX;
}

/**/

/* Database Description packet */

/*
 * parse packet and put on the nbrs link state req list
 */
static int
db_parse __PF4(dbh, struct DB_HDR *,
	       nbr, struct NBR *,
	       intf, struct INTF *,
	       len, size_t)
{
    struct LS_HDR *dbp;
    struct LSDB *db;
    struct AREA *a = intf->area;
    struct LS_REQ *lr;
    u_int32 diff;		/* to calculate db elapsed time */

    dbp = &(dbh->dbp);

    for (; len; len -= DB_PIECE_SIZE) {
	/* If we don't have a copy or if nbrs info is newer */

	switch (dbp->ls_type) {
	case LS_RTR:
	case LS_NET:
	case LS_SUM_NET:
	case LS_SUM_ASB:
	case LS_ASE:
	    /* OK */
	    break;

	case LS_GM:
	case LS_NSSA:
	    /* Fall through */

	default:
	    ospf_log_rx(BAD_LSA_TYPE,
			intf->ifap->ifa_addr_local,
			nbr->nbr_addr);
	    (*(nbr_trans[SEQ_MISMATCH][nbr->state])) (intf, nbr);
	    return 1;
	}
	if (db = FindLSA(a, dbp->ls_id, dbp->adv_rtr, dbp->ls_type))
	    diff = time_sec - DB_TIME(db);
	else
	    diff = 0;
	dbp->ls_age = ntohs(dbp->ls_age);
	if (!db || MORE_RECENT(dbp, &(DB_RTR(db)->ls_hdr), diff)) {
	    /* piece is newer or we don't have it - add to list */
	    LS_REQ_ALLOC(lr);
	    lr->ls_id = dbp->ls_id;
	    lr->adv_rtr = dbp->adv_rtr;
	    lr->ls_age = htons(dbp->ls_age);
	    lr->ls_seq = dbp->ls_seq;
	    lr->ls_chksum = dbp->ls_chksum;
	    EN_Q((nbr->ls_req[dbp->ls_type]), lr);
	    nbr->reqcnt++;
	}
	dbp++;
    }
    return FLAG_NO_PROBLEM;
}


static int
ospf_rx_db  __PF5(dbh, struct DB_HDR *,
		     intf, struct INTF *,
		     src, sockaddr_un *,
		     router_id, sockaddr_un *,
		     olen, size_t)
{
    int	sched = 0;
    int ret = GOOD_RX;
    int reqcnt;
    struct NBR *n;
    struct NBR *nbr = NBRNULL;
    struct DB_HDR *d;
    struct LSDB_SUM *nextds;
    struct LSDB_LIST *txq = LLNULL;
    struct AREA *area = intf->area;	/* area received from */

    /* Locate nbr */
    if (intf->type > NONBROADCAST) {
	n = FirstNbr(intf);
	if (n->nbr_id &&
	    sockaddrcmp_in(n->nbr_id, router_id)) {
	    nbr = n;
	}
    } else {
	NBRS_LIST(n, intf) {
	    if (sockaddrcmp_in(src, n->nbr_addr)) {
		nbr = n;
		break;
	    }
	} NBRS_LIST_END(n, intf) ;
    }
    if (nbr == NBRNULL)
	return CANT_FIND_NBR1;

    if (BIT_MATCH(dbh->options, OPT_E_bit) == BIT_MATCH(area->area_flags, OSPF_AREAF_STUB))
	return BAD_DD_E_bit;

    olen -= (OSPF_HDR_SIZE + DB_HDR_SIZE);
    GNTOHL(dbh->seq);
    reqcnt = nbr->reqcnt;

    switch (nbr->state) {
    case NDOWN:
    case NATTEMPT:
	return LOW_NBR_STATE1;
    case N2WAY:
	return GOOD_RX;
    case NINIT:
	(*(nbr_trans[TWOWAY][nbr->state])) (intf, nbr);
	/* if nbr didn't make it to NEXTSTART chuckit */
	if (nbr->state != NEXSTART)
	    goto we_are_through;
	send_dbsum(intf, nbr, NOT_RETRANS);
	/* if forming adjacency continue processing */
    case NEXSTART:
	/* if first exchange pkt and this nbr's id is > than ours  */
	if ((BIT_MATCH(dbh->I_M_MS, bit_I | bit_M | bit_MS) && !olen) &&
	    ntohl(NBR_ID(nbr)) > ntohl(MY_ID)) {
	    /* We are now in SLAVE mode */

	    nbr->mode = SLAVE;
	    nbr->I_M_MS = 0;
	    nbr->seq = dbh->seq;
	    (*(nbr_trans[NEGO_DONE][nbr->state])) (intf, nbr);
	    /* did we make it to NEXCHANGE? (out of bufs?) */
	    if (nbr->state != NEXCHANGE) {
		nbr->I_M_MS = (bit_I | bit_M | bit_MS);
		nbr->mode = 0;
		goto we_are_through;
	    }
	    /* Slave sends in response */
	    send_dbsum(intf, nbr, NOT_RETRANS);
	} else if (!BIT_MATCH(dbh->I_M_MS, bit_I | bit_MS) &&
		   (nbr->seq == dbh->seq) &&
		   (ntohl(NBR_ID(nbr)) < ntohl(MY_ID))) {
	    /* check to see if it was an ack (we are master) */
	    nbr->mode = MASTER;
	    nbr->I_M_MS = bit_MS;	/* set to master */
	    (*(nbr_trans[NEGO_DONE][nbr->state])) (intf, nbr);
	    /* did we make it to NEXCHANGE? (out of bufs?) */
	    if (nbr->state != NEXCHANGE) {
		nbr->I_M_MS = (bit_I | bit_M | bit_MS);
		nbr->mode = 0;
		goto we_are_through;
	    }
	    if (olen) {
		if (db_parse(dbh, nbr, intf, olen))	/* out of bufs */
		    goto we_are_through;
	    }
	    nbr->seq++;
	    send_dbsum(intf, nbr, NOT_RETRANS);
	} else if (NBR_ID(nbr) == MY_ID) {
	    ret = BAD_RTRID;
	    goto we_are_through;
	}
	break;

    case NEXCHANGE:
	/* check for master mismatch */
	if ((BIT_MATCH(dbh->I_M_MS, bit_MS) == (nbr->mode == MASTER)) ||
	    BIT_TEST(dbh->I_M_MS, bit_I)) {
	    (*(nbr_trans[SEQ_MISMATCH][nbr->state])) (intf, nbr);
	    goto we_are_through;
	}
	if (nbr->mode == MASTER) {
	    if (nbr->seq == dbh->seq) {
		if (nbr->dbsum == LSDB_SUM_NULL)
		    d = (struct DB_HDR *) 0;
		else
		    d = &(nbr->dbsum->dbpkt->un.database);

		/* if it's SLAVE's last and my last */
		nbr->seq++;
		/* parse packet and put on the nbrs link state req list */
		if (olen) {
		    if (db_parse(dbh, nbr, intf, olen))
			goto we_are_through;
		}
		if (!BIT_TEST(dbh->I_M_MS, bit_M) &&	/* slave's last */
		    (d != (struct DB_HDR *) 0) &&	/* pkts left */
		    !BIT_TEST(d->I_M_MS, bit_M)) {
		    /* then, we're up to loading state */
		    (*(nbr_trans[EXCH_DONE][nbr->state])) (intf, nbr);
		    goto we_are_through;
		}
		/* free head of list f more bit was set */
		if ((d != (struct DB_HDR *) 0) &&
		    BIT_TEST(d->I_M_MS, bit_M)) {
		    /* One less dbsum pkt */
		    nbr->dbcnt -= nbr->dbsum->cnt;
		    nextds = nbr->dbsum->next;
		    DB_FREE_PKT(nbr->dbsum);
		    FREE(nbr->dbsum, OMEM_DBSUM);
		    nbr->dbsum = nextds;
		}
		send_dbsum(intf, nbr, NOT_RETRANS);

	    } else if (nbr->seq - 1 != dbh->seq) {
		(*(nbr_trans[SEQ_MISMATCH][nbr->state])) (intf, nbr);
	    }
	} else {		/* we are in SLAVE mode  */
	    if (nbr->seq + 1 == dbh->seq) {
		nbr->seq = dbh->seq;

		/* remove last sent */
		if (nbr->dbsum != (struct LSDB_SUM *) 0) {
		    /* One less dbsum pkt */
		    nbr->dbcnt -= nbr->dbsum->cnt;
		    nextds = nbr->dbsum->next;
		    DB_FREE_PKT(nbr->dbsum);
		    FREE(nbr->dbsum, OMEM_DBSUM);
		    nbr->dbsum = nextds;
		}
		if (nbr->dbsum != (struct LSDB_SUM *) 0)
		    d = &(nbr->dbsum->dbpkt->un.database);
		else
		    d = (struct DB_HDR *) NULL;

		/* if it's MASTER's last and my last */
		if (!BIT_TEST(dbh->I_M_MS, bit_M) &&	/* master's last */
		    ((d == (struct DB_HDR *) NULL) ||	/* no pkts left */
		     !BIT_TEST(d->I_M_MS, bit_M)))	/* my more bit is off */
		    sched++;	/* schedule an exchange done event */

		if (db_parse(dbh, nbr, intf, olen))	/* run out of bufs */
		    goto we_are_through;
		send_dbsum(intf, nbr, NOT_RETRANS);

		if (sched) {	/* up to loading state */
		    (*(nbr_trans[EXCH_DONE][nbr->state])) (intf, nbr);
		}
	    } else if (nbr->seq == dbh->seq)
		send_dbsum(intf, nbr, NOT_RETRANS);
	    else {
		(*(nbr_trans[SEQ_MISMATCH][nbr->state])) (intf, nbr);
	    }
	}
	break;

    case NLOADING:
    case NFULL:
	if (nbr->mode == SLAVE_HOLD && nbr->seq == dbh->seq)
	    /* haven't timed out yet */
	    send_dbsum(intf, nbr, NOT_RETRANS);
	else if ((nbr->mode == MASTER && nbr->seq - 1 == dbh->seq) ||
		 (nbr->mode == SLAVE && nbr->seq == dbh->seq))
	    goto we_are_through;
	else {
	    (*(nbr_trans[SEQ_MISMATCH][nbr->state])) (intf, nbr);
	}

	break;
    }

  we_are_through:
    if (BIT_TEST(intf->flags, OSPF_INTFF_NBR_CHANGE)) {
	(*(if_trans[NBR_CHANGE][intf->state])) (intf);
    }

    /* build net and rtr lsa if necessary */
    if (BIT_TEST(intf->flags, OSPF_INTFF_BUILDNET)) {
	area->spfsched |= build_net_lsa(intf, &txq, 0);
	BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);
    }
    if (area->build_rtr) {
	area->spfsched |= build_rtr_lsa(area, &txq, 0);
	area->build_rtr = FALSE;
    }
    if (txq != LLNULL) {	/* may be locked out */
	self_orig_area_flood(area, txq, LS_RTR);
	ospf_freeq((struct Q **)&txq, OMEM_LL);
    }

    /* Can we send req pkt? */
    if ((nbr->state >= NEXCHANGE) &&
	(reqcnt == 0) &&
	(nbr->reqcnt > 0))
	send_req(intf, nbr, 0);

    return ret;
}


/**/

/* Hello packet */


static int
ospf_rx_hello  __PF5(hello, struct HELLO_HDR *,
		     intf, struct INTF *,
		     src, sockaddr_un *,
		     router_id, sockaddr_un *,
		     olen, size_t)
{
    int 	nbr_is_dr = FALSE, nbr_is_bdr = FALSE;
    int		rtr_heard_from = FALSE;
    int		backup_seen = FALSE;
    struct 	NBR *nbr;
    struct 	NBR *newnbr = NBRNULL;
    struct 	RHF *rhf, *last_rhf;
    struct 	LSDB_LIST *txq = LLNULL;
    struct 	AREA *area = intf->area;	/* area received from */

    if (intf->state == IDOWN) {
	/* wait until virtual nbr is brought up by trans area running SPF */
	if (intf->type == VIRTUAL_LINK)
	    return UNKNOWN_VIRT_NBR;
	ospf_ifup(intf);
	return RX_ON_DOWN_IF;
    }

    /* 
     * a few validity checks 
     */
    if (intf->type != VIRTUAL_LINK
	&& hello->netmask != INTF_MASK(intf))
	return BAD_HELLO_MASK;
    if (ntohs(hello->HelloInt) != intf->hello_timer)
	return BAD_HELLO_TIMER;
    if (ntohl(hello->DeadInt) != intf->dead_timer)
	return BAD_DEAD_TIMER;
    if (BIT_MATCH(hello->options, OPT_E_bit) == BIT_MATCH(area->area_flags, OSPF_AREAF_STUB))
	return BAD_HELLO_E_bit;

    /*
     * for multi-access nets find the nbr that origniated this hello
     */
    if (intf->type <= NONBROADCAST) {
	for (nbr = intf->nbr.next; nbr != NBRNULL; nbr = nbr->next) {
	    if (!NBR_ADDR(nbr))
		continue;
	    if (sockaddrcmp_in(src, nbr->nbr_addr)) {
		newnbr = nbr;
		break;
	    }
	}
    }
 
    if (!newnbr) {
	/* New neighbor */

	if (sockaddrcmp_in(router_id, ospf.router_id)) {
	    return HELLO_ID_CONFUSION;
	}
	
	switch (intf->type) {
	case POINT_TO_POINT:
	    newnbr = &(intf->nbr);
	    if (newnbr->nbr_addr) {
		sockfree(newnbr->nbr_addr);
	    }
	    newnbr->nbr_addr = sockdup(src);
	    break;

	case VIRTUAL_LINK:
	    newnbr = &(intf->nbr);
	    break;

	case NONBROADCAST:
	    /* NBMA DR elig nbrs should be configured */
	    if (intf->pri) {
		ospf_log_rx(UNKNOWN_NBMA_NBR,
			    intf->ifap->ifa_addr_local,
			    src);
	    }
	    /* Fall through */

	case BROADCAST:
	    NBR_ALLOC(newnbr);
	    if (!newnbr) {
		return GOOD_RX;
	    }
	    newnbr->nbr_addr = sockdup(src);
	    nbr_enq(intf,newnbr);
	    break;
	}
	if (intf->type != VIRTUAL_LINK) {
	    if (newnbr->nbr_id) {
		sockfree(newnbr->nbr_id);
	    }
	    newnbr->nbr_id = sockdup(router_id);
#ifdef	notdef
	    if (newnbr->ifap) {
		ifa_free(newnbr->ifap);
	    }
	    newnbr->ifap = ifa_alloc(intf->ifap);
#endif	/* notdef */
	    newnbr->intf = intf;
	    newnbr->pri = hello->rtr_priority;
	} else if (!sockaddrcmp_in(newnbr->nbr_id, router_id)) {
	    return UNKNOWN_VIRT_NBR;
	}
    } else {
	/* Old neighbor */

	if (!newnbr->nbr_id ||
	    !sockaddrcmp_in(newnbr->nbr_id, router_id)) {
	    /* No neighbor ID, or it changed */
	    if (newnbr->nbr_id) {
		sockfree(newnbr->nbr_id);
	    }
	    newnbr->nbr_id = sockdup(router_id);
	}
    }
    reset_inact_tmr(newnbr);		/* if not set don't set */


    /* 
     * have already reset rtr dead timer or will start in NHello 
     */
    if (intf->type == VIRTUAL_LINK || intf->type == NONBROADCAST) {
	if ((newnbr->state == NATTEMPT) || (newnbr->state == NDOWN))
	    (*(nbr_trans[HELLO_RX][newnbr->state])) (intf, newnbr);
    } else if (newnbr->state == NDOWN)
	(*(nbr_trans[HELLO_RX][newnbr->state])) (intf, newnbr);

    /* 
     * see if THIS rtr appears in the router-heard-from list 
     */
    last_rhf = (struct RHF *) ((u_int32) & (hello->rhf) +
			       (olen - (OSPF_HDR_SIZE + HELLO_HDR_SIZE)));

    for (rhf = (struct RHF *) & (hello->rhf);
	 rhf <  last_rhf;
	 rhf++) {
	if (rhf->rtr == MY_ID) {
	    if (newnbr->state < N2WAY)
		(*(nbr_trans[TWOWAY][newnbr->state])) (intf, newnbr);
	    rtr_heard_from++; 		/* flag that we found it */
	    break;
	}
    }

    /* drop back to one way until we can see ourself in this nbr's hello */
    if (!rtr_heard_from) {
	(*(nbr_trans[ONEWAY][newnbr->state])) (intf, newnbr);
    } else if (intf->type <= NONBROADCAST) {

	if (hello->dr == NBR_ADDR(newnbr))
	    nbr_is_dr++;

	if (hello->bdr == NBR_ADDR(newnbr))
	    nbr_is_bdr++;

	/* 
	 * if this nbr is bdr or it's dr and there is no bdr 
	 */
	if ((intf->state == IWAITING) &&
	    (((nbr_is_dr && !(hello->bdr))) || (nbr_is_bdr))) 
	{
		newnbr->dr = hello->dr;
                newnbr->bdr = hello->bdr;
		backup_seen = TRUE;
	} else if (hello->rtr_priority != newnbr->pri) /* priority change? */ {
	    newnbr->pri = hello->rtr_priority;
	    if (intf->state >= IDr)
		BIT_SET(intf->flags, OSPF_INTFF_NBR_CHANGE);	/* schedule nbr change */
    	} else {	/* dr/bdr changeola? */
    	    if ((hello->dr != newnbr->dr) &&
      	    	((newnbr->dr == NBR_ADDR(newnbr)) || (nbr_is_dr)))
		BIT_SET(intf->flags, OSPF_INTFF_NBR_CHANGE);
    	    else if ((hello->bdr != newnbr->bdr) &&
      	    	((newnbr->bdr == NBR_ADDR(newnbr)) || (nbr_is_bdr)))
		BIT_SET(intf->flags, OSPF_INTFF_NBR_CHANGE);
	}

	newnbr->dr = hello->dr;
	newnbr->bdr = hello->bdr;
    }


    /* If not elig send hello to elig nbr if nbr isn't dr or bdr */
    if (intf->type == NONBROADCAST &&
	(!intf->pri) &&
	newnbr->pri &&
	intf->dr != newnbr &&
	intf->bdr != newnbr)
	send_hello(intf, newnbr, FALSE);

    if (backup_seen) {
	(*(if_trans[BACKUP_SEEN][intf->state])) (intf);
    } else if (BIT_TEST(intf->flags, OSPF_INTFF_NBR_CHANGE)) {
	(*(if_trans[NBR_CHANGE][intf->state])) (intf);
    }

    /* build net and rtr lsa if necessary */
    if (BIT_TEST(intf->flags, OSPF_INTFF_BUILDNET)) {
	area->spfsched |= build_net_lsa(intf, &txq, 0);
	BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);
    }
    if (area->build_rtr) {
	area->spfsched |= build_rtr_lsa(area, &txq, 0);
	area->build_rtr = FALSE;
    }
    if (txq != LLNULL) {		/* may be locked out */
	self_orig_area_flood(area, txq, LS_RTR);
	ospf_freeq((struct Q **) &txq, OMEM_LL);
    }

    return GOOD_RX;
}


/**/

/* LS Request */


/*
 * ospf_rx_lsreq - receive an ls request
 *	   - while parsing build and send ls pkt
 */
/* ospf_rxreq.c */
static int
ospf_rx_lsreq  __PF5(ls_req, struct LS_REQ_HDR *,
		     intf, struct INTF *,
		     src, sockaddr_un *,
		     router_id, sockaddr_un *,
		     olen, size_t)
{
    struct NBR *n, *nbr = NBRNULL;
    struct OSPF_HDR *pkt;
    struct LS_REQ_PIECE *req = &ls_req->req;
    struct AREA *a = intf->area;
    struct LSDB *db;
    struct LSDB_LIST *txq = LLNULL;
    u_int32 *adv_cnt;
    union ADV *adv;
    size_t len = OSPF_HDR_SIZE + 4;/* max len of pkt is INTF_MTU */
    int intf_mtu = INTF_MTU(intf);
    int ret = GOOD_RX;

    /* Locate nbr */
    if (intf->type > NONBROADCAST) {
	n = FirstNbr(intf);
	if (n->nbr_id &&
	    sockaddrcmp_in(n->nbr_id, router_id)) {
	    nbr = n;
	}
    } else {
	NBRS_LIST(n, intf) {
	    if (sockaddrcmp_in(src, n->nbr_addr)) {
		nbr = n;
		break;
	    }
	} NBRS_LIST_END(n, intf) ;
    }

    if (nbr == NBRNULL)
	return CANT_FIND_NBR4;
    if (nbr->state < NEXCHANGE)
	return LOW_NBR_STATE4;

    olen -= OSPF_HDR_SIZE;
    if (!olen)
	return EMPTY_LS_REQ;

    NEW_LSU(pkt, adv_cnt, adv);
    /* Out of pkt buffers? */
    if (!pkt)
	return GOOD_RX;

    for (; olen; olen -= LS_REQ_PIECE_SIZE) {
	if ((req->ls_type < LS_RTR || req->ls_type > LS_ASE) ||
	    !(db = FindLSA(a, req->ls_id, req->adv_rtr, req->ls_type))) {
	    (*(nbr_trans[BAD_LS_REQ][nbr->state])) (intf, nbr);
	    ret = BOGUS_REQ;
	    goto we_are_through;
	}
	if ((len + ntohs(LS_LEN(db))) > intf_mtu) {
	    GHTONL(*adv_cnt);
	    ospf_txpkt(pkt, intf, O_LSU, len, NBR_ADDR(nbr), NOT_RETRANS);
	    len = OSPF_HDR_SIZE + 4;	/* 4 for adv_cnt */
	    NEW_LSU(pkt, adv_cnt, adv);
	}
	(*adv_cnt)++;
	ADV_COPY(DB_RTR(db), adv, ntohs(LS_LEN(db)));
	adv->rtr.ls_hdr.ls_age = htons(MIN(ADV_AGE(db) + intf->transdly, MaxAge));
	adv = (union ADV *) ((long) adv + ntohs(LS_LEN(db)));
	len += ntohs(LS_LEN(db));
	req++;
    }

    if (*adv_cnt) {		/* any left to send? */
	GHTONL(*adv_cnt);
	ospf_txpkt(pkt, intf, O_LSU, len, NBR_ADDR(nbr), FALSE);
    }

  we_are_through:

    if (BIT_TEST(intf->flags, OSPF_INTFF_NBR_CHANGE)) {
	(*(if_trans[NBR_CHANGE][intf->state])) (intf);
    }

    /* build net and rtr lsa if necessary */
    if (BIT_TEST(intf->flags, OSPF_INTFF_BUILDNET)) {
	a->spfsched |= build_net_lsa(intf, &txq, 0);
	BIT_RESET(intf->flags, OSPF_INTFF_BUILDNET);
    }
    if (a->build_rtr) {
	a->spfsched |= build_rtr_lsa(a, &txq, 0);
	a->build_rtr = FALSE;
    }
    if (txq != LLNULL) {		/* may be locked out */
	self_orig_area_flood(a, txq, LS_RTR);
	ospf_freeq((struct Q **) &txq, OMEM_LL);
    }

    return ret;
}


/**/

void
ospf_rxpkt __PF4(ip, struct ip *,
		 o_hdr, struct OSPF_HDR *,
		 src, sockaddr_un *,
		 dst, sockaddr_un *)
{
    size_t len;
    int ret = 0;
    u_int32 *auth_key;
    u_int16 auth_type;
    struct AREA *area = AREANULL;
    flag_t spf_flag = OSPF_AREAF_SPF_RUN|OSPF_AREAF_SPF_FULL;
    sockaddr_un *router_id = sockbuild_in(0, o_hdr->rtr_id);
    struct INTF *intf;
#define	bad_packet(x)	{ ret = x; goto Return; }

    /* Log current time */
    ospf_get_sys_time();

    TRAP_REF_UPDATE;	/* Update the trap event counter */


    if (o_hdr->type == O_MON) {
	/* Monitor packet */

	intf = (struct INTF *) 0;
    } else {
	if_addr *ifap = if_withdst(src);

	/* check that IP destination is to interface or IP multicast addr */

	intf = ifap ? IF_INTF(ifap) : (struct INTF *) 0;

#ifdef	IP_MULTICAST
	if (sockaddrcmp_in(dst, ospf_addr_allspf)) {
	    /* Multicast to All SPF routers */

	    if (!intf) {
		/* Not on attached interface running OSPF */
		bad_packet(BAD_IP_DEST);
	    }

	    switch (intf->type) {
	    case POINT_TO_POINT:
	    case BROADCAST:
		break;

	    default:
		bad_packet(BAD_IP_DEST);
	    }
	} else if (sockaddrcmp_in(dst, ospf_addr_alldr)) {
	    /* Multicast to All DR routers */

	    if (!intf) {
		/* Not on attached interface running OSPF */
		bad_packet(BAD_IP_DEST);
	    }

	    switch (intf->state) {
	    case IDr:
	    case IBACKUP:
		if (intf->type == BROADCAST && intf->pri) {
		    break;
		}
		/* Fall through */

	    default:
		bad_packet(BAD_IP_DEST);
	    }
	} else 
#endif	/* IP_MULTICAST */
	{
	    /* Unicast packet */
	
	    if (intf) {
		/* Local source */

		if (!sockaddrcmp_in(ifap->ifa_addr_local, dst)) {
		    /* Not addressed directly to us */
		    bad_packet(BAD_IP_DEST);
		}
	    } else if (IAmBorderRtr) {
		/* Non-local source, could be a virtual link */

		if (!(ifap = if_withlcladdr(dst, FALSE))
		    || !(intf = IF_INTF(ifap))) {
		    /* Not addressed to us */
		    bad_packet(BAD_IP_DEST);
		}
	    } else {
		/* Non-local source, should not be a virtual link */

		bad_packet(BAD_IP_DEST);
	    }
	}

	if (sockaddrcmp_in(src, ifap->ifa_addr_local)) {
	    /* Echo of my own multicast */
	    bad_packet(MY_IP_SRC);
	}

	/* Check for a down to up transition of the interface */
	if_rtupdate(ifap);
    }

    if (IP_PROTOCOL(ip) != IPPROTO_OSPF)
	bad_packet(BAD_IP_PROTOID);

    len = ntohs(o_hdr->length);
    if (len < OSPF_HDR_SIZE)
	bad_packet(PKT_TOO_SMALL);
    if (len > IP_LENGTH(ip))
	bad_packet(BAD_OSPF_LENGTH);

    /* OSPF hdr */
    if (o_hdr->version != OSPF_VERSION) {
	bad_packet(BAD_OSPF_VERSION);
    }

    /* Checksum the packet, exclusive of the authentication field */
    {
	struct iovec v[2], *vp = v;

	/* First the packet header not including the authentication */
	vp->iov_base = (caddr_t) o_hdr;
	vp->iov_len = OSPF_HDR_SIZE - OSPF_AUTH_SIZE;
	vp++;

	if (len > OSPF_HDR_SIZE) {
	    /* Then the rest of the packet */

	    vp->iov_base = (caddr_t) o_hdr->Auth + OSPF_AUTH_SIZE;
	    vp->iov_len = len - OSPF_HDR_SIZE;
	    vp++;
	} else {
	    /* Just a header */

	    vp->iov_base = (caddr_t) 0;
	    vp->iov_base = 0;
	}

	if (inet_cksumv(v, vp - v, len - OSPF_AUTH_SIZE)) {
	    bad_packet(BAD_OSPF_CHKSUM);
	}
    }

    if (o_hdr->type == O_MON) {
	auth_type = ospf.mon_authtype;
	auth_key = ospf.mon_authkey;
    } else {
	/* the area id check stuff - VIRTUAL or Standard link? */
	if (intf
	    && o_hdr->area_id == intf->area->area_id) {
	    /* NOT virtual */

	    if ((sock2ip(src) & INTF_MASK(intf)) != (INTF_ADDR(intf) & INTF_MASK(intf)))
		bad_packet(BAD_IF_AREAID);
	    area = intf->area;
	} else {
	    struct INTF *vintf;

	    /* pkt should be from virtual link */

	    if (!IAmBorderRtr)
		bad_packet(RXPACKET_TO_ABR);

	    VINTF_LIST(vintf) {
		/*
		 * if VL 1) should be from configured nbr 2) nbr is area border
		 * rtr 3) trans area is same as receiving IF's area
		 */
		if (sockaddrcmp_in(vintf->nbr.nbr_id, router_id) &&
		    o_hdr->area_id == OSPF_BACKBONE &&	/* from backbone */
		    vintf->transarea == intf->area) {
		    area = &ospf.backbone;
		    break;
		}
	    } VINTF_LIST_END(vintf) ;

	    if (!(intf = vintf)) {
		bad_packet(BAD_VL);
	    }
	}

	auth_type = area->authtype;
	auth_key = intf->authkey;
    }

    /* Check the authentication */
    if (ntohs(o_hdr->AuType) != auth_type)
	bad_packet(BAD_AUTH_TYPE);

    switch (auth_type) {
    case OSPF_AUTH_NONE:
	break;

    case OSPF_AUTH_SIMPLE:
	if (o_hdr->Auth[0] != auth_key[0] ||
	    o_hdr->Auth[1] != auth_key[1]) {
	    bad_packet(BAD_AUTH_KEY);
	}
	break;

    default:
	bad_packet(BAD_AUTH_TYPE);
    }

    /* Log the packet */
    if (o_hdr->type != O_MON &&
	BIT_TEST(ospf.trace_flags, TR_OSPF_PKT)) {
	ospf_trace(o_hdr, len, (u_int) o_hdr->type, 1, intf, src, dst);
    }

    
    /* Handle the packet */
    switch (o_hdr->type) {
    case O_MON:
	ret = ospf_rx_mon(&o_hdr->un.mon, intf, src, router_id, len);
	break;
	
    case O_HELLO:
	ret = ospf_rx_hello(&o_hdr->un.hello, intf, src, router_id, len);
	break;
	    
    case O_DB_DESCRIPT:
	ret = ospf_rx_db(&o_hdr->un.database, intf, src, router_id, len);
	break;
	
    case O_LSR:
	ret = ospf_rx_lsreq(&o_hdr->un.ls_req, intf, src, router_id, len);
	break;
	
    case O_LSU:
	BIT_RESET(spf_flag, OSPF_AREAF_SPF_FULL);
	ret = ospf_rx_lsupdate(&o_hdr->un.ls_update, intf, src, router_id, len);
	break;

    case O_ACK:
	ret = ospf_rx_lsack(&o_hdr->un.ls_ack, intf, src, router_id, len);
	break;

    default:
	bad_packet(BAD_OSPF_TYPE);
    }

    assert(ret < LASTLOG);
    
    if (ret == GOOD_RX) {
	ret = o_hdr->type;
    }

    /* Schedule an SPF run if necessary */
    if (area && area->spfsched) {

	BIT_SET(area->area_flags, spf_flag);
    }

 Return:
    ospf_log_rx(ret, src, dst);
    
    return;
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
