static char sccsid[] = "@(#)13	1.1  src/tcpip/usr/sbin/gated/ospf_txpkt.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:00";
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
 * ospf_txpkt.c,v 1.16.2.1 1993/08/27 22:28:58 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


void
send_hello __PF3(intf, struct INTF *,
		 nbrptr, struct NBR *,
		 shutdown, int) 
{
    struct OSPF_HDR *pkt = task_get_send_buffer(struct OSPF_HDR *);
    struct HELLO_HDR *hello;
    u_int32 to = 0;
    size_t len = HELLO_HDR_SIZE + OSPF_HDR_SIZE;
    /* if given a pointer to a neighbor, just send to the neighbor */
    struct NBR *nbr;
    struct RHF *rhf;
    struct AREA *area = intf->area;

    hello = &(pkt->un.hello);
    hello->netmask = INTF_MASK(intf);
    hello->DeadInt = htonl(intf->dead_timer);
    hello->HelloInt = htons(intf->hello_timer);
    hello->rtr_priority = intf->pri;	/* if priority	 */
    hello->options = BIT_TEST(area->area_flags, OSPF_AREAF_STUB) ? 0 : OPT_E_bit;

    /* build rtrs heard from stuff  and dr and bdr only if not shutting down */
    if (!shutdown) {
	hello->dr = (intf->dr != NBRNULL) ? NBR_ADDR(intf->dr) : 0;
	hello->bdr = (intf->bdr != NBRNULL) ? NBR_ADDR(intf->bdr) : 0;
	rhf = &(hello->rhf);
	if (intf->type > NONBROADCAST) {	/* VIRTUAL or PTP */
	    if (intf->nbr.state > NATTEMPT) {
		rhf->rtr = NBR_ID(&intf->nbr);
		len += RHF_SIZE;
	    }
	} else			/* NBMA or BROADCAST */
	    for (nbr = intf->nbr.next; nbr != NBRNULL; nbr = nbr->next) {
		if (nbr->state > NATTEMPT) {
		    rhf->rtr = NBR_ID(nbr);
		    len += RHF_SIZE;
		    rhf++;
		}
	    }
    }

    switch (intf->type) {
#ifdef	IP_MULTICAST
    case BROADCAST:
	to = sock2ip(ospf_addr_allspf);
	break;
#endif	/* IP_MULTICAST */

    case POINT_TO_POINT:
#ifdef	IP_MULTICAST
	if (BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) {
	    to = sock2ip(ospf_addr_allspf);
	    break;
	}
#endif	/* IP_MULTICAST */
	/* Fall Through */

    case VIRTUAL_LINK:
	to = NBR_ADDR(&intf->nbr);
	break;

    case NONBROADCAST:
	/* check to see if this was a poll timer or response */
	if (nbrptr)
	    to = NBR_ADDR(nbrptr);
	else
	    switch (intf->state) {
	    case IWAITING:	/* send to all eligible */
	    case IDrOTHER:	/* also Dr and backup cases */
		if (intf->pri)	/* if elig */
		    to = ALL_ELIG_NBRS;
		else
		    to = DR_and_BDR;
		break;
	    case IDr:
	    case IBACKUP:
		/* if (!nbrptr) - Send to all up nbrs */
		to = ALL_UP_NBRS;
		break;
	    }
	break;

    default:
	assert(FALSE);
	break;
    }
    ospf_txpkt(pkt, intf, O_HELLO, len, to, NOT_RETRANS);
}


void
send_exstart __PF3(intf, struct INTF *,
		   nbr, struct NBR *,
		   rt, int)	/* retrans flag */
{
    struct DB_HDR *dbh;
    struct OSPF_HDR *pkt = task_get_send_buffer(struct OSPF_HDR *);
    struct AREA *area = intf->area;

    dbh = (struct DB_HDR *) ((long) pkt + OSPF_HDR_SIZE);
    /* set bits */
    dbh->phill2 = 0;
    dbh->I_M_MS = nbr->I_M_MS;
    dbh->options = BIT_TEST(area->area_flags, OSPF_AREAF_STUB) ? 0 : OPT_E_bit;
    dbh->seq = htonl(nbr->seq);
    ospf_txpkt(pkt,
	       intf,
	       O_DB_DESCRIPT,
	       (size_t) (DB_HDR_SIZE + OSPF_HDR_SIZE),
#ifdef	IP_MULTICAST
	       (intf->type == POINT_TO_POINT && BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) ?
	       sock2ip(ospf_addr_allspf) :
#endif	/* IP_MULTICAST */
	       NBR_ADDR(nbr),
	       rt);
}


/*
 *	Send first on nbr list - don't free until ack has been received
 */
