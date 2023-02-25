static char sccsid[] = "@(#)36	1.5  src/tcpip/usr/sbin/gated/bgp_init.c, tcprouting, tcpip411, GOLD410 12/6/93 17:33:27";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: BGP_RX_BUFSIZE
 *		PROTOTYPE
 *		_PROTOTYPE
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF7
 *		bgp_aux_terminate2628
 *		bgp_buffer_alloc
 *		bgp_buffer_copy
 *		bgp_buffer_free
 *		bgp_cleanup
 *		bgp_close_socket1194
 *		bgp_conf_check
 *		bgp_conf_group_add3954
 *		bgp_conf_group_alloc3916
 *		bgp_conf_group_check4058
 *		bgp_conf_peer_add4075
 *		bgp_conf_peer_alloc3942
 *		bgp_connect_complete1734
 *		bgp_connect_start1777
 *		bgp_connect_timeout1869
 *		bgp_delete_connect_timer1673
 *		bgp_delete_traffic_timer1938
 *		bgp_dump
 *		bgp_fake_terminate3533
 *		bgp_find_group_by_addr2478
 *		bgp_find_peer
 *		bgp_group_add
 *		bgp_group_alloc
 *		bgp_group_delete3776
 *		bgp_group_dump
 *		bgp_group_free
 *		bgp_group_free_all
 *		bgp_group_init
 *		bgp_group_remove
 *		bgp_group_task_create
 *		bgp_init
 *		bgp_init_traffic_timer1953
 *		bgp_listen_accept2069
 *		bgp_listen_init
 *		bgp_listen_start2165
 *		bgp_listen_stop
 *		bgp_new_peer
 *		bgp_peer_add
 *		bgp_peer_alloc
 *		bgp_peer_close
 *		bgp_peer_connected1692
 *		bgp_peer_create
 *		bgp_peer_delete
 *		bgp_peer_dump
 *		bgp_peer_established3207
 *		bgp_peer_free
 *		bgp_peer_free_all
 *		bgp_peer_init
 *		bgp_peer_remove
 *		bgp_peer_start
 *		bgp_pp_create
 *		bgp_pp_delete
 *		bgp_pp_delete_all
 *		bgp_pp_dump
 *		bgp_pp_timeout
 *		bgp_recv_change
 *		bgp_recv_setbuf
 *		bgp_recv_setdirect1123
 *		bgp_recv_setopts1099
 *		bgp_recv_setup
 *		bgp_reinit
 *		bgp_reset_connect_timer1655
 *		bgp_send_reset
 *		bgp_send_set
 *		bgp_set_connect_timer1585
 *		bgp_set_peer_if
 *		bgp_set_traffic_timer1907
 *		bgp_sort_add
 *		bgp_sort_remove
 *		bgp_spool_add
 *		bgp_spool_flush
 *		bgp_spool_remove1423
 *		bgp_spool_trywrite1453
 *		bgp_spool_write
 *		bgp_task_create
 *		bgp_terminate
 *		bgp_traffic_timeout1989
 *		bgp_use_protopeer2882
 *		bgp_var_init
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
 * bgp_init.c,v 1.24.2.2 1993/05/17 18:35:03 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#include "inet.h"
#include "bgp.h"

/*
 * Miscellany
 */
int doing_bgp = FALSE;			/* Is BGP active? */
flag_t bgp_default_trace_flags;		/* Trace flags from parser */
pref_t bgp_default_preference;		/* Preference for BGP routes */
metric_t bgp_default_metric;

bgpPeerGroup *bgp_groups;		/* Linked list of BGP groups */
int bgp_n_groups;			/* Number of BGP groups */

int bgp_n_peers;			/* Number of BGP peers */
int bgp_n_unconfigured;			/* Number of unconfigured peers */
int bgp_n_established;			/* Number of established peers */

bgpProtoPeer *bgp_protopeers;		/* Link list of BGP proto-peers */
int bgp_n_protopeers;			/* Number of active protopeers */

bgpPeer *bgp_sort_list = NULL;		/* List of sorted peers, for SNMP */

adv_entry *bgp_import_list = NULL;	/* List of BGP advise entries */
adv_entry *bgp_import_aspath_list = NULL;	/* List of BGP import aspath policy */
adv_entry *bgp_export_list = NULL;	/* List of BGP export entries */

static u_short bgp_port;		/* Well known BGP port */
task *bgp_listen_task = (task *) 0;	/* Task we use for listening */

static bgpPeer *bgp_free_list = NULL;	/* List of free'd peer structures */
static int bgp_n_free = 0;		/* Number of free peer structures */

static bgpPeerGroup *bgp_group_free_list = NULL; /* Free list for groups */
static int bgp_n_groups_free = 0;	/* Number of free group structures */


/*
 * Forward declarations
 */
PROTOTYPE(bgp_terminate, static void, (task *));
PROTOTYPE(bgp_fake_terminate, static void, (task *));
PROTOTYPE(bgp_dump, static void, (task *, FILE *));
PROTOTYPE(bgp_group_dump, static void, (task *, FILE *));
PROTOTYPE(bgp_peer_dump, static void, (task *, FILE *));
PROTOTYPE(bgp_pp_dump, static void, (task *, FILE *));
PROTOTYPE(bgp_pp_timeout, static void, (timer *, time_t));
PROTOTYPE(bgp_connect_start, static void, (bgpPeer *));
PROTOTYPE(bgp_traffic_timeout, static void, (timer *, time_t));
PROTOTYPE(bgp_connect_timeout, static void, (timer *, time_t));
PROTOTYPE(bgp_init_traffic_timer, static void, (bgpPeer *));
PROTOTYPE(bgp_cleanup, static void, (task *));
PROTOTYPE(bgp_reinit, static void, (task *));
PROTOTYPE(bgp_peer_start, static void, (bgpPeer *, time_t));
PROTOTYPE(bgp_set_peer_if, static if_addr *, (bgpPeer *, int));
PROTOTYPE(bgp_ifachange, static void, (task *, if_addr *));
PROTOTYPE(bgp_spool_trywrite, static void, (task *));

/*
 * Pretty-printing info
 */
const bits bgp_flag_bits[] =
{
    {BGPF_DELETE, "Delete"},
    {BGPF_UNCONFIGURED, "Unconfigured"},
    {BGPF_TRAFFIC, "TrafficTimer"},
    {BGPF_CONNECT, "ConnectTimer"},
    {BGPF_TRY_CONNECT, "TryConnect"},
    {BGPF_WRITEFAILED, "WriteFailed"},
    {BGPF_IDLED, "Idled"},
    {BGPF_GENDEFAULT, "GenDefault"},
    {0}
};

const bits bgp_group_flag_bits[] =
{
    {BGPGF_DELETE, "Delete"},
    {BGPGF_IDLED, "Idled"},
    {0}
};

const bits bgp_option_bits[] =
{
    {BGPO_METRIC_OUT, "MetricOut"},
    {BGPO_LOCAL_AS, "LocalAs"},
    {BGPO_NOGENDEFAULT, "NoGenDefault"},
    {BGPO_GATEWAY, "Gateway"},
    {BGPO_PREFERENCE, "Preference"},
    {BGPO_LCLADDR, "LocalAddress"},
    {BGPO_HOLDTIME, "HoldTime"},
    {BGPO_PASSIVE, "Passive"},
    {BGPO_VERSION, "VersionSpecified"},
    {BGPO_DEFAULTIN, "DefaultIn"},
    {BGPO_DEFAULTOUT, "DefaultOut"},
    {BGPO_KEEPALL, "KeepAllRoutes"},
    {0}
};

const bits bgp_state_bits[] =
{
    { 0, "NoState" },
    {BGPSTATE_IDLE, "Idle"},
    {BGPSTATE_CONNECT, "Connect"},
    {BGPSTATE_ACTIVE, "Active"},
    {BGPSTATE_OPENSENT, "OpenSent"},
    {BGPSTATE_OPENCONFIRM, "OpenConfirm"},
    {BGPSTATE_ESTABLISHED, "Established"},
    {0}
};
BGP_MAKE_CODES(bgp_state_codes, bgp_state_bits);

const bits bgp_event_bits[] =
{
    { 0, "NoEvent" },
    {BGPEVENT_START, "Start"},
    {BGPEVENT_STOP, "Stop"},
    {BGPEVENT_OPEN, "Open"},
    {BGPEVENT_CLOSED, "Closed"},
    {BGPEVENT_OPENFAIL, "OpenFail"},
    {BGPEVENT_ERROR, "TransportError"},
    {BGPEVENT_CONNRETRY, "ConnectRetry"},
    {BGPEVENT_HOLDTIME, "HoldTime"},
    {BGPEVENT_KEEPALIVE, "KeepAlive"},
    {BGPEVENT_RECVOPEN, "RecvOpen"},
    {BGPEVENT_RECVKEEPALIVE, "RecvKeepAlive"},
    {BGPEVENT_RECVUPDATE, "RecvUpdate"},
    {BGPEVENT_RECVNOTIFY, "RecvNotify"},
    {0}
};
BGP_MAKE_CODES(bgp_event_codes, bgp_event_bits);

const bits bgp_message_type_bits[] =
{
    { 0, "invalid" },
    {BGP_OPEN, "Open"},
    {BGP_UPDATE, "Update"},
    {BGP_NOTIFY, "Notification"},
    {BGP_KEEPALIVE, "KeepAlive"},
    {0}
};
BGP_MAKE_CODES(bgp_message_type_codes, bgp_message_type_bits);

const bits bgp_error_bits[] =
{
    { 0, "None" },
    {BGP_ERR_HEADER, "Message Header Error"},
    {BGP_ERR_OPEN, "Open Message Error"},
    {BGP_ERR_UPDATE, "Update Message Error"},
    {BGP_ERR_HOLDTIME, "Hold Timer Expired Error"},
    {BGP_ERR_FSM, "Finite State Machine Error"},
    {BGP_CEASE, "Cease"},
    {0}
};
BGP_MAKE_CODES(bgp_error_codes, bgp_error_bits);

const bits bgp_header_error_bits[] =
{
    {BGP_ERR_UNSPEC, "unspecified error"},
    {BGP_ERRHDR_UNSYNC, "lost connection synchronization"},
    {BGP_ERRHDR_LENGTH, "bad length"},
    {BGP_ERRHDR_TYPE, "bad message type"},
    {0}
};
BGP_MAKE_CODES(bgp_header_error_codes, bgp_header_error_bits);

const bits bgp_open_error_bits[] =
{
    {BGP_ERR_UNSPEC, "unspecified error"},
    {BGP_ERROPN_VERSION, "unsupported version"},
    {BGP_ERROPN_AS, "bad AS number"},
    {BGP_ERROPN_BGPID, "bad BGP ID"},
    {BGP_ERROPN_AUTHCODE, "unsupported authentication code"},
    {BGP_ERROPN_AUTH, "authentication failure"},
    {0}
};
BGP_MAKE_CODES(bgp_open_error_codes, bgp_open_error_bits);

const bits bgp_update_error_bits[] =
{
    {BGP_ERR_UNSPEC, "unspecified error"},
    {BGP_ERRUPD_ATTRLIST, "invalid attribute list"},
    {BGP_ERRUPD_UNKNOWN, "unknown well known attribute"},
    {BGP_ERRUPD_MISSING, "missing well known attribute"},
    {BGP_ERRUPD_FLAGS, "attribute flags error"},
    {BGP_ERRUPD_LENGTH, "bad attribute length"},
    {BGP_ERRUPD_ORIGIN, "bad ORIGIN attribute"},
    {BGP_ERRUPD_ASLOOP, "AS loop detected"},
    {BGP_ERRUPD_NEXTHOP, "invalid NEXT_HOP"},
    {BGP_ERRUPD_OPTATTR, "error with optional attribute"},
    {0}
};
BGP_MAKE_CODES(bgp_update_error_codes, bgp_update_error_bits);

const bits bgp_group_bits[] =
{
    {BGPG_EXTERNAL, "External" },
    {BGPG_INTERNAL, "Internal" },
    {BGPG_INTERNAL_IGP, "IGP" },
    {BGPG_TEST, "Test" },
    {0}
};
BGP_MAKE_CODES(bgp_group_codes, bgp_group_bits);

#ifdef notdef
const static bits bgp_group_initial_bits[] =
{
    { 0, "X" },
    {BGPG_EXTERNAL, "E" },
    {BGPG_INTERNAL, "IP" },
    {BGPG_INTERNAL_IGP, "I" },
    {BGPG_TEST, "T" },
    {0}
};
#endif

/*
 * BGP memory management.
 *
 * There are several types of objects which BGP manages the allocation
 * of.  Peer structures are one of them, but are malloc'd and free'd to
 * agree with the parser.
 *
 * Other objects are the 4k input packet buffers, output packet buffers
 * and structures used for sorting updates.  These we allocate and free
 * from a pool of 4k blocks maintained by the task_block_*() routines.
 *
 * Proto-peer structures are allocated from a separate (appropriately
 * sized) pool.
 */
block_t bgp_buf_blk_index;		/* returned by call to task_block_init() */
static block_t bgp_pp_blk_index;	/* ibid */

/*
 * Connect timer slot maintenance.  We attempt to spread connection
 * attempt times into slots to reduce the impact of starting a bunch
 * of connections all at once.  This is an utter waste of time if
 * you've only got a few peers, but may help if you've got a lot.
 *
 * This is the slot array.
 */
static byte bgp_connect_slots[BGPCONN_N_SLOTS] = {0};

/*
 * Write spooling.  When configured to do so we spool write data internally
 * when we get an EWOULDBLOCK on a write.  The write data is saved in
 * buffers which are page-sized (or an even fraction of a page) with a
 * header at the start.  We fetch the buffer using the task_block_*()
 * routines.
 */
static block_t bgp_spool_hdr_blk_index;	/* spool buffer header */
static block_t bgp_spool_buf_blk_index;	/* buffer allocation */
static size_t bgp_spool_buf_size;	/* size of spool buffers */

/*
 * Macro to determine the buffer size to which a socket should be set.
 */
#define	BGP_RX_BUFSIZE(size) \
    (((size) == 0 || (size) >= task_maxpacket) ? task_maxpacket : (size))


/*
 *	Pretty name formatting
 */

/*
 * bgp_make_names - routine to create a pretty names for a BGP peer
 */
static void
bgp_make_names __PF1(bnp, bgpPeer *)
{
    (void) sprintf(bnp->bgp_name, "%A (%s AS %u)",
		   bnp->bgp_addr,
		   trace_state(bgp_group_bits, bnp->bgp_type),
		   (u_int) bnp->bgp_peer_as);
    if (bnp->bgp_local_as != inet_autonomous_system) {
	(void) sprintf(bnp->bgp_task_name, "BGP_%u_%u",
		       (u_int) bnp->bgp_peer_as,
		       (u_int) bnp->bgp_local_as);
    } else {
	(void) sprintf(bnp->bgp_task_name, "BGP_%u",
		       (u_int) bnp->bgp_peer_as);
    }
}


/*
 * bgp_make_pp_name - make a name for a protopeer
 */
static void
bgp_make_pp_name __PF1(bpp, bgpProtoPeer *)
{
    (void) sprintf(bpp->bgpp_name, "%#A (proto)",
		   bpp->bgpp_task->task_addr);
}

/*
 * bgp_make_group_names - make some names for a group
 */
static void
bgp_make_group_names __PF1(bgp, bgpPeerGroup *)
{
    if (bgp->bgpg_type == BGPG_INTERNAL ||
	bgp->bgpg_type == BGPG_INTERNAL_IGP ||
	bgp->bgpg_local_as != inet_autonomous_system) {
        (void) sprintf(bgp->bgpg_name, "group type %s AS %u",
		       trace_state(bgp_group_bits, bgp->bgpg_type),
		       (u_int) bgp->bgpg_peer_as);
	(void) sprintf(bgp->bgpg_task_name, "BGP_Group_%u",
		       (u_int) bgp->bgpg_peer_as);
    } else {
        (void) sprintf(bgp->bgpg_name, "group type %s AS %u local %u",
		       trace_state(bgp_group_bits, bgp->bgpg_type),
		       (u_int) bgp->bgpg_peer_as,
		       (u_int) bgp->bgpg_local_as);
	(void) sprintf(bgp->bgpg_task_name, "BGP_Group_%u_%u",
		       (u_int) bgp->bgpg_peer_as,
		       (u_int) bgp->bgpg_local_as);
    }
}


/*
 *	Sort list maintenance
 */

/*
 * bgp_sort_add - add a peer to the sort list
 */
PROTOTYPE(bgp_sort_add,
	  static void,
	 (bgpPeer *));
static void
bgp_sort_add(add_bnp)
register bgpPeer *add_bnp;
{
    register u_long rmt;

    if (add_bnp->bgp_sort_next != NULL) {
    }

    /*
     * Load up the local rmt address.  This is an efficiency hack, since
     * most peers will be at different addresses.
     */
    rmt = ntohl(sock2ip(add_bnp->bgp_addr));

    /*
     * See if this goes first in the list
     */
    if (bgp_sort_list == NULL
	|| BGPSORT_LT(add_bnp, rmt, bgp_sort_list)) {
	add_bnp->bgp_sort_next = bgp_sort_list;
	bgp_sort_list = add_bnp;
    } else {
        register bgpPeer *bnpprev, *bnp;

	/*
	 * Not first, run through list
	 */
	for (bnpprev = bgp_sort_list;
	     (bnp = bnpprev->bgp_sort_next) != NULL;
	     bnpprev = bnp) {
	    if (BGPSORT_LT(add_bnp, rmt, bnp)) {
		break;
	    }
	}

	/* Already on sort list */
	assert(add_bnp != bnpprev &&
	       add_bnp != bnp);

	add_bnp->bgp_sort_next = bnp;
	bnpprev->bgp_sort_next = add_bnp;
    }
}


/*
 * bgp_sort_remove - remove a peer from the sort list
 */
PROTOTYPE(bgp_sort_remove,
	  static void,
	 (bgpPeer *));
static void
bgp_sort_remove(rem_bnp)
register bgpPeer *rem_bnp;
{
    /*
     * See if it is first on the list
     */

    /* Sort list empty */
    assert(bgp_sort_list);

    if (bgp_sort_list == rem_bnp) {
	bgp_sort_list = rem_bnp->bgp_sort_next;
    } else {
	register bgpPeer *bnp;

	/*
	 * Not first, must be further in
	 */
	for (bnp = bgp_sort_list;
	     bnp->bgp_sort_next != NULL;
	     bnp = bnp->bgp_sort_next) {
	    if (bnp->bgp_sort_next == rem_bnp) {
		break;
	    }
	}

	/* Not on sort list */
	assert(bnp->bgp_sort_next);

	bnp->bgp_sort_next = rem_bnp->bgp_sort_next;
    }
    rem_bnp->bgp_sort_next = NULL;
}



/*
 *	Peer structure allocation and basic unconfigured peer support
 */

/*
 * bgp_peer_alloc - return a new peer structure
 */
PROTOTYPE(bgp_peer_alloc,
	  static bgpPeer *,
	 (task *));
static bgpPeer *
bgp_peer_alloc(tp)
task *tp;
{
    bgpPeer *bnp;

    /*
     * If we've got one in the free list, return it, else malloc() one.
     */
    if (bgp_free_list != NULL) {
	bnp = bgp_free_list;
	bgp_free_list = bnp->bgp_next;
	bgp_n_free--;
    } else {
	bnp = (bgpPeer *) task_mem_malloc(tp, sizeof(bgpPeer));
    }

    return (bnp);
}

