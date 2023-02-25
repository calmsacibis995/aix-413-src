/* @(#)47	1.4  src/tcpip/usr/sbin/gated/hello.h, tcprouting, tcpip411, GOLD410 12/6/93 17:45:13 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: HELLO_HYST
 *		HELLO_TARGET_VALID
 *		HELLO_TSI_CHANGED
 *		HELLO_TSI_HOLDDOWN
 *		HELLO_TSI_METRIC
 *		METRIC_DIFF
 *		PROTOTYPE
 *		PickUp_hellohdr
 *		PickUp_hm_hdr
 *		PickUp_type0pair
 *		PickUp_type1pair
 *		PutDown_hellohdr
 *		PutDown_hm_hdr
 *		PutDown_type0pair
 *		PutDown_type1pair
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
 * hello.h,v 1.13.2.1 1993/08/27 22:28:40 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#ifdef	PROTO_HELLO

#ifndef IPPROTO_HELLO
#define	IPPROTO_HELLO		63
#endif	/* IPPROTO_HELLO */
#define	HELLO_UNREACHABLE	30000			/* in ms */
#define	HELLO_HOP		100			/* minimum delay */
#define	HELLO_T_UPDATE		(time_t) 15		/* in seconds */
#define	HELLO_T_HOLDDOWN	(time_t) 120		/* in seconds */
#define	HELLO_T_EXPIRE		(time_t) 180		/* in seconds */
#define	HELLO_T_FLASH	(time_t) (random() % (HELLO_T_MAX - HELLO_T_MIN) + HELLO_T_MIN)
#define	HELLO_T_MAX	(time_t) 5
#define	HELLO_T_MIN	(time_t) 1
#define	HELLO_HYST(s)	(s >> 2)	/* 25% of old route, in ms */

/* For parser */
#define	HELLO_LIMIT_DELAY	HELLO_HOP, HELLO_UNREACHABLE

#define	HELLO_CONFIG_NOIN		1
#define	HELLO_CONFIG_NOOUT		2
#define	HELLO_CONFIG_METRICIN		3
#define	HELLO_CONFIG_METRICOUT		4
#define	HELLO_CONFIG_MAX		5


#define	HELLO_DEFAULT	0		/* net 0 as default */

#define	METRIC_DIFF(x,y)	(x > y ? x - y : y - x)

/* Task specific info */
#define	HELLO_TSIF_CHANGED	0x8000		/* This route has been changed */
#define	HELLO_TSI_METRICMASK	0x7fff		/* HELLO Metric */
#define	HELLO_TSI_HOLDCOUNT	(HELLO_T_HOLDDOWN/HELLO_T_UPDATE)	/* Number of updates per holddown */
#define	HELLO_TSI_METRIC(tsi)	(tsi & HELLO_TSI_METRICMASK)
#define	HELLO_TSI_HOLDDOWN(tsi)	(tsi < HELLO_HOP)
#define	HELLO_TSI_CHANGED(tsi)	BIT_TEST(tsi, HELLO_TSIF_CHANGED)

/*	Define the DCN HELLO protocol packet			*/

struct hellohdr {
    u_int16 h_cksum;			/* Ip checksum of this header and data ares */
    u_int16 h_date;			/* Julian days since 1 January 1972 */
    u_int32 h_time;			/* Local time (milliseconds since midnight UT) */
    u_int16 h_tstp;			/* (used to calculate delay/offset) */
};

#define	Size_hellohdr	10
#define	PickUp_hellohdr(s, hellohdr) \
  PickUp(s, hellohdr.h_cksum); \
  PickUp(s, hellohdr.h_date); GNTOHS(hellohdr.h_date); \
  PickUp(s, hellohdr.h_time); GNTOHL(hellohdr.h_time); \
  PickUp(s, hellohdr.h_tstp); GNTOHS(hellohdr.h_tstp);
#define	PutDown_hellohdr(s, hellohdr) \
  PutDown(s, hellohdr.h_cksum);\
  GHTONS(hellohdr.h_date); PutDown(s, hellohdr.h_date);\
  GHTONL(hellohdr.h_time); PutDown(s, hellohdr.h_time);\
  GHTONS(hellohdr.h_tstp); PutDown(s, hellohdr.h_tstp);

#define	H_DATE_BITS	0xC000		/* Flag bits */
#define	H_DATE_LEAPADD	0x4000		/* Insert leap second at end of current day */
#define	H_DATE_LEAPDEL	0x8000		/* Delete leap second at end of current day */
#define	H_DATE_UNSYNC	0xC000		/* Clock is unsynchronized */

