static char sccsid[] = "@(#)71	1.8  src/tcpip/usr/sbin/gated/rt_table.c, tcprouting, tcpip411, GOLD410 7/8/94 15:16:48";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: _PROTOTYPE
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF6
 *		__PF7
 *		assert_noflash
 *		reset_active
 *		reset_delete
 *		reset_flash
 *		reset_hidden
 *		reset_holddown
 *		rt_check_open
 *		rt_error
 *		rtbit_alloc
 *		rtbit_free
 *		rtbit_reset_all
 *		set_active
 *		set_delete
 *		set_flash
 *		set_hidden
 *		set_holddown
 *		set_onlist
 *		set_release
 *		sizeof
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
 *  rt_table.c,v 1.53.2.10 1993/09/28 18:26:03 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_RT_VAR

#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"

const bits rt_state_bits[] =
{
    {RTS_NOAGE,			"NoAge"},
    {RTS_REMOTE,		"Remote"},
    {RTS_REFRESH,		"Refresh"},
    {RTS_NOTINSTALL,		"NotInstall"},
    {RTS_NOADVISE,		"NoAdvise"},
    {RTS_INTERIOR,		"Int"},
    {RTS_EXTERIOR,		"Ext"},
    {RTS_HOLDDOWN,		"HoldDown"},
    {RTS_DELETE,		"Delete"},
    {RTS_HIDDEN,		"Hidden"},
    {RTS_ACTIVE,		"Active"},
    {RTS_INITIAL,		"Initial"},
    {RTS_RELEASE,		"Release"},
    {RTS_FLASH,			"Flash"},
    {RTS_ONLIST,		"OnList"},
    {RTS_RETAIN,		"Retain"},
    {RTS_AGGREGATE,		"Aggregate"},
    {RTS_GROUP,			"Group"},
    {RTS_GATEWAY,		"Gateway"},
    {RTS_REJECT,		"Reject"},
    {RTS_STATIC,		"Static"},
    {RTS_BLACKHOLE,		"Blackhole"},
    {RTS_IFSUBNETMASK,		"IfSubnetMask"},
    {0}
};

const bits rt_proto_bits[] = {
    {RTPROTO_ANY,	"Any" },
    {RTPROTO_DIRECT,	"Direct"},
    {RTPROTO_KERNEL,	"Kernel"},
    {RTPROTO_REDIRECT,	"Redirect"},
    {RTPROTO_DEFAULT,	"Default"},
    {RTPROTO_OSPF,	"OSPF"},
    {RTPROTO_OSPF_ASE,	"OSPF_ASE"},
    {RTPROTO_IGRP,	"IGRP"},
    {RTPROTO_HELLO,	"HELLO"},
    {RTPROTO_RIP,	"RIP"},
    {RTPROTO_BGP,	"BGP"},
    {RTPROTO_EGP,	"EGP"},
    {RTPROTO_STATIC,	"Static"},
    {RTPROTO_SNMP,	"MIB"},
    {RTPROTO_IDPR,	"IDPR"},
    {RTPROTO_ISIS,	"IS-IS"},
    {RTPROTO_SLSP,	"SLSP"},
    {RTPROTO_IDRP,	"IDRP"},
    {RTPROTO_INET,	"INET"},
    {RTPROTO_IGMP,	"IGMP"},
    {RTPROTO_AGGREGATE,	"AGGREGATE"},
    {RTPROTO_DVMRP,	"DVMRP"},
    {0}
};

task *rt_task = (task *) 0;
timer *rt_timer = (timer *) 0;

struct rtaf_info rtaf_info[AF_MAX] = { { 0 } };

static task *rt_opentask = (task *) 0;	/* Protocol that has table open */

static block_t rt_block_index = (block_t) 0;		/* Block allocation index for an rt_entry */
static block_t rth_block_index = (block_t) 0;
static block_t rtchange_block_index = (block_t) 0;

#define	rt_check_open(p, name)	assert(rt_opentask)


/**/
static rtbit_info rtbit_map[RTBIT_SIZE];		/* Byte allocation table */
typedef u_short	rtbit_type;
static rtbit_type rttsi_map[RTBIT_SIZE * MAX(sizeof(u_long), sizeof(u_long *))/RTTSI_SIZE];	/* Bit allocation map */

static block_t rttsi_block_index;

/* Allocate a tsi field */
static void
rttsi_alloc  __PF1(ip, rtbit_info *)
{
    int i;
    rtbit_type mask0 = 0;

    /* Verify that the map size is the same as tsi blocks */
    assert(sizeof (rtbit_type) * NBBY == RTTSI_SIZE);

    /* Generate the mask we are looking for */
    for (i = 1; i <= ip->length; i++) {
	mask0 |= 1 << (RTTSI_SIZE - i);
    }

    /* Find a place where the mask fits */
    /* This will not find a mask that crosses an 8 byte boundry */
    for (i = 0; i < sizeof (rttsi_map) / sizeof (rtbit_type); i++) {
	rtbit_type mask = mask0;

	ip->index = i * RTTSI_SIZE;

	do {
	    if (!(rttsi_map[i] & mask)) {
		rttsi_map[i] |= mask;
		return;
	    }
	    ip->index++;
	} while (!(mask & 1) && (mask >>= 1));
    }

    assert(FALSE);	/* No bits available */
}

/* Set the tsi for a route */
void
rttsi_get __PF3(rth, rt_head *,
		bit, u_int,
		value, byte *)
{
    register rtbit_info *ip = &rtbit_map[bit-1];
    register u_int block = ip->index / RTTSI_SIZE;
    register rt_tsi *tsi = rth->rth_tsi;
    register int i = ip->length;

    while (tsi && block--) {
	tsi = tsi->tsi_next;
    }
    if (tsi) {
	byte *cp = &tsi->tsi_tsi[ip->index % RTTSI_SIZE];

	while (i--) {
	    *value++ = *cp++;
	}
    } else {
	while (i--) {
	    *value++ = (char) 0;
	}
    }
}


/* Get the tsi for a route */
void
rttsi_set __PF3(rth, rt_head *,
		bit, u_int,
		value, byte *)
{
    register rtbit_info *ip = &rtbit_map[bit-1];
    register u_int block = ip->index / RTTSI_SIZE;
    register rt_tsi *tsi = rth->rth_tsi;
    register int i = ip->length;
    register byte *cp;

    if (!tsi) {
	rth->rth_tsi = (rt_tsi *) task_block_alloc(rttsi_block_index);
	tsi = rth->rth_tsi;
    }
    while (block--) {
	if (!tsi->tsi_next) {
	    tsi->tsi_next = (rt_tsi *) task_block_alloc(rttsi_block_index);
	}
	tsi = tsi->tsi_next;
    }
    cp = &tsi->tsi_tsi[ip->index % RTTSI_SIZE];
    while (i--) {
	*cp++ = *value++;
    }
}


/* Reset the tsi for a route */
void
rttsi_reset __PF2(rth, rt_head *,
		  bit, u_int)
{
    register rtbit_info *ip = &rtbit_map[bit-1];
    register u_int block = ip->index / RTTSI_SIZE;
    register rt_tsi *tsi = rth->rth_tsi;
    register int i = ip->length;
    register byte *cp;

    if (!tsi) {
	return;
    }
    while (block--) {
	if (!tsi->tsi_next) {
	    return;
	}
	tsi = tsi->tsi_next;
    }
    cp = &tsi->tsi_tsi[ip->index % RTTSI_SIZE];
    while (i--) {
	*cp++ = (byte) 0;
    }
}


/* Free the TSI field */
static inline void
rttsi_release __PF1(release_rth, rt_head *)
{
    register rt_tsi *tsi = release_rth->rth_tsi;

    while (tsi) {
	register rt_tsi *otsi = tsi;

	tsi = tsi->tsi_next;

	task_block_free(rttsi_block_index, (void_t) otsi);
    }
}


static void
rttsi_free __PF1(ip, rtbit_info *)
{
    int i;
    rtbit_type mask = 0;

    for (i = 1; i <= ip->length; i++) {
	mask |= 1 << (RTTSI_SIZE - i);
    }

    rttsi_map[ip->index / RTTSI_SIZE] &= ~(mask >> (ip->index % RTTSI_SIZE));

    ip->index = ip->length = 0;
}


static void
rttsi_dump __PF2(fd, FILE *,
		 rth, rt_head *)
{
    int first = TRUE;
    u_int bit;
    char buf[LINE_MAX];

    for (bit = 1; bit <= RTBIT_SIZE; bit++) {
	if (rtbit_map[bit-1].dump) {
	    *buf = (char) 0;
	    rtbit_map[bit-1].dump(rth, rtbit_map[bit-1].data, buf);
	    if (*buf) {
		if (first) {
		    (void) fprintf(fd,
				   "\t\t\tTSI:\n");
		    first = FALSE;
		}
		(void) fprintf(fd,
			       "\t\t\t\t%s\n",
			       buf);
	    }
	}
    }
}



/**/
/*
 *	Remove an rt_head pointer.
 */
static void
rth_remove __PF1(remove_rth, rt_head *)
{
    rt_table_delete(remove_rth);

    /* Count this rt_head */
    rtaf_info[socktype(remove_rth->rth_dest)].rtaf_dests--;

    sockfree(remove_rth->rth_dest);

    rttsi_release(remove_rth);

    task_block_free(rth_block_index, (void_t) remove_rth);
}


/*
 *	Locate the rt_head pointer for this destination.  Create one if it does not exist.
 */
