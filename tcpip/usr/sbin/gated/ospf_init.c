static char sccsid[] = "@(#)92	1.1  src/tcpip/usr/sbin/gated/ospf_init.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:04";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF6
 *		ospf_var_init
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
 * ospf_init.c,v 1.53.2.13 1993/09/24 02:16:42 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_CTYPE
#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"
#include "krt.h"

struct OSPF ospf = { 0 };
block_t	ospf_router_index = 0;
static int ospf_intf_offset;	/* To prevent all the interfaces from firing at the same time */


/**/
#ifdef	IP_MULTICAST
static if_addr *ospf_multicast_ifap;

sockaddr_un *ospf_addr_allspf = 0;
sockaddr_un *ospf_addr_alldr = 0;

static const bits ospf_if_bits[] = {
    { OSPF_IFPS_ALLSPF,	"AllSPF" },
    { OSPF_IFPS_ALLDR,	"AllDR" },
    { 0 }
} ;

static void
ospf_multicast_init __PF1(tp, task *)
{
    if (!ospf_addr_allspf) {
	/* Initialize address constants */
	ospf_addr_allspf = sockdup(sockbuild_in(0, htonl(OSPF_ADDR_ALLSPF)));
	ospf_addr_alldr = sockdup(sockbuild_in(0, htonl(OSPF_ADDR_ALLDR)));
    }

    krt_multicast_add(ospf_addr_allspf);
    krt_multicast_add(ospf_addr_alldr);
}


static void
ospf_multicast_cleanup __PF1(tp, task *)
{
    krt_multicast_delete(ospf_addr_allspf);
    krt_multicast_delete(ospf_addr_alldr);
}
#endif	/* IP_MULTICAST */

/* Packet reception */

/*
 *	Receive a packet and figure out who it is from and to
 */
static void
ospf_recv __PF1(tp, task *)
{
    int n_packets = TASK_PACKET_LIMIT;
    size_t len;
    struct AREA *area;
    sockaddr_un *dst = (sockaddr_un *) 0;

    while (n_packets-- && !task_receive_packet(tp, &len)) {
	register struct ip *ip;
	register struct OSPF_HDR *o_hdr;

	task_parse_ip(ip, o_hdr, struct OSPF_HDR *);
	
	/* Build destination address */
	if (!dst || sock2ip(dst) != ip->ip_dst.s_addr) {
	    if (dst) {
		sockfree(dst);
	    }
	    dst = sockdup(sockbuild_in(0, ip->ip_dst.s_addr));
	}
		
	/* Remember the schedule status of each area */
	AREA_LIST(area) {
	    area->spfsched_save |= area->spfsched;
	    area->spfsched = 0;
	} AREA_LIST_END(area) ;

	/* Process the packet */
	ospf_rxpkt(ip,
		   o_hdr,
		   task_recv_srcaddr,
		   dst);
    }

    /* Free destination address */
    if (dst) {
	sockfree(dst);
    }

    if (++n_packets != TASK_PACKET_LIMIT) {
	/* Scan the areas to see if we need to run the Dijkstra */

	AREA_LIST(area) {

	    area->spfsched |= area->spfsched_save;
	    area->spfsched_save = 0;

	    if (BIT_TEST(area->area_flags, OSPF_AREAF_SPF_RUN)) {
		/* Need to run SPF */

		run_spf(area, !BIT_TEST(area->area_flags, OSPF_AREAF_SPF_FULL));
	    }
	    BIT_RESET(area->area_flags, OSPF_AREAF_SPF_RUN|OSPF_AREAF_SPF_FULL);
	} AREA_LIST_END(area) ;
    }

}


/**/


void
ospf_txpkt __PF6(pkt, struct OSPF_HDR *,
		 intf, struct INTF *,
		 type, u_int,
		 length, size_t,
		 to, u_int32,
		 retrans, int)
{
    struct NBR *nbr;
    flag_t flag;
    sockaddr_un *src = intf->ifap->ifa_addr_local;
    sockaddr_un *dst = sockdup(sockbuild_in(0, to));


    pkt->version = OSPF_VERSION;
    pkt->type = type;
    pkt->length = htons(length);
    pkt->rtr_id = MY_ID;
    pkt->checksum = 0;
    pkt->Auth[0] = pkt->Auth[1] = 0;

    if (intf->type == VIRTUAL_LINK) {
	pkt->area_id = OSPF_BACKBONE;
	pkt->AuType = htons(ospf.backbone.authtype);
	flag = 0;
    } else {
	pkt->area_id = intf->area->area_id;
	pkt->AuType = htons(intf->area->authtype);
	flag = MSG_DONTROUTE;
    }

    pkt->checksum = inet_cksum((void_t) pkt, length);

    switch (ntohs(pkt->AuType)) {
    case OSPF_AUTH_NONE:
	break;

    case OSPF_AUTH_SIMPLE:
	pkt->Auth[0] = intf->authkey[0];
	pkt->Auth[1] = intf->authkey[1];
	break;
    }
    
    if (BIT_TEST(ospf.trace_flags, TR_OSPF_PKT)) {
	ospf_trace(pkt, length, type, 0, intf, src, dst);
    }

#define	ospf_send_packet(pkt, len, flag, addr) \
    if (task_send_packet(ospf.task, (void_t) pkt, len, flag, addr) < 0) { \
	trace(TR_OSPF, LOG_ERR, "OSPF SENT %-15A -> %-15A: %m", \
	      src, \
	      addr); \
    } \

															  
    /* Handle NBMA send cases */
    switch (to) {
    case ALL_UP_NBRS:
	for (nbr = intf->nbr.next; nbr != NBRNULL; nbr = nbr->next) {
	    if (nbr->state != NDOWN) {
		ospf_send_packet(pkt, length, flag, nbr->nbr_addr);
	    }
	}
	break;

    case ALL_ELIG_NBRS:
	for (nbr = intf->nbr.next; nbr != NBRNULL; nbr = nbr->next) {
	    if (nbr->pri) {
		ospf_send_packet(pkt, length, flag, nbr->nbr_addr);
	    }
	}
	break;

    case ALL_EXCH_NBRS:
	for (nbr = intf->nbr.next; nbr != NBRNULL; nbr = nbr->next) {
	    if (nbr->state >= NEXCHANGE) {
		ospf_send_packet(pkt, length, flag, nbr->nbr_addr);
	    }
	}
	break;

    case DR_and_BDR:
	if (intf->dr) {
	    ospf_send_packet(pkt, length, flag, intf->dr->nbr_addr);
	}
	if (intf->bdr) {
	    ospf_send_packet(pkt, length, flag, intf->bdr->nbr_addr);
	}
	break;

    default:
#ifdef	IP_MULTICAST
	if (inet_class_of(dst) == INET_CLASSC_MULTICAST) {
	    /* Multicast sends fail if MSG_DONTROUTE is set */
	    flag = 0;

	    /* Set the source address of multicast packets to be this interface */
	    if (ospf_multicast_ifap != intf->ifap) {
		ifa_free(ospf_multicast_ifap);
		ospf_multicast_ifap = ifa_alloc(intf->ifap);
		if (task_set_option(ospf.task,
				    TASKOPTION_MULTI_IF,
				    ospf_multicast_ifap) < 0) {
		    /* Interface may have gone away, we should detect it soon */
		    
		    goto Return;
		}
	    }
	}
#endif	/* IP_MULTICAST */
	    
	ospf_send_packet(pkt, length, flag, dst);
	break;
    }

    ospf_cumlog[type + 6]++;

#ifdef	IP_MULTICAST
 Return:
#endif	/* IP_MULTICAST */
    sockfree(dst);
#undef	ospf_send_packet
}


/**/
/* Initialization */

const bits ospf_router_type_bits[] = {
    { RTR_IF_TYPE_RTR, "Router" },
    { RTR_IF_TYPE_TRANS_NET, "Transit net" },
    { RTR_IF_TYPE_STUB_NET, "Stub net" },
    { RTR_IF_TYPE_VIRTUAL, "Virtual" },
    {0}
};


const bits ospf_if_type_bits[] = {
    { BROADCAST, "Broadcast" },
    { NONBROADCAST, "NBMA" },
    { POINT_TO_POINT, "PointToPoint" },
    { VIRTUAL_LINK, "Virtual" },
    {0}
};

