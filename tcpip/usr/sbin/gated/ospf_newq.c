static char sccsid[] = "@(#)99	1.1  src/tcpip/usr/sbin/gated/ospf_newq.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:21";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF2
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
 * ospf_newq.c,v 1.12 1993/04/09 23:50:22 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"

/*
 * Event: new lsa generated and Area's LINK_LIST has pointer to
 *  all neighbors: link_ptr in nbr has to be removed
 *	- search for link pointer and remove it from a neighbor's list
 *	- remove from nbr
 */
int
rem_db_ptr __PF2(nbr, struct NBR *,
		 db, struct LSDB *)
{
    register struct LSDB_LIST *lp = nbr->retrans[XHASH_QUEUE(db)].ptr[NEXT];

    while (lp && lp->lsdb < db) {
	lp = lp->ptr[NEXT];
    }

    if (lp && lp->lsdb == db) {
	DEL_Q(lp, TRUE, OMEM_LSDB);
	nbr->rtcnt--;
	return TRUE;
    }

    return FALSE;
}


/*
 * Event: ack has been received and neighbor has pointer to LINK_LIST
 *	  or nbr's state has dropped and are freeing nbr's lsdb list
 *	- search for nbr pointer in lsdb's nbr list and remove it from list
 */
int
rem_nbr_ptr __PF2(db, struct LSDB *,
		  nbr, struct NBR *)
{
    struct NBR_LIST *nl;

    assert(GOT_GUTS(db));

    for (nl = DB_RETRANS(db); nl != NLNULL; nl = nl->ptr[NEXT]) {
	if (nl->nbr == nbr) {
	    REM_Q((DB_RETRANS(db)), nl, TRUE, OMEM_NL);
	    return TRUE;
	}
    }
    return FALSE;
}

void
ospf_freeq __PF2(qhp, struct Q **,
		 type, int)
{
    struct Q *q;

    for (q = *qhp; *qhp != QNULL;) {
	*qhp = q->ptr[NEXT];
	FREE(((char *) q), type);
	q = *qhp;
    }
}

/*
 *  add_nbr_retrans - add an lsdb ptr to the nbr's retrans list
 */
void
add_nbr_retrans __PF2(nbr, struct NBR *,
		      db, struct LSDB *)
{
    struct LSDB_LIST *hp = &nbr->retrans[XHASH_QUEUE(db)];
    struct LSDB_LIST *ll;
    register struct LSDB_LIST *lp = hp;

    LL_ALLOC(ll);
    ll->lsdb = db;
    nbr->rtcnt++;

    if (!lp) {
	/* First entry on list */

	ADD_Q(hp, ll);
    } else {
	/* Insert in order */

	while (lp->ptr[NEXT] && lp->ptr[NEXT]->lsdb < db) {
	    lp = lp->ptr[NEXT] ;
	}

	ADD_Q(lp, ll);
    }
}

/*
 *  add_db_retrans - add a nbr ptr to the lsdb's retrans list
 */
void
add_db_retrans __PF2(db, struct LSDB *,
		     nbr, struct NBR *)
{
    struct NBR_LIST *nl;

    NL_ALLOC(nl);
    nl->nbr = nbr;
    EN_Q((DB_RETRANS(db)), nl);
}


/*
 * rem_db_retrans - clear lsdb's retrans list and free lsdb prtr from neighbors
 */
void
rem_db_retrans __PF1(db, struct LSDB *)
{
    struct NBR_LIST *nl, *next_nl;

    /* remove from all nbrs' lists */
    for (nl = DB_RETRANS(db); nl != NLNULL; nl = next_nl) {
	next_nl = nl->ptr[NEXT];
	/* remove from nbr */
	rem_db_ptr(nl->nbr, db);
	/* remove from lsdb's nbr list */
	REM_Q((DB_RETRANS(db)), nl, TRUE, OMEM_NL);
    }
    DB_RETRANS(db) = NLNULL;
}

/*
 * rem_nbr_retrans - remove all lsdb ptrs from nbr retrans list
 */
void
rem_nbr_retrans __PF1(nbr, struct NBR *)
{
    struct LSDB_LIST *ll;
    register struct LSDB_LIST *hp = (struct LSDB_LIST *) nbr->retrans;
    register struct LSDB_LIST *limit = hp + OSPF_HASH_QUEUE;

    /* remove from all nbrs' lists */
    do {
	while (ll = hp->ptr[NEXT]) {
	    assert(GOT_GUTS(ll->lsdb));

	    /* remove from lsdb */
	    rem_nbr_ptr(ll->lsdb, nbr);

	    /* remove from nbr's retrans list */
	    DEL_Q(ll, TRUE, LSDB_LIST);
	    nbr->rtcnt--;
	}
    } while (++hp < limit) ;

    assert(!nbr->rtcnt);
}