/*
 * bgp_peer_free - move peer structure onto free list
 */
PROTOTYPE(bgp_peer_free,
	  static void,
	 (bgpPeer *));
static void
bgp_peer_free(bnp)
bgpPeer *bnp;
{
    /*
     * XXX Someday might free things if we get more than a certain number
     * on the free list.
     */
    bnp->bgp_next = bgp_free_list;
    bgp_free_list = bnp;
    bgp_n_free++;
}


/*
 * bgp_peer_free_all - really free all structures on the free list
 */
PROTOTYPE(bgp_peer_free_all,
	  static void,
	 (void));
static void
bgp_peer_free_all()
{
    bgpPeer *bnp;
    bgpPeer *bnpnext;

    for (bnp = bgp_free_list; bnp != NULL; bnp = bnpnext) {
	bnpnext = bnp->bgp_next;
	task_mem_free(bgp_listen_task, (caddr_t) bnp);
    }
    bgp_free_list = NULL;
    bgp_n_free = 0;
}


/*
 * bgp_peer_add - add a peer to a peer group
 */
PROTOTYPE(bgp_peer_add,
	  static void,
	 (bgpPeerGroup *,
	  bgpPeer *));
static void
bgp_peer_add(bgp, bnp)
bgpPeerGroup *bgp;
bgpPeer *bnp;
{
    flag_t flags;

    /*
     * Put deleted before unconfigured, and normal before
     * deleted.
     */
    flags = 0;
    if (!BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)) {
	flags = BGPF_UNCONFIGURED;
	if (!BIT_TEST(bnp->bgp_flags, BGPF_DELETE)) {
	    BIT_SET(flags, BGPF_DELETE);
	}
    }

    if (bgp->bgpg_peers == NULL
	|| BIT_TEST(bgp->bgpg_peers->bgp_flags, flags)) {
	bnp->bgp_next = bgp->bgpg_peers;
	bgp->bgpg_peers = bnp;
    } else {
	register bgpPeer *bnp2;

	for (bnp2 = bgp->bgpg_peers;
	     bnp2->bgp_next != NULL;
	     bnp2 = bnp2->bgp_next) {
	    if (BIT_TEST(bnp2->bgp_next->bgp_flags, flags)) {
		break;
	    }
	}

	bnp->bgp_next = bnp2->bgp_next;
	bnp2->bgp_next = bnp;
    }
    bnp->bgp_group = bgp;
    bgp->bgpg_n_peers++;
    bgp_n_peers++;
    if (BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)) {
        bgp_n_unconfigured++;
    }
    bgp_sort_add(bnp);
}


/*
 * bgp_peer_remove - remove a peer from its peer group
 */
PROTOTYPE(bgp_peer_remove,
	  static void,
	 (bgpPeer *));
static void
bgp_peer_remove(bnp)
bgpPeer *bnp;
{
    bgpPeerGroup *bgp;

    bgp = bnp->bgp_group;
    if (bgp->bgpg_peers == NULL) {
	goto fuckup;
    }

    bgp_sort_remove(bnp);
    if (bgp->bgpg_peers == bnp) {
	bgp->bgpg_peers = bnp->bgp_next;
    } else {
	bgpPeer *bnp2;

	for (bnp2 = bgp->bgpg_peers;
	     bnp2->bgp_next != bnp && bnp2->bgp_next != NULL;
	     bnp2 = bnp2->bgp_next) {
	    /* nothing */
	}

	if (bnp2->bgp_next == NULL) {
	    goto fuckup;
	}
	bnp2->bgp_next = bnp->bgp_next;
    }

    bgp->bgpg_n_peers--;
    bgp_n_peers--;
    if (BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)) {
        bgp_n_unconfigured--;
    }
    return;

fuckup:
    /* Not found in list */
    assert(FALSE);
}


/*
 *	Group structure allocation.  Only used during (re)configuration
 */

/*
 * bgp_group_alloc - return a new group structure
 */
static bgpPeerGroup *
bgp_group_alloc()
{
    bgpPeerGroup *bgp;

    /*
     * If we've got one in the free list, return it, else malloc() one.
     */
    if (bgp_group_free_list != NULL) {
	bgp = bgp_group_free_list;
	bgp_group_free_list = bgp->bgpg_next;
	bgp_n_groups_free--;
    } else {
	bgp = (bgpPeerGroup *) task_mem_malloc(bgp_listen_task,
					       sizeof(bgpPeerGroup));
    }

    return (bgp);
}

/*
 * bgp_group_free - move group structure onto free list
 */
PROTOTYPE(bgp_group_free,
	  static void,
	 (bgpPeerGroup *));
static void
bgp_group_free(bgp)
bgpPeerGroup *bgp;
{
    /*
     * XXX Someday might free things if we get more than a certain number
     * on the free list.
     */
    bgp->bgpg_next = bgp_group_free_list;
    bgp_group_free_list = bgp;
    bgp_n_groups_free++;
}


/*
 * bgp_group_free_all - really free all structures on the free list
 */
PROTOTYPE(bgp_group_free_all,
	  static void,
	 (void));
static void
bgp_group_free_all()
{
    bgpPeerGroup *bgp;
    bgpPeerGroup *bgpnext;

    for (bgp = bgp_group_free_list; bgp != NULL; ) {
	bgpnext = bgp->bgpg_next;
	task_mem_free(bgp_listen_task, (caddr_t) bgp);
	bgp = bgpnext;
    }
    bgp_group_free_list = NULL;
    bgp_n_groups_free = 0;
}


/*
 * bgp_group_add - add a group to the group list
 */
PROTOTYPE(bgp_group_add,
	  static void,
	 (bgpPeerGroup *));
static void
bgp_group_add(bgp)
bgpPeerGroup *bgp;
{
    /*
     * Try to keep them in the order they appear in the configuration file
     */
    bgp->bgpg_next = NULL;
    if (bgp_groups == NULL) {
	bgp_groups = bgp;
    } else {
	register bgpPeerGroup *bgp2;

	for (bgp2 = bgp_groups;
	  bgp2->bgpg_next != NULL;
	  bgp2 = bgp2->bgpg_next) {
	    /* nothing */
	}

	bgp2->bgpg_next = bgp;
    }
    bgp_n_groups++;
}


/*
 * bgp_group_remove - remove a group from its peer group
 */
PROTOTYPE(bgp_group_remove,
	  static void,
	 (bgpPeerGroup *));
static void
bgp_group_remove(bgp)
bgpPeerGroup *bgp;
{
    if (bgp_groups == NULL) {
	goto fuckup;
    }
    if (bgp == bgp_groups) {
	bgp_groups = bgp->bgpg_next;
    } else {
	bgpPeerGroup *bgp2;

	for (bgp2 = bgp_groups;
	  bgp2->bgpg_next != bgp && bgp2->bgpg_next != NULL;
	  bgp2 = bgp2->bgpg_next) {
	    /* nothing */
	}

	if (bgp2->bgpg_next == NULL) {
	    goto fuckup;
	}
	bgp2->bgpg_next = bgp->bgpg_next;
    }

    bgp_n_groups--;
    return;

fuckup:
    /* Not found in group list */
    assert(FALSE);
}



/*
 *
 *	Memory block allocation routines
 *
 */

/*
 * bgp_buffer_alloc - allocate and initialize an input buffer
 */
PROTOTYPE(bgp_buffer_alloc,
	  static void,
	 (bgpBuffer *));
static void
bgp_buffer_alloc(bp)
bgpBuffer *bp;
{
    /*
     * Allocate memory.  Init bucket if we need it.  Fix up pointers
     * in buffer allocation
     */
    if (!bgp_buf_blk_index) {
	bgp_buf_blk_index = task_block_init(BGPRECVBUFSIZE);
    }
    if (bp->bgpb_buffer != NULL) {
	trace(TR_ALL, LOG_ERR,
	      "bgp_buffer_alloc: buffer already allocated, something screwed up!!");
    } else {
	bp->bgpb_buffer = (byte *)task_block_alloc(bgp_buf_blk_index);
    }
    bp->bgpb_bufpos = bp->bgpb_readptr = bp->bgpb_buffer;
    bp->bgpb_endbuf = bp->bgpb_buffer + BGPRECVBUFSIZE;
}


/*
 * bgp_buffer_free - free an input buffer
 */
PROTOTYPE(bgp_buffer_free,
	  static void,
	 (bgpBuffer *));
static void
bgp_buffer_free(bp)
bgpBuffer *bp;
{
    if (bp->bgpb_buffer != NULL) {
	task_block_free(bgp_buf_blk_index, (caddr_t)bp->bgpb_buffer);
    } else {
	trace(TR_ALL, LOG_ERR,
	      "bgp_buffer_free: buffer not present, something screwed up!!");
    }
    bp->bgpb_buffer = bp->bgpb_bufpos = NULL;
    bp->bgpb_readptr = bp->bgpb_endbuf = NULL;
}


/*
 * bgp_buffer_copy - copy memory buffer from one structure to another
 */
PROTOTYPE(bgp_buffer_copy,
	  static void,
	 (bgpBuffer *,
	  bgpBuffer *));
static void
bgp_buffer_copy(bpfrom, bpto)
bgpBuffer *bpfrom;
bgpBuffer *bpto;
{
    if (bpto->bgpb_buffer != NULL) {
	trace(TR_ALL, LOG_ERR,
	      "bgp_buffer_copy: destination already has buffer allocated!!");
	bgp_buffer_free(bpto);
    }
    *bpto = *bpfrom;	/* struct copy */
    bpfrom->bgpb_buffer = bpfrom->bgpb_bufpos = NULL;
    bpfrom->bgpb_readptr = bpfrom->bgpb_endbuf = NULL;
}


/*
 *	Protopeer creation/deletion
 */

/*
 * bgp_pp_create - allocate and init a protopeer structure
 */
PROTOTYPE(bgp_pp_create,
	  static bgpProtoPeer *,
	 (task *,
	  sockaddr_un *));
static bgpProtoPeer *
bgp_pp_create(tp, lcladdr)
task *tp;
sockaddr_un *lcladdr;
{
    bgpProtoPeer *ppp;

    /*
     * If no bucket allocated, do it now.
     */
    if (!bgp_pp_blk_index) {
	bgp_pp_blk_index = task_block_init(sizeof(bgpProtoPeer));
    }
    ppp = (bgpProtoPeer *)task_block_alloc(bgp_pp_blk_index);

    /*
     * Initialize structure and hook it into chain.
     */
    ppp->bgpp_task = tp;
    tp->task_data = (caddr_t)ppp;
    ppp->bgpp_myaddr = lcladdr;
    ppp->bgpp_connect_time = bgp_time_sec;
    bgp_make_pp_name(ppp);
    bgp_buffer_alloc(&(ppp->bgpp_inbuf));

    ppp->bgpp_next = bgp_protopeers;
    bgp_protopeers = ppp;
    bgp_n_protopeers++;
    return (ppp);
}


/*
 * bgp_pp_delete - free a protopeer entirely
 */
void
bgp_pp_delete(ppp)
bgpProtoPeer *ppp;
{
    if (ppp->bgpp_task != NULL) {
	task_delete(ppp->bgpp_task);
    }
    if (ppp->bgpp_myaddr != NULL) {
	sockfree(ppp->bgpp_myaddr);
	ppp->bgpp_myaddr = NULL;
    }
    if (ppp->bgpp_buffer != NULL) {
	bgp_buffer_free(&(ppp->bgpp_inbuf));
    }

    if (ppp == bgp_protopeers) {
	bgp_protopeers = ppp->bgpp_next;
    } else {
        bgpProtoPeer *pp2;

	for (pp2 = bgp_protopeers; pp2 != NULL; pp2 = pp2->bgpp_next) {
	    if (pp2->bgpp_next == ppp) {
		pp2->bgpp_next = ppp->bgpp_next;
		break;
	    }
	}

	/* Not found in list */
	assert(pp2);
    }
    bgp_n_protopeers--;
    task_block_free(bgp_pp_blk_index, (caddr_t)ppp);
}


/*
 * bgp_pp_delete_all - delete all protopeers
 */
PROTOTYPE(bgp_pp_delete_all,
	  static void,
	 (void));
static void
bgp_pp_delete_all()
{
    while (bgp_protopeers != NULL) {
	bgp_pp_delete(bgp_protopeers);
    }
}


/*
 *	Group/peer task creation
 */

/*
 * bgp_task_create - create a task for a BGP peer
 */
PROTOTYPE(bgp_task_create,
	  static void,
	 (bgpPeer *));
static void
bgp_task_create(bnp)
bgpPeer *bnp;
{
    task *tp;

    tp = bnp->bgp_task = task_alloc(bnp->bgp_task_name, TASKPRI_EXTPROTO);
    tp->task_trace_flags = bnp->bgp_trace_flags;
    tp->task_addr = sockdup(bnp->bgp_addr);
    tp->task_rtproto = RTPROTO_BGP;
    tp->task_terminate = bgp_fake_terminate;
    tp->task_dump = bgp_peer_dump;
    tp->task_data = (caddr_t) bnp;
    tp->task_n_timers = BGP_PEER_TIMER_MAX;
    if (!task_create(tp)) {
	task_quit(EINVAL);
    }
}


/*
 * bgp_group_task_create - create a task for a BGP group
 */
PROTOTYPE(bgp_group_task_create,
	  static void,
	 (bgpPeerGroup *));
static void
bgp_group_task_create(bgp)
bgpPeerGroup *bgp;
{
    task *tp;

    tp = task_alloc(bgp->bgpg_task_name, TASKPRI_EXTPROTO);
    tp->task_trace_flags = bgp->bgpg_trace_flags;
    tp->task_rtproto = RTPROTO_BGP;
    tp->task_data = (caddr_t) bgp;
    tp->task_terminate = bgp_fake_terminate;
    tp->task_dump = bgp_group_dump;
    if (!task_create(tp)) {
	task_quit(EINVAL);
    }
    bgp->bgpg_task = tp;
}


/*
 *	Socket manipulation
 */

/*
 * bgp_recv_change - change the receiver for read ready requests
 */
void
bgp_recv_change(bnp, recv_rtn, recv_rtn_name)
bgpPeer *bnp;
void (*recv_rtn)();
const char *recv_rtn_name;
{
    task *tp;

    /*
     * Make sure there's already a socket associated with this
     */
    tp = bnp->bgp_task;
    /* Socket not initialized */
    assert(tp->task_socket >= 0);

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_read_change: peer %s receiver changed to %s",
	      bnp->bgp_name,
	      recv_rtn_name);
    }
    
    tp->task_recv = recv_rtn;
    task_set_socket(tp, tp->task_socket);
}

/*
 * bgp_send_set - set up a write processing routine for this peer's task
 */
PROTOTYPE(bgp_send_set,
	  static void,
	 (bgpPeer *,
	  _PROTOTYPE(send_rtn,
		     void,
		     (task *))));
static void
bgp_send_set(bnp, send_rtn)
bgpPeer *bnp;
_PROTOTYPE(send_rtn,
	   void,
	   (task *));
{
    task *tp;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	  "bgp_set_send_rtn: setting task write routine for peer %s",
	  bnp->bgp_name);
    }

    /*
     * Set the task_write routine.  If the socket is open call task_set_socket.
     */
    tp = bnp->bgp_task;

    tp->task_write = send_rtn;
    if (tp->task_socket >= 0) {
	task_set_socket(tp, tp->task_socket);
    }
}


/*
 * bgp_send_reset - unset the write processing routine for this peer's task
 */
PROTOTYPE(bgp_send_reset,
	  static void,
	  (bgpPeer *));
static void
bgp_send_reset(bnp)
bgpPeer *bnp;
{
    task *tp;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	  "bgp_set_send_rtn: removing task write routine from peer %s",
	  bnp->bgp_name);
    }

    /*
     * NULL the write routine in the task.  If the socket is open call
     * task_set_socket() to propagate this.
     */
    tp = bnp->bgp_task;
    tp->task_write = NULL;
    if (tp->task_socket >= 0) {
	task_set_socket(tp, tp->task_socket);
    }
}


PROTOTYPE(bgp_recv_setbuf,
	  static void,
	  (task *, size_t, size_t));
static void
bgp_recv_setbuf(tp, recvbuf, sendbuf)
task *tp;
size_t recvbuf;
size_t sendbuf;
{
    int size;

    size = (int) BGP_RX_BUFSIZE(recvbuf);
    if (task_set_option(tp, TASKOPTION_RECVBUF, size) < 0) {
	task_quit(errno);
    }
    size = (int) BGP_RX_BUFSIZE(sendbuf);
    if (task_set_option(tp, TASKOPTION_SENDBUF, size) < 0) {
	task_quit(errno);
    }
}


/*
 * bgp_recv_setopts - set options on a task to be used for reading
 */
PROTOTYPE(bgp_recv_setopts,
	  static void,
	 (task *, size_t, size_t));
static void
bgp_recv_setopts(tp, recvbuf, sendbuf)
task *tp;
size_t recvbuf;
size_t sendbuf;
{
    if (task_set_option(tp, TASKOPTION_NONBLOCKING, TRUE) < 0) {
	task_quit(errno);
    }
    bgp_recv_setbuf(tp, recvbuf, sendbuf);

    /* Linger will block the close for the linger timeout, but it doesn't
       work on a non-blocking socket */
    if (task_set_option(tp, TASKOPTION_LINGER, 0) < 0) {
	task_quit(errno);
    }
}

/*
 * bgp_recv_setdirect - set socket for host on direct network
 */
PROTOTYPE(bgp_recv_setdirect,
	  static void,
	 (task *));
static void
bgp_recv_setdirect(tp)
task *tp;
{
    /* Don't use routing table */
    if (task_set_option(tp, TASKOPTION_DONTROUTE, TRUE) < 0) {
	task_quit(errno);
    }

    /* Set TTL to 1 if supported */
    (void) task_set_option(tp, TASKOPTION_TTL, 1);
}


/*
 * bgp_recv_setup - set up an open socket to receive data via task
 */
PROTOTYPE(bgp_recv_setup,
	  static void,
	 (bgpPeer *,
	  int,
	  _PROTOTYPE(recv_rtn,
		     void,
		     (task *)),
	  const char *));
static void
bgp_recv_setup(bnp, recv_socket, recv_rtn, recv_rtn_name)
bgpPeer *bnp;
int recv_socket;
_PROTOTYPE(recv_rtn,
	   void,
	   (task *));
const char *recv_rtn_name;
{
    _PROTOTYPE(savesendrtn, void, (task *));
    task *tp = bnp->bgp_task;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_recv_setup: peer %s socket %d set for reading",
	      bnp->bgp_name,
	      recv_socket);
    }

    /*
     * If this task has been using a socket, reset it first
     */
    savesendrtn = tp->task_write;
    if (tp->task_socket >= 0) {
	task_reset_socket(tp);
    }

    tp->task_write = savesendrtn;
    tp->task_recv = recv_rtn;
    task_set_socket(tp, recv_socket);

    /*
     * Set up appropriate options
     */
    bgp_recv_setopts(tp, bnp->bgp_recv_bufsize, bnp->bgp_send_bufsize);
    if (bnp->bgp_ifap != NULL && !BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
	bgp_recv_setdirect(tp);
    }
}


