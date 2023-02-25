/* @(#)20	1.1  src/tcpip/usr/sbin/gated/rt_var.h, tcprouting, tcpip411, GOLD410 12/6/93 17:27:23 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		RTLIST_ADD
 *		rt_aggregate_brief
 *		rt_aggregate_entry
 *		rt_routers_compare
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
 * rt_var.h,v 1.11 1993/04/05 04:29:20 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* A number is allocated by each routing protocol per destination, one for */
/* link-state protocols, one per peer for peer/neighbor based protocols, */
/* and one per interface and sourcegateway for distance vector protocols. */

/* These bits index into the rt_bits structure in each rt_entry and into */
/* the rt_unreach array of unsigned chars in each rt_head.  The intended */
/* use is that a protocol will set it's bit when it decides to announce a */
/* route to a destination and reset it when it is nolonger announcing it. */
/* When a bit is reset the count of set bits in the rt_entry is */
/* decremented.  When this count reaches zero, the count of routes */
/* announcing a route to this destination is decremented.  If an rt_entry */
/* is scheduled for delete when it's count of announcement bits reaches */
/* zero it is released. */

/* The intended use of the rt_unreach array is that the protocol sets a */
/* value in it when a route to a destination goes into holddown and counts */
/* it down during periodic events (such as sending non-flash updates). */
/* When it's value reaches zero, the announcement bit in the held-down */
/* rt_entry is reset. */

/* Don't change this value here, change it in the config file */
#define	RTTSI_SIZE	16

typedef struct _rt_tsi {
    struct _rt_tsi *tsi_next;
    byte tsi_tsi[RTTSI_SIZE];
} rt_tsi;

typedef struct _rtbit_info {
    task	*task;		/* Task that owns this bit */
    void_t	data;		/* Task specific data */
    _PROTOTYPE(dump,
	       void,
	       (rt_head *,
		void_t,
		char *));	/* To display what it means */
    u_short	index;		/* Offset to bytes */
    u_short	length;		/* Number of bytes */
} rtbit_info;


#define	RT_TIMER_AGE	0	/* To age routing table entries */
#define	RT_TIMER_MAX	1


struct rtaf_info {
    u_int	rtaf_routes;		/* Count of rt_entrys */
    u_int	rtaf_dests;	       	/* Count of rt_heads */
    u_int	rtaf_actives;		/* Count of active rt_entrys */
    u_int	rtaf_holddowns;		/* Count of holddown rt_entrys */
    u_int	rtaf_hiddens;		/* Count of hidden rt_entrys */
    u_int	rtaf_deletes;		/* Count of deleted rt_entrys */

    /* XXX - Should have all the family specific routines in it */
};
extern struct rtaf_info rtaf_info[AF_MAX];


PROTOTYPE(rt_table_locate,
	  extern rt_head *,
	  (sockaddr_un *dst,
	   sockaddr_un *mask));
PROTOTYPE(rt_table_delete,
	  extern void,
	  (rt_head *));
PROTOTYPE(rt_table_add,
	  extern void,
	  (rt_head *rth));
PROTOTYPE(rt_table_init,
	  extern void,
	  (void));
PROTOTYPE(rt_table_dump,
	  extern void,
	  (task *,
	   FILE *));
#if	RT_N_MULTIPATH > 1
PROTOTYPE(rt_routers_compare,
	  extern int,
	  (rt_entry *,
	   sockaddr_un **));
#else	/* RT_N_MULTIPATH > 1 */
#define	rt_routers_compare(rt, routers)	sockaddrcmp(rt->rt_router, routers[0])
#endif	/* RT_N_MULTIPATH */

/* Add an entry to a change list */
#define RTLIST_ADD(rtl, rth) \
if (!rtl) { \
    rtl = (rt_list *) task_block_alloc(rtlist_block_index); \
    rtl->rtl_root = rtl; \
    rtl->rtl_fillp = rtl->rtl_entries; \
    *rtl->rtl_fillp = (void_t) rth; \
    rtl->rtl_root->rtl_count++; \
} else if (rtl->rtl_fillp < rtl->rtl_entries || (void_t) rth != *rtl->rtl_fillp) { \
    if (!rtl || (caddr_t) ++rtl->rtl_fillp == (caddr_t) rtl + RTL_SIZE) { \
	rtl->rtl_fillp--; \
        rtl->rtl_next = (rt_list *) task_block_alloc(rtlist_block_index); \
	rtl->rtl_next->rtl_root = rtl->rtl_root; \
	rtl = rtl->rtl_next; \
	rtl->rtl_fillp = rtl->rtl_entries; \
    } \
    *rtl->rtl_fillp = (void_t) rth; \
    rtl->rtl_root->rtl_count++; \
}

extern	rt_list	*rt_change_list;

extern int rt_n_changes;

#ifdef	PROTO_ISODE_SNMP
PROTOTYPE(rt_mib_init,
	  extern void,
	  (void));
#endif	/* PROTO_ISODE_SMUX */
#ifdef	PROTO_SNMP
PROTOTYPE(rt_mib_free_rt,
	  extern void,
	  (rt_entry *));
#endif	/* PROTO_SNMP */

/**/

/* Static routes */

PROTOTYPE(rt_static_cleanup,
	  extern void,
	  (task *));
PROTOTYPE(rt_static_reinit,
	  extern void,
	  (task *));
PROTOTYPE(rt_static_ifachange,
	  extern void,
	  (task *));
PROTOTYPE(rt_static_terminate,
	  extern void,
	  (task *));
PROTOTYPE(rt_static_dump,
	  extern void,
	  (task *,
	   FILE *));
PROTOTYPE(rt_static_init,
	  extern void,
	  (task *));

/**/

/* Aggregate routes */

#define	rt_aggregate_entry(aggr_rt)	((rt_aggr_entry *) aggr_rt->rt_data->rtd_data)
#define	rta_brief	rta_preference
#define rt_aggregate_brief(aggr_rt)	rt_aggregate_entry(aggr_rt)->rta_brief

PROTOTYPE(rt_aggregate_add,
	  extern void,
	  (rt_entry *));
PROTOTYPE(rt_aggregate_delete,
	  extern void,
	  (rt_entry *));
PROTOTYPE(rt_aggregate_change,
	  extern void,
	  (rt_entry *));
PROTOTYPE(rt_aggregate_cleanup,
	  extern void,
	  (task *));
PROTOTYPE(rt_aggregate_dump,
	  extern void,
	  (FILE *));
PROTOTYPE(rt_aggregate_dump_rt,
	  extern void,
	  (FILE *,
	   rt_entry *));
PROTOTYPE(rt_aggregate_list_add,
	  extern void,
	  (proto_t,
	   sockaddr_un *,
	   sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(rt_aggregate_reinit,
	  extern void,
	  (task *));
PROTOTYPE(rt_aggregate_init,
	  extern void,
	  (task *));
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
