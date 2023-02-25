/* @(#)66	1.4  src/tcpip/usr/sbin/gated/rip.h, tcprouting, tcpip411, GOLD410 12/6/93 17:56:42 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		RIP_MAXSIZE
 *		RIP_TSI_CHANGED
 *		RIP_TSI_HOLDDOWN
 *		RIP_TSI_METRIC
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
 * rip.h,v 1.18.2.1 1993/08/27 22:28:32 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#ifdef	PROTO_RIP
/*
 * Routing Information Protocol
 *  
 * Originally derived from Xerox NS Routing Information Protocol by
 * changing 32-bit net numbers to sockaddr's and padding stuff to 32-bit
 * boundaries.
 */

#define	RIP_VERSION_0	0
#define	RIP_VERSION_1	1
#define	RIP_VERSION_2	2

struct rip_sockaddr {
    u_int16	rip_family;
    u_int16	rip_zero;
    u_int32	rip_addr;
    u_int32	rip_zero1[2];
} ;

struct netinfo {
    u_int16	rip_family;
    u_int16	rip_tag;
    u_int32	rip_dest;
    u_int32	rip_dest_mask;
    u_int32	rip_router;
    u_int32	rip_metric;			/* cost of route */
};

#define RIP_METRIC_UNREACHABLE	16
#define	RIP_METRIC_SHUTDOWN	(RIP_METRIC_UNREACHABLE - 1)

#define	RIP_AUTH_SIZE	16

struct authinfo {
    u_int16	auth_family;			/* RIP_AF_AUTH */
    u_int16	auth_type;
    u_int8	auth_data[RIP_AUTH_SIZE];
};

#define	RIP_AF_UNSPEC	0
#define	RIP_AF_INET	2
#define	RIP_AF_AUTH	0xffff

#define	RIP_AUTH_NONE	0
#define	RIP_AUTH_SIMPLE	2

struct rip {
    /* XXX - using bytes causes alignment problems */
    byte	rip_cmd;		/* request/response */
    byte	rip_vers;		/* protocol version # */
    u_int16	rip_domain;		/* RIP-II */
};

struct entryinfo {
    struct rip_sockaddr rtu_dst;
    struct rip_sockaddr rtu_router;
    u_int16	rtu_flags;
    s_int16	rtu_state;
    s_int32	rtu_timer;
    s_int32		rtu_metric;
    u_int32	rtu_int_flags;
    char rtu_int_name[IFNAMSIZ];
};

/*
 * Packet types.
 */
#define	RIPCMD_REQUEST		1	/* want info */
#define	RIPCMD_RESPONSE		2	/* responding to request */
#define	RIPCMD_TRACEON		3	/* turn tracing on */
#define	RIPCMD_TRACEOFF		4	/* turn it off */
#define	RIPCMD_POLL		5	/* like request, but anyone answers */
#define	RIPCMD_POLLENTRY	6	/* like poll, but for entire entry */
#define	RIPCMD_MAX		7

#ifdef RIPCMDS
static const bits rip_cmd_bits[] = {
    { 0,		"#0" },
    { RIPCMD_REQUEST,	"Request" } ,
    { RIPCMD_RESPONSE,	"Response" },
    { RIPCMD_TRACEON,	"TraceOn" },
    { RIPCMD_TRACEOFF,	"TraceOff" },
    { RIPCMD_POLL,	"Poll" },
    { RIPCMD_POLLENTRY,	"PollEntry" },
    { 0 }
};    
#endif	/* RIPCMDS */

#define RIP_PKTSIZE	512
#define	RIP_MAXSIZE(ifap)	MIN(RIP_PKTSIZE, ifap->ifa_mtu - sizeof (struct udphdr))

/*
 * Timer values used in managing the routing table.
 * Every update forces an entry's timer to be reset.  After
 * EXPIRE_TIME without updates, the entry is marked invalid,
 * but held onto until GARBAGE_TIME so that others may
 * see it "be deleted".
 */
#define	TIMER_RATE		30	/* alarm clocks every 30 seconds */

#define	SUPPLY_INTERVAL		30	/* time to supply tables */

#define	EXPIRE_TIME		180	/* time to mark entry invalid */
#define	GARBAGE_TIME		240	/* time to garbage collect */


#define	RIP_ADDR_MC	0xe0000009	/* 224.0.0.9 */