/*
 * bgp_close_socket - close a peer's socket
 */
PROTOTYPE(bgp_close_socket,
	  static void,
	 (bgpPeer *));
static void
bgp_close_socket(bnp)
bgpPeer *bnp;
{
    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_close_socket: peer %s",
	      bnp->bgp_name);
    }

    task_close(bnp->bgp_task);
    if (bnp->bgp_myaddr != NULL) {
	sockfree(bnp->bgp_myaddr);
	bnp->bgp_myaddr = NULL;
    }
    if (BGP_OPTIONAL_SHAREDIF(bnp->bgp_group) && bnp->bgp_ifap != NULL) {
	(void) ifa_free(bnp->bgp_ifap);
	bnp->bgp_ifap = NULL;
    }
}


/*
 *	Write spool support
 */

/*
 * bgp_spool_flush - flush as much data as possible from this peer's spool.
 *		     Return TRUE if this was error free (whether we actually
 *		     wrote anything or not), FALSE on hard errors.
 */
PROTOTYPE(bgp_spool_flush,
	  static int,
	  (bgpPeer *));
static int
bgp_spool_flush(bnp)
bgpPeer *bnp;
{
    bgp_spool_header *bshp;
    bgp_spool_buffer *bsbp;
    int sent_len, rc = 0, len;
    int times_around;
    int blocked;
    int total_sent;

    /*
     * Fetch the spool header.  If there isn't one we're in trouble
     */
    bshp = bnp->bgp_spool;
    assert(bshp != NULL);

    /*
     * Fetch the first spool buffer in the chain.  If nothing to do, return.
     * If there is something to do, compute the length of the first buffer.
     */
    bsbp = bshp->bgp_sph_head;
    if (bsbp == NULL) {
	/* XXX Consistancy check this */
	assert(bshp->bgp_sph_tail == NULL);
	assert(bshp->bgp_sph_count == 0);
	return (TRUE);
    }
    len = bsbp->bgp_sp_endp - bsbp->bgp_sp_startp;
    assert(len != 0);		/* XXX */

    /*
     * Go around while we've still got stuff to write
     */
    blocked = 0;
    times_around = 3;
    total_sent = 0;
    do {
	sent_len = write(bnp->bgp_task->task_socket,
			 (caddr_t)bsbp->bgp_sp_startp,
			 len);
	if (sent_len == len) {
	    /*
	     * Complete buffer gone, dump it and try the next one.
	     */
	    total_sent += sent_len;
	    bshp->bgp_sph_head = bsbp->bgp_sp_next;
	    task_block_free(bgp_spool_buf_blk_index, (caddr_t)bsbp);
	    bsbp = bshp->bgp_sph_head;
	    if (bsbp == NULL) {
		/*
		 * All done, clean up and terminate loop
		 */
		assert(bshp->bgp_sph_count == total_sent);	/* XXX */
		bshp->bgp_sph_tail = NULL;
		len = 0;
	    } else {
		/*
		 * Start us on the next buffer
		 */
		times_around = 3;
		len = bsbp->bgp_sp_endp - bsbp->bgp_sp_startp;
		assert(len != 0);		/* XXX */
	    }
	} else if (sent_len < 0) {
	    rc = errno;
	    switch(rc) {
	    case EHOSTUNREACH:
	    case ENETUNREACH:
		trace(TR_ALL, 0, "bgp_spool_flush: retrying write to %s: %m",
		  bnp->bgp_name);
		times_around--;
		break;

	    case EINTR:
		times_around--;
		break;

	    case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	    case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
		/*
		 * Would have blocked, we expect this in here.  Terminate
		 * the loop so we return.
		 */
		len = 0;
		break;

	    default:
		trace(TR_ALL, LOG_ERR,
		"bgp_spool_flush: sending %d (sent %d) bytes to %s failed: %m",
		  len,
		  total_sent,
		  bnp->bgp_name);
		bshp->bgp_sph_count -= total_sent;
		errno = rc;
		return (FALSE);
	    }
	} else if (sent_len == 0) {
	    trace(TR_ALL, LOG_ERR,
	"bgp_spool_flush: sending %d (sent %d) bytes to %s: connection closed",
	      len,
	      total_sent,
	      bnp->bgp_name);
	    bshp->bgp_sph_count -= total_sent;
	    return (FALSE);
	} else {
	    /*
	     * Partial write, i.e. 0 < sent_len < len.  Advance the pointer
	     * in the buffer and try again.
	     */
	    len -= sent_len;
	    total_sent += sent_len;
	    bsbp->bgp_sp_startp += sent_len;
	    times_around = 3;
	}
    } while (len > 0 && times_around > 0);

    /*
     * Decrement the buffer count by the amount we sent.  If we are looping
     * emit an error and return, otherwise give them an informational message.
     */
    bshp->bgp_sph_count -= total_sent;
    if (times_around == 0) {
	errno = rc;
	trace(TR_ALL, LOG_ERR,
	  "bgp_spool_flush: sending to %s (sent %d, %d remain) looping: %m",
	  total_sent,
	  bshp->bgp_sph_count,
	  bnp->bgp_name);
	errno = rc;
	return (FALSE);
    }

    trace(TR_BGP, 0,
      "bgp_spool_flush: sent %d byte%s to %s, %d byte%s still spooled",
      total_sent,
      ((total_sent == 1) ? "" : "s"),
      bnp->bgp_name,
      bshp->bgp_sph_count,
      ((bshp->bgp_sph_count == 1) ? "" : "s"));
    return (TRUE);
}


/*
 * bgp_spool_add - add a spool header to a peer and set up his write routine.
 */
PROTOTYPE(bgp_spool_add,
	  static void,
	  (bgpPeer *));
static void
bgp_spool_add(bnp)
bgpPeer *bnp;
{
    bgp_spool_header *bshp;

    assert(bnp->bgp_spool == NULL);	/* XXX */

    if (bgp_spool_hdr_blk_index == NULL) {
	/*
	 * First time through here.  Fetch a block_t pointer for the
	 * spool header structures.  Determine the buffer size we will
	 * use and fetch a block_t pointer for that too.
	 */
	bgp_spool_hdr_blk_index = task_block_init(sizeof(bgp_spool_header));
	if (task_pagesize > BGPMAXPACKETSIZE) {
	    bgp_spool_buf_size = BGPMAXPACKETSIZE;
	} else {
	    bgp_spool_buf_size = task_pagesize;
	}
	bgp_spool_buf_blk_index = task_block_init(bgp_spool_buf_size);
    }

    /*
     * Fetch the spool block header, attach it to the peer, initialize
     * it and hook the peer onto the tail of the list.
     */
    bshp = (bgp_spool_header *)task_block_alloc(bgp_spool_hdr_blk_index);
    bnp->bgp_spool = bshp;
    bshp->bgp_sph_next = NULL;
    bshp->bgp_sph_head = bshp->bgp_sph_tail = NULL;
    bshp->bgp_sph_count = 0;

    bgp_send_set(bnp, bgp_spool_trywrite);
}


/*
 * bgp_spool_remove - delete a peer's spool buffers and header.  Remove
 *		      the write routine from his task.
 */
PROTOTYPE(bgp_spool_remove,
	  static void,
	  (bgpPeer *));
static void
bgp_spool_remove(bnp)
bgpPeer *bnp;
{
    bgp_spool_header *bshp;
    bgp_spool_buffer *bsbp;

    if (bnp->bgp_spool != NULL) {
	bshp = bnp->bgp_spool;

	/*
	 * Dump his buffers and spool header.
	 */
	bsbp = bshp->bgp_sph_head;
	while (bsbp != NULL) {
	    bgp_spool_buffer *next_bsbp = bsbp->bgp_sp_next;
	    task_block_free(bgp_spool_buf_blk_index, (caddr_t)bsbp);
	    bsbp = next_bsbp;
	}
	task_block_free(bgp_spool_hdr_blk_index, (caddr_t)bshp);
	bnp->bgp_spool = NULL;
    }

    bgp_send_reset(bnp);
}

/*
 * bgp_spool_trywrite - select() sez peer is ready for writing, try to write
 *			some of the spooled data.
 */
static void
bgp_spool_trywrite(tp)
task *tp;
{
    bgpPeer *bnp;

    /*
     * Fetch the peer pointer.  The peer should have a spool buffer
     * attached if we are here.
     */
    bnp = (bgpPeer *)tp->task_data;
    assert(bnp->bgp_spool != NULL);

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	  "bgp_spool_trywrite: write ready for peer %s",
	  bnp->bgp_name);
    }

    /*
     * Flush what we can.  If we empty the spool, dump it.  If we
     * get an error, dump the peer.
     */
    if (bgp_spool_flush(bnp)) {
	if (bnp->bgp_spool->bgp_sph_count == 0) {
	    bgp_spool_remove(bnp);
	}
    } else {
	bgp_peer_close(bnp, BGPEVENT_ERROR);
    }
}


/*
 * bgp_spool_write - spool outgoing data to a peer, his socket may be full
 */
int
bgp_spool_write(bnp, pkt, length)
bgpPeer *bnp;
byte *pkt;
int length;
{
    bgp_spool_header *bshp;
    bgp_spool_buffer *bsbp, *old_bsbp;
    int bspace;

    /*
     * Sanity
     */
    if (length == 0) {
	return (TRUE);
    }

    /*
     * If this guy isn't on the spool list already, add him.  Otherwise
     * check to see if there is room for the write data.
     */
    bshp = bnp->bgp_spool;
    if (bshp == NULL) {
	bgp_spool_add(bnp);
	bshp = bnp->bgp_spool;
    } else if ((bshp->bgp_sph_count + length) > bnp->bgp_spool_bufsize) {
	/*
	 * No room, try to make some.
	 */
	if (!bgp_spool_flush(bnp)) {
	    return (FALSE);		/* bgp_spool_flush prints error */
	}
	if ((bshp->bgp_sph_count + length) > bnp->bgp_spool_bufsize) {
	    trace(TR_ALL, LOG_ERR,
"bgp_spool_write: peer %s write %d current spool %d exceeds limit of %d bytes",
	      bnp->bgp_name,
	      length,
	      bshp->bgp_sph_count,
	      bnp->bgp_spool_bufsize);
	    return (FALSE);
	}
    }

    /*
     * Loop until we have copied the entire request.
     */
    bshp->bgp_sph_count += length;
    bsbp = bshp->bgp_sph_tail;
    if (bsbp == NULL) {
	bspace = 0;
    } else {
        bspace = BGP_SPBUF_ENDBUF(bsbp, bgp_spool_buf_size) - bsbp->bgp_sp_endp;
    }

    for (;;) {		/* exits from middle */
	if (bspace > 0) {
	    if (bspace >= length) {
		bcopy((caddr_t)pkt, (caddr_t)bsbp->bgp_sp_endp, length);
		bsbp->bgp_sp_endp += length;
		break;
	    }
	    bcopy((caddr_t)pkt, (caddr_t)bsbp->bgp_sp_endp, bspace);
	    bsbp->bgp_sp_endp += bspace;
	    length -= bspace;
	    pkt += bspace;
	}

	/*
	 * More to do, get another buffer
	 */
	old_bsbp = bsbp;
	bsbp =(bgp_spool_buffer *) task_block_alloc(bgp_spool_buf_blk_index);
	bsbp->bgp_sp_startp = bsbp->bgp_sp_endp = BGP_SPBUF_STARTBUF(bsbp);
	bsbp->bgp_sp_next = NULL;
	bshp->bgp_sph_tail = bsbp;
	if (old_bsbp == NULL) {
	    assert(bshp->bgp_sph_head == NULL);		/* XXX */
	    bshp->bgp_sph_head = bsbp;
	} else {
	    old_bsbp->bgp_sp_next = bsbp;
	}
	bspace = BGP_SPBUF_ENDBUF(bsbp, bgp_spool_buf_size)
	  - BGP_SPBUF_STARTBUF(bsbp);
    }
    return (TRUE);
}



/*
 * bgp_set_connect_timer - create, if necessary, and start connect timer
 */
PROTOTYPE(bgp_set_connect_timer,
	  static void,
	 (bgpPeer *,
	  time_t));
static void
bgp_set_connect_timer(bnp, timeo)
bgpPeer *bnp;
time_t timeo;
{
    register int slot;
    register time_t add_interval;

    if (BIT_TEST(bnp->bgp_flags, BGPF_CONNECT)) {
	BGPCONN_DONE(bnp);
	BIT_RESET(bnp->bgp_flags, BGPF_TRY_CONNECT);
    }

    slot = BGPCONN_SLOT(timeo);
    if (bgp_connect_slots[slot] == 0) {
	/*
	 * Common case, I think.  His interval is okay, just bump
	 * the count and save the slot number.
	 */
	add_interval = 0;
    } else {
	register int i, s;
	register time_t intr;

	/*
	 * Slot not zero.  Go searching through looking for a better one.
	 */
	s = slot;
	add_interval = intr = 0;
	for (i = BGPCONN_SLOTDELAY; i > 0; i--) {
	    s = BGPCONN_NEXTSLOT(s);
	    intr += BGPCONN_SLOTSIZE;
	    if (bgp_connect_slots[s] < bgp_connect_slots[slot]) {
		slot = s;
		add_interval = intr;
		if (bgp_connect_slots[s] == 0) {
		    break;
		}
	    }
	}
    }

    /*
     * slot is now the best of them.  Save this and use it
     */
    bgp_connect_slots[slot]++;
    bnp->bgp_connect_slot = slot + 1;
    if (!BIT_TEST(bnp->bgp_flags, BGPF_CONNECT)) {
        (void) timer_create(bnp->bgp_task,
			BGPTIMER_CONNECT,
			"Connect",
			TIMERF_ABSOLUTE,
			(timeo + add_interval),
			(time_t) 0,
			bgp_connect_timeout,
			(caddr_t) 0);
	BIT_SET(bnp->bgp_flags, BGPF_CONNECT);
    } else {
	timer_set(bnp->bgp_task->task_timer[BGPTIMER_CONNECT],
	  (timeo + add_interval), (time_t) 0);
    }
}


/*
 * bgp_reset_connect_timer - reset the connect timer but don't delete it
 */
PROTOTYPE(bgp_reset_connect_timer,
	  static void,
	 (bgpPeer *));
static void
bgp_reset_connect_timer(bnp)
bgpPeer *bnp;
{
    if (BIT_TEST(bnp->bgp_flags, BGPF_CONNECT)) {
	BGPCONN_DONE(bnp);
	BIT_RESET(bnp->bgp_flags, BGPF_TRY_CONNECT);
	timer_reset(bnp->bgp_task->task_timer[BGPTIMER_CONNECT]);
    }
}


/*
 * bgp_delete_connect_timer - delete the connect timer
 */
PROTOTYPE(bgp_delete_connect_timer,
	  static void,
	 (bgpPeer *));
static void
bgp_delete_connect_timer(bnp)
bgpPeer *bnp;
{
    if (BIT_TEST(bnp->bgp_flags, BGPF_CONNECT)) {
	BIT_RESET(bnp->bgp_flags, BGPF_CONNECT|BGPF_TRY_CONNECT);
	BGPCONN_DONE(bnp);
	timer_delete(bnp->bgp_task->task_timer[BGPTIMER_CONNECT]);
    }
}


/*
 * bgp_peer_connected - a connection to a peer has opened, send an
 *   open message and prepare to receive the response.
 */
PROTOTYPE(bgp_peer_connected,
	  static void,
	 (bgpPeer *));
static void
bgp_peer_connected(bnp)
bgpPeer *bnp;
{
    sockaddr_un *addr;
    task *tp = bnp->bgp_task;

    trace(TR_BGP, 0, "bgp_peer_connected: connection established with %s",
	bnp->bgp_name);

    addr = task_get_addr_local(tp);
    if (addr == NULL) {
	int rc = errno;
	trace(TR_ALL, LOG_ERR,
	  "bgp_peer_connected: task_get_addr_local(%s): %m",
	  bnp->bgp_name);
	task_quit(rc);
    }

    if (bnp->bgp_myaddr == NULL) {
	bnp->bgp_myaddr = sockdup(addr);
    } else {
	sock2port(bnp->bgp_myaddr) = sock2port(addr);
    }

    bgp_delete_connect_timer(bnp);
    bgp_recv_setup(bnp, tp->task_socket, bgp_recv_open, "bgp_recv_open");
    bgp_buffer_alloc(&(bnp->bgp_inbuf));
    bgp_event(bnp, BGPEVENT_OPEN, BGPSTATE_OPENSENT);
    if (!bgp_send_open(bnp, BGP_VERSION_UNSPEC)) {
	bgp_peer_close(bnp, BGPEVENT_ERROR);
    } else {
        bgp_init_traffic_timer(bnp);
    }
}

/*
 * bgp_connect_complete - connect has completed, ready socket to recv
 */
PROTOTYPE(bgp_connect_complete,
	  static void,
	 (task *));
static void
bgp_connect_complete(tp)
task *tp;
{
    sockaddr_un *addr;
    bgpPeer *bnp = (bgpPeer *) tp->task_data;

    /*
     * Bump the connect failed counter here (it is somewhat misnamed
     * since it is bumped even if the connection fails).  It will be
     * cleared if the peer manages to become established.
     */
    bnp->bgp_connect_failed++;

    addr = task_get_addr_remote(tp);
    if (addr == NULL) {
	/*
	 * Nothing to do here except close socket and wait for
	 * new restart of the connection
	 */
	trace(TR_EXT, 0, "bgp_connect_complete: connection error: %m");
	bgp_close_socket(bnp);
        bgp_event(bnp, BGPEVENT_OPENFAIL, BGPSTATE_IDLE);
	/*
	 * See if we should do an immediate reconnect.  Otherwise,
	 * timer will start us.
	 */
	if (BIT_TEST(bnp->bgp_flags, BGPF_TRY_CONNECT)) {
	    BIT_RESET(bnp->bgp_flags, BGPF_TRY_CONNECT);
            bgp_event(bnp, BGPEVENT_START, BGPSTATE_CONNECT);
	    bgp_connect_start(bnp);
	} else {
            bgp_event(bnp, BGPEVENT_START, BGPSTATE_ACTIVE);
	}
    } else {
	bgp_peer_connected(bnp);
    }
}


/*
 * bgp_connect_start - initiate a connection to the neighbour
 */
