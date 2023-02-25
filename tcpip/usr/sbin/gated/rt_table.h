/* @(#)72	1.6  src/tcpip/usr/sbin/gated/rt_table.h, tcprouting, tcpip411, GOLD410 12/6/93 17:59:28 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: AGGR_LIST
 *		AGGR_LIST_END
 *		PROTOTYPE
 *		RTBIT_CLR
 *		RTBIT_ISSET
 *		RTBIT_SET
 *		RTBIT_ZERO
 *		RTDATA_LIST
 *		RTDATA_LIST_END
 *		RTLIST_RESET
 *		RTPARMS_INIT
 *		RTPROTO_BIT
 *		RT_ALLRT
 *		RT_IFRT
 *		RT_LIST
 *		howmany
 *		rt_change
 *		rt_change_aspath
 *		rt_default_reset
 *		rt_refresh
 *		rtbit_isset
 *		rtbit_n_set
 *		rtd_link
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
 * rt_table.h,v 1.34.2.2 1993/08/27 22:28:38 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/*
 * rt_table.h
 *
 * Routing table data and parameter definitions.
 *
 */

/* Target specific information */

PROTOTYPE(rttsi_get, extern void, (rt_head *, u_int, byte *));
PROTOTYPE(rttsi_set, extern void, (rt_head *, u_int, byte *));
PROTOTYPE(rttsi_reset, extern void, (rt_head *, u_int));

/* Macros to support a bit per protocol in the routing structure */

/* Don't change this value here, change it in the config file */
#ifndef	RTBIT_SIZE
#define	RTBIT_SIZE	24
#endif	/* RTBIT_SIZE */

#ifndef	NBBY
#define	RTBIT_NBBY	8
#else	/* NBBY */
#define	RTBIT_NBBY	NBBY
#endif	/* NBBY */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef u_char rtbit_mask;

#define	RTBIT_NBITS	(sizeof(rtbit_mask) * RTBIT_NBBY)	/* bits per mask */

typedef	struct _rtbits {
    u_char	rtb_n_set;
    rtbit_mask	rtb_bits[howmany(RTBIT_SIZE, RTBIT_NBITS)];
} rtbits;

#define	RTBIT_SET(n, p)	((p)->rtb_bits[((n)-1)/RTBIT_NBITS] |= (1 << (((n)-1) % RTBIT_NBITS)))
#define	RTBIT_CLR(n, p)	((p)->rtb_bits[((n)-1)/RTBIT_NBITS] &= ~(1 << (((n)-1) % RTBIT_NBITS)))
#define	RTBIT_ISSET(n, p)	((p)->rtb_bits[((n)-1)/RTBIT_NBITS] & (1 << (((n)-1) % RTBIT_NBITS)))
#define	RTBIT_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#define	rtbit_isset(rt, bit)	RTBIT_ISSET(bit, &(rt)->rt_bits)
#define	rtbit_n_set(rt)	((int)((rt)->rt_bits.rtb_n_set))

/*
 * The number of multipath routes supported by the forwarding engine.
 */

#ifndef	RT_N_MULTIPATH
#define	RT_N_MULTIPATH	1
#endif	/* RT_N_MULTIPATH */

/* Structure used to indicate changes to a route */

typedef struct {
    flag_t	rtc_flags;
    int		rtc_n_gw;
    sockaddr_un	*rtc_routers[RT_N_MULTIPATH];
    struct _if_addr	*rtc_ifaps[RT_N_MULTIPATH];
    metric_t	rtc_metric;
    metric_t	rtc_tag;
#ifdef	PROTO_ASPATHS
    as_path	*rtc_aspath;
#endif	/* PROTO_ASPATHS */
} rt_changes;

#define	RTCF_NEXTHOP	BIT(0x01)	/* Next hop change */
#define	RTCF_METRIC	BIT(0x02)	/* Metric change */
#define	RTCF_ASPATH	BIT(0x04)	/* AS path change */
#define	RTCF_TAG	BIT(0x08)	/* Tag change */


/*
 *	Each rt_head entry contains a destination address and the root entry of
 *	a doubly linked lists of type rt_entry.  Each rt_entry contains
 *	information about how this occurance of a destination address was
 *	learned, next hop, ...
 *
 *	The rt_entry structure contains a pointer back to it's rt_head
 *	structure.
 */

/*
 *	Define link field as a macro.  These three fields must be in the same
 *	relative order in the rt_head and rt_entry structures.
 */
