static char sccsid[] = "@(#)86	1.1  src/tcpip/usr/sbin/gated/ospf_choose_dr.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:49";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: MaxNbr
 *		__PF1
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
 * ospf_choose_dr.c,v 1.11 1993/03/22 02:39:57 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


#define MaxNbr(MAX,N){\
	 if ((N->pri > MAX->pri) ||\
	    ((N->pri == MAX->pri) && (ntohl(NBR_ID(N)) > ntohl(NBR_ID(MAX)))))\
			MAX = N;}

/*
 *	Choose bdr for intf
 *	- Event: nbr change or wait timer
 */
static void
bdr_bakeoff  __PF1(intf, struct INTF *)
{
    struct NBR *n = NBRNULL;
    struct NBR *BestBDr = NBRNULL;

    /* Hi is for nbrs who haven't declaired themselves bdr or dr */
    struct NBR *BestBDrHi = NBRNULL;

    /* 
     * choose bdr 
     */
    for (n = &(intf->nbr); n != NBRNULL; n = n->next) {
	/* if we can't see ourself, n isn't elig or n is dr... */
	if ((n->state < N2WAY) ||
	    (!n->pri) ||
	    (n->dr == NBR_ADDR(n)) ||
	    (intf->dr == n))
	    continue;

	if (n->bdr == NBR_ADDR(n)) {	
	    /* 
	     * nbr has declaired himself bdr 
	     */
	    if (BestBDr == NBRNULL) {
		BestBDr = n;
	    } else {
		MaxNbr(BestBDr, n);
	    }
	} else {			/* keep track of the highest priority choice */
	    /* exclude nbrs that are think they are dr */
	    if (BestBDrHi == NBRNULL) {
		BestBDrHi = n;
	    } else {
		MaxNbr(BestBDrHi, n);
	    }
	}
    }

    if (BestBDr != NBRNULL) {
	intf->bdr = BestBDr;
    } else if (BestBDrHi != NBRNULL) {
	intf->bdr = BestBDrHi;
    }

    /* 
     * set this rtr's nbr structure to correct bdr 
     */
    if (intf->bdr != NBRNULL) {
	intf->nbr.bdr = NBR_ADDR(intf->bdr);
    }
}


/*
 *	Choose dr for intf
 *	- Event: nbr change or wait timer
 *	  Return 1 if DR = BDR else return 0
 */
static void
dr_bakeoff __PF1(intf, struct INTF *)
{

    struct NBR *n = NBRNULL;
    struct NBR *BestDr = NBRNULL;

    /* choose dr */
    for (n = &(intf->nbr); n != NBRNULL; n = n->next) {
	/*
	 * can see ourself in nbrs hello && nbr
	 *  has declaired himself dr
	 */
	if ((n->state >= N2WAY) &&
	    (n->pri) &&
	    (n->dr == NBR_ADDR(n))) {
	    if (BestDr == NBRNULL) {
		BestDr = n;
	    } else {
		MaxNbr(BestDr, n);
	    }
	}
    }

    if (BestDr != NBRNULL) {
	intf->dr = BestDr;
	intf->nbr.dr = NBR_ADDR(intf->dr);
    } else if (intf->bdr != NBRNULL) {
	/* promote backup */
	intf->dr = intf->bdr;
	intf->nbr.dr = NBR_ADDR(intf->bdr);
	/* This rtr can't be DR and BDr */
	if (intf->dr == &(intf->nbr)) {
	    intf->nbr.bdr = 0;
	    intf->bdr = NBRNULL;
	}
    }
}

