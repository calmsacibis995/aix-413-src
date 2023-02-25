static char sccsid[] = "@(#)12	1.1  src/tcpip/usr/sbin/gated/ospf_trace.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:58";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF2
 *		__PF4
 *		__PF7
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
 * ospf_trace.c,v 1.17 1993/04/05 04:28:55 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


static const char *pkttype[] = {
    "Interface Check",
    "Hello",
    "Database Description",
    "Link State Request",
    "Link State Update",
    "Link State Ack"
};


static const bits ospf_imms_bits[] = {
    { bit_I,	"I" },
    { bit_M,	"M" },
    { bit_MS,	"MS" },
    { 0, NULL }
};

static const bits ospf_option_bits[] = {
    { OPT_T_bit,	"TOS" },
    { OPT_E_bit,	"Externals" },
    { OPT_M_bit,	"Multicast" },
    { OPT_N_bit,	"NSSA" },
    { 0, NULL }
};

const bits ospf_ls_type_bits[] = {
    { LS_STUB,		"Stub" },
    { LS_RTR,		"Router" },
    { LS_NET,		"Net" },
    { LS_SUM_NET,	"SumNet" },
    { LS_SUM_ASB,	"SumASB" },
    { LS_ASE,		"ASE" },
    { LS_GM,		"Group" },
    { LS_NSSA,		"NSSA" },
    {0}
};


/*
 * Print the LSA hdr
 */
static void
ospf_log_ls_hdr __PF2(ls_hdr, struct LS_HDR *,
		      prefix, const char *)
{
    trace(TR_OSPF, 0, "%s\t%-6s\tId: %-15A  AdvRtr: %-15A  Age: %#T",
	  prefix,
	  trace_state(ospf_ls_type_bits, ls_hdr->ls_type),
	  sockbuild_in(0, ls_hdr->ls_id),
	  sockbuild_in(0, ls_hdr->adv_rtr),
	  ntohs(ls_hdr->ls_age));
    trace(TR_OSPF, 0, "%s\t\tLen: %3d  Seq #: %8x  Checksum: %#4x",
	  prefix,
	  ntohs(ls_hdr->length),
	  ntohl(ls_hdr->ls_seq),
	  ntohs(ls_hdr->ls_chksum));
}


static int
ospf_log_lsa __PF2(adv, union LSA_PTR,
		   prefix, const char *)
{
    int cnt, i;
    struct NET_LA_PIECES *att_rtr;
    struct RTR_LA_PIECES *linkp;

    ospf_log_ls_hdr(&adv.rtr->ls_hdr, prefix);

    switch (adv.rtr->ls_hdr.ls_type) {
    case LS_RTR:
	trace(TR_OSPF, 0, "%s\t\tCapabilities: As Border: %s Area Border: %s",
	      prefix,
	      (ntohs(adv.rtr->E_B) & bit_E) ? "On" : "Off",
	      (ntohs(adv.rtr->E_B) & bit_B) ? "On" : "Off");

	for (cnt = ntohs(adv.rtr->lnk_cnt),
	     i = 0,
	     linkp = (struct RTR_LA_PIECES *) & (adv.rtr->link);
	     i < cnt;
	     linkp = (struct RTR_LA_PIECES *) ((long) linkp +
					      RTR_LA_PIECES_SIZE +
					      ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE)),
	     i++) {
	    trace(TR_OSPF, 0, "%s\t\t%-8s  ID: %-15A  Data: %-15A metric: %-4d",
		  prefix,
		  ospf_con_types[linkp->con_type - 1],
		  sockbuild_in(0, linkp->lnk_id),
		  sockbuild_in(0, linkp->lnk_data),
		  ntohs(linkp->tos0_metric));
	}
	break;

    case LS_NET:
	trace(TR_OSPF, 0, "%s\t\tMask: %A",
	      prefix,
	      sockbuild_in(0, adv.net->net_mask));

	cnt = ntohs(adv.net->ls_hdr.length) - NET_LA_HDR_SIZE;
	for (att_rtr = &(adv.net->att_rtr), i = 0;
	     i < cnt;
	     att_rtr++, i += 4) {
	    trace(TR_OSPF, 0, "%s\t\tAttached router: %A",
		  prefix,
		  sockbuild_in(0, att_rtr->lnk_id));
	}
	break;

    case LS_SUM_NET:
	trace(TR_OSPF, 0, "%s\t\tMask: %-15A",
	      prefix,
	      sockbuild_in(0, adv.sum->net_mask));
	/* Fall through */

    case LS_SUM_ASB:
	trace(TR_OSPF, 0, "%s\t\tTos 0 metric: %-4d",
	      prefix,
	      ntohl(adv.sum->tos0.tos_metric));
	break;

    case LS_ASE:
	trace(TR_OSPF, 0, "%s\t\tMask: %-15A  Tos 0 metric: %-5d  Type: %-1d",
	      prefix,
	      sockbuild_in(0, adv.ase->net_mask),
	      ADV_BIG_METRIC(adv.ase),
	      ADV_ASE_TYPE2(adv.ase) ? 2 : 1);

	trace(TR_OSPF, 0, "%s\t\tForwarding Address: %-15A Tag: %A",
	      prefix,
	      sockbuild_in(0, adv.ase->tos0.ForwardAddr),
	      ospf_path_tag_dump(inet_autonomous_system, adv.ase->tos0.ExtRtTag));
	break;

    case LS_GM:
	switch (adv.gm->vertex.type) {
	case GM_TYPE_RTR:
	    trace(TR_OSPF, 0, "%s\t\tRouter ID: %-15A",
		  prefix,
		  sockbuild_in(0, adv.gm->vertex.id));
	    break;

	case GM_TYPE_NET:
	    trace(TR_OSPF, 0, "%s\t\tDesignated Router: %-15A",
		  prefix,
		  sockbuild_in(0, adv.gm->vertex.id));
	    break;
	}
	break;
	
    case LS_NSSA:
	break;
    }

    return ntohs(adv.rtr->ls_hdr.length);
}