#define rt_link struct _rt_entry *rt_forw, *rt_back; struct _rt_head *rt_head

struct _rt_head {
    struct  _radix_node *rth_radix_node;	/* Tree glue and other values */
    sockaddr_un *rth_dest;			/* The destination */
    sockaddr_un *rth_dest_mask;			/* Subnet mask for this route */
    flag_t rth_state;				/* Global state bits */
    struct _rt_entry *rth_active;		/* Pointer to the active route */
    struct _rt_entry *rth_last_active;		/* Pointer to the previous	 active route */
    struct _rt_entry *rth_holddown;		/* Pointer to route in holddown */
    rt_changes *rth_changes;			/* Pointer to changes in active route */
    rt_link;					/* Routing table chain */
    struct _rt_tsi *rth_tsi;			/* Target specific information */
    byte rth_entries;				/* Number of routes for this destintation */
    byte rth_n_announce;			/* Number of routes with announce bits set */
};


#ifdef	ROUTE_AGGREGATION
/* Route aggregation structure */

typedef struct _rt_aggr_entry {
    struct _rt_aggr_entry *rta_forw;
    struct _rt_aggr_entry *rta_back;
    rt_entry *rta_rt;				/* Back pointer to this route */
    rt_entry *rta_aggr_rt;			/* Pointer to aggregate route */
    pref_t rta_preference;			/* Saved policy preference */
} rt_aggr_entry;

#define	AGGR_LIST(list, rta) \
    for (rta = (list)->rta_forw; rta != (list); rta = rta->rta_forw)

#define	AGGR_LIST_END(list, rta)

#endif	/* ROUTE_AGGREGATION */


/* Prefix of protocol independent data */
typedef struct _rt_data {
    struct _rt_data *rtd_forw;			/* Chain pointers */
    struct _rt_data *rtd_back;			/* ... */
    int rtd_refcount;				/* Reference count */
    size_t rtd_length;				/* Length of data (after this head) */
    _PROTOTYPE(rtd_dump,
	       void,
	       (FILE *,
		rt_entry *));			/* Routine to format data */
    _PROTOTYPE(rtd_free,
	       void,
	       (struct _rt_data *));		/* Routine to cleanup and free */
    void_t rtd_data;				/* Pointer to data (which follows this structure) */
} rt_data;

#define	rtd_link(rtd)	((rtd)->rtd_refcount++, (rtd))

struct _rt_entry {
    rt_link;					/* Chain and head pointers */
#define	rt_dest		rt_head->rth_dest	/* Route resides in rt_head */
#define	rt_dest_mask	rt_head->rth_dest_mask	/* Mask resides in rt_head */
#define	rt_active	rt_head->rth_active	/* Pointer to the active route */
#define	rt_holddown	rt_head->rth_holddown	/* Pointer to the route in holddown */
#define	rt_n_announce	rt_head->rth_n_announce
    int rt_n_gw;				/* Number of next hops */
    sockaddr_un *rt_routers[RT_N_MULTIPATH];	/* Next Hops */
#define	rt_router	rt_routers[0]		/* The first one */
    struct _if_addr *rt_ifaps[RT_N_MULTIPATH];	/* Interface to send said packets to */
#define	rt_ifap		rt_ifaps[0]		/* The first one */
    gw_entry *rt_gwp;				/* Gateway we learned this route from */
    time_t rt_timer;				/* Age of this route */
    metric_t rt_metric;				/* Interior metric of this route */
    metric_t rt_tag;				/* Route tag */
    flag_t rt_state;				/* Gated flags for this route */
    rt_data *rt_data;				/* Protocol specific data */
    rtbits rt_bits;				/* Protocol specific bits */
#define	rt_n_bitsset	rt_bits.rtb_n_set
    pref_t rt_preference;			/* Preference for this route */
#ifdef	ROUTE_AGGREGATION
    rt_aggr_entry *rt_aggregate;		/* Aggregate list entry */
#else	/* ROUTE_AGGREGATION */
    rt_entry *rt_aggregate;			/* Aggregate route */
#endif	/* ROUTE_AGGREGATION */
#ifdef	PROTO_ASPATHS
    struct _as_path *rt_aspath;			/* AS path for this route */
#endif	/* PROTO_ASPATHS */
};


/*
 * "State" of routing table entry.
 */
