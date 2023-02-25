static char sccsid[] = "@(#)48	1.6  src/tcpip/usr/sbin/gated/icmp.c, tcprouting, tcpip411, GOLD410 12/6/93 17:46:07";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
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
 *  icmp.c,v 1.18 1993/03/22 02:39:09 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/*
 *  Routines for handling ICMP messages
 */

#include "include.h"
#include "inet.h"
#include "icmp.h"
#include "ip_icmp.h"

#if	defined(PROTO_ICMP) && !defined(KRT_RT_SOCK)

task *icmp_task = (task *) 0;

static block_t icmp_block_index;

struct icmptype {
    const char *typename;
    int codes;
    const char *codenames[6];
};

static const struct icmptype icmp_types[ICMP_MAXTYPE + 1] =
{
    {"EchoReply", 0,
     {""}},
    {"", 0,
     {""}},
    {"", 0,
     {""}},
    {"UnReachable", 5,
   {"Network", "Host", "Protocol", "Port", "NeedFrag", "SourceFailure"}},
    {"SourceQuench", 0,
     {""}},
    {"ReDirect", 4,
     {"Network", "Host", "TOSNetwork", "TOSHost"}},
    {"", 0,
     {""}},
    {"", 0,
     {""}},
    {"Echo", 0,
     {""}},
    {"RouterAdvertisement", 0,
     {""}},
    {"", 0,
     {"RouterSolicitation"}},
    {"TimeExceeded", 1,
     {"InTransit", "Reassembly"}},
    {"ParamProblem", 0,
     {""}},
    {"TimeStamp", 0,
     {""}},
    {"TimeStampReply", 0,
     {""}},
    {"InfoRequest", 0,
     {""}},
    {"InfoRequestReply", 0,
     {""}},
    {"MaskRequest", 0,
     {""}},
    {"MaskReply", 0,
     {""}}
};

static const bits icmp_proto_bits[] =
{
#ifdef	IPPROTO_ICMP
    { IPPROTO_ICMP,	"icmp" },
#endif	/* IPPROTO_ICMP */
#ifdef	IPPROTO_IGMP    
    { IPPROTO_IGMP,	"igmp" },
#endif	/* IPPROTO_IGMP */
#ifdef	IPPROTO_GGP    
    { IPPROTO_GGP,	"ggp" },
#endif	/* IPPROTO_GGP */
#ifdef	IPPROTO_TCP    
    { IPPROTO_TCP,	"tcp" },
#endif	/* IPPROTO_TCP */
#ifdef	IPPROTO_EGP    
    { IPPROTO_EGP,	"egp" },
#endif	/* IPPROTO_EGP */
#ifdef	IPPROTO_UDP    
    { IPPROTO_UDP,	"udp" },
#endif	/* IPPROTO_UDP */
#ifdef	IPPROTO_OSPF    
    { IPPROTO_OSPF,	"ospf" },
#endif	/* IPPROTO_OSPF */
    { 0,		NULL }
} ;

struct icmp_list {
    struct icmp_list *forw, *back;
    sockaddr_un *who;
    struct icmp pkt;
};


/*
 * icmp_recv() handles ICMP redirect messages.
 */