const bits ospf_if_state_bits[] = {
    { IDOWN, "Down" },
    { ILOOPBACK, "Loopback" },
    { IWAITING, "Waiting" },
    { IPOINT_TO_POINT, "P To P" },
    { IDr, "DR" },
    { IBACKUP, "BackupDR" },
    { IDrOTHER, "DR Other" },
    {0}
};


const bits ospf_neighbor_mode_bits[] = {
    { 0, "None" },
    { SLAVE, "Slave" },
    { MASTER, "Master" },
    { 0, "Null" },
    { SLAVE_HOLD, "Hold" },
    {0}
};

const bits ospf_neighbor_state_bits[] = {
    { NDOWN, "Down" },
    { NATTEMPT, "Attempt" },
    { NINIT, "Init" },
    { N2WAY, "2 Way" },
    { NEXSTART, "Exch Start" },
    { NEXCHANGE, "Exchange" },
    { NLOADING, "Loading" },
    { NFULL, "Full" },
    { 8, "SCVirtual" },
    {0}
};

const bits ospf_change_bits[] = {
    { E_UNCHANGE,		"UnChanged" },
    { E_NEW,			"New" },
    { E_NEXTHOP,		"NextHop" },
    { E_METRIC,			"Metric" },
    { E_WAS_INTER_NOW_INTRA,	"WasInterNowIntra" },
    { E_WAS_INTRA_NOW_INTER,	"WasIntraNowInter" },
    { E_ASE_METRIC,		"AseMetric" },
    { E_WAS_ASE,		"WasAse" },
    { E_WAS_INTRA_NOW_ASE,	"WasIntraNowAse" },
    { E_WAS_INTER_NOW_ASE,	"WasInterNowAse" },
    { E_ASE_TYPE ,		"AseType" },
    { E_ASE_TAG ,		"AseTag" },
    { E_DELETE,			"Delete" },
    { 0 }
};

static const bits ospf_nh_bits[] = {
    { 0 },
    { NH_DIRECT,		"Direct" },
    { NH_NBR,			"Neighbor" },
    { NH_DIRECT_FORWARD,	"Forward" },
    { NH_VIRTUAL,		"Virtual" },
    { NH_LOCAL,			"Local" },
    { 0 }
} ;


/*
 *	Dump interface info
 */
static void
ospf_interface_dump __PF2(tp, task *,
			  fp, FILE *)
{
    struct INTF *intf = (struct INTF *) tp->task_data;
    struct AREA *area = intf->area;
    struct NBR *n;

    /* Interface Info */
    if (intf->ifap) {
	(void) fprintf(fp, "\tInterface: %s\n",
		       intf->ifap->ifa_name);
    }
    (void) fprintf(fp, "\tArea: %A\t\tCost: %d\n",
		   sockbuild_in(0, area->area_id),
		   intf->cost);
    (void) fprintf(fp, "\tState: %s\t\tType: %s\n",
		   trace_state(ospf_if_state_bits, intf->state),
		   trace_state(ospf_if_type_bits, intf->type-1));

    if (intf->type == VIRTUAL_LINK) {
	(void) fprintf(fp, "\tTransit Area: %A\n",
		       sockbuild_in(0, intf->transarea->area_id));
    } else {
	(void) fprintf(fp, "\tPriority: %d\n",
		       intf->pri);
	if (intf->dr) {
	    (void) fprintf(fp, "\tDesignated Router: %A\n",
			   intf->dr->nbr_addr);
	}
	if (intf->bdr) {
	    (void) fprintf(fp, "\tBackup Designated Router: %A\n",
			   intf->bdr->nbr_addr);
	}
    }

    (void) fprintf(fp, "\tAuthentication: ");
    switch (area->authtype) {
    case OSPF_AUTH_NONE:
	(void) fprintf(fp, "none\n");
	break;

    case OSPF_AUTH_SIMPLE:
        {
	    byte *bp = (byte *) intf->authkey;

	    (void) fprintf(fp, "%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n",
			   *bp++, *bp++, *bp++, *bp++,
			   *bp++, *bp++, *bp++, *bp++);
	}
	break;
    }
    
    (void) fprintf(fp, "\tTimers:\n\t\tHello: %#T  Poll: %#T  Dead: %#T  Retrans: %#T\n",
		   intf->hello_timer,
		   intf->poll_timer,
		   intf->dead_timer,
		   intf->retrans_timer);

    /* Neighbor info */
    if (FirstNbr(intf)) {
	(void) fprintf(fp, "\tNeighbors:\n");

	NBRS_LIST(n, intf) {
	    (void) fprintf(fp, "\t\tRouterID: ");
	    if (n->nbr_id) {
		(void) fprintf(fp, "%-15A",
			n->nbr_id);
	    } else {
		(void) fprintf(fp, "%-15s", "Unknown");
	    }
	    (void) fprintf(fp, "\tAddress: %-15A\n",
			   n->nbr_addr);

	    (void) fprintf(fp, "\t\tState: %s\tMode: %-6s\tPriority: %d\n",
			   trace_state(ospf_neighbor_state_bits, n->state),
			   trace_state(ospf_neighbor_mode_bits, n->mode),
			   n->pri);

	    (void) fprintf(fp, "\t\tDR: %A\tBDR: %A\n",
			   n->dr ? sockbuild_in(0, n->dr) : sockbuild_str("None"),
			   n->bdr ? sockbuild_in(0, n->bdr) : sockbuild_str("None"));

	    (void) fprintf(fp, "\t\tLast Hello: %T\tLast Exchange: %T\n",
			   n->last_hello,
			   n->last_exch);

	    /* Retrans list */
	    if (n->rtcnt) {
		int hash = OSPF_HASH_QUEUE;
		struct LSDB_LIST *ll;

		while (hash--) {
		    (void) fprintf(fp, "\t\tRetrans list:\n");
		    for (ll = (struct LSDB_LIST *) &n->retrans[hash];
			 ll;
			 ll = ll->ptr[NEXT]) {
			if (!ll->lsdb || NO_GUTS(ll->lsdb))
			    continue;
			(void) fprintf(fp, "\t\t\tType: %s\t%A %A\n",
				       trace_state(ospf_ls_type_bits, LS_TYPE(ll->lsdb)),
				       sockbuild_in(0, ll->lsdb->key[0]),
				       sockbuild_in(0, ll->lsdb->key[1]));
		    }
		}
	    }
	    (void) fprintf(fp, "\n");
	} NBRS_LIST_END(n, intf) ;
    }
}


/*
 * Interface fini routine
 */
static void
ospf_interface_fini __PF1(intf, struct INTF *)
{
    if_addr *ifap = intf->ifap;

    /* Send hello with rhf reset to notify all neighbors were going away */
    if (intf->state > IDOWN) {
	send_hello(intf, 0,  TRUE);
    }

    if (intf->type != VIRTUAL_LINK) { 
#ifdef	IP_MULTICAST
	if (BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) {
	    /* Multicast may be enabled on this interface */

	    if (BIT_TEST(ifap->ifa_ps[ospf.task->task_rtproto].ips_state, OSPF_IFPS_ALLSPF)) {
		/* Remove ourselves from the All SPF Routers group */

		(void) task_set_option(ospf.task,
				       TASKOPTION_GROUP_DROP,
				       ifap,
				       ospf_addr_allspf);
	    } 
	    
	    if (BIT_TEST(ifap->ifa_ps[ospf.task->task_rtproto].ips_state, OSPF_IFPS_ALLDR)) {
		/* Remove ourselves from the All Designated Routers group */
		
		(void) task_set_option(ospf.task,
				       TASKOPTION_GROUP_DROP,
				       ifap,
				       ospf_addr_alldr);
	    }
	}
	
	/* Reset cached interface if necesary */
	if (ospf_multicast_ifap == ifap) {
	    ifa_free(ospf_multicast_ifap);
	    ospf_multicast_ifap = (if_addr *) 0;
	}
#endif	/* IP_MULTICAST */

	/* Delete all neighbors for an NBMA interface */
	if (intf->type == NONBROADCAST) {
	    struct NBR *n;
	
	    while (n = intf->nbr.next) {
		if (n->nbr_id) {
		    sockfree(n->nbr_id);
		    n->nbr_id = (sockaddr_un *) 0;
		}
		if (n->nbr_addr) {
		    sockfree(n->nbr_addr);
		    n->nbr_addr = (sockaddr_un *) 0;
		}
		intf->nbr.next = n->next;
		ospf.nbrcnt--;
		FREE(n, OMEM_NBR);
	    }
	}

	/* Unlink the interface from if_addr */
	ifap->ifa_ospf_intf = (void_t) 0;
	ifa_free(ifap);
	intf->ifap = (if_addr *) 0;

	/* Free the interface */
	if (intf == intf->area->intf) {
	    intf->area->intf = intf->next;
	} else {
	    struct INTF *last;

	    INTF_LIST(last, intf->area) {
		if (last->next == intf) {
		    last->next = intf->next;
		    break;
		}
	    } INTF_LIST_END(last, intf->area) ;

	    assert(last) ;
	}
    } else {
	/* Virtual Link */

	if (ifap) {
	    ifa_free(ifap);
	    intf->ifap = (if_addr *) 0;
	}
	
	if (intf == ospf.vl) {
	    ospf.vl = intf->next;
	} else {
	    struct INTF *last;

	    VINTF_LIST(last) {
		if (last->next == intf) {
		    last->next = intf->next;
		    break;
		}
	    } VINTF_LIST_END(last) ;

	    assert(last) ;
	}
	ospf.vcnt--;
    }

    INTF_FREE(intf);
}