#define	RTS_ELIGIBLE		BIT(0x0)	/* Route is eligible to become active */
#define	RTS_NOAGE		BIT(0x01)	/* don't time out route */
#define	RTS_REMOTE		BIT(0x02)	/* route is for ``remote'' entity */
#define	RTS_REFRESH		BIT(0x04)	/* Route was refreshed, reset timer */
#define RTS_NOTINSTALL  	BIT(0x08)	/* don't install this route in kernel */
#define RTS_NOADVISE		BIT(0x10)	/* This route not to be advised */
#define RTS_INTERIOR    	BIT(0x20)	/* an interior route */
#define RTS_EXTERIOR    	BIT(0x40)	/* an exterior route */
#define	RTS_NETROUTE		RTS_INTERIOR|RTS_EXTERIOR
#define	RTS_HOLDDOWN		BIT(0x80)	/* Route is held down */
#define	RTS_DELETE		BIT(0x0100)	/* Route is deleted */
#define	RTS_HIDDEN		BIT(0x0200)	/* Route is present but not used because of policy */
#define	RTS_ACTIVE		BIT(0x0400)	/* Route is active */
#define	RTS_INITIAL		BIT(0x0800)	/* This route is being added */
#define	RTS_STATEMASK		(RTS_ACTIVE|RTS_DELETE|RTS_HIDDEN|RTS_INITIAL)
#define	RTS_RELEASE		BIT(0x1000)	/* This route is scheduled for release */
#define	RTS_FLASH		BIT(0x2000)	/* This route is scheduled for a flash */
#define	RTS_ONLIST		BIT(0x4000)	/* This route is on the flash list */
#define	RTS_RETAIN		BIT(0x8000)	/* This static route retained at shutdown */
#define	RTS_AGGREGATE		BIT(0x010000)	/* This is a aggregate route */
#define	RTS_GROUP		BIT(0x020000)	/* This is a multicast group */
#define	RTS_GATEWAY		BIT(0x040000)	/* This is not an interface route */
#define	RTS_REJECT		BIT(0x080000)	/* Send unreachable when trying to use this route */
#define	RTS_STATIC		BIT(0x100000)	/* Added by route command */
#define	RTS_BLACKHOLE		BIT(0x200000)	/* Silently drop packets to this net */
#define	RTS_IFSUBNETMASK	BIT(0x400000)	/* Subnet mask derived from interface */


#define RT_T_AGE	60		/* maximum time in seconds between route age increments. */
#define RT_T_EXPIRE	180		/* Age at which an interior route goes into holddown */
#define	RT_T_FREE	300		/* Length of time to keep an entry on the free list */


#define	RTPROTO_ANY		0	/* Matches any protocol */
#define RTPROTO_DIRECT		1	/* route is directly connected */
#define RTPROTO_KERNEL		2	/* route was installed in kernel when we started */
#define RTPROTO_REDIRECT	3	/* route was received via a redirect */
#define RTPROTO_DEFAULT		4	/* route is GATEWAY default */
#define	RTPROTO_OSPF		5	/* OSPF AS Internal routes */
#define	RTPROTO_OSPF_ASE	6	/* OSPF AS External routes */
#define	RTPROTO_IGRP		7	/* cisco IGRP */
#define RTPROTO_HELLO		8	/* DCN HELLO */
#define RTPROTO_RIP		9	/* Berkeley RIP */
#define	RTPROTO_BGP		10	/* Border gateway protocol */
#define RTPROTO_EGP		11	/* route was received via EGP */
#define	RTPROTO_STATIC		12	/* route is static */
#define	RTPROTO_SNMP		13	/* route was installed by SNMP */
#define	RTPROTO_IDPR		14	/* InterDomain Policy Routing */
#define	RTPROTO_ISIS		15	/* IS-IS */
#define	RTPROTO_SLSP		16	/* Simple Link State Protocol */
#define	RTPROTO_IDRP		17	/* InterDomain Routing Protocol */
#define	RTPROTO_INET		18	/* For INET specific stuff */
#define	RTPROTO_IGMP		19	/* For IGMP stuff */
#define	RTPROTO_AGGREGATE	20	/* Aggregate route */
#define	RTPROTO_DVMRP		21	/* Distance Vector Multicast Routing Protocol */
#define	RTPROTO_MAX		22	/* The number of protocols allocated */

#define	RTPROTO_BIT(proto)	((flag_t) (1 << ((proto) - 1)))
#define	RTPROTO_BIT_ANY		((flag_t) -1)

