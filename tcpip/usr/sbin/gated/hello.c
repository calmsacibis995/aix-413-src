static char sccsid[] = "@(#)46	1.5  src/tcpip/usr/sbin/gated/hello.c, tcprouting, tcpip411, GOLD410 12/6/93 17:44:49";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF6
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
 *  hello.c,v 1.37.2.3 1993/06/02 23:55:19 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/*
 *	Hello output routines were taken from Mike Petry (petry@trantor.umd.edu)
 *	Also, hello input routines were written by Bill Nesheim, Cornell
 *	CS Dept,  Currently at nesheim@think.com
 */

#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "targets.h"
#include "hello.h"

#define	HELLO_TIMER_UPDATE	0	/* To send updates */
#define	HELLO_TIMER_FLASH	1	/* To maintain update spacing */
#define	HELLO_TIMER_MAX		2	/* Maximum number we will create */

flag_t hello_flags;			/* Options */
flag_t hello_trace_flags;		/* Trace flags from parser */
metric_t hello_default_metric = HELLO_UNREACHABLE;	/* Default metric to use when propogating */
pref_t hello_preference = RTPREF_HELLO;	/* Preference for HELLO routes */

static target *hello_targets;		/* Target list */

int hello_n_trusted;			/* Number of trusted gateways */
int hello_n_source;			/* Number of source gateways */
gw_entry *hello_gw_list;		/* List of HELLO gateways */
adv_entry *hello_import_list;		/* List of nets to import from HELLO */
adv_entry *hello_export_list;		/* LIst of sources to exports routes to HELLO */
adv_entry *hello_int_policy;		/* List of interface policy */


static const bits hello_flag_bits[] = {
{ HELLOF_ON,		"ON" },
{ HELLOF_BROADCAST,	"Broadcast" },
{ HELLOF_SOURCE,	"Source" },
{ HELLOF_CHOOSE,	"Choose" },
{ HELLOF_FLASHDUE,	"FlashDue" },
{ HELLOF_NOFLASH,	"NoFlash" },    
{ HELLOF_RECONFIG,	"ReConfig" },
{ 0 }
} ;

metric_t hop_to_hello[] =
{					/* Translate interface metric to hello metric */
    0,					/* 0 */
    100,				/* 1 */
    148,				/* 2 */
    219,				/* 3 */
    325,				/* 4 */
    481,				/* 5 */
    713,				/* 6 */
    1057,				/* 7 */
    1567,				/* 8 */
    2322,				/* 9 */
    3440,				/* 10 */
    5097,				/* 11 */
    7552,				/* 12 */
    11190,				/* 13 */
    16579,				/* 14 */
    24564,				/* 15 */
    30000				/* 16 */
};


/* Routines for support of the hello window system */

/*
 * initialize the sliding HELLO history window.
 */
static void
hello_win_init __PF2(rtd, rt_data *,
		     tdelay, metric_t)
{
    struct hello_win *hwp = (struct hello_win *) rtd->rtd_data;
    int msf = 0;

    while (msf < HWINSIZE)
	hwp->h_win[msf++] = HELLO_UNREACHABLE;
    hwp->h_index = 0;
    hwp->h_min = tdelay;
    hwp->h_min_ttl = 0;
    hwp->h_win[0] = tdelay;
}


/*
 * add a HELLO derived time delay to the route entries HELLO window.
 */
static void
hello_win_add __PF2(rtd, rt_data *,
		    tdelay, metric_t)
{
    struct hello_win *hwp = (struct hello_win *) rtd->rtd_data;
    int msf, t_index = 0;

    hwp->h_index++;
    if (hwp->h_index >= HWINSIZE)
	hwp->h_index = 0;
    hwp->h_win[hwp->h_index] = tdelay;
    if (tdelay > hwp->h_min)
	hwp->h_min_ttl++;
    else {
	hwp->h_min = tdelay;
	hwp->h_min_ttl = 0;
    }
    if (hwp->h_min_ttl >= HWINSIZE) {
	hwp->h_min = HELLO_UNREACHABLE;
	for (msf = 0; msf < HWINSIZE; msf++)
	    if (hwp->h_win[msf] <= hwp->h_min) {
		hwp->h_min = hwp->h_win[msf];
		t_index = msf;
	    }
	hwp->h_min_ttl = 0;
	if (t_index < hwp->h_index)
	    hwp->h_min_ttl = hwp->h_index - t_index;
	else if (t_index > hwp->h_index)
	    hwp->h_min_ttl = HWINSIZE - (t_index - hwp->h_index);
    }
}


/*
 *	Dump info about a HELLO route
 */
static void
hello_rt_dump __PF2(fd, FILE *,
		    rt, rt_entry *)
{
    int cnt, ind;
    struct hello_win *hwp = (struct hello_win *) rt->rt_data->rtd_data;

    (void) fprintf(fd, "\t\t\tMinimum HELLO time delay in last %d updates: %d\n",
		   HWINSIZE,
		   hwp->h_min);
    (void) fprintf(fd, "\t\t\tLast %d HELLO time delays:\n\t\t",
		   HELLO_REPORT);
    ind = hwp->h_index;
    for (cnt = HELLO_REPORT; cnt; cnt--) {
	(void) fprintf(fd, "%d ",
		       hwp->h_win[ind]);
	if (++ind >= HWINSIZE) {
	    ind = 0;
	}
    }
    (void) fprintf(fd, "\n");
}

/*
 *	Trace a HELLO packet
 */