static inline rt_head *
rth_locate __PF4(locate_dst, sockaddr_un *,
		 locate_mask, sockaddr_un *,
		 locate_state, flag_t *,
		 locate_errmsg, const char **)
{
    rt_head *locate_rth = (rt_head *) 0;

    *locate_errmsg = (char *) 0;

    if (BIT_TEST(*locate_state, RTS_GROUP)) {
	assert(!locate_mask);
    } else {
	if (!locate_mask) {
	    *locate_errmsg = "mask not specified";
	    return (rt_head *) 0;
	}
	/* Locate proper mask */
	locate_mask = mask_locate(locate_mask);
    }

    /* Locate this entry in the table */
    locate_rth = rt_table_locate(locate_dst, locate_mask);
    if (locate_rth) {
	/* Existing route */

	if (locate_rth->rth_dest_mask != locate_mask) {
	    *locate_errmsg = "mask conflict";
	    return (rt_head *) 0;
	}
    } else {
	/* New route */
	
	locate_rth = (rt_head *) task_block_alloc(rth_block_index);

	/* Copy destination */
	locate_rth->rth_dest = sockdup(locate_dst);

	/* Clean up the address */
	sockclean(locate_rth->rth_dest);
	
	/* Set the mask */
	if (locate_mask) {
	    locate_rth->rth_dest_mask = locate_mask;
	}

	/* Count this rt_head */
	rtaf_info[socktype(locate_rth->rth_dest)].rtaf_dests++;

	if (BIT_TEST(*locate_state, RTS_GROUP)) {
	    BIT_SET(locate_rth->rth_state, RTS_GROUP);
	} else {
	    switch (socktype(locate_dst)) {
#ifdef	PROTO_INET
	    case AF_INET:
		if (sock2host(locate_rth->rth_dest, locate_rth->rth_dest_mask)) {
		    *locate_errmsg = "host bits not zero";
		    goto Return;
		} else {
		    sockaddr_un *natural_mask = inet_mask_natural(locate_rth->rth_dest);

		    if (locate_rth->rth_dest_mask != natural_mask
			&& mask_refines(locate_rth->rth_dest_mask, natural_mask)) {
			/* Make sure this is not the zero subnet */

			if (!(sock2ip(locate_rth->rth_dest) &
			      (sock2ip(locate_rth->rth_dest_mask) ^ sock2ip(natural_mask)))) {
			    *locate_errmsg = "zero subnet not allowed";
			    goto Return;
			}
		    }
		}
		break;
#endif	/* PROTO_INET */
	    }
	}

	locate_rth->rt_forw = locate_rth->rt_back = (rt_entry *) &locate_rth->rt_forw;
	locate_rth->rt_head = locate_rth;

	/* Add this entry to the table */
	rt_table_add(locate_rth);

    Return:
	if (*locate_errmsg) {
	    if (locate_rth->rth_dest) {
		sockfree(locate_rth->rth_dest);
	    }
	    task_block_free(rth_block_index, (void_t) locate_rth);
	    locate_rth = (rt_head *) 0;
	}
    }

    return locate_rth;
}


/**/
static inline void
rtchanges_free  __PF1(rtc_rth, rt_head *)
{
    rt_changes *rtc_rtcp = rtc_rth->rth_changes;

    if (!rtc_rtcp) {
	return;
    }
    
#ifdef	PROTO_ASPATHS
    if (BIT_TEST(rtc_rtcp->rtc_flags, RTCF_ASPATH) &&
	rtc_rtcp->rtc_aspath) {
	aspath_unlink(rtc_rtcp->rtc_aspath);
    }
#endif	/* PROTO_ASPATHS */

    if (BIT_TEST(rtc_rtcp->rtc_flags, RTCF_NEXTHOP)) {
	int i = rtc_rtcp->rtc_n_gw;

	/* Free the routers */
	while (i--) {
	    if (rtc_rtcp->rtc_routers[i]) {
		sockfree(rtc_rtcp->rtc_routers[i]);
		(void) ifa_free(rtc_rtcp->rtc_ifaps[i]);
	    }
	}
    }

    task_block_free(rtchange_block_index, (void_t) rtc_rtcp);

    rtc_rth->rth_changes = (rt_changes *) 0;
}


/**/
/* Free the interfaces from a route that was deleted */
/* This must be defered as the interface code may need */
/* to delete a route, but it can not because the routing */
/* table is open */
static void
rt_free_ifa __PF2(tp, task *,
		  data, void_t)
{
    if_addr *ifap = (if_addr *) data;

    (void) ifa_free(ifap);
}


/* Free a route */
static inline rt_entry *
rt_free  __PF1(free_rt, rt_entry *)
{
    if (free_rt) {
	register int free_i;
	register rt_head *free_rth = free_rt->rt_head;
	rt_entry *prev_rt = free_rt->rt_back;

#ifdef        PROTO_ASPATHS
	/* Free the AS path.  Do it before freeing anything else */
	if (free_rt->rt_aspath) {
	    aspath_rt_free(free_rt);
	}
#endif        /* PROTO_ASPATHS */
	
	if (free_rth) {
	    if (free_rt == free_rth->rth_last_active) {
		/* This is the last active route reset it */
		free_rth->rth_last_active = (rt_entry *) 0;

		if (free_rth->rth_changes) {
		    /* Clean up rt_change block */
		    rtchanges_free(free_rth);
		}
	    }
	    if (!free_rth->rth_entries) {
		if (!BIT_TEST(free_rth->rth_state, RTS_ONLIST)) {
		    /* This is not on a list OK to remove it now */

		    rth_remove(free_rth);
		    prev_rt = (rt_entry *) 0;
		}
	    }
	}

#ifdef	PROTO_SNMP
	/* Make sure the SNMP code does not have a cached pointer to this route */
	rt_mib_free_rt(free_rt);
#endif	/* PROTO_SNMP */

	/* Release any route specific data, remove the route from the change */
	/* and free this route */
	if (free_rt->rt_data) {
	    rtd_unlink(free_rt->rt_data);
	}

	for (free_i = 0; free_i < free_rt->rt_n_gw; free_i++) {
	    sockfree(free_rt->rt_routers[free_i]);
	    if (free_rt->rt_ifaps[free_i]) {
		if (free_rt->rt_ifaps[free_i]->ifa_refcount == 1) {
		    /* Defer this free until the end of the flash update */
		    /* as an interface route may be deleted and the routing */
		    /* table is still open */

		    task_crash_add(rt_task, rt_free_ifa, free_rt->rt_ifaps[free_i]);
	        } else {
		    /* Free it now */

		    (void) ifa_free(free_rt->rt_ifaps[free_i]);
		}
	    }
	}

	/* And finally free the block */
	task_block_free(rt_block_index, (void_t) free_rt);

	free_rt = prev_rt;
    }

    return free_rt;
}

/**/
/* Routing table state machine support routines */

/*
 * rt_trace() traces changes to the routing tables
 */
static inline void
rt_trace __PF2(t_rt, rt_entry *,
	       action, const char *)
{
    int pri = (t_rt->rt_gwp->gw_proto == RTPROTO_DIRECT) ? LOG_DEBUG : 0;

    /* XXX - Need indication of active and holddown */
    
    tracef("%-8s %-15A ",
	   action,
	   t_rt->rt_dest);
    if (t_rt->rt_dest_mask) {
	tracef(" %-15A ",
	       t_rt->rt_dest_mask);
    }

    switch (t_rt->rt_n_gw) {
	register int i;

    case 0:
	break;

    case 1:
	tracef("gw %-15A",
	       t_rt->rt_router);
	break;

    default:
	tracef("gw");
	for (i = 0; i < t_rt->rt_n_gw; i++) {
	    tracef("%c%A",
		   i ? ',' : ' ',
		   t_rt->rt_routers[i]);
	}

	break;
    }

    tracef(" %-8s pref %3d metric %d",
	   trace_state(rt_proto_bits, t_rt->rt_gwp->gw_proto),
	   t_rt->rt_preference,
	   t_rt->rt_metric);

    switch (t_rt->rt_n_gw) {
	register int i;

    case 0:
	break;

    default:
	for (i = 0; i < t_rt->rt_n_gw; i++) {
	    tracef("%c%s",
		   i ? ',' : ' ',
		   t_rt->rt_ifaps[i]->ifa_name);
	}
	break;
    }

    tracef(" <%s>",
	   trace_bits(rt_state_bits, t_rt->rt_state));
    if (t_rt->rt_gwp->gw_peer_as) {
	tracef("  as %d", t_rt->rt_gwp->gw_peer_as);
    }
    /* XXX - Format protocol specific information? */
    trace(TR_RT | TR_NOSTAMP, pri, NULL);

}


static const char *log_add = "ADD";
static const char *log_change = "CHANGE";
static const char *log_release = "RELEASE";

#define	set_delete(rt)		BIT_SET((rt)->rt_state, RTS_DELETE); \
    rtaf_info[socktype((rt)->rt_dest)].rtaf_deletes++
#define	reset_delete(rt)	BIT_RESET((rt)->rt_state, RTS_DELETE); \
    rtaf_info[socktype((rt)->rt_dest)].rtaf_deletes--

#define	set_release(rt)		BIT_SET((rt)->rt_state, RTS_RELEASE)

#define	set_holddown(rt) { \
	BIT_SET((rt)->rt_state, RTS_HOLDDOWN); \
	if (!(rt)->rt_holddown) { \
	    (rt)->rt_holddown = (rt); \
	    rtaf_info[socktype((rt)->rt_dest)].rtaf_holddowns++; \
	} \
    }
#define	reset_holddown(rt) { \
	BIT_RESET((rt)->rt_state, RTS_HOLDDOWN); \
	if ((rt)->rt_holddown == (rt)) { \
	    (rt)->rt_holddown = (rt_entry *) 0; \
	    rtaf_info[socktype((rt)->rt_dest)].rtaf_holddowns--; \
	} \
    }

#define	set_active(rt)		(rt)->rt_active = rt; \
    BIT_SET((rt)->rt_state, RTS_ACTIVE); \
    rtaf_info[socktype((rt)->rt_dest)].rtaf_actives++
#define	reset_active(rt)	(rt)->rt_active = (rt_entry *) 0; \
    BIT_RESET((rt)->rt_state, RTS_ACTIVE); \
    rtaf_info[socktype((rt)->rt_dest)].rtaf_actives--; \
    rtchanges_free(rt->rt_head)

#define	set_hidden(rt)		BIT_SET((rt)->rt_state, RTS_HIDDEN); \
    rtaf_info[socktype((rt)->rt_dest)].rtaf_hiddens++
#define	reset_hidden(rt)	BIT_RESET((rt)->rt_state, RTS_HIDDEN); \
    rtaf_info[socktype((rt)->rt_dest)].rtaf_hiddens--

#define	rt_error(name)		trace(TR_ALL, LOG_ERR, "rt_event_%s: fatal state error", name); task_quit(EINVAL)
#define	set_flash(rt)		BIT_SET((rt)->rt_head->rth_state, RTS_FLASH)
#define	reset_flash(rt)		BIT_RESET((rt)->rt_head->rth_state, RTS_FLASH)
#define	assert_noflash()	assert(!BIT_TEST(task_state, TASKS_FLASH|TASKS_NEWPOLICY))
#define	set_onlist(rt)		BIT_SET((rt)->rt_head->rth_state, RTS_ONLIST); RTLIST_ADD(rt_change_list, (rt)->rt_head)

/* Select the active route and return a pointer to it or a NULL pointer */