static void
ospf_interface_terminate __PF1(tp, task *)
{
    struct INTF *intf = (struct INTF *) tp->task_data;

    /* Make the interface go away */
    ospf_interface_fini(intf);

    /* Delete the task */
    task_delete(tp);
}


/*
 * Interface init routine
 */
static void
ospf_interface_init __PF2(intf, struct INTF *,
			  offset, int)
{
    if_addr *ifap = intf->ifap;

    intf->task = task_alloc("OSPF", TASKPRI_PROTO);
    if (intf->type == VIRTUAL_LINK) {
    	/* Virtual Link - no interface */
    	
	trace(TR_OSPF, 0, "ospf_interface_init: initializing virtual interface to neighbor ID %A area %A",
	      intf->nbr.nbr_id,
	      sockbuild_in(0, intf->area->area_id));

	/* Task address is neighbor ID */
	intf->task->task_addr = sockdup(intf->nbr.nbr_id);
    } else {
    	/* Non-virtual interface */
    	
	trace(TR_OSPF, 0, "ospf_interface_init: initializing interface %A  area %A",
	      ifap->ifa_addr,
	      sockbuild_in(0, intf->area->area_id));

	/* Task address is interface address */
	intf->task->task_addr = sockdup(ifap->ifa_addr);

	/* Set interface address pointer back to our interface structure */
	ifap->ifa_ospf_intf = (void_t) intf;

	/* Indicate there is a routing protocol active on this interface */
	BIT_SET(ifap->ifa_rtactive, RTPROTO_BIT(RTPROTO_OSPF));
    }
    intf->task->task_data = (void_t) intf;
    intf->task->task_n_timers = INTF_TIMER_MAX;
    intf->task->task_dump = ospf_interface_dump;
    intf->task->task_terminate = ospf_interface_terminate;
    /* No cleanup necessary, just terminate */
    intf->task->task_cleanup = ospf_interface_terminate;

    if (!task_create(intf->task)) {
	task_quit(EINVAL);
    }

    if (intf->type != VIRTUAL_LINK) {

	/* Make sure our buffer sizes are large enough */
	task_alloc_recv(ospf.task, (size_t) ifap->ifa_mtu + IP_MAXHDRLEN);
	task_alloc_send(ospf.task, (size_t) ifap->ifa_mtu);

	switch (intf->type) {
	case POINT_TO_POINT:
	    /* Fall through */

#ifdef	IP_MULTICAST
	case BROADCAST:
	    if (BIT_TEST(intf->flags, OSPF_INTFF_MULTICAST)) {
		static int multicast_init = FALSE;
    
		/* Set multicast options */

		if (!multicast_init) {
		    /* Disable reception of our own packets */
		    if (task_set_option(ospf.task,
					TASKOPTION_MULTI_LOOP,
					FALSE) < 0) {
			task_quit(errno);
		    }
		    multicast_init = TRUE;
		}

		/* Configure the multicast groups for this interface */
		if ((task_set_option(ospf.task,
				     TASKOPTION_GROUP_ADD,
				     ifap,
				     ospf_addr_allspf) < 0)
		    && (errno != EADDRINUSE)) {
		    /* Failure other than the address is already there */

		    if (intf->type == POINT_TO_POINT) {
			/* This may be due to a bug in the Deering multicast code */
			/* that does not let you set the multicast address with the */
			/* remote end of a P2P link, just disable multicast for this */
			/* interface and continue */

			BIT_RESET(intf->flags, OSPF_INTFF_MULTICAST);
		    } else {
			/* Multicast is required on BROADCAST interfaces */

			task_quit(errno);
		    }
		} else {
		    /* Indicate we joined the group */
		    BIT_SET(ifap->ifa_ps[ospf.task->task_rtproto].ips_state, OSPF_IFPS_ALLSPF);

		    /* Only join the group for all designated routers if we are elligible to become one */
		    if (intf->pri) {
			if ((task_set_option(ospf.task,
					     TASKOPTION_GROUP_ADD,
					     ifap,
					     ospf_addr_alldr) < 0)
			    && (errno != EADDRINUSE)) {
			    /* Failure other than the address is already there */

			    task_quit(errno);
			}

			/* Indicate we joined the group */
			BIT_SET(ifap->ifa_ps[ospf.task->task_rtproto].ips_state, OSPF_IFPS_ALLDR);
		    }
		}
	    }
	    /* Fall through */
#endif	/* IP_MULTICAST */

	case NONBROADCAST:
	    break;

	default:
	    assert(FALSE);
	}

    }
	

    /* create timers */
    /* hello timer */
    (void) timer_create(intf->task,	
			INTF_TIMER_HELLO,
			"Hello",
			0,
			intf->hello_timer,
			(time_t) offset,
			tq_hellotmr,
			(void_t) 0);
    /* retransmit timer */
    (void) timer_create(intf->task,
			INTF_TIMER_RXMIT,
			"Retransmit",
			0,
			intf->retrans_timer,
			(time_t) offset,
			tq_retrans,
			(void_t) 0);

    if (intf->type != VIRTUAL_LINK
	&& BIT_TEST(ifap->ifa_state, IFS_UP)
	&& !BIT_TEST(task_state, TASKS_TEST)) {
	/* Init this interface */
	
	(*(if_trans[INTF_UP][IDOWN]))(intf);
    }
}


/*
 * Area terminate routine
 */
static void
ospf_area_terminate __PF1(area, struct AREA *)
{
    struct INTF *intf;
    struct LSDB_LIST *txq = LLNULL;
    int i;

    /* Delete any interfaces that were disabled */
    while (intf = area->intf) {
	ospf_interface_fini(intf);
    } ;

    if (ospf.router_id) {
	/* build_rtr and run spf */
	reset_rtr_lock(area);
	area->build_rtr = FALSE;
	area->spfsched |= build_rtr_lsa(area, &txq, 0);
	ospf_freeq((struct Q **)&txq, OMEM_LL);

	/* Now that interfaces are deleted, run spf to cleanup */
	/* XXX this could be optimized to only run ASE's once */
	area->spfsched = ALLSCHED;
	run_spf(area, 0);
    }
    
    /* Free NetRanges */
    ospf_freeRangeList(area);
    
    /* Free hashtable */
    for(i = LS_STUB; i < LS_ASE; i++) {
	FREE((area->htbl[i]),OMEM_LSDB);
    }

    /* Free LSDB link-list heads */
    DBGUTS_FREE(&area->spf);	 		/* spf tree */ 
    DBGUTS_FREE(&area->candidates);		/* spf candidate list */
    DBGUTS_FREE(&area->asblst);			/* asb rtrs from attached areas */
    DBGUTS_FREE(&area->sumnetlst);		/* nets from attached areas */
    DBGUTS_FREE(&area->interlst);		/* routes from backbone */
    DBGUTS_FREE(&area->dflt_sum);		/* if ABRtr and stub area */
    if (area->area_id != OSPF_BACKBONE) {
	AREA_FREE(area);
    }
    ospf.acnt--;
}


