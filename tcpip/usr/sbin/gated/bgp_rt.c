static char sccsid[] = "@(#)37	1.5  src/tcpip/usr/sbin/gated/bgp_rt.c, tcprouting, tcpip411, GOLD410 12/6/93 17:34:39";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: BRT_BGP_OR_EGP
 *		BRT_GET_METRIC
 *		BRT_GROUP
 *		BRT_ISBGP
 *		BRT_PUT_METRIC
 *		LIST_ALLOC
 *		PROTOTYPE
 *		SEND_ADDR
 *		SEND_PKT_FULL
 *		__PF2
 *		__PF3
 *		__PF4
 *		bgp_add_to_list
 *		bgp_aux_flash
 *		bgp_aux_newpolicy2907
 *		bgp_flash
 *		bgp_group_tsi_dump
 *		bgp_is_on_if
 *		bgp_list_free_return
 *		bgp_list_free_save
 *		bgp_list_more
 *		bgp_newpolicy
 *		bgp_peer_tsi_dump
 *		bgp_recv_update
 *		bgp_rt_reinit
 *		bgp_rt_send_all
 *		bgp_rt_send_ext
 *		bgp_rt_send_group_peer
 *		bgp_rt_send_igp
 *		bgp_rt_send_init1814
 *		bgp_rt_send_int
 *		bgp_rt_send_message_flush
 *		bgp_rt_send_test
 *		bgp_rt_terminate2051
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
 * bgp_rt.c,v 1.19.2.1 1993/06/14 19:48:07 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#include "inet.h"
#include "bgp.h"

/*
 * There are basically six entry points in here, bgp_recv_update(),
 * bgp_rt_terminate(), bgp_rt_send_init(), bgp_flash(), bgp_aux_flash()
 * and bgp_rt_reinit().
 *
 * Oh, I forgot bgp_newpolicy(), and bgp_aux_newpolicy().  That
 * makes eight.
 *
 * bgp_recv_update() is called when there is data to be read from a
 * neighbour's socket (it is the neighbour-task's "read" routine when
 * the neighbour is in established state).  It reads data from the socket
 * into an input buffer in fairly large gulps, processes all complete
 * packets contained in the input buffer, and then goes around for more
 * data.
 *
 * bgp_rt_terminate() is called when a connection to a peer has been
 * terminated or otherwise detected down.  It deletes all routing information
 * received from the terminated peer.
 *
 * bgp_rt_send_init() is called when a new neighbour session is
 * established.  It sends a full set of routing announcements to synchronize
 * the state of the neighbour router with our current condition.  This
 * is used when the KEEPALIVE message which ack's our open to the neighbour
 * has been successfully received.
 *
 * bgp_flash() is called during flash updates to send updated
 * routing information to all established BGP peers.  bgp_flash()
 * scans the group/peer lists looking for peers in established state,
 * and calls the appropriate routine based on the type of group.
 * External peers are processed one at a time.  Test and Internal
 * group peers are processed in parallel to take advantage of the
 * policy uniformity.
 *
 * bgp_aux_flash() is like bgp_flash(), except it is called to flash
 * a single internal group which is operating as an auxiliary to an IGP.
 * This allows the group to use the IGP's policy to control its actions.
 *
 * bgp_rt_reinit() is called after policy may have changed, to allow
 * the reevalutation of import policy of routes already in the table.
 *
 * bgp_newpolicy() is called at the end of a reconfiguration, to
 * allow reevaluation of exported routes based on new policy from
 * the reconfiguration, and to resync with other routing table changes
 * the reconfig may have caused.  This is like bgp_flash(), but with
 * an awareness that the policy may have changed.  bgp_aux_newpolicy()
 * does the same thing for INTERNAL_IGP peers.
 */

/*
 * Outgoing message data.  We build a message packet in the following
 * non-local memory.  The macro adds a new route to the message, while
 * subroutines build the headers and actually transmit the packet.
 */
#define	send_message  ((byte *)task_send_buffer) /* memory to build packet */
static byte *send_hdr_end;		/* end of the headers */
static byte *send_next_route;		/* where the next route goes */


/*
 * See if the send packet has no room for another route
 */
#define	SEND_PKT_FULL() \
	((send_next_route-send_message) > (BGPMAXSENDPKTSIZE-BGP_ROUTE_LENGTH))

/*
 * Add a route to the send packet.
 */
#define	SEND_ADDR(sun)	BGP_PUT_ADDR(sun, send_next_route)

/*
 * Return codes for send_message_flush.  These are used to inform
 * the caller when we've had difficulty writing to the peer.
 */
#define	SEND_OK		(0)
#define	SEND_ALLFAILED	(1)
#define	SEND_SOMEFAILED	(2)


/*
 * Sort list memory management.  We allocate input-buffer-sized blocks
 * of memory (see BGPRECVBUFSIZE) to divide into sizeof(bgp_send_list)
 * (12 byte?) chunks, which are used to build a list of advertisable
 * routes sorted by AS path.  The first structure in the list is used as
 * a chain pointer to keep the big blocks together so we can free them
 * quickly.  The remainder are allocated to the procedure by a macro.
 */
static bgp_send_list *list_block_first = NULL;	/* first block in chain */
static bgp_send_list *list_block_current;	/* current block in chain */
static bgp_send_list *list_next = NULL;		/* next structure to allocate */
static bgp_send_list *list_end = NULL;		/* last structure to allocate */
/* number of structures per block */
#define	list_num_per_block	(BGPRECVBUFSIZE/sizeof(bgp_send_list))

#define	LIST_ALLOC() ((list_next == list_end) ? bgp_list_more() : list_next++)

/*
 * Definitions for sent update types.  It is either an initial update,
 * a flash update or a newpolicy update.
 */
#define	BRTUPD_NEWPOLICY	(0)
#define	BRTUPD_INITIAL		(1)
#define	BRTUPD_FLASH		(2)

static const bits bgp_rt_update_bits[] = {
    {BRTUPD_NEWPOLICY, "new policy"},
    {BRTUPD_INITIAL, "initial"},
    {BRTUPD_FLASH, "flash"},
    {0}
};

/*
 * For some types of peers we save the metric we send in the tsi field.
 * We save it in three bytes to be economical.  These get and put the
 * metric.  Note
 */
#define	BRT_TSI_SIZE	(3)

#define	BRT_PUT_METRIC(rth, bit, metric) \
    do { \
	register metric_t Xmet; \
	byte Xtb[3]; \
	Xmet = (metric); \
	if (Xmet == BGP_METRIC_NONE) { \
	    Xtb[0] = Xtb[1] = Xtb[2] = 0; \
	} else { \
	    Xtb[0] = 0x1; \
	    Xtb[1] = (byte)(Xmet >> 8); \
	    Xtb[2] = (byte)Xmet; \
	} \
	rttsi_set((rth), (bit), Xtb); \
    } while (0)

#define	BRT_GET_METRIC(rth, bit, metric) \
    do { \
	byte Xtb[3]; \
	rttsi_get((rth), (bit), Xtb); \
	if (Xtb[0] == 0) { \
	    (metric) = BGP_METRIC_NONE; \
	} else { \
	    (metric) = ((metric_t)(Xtb[1]) << 8) | ((metric_t)(Xtb[2])); \
	} \
    } while (0)

/*
 * See if this route came from a BGP peer
 */
#define	BRT_ISBGP(rt)	((rt)->rt_gwp->gw_proto == RTPROTO_BGP)

/*
 * See if this route came from BGP or EGP.  Since EGP may not be compiled
 * in we've got a couple of alternatives.
 */
#ifdef	PROTO_EGP
#define	BRT_BGP_OR_EGP(rt) \
    (BRT_ISBGP(rt) || (rt)->rt_gwp->gw_proto == RTPROTO_EGP)
#else	/* PROTO_EGP */
#define	BRT_BGP_OR_EGP(rt)	BRT_ISBGP(rt)
#endif	/* PROTO_EGP */

/*
 * Given a route from a BGP peer, return his group
 */
#define	BRT_GROUP(rt) \
    (((bgpPeer *)((rt)->rt_gwp->gw_task->task_data))->bgp_group)


/*
 * XXX THIS SHOULD NOT BE HERE
 */
#define	bgp_is_on_if(ifap, host) \
    (BIT_TEST((ifap)->ifa_state, IFS_POINTOPOINT) \
      ? (sockaddrcmp_in((host), (ifap)->ifa_addr) \
	|| sockaddrcmp_in((host), (ifap)->ifa_addr_local)) \
      : ((sock2ip((ifap)->ifa_subnetmask) & sock2ip(host)) \
	== sock2ip((ifap)->ifa_subnet)))


/*
 * bgp_peer_tsi_dump - pretty print a peer's tsi info
 */
PROTOTYPE(bgp_peer_tsi_dump,
	  static void,
	 (rt_head *,
	  void_t data,
	  char *));
static void
bgp_peer_tsi_dump(rth, data, buf)
rt_head *rth;
void_t data;
char *buf;
{
    bgpPeer *bnp = (bgpPeer *)data;
    metric_t metric;
    char mybuf[50];

    if ((rth->rth_active) != NULL
      && rtbit_isset(rth->rth_active, bnp->bgp_task->task_rtbit)) {
	BRT_GET_METRIC(rth, bnp->bgp_task->task_rtbit, metric);
	/* pretty stinky */
	if (metric == BGP_METRIC_NONE) {
	    (void) sprintf(mybuf, "none");
	} else {
	    (void) sprintf(mybuf, "%d", metric);
	}
	(void) sprintf(buf, "BGP %s metric %s", bnp->bgp_name, mybuf);
    }
}


/*
 * bgp_group_tsi_dump - pretty print a group's tsi info
 */
PROTOTYPE(bgp_group_tsi_dump,
	  static void,
	 (rt_head *,
	  void_t data,
	  char *));
static void
bgp_group_tsi_dump(rth, data, buf)
rt_head *rth;
void_t data;
char *buf;
{
    bgpPeerGroup *bgp = (bgpPeerGroup *)data;
    metric_t metric;
    char mybuf[50];

    if (rth->rth_active != NULL &&
	rtbit_isset(rth->rth_active, bgp->bgpg_task->task_rtbit)) {
	BRT_GET_METRIC(rth, bgp->bgpg_task->task_rtbit, metric);
	/* pretty stinky */
	if (metric == BGP_METRIC_NONE) {
	    (void) sprintf(mybuf, "none");
	} else {
	    (void) sprintf(mybuf, "%d", metric);
	}
        (void) sprintf(buf, "BGP %s metric %s", bgp->bgpg_name, mybuf);
    }
}



/*
 * bgp_list_more - get more memory for send list structures
 */
PROTOTYPE(bgp_list_more,
	  static bgp_send_list *,
	 (void));
static bgp_send_list *
bgp_list_more()
{
    bgp_send_list *blp;

    /*
     * Either move the current pointer to the next block, or fetch
     * a new block and link it to the block list.  Initialize the
     * end pointer past the last structure, the next pointer to
     * the third structure, and return the second.
     */
    if (list_block_first == NULL || list_block_current->bgpl_next == NULL) {
    	blp = (bgp_send_list *)task_block_alloc(bgp_buf_blk_index);
	blp->bgpl_next = NULL;
	if (list_block_first == NULL) {
	    list_block_first = list_block_current = blp;
	} else {
	    list_block_current->bgpl_next = blp;
	    list_block_current = blp;
	}
    } else {
	list_block_current = list_block_current->bgpl_next;
	blp = list_block_current;
    }

    list_end = blp + list_num_per_block;
    list_next = blp + 2;		/* One for block, one to return */
    return (blp + 1);
}


/*
 * bgp_list_free_save - free the list blocks, but don't actually give them back
 *			to task_block_free() since we may reuse them.
 */
PROTOTYPE(bgp_list_free_save,
	  static void,
	 (void));
static void
bgp_list_free_save()
{
    if (list_block_first != NULL) {
        register bgp_send_list *blp;

	blp = list_block_first;
	list_block_current = blp;
	list_end = blp + list_num_per_block;
	list_next = blp + 1;
    }
}


/*
 * bgp_list_free_return - free the list blocks and return them to the
 *			  task_block_* routines.
 */
PROTOTYPE(bgp_list_free_return,
	  static void,
	 (void));