void
send_dbsum __PF3(intf, struct INTF *,
		 nbr, struct NBR *,
		 rt, int)	/* retrans flag */
{
    size_t len;
    struct DB_HDR *dbh;
    struct LSDB_SUM *ds;
    struct OSPF_HDR *pkt;
    struct AREA *area = intf->area;

    if (nbr->dbsum == LSDB_SUM_NULL) {
	ds = dbsum_alloc(OSPF_HDR_SIZE + DB_HDR_SIZE);
	if (ds == LSDB_SUM_NULL)
	    return;
	ds->len = OSPF_HDR_SIZE + DB_HDR_SIZE;
	nbr->dbsum = ds;
    } else {
	ds = nbr->dbsum;
    }

    pkt = (struct OSPF_HDR *) ds->dbpkt;
    dbh = (struct DB_HDR *) & (pkt->un.database);
    len = ds->len;
    dbh->phill2 = 0;
    dbh->I_M_MS |= nbr->I_M_MS;
    dbh->options = BIT_TEST(area->area_flags, OSPF_AREAF_STUB) ? 0 : OPT_E_bit;
    dbh->seq = htonl(nbr->seq);

    ospf_txpkt(pkt,
	       intf,
	       O_DB_DESCRIPT,
	       len,
#ifdef	IP_MULTICAST
	       (intf->type == POINT_TO_POINT && BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) ?
	       sock2ip(ospf_addr_allspf) :
#endif	/* IP_MULTICAST */
	       NBR_ADDR(nbr),
	       rt);
}


/*
 *  send_req - build and send a db req packet
 *	     - called by exstart and retrans time; in both cases
 *		we'll just send what hasn't been taken off the req list
 */
void
send_req __PF3(intf, struct INTF *,
	       nbr, struct NBR *,
	       rt, int)	/* retrans flag */
{
    struct LS_REQ *r;
    struct OSPF_HDR *pkt = task_get_send_buffer(struct OSPF_HDR *);
    struct LS_REQ_HDR *req_hdr = &pkt->un.ls_req;
    struct LS_REQ_PIECE *req = &req_hdr->req;
    size_t len = OSPF_HDR_SIZE;	/* max len of pkt is INTF_MTU(intf) */
    int type;

    if (NO_REQ(nbr)) {
	return;
    }

    for (type = LS_RTR; type <= LS_ASE; type++) {
	for (r = nbr->ls_req[type]; r != LS_REQ_NULL; r = r->ptr[NEXT]) {
	    req->phill1 = req->phill2 = 0;
	    req->ls_type = type;
	    req->ls_id = r->ls_id;
	    req->adv_rtr = r->adv_rtr;
	    len += LS_REQ_PIECE_SIZE;
	    if (len + LS_REQ_PIECE_SIZE > INTF_MTU(intf)) {
		goto send;
	    }
	    req++;
	}
    }

 send:
    ospf_txpkt(pkt,
	       intf,
	       O_LSR,
	       len,
#ifdef	IP_MULTICAST
	       (intf->type == POINT_TO_POINT && BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) ?
	       sock2ip(ospf_addr_allspf) :
#endif	/* IP_MULTICAST */
	       NBR_ADDR(nbr),
	       rt);
}


/*
 *  send_ack - build an ack packet from ack list, pass it to send ack
 */