/*
 *	Preferences of the various route types
 */
#define	RTPREF_DIRECT		0	/* Routes to interfaces */
#define	RTPREF_OSPF		10	/* OSPF Internal route */
#define	RTPREF_ISIS_L1		15	/* IS-IS level 1 route */
#define	RTPREF_ISIS_L2		18	/* IS-IS level 2 route */
#define	RTPREF_DEFAULT		20	/* defaultgateway and EGP default */
#define	RTPREF_REDIRECT		30	/* redirects */
#define	RTPREF_KERNEL		40	/* learned via route socket */
#define	RTPREF_SNMP		50	/* route installed by network management */
#define	RTPREF_STATIC		60	/* Static routes */
#define	RTPREF_IGP		70	/* NSFnet backbone SPF */
#define	RTPREF_IGRP		80	/* Cisco IGRP */
#define	RTPREF_HELLO		90	/* DCN Hello */
#define	RTPREF_RIP		100	/* Berkeley RIP */
#define	RTPREF_DIRECT_AGGREGATE	110	/* P2P interface aggregate routes */
#define	RTPREF_DIRECT_DOWN	120	/* Routes to interfaces that are down */
#define	RTPREF_AGGREGATE	130	/* Aggregate default preference */
#define	RTPREF_OSPF_ASE		150	/* OSPF External route */
#define	RTPREF_IDPR		160	/* InterDomain Policy Routing */
#define	RTPREF_BGP_EXT		170	/* Border Gateway Protocol - external peer */
#define	RTPREF_EGP		200	/* Exterior Gateway Protocol */
#define	RTPREF_KERNEL_REMNANT	254	/* Routes in kernel at startup */


/* Structure with route parameters passed to rt_add. */

typedef struct {
    sockaddr_un *rtp_dest;
    sockaddr_un *rtp_dest_mask;
    int		rtp_n_gw;
    sockaddr_un *rtp_routers[RT_N_MULTIPATH];
#define	rtp_router	rtp_routers[0]
    gw_entry	*rtp_gwp;
    metric_t	rtp_metric;
    metric_t	rtp_tag;
    flag_t	rtp_state;
    pref_t	rtp_preference;
    rt_data	*rtp_rtd;
#ifdef	PROTO_ASPATHS
    as_path	*rtp_asp;
#endif	/* PROTO_ASPATHS */
} rt_parms;

#define	RTPARMS_INIT(n_gw, metric, state, preference) \
    { \
	(sockaddr_un *) 0, \
	(sockaddr_un *) 0, \
	n_gw, \
	{ (sockaddr_un *) 0 }, \
	(gw_entry *) 0, \
	metric, \
	(metric_t) 0, \
	state, \
	preference \
    }
											     

/* Macros to access the routing table - when the table format changes I */
/* just change these and everything works, right?  */

/*
 *	Change lists
 */

#define	RTL_SIZE	task_pagesize	/* Size of change list (one page) */

struct _rt_list {
    struct _rt_list *rtl_next;			/* Pointer to next on chain */
    struct _rt_list *rtl_root;			/* Pointer to root of list */
    void_t *rtl_fillp;				/* Pointer to last filled location */
    u_int rtl_count;				/* Number of entries on this list */	
    void_t rtl_entries[1];			/* Pointers to routes */
};


extern block_t rtlist_block_index;

/* Reset a list */
#define	RTLIST_RESET(rtl)	if (rtl) rtl = rtl->rtl_root; \
    while (rtl) { \
	register rt_list *rtln = rtl->rtl_next; \
	task_block_free(rtlist_block_index, (void_t) rtl); \
	rtl = rtln; \
    }

/* Scan a change list */
#define	RT_LIST(rth, rtl, type) \
    if (rtl) { \
	rt_list *rtl_root = rtl->rtl_root; \
	do { \
	    register void_t *rthp; \
	    for (rthp = (void_t *) rtl->rtl_entries; rthp <= rtl->rtl_fillp; rthp++) \
		if (rth = (type *) *rthp)

#define	RT_LIST_END(rth, rtl, type) \
	    rth = (type *) 0; \
	} while (rtl = rtl->rtl_next) ; \
	rtl = rtl_root; \
    }


/* Scan all routes for this destination */
#define	RT_ALLRT(rt, rth)	{ for (rt = (rth)->rt_forw; rt != (rt_entry *) &(rth)->rt_forw; rt = rt->rt_forw)
#define	RT_ALLRT_END(rt, rth)	if (rt == (rt_entry *) &(rth)->rt_forw) rt = (rt_entry *) 0; }

