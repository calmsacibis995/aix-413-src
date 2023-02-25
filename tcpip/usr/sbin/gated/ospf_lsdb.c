static char sccsid[] = "@(#)95	1.1  src/tcpip/usr/sbin/gated/ospf_lsdb.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:11";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
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
 * ospf_lsdb.c,v 1.14 1993/03/22 02:40:10 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"

/*
 * 		Link State Db stuff
 */


/*
 * addLSA 		Add lsa to the linked list
 */
int
addLSA __PF6(db, struct LSDB **,
	     area, struct AREA *,
	     key0, u_int32,
	     key1, u_int32,
	     data, void_t,
	     type, u_int)
{
    struct LSDB *e, *hp;
    int hash = XHASH(key0, key0);

    assert(type <= LS_ASE);

    /* First key */
    for (hp = (e = &(area->htbl[type][hash]));
	 DB_NEXT(e) && (key0 < DB_NEXT(e)->key[0]);
	 e = DB_NEXT(e));

    if (!DB_NEXT(e) || (DB_NEXT(e)->key[0] != key0))
	goto install;

    /* Ls id is all that is needed for LS_NET && LS_RTR */
    if (type == LS_NET || type == LS_RTR) {
	*db = DB_NEXT(e);
	if (NO_GUTS(*db))
	    goto install2;
	return TRUE;
    }
    /* handle second key */
    if (DB_NEXT(e)->key[1] == key1) {	/* may have found it */
	/* had it at one time? */
	*db = DB_NEXT(e);
	if (NO_GUTS(*db))
	    goto install2;
	return TRUE;
    }
    /* install *db as e->lsdbnext */
    if (!DB_NEXT(e) || key1 > DB_NEXT(e)->key[1])
	goto install;

    /* else e->lsdbnext->key[1] > key1 so find where to install */
    for (; DB_NEXT(e) &&
	 key0 == DB_NEXT(e)->key[0] &&
	 key1 < DB_NEXT(e)->key[1];
	 e = DB_NEXT(e));

    if (DB_NEXT(e) &&
	(DB_NEXT(e)->key[1] == key1) &&
	(DB_NEXT(e)->key[0] == key0)) {
	*db = DB_NEXT(e);
	if (NO_GUTS(*db))
	    goto install2;
	return TRUE;
    }

  install:

    DB_ALLOC((*db));
    if (!(*db))
	return FALSE;
    DB_COUNT(hp)++;
    DB_NEXT(*db) = DB_NEXT(e);
    DB_NEXT(e) = *db;

  /* 
   * had a previously used one around 
   */
  install2:				
    (*db)->key[0] = key0;
    (*db)->key[1] = key1;
    DBGUTS_ALLOC(*db);
    if (NO_GUTS(*db)) {
	(*db) = LSDBNULL;
	return FALSE;
    }
    DB_RTR(*db) = (struct RTR_LA_HDR *) data;
    DB_FREEME(*db) = FALSE;
    DB_DIST(*db) = (type < LS_SUM_NET) ? RTRLSInfinity : SUMLSInfinity;
    DB_MYHASH(*db) = hash;
    DB_MY_AREA(*db) = area;

    switch(type) {
    case LS_STUB:
	assert(FALSE);
	break;

    case LS_RTR:
    case LS_NET:
    case LS_SUM_NET:
    case LS_SUM_ASB:
	ospf.db_cnt++;
	area->db_cnts[type]++;
	area->db_int_cnt++;
	break;

    case LS_ASE:
	ospf.db_cnt++;
	ospf.db_ase_cnt++;
	break;
    }

    /* 
     * successful add, not found 
     */
    return FALSE;
}

/*
 * findLSA 		Add sum lsa or ase lsa to the linked list
 */