static inline rt_entry *
select_active  __PF1(select_rth, rt_head *)
{
    register rt_entry *select_rt = select_rth->rt_forw;
	
    if (select_rt == (rt_entry *) &select_rth->rt_forw) {
	/* No routes to become active */

	return (rt_entry *) 0;
    }

    if (BIT_TEST(select_rt->rt_state, RTS_DELETE|RTS_RELEASE|RTS_HIDDEN)) {
	/* This route is scheduled for delete, release or is hidden */

	return (rt_entry *) 0;
    }

    if (BIT_TEST(select_rt->rt_gwp->gw_flags, GWF_NEEDHOLD) &&
	(select_rth->rth_n_announce >
	 (select_rth->rth_active && select_rth->rth_active->rt_n_bitsset) + (select_rt->rt_n_bitsset != 0))) {
	/* This route honors holddowns and there is a route in holddown */

	return (rt_entry *) 0;
    }

    return select_rt;
}


/*
 *	Remove an rt_entry structure from the doubly linked list
 *	pointed to by it's rt_head
 */

static void
rt_remove  __PF1(remove_rt, rt_entry *)
{
    if (!--remove_rt->rt_head->rth_entries) {
	rtaf_info[socktype(remove_rt->rt_dest)].rtaf_routes--;
    }
    remque((struct qelem *) remove_rt);
}


/*	Insert an rt_entry structure in preference order in the doubly linked	*/
/*	list pointed to by it's rt_head.  If two routes with identical		*/
/*	preference are found, the one witht he shorter as path length is used.	*/
/*	If the as path lengths are the same, the route with the lower next-hop	*/
/*	IP address is prefered. This insures that the selection of the prefered	*/
/*	route is deterministic.							*/
static void
rt_insert __PF1(insert_rt, rt_entry *)
{
#define	rt_shouldskip(rt)	((rt)->rt_state & (RTS_DELETE|RTS_HIDDEN))
    rt_entry *insert_rt1;
    rt_head *insert_rth = insert_rt->rt_head;
    int rt_held = rt_shouldskip(insert_rt);

    RT_ALLRT(insert_rt1, insert_rth) {
	int rt1_held = rt_shouldskip(insert_rt1);
	
	if (rt_held && !rt1_held) {
	    /* Deleted routes go behind non-deleted routes */
	    continue;
	}
	if (rt1_held && !rt_held) {
	    /* non-deleted routes preceed deleted routes */
	    break;
	}
	if (insert_rt->rt_preference < insert_rt1->rt_preference) {
	    /* This preference is better */
	    break;
	} else if (insert_rt->rt_preference == insert_rt1->rt_preference) {
#ifdef	PROTO_ASPATHS
	    /* See if the AS path provides any hints about which route to use */
	    /* XXX Need to do something similar for ISO */
	    int path_which = aspath_prefer(insert_rt, insert_rt1);

	    if (path_which < 0) {
		break;
	    }
	    if (path_which == 0) {
#endif	/* PROTO_ASPATHS */
		if (insert_rt->rt_gwp->gw_proto == insert_rt1->rt_gwp->gw_proto
		    && insert_rt->rt_gwp->gw_peer_as == insert_rt1->rt_gwp->gw_peer_as) {
		    /* Same protocol and AS */

		    if (insert_rt->rt_metric < insert_rt1->rt_metric) {
			/* Use the lower metric */ 
			break;
		    }
		    if (insert_rt->rt_metric > insert_rt1->rt_metric) {
			/* Current one is more attractive */
			continue;
		    }
		}

		/* Default to comparing the router address to be deterministic */
		if (sockaddrcmp2(insert_rt->rt_router, insert_rt1->rt_router) < 0) {
		    /* This router address is lower, use it */
		    break;
		}
#ifdef	PROTO_ASPATHS
	    }
#endif	/* PROTO_ASPATHS */
	}
    } RT_ALLRT_END(insert_rt1, insert_rth);

    /* Insert prior to element if found, or behind the element at the end of a list. */
    /* For an empty list this ends up being behind the first element. */
    insque((struct qelem *) insert_rt, (struct qelem *) (insert_rt1 ? insert_rt1->rt_back : insert_rth->rt_back));

    if (!insert_rth->rth_entries++) {
	rtaf_info[socktype(insert_rt->rt_dest)].rtaf_routes++;
    }
}


static void
rt_event_active  __PF2(rt, rt_entry *,
		       log, int)
{
    
    switch (rt->rt_state & RTS_STATEMASK) {

    case RTS_INITIAL:
	rt_error("active");
	break;
	    
    case RTS_ELIGIBLE:
	set_active(rt);
	reset_holddown(rt);
	break;
	    
    case RTS_HIDDEN:
	rt_error("active");
	break;
	    
    case RTS_ACTIVE:
	break;
	    
    case RTS_DELETE:
	rt_error("active");
	break;
	    
    case RTS_HIDDEN|RTS_DELETE:
	rt_error("active");
	break;
	    
    default:
	rt_error("active");
    }
    
    set_flash(rt);
    
    if (log && BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_change);
    }
}


static void
rt_event_inactive __PF1(rt, rt_entry *)
{
    switch (rt->rt_state & RTS_STATEMASK) {
	
    case RTS_INITIAL:
	rt_error("inactive");
	break;
	    
    case RTS_ELIGIBLE:
    case RTS_HIDDEN:
	rt_error("inactive");
	break;
	    
    case RTS_ACTIVE:
	reset_active(rt);
	if (rt->rt_n_bitsset) {
	    set_holddown(rt);
	    set_flash(rt);
	}
	break;
	    
    case RTS_DELETE:
    case RTS_HIDDEN|RTS_DELETE:
    default:
	rt_error("inactive");
    }
    
    if (BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_change);
    }
}


static void
rt_event_preference __PF2(pref_rt, rt_entry *,
			  gateway_changed, int)
{
    rt_entry *new_active;
    
    switch (pref_rt->rt_state & RTS_STATEMASK) {

    case RTS_INITIAL:
	rt_error("preference");
	break;
	    
    case RTS_ELIGIBLE:
	rt_remove(pref_rt);
	if (pref_rt->rt_preference >= 0) {
	    rt_insert(pref_rt);
	    if (select_active(pref_rt->rt_head) == pref_rt) {
		if (pref_rt->rt_active) {
		    rt_event_inactive(pref_rt->rt_active);
		}
		rt_event_active(pref_rt, TRUE);
	    }
	} else {
	    set_hidden(pref_rt);
	    rt_insert(pref_rt);
	}	
	break;
	    
    case RTS_HIDDEN:
	rt_remove(pref_rt);
	if (pref_rt->rt_preference >= 0) {
	    reset_hidden(pref_rt);
	    rt_insert(pref_rt);
	    if (select_active(pref_rt->rt_head) == pref_rt) {
		if (pref_rt->rt_active) {
		    rt_event_inactive(pref_rt->rt_active);
		}
		rt_event_active(pref_rt, TRUE);
	    }
	} else {
	    rt_insert(pref_rt);
	}
	break;
	
    case RTS_ACTIVE:
	rt_remove(pref_rt);
	if (pref_rt->rt_preference >= 0) {
	    rt_insert(pref_rt);
	    if (new_active = select_active(pref_rt->rt_head)) {
		if (new_active != pref_rt) {
		    reset_active(pref_rt);
		    if (pref_rt->rt_n_bitsset) {
		        set_holddown(pref_rt);
		        set_flash(pref_rt);
		    }
		    if (new_active) {
		        rt_event_active(new_active, TRUE);
		    }
		} else if (gateway_changed) {
		    set_flash(pref_rt);
                }
	    }
	} else {
	    set_hidden(pref_rt);
	    reset_active(pref_rt);
	    if (pref_rt->rt_n_bitsset) {
		set_holddown(pref_rt);
		set_flash(pref_rt);
	    }
	    rt_insert(pref_rt);
	    if (new_active = select_active(pref_rt->rt_head)) {
		rt_event_active(new_active, TRUE);
	    }
	}
	break;
	    
    case RTS_DELETE:
	reset_delete(pref_rt);
	rt_remove(pref_rt);
	if (pref_rt->rt_preference >= 0) {
	    rt_insert(pref_rt);
	    if (select_active(pref_rt->rt_head) == pref_rt) {
		if (pref_rt->rt_active) {
		    rt_event_inactive(pref_rt->rt_active);
		}
		rt_event_active(pref_rt, TRUE);
	    }
	} else {
	    set_hidden(pref_rt);
	    rt_insert(pref_rt);
	}
	break;
	
    case RTS_HIDDEN|RTS_DELETE:
	rt_remove(pref_rt);
	reset_delete(pref_rt);
	if (pref_rt->rt_preference >= 0) {
	    reset_hidden(pref_rt);
	    rt_insert(pref_rt);
	    if (select_active(pref_rt->rt_head) == pref_rt) {
		if (pref_rt->rt_active) {
		    rt_event_inactive(pref_rt->rt_active);
		}
		rt_event_active(pref_rt, TRUE);
	    }
	} else {
	    rt_insert(pref_rt);
	}
	break;
	
    default:
	rt_error("preference");
    }	
    
    if (BIT_COMPARE(pref_rt->rt_head->rth_state, RTS_FLASH|RTS_ONLIST, RTS_FLASH)) {
	assert_noflash();
	set_onlist(pref_rt);
    }
    reset_flash(pref_rt);
    
    if (BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(pref_rt, log_change);
    }
}	


#ifdef	notdef
static void
rt_event_gateway __PF1(rt, rt_entry *)
{
    rt_entry *new_active;
    
    switch (rt->rt_state & RTS_STATEMASK) {
	
    case RTS_INITIAL:
	rt_error("gateway");
	break;
	
    case RTS_ELIGIBLE:
	rt_remove(rt);
	rt_insert(rt);
	if (select_active(rt->rt_head) == rt) {
	    if (rt->rt_active) {
		rt_event_inactive(rt->rt_active);
	    }
	    rt_event_active(rt, TRUE);
	}
	break;
	
    case RTS_HIDDEN:
	break;
	
    case RTS_ACTIVE:
	rt_remove(rt);
	rt_insert(rt);
	if ((new_active = select_active(rt->rt_head)) == rt) {
	    set_flash(rt);
	} else if (new_active) {
	    reset_active(rt);
	    rt_event_active(new_active, TRUE);
	}
	break;
	
    case RTS_DELETE:
	reset_delete(rt);
	rt_remove(rt);
	rt_insert(rt);
	if (select_active(rt->rt_head) == rt) {
	    if (rt->rt_active) {
		rt_event_inactive(rt->rt_active);
	    }
	    rt_event_active(rt, TRUE);
	}
	break;
	
    case RTS_HIDDEN|RTS_DELETE:
	reset_delete(rt);
	break;
	
    default:
	rt_error("gateway");
    }	
    
    if (BIT_COMPARE(rt->rt_head->rth_state, RTS_FLASH|RTS_ONLIST, RTS_FLASH)) {
	assert_noflash();
	set_onlist(rt);
    }
    reset_flash(rt);
    
    if (BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_change);
    }
}
#endif	/* notdef */