/* Only route in use for this destination */
#define	RT_IFRT(rt, rth)	if ((rt = rth->rt_holddown) || (rt = rth->rth_active))
#define	RT_IFRT_END

/*
 *	Macro to scan through a list of protocol specific data blocks
 */
#define	RTDATA_LIST(rtd, head) for (rtd = (head)->rtd_forw; rtd != (head); rtd = rtd->rtd_forw)

#define	RTDATA_LIST_END(rtd, head) if (rtd == (head)) rtd = (rt_data *) 0

#ifdef	ROUTE_AGGREGATION
/*
 *	Aggregate lists
 */

extern adv_entry *aggregate_list;	/* Aggregation policy */

#endif	/* ROUTE_AGGREGATION */

 /*  Macro implementation of rt_refresh.  If this is not defined, a	*/
 /*  function will be used.						*/
#define	rt_refresh(rt)	BIT_SET(rt->rt_state, RTS_REFRESH)

extern const bits rt_state_bits[];	/* Route state bits */
extern const bits rt_proto_bits[];	/* Protocol types */
extern struct _task *rt_task;
extern struct _timer *rt_timer;
extern gw_entry *rt_gw_list;		/* List of gateways for static routes */
extern gw_entry *rt_gwp;		/* Gateway structure for static routes */


PROTOTYPE(rt_init,
	  extern void,
	  (void));		/* Initialize routing table )*/
PROTOTYPE(rt_init_mib,
	  extern void,
	  (int));
PROTOTYPE(rt_open,
	  extern void,
	  (task * tp));		/* Open routing table for updating )*/
PROTOTYPE(rt_close,
	  extern void,
	  (task * tp,
	   gw_entry * gwp,
	   int changes,
	   char *message));	/* Signal completion of updates */
PROTOTYPE(rt_add,
	  extern rt_entry *,
	  (rt_parms *));
#ifdef	PROTO_ASPATHS
PROTOTYPE(rt_change_aspath,
	   extern rt_entry *,
	   (rt_entry *rt,
	    metric_t metric,
	    metric_t tag,
	    pref_t preference,
	    int n_gw,
	    sockaddr_un **gateway,
	    as_path *));
#define	rt_change(rt, m, t, p, n_gw, r)	rt_change_aspath(rt, m, t, p, n_gw, r, rt->rt_aspath)
#else	/* PROTO_ASPATHS */
PROTOTYPE(rt_change,
	   extern rt_entry *,
	   (rt_entry *rt,
	    metric_t metric,
	    metric_t tag,
	    pref_t preference,
	    int n_gw,
	    sockaddr_un **gateway));
#define	rt_change_aspath(rt, m, t, p, n_gw, r, a)	rt_change(rt, m, t, p, n_gw, r)
#endif	/* PROTO_ASPATHS */
PROTOTYPE(rt_delete,
	  extern void,
	  (rt_entry *));	/* Delete a route from the routing table */
PROTOTYPE(rt_flash,
	  extern void,
	  (rt_entry *));
PROTOTYPE(rt_flash_update,
	  extern void,
	  (int));
PROTOTYPE(rt_gwunreach,
	  extern int,
	  (task *,
	   gw_entry *));	/* Process an unreachable gateway */
PROTOTYPE(rtd_alloc,
	  extern rt_data *,
	  (size_t));	/* Allocate protocol specific data block */
PROTOTYPE(rtd_insert,
	  extern rt_data *,
	  (rt_data *,
	   rt_data *));	/* Same as locate given an rtd */
PROTOTYPE(rtd_locate,
	  extern rt_data *,
	  (void_t,
	   size_t,
	   rt_data *));	/* Locate or allocate a data block */
PROTOTYPE(rtd_unlink,
	  extern void,
	  (rt_data *));	/* Dereference a block */
PROTOTYPE(rt_lookup,
	  extern rt_entry *,
	  (flag_t,
	   flag_t,
	   sockaddr_un *dst,
	   flag_t));		/* Lookup a route the way the kernel would */
PROTOTYPE(rt_locate,
	  extern rt_entry *,
	  (flag_t,
	   sockaddr_un *,
	   sockaddr_un *,
	   flag_t));	/* Locate a route given dst, table and proto */