struct LSDB *
findLSA __PF4(hp, struct LSDB *,
	      key0, u_int32,
	      key1, u_int32,
	      type, u_int)
{
    struct LSDB *e = hp;

    assert(type <= LS_ASE);
    /* 
     *  First key 
     */
    for (; (DB_NEXT(e) != LSDBNULL) && (key0 < DB_NEXT(e)->key[0]);
	 e = DB_NEXT(e)) ;
    if ((DB_NEXT(e) == LSDBNULL) ||
	(DB_NEXT(e)->key[0] != key0))
	return LSDBNULL;

    /* 
     * LS_NET and LS_RTR just use ls id 
     */
    if (type == LS_NET || type == LS_RTR) {
	if (GOT_GUTS(DB_NEXT(e)))
	    return DB_NEXT(e);
	else
	    return LSDBNULL;
    }

    /* 
     * handle second key 
     */
    if (key1 > DB_NEXT(e)->key[1])
	return LSDBNULL;

    if (DB_NEXT(e)->key[1] == key1) {
	if (GOT_GUTS(DB_NEXT(e)))
	    return DB_NEXT(e);
	else
	    return LSDBNULL;
    }

    /* 
     * else e->lsdbnext->key[1] > key1 so contunue to search 
     */
    for (; (DB_NEXT(e) != LSDBNULL) &&
	 (key0 == DB_NEXT(e)->key[0]) &&
	 (key1 < DB_NEXT(e)->key[1]);
	 e = DB_NEXT(e)) ;

    if (DB_NEXT(e) != LSDBNULL &&
	(DB_NEXT(e)->key[1] == key1) &&
	(DB_NEXT(e)->key[0] == key0) &&
	(GOT_GUTS(DB_NEXT(e))))
	return DB_NEXT(e);

    return LSDBNULL;
}


/*
 *	Add a stub network lsa
 */
int
ospf_add_stub_lsa __PF5(db, struct LSDB **,
			area, struct AREA *,
			net, u_int32,
			advrtr, u_int32,
			mask, u_int32)
{
    struct LSDB *e, *hp;
    int hash = XHASH(advrtr, advrtr);

    /* Stubs are sorted only by advertizing router */
    for (hp = (e = &(area->htbl[LS_STUB][hash]));
	 DB_NEXT(e);
	 e = DB_NEXT(e)) {

	if (advrtr > DB_NEXT(e)->key[0]) {
	    /* Insert here */

	    break;
	} else if (advrtr == DB_NEXT(e)->key[0]
		   && net == DB_NEXT(e)->key[1]) {
	    /* Getting warmer */

	    if (NO_GUTS(DB_NEXT(e))) {
		/* An empty entry, reuse */

		*db = DB_NEXT(e);
		goto install2;
	    }

	    if (mask == DB_MASK(DB_NEXT(e))) {
		/* Found it */

		*db = DB_NEXT(e);
		return TRUE;
	    }
	}
    }

    DB_ALLOC((*db));
    if (!(*db)) {
	return FALSE;
    }
    DB_COUNT(hp)++;
    DB_NEXT(*db) = DB_NEXT(e);
    DB_NEXT(e) = *db;

  /* 
   * had a previously used one around 
   */
  install2:				
    (*db)->key[0] = advrtr;
    (*db)->key[1] = net;
    DBGUTS_ALLOC(*db);
    if (NO_GUTS(*db)) {
	*db = LSDBNULL;
	return FALSE;
    }
    DB_RTR(*db) = (struct RTR_LA_HDR *) 0;
    DB_FREEME(*db) = FALSE;
    DB_DIST(*db) = RTRLSInfinity;
    DB_MYHASH(*db) = hash;
    DB_MY_AREA(*db) = area;

    area->db_cnts[LS_STUB]++;

    /* 
     * successful add, not found 
     */
    return FALSE;
}


/*
 * free a db entry
 *	- called by RxLsAck, RxLinkUp or tq_dbage
 *	- spf will have just been run so parent list and routes will have
 * 	  been freed
 *	- leave entry around db age will free the rest for
 *	  LS_ASE and LS_SUM_NET else since most other entries may be back
 *	  just free structure
 */
void
db_free __PF1(db, struct LSDB *)
{
    struct AREA *area;

    /* 
     * Still some outstanding acks? Wait untill dbfreelist is scanned again
     */
    if (NO_GUTS(db))
	return;

    if (DB_RTR(db) != ADVNULL) {
	area = DB_MY_AREA(db);

	if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSA_BLD)) {
	    ospf_trace_build(area, area, DB_ADV(db), TRUE);
	}

    	switch(LS_TYPE(db)) {
	case LS_STUB:
	    area->db_cnts[LS_STUB]--;
	    break;

	case LS_RTR:
	case LS_NET:
	case LS_SUM_NET:
	case LS_SUM_ASB:
	    ospf.db_cnt--;
	    area->db_cnts[LS_TYPE(db)]--;
	    area->db_int_cnt--;
	    area->db_chksumsum -= LS_CKS(db);
	    break;

	case LS_ASE:
	    ospf.db_cnt--;
	    ospf.db_ase_cnt--;
	    ospf.db_chksumsum -= LS_CKS(db);
	    break;
    	}

	DBADV_FREE(db, LS_TYPE(db));
    }

    /* 
     * Free from current queue 
     */
    DEL_DBQ(db);
    DB_FREEME(db) = FALSE;
    if (GOT_GUTS(db))
	DBGUTS_FREE(db);
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