/*
 *	Display OSPF information
*/
static void
ospf_dump __PF2(tp, task *,
		fp, FILE *) {

    struct AREA *area;
    struct LSDB *e, *hp;
    struct NBR_LIST *nl;
    int type;

    /* OSPF information */
    (void) fprintf(fp, "\tRouterID: %A",
		   ospf.router_id);
    if (IAmBorderRtr || IAmASBorderRtr) {
	(void) fprintf(fp, "\tBorder Router:%s%s\n",
		       IAmBorderRtr ? " Area" : "",
		       IAmASBorderRtr ? " AS" : "");
    } else {
	(void) fprintf(fp, "\n");
    }
    
    (void) fprintf(fp, "\tPreference:\tInter/Intra: %d\tExternal: %d\n",
		   ospf.preference,
		   ospf.preference_ase);
    (void) fprintf(fp, "\tDefault:\tMetric: %d\tTag: %A\tType: %d\n",
		   ospf.export_metric,
		   ospf_path_tag_dump(inet_autonomous_system, ospf.export_tag),
		   ospf.export_type);
    (void) fprintf(fp, "\tSPF count: %d\n",
		   SPFCNT);

    (void) fprintf(fp, "\tMonitor authentication: ");
    switch (ospf.mon_authtype) {
    case OSPF_AUTH_NONE:
	(void) fprintf(fp, "none\n");
	break;

    case OSPF_AUTH_SIMPLE:
        {
	    byte *bp = (byte *) ospf.mon_authkey;

	    (void) fprintf(fp, "%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x\n",
			   *bp++, *bp++, *bp++, *bp++,
			   *bp++, *bp++, *bp++, *bp++);
	}
	break;
    }
    
    if (ospf.import_list) {
	adv_entry *list;

	(void) fprintf(fp, "\tImport controls:\n");
	
	ADV_LIST(ospf.import_list, list) {
	    int first = TRUE;
	    
	    if (BIT_TEST(list->adv_flag, ADVFOT_PREFERENCE)) {
		if (first) {
		    (void) fprintf(fp, "\t");
		    first = FALSE;
		}
		(void) fprintf(fp, "\tPreference %d",
			       list->adv_result.res_preference);
	    }
	    if (PS_FUNC_VALID(list, ps_print)) {
		if (first) {
		    (void) fprintf(fp, "\t");
		    first = FALSE;
		}
		(void) fprintf(fp, "\t%s",
			       PS_FUNC(list, ps_print)(list->adv_ps, TRUE));
	    }
	    if (!first) {
		(void) fprintf(fp, ":\n");
	    }
			   
	    control_dmlist_dump(fp, 3, list->adv_list, (adv_entry *) 0, (adv_entry *) 0);
	    (void) fprintf(fp, "\n");
	} ADV_LIST_END(ospf.import_list, adv) ;
    }

    if (ospf.export_list) {
	adv_entry *list;

	(void) fprintf(fp, "\tExport controls:\n");
	
	ADV_LIST(ospf.export_list, list) {
	    adv_entry *adv;
	    int first = TRUE;
	    
	    if (BIT_TEST(list->adv_flag, ADVFOT_METRIC)) {
		if (first) {
		    (void) fprintf(fp, "\t");
		    first = FALSE;
		}
		(void) fprintf(fp, "\tMetric %d",
			       list->adv_result.res_metric);
	    }
	    if (PS_FUNC_VALID(list, ps_print)) {
		if (first) {
		    (void) fprintf(fp, "\t");
		    first = FALSE;
		}
		(void) fprintf(fp, "\t%s",
			       PS_FUNC(list, ps_print)(list->adv_ps, TRUE));
	    }
	    if (!first) {
		(void) fprintf(fp, ":\n");
	    }
			   
	    if (adv = list->adv_list) {
		do {
		    control_entry_dump(fp, 3, adv);
		    if (adv->adv_list) {
			control_dmlist_dump(fp, 3, adv->adv_list, (adv_entry *) 0, (adv_entry *) 0);
		    }
		    do {
			adv = adv->adv_next;
		    } while (adv && !BIT_TEST(adv->adv_flag, ADVF_FIRST));
		} while (adv);
	    }
	    (void) fprintf(fp, "\n");
	} ADV_LIST_END(ospf.export_list, adv) ;
    }
    (void) fprintf(fp, "\n");

    /* Area information */
    AREA_LIST(area) {

	(void) fprintf(fp, "\tArea %A:\n",
		       sockbuild_in(0, area->area_id));

	(void) fprintf(fp, "\t\tAuthtype: ");
	switch (area->authtype) {
	case OSPF_AUTH_NONE:
	    (void) fprintf(fp, "none");
	    break;

	case OSPF_AUTH_SIMPLE:
	    (void) fprintf(fp, "simple");
	    break;

	default:
	    (void) fprintf(fp, "invalid(%d)",
			   area->authtype);
	}

	if (BIT_TEST(area->area_flags, OSPF_AREAF_STUB)) {
	    (void) fprintf(fp, "\tStub");
	} else if (BIT_TEST(area->area_flags, OSPF_AREAF_NSSA)) {
	    (void) fprintf(fp, "\tNSSA");
	}
	if (BIT_TEST(area->area_flags, OSPF_AREAF_STUB_DEFAULT)) {
	    (void) fprintf(fp, "\tDefault cost: %d",
			   area->dflt_metric);
	}
	(void) fprintf(fp, "\n\n");

	/* Print ranges */
	if (area->nr.ptr[NEXT]) {
	    register struct NET_RANGE *nr;

	    (void) fprintf(fp, "\t\tRanges:\n");

	    RANGE_LIST(nr, area) {
		(void) fprintf(fp, "\t\t\t%-15A %-15A\tcost %u\n",
			       sockbuild_in(0, nr->net),
			       sockbuild_in(0, nr->mask),
			       nr->cost);
	    } RANGE_LIST_END(nr, area) ;

	    (void) fprintf(fp, "\n");
	}

	/* Print stub hosts */
	if (area->hosts.ptr[NEXT]) {
	    struct OSPF_HOSTS *host;

	    (void) fprintf(fp, "\t\tStub hosts:\n");
	    
	    for (host = area->hosts.ptr[NEXT]; host != HOSTSNULL; host = host->ptr[NEXT]) {
		(void) fprintf(fp, "\t\t\t%-15A\tcost %u\n",
			       sockbuild_in(0, host->if_addr),
			       host->cost);
	    }
	}
	
	(void) fprintf(fp, "\t\tLink State Database:\n");
	for (type = LS_STUB; type < LS_ASE; type++) {
	    for (hp = area->htbl[type]; hp < &(area->htbl[type][HTBLSIZE]); hp++) {
		for (e = hp; DB_NEXT(e); e = DB_NEXT(e)) {
		    int i, cnt;
		    struct NET_LA_PIECES *att_rtr;
		    struct RTR_LA_PIECES *linkp;
		    struct LSDB *lp = DB_NEXT(e);

		    if (NO_GUTS(lp)) {
			/* Not valid */
			continue;
		    }

		    (void) fprintf(fp, "\t\t%s\tAdvRtr: %A\tLen: %d\tAge: %#T\tSeq: %08x\n",
				   trace_state(ospf_ls_type_bits, type),
				   sockbuild_in(0, ADV_RTR(lp)),
				   ntohs(LS_LEN(lp)),
				   MIN(ADV_AGE(lp), MaxAge),
				   ntohl(LS_SEQ(lp)));

		    switch (type) {
		    case LS_STUB:
		    case LS_NET:
			(void) fprintf(fp, "\t\t\tRouter: %A Netmask: %A Network: %A\n",
				       sockbuild_in(0, LS_ID(lp)),
				       sockbuild_in(0, DB_MASK(lp)),
				       sockbuild_in(0, DB_NETNUM(lp)));

			cnt = ntohs(DB_NET(lp)->ls_hdr.length) - NET_LA_HDR_SIZE;

			for (att_rtr = &(DB_NET(lp)->att_rtr), i = 0;
			     i < cnt;
			     att_rtr++, i += 4) {
			    (void) fprintf(fp, "\t\t\tAttached Router: %A\n",
					   sockbuild_in(0, att_rtr->lnk_id));
			}
			break;
			
		    case LS_RTR:
			(void) fprintf(fp, "\t\t\tRouterID: %A\tArea Border: %s\tAS Border: %s\n",
				       sockbuild_in(0, LS_ID(lp)),
				       (ntohs(DB_RTR(lp)->E_B) & bit_B) ? "On" : "Off",
				       (ntohs(DB_RTR(lp)->E_B) & bit_E) ? "On" : "Off");

			for (cnt = ntohs(DB_RTR(lp)->lnk_cnt),
			     i = 0,
			     linkp = (struct RTR_LA_PIECES *) & (DB_RTR(lp)->link);
			     i < cnt;
			     linkp = (struct RTR_LA_PIECES *) ((long) linkp +
							      RTR_LA_PIECES_SIZE +
							      ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE)),
			     i++) {
			    (void) fprintf(fp, "\t\t\t\tType: %s\tCost: %u\n",
					   trace_state(ospf_router_type_bits, linkp->con_type - 1),
					   ntohs(linkp->tos0_metric));
			    switch (linkp->con_type) {
			    case RTR_IF_TYPE_RTR:
			    case RTR_IF_TYPE_VIRTUAL:
				(void) fprintf(fp, "\t\t\t\tRouterID: %A\tAddress: %A\n",
					       sockbuild_in(0, linkp->lnk_id),
					       sockbuild_in(0, linkp->lnk_data));
				break;
				
			    case RTR_IF_TYPE_TRANS_NET:
				(void) fprintf(fp, "\t\t\t\tDR: %A\tAddress: %A\n",
					       sockbuild_in(0, linkp->lnk_id),
					       sockbuild_in(0, linkp->lnk_data));
				break;
				
			    case RTR_IF_TYPE_STUB_NET:
				(void) fprintf(fp, "\t\t\t\tNetwork: %A\tNetMask: %A\n",
					       sockbuild_in(0, linkp->lnk_id),
					       sockbuild_in(0, linkp->lnk_data));
				break;
			    }
			}
			break;

		    case LS_SUM_NET:
			(void) fprintf(fp, "\t\t\tNetwork: %A\tNetmask: %A\tCost: %u\n",
				       sockbuild_in(0, LS_ID(lp)),
				       sockbuild_in(0, DB_SUM(lp)->net_mask),
				       BIG_METRIC(lp));
			break;

		    case LS_SUM_ASB:
			(void) fprintf(fp, "\t\t\tRouterID: %A\tCost: %u\n",
				       sockbuild_in(0, LS_ID(lp)),
				       BIG_METRIC(lp));
			break;
		    }

		    /* Retransmission list */
		    if (nl = DB_RETRANS(lp)) {
			(void ) fprintf(fp, "\t\tRetransmission List:\n");

			do {
			    (void) fprintf(fp, "\t\t\tNeighbor: %-15A\n",
					   nl->nbr->nbr_addr);
			} while (nl = nl->ptr[NEXT]) ;
		    }
		    (void) fprintf(fp, "\n");
		}
	    }
	}
    } AREA_LIST_END(area) ;

    /* Print AS External LSDB */
    (void) fprintf(fp, "\n\tAS Externals:\n\n");
    area = ospf.area;
    for (hp = area->htbl[LS_ASE]; hp && hp < &(area->htbl[LS_ASE][HTBLSIZE]); hp++) {
	for (e = hp; DB_NEXT(e); e = DB_NEXT(e)) {
	    struct LSDB *lp = DB_NEXT(e);

	    if (NO_GUTS(lp)) {
		/* Not valid */
		continue;
	    }

	    (void) fprintf(fp, "\t\tAdvRtr: %A\tLen: %d\tAge: %#T\tSeq: %08x\n",
			   sockbuild_in(0, ADV_RTR(lp)),
			   ntohs(LS_LEN(lp)),
			   MIN(ADV_AGE(lp), MaxAge),
			   ntohl(LS_SEQ(lp)));
	    (void) fprintf(fp, "\t\tNetwork: %A\tNetmask: %A\tCost: %u\n",
			   sockbuild_in(0, LS_ID(lp)),
			   sockbuild_in(0, DB_ASE(lp)->net_mask),
			   BIG_METRIC(lp));
	    (void) fprintf(fp, "\t\tType: %d\tForward: %A\tTag: %A\n",
			   ASE_TYPE1(lp) ? 1 : 2,
			   sockbuild_in(0, DB_ASE_FORWARD(lp)),
			   ospf_path_tag_dump(inet_autonomous_system, DB_ASE(lp)->tos0.ExtRtTag));

	    /* Retransmission list */
	    if (nl = DB_RETRANS(lp)) {
		(void) fprintf(fp, "\t\tRetransmission List:\n");
		
		do {
		    (void) fprintf(fp, "\t\t\tNeighbor: %-15A\n",
				   nl->nbr->nbr_addr);
		} while (nl = nl->ptr[NEXT]) ;
	    }
	    (void) fprintf(fp, "\n");
	}
    }

    /* Next hops */
    if (ospf.nh_list.nh_forw != &ospf.nh_list) {
	register struct NH_BLOCK *np;

	(void) fprintf(fp, "\tNext hops:\n");

	NH_LIST(np) {
	    (void) fprintf(fp, "\t\t%A\t%-8s\tintf %-15A\trefcount %d\n",
			   sockbuild_in(0, np->nh_addr),
			   trace_state(ospf_nh_bits, np->nh_type),
			   np->nh_ifap->ifa_addr,
			   np->nh_refcount);
	} NH_LIST_END(np) ;
    }

    /* Routes waiting to be exported into OSPF */
    if (ospf.export_queue.forw != &ospf.export_queue) {
	ospf_export_entry *ce = ospf.export_queue.forw;

	(void) fprintf(fp, "\tExport queue:  %d entries\n",
		       ospf.export_queue_size);

	do {
	    const char *actptr;
	    const char *action;
	    rt_entry *rt;

	    if (ospf.export_queue_delete == ce
	      || ospf.export_queue_change == ce) {
		actptr = "     -> ";
	    } else {
		actptr = "\t";
	    }
	    if (ce->new_rt != NULL) {
		rt = ce->new_rt;
		if (ce->old_rt != NULL) {
		    action = "Change";
		} else {
		    action = "Add";
		}
	    } else {
		rt = ce->old_rt;
		action = "Delete";
	    }
	    (void) fprintf(fp, "\t%s%-8s %-15A",
			   actptr,
			   action,
			   rt->rt_dest);
	    if (rt->rt_dest_mask) {
		(void) fprintf(fp, " mask %-15A",
			       rt->rt_dest_mask);
	    }
	    (void) fprintf(fp, "metric %8x tag %A\n",
			   ntohl(ce->metric),
			   ospf_path_tag_dump(inet_autonomous_system, ce->tag));
	} while ((ce = ce->forw) != &ospf.export_queue);

	(void) fprintf(fp, "\n");
    }
}