/*ARGSUSED*/
static void
hello_trace __PF6(comment, const char *,
		  src, sockaddr_un *,
		  dst, sockaddr_un *,
		  packet, void_t,
		  length, size_t,
		  nets, int)
{
    int i;
    const char *cp;
    byte *hello = (byte *) packet;
    byte *end = hello + length;
    struct hm_hdr hm_hdr;
    struct hellohdr hellohdr;
    struct type0pair type0pair;
    struct type1pair type1pair;

    tracef("HELLO %s %A -> %A  %d bytes",
	   comment,
	   src,
	   dst,
	   length);
    if (nets >= 0) {
	tracef("  %d nets", nets);
    }

    /* Calculate the checksum of this packet */
    if (inet_cksum(packet, length)) {
	tracef(" *checksum bad*");
    }
    trace(TR_HELLO, 0, NULL);

    PickUp_hellohdr(hello, hellohdr);

    if (BIT_TEST(trace_flags, TR_UPDATE)) {
	switch (hellohdr.h_date & H_DATE_BITS) {
	    case H_DATE_LEAPADD:
		cp = "add_leap_second ";
		break;
	    case H_DATE_LEAPDEL:
		cp = "del_leap_second ";
		break;
	    case H_DATE_UNSYNC:
		cp = "unsync ";
		break;
	    default:
		cp = "";
	}
	BIT_RESET(hellohdr.h_date, H_DATE_BITS);
	trace(TR_NOSTAMP | TR_HELLO, 0, "HELLO %s %s%d/%d/%d %02d:%02d:%02d.%03d GMT  tstp %d",
	      comment,
	      cp,
	      (hellohdr.h_date >> H_DATE_MON_SHIFT) & H_DATE_MON_MASK,
	      (hellohdr.h_date >> H_DATE_DAY_SHIFT) & H_DATE_DAY_MASK,
	      ((hellohdr.h_date >> H_DATE_YEAR_SHIFT) & H_DATE_YEAR_MASK) + H_DATE_YEAR_BASE,
	      hellohdr.h_time / (60 * 60 * 1000),
	      (hellohdr.h_time / (60 * 1000)) % 60,
	      (hellohdr.h_time / (1000)) % 60,
	      hellohdr.h_time % 1000,
	      hellohdr.h_tstp);

	while (hello < end) {
	    PickUp_hm_hdr(hello, hm_hdr);
	    trace(TR_NOSTAMP | TR_HELLO, 0, "%s\ttype %d  count %d", comment, hm_hdr.hm_type, hm_hdr.hm_count);
	    for (i = 0; i < hm_hdr.hm_count; i++) {
		switch (hm_hdr.hm_type) {
		    case 0:
			PickUp_type0pair(hello, type0pair);
			trace(TR_NOSTAMP | TR_HELLO, 0, "%s\t\tdelay %d  offset %d",
			      comment,
			      type0pair.d0_delay,
			      type0pair.d0_offset);
			break;
		    case 1:
			PickUp_type1pair(hello, type1pair);
			trace(TR_NOSTAMP | TR_HELLO, 0, "%s\t\t%-15A  delay %5d  offset %d",
			      comment,
			      sockbuild_in(0, type1pair.d1_dst),
			      type1pair.d1_delay,
			      type1pair.d1_offset);
			break;
		    default:
			trace(TR_NOSTAMP | TR_HELLO, 0, "%s\t\tInvalid type - giving up!", comment);
			return;
		}
	    }
	}
	trace(TR_HELLO, 0, NULL);
    }
}


/*
 * hello_send():
 * 	Fill in the hello header and checksum, then send the packet.
 */
static void
hello_send __PF2(tp, task *,
		 tlp, target *)
{
    byte *hello;
    struct hellohdr hellohdr;
    struct hm_hdr hm_hdr;
    struct tm *gmt;
    struct timeval timep;
    int error = FALSE;
    size_t length = (byte *) tlp->target_fillp - (byte *) tlp->target_packet;

    /*
     * All packets in an update must be the same size, padding should be used
     * if necessary to insure this.  This is for timing that we don't do, but
     * lets be consistent.
     */
    if (length > tlp->target_maxsize) {
	/* Remember the largest packet */
	tlp->target_maxsize = length;
    } else {
	/* Pad this packet out to the size of the largest sent */
	bzero ((caddr_t) tlp->target_fillp, tlp->target_maxsize - length);
	length = tlp->target_maxsize;
    }

    /* Reset the fill pointer for next time */
    tlp->target_fillp = tlp->target_packet;
    
    (void) gettimeofday(&timep, (struct timezone *) 0);
    gmt = (struct tm *) gmtime(&timep.tv_sec);

    /*
     * set the date field in the HELLO header.  Be very careful here as
     * the last two bits (14&15) should be set so the Fuzzware doesn't use
     * this packet to synchronize its Master Clock.  Using bitwise OR's
     * instead of addition just to be safe when dealing with h_date which
     * is an unsigned short.
     */
    hellohdr.h_date = ((gmt->tm_year - H_DATE_YEAR_BASE) & H_DATE_YEAR_MASK) |
	((gmt->tm_mday & H_DATE_DAY_MASK) << H_DATE_DAY_SHIFT) |
	    (((gmt->tm_mon + 1) & H_DATE_MON_MASK) << H_DATE_MON_SHIFT) |
		H_DATE_UNSYNC;
    /*
     * milliseconds since midnight UT of current day
     */
    hellohdr.h_time = (gmt->tm_sec + gmt->tm_min * 60 + gmt->tm_hour * 3600) * 1000 + timep.tv_usec / 1000;

    /*
     * 16 bit field used in rt calculation,  0 for ethernets
     */
    hellohdr.h_tstp = 0;
    hellohdr.h_cksum = 0;
    hello = (byte *) tlp->target_packet;
    PutDown_hellohdr(hello, hellohdr);

    /* Build the hm header */
    hm_hdr.hm_type = 1;
    hm_hdr.hm_count = (length - Size_hellohdr - Size_hm_hdr) / Size_type1pair;

    /* Check for packet size skew */
    assert((hm_hdr.hm_count * Size_type1pair) == (length - Size_hellohdr - Size_hm_hdr));

    PutDown_hm_hdr(hello, hm_hdr);

    /* Calculate the checksum and put it into the packet */
    hellohdr.h_cksum = inet_cksum((void_t) tlp->target_packet, length);
    hello = (byte *) tlp->target_packet;
    PutDown(hello, hellohdr.h_cksum);

    if (task_send_packet(tp,
			 (void_t) tlp->target_packet,
			 length,
			 0,
			 *tlp->target_dst) < 0) {
	error = TRUE;
    }
    if (BIT_TEST(trace_flags, TR_HELLO)) {
	hello_trace(error ? "*NOT* SENT" : "SENT",
		    *tlp->target_src,
		    *tlp->target_dst,
		    tlp->target_packet,
		    length,
		    (int) hm_hdr.hm_count);
    }
    return;
}


/*
 *	Process an incomming HELLO packet
 */