void
ospf_choose_dr __PF1(intf, struct INTF *)
{

    struct NBR *olddr = intf->dr;
    struct NBR *oldbdr = intf->bdr;
    struct NBR *n;

    intf->bdr = intf->dr = NBRNULL;
    bdr_bakeoff(intf);
    dr_bakeoff(intf);

    /* 
     * rerun backup if bdr or dr and this rtr is either or used to be 
     */
    if ( ((olddr != intf->dr) &&
		((olddr == &(intf->nbr)) || (intf->dr == &(intf->nbr)))) ||
         ((oldbdr != intf->bdr) &&
		((oldbdr == &(intf->nbr)) || (intf->bdr == &(intf->nbr)))) )
	bdr_bakeoff(intf);

    /* 
     * set to correct state 
     */
    if (intf->dr == &(intf->nbr))	/* I am dr */
	intf->state = IDr;
    else if (intf->bdr == &(intf->nbr))	/* I am bdr */
	intf->state = IBACKUP;
    else
	intf->state = IDrOTHER;

    if (BIT_TEST(ospf.trace_flags, TR_OSPF_TRANS)) {
	trace(TR_OSPF, 0, "OSPF DR ELECTION Interface %A:  DR: %A  BDR: %A",
	      intf->ifap->ifa_addr,
	      intf->dr ? intf->dr->nbr_id : (sockaddr_un *) 0,
	      intf->bdr ? intf->bdr->nbr_id : (sockaddr_un *) 0);
	trace(TR_OSPF, 0, NULL);
    }

    /* handle adjacency madness */
    if (intf->state == IDr || intf->state == IBACKUP) {
	/* 
	 * if we weren't dr or bdr do the adj_ok thing to nbrs 
	 */
	if ((olddr != &(intf->nbr) && oldbdr != &(intf->nbr))) {
	    for (n = intf->nbr.next; n != NBRNULL; n = n->next) {
		if (n->state == N2WAY)
		    (*(nbr_trans[ADJ_OK][n->state])) (intf, n);
		else if (intf->type == NONBROADCAST && n->state < N2WAY)
		    (*(nbr_trans[START][n->state])) (intf, n);
	    }
	}
	/* 
	 * flag build_net 
   	 */
	if (intf->state == IDr)
	    BIT_SET(intf->flags, OSPF_INTFF_BUILDNET);
    } else {
	if (olddr == &(intf->nbr) || oldbdr == &(intf->nbr)) {
	    /* 
	     * usedta be but no more so rst adj and adj with new dr 
	     */
	    for (n = intf->nbr.next; n != NBRNULL; n = n->next)
		if ((n != intf->dr && n != intf->bdr) &&
		    (n->state > N2WAY))
		    (*(nbr_trans[RST_ADJ][n->state])) (intf, n);
	    if (intf->dr != NBRNULL && intf->dr->state <= N2WAY)
		(*(nbr_trans[ADJ_OK][intf->dr->state])) (intf, intf->dr);
	    if (intf->bdr != NBRNULL && intf->bdr->state <= N2WAY)
		(*(nbr_trans[ADJ_OK][intf->bdr->state])) (intf, intf->bdr);
	} else {
	    /* 
	     * we weren't dr or bdr but the current one has changed 
	     */
	    if (intf->dr != NBRNULL &&
		intf->dr != olddr &&
		intf->dr != oldbdr)
		/* establish adj with new dr */
		(*(nbr_trans[ADJ_OK][intf->dr->state])) (intf, intf->dr);
	    if (intf->bdr != NBRNULL &&
		intf->bdr != olddr &&
		intf->dr != oldbdr &&
		intf->dr != intf->bdr)
		/* establish adj with new bdr */
		(*(nbr_trans[ADJ_OK][intf->bdr->state])) (intf, intf->bdr);
	    /* 
	     * reset the old ones if necessary 
	     */
	    if (olddr != NBRNULL &&
		(olddr != intf->dr && olddr != intf->bdr) &&
		(olddr->state > N2WAY))
		(*(nbr_trans[RST_ADJ][olddr->state])) (intf, olddr);
	    if (oldbdr != NBRNULL &&
		(oldbdr != intf->dr && oldbdr != intf->bdr) &&
		(oldbdr->state > N2WAY))
		(*(nbr_trans[RST_ADJ][oldbdr->state])) (intf, oldbdr);
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