static void
bgp_connect_start(bnp)
bgpPeer *bnp;
{
    task *tp = bnp->bgp_task;
    int bgp_socket;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_connect_start: peer %s",
	      bnp->bgp_name);
    }

    bgp_event(bnp, BGPEVENT_CONNRETRY, BGPSTATE_CONNECT);

    if ((bgp_socket = task_get_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	task_quit(errno);
    }
    BIT_SET(tp->task_flags, TASKF_CONNECT);
    tp->task_connect = bgp_connect_complete;
    task_set_socket(tp, bgp_socket);

    if (task_set_option(tp, TASKOPTION_NONBLOCKING, TRUE) < 0) {
	task_quit(errno);
    }
    if (task_set_option(tp, TASKOPTION_REUSEADDR, TRUE) < 0) {
	task_quit(errno);
    }

    /*
     * Here's how we do it.  If there is a local address specified,
     * copy this to bnp->bgp_myaddr (assumes the port is zero'd) and bind the
     * local end to this.  On the peer address set the port to bgp_port.
     */
    assert(bnp->bgp_myaddr == NULL);
    if (BGP_OPTIONAL_SHAREDIF(bnp->bgp_group)) {
	(void) bgp_set_peer_if(bnp, 0);
    }
    if (bnp->bgp_ifap != NULL || bnp->bgp_lcladdr != NULL) {
    	if (bnp->bgp_lcladdr != NULL) {
	    bnp->bgp_myaddr = sockdup(bnp->bgp_lcladdr->ifae_addr);
    	} else {
	    bnp->bgp_myaddr = sockdup(bnp->bgp_ifap->ifa_addr_local);
	}
	if (task_addr_local(tp, bnp->bgp_myaddr)) {
	    if (errno == EADDRNOTAVAIL) {
		trace(TR_ALL, LOG_WARNING,
  "bgp_connect_start: peer %s local address %A unavailable, connection failed",
		  bnp->bgp_name,
		  bnp->bgp_myaddr);
		bnp->bgp_connect_failed++;
		bgp_peer_close(bnp, BGPEVENT_OPENFAIL);
		return;
	    }
	    task_quit(errno);
	}
    }

    /* XXX Grot, fix */
    sock2port(bnp->bgp_addr) = bgp_port;
    sock2port(tp->task_addr) = bgp_port;
    if (task_connect(tp)) {
	if (errno == EINPROGRESS) {
	    if (bnp->bgp_myaddr == NULL) {
		sockaddr_un *addr;
		addr = task_get_addr_local(tp);
		if (addr == NULL) {
		    int rc = errno;
		    trace(TR_ALL, LOG_ERR,
		      "bgp_peer_connected: task_get_addr_local(%s): %m",
		      bnp->bgp_name);
		    task_quit(rc);
		}
		bnp->bgp_myaddr = sockdup(addr);
	    }
            bgp_set_connect_timer(bnp, BGPCONN_INTERVAL(bnp));
	} else {
	    trace(TR_BGP|TR_TASK, LOG_WARNING,
	      "bgp_connect_start: connect %s: %m",
	      bnp->bgp_name);
	      bnp->bgp_connect_failed++;
	      bgp_peer_close(bnp, BGPEVENT_OPENFAIL);
	}
    } else {
        bnp->bgp_connect_failed++;
	bgp_peer_connected(bnp);
    }
}


/*
 * bgp_connect_timeout - called when connect timer expires, attempt a connect
 */
static void
bgp_connect_timeout(tip, interval)
timer *tip;
time_t interval;
{
    task *tp = tip->timer_task;
    bgpPeer *bnp = (bgpPeer *) tp->task_data;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_connect_timeout: %s",
	      timer_name(tip));
    }

    if (bnp->bgp_connect_slot != 0) {
	bgp_connect_slots[bnp->bgp_connect_slot - 1]--;
        bnp->bgp_connect_slot = 0;
    }

    /*
     * If we're still waiting for connection completion, flag for
     * restart when the connection times out.  Otherwise, do it now.
     */
    if (bnp->bgp_state == BGPSTATE_CONNECT) {
	bgp_reset_connect_timer(bnp);
	BIT_SET(bnp->bgp_flags, BGPF_TRY_CONNECT);
    } else {
        bgp_connect_start(bnp);
    }
}


/*
 * bgp_set_traffic_timer - set the traffic timer
 */
PROTOTYPE(bgp_set_traffic_timer,
	  static void,
	 (bgpPeer *,
	  time_t));
static void
bgp_set_traffic_timer(bnp, timeo)
bgpPeer *bnp;
time_t timeo;
{
    if (!BIT_TEST(bnp->bgp_flags, BGPF_TRAFFIC)) {
        (void) timer_create(bnp->bgp_task,
			BGPTIMER_TRAFFIC,
			"Traffic",
			TIMERF_ABSOLUTE,
			timeo,
			(time_t) 0,
			bgp_traffic_timeout,
			(caddr_t) 0);
	BIT_SET(bnp->bgp_flags, BGPF_TRAFFIC);
	bnp->bgp_last_rcvd = bnp->bgp_last_sent
	  = bnp->bgp_last_checked = bgp_time_sec;
    } else {
	timer_set(bnp->bgp_task->task_timer[BGPTIMER_TRAFFIC],
	  timeo, (time_t) 0);
    }
    bnp->bgp_traffic_interval = timeo;
}


/*
 * bgp_delete_traffic_timer - delete the traffic timer
 */
PROTOTYPE(bgp_delete_traffic_timer,
	  static void,
	 (bgpPeer *));
static void
bgp_delete_traffic_timer(bnp)
bgpPeer *bnp;
{
    if (BIT_TEST(bnp->bgp_flags, BGPF_TRAFFIC)) {
	BIT_RESET(bnp->bgp_flags, BGPF_TRAFFIC);
	timer_delete(bnp->bgp_task->task_timer[BGPTIMER_TRAFFIC]);
	bnp->bgp_last_rcvd = bnp->bgp_last_sent = bnp->bgp_last_checked = 0;
    }
}


/*
 * bgp_init_traffic_timer - (re)start the traffic timer for a peer
 */
static void
bgp_init_traffic_timer(bnp)
bgpPeer *bnp;
{
    task *tp = bnp->bgp_task;

    /*
     * If we are not established, just set the timer for our
     * current holdtime.  This will provide a time out for open
     * negotiation.  If we are established, however, set the timer
     * for the minimum of our holdtime/3 and his holdtime.  In
     * either case set the times to the current time.
     */
    if (BIT_TEST(bnp->bgp_flags, BGPF_TRAFFIC)) {
	timer_reset(tp->task_timer[BGPTIMER_TRAFFIC]);
    }
    bnp->bgp_last_rcvd = bnp->bgp_last_sent
      = bnp->bgp_last_checked = bgp_time_sec;
    if (bnp->bgp_state != BGPSTATE_ESTABLISHED) {
	bgp_set_traffic_timer(bnp, bnp->bgp_holdtime_out);
    } else {
	time_t timeo;

	timeo = bnp->bgp_holdtime_out/3;
	if (bnp->bgp_hisholdtime < timeo) {
	    timeo = bnp->bgp_hisholdtime;
	}
	bgp_set_traffic_timer(bnp, timeo);
    }
}



/*
 * bgp_traffic_timeout - set the traffic timeout
 */
static void
bgp_traffic_timeout(tip, interval)
timer *tip;
time_t interval;
{
    task *tp = tip->timer_task;
    bgpPeer *bnp = (bgpPeer *) tp->task_data;
    time_t next_rcvd_time, next_sent_time;
    time_t next_interval;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_traffic_timeout: peer %s last checked %d last recv'd %d last sent %d",
	      bnp->bgp_name,
	      bgp_time_sec - bnp->bgp_last_checked,
	      bgp_time_sec - bnp->bgp_last_rcvd,
	      bgp_time_sec - bnp->bgp_last_sent);
    }

    /*
     * If we're past the hold time, inform someone and return.
     * Otherwise, if we're past the keepalive time, send a keepalive.
     */
    bnp->bgp_last_checked = bgp_time_sec;

    /*
     * If we aren't in establshed, or if he exceeded the holdtime,
     * terminate him.
     */
    next_rcvd_time = bnp->bgp_last_rcvd + bnp->bgp_hisholdtime;
    if (bnp->bgp_state != BGPSTATE_ESTABLISHED || (next_rcvd_time
      = bnp->bgp_last_rcvd + bnp->bgp_hisholdtime) <= bgp_time_sec) {
	trace(TR_EXT, LOG_ERR,
	  "bgp_traffic_timeout: holdtime expired for %s",
	  bnp->bgp_name);
	bgp_send_notify_none(bnp, BGP_ERR_HOLDTIME, 0);
	bgp_peer_close(bnp, BGPEVENT_HOLDTIME);
	return;
    }

    next_sent_time = bnp->bgp_last_sent + bnp->bgp_holdtime_out/3;
    if (next_sent_time <= bgp_time_sec) {
	/* Only send one when established */
	if (bnp->bgp_state == BGPSTATE_ESTABLISHED) {
	    if (!bgp_send_keepalive(bnp)) {
		trace(TR_ALL, LOG_ERR,
		  "bgp_traffic_timeout: error sending KEEPALIVE to %s: %m",
		  bnp->bgp_name);
		bgp_peer_close(bnp, BGPEVENT_ERROR);
		return;
	    }
	}
	bnp->bgp_last_sent = bgp_time_sec;
	next_sent_time = bgp_time_sec + bnp->bgp_holdtime_out/3;
    }

    /*
     * Compute the next interval to use.  If it is less than what
     * we're using now, or greater by more than BGP_TIME_ERROR seconds,
     * change the interval.
     */
    if (next_rcvd_time < next_sent_time) {
	next_interval = next_rcvd_time - bgp_time_sec;
    } else {
	next_interval = next_sent_time - bgp_time_sec;
    }
    if ((next_interval > bnp->bgp_traffic_interval
        && (next_interval - bnp->bgp_traffic_interval) > BGP_TIME_ERROR)
      || (next_interval < bnp->bgp_traffic_interval)) {
	bgp_set_traffic_timer(bnp, next_interval);
    }
}


/*
 * bgp_listen_accept - process an incoming connection
 */
PROTOTYPE(bgp_listen_accept,
	  static void,
	 (task *));
static void
bgp_listen_accept(listen_tp)
task *listen_tp;
{
    task *tp;
    bgpProtoPeer *ppp;
    struct sockaddr_in inaddr;
    int inaddrlen;
    int s;
    sockaddr_un *myaddr;

    /*
     * Accept the connection
     */
    inaddrlen = sizeof(inaddr);
    if ((s = accept(listen_tp->task_socket,
      (struct sockaddr *) &inaddr, &inaddrlen)) < 0) {
	trace(TR_ALL, LOG_ERR, "bgp_listen_accept: accept(%d): %m",
	  listen_tp->task_socket);
	return;
    }

    /*
     * Now allocate a task to go with this.  Stick the address/port
     * in there, along with the socket number.
     */
    tp = task_alloc("BGP_Proto", TASKPRI_EXTPROTO);
    tp->task_socket = s;
    tp->task_addr = sockdup(sock2gated((struct sockaddr *) &inaddr, inaddrlen));

    /*
     * Determine the local address if we don't know it already
     */
    myaddr = task_get_addr_local(tp);
    if (myaddr == NULL) {
	trace(TR_ALL, LOG_ERR,
	      "bgp_listen_accept: task_get_addr_local() failed, terminating!!");
	task_quit(EINVAL);
    }
    myaddr = sockdup(myaddr);


    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_listen_accept: accepting connection from %#A (local %#A)",
	      tp->task_addr,
	      myaddr);
    }

    /*
     * Initialize the task structure
     */
    tp->task_rtproto = RTPROTO_BGP;
    tp->task_recv = bgp_pp_recv;
    tp->task_dump = bgp_pp_dump;
    tp->task_terminate = bgp_fake_terminate;
    tp->task_n_timers = BGP_PEER_TIMER_MAX;
    tp->task_trace_flags = bgp_default_trace_flags;	/* XXX not here */


    if (!task_create(tp)) {
	task_quit(EINVAL);
    }

    /*
     * Set options for receiving
     */
    bgp_recv_setopts(tp, BGP_RECV_BUFSIZE, BGP_SEND_BUFSIZE);

    /*
     * Build a proto-peer structure to go along with this
     */
    ppp = bgp_pp_create(tp, myaddr);

    /*
     * Set a timeout for the receive using the traffic timer
     */
    (void) timer_create(tp,
			BGPTIMER_TRAFFIC,
			"OpenTimeOut",
			TIMERF_ABSOLUTE,
			(time_t) BGP_OPEN_TIMEOUT,
			(time_t) 0,
			bgp_pp_timeout,
			(caddr_t) 0);
}


/*
 * bgp_listen_start - delete the startup timer and start listening on
 *   the BGP socket
 */
PROTOTYPE(bgp_listen_start,
	  static void,
	 (timer *,
	  time_t));
static void
bgp_listen_start(tip, interval)
timer *tip;
time_t interval;
{
    task *tp = tip->timer_task;
    int s;

    /* Must be listen task */
    assert(tp == bgp_listen_task);

    if ((s = task_get_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	s = errno;
	trace(TR_ALL, LOG_ERR,
	  "bgp_listen_start: couldn't get BGP listen socket!!");
	task_quit(s);
    }

    tp->task_flags |= TASKF_ACCEPT;
    tp->task_addr = sockdup(inet_addr_any);
    sock2port(tp->task_addr) = bgp_port;
    tp->task_accept = bgp_listen_accept;
    task_set_socket(tp, s);

    if (task_set_option(tp, TASKOPTION_REUSEADDR, TRUE) < 0) {
	task_quit(errno);
    }
    if (task_addr_local(tp, tp->task_addr)) {
	task_quit(errno);
    }

    if (listen(s, 5) < 0) {	/* XXX - need a task routine to listen */
	trace(TR_ALL, LOG_ERR, "bgp_listen_init: listen: %m");
	task_quit(errno);
    }
}


/*
 * bgp_listen_init - create a task for listening, and set a timer so we'll
 *   actually start doing this a little while into the future.
 */
PROTOTYPE(bgp_listen_init,
	  static void,
	 (void));
static void
bgp_listen_init()
{
    int bgp_socket;

    if (!bgp_listen_task) {
	if ((bgp_socket = task_get_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    task_quit(errno);
	}

	bgp_listen_task = task_alloc("BGP", TASKPRI_PROTO);
	bgp_listen_task->task_trace_flags = bgp_default_trace_flags;
	bgp_listen_task->task_rtproto = RTPROTO_BGP;
	bgp_listen_task->task_cleanup = bgp_cleanup;
	bgp_listen_task->task_flash = bgp_flash;
	bgp_listen_task->task_reinit = bgp_reinit;
	bgp_listen_task->task_newpolicy = bgp_newpolicy;
	bgp_listen_task->task_dump = bgp_dump;
	bgp_listen_task->task_terminate = bgp_terminate;
	bgp_listen_task->task_ifachange = bgp_ifachange;
	bgp_listen_task->task_n_timers = BGP_LISTEN_TIMER_MAX;
	    
	if (!task_create(bgp_listen_task)) {
	    task_quit(EINVAL);
	}

	(void) timer_create(bgp_listen_task,
			    BGPTIMER_LISTEN,
			    "Listen_Init",
			    TIMERF_ABSOLUTE|TIMERF_DELETE,
			    (time_t) BGP_LISTEN_TIMEOUT,
			    (time_t) 0,
			    bgp_listen_start,
			    (caddr_t) 0);
    }
}


/*
 * bgp_listen_stop - stop listening for connections by deleting the listen task
 */
PROTOTYPE(bgp_listen_stop,
	  static void,
	 (void));
static void
bgp_listen_stop()
{
    task_delete(bgp_listen_task);
    bgp_listen_task = NULL;
}


/*
 * bgp_set_peer_if - for a peer which can use an interface on a shared
 *   subnet, find the shared interface.  Return the interface pointer
 *   if one is found, NULL otherwise.  Complain if the interface isn't
 *   there and he told you to be verbose.
 */
static if_addr *
bgp_set_peer_if(bnp, logit)
bgpPeer *bnp;
int logit;
{
    if_addr *ifap;

    if (BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
	ifap = if_withdst(bnp->bgp_gateway);
    } else {
	ifap = if_withdst(bnp->bgp_addr);
    }

    if (ifap == NULL) {
	if (logit) {
	    if (BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
		trace(TR_ALL, LOG_WARNING,
		      "bgp_set_peer_if: BGP peer %s interface for gateway %A not found.  Leaving peer idled",
		      bnp->bgp_name,
		      bnp->bgp_gateway);
	    } else {
		trace(TR_ALL, LOG_WARNING,
	"bgp_set_peer_if: BGP peer %s interface not found.  Leaving peer idled",
		  bnp->bgp_name);
	    }
	}
	return ((if_addr *)0);
    }

    if (bnp->bgp_lcladdr!=NULL && ifap->ifa_addrent_local!=bnp->bgp_lcladdr) {
	if (logit) {
	    trace(TR_ALL, LOG_WARNING,
		  "bgp_set_peer_if: BGP peer %s local address %A not on shared net.  Leaving peer idled",
		  bnp->bgp_name,
		  bnp->bgp_lcladdr->ifae_addr);
	}
	return ((if_addr *)0);
    }

    bnp->bgp_ifap = ifa_alloc(ifap);
    return (ifap);
}


/*
 *	Group and peer location routines
 */

/*
 * bgp_find_group - find a bgp group that matches the peer's address,
 *		    as and authentication.
 */
bgpPeerGroup *
bgp_find_group __PF7(addr, sockaddr_un *,
		     lcladdr, sockaddr_un *,
		     as, as_t,
		     myas, as_t,
		     authcode, int,
		     authdata, byte *,
		     authdatalen, int)
{
    register bgpPeerGroup *bgp;
    register bgpPeer *bnp;
    bgpPeerGroup *okgroup = (bgpPeerGroup *)NULL;
    if_addr *ifap;

    /*
     * Determine the shared interface if there is one
     */
    ifap = if_withdst(addr);
    if (ifap != NULL && !sockaddrcmp_in(ifap->ifa_addr_local, lcladdr)) {
	ifap = NULL;
    }

    /*
     * We look through the list of bgpPeerGroups looking for something
     * which matches the address, incoming AS and authentication.  We
     * save the first one we match.  As there could be more than one
     * matching group, however, we search the entire table for a matching
     * peer as well.  If the matching peer is in another group we return
     * that group instead.
     *
     * This is a crock of shit, actually.  I should think harder about
     * what it is exactly we are trying to match.
     */
    BGP_GROUP_LIST(bgp) {
	if (as != bgp->bgpg_peer_as) {
	    continue;
	}
	if (myas != 0 && myas != bgp->bgpg_local_as) {
	    continue;
	}
	if (BIT_TEST(bgp->bgpg_flags, BGPGF_IDLED)) {
	    continue;
	}
	if (okgroup == NULL
	  && bgp->bgpg_allow != NULL
	  && adv_destmask_match(bgp->bgpg_allow, addr)) {
	    int groupisok = 1;

	    if (BGP_NEEDS_SHAREDIF(bgp)) {
		if (bgp->bgpg_gateway != NULL) {
		    if_addr *ifap2;

		    ifap2 = if_withdst(bgp->bgpg_gateway);
		    if (!sockaddrcmp_in(ifap2->ifa_addr_local, lcladdr)) {
			groupisok = 0;
		    }
		} else if (ifap == NULL) {
		    groupisok = 0;
		}
	    }
	    if (groupisok
		&& bgp->bgpg_lcladdr != NULL
		&& !sockaddrcmp_in(bgp->bgpg_lcladdr->ifae_addr, lcladdr)) {
		groupisok = 0;
	    }
	    if (groupisok && !bgp_open_auth(NULL, &bgp->bgpg_authinfo,
	      authcode, authdata, authdatalen)) {
		groupisok = 0;
	    }
	    if (groupisok) {
		okgroup = bgp;
	    }
	}

	BGP_PEER_LIST(bgp, bnp) {
	    if (sockaddrcmp_in(addr, bnp->bgp_addr)) {
		break;
	    }
	} BGP_PEER_LIST_END(bgp, bnp);

	if (bnp != NULL) {
	    int peerstinks = 0;

	    if (BIT_TEST(bnp->bgp_flags, BGPF_IDLED|BGPF_UNCONFIGURED)) {
		peerstinks = 1;
	    } else if (BGP_NEEDS_SHAREDIF(bgp)) {
		if (bnp->bgp_gateway != NULL) {
		    if (!sockaddrcmp_in(bnp->bgp_ifap->ifa_addr_local,
					lcladdr)) {
			peerstinks = 1;
		    }
		} else if (bnp->bgp_ifap != ifap) {
		    peerstinks = 1;
		}
	    } else if (bnp->bgp_lcladdr != NULL) {
		if (!sockaddrcmp_in(bnp->bgp_lcladdr->ifae_addr, lcladdr)) {
		    peerstinks = 1;
		}
	    }

	    if (!peerstinks && !bgp_open_auth(NULL, &bnp->bgp_authinfo,
	      authcode, authdata, authdatalen)) {
		peerstinks = 1;
	    }

	    if (!peerstinks) {
		/* good one, return group */
		return (bgp);
	    }

	    /*
	     * If this peer is rotten but we liked his group, forget
	     * about the group.  The peer entry overrides.
	     */
	    if (bnp->bgp_group == okgroup) {
		okgroup = NULL;
	    }
	}
    } BGP_GROUP_LIST_END(bgp);

    return (okgroup);
}


/*
 * bgp_find_peer - find a peer in a particular group, return NULL if
 *   not there.  This assumes the bgp_find_group() was previously called,
 *   so that it is sufficient to simply match the peer address.
 */
bgpPeer *
bgp_find_peer(bgp, addr, lcladdr)
bgpPeerGroup *bgp;
sockaddr_un *addr;
sockaddr_un *lcladdr;
{
    register bgpPeer *bnp;

    BGP_PEER_LIST(bgp, bnp) {
	if (sockaddrcmp_in(addr, bnp->bgp_addr)) {
	    return (bnp);
	}
    } BGP_PEER_LIST_END(bgp, bnp);

    return (bgpPeer *)NULL;
}


/*
 * bgp_find_group_by_addr - find a group when the only thing we know
 *   about a peer is its addresses.  This is a gross and rotten, and
 *   unlikely to return particularly correct answers, but is necessary
 *   to return error messages to protopeers.
 *
 *   I would combine this with bgp_find_group(), but I really think
 *   the spec should change to allow NOTIFY messages before OPEN
 *   messages so this could just go away.  So it is separate to make
 *   it easy to delete.
 */
bgpPeerGroup *
bgp_find_group_by_addr(addr, lcladdr)
sockaddr_un *addr;
sockaddr_un *lcladdr;
{
    register bgpPeerGroup *bgp;
    register bgpPeer *bnp;
    bgpPeerGroup *okgroup = (bgpPeerGroup *)NULL;
    if_addr *ifap;

    /*
     * Determine the shared interface if there is one
     */
    ifap = if_withdst(addr);
    if (ifap != NULL && !sockaddrcmp_in(ifap->ifa_addr_local, lcladdr)) {
	ifap = NULL;
    }

    /*
     * Look for group and peer address matches.  Note that the requirement
     * for a shared interface will help eliminate some groups.
     */
    BGP_GROUP_LIST(bgp) {
	if (okgroup == NULL
	  && bgp->bgpg_allow != NULL
	  && adv_destmask_match(bgp->bgpg_allow, addr)) {
	    int groupisok = 1;

	    if (BGP_NEEDS_SHAREDIF(bgp)) {
		if (bgp->bgpg_gateway != NULL) {
		    if_addr *ifap2;

		    ifap2 = if_withdst(bgp->bgpg_gateway);
		    if (!sockaddrcmp_in(ifap2->ifa_addr_local, lcladdr)) {
			groupisok = 0;
		    }
		} else if (ifap == NULL) {
		    groupisok = 0;
		}
	    }
	    if (groupisok
		&& BIT_TEST(bgp->bgpg_options, BGPO_LCLADDR)
		&& !sockaddrcmp_in(bgp->bgpg_lcladdr->ifae_addr, lcladdr)) {
		groupisok = 0;
	    }
	    if (groupisok) {
		okgroup = bgp;
	    }
	}

	BGP_PEER_LIST(bgp, bnp) {
	    if (sockaddrcmp_in(addr, bnp->bgp_addr)) {
		break;
	    }
	} BGP_PEER_LIST_END(bgp, bnp);

	if (bnp != NULL) {
	    int peerstinks = 0;

	    if (BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)) {
		peerstinks = 1;
	    } else if (BGP_NEEDS_SHAREDIF(bgp)) {
		if (bnp->bgp_gateway != NULL) {
		    if (!sockaddrcmp_in(bnp->bgp_ifap->ifa_addr_local,
					lcladdr)) {
			peerstinks = 1;
		    }
		} else if (bnp->bgp_ifap != ifap) {
		    peerstinks = 1;
		}
	    } else if (bnp->bgp_lcladdr != NULL) {
		if (!sockaddrcmp_in(bnp->bgp_lcladdr->ifae_addr, lcladdr)) {
		    peerstinks = 1;
		}
	    }

	    if (!peerstinks) {
		/* good one, return group */
		return (bgp);
	    }

	    /*
	     * If this peer is rotten but we liked his group, forget
	     * about the group.  The peer entry overrides.
	     */
	    if (bnp->bgp_group == okgroup) {
		okgroup = NULL;
	    }
	}
    } BGP_GROUP_LIST_END(bgp);

    return (okgroup);
}


/*
 *	Auxilliary protocol initiation and termination
 */

/*
 * bgp_aux_initiate - initiate an internal BGP session when the IGP
 *   is up and ready to go.
 */
static void
bgp_aux_initiate __PF3(tp, task *,
		       proto, proto_t,
		       rtbit, u_int)
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;

    bgp = (bgpPeerGroup *)tp->task_data;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_aux_initiate: starting %s for %s",
	  bgp->bgpg_name,
	  trace_state(rt_proto_bits, proto));
    }

    /* Non IGP group */
    assert(bgp->bgpg_type == BGPG_INTERNAL_IGP);

    /* Non idled group */
    assert(BIT_TEST(bgp->bgpg_flags, BGPGF_IDLED));

    /*
     * All okay.  Save the rtbit.  Go through the list of peers,
     * initializing any which are ready to go.
     */
    bgp->bgpg_igp_rtbit = rtbit;
    bgp->bgpg_igp_proto = proto;
    BIT_RESET(bgp->bgpg_flags, BGPGF_IDLED);

    BGP_PEER_LIST(bgp, bnp) {
	/* Peer not idle */
	assert(bnp->bgp_state == BGPSTATE_IDLE);

	if (!BIT_TEST(bnp->bgp_flags, BGPF_IDLED)) {
	    bgp_peer_start(bnp, BGPCONN_INIT);
	}
    } BGP_PEER_LIST_END(bgp, bnp);
}


