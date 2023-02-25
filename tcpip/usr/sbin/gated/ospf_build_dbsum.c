static char sccsid[] = "@(#)84	1.1  src/tcpip/usr/sbin/gated/ospf_build_dbsum.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:44";
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
 * ospf_build_dbsum.c,v 1.12 1993/03/22 02:39:52 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"

/*
 * dbsum_alloc() - allocate a link up whose data portion is a db_hdr
 *			called by build_dbsum
 */
struct LSDB_SUM *
dbsum_alloc __PF1(len, int)
{
    struct LSDB_SUM *ds;

    DBS_ALLOC(ds);
    OSPF_PKT_ALLOC(ds->dbpkt, len);
    if (ds->dbpkt != OSPF_HDR_NULL)
	return ds;
    else {
	FREE(ds, OMEM_DBSUM);
	return LSDB_SUM_NULL;
    }
}

/*
 * build_dbsum - build dbsum packets which includes the lsdb for this rtr
 *			put packets on the nbrs dbsum list
 *	       - called by RxDbDescr going into next state
 */
int
build_dbsum __PF2(intf, struct INTF *,
		  nbr, struct NBR *)
{
    struct DB_HDR *dbh;
    struct LS_HDR *dbp;
    struct LSDB *e, *hp;
    struct AREA *a = intf->area;
    struct LSDB_SUM *ds = LSDB_SUM_NULL;
    struct LSDB_SUM *lastds = LSDB_SUM_NULL;
    int len = (OSPF_HDR_SIZE + DB_HDR_SIZE);
    int type;
    int intf_mtu = INTF_MTU(intf);
    int hi_type = LS_ASE;

    if (BIT_TEST(a->area_flags, OSPF_AREAF_STUB) ||
	intf->type == VIRTUAL_LINK) {
	/* Don't include LS_ASE if stub area also it's a bozo no-no to send LS_ASE over virtual links */
	hi_type = LS_SUM_ASB;
    }

    freeDbSum(nbr);		/* summary of area db */
    freeLsReq(nbr);		/* request list for this nbr */

    ds = (struct LSDB_SUM *) dbsum_alloc(intf_mtu);
    if (ds == LSDB_SUM_NULL)
	return FLAG_NO_BUFS;
    dbh = &(ds->dbpkt->un.database);
    dbp = &(dbh->dbp);
    nbr->dbsum = ds;

    /* grab all of the links */
    for (type = LS_RTR; type <= hi_type; type++) {
	for (hp = a->htbl[type]; hp < &(a->htbl[type][HTBLSIZE]); hp++) {
	    for (e = DB_NEXT(hp); e; e = DB_NEXT(e)) {
		/* Too far gone ... */
		if (NO_GUTS(e))
		    continue;
		if (ADV_AGE(e) >= MaxAge) {
		    add_nbr_retrans(nbr, e);
		    add_db_retrans(e, nbr);
		    continue;
		}
		if (len + DB_PIECE_SIZE >= intf_mtu) {
		    /* finish off this pkt */
		    /* assume more for now */
		    dbh->I_M_MS = bit_M;
		    ds->len = len;
		    lastds = ds;
		    /* alloc another */
		    lastds->next = (ds = dbsum_alloc(intf_mtu));
		    if (ds == LSDB_SUM_NULL) {
			freeDbSum(nbr);
			REM_NBR_RETRANS(nbr);
			return FLAG_NO_BUFS;
		    }
		    len = (OSPF_HDR_SIZE + DB_HDR_SIZE);
		    dbh = &(ds->dbpkt->un.database);
		    dbp = &(dbh->dbp);
		}
		(*dbp) = DB_LS_HDR(e);
		dbp->ls_age = htons((ADV_AGE(e)));
		len += DB_PIECE_SIZE;
		nbr->dbcnt++;
		ds->cnt++;
		dbp++;
	    }
	}
    }

    /* slave transitions first so master is first to have M bit off */
    if (len == OSPF_HDR_SIZE + DB_HDR_SIZE) {	/* check for empty pkt */
	/* will always have at one sum with least this rtrs lsa in pkt */
	if (ds == nbr->dbsum)
	    nbr->dbsum = LSDB_SUM_NULL;
	DB_FREE_PKT(ds);
	FREE(ds, OMEM_DBSUM);
	if (lastds) {
	    lastds->next = LSDB_SUM_NULL;
	    /* turn off more */
	    lastds->dbpkt->un.database.I_M_MS = 0;
	}
    } else {
	/* leave on more bit if MASTER */
	if (nbr->mode == SLAVE)
	    ds->dbpkt->un.database.I_M_MS = 0;	/* turn off more */
	ds->next = LSDB_SUM_NULL;
	ds->len = len;
    }

    return FLAG_NO_PROBLEM;		/* BufChecks went OK */
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
