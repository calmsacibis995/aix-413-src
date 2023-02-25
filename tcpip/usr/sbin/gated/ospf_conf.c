static char sccsid[] = "@(#)87	1.1  src/tcpip/usr/sbin/gated/ospf_conf.c, tcprouting, tcpip411, GOLD410 12/6/93 17:25:51";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF2
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
 * ospf_conf.c,v 1.19.2.1 1993/08/27 11:23:44 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"

/*
 * Add a net to an area
 *   - nets are used to build summary links
 *   - calling routine has checked for duplicates
 */
void
ospf_parse_add_net __PF3(a, struct AREA *,
			 net, sockaddr_un *,
			 mask, sockaddr_un *)
{
    struct NET_RANGE *range;

    a->nrcnt++;
    NR_ALLOC(range);
    range->net = sock2ip(net);
    range->mask = sock2ip(mask);
    range_enq(a,range);
}


/*
 * Add host to be advertised by this router
 */
void
ospf_parse_add_host __PF3(a, struct AREA *,
			  if_address, u_int32,	/* XXX - Why not ifap or intf? */
			  cost, metric_t)
{
    struct OSPF_HOSTS *newhost;

    a->hostcnt++;
    HOST_ALLOC(newhost);
    newhost->if_addr = if_address;
    newhost->cost = cost;
    host_enq(a,newhost);
}


/*
 * Allocate a new area
 */
struct AREA *
ospf_parse_area_alloc __PF2(area_id, u_int32,
			    parse_error, char *)
{
    int i;
    struct AREA *area;

    if (area_id == OSPF_BACKBONE) {
	/* This is the backbone, put it at the head of the list */

	area = &ospf.backbone;
	ospf.backbone.next = ospf.area;
	ospf.area = &ospf.backbone;
    } else {
	/* Not the backbone.  Put on list in area order */

	struct AREA *ap, *last = (struct AREA *) 0;

	AREA_LIST(ap) {
	    if (area_id == ap->area_id) {
		/* Duplicate area */

		sprintf(parse_error, "duplicate area");
		return (struct AREA *) 0;
	    } else if (area_id < ap->area_id) {
		/* Insert here */

		break;
	    }
	    last = ap;
	} AREA_LIST_END(ap) ;
	/* if area_id == ap->area_id, then error ? */

	AREA_ALLOC(area) ;

	if (last) {
	    area->next = ap;
	    last->next = area;
	} else {
	    area->next = ospf.area;
	    ospf.area = area;
	}
    }
    ospf.acnt++;
	
    /* How to check for 0 but invalid id? */
    area->area_id = area_id;

    /* set up hash table for lsdb */
    for(i = LS_STUB; i < LS_ASE; i++) {
	DBBLOCK_ALLOC((area->htbl[i]), HTBLSIZE);
    }

    /* set ase to global */
    area->htbl[LS_ASE] = ospf.ase;

    /* LSDB link-list heads */
    DBGUTS_ALLOC(&area->spf);	/* spf tree */
    DBGUTS_ALLOC(&area->candidates); /* spf candidate list */
    DBGUTS_ALLOC(&area->asblst); /* asb rtrs from attached areas */
    DBGUTS_ALLOC(&area->sumnetlst);/* nets from attached areas */
    DBGUTS_ALLOC(&area->interlst); /* routes from backbone */
    DBGUTS_ALLOC(&area->dflt_sum); /* if ABRtr and stub area */

    return area;
}


int
ospf_parse_area_check  __PF2(a, struct AREA *,
			     parse_error, char *)
{
    if (a->area_id) {
	/* Not the backbone */

	if (!a->intf_policy) {
	    sprintf(parse_error, "no interfaces for area %A",
		    sockbuild_in(0, a->area_id));

	    return TRUE;
	}	
    } else {
	/* The backbone */

	if (!a->intf_policy && !ospf.vcnt) {
	    sprintf(parse_error, "no interfaces for backbone area");

	    return TRUE;
	}
    }

    return FALSE;
}


#ifdef	notyet
/*
 *  Create a non-virtual interface
 */