/**/

/*
 *	OSPF termination
 */
static void
ospf_terminate __PF1(tp, task *)
{
    struct AREA *area;
    struct INTF *intf;
    struct LSDB *db, *dbnext;

#ifdef	PROTO_SNMP
    ospf_init_mib(FALSE);
#endif	/* PROTO_SNMP */

    /* Free up any virtual links that do not have tasks */
    while (intf = ospf.vl) {
	ospf_interface_fini(intf);
    } ;

    area = ospf.area;
    ospf.area = (struct AREA *) 0;
    while(area) {
	struct AREA *next = area->next; 

	ospf_area_terminate(area);
	area = next;
    }

    /* Clear ASE list  (will reevaluate after reconfig) */
    if (GOT_GUTS(&ospf.my_ase_list)) {
	for (db = DB_PTR(&ospf.my_ase_list)[NEXT]; db; db = dbnext) {
	    dbnext = DB_PTR(db)[NEXT];
	    db_free(db);
	}
	DBGUTS_FREE(&ospf.my_ase_list);
    }
    if (GOT_GUTS(&ospf.db_free_list)) {
	for (db = DB_PTR(&ospf.db_free_list)[NEXT]; db; db = dbnext) {
	    dbnext = DB_PTR(db)[NEXT];
	    db_free(db);
	}
	DBGUTS_ALLOC(&ospf.db_free_list);
    }

    /* Re-enable redirects on said interface */
    redirect_enable(RTPROTO_OSPF);

#ifdef	IP_MULTICAST
    if (tp) {
	ospf_multicast_cleanup(tp);
    }
#endif	/* IP_MULTICAST */

    /* Free other memory */
    if (ospf.router_id) {
	sockfree(ospf.router_id);
	ospf.router_id =(sockaddr_un *) 0;
    }
    ospf.vl = (struct INTF *) 0;

    /* Free up rtbit */
    if (IAmASBorderRtr) {
	/* Clean up routing table interface */

	ospf_policy_cleanup(tp);
    }

    /* Finally free up the task */
    if (tp) {
	task_delete(tp);
	ospf.task = (task *) 0;
    }
}