static void
bgp_list_free_return()
{
    register bgp_send_list *blp, *blpnext;

    blp = list_block_first;
    while (blp != NULL) {
	blpnext = blp->bgpl_next;
	task_block_free(bgp_buf_blk_index, (caddr_t)blp);
	blp = blpnext;
    }
    list_block_first = list_next = list_end = NULL;
}


/*
 * bgp_rt_send_message_init - initialize the outgoing message, setting up the
 *			   path attributes.
 */
static void
bgp_rt_send_message_init __PF4(my_as, as_t,
			       asp, as_path *,
			       asip, as_path_info *,
			       next_hop_ptr, byte **)
{
    register byte *cp;
    register byte *endp;

    /*
     * Point at the start of the send packet, then skip past the
     * place where the header will go.
     */
    cp = send_message;
    BGP_SKIP_HEADER(cp);

    /*
     * Insert the path attributes into the packet.  Leave room for the
     * length, then fill it in afterward.
     */
    endp = aspath_format(my_as, asp, asip, next_hop_ptr, cp + BGP_ATTR_SIZE_LEN);
    /* XXX relies on goodness of macro */
    BGP_PUT_SHORT((endp - cp) - BGP_ATTR_SIZE_LEN, cp);

    /*
     * Initialize the packet pointers.
     */
    send_hdr_end = send_next_route = endp;
}


/*
 * bgp_rt_send_unreachable_init - initialize the outgoing message with
 *			       unreachable attributes.
 */
static void
bgp_rt_send_unreachable_init __PF3(my_as, as_t,
				   next_hop_ptr, byte **,
				   internal, int)
{
    register byte *cp;
    register byte *endp;
    as_path_info api;

    /*
     * Skip past the header for now
     */
    cp = send_message;
    BGP_SKIP_HEADER(cp);

    /*
     * Initialize the AS path info appropriately.
     */
    bzero((caddr_t)&api, sizeof(api));
    if (internal) {
        api.api_flags = APIF_INTERNAL|APIF_UNREACH;
    } else {
	api.api_flags = APIF_UNREACH;
    }

    /*
     * Insert the path attributes into the packet.  Leave room for the
     * length, then fill it in afterward.
     */
    endp = aspath_format(my_as,
			 (as_path *) 0,
			 &api,
			 next_hop_ptr,
			 cp + BGP_ATTR_SIZE_LEN);
    /* XXX relies on goodness of macro */
    BGP_PUT_SHORT((endp - cp) - BGP_ATTR_SIZE_LEN, cp);

    /*
     * Initialize the packet pointers.
     */
    send_hdr_end = send_next_route = endp;
}


/*
 * bgp_rt_send_message_flush - flush the message we have collected to the
 *			remote peer(s).  This is complexified by
 *			error handling.  If we get an error we must
 *			mark the peer for later termination.
 */
PROTOTYPE(bgp_rt_send_message_flush,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  if_addr *,
	  byte *));
static int
bgp_rt_send_message_flush(bgp, send_bnp, ifap, next_hop_ptr)
bgpPeerGroup *bgp;
bgpPeer *send_bnp;
if_addr *ifap;
byte *next_hop_ptr;
{
    int len;
    int done_okay = 0;
    int failed = 0;

    /*
     * Make sure there's something to do
     */
    if (send_next_route == send_hdr_end) {
	return (SEND_OK);
    }

    /*
     * Set up the header for the message.
     */
    len = send_next_route - send_message;
    BGP_PUT_HDRLEN(len, send_message);
    BGP_PUT_HDRTYPE(BGP_UPDATE, send_message);

    /*
     * Now send the message.  We either send to a single peer or send
     * to the whole group.
     */
    if (send_bnp != NULL) {
	if ((send_bnp->bgp_state != BGPSTATE_ESTABLISHED) ||
	    BIT_TEST(send_bnp->bgp_flags, BGPF_WRITEFAILED)) {
	    return (SEND_ALLFAILED);
	}
	if (next_hop_ptr != NULL) {
	    register byte *cp = next_hop_ptr;

	    BGP_PUT_ADDR(send_bnp->bgp_myaddr, cp);
	}
        BGP_ADD_AUTH(&(send_bnp->bgp_authinfo), send_message, len);
	if (bgp_send(send_bnp, send_message, len)) {
	    done_okay++;
	    send_bnp->bgp_last_sent = time_sec;
	    send_bnp->bgp_out_updates++;
	} else {
	    send_bnp->bgp_flags |= BGPF_WRITEFAILED;
	    failed++;
	}
    } else {
	register bgpPeer *bnp;

	BGP_PEER_LIST(bgp, bnp) {
	    if (BIT_TEST(bnp->bgp_flags, BGPF_WRITEFAILED)) {
		if (BIT_TEST(bnp->bgp_flags, BGPF_WRITEFAILED)) {
		    failed++;
		}
		continue;
	    }
	    if ((bnp->bgp_state != BGPSTATE_ESTABLISHED)
	      || (next_hop_ptr == NULL && ifap != bnp->bgp_ifap)) {
		continue;
	    }
	    if (next_hop_ptr != NULL) {
		register byte *cp = next_hop_ptr;
		BGP_PUT_ADDR(bnp->bgp_myaddr, cp);
	    }
            BGP_ADD_AUTH(&(bnp->bgp_authinfo), send_message, len);
	    if (bgp_send(bnp, send_message, len)) {
		done_okay++;
	        bnp->bgp_last_sent = time_sec;
	        bnp->bgp_out_updates++;
	    } else {
		bnp->bgp_flags |= BGPF_WRITEFAILED;
		failed++;
	    }
	} BGP_PEER_LIST_END(bgp, bnp);
    }

    /*
     * Reset the route pointer and return an appropriate error code
     */
    send_next_route = send_hdr_end;
    if (done_okay == 0)
	return (SEND_ALLFAILED);
    if (failed == 0)
	return (SEND_OK);
    return (SEND_SOMEFAILED);
}


/*
 * bgp_rt_send_all - takes a send list for a peer, or a group of peers,
 *		  and sends it on to all interested parties.  This
 *		  assumes that each as_path list is assembled correctly,
 *		  but remains unsorted by metric and next hop.
 */
PROTOTYPE(bgp_rt_send_all,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  bgp_send_list *,
	  if_addr *));
static int
bgp_rt_send_all(bgp, bnp_send, send_list, ifap)
bgpPeerGroup *bgp;
bgpPeer *bnp_send;
bgp_send_list *send_list;
if_addr *ifap;
{
    register bgp_send_list *blp, *blpprev, *blpdone;
    register rt_entry *rtp;
    register sockaddr_un *localaddr;
    register sockaddr_un *next_hop;
    bgp_send_list *listhead;
    as_path_info path_info;
    int error = SEND_OK;

    /*
     * If we'll need the local interface address, fetch it.
     */
    if (ifap != NULL) {
	localaddr = ifap->ifa_addr_local;
    } else {
	localaddr = NULL;	/* to keep hcc quiet */
    }

    /*
     * For each AS path we build a packet for the first route in
     * the list and scan the remainder looking for matches to this.
     * We free up structure members as we go.
     */
    bzero((caddr_t)&path_info, sizeof(path_info));
    if (bgp->bgpg_type == BGPG_INTERNAL ||
	bgp->bgpg_type == BGPG_INTERNAL_IGP) {
	path_info.api_flags = APIF_NEXTHOP|APIF_METRIC|APIF_INTERNAL;
    } else {
	path_info.api_flags = APIF_NEXTHOP|APIF_METRIC;
    }

    for (listhead = send_list; listhead != NULL;
      listhead = listhead->bgpl_asnext) {
	blpdone = listhead;

	while ((blp = blpdone->bgpl_next) != NULL) {
	    /*
	     * Initialize the send buffer based on the path attributes
	     * of the first route
	     */
	    path_info.api_metric = blp->bgpl_metric;
	    rtp = blp->bgpl_route;
	    if (ifap != NULL && rtp->rt_ifap != ifap) {
		/*
		 * Peer is on a net shared with us, but the next hop
		 * is on another network.
		 */
		path_info.api_nexthop = localaddr;
	    } else {
		/*
		 * Either the shared net is the net the next hop
		 * is on, or there isn't a shared net.  Either way
		 * we send the route's next hop.
		 */
		path_info.api_nexthop = rtp->rt_router;
	    }

	    /*
	     * Initialize the route with this guy's path attributes
	     * and the stuff we just computed.  Put the first route
	     * in there.
	     */
	    bgp_rt_send_message_init(bgp->bgpg_local_as, rtp->rt_aspath,
				     &path_info, (byte **)NULL);
	    SEND_ADDR(rtp->rt_dest);

	    /*
	     * Now look through the remainder of the list for matches.
	     * add each one we find to the message and move it up above
	     * the blpdone pointer.  This will save us scanning them
	     * again.
	     */
	    blpprev = blpdone = blp;
	    blp = blp->bgpl_next;
	    while (blp != NULL) {
		/*
		 * If the metric isn't the same, try again.
		 */
		if (blp->bgpl_metric != path_info.api_metric) {
		    blpprev = blp;
		    blp = blp->bgpl_next;
		    continue;
		}

		/*
		 * Compute this guy's next hop for this route.  If
		 * it isn't the same as the one in the packet, forget
		 * it.
		 */
		rtp = blp->bgpl_route;
		if (ifap != NULL && rtp->rt_ifap != ifap) {
		    next_hop = localaddr;
		} else {
		    next_hop = rtp->rt_router;
		}

		if (!sockaddrcmp_in(path_info.api_nexthop, next_hop)) {
		    blpprev = blp;
		    blp = blp->bgpl_next;
		    continue;
		}

		/*
		 * We've got a winner.  Make room for the route in the
		 * message, then store it away.
		 */
		if (SEND_PKT_FULL()) {
		    error = bgp_rt_send_message_flush(bgp, bnp_send,
			ifap, (byte *)0);
		    if (error == SEND_ALLFAILED) {
			goto deadmeat;
		    }
		}
		SEND_ADDR(rtp->rt_dest);

		/*
		 * Finally, bump this entry up past blpdone so we don't
		 * do it again.
		 */
		if (blpprev == blpdone) {
		    blpprev = blpdone = blp;
		    blp = blp->bgpl_next;
		} else {
		    blpprev->bgpl_next = blp->bgpl_next;
		    blp->bgpl_next = blpdone->bgpl_next;
		    blpdone->bgpl_next = blp;
		    blpdone = blp;
		    blp = blpprev->bgpl_next;
		}
	    }

	    /*
	     * Flush any remaining message and go around again if we've
	     * still got some live ones.
	     */
	    error = bgp_rt_send_message_flush(bgp, bnp_send, ifap, (byte *)0);
	    if (error == SEND_ALLFAILED) {
		goto deadmeat;
	    }
	}
    }

deadmeat:
    return error;
}


/*
 * bgp_add_to_list - add a route to a send list
 */
PROTOTYPE(bgp_add_to_list,
	  static bgp_send_list *,
	 (bgp_send_list *,
	  rt_entry *,
	  metric_t metric));
static bgp_send_list *
bgp_add_to_list(listp, rt, metric)
bgp_send_list *listp;
rt_entry *rt;
metric_t metric;
{
    register bgp_send_list *blp, *blpnew;
    register as_path *asp;

    /*
     * Get a list entry to store this in.
     */
    blpnew = LIST_ALLOC();
    blpnew->bgpl_route = rt;
    blpnew->bgpl_metric = metric;

    asp = rt->rt_aspath;
    if (listp != NULL) {
	/*
	 * Search through looking for an existing head for the route's path.
	 * If we find one simply insert the route at the front of the list.
	 */
	for (blp = listp; /* exits at end */; blp = blp->bgpl_asnext) {
	    if (blp->bgpl_aspath == asp) {
		blpnew->bgpl_next = blp->bgpl_next;
		blp->bgpl_next = blpnew;
		return (listp);
	    }
	    if (blp->bgpl_asnext == NULL)
		break;
	}
        blp->bgpl_asnext = LIST_ALLOC();
        blp = blp->bgpl_asnext;
    } else {
	blp = LIST_ALLOC();
	listp = blp;
    }

    /*
     * Here we didn't find a matching list.  Add one to the end.
     */
    blpnew->bgpl_next = NULL;
    blp->bgpl_asnext = NULL;
    blp->bgpl_next = blpnew;
    blp->bgpl_aspath = asp;
    return (listp);
}