PROTOTYPE(rt_locate_gw,
	  extern rt_entry *,
	  (flag_t,
	   sockaddr_un *,
	   sockaddr_un *,
	   gw_entry *));						/* Locate a route given dst, table, proto and gwp */
PROTOTYPE(rtbit_alloc,
	  extern u_int,
	  (task *tp,
	   u_int,
	   void_t,
	   _PROTOTYPE(dump,
		      void,
		      (rt_head *,
		       void_t,
		       char *))));		/* Allocate a bit */
PROTOTYPE(rtbit_reset_all,
	  extern void,
	  (task *, u_int, gw_entry *));	/* Release any routes and free the bit */
PROTOTYPE(rthlist_active,
	  extern rt_list *,
	  (int family));		/* Get a list of active routes */
PROTOTYPE(rtlist_active,
	  extern rt_list *,
	  (int family));		/* Get a list of active routes */
PROTOTYPE(rtlist_all,
	  extern rt_list *,
	  (int family));		/* Get a list of all routes */
PROTOTYPE(rthlist_all,
	  extern rt_list *,
	  (int family));		/* Get a list of all routes */
PROTOTYPE(rtlist_gw,
	  extern rt_list *,
	  (gw_entry *gwp));		/* Get a complete list of routes by gateway */
PROTOTYPE(rtlist_proto,
	  extern rt_list *,
	  (int family,
	   flag_t proto));		/* Get a complete list of routes by protocol */
PROTOTYPE(rtbit_set,
	  extern void,
	  (rt_entry *, u_int));		/* Set an announcement bit */
PROTOTYPE(rtbit_reset,
	  extern rt_entry *,
	  (rt_entry *, u_int));		/* Reset and announcement bit */
PROTOTYPE(rt_table_init_family,
	  extern void,
	  (int af,
	   int offset,
	   sockaddr_un *,
	   sockaddr_un *));		/* Create a table for family */
#ifdef	ROTUE_AGGREGATION
PROTOTYPE(rt_aggregate_list_add,
	  extern void,
	  (proto_t,
	   sockaddr_un *,
	   sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(aggregate,
	  extern dest_mask *,
	  (struct _rt_entry *,
	   adv_entry *,
	   pref_t *));
PROTOTYPE(aggregate_dump,
	  extern void,
	  (FILE *));
#endif	/* ROUTE_AGGREGATION */
PROTOTYPE(rt_parse_route,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *,
	   adv_entry *,
	   adv_entry *,
	   pref_t,
	   flag_t,
	   char *));
#ifdef	PROTO_SNMP
PROTOTYPE(rt_table_getnext,
	  extern rt_head *,
	  (sockaddr_un *,
	   int,
	   flag_t,
	   flag_t));
#endif	/* PROTO_SNMP */


extern int rt_default_active;		/* TRUE if gateway default is active */
extern int rt_default_needed;		/* TRUE if gateway default is needed */
extern rt_parms rt_default_rtparms;

/* Delete pseudo default at termination time */
#define	rt_default_reset()	rt_default_active = 1; (void) rt_default_delete()

PROTOTYPE(rt_default_add,		/* Request installation of gateway default */
	  extern void,
	  (void));
PROTOTYPE(rt_default_delete,		/* Request deletion of gateway default */
	  extern void,
	  (void));



/* Redirects */
#define	REDIRECT_CONFIG_NOIN	1
#define	REDIRECT_CONFIG_MAX	2

extern flag_t redirect_trace_flags;	/* Trace flags from parser */
extern int redirect_n_trusted;		/* Number of trusted ICMP gateways */
extern pref_t redirect_preference;	/* Preference for ICMP redirects */
extern gw_entry *redirect_gw_list;	/* List of learned and defined ICMP gateways */
extern adv_entry *redirect_import_list;	/* List of routes that we can import */
extern adv_entry *redirect_int_policy;	/* List of interface policy */
extern gw_entry *redirect_gwp;		/* Gateway pointer for redirect routes */
PROTOTYPE(redirect,
	  extern void,
	  (task *,
	   sockaddr_un *,
	   sockaddr_un *,
	   sockaddr_un *,
	   sockaddr_un *,
	   int));	       	/* Process a redirect */
PROTOTYPE(redirect_init,
	  extern void,
	  (void));
PROTOTYPE(redirect_var_init,
	  extern void,
	  (void));
PROTOTYPE(redirect_disable,
	  extern void,
	  (proto_t));
PROTOTYPE(redirect_enable,
	  extern void,
	  (proto_t));


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