static void
rt_event_unreachable __PF1(rt, rt_entry *)
{
    const char *log_type = (const char *) 0;
    rt_entry *new_active;
    
    switch (rt->rt_state & RTS_STATEMASK) {
	
    case RTS_INITIAL:
	rt_error("unreachable");
	break;
	
    case RTS_ELIGIBLE:
	rt_remove(rt);
	if (rt->rt_n_bitsset) {
	    set_delete(rt);
	    rt_insert(rt);
	    log_type = log_change;
	} else {
	    set_release(rt);
	    log_type = log_release;
	}
	break;
	
    case RTS_HIDDEN:
	rt_remove(rt);
	if (rt->rt_n_bitsset) {
	    set_delete(rt);
	    rt_insert(rt);
	    log_type = log_change;
	} else {
	    reset_hidden(rt);
	    set_release(rt);
	    log_type = log_release;
	}
	break;
	
    case RTS_ACTIVE:
	rt_remove(rt);
	reset_active(rt);
	if (rt->rt_n_bitsset) {
	    set_holddown(rt);
	    set_flash(rt);
	    set_delete(rt);
	    rt_insert(rt);
	    log_type = log_change;
	} else {
	    set_release(rt);
	    log_type = log_release;
	}
	if (new_active = select_active(rt->rt_head)) {
	    rt_event_active(new_active, TRUE);
	}
	break;
	    
    case RTS_DELETE:
	rt_error("unreachable");	/* XXX - ignore */
	break;
	
    case RTS_HIDDEN|RTS_DELETE:
	rt_error("unreachable");	/* XXX - ignore */
	break;
	
    default:
	rt_error("unreachable");
    }	
    
    if (BIT_COMPARE(rt->rt_head->rth_state, RTS_FLASH|RTS_ONLIST, RTS_FLASH)) {
	assert_noflash();
	set_onlist(rt);
    }
    reset_flash(rt);
    
    if (log_type && BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_type);
    }
}	


#ifdef	notdef
static inline void
rt_event_change __PF1(rt, rt_entry *)
{
    switch (rt->rt_state & RTS_STATEMASK) {
	
    case RTS_INITIAL:
	rt_error("change");
	break;
	
    case RTS_ELIGIBLE:
    case RTS_HIDDEN:
	break;
	
    case RTS_ACTIVE:
	set_flash(rt);
	break;
	
    case RTS_DELETE:
    case RTS_HIDDEN|RTS_DELETE:
	reset_delete(rt);
	break;
	
    default:
	rt_error("change");
    }	
    
    if (BIT_COMPARE(rt->rt_head->rth_state, RTS_FLASH|RTS_ONLIST, RTS_FLASH)) {
	assert_noflash();
	set_onlist(rt);
    }
    reset_flash(rt);
    
    if (BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_change);
    }
}
#endif	/* notdef */


static inline void
rt_event_bit_set __PF2(rt, rt_entry *,
		       bit, u_int)
{
    RTBIT_SET(bit, &rt->rt_bits);
    
    switch (rt->rt_state & RTS_STATEMASK) {
	    
    case RTS_INITIAL:
    case RTS_ELIGIBLE:
    case RTS_HIDDEN:
	rt_error("bit_set");
	break;
	
    case RTS_ACTIVE:
	if (!rt->rt_n_bitsset++) {
	    rt->rt_head->rth_n_announce++;
	}
	break;
	
    case RTS_DELETE:
    case RTS_HIDDEN|RTS_DELETE:
    default:
	rt_error("bit_set");
    }
}


static inline void
rt_event_bit_reset  __PF2(rt, rt_entry *,
			  bit, u_int)
{
    const char *log_type = (const char *) 0;
    rt_entry *new_active = (rt_entry *) 0;
    
    RTBIT_CLR(bit, &rt->rt_bits);
    
    switch (rt->rt_state & RTS_STATEMASK) {
	
    case RTS_INITIAL:
	rt_error("bit_reset");
	break;
	
    case RTS_ELIGIBLE:
	log_type = log_change;
	/* Fall Through */
	
    case RTS_HIDDEN:
	if (!--rt->rt_n_bitsset) {
	    rt->rt_head->rth_n_announce--;
	    reset_holddown(rt);
	    rt_remove(rt);
	    rt_insert(rt);
	    if ((new_active = select_active(rt->rt_head)) && new_active != rt->rt_active) {
		rt_event_active(new_active, TRUE);
	    }
	}
	break;
	
    case RTS_ACTIVE:
	if (!--rt->rt_n_bitsset) {
	    rt->rt_head->rth_n_announce--;
	}
	break;
	
    case RTS_DELETE:
	if (!--rt->rt_n_bitsset) {
	    rt->rt_head->rth_n_announce--;
	    reset_holddown(rt);
	    set_release(rt);
	    rt_remove(rt);
	    if ((new_active = select_active(rt->rt_head)) && new_active != rt->rt_active) {
		rt_event_active(new_active, TRUE);
	    }
	    log_type = log_release;
	}
	break;
	
    case RTS_HIDDEN|RTS_DELETE:
	if (!--rt->rt_n_bitsset) {
	    rt->rt_head->rth_n_announce--;
	    reset_holddown(rt);
	    reset_hidden(rt);
	    set_release(rt);
	    rt_remove(rt);
	    if ((new_active = select_active(rt->rt_head)) && new_active != rt->rt_active) {
		rt_event_active(new_active, TRUE);
	    }
	    log_type = log_release;
	}
	break;
	
    default:
	rt_error("bit_reset");
    }		
    
    if (log_type && BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_type);
    }

    if (BIT_COMPARE(rt->rt_head->rth_state, RTS_FLASH|RTS_ONLIST, RTS_FLASH) &&
	!BIT_TEST(task_state, TASKS_NEWPOLICY)) {
	set_onlist(rt);
	rt_n_changes++;
    }
    reset_flash(rt);
}


static void
rt_event_initialize __PF1(rt, rt_entry *)
{
    switch (rt->rt_state & RTS_STATEMASK) {
    case RTS_INITIAL:
	BIT_RESET(rt->rt_state, RTS_INITIAL);
	if (rt->rt_preference >= 0) {
	    rt_insert(rt);
	    if (select_active(rt->rt_head) == rt) {
		if (rt->rt_active) {
		    rt_event_inactive(rt->rt_active);
		}
		rt_event_active(rt, FALSE);
	    }
	} else {
	    set_hidden(rt);
	    rt_insert(rt);
	}
	break;
	    
    case RTS_ELIGIBLE:
    case RTS_HIDDEN:
    case RTS_ACTIVE:
    case RTS_DELETE:
    case RTS_HIDDEN|RTS_DELETE:
    default:
	rt_error("initialize");
    }

    if (BIT_COMPARE(rt->rt_head->rth_state, RTS_FLASH|RTS_ONLIST, RTS_FLASH)) {
	assert_noflash();
	set_onlist(rt);
    }
    reset_flash(rt);
    
    if (BIT_TEST(trace_flags, TR_RT)) {
	rt_trace(rt, log_add);
    }
}


/*
 * Cause a flash update to happen
 */
void
rt_flash_update __PF1(flash, int)
{
    int crash = 1;
    const char *type;
    register rt_list *list = rt_change_list;

    if (list) {
	/* Reset the change list */
	rt_change_list = (rt_list *) 0;
	
	/* Get the root of the list */
	list = list->rtl_root;

	/* Update the kernel */
	trace(TR_RT, 0, NULL);
	trace(TR_RT, 0, "rt_flash_update: flash updating kernel with %d entries",
	      list->rtl_count);
	krt_flash(list);
    }

    if (flash) {
	type = "flash update";
    } else {
	register rt_head *rth;
	
	type = "new policy";

	/* Reset the short list */
	RT_LIST(rth, list, rt_head) {
	    /* Indicate no longer on list */

	    BIT_RESET(rth->rth_state, RTS_ONLIST);
	} RT_LIST_END(rth, list, rt_head) ;

	RTLIST_RESET(list);

	/* Get a full list */
	list = rthlist_all(AF_UNSPEC);

	/* Flag the routes as being on the list in case any are deleted */
	RT_LIST(rth, list, rt_head) {
	    BIT_SET(rth->rth_state, RTS_ONLIST);
	} RT_LIST_END(rth, list, rt_head) ;
    }
    
    if (list) {
	/* Update the  protocols */
	trace(TR_RT, 0, NULL);
	trace(TR_RT, 0, "rt_flash_update: %s started with %d entries",
	      type,
	      list->rtl_count);

	if (!BIT_TEST(task_state, TASKS_NOFLASH)) {
	    crash = task_flash(list, flash);
	}

	/* Make sure no one changed anything while we were flashing */
	assert(!rt_change_list);

	trace(TR_RT, 0, "rt_flash_update: %s ended with %d entries",
	      type,
	      list->rtl_count);
	trace(TR_RT, 0, NULL);

	if (list->rtl_count) {
	    register rt_head *rth;

	    RT_LIST(rth, list, rt_head) {
		/* Indicate no longer on list */
		BIT_RESET(rth->rth_state, RTS_ONLIST);

		if (!rth->rth_entries) {
		    /* Nothing in this head, free it */

		    rth_remove(rth);
		} else {
		    /* Free up change block */
		    if (rth->rth_changes) {
			/* Clean up rt_change block */
			rtchanges_free(rth);
		    }

		    /* Reset last active pointer */
		    rth->rth_last_active = rth->rth_active;
		}
	    } RT_LIST_END(rth, list, rt_head) ;
	}
		    
	/* And reset this list */
	RTLIST_RESET(list);
    }

    /* Now that we have done the flash, it's time for the crash */
    if (crash) {
	task_crash();

	if (rt_change_list && !flash) {
	    /* And follow with a flash update of any changes */

	    rt_flash_update(TRUE);
	}
    }
}

/**/
/* Allocate a bit for the protocol specific bit in the routing table */
u_int
rtbit_alloc(tp, size, data, dump)
task *tp;
u_int size;
void_t data;
_PROTOTYPE(dump,
	   void,
	   (rt_head *,
	    void_t,
	    char *));
{
    u_int bit;
    rtbit_info *ip = rtbit_map;

    for (bit = 1; bit <= RTBIT_SIZE; ip++, bit++) {
	if (!ip->task) {
	    break;
	}
    }

    assert(bit <= RTBIT_SIZE);

    /* Indicate this bit has been allocated */
    ip->task = tp;
    ip->data = data;
    ip->dump = dump;

    if (size) {
	ip->length = size;
	rttsi_alloc(ip);
    }

    return bit;
}