/**/

/*
 *	Cleanup before reconfiguration
 */
static void
ospf_cleanup __PF1(tp, task *)
{
    struct AREA *area;

    AREA_LIST(area) {
	adv_free_list(area->intf_policy);
	area->intf_policy = (adv_entry *) 0;
    } AREA_LIST_END(area) ;
    
    adv_free_list(ospf.import_list);
    ospf.import_list = (adv_entry *) 0;
    adv_free_list(ospf.export_list);
    ospf.export_list = (adv_entry *) 0;

    if (ospf.backbone.intf_policy) {
 	adv_free_list(ospf.backbone.intf_policy);
 	ospf.backbone.intf_policy = (adv_entry *) 0;
    }
    
    if (ospf.backbone.nrcnt) {
 	ospf_freeRangeList(&ospf.backbone);
    }
   
    /* Reconfig currently requires complete shutdown */
    ospf_terminate(tp);
}


/*
 *	Initialize static variables
 */
void
ospf_var_init()
{
    ospf.ospf_admin_stat = OSPF_DISABLED;
    ospf.preference = RTPREF_OSPF;
    ospf.preference_ase = RTPREF_OSPF_ASE;
    ospf.export_metric = OSPF_DEFAULT_METRIC;
    ospf.export_tag = OSPF_DEFAULT_TAG;
    ospf.export_type = OSPF_DEFAULT_TYPE;
    ospf.trace_flags = TR_OSPF_DEFAULT;
    ospf.export_interval = MinASEInterval;
    ospf.export_limit = ASEGenLimit;

#ifdef	IP_MULTICAST
    int_ps_bits[RTPROTO_OSPF] = ospf_if_bits;
#endif	/* IP_MULTICAST */

    if (!ospf.export_queue.forw) {
	ospf.export_queue.forw =
	    ospf.export_queue.back =
		ospf.export_queue_delete =
		    ospf.export_queue_change =
			&ospf.export_queue;
	ospf.export_queue_size = 0;
    }

    if (!ospf.nh_list.nh_forw) {
	ospf.nh_list.nh_forw = ospf.nh_list.nh_back = &ospf.nh_list;
    }

    if (!ospf_router_index) {
	ospf_router_index = task_block_init(sizeof (ospf_config_router));
    }
}


/*
 *	After reconfiguration
 */
static void
ospf_reinit __PF1(tp, task *)
{
    int entries = 0;
    rt_entry *rt;
    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(RTPROTO_OSPF_ASE));

    /* Set autonomous system from gated's default AS */
    /* XXX - What needs to be done if this changes during reinit? */
    ospf.gwp->gw_local_as = ospf.gwp_ase->gw_local_as = inet_autonomous_system;
    
    /* Open the routing table */
    rt_open(tp);

    RT_LIST(rt, rtl, rt_entry) {
	pref_t preference = ospf.preference_ase;

	/* Calculate preference of this route */
	(void) import(rt->rt_dest,
		      ospf.import_list,
		      (adv_entry *) 0,
		      (adv_entry *) 0,
		      &preference,
		      (if_addr *) 0,
		      (void_t) ORT_V(rt));

	if (rt->rt_preference != preference) {
	    /* The preference has changed, change the route */
	    (void) rt_change(rt,
			     rt->rt_metric,
			     rt->rt_tag,
			     preference,
			     0, (sockaddr_un **) 0);
	}
	entries++;
    } RT_LIST_END(rt, rtl, rt_entry) ;

    RTLIST_RESET(rtl);
    
    /* Close the routing table */
    rt_close(tp, (gw_entry *) 0, entries, NULL);

#ifdef	PROTO_SNMP
    /* Update the MIB info */
    o_vintf_get();
#endif	/* PROTO_SNMP */
}


/*
 *	Manage next hop entries for our interfaces
 */
static void
ospf_nh_init __PF1(ifap, if_addr *)
{
    /* Make sure we have a next hop for this guy */

    int nh_type = NH_DIRECT;

    if (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
	ifap->ifa_ospf_nh = (void_t) add_nh_entry(ifap,
						  sock2ip(ifap->ifa_addr),
						  nh_type);
	nh_type = NH_LOCAL;
    } else if (BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)
	       && BIT_TEST(inet_class_flags(ifap->ifa_addr_local), INET_CLASSF_LOOPBACK)) {
	return;
    }
	
    ifap->ifa_ospf_nh_lcl = (void_t) add_nh_entry(ifap,
						  sock2ip(ifap->ifa_addr_local),
						  nh_type);
}


/*
 *	Manage next hop entries for our interfaces
 */
static void
ospf_nh_fini __PF1(ifap, if_addr *)
{
    if (ifap->ifa_ospf_nh) {
	free_nh_entry((struct NH_BLOCK **) &ifap->ifa_ospf_nh);
    }

    if (ifap->ifa_ospf_nh_lcl) {
	free_nh_entry((struct NH_BLOCK **) &ifap->ifa_ospf_nh_lcl);
    }
}