/*
 * bgp_rt_send_group_peer - send an initial set of routes to a peer
 *   in a TEST, INTERNAL or INTERNAL_IGP group where there exists at least one
 *   established peer already.  This is a useful special case since there is
 *   no need to check group policy here.  We just advertise whatever
 *   the last guy did.
 */
PROTOTYPE(bgp_rt_send_group_peer,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  rt_list *));
static int
bgp_rt_send_group_peer(bgp, bnp, rtl)
bgpPeerGroup *bgp;
bgpPeer *bnp;
rt_list *rtl;
{
    rt_head *rth;
    u_int rtbit;
    bgp_send_list *blp = NULL;
    int error = SEND_OK;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_rt_send_test: initial update %s neighbour %s",
	      bgp->bgpg_name,
	      bnp->bgp_name);
    }

    rt_open(bnp->bgp_task);
    rtbit = bgp->bgpg_task->task_rtbit;

    RT_LIST(rth, rtl, rt_head) {
	register rt_entry *rt;

	/*
	 * If this isn't an inet route, continue
	 */
	if (socktype(rth->rth_dest) != AF_INET) {
	    continue;
	}

	/*
	 * If there is an active route which is announced, and if our bit is
	 * set, send this out.
	 */
	if (rth->rth_n_announce != 0 &&
	    (rt = rth->rth_active) != NULL &&
	    rtbit_isset(rt, rtbit)) {
	    register metric_t metric = BGP_METRIC_NONE;

	    /*
	     * If this route is external and the peer is a test peer,
	     * we will include the incoming metric in the announcement if
	     * there was one.
	     */
	    if (bgp->bgpg_type == BGPG_INTERNAL) {
		BRT_GET_METRIC(rth, rtbit, metric);
	    } else if (bgp->bgpg_type == BGPG_TEST && BRT_BGP_OR_EGP(rt)) {
		metric = rt->rt_metric;
	    }
	    blp = bgp_add_to_list(blp, rt, metric);
	}
    } RT_LIST_END(rth, rtl, rt_head);

    /*
     * If we found something to send, send it and free the list.
     */
    if (blp != NULL) {
	error = bgp_rt_send_all(bgp, bnp, blp, bnp->bgp_ifap);
	bgp_list_free_save();
    }
    rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    return (error);
}


/*
 * bgp_rt_send_test - send updates to a group of test peers, or send
 *   initial updates to one of them.  The peer pointer is non-NULL only
 *   when we are sending the initial blast of routes to a single peer.
 */
PROTOTYPE(bgp_rt_send_test,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  rt_list *,
	  int));
static int
bgp_rt_send_test(bgp, bnp, rtl, what)
bgpPeerGroup *bgp;
bgpPeer *bnp;
rt_list *rtl;
int what;
{
    rt_head *rth;
    rt_entry *rt, *oldrt;
    bgp_send_list *blp = NULL;
    u_int rtbit;
    byte *next_hop_ptr;
    int error = SEND_OK;
    int unreachables = 0;

    TRACE_PROTO(TR_BGP) {
	if (bnp != NULL) {
	    trace(TR_BGP, 0,
		  "bgp_rt_send_test: %s update %s neighbour %s",
		  trace_state(bgp_rt_update_bits, what),
		  bgp->bgpg_name,
		  bnp->bgp_name);
	} else {
	    trace(TR_BGP, 0, "bgp_rt_send_test: %s update %s",
		  trace_state(bgp_rt_update_bits, what),
		  bgp->bgpg_name);
	}
    }

    /*
     * Open the routing table in case we need to do some bit diddling.
     * Save the rtbit for easy access.  Then loop through the routes.
     */
    rt_open((bnp == NULL) ? bgp->bgpg_task : bnp->bgp_task);
    rtbit = bgp->bgpg_task->task_rtbit;

    RT_LIST(rth, rtl, rt_head) {
        metric_t metric;

	/*
	 * If this isn't an inet route, continue
	 */
	if (socktype(rth->rth_dest) != AF_INET) {
	    continue;
	}

	/*
	 * If this isn't an initial update (in which case, by definition,
	 * there won't be any bits set yet) and someone has announced this
	 * route, look for an existing route announced by us.
	 */
	oldrt = NULL;
	if (what != BRTUPD_INITIAL) {
	    if (rth->rth_last_active != NULL &&
		rtbit_isset(rth->rth_last_active, rtbit)) {
		oldrt = rth->rth_last_active;
	    }
	}

	/*
	 * If there is no existing route, and no active route, we're done.
	 */
	rt = rth->rth_active;
	if (oldrt == NULL && rt == NULL) {
	    continue;
	}

	/*
	 * If we've got one to send and it has an AS path, try to send it.
	 */
	if (rt != NULL &&
	    rt->rt_aspath != NULL &&
	    !BIT_TEST(rt->rt_state, RTS_NOADVISE)) {
	    /*
	     * If there isn't an existing announced route, check for the
	     * basic announceability of this route via BGP.  We are only
	     * interested in class A, B or C routes with a natural net mask.
	     * If it is not one of these, forget it.
	     *
	     * If the existing announced route is the active route we can
	     * avoid announcing that if the only change has been in the
	     * metric and we don't care about it.
	     */
	    if (oldrt == NULL) {
		byte class = inet_class_flags(rt->rt_dest);

		if (BIT_TEST(class, INET_CLASSF_DEFAULT)
		  && !BIT_TEST(bgp->bgpg_options, BGPO_DEFAULTOUT)) {
		    continue;
		} else if (!BIT_TEST(class, INET_CLASSF_NETWORK) ||
			   rt->rt_dest_mask != inet_mask_natural(rt->rt_dest)) {
		    continue;
		}
	    } else if (oldrt == rt) {
		if (rth->rth_changes == NULL
		  || (rth->rth_changes->rtc_flags == RTCF_METRIC
		    && !BRT_BGP_OR_EGP(rt))) {
		    continue;
		}
	    } else {
		rt_changes *rtc = rth->rth_changes;
		as_path *asp = oldrt->rt_aspath;
		sockaddr_un *nh = oldrt->rt_router;

		if (rtc != NULL) {
		    if (BIT_TEST(rtc->rtc_flags, RTCF_ASPATH)) {
			asp = rtc->rtc_aspath;
		    }
		    if (BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)) {
			nh = rtc->rtc_routers[0];
		    }
		}
		if (rt->rt_aspath == asp
		  && sock2ip(rt->rt_router) == sock2ip(nh)) {
		    int rtbgp = BRT_BGP_OR_EGP(rt);
		    int oldrtbgp = BRT_BGP_OR_EGP(rt);
		    metric_t met = oldrt->rt_metric;

		    if (rtc != NULL && BIT_TEST(rtc->rtc_flags, RTCF_METRIC)) {
			met = rtc->rtc_metric;
		    }

		    if ((!rtbgp && !oldrtbgp)
		      || (!rtbgp && met == BGP_METRIC_NONE)
		      || (!oldrtbgp && rt->rt_metric == BGP_METRIC_NONE)
		      || (rt->rt_metric == met)) {
		        rtbit_set(rt, rtbit);
		        (void) rtbit_reset(oldrt, rtbit);
		        continue;
		    }
		}
	    }

	    if (BRT_BGP_OR_EGP(rt)) {
		metric = rt->rt_metric;
	    } else {
	        metric = BGP_METRIC_NONE;
	    }

	    blp = bgp_add_to_list(blp, rt, metric);
	    if (oldrt == rt) {
		oldrt = NULL;
	    } else {
	        rtbit_set(rt, rtbit);
	    }
	} else {
	    rt = NULL;
	}

	/*
	 * Now check out the old route to see if we need to send an
	 * unreachable.  If there is an active route we only need to
	 * announce the old route as down if it has actually gone down.
	 */
	if (oldrt != NULL) {
	    if (rt == NULL
	      || BIT_TEST(oldrt->rt_state, RTS_DELETE|RTS_HOLDDOWN)) {
		if (unreachables == 0) {
		    bgp_rt_send_unreachable_init(bgp->bgpg_local_as,
		      &next_hop_ptr, 0);
		} else if (SEND_PKT_FULL()) {
		    error = bgp_rt_send_message_flush(bgp, bnp,
		      (if_addr *)0, next_hop_ptr);
		    if (error == SEND_ALLFAILED) {
			goto deadmeat;
		     }
	        }
	        SEND_ADDR(oldrt->rt_dest);
	        unreachables++;
	    }
	    (void) rtbit_reset(oldrt, rtbit);
	}
	/* end of loop.  continue's goto here */
    } RT_LIST_END(rth, rtl, rt_head);

    /*
     * If we've sent some unreachable gunk, flush it.
     */
    if (unreachables > 0) {
	error = bgp_rt_send_message_flush(bgp, bnp, (if_addr *)0,
	  next_hop_ptr);
	if (error == SEND_ALLFAILED) {
	    goto deadmeat;
	}
    }

    /*
     * If we've got some reachable info to send, do that now.
     * This is incredibly grotty because of the need to send
     * by interface, particularly because we don't assume a
     * maximum number of interfaces.  We overload this into
     * the send list structure instead.
     */
    if (blp != NULL) {
	if (bnp != NULL) {
	    error = bgp_rt_send_all(bgp, bnp, blp, bnp->bgp_ifap);
	} else {
	    bgp_send_list *ifstart, *iflp;
	    bgpPeer *bnp2;
	    int total, okay, allfailed;

	    ifstart = NULL;
	    BGP_PEER_LIST(bgp, bnp2) {
		if (bnp2->bgp_state != BGPSTATE_ESTABLISHED
		  || (bnp2->bgp_flags & BGPF_WRITEFAILED)) {
		    continue;
		}

		for (iflp = ifstart; iflp != NULL; iflp = iflp->bgpl_next) {
		    if (iflp->bgpl_ifap == bnp2->bgp_ifap) {
			break;
		    }
		}

		if (iflp == NULL) {
		    iflp = LIST_ALLOC();
		    iflp->bgpl_ifap = bnp2->bgp_ifap;
		    iflp->bgpl_next = ifstart;
		    ifstart = iflp;
		}
	    } BGP_PEER_LIST_END(bgp, bnp2);

	    total = okay = allfailed = 0;
	    for (iflp = ifstart; iflp != NULL; iflp = iflp->bgpl_next) {
		int err2;

	        err2 = bgp_rt_send_all(bgp, (bgpPeer *)0, blp, iflp->bgpl_ifap);
		total++;
		if (err2 == SEND_OK) {
		    okay++;
		} else if (err2 = SEND_ALLFAILED) {
		    allfailed++;
		}
	    }

	    if (total == allfailed) {
		error = SEND_ALLFAILED;
	    } else if (total != okay) {
		error = SEND_SOMEFAILED;
	    } /* else use error value from above */
	}
    }

    /*
     * Thank god that's done.  Free the sort list if there is one.
     */
deadmeat:
    if (blp != NULL) {
	bgp_list_free_save();	/* In case someone else wants it */
    }
    if (bnp != NULL) {
        rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    } else {
        rt_close(bgp->bgpg_task, (gw_entry *)0, 0, NULL);
    }
    return (error);
}


/*
 * bgp_rt_send_int - send updates to a group of internal peers, or send
 *   initial updates to one of them.  The peer pointer is non-NULL only
 *   when we are sending the initial blast of routes to a single peer.
 *   This is a lot like the code above.
 */
PROTOTYPE(bgp_rt_send_int,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  rt_list *,
	  int));
