static char sccsid[] = "@(#)32	1.1  src/tcpip/usr/sbin/gated/targets.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:55";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		_PROTOTYPE
 *		__PF2
 *		__PF3
 *		target_alloc
 *		target_build
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
 *  targets.c,v 1.17 1993/04/14 21:25:04 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#include "targets.h"

/**/

static const bits target_flag_bits[] = {
    { TARGETF_BROADCAST,	"Broadcast" },
    { TARGETF_SOURCE,		"Source" },
    { TARGETF_SUPPLY,		"Supply" },
    { TARGETF_ALLINTF,		"AllInterfaces" },
    { TARGETF_POLICY,		"Policy" },
    { 0, NULL },
};

static block_t target_block_index;

/* Free a target list */
target *
target_free __PF2(tp, task *,
		  old, target *)
{
    target *tlp;
    target *next = old;
    
    while (tlp = next) {
	next = tlp->target_next;

	/* Invoke protocol specific free routine */
	if (tlp->target_reset) {
	    tlp->target_reset(tp, tlp);
	}
	
	/* Free the bit if allocated */
	if (tlp->target_rtbit) {
	    /* Release any routes we have announced */

	    rtbit_reset_all(tp, tlp->target_rtbit, tlp->target_gwp);
	}

	/* Free the interface */
	if (tlp->target_ifap) {
	    (void) ifa_free(tlp->target_ifap);
	}

	/* Free the target entry */
	task_block_free(target_block_index, (void_t) tlp);
    }

    return (target *) 0;
}


target *
target_locate __PF3(list, target *,
		    ifap, if_addr *,
		    gwp, gw_entry *)
{
    target *tlp;
    
    TARGET_LIST(tlp, list) {
	if ((BIT_TEST(tlp->target_flags, TARGETF_BROADCAST)
	     && tlp->target_ifap == ifap)
	    || (BIT_TEST(tlp->target_flags, TARGETF_SOURCE)
		&& tlp->target_gwp == gwp)) {

	    return tlp;
	}
    } TARGET_LIST_END(tlp, list) ;

    return (target *) 0;
}


PROTOTYPE(target_alloc,
	  static inline target *,
	  (task *,
	   target **,
	   target **,
	   if_addr *,
	   gw_entry *,
	   u_int,
	   _PROTOTYPE(ta_dump,
		      void,
		      (rt_head *,
		       void_t,
		       char *))));	   
static inline target *
target_alloc(ta_tp, old_list, new_list, ifap, gwp, ta_tsi_size, ta_dump)
task *ta_tp;
target **old_list;
target **new_list;
if_addr *ifap;
gw_entry *gwp;
u_int ta_tsi_size;
_PROTOTYPE(ta_dump,
	   void,
	   (rt_head *,
	    void_t,
	    char *));
{
    target *prev;
    target *ta_tlp;

    /* Locate this target on the old list */
    for (ta_tlp = *old_list, prev = (target *) 0;
	 ta_tlp;
	 prev = ta_tlp, ta_tlp = ta_tlp->target_next) {
	if (ta_tlp->target_ifap == ifap
	    && ta_tlp->target_gwp == gwp) {
	    if (prev) {
		prev->target_next = ta_tlp->target_next;
	    } else {
		*old_list = ta_tlp->target_next;
	    }
	    ta_tlp->target_next = (target *) 0;
	    break;
	}
    }

    /* If not on the old list, allocate a new one */
    if (!ta_tlp) {
	/* Allocate our block index */
	if (!target_block_index) {
	    target_block_index = task_block_init(sizeof (target));
	}

	/* Allocate this block */
    	ta_tlp = (target *) task_block_alloc(target_block_index);
    }

    if (ta_tsi_size) {
	if (!BIT_TEST(ta_tlp->target_flags, TARGETF_SUPPLY)) {
	    /* Allocate a bit for this target */

	    ta_tlp->target_rtbit = rtbit_alloc(ta_tp, ta_tsi_size, (caddr_t) ta_tlp, ta_dump);

	    /* Indicate we supply to this guy */
	    BIT_SET(ta_tlp->target_flags, TARGETF_SUPPLY);
	}
    } else if (BIT_TEST(ta_tlp->target_flags, TARGETF_SUPPLY)) {
	/* Free the bit for this target */

	/* Release any routes we have announced */
	rtbit_reset_all(ta_tp, ta_tlp->target_rtbit, ta_tlp->target_gwp);
	ta_tlp->target_rtbit = 0;

	/* Indicate that we don't want to supply any more */
	BIT_RESET(ta_tlp->target_flags, TARGETF_SUPPLY);
    }
    
    /* Append to the end of the new list */
    if (*new_list) {
	for (prev = *new_list; prev->target_next; prev = prev->target_next);
	prev->target_next = ta_tlp;
    } else {
	*new_list = ta_tlp;
    }

    return ta_tlp;
}