int
send_ack __PF3(intf, struct INTF *,
	       nbr, struct NBR *,
	       qhp, struct LS_HDRQ *)
{
    struct OSPF_HDR *pkt = task_get_send_buffer(struct OSPF_HDR *);
    struct LS_HDRQ *al = qhp->ptr[NEXT];
    struct LS_ACK_HDR *ahdr;
    struct LS_HDR *ap;
    u_int32 to = 0;
    size_t len = OSPF_HDR_SIZE;

    /* First get 'to' handled */
    if (nbr) {
	to = 
#ifdef	IP_MULTICAST
	    (intf->type == POINT_TO_POINT && BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) ?
		sock2ip(ospf_addr_allspf) :
#endif	/* IP_MULTICAST */
		    NBR_ADDR(nbr);	/* direct ack */
    } else {
	switch (intf->type) {
#ifdef	IP_MULTICAST
	case BROADCAST:
	    switch (intf->state) {
	    case IDr:
	    case IBACKUP:
		to = sock2ip(ospf_addr_allspf);
		break;

	    default:
		to = sock2ip(ospf_addr_alldr);
		break;
	    }
	    break;
#endif	/* IP_MULTICAST */

	case NONBROADCAST:
	    to = ALL_EXCH_NBRS;
	    break;

	case POINT_TO_POINT:
#ifdef	IP_MULTICAST
	    if (BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) {
		to = sock2ip(ospf_addr_allspf);
		break;
	    }
#endif	/* IP_MULTICAST */
	    /* Fall through */

	case VIRTUAL_LINK:
	    to = NBR_ADDR(&intf->nbr);
	    break;

	default:
	    assert(FALSE);
	    break;
	}
    }


    ahdr = (struct LS_ACK_HDR *) & pkt->un.ls_ack;
    ap = &(ahdr->ack_piece);

    while (al) {
	if (len + ACK_PIECE_SIZE >= INTF_MTU(intf)) {
	    ospf_txpkt(pkt, intf, O_ACK, len, to, NOT_RETRANS);

	    if (!nbr && !ACK_QUEUE_FULL(intf)) {
		/* Since we had a full packet we know that we were not being called */
		/* by the Ack timer.  There are not enough acks left to fill a packet */
		/* so return with an indication that the timer should be started */

		return TRUE;
	    }

	    len = OSPF_HDR_SIZE;
	    ahdr = (struct LS_ACK_HDR *) & pkt->un.ls_ack;
	    ap = &(ahdr->ack_piece);
	}

	*ap = al->ls_hdr;	/* struct copy */
	len += ACK_PIECE_SIZE;
	ap++;

	qhp->ptr[NEXT] = al->ptr[NEXT];
	FREE(al, OMEM_ACKLST);
	if (!nbr) {
	    intf->ack_cnt--;
	    /* XXX - stop if not a full packet */
	}
	al = qhp->ptr[NEXT];
    }

    if (len > OSPF_HDR_SIZE) {
	ospf_txpkt(pkt, intf, O_ACK, len, to, NOT_RETRANS);
    }

    return FALSE;
}


/*
 * send_lsu - go through db_list and send ls update pkt
 */
int
send_lsu __PF5(db_list, struct LSDB_LIST *,
	       hash, int,
	       nbr, struct NBR *,
	       intf, struct INTF *,
	       rt, int)		/* retrans flag */
{
    struct OSPF_HDR *pkt;
    struct LSDB_LIST *dbl;
    union ADV *adv;
    u_int32 to = 0;
    u_int32 *adv_cnt;
    u_int16 adv_age;
    size_t len = OSPF_HDR_SIZE + LS_UPDATE_HDR_SIZE;
    struct LSDB_LIST *ll;
    struct LSDB_LIST lsdb_list;

    if (intf->state == IDOWN)
	return 0;

    NEW_LSU(pkt, adv_cnt, adv);
    if (nbr != NBRNULL) {
	to = NBR_ADDR(nbr);
    } else {
	switch (intf->type) {
#ifdef	IP_MULTICAST
	case BROADCAST:
	    switch (intf->state) {
	    case IDr:
	    case IBACKUP:
		to = sock2ip(ospf_addr_allspf);
		break;

	    default:
		to = sock2ip(ospf_addr_alldr);
		break;
	    }
	    break;
#endif	/* IP_MULTICAST */

	case POINT_TO_POINT:
#ifdef	IP_MULTICAST
	    if (BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) {
		to = sock2ip(ospf_addr_allspf);
		break;
	    }
#endif	/* IP_MULTICAST */
	    /* Fall through */

	case VIRTUAL_LINK:
	    to = NBR_ADDR(&intf->nbr);
	    break;

	case NONBROADCAST:
	    to = ALL_EXCH_NBRS;
	    break;

	default:
	    assert(FALSE);
	    break;
	}
    }

    if (hash > 1) {
	ll = db_list;
    } else {
	lsdb_list.ptr[NEXT] = db_list;
	ll = &lsdb_list;
    }

    while (hash--) {
	/* For each hash bucket (only one unless a retrans) */

	for (dbl = ll[hash].ptr[NEXT]; dbl != LLNULL; dbl = dbl->ptr[NEXT]) {
	    assert(dbl->lsdb);
	    if (rt) {
		if ((time_sec - DB_TIME(dbl->lsdb)) < intf->retrans_timer)
		    continue;
	    } else {		/* !rt */
		if (dbl->flood != FLOOD || (NO_GUTS(dbl->lsdb))) {
		    continue;
		}
	    }
	    if (len + ntohs(LS_LEN(dbl->lsdb)) >= INTF_MTU(intf)) {
		if (*adv_cnt == 0) {
		    /* we've got a big'un */
		    
		    NEW_LSU(pkt, adv_cnt, adv);
		    goto big_un;
		}
		GHTONL(*adv_cnt);

		ospf_txpkt(pkt, intf, O_LSU, len, to, rt);

		/* if rt (retrans flag) is true just send one */
		if (rt)
		    return 0;

		NEW_LSU(pkt, adv_cnt, adv);
		len = OSPF_HDR_SIZE + LS_UPDATE_HDR_SIZE;
	    }

	big_un:
	    ADV_COPY(DB_RTR(dbl->lsdb), adv, ntohs(LS_LEN(dbl->lsdb)));
	    adv_age = ((ADV_AGE(dbl->lsdb)) + intf->transdly);
	    adv->rtr.ls_hdr.ls_age = (adv_age > MaxAge) ? htons(MaxAge) : htons(adv_age);
	    adv = (union ADV *) ((long) adv + ntohs(LS_LEN(dbl->lsdb)));
	    len += ntohs(LS_LEN(dbl->lsdb));
	    (*adv_cnt)++;
	}
    }

    if (*adv_cnt) {		/* any left to send? */
	GHTONL(*adv_cnt);
	ospf_txpkt(pkt, intf, O_LSU, len, to, rt);
    }
    return 0;
}