static void
hello_recv __PF1(tp, task *)
{
    int n_packets = TASK_PACKET_LIMIT;
    size_t count;

    while (n_packets-- && !task_receive_packet(tp, &count)) {
	int i;
	struct ip *ip;
	register byte *hello;
	byte *end;
	size_t hello_len;
	struct hm_hdr hm_hdr;
	if_addr *ifap, *ifapc;
	rt_parms rtparms;

	task_parse_ip(ip, hello, byte *);

	hello_len = ip->ip_len;
	end = hello + hello_len;

	if (BIT_TEST(trace_flags, TR_HELLO)) {
	    hello_trace("RECV",
			task_recv_srcaddr,
			sockbuild_in(0, ip->ip_dst.s_addr),
			(void_t) hello,
			hello_len,
			-1);
	}

	bzero((caddr_t) &rtparms, sizeof (rtparms));
	rtparms.rtp_n_gw = 1;
	rtparms.rtp_router = task_recv_srcaddr;
	rtparms.rtp_gwp = gw_timestamp(&hello_gw_list,
				       tp->task_rtproto,
				       tp,
				       (as_t) 0,
				       (as_t) 0,
				       HELLO_T_EXPIRE,
				       rtparms.rtp_router,
				       GWF_NEEDHOLD);

	rtparms.rtp_state = (flag_t) 0;

	/* If we have a list of trusted gateways, verify that this gateway is trusted */
	if (hello_n_trusted && !BIT_TEST(rtparms.rtp_gwp->gw_flags, GWF_TRUSTED)) {
	    BIT_SET(rtparms.rtp_gwp->gw_flags, GWF_REJECT);
	    continue;
	}

	/* Do we share a net with the sender? */
	if (!(ifap = if_withdst(rtparms.rtp_router))) {
	    trace(TR_EXT, LOG_WARNING, "hello_recv: gw %A no shared net?",
		  rtparms.rtp_router);
	    BIT_SET(rtparms.rtp_gwp->gw_flags, GWF_REJECT);
	    continue;
	}

	/* If this packet is addressed from us, flag the interface as up and ignore the packet */
	ifapc = if_withlcladdr(rtparms.rtp_router, FALSE);
	if (ifapc) {
	    if_rtupdate(ifapc);
	    continue;
	}

	/* update the interface timer on interface the packet came in on. */
	if_rtupdate(ifap);

	/* Ignore this packet */
	if (BIT_TEST(ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN)) {
	    continue;
	}

	/* check the hello checksum */
	if (inet_cksum((void_t) hello, hello_len)) {
	    trace(TR_HELLO, LOG_WARNING, "hello_recv: bad HELLO checksum from %A",
		  rtparms.rtp_router);
	    BIT_SET(rtparms.rtp_gwp->gw_flags, GWF_CHECKSUM);
	    continue;
	}

	rt_open(tp);

	/* message is made up of one or more sub messages */
	hello += Size_hellohdr;
	while (hello < end) {
	    PickUp_hm_hdr(hello, hm_hdr);
	    switch (hm_hdr.hm_type) {
	    case 0:
		hello += Size_type0pair * hm_hdr.hm_count;
		/* not interested in type 0 messages */
		break;

	    case 1:
		for (i = 0; i < hm_hdr.hm_count; i++) {
		    register rt_entry *rt;
		    struct type1pair type1pair;

		    PickUp_type1pair(hello, type1pair);
		    rtparms.rtp_dest = sockbuild_in(0, type1pair.d1_dst);

		    if (ifap == if_withmyaddr(rtparms.rtp_dest)) {
			/* Ignore route to interface */
			continue;
		    }

		    rtparms.rtp_preference = hello_preference;
		    rtparms.rtp_metric = type1pair.d1_delay;
		
		    /* Force delay to be valid */
		    if (rtparms.rtp_metric > HELLO_UNREACHABLE) {
			rtparms.rtp_metric = HELLO_UNREACHABLE;
		    }
		    /*
		     *	Add the interface metric converted to a HELLO delay.
		     */
		    rtparms.rtp_metric += ifap->ifa_ps[tp->task_rtproto].ips_metric_in;

		    rtparms.rtp_state = RTS_INTERIOR;
		    
		    rtparms.rtp_dest_mask = inet_mask_withif(rtparms.rtp_dest, ifap, &rtparms.rtp_state);

		    rt = rt_locate(rtparms.rtp_state,
				   rtparms.rtp_dest,
				   rtparms.rtp_dest_mask,
				   RTPROTO_BIT(tp->task_rtproto));
		    if (!rt) {
			rt_head *rth;
	    
			/* No route installed.  See if we are announcing another route */
			rt = rt_locate(RTS_NETROUTE,
				       rtparms.rtp_dest,
				       rtparms.rtp_dest_mask,
				       RTPROTO_BIT_ANY);
			if (rt &&
			    (rth = rt->rt_head) &&
			    rth->rth_n_announce &&
			    (rt == rth->rth_active || rt == rth->rth_holddown)) {
			    /* We are announcing this route */
			    register target *tlp;

			    /* HELLO won't announce an active route if one in holddown */
			    /* so check the holddown route first */
			    rt = rth->rth_holddown;
			    if (!rt) {
				rt = rth->rth_active;
			    }

			    TARGET_LIST(tlp, hello_targets) {
				if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY) &&
				    rtbit_isset(rt, tlp->target_rtbit)) {
				    /* We are sending to this target */
				    byte tsi;

				    rttsi_get(rth, tlp->target_rtbit, &tsi);

				    if (HELLO_TSI_HOLDDOWN(tsi) ||
					HELLO_TSI_METRIC(tsi) < rtparms.rtp_metric) {
					/* We are announcing this route from another protocol */

					break;
				    }
				}
			    } TARGET_LIST_END(tlp, hello_targets) ;

			    if (tlp) {
				/* Announced via another protocol, ignore this route */
				continue;
			    }
			}
	    
			/* New route */
			if (rtparms.rtp_metric < HELLO_UNREACHABLE &&
			    import(rtparms.rtp_dest,
				   hello_import_list,
				   ifap->ifa_ps[tp->task_rtproto].ips_import,
				   rtparms.rtp_gwp->gw_import,
				   &rtparms.rtp_preference,
				   ifap,
				   (void_t) 0)) {
			    /* Allocate space for the hysterisis */
			    rtparms.rtp_rtd = rtd_alloc(sizeof(struct hello_win));
			    
			    rtparms.rtp_rtd->rtd_dump = hello_rt_dump;
			    hello_win_init(rtparms.rtp_rtd, rtparms.rtp_metric);

			    /* Add new route */
			    rt = rt_add(&rtparms);
			    if (!rt) {
				rtd_unlink(rtparms.rtp_rtd);
			    }
			} else {
			    BIT_SET(rtparms.rtp_gwp->gw_flags, GWF_IMPORT);
			}
		    } else if (sockaddrcmp_in(rt->rt_router, rtparms.rtp_router)) {
			/* same route */

			if (rtparms.rtp_metric >= HELLO_UNREACHABLE) {
			    /* Now unreachable */
			    if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
				/* Delete route */
				hello_win_add(rt->rt_data, HELLO_UNREACHABLE);
				rt_delete(rt);
			    }
			} else if (BIT_TEST(rt->rt_state, RTS_DELETE) ||
				   (METRIC_DIFF(rt->rt_metric, rtparms.rtp_metric) >= HELLO_HYST(rt->rt_metric))) {
			    /* Better metric */
			    if (rt = rt_change(rt,
					       rtparms.rtp_metric,
					       rtparms.rtp_tag,
					       rt->rt_preference,
					       0, (sockaddr_un **) 0)) {
				hello_win_add(rt->rt_data, (metric_t) rtparms.rtp_metric);
			    }
			} else {
			    /* No change */
			    rt_refresh(rt);
			}
		    } else if (((rtparms.rtp_metric < rt->rt_metric
				 && METRIC_DIFF(rtparms.rtp_metric, rt->rt_metric) >= HELLO_HYST(rt->rt_metric))
				|| (rt->rt_timer > (rt->rt_gwp->gw_timer_max / 2)
				    && rt->rt_metric == rtparms.rtp_metric
				    && !BIT_TEST(rt->rt_state, RTS_REFRESH)))
			       && import(rtparms.rtp_dest,
					 hello_import_list,
					 ifap->ifa_ps[tp->task_rtproto].ips_import,
					 rtparms.rtp_gwp->gw_import,
					 &rtparms.rtp_preference,
					 ifap,
					 (void_t) 0)) {
			/* Better metric or same metric and old route has */
			/* not been refreashed */

			(void) rt_change(rt,
					 rtparms.rtp_metric,
					 rtparms.rtp_tag,
					 rt->rt_preference,
					 1, &rtparms.rtp_router);
		    }
		}			/* for each advertized net */
		break;

	    default:
		trace(TR_INT, LOG_ERR, "hello_recv: invalid type %d", hm_hdr.hm_type);
		BIT_SET(rtparms.rtp_gwp->gw_flags, GWF_FORMAT);
		/* Force end of packet */
		hello = end;
	    } /* switch (mh->hm_type) */
	} 	/* while not end of packet */

	rt_close(tp, rtparms.rtp_gwp, (int) hm_hdr.hm_count, NULL);
    }
}