/*
 * Free nbr's db summary list
 */
void
freeDbSum __PF1(nbr, struct NBR *)
{
    struct LSDB_SUM *nextd;

    for (nextd = nbr->dbsum; nextd != LSDB_SUM_NULL;) {
	nbr->dbsum = nextd->next;
	DB_FREE_PKT(nextd);
	FREE(nextd, OMEM_DBSUM);
	nextd = nbr->dbsum;
    }
    nbr->dbsum = LSDB_SUM_NULL;
    nbr->dbcnt = 0;
}

/*
 * Free nbr's ls request list
 */
void
freeLsReq __PF1(nbr, struct NBR *)
{
    int type;

    for (type = LS_RTR; type <= LS_ASE; type++) {
	if (nbr->ls_req[type] != LS_REQ_NULL)
	    ospf_freeq((struct Q **)&(nbr->ls_req[type]), OMEM_LS_REQ);
	nbr->ls_req[type] = LS_REQ_NULL;
    }
    nbr->reqcnt = 0;
}

/*
 * Free list of acks to be sent to this nbr
 */
void
freeAckList __PF1(intf, struct INTF *)
{
    struct LS_HDRQ *ack = intf->acks.ptr[NEXT];

    intf->acks.ptr[NEXT] = (struct LS_HDRQ *) 0;

    while (ack) {
	struct LS_HDRQ *next = ack->ptr[NEXT];

	FREE(ack, OMEM_ACKLST);

	ack = next;
    }
}


/*
 * Free list of NetRanges for area
 */
void
ospf_freeRangeList __PF1(area, struct AREA *)
{
    struct NET_RANGE *nr = area->nr.ptr[NEXT];
 
    area->nr.ptr[NEXT] = (struct NET_RANGE *) 0;
 
    while (nr) {
 	struct NET_RANGE *next = nr->ptr[NEXT];
 
 	FREE(nr, OMEM_NET);
 	
 	area->nrcnt--;
 	nr = next;
     }
}


#ifdef	notdef
/*
 * add_ack: add acks to link list - used to call send_ack
 */
void
add_ack __PF2(qhp, struct LS_HDRQ *,
	      db, struct LSDB *)
{
    struct LS_HDRQ *ack;

    HDRQ_ALLOC(ack);
    ack->ls_hdr = DB_LS_HDR(db);	/* struct copy */
    ADD_Q(qhp, ack);

    if (!TASK_TIMER(ospf.task, SYS_TIMER_ACK)->timer_interval) {
	/* Timer not active, start it */

	timer_set(TASK_TIMER(ospf.task, SYS_TIMER_ACK), OSPF_T_ACK, (time_t) 0);
    }
}
#endif	/* notdef */

/*
 * Add neighbor to NBMA or BROADCAST intf in sorted order
 */
void
nbr_enq __PF2(intf, struct INTF *,
	      nbr, struct NBR *)
{
    struct NBR *last_nbr;

    for (last_nbr = &(intf->nbr); ; last_nbr = last_nbr->next) {
        if (last_nbr->next == NBRNULL ||
	    ntohl(NBR_ADDR(last_nbr->next)) > ntohl(NBR_ADDR(nbr))) {
	    nbr->next = last_nbr->next;
	    last_nbr->next = nbr;
	    break;
        }
    }
    ospf.nbrcnt++;
#ifdef	notdef
    ospf.nbr_sb_not_valid = TRUE;
#endif	/* notdef */
}


/*
 * Add net range to area in sorted order for ease of MIB ness
 */
void
range_enq __PF2(area, struct AREA *,
		range, struct NET_RANGE *)
{
    struct NET_RANGE *nr = &area->nr;

    do {
        if (nr->ptr[NEXT] == NRNULL ||
	    ntohl(nr->ptr[NEXT]->net) > ntohl(range->net)) {
	    ADD_Q(nr, range);
	    return;
        }
    } while (nr = nr->ptr[NEXT]) ;
}

/*
 * Add host to area in sorted order for ease of MIB ness
 */
void
host_enq __PF2(area, struct AREA *,
	       host, struct OSPF_HOSTS *)
{
    struct OSPF_HOSTS *h;

    for(h = &(area->hosts); ; h = h->ptr[NEXT]) {
        if (h->ptr[NEXT] == HOSTSNULL ||
	    ntohl(h->ptr[NEXT]->if_addr) > ntohl(host->if_addr)) {
	    ADD_Q(h, host);
	    return;
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