/* Free an allocated bit */
static void
rtbit_free(tp, bit)
task *tp;
u_int bit;
{
    register rtbit_info *ip = &rtbit_map[bit-1];

    assert(ip->task == tp);

    ip->task = (task *) 0;
    ip->data = (void_t) 0;
    ip->dump = 0;

    if (ip->length) {
	rttsi_free(ip);
    }
}


/* Set the announcement bit for this network */
void
rtbit_set __PF2(set_rt, rt_entry *,
		set_bit, u_int)
{
    rt_check_open(set_rt->rt_gwp->gw_proto, "rtbit_set");

    if (!RTBIT_ISSET(set_bit, &set_rt->rt_bits)) {
	rt_event_bit_set(set_rt, set_bit);
    }

#ifdef	RT_SANITY
    rt_sanity();
#endif	/* RT_SANITY */
    return;
}


/* Reset the announcement bit for this network */
rt_entry *
rtbit_reset __PF2(reset_rt, rt_entry *,
		  reset_bit, u_int)
{
    rt_check_open(reset_rt->rt_gwp->gw_proto, "rtbit_set");

    if (RTBIT_ISSET(reset_bit, &reset_rt->rt_bits)) {
	rt_event_bit_reset(reset_rt, reset_bit);

	if (BIT_TEST(reset_rt->rt_state, RTS_RELEASE)) {
	    reset_rt = rt_free(reset_rt);
	}
    }
    
#ifdef	RT_SANITY
    rt_sanity();
#endif	/* RT_SANITY */
    return reset_rt;
}

/* Reset the bits on all routes and free the bit */
void
rtbit_reset_all(tp, bit, gwp)
task *tp;
u_int bit;
gw_entry *gwp;
{
    int changes = 0;
    register rt_entry *rt;
    register rt_list *rtl = rtlist_all(AF_UNSPEC);
    
    rt_open(tp);

    RT_LIST(rt, rtl, rt_entry) {
	if (rtbit_isset(rt, bit)) {
	    changes++;

	    /* Clear the TSI field */
	    rttsi_reset(rt->rt_head, bit);

	    /* Reset this bit */
	    (void) rtbit_reset(rt, bit);
	}
    } RT_LIST_END(rt, rtl, rt_entry);

    RTLIST_RESET(rtl) ;
    
    rt_close(tp, gwp, changes, NULL);
    
    rtbit_free(tp, bit);
}


static void
rtbit_dump __PF1(fd, FILE *)
{
    u_int bit;
    rtbit_info *ip = rtbit_map;

    (void) fprintf(fd,
		   "\tBit allocations:\n");

    for (bit = 1; bit <= RTBIT_SIZE; ip++, bit++) {
	if (ip->task) {
	    (void) fprintf(fd,
			   "\t\t%d\t%s",
			   bit,
			   task_name(ip->task));
	    if (ip->length) {
		(void) fprintf(fd,
			       "\tbyte index: %d\tlength: %d",
			       ip->index,
			       ip->length);
	    }
	    (void) fprintf(fd, "\n");
	}
    }
    (void) fprintf(fd, "\n");
}


/**/

int rt_n_changes = 0;		/* Number of changes to routing table */

/*
 *	rt_open: Make table available for updating
 */
void
rt_open __PF1(tp, task *)
{
    assert(!rt_opentask);
    rt_opentask = tp;
    rt_n_changes = 0;
}


/*
 *	rt_close: Clean up after table updates
 */
void
rt_close __PF4(tp, task *,
	       gwp, gw_entry *,
	       changes, int,
	       message, char *)
{
    assert(rt_opentask == tp);

    rt_opentask = (task *) 0;
    if (rt_n_changes) {
	if (BIT_TEST(trace_flags, TR_RT)) {
	    tracef("rt_close: %d",
		   rt_n_changes);
	    if (changes) {
		tracef("/%d", changes);
	    }
	    tracef(" route%s proto %s",
		   rt_n_changes > 1 ? "s" : "",
		   task_name(tp));
	    if (gwp && gwp->gw_addr) {
		tracef(" from %A",
		       gwp->gw_addr);
	    }
	    if (message) {
		tracef(" %s",
		       message);
	    }
	    trace(TR_RT, 0, NULL);
	    trace(TR_RT, 0, NULL);
	}
	rt_n_changes = 0;
    }

    return;
}

/**/
/*
 * rt_gwunreach() deletes all exterior routes from the routing table for a
 * specified gateway
 */
int
rt_gwunreach __PF2(tp, task *,
		   gwp, gw_entry *)
{
    int changes = 0;
    register rt_entry *rt = (rt_entry *) 0;
    register rt_list *rtl = rtlist_gw(gwp);

    rt_open(tp);

    RT_LIST(rt, rtl, rt_entry) {
	if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
	    rt_delete(rt);
	    changes++;
	}
    } RT_LIST_END(rt, rtl, rt_entry);

    RTLIST_RESET(rtl) ;

    rt_close(tp, gwp, changes, NULL);

    return changes;
}


/**/

/* Looks up a destination network route with a specific protocol mask. */
/* Specifying a protocol of zero will match all protocols. */
rt_entry *
rt_locate __PF4(state, flag_t,
		dst, sockaddr_un *,
		mask, sockaddr_un *,
		proto_mask, flag_t)
{
    register rt_head *rth = rt_table_locate(dst, mask);

    if (rth) {
	register rt_entry *rt;

	RT_ALLRT(rt, rth) {
	    if (rt->rt_state & state & (RTS_NETROUTE|RTS_GROUP) &&
		BIT_TEST(proto_mask, RTPROTO_BIT(rt->rt_gwp->gw_proto))) {
		return rt;
	    }
	} RT_ALLRT_END(rt, rth);
    }

    return (rt_entry *) 0;
}


/* Look up a route with a destination address, protocol and source gateway */
rt_entry *
rt_locate_gw __PF4(state, flag_t,
		   dst, sockaddr_un *,
		   mask, sockaddr_un *,
		   gwp, gw_entry *)
{
    register rt_head *rth = rt_table_locate(dst, mask);

    if (rth) {
	register rt_entry *rt;
	
	RT_ALLRT(rt, rth) {
	    if (rt->rt_state & state & (RTS_NETROUTE|RTS_GROUP) &&
		(rt->rt_gwp == gwp)) {
		return rt;
	    }
	} RT_ALLRT_END(rt, rth);
    }

    return (rt_entry *) 0;
}


#if	RT_N_MULTIPATH > 1
/*
 *	Do a linear sort of the routers to get them in ascending order
 */
static inline void
rt_routers_sort __PF2(routers, sockaddr_un **,
		      nrouters, int)
{
    register int i, j;
    
    for (i = 0; i < (nrouters-1); i++) {
	for (j = i+1; j < nrouters; j++) {
	    if (sockaddrcmp2(routers[i], routers[j]) > 0) {
		register sockaddr_un *swap = routers[i];

		routers[i] = routers[j];
		routers[j] = swap;
	    }
	}
    }
}

int
rt_routers_compare __PF2(rt, rt_entry *,
			 routers, sockaddr_un **)
{
    register int i = rt->rt_n_gw;

    /* Get them into the same order */
    rt_routers_sort(routers, i);
		
    while (i--) {
	if (!sockaddrcmp(routers[i], rt->rt_routers[i])) {
	    /* Found one that was different */
	    return FALSE;
	}
    }

    return TRUE;
}
#endif	/* RT_N_MULTIPATH > 1 */


/*  Add a route to the routing table after some checking.  The route	*/
/*  is added in preference order.  If the active route changes, the	*/
/*  kernel routing table is updated.					*/
rt_entry *
rt_add __PF1(pp, register rt_parms *)
{
    int i;
    const char *errmsg = (char *) 0;
    flag_t level = TR_RT;
    int pri = 0;
    rt_entry *rt = (rt_entry *) 0;

    rt_check_open(pp->rtp_gwp->gw_proto, "rt_add");

    /* Allocate an entry */
    rt = (rt_entry *) task_block_alloc(rt_block_index);

    /* If this is a martian net it doesn't get into MY tables */
    if (is_martian(pp->rtp_dest)
	&& !BIT_TEST(pp->rtp_state, RTS_NOADVISE)) {
	/* It's a martian!  Make sure it does not get used */

	BIT_SET(rt->rt_state, RTS_NOTINSTALL|RTS_NOADVISE);
	errmsg = "MARTIAN will not be propagated";
    }

    /* Locate the head for this entry */
    rt->rt_head = rth_locate(pp->rtp_dest, pp->rtp_dest_mask, &pp->rtp_state, &errmsg);
    if (errmsg) {
	level = TR_ALL;
	pri = LOG_WARNING;
	goto Error;
    }

    for (i = 0; i < pp->rtp_n_gw; i++) {
	rt->rt_routers[i] = sockdup(pp->rtp_routers[i]);

	/* Clean up the address */
	switch (socktype(rt->rt_routers[i])) {
#ifdef	PROTO_INET
	case AF_INET:
	    sock2port(rt->rt_routers[i]) = 0;
	    break;
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
	case AF_ISO:
	    /* XXX - What do we need here? */
	    break;
#endif	/* PROTO_ISO */
	}
    }
    rt->rt_n_gw = pp->rtp_n_gw;
#if	RT_N_MULTIPATH > 1
    rt_routers_sort(rt->rt_routers, rt->rt_n_gw);
#endif	/* RT_N_MULTIPATH > 1 */

    rt->rt_gwp = pp->rtp_gwp;
    rt->rt_metric = pp->rtp_metric;
    rt->rt_tag = pp->rtp_tag;
    rt->rt_timer = 0;
    rt->rt_state = pp->rtp_state | RTS_REFRESH | RTS_INITIAL | (rt->rt_head->rth_state & ~(RTS_ONLIST));
    rt->rt_preference = pp->rtp_preference;
    if (!BIT_MATCH(rt->rt_state, rt->rt_head->rth_state)) {
	/* XXX - this route does not match */
    }

    /* Set the gateway flags */
    if (BIT_TEST(rt->rt_state, RTS_EXTERIOR)) {
	BIT_SET(rt->rt_state, RTS_GATEWAY);
    } else {
	switch (rt->rt_gwp->gw_proto) {
	    case RTPROTO_KERNEL:
	    case RTPROTO_STATIC:
	    case RTPROTO_DIRECT:
		break;

	    default:
		BIT_SET(rt->rt_state, RTS_GATEWAY);
		break;
	}
    }

    for (i = 0; i < pp->rtp_n_gw; i++) {
	rt->rt_ifaps[i] = ifa_alloc(if_withroute(rt->rt_dest, rt->rt_routers[i], rt->rt_state));
	if (!rt->rt_ifaps[i]) {
	    /* XXX - which interface */
	    errmsg = "interface not found for";
	    level = TR_ALL;
	    pri = LOG_WARNING;
	    goto Error;
	}

	/* If this is not an interface route, ignore routes to the destination(s) of this interface */
	if (BIT_TEST(rt->rt_state, RTS_GATEWAY) &&
	    !BIT_MATCH(rt->rt_state, RTS_NOTINSTALL|RTS_NOADVISE|RTS_REJECT) &&
	    rt->rt_ifaps[i] == if_withmyaddr(rt->rt_dest) &&
	    (!BIT_TEST(rt->rt_ifaps[i]->ifa_state, IFS_POINTOPOINT)
	     || !sockaddrcmp(rt->rt_dest, rt->rt_ifaps[i]->ifa_addr_local))) {
	    /* Make it unusable */

	    BIT_SET(rt->rt_state, RTS_NOTINSTALL|RTS_NOADVISE);
	    if (rt->rt_preference > 0) {
		rt->rt_preference = -rt->rt_preference;
	    }
#ifdef	notdef
	    /* XXX - All protocols should do this test so we don't have to */
	    errmsg = "ignoring route to interface";
	    goto Error;
#endif	/* notdef */
	}
    }

    if (pp->rtp_rtd) {
	rt->rt_data = rtd_link(pp->rtp_rtd);
    }
    
    rt_n_changes++;

#ifdef	PROTO_ASPATHS
    /* Create an AS path for this route */
    aspath_rt_build(rt, pp->rtp_asp);
#endif	/* PROTO_ASPATHS */

#ifdef	ROUTE_AGGREGATION
    /* See if we need to create any aggregate routes */
    if (pp->rtp_preference >= 0) {
	rt_aggregate_add(rt);
    }
#endif	/* ROUTE_AGGREGATION */
    
    rt_event_initialize(rt);

 Error:
    if (errmsg) {
	tracef("rt_add: %s ",
	       errmsg);
	if (BIT_TEST(pp->rtp_state, RTS_GROUP)) {
	    tracef("group %A",
		   pp->rtp_dest);
	} else {
	    tracef("%A",
		   pp->rtp_dest);
	    if (pp->rtp_dest_mask) {
		tracef("/%A",
		       pp->rtp_dest_mask);
	    }
	}
	tracef(" gw");
	for (i = 0; i < pp->rtp_n_gw; i++) {
	    tracef("%c%A",
		   i ? ',' : ' ',
		   pp->rtp_routers[i]);
	}

	if (pp->rtp_gwp->gw_addr && (pp->rtp_n_gw > 1 || !sockaddrcmp(pp->rtp_routers[0], pp->rtp_gwp->gw_addr))) {
	    tracef(" from %A",
		   pp->rtp_gwp->gw_addr);
	}
	
	tracef(" %s",
	       trace_state(rt_proto_bits, pp->rtp_gwp->gw_proto));
	if (pp->rtp_gwp->gw_peer_as) {
	    tracef(" AS %d",
	       pp->rtp_gwp->gw_peer_as);
	}

	trace(level, pri, NULL);

	if (rt) {
	    (void) rt_free(rt);
	    rt = (rt_entry *) 0;
	}
    }

#ifdef	RT_SANITY
    rt_sanity();
#endif	/* RT_SANITY */
    
    return rt;
}



 /* rt_change() changes a route &/or notes that an update was received.	*/
 /* returns 1 if change made.  Updates the kernel's routing table if	*/
 /* the router has changed, or a preference change has made another	*/
 /* route active							*/