/*
 * bgp_aux_terminate - the IGP is giving up, idle the associated BGP
 */
PROTOTYPE(bgp_aux_terminate,
	  static void,
	 (task *));
static void
bgp_aux_terminate(tp)
task *tp;
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;

    bgp = (bgpPeerGroup *)tp->task_data;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_aux_terminate: terminating %s for %s",
	  bgp->bgpg_name,
	  trace_state(rt_proto_bits, bgp->bgpg_igp_proto));
    }

    /* Non-IGP group */
    assert(bgp->bgpg_type == BGPG_INTERNAL_IGP);

    /* Already idled */
    assert(!BIT_TEST(bgp->bgpg_flags, BGPGF_IDLED));

    /*
     * Okay.  Idle the group and terminate all running peers.
     */
    BIT_SET(bgp->bgpg_flags, BGPGF_IDLED);
    BGP_PEER_LIST(bgp, bnp) {
	if (bnp->bgp_state != BGPSTATE_IDLE) {
	    if (bnp->bgp_state == BGPSTATE_OPENSENT
	      || bnp->bgp_state == BGPSTATE_OPENCONFIRM
	      || bnp->bgp_state == BGPSTATE_ESTABLISHED) {
		bgp_send_notify_none(bnp, BGP_CEASE, 0);
	    }
	    bgp_peer_close(bnp, BGPEVENT_STOP);
	}
    }
    bgp->bgpg_igp_rtbit = 0;
    bgp->bgpg_igp_proto = 0;
}



/*
 *	Protopeer support routines.
 */

/*
 * bgp_pp_dump - dump BGP protopeer status to dump file
 */
static void
bgp_pp_dump(tp, fd)
task *tp;
FILE *fd;
{
    bgpProtoPeer *bpp;

    bpp = (bgpProtoPeer *) tp->task_data;

    /*
     * Not much to dump here
     */
    (void) fprintf(fd,
       "\n\tPeer Address: %#A\tLocal Address: %#A\tConnected seconds: %d\n",
	bpp->bgpp_addr,
	bpp->bgpp_myaddr,
	bgp_time_sec - bpp->bgpp_connect_time);

    if (bpp->bgpp_bufpos != bpp->bgpp_readptr) {
	(void) fprintf(fd, "\t\tReceived and buffered octets: %u\n",
	  bpp->bgpp_readptr - bpp->bgpp_bufpos);
    }
}


/*
 * bgp_pp_timeout - receive a timeout notification for a protopeer
 */
static void
bgp_pp_timeout(tip, interval)
timer *tip;
time_t interval;
{
    task *tp = tip->timer_task;
    bgpProtoPeer *bpp = (bgpProtoPeer *) tp->task_data;

    trace(TR_ALL, LOG_WARNING,
	"bgp_pp_timeout: peer %s timed out waiting for OPEN",
	bpp->bgpp_name);

    /*
     * Same as above.  Send the default open message, then send
     * a holdtime notification.  Then dump him.
     */
    bgp_pp_notify_none(bpp, (bgpPeerGroup *)0, (bgpPeer *)0,
      BGP_ERR_HOLDTIME, 0);
    bgp_pp_delete(bpp);
}



/*
 *	Peer creation/deletion/state transition handling
 */


/*
 * bgp_peer_create - create a peer in the specified group
 */
PROTOTYPE(bgp_peer_create,
	  static bgpPeer *,
	 (bgpPeerGroup *));
static bgpPeer *
bgp_peer_create(bgp)
bgpPeerGroup *bgp;
{
    bgpPeer *bnp;

    /*
     * Fetch a peer structure and zero it
     */
    bnp = bgp_peer_alloc(bgp->bgpg_task);
    bzero((caddr_t)bnp, sizeof(bgpPeer));
    bnp->bgp_group = bgp;

    /*
     * Copy configuration information out of the group structure
     */
    bnp->bgp_conf = bgp->bgpg_conf;	/* struct copy */
    if (BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
	bnp->bgp_gateway = sockdup(bnp->bgp_gateway);
    }
    if (BIT_TEST(bnp->bgp_options, BGPO_LCLADDR)) {
	bnp->bgp_lcladdr = ifae_alloc(bnp->bgp_lcladdr);
    }

    /*
     * Now set up the gw entry.  Mostly zeros at this point
     */
    bnp->bgp_proto = RTPROTO_BGP;
    bnp->bgp_peer_as = bgp->bgpg_peer_as;
    bnp->bgp_local_as = bgp->bgpg_local_as;
    if (bgp->bgpg_type == BGPG_INTERNAL_IGP) {
	BIT_SET(bnp->bgp_gw.gw_flags, GWF_AUXPROTO);
    }

    /*
     * That's all we can do for now
     */
    return (bnp);
}


/*
 * bgp_new_peer - create a new peer from a protopeer in the specified
 *   peer group
 */
bgpPeer *
bgp_new_peer(bgp, bpp, length)
bgpPeerGroup *bgp;
bgpProtoPeer *bpp;
int length;
{
    bgpPeer *bnp;
    int save_socket;

    /*
     * Fetch a new peer
     */
    bnp = bgp_peer_create(bgp);

    /*
     * Set the flags.  Mark this guy unconfigured
     */
    bnp->bgp_flags = BGPF_UNCONFIGURED;

    /*
     * Now set up the gw entry.  This mostly requires assembling the
     * data from other places.
     */
    bnp->bgp_addr = sockdup(bpp->bgpp_task->task_addr);
    bgp_make_names(bnp);
    /* XXX this policy stuff isn't quite right.  Think about it */
    switch (bgp->bgpg_type) {
    case BGPG_EXTERNAL:
    case BGPG_INTERNAL:
	bnp->bgp_import = control_exterior_locate(bgp_import_list, bnp->bgp_peer_as);
	bnp->bgp_export = control_exterior_locate(bgp_export_list, bnp->bgp_peer_as);
	break;
    }

    /*
     * Copy the local address from the protopeer.  If this requires
     * an interface pointer, determine this now.
     */
    bnp->bgp_myaddr = bpp->bgpp_myaddr;
    bpp->bgpp_myaddr = NULL;
    if (BGP_USES_SHAREDIF(bgp)) {
	if (bgp_set_peer_if(bnp, 0) == NULL) {
	    assert(!BGP_NEEDS_SHAREDIF(bgp));
	}
    }

    /*
     * Link it to the group structure.  This must not be moved north
     * of here since bgp_peer_add() also does a bgp_sort_add(), and
     * bgp_sort_add depends on the addresses.
     */
    bgp_peer_add(bgp, bnp);

    /*
     * Initialize counters
     */
    bnp->bgp_in_notupdates = 1;
    bnp->bgp_in_octets = (u_long)length;

    /*
     * Copy the buffer in from the protopeer
     */
    bgp_buffer_copy(&(bpp->bgpp_inbuf), &(bnp->bgp_inbuf));

    /*
     * Okay so far.  Now save the socket and reset it in the protopeer's
     * task.  Then delete the protopeer and create a new task for the peer.
     */
    save_socket = bpp->bgpp_task->task_socket;
    task_reset_socket(bpp->bgpp_task);
    bgp_pp_delete(bpp);
    bgp_task_create(bnp);
    task_set_socket(bnp->bgp_task, save_socket);
    bgp_recv_setbuf(bnp->bgp_task, bnp->bgp_recv_bufsize,bnp->bgp_send_bufsize);

    /*
     * Churn this peer into active state from idle, the caller will
     * do the rest.
     */
    bnp->bgp_state = BGPSTATE_IDLE;
    /* XXX This transition to active violates the FSM.  The FSM is shitty */
    bgp_event(bnp, BGPEVENT_START, BGPSTATE_ACTIVE);

    /*
     * Start the connection timer.
     */
    bgp_init_traffic_timer(bnp);

    return (bnp);
}


/*
 * bgp_use_protopeer - a protopeer has been found to match a configured
 *   peer.  We shut down the peer's socket, delete the connect timer
 *   if there is one, reset the traffic timer if this is running, move
 *   the protopeer's socket to the peer and do the state transitions to
 *   get this to active.
 */
void
bgp_use_protopeer(bnp, bpp, length)
bgpPeer *bnp;
bgpProtoPeer *bpp;
int length;
{
    task *tp;
    int save_socket;

    /*
     * Check to make sure we aren't in idle state.  If we are
     * someone really screwed up.
     */
    /* State is IDLE!! */
    assert(bnp->bgp_state != BGPSTATE_IDLE);

    /*
     * Connect timer first
     */
    tp = bnp->bgp_task;
    if (BIT_TEST(bnp->bgp_flags, BGPF_CONNECT)) {
	bgp_delete_connect_timer(bnp);
    }

    /*
     * Got that all shut down.  If the peer has a socket (i.e.
     * is not in IDLE or ACTIVE) close the socket and delete
     * it.  If the peer has buffers (i.e. not IDLE or ACTIVE or
     * CONNECT) delete those too. Then move the protopeer's socket
     * and dump the protopeer.
     */
    if (bnp->bgp_state != BGPSTATE_IDLE && bnp->bgp_state != BGPSTATE_ACTIVE) {
	task_close(tp);
	if (bnp->bgp_state != BGPSTATE_CONNECT) {
	    bgp_buffer_free(&(bnp->bgp_inbuf));
	}
    }
    save_socket = bpp->bgpp_task->task_socket;
    task_reset_socket(bpp->bgpp_task);
    task_set_socket(tp, save_socket);
    bgp_recv_setbuf(tp, bnp->bgp_recv_bufsize, bnp->bgp_send_bufsize);
    bgp_buffer_copy(&(bpp->bgpp_inbuf), &(bnp->bgp_inbuf));

    sock2port(bnp->bgp_addr) =
	sock2port(tp->task_addr) =
	    sock2port(bpp->bgpp_task->task_addr);

    /*
     * Sanity check this to make sure the local address is correct.  The
     * code which found this peer should have done this already but an
     * extra check is cheap.
     */
    if (BGP_NEEDS_SHAREDIF(bnp->bgp_group)) {
	assert(bnp->bgp_ifap != NULL);
	assert(sockaddrcmp_in(bnp->bgp_ifap->ifa_addr_local, bpp->bgpp_myaddr));
    } else if (BIT_TEST(bnp->bgp_options, BGPO_LCLADDR)) {
	assert(sockaddrcmp_in(bnp->bgp_lcladdr->ifae_addr, bpp->bgpp_myaddr));
    }

    if (bnp->bgp_myaddr != NULL) {
	sockfree(bnp->bgp_myaddr);
    }
    bnp->bgp_myaddr = bpp->bgpp_myaddr;
    bpp->bgpp_myaddr = NULL;

    /*
     * If there is no shared interface currently, but the flavour of BGP
     * wants to know should a shared interface exist, determine this now.
     */
    if (bnp->bgp_ifap == NULL && BGP_USES_SHAREDIF(bnp->bgp_group)) {
	if_addr *ifap;

	if (bnp->bgp_gateway != NULL) {
	    ifap = if_withdst(bnp->bgp_gateway);
	} else {
	    ifap = if_withdst(bnp->bgp_addr);
	}

	if (ifap != NULL && sockaddrcmp_in(ifap->ifa_addrent_local->ifae_addr,
					   bnp->bgp_myaddr)) {
	    bnp->bgp_ifap = ifa_alloc(ifap);
	}
    }

    /*
     * Dump the protopeer.
     */
    bgp_pp_delete(bpp);

    /*
     * Straighten up a few structure variables
     */
    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
    bnp->bgp_in_updates = bnp->bgp_out_updates = 0;
    bnp->bgp_out_notupdates = 0;
    bnp->bgp_out_octets = 0;
    bnp->bgp_in_notupdates = 1;
    bnp->bgp_in_octets = (u_long)length;

    /*
     * Done that.  Now do the right thing with regard to the state.
     */
    if (bnp->bgp_state != BGPSTATE_ACTIVE) {
	if (bnp->bgp_state != BGPSTATE_IDLE) {
	    bgp_event(bnp, BGPEVENT_STOP, BGPSTATE_IDLE);
	}
	/* XXX Violate the FSM.  Make my day. */
	bgp_event(bnp, BGPEVENT_START, BGPSTATE_ACTIVE);
    }

    /*
     * Finally, set the traffic timer to time out OPENCONFIRM state
     */
    bgp_init_traffic_timer(bnp);

    /*
     * Ha! Ah gone!
     */
}


/*
 * bgp_peer_start - start up a peer from idle state
 */