static int
bgp_rt_send_int(bgp, bnp, rtl, what)
bgpPeerGroup *bgp;
bgpPeer *bnp;
rt_list *rtl;
int what;
{
    rt_head *rth;
    rt_entry *rt, *oldrt;
    bgp_send_list *blp = NULL;
    u_int rtbit;
    byte *next_hop_ptr;
    int error = SEND_OK;
    int unreachables = 0;
    int notchanged;
    if_addr *ifap;
    adv_results results;

    TRACE_PROTO(TR_BGP) {
	if (bnp != NULL) {
	    trace(TR_BGP, 0,
		  "bgp_rt_send_int: %s update %s neighbour %s",
		  trace_state(bgp_rt_update_bits, what),
		  bgp->bgpg_name,
		  bnp->bgp_name);
	} else {
	    trace(TR_BGP, 0, "bgp_rt_send_int: %s update %s",
		  trace_state(bgp_rt_update_bits, what),
		  bgp->bgpg_name);
	}
    }

    /*
     * Open the routing table in case we need to do some bit diddling.
     * Save the rtbit for easy access.  Then loop through the routes.
     */
    rt_open((bnp == NULL) ? bgp->bgpg_task : bnp->bgp_task);
    rtbit = bgp->bgpg_task->task_rtbit;
    ifap = bgp->bgpg_peers->bgp_ifap;

    RT_LIST(rth, rtl, rt_head) {
	/*
	 * If this isn't an inet route, continue
	 */
	if (socktype(rth->rth_dest) != AF_INET) {
	    continue;
	}

	/*
	 * If this isn't an initial update (in which case, by definition,
	 * there won't be any bits set yet) and someone has announced this
	 * route, look for an existing route announced by us.
	 */
	oldrt = NULL;
	if (what != BRTUPD_INITIAL) {
	    if (rth->rth_last_active != NULL
	      && rtbit_isset(rth->rth_last_active, rtbit)) {
		oldrt = rth->rth_last_active;
	    }
	}

	/*
	 * Try to avoid sending this.  It shouldn't be sent if it
	 * isn't class A/B/C.  It shouldn't be sent if the AS
	 * path isn't there.  It shouldn't be sent if it came from
	 * another internal peer in the same group.  It shouldn't
	 * be sent if there were non-significant changes in the
	 * next hop.
	 */
	rt = rth->rth_active;

	if (rt != NULL) {
	    notchanged = 0;
	    if (oldrt == NULL) {
		byte class = inet_class_flags(rt->rt_dest);

		if (BIT_TEST(class, INET_CLASSF_DEFAULT)
		  && !BIT_TEST(bgp->bgpg_options, BGPO_DEFAULTOUT)) {
		    continue;
		} else if (!BIT_TEST(class, INET_CLASSF_NETWORK) ||
			   rt->rt_dest_mask != inet_mask_natural(rt->rt_dest)) {
		    continue;
		}
	    }

	    if (rt->rt_aspath == NULL
	       || BIT_TEST(rt->rt_state, RTS_NOADVISE)
	       || (BRT_ISBGP(rt) && BRT_GROUP(rt) == bgp)) {
		rt = NULL;
		goto dooldroute;
	    }

	    if (oldrt == rt) {
		rt_changes *rtc = rth->rth_changes;

		if (what == BRTUPD_NEWPOLICY) {
		    if (sock2ip(rt->rt_dest) == INADDR_DEFAULT
		      && !BIT_TEST(bgp->bgpg_options, BGPO_DEFAULTOUT)) {
			rt = NULL;
			goto dooldroute;
		    }
		    if (rtc == NULL) {
			notchanged = 1;
		    } else {
			if ((rtc->rtc_flags
			  & ~(RTCF_METRIC|RTCF_NEXTHOP)) == 0) {
			    notchanged = 1;
			    if (BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)
			      && (ifap == rt->rt_ifap
				|| ifap == rtc->rtc_ifaps[0])) {
				notchanged = 0;
			    } else if (BIT_TEST(rtc->rtc_flags, RTCF_METRIC)
			      && BRT_BGP_OR_EGP(rt)) {
				notchanged = 0;
			    }
			}
		    }
		} else {
		    if (rtc == NULL) {
			continue;
		    }
		    if ((rtc->rtc_flags
		      & ~(RTCF_METRIC|RTCF_NEXTHOP)) == 0) {
			if (BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)
			  && (ifap == rt->rt_ifap
			    || ifap == rtc->rtc_ifaps[0])) {
			    /* nothing */
			} else if (BIT_TEST(rtc->rtc_flags, RTCF_METRIC)
			  && BRT_BGP_OR_EGP(rt)) {
			    /* nothing */
			} else {
			    continue;
			}
		    }
		}
	    }

	    if (BRT_BGP_OR_EGP(rt)) {
	        results.res_metric = rt->rt_metric;
	    } else {
		results.res_metric = bgp->bgpg_metric_out;
	    }
	    if (!export(rt,
			(proto_t) 0,
			bgp->bgpg_peers->bgp_export,
			(adv_entry *) 0,
			(adv_entry *) 0,
			&results)) {
		rt = NULL;
		goto dooldroute;
	    }

	    if (oldrt != NULL && oldrt != rt) {
		/*
		 * Here there is an existing route which isn't the
		 * same as the active route.  See if these routes are
		 * identical with respect to this peer.  If so, mark
		 * this route as unchanged so we don't send anything
		 * if policy doesn't require it.
		 *
		 * XXX This is imperfect since paths with a local AS
		 *     of 0 and paths with a local AS of my AS won't compare.
		 */
		rt_changes *rtc = rth->rth_changes;
		as_path *asp = oldrt->rt_aspath;
		sockaddr_un *nh = oldrt->rt_router;
		if_addr *oifap = oldrt->rt_ifap;

		if (rtc != NULL) {
		    if (BIT_TEST(rtc->rtc_flags, RTCF_ASPATH)) {
			asp = rtc->rtc_aspath;
		    }
		    if (BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)) {
			nh = rtc->rtc_routers[0];
			oifap = rtc->rtc_ifaps[0];
		    }
		}
		if (rt->rt_aspath == asp
		  && (sock2ip(rt->rt_router) == sock2ip(nh)
		    || (ifap != rt->rt_ifap && ifap != oifap))) {
			notchanged = 1;
		}
	    }

	    if (notchanged) {
		metric_t oldmetric;

		BRT_GET_METRIC(rth, rtbit, oldmetric);
		if (oldmetric == results.res_metric) {
		    if (oldrt != rt) {
			rtbit_set(rt, rtbit);
			(void) rtbit_reset(oldrt, rtbit);
		    }
		    continue;
		}
	    }

	    if (rt != NULL) {
		blp = bgp_add_to_list(blp, rt, BGP_METRIC_NONE);
		BRT_PUT_METRIC(rth, rtbit, results.res_metric);
		if (oldrt == rt) {
		    oldrt = NULL;
		} else {
	    	    rtbit_set(rt, rtbit);
		}
	    }
	}

dooldroute:
	/*
	 * Now check out the old route to see if we need to send an
	 * unreachable.  If there is an active route we only need to
	 * announce the old route as down if it has actually gone down.
	 */
	if (oldrt != NULL) {
	    if (rt == NULL
	      || BIT_TEST(oldrt->rt_state, RTS_DELETE|RTS_HOLDDOWN)) {
		if (unreachables == 0) {
		    bgp_rt_send_unreachable_init(bgp->bgpg_local_as,
		      &next_hop_ptr, 1);
		} else if (SEND_PKT_FULL()) {
		    error = bgp_rt_send_message_flush(bgp, bnp,
		      ifap, next_hop_ptr);
		    if (error == SEND_ALLFAILED) {
			goto deadmeat;
		    }
		}
	        SEND_ADDR(oldrt->rt_dest);
	        unreachables++;
	    }
	    (void) rtbit_reset(oldrt, rtbit);
	}
	/* end of loop.  continue's goto here */
    } RT_LIST_END(rth, rtl, rt_head);

    /*
     * If we've sent some unreachable gunk, flush it.
     */
    if (unreachables > 0) {
	error = bgp_rt_send_message_flush(bgp, bnp,
	  (if_addr *)0, next_hop_ptr);
	if (error == SEND_ALLFAILED) {
	    goto deadmeat;
	}
    }

    /*
     * If we've got some reachable info to send, do that now.
     */
    if (blp != NULL) {
	error = bgp_rt_send_all(bgp, bnp, blp, ifap);
    }

deadmeat:
    /*
     * Free the sort list if there is one.
     */
    if (blp != NULL) {
	bgp_list_free_save();	/* In case someone else wants it */
    }
    if (bnp != NULL) {
        rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    } else {
        rt_close(bgp->bgpg_task, (gw_entry *)0, 0, NULL);
    }
    return (error);
}

/*
 * bgp_rt_send_igp - send updates to a group of internal peers operating
 *   with an igp, or send initial updates to one of them.  The peer pointer
 *   is non-NULL only when we are sending the initial blast of routes to
 *   a single peer.  This is a lot like the code above too.
 */
PROTOTYPE(bgp_rt_send_igp,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  rt_list *,
	  int));
static int
bgp_rt_send_igp(bgp, bnp, rtl, what)
bgpPeerGroup *bgp;
bgpPeer *bnp;
rt_list *rtl;
int what;
{
    rt_head *rth;
    rt_entry *rt, *oldrt;
    bgp_send_list *blp = NULL;
    u_int rtbit, igp_rtbit;
    byte *next_hop_ptr;
    int error = SEND_OK;
    int unreachables = 0;

    /*
     * If the igp does not have an rtbit, there isn't anything for us to do.
     */
    igp_rtbit = bgp->bgpg_igp_rtbit;
    if (!igp_rtbit) {
	return error;
    }

    TRACE_PROTO(TR_BGP) {
	if (bnp != NULL) {
	    trace(TR_BGP, 0,
		  "bgp_rt_send_igp: %s update %s neighbour %s",
		  trace_state(bgp_rt_update_bits, what),
		  bgp->bgpg_name,
		  bnp->bgp_name);
	} else {
	    trace(TR_BGP, 0, "bgp_rt_send_igp: %s update %s",
		  trace_state(bgp_rt_update_bits, what),
		  bgp->bgpg_name);
	}
    }

    /*
     * Open the routing table in case we need to do some bit diddling.
     * Save the rtbit for easy access.  Then loop through the routes.
     */
    rt_open((bnp == NULL) ? bgp->bgpg_task : bnp->bgp_task);
    rtbit = bgp->bgpg_task->task_rtbit;

    RT_LIST(rth, rtl, rt_head) {
	/*
	 * If this isn't an inet route, continue
	 */
	if (socktype(rth->rth_dest) != AF_INET) {
	    continue;
	}

	/*
	 * If this isn't an initial update (in which case, by definition,
	 * there won't be any bits set yet) and someone has announced this
	 * route, look for an existing route announced by us.
	 */
	oldrt = NULL;
	if (what != BRTUPD_INITIAL) {
	    if (rth->rth_last_active != NULL
	      && rtbit_isset(rth->rth_last_active, rtbit)) {
		oldrt = rth->rth_last_active;
	    }
	}

	/*
	 * Try to avoid sending this.  If we are running over an IGP
	 * we only consider routes exported by that IGP.  It shouldn't
	 * be sent if it isn't class A/B/C.  It shouldn't be sent if the AS
	 * path isn't there.  It shouldn't be sent if the IGP didn't
	 * import it.  It shouldn't be sent if the IGP could manage
	 * to carry all information itself.  It shouldn't be sent if
	 * the only change was a metric or next hop change.
	 */
	rt = rth->rth_active;

	if (rt != NULL) {
	    if (oldrt == NULL) {
		byte class = inet_class_flags(rt->rt_dest);

		if (!BIT_TEST(class, INET_CLASSF_NETWORK) ||
			   rt->rt_dest_mask != inet_mask_natural(rt->rt_dest)) {
		    continue;
		}
	    }

	    if (rt->rt_aspath == NULL || !rtbit_isset(rt, igp_rtbit)) {
		rt = NULL;
		goto dooldroute;
	    }

	    if (oldrt == rt) {
		/* Ignore metric and next hop changes */
		if (rth->rth_changes == NULL
		  || (rth->rth_changes->rtc_flags
		    & ~(RTCF_NEXTHOP|RTCF_METRIC)) == 0) {
		    continue;
		}
	    }

	    if (!aspath_adv_ibgp(bgp->bgpg_local_as, bgp->bgpg_igp_proto, rt)) {
		/* No need to advertise via IBGP */
		rt = NULL;
	    } else {
	        if (oldrt != NULL && oldrt != rt) {
		    /*
		     * See if this route is unchanged with respect to the
		     * old one.  If so don't bother sending anything.
		     *
		     * XXX This is imperfect since routes with a local
		     *  AS of zero won't match routes with our local AS
		     */
		    as_path *asp = oldrt->rt_aspath;

		    if (rth->rth_changes != NULL
		      && BIT_SET(rth->rth_changes->rtc_flags, RTCF_ASPATH)) {
			asp = rth->rth_changes->rtc_aspath;
		    }

		    if (rt->rt_aspath == asp) {
			(void) rtbit_reset(oldrt, rtbit);
			rtbit_set(rt, rtbit);
			continue;
		    }
		}
		blp = bgp_add_to_list(blp, rt, BGP_METRIC_NONE);
		if (oldrt == rt) {
		    oldrt = NULL;
		} else {
	    	    rtbit_set(rt, rtbit);
		}
	    }
	}

dooldroute:
	/*
	 * Now check out the old route to see if we need to send an
	 * unreachable.  If there is a new active route we don't bother
	 * advertising the old one anyway.  While this violates the
	 * BGP spec, in this case there is no way to avoid violating
	 * the spec in general since some routes are going to be
	 * propagated entirely by OSPF and we have no control over
	 * how these get announced.  Better to do what is efficient.
	 */
	if (oldrt != NULL) {
	    if (rt == NULL) {
		if (unreachables == 0) {
		    bgp_rt_send_unreachable_init(bgp->bgpg_local_as,
		      &next_hop_ptr, 1);
		} else if (SEND_PKT_FULL()) {
		    error = bgp_rt_send_message_flush(bgp, bnp,
		      (if_addr *)0, next_hop_ptr);
		    if (error == SEND_ALLFAILED) {
			goto deadmeat;
		    }
		}
	        SEND_ADDR(oldrt->rt_dest);
	        unreachables++;
	    }
	    (void) rtbit_reset(oldrt, rtbit);
	}
	/* end of loop.  continue's goto here */
    } RT_LIST_END(rth, rtl, rt_head);

    /*
     * If we've sent some unreachable gunk, flush it.
     */
    if (unreachables > 0) {
	error = bgp_rt_send_message_flush(bgp, bnp,
	  (if_addr *)0, next_hop_ptr);
	if (error == SEND_ALLFAILED) {
	    goto deadmeat;
	}
    }

    /*
     * If we've got some reachable info to send, do that now.
     */
    if (blp != NULL) {
	error = bgp_rt_send_all(bgp, bnp, blp, (if_addr *)0);
    }

deadmeat:
    /*
     * Free the sort list if there is one.
     */
    if (blp != NULL) {
	bgp_list_free_save();	/* In case someone else wants it */
    }
    if (bnp != NULL) {
        rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    } else {
        rt_close(bgp->bgpg_task, (gw_entry *)0, 0, NULL);
    }
    return (error);
}