/* Send HELLO updates to all targets on the list */
/*ARGSUSED*/
static void
hello_supply __PF3(tp, task *,
		   targets, target *,
		   flash_update, int)
{
    byte *packet = task_get_send_buffer(byte *);
    rt_list *rtl = rthlist_active(AF_INET);
    register rt_head *rth;
    target *tlp;

    /* Allocate the packets */
    TARGET_LIST(tlp, targets) {
	if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
	    tlp->target_packet = (void_t) packet;
	    tlp->target_maxsize = 0;	/* Reset maximum packet size */
	    tlp->target_fillp = (void_t) ((byte *) tlp->target_packet + Size_hellohdr + Size_hm_hdr);
	    packet = (byte *) packet + tlp->target_ifap->ifa_mtu;
	}
    } TARGET_LIST_END(tlp, targets) ;

    rt_open(tp);

    RT_LIST(rth, rtl, rt_head) {
	TARGET_LIST(tlp, targets) {
	    u_short tsi;
	    struct type1pair type1pair;
	    rt_entry *release_rt = (rt_entry *) 0;
	    byte *fillp = (byte *) tlp->target_fillp;

	    if (!tlp->target_packet) {
		continue;
	    }
	    
	    rttsi_get(rth, tlp->target_rtbit, (byte *) &tsi);
	    if (!tsi) {
		/* This route is not valid for this target */
		continue;
	    }
	    
	    if (flash_update) {
		if (!HELLO_TSI_CHANGED(tsi)) {
		    /* Route has not changed, do not flash it */
		    continue;
		}
		BIT_RESET(tsi, HELLO_TSIF_CHANGED);
	    } else {
		BIT_RESET(tsi, HELLO_TSIF_CHANGED);
		if (HELLO_TSI_HOLDDOWN(tsi)) {
		    if (!--tsi) {
			/* Holddown is over */

			rt_entry *rt1;
		    
			RT_ALLRT(rt1, rth) {
			    if (rtbit_isset(rt1, tlp->target_rtbit)) {
				/* We were announcing this route */
			    
				release_rt = rt1;
				break;
			    }
			} RT_ALLRT_END(rt1, rth) ;

			/* The flash code will find us a new route to announce */
		    }
		}
	    }

	    /* Update the tsi field */
	    rttsi_set(rth, tlp->target_rtbit, (byte *) &tsi);

	    if ((fillp - (byte *) tlp->target_packet) > (tlp->target_ifap->ifa_mtu - Size_type1pair)) {
		hello_send(tp, tlp);
	    }

	    /* Put this entry in the packet */
	    type1pair.d1_delay = HELLO_TSI_HOLDDOWN(tsi) ? HELLO_UNREACHABLE : HELLO_TSI_METRIC(tsi);
	    type1pair.d1_offset = 0;	/* should be signed clock offset */
	    type1pair.d1_dst = sock2ip(rth->rth_dest);	/* struct copy */
	    PutDown_type1pair(fillp, type1pair);
	    tlp->target_fillp = (void_t) fillp;

	    /* Release this route if necessary */
	    if (release_rt && !rtbit_reset(release_rt, tlp->target_rtbit)) {
		/* There are no more routes for this head */
		goto Released;
	    }

	} TARGET_LIST_END(tlp, targets);

    Released: ;
    } RT_LIST_END(rth, rtl, rt_head);

    RTLIST_RESET(rtl) ;
    
    /* Send any packets with data remaining in them */
    TARGET_LIST(tlp, targets) {
	if (tlp->target_packet) {
	    if (((byte *) tlp->target_fillp - (byte *) tlp->target_packet) > Size_hellohdr + Size_hm_hdr) {
		/* XXX - need to remember the size of the first packet and pad the remaining packets to that size */
		hello_send(tp, tlp);
	    }
	    tlp->target_packet = tlp->target_fillp = (void_t) 0;
	}
    } TARGET_LIST_END(tlp, targets) ;

    rt_close(tp, (gw_entry *) 0, 0, NULL);
}


/*
 *	send HELLO packets
 */
/*ARGSUSED*/
static void
hello_job __PF2(tip, timer *,
		interval, time_t)
{
    task *tp = tip->timer_task;
    
    hello_supply(tp, hello_targets, FALSE);

    /* Indicate that flash updates are possible as soon as the timer fires */
    timer_set(TASK_TIMER(tp, HELLO_TIMER_FLASH), HELLO_T_FLASH, (time_t) 0);
    BIT_RESET(hello_flags, HELLOF_NOFLASH|HELLOF_FLASHDUE);
}


/*
 *	send a flash update packet
 */
/*ARGSUSED*/
static void
hello_do_flash __PF2(tip, timer *,
		     interval, time_t)
{
    task *tp = tip->timer_task;
    
    if (BIT_TEST(hello_flags, HELLOF_FLASHDUE)) {
	/* Do a flash update */

	trace(TR_TASK, 0, "hello_do_flash: Doing flash update for HELLO");

	hello_supply(tp, hello_targets, TRUE);

	trace(TR_TASK, 0, "hello_do_flash: Flash update done");

	/* Indicate no flash update is due */
	BIT_RESET(hello_flags, HELLOF_FLASHDUE);

	/* Schedule the next flash update */
	if (time_sec + HELLO_T_MIN + HELLO_T_MAX < TASK_TIMER(tp, HELLO_TIMER_UPDATE)->timer_next_time) {
	    /* We can squeeze another flash update in before the full update */

	    timer_set(tip, HELLO_T_FLASH, (time_t) 0);
	} else {
	    /* The next flash update will be scheduled after the full update */

	    timer_reset(tip);
	    BIT_SET(hello_flags, HELLOF_NOFLASH);
	}
    } else {
	/* The next flash update can happen immediately */

	timer_reset(tip);
    }
}