/*
 * Send ase pkt to all nbrs
 */
static void
ase_blast __PF3(intf, struct INTF *,
		start, struct LSDB *,
		end, struct LSDB *)
{
    u_int32 to = (u_int32) 0;
    struct OSPF_HDR *pkt;
    struct LSDB *db;
    union ADV *adv;
    u_int32 *adv_cnt;
    size_t len = OSPF_HDR_SIZE + 4;	/* 4 for adv_cnt */

    NEW_LSU(pkt, adv_cnt, adv);
    switch (intf->type) {
#ifdef	IP_MULTICAST
    case BROADCAST:
	switch (intf->state) {
	case IDr:
	case IBACKUP:
	    to = sock2ip(ospf_addr_allspf);
	    break;

	default:
	    to = sock2ip(ospf_addr_alldr);
	    break;
	}
	break;
#endif	/* IP_MULTICAST */

    case POINT_TO_POINT:
#ifdef IP_MULTICAST
	if (BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) {
	    to = sock2ip(ospf_addr_allspf);
	    break;
	}
#endif	/* IP_MULTICAST */
	to = NBR_ADDR(&intf->nbr);
	break;

    case NONBROADCAST:
	to = ALL_EXCH_NBRS;
	break;

    default:
	assert(FALSE);
	break;
    }

    for (db = start; db != LSDBNULL; db = DB_PTR(db)[NEXT]) {
	if (time_sec != DB_TIME(db)) {
	    if (db == end) break;
	    continue;
	}
	if (len + ntohs(LS_LEN(db)) >= INTF_MTU(intf)) {
	    GNTOHL(*adv_cnt);
	    ospf_txpkt(pkt, intf, O_LSU, len, to, NOT_RETRANS);
	    len = OSPF_HDR_SIZE + 4;	/* 4 for adv_cnt */
	    NEW_LSU(pkt, adv_cnt, adv);
	}
	ADV_COPY(DB_ASE(db), adv, ntohs(LS_LEN(db)));
	adv->ase.ls_hdr.ls_age = htons(intf->transdly);
	adv = (union ADV *) ((long) adv + ntohs(LS_LEN(db)));
	len += ntohs(LS_LEN(db));
	(*adv_cnt)++;
	if (db == end)
	    break;
    }

    if (*adv_cnt) {			/* any left to send? */
	GNTOHL(*adv_cnt);
	ospf_txpkt(pkt, intf, O_LSU, len, to, NOT_RETRANS);
    }
}



/*
 * send_ase - go through my_ase_list and send ase ls update pkt
 *    	    - flood everything between start and end
 */
void
send_ase __PF2(start, struct LSDB *,
	       end, struct LSDB *)
{
    struct AREA *a;
    struct INTF *intf;
    struct NBR *n;
    struct LSDB *db;

    if (DB_PTR(&ospf.my_ase_list)[NEXT] == LSDBNULL)
	return;

    /* put everything on retrans lists */
    AREA_LIST(a) {
	if (BIT_TEST(a->area_flags, OSPF_AREAF_STUB)) {
	    continue;
	}
	INTF_LIST(intf, a) {
	    if ((intf->state < IWAITING) || (!intf->nbrEcnt))
		continue;
	    NBRS_LIST(n, intf) {
		if (n->state >= NEXCHANGE) {
		    for (db = start; 
			 db != LSDBNULL;
			 db = DB_PTR(db)[NEXT]) {
			if (time_sec == DB_TIME(db)) {
			    add_nbr_retrans(n, db);
			    add_db_retrans(db, n);
			}
			if (db == end)
	    		    break;
		    }
		}
	    } NBRS_LIST_END(n, intf) ;
	    ase_blast(intf, start, end);
	} INTF_LIST_END(intf, a) ;
    } AREA_LIST_END(a) ;
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