/*
 * bgp_rt_send_ext - send (initial or flash) updates to an external peer.
 *   This is a lot like the code above, too.
 */
PROTOTYPE(bgp_rt_send_ext,
	  static int,
	 (bgpPeerGroup *,
	  bgpPeer *,
	  rt_list *,
	  int));
static int
bgp_rt_send_ext(bgp, bnp, rtl, what)
bgpPeerGroup *bgp;
bgpPeer *bnp;
rt_list *rtl;
int what;
{
    rt_head *rth;
    rt_entry *rt, *oldrt;
    bgp_send_list *blp = NULL;
    u_int rtbit;
    byte *next_hop_ptr;
    int error = SEND_OK;
    int unreachables = 0;
    int notchanged;
    adv_results results;

    TRACE_PROTO(TR_BGP) {
	trace(TR_BGP, 0,
	      "bgp_rt_send_ext: %s update %s neighbour %s",
	      trace_state(bgp_rt_update_bits, what),
	      bgp->bgpg_name,
	      bnp->bgp_name);
    }

    /*
     * Open the routing table in case we need to do some bit diddling.
     * Save the rtbit for easy access.  Then loop through the routes.
     */
    rt_open(bnp->bgp_task);
    rtbit = bnp->bgp_task->task_rtbit;

    RT_LIST(rth, rtl, rt_head) {
	/*
	 * If this isn't an inet route, continue
	 */
	if (socktype(rth->rth_dest) != AF_INET) {
	    continue;
	}

	/*
	 * If this isn't an initial update (in which case, by definition,
	 * there won't be any bits set yet) and someone has announced this
	 * route, look for an existing route announced by us.
	 */
	oldrt = NULL;
	if (what != BRTUPD_INITIAL) {
	    if (rth->rth_last_active != NULL
	      && rtbit_isset(rth->rth_last_active, rtbit)) {
		oldrt = rth->rth_last_active;
	    }
	}

	/*
	 * Try to avoid sending this.  It shouldn't be sent if it isn't
	 * class A/B/C.  It shouldn't be sent if the AS path isn't there.
	 * It shouldn't be sent if the only change was a metric, or if an
	 * insignificant (to this peer) change occured in the next hop).
	 * It shouldn't be sent if policy doesn't allow it
	 */
	rt = rth->rth_active;

	if (rt != NULL) {
	    notchanged = 0;
	    if (oldrt == NULL) {
	        /* Check out class, and whether it has an AS path */
		byte class;

		class = inet_class_flags(rt->rt_dest);
		if (BIT_TEST(class, INET_CLASSF_DEFAULT)
		  && !BIT_TEST(bnp->bgp_options, BGPO_DEFAULTOUT)) {
		    continue;
		} else if (!BIT_TEST(class, INET_CLASSF_NETWORK) ||
			   rt->rt_dest_mask != inet_mask_natural(rt->rt_dest)) {
		    continue;
		}
	    }

	    if (rt->rt_aspath == NULL
	      || BIT_TEST(rt->rt_state, RTS_NOADVISE)) {
		rt = NULL;
		goto dooldroute;
	    }

	    if (what == BRTUPD_NEWPOLICY) {
		if (sock2ip(rt->rt_dest) == INADDR_DEFAULT
		  && !BIT_TEST(bnp->bgp_options, BGPO_DEFAULTOUT)) {
		    rt = NULL;
		    goto dooldroute;
		}
		if (oldrt == rt) {
		    rt_changes *rtc = rth->rth_changes;

		    if (rtc == NULL) {
			notchanged = 1;
		    } else if ((rtc->rtc_flags
		      & ~(RTCF_METRIC|RTCF_NEXTHOP)) == 0) {
		        /* If the next hop changed, see if it is significant */
			if (!BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)
			  || (bnp->bgp_ifap != NULL
			    && bnp->bgp_ifap != rt->rt_ifap
			    && bnp->bgp_ifap != rtc->rtc_ifaps[0])) {
			    notchanged = 1;
			}
		    }
		}
	    } else if (oldrt == rt) {
		/* We can ignore metric changes and some next hop changes */
		rt_changes *rtc = rth->rth_changes;

		if (rtc == NULL) {
		    continue;	/* unchanged */
		}

		if ((rtc->rtc_flags
		  & ~(RTCF_METRIC|RTCF_NEXTHOP)) == 0) {
		    /* If the next hop changed, see if it is significant */
		    if (!BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)
		      || (bnp->bgp_ifap != NULL
			&& bnp->bgp_ifap != rt->rt_ifap
			&& bnp->bgp_ifap != rtc->rtc_ifaps[0])) {
			continue;
		    }
		}
	    }

	    results.res_metric = bnp->bgp_metric_out;
	    if (!export(rt, (proto_t) 0, bnp->bgp_export,
	      (adv_entry *) 0, (adv_entry *) 0, &results)) {
		rt = NULL;
		goto dooldroute;
	    }

	    if (oldrt != rt && oldrt != NULL) {
		/*
		 * Here there is an existing route which isn't the
		 * same as the active route.  See if these routes are
		 * identical with respect to this peer.  If so, mark
		 * this route as unchanged so we don't send anything
		 * if policy doesn't require it.
		 *
		 * XXX This is imperfect since paths with a local AS
		 *     of 0 and paths with a local AS of my AS won't compare.
		 */
		rt_changes *rtc = rth->rth_changes;
		as_path *asp = oldrt->rt_aspath;
		sockaddr_un *nh = oldrt->rt_router;
		if_addr *oifap = oldrt->rt_ifap;

		if (rtc != NULL) {
		    if (BIT_TEST(rtc->rtc_flags, RTCF_ASPATH)) {
			asp = rtc->rtc_aspath;
		    }
		    if (BIT_TEST(rtc->rtc_flags, RTCF_NEXTHOP)) {
			nh = rtc->rtc_routers[0];
			oifap = rtc->rtc_ifaps[0];
		    }
		}
		if (rt->rt_aspath == asp
		  && (sock2ip(rt->rt_router) == sock2ip(nh)
		    || (bnp->bgp_ifap != rt->rt_ifap && bnp->bgp_ifap != oifap))) {
			notchanged = 1;
		}
	    }

	    if (notchanged) {
		metric_t oldmetric;
		/*
		 * Check to see if we really need to readvertise
		 * this.  Compare old metric to current metric
		 */
		BRT_GET_METRIC(rth, rtbit, oldmetric);
		if (oldmetric == results.res_metric) {
		    if (oldrt != rt) {
			rtbit_set(rt, rtbit);
			(void) rtbit_reset(oldrt, rtbit);
		    }
		    continue;
		}
	    }

	    blp = bgp_add_to_list(blp, rt, results.res_metric);
	    BRT_PUT_METRIC(rth, rtbit, results.res_metric);
	    if (oldrt != rt) {
	    	rtbit_set(rt, rtbit);
	    } else {
		oldrt = NULL;
	    }
	}

dooldroute:
	/*
	 * Now check out the old route to see if we need to send an
	 * unreachable.  If there is an active route we only need to
	 * announce the old route as down if it has actually gone down.
	 */
	if (oldrt != NULL) {
	    if (rt == NULL
	      || BIT_TEST(oldrt->rt_state, RTS_DELETE|RTS_HOLDDOWN)) {
		if (unreachables == 0) {
		    bgp_rt_send_unreachable_init(bgp->bgpg_local_as,
		      &next_hop_ptr, 0);
		} else if (SEND_PKT_FULL()) {
		    error = bgp_rt_send_message_flush(bgp, bnp,
		      (if_addr *)0, next_hop_ptr);
		    if (error == SEND_ALLFAILED) {
			goto deadmeat;
		    }
		}
	        SEND_ADDR(oldrt->rt_dest);
	        unreachables++;
	    }
	    (void) rtbit_reset(oldrt, rtbit);
	}
	/* end of loop.  continue's goto here */
    } RT_LIST_END(rth, rtl, rt_head);

    /*
     * If we've sent some unreachable gunk, flush it.
     */
    if (unreachables > 0) {
	error = bgp_rt_send_message_flush(bgp, bnp, bnp->bgp_ifap, next_hop_ptr);
	if (error == SEND_ALLFAILED) {
	    goto deadmeat;
	}
    }

    /*
     * If we've got some reachable info to send, do that now.
     */
    if (blp != NULL) {
	error = bgp_rt_send_all(bgp, bnp, blp, bnp->bgp_ifap);
    }

deadmeat:
    /*
     * Free the sort list if there is one.
     */
    if (blp != NULL) {
	bgp_list_free_save();	/* In case someone else wants it */
    }
    rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    return (error);
}


/*
 * bgp_rt_send_init - initialize the peer by allocating a routing bit,
 *   if necessary, and sending an initial blast of routes.
 */