struct INTF *
ospf_intf_create __PF3(a, struct AREA *,
		       ifap, if_addr *,
		       ifcp, ospf_intf_conf *)
{
    int i;
    struct INTF *intf;
	 
    /* Check for duplicate interface */
    INTF_LIST(intf, a) {
	if (intf->ifap == ifap) {
	    sprintf(parse_error, "Duplicate interface specified");
	    return (struct INTF *) 0;
	}
    } INTF_LIST_END(intf, a) ;

    INTF_ALLOC(intf);
    if (a->intf) {
	struct INTF *last;

	INTF_LIST(last, a) {
	    if (!last->next) {
		break;
	    }
	} INTF_LIST_END(last, a) ;

	last->next = intf;
    } else {
	a->intf = intf;
    }
    a->ifcnt++;

    intf->ifap = ifa_alloc(ifap);
    intf->area = a;
    intf->type = type;
    intf->state = IDOWN;
    intf->transdly = OSPF_DFLT_TRANSDLY;
    intf->pri = 0;
    intf->retrans_timer = OSPF_DFT_RETRANS;
    intf->cost = OSPF_DFLT_COST;
    BIT_SET(intf->flags, OSPF_INTFF_ENABLE);

#ifdef	IP_MULTICAST
    if (BIT_TEST(intf->ifap->ifa_state, IFS_MULTICAST)) {
	/* Assume we want multicast */
	BIT_SET(intf->flags, OSPF_INTFF_MULTICAST);
    }
#endif	/* IP_MULTICAST */

    switch (BIT_TEST(ifcp->ifc_flags, OSPF_IFC_P2P|OSPF_IFC_BROADCAST|OSPF_IFC_NBMA)) {
#ifdef	IP_MULTICAST
    case OSPF_IFC_BROADCAST:
	intf->type = BROADCAST;
	intf->hello_timer = BIT_TEST(ifpc->ifc_flags, OSPF_IFC_HELLO) ? ifpc->ifc_hello : OSPF_BC_DFLT_HELLO;
	goto common;

#endif	/* IP_MULTICAST */
    case OSPF_IFC_NBMA:
	inff->type = NONBROADCAST;
	if (BIT_TEST(ifap->ifa_state, IFS_BROADCAST)) {
	    intf->poll_timer = OSPF_DFLT_POLL_INT;
	    intf->hello_timer = OSPF_BC_DFLT_HELLO;
	} else {
	    intf->poll_timer = OSPF_DFLT_POLL_INT;		/* XXX - different default? */
	    intf->hello_timer = OSPF_NBMA_DFLT_HELLO;
	}
	intf->pollmod = 1;
#ifdef	IP_MULTICAST
	BIT_RESET(intf->flags, OSPF_INTFF_MULTICAST);
	/* Fall through */

    common:
#endif	/* IP_MULTICAST */
	/* Common to broadcast and nbma */
	intf->nbr.nbr_id = sockdup(ospf.router_id);
	intf->nbr.nbr_addr = sockdup(intf->ifap->ifa_addr);
	intf->nbr.pri = intf->pri;
	intf->nbr.state = N2WAY;
	intf->dead_timer = intf->hello_timer * 4;
	break;

    case OSPF_IFC_P2P:
	intf->type = POINT_TO_POINT;
	intf->hello_timer = OSPF_PTP_DFLT_HELLO;
	intf->dead_timer = intf->hello_timer * 4;
	intf->nbr.nbr_addr = sockdup(intf->ifap->ifa_addr);
	intf->nbr.ifap = ifa_alloc(ifap);
	break;
    }
    ospf.nintf++;

    return intf;
}
#endif	/* notyet */


/*
 * Alloc an non-virtual interface and set default values
 */