static void
bgp_peer_start(bnp, start_interval)
bgpPeer *bnp;
time_t start_interval;
{
    int connect_now;

    /*
     * This doesn't actually do much.  If the peer is passive just
     * transition to ACTIVE state.  Otherwise we need to set the
     * connect timer to start us up.  If the start interval is
     * zero do an immediate connect
     */
    if (BIT_TEST(bnp->bgp_options, BGPO_PASSIVE) || start_interval != 0) {
	/* XXX Violate stinking FSM */
	bgp_event(bnp, BGPEVENT_START, BGPSTATE_ACTIVE);
	if (BIT_TEST(bnp->bgp_options, BGPO_PASSIVE)) {
	    return;
	}
	connect_now = 0;
    } else {
	bgp_event(bnp, BGPEVENT_START, BGPSTATE_CONNECT);
	connect_now = 1;
	start_interval = BGPCONN_INTERVAL(bnp);
    }

    bgp_set_connect_timer(bnp, start_interval);
    if (connect_now) {
	bgp_connect_start(bnp);
    }
}



/*
 * bgp_peer_close - shut down a connection to IDLE from whatever state it
 *   is in, then start it up again as appropriate.
 *
 * This used to be bgp_neighbour_close(), but was renamed in recognition
 * of the odd spelling some people use.  Should purge the odd spelling
 * from the configuration file, actually.
 */
void
bgp_peer_close(bnp, event)
bgpPeer *bnp;
int event;
{
    task *tp = bnp->bgp_task;
    bgpPeerGroup *bgp = bnp->bgp_group;
    int needs_restart;
    time_t interval;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_peer_close: closing peer %s, state is %d (%s)",
	      bnp->bgp_name,
	      bnp->bgp_state,
	      trace_state(bgp_state_bits, bnp->bgp_state));
    }

    /*
     * If the peer has spooled write data, flush what we can and dump the
     * rest.
     */
    if (bnp->bgp_spool != NULL) {
	if (!BIT_TEST(bnp->bgp_flags, BGPF_WRITEFAILED)) {
	    (void) bgp_spool_flush(bnp);
	    if (bnp->bgp_spool != NULL) {
		bgp_spool_remove(bnp);
	    }
	} else {
	    bgp_spool_remove(bnp);
	}
    }

    /*
     * If this guy failed while writing, clean up his state so we
     * don't get confused next time.
     */
    BIT_RESET(bnp->bgp_flags, BGPF_WRITEFAILED);

    /*
     * If it is in ESTABLISHED there may be routes in the table from it.
     * Delete them now, then free the routing bit.  Drop us out of
     * established *before* doing this so we don't try to flash this
     * peer.  Otherwise close the socket if it is open and drop us to IDLE.
     */
    if (bnp->bgp_state == BGPSTATE_ESTABLISHED) {
        bgp_event(bnp, event, BGPSTATE_IDLE);
	bgp_n_established--;
	bgp->bgpg_n_established--;
	bgp_rt_terminate(bnp);
	task_close(tp);
    } else if (bnp->bgp_state != BGPSTATE_IDLE) {
	if (bnp->bgp_state != BGPSTATE_ACTIVE) {
	    task_close(tp);
	}
        bgp_event(bnp, event, BGPSTATE_IDLE);
    }

    /*
     * Decide if this is one we will immediately restart.  Do this here
     * so we know whether to delete the connect timer or not.
     */
    if (BIT_TEST(bnp->bgp_flags, BGPF_IDLED|BGPF_UNCONFIGURED|BGPF_DELETE)
      || BIT_TEST(bgp->bgpg_flags, BGPGF_IDLED|BGPGF_DELETE)) {
	needs_restart = 0;
    } else {
	needs_restart = 1;
    }

    /*
     * Delete the traffic timer if there.  Delete the connect timer unless
     * it may be needed.
     */
    bgp_delete_traffic_timer(bnp);
    if (BIT_TEST(bnp->bgp_flags, BGPF_CONNECT)) {
	if (!needs_restart || BIT_TEST(bnp->bgp_options, BGPO_PASSIVE)) {
	    bgp_delete_connect_timer(bnp);
	} else {
	    bgp_reset_connect_timer(bnp);
	}
    }

    /*
     * If we have a buffer, drop it now.
     */
    if (bnp->bgp_buffer != NULL) {
	bgp_buffer_free(&(bnp->bgp_inbuf));
    }

    /*
     * Delete local address and zero the port numbers in the peer's address.
     * Forget any interface pointer we don't need.
     */
    if (bnp->bgp_myaddr != NULL) {
	sockfree(bnp->bgp_myaddr);
	bnp->bgp_myaddr = NULL;
    }
    sock2port(bnp->bgp_addr) = 0;
    if (tp->task_addr != NULL) {
        sock2port(tp->task_addr) = 0;
    }
    if (bnp->bgp_ifap != NULL && !BGP_NEEDS_SHAREDIF(bnp->bgp_group)) {
	(void) ifa_free(bnp->bgp_ifap);
	bnp->bgp_ifap = NULL;
    }

    /*
     * If the peer is unconfigured or deleted, dump the bastard.
     */
    if (!needs_restart) {
        if (BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED|BGPF_DELETE)) {
            task_delete(tp);
            bnp->bgp_task = NULL;
	    bgp_peer_remove(bnp);
	    if (bnp->bgp_gateway != NULL) {
		sockfree(bnp->bgp_gateway);
		bnp->bgp_gateway = NULL;
	    }
	    if (bnp->bgp_myaddr != NULL) {
		sockfree(bnp->bgp_myaddr);
		bnp->bgp_myaddr = NULL;
	    }
	    if (bnp->bgp_lcladdr) {
		ifae_free(bnp->bgp_lcladdr);
		bnp->bgp_lcladdr = NULL;
	    }
	    if (bnp->bgp_addr != NULL) {
		sockfree(bnp->bgp_addr);
		bnp->bgp_addr = NULL;
	    }
	    if (bnp->bgp_ifap != NULL) {
		(void) ifa_free(bnp->bgp_ifap);
		bnp->bgp_ifap = NULL;
	    }
	    bgp_peer_free(bnp);
	}
	return;
    }

    /*
     * If it is passive, or if the peer is busy with version
     * negotiation, restart immediately.  Otherwise, restart
     * as appropriate for the number of connection failures
     * it has had.
     */
    if (BIT_TEST(bnp->bgp_options, BGPO_PASSIVE)
      || bnp->bgp_hisversion != BGP_VERSION_UNSPEC) {
	interval = 0;
	bnp->bgp_connect_failed = 0;
    } else {
	interval = BGPCONN_INTERVAL(bnp);
    }
    bgp_peer_start(bnp, interval);
}


/*
 * bgp_peer_established - transition a peer to established state
 */
void
bgp_peer_established(bnp)
bgpPeer *bnp;
{
    task *tp = bnp->bgp_task;
    bgpPeerGroup *bgp = bnp->bgp_group;

    /* State must be OpenConfirm */
    assert(bnp->bgp_state == BGPSTATE_OPENCONFIRM);

    /*
     * The *only* way we get to ESTABLISHED from OPENCONFIRM is
     * if we receive a keepalive.  Tell the world.
     */
    bgp_event(bnp, BGPEVENT_RECVKEEPALIVE, BGPSTATE_ESTABLISHED);

    /*
     * Change the receiver to bgp_recv_update.
     */
    bgp_recv_change(bnp, bgp_recv_update, "bgp_recv_update");

    /*
     * Mark this guy established
     */
    bgp->bgpg_n_established++;
    bgp_n_established++;

    /*
     * Clear the version negotiation info and the connect errors counter.
     */
    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
    bnp->bgp_connect_failed = 0;

    /*
     * Reset his traffic timer now.
     */
    bgp_init_traffic_timer(bnp);

    /*
     * Send the initial blast of routes.  I really wish we could try
     * reading the socket first, since this might prevent a lot of
     * unnecessary sending, but everything assumes that all ESTABLISHED
     * peers are synchronized which they aren't until after the initial
     * send.  Oh, well.
     */
    bgp_rt_send_init(bnp);

    /*
     * For good measure, do a read.  This whole process kind of leaves
     * the rest of gated locked out for a substantial amount of time, but
     * I dare not go ahead without doing the read now if there are
     * buffered messages.  Note that an error in bgp_rt_send_init() would
     * have taken this out of ESTABLISHED
     */
    if (bnp->bgp_state == BGPSTATE_ESTABLISHED
      && BGPBUF_SPACE(bnp) >= BGP_HEADER_LEN) {
        bgp_recv_update(tp);
    }
}


/*
 * bgp_ifachange - deal with changes in the interface configuration
 */
static void
bgp_ifachange __PF2(tp, task *,
		    ifap, if_addr *)
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp, *bnpnext;
    int check_up = 0;		/* interface may have come up */
    int check_down = 0;		/* interface may have gone down */
    int affected = 0;		/* count of affected peers */

    /*
     * This is what BGP peers care about:
     *
     * Peers in EXTERNAL and INTERNAL groups want a shared interface
     * with their neighbours, and care about this.  If the shared
     * interface goes down the peers which use it should be idled.
     * If the local address on the shared interface changes, peers which
     * use it may need to be taken down and restarted.  If we get a
     * new interface which is "better" than the current one (i.e. a
     * P2P interface with an address we were formerly assuming to be
     * on a multiaccess network) we need to restart the peer since
     * next hops might have changed.
     *
     * Peers in TEST groups care about shared interfaces as the above
     * when one exists, but will run without a shared interface when
     * none exists.  The effects the next hops which are advertised.
     * We only need idle a TEST peer if the local address is configured
     * and that address doesn't exist, but may want to take it down
     * and bring it back up if a shared interface comes up or goes
     * down.
     *
     * Peers in IGP groups couldn't care less about the shared interface,
     * and just want to make sure they are using a valid local address.
     * If all interfaces with that local address go away we may need to
     * idle the peer (if the local address was configured) or restart
     * it (if it acquired its local address by defaulting the connect()).
     *
     * Jeff told me that I should only count local addresses as "up" if
     * the local address is on an interface which is "up".  This is more
     * severe than necessary, but blame him.
     */

    /*
     * We only care if the interface went up or down, or if it changed
     * while it was up.  Ignore everything else.
     */
    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	if (BIT_TEST(ifap->ifa_change,
	  IFC_ADD|IFC_ADDR|IFC_NETMASK|IFC_UPDOWN)) {
	    check_up = 1;
	}
	if (BIT_TEST(ifap->ifa_change, IFC_DELETE|IFC_ADDR|IFC_NETMASK)) {
	    check_down = 1;
	}
    } else if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
#ifdef	notdef
    } else if (BIT_TEST(ifap->ifa_change, IFC_DELETE|IFC_UPDOWN)) {
#endif	/* notdef */
	check_down = 1;
    }
    
    if (!check_up && !check_down) {
	TRACE_PROTO(TR_BGP) {
	    trace(TR_BGP, 0,
	      "bgp_ifachange: interface %A(%s) changes not interesting to us",
	      ifap->ifa_addr,
	      ifap->ifa_name);
	}
	return;
    }

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_ifachange: interface %A(%s) %s, checking who this bothers",
	      ifap->ifa_addr,
	      ifap->ifa_name,
	      (check_up ? (check_down ? "changed" : "came up") : "went down"));
    }

    /*
     * Scan through the groups.  For those groups which need to be
     * checked, scan through the peers.
     */
    BGP_GROUP_LIST(bgp) {
	for (bnp = bgp->bgpg_peers; bnp != NULL; bnp = bnpnext) {
	    int stopit = 0;
	    bnpnext = bnp->bgp_next;

	    if (!BGP_NEEDS_SHAREDIF(bgp)) {
		if (check_up && BIT_TEST(bnp->bgp_flags, BGPF_IDLED)) {
		    /*
		     * Check to see if an appropriate interface for him
		     * has arrived.  The only way he could get in this
		     * state is if a local address was specified.
		     */
		    assert(bnp->bgp_lcladdr != NULL);
		    if (IFAE_ADDR_EXISTS(bnp->bgp_lcladdr)) {
			BIT_RESET(bnp->bgp_flags, BGPF_IDLED);
			stopit = 1;
			TRACE_PROTO(TR_BGP) {
			    trace(TR_BGP, 0,
		"bgp_ifachange: peer %s started, local address %A now exists",
			      bnp->bgp_name,
			      bnp->bgp_lcladdr->ifae_addr);
			}
		    }
		} else if (check_down
		  && !BIT_TEST(bnp->bgp_flags, BGPF_IDLED)) {

		    if (bnp->bgp_lcladdr != NULL) {
			if (!IFAE_ADDR_EXISTS(bnp->bgp_lcladdr)) {
			    if (!BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)) {
				BIT_SET(bnp->bgp_flags, BGPF_IDLED);
			    }
			    TRACE_PROTO(TR_BGP) {
				trace(TR_BGP, 0,
		"bgp_ifachange: peer %s %s, local address %A no longer exists",
				  bnp->bgp_name,
				  (BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)
				    ? "terminated" : "idled"),
				  bnp->bgp_lcladdr->ifae_addr);
			    }
			    stopit = 1;
			}
		    } else if (bnp->bgp_myaddr != NULL) {
			if_addr_entry *ifae;

			if ((ifae = ifae_locate(bnp->bgp_myaddr,
			    &if_local_list)) == NULL
			  || !IFAE_ADDR_EXISTS(ifae)) {
			    TRACE_PROTO(TR_BGP) {
				trace(TR_BGP, 0,
		"bgp_ifachange: peer %s %s, local address %A no longer exists",
				  bnp->bgp_name,
				  (BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)
				    ? "terminated" : "restarted"),
				  bnp->bgp_myaddr);
			    }
			    stopit = 1;
			}
		    }
		}
	    }

	    if (!stopit && BGP_USES_SHAREDIF(bgp)) {
		if_addr *new_ifap;

		/*
		 * Find the current view of the shared interface
		 */
		if (BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
		    new_ifap = if_withdst(bnp->bgp_gateway);
		} else {
		    new_ifap = if_withdst(bnp->bgp_addr);
		}
		if (new_ifap != NULL && bnp->bgp_lcladdr != NULL
		  && new_ifap->ifa_addrent_local != bnp->bgp_lcladdr) {
		    new_ifap = NULL;
		}
		if (new_ifap != bnp->bgp_ifap
		 || (new_ifap != NULL && bnp->bgp_myaddr != NULL
		   && !sockaddrcmp_in(new_ifap->ifa_addrent_local->ifae_addr,
				     bnp->bgp_myaddr))) {
		    if_addr *old_ifap = bnp->bgp_ifap;

		    /*
		     * Something is different.  If we have a new interface
		     * we'll need to shut down this peer (if running) and
		     * (re)start him.
		     */
		    if (BGP_NEEDS_SHAREDIF(bgp)) {
			if (new_ifap != NULL) {
			    (void) ifa_alloc(new_ifap);
			}
			if (bnp->bgp_ifap != NULL) {
			    (void) ifa_free(bnp->bgp_ifap);
			}
			bnp->bgp_ifap = new_ifap;
			if (new_ifap == NULL) {
			    if (!BIT_TEST(bnp->bgp_flags, BGPF_UNCONFIGURED)) {
				BIT_SET(bnp->bgp_flags, BGPF_IDLED);
			    }
			} else {
			    BIT_RESET(bnp->bgp_flags, BGPF_IDLED);
			}
		    }
		    TRACE_PROTO(TR_BGP) {
			const char *action = "restarted";
			const char *change;

			if (new_ifap == NULL) {
			    change = "went down";
			    if (BIT_TEST(bnp->bgp_flags, BGPF_IDLED)) {
				action = "idled";
			    } else {
				action = "terminated";
			    }
			} else if (old_ifap != NULL) {
			    change = "changed";
			} else {
			    change = "came up";
			    if (!BGP_OPTIONAL_SHAREDIF(bgp)) {
				action = "started";
			    }
			}
			trace(TR_BGP, 0,
			  "bgp_ifachange: peer %s %s, shared interface %s",
			  bnp->bgp_name,
			  action,
			  change);
		    }
		    stopit = 1;
		}
	    }

	    /*
	     * Stop and/or start peer if his status changed and if
	     * his group is not idled.
	     */
	    if (stopit) {
		affected++;
		if (BIT_TEST(bgp->bgpg_flags, BGPGF_IDLED)) {
		    TRACE_PROTO(TR_BGP) {
			trace(TR_BGP, 0,
			  "bgp_ifachange: peer %s not started, group is idle",
			  bnp->bgp_name);
		    }
		} else {
		    if (bnp->bgp_state == BGPSTATE_OPENSENT
		      || bnp->bgp_state == BGPSTATE_OPENCONFIRM
		      || bnp->bgp_state == BGPSTATE_ESTABLISHED) {
			bgp_send_notify_none(bnp, BGP_CEASE, 0);
		    }
		    bnp->bgp_connect_failed = 0;
		    bnp->bgp_hisversion = BGP_VERSION_UNSPEC;
		    if (bnp->bgp_state != BGPSTATE_IDLE) {
			/*
			 * bgp_peer_close will do the right thing.
			 *
			 * XXX should we ask for an immediate restart?
			 */
			bgp_peer_close(bnp, BGPEVENT_STOP);
		    } else {
			bgp_peer_start(bnp, (time_t) BGPCONN_IFUP);
		    }
	        }
	    }
	}
    } BGP_GROUP_LIST_END(bgp);

    if (!affected) {
	TRACE_PROTO(TR_BGP) {
	    trace(TR_BGP, 0,
	      "bgp_ifachange: no BGP peers found affected by interface change");
	}
    }
}


/*
 * bgp_fake_terminate - terminate routine which doesn't do anything.
 */
static void
bgp_fake_terminate(tp)
task *tp;
{
    /* nothing */
}


/*
 * bgp_terminate - terminate BGP cleanly and in sequence.
 */
static void
bgp_terminate(tp)
task *tp;
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;
    int delete_task;

    /*
     * Kill all the protopeers.  Don't send them anything, it would be
     * an FSM violation anyway.
     */
    bgp_pp_delete_all();

    /*
     * Go through each group IDLEing all peers and deleting their
     * tasks.  Then delete the group's task.  Finally, delete the
     * listen task.  Since terminating a peer may cause it to
     * be reordered in the list, we search the list from the beginning
     * multiple times.
     */
    BGP_GROUP_LIST(bgp) {
	do {
	    BGP_PEER_LIST(bgp, bnp) {
		if (bnp->bgp_task == NULL) {
		    continue;
		}
		delete_task = 1;
		if (bnp->bgp_state != BGPSTATE_IDLE) {
		    /*
		     * If it is far enough along to have sent an OPEN message,
		     * send a CEASE.
		     */
		    if (bnp->bgp_state == BGPSTATE_OPENSENT
		      || bnp->bgp_state == BGPSTATE_OPENCONFIRM
		      || bnp->bgp_state == BGPSTATE_ESTABLISHED) {
			bgp_send_notify_none(bnp, BGP_CEASE, 0);
		    }

		    /*
		     * Determine whether we need to delete the
		     * task after the peer is closed.  If not,
		     * idle the peer so it won't be auto-started.
		     */
		    if (BIT_TEST(bnp->bgp_flags,
		      BGPF_UNCONFIGURED|BGPF_DELETE)) {
			delete_task = 0;
		    } else {
			BIT_SET(bnp->bgp_flags, BGPF_IDLED);
		    }
		    bgp_peer_close(bnp, BGPEVENT_STOP);
		}

		/*
		 * Now delete its task if someone else hasn't.
		 */
		if (delete_task) {
		    task_delete(bnp->bgp_task);
		    bnp->bgp_task = NULL;
		}

		/*
		 * Break out of loop so we start again from the beginning.
		 */
		break;
	    } BGP_PEER_LIST_END(bgp, bnp);
	} while (bnp != NULL);

	/*
	 * Now delete the group's task
	 */
	task_delete(bgp->bgpg_task);
	bgp->bgpg_task = NULL;
    } BGP_GROUP_LIST_END(bgp);

    /*
     * Finally, stop listening.
     */
    bgp_listen_stop();
}