void
bgp_rt_send_init(bnp)
bgpPeer *bnp;
{
    bgpPeerGroup *bgp;
    rt_list *rtl;
    int error = SEND_OK;

    bgp = bnp->bgp_group;
    rtl = rthlist_active(AF_INET);

    if (rtl != NULL) {
	if (bgp->bgpg_type == BGPG_EXTERNAL) {
	    bnp->bgp_task->task_rtbit = rtbit_alloc(bnp->bgp_task,
	      BRT_TSI_SIZE, (caddr_t)bnp, bgp_peer_tsi_dump);
	    error = bgp_rt_send_ext(bgp, bnp, rtl, BRTUPD_INITIAL);
	    if (error == SEND_OK
	      && !BIT_TEST(bnp->bgp_options, BGPO_NOGENDEFAULT)) {
		rt_default_add();
		BIT_SET(bnp->bgp_flags, BGPF_GENDEFAULT);
	    }
	} else {
	    if (bgp->bgpg_n_established > 1) {
		error = bgp_rt_send_group_peer(bgp, bnp, rtl);
	    } else if (bgp->bgpg_type == BGPG_INTERNAL) {
		bgp->bgpg_task->task_rtbit = rtbit_alloc(bgp->bgpg_task,
		  BRT_TSI_SIZE, (caddr_t)bgp, bgp_group_tsi_dump);
	        error = bgp_rt_send_int(bgp, bnp, rtl, BRTUPD_INITIAL);
	    } else {
		bgp->bgpg_task->task_rtbit = rtbit_alloc(bgp->bgpg_task,
		  0, (caddr_t)0, (void (*)())0);
		if (bgp->bgpg_type == BGPG_INTERNAL_IGP) {
		    error = bgp_rt_send_igp(bgp, bnp, rtl, BRTUPD_INITIAL);
		} else {
	            error = bgp_rt_send_test(bgp, bnp, rtl, BRTUPD_INITIAL);
		}
	    }
	}
	bgp_list_free_return();
    }

    RTLIST_RESET(rtl);

    /*
     * Yakov says he uses the keepalive at the end of the blast for
     * something, so send one now.  It is harmless.  It also makes
     * sure we actually send something so the time we record is correct.
     */
    if (error != SEND_OK || !bgp_send_keepalive(bnp)) {
	bgp_peer_close(bnp, BGPEVENT_ERROR);
    } else {
        bnp->bgp_last_sent = time_sec;
    }
}


/*
 * bgp_flash_done - come here when we have a peer failure.
 */
static void
bgp_flash_done __PF2(tp, task *,
		     data, void_t)
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp, *bnpnext;

    /*
     * We've had a failure somewhere.  Scan the group list looking for
     * it.  Ignore IGP peers since they have their own routine.
     */
    BGP_GROUP_LIST(bgp) {
	if (bgp->bgpg_n_established == 0
	  || bgp->bgpg_type == BGPG_INTERNAL_IGP) {
	    continue;
	}
	bnp = bgp->bgpg_peers;
	while (bnp != NULL) {
	    bnpnext = bnp->bgp_next;
	    if (BIT_TEST(bnp->bgp_flags, BGPF_WRITEFAILED)) {
		bgp_peer_close(bnp, BGPEVENT_ERROR);
		if (bgp->bgpg_n_established == 0) {
		    break;
		}
	    }
	    bnp = bnpnext;
	}
    } BGP_GROUP_LIST_END(bgp);
}


/*
 * bgp_flash - send a flash update to all established peers
 */
void
bgp_flash(tp, rtl)
task *tp;
rt_list *rtl;
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;
    int failed = 0;

    /*
     * Quick return.  If no peers are established, skip this
     */
    if (bgp_n_established == 0) {
	return;
    }

    /*
     * Scan through the group list looking for groups with established
     * peers.  For each external group, call the flash routine for each
     * individual peer.  For the other groups call the group flash routine,
     * then search for failed peers if we receive an indication.
     */
    BGP_GROUP_LIST(bgp) {
	if (bgp->bgpg_n_established == 0) {
	    continue;
	}

	switch (bgp->bgpg_type) {
	case BGPG_EXTERNAL:
	    BGP_PEER_LIST(bgp, bnp) {
		if (bnp->bgp_state == BGPSTATE_ESTABLISHED) {
		    if (bgp_rt_send_ext(bgp, bnp, rtl, BRTUPD_FLASH)
		      != SEND_OK) {
			failed++;
		    }
		}
	    } BGP_PEER_LIST_END(bgp, bnp);
	    break;

	case BGPG_INTERNAL:
	    if (bgp_rt_send_int(bgp, (bgpPeer *)0, rtl, BRTUPD_FLASH)
	      != SEND_OK) {
		failed++;
	    }
	    break;

	case BGPG_TEST:
	    if (bgp_rt_send_test(bgp, (bgpPeer *)0, rtl, BRTUPD_FLASH)
	      != SEND_OK) {
		failed++;
	    }
	    break;

	case BGPG_INTERNAL_IGP:
	    /* Let the IGP flash us */
	    break;
	}
    } BGP_GROUP_LIST_END(bgp);
    bgp_list_free_return();

    /*
     * If we've had some errors, call the
     */
    if (failed > 0) {
	task_crash_add(bgp_listen_task, bgp_flash_done, (void_t) 0);
    }
}


/*
 * bgp_aux_flash_done - called when a flash of an auxilliary has been done
 *   and there have been errors.  This finds the error'd peers in the
 *   group and closes them.
 */
static void
bgp_aux_flash_done __PF2(tp, task *,
			 data, void_t)
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp, *bnpnext;

    bgp = (bgpPeerGroup *)tp->task_data;
    for (bnp = bgp->bgpg_peers; bnp != NULL; bnp = bnpnext) {
	bnpnext = bnp->bgp_next;
	if (BIT_TEST(bnp->bgp_flags, BGPF_WRITEFAILED)) {
	    bgp_peer_close(bnp, BGPEVENT_ERROR);
	    if (bgp->bgpg_n_established == 0) {
		break;
	    }
	}
    }
}


/*
 * bgp_aux_flash - flash update a group of internal peers at the
 *   request of the IGP the internal group is operating with.
 */
void
bgp_aux_flash(tp, rtl)
task *tp;
rt_list *rtl;
{
    bgpPeerGroup *bgp;
    int error;

    /*
     * Get the group.  If it isn't the correct flavour of internal
     * group, complain.
     */
    bgp = (bgpPeerGroup *)tp->task_data;
    if (bgp->bgpg_type != BGPG_INTERNAL_IGP) {
	trace(TR_ALL, LOG_ERR,
	      "bgp_aux_flash: group %s in auxiliary flash type %s, should be internal_igp",
	      bgp->bgpg_name,
	      trace_state(bgp_group_bits, bgp->bgpg_type));
	task_quit(EINVAL);
    }

    /*
     * If we have nothing established yet, return.  Otherwise do
     * the flash.
     */
    if (bgp->bgpg_n_established == 0) {
	return;
    }

    error = bgp_rt_send_igp(bgp, (bgpPeer *)0, rtl, BRTUPD_FLASH);
    bgp_list_free_return();

    /*
     * If something failed, call flash_done after the flash
     * to terminate them.
     */
    if (error != SEND_OK) {
	task_crash_add(tp, bgp_aux_flash_done, (void_t) 0);
    }
}


/*
 * bgp_rt_terminate - delete all the routes in the table from the
 *		      specified peer.
 */
void
bgp_rt_terminate(bnp)
bgpPeer *bnp;
{
    bgpPeerGroup *bgp;

    /*
     * Shitcan this guy's routes, except for test peers which don't have any.
     */
    bgp = bnp->bgp_group;
    if (bgp->bgpg_type != BGPG_TEST) {
        (void) rt_gwunreach(bnp->bgp_task, &(bnp->bgp_gw));
    }
    if (BIT_TEST(bnp->bgp_flags, BGPF_GENDEFAULT)) {
	rt_default_delete();
	BIT_RESET(bnp->bgp_flags, BGPF_GENDEFAULT);
    }

    /*
     * We must be careful here.  If the peer is external, the routing
     * bit is in the peer task and should be reset in the routing table.
     * Otherwise the bit is in the group task, and should only be reset
     * if this is the last established guy.  For the latter we assume
     * that this routine will be called after the peer is taken out
     * of established state.
     */
    if (bgp->bgpg_type == BGPG_EXTERNAL) {
	rtbit_reset_all(bnp->bgp_task,
			bnp->bgp_task->task_rtbit,
			&(bnp->bgp_gw));
    } else if (bgp->bgpg_n_established == 0) {
	rtbit_reset_all(bgp->bgpg_task,
			bgp->bgpg_task->task_rtbit,
			&(bnp->bgp_gw));
    }
}


/*
 * bgp_recv_update - receive an update from a peer.  This is actually
 *		     the task_read routine when we are in established state
 */