struct INTF *
ospf_parse_intf_alloc __PF3(a, struct AREA *,
			    type, int,
			    ifap, if_addr *)
{
    struct INTF *intf;
	 
    INTF_ALLOC(intf);
    if (a->intf) {
	struct INTF *last;

	INTF_LIST(last, a) {
	    if (!last->next) {
		break;
	    }
	} INTF_LIST_END(last, a) ;

	last->next = intf;
    } else {
	a->intf = intf;
    }
    a->ifcnt++;

    intf->ifap = ifa_alloc(ifap);
    intf->area = a;
    intf->type = type;
    intf->state = IDOWN;
    intf->transdly = OSPF_DFLT_TRANSDLY;
    intf->pri = 0;
    intf->retrans_timer = OSPF_DFT_RETRANS;
    intf->cost = OSPF_DFLT_COST;
    BIT_SET(intf->flags, OSPF_INTFF_ENABLE);
    if (BIT_TEST(intf->ifap->ifa_state, IFS_MULTICAST)) {
	/* Assume we want multicast */
	BIT_SET(intf->flags, OSPF_INTFF_MULTICAST);
    }

    switch (type) {
    case BROADCAST:
	intf->hello_timer = OSPF_BC_DFLT_HELLO;
	goto common;

    case NONBROADCAST:
	if (BIT_TEST(ifap->ifa_state, IFS_BROADCAST)) {
	    intf->poll_timer = OSPF_DFLT_POLL_INT;
	    intf->hello_timer = OSPF_BC_DFLT_HELLO;
	} else {
	    intf->poll_timer = OSPF_DFLT_POLL_INT;		/* XXX - different default? */
	    intf->hello_timer = OSPF_NBMA_DFLT_HELLO;
	}
	intf->pollmod = 1;
	BIT_RESET(intf->flags, OSPF_INTFF_MULTICAST);
	/* Fall through */

    common:
	/* Common to broadcast and nbma */
	intf->nbr.nbr_id = sockdup(ospf.router_id);
	intf->nbr.nbr_addr = sockdup(intf->ifap->ifa_addr);
	intf->nbr.pri = intf->pri;
	intf->nbr.state = N2WAY;
	intf->dead_timer = intf->hello_timer * 4;
	break;

    case POINT_TO_POINT:
	intf->hello_timer = OSPF_PTP_DFLT_HELLO;
	intf->dead_timer = intf->hello_timer * 4;
	intf->nbr.nbr_addr = sockdup(intf->ifap->ifa_addr);
#ifdef	notdef
	intf->nbr.ifap = ifa_alloc(ifap);
#endif	/* notdef */
	intf->nbr.intf = intf;
	break;
    }
    ospf.nintf++;

    return intf;
}


/*
 * virtual link allocate and set default metrics
 */
struct INTF *
ospf_parse_virt_parse __PF5(a, struct AREA *,
			    addr, sockaddr_un *,
			    transarea, u_int32,
			    list, config_list *,
			    parse_error, char *)
{
    struct INTF *intf;

    /* Check address */
    if (socktype(addr) != AF_INET) {
	sprintf(parse_error, "neighbor-id must be an IP address");
	return (struct INTF *) 0;
    }

    /* Check area */
    if (a->area_id != OSPF_BACKBONE) {
	sprintf(parse_error, "virtual links only allowed in `backbone' area");
	return (struct INTF *) 0;
    }

    /* Check transit area */
    if (transarea == OSPF_BACKBONE) {
	sprintf(parse_error, "transit-area can not be the `backbone' area");
	return (struct INTF *) 0;
    }

    /* Allocate the interface */
    INTF_ALLOC(intf);
    if (ospf.vcnt) {
	struct INTF *last;

	VINTF_LIST(last) {
	    if (!last->next) {
		break;
	    }
	} VINTF_LIST_END(last) ;

	last->next = intf;
    } else {
	ospf.vl = intf;
    }
    ospf.vcnt++;

    intf->area = a;
    intf->hello_timer = OSPF_VIRT_DFLT_HELLO;
    intf->dead_timer = intf->hello_timer * 4;
    intf->type = VIRTUAL_LINK;
    intf->state = IDOWN;
    intf->transdly = OSPF_VIRT_DFLT_TRANSDLY;
    intf->pri = 0;
    intf->retrans_timer = OSPF_VIRT_DFT_RETRANS;
    intf->nbr.nbr_id = addr;
    intf->transarea = (struct AREA *) transarea;

    if (list && list->conflist_list) {
	register config_entry *cp;

	CONFIG_LIST(cp, list->conflist_list) {

	    switch (cp->config_type) {
	    case OSPF_CONFIG_ENABLE:
		if ((int) cp->config_data) {
		    BIT_SET(intf->flags, OSPF_INTFF_ENABLE);
		} else {
		    BIT_RESET(intf->flags, OSPF_INTFF_ENABLE);
		}
		break;
		    
	    case OSPF_CONFIG_RETRANSMIT:
		intf->retrans_timer = (time_t) cp->config_data;
		break;

	    case OSPF_CONFIG_TRANSIT:
		intf->transdly = (time_t) cp->config_data;
		break;

	    case OSPF_CONFIG_PRIORITY:
		trace(TR_OSPF, LOG_INFO,
		      "ospf_parse_virt_parse: priority option ignored for virtual link to %A",
		      addr);
		break;

	    case OSPF_CONFIG_HELLO:
		intf->hello_timer = (time_t) cp->config_data;
		break;
		    
	    case OSPF_CONFIG_ROUTERDEAD:
		intf->dead_timer = (time_t) cp->config_data;
		break;
		    
	    case OSPF_CONFIG_AUTHKEY:
		bcopy((caddr_t) ((sockaddr_un *) cp->config_data)->s.ss_string,
		      (caddr_t) intf->authkey,
		      OSPF_AUTH_SIZE);
		break;

	    default:
		assert(FALSE);

	    }
	} CONFIG_LIST_END(cp, list->conflist_list) ;
    }

    return intf;
}