static void
ospf_ifachange __PF2(tp, task *,
		     ifap, if_addr *)
{
    int action = IFC_NOCHANGE;
    struct INTF *intf = IF_INTF(ifap);

    if (socktype(ifap->ifa_addr) != AF_INET) {
	return;
    }
    
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
	if (intf) {
	    /* We already know about this interface */
	    action = IFC_REFRESH;
	    break;
	}
	/* Fall through */

    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    goto Up;
	}
	break;

    case IFC_DELETE:
	break;

    case IFC_DELETE|IFC_UPDOWN:
	goto Down;

    default:
	/* Something has changed */
	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
	    /* UP <-> DOWN transition */

	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		/* Interface is now UP */

	    Up:
		ospf_nh_init(ifap);

		assert(!intf);

		if (!BIT_TEST(ifap->ifa_state, IFS_LOOPBACK)) {
		    action = IFC_ADD;
		}
	    } else {
		/* Interface is now down */

	    Down:
		ospf_nh_fini(ifap);

		if (intf) {
		    if (BIT_TEST(intf->flags, OSPF_INTFF_ENABLE)) {
			ospf_ifdown(intf);
		    }

		    ospf_interface_terminate(intf->task);
		}
	    }
	} else if (intf) {
	    /* Something about an interface we know about has changed */

	    if (BIT_TEST(ifap->ifa_change, IFC_METRIC)) {
		/* The metric has changed */

		if (!BIT_TEST(intf->flags, OSPF_INTFF_COSTSET)) {
		    intf->cost = ifap->ifa_metric + OSPF_HOP;
		}
		set_rtr_sched(intf->area);
	    }
	    if (BIT_TEST(ifap->ifa_change, IFC_NETMASK)) {
		/* Toggle the interface */
		(*if_trans[INTF_DOWN][intf->state]) (intf);
		(*if_trans[INTF_UP][IDOWN]) (intf);
	    }
	    if (BIT_TEST(ifap->ifa_change, IFC_ADDR)) {
		set_rtr_sched(intf->area);
	    }
	}
	/* BROADCAST change - we don't use the broadcast address */
	/* MTU change - should take effect on output */
	/* SEL change - homey don't do ISO */
	break;
    }

    if (action) {
	/* Check configuration */
	struct AREA *area = (struct AREA *) 0, *ap;
	config_entry **list = (config_entry **) 0;

	AREA_LIST(ap) {
	    config_entry **new_list = config_resolv(ap->intf_policy,
						    ifap,
						    OSPF_CONFIG_MAX);
	    if (new_list) {
		if (list) {
		    trace(TR_OSPF, LOG_WARNING, "ospf_ifachange: interface %A (%s) configured in two areas",
			  ifap->ifa_addr,
			  ifap->ifa_name);
		    config_resolv_free(new_list, OSPF_CONFIG_MAX);
		    goto Ignore;
		}
		list = new_list;
		area = ap;
	    }
	} AREA_LIST_END(ap) ;

	if (list) {
	    /* Interface was configured - create or update configuration */
	    int type = OSPF_CONFIG_MAX;
	    config_entry *cp = list[OSPF_CONFIG_TYPE];
	    int intf_type = cp ? (int) cp->config_data : 0;

	    /* Figure out type */
	    switch (BIT_TEST(ifap->ifa_state, IFS_BROADCAST|IFS_POINTOPOINT)) {
	    case IFS_BROADCAST:
		switch (intf_type) {
		case NONBROADCAST:
		    break;

		default:
#ifdef	IP_MULTICAST
		    intf_type = BROADCAST;
		    break;
#else	/* IP_MULTICAST */
		    trace(TR_OSPF, LOG_WARNING,
			  "ospf_ifachange: system does not support multicast; ignoring interface %A (%s)",
			  ifap->ifa_addr,
			  ifap->ifa_name);
		    goto Ignore;
#endif	/* IP_MULTICAST */
		}
		break;

	    case 0:
		intf_type = NONBROADCAST;
		break;

	    case IFS_POINTOPOINT:
		switch (intf_type) {
		case 0:
		case POINT_TO_POINT:
		    break;

		default:
		    trace(TR_OSPF, LOG_WARNING,
			  "ospf_ifachange: ignoring NON-BROADCAST specification for point-to-point interface %A (%s)",
			  ifap->ifa_addr,
			  ifap->ifa_name);
		    break;
		}
		intf_type = POINT_TO_POINT;
		break;
	    }

	    switch ((int) intf) {
	    default:
		/* See if type has changed */

		if (intf_type == intf->type) {
		    /* Nothing to do */
		    break;
		}
		/* Type has changed, delete interface and create a new one */

		ospf_ifdown(intf);
		ospf_interface_terminate(intf->task);
		/* Fall through */

	    case 0:
		/* New interface, allocate structure */

		intf = ospf_parse_intf_alloc(area, intf_type, ifap);
		break;
	    }

	    /* Default the cost */
	    intf->cost = ifap->ifa_metric + OSPF_HOP;

	    /* Fill in the parameters */
	    while (--type) {
		if (cp = list[type]) {
		    switch (type) {
		    case OSPF_CONFIG_TYPE:
			/* Dealt with above */
			break;

		    case OSPF_CONFIG_COST:
			intf->cost = (u_int) cp->config_data;
			/* Indicate cost was manually configured */
			BIT_SET(intf->flags, OSPF_INTFF_COSTSET);
			break;
		    
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
			switch (intf->type) {
			case BROADCAST:
			case NONBROADCAST:
			    intf->nbr.pri = intf->pri = (u_int) cp->config_data;
			    break;

			case POINT_TO_POINT:
			    trace(TR_OSPF, LOG_INFO,
				  "ospf_ifachange: priority option ignored for point-to-point interface %A (%s)",
				  ifap->ifa_addr,
				  ifap->ifa_name);
			    break;
			}
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
		    
		    case OSPF_CONFIG_POLL:
			intf->poll_timer = (time_t) cp->config_data;
			break;
		    
		    case OSPF_CONFIG_ROUTERS:
		        {
			    ospf_config_router *ocrp = (ospf_config_router *) cp->config_data;
			    struct NBR *nbr_list = intf->nbr.next;
			    struct NBR *nbr;

			    /* We have old list */
			    intf->nbr.next = (struct NBR *) 0;

			    do {
				register u_int32 new_addr = ntohl(ocrp->ocr_router.s_addr);
				
				struct NBR *nbr_last = (struct NBR *) 0;

				if (ifap != if_withdst(sockbuild_in(0, ocrp->ocr_router.s_addr))) {
				    /* Neighbor is not on the same net */

				    continue;
				}

				/* See if we have this neighbor already */
				for (nbr = nbr_list; nbr; nbr_last = nbr, nbr = nbr->next ) {
				    register u_int32 nbr_addr = ntohl(NBR_ADDR(nbr));

				    if (new_addr == nbr_addr) {
					/* Found it */

					break;
				    } else if (new_addr < nbr_addr) {
					/* Won't find it here */

					goto New;
				    }
				}

				if (nbr) {
				    /* Exists, remove from old list */

				    if (nbr_last) {
					nbr_last->next = nbr->next;
				    } else {
					nbr_list = nbr->next;
				    }
				    nbr->next = (struct NBR *) 0;

				    /* Drop the count to compensate for future increase */
				    ospf.nbrcnt--;
				} else {
				    /* New neighbor */

				New:
				    nbr = (struct NBR *) task_mem_calloc(tp, 1, sizeof (struct NBR));

				    nbr->nbr_addr = sockdup(sockbuild_in(0, ocrp->ocr_router.s_addr));
				    nbr->intf = intf;
				}
				/* Add to list for this interface */
				nbr_enq(intf, nbr);

				/* Set the priority */
				nbr->pri = ocrp->ocr_priority;

				/* XXX - What to do if priority has changed? */
			    } while (ocrp = ocrp->ocr_next) ;

			    /* Free the remaining neighbors */
			    while (nbr = nbr_list) {
				/* Bring the neighbor down */
				(*(nbr_trans[KILL_NBR][nbr->state])) (intf, nbr);

				if (nbr->nbr_id) {
				    sockfree(nbr->nbr_id);
				    nbr->nbr_id = (sockaddr_un *) 0;
				}
				if (nbr->nbr_addr) {
				    sockfree(nbr->nbr_addr);
				    nbr->nbr_addr = (sockaddr_un *) 0;
				}
				nbr_list = nbr->next;
				ospf.nbrcnt--;
				FREE(nbr, OMEM_NBR);
			    }
			}
			break;
		    }
		}
	    }
	} else if (intf) {
	    /* Interface is no longer in configuration, delete it */

	    ospf_ifdown(intf);

	    ospf_interface_terminate(intf->task);
	    intf = (struct INTF *) 0;
	}

	if (intf) {
	    /* Verify the configuration */
	    ospf_parse_intf_check(intf);

	    /* Create the control blocks and fire it up */
	    if (BIT_TEST(intf->flags, OSPF_INTFF_ENABLE)) {
		ospf_interface_init(intf, ospf_intf_offset++);
	    }

	    /* Schedule rebuild of router links */
	    set_rtr_sched(intf->area);
	}

    Ignore:
	if (list) {
	    config_resolv_free(list, OSPF_CONFIG_MAX);
	}
    }

    /* Check router id */
    if (!sockaddrcmp(ospf.router_id, inet_routerid)) {
	/* Router ID has changed */

	/* XXX - The proper way to deal with this would be to shut down OSPF */
	/* XXX - then restart with the new router ID */

	trace(TR_OSPF, LOG_WARNING, "ospf_ifachange: router ID changed from %A to %A; terminating OSPF",
	      ospf.router_id,
	      inet_routerid);

	/* Locate and terminate all other interfaces */
	IF_ADDR(ifap) {
	    ospf_nh_fini(ifap);

	    intf = IF_INTF(ifap);
	    if (intf) {
                if (BIT_TEST(intf->flags, OSPF_INTFF_ENABLE)) {
                    ospf_ifdown(intf);
                }
 
	        ospf_interface_terminate(intf->task);
  	    }
        } IF_ADDR_END(ifap);
	ospf_cleanup(tp);

	return;
#ifdef	notdef
	ospf_var_init();
	ospf_init();
#endif	/* notdef */
    }

#ifdef	PROTO_SNMP
    /* Update the MIB info */
    o_intf_get();
#endif	/* PROTO_SNMP */
}