void
bgp_recv_update(tp)
task *tp;
{
    register byte *cp, *cpend;
    register bgpPeer *bnp;
    register rt_entry *rt;
    as_path *asp = NULL;
    bgpPeerGroup *bgp;
    as_path_info api;
    rt_parms rtparms;
    sockaddr_un nexthop;
    sockaddr_un dest;
    sockaddr_un *mask;
    int len, type;
    byte *attrp;
    int attrlen, routelen;
    int error_code;
    byte *error_data;
    int count, maybemore = 1;
    u_long max_octets;
    int event = BGPEVENT_RECVUPDATE;

    /*
     * Go around forever trying to read the socket.  We break when we can't
     * get enough to make a full packet.
     */
    bnp = (bgpPeer *)tp->task_data;
    bgp = bnp->bgp_group;
    bzero((caddr_t) &rtparms, sizeof(rtparms));
    rtparms.rtp_dest = &dest;
    rtparms.rtp_n_gw = 1;
    rtparms.rtp_router = &nexthop;
    rtparms.rtp_gwp = &bnp->bgp_gw;
    if (bgp->bgpg_type == BGPG_INTERNAL) {
	rtparms.rtp_state = RTS_NOAGE|RTS_INTERIOR;
    } else if (bgp->bgpg_type == BGPG_INTERNAL_IGP) {
	rtparms.rtp_state = RTS_NOAGE|RTS_INTERIOR|RTS_NOTINSTALL|RTS_NOADVISE;
    } else {
	rtparms.rtp_state = RTS_NOAGE|RTS_EXTERIOR;
    }
    rtparms.rtp_rtd = NULL;
    max_octets = bnp->bgp_in_octets + BGPMAXREAD;
    sockclear_in(&dest);
    rt_open(tp);

    for (;;) {
	/*
	 * Try to read some.  We keep track of whether we got a full
	 * buffer or not so we'll know whether there is a good chance
	 * of getting some next time around.
	 */
	count = bgp_recv(tp, &bnp->bgp_inbuf, 0, bnp->bgp_name);
	if (count < 0) {
	    event = BGPEVENT_CLOSED;
	    goto deadduck;
	}

	cp = bnp->bgp_bufpos;
	if (BGPBUF_SPACE(bnp) > 0) {
	    maybemore = 0;
	}

	/*
	 * Now loop reading messages out of this buffer.  We do this until
	 * we haven't got enough data for a full message.
	 */
	for (;;) {
	    /*
	     * See if we've got a full header now.  If not break
	     * out.  If so, decode it.
	     */
	    if (BGPBUF_LEFT(bnp, cp) < BGP_HEADER_LEN) {
		break;
	    }

	    BGP_GET_HDRLEN(len, cp);

	    /*
	     * Check for a strange length.
	     */
	    if (len < BGP_HEADER_LEN || len > BGPMAXPACKETSIZE) {
		trace(TR_EXT, LOG_WARNING,
		      "bgp_recv_update: peer %s: strange message header length %d",
		      bnp->bgp_name, len);
	    	bgp_send_notify_word(bnp, BGP_ERR_HEADER,
				     BGP_ERRHDR_LENGTH, len);
		event = BGPEVENT_ERROR;	/* not quite right */
		goto deadduck;
	    }

	    /*
	     * See if we've got the full message buffered.  If not,
	     * break out of this.
	     */
	    if (BGPBUF_LEFT(bnp, cp) < len) {
		break;
	    }

	    /*
	     * If we are tracing, do it now.  This is as uneditted as
	     * the packet gets.
	     */
	    bgp_trace(bnp, (bgpProtoPeer *)NULL, "BGP RECV ", 0, cp);

	    /*
	     * Check for an in-range type.  If not complain about this.
	     */
	    BGP_GET_HDRTYPE(type, cp);
	    if (type == 0 || type > BGP_KEEPALIVE) {
	        trace(TR_EXT, LOG_WARNING,
		      "bgp_recv_update: peer %s unrecognized message type %d",
		      bnp->bgp_name,
		      type);
		bgp_send_notify_byte(bnp, BGP_ERR_HEADER,
				     BGP_ERRHDR_TYPE, type);
		event = BGPEVENT_ERROR;	/* not quite right */
		goto deadduck;
	    }

	    /*
	     * Record that we received something
	     */
	    bnp->bgp_last_rcvd = time_sec;

	    /*
	     * Check message authentication.  If failed we're dead.
	     */
	    if (!BGP_CHECK_AUTH(&(bnp->bgp_authinfo), bnp->bgp_task, cp, len)) {
		bgp_send_notify_none(bnp, BGP_ERR_HEADER, BGP_ERRHDR_UNSYNC);
		event = BGPEVENT_CLOSED;	/* not quite right */
		goto deadduck;
	    }

	    /*
	     * Check the type.  This sets the event and filters out
	     * everything except updates.  For keepalives we go
	     * around again.
	     */
	    bnp->bgp_in_octets += (u_long) len;
	    switch(type) {
	    case BGP_OPEN:
		trace(TR_ALL, LOG_ERR,
		      "bgp_recv_update: received OPEN message from %s, state is ESTABLISHED",
		      bnp->bgp_name);
		bnp->bgp_in_notupdates++;
		bgp_send_notify_none(bnp, BGP_ERR_FSM, 0);
		event = BGPEVENT_RECVOPEN;
		goto deadduck;

	    case BGP_KEEPALIVE:
		bnp->bgp_in_notupdates++;
		if (len != BGP_HEADER_LEN) {
		    bgp_send_notify_word(bnp, BGP_ERR_HEADER,
		      BGP_ERRHDR_LENGTH, len);
		    event = BGPEVENT_RECVKEEPALIVE;
		    goto deadduck;
		}
		bnp->bgp_lastevent = BGPEVENT_RECVKEEPALIVE;
		break;

	    case BGP_NOTIFY:
		bnp->bgp_in_notupdates++;
		bgp_log_notify(bnp->bgp_name, cp, len, 0);
		event = BGPEVENT_RECVNOTIFY;
		goto deadduck;

	    case BGP_UPDATE:
	        bnp->bgp_in_updates++;
		break;
	    }

	    cpend = cp + len;
	    BGP_SKIP_HEADER(cp);
	    if (type == BGP_KEEPALIVE) {
		bnp->bgp_bufpos = cp;
		continue;
	    }

	    /*
	     * Here we have an update message.  Make sure we have enough
	     * length for the attributes, then fetch the attributes and
	     * their length.
	     */
	    if ((cpend - cp) < BGP_UPDATE_MIN_LEN) {
		bgp_send_notify_word(bnp, BGP_ERR_HEADER,
		  BGP_ERRHDR_LENGTH, len);
		trace(TR_EXT, LOG_WARNING,
		      "bgp_recv_update: peer %s UPDATE length %d too small",
		      bnp->bgp_name,
		      len);
		goto deadduck;
	    }

	    /*
	     * Fetch the attribute length and a pointer to the attributes.
	     * If this length was screwed up, or what it leaves us isn't
	     * divisible by 4, we're dead.
	     */
	    BGP_GET_UPDATE(attrlen, attrp, cp);
	    routelen = cpend - cp;
	    if (routelen < 0 || (routelen & (BGP_ROUTE_LENGTH-1)) != 0) {
		trace(TR_EXT, LOG_WARNING,
		      "bgp_recv_update: peer %s UPDATE path attributes malformed",
		      bnp->bgp_name);
		bgp_send_notify_none(bnp, BGP_ERR_UPDATE, BGP_ERRUPD_ATTRLIST);
		goto deadduck;
	    }

	    /*
	     * From here on the going gets a little expensive.  For TEST
	     * peers, don't bother with the rest of the message.
	     */
	    if (bnp->bgp_group->bgpg_type == BGPG_TEST) {
		cp = cpend;
	        bnp->bgp_bufpos = cp;
		continue;
	    }

	    /*
	     * Now fetch the AS path.  If this detects an error, barf.
	     */
	    api.api_metric = BGP_METRIC_NONE;
	    api.api_nexthop = &nexthop;
	    asp = aspath_attr(attrp,
			      attrlen,
#ifdef	PATH_VERSION_4
			      PATH_VERSION_2OR3,
#endif	/* PATH_VERSION_4 */
			      bnp->bgp_local_as,
			      &api,
			      &error_code,
			      &error_data);
	    if (asp == NULL) {
		bgp_path_attr_error(bnp, error_code, error_data,
		  "bgp_recv_update");
		goto deadduck;
	    }

	    /*
	     * If the next hop wasn't included, complain about this.
	     */
	    if (!BIT_TEST(api.api_flags, APIF_NEXTHOP)) {
		trace(TR_EXT, LOG_WARNING,
		      "bgp_recv_update: peer %s UPDATE no next hop found",
		      bnp->bgp_name);
		bgp_send_notify_byte(bnp, BGP_ERR_UPDATE,
				     BGP_ERRUPD_MISSING, PA_TYPE_NEXTHOP);
		goto deadduck;
	    }


	    /*
	     * Process unreachables separately since we aren't overly
	     * fussy about path attributes in this case and we can gain
	     * a bit of speed processing these.
	     */
	    if (BIT_TEST(api.api_flags, APIF_UNREACH)) {
	        int route_count = 0;
		byte netclass;

		while (cp < cpend) {
		    route_count++;
		    BGP_GET_ADDR(&dest, cp);
		    netclass = inet_class_flags(&dest);
		    mask = inet_mask_natural(&dest);
		    if (!(netclass & INET_CLASSF_NETWORK)
		      || sock2host(&dest, mask) != 0) {
			trace(TR_EXT, LOG_ERR,
			      "bgp_recv_update: peer %s update included invalid net %A (%d of %d)",
			      bnp->bgp_name,
			      &dest,
			      route_count,
			      routelen/BGP_ROUTE_LENGTH);
			bgp_send_notify_none(bnp, BGP_ERR_UPDATE,
					     BGP_ERRUPD_BADNET);
			goto deadduck;
		    }

		    /*
		     * Try to find existing net from this guy.  If it doesn't
		     * exist, complain if we think we have them all.
		     */
		    rt = rt_locate_gw(RTS_NETROUTE, &dest, mask, &bnp->bgp_gw);
		    if (rt == NULL) {
			if (BIT_TEST(bnp->bgp_options, BGPO_KEEPALL)) {
			    trace(TR_EXT, LOG_WARNING,
				  "bgp_recv_update: peer %s unreachable net %A not sent by him (%d of %d)",
				  bnp->bgp_name,
				  &dest,
				  route_count,
				  routelen/BGP_ROUTE_LENGTH);
			}
			continue;
		    }
		    if (BIT_TEST(rt->rt_state, RTS_DELETE)) {
			trace(TR_EXT, LOG_WARNING,
			      "bgp_recv_update: peer %s net %A already deleted!",
			      bnp->bgp_name,
			      &dest);
		    } else {
			rt_delete(rt);
		    }
		}
	    } else {
		register int i;
		as_entry *ase;
		int hasasout, nouse;
		int noinstall = 0;
		int route_count = 0;

		/*
		 * We need to be a bit more picky here about the path
		 * attributes he sent us.  If it is an external peer,
		 * make sure the first AS in the path is the one he
		 * is operating in.  If it is external or internal
		 * make sure that the next hop in the packet is on
		 * the shared network.
		 */
		if (bgp->bgpg_type == BGPG_EXTERNAL) {
		    if (asp->path_len == 0
		      || (as_t)(ntohs(*PATH_SHORT_PTR(asp)))
			!= bnp->bgp_peer_as) {
			trace(TR_EXT, LOG_ERR,
			      "bgp_recv_update: peer %s AS %u received path with first AS %u",
			      bnp->bgp_name,
			      (u_int)bnp->bgp_peer_as,
			      (u_int)(ntohs(*PATH_SHORT_PTR(asp))));
			/* XXX there is no error for this */
			bgp_send_notify_aspath(bnp, BGP_ERR_UPDATE,
					       BGP_ERRUPD_ASLOOP, asp);
			goto deadduck;
		    }
		}

		if (bgp->bgpg_type != BGPG_INTERNAL_IGP) {
		    /* BGPG_INTERNAL || BGPG_EXTERNAL */
		    if (!BIT_TEST(bnp->bgp_options, BGPO_GATEWAY)) {
		    	if (!bgp_is_on_if(bnp->bgp_ifap, &nexthop)) {
			    trace(TR_EXT, LOG_ERR,
				  "bgp_recv_update: peer %s next hop address %A improper or not on shared net",
				  bnp->bgp_name,
				  &nexthop);
			    bgp_send_notify_none(bnp, BGP_ERR_UPDATE,
						 BGP_ERRUPD_NEXTHOP);
			    goto deadduck;
			}
		    } else {
			sock2ip(&nexthop) = sock2ip(bnp->bgp_gateway);
		    }
		} else {
		    /*
		     * XXX loopback next hop.  For INTERNAL_IGP
		     * the next hop is never used, and can't be expected
		     * to be on a local interface.  Use the loopback
		     * interface for now.
		     */
		    sock2ip(&nexthop) = sock2ip(inet_addr_loopback);
		}

		/*
		 * See if one of our AS's is in the path.  If so this
		 * is unusable.
		 */
		hasasout = nouse = 0;
		AS_LIST(ase) {
		    register u_short as = (u_short)htons(ase->as_as);
		    register u_short *ap = PATH_SHORT_PTR(asp);
		    for (i = (asp->path_len/2); i > 0; i--, ap++) {
			if (as == *ap) {
			    break;
			}
		    }

		    if (i > 0) {
			nouse++;
			if (ase->as_as == bnp->bgp_local_as) {
			    hasasout++;
			}
		    }
		} AS_LIST_END(ase);

		/*
		 * If this is an internal peer and we saw the AS of the
		 * internal session in the path drop him quick.
		 */
		if (bgp->bgpg_type != BGPG_EXTERNAL && hasasout) {
		    trace(TR_EXT, LOG_ERR,
			  "bgp_recv_update: internal peer %s AS %u received path with internal AS",
			  bnp->bgp_name,
			  bnp->bgp_local_as);
		    bgp_send_notify_aspath(bnp, BGP_ERR_UPDATE,
					   BGP_ERRUPD_ASLOOP, asp);
		    goto deadduck;
		}

		/*
		 * XXX if we were to implement AS partition healing we
		 * should check now to see if this path has hasasout
		 * set and is potentially usable for healing.  If so,
		 * transform the path into something we could use.
		 */

		/*
		 * If this route has hasasout set it will never be usable
		 * (if it just has nouse set it may become usable if
		 * a reconfiguration deletes that AS).  If we haven't been
		 * told to keep these routes in the table (note that not
		 * keeping them screws up SNMP a little bit) we don't
		 * install them.  This can save beaucoup de memory.
		 */
		if (hasasout && !BIT_TEST(bnp->bgp_options, BGPO_KEEPALL)) {
		    noinstall = 1;
		} else if (bgp->bgpg_type == BGPG_INTERNAL_IGP) {
		    nouse++;
		}

		/*
		 * If this is an internal or external peer where
		 * the next hop is supposed to be significant, and if
		 * the route is usable by virtue of not including one
		 * of our ASes in the path, then the only way the next hop
		 * could be our own address is essentially by a protocol
		 * violation.  Check to see if this is the case and, if
		 * so, complain and mark the route unuseable.
		 *
		 * XXX should we check for other local addresses on the
		 * same net as well?
		 * XXX next hop reset to localhost since routing table might
		 * belch.  Should fix routing table to allow random next hops
		 * on unuseable routes instead.
		 */
		if (!nouse && sock2ip(&nexthop) == sock2ip(bnp->bgp_myaddr)) {
		    trace(TR_ALL, LOG_ERR,
		     "bgp_recv_update: peer %s next hop %A is our own address!",
		      bnp->bgp_name,
		      &nexthop);
		    nouse++;
		    sock2ip(&nexthop) = sock2ip(inet_addr_loopback); /* XXX */
		}

		if (nouse) {
		    rtparms.rtp_preference = -1;
		}

		/*
		 * Try to find a match for this path so we don't have
		 * to keep looking it up later.
		 */
		if (!noinstall) {
		    rtparms.rtp_asp = asp = aspath_find(asp);
		}

		/*
		 * By now we should be happy as a pig in shit with the
		 * AS path attributes.  Go dig out the networks.
		 */
		while (cp < cpend) {
		    byte netclass;
		    int nousenet = 0;

		    route_count++;
		    BGP_GET_ADDR(&dest, cp);
		    netclass = inet_class_flags(&dest);
		    rtparms.rtp_dest_mask = inet_mask_natural(&dest);
		    if (!BIT_TEST(netclass, INET_CLASSF_NETWORK|INET_CLASSF_LOOPBACK)
			|| sock2host(&dest, rtparms.rtp_dest_mask) != 0) {
			trace(TR_EXT, LOG_ERR,
			      "bgp_recv_update: peer %s update included invalid net %A (%d of %d)",
			      bnp->bgp_name,
			      &dest,
			      route_count,
			      routelen/BGP_ROUTE_LENGTH);
			bgp_send_notify_none(bnp, BGP_ERR_UPDATE,
					     BGP_ERRUPD_BADNET);
			goto deadduck;
		    }

		    /*
		     * Handle special case networks.  Usually we just complain,
		     * though we should probably dump the asshole if he sent
		     * us a loopback network.
		     */
		    if (BIT_TEST(netclass, INET_CLASSF_DEFAULT|INET_CLASSF_LOOPBACK)) {
			if (BIT_TEST(netclass, INET_CLASSF_DEFAULT)) {
			    if (sock2ip(&dest) != INADDR_DEFAULT) {
				trace(TR_ALL, LOG_ERR,
				      "bgp_recv_update: peer %s update included invalid default %A (%d of %d)",
				      bnp->bgp_name,
				      &dest,
				      route_count,
				      routelen/BGP_ROUTE_LENGTH);
				bgp_send_notify_none(bnp, BGP_ERR_UPDATE,
						     BGP_ERRUPD_BADNET);
				goto deadduck;
			    }
			    rtparms.rtp_dest_mask = inet_mask_default;
			    if (!noinstall && !nouse
			      && !BIT_TEST(bnp->bgp_options, BGPO_DEFAULTIN)) {
				trace(TR_EXT, LOG_WARNING,
				      "bgp_recv_update: ignoring default route from peer %s (%d of %d)",
				      bnp->bgp_name,
				      route_count,
				      routelen/BGP_ROUTE_LENGTH);
				nousenet++;
				rtparms.rtp_preference = -1;
			    }
		        } else {
			    trace(TR_EXT, LOG_WARNING,
				  "bgp_recv_update: ignoring loopback route from peer %s (%d of %d)",
				  bnp->bgp_name,
				  route_count,
				  routelen/BGP_ROUTE_LENGTH);
			    nousenet++;
			    rtparms.rtp_preference = -1;
			}
		    }

		    /*
		     * Find any existing route from this guy.  Since our
		     * preference depends only on the route and the AS path
		     * we can avoid calling the policy routine if the AS
		     * path is the same.  This way we can short circuit the
		     * rest.
		     */
		    rt = rt_locate_gw(RTS_NETROUTE, &dest,
				      rtparms.rtp_dest_mask,
				      &bnp->bgp_gw);
		    if (noinstall) {
			/*
			 * Just delete any existing route.
			 */
			if (rt && !BIT_TEST(rt->rt_state, RTS_DELETE)) {
			    rt_delete(rt);
			}
			continue;
		    }

		    /*
		     * If the route is deleted and has announcement bits
		     * set, pretend we didn't see it.  This ensures that
		     * if we get a delete-add from the neighbour (a
		     * common occurance) it is propagated as a delete-add
		     * as well.  This is required by the spec.
		     */
		    if (rt != NULL
		      && BIT_TEST(rt->rt_state, RTS_DELETE)
		      && rt->rt_n_bitsset > 0) {
			rt = NULL;
		    }

		    if (rt != NULL && asp == rt->rt_aspath) {
			(void) rt_change(rt,
					 api.api_metric,
					 rt->rt_tag,
					 rt->rt_preference,
					 1, &api.api_nexthop);
			continue;
		    }

		    /*
		     * If this is an unuseable route, or if this is an
		     * internal peer associated with an IGP, we may not
		     * need to call the policy routines, otherwise we
		     * will.
		     */
		    if (!nouse && !nousenet) {
			rtparms.rtp_preference = bnp->bgp_preference;
		        (void) import(&dest,
				      bnp->bgp_import,
				      bgp_import_aspath_list,
				      (adv_entry *)0,
				      &rtparms.rtp_preference,
				      bnp->bgp_ifap,
				      (void_t) asp);
		    }

		    /*
		     * If we've got a route already and it isn't deleted,
		     * change it to match this.  Otherwise, add it.
		     */
		    if (rt != NULL) {
			(void) rt_change_aspath(rt,
						api.api_metric,
						rtparms.rtp_tag,
						rtparms.rtp_preference,
						1,
						&api.api_nexthop,
						asp);
		    } else {
			rtparms.rtp_metric = api.api_metric;
		        (void) rt_add(&rtparms);
		    }
		}	/* end of while (cp < cpend) */
	    }		/* end of if (unreach) */
	    aspath_free(asp);
	    asp = NULL;
	}		/* end of for (;;) for messages in buffer */

	/*
	 * Delete the messages we just processed from the buffer, then
	 * go around for more if there is likely to be any and we haven't
	 * read our fill.
	 */
	bnp->bgp_bufpos = cp;
	if (!maybemore || bnp->bgp_in_octets >= max_octets) {
	    break;
	}
    }			/* end of for (;;) read more stuff */
    rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    return;

    /*
     * We come here for errors which have screwed us up beyond repair.
     * Delete any AS path we have and close us down.
     */
deadduck:
    if (asp != NULL) {
	aspath_free(asp);
    }
    rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);
    bgp_peer_close(bnp, event);
}