rt_entry *
#ifdef	PROTO_ASPATHS
rt_change_aspath __PF7(rt, rt_entry *,
		       metric, metric_t,
		       tag, metric_t,
		       preference, pref_t,
		       n_gw, int,
		       gateway, sockaddr_un **,
		       asp, as_path *)
#else	/* PROTO_ASPATHS */
rt_change __PF6(rt, rt_entry *,
		metric, metric_t,
		tag, metric_t,
		preference, pref_t,
		n_gw, int,
		gateway, sockaddr_un **)
#endif	/* PROTO_ASPATHS */
{
    rt_changes *rtcp;
    int gateway_changed = FALSE;
    int preference_changed = FALSE;
    flag_t old_rtc_flags = 0;

    /* XXX - how to do aggregate change? */

    BIT_SET(rt->rt_state, RTS_REFRESH);	/* ensures route age reset */

    /* Allocate a change block if necessary */
    if (rt == rt->rt_head->rth_last_active
	&& rt->rt_n_bitsset) {
	rtcp = rt->rt_head->rth_changes;
	if (!rtcp) {
	    rtcp = rt->rt_head->rth_changes = (rt_changes *) task_block_alloc(rtchange_block_index);
	} else {
	    old_rtc_flags = rtcp->rtc_flags;
	}
    } else {
	rtcp = (rt_changes *) 0;
    }

    if (n_gw) {
	int should_copy = rtcp && !BIT_TEST(rtcp->rtc_flags, RTCF_NEXTHOP);
	if_addr *ifap;
#if	RT_N_MULTIPATH > 1
	sockaddr_un **routers = gateway;
	sockaddr_un *sorted_routers[RT_N_MULTIPATH];
	if_addr *sorted_ifaps[RT_N_MULTIPATH];
	register int i;

	if (n_gw > 1) {
	    for (i = 0; i < n_gw; i++) {
		sorted_routers[i] = *gateway++;
	    }
	    rt_routers_sort(sorted_routers, n_gw);
	    routers = sorted_routers;
	}

	/*
	 * Check to see if anything has changed
	 */
	if ((i = n_gw) == rt->rt_n_gw) {
	    while (i--) {
		if (!sockaddrcmp(rt->rt_routers[i], routers[i])) {
		    break;
		}
	    }
	}

	/*
	 * Copy his routers.  If we need to save them, do that too.
	 */
	if (i >= 0) {
	    /* Compute the ifap's for the new routes */
	    for (i = 0; i < n_gw; i++) {
		sorted_routers[i] = sockdup(*routers);
		sockclean(sorted_routers[i]);
		routers++;
		ifap = if_withroute(rt->rt_dest, sorted_routers[i], rt->rt_state);
		if (!ifap) {
		    int j;

		    trace(TR_ALL, LOG_WARNING, "rt_change: interface not found for net %-15A gateway %A",
			  rt->rt_dest,
			  sorted_routers[i]);
		    for (j = 0; j <= i; j++) {
			sockfree(sorted_routers[j]);
		    }
		    return (rt_entry *) 0;
		}
		sorted_ifaps[i] = ifa_alloc(ifap);
	    }

	    for (i = 0; i < n_gw; i++) {
		if (i < rt->rt_n_gw) {
		    if (should_copy) {
			rtcp->rtc_routers[i] = rt->rt_routers[i];
			rtcp->rtc_ifaps[i] = rt->rt_ifaps[i];
		    } else {
			sockfree(rt->rt_routers[i]);
			(void) ifa_free(rt->rt_ifaps[i]);
		    }
		}
		rt->rt_routers[i] = sorted_routers[i];
		rt->rt_ifaps[i] = sorted_ifaps[i];
	    }
	    for ( ; i < rt->rt_n_gw; i++) {
		if (should_copy) {
		    rtcp->rtc_routers[i] = rt->rt_routers[i];
		    rtcp->rtc_ifaps[i] = rt->rt_ifaps[i];
		} else {
		    sockfree(rt->rt_routers[i]);
		    (void) ifa_free(rt->rt_ifaps[i]);
		}
		rt->rt_routers[i] = NULL;
	    }
	    if (should_copy) {
		rtcp->rtc_n_gw = rt->rt_n_gw;
		BIT_SET(rtcp->rtc_flags, RTCF_NEXTHOP);
	    } else if (rtcp) {
		/*
		 * Check to see if we have changed the next hop back to what it was
		 * previously.  If so, delete the change information.
		 */
		if (rtcp->rtc_n_gw == n_gw) {
		    for (i = 0; i < n_gw; i++) {
			if (!sockaddrcmp(rtcp->rtc_routers[i], rt->rt_routers[i])) {
			    break;
			}
		    }
		    if (i == n_gw) {
			/*
			 * Same as before, delete change info.
			 */
			BIT_RESET(rtcp->rtc_flags, RTCF_NEXTHOP);
			for (i = 0; i < n_gw; i++) {
			    sockfree(rtcp->rtc_routers[i]);
			    (void) ifa_free(rtcp->rtc_ifaps[i]);
			    rtcp->rtc_routers[i] = NULL;
			}
			rtcp->rtc_n_gw = 0;
		    }
		}
	    }
	    rt->rt_n_gw = n_gw;
	    gateway_changed = TRUE;
	}
#else	/* RT_N_MULTIPATH == 1 */
	if (!sockaddrcmp(rt->rt_router, *gateway)) {
	    sockaddr_un *newrouter;

	    /* XXX - We check for interface change only in this case? */

	    newrouter = sockdup(*gateway);
	    sockclean(newrouter);
	    ifap = if_withroute(rt->rt_dest, newrouter, rt->rt_state);
	    if (!ifap) {
		trace(TR_ALL, LOG_WARNING, "rt_change: interface not found for net %-15A gateway %A",
		      rt->rt_dest,
		      newrouter);
		return (rt_entry *) 0;
	    }
	    ifa_alloc(ifap);
	    if (should_copy) {
		rtcp->rtc_routers[0] = rt->rt_router;
		rtcp->rtc_ifaps[0] = rt->rt_ifap;
		rtcp->rtc_n_gw = 1;
		BIT_SET(rtcp->rtc_flags, RTCF_NEXTHOP);
	    } else {
		sockfree(rt->rt_router);
		(void) ifa_free(rt->rt_ifap);
		if (rtcp) {
		    /*
		     * If the new router is the same as the changed version, deleted the changed.
		     */
		    if (sockaddrcmp(rt->rt_router, rtcp->rtc_routers[0])) {
			BIT_RESET(rtcp->rtc_flags, RTCF_NEXTHOP);
			sockfree(rtcp->rtc_routers[0]);
			(void) ifa_free(rtcp->rtc_ifaps[0]);
			rtcp->rtc_routers[0] = (sockaddr_un *) 0;
			rtcp->rtc_ifaps[0] = (if_addr *) 0;
			rtcp->rtc_n_gw = 0;
		    }
		}
	    }
	    rt->rt_router = newrouter;
	    rt->rt_ifap = ifap;
	    gateway_changed = TRUE;
	}
#endif	/* RT_N_MULTIPATH */
    }

    if (preference != rt->rt_preference) {
	rt->rt_preference = preference;
	preference_changed = TRUE;
    }

    if (metric != rt->rt_metric) {
	if (rtcp) {
	    if (!BIT_TEST(rtcp->rtc_flags, RTCF_METRIC)) {
		BIT_SET(rtcp->rtc_flags, RTCF_METRIC);
		rtcp->rtc_metric = rt->rt_metric;
	    } else if (rtcp->rtc_metric == metric) {
		BIT_RESET(rtcp->rtc_flags, RTCF_METRIC);
	    }
	}
	rt->rt_metric = metric;
	gateway_changed = TRUE;
    }

    if (tag != rt->rt_tag) {
	if (rtcp) {
	    if (!BIT_TEST(rtcp->rtc_flags, RTCF_TAG)) {
		BIT_SET(rtcp->rtc_flags, RTCF_TAG);
		rtcp->rtc_tag = rt->rt_tag;
	    } else if (rtcp->rtc_tag == tag) {
		BIT_RESET(rtcp->rtc_flags, RTCF_TAG);
	    }
	}
	rt->rt_tag = tag;
	gateway_changed = TRUE;
    }

#ifdef	PROTO_ASPATHS
    if (asp != rt->rt_aspath) {
	/* Path has changed */

	if (rtcp) {
	    if (!BIT_TEST(rtcp->rtc_flags, RTCF_ASPATH)) {
		/* Save the path, don't unlink it */
		rtcp->rtc_aspath = rt->rt_aspath;
		BIT_SET(rtcp->rtc_flags, RTCF_ASPATH);
		rt->rt_aspath = (as_path *) 0;
	    } else if (asp == rtcp->rtc_aspath) {
		/* AS path changed back.  Unlink the route's AS path then */
		/* simply transfer the changed path back to the route */
		/* This assumes an INTERNAL_IGP route will never have been active */
		if (rt->rt_aspath) {
		    aspath_rt_free(rt);
		}
		rt->rt_aspath = asp;
		rtcp->rtc_aspath = NULL;
		BIT_RESET(rtcp->rtc_flags, RTCF_ASPATH);
	    }
	}

	if (rtcp == NULL || BIT_TEST(rtcp->rtc_flags, RTCF_ASPATH)) {
	    if (rt->rt_aspath) {
		aspath_rt_free(rt);
	    }
	    aspath_rt_build(rt, asp);
	}
	if (!BIT_TEST(rt->rt_state, RTS_DELETE|RTS_HOLDDOWN)) {
	    gateway_changed = TRUE;
	}
    }
#endif	/* PROTO_ASPATHS */

#ifdef	ROUTE_AGGREGATION
    if (gateway_changed || preference_changed) {
	/* Need to update aggregate information */

	if (preference_changed && preference < 0) {
	    if (rt->rt_aggregate) {
		/* No longer valid to contribute to an aggregate */
		rt_aggregate_delete(rt);
	    }
	} else {
	    /* Existing aggregate */

	    rt_aggregate_change(rt);
	}
    }
#endif	/* ROUTE_AGGREGATION */
    
    if (gateway_changed || preference_changed
	|| BIT_TEST(rt->rt_state, RTS_DELETE)) {
	rt_event_preference(rt, gateway_changed);
	rt_n_changes++;
    }

    if (rtcp && !rtcp->rtc_flags) {
	/* No changes - release */
	rtchanges_free(rt->rt_head);
    }
    
#ifdef	RT_SANITY
    rt_sanity();
#endif	/* RT_SANITY */
    return rt;
}