/*
 * bgp_group_init - initialize a BGP group into shape
 */
PROTOTYPE(bgp_group_init,
	  static void,
	 (bgpPeerGroup *));
static void
bgp_group_init(bgp)
bgpPeerGroup *bgp;
{
    /*
     * All that is required of us here is to make the group's
     * name and associated task.  Except for an IGP group, where
     * we need to register it, and don't bother with the task.
     */
    bgp_make_group_names(bgp);
    bgp_group_task_create(bgp);

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0, "bgp_group_init: initializing %s", bgp->bgpg_name);
    }

    if (bgp->bgpg_type == BGPG_INTERNAL_IGP) {
	BIT_SET(bgp->bgpg_flags, BGPGF_IDLED);
	task_aux_register(bgp->bgpg_task,
			  RTPROTO_OSPF_ASE,
			  bgp_aux_initiate,
			  bgp_aux_terminate,
			  bgp_aux_flash,
			  bgp_aux_newpolicy);
    }
}


/*
 * bgp_peer_init - initialize a BGP peer into running state
 */
PROTOTYPE(bgp_peer_init,
	  static void,
	 (bgpPeer *));
static void
bgp_peer_init(bnp)
bgpPeer *bnp;
{
    /*
     * The peer given to us here is configured, and in its group's
     * peer list, but is barely out of configuration and needs a
     * task and other junk to go along with this.  Do this now.
     */
    
    /*
     * Copy some stuff into the gw entry from other places.
     */
    bnp->bgp_local_as = bnp->bgp_conf_local_as;
    bnp->bgp_peer_as = bnp->bgp_group->bgpg_peer_as;

    /*
     * Make a name for this peer, and create a task.
     */
    bgp_make_names(bnp);
    bgp_task_create(bnp);

    /*
     * Determine if this peer requires a shared interface and, if
     * so, whether we have one.  If not, emit a warning and idle
     * the peer.  If it doesn't use a shared interface make sure the
     * local address is valid.  If not, idle the peer.
     */
    if (BGP_NEEDS_SHAREDIF(bnp->bgp_group)) {
	if (bgp_set_peer_if(bnp, 1) == NULL) {
	    BIT_SET(bnp->bgp_flags, BGPF_IDLED);
	}
    } else if (bnp->bgp_lcladdr != NULL) {
	if (!IFAE_ADDR_EXISTS(bnp->bgp_lcladdr)) {
	    trace(TR_BGP, LOG_WARNING,
  "bgp_peer_init: BGP peer %s local address %A not found.  Leaving peer idled",
	      bnp->bgp_name,
	      bnp->bgp_lcladdr->ifae_addr);
	    BIT_SET(bnp->bgp_flags, BGPF_IDLED);
	}
    }

    /*
     * Put the peer in idle state.  If the peer isn't idled then start it up.
     */
    bnp->bgp_state = BGPSTATE_IDLE;
    if (!BIT_TEST(bnp->bgp_group->bgpg_flags, BGPGF_IDLED)
      && !BIT_TEST(bnp->bgp_flags, BGPF_IDLED)) {
	bgp_peer_start(bnp, BGPCONN_INIT);
    }
}


/*
 * bgp_peer_delete - delete a peer entirely.  The peer may be running.
 */
PROTOTYPE(bgp_peer_delete,
	  static void,
	 (bgpPeer *));
static void
bgp_peer_delete(bnp)
bgpPeer *bnp;
{
    if (bnp->bgp_task != NULL) {
	/*
	 * If the peer has a task, bgp_peer_close() can do the work.
	 * Mark him deleted.
	 */
	if (bnp->bgp_state == BGPSTATE_OPENSENT
	  || bnp->bgp_state == BGPSTATE_OPENCONFIRM
	  || bnp->bgp_state == BGPSTATE_ESTABLISHED) {
	    bgp_send_notify_none(bnp, BGP_CEASE, 0);
	}
        BIT_SET(bnp->bgp_flags, BGPF_DELETE);
        bgp_peer_close(bnp, BGPEVENT_STOP);
    } else {
	/*
	 * The peer has no task.  This can occur when some dork uses
	 * a bgp no in the configuration file, followed by some
	 * BGP configuration.  Dump him.
	 */
	bgp_peer_remove(bnp);
	if (bnp->bgp_gateway != NULL) {
	    sockfree(bnp->bgp_gateway);
	    bnp->bgp_gateway = NULL;
	}
	if (bnp->bgp_myaddr != NULL) {
	    sockfree(bnp->bgp_myaddr);
	    bnp->bgp_myaddr = NULL;
	}
	if (bnp->bgp_lcladdr) {
	    ifae_free(bnp->bgp_lcladdr);
	    bnp->bgp_lcladdr = NULL;
	}
	if (bnp->bgp_addr != NULL) {
	    sockfree(bnp->bgp_addr);
	    bnp->bgp_addr = NULL;
	}
	bgp_peer_free(bnp);
    }
}


/*
 * bgp_group_delete - delete a whole group.  We call bgp_peer_delete()
 *   repeatedly to get rid of the peers.
 */
PROTOTYPE(bgp_group_delete,
	  static void,
	 (bgpPeerGroup *));
static void
bgp_group_delete(bgp)
bgpPeerGroup *bgp;
{
    while (bgp->bgpg_peers != NULL) {
	bgp_peer_delete(bgp->bgpg_peers);
    }

    if (bgp->bgpg_task != NULL) {
        task_delete(bgp->bgpg_task);
	bgp->bgpg_task = NULL;
    }

    /*
     * XXX Free adv_entry
     */
    if (bgp->bgpg_allow != NULL) {
	adv_free_list(bgp->bgpg_allow);
	bgp->bgpg_allow = NULL;
    }

    /*
     * Free up the gateway address if there is one
     */
    if (bgp->bgpg_gateway != NULL) {
	sockfree(bgp->bgpg_gateway);
	bgp->bgpg_gateway = NULL;
    }

    /*
     * Remove the structure from the linked list and free it.
     */
    bgp_group_remove(bgp);
    bgp_group_free(bgp);
}



/*
 *	(Re)Configuration related routines
 */

/*
 * bgp_var_init - initialize default globals
 */
void
bgp_var_init()
{
    doing_bgp = FALSE;
    bgp_default_trace_flags = trace_flags;
    bgp_default_metric = BGP_METRIC_NONE;
    bgp_default_preference = RTPREF_BGP_EXT;
}

/*
 * bgp_cleanup - this is called before the configuration file is to
 *   be reread.  Loop through marking all our groups and peers deleted,
 *   then reset our globals.
 */
/*ARGSUSED*/
static void
bgp_cleanup(tp)
task *tp;
{
    bgpPeerGroup *bgp;

    BGP_GROUP_LIST(bgp) {
	bgpPeer *bnp;

	BIT_SET(bgp->bgpg_flags, BGPGF_DELETE);

	BGP_PEER_LIST(bgp, bnp) {
	    BIT_SET(bnp->bgp_flags, BGPF_DELETE);
	    bnp->bgp_import = bnp->bgp_export = (adv_entry *) 0;
	} BGP_PEER_LIST_END(bgp, bnp);

    } BGP_GROUP_LIST_END(bgp);

    adv_free_list(bgp_import_list);
    bgp_import_list = (adv_entry *) 0;
    adv_free_list(bgp_import_aspath_list);
    bgp_import_aspath_list = (adv_entry *) 0;
    adv_free_list(bgp_export_list);
    bgp_export_list = (adv_entry *) 0;
    bgp_var_init();
}

/*
 * bgp_reinit - this is called after the configuration file has been
 *   reread, and after bgp_init() has been called to start everyone
 *   and clean up unwanted peers.  What we are supposed to do here
 *   is reevaluate each peer's import policy.  For each established
 *   external and internal (not igp) peer, call bgp_rt_reinit.
 */
/*ARGSUSED*/
static void
bgp_reinit(tp)
task *tp;
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;

    BGP_GROUP_LIST(bgp) {
	if (bgp->bgpg_n_established == 0
	  || (bgp->bgpg_type != BGPG_EXTERNAL
	    && bgp->bgpg_type != BGPG_INTERNAL)) {
	    continue;		/* goto in diguise */
	}
	BGP_PEER_LIST(bgp, bnp) {
	    if (bnp->bgp_state == BGPSTATE_ESTABLISHED) {
		bgp_rt_reinit(bnp);
	    }
	} BGP_PEER_LIST_END(bgp, bnp);
    } BGP_GROUP_LIST_END(bgp);
}


/*
 * bgp_conf_check - final check of configuration
 */
int
bgp_conf_check(parse_error)
char *parse_error;
{
    if (!inet_autonomous_system) {
	(void) sprintf(parse_error, "autonomous-system not specified");
	return FALSE;
    }
    if (!bgp_n_groups) {
	(void) sprintf(parse_error, "no BGP groups specified");
	return FALSE;
    }

    return TRUE;
}


/*
 * bgp_conf_group_alloc - fetch a group structure for the parser
 */
bgpPeerGroup *
bgp_conf_group_alloc()
{
    bgpPeerGroup *bgp;

    /*
     * Get a structure and zero it
     */
    bgp = bgp_group_alloc();
    bzero((caddr_t)bgp, sizeof(bgpPeerGroup));

    /*
     * Set up a few of the options to reasonable defaults
     */
    bgp->bgpg_trace_flags = bgp_default_trace_flags;
    bgp->bgpg_holdtime_out = BGP_HOLDTIME;
    bgp->bgpg_metric_out = bgp_default_metric;
    bgp->bgpg_local_as = inet_autonomous_system;
    bgp->bgpg_preference = bgp_default_preference;

    return (bgp);
}

/*
 * bgp_conf_peer_alloc - fetch a peer structure for the parser
 */
bgpPeer *
bgp_conf_peer_alloc(bgp)
bgpPeerGroup *bgp;
{
    return bgp_peer_create(bgp);
}


/*
 * bgp_conf_group_add - add a group to the list if there isn't something
 *   wrong with it.
 */
bgpPeerGroup *
bgp_conf_group_add(bgp, errstr)
bgpPeerGroup *bgp;
char *errstr;
{
    bgpPeerGroup *obgp = NULL;

    /*
     * See if the group is sensible at all
     */
    if (bgp->bgpg_type == BGPG_INTERNAL
      || bgp->bgpg_type == BGPG_INTERNAL_IGP) {
	if (!BIT_TEST(bgp->bgpg_options, BGPO_LOCAL_AS)) {
	    bgp->bgpg_local_as = bgp->bgpg_peer_as;
	} else if (bgp->bgpg_local_as != bgp->bgpg_peer_as) {
	    (void) sprintf(errstr,
  "the BGP local AS (%u) and peer AS (%u) must be the same for internal peers",
	      (u_int)bgp->bgpg_local_as,
	      (u_int)bgp->bgpg_peer_as);
	    return (bgpPeerGroup *)NULL;
	}

	if (bgp->bgpg_type == BGPG_INTERNAL_IGP
	  && BIT_TEST(bgp->bgpg_options, BGPGO_VERSION)
	  && bgp->bgpg_conf_version <= BGP_VERSION_2) {
	    (void) sprintf(errstr,
  "internal BGP peers associated with an IGP must run at version 3 or better");
	    return (bgpPeerGroup *)NULL;
	}
    } else if (bgp->bgpg_type == BGPG_EXTERNAL) {
	if (bgp->bgpg_local_as == bgp->bgpg_peer_as) {
	    (void) sprintf(errstr,
	      "external peer must not have the same AS as we do locally (%u)",
	      (u_int)bgp->bgpg_local_as);
	    return (bgpPeerGroup *)NULL;
	}
    }

    if (bgp->bgpg_holdtime_out < BGP_MIN_HOLDTIME) {
	(void) sprintf(errstr,
"the holdtime for BGP group peers (%d) is less than the minimum permitted (%d)",
	  bgp->bgpg_holdtime_out,
	  BGP_MIN_HOLDTIME);
	return (bgpPeerGroup *)NULL;
    }

    /*
     * Find out if the group is there already
     * XXX We may want to distinguish by more of this stuff.
     */
    if (bgp_groups != NULL) {
	BGP_GROUP_LIST(obgp) {
	    if (obgp->bgpg_local_as == bgp->bgpg_local_as
	      && obgp->bgpg_peer_as == bgp->bgpg_peer_as
	      && obgp->bgpg_type == bgp->bgpg_type) {
		break;
	    }
	} BGP_GROUP_LIST_END(obgp);
    }

    if (obgp == NULL) {
	/*
	 * This guy is unique, return him.
	 */
	bgp_group_add(bgp);
	return (bgp);
    } else if (!BIT_TEST(obgp->bgpg_flags, BGPGF_DELETE)) {
	(void) sprintf(errstr,
  "duplicate BGP group found, groups must differ by type and/or AS");
	return (bgpPeerGroup *)NULL;
    }

    /*
     * Tricky here.  What we do is transfer the options from
     * the new guy to the old guy and delete the new guy.  We'll
     * deal with option changes when we deal with peers.
     */
    if (obgp->bgpg_gateway != NULL) {
	sockfree(obgp->bgpg_gateway);
    }
    obgp->bgpg_conf = bgp->bgpg_conf;		/* Struct copy */
    bgp->bgpg_gateway = NULL;
    if (obgp->bgpg_allow != NULL) {
	adv_free_list(obgp->bgpg_allow);
    }
    obgp->bgpg_allow = bgp->bgpg_allow;
    bgp->bgpg_allow = NULL;

    /*
     * That should be everything.  Unlink the old group and link it
     * to the end.  Then free the new group.
     */
    BIT_RESET(obgp->bgpg_flags, BGPGF_DELETE);
    bgp_group_remove(obgp);
    bgp_group_add(obgp);
    bgp_group_free(bgp);
    bgp_make_group_names(obgp);
    return (obgp);
}


/*
 * bgp_conf_group_check - see if this group is okay
 */
int
bgp_conf_group_check(bgp, errstr)
bgpPeerGroup *bgp;
char *errstr;
{
    if (bgp->bgpg_n_peers == 0 && bgp->bgpg_allow == NULL) {
	(void) sprintf(errstr,
    "BGP group has neither configured peers or an allow list: invalid configuration");
	return FALSE;
    }
    return TRUE;
}


/*
 * bgp_conf_peer_add - add a peer to a group
 */
bgpPeer *
bgp_conf_peer_add(bgp, bnp, errstr)
bgpPeerGroup *bgp;
bgpPeer *bnp;
char *errstr;
{
    bgpPeer *obnp;

    /*
     * Make sure this peer is vaguely sensible
     */
    if (bnp->bgp_conf_local_as != bgp->bgpg_local_as) {
	(void) sprintf(errstr,
		       "the local AS number (%u) used for BGP peer %A must be that of the group (%u)",
		       (u_int) bnp->bgp_conf_local_as,
		       bnp->bgp_addr,
		       (u_int) bgp->bgpg_local_as);
	return (bgpPeer *)NULL;
    }

    if (bgp->bgpg_type == BGPG_INTERNAL_IGP
	&& BIT_TEST(bnp->bgp_options, BGPO_VERSION)
	&& bnp->bgp_conf_version <= BGP_VERSION_2) {
	(void) sprintf(errstr,
		       "internal BGP peer %A associated with an IGP must run at version 3 or better",
		       bnp->bgp_addr);
	return (bgpPeer *)NULL;
    }

    if (bnp->bgp_holdtime_out < BGP_MIN_HOLDTIME) {
	(void) sprintf(errstr,
		       "the holdtime for BGP peer %A (%d) is less than the minimum permitted (%d)",
		       bnp->bgp_addr,
		       bnp->bgp_holdtime_out,
		       BGP_MIN_HOLDTIME);
	return (bgpPeer *) 0;
    }

    if (bgp->bgpg_type == BGPG_INTERNAL) {
	if (bnp->bgp_metric_out != bgp->bgpg_metric_out) {
	    (void) sprintf(errstr,
			   "the metricout option should be used in the internal group declaration only");
	    return (bgpPeer *) 0;
	}
	if (BIT_TEST(bnp->bgp_options, BGPO_DEFAULTIN)
	    != BIT_TEST(bgp->bgpg_options, BGPO_DEFAULTIN)) {
	    (void) sprintf(errstr,
			   "the importdefault flag should be used in the internal group declaration only");
	    return (bgpPeer *) 0;
	}
	if (BIT_TEST(bnp->bgp_options, BGPO_DEFAULTOUT)
	    != BIT_TEST(bgp->bgpg_options, BGPO_DEFAULTOUT)) {
	    (void) sprintf(errstr,
			   "the exportdefault flag should be used in the internal group declaration only");
	    return (bgpPeer *) 0;
	}
	if (bnp->bgp_lcladdr != bgp->bgpg_lcladdr) {
	    (void) sprintf(errstr,
			   "the interface option should be used in the internal group declaration only");
	    return (bgpPeer *) 0;
	}
    }

    /*
     * Search through looking for a peer with the same address.  If
     * we find one, check him out.
     */
    if (bgp->bgpg_peers == NULL) {
	obnp = NULL;
    } else {
	BGP_PEER_LIST(bgp, obnp) {
	    if (sockaddrcmp_in(bnp->bgp_addr, obnp->bgp_addr)) {
		break;
	    }
	} BGP_PEER_LIST_END(bgp, obnp);
    }

    if (obnp == NULL) {
	/*
	 * Unique peer.  Add him to the group and forget about it.
	 */
	bgp_peer_add(bgp, bnp);
	return (bnp);
    } else if (!BIT_TEST(obnp->bgp_flags, BGPF_DELETE)) {
	(void) sprintf(errstr,
		       "duplicate entries for peer %A found in group type %s AS %u",
		       bnp->bgp_addr,
		       trace_state(bgp_group_bits, bgp->bgpg_type),
		       (u_int) bgp->bgpg_peer_as);
	return (bgpPeer *)NULL;
    }

    /*
     * Here we've got a deleted duplicate.  If there have been any
     * significant changes to it leave it deleted, otherwise copy
     * the insignificant changes over.
     */
    if (obnp->bgp_metric_out != bnp->bgp_metric_out
	|| obnp->bgp_conf_version != bnp->bgp_conf_version
	|| obnp->bgp_holdtime_out != bnp->bgp_holdtime_out
	|| obnp->bgp_conf_local_as != bnp->bgp_conf_local_as
	|| obnp->bgp_peer_as != bgp->bgpg_peer_as
	|| (BIT_TEST(bnp->bgp_options, BGPO_KEEPALL)
	    && !BIT_TEST(obnp->bgp_options, BGPO_KEEPALL))
	|| (obnp->bgp_gateway == NULL && bnp->bgp_gateway != NULL)
	|| (obnp->bgp_gateway != NULL && bnp->bgp_gateway == NULL)
	|| (obnp->bgp_gateway != NULL && !sockaddrcmp_in(bnp->bgp_gateway, obnp->bgp_gateway))) {
	bgp_peer_add(bgp, bnp);
	return (bnp);
    }

    /*
     * If the peer has a socket open, check his receive and send buffer
     * settings.  If they've changed we'll need to replace him.  If
     * he has more data spooled on his socket than permitted by the
     * new peer's configuration, replace him too.
     */
    if (obnp->bgp_state != BGPSTATE_IDLE && obnp->bgp_state !=BGPSTATE_ACTIVE) {
	if (BGP_RX_BUFSIZE(obnp->bgp_recv_bufsize)
	    != BGP_RX_BUFSIZE(bnp->bgp_recv_bufsize)
	  || BGP_RX_BUFSIZE(obnp->bgp_send_bufsize)
	    != BGP_RX_BUFSIZE(bnp->bgp_send_bufsize)
	  || (obnp->bgp_spool != NULL
	    && bnp->bgp_spool_bufsize < obnp->bgp_spool->bgp_sph_count)) {
	    bgp_peer_add(bgp, bnp);
	    return (bnp);
	}
    }

    /*
     * Check out the interface.  This only matters if the guy is
     * up and running, unless the fellow is in connect.  This code
     * is funky
     */
    if (bnp->bgp_ifap != NULL) {
	if (obnp->bgp_lcladdr != NULL) {
	    if (!sockaddrcmp_in(bnp->bgp_ifap->ifa_addr_local,
				obnp->bgp_lcladdr->ifae_addr)) {
		bgp_peer_add(bgp, bnp);
		return (bnp);
	    }
	} else if (obnp->bgp_state == BGPSTATE_CONNECT
		   && (obnp->bgp_ifap == NULL
		       || !sockaddrcmp_in(obnp->bgp_ifap->ifa_addr_local,
					  bnp->bgp_ifap->ifa_addr_local))) {
	    bgp_peer_add(bgp, bnp);
	    return (bnp);
	}
    }

    /*
     * Here we've got changes we can live with.  Copy them over
     * into the old peer and return him.
     */
    if (obnp->bgp_gateway != NULL) {
	sockfree(obnp->bgp_gateway);
    }
    if (obnp->bgp_lcladdr != NULL) {
	ifae_free(obnp->bgp_lcladdr);
    }
    obnp->bgp_conf = bnp->bgp_conf;		/* Struct copy */
    bnp->bgp_gateway = NULL;
    bnp->bgp_lcladdr = NULL;
    bgp_peer_free(bnp);
    BIT_RESET(obnp->bgp_flags, BGPF_UNCONFIGURED|BGPF_DELETE);
    bgp_peer_remove(obnp);
    bgp_peer_add(bgp, obnp);
    bgp_make_names(obnp);

    return (obnp);
}