void
ospf_parse_intf_check __PF1(intf, struct INTF *)
{
    /* if wait and dead tmr aren't set default to 4 times hello tmr */
    if (intf->dead_timer == 0)
	intf->dead_timer = intf->hello_timer * 4;
}


/*
 * Check configuration for valid params
 */
int
ospf_parse_valid_check __PF1(parse_error, char *)
{

    /* recheck configuration */
    if (!ospf.acnt) {
	sprintf(parse_error, "ospf_conf: No Areas defined");
	return TRUE;
    }

    if (ospf.acnt < 2 && ospf.vcnt) {
	sprintf(parse_error, "ospf_conf: virtual link configured < 2 areas");
	return TRUE;
    }
    
    if ((ospf.acnt > 2) && !(GOTBACKBONE)) {
	sprintf(parse_error, "ospf_conf: 2 or more areas have been defined: need to configure backbone ");
	return TRUE;
    }

    /* Resolve transit areas */
    if (ospf.vcnt) {
	struct INTF *intf;

	VINTF_LIST(intf) {
	    struct AREA *area;
	    u_int32 transarea = (u_int32) intf->transarea;

	    AREA_LIST(area) {
		if (area->area_id == transarea) {
		    /* Found it */

		    break;
		} else if (area->area_id > transarea) {
		    /* No such luck */

		    area = (struct AREA *) 0;
		}
	    } AREA_LIST_END(area) ;

	    if (area) {
		/* Found our area */

		BIT_SET(area->area_flags, OSPF_AREAF_TRANSIT);
		intf->transarea = area;
	    } else {
		/* No area */

		sprintf(parse_error, "could not find transit-area %A for virtual link to %A",
			sockbuild_in(0, transarea),
			intf->nbr.nbr_id);
		return TRUE;
	    }
	} VINTF_LIST_END(intf) ;
    }

    return FALSE;
}


ospf_config_router *
ospf_parse_router_alloc __PF2(router, struct in_addr,
			      priority, u_int)
{
    ospf_config_router *ocr = (ospf_config_router *) task_block_alloc(ospf_router_index);

    ocr->ocr_router = router;
    ocr->ocr_priority = priority;

    return ocr;
}


void
ospf_config_free __PF1(cp, config_entry *)
{
    switch (cp->config_type) {
    case OSPF_CONFIG_AUTHKEY:
	sockfree((sockaddr_un *) cp->config_data);
	break;
	
    case OSPF_CONFIG_ROUTERS:
        {
	    ospf_config_router *ocr = (ospf_config_router *) cp->config_data;

	    do {
		ospf_config_router *next = ocr->ocr_next;

		task_block_free(ospf_router_index, (void_t) ocr);

		ocr = next;
	    } while (ocr) ;
	}
	break;
	
    default:
	/* Not allocated */
	break;
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