/* User has declared a route unreachable.  */
void
rt_delete __PF1(delete_rt, rt_entry *)
{
    rt_check_open(delete_rt->rt_gwp->gw_proto, "rt_delete");

#ifdef	ROUTE_AGGREGATION
    if (delete_rt->rt_aggregate) {
	rt_aggregate_delete(delete_rt);
    }
#endif	/* ROUTE_AGGREGATION */

    rt_event_unreachable(delete_rt);

    if (BIT_TEST(delete_rt->rt_state, RTS_RELEASE)) {
	(void) rt_free(delete_rt);
    }
    rt_n_changes++;
    
#ifdef	RT_SANITY
    rt_sanity();
#endif	/* RT_SANITY */
}


#ifndef	rt_refresh
/*
 * rt_refresh() flags the route as being refreshed so the route timer will be reset the next time rt_time is run.
 */
rt_refresh __PF1(rt, rt_entry *)
{

    rt_check_open(rt->rt_gwp->gw_proto, "rt_refresh");

    BIT_SET(rt->rt_state, RTS_REFRESH);
}

#endif	/* rt_refresh */


/**/
/*
 *	Routines to handle route specific data
 */
rt_data *
rtd_alloc __PF1(length, size_t)
{
    rt_data *rtd;

    rtd = (rt_data *) task_mem_calloc((task *) 0,
				      1,
				      (size_t) (sizeof (*rtd) + length));
    rtd->rtd_data = (caddr_t) rtd + sizeof(*rtd);
    rtd->rtd_length = length;

    return rtd;
}


rt_data *
rtd_locate __PF3(data, void_t,
		 length, size_t,
		 head, rt_data *)
{
    rt_data *rtd;

    RTDATA_LIST(rtd, head) {
	if (rtd->rtd_length == length
	    && !bcmp((caddr_t) rtd->rtd_data, (caddr_t) data, length)) {
	    break;
	}
    } RTDATA_LIST_END(rtd, head);

    if (!rtd) {
	rtd = rtd_alloc(length);

	bcopy((caddr_t) data, (caddr_t) rtd->rtd_data, length);
	insque((struct qelem *) rtd, (struct qelem *) head);
    }
    rtd->rtd_refcount++;

    return rtd;
}


rt_data *
rtd_insert __PF2(rtd, rt_data *,
		 head, rt_data *)
{
    rt_data *rtd1;

    RTDATA_LIST(rtd1, head) {
	if ((rtd->rtd_length == rtd1->rtd_length) &&
	 !bcmp((caddr_t) rtd->rtd_data, (caddr_t) rtd1->rtd_data, rtd->rtd_length)) {
	    break;
	}
    } RTDATA_LIST_END(rtd1, head);

    if (rtd1) {
	task_mem_free((task *) 0, (caddr_t) rtd);
	rtd = rtd1;
    } else {
	insque((struct qelem *) rtd, (struct qelem *) head);
    }

    rtd->rtd_refcount++;

    return rtd;
}


void
rtd_unlink __PF1(rtd, rt_data *)
{
    if (--rtd->rtd_refcount < 1) {
	if (rtd->rtd_forw) {
	    remque((struct qelem *) rtd);
	}
	if (rtd->rtd_free) {
	    rtd->rtd_free(rtd);
	}
	task_mem_free((task *) 0, (caddr_t) rtd);
    }
}


#ifdef	PROTO_INET
/**/

/*
 * Handle the internally generated default route.
 */

int rt_default_active = 0;		/* Number of requests to install default */
int rt_default_needed = 0;		/* TRUE if we need to generate a default */
static rt_entry *rt_default_rt;		/* Pointer to the default route */
rt_parms rt_default_rtparms = RTPARMS_INIT(1,
					   (metric_t) 0,
					   RTS_INTERIOR|RTS_NOAGE,
					   RTPREF_DEFAULT);

/*
 * This is only a pseudo route so use the loopback interface,
 * it should not go down and produce
 * undesirable side effects.
 */  
#define	RT_DEFAULT_ADD	rt_default_rt = rt_add(&rt_default_rtparms)

#define	RT_DEFAULT_DELETE	(void) rt_delete(rt_default_rt); \
    				rt_default_rt = (rt_entry *) 0

static void
rt_default_reinit  __PF1(tp, task *)
{
    if (!rt_default_rtparms.rtp_gwp) {
	rt_default_rtparms.rtp_gwp = gw_init((gw_entry *) 0,
					     RTPROTO_DEFAULT,
					     tp,
					     (as_t) 0,
					     (as_t) 0,
					     (time_t) 0,
					     (sockaddr_un *) 0,
					     (flag_t) 0);
	rt_default_rtparms.rtp_dest = inet_addr_default;
	rt_default_rtparms.rtp_dest_mask = inet_mask_default;
    }
    /* Make sure we have a next hop */
    if (!rt_default_rtparms.rtp_router) {
	rt_default_rtparms.rtp_router = sockdup(inet_addr_loopback);
    }
    /* Do not install if our next hop is the loopback interface */
    if (sockaddrcmp(rt_default_rtparms.rtp_router, inet_addr_loopback)) {
	BIT_SET(rt_default_rtparms.rtp_state, RTS_NOTINSTALL);
    }

    rt_open(tp);
    
    if (rt_default_needed) {
	/* Default is enabled */

	if (rt_default_active) {
	    /* Should be installed */

	    if (!rt_default_rt) {
		/* It is not, add it */

		RT_DEFAULT_ADD;
	    } else {
		/* It exists, verify parameters */

		if (rt_default_rt->rt_preference != rt_default_rtparms.rtp_preference
		    || !sockaddrcmp(rt_default_rt->rt_router,
				    rt_default_rtparms.rtp_router)) {
		    /* Parmeters are not correct, delete and re-add */
		    
		    RT_DEFAULT_DELETE;
		    RT_DEFAULT_ADD;

		}
	    }
	}
    } else {
	/* Default is disabled */

	if (rt_default_rt) {
	    /* Get rid of current default */

	    RT_DEFAULT_DELETE;
	}
    }

    rt_close(tp, (gw_entry *) 0, 0, NULL);
}


void
rt_default_add __PF0(void)
{

    if (!rt_default_active++ && rt_default_needed) {
	/* First request to add and default is enabled, add it */

	rt_open(rt_task);
	RT_DEFAULT_ADD;
	rt_close(rt_task, (gw_entry *) 0, 0, NULL);
    }
}


void
rt_default_delete __PF0(void)
{

    if (!--rt_default_active && rt_default_rt) {
	/* Last request to delete and default is installed, remove it */

	rt_open(rt_task);
	RT_DEFAULT_DELETE;
	rt_close(rt_task, (gw_entry *) 0, 0, NULL);
    }
}


static void
rt_default_cleanup __PF1(tp, task *)
{
    rt_default_needed = FALSE;

    rt_default_rtparms.rtp_preference = RTPREF_DEFAULT;
    if (rt_default_rtparms.rtp_router) {
	sockfree(rt_default_rtparms.rtp_router);
	rt_default_rtparms.rtp_router = (sockaddr_un *) 0;
    }
    BIT_RESET(rt_default_rtparms.rtp_state, RTS_NOTINSTALL);
}


#undef	RT_DEFAULT_ADD
#undef	RT_DEFAULT_DELETE
#endif	/* PROTO_INET */