/*
 *	Schedule or do a flash update
 */
static void
hello_need_flash __PF1(tp, task *)
{
    timer *tip = TASK_TIMER(tp, HELLO_TIMER_FLASH);

    if (!tip) {
	BIT_RESET(hello_flags, HELLOF_FLASHDUE);
	return;
    }

    /* Indicate we need a flash update */
    BIT_SET(hello_flags, HELLOF_FLASHDUE);

    /* And see if we can do it now */
    if (!tip->timer_next_time
	&& !BIT_TEST(hello_flags, HELLOF_NOFLASH)) {
	/* Do it now */

	hello_do_flash(tip, (time_t) 0);
    }
}


/*
 *	Evaluate policy for changed routes.
 */
static int
hello_policy __PF3(tp, task *,
		   tlp, target *,
		   change_list, rt_list *)
{
    if_addr *ifap = tlp->target_ifap;
    int changes = 0;
    rt_head *rth;

    RT_LIST(rth, change_list, rt_head) {
	register rt_entry *new_rt = rth->rth_active;
	register rt_entry *old_rt;
	adv_results result;

	/* Were we announcing an old route? */
	if (rth->rth_last_active
	    && rtbit_isset(rth->rth_last_active, tlp->target_rtbit)) {
	    /* We were announcing the last active route */
	    
	    old_rt = rth->rth_last_active;
	} else if (rth->rth_holddown
		   && rtbit_isset(rth->rth_holddown, tlp->target_rtbit)) {
	    /* We were doing a holddown on a route */

	    old_rt = rth->rth_holddown;
	} else if (rth->rth_n_announce) {
	    /* If we have a route in holddown, the above might not catch it */
 
	    RT_ALLRT(old_rt, rth) {
		if (rtbit_isset(old_rt, tlp->target_rtbit)) {
		    break;
		}
	    } RT_ALLRT_END(old_rt, rth) ;
	} else {
	    /* No old route */

	    old_rt = (rt_entry *) 0;
	}

	/* See if there is a new route and if it can be announced */
	if (new_rt = rth->rth_active) {

	    if (socktype(new_rt->rt_dest) != AF_INET) {
		goto no_export;
	    }
	    
	    if (BIT_TEST(new_rt->rt_state, RTS_NOADVISE|RTS_GROUP)) {
		/* Absolutely not */
		goto no_export;
	    }

	    if (new_rt->rt_ifap == ifap &&
		new_rt->rt_gwp->gw_proto == RTPROTO_DIRECT) {
		/* Do not send interface routes back to the same interface */
		goto no_export;
	    }

	    /* Subnets and host routes do not go everywhere */
	    if (new_rt->rt_dest_mask == inet_mask_host) {
		if (new_rt->rt_gwp->gw_proto != RTPROTO_DIRECT &&
		    ((sock2ip(ifap->ifa_net) ^ sock2ip(rth->rth_dest)) & sock2ip(ifap->ifa_netmask)) &&
		    !((sock2ip(new_rt->rt_ifap->ifa_net) ^ sock2ip(rth->rth_dest)) & sock2ip(new_rt->rt_ifap->ifa_netmask))) {
		    /* Host is being sent to another network and we learned it via it's home network */
		    /* XXX - This assumes we are announcing the network route */
		    goto no_export;
		}
	    } else if (new_rt->rt_dest_mask != inet_mask_natural(new_rt->rt_dest)) {
		if ((sock2ip(rth->rth_dest) & sock2ip(ifap->ifa_netmask)) != sock2ip(ifap->ifa_net)) {
		    /* Only send subnets to interfaces of the same network */
		    goto no_export;
		}
#ifdef	ROUTE_AGGREGATION
		if (ntohl(sock2ip(rth->rth_dest_mask)) > ntohl(sock2ip(inet_mask_natural(rth->rth_dest)))) {
		    /* Do not sent aggregate routes */

		    break;
		}
#endif	/* ROUTE_AGGREGATION */
		if (rth->rth_dest_mask != ifap->ifa_subnetmask) {
		    /* Only send subnets that have the same mask */
		    goto no_export;
		}
	    } else {
		if (sockaddrcmp_in(rth->rth_dest, ifap->ifa_net)) {
		    /* Do not send the whole net to a subnet */
		    goto no_export;
		}
	    }

	    if ((new_rt->rt_gwp->gw_proto == tp->task_rtproto) && (ifap == new_rt->rt_ifap)) {
		/* Split horizon */
		goto no_export;
	    }

	    if (new_rt->rt_ifap
		&& !BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_UP)) {
		/* The interface is down */
		goto no_export;
	    }

	    /* Assign default metric */
	    if (!new_rt->rt_ifap
		|| BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_LOOPBACK)) {
		/* Routes via the loopback interface must have an explicit metric */ 

		result.res_metric = HELLO_UNREACHABLE;
	    } else if (new_rt->rt_gwp->gw_proto == RTPROTO_DIRECT) {
		/* Interface routes */

		if (BIT_TEST(new_rt->rt_ifap->ifa_state, IFS_POINTOPOINT)) {
		    /* Add a hop for the P2P link */

		    result.res_metric = HELLO_HOP * 2;
		} else {
		    /* Default to one hop */

		    result.res_metric = HELLO_HOP;
		}
	    } else {
		/* Use configured default metric */

		result.res_metric = hello_default_metric;
	    }

		
	    if (!export(new_rt,
			tp->task_rtproto,
			hello_export_list,
			ifap->ifa_ps[tp->task_rtproto].ips_export,
			tlp->target_gwp ? tlp->target_gwp->gw_export : (adv_entry *) 0,
			&result)) {
		/* Policy prohibits announcement */

		goto no_export;
	    } else {
		/* Add the interface metric */

		result.res_metric += ifap->ifa_ps[tp->task_rtproto].ips_metric_out;
	    }

	    if (result.res_metric >= HELLO_UNREACHABLE) {
	    no_export:
		new_rt = (rt_entry *) 0;
	    } else {
		/* Finally, we figured out that we can export it! */

		if (new_rt != old_rt) {
		    /* Not just a metric change, set new announce bit */
		    rtbit_set(new_rt, tlp->target_rtbit);
		    if (old_rt) {
			/* Reset announce bit on old route */
			(void) rtbit_reset(old_rt, tlp->target_rtbit);
		    }
		}
	    }
	}

	/* We've figured out what needs to be changed, now update the TSI */
	if (new_rt) {
	    /* New route or changed metric */
	    u_short tsi;

	    rttsi_get(rth, tlp->target_rtbit, (byte *) &tsi);

	    if (HELLO_TSI_HOLDDOWN(tsi) ||
		HELLO_TSI_METRIC(tsi) != result.res_metric) {
		/* Only flash if metric has changed */
		
		tsi = HELLO_TSIF_CHANGED | result.res_metric;
		rttsi_set(rth, tlp->target_rtbit, (byte *) &tsi);
		changes++;
	    }
	} else if (old_rt) {
	    /* No new route, just an old one */
	    u_short tsi;

	    /* Look at how we were announcing it */
	    rttsi_get(rth, tlp->target_rtbit, (byte *) &tsi);

	    if (tsi && !HELLO_TSI_HOLDDOWN(tsi)) {
		/* Route has been announce and is not already in holddown */
		
		tsi = HELLO_TSIF_CHANGED | HELLO_TSI_HOLDCOUNT;
		rttsi_set(rth, tlp->target_rtbit, (byte *) &tsi);
		changes++;
	    }
	}
    } RT_LIST_END(rth, change_list, rt_head) ;

    return changes;
}