void
ospf_trace __PF7(pkt, struct OSPF_HDR *,
		 len, size_t,
		 type, u_int,
		 direction, int,
		 intf, struct INTF *,
		 src, sockaddr_un *,
		 dst, sockaddr_un *)
{
    const char *prefix = direction ? "OSPF RECV" : "OSPF SENT";

    trace(TR_OSPF, 0, "%s %A(%s) -> %A  %s  Vers: %d  Len: %d",
	  prefix,
	  src,
	  intf->ifap ? intf->ifap->ifa_name : "n/a",
	  ospf_addr2str(dst),
	  pkttype[pkt->type],
	  pkt->version,
	  ntohs(pkt->length));

    trace(TR_OSPF, 0, "%s RouterID: %A  Area: %A  Checksum: %#x",
	  prefix,
	  sockbuild_in(0, pkt->rtr_id),
	  sockbuild_in(0, pkt->area_id),
	  ntohs(pkt->checksum));

    {
	register byte *bp = (byte *) pkt->Auth;

	trace(TR_OSPF, 0, "%s Auth: Type: %d Key: %02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x",
	      prefix,
	      ntohs(pkt->AuType),
	      *bp++, *bp++, *bp++, *bp++,
	      *bp++, *bp++, *bp++, *bp++);
    }


    switch (type) {
    case O_HELLO:
        if (BIT_TEST(ospf.trace_flags, TR_OSPF_HELLO)) {
	    int i = 1;
	    
	    struct HELLO_HDR *hello = &pkt->un.hello;
	    struct RHF *rhf = &hello->rhf;
	    caddr_t limit = (caddr_t) rhf + len - OSPF_HDR_SIZE - HELLO_HDR_SIZE;

	    trace(TR_OSPF, 0, "%s Netmask: %-12A Hello Int: %-4d Options: <%s>",
		  prefix,
		  sockbuild_in(0, hello->netmask),
		  ntohs(hello->HelloInt),
		  trace_bits(ospf_option_bits, (flag_t) hello->options));
	    trace(TR_OSPF, 0, "%s Pri: %-4d DeadInt: %-4d DR: %-15A BDR: %-15A",
		  prefix,
		  hello->rtr_priority,
		  ntohl(hello->DeadInt),
		  sockbuild_in(0, hello->dr),
		  sockbuild_in(0, hello->bdr));

	    if ((caddr_t) rhf < limit) {
		tracef("%s Attached routers:",
		       prefix);

		for (; (caddr_t) rhf < limit; rhf++) {
		    tracef("\t%-15A",
			   sockbuild_in(0, rhf->rtr));

		    if (++i > 3) {
			i = 0;
			trace(TR_OSPF, 0, NULL);
			tracef("%s\t",
			       prefix);
		    }
		}
		if (i > 0) {
		    trace(TR_OSPF, 0, NULL);
		}
	    }
	}
	break;
 
    case O_DB_DESCRIPT:
        if (BIT_TEST(ospf.trace_flags, TR_OSPF_DD)) {
	    struct DB_HDR *dbh = &pkt->un.database;
	    struct LS_HDR *dbp;
 
	    tracef("%s Flags <%s>",
		   prefix,
		   trace_bits(ospf_imms_bits, (flag_t) dbh->I_M_MS));
	    trace(TR_OSPF, 0, " Options: <%s> seq: %8x",
		  trace_bits(ospf_option_bits, (flag_t) dbh->options),
		  ntohl(dbh->seq));

	    for ( dbp = &dbh->dbp, len -= (OSPF_HDR_SIZE + DB_HDR_SIZE);
		 len;
		 len -= DB_PIECE_SIZE, dbp++ ) {

		switch (dbp->ls_type) {
		case LS_RTR:
		case LS_NET:
		case LS_SUM_NET:
		case LS_SUM_ASB:
		case LS_ASE:
		case LS_GM:
		case LS_NSSA:
		    /* OK */
		    ospf_log_ls_hdr(dbp, prefix);
		    break;

		default:
		    trace(TR_OSPF, 0, "%s\t\tBAD_LSA_TYPE %d",
			  prefix,
			  dbp->ls_type);
		    break;;
		}
	    }
	}
	break;
 
    case O_LSR:
        if (BIT_TEST(ospf.trace_flags, TR_OSPF_REQ)) {
	    struct LS_REQ_HDR *ls_req = &pkt->un.ls_req;
	    struct LS_REQ_PIECE *req;

	    for ( req = &ls_req->req, len -= OSPF_HDR_SIZE;
		 len; 
		 len -= LS_REQ_PIECE_SIZE, req++ ) {

		switch (req->ls_type) {
		case LS_RTR:
		case LS_NET:
		case LS_SUM_NET:
		case LS_SUM_ASB:
		case LS_ASE:
		case LS_GM:
		case LS_NSSA:
		    /* OK */
		    trace(TR_OSPF, 0, "%-6s\t%s\tId: %-15A  AdvRtr: %-15A",
			  prefix,
			  trace_state(ospf_ls_type_bits, req->ls_type),
			  sockbuild_in(0, req->ls_id),
			  sockbuild_in(0, req->adv_rtr));
		    break;

		default:
		    trace(TR_OSPF, 0, "%s\t\tBAD_LSA_TYPE %d",
			  prefix,
			  req->ls_type);
		    break;
		}
	    }
	}
	break;
	
    case O_LSU:
        if (BIT_TEST(ospf.trace_flags, TR_OSPF_LSU)) {
	    int adv_len = 0;
	    struct LS_UPDATE_HDR *lsup = &pkt->un.ls_update;
	    union LSA_PTR adv;
	    u_int32 i = ntohl(lsup->adv_cnt);

	    trace(TR_OSPF, 0, "%s Advertisement count: %-4d",
		  prefix,
		  ntohl(lsup->adv_cnt));
	    for (adv.rtr = (struct RTR_LA_HDR *) & (lsup->adv.rtr);
		 i && len;
		 i--,
		 len -= adv_len,
		 adv.rtr = (struct RTR_LA_HDR *) ((long) adv.rtr + adv_len)) {
		adv_len = ospf_log_lsa(adv, prefix);
	    }
	}
	break;

    case O_ACK:
        if (BIT_TEST(ospf.trace_flags, TR_OSPF_ACK)) {
	    struct LS_ACK_HDR *ls_ack = &pkt->un.ls_ack;
	    struct LS_HDR *ack;
 
	    for( ack = &ls_ack->ack_piece, len -= OSPF_HDR_SIZE;
		len > 0;
		len -= ACK_PIECE_SIZE, ack++ ) {

		switch (ack->ls_type) {
		case LS_RTR:
		case LS_NET:
		case LS_SUM_NET:
		case LS_SUM_ASB:
		case LS_ASE:
		case LS_GM:
		case LS_NSSA:
		    /* OK */
		    ospf_log_ls_hdr(ack, prefix);
		    break;

		default:
		    trace(TR_OSPF, 0, "%s\t\tBAD_LSA_TYPE %d",
			  prefix,
			  ack->ls_type);
		    break;
		}
	    }
	}
	break;
    }

    trace(TR_OSPF, 0, NULL);
}


void
ospf_trace_build  __PF4(from, struct AREA *,
			to, struct AREA *,
			lsa, union LSA_PTR,
			freed, int)
{
    const char *prefix = freed ? "OSPF LSA FREE" : "OSPF LSA BUILD";

    switch (lsa.rtr->ls_hdr.ls_type) {
    case LS_STUB:
    case LS_RTR:
    case LS_NET:
	trace(TR_OSPF, 0, "%s Area: %A",
	      prefix,
	      sockbuild_in(0, from->area_id));
	break;

    case LS_SUM_NET:
    case LS_SUM_ASB:
	trace(TR_OSPF, 0, "%s From Area: %A  To Area: %A",
	      prefix,
	      sockbuild_in(0, from->area_id),
	      sockbuild_in(0, to->area_id));
	break;
	
    case LS_ASE:
	break;

    case LS_GM:
	break;
	
    case LS_NSSA:
	break;
	
    default:
	assert(FALSE);
	break;
    }

    (void) ospf_log_lsa(lsa, prefix);

    trace(TR_OSPF, 0, NULL);
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