#define RIP_T_UPDATE	(time_t) 30
#define	RIP_T_HOLDDOWN	(time_t) 120
#define	RIP_T_FLASH	(time_t) (random() % (RIP_T_MAX - RIP_T_MIN) + RIP_T_MIN)
#define	RIP_T_MAX	(time_t) 5
#define	RIP_T_MIN	(time_t) 1
#define	RIP_T_EXPIRE	(time_t) 180

#define RIP_PORT	520
#define	RIP_HOP		1	/* Minimum hop count when passing through */
#define	RIP_LIMIT_METRIC	RIP_HOP, RIP_METRIC_UNREACHABLE	/* For parser */

#define	RIP_CONFIG_NOIN			1
#define	RIP_CONFIG_NOOUT		2
#define	RIP_CONFIG_METRICIN		3
#define	RIP_CONFIG_METRICOUT		4
#define	RIP_CONFIG_FLAG			5
#define	RIP_CONFIG_MAX			6

extern flag_t rip_flags;		/* Option flags */
extern flag_t rip_trace_flags;		/* Trace flags from parser */
extern metric_t rip_default_metric;	/* Default metric to use when propogating */
extern pref_t rip_preference;		/* Preference for RIP routes */
extern int rip_n_trusted;		/* Number of Trusted RIP gateways */
extern int rip_n_source;		/* Number of gateways to receive explicate RIP info */
extern adv_entry *rip_import_list;	/* List of nets to import and not import */
extern adv_entry *rip_export_list;	/* List of nets to export */
extern adv_entry *rip_int_policy;	/* List of interface policy */
extern gw_entry *rip_gw_list;		/* List of RIP gateways */

/* Values for rip_flags */
#define	RIPF_ON		BIT(0x01)	/* RIP is enabled */
#define	RIPF_BROADCAST	BIT(0x02)	/* Broadcast to all interfaces */
#define	RIPF_SOURCE	BIT(0x04)	/* Source packets to our peers */
#define	RIPF_CHOOSE	BIT(0x08)	/* Broadcast if more than one interface */
#define	RIPF_NOCHECK	BIT(0x10)	/* Don't check zero fields */
#define	RIPF_FLASHDUE	BIT(0x20)	/* Flash update is due */
#define	RIPF_NOFLASH	BIT(0x40)	/* Can not do a flash update until after the next normal update */
#define	RIPF_RECONFIG	BIT(0x80)	/* Initial processing or reconfiguration */

#define	RIP_TSIF_CHANGED	0x80		/* This route has been changed */
#define	RIP_TSIF_HOLDDOWN	0x40		/* This route is in holddown */
#define	RIP_TSI_METRICMASK	0x001f		/* RIP Metric */
#define	RIP_TSI_HOLDCOUNT	(RIP_T_HOLDDOWN/RIP_T_UPDATE)	/* Number of updates per holddown */
#define	RIP_TSI_METRIC(tsi)	(tsi & RIP_TSI_METRICMASK)
#define	RIP_TSI_HOLDDOWN(tsi)	BIT_TEST(tsi, RIP_TSIF_HOLDDOWN)
#define	RIP_TSI_CHANGED(tsi)	BIT_TEST(tsi, RIP_TSIF_CHANGED)

#define	RIPTF_POLL		TARGETF_USER1	/* Target has been polled */
#define	RIPTF_V2MC		TARGETF_USER2	/* Use v2 MC for this target */
#define	RIPTF_V2BC		TARGETF_USER3	/* Use v1 compatible v2 features */
#define	RIPTF_V2		(RIPTF_V2MC|RIPTF_V2BC)
#define	RIPTF_MCSET		TARGETF_USER4	/* MC has been enabled on this interface */

#define	RIP_IFPS_V2MC	IFPS_POLICY1	/* Should send V2 MC packets */
#define	RIP_IFPS_V2BC	IFPS_POLICY2	/* Should send V1 compatible V2 BC packets */
#define	RIP_IFPS_V2	(RIP_IFPS_V2MC|RIP_IFPS_V2BC)
#define	RIP_IFPS_V1	0

#define	RIP_IFPS_NOMC	IFPS_KEEP1	/* Unable to enable MC on this IF */

PROTOTYPE(rip_init,
	  extern void,
	  (void));
PROTOTYPE(rip_var_init,
	  extern void,
	  (void));
#if	defined(PROTO_SNMP) && defined(MIB_RIP)

u_int rip_global_changes;
u_int rip_global_responses;

PROTOTYPE(rip_init_mib,
	  extern void,
	  (int));
#endif	/* defined(PROTO_SNMP) && defined(MIB_RIP) */

#endif	/* PROTO_RIP */


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