/*
 *	Process changes in the routing table.
 */
static void
hello_flash __PF2(tp, task *,
		  change_list, rt_list *)
{
    int changes = 0;
    target *tlp;

    rt_open(tp);
    
    /* Re-evaluate policy */
    TARGET_LIST(tlp, hello_targets) {
	if (BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
	    changes += hello_policy(tp, tlp, change_list);
	}
    } TARGET_LIST_END(tlp, hello_targets) ;
    
    /* Close the table */
    rt_close(tp, (gw_entry *)0, 0, NULL);

    if (changes) {
	/* Schedule a flash update */

	hello_need_flash(tp);
    }
}


/*
 *	Re-evaluate routing table
 */
static void
hello_newpolicy __PF2(tp, task *,
		    change_list, rt_list *)
{
    /* Indicate reconfig done */
    BIT_RESET(hello_flags, HELLOF_RECONFIG);

    /* And evaluate policy */
    hello_flash(tp, change_list);
}



/*
 *	Initialize static variables
 */
void
hello_var_init __PF0(void)
{
    hello_flags = HELLOF_CHOOSE;
    hello_trace_flags = trace_flags;

    hello_default_metric = HELLO_UNREACHABLE;
    hello_preference = RTPREF_HELLO;
}


/*
 *	Cleanup before re-init
 */
/*ARGSUSED*/
static void
hello_cleanup __PF1(tp, task *)
{
    adv_cleanup(RTPROTO_HELLO,
		&hello_n_trusted,
		&hello_n_source,
		hello_gw_list,
		&hello_int_policy,
		&hello_import_list,
		&hello_export_list);
}


static void
hello_tsi_dump __PF3(rth, rt_head *,
		     data, void_t,
		     buf, char *)
{
    target *tlp = (target *) data;
    u_short tsi;
    const char *changed;
    const char *holddown;

    rttsi_get(rth, tlp->target_rtbit, (byte *) &tsi);

    if (tsi) {
	changed = HELLO_TSI_CHANGED(tsi) ? "changed " : "";
	holddown = HELLO_TSI_HOLDDOWN(tsi) ? "holddown " : "";
    
	(void) sprintf(buf, "HELLO %-15A %s%s %d",
		       *tlp->target_dst,
		       changed,
		       holddown,
		       HELLO_TSI_METRIC(tsi) * (*holddown ? HELLO_T_UPDATE : 1));
    }
    
    return;
}


/*
 *	Update the target list
 */
static void
hello_target_list __PF1(tp, task *)
{
    int targets;
    flag_t target_flags = 0;
    static int n_targets, n_source;

    /* If broadcast/nobroadcast not specified, figure out if we */
    /* need to broadcast packets */
    if (BIT_TEST(hello_flags, HELLOF_CHOOSE)) {
	if (if_n_addr[AF_INET].up > 1
	    && inet_ipforwarding) {

	    BIT_SET(hello_flags, HELLOF_BROADCAST);
	} else {

	    BIT_RESET(hello_flags, HELLOF_BROADCAST);
	}
    }

    if (BIT_TEST(hello_flags, HELLOF_SOURCE|HELLOF_BROADCAST)) {
	/* We are supplying updates */

	/* Gateways do not listen to redirects */
	redirect_disable(tp->task_rtproto);

	/* Make sure the timers are active */
	if (!TASK_TIMER(tp, HELLO_TIMER_UPDATE)) {
	    /* Create the update timer */

	    (void) timer_create(tp,
				HELLO_TIMER_UPDATE,
				"Update",
				0,
				(time_t) HELLO_T_UPDATE,
				(time_t) HELLO_T_MAX,
				hello_job,
				(void_t) 0);
	}

	if (!TASK_TIMER(tp, HELLO_TIMER_FLASH)) {
	    /* Create flash update timer */

	    (void) timer_create(tp,
				HELLO_TIMER_FLASH,
				"Flash",
				TIMERF_ABSOLUTE,
				(time_t) HELLO_T_FLASH,
				(time_t) HELLO_T_MAX,
				hello_do_flash,
				(void_t) 0);
	}
    } else {
	/* We are just listening */

	/* Hosts do listen to redirects */
	redirect_enable(tp->task_rtproto);

	/* Make sure the timers do not exist */
	if (TASK_TIMER(tp, HELLO_TIMER_UPDATE)) {
	    timer_delete(TASK_TIMER(tp, HELLO_TIMER_UPDATE));
	}

	if (TASK_TIMER(tp, HELLO_TIMER_FLASH)) {
	    timer_delete(TASK_TIMER(tp, HELLO_TIMER_FLASH));
	}
    }


    /* Set flags for target list build */
    if (BIT_TEST(hello_flags, HELLOF_BROADCAST)) {
	BIT_SET(target_flags, TARGETF_BROADCAST);
    }
    if (BIT_TEST(hello_flags, HELLOF_SOURCE)) {
	BIT_SET(target_flags, TARGETF_SOURCE);
    }

    /* Build or update target list */
    targets = target_build(tp,
			   &hello_targets,
			   hello_gw_list,
			   target_flags,
			   sizeof (u_short),
			   hello_tsi_dump);

    /* Allocate the send and receive buffers */
    {
	mtu_t mtu_max = 0;
	target *tlp;
	
	TARGET_LIST(tlp, hello_targets) {
	    if (tlp->target_ifap->ifa_mtu > mtu_max) {
		mtu_max = tlp->target_ifap->ifa_mtu;
	    }
	} TARGET_LIST_END(tlp, hello_targets) ;

	task_alloc_send(tp,
			(size_t) (targets * mtu_max));
	task_alloc_recv(tp,
			(size_t) (targets * mtu_max));
    }


    /* Evaluate policy for new targets */
    {
	int changes = 0;
	int opened = 0;
	rt_list *rtl = (rt_list *) 0;
	rt_list *rthl = (rt_list *) 0;
	target *tlp;

	TARGET_LIST(tlp, hello_targets) {
	    if (BIT_TEST(tlp->target_flags, TARGETF_BROADCAST)) {
		if (BIT_TEST(tlp->target_ifap->ifa_ps[tp->task_rtproto].ips_state, IFPS_NOIN)) {
		    register rt_entry *rt;

		    if (!rtl) {
			/* Get a list of all routes we installed */

			rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

			if (!opened) {
			    rt_open(tp);
			    opened++;
			}
		    }

		    RT_LIST(rt, rtl, rt_entry) {
			if (rt->rt_ifap == tlp->target_ifap
			    && !BIT_TEST(rt->rt_state, RTS_DELETE)) {
			    rt_delete(rt);
			    changes++;
			}			    
		    } RT_LIST_END(rt, rtl, rt_entry) ;
		}
	    }
	    switch (BIT_TEST(tlp->target_flags, TARGETF_POLICY|TARGETF_SUPPLY)) {
	    case TARGETF_SUPPLY:
		/* Need to run policy for this target */

		if (!rthl) {
		    /* Get target list */
		    rthl = rthlist_active(AF_INET);

		    if (!opened) {
			rt_open(tp);
			opened++;
		    }
		}

		/* and run policy */
		changes += hello_policy(tp, tlp, rthl);

		/* Indicate policy has been run */
		BIT_SET(tlp->target_flags, TARGETF_POLICY);
		break;

	    case TARGETF_POLICY:
		/* Indicate policy run on this target */

		BIT_RESET(tlp->target_flags, TARGETF_POLICY);
		break;

	    default:
		break;
	    }
	} TARGET_LIST_END(tlp, hello_targets) ;

	if (rtl) {
	    RTLIST_RESET(rtl);
	}
	if (rthl) {
	    RTLIST_RESET(rthl);
	}

	if (opened) {
	    rt_close(tp, (gw_entry *) 0, 0, NULL);
	}
	
	if (changes
	    && !BIT_TEST(hello_flags, HELLOF_RECONFIG)) {
	    hello_need_flash(tp);
	}
    }

    if (targets != n_targets
	|| hello_n_source != n_source) {

	tracef("hello_target_list: ");
	if (targets) {
	    tracef("supplying updates to");
	    if (targets - hello_n_source) {
		tracef(" %d interface%s",
		       targets - hello_n_source,
		       (targets - hello_n_source) > 1 ? "s" : "");		
	    }
	    if (hello_n_source) {
		tracef(" %d gateways",
		       hello_n_source);
	    }
	} else {
	    tracef("just listening");
	}
	n_targets = targets;
	n_source = hello_n_source;
	trace(TR_HELLO, LOG_INFO, NULL);
	trace(TR_HELLO, 0, NULL);
    }
}