/**/
static void
rt_expire __PF2(tip, timer *,
		interval, time_t)
{
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = rtlist_all(AF_UNSPEC);

    rt_open(rt_task);

    RT_LIST(rt, rtl, rt_entry) {
	if (BIT_TEST(rt->rt_state, RTS_REFRESH)) {
	    /* Route has been refreshed, reset the timer */
	    BIT_RESET(rt->rt_state, RTS_REFRESH);
	    rt->rt_timer = 0;
	} else {
	    /* Else, update the timer */
	    rt->rt_timer += interval;
	}

	if (rt->rt_timer >= rt->rt_gwp->gw_timer_max &&
	    !BIT_TEST(rt->rt_state, RTS_DELETE|RTS_NOAGE)) {
	    rt_delete(rt);
	}
    } RT_LIST_END(rt, rtl, rt_entry);

    RTLIST_RESET(rtl) ;

    rt_close(rt_task, (gw_entry *) 0, 0, NULL);

    return;
}


/**/
/*
 *	In preparation for a re-parse, reset the NOAGE flags on static routes
 *	so they will be deleted if they are not refreshed.
 */
/*ARGSUSED*/
static void
rt_cleanup __PF1(tp, task *)
{

    rt_static_cleanup(tp);

#ifdef	PROTO_INET
    /* Cleanup default route */
    rt_default_cleanup(tp);
#endif	/* PROTO_INET */

#ifdef	ROUTE_AGGREGATION
    /* Cleanup aggregation policy */
    rt_aggregate_cleanup(tp);
#endif	/* ROUTE_AGGREGATION */

    /* Probably not the best place for this ... */
    martian_cleanup();
}


/*
 *	Delete any static routes that do not have the NOAGE flag.
 */
/*ARGSUSED*/
static void
rt_reinit __PF1(tp, task *)
{

#ifdef	ROUTE_AGGREGATION
    rt_aggregate_reinit(tp);
#endif	/* ROUTE_AGGREGATION */

    rt_static_reinit(tp);

#ifdef	PROTO_INET
    /* Make sure default route is in the right state */
    rt_default_reinit(tp);
#endif	/* PROTO_INET */

#ifdef	PROTO_SNMP
    /* Make sure the MIB is registered */
    rt_init_mib(TRUE);
#endif	/* PROTO_SNMP */
}


/*
 *	Deal with an interface state change
 */
static void
rt_ifachange __PF2(tp, task *,
		   ifap, if_addr *)
{
    rt_static_ifachange(tp);
}


/*
 *	Terminating - clean up
 */
static void
rt_terminate __PF1(tp, task *)
{

    rt_static_terminate(tp);

#ifdef	PROTO_INET
    /* Remove the internal default route */
    rt_default_reset();
#endif	/* PROTO_INET */

    task_delete(tp);
}


/*
 *	Dump routing table to dump file
 */
static void
rt_dump  __PF2(tp, task *,
	       fd, FILE *)
{
    int af = 0;
    register rt_head *rth;
    register rt_list *rtl = rthlist_all(AF_UNSPEC);

    /* Dump the control info */
    control_dump(fd);

#ifdef	ROUTE_AGGREGATION
    /* Dump the aggregate info */
    rt_aggregate_dump(fd);
#endif	/* ROUTE_AGGREGATION */

    /*
     * Dump the static routes
     */
    rt_static_dump(tp, fd);
    
    /*
     * Dump the static gateways
     */
    if (rt_gw_list) {
	(void) fprintf(fd, "\tGateways referenced by static routes:\n");

	gw_dump(fd,
		"\t\t",
		rt_gw_list,
		RTPROTO_STATIC);

	(void) fprintf(fd, "\n");
    }
    (void) fprintf(fd, "\n");

    /* Print the bit allocation info */
    rtbit_dump(fd);
    
    /* Print netmasks */
    mask_dump(fd);

    /* Routing table information */
    (void) fprintf(fd, "Routing Tables:\n");

    rt_table_dump(tp, fd);
    
#ifdef	PROTO_INET
    (void) fprintf(fd,
		   "\tGenerate Default: %s\n",
		   rt_default_needed ? "yes" : "no");
#endif	/* PROTO_INET */
    (void) fprintf(fd, "\n");

    /*
     * Dump all the routing information
     */

    RT_LIST(rth, rtl, rt_head) {
	register u_int i;
	register rt_entry *rt;

	if (socktype(rth->rth_dest) != af) {
	    af = socktype(rth->rth_dest);

	    (void) fprintf(fd, "\n\tRouting table for %s (%d):\n",
			   gd_lower(trace_value(task_domain_bits, af)),
			   af);
	    (void) fprintf(fd, "\t\tDestinations: %d\tRoutes: %d\n",
			   rtaf_info[af].rtaf_dests,
			   rtaf_info[af].rtaf_routes);
	    (void) fprintf(fd, "\t\tHolddown: %d\tDelete: %d\tHidden: %d\n\n",
			   rtaf_info[af].rtaf_holddowns,
			   rtaf_info[af].rtaf_deletes,
			   rtaf_info[af].rtaf_hiddens);
	}
	
	(void) fprintf(fd,
		  "\t%-15A",
		       rth->rth_dest);
	if (!BIT_TEST(rth->rth_state, RTS_GROUP)) {
	    (void) fprintf(fd,
			   "\tmask %-15A",
			   rth->rth_dest_mask);
	}
	(void) fprintf(fd,
		  "\n\t\t\tentries %d\tannounce %d",
		       rth->rth_entries,
		       rth->rth_n_announce);
	if (rth->rth_state) {
	    (void) fprintf(fd,
			   "\tstate <%s>",
			   trace_bits(rt_state_bits, rth->rth_state));
	}
	(void) fprintf(fd, "\n");

	rttsi_dump(fd, rth);

	(void) fprintf(fd, "\n");

	RT_ALLRT(rt, rth) {
	    (void) fprintf(fd,
			   "\t\t%s\tPreference: %3d",
			   trace_state(rt_proto_bits, rt->rt_gwp->gw_proto),
			   rt->rt_preference);
	    if (rt->rt_gwp->gw_addr && !sockaddrcmp(rt->rt_gwp->gw_addr, rt->rt_router)) {
		(void) fprintf(fd,
			       "\t\tSource: %A\n",
			       rt->rt_gwp->gw_addr);
	    } else {
		(void) fprintf(fd,
			       "\n");
	    }

	    for (i = 0; i < rt->rt_n_gw; i++) {
		(void) fprintf(fd,
			       "\t\t\tNextHop: %-15A\tInterface: %A(%s)\n",
			       rt->rt_routers[i],
			       rt->rt_ifaps[i]->ifa_addr,
			       rt->rt_ifaps[i]->ifa_name);
	    }

	    (void) fprintf(fd,
			   "\t\t\tState: <%s>\n",
			   trace_bits(rt_state_bits, rt->rt_state));

	    if (rt->rt_gwp->gw_peer_as || rt->rt_gwp->gw_local_as) {
		(void) fprintf(fd,
			       "\t\t");
		if (rt->rt_gwp->gw_local_as) {
		    (void) fprintf(fd,
				   "\tLocal AS: %5u",
				   rt->rt_gwp->gw_local_as);
		}
		if (rt->rt_gwp->gw_peer_as) {
		    (void) fprintf(fd,
				   "\tPeer AS: %5u",
				   rt->rt_gwp->gw_peer_as);
		}
		(void) fprintf(fd,
			       "\n");
	    }
	    (void) fprintf(fd,
			   "\t\t\tAge: %#T",
			   rt->rt_timer);
	    if (rt->rt_gwp->gw_timer_max) {
		(void) fprintf(fd,
			       "\tMax Age: %#T",
			       rt->rt_gwp->gw_timer_max);
	    }
	    (void) fprintf(fd,
			   "\tMetric: %d\tTag: %u\n",
			   rt->rt_metric,
			   rt->rt_tag);

	    if (rt->rt_gwp->gw_task) {
		(void) fprintf(fd,
			       "\t\t\tTask: %s\n",
			       task_name(rt->rt_gwp->gw_task));
	    }

	    if (rt->rt_n_bitsset) {
		(void) fprintf(fd,
			       "\t\t\tAnnouncement bits(%d):",
			       rt->rt_n_bitsset);
		for (i = 1; i <= RTBIT_SIZE; i++)  {
		    if (rtbit_isset(rt, i)) {
			(void) fprintf(fd,
				       " %d-%s",
				       i,
				       task_name(rtbit_map[i-1].task));
		    }
		}
		(void) fprintf(fd,
			       "\n");
	    }

#ifdef	PROTO_ASPATHS
	    /* Format AS path */
	    if (rt->rt_aspath) {
		aspath_dump(fd, rt->rt_aspath, "\t\t\t");
	    }
#endif	/* PROTO_ASPATHS */

	    /* Format protocol specific data */
	    if (rt->rt_data && rt->rt_data->rtd_dump) {
		rt->rt_data->rtd_dump(fd, rt);
	    }

#ifdef	ROUTE_AGGREGATION
	    /* Format aggregate information */
	    rt_aggregate_dump_rt(fd, rt);
#endif	/* ROUTE_AGGREGATION */

	    (void) fprintf(fd, "\n");
	} RT_ALLRT_END(rt, rth);

	(void) fprintf(fd, "\n");

    } RT_LIST_END(rth, rtl, rt_head) ;
}


/*
 *  Initialize the routing table.
 *
 *  Also creates a timer and task for the job of aging the routing table
 */
void
rt_init __PF0(void)
{

    /* Init the routing table */
    rt_table_init();
    
    /* Allocate the routing table task */
    rt_task = task_alloc("RT", TASKPRI_RT);
    rt_task->task_cleanup = rt_cleanup;
    rt_task->task_reinit = rt_reinit;
    rt_task->task_dump = rt_dump;
    rt_task->task_terminate = rt_terminate;
    rt_task->task_ifachange = rt_ifachange;
    rt_task->task_n_timers = RT_TIMER_MAX;
    if (!task_create(rt_task)) {
	task_quit(EINVAL);
    }

    rt_timer = timer_create(rt_task,
			    RT_TIMER_AGE,
			    "Age",
			    0,
			    (time_t) RT_T_AGE,
			    (time_t) 0,
			    rt_expire,
			    (void_t) 0);

    rt_block_index = task_block_init(sizeof (rt_entry));
    rth_block_index = task_block_init(sizeof (rt_head));
    rtchange_block_index = task_block_init(sizeof (rt_changes));
    rtlist_block_index = task_block_init((size_t) RTL_SIZE);
    rttsi_block_index = task_block_init(sizeof (struct _rt_tsi));

    rt_static_init(rt_task);

#ifdef	PROTO_INET
    rt_default_cleanup(rt_task);
#endif	/* PROTO_INET */

#ifdef	ROUTE_AGGREGATION
    rt_aggregate_init(rt_task);
#endif	/* ROUTE_AGGREGATION */

    redirect_init();
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