static void
icmp_recv __PF1(tp, task *)
{
    int n_packets = TASK_PACKET_LIMIT * 10;
    size_t count;
    register struct icmp_list *il;
    static struct icmp_list head = { &head, &head };

    while (n_packets-- && !task_receive_packet(tp, &count)) {
	register struct icmp_list *ilp;
	const char *type_name, *code_name;
	const struct icmptype *itp;
#ifndef	ICMP_NOIP_HEADER
	struct ip *ip;
#endif	/* ICMP_NOIP_HEADER */
	struct icmp *icmp;

	/* Allocate a block to store this packet */
	il = (struct icmp_list *) task_block_alloc(icmp_block_index);
#ifndef	ICMP_NOIP_HEADER
	task_parse_ip(ip, icmp, struct icmp *);

	count -= sizeof (struct ip);
#endif	/* ICMP_NOIP_HEADER */
	il->who = sockdup(task_recv_srcaddr);

	/* Save the packet */
        bcopy((caddr_t) icmp,
	      (caddr_t) &il->pkt,
	      MIN(count, sizeof (struct icmp)));

        if (il->pkt.icmp_type <= ICMP_MAXTYPE) {
            itp = &icmp_types[il->pkt.icmp_type];
	    type_name = itp->typename;
	    if (il->pkt.icmp_code <= itp->codes) {
	        code_name = itp->codenames[il->pkt.icmp_code];
	    } else {
	        code_name = "Invalid";
	    }
        } else {
	    type_name = "Invalid";
	    code_name = "";
        }
    
        tracef("ICMP RECV %A ",
               il->who);
#ifndef	ICMP_NOIP_HEADER
	tracef("-> %A ",
	       sockbuild_in(0, ip->ip_dst.s_addr));
#endif	/* ICMP_NOIP_HEADER */
        tracef("type %s(%u) code %s(%u)",
	       type_name,
	       il->pkt.icmp_type,
	       code_name,
	       il->pkt.icmp_code);

        if (BIT_TEST(trace_flags, TR_UPDATE)) {
	    int do_mask = FALSE;
	    int do_dest = FALSE;
	    int do_idseq = FALSE;
    
	    switch (il->pkt.icmp_type) {
	    case ICMP_ECHOREPLY:
	    case ICMP_ECHO:
		do_idseq = TRUE;
		break;
    
	    case ICMP_UNREACH:
		/* XXX - Port? */
		do_dest = TRUE;
		break;
    
	    case ICMP_SOURCEQUENCH:
		break;
    
	    case ICMP_REDIRECT:
		tracef(" dest %A via %A",
		       sockbuild_in(0, il->pkt.icmp_ip.ip_dst.s_addr),
		       sockbuild_in(0, il->pkt.icmp_gwaddr.s_addr));
		break;
    
	    case ICMP_TIMXCEED:
	    case ICMP_PARAMPROB:
		do_dest = TRUE;
		break;

	    case ICMP_TSTAMP:
	    case ICMP_TSTAMPREPLY:
		tracef(" originate %T.%u receive %T.%u transmit %T.%u",
		       il->pkt.icmp_otime / 1000,
		       il->pkt.icmp_otime % 1000,
		       il->pkt.icmp_rtime / 1000,
		       il->pkt.icmp_rtime % 1000,
		       il->pkt.icmp_ttime / 1000,
		       il->pkt.icmp_ttime % 1000);
		break;
    
	    case ICMP_IREQ:
	    case ICMP_IREQREPLY:
		do_idseq = do_dest = TRUE;
		break;
    
	    case ICMP_MASKREQ:
	    case ICMP_MASKREPLY:
		do_idseq = do_mask = TRUE;
		break;

	    case ICMP_ROUTERADV:
		tracef(" addresses %d entries per %d lifetime %#T",
		       il->pkt.icmp_addrnum,
		       il->pkt.icmp_addrsiz,
		       ntohs(il->pkt.icmp_lifetime));
		TRACE_UPDATE(TR_ICMP) {
		    int i = il->pkt.icmp_addrnum;
		    
		    while (i--) {
			trace(TR_ICMP, 0, NULL);
			tracef("ICMP RECV	router %A preference %u",
			       sockbuild_in(0, il->pkt.icmp_rdisc[i].ird_addr.s_addr),
			       ntohl(il->pkt.icmp_rdisc[i].ird_pref));
		    }			
		}
		break;

	    case ICMP_ROUTERSOL:
		break;
	    }

    	    if (do_idseq) {
	        tracef(" id %u sequence %u",
		       il->pkt.icmp_id,
		       il->pkt.icmp_seq);
	    }
	    if (do_dest) {
	        tracef(" dest %A protocol %s(%u)",
		       sockbuild_in(0, il->pkt.icmp_ip.ip_dst.s_addr),
		       trace_value(icmp_proto_bits, il->pkt.icmp_ip.ip_p),
		       il->pkt.icmp_ip.ip_p);
	    }
	    if (do_mask) {
	        tracef(" mask %A",
		       sockbuild_in(0, il->pkt.icmp_mask));
	    }
        }

	switch (il->pkt.icmp_type) {
	case ICMP_REDIRECT:
	    /* Check for duplicate */
	    for (ilp = head.forw; ilp != &head; ilp = ilp->forw) {
		if ((sock2ip(il->who) == sock2ip(ilp->who)) &&
		    (il->pkt.icmp_type == ilp->pkt.icmp_type) &&
		    (il->pkt.icmp_code == ilp->pkt.icmp_code) &&
		    (il->pkt.icmp_gwaddr.s_addr == ilp->pkt.icmp_gwaddr.s_addr) &&
		    (il->pkt.icmp_ip.ip_dst.s_addr == ilp->pkt.icmp_ip.ip_dst.s_addr)) {
		    /* Found a duplicate */

		    tracef(": Duplicate");
		    goto free_up;
		}
	    }

#ifdef	PROTO_RDISC
	case ICMP_ROUTERADV:
	case ICMP_ROUTERSOL:
#endif	/* PROTO_RDISC */

	    /* Not a duplicate, add to end of list */
	    insque((struct qelem *) il, (struct qelem *) head.back);
	    break;

	default:
	free_up:
	    sockfree(il->who);
	    task_block_free(icmp_block_index, (void_t) il);
	}

        trace(TR_ICMP, 0, NULL);
    }

    while ((il = head.forw) != &head) {
	/* Process the requests */

	switch (il->pkt.icmp_type) {
	    /* Types to keep are filtered above */
	case ICMP_REDIRECT:
	    {
		int host_redirect = TRUE;
		sockaddr_un *gateway, *dest;
    
		switch (il->pkt.icmp_code) {
		case ICMP_REDIRECT_NET:
		case ICMP_REDIRECT_TOSNET:
#ifndef	HOST_REDIRECTS_ONLY
		    host_redirect = FALSE;
		    /* Fall through */

#endif	/* HOST_REDIRECT_ONLY */	        
		case ICMP_REDIRECT_HOST:
		case ICMP_REDIRECT_TOSHOST:
		    gateway = sockdup(sockbuild_in(0, il->pkt.icmp_gwaddr.s_addr));
		    dest = sockdup(sockbuild_in(0, il->pkt.icmp_ip.ip_dst.s_addr));

		    redirect(icmp_task,
			     dest,
			     host_redirect ? inet_mask_host : inet_mask_natural(dest),
			     gateway,
			     il->who,
			     host_redirect);

		    sockfree(dest);
		    sockfree(gateway);
		}
	    }
	    break;

#ifdef	PROTO_RDISC
	case ICMP_ROUTERADV:
	    rdisc_recv_adv(il->who, il->pkt);
	    break;

	case ICMP_ROUTERSOL:
	    rdisc_recv_sol(il->who, il->pkt);
	    break;
#endif	/* PROTO_RDISC */

	    /* XXX - need we look at other types? */
	}

	/* Free this entry */
	sockfree(il->who);
	remque((struct qelem *) il);
	task_block_free(icmp_block_index, (void_t) il);
    }

    return;
}


/*
 *  Initialize ICMP socket and ICMP task
 */

void
icmp_init __PF0(void)
{

    if (!icmp_task) {
	icmp_task = task_alloc("ICMP", TASKPRI_ICMP);
	icmp_task->task_proto = IPPROTO_ICMP;
	if ((icmp_task->task_socket = task_get_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
	    task_quit(errno);
	}
	icmp_task->task_recv = icmp_recv;
	if (!task_create(icmp_task)) {
	    task_quit(EINVAL);
	}

        if (task_set_option(icmp_task,
			    TASKOPTION_NONBLOCKING,
			    TRUE) < 0) {
	    task_quit(errno);
        }
	task_alloc_recv(icmp_task, ICMP_PACKET_MAX);

	/* Do all we can to avoid losing packets */
	if (task_set_option(icmp_task,
			    TASKOPTION_RECVBUF,
			    task_maxpacket) < 0) {
	    task_quit(errno);
	}

	icmp_block_index = task_block_init(sizeof (struct icmp_list));
    }
}

#endif	/* defined(PROTO_ICMP) && !defined(KRT_RT_SOCK) */


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