/*
 * bgp_init - start up bgp
 */
void
bgp_init()
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;
    struct servent *sp;

    if (!doing_bgp) {
	/*
	 * Get rid of any groups which might be present
	 */
	while (bgp_groups != NULL) {
	    bgp_group_delete(bgp_groups);
	}

	/*
	 * If we have policy, free it.
	 */
 	if (bgp_import_list) {
 	    adv_free_list(bgp_import_list);
 	    bgp_import_list = (adv_entry *) 0;
 	}
 	if (bgp_import_aspath_list) {
 	    adv_free_list(bgp_import_aspath_list);
 	    bgp_import_aspath_list = (adv_entry *) 0;
 	}
 	if (bgp_export_list) {
 	    adv_free_list(bgp_export_list);
 	    bgp_export_list = (adv_entry *) 0;
 	}
 
	/*
	 * If BGP has been running before there will be a load of crap
	 * which needs to be taken down.  Do so now.
	 */
	if (bgp_listen_task != NULL) {
#ifdef	PROTO_SNMP
	    bgp_init_mib(FALSE);
#endif	/* PROTO_SNMP */
	    bgp_pp_delete_all();
	    bgp_listen_stop();

	    bgp_peer_free_all();	/* free malloc()'d peer structures */
	    bgp_group_free_all();	/* free malloc()'d group structures */
	}
    } else {
	/*
	 * Determine the well-known port to use if we don't know it
	 */
	if (bgp_port == 0) {
	    if ((sp = getservbyname("bgp", "tcp")) == NULL) {
		bgp_port = htons(BGP_PORT);
		trace(TR_ALL, LOG_WARNING,
	"bgp_init: getservbyname(\"bgp\", \"tcp\") failed, using port %d",
		  BGP_PORT);
	    } else {
		bgp_port = sp->s_port;
	    }
	}

	/*
	 * Initialize the listen task if we haven't got that, and make
	 * sure the send buffer is big enough.
	 */
	if (bgp_listen_task == NULL) {
	    bgp_listen_init();
#ifdef	PROTO_SNMP
	    bgp_init_mib(TRUE);
#endif	/* PROTO_SNMP */
	    task_alloc_send(bgp_listen_task, BGPMAXSENDPKTSIZE);
	} else {
	    bgpPeerGroup *bgpnext;
	    /*
	     * BGP was running before and this is just a change.  Go
	     * around deleting groups which are gone.  Careful about
	     * groups disappearing from under us.
	     */
	    for (bgp = bgp_groups; bgp != NULL; bgp = bgpnext) {
		bgpnext = bgp->bgpg_next;
		if (BIT_TEST(bgp->bgpg_flags, BGPGF_DELETE)) {
		    bgp_group_delete(bgp);
		}
	    }

	    /*
	     * Go through all the peers now, looking for deleted guys.
	     * Any one who is deleted is checked to see if they would
	     * qualify as an unconfigured peer.  If so they are marked
	     * that way, if not they are dumped.
	     */
	    BGP_GROUP_LIST(bgp) {
		bgpPeer *bnpnext;

		for (bnp = bgp->bgpg_peers; bnp != NULL; bnp = bnpnext) {
		    bnpnext = bnp->bgp_next;
		    if (BIT_TEST(bnp->bgp_flags, BGPF_DELETE)) {
	      		if ((bnp->bgp_state == BGPSTATE_OPENSENT
			    || bnp->bgp_state == BGPSTATE_OPENCONFIRM
			    || bnp->bgp_state == BGPSTATE_ESTABLISHED)
			  && bgp->bgpg_allow != NULL
			  && adv_destmask_match(bgp->bgpg_allow,
			    bnp->bgp_addr)) {
			    bgpPeer *bnpchk;
			    int terminate_him = 0;
			    /*
			     * We're still not in the clear here.  Check
			     * to see if there is already a configured
			     * peer in the list with the same address.
			     * If so, terminate this guy anyway.
			     */
			    for (bnpchk = bgp->bgpg_peers;
				 bnpchk != bnp;
				 bnpchk = bnpchk->bgp_next) {
				if (sockaddrcmp_in(bnpchk->bgp_addr,
						   bnp->bgp_addr)) {
				    terminate_him = 1;
				    break;
				}
			    }

			    /*
			     * And we're *still* not in the clear here.  As
			     * an unconfigured peer he would inherit options
			     * from the group.  See if his current options
			     * match.
			     */
			    if (!terminate_him &&
				(bnp->bgp_metric_out != bgp->bgpg_metric_out
			      || bnp->bgp_conf_version != bgp->bgpg_conf_version
			      || bnp->bgp_lcladdr != bgp->bgpg_lcladdr
			      || (bnp->bgp_conf_local_as != bgp->bgpg_local_as
				&& bnp->bgp_local_as != bgp->bgpg_local_as)
			      || bnp->bgp_holdtime_out
				!= bgp->bgpg_holdtime_out
			      || bnp->bgp_peer_as != bgp->bgpg_peer_as
			      || (!BIT_TEST(bnp->bgp_options, BGPO_KEEPALL)
				&& BIT_TEST(bgp->bgpg_options, BGPO_KEEPALL))
			      || (bnp->bgp_gateway == NULL
				&& bgp->bgpg_gateway != NULL)
			      || (bnp->bgp_gateway != NULL
				&& bgp->bgpg_gateway == NULL)
			      || (bnp->bgp_gateway != NULL
				&& !sockaddrcmp_in(bnp->bgp_gateway,
				  bgp->bgpg_gateway))
			      || BGP_RX_BUFSIZE(bnp->bgp_recv_bufsize)
				!= BGP_RX_BUFSIZE(bgp->bgpg_recv_bufsize)
			      || BGP_RX_BUFSIZE(bnp->bgp_send_bufsize)
				!= BGP_RX_BUFSIZE(bgp->bgpg_send_bufsize)
			      || (bnp->bgp_spool != NULL
				&& bgp->bgpg_spool_bufsize
				  < bnp->bgp_spool->bgp_sph_count))) {
				terminate_him = 1;
			    }

			    if (terminate_him) {
				bgp_peer_delete(bnp);
			    } else {
				bnp->bgp_spool_bufsize
				  = bgp->bgpg_spool_bufsize;
				BIT_RESET(bnp->bgp_flags, BGPF_DELETE);
				BIT_SET(bnp->bgp_flags, BGPF_UNCONFIGURED);
				bgp_peer_remove(bnp);
				bgp_peer_add(bgp, bnp);
    				bgp_make_names(bnp);
			    }
			} else {
			    bgp_peer_delete(bnp);
			}
		    }
		}
	    } BGP_GROUP_LIST_END(bgp);

	    /*
	     * Free all malloc'd peer and group structures so we start
	     * clean.
	     */
	    bgp_peer_free_all();
	    bgp_group_free_all();
	}

	/*
	 * Scan the group list looking for guys who have no tasks.
	 * Create them for anyone who needs them.
	 */
	BGP_GROUP_LIST(bgp) {
	    if (bgp->bgpg_task == NULL) {
		bgp_group_init(bgp);
	    }
	} BGP_GROUP_LIST_END(bgp);

	/*
	 * Now go around looking for peers with no tasks.  Initialize these
	 * guys also.  In addition, find the policy for everyone who needs
	 * it.
	 */
	BGP_GROUP_LIST(bgp) {
	    BGP_PEER_LIST(bgp, bnp) {
		if (bnp->bgp_task == NULL) {
		    bgp_peer_init(bnp);
		}
		/* XXX this policy stuff isn't quite right.  Think about it */
		switch (bnp->bgp_type) {
		case BGPG_EXTERNAL:
		case BGPG_INTERNAL:
		    bnp->bgp_import = control_exterior_locate(bgp_import_list, bnp->bgp_peer_as);
		    bnp->bgp_export = control_exterior_locate(bgp_export_list, bnp->bgp_peer_as);
		    break;
		}
	    } BGP_PEER_LIST_END(bgp, bnp);
	} BGP_GROUP_LIST_END(bgp);
    }
}


/*
 *	Dump routines of a variety of flavours
 */

/*
 * bgp_dump - dump BGP's global state
 */
static void
bgp_dump(tp, fd)
task *tp;
FILE *fd;
{
    /*
     * Not much to dump here.  Print out the counts of things we're
     * keeping track of.
     */
    (void) fprintf(fd,
 "\tGroups: %d\tPeers: %d (%d configured)\tActive Incoming: %d\n",
      bgp_n_groups,
      bgp_n_peers,
      bgp_n_peers - bgp_n_unconfigured,
      bgp_n_protopeers);
    (void) fprintf(fd,
      "\tFree Peers: %d\tFree Groups: %d\t\tState: %s\n",
      bgp_n_free,
      bgp_n_groups_free,
      ((tp->task_socket == (-1)) ? "Initializing" : "Listening"));

    if (bgp_import_list) {
	control_exterior_dump(fd, 1, control_import_dump, bgp_import_list);
    }
    if (bgp_import_aspath_list) {
	/* XXX */
	control_exterior_dump(fd, 1, control_import_dump, bgp_import_aspath_list);
    }
    if (bgp_export_list) {
	control_exterior_dump(fd, 1, control_export_dump, bgp_export_list);
    }

}


/*
 * bgp_group_dump - dump the state of a peer group
 */
static void
bgp_group_dump(tp, fd)
task *tp;
FILE *fd;
{
    bgpPeerGroup *bgp = (bgpPeerGroup *)tp->task_data;

    (void) fprintf(fd,
      "\tGroup Type: %s\tAS: %u\tLocal AS: %u\tFlags: <%s>\n",
      trace_state(bgp_group_bits, bgp->bgpg_type),
      (u_int)bgp->bgpg_peer_as,
      (u_int)bgp->bgpg_local_as,
      trace_bits(bgp_group_flag_bits, bgp->bgpg_flags));

    (void) fprintf(fd,
      "\tTotal Peers: %d\t\tEstablished Peers: %d\n",
      bgp->bgpg_n_peers,
      bgp->bgpg_n_established);

    if (bgp->bgpg_type != BGPG_EXTERNAL) {
        int wroteit = 0;

	if (bgp->bgpg_n_established > 0) {
	    (void) fprintf(fd, "\tRouting Bit: %u", tp->task_rtbit);
	    wroteit = 1;
	}
	if (bgp->bgpg_igp_proto != 0) {
	    (void) fprintf(fd, "\tIGP Proto: %s\tIGP Routing Bit: %u\n",
	      trace_state(rt_proto_bits, bgp->bgpg_igp_proto),
	      bgp->bgpg_igp_rtbit);
	} else if (wroteit) {
	    (void) fprintf(fd, "\n");
	}
    }
    if (bgp->bgpg_allow != NULL) {
	(void) fprintf(fd, "\tAllowed Unconfigured Peer Addresses:\n");
	control_dmlist_dump(fd, 2, bgp->bgpg_allow,
	  (adv_entry *)0, (adv_entry *)0);
    }
}


/*
 * bgp_peer_dump - dump info about a BGP peer
 */
static void
bgp_peer_dump(tp, fd)
task *tp;
FILE *fd;
{
    bgpPeer *bnp = (bgpPeer *)tp->task_data;
    int printed;

    (void) fprintf(fd, "\tPeer: %#A\tLocal: ", bnp->bgp_addr);
    if (bnp->bgp_myaddr != NULL) {
	(void) fprintf(fd, "%#A", bnp->bgp_myaddr);
    } else if (bnp->bgp_ifap != NULL) {
	(void) fprintf(fd, "%A", bnp->bgp_ifap->ifa_addr_local);
    } else if (bnp->bgp_lcladdr != NULL) {
	(void) fprintf(fd, "%A", bnp->bgp_lcladdr->ifae_addr);
    } else {
	(void) fprintf(fd, "unspecified");
    }
    (void) fprintf(fd, "\tType: %s\n",
		   trace_state(bgp_group_bits, bnp->bgp_type));
    
    (void) fprintf(fd, "\tState: %s\tFlags: <%s>\n",
		   trace_state(bgp_state_bits, bnp->bgp_state),
		   trace_bits(bgp_flag_bits, bnp->bgp_flags));

    (void) fprintf(fd,
		   "\tLast State: %s\tLast Event: %s\tLast Error: %s\n",
		   trace_state(bgp_state_bits, bnp->bgp_laststate),
		   trace_state(bgp_event_bits, bnp->bgp_lastevent),
		   trace_state(bgp_error_bits, bnp->bgp_last_code));

    (void) fprintf(fd, "\tOptions: <%s>\n",
		   trace_bits(bgp_option_bits, bnp->bgp_options));
    printed = 0;
    if (BIT_TEST(bnp->bgp_options, BGPO_VERSION)) {
	(void) fprintf(fd, "\t\tVersion: %d", bnp->bgp_conf_version);
	printed++;
    }
    if (BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
	if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tGateway: %A", bnp->bgp_gateway);
	printed++;
    }
    if (BIT_TEST(bnp->bgp_options, BGPO_LCLADDR)) {
	if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tLocal Address: %A",
		       bnp->bgp_lcladdr->ifae_addr);
	printed++;
    }
    if (BIT_TEST(bnp->bgp_options, BGPO_HOLDTIME)) {
	if (printed == 3) {
	    (void) fprintf(fd, "\n\t");
	    printed = 0;
	} else if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tHoldtime: %d", bnp->bgp_holdtime_out);
	printed++;
    }
    if (BIT_TEST(bnp->bgp_options, BGPO_METRIC_OUT)) {
	if (printed == 3) {
	    (void) fprintf(fd, "\n\t");
	    printed = 0;
	} else if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tMetric Out: %u", (u_int)bnp->bgp_metric_out);
	printed++;
    }
    if (BIT_TEST(bnp->bgp_options, BGPO_PREFERENCE)) {
	if (printed == 3) {
	    (void) fprintf(fd, "\n\t");
	    printed = 0;
	} else if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tPreference: %u", (u_int)bnp->bgp_preference);
	printed++;
    }
    if (bnp->bgp_recv_bufsize != 0) {
	if (printed == 3) {
	    (void) fprintf(fd, "\n\t");
	    printed = 0;
	} else if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tReceive Bufsize: %u", bnp->bgp_recv_bufsize);
	printed++;
    }
    if (bnp->bgp_send_bufsize != 0) {
	if (printed == 3) {
	    (void) fprintf(fd, "\n\t");
	    printed = 0;
	} else if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tSend Bufsize: %u", bnp->bgp_send_bufsize);
	printed++;
    }
    if (bnp->bgp_spool_bufsize != 0) {
	if (printed == 3) {
	    (void) fprintf(fd, "\n\t");
	    printed = 0;
	} else if (printed == 0) {
	    (void) fprintf(fd, "\t");
	}
	(void) fprintf(fd, "\tSpool Limit: %u", bnp->bgp_spool_bufsize);
	printed++;
    }

    if (printed) {
	(void) fprintf(fd, "\n");
    }

    if (bnp->bgp_state != BGPSTATE_IDLE
	&& bnp->bgp_state != BGPSTATE_CONNECT
	&& bnp->bgp_state != BGPSTATE_ACTIVE) {
	if (bnp->bgp_state != BGPSTATE_OPENSENT) {
	    (void) fprintf(fd, "\tPeer Version: %d\tPeer ID: %A\t",
			   bnp->bgp_version,
			   sockbuild_in(0, bnp->bgp_id));
	    (void) fprintf(fd, "Local ID: %A\tPeer Holdtime: %d\n",
			   sockbuild_in(0, bnp->bgp_out_id),
			   bnp->bgp_hisholdtime);
	}
	(void) fprintf(fd,
		       "\tLast traffic (seconds):\tReceived %u\tSent %u\tChecked %u\n",
		       bgp_time_sec - bnp->bgp_last_rcvd,
		       bgp_time_sec - bnp->bgp_last_sent,
		       bgp_time_sec - bnp->bgp_last_checked);
	(void) fprintf(fd,
		       "\tInput messages:\tTotal %lu\tUpdates %lu\tOctets %lu\n",
		       bnp->bgp_in_updates + bnp->bgp_in_notupdates,
		       bnp->bgp_in_updates,
		       bnp->bgp_in_octets);
	(void) fprintf(fd,
		       "\tOutput messages:\tTotal %lu\tUpdates %lu\tOctets %lu\n",
		       bnp->bgp_out_updates + bnp->bgp_out_notupdates,
		       bnp->bgp_out_updates,
		       bnp->bgp_out_octets);
    }
    printed = 0;
    if (bnp->bgp_bufpos != bnp->bgp_readptr) {
	(void) fprintf(fd, "\tReceived and buffered octets: %u",
		       bnp->bgp_readptr - bnp->bgp_bufpos);
	printed++;
    }
    if (bnp->bgp_spool != NULL) {
	(void) fprintf(fd, "\tSpooled octets: %u\n",
		       bnp->bgp_spool->bgp_sph_count);
	printed = 0;
    }
    if (printed) {
	(void) fprintf(fd, "\n");
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