#define	H_DATE_MON_SHIFT	10
#define	H_DATE_MON_MASK		0x0f
#define	H_DATE_DAY_SHIFT	5
#define	H_DATE_DAY_MASK		0x1f
#define	H_DATE_YEAR_SHIFT	0
#define	H_DATE_YEAR_MASK	0x1f
#define	H_DATE_YEAR_BASE	72


struct hm_hdr {
    u_int8 hm_count;			/* Number of elements that follow */
    u_int8 hm_type;			/* Type of elements */
};

#define	Size_hm_hdr	2
#define	PickUp_hm_hdr(s, hm_hdr) \
  PickUp(s, hm_hdr.hm_count);\
  PickUp(s, hm_hdr.hm_type);
#define	PutDown_hm_hdr(s, hm_hdr) \
  PutDown(s, hm_hdr.hm_count);\
  PutDown(s, hm_hdr.hm_type);


struct type0pair {
    u_int16 d0_delay;			/* Delay to peer (milliseconds) */
    u_int16 d0_offset;			/* Clock offset of peer (milliseconds) */
};

#define	Size_type0pair	4
#define	PickUp_type0pair(s, type0pair) \
  PickUp(s, type0pair.d0_delay);  GNTOHS(type0pair.d0_delay);\
  PickUp(s, type0pair.d0_offset); GNTOHS(type0pair.d0_offset);
#define	PutDown_type0pair(s, type0pair) \
  GHTONS(type0pair.d0_delay); PutDown(s, type0pair.d0_delay);\
  GHTONS(type0pair.d0_offset); PutDown(s, type0pair.d0_offset);


struct type1pair {
    u_int32 d1_dst;			/* IP host/network address */
    u_int16 d1_delay;			/* Delay to peer (milliseconds) */
    s_int16 d1_offset;			/* CLock offset of peer (milliseconds) */
};

#define	Size_type1pair	8
#define	PickUp_type1pair(s, type1pair) \
  PickUp(s, type1pair.d1_dst);\
  PickUp(s, type1pair.d1_delay); GNTOHS(type1pair.d1_delay);\
  PickUp(s, type1pair.d1_offset); GNTOHS(type1pair.d1_offset);
#define	PutDown_type1pair(s, type1pair) \
  PutDown(s, type1pair.d1_dst);\
  GHTONS(type1pair.d1_delay); PutDown(s, type1pair.d1_delay);\
  GHTONS(type1pair.d1_offset); PutDown(s, type1pair.d1_offset);


#define	WINDOW_INTERVAL		6	/* in minutes */
#define HELLO_INTERVAL		15	/* HELLO rate coming in, in secs */
#define HWINSIZE		(WINDOW_INTERVAL * (60 / HELLO_INTERVAL))
#define HELLO_REPORT	8		/* how far back we will report */

struct hello_win {
    rt_data h_head;
    metric_t h_win[HWINSIZE];
    int h_index;
    metric_t h_min;
    int h_min_ttl;
};

extern flag_t hello_flags;		/* Options */
extern flag_t hello_trace_flags;	/* Trace flags from parser */
extern metric_t hello_default_metric;	/* Default metric to use when propogating */
extern pref_t hello_preference;		/* Preference for HELLO routes */
extern int hello_n_trusted;		/* Number of trusted gateways */
extern int hello_n_source;		/* Number of gateways to receive explicit HELLO packets */
extern gw_entry *hello_gw_list;		/* List of defined and learned HELLO gateways */
extern adv_entry *hello_import_list;
extern adv_entry *hello_export_list;
extern adv_entry *hello_int_policy;	/* List of interface policy */

/* Values for hello_flags */
#define	HELLOF_ON		BIT(0x01)	/* HELLO is enabled */
#define	HELLOF_BROADCAST	BIT(0x02)	/* Broadcast to all interfaces */
#define	HELLOF_SOURCE		BIT(0x04)	/* Source packets to our peers */
#define	HELLOF_CHOOSE		BIT(0x08)	/* Broadcast if more than one interface */
#define	HELLOF_FLASHDUE		BIT(0x10)	/* A Flash update is due when flash time expires */
#define	HELLOF_NOFLASH		BIT(0x20)	/* No flash update until normal update */
#define	HELLOF_RECONFIG		BIT(0x40)	/* In the process of reconfiguration */

#define	HELLO_TARGET_VALID(tlp)	((BIT_TEST(hello_flags, HELLOF_SOURCE) && BIT_TEST(tlp->flag, TARGETF_SOURCE)) || \
				 (BIT_TEST(hello_flags, HELLOF_BROADCAST) && BIT_TEST(tlp->flag, TARGETF_BROADCAST)))

PROTOTYPE(hello_init, extern void, (void));
PROTOTYPE(hello_var_init, extern void, (void));

#endif	/* PROTO_HELLO */


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