/*
 *	Reinit after parse
 */
/*ARGSUSED*/
static void
hello_reinit __PF1(tp, task *)
{
    int entries = 0;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

    /* Open the routing table */
    rt_open(tp);

    RT_LIST(rt, rtl, rt_entry) {
	pref_t preference = hello_preference;

	/* Calculate preference of this route */
	if (import(rt->rt_dest,
		   hello_import_list,
		   rt->rt_ifap->ifa_ps[tp->task_rtproto].ips_import,
		   rt->rt_gwp->gw_import,
		   &preference,
		   rt->rt_ifap,
		   (void_t) 0)) {
	    if (rt->rt_preference != preference) {
		/* The preference has changed, change the route */
		(void) rt_change(rt,
				 rt->rt_metric,
				 rt->rt_tag,
				 preference,
				 0, (sockaddr_un **) 0);
	    }
	    entries++;
	} else {
	    /* This route is now restricted */
	    rt_delete(rt);
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;

    RTLIST_RESET(rtl);
    
    /* Close the routing table */
    rt_close(tp, (gw_entry *) 0, entries, NULL);

    /* Indicate a reconfig in process */
    BIT_SET(hello_flags, HELLOF_RECONFIG);
}


/*
 *	Terminating - clean up
 */
static void
hello_terminate __PF1(tp, task *)
{
    /* Release the target list, bit assignments, and buffers */
    hello_targets = target_free(tp, hello_targets);

    {
	rt_entry *rt = (rt_entry *) 0;
	rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	rt_open(tp);
    
	RT_LIST(rt, rtl, rt_entry) {
	    if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		rt_delete(rt);
	    }
	} RT_LIST_END(rt, rtl, rt_entry) ;

	RTLIST_RESET(rtl);

	rt_close(tp, (gw_entry *) 0, 0, NULL);
    }
    
    task_delete(tp);
}


/*
 *	Deal with interface policy
 */
static void
hello_control_reset __PF2(tp, task *,
			  ifap, if_addr *)
{
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];

    BIT_RESET(ips->ips_state, IFPS_RESET);
    ips->ips_metric_in = hop_to_hello[ifap->ifa_metric] + HELLO_HOP;
    ips->ips_metric_out = (metric_t) 0;
}


static void
hello_control_set __PF2(tp, task *,
			ifap, if_addr *)
{
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];
    config_entry **list = config_resolv(hello_int_policy,
					ifap,
					HELLO_CONFIG_MAX);

    /* Init */
    hello_control_reset(tp, ifap);

    if (list) {
	int type = HELLO_CONFIG_MAX;
	config_entry *cp;

	/* Fill in the parameters */
	while (--type) {
	    if (cp = list[type]) {
		switch (type) {
		case HELLO_CONFIG_NOIN:
		    BIT_SET(ips->ips_state, IFPS_NOIN);
		    break;

		case HELLO_CONFIG_NOOUT:
		    BIT_SET(ips->ips_state, IFPS_NOOUT);
		    break;

		case HELLO_CONFIG_METRICIN:
		    BIT_SET(ips->ips_state, IFPS_METRICIN);
		    ips->ips_metric_in = (metric_t) cp->config_data;
		    break;

		case HELLO_CONFIG_METRICOUT:
		    BIT_SET(ips->ips_state, IFPS_METRICOUT);
		    ips->ips_metric_out = (metric_t) cp->config_data;
		    break;
		}
	    }
	}

	config_resolv_free(list, HELLO_CONFIG_MAX);
    }
}


/*
 *	Deal with an interface status change
 */