static void
ospf_startup __PF0(void)
{
    struct INTF *intf;
    u_int area_offset = 0;
    int area_index = 0;
    struct AREA *a;

    if (!inet_routerid_entry) {
	trace(TR_ALL, LOG_WARNING, "ospf_startup: Router ID not defined");
	return;
    }
    if (!ospf.router_id) {
	ospf.router_id = sockdup(inet_routerid);
    }

    /* get head of global lists */
    DBGUTS_ALLOC(&ospf.db_free_list);
    DBGUTS_ALLOC(&ospf.my_ase_list);
    

    ospf.task = task_alloc("OSPF", TASKPRI_PROTO + 1);
    ospf.task->task_proto = IPPROTO_OSPF;
    ospf.task->task_rtproto = RTPROTO_OSPF;
    ospf.task->task_data = (void_t) &ospf;
    ospf.task->task_terminate = ospf_terminate;
    ospf.task->task_ifachange = ospf_ifachange;
    ospf.task->task_dump = ospf_dump;
    ospf.task->task_cleanup = ospf_cleanup;
    ospf.task->task_reinit = ospf_reinit;
    ospf.task->task_n_timers = SYS_TIMER_MAX + ospf.acnt;
    ospf.task->task_trace_flags = ospf.trace_flags;

    ospf.task->task_recv = ospf_recv;
    if ((ospf.task->task_socket = task_get_socket(AF_INET, SOCK_RAW, IPPROTO_OSPF)) < 0) {
	task_quit(errno);
    }

    if (!task_create(ospf.task)) {
	task_quit(EINVAL);
    }

    ospf_policy_init(ospf.task);

#ifdef	IP_MULTICAST
    ospf_multicast_init(ospf.task);
#endif	/* IP_MULTICAST */
	
    /* Create the gateway structures */
    if (!ospf.gwp) {
	ospf.gwp = gw_init((gw_entry *) 0,
			   RTPROTO_OSPF,
			   ospf.task,
			   (as_t) 0,
			   (as_t) 0,
			   (time_t) 0,
			   (sockaddr_un *) 0,
			   (flag_t) 0);
    }

    if (!ospf.gwp_ase) {
	ospf.gwp_ase = gw_init((gw_entry *) 0,
			       RTPROTO_OSPF_ASE,
			       ospf.task,
			       (as_t) 0,
			       (as_t) 0,
			       (time_t) 0,
			       (sockaddr_un *) 0,
			       (flag_t) 0);
    }
	
    /* Set the receive buffer size */
    if (task_set_option(ospf.task,
			TASKOPTION_RECVBUF,
			task_maxpacket) < 0) {
	task_quit(errno);
    }

    /* Set the send buffer size */
    if (task_set_option(ospf.task,
			TASKOPTION_SENDBUF,
			task_maxpacket) < 0) {
	task_quit(errno);
    }
    if (task_set_option(ospf.task,
			TASKOPTION_NONBLOCKING,
			(caddr_t) TRUE) < 0) {
	task_quit(errno);
    }

#ifdef	IPTOS_PREC_INTERNETCONTROL
    /* Set Precendence for OSPF packets */
    (void) task_set_option(ospf.task,
			   TASKOPTION_TOS,
			   IPTOS_PREC_INTERNETCONTROL);
#endif	/* IPTOS_PREC_INTERNETCONTROL */

    /* Create global timers */

    /* create lsa generate timers */
    (void) timer_create(ospf.task,	
			SYS_TIMER_INT_LSA,
			"LSAGenInt",
			0,
			LSRefreshTime,
			OFF1,
			tq_IntLsa,
			(void_t) 0);
    if (IAmBorderRtr) {
	(void) timer_create(ospf.task,	
			    SYS_TIMER_SUM_LSA,
			    "LSAGenSum",
			    0,
			    LSRefreshTime,
			    OFF2,
			    tq_SumLsa,
			    (void_t) 0);
    }
	
    /* create age timers */
    (void) timer_create(ospf.task,	
			SYS_TIMER_INT_AGE,
			"LSDBIntAge",
			0,
			DbAgeTime,
			OFF4,
			tq_int_age,
			(void_t) 0);
    (void) timer_create(ospf.task,	
			SYS_TIMER_SUM_AGE,
			"LSDBSumAge",
			0,
			DbAgeTime,
			OFF5,
			tq_sum_age,
			(void_t) 0);
    (void) timer_create(ospf.task,	
			SYS_TIMER_ASE_AGE,
			"LSDBAseAge",
			0,
			AseAgeTime,
			OFF6,
			tq_ase_age,
			(void_t) 0);

    /* ack timer */
    (void) timer_create(ospf.task,
			SYS_TIMER_ACK,
			"Ack",
			0,
			0,		/* OSPF_T_ACK */
			(time_t) 0,
			tq_ack,
			(void_t) 0);

    /* Setup the areas */
    AREA_LIST(a) {
	struct LSDB_LIST *txq = LLNULL;
    
	(void) timer_create(ospf.task,
			    SYS_TIMER_MAX + area_index++,
			    "Lock",
			    TIMERF_ABSOLUTE,
			    (time_t) MinLSInterval,
			    (time_t) area_offset++,
			    tq_lsa_lock,
			    (void_t) a);

	/* build_rtr and run spf */
	reset_rtr_lock(a);
	a->build_rtr = FALSE;
	a->spfsched |= build_rtr_lsa(a, &txq, 0);

	/* generate default summary for area */
	if (IAmBorderRtr &&
	    BIT_TEST(a->area_flags, OSPF_AREAF_STUB_DEFAULT)) {
	    build_sum_dflt(a);
	}

	ospf_freeq((struct Q **)&txq, OMEM_LL);
    } AREA_LIST_END(a) ;
	
    /* Set timers for virtual links */
    if (IAmBorderRtr && ospf.vcnt) {
	VINTF_LIST(intf) {
	    ospf_interface_init(intf, 0);
	} VINTF_LIST_END(intf) ;
    }

    /* Do the initial run of the SPF for all areas */
    AREA_LIST(a) {
	run_spf(a, 0);
    } AREA_LIST_END(a) ;

    /* Indicate we are active */
    redirect_disable(RTPROTO_OSPF);

#ifdef	PROTO_SNMP
    ospf_init_mib(TRUE);
#endif	/* PROTO_SNMP */
}


/*
 *	OSPF protocol initialization
 */
void
ospf_init __PF0(void)
{

    switch (ospf.ospf_admin_stat) {
    case OSPF_ENABLED:
	/* XXX - If no RouterId, defer startup */
	ospf_startup();
	break;

    case OSPF_DISABLED:
	ospf_cleanup(ospf.task);
	assert(!ospf.task);
	break;

    default:
	assert(FALSE);
    }
}


static int
ospf_adv_dstmatch __PF3(p, void_t,
			dst, sockaddr_un *,
			ps_data, void_t)
{
    adv_entry *adv = (adv_entry *) p;
    register u_int32 ls_tag = ntohl(LS_ASE_TAG((struct LSDB *) ps_data));

    if (BIT_TEST(ls_tag, PATH_OSPF_TAG_TRUSTED)) {
	/* Extract arbitrary data from tag */

	ls_tag = (ls_tag & PATH_OSPF_TAG_USR_MASK) >> PATH_OSPF_TAG_USR_SHIFT;
    }

    if (BIT_TEST(adv->adv_result.res_flag, OSPF_EXPORT_TAG) &&
	OSPF_ADV_TAG(adv) != ls_tag) {
	return FALSE;
    }

    return TRUE;
}


static int
ospf_adv_compare __PF2(p1, void_t,
		       p2, void_t)
{
    register adv_entry *adv1 = (adv_entry *) p1;
    register adv_entry *adv2 = (adv_entry *) p2;

    if (adv1->adv_result.res_flag != adv2->adv_result.res_flag) {
	return FALSE;
    }

    if (BIT_TEST(adv1->adv_result.res_flag, OSPF_EXPORT_TAG)) {
	if (OSPF_ADV_TAG(adv1) != OSPF_ADV_TAG(adv2)) {
	    return FALSE;
	}
    }

    return TRUE;
}


static char *
ospf_adv_print __PF2(p, void_t,
		     first, int)
{
    adv_entry *adv = (adv_entry *) p;
    static char line[LINE_MAX];

    *line = (char) 0;

    if (BIT_TEST(adv->adv_result.res_flag, OSPF_EXPORT_TYPE1|OSPF_EXPORT_TYPE2)) {
	if (BIT_TEST(adv->adv_result.res_flag, OSPF_EXPORT_TYPE1)) {
	    (void) strcat(line, "type 1");
	}

	if (BIT_TEST(adv->adv_result.res_flag, OSPF_EXPORT_TYPE2)) {
	    (void) strcat(line, "type 2");
	}
    } else {
	(void) strcat(line, "type 1,2");
    }

    if (BIT_TEST(adv->adv_result.res_flag, OSPF_EXPORT_TAG)) {
	register u_int32 tag = OSPF_ADV_TAG(adv);

	if (BIT_TEST(tag, PATH_OSPF_TAG_TRUSTED)) {
	    (void) sprintf(line, " tag as %u",
			   (tag & PATH_OSPF_TAG_USR_MASK) >> PATH_OSPF_TAG_USR_SHIFT);
	} else {
	    (void) sprintf(line, "tag %u",
			   tag);
	}
    }

    return line;
}

adv_psfunc ospf_adv_psfunc = {
    0,
    ospf_adv_dstmatch,
    ospf_adv_compare,
    ospf_adv_print,
    0,
} ;


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