/* Allocate and build a target list for the given parameters */
int
target_build(tp, list, gw_list, flags, tsi_size, dump)
task *tp;
target **list;
gw_entry *gw_list;
flag_t flags;
u_int tsi_size;
_PROTOTYPE(dump,
	   void,
	   (rt_head *,
	    void_t,
	    char *));
{
    int targets = 0;
    if_addr *ifap;
    target *old = *list;
    target *tlp;

    *list = (target *) 0;

    /* Reset the active bits on any interfaces */
    TARGET_LIST(tlp, old) {
	BIT_RESET(tlp->target_ifap->ifa_rtactive, RTPROTO_BIT(tp->task_rtproto));
    } TARGET_LIST_END(tlp, old) ;

    /* First add interfaces */
    if (BIT_TEST(flags, TARGETF_ALLINTF|TARGETF_BROADCAST)) {
	IF_ADDR(ifap) {
	    sockaddr_un **dest;

	    if (socktype(ifap->ifa_addr) != AF_INET
		|| !BIT_TEST(ifap->ifa_state, IFS_UP)
		|| BIT_TEST(ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOOUT)) {
		/* Interface is down or prohibited from announcing */
		/* this protocol */

		continue;
	    }

	    switch (BIT_TEST(ifap->ifa_state, IFS_BROADCAST|IFS_POINTOPOINT|IFS_LOOPBACK)) {
	    case IFS_LOOPBACK:
		/* The default is not to send packets to the loopback */
		/* interface.  This can be overridden by specifying the */
		/* loopback address in the sourcegateway clause */
		continue;

	    case IFS_BROADCAST:
		/* On broadcast interfaces, send to the broadcast address */
		
		dest = &ifap->ifa_addr_broadcast;
		break;

	    case IFS_POINTOPOINT:
		/* On P2P interfaces, send to the destination address. */

		dest = &ifap->ifa_addr;
		break;

	    default:
		/* On NBMA interfaces we send packets to our self in order */
		/* to test that the interface is working.  This assumes that */
		/* packets send to myself over that interface will actually */
		/* get looped back by the hardware */

		if (BIT_TEST(ifap->ifa_state, IFS_NOAGE)) {
		    /* The test is not desired */

		    continue;
		}
		dest = &ifap->ifa_addr;
		break;
	    }
		
	    /* Locate old or allocate new */
	    tlp = target_alloc(tp,
			       &old,
			       list,
			       ifap,
			       (gw_entry *) 0,
			       BIT_TEST(flags, TARGETF_BROADCAST) ? tsi_size : 0,
			       dump);

	    /* Fill in the information */
	    tlp->target_dst = dest;
	    tlp->target_src = &ifap->ifa_addr_local;
	    tlp->target_gwp = (gw_entry *) 0;
	    if (tlp->target_ifap != ifap) {
		(void) ifa_alloc(ifap);
		if (tlp->target_ifap) {
		    (void) ifa_free(tlp->target_ifap);
		}
	    }
	    tlp->target_ifap = ifap;
	    BIT_RESET(tlp->target_flags, TARGETF_SOURCE);
	    BIT_SET(tlp->target_flags, TARGETF_BROADCAST);
	    if (BIT_TEST(flags, TARGETF_BROADCAST)) {
		/* Indicate we are active on this interface */

		BIT_SET(ifap->ifa_rtactive, RTPROTO_BIT(tp->task_rtproto));
	    }

	    /* And count it */
	    if (BIT_TEST(flags, TARGETF_BROADCAST)) {
		targets++;
	    }
	} IF_ADDR_END(ifap) ;
    }

    /* Then add the source gateways if any */
    if (BIT_TEST(flags, TARGETF_SOURCE)) {
	gw_entry *gwp;

	GW_LIST(gw_list, gwp) {
	    if (!BIT_TEST(gwp->gw_flags, GWF_SOURCE)) {
		continue;
	    }

	    if (!(ifap = if_withdst(gwp->gw_addr))) {
		if (BIT_TEST(task_state, TASKS_STRICTIFS)) {
		    trace(TR_ALL, LOG_INFO, "target_build: Ignoring source gateway %A not on attached net",
			  gwp->gw_addr);
		}
		continue;
	    }

	    if (!BIT_TEST(ifap->ifa_state, IFS_UP) ||
		BIT_TEST(ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOOUT)) {
		continue;
	    }

	    /* Look to see if this destination is the remote end of */
	    /* a P2P link I am already sending to */
	    TARGET_LIST(tlp, *list) {
		if (sockaddrcmp(gwp->gw_addr, *tlp->target_dst)) {
		    if (!BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
			/* Allocate a bit for this target */

			tlp->target_rtbit = rtbit_alloc(tp, tsi_size, (caddr_t) tlp, dump);

			/* Indicate we supply to this guy */
			BIT_SET(tlp->target_flags, TARGETF_SUPPLY);
		    }
		    tlp->target_gwp = gwp;
		    break;
		}
	    } TARGET_LIST_END(tlp, *list) ;

	    if (!tlp) {
		/* Locate old or allocate new */
		tlp = target_alloc(tp,
				   &old,
				   list,
				   ifap,
				   gwp,
				   tsi_size,
				   dump);

		/* Fill in the information */
		tlp->target_dst = &gwp->gw_addr;
		tlp->target_src = &ifap->ifa_addr_local;
		if (tlp->target_ifap != ifap) {
		    (void) ifa_alloc(ifap);
		    if (tlp->target_ifap) {
			(void) ifa_free(tlp->target_ifap);
		    }
		}
		tlp->target_ifap = ifap;
		tlp->target_gwp = gwp;
		BIT_RESET(tlp->target_flags, TARGETF_BROADCAST);
	    }

	    /* Indicate we are supplying to this gateway */
	    BIT_SET(tlp->target_flags, TARGETF_SOURCE);

	    /* Indicate we are active on this interface */
	    BIT_SET(ifap->ifa_rtactive, RTPROTO_BIT(tp->task_rtproto));

	    /* And count it */
	    targets++;
	    
	} GW_LISTEND;
    }

    /* Finally, free any remaining targets */
    if (old) {
	target_free(tp, old);
    }

    return targets;
}


/*
 *	Dump a target list
 */
void
target_dump __PF3(fp, FILE *,
		  list, target *,
		  bp, const bits *)
{
    target *tlp;

    if (list) {
	(void) fprintf(fp, "\n\tTargets:\n");
	
	TARGET_LIST(tlp, list) {
	    (void) fprintf(fp, "\t\t%-15A -> %-15A\tInterface: %A(%s)\n",
			   *tlp->target_src,
			   *tlp->target_dst,
			   tlp->target_ifap->ifa_addr,
			   tlp->target_ifap->ifa_name);
	    (void) fprintf(fp, "\t\t\tFlags: <%s>\n",
			   trace_bits2(bp, target_flag_bits, tlp->target_flags));
	    (void) fprintf(fp, "\t\t\tMaxsize: %u",
			   tlp->target_maxsize);
	    if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
		(void) fprintf(fp, "\tBit: %d",
			       tlp->target_rtbit);
	    }
	    (void) fprintf(fp, "\n\n");
	} TARGET_LIST_END(tlp, list) ;

	(void) fprintf(fp, "\n");
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
 */