static void
hello_ifachange __PF2(tp, task *,
		      ifap, if_addr *)
{
    int changes = 0;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = (rt_list *) 0;

    if (socktype(ifap->ifa_addr) != AF_INET) {
	return;
    }
    
    rt_open(tp);
    
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    hello_control_set(tp, ifap);
	}
	break;

    case IFC_DELETE:
    case IFC_DELETE|IFC_UPDOWN:
    Delete:
	rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	RT_LIST(rt, rtl, rt_entry) {
	    if (rt->rt_gwp->gw_proto == tp->task_rtproto &&
		rt->rt_ifap == ifap &&
		!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		rt_delete(rt);
		changes++;
	    }
	} RT_LIST_END(rt, rtl, rt_entry) ;
	hello_control_reset(tp, ifap);
	break;

    default:
	/* Something has changed */

	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		/* Down to Up transition */

		hello_control_set(tp, ifap);
	    } else {
		/* UP to DOWN transition */

		goto Delete;
	    }
	}
	if (BIT_TEST(ifap->ifa_change, IFC_NETMASK)) {
	    /* The netmask has changed, delete any routes that */
	    /* point at gateways that are no longer reachable */

	    target *tlp;
	    
	    rtl = rtlist_proto(AF_INET, RTPROTO_BIT(tp->task_rtproto));

	    RT_LIST(rt, rtl, rt_entry) {
		if (rt->rt_ifap == ifap
		    && !BIT_TEST(rt->rt_state, RTS_DELETE)
		    && (if_withdstaddr(rt->rt_router) != ifap
			|| BIT_TEST(rt->rt_state, RTS_IFSUBNETMASK))) {
		    /* Interface for this route has changed or we derived the subnet mask */

		    rt_delete(rt);
		    changes++;
		}
	    } RT_LIST_END(rt, rtl, rt_entry) ;

	    TARGET_LIST(tlp, hello_targets) {
		if (tlp->target_ifap == ifap
		    && BIT_TEST(tlp->target_flags, TARGETF_SUPPLY)) {
		    /* Some subnet masks may have been derrived, indicate that policy needs to be rerun */

		    BIT_RESET(tlp->target_flags, TARGETF_POLICY);
		}
	    } TARGET_LIST_END(tlp, hello_targets) ;
	}
	if (BIT_TEST(ifap->ifa_change, IFC_METRIC)) {
	    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];
	    
	    if (!BIT_TEST(ips->ips_state, IFPS_METRICIN)) {
		ips->ips_metric_in = hop_to_hello[ifap->ifa_metric] + HELLO_HOP;
	    }
	}
	/* A METRIC change will take effect in 15 seconds with the next update received */
	/* A BROADCAST change is automatic since we have a pointer to the pointer to */
	/* 	the broadcast address */
	/* An ADDR change will take effect when the peers notice that the */
	/* 	old address is no longer sending */
	/* An MTU change will take effect when we get around to sending packets */
	/* A SEL change is of no interest to us */
	break;
    }

    if (rtl) {
	RTLIST_RESET(rtl);
    }
    
    rt_close(tp, (gw_entry *) 0, changes, NULL);

    /* Update target list */
    hello_target_list(tp);
}


static void
hello_int_dump __PF2(fd, FILE *,
		     list, config_entry *)
{
    register config_entry *cp;

    CONFIG_LIST(cp, list) {
	switch (cp->config_type) {
	case HELLO_CONFIG_NOIN:
	    (void) fprintf(fd, " nohelloin");
	    break;

	case HELLO_CONFIG_NOOUT:
	    (void) fprintf(fd, " nohelloout");
	    break;

	case HELLO_CONFIG_METRICIN:
	    (void) fprintf(fd, " metricin %u",
			   (metric_t) cp->config_data);
	    break;

	case HELLO_CONFIG_METRICOUT:
	    (void) fprintf(fd, " metricout %u",
			   (metric_t) cp->config_data);
	    break;

	default:
	    assert(FALSE);
	    break;
	}
    } CONFIG_LIST_END(cp, list) ;
}

/*
 *	Dump info about HELLO
 */
static void
hello_dump __PF2(tp, task *,
		 fd, FILE *)
{
    (void) fprintf(fd, "\tFlags: %s\tDefault metric: %d\t\tDefault preference: %d\n",
		   trace_bits(hello_flag_bits, hello_flags),
		   hello_default_metric,
		   hello_preference);
    target_dump(fd, hello_targets, (bits *) 0);
    if (hello_gw_list) {
	(void) fprintf(fd, "\tActive gateways:\n");
	gw_dump(fd,
		"\t\t",
		hello_gw_list,
		tp->task_rtproto);
	(void) fprintf(fd, "\n");
    }
    if (hello_int_policy) {
	(void) fprintf(fd, "\tInterface policy:\n");
	control_interface_dump(fd, 2, hello_int_policy, hello_int_dump);
    }
    control_import_dump(fd, 1, RTPROTO_HELLO, hello_import_list, hello_gw_list);
    control_export_dump(fd, 1, RTPROTO_HELLO, hello_export_list, hello_gw_list);
    (void) fprintf(fd, "\n");
}


/*
 *	Initialize HELLO socket and task
 */
/*ARGSUSED*/
void
hello_init __PF0(void)
{
    int hello_socket = 1;
    _PROTOTYPE(flash,
	       void,
	       (task *,
		rt_list *)) = hello_flash;	/* Hack for UTX/32 and Ultrix */
    _PROTOTYPE(newpolicy,
	       void,
	       (task *,
		rt_list *)) = hello_newpolicy;	/* ditto */
    static task *hello_task;


    if (BIT_TEST(hello_flags, HELLOF_ON)) {
	if (!hello_task) {
	    if ((hello_socket = task_get_socket(AF_INET, SOCK_RAW, IPPROTO_HELLO)) < 0) {
		task_quit(errno);
	    }
	    hello_task = task_alloc("HELLO", TASKPRI_PROTO);
	    hello_task->task_trace_flags = hello_trace_flags;
	    hello_task->task_proto = IPPROTO_HELLO;
	    hello_task->task_socket = hello_socket;
	    hello_task->task_rtproto = RTPROTO_HELLO;
	    hello_task->task_recv = hello_recv;
	    hello_task->task_cleanup = hello_cleanup;
	    hello_task->task_reinit = hello_reinit;
	    hello_task->task_dump = hello_dump;
	    hello_task->task_terminate = hello_terminate;
	    hello_task->task_ifachange = hello_ifachange;
	    hello_task->task_flash = flash;
	    hello_task->task_newpolicy = newpolicy;
	    hello_task->task_n_timers = HELLO_TIMER_MAX;

	    if (task_set_option(hello_task,
				TASKOPTION_RECVBUF,
				task_maxpacket) < 0) {
		task_quit(errno);
	    }
	    if (task_set_option(hello_task,
				TASKOPTION_DONTROUTE,
				TRUE) < 0) {
		task_quit(errno);
	    }
	    if (task_set_option(hello_task,
				TASKOPTION_NONBLOCKING,
				(caddr_t) TRUE) < 0) {
		task_quit(errno);
	    }
	    (void) task_set_option(hello_task,
				   TASKOPTION_TTL,
				   1);
	    if (!task_create(hello_task)) {
		task_quit(EINVAL);
	    }

	    if (HELLO_T_UPDATE < rt_timer->timer_interval) {
		timer_interval(rt_timer, (time_t) HELLO_T_UPDATE);
	    }
	} else {
	    hello_task->task_trace_flags = hello_trace_flags;
	}
    } else {
	hello_cleanup((task *) 0);

	if (hello_task) {
	    hello_terminate(hello_task);
	    hello_task = (task *) 0;
	}
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