/*
 * bgp_rt_reinit - redo all policy after a reinit.  The things which can
 *     change which effect us are the AS list, the KEEPALL flag being
 *     turned off, and the policy changing.
 */
void
bgp_rt_reinit(bnp)
bgpPeer *bnp;
{
    register rt_entry *rt;
    register as_path *asp;
    rt_list *rtl;
    as_entry *ase;
    pref_t preference;
    int hasasout, nouse;

    rtl = rtlist_gw(&bnp->bgp_gw);
    rt_open(bnp->bgp_task);

    RT_LIST(rt, rtl, rt_entry) {
	/*
	 * Check out the AS path to see if this one is one we shouldn't
	 * be using.
	 */
	hasasout = nouse = 0;
	asp = rt->rt_aspath;

	AS_LIST(ase) {
	    register u_short as = (u_short)htons(ase->as_as);
	    register u_short *ap = PATH_SHORT_PTR(asp);
	    register int i;

	    for (i = (asp->path_len/2); i > 0; i--, ap++) {
		if (as == *ap) {
		    break;
		}
	    }

	    if (i > 0) {
		nouse++;
		if (ase->as_as == bnp->bgp_local_as) {
		    hasasout++;
		}
	    }
	} AS_LIST_END(ase);

	/*
	 * If this is one we kept, but we don't need to any more,
	 * forget the route.  Otherwise, check for a preference
	 * change.
	 *
	 * XXX The check to see if the next hop is the loopback address
	 * below detects routes which are unusable due to them being
	 * sent with a next hop of our own address.  If this changes
	 * in bgp_recv_update() above, change it here too.
	 */
	if (hasasout) {
	    if (!BIT_TEST(bnp->bgp_options, BGPO_KEEPALL) &&
		!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		rt_delete(rt);
	    }
	} else {
	    if (nouse || sock2ip(rt->rt_router) == sock2ip(inet_addr_loopback)) {
	        preference = -1;
	    } else if (sock2ip(rt->rt_dest) == INADDR_DEFAULT
	      && !BIT_TEST(bnp->bgp_options, BGPO_DEFAULTIN)) {
		preference = -1;
	    } else {
		preference = bnp->bgp_preference;
		if (bnp->bgp_import != NULL) {
		    (void) import(rt->rt_dest,
				  bnp->bgp_import,
				  bgp_import_aspath_list,
				  (adv_entry *) 0,
				  &preference,
				  bnp->bgp_ifap,
				  (void_t) rt->rt_aspath);
		}
	    }

	    if (rt->rt_preference != preference) {
		(void) rt_change(rt,
				 rt->rt_metric,
				 rt->rt_tag,
				 preference,
				 0, (sockaddr_un **)0);
	    }
	}
    } RT_LIST_END(rt, rtl, rt_entry);

    RTLIST_RESET(rtl);

    rt_close(bnp->bgp_task, &bnp->bgp_gw, 0, NULL);

    /*
     * Check out default generation.  The _add() before and the
     * _delete() after the rt_close() for maximum consistancy.
     */
    if (!BIT_TEST(bnp->bgp_options, BGPO_NOGENDEFAULT) &&
	!BIT_TEST(bnp->bgp_flags, BGPF_GENDEFAULT)) {
	rt_default_add();
	BIT_SET(bnp->bgp_flags, BGPF_GENDEFAULT);
    }

    if (BIT_TEST(bnp->bgp_options, BGPO_NOGENDEFAULT) &&
	BIT_TEST(bnp->bgp_flags, BGPF_GENDEFAULT)) {
	rt_default_delete();
	BIT_RESET(bnp->bgp_flags, BGPF_GENDEFAULT);
    }
}


/*
 * bgp_newpolicy - reconsider policy for all our peers.  This is
 *   a whole lot like bgp_flash() above.
 */
void
bgp_newpolicy(tp, rtl)
task *tp;
rt_list *rtl;
{
    bgpPeerGroup *bgp;
    bgpPeer *bnp;
    int failed = 0;

    /*
     * Quick return.  If no peers are established, skip this
     */
    if (bgp_n_established == 0) {
	return;
    }

    /*
     * Scan through the group list looking for groups with established
     * peers.  For each external group, call the flash routine for each
     * individual peer.  For the other groups call the group flash routine,
     * then search for failed peers if we receive an indication.
     */
    BGP_GROUP_LIST(bgp) {
	if (bgp->bgpg_n_established == 0) {
	    continue;
	}

	switch (bgp->bgpg_type) {
	case BGPG_EXTERNAL:
	    BGP_PEER_LIST(bgp, bnp) {
		if (bnp->bgp_state == BGPSTATE_ESTABLISHED) {
		    if (bgp_rt_send_ext(bgp, bnp, rtl, BRTUPD_NEWPOLICY)
		      != SEND_OK) {
			failed++;
		    }
		}
	    } BGP_PEER_LIST_END(bgp, bnp);
	    break;

	case BGPG_INTERNAL:
	    if (bgp_rt_send_int(bgp, (bgpPeer *)0, rtl, BRTUPD_NEWPOLICY)
	      != SEND_OK) {
		failed++;
	    }
	    break;

	case BGPG_TEST:
	    if (bgp_rt_send_test(bgp, (bgpPeer *)0, rtl, BRTUPD_NEWPOLICY)
	      != SEND_OK) {
		failed++;
	    }
	    break;

	case BGPG_INTERNAL_IGP:
	    /* Let the IGP flash us */
	    break;
	}
    } BGP_GROUP_LIST_END(bgp);
    bgp_list_free_return();

    /*
     * XXX Someday shouldn't call this directly
     */
    if (failed > 0) {
	bgp_flash_done(tp, (void_t) 0);
    }
}

/*
 * bgp_aux_newpolicy - make new policy adjustments for a group of
 *   internal peers at the request of the IGP the internal group is
 *   operating with.
 */
void
bgp_aux_newpolicy(tp, rtl)
task *tp;
rt_list *rtl;
{
    bgpPeerGroup *bgp;
    int error;

    /*
     * Get the group.  If it isn't the correct flavour of internal
     * group, complain.
     */
    bgp = (bgpPeerGroup *)tp->task_data;
    if (bgp->bgpg_type != BGPG_INTERNAL_IGP) {
	trace(TR_ALL, LOG_ERR,
	      "bgp_aux_newpolicy: group %s in new policy is type %s, should be internal_igp",
	      bgp->bgpg_name,
	      trace_state(bgp_group_bits, bgp->bgpg_type));
	task_quit(EINVAL);
    }

    /*
     * If we have nothing established yet, return.  Otherwise do
     * the flash.
     */
    if (bgp->bgpg_n_established == 0) {
	return;
    }

    error = bgp_rt_send_igp(bgp, (bgpPeer *)0, rtl, BRTUPD_NEWPOLICY);
    bgp_list_free_return();

    /*
     * XXX If something failed, call flash_done to terminate them.  This
     * should be delayed until after the flash.
     */
    if (error != SEND_OK) {
	bgp_aux_flash_done(tp, (void_t) 0);
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
