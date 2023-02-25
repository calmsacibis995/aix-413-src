static char sccsid[] = "@(#)18	1.1  src/tcpip/usr/sbin/gated/rt_redirect.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:18";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF2
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
 *  rt_redirect.c,v 1.21 1993/04/10 12:25:39 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_RT_VAR
#include "include.h"
#include "inet.h"
#include "krt.h"


/*
 * redirect() changes the routing tables in response to a redirect
 * message or indication from the kernel
 */

flag_t redirect_trace_flags = 0;	/* Trace flags from parser */
int redirect_n_trusted = 0;		/* Number of trusted ICMP gateways */
pref_t redirect_preference = RTPREF_REDIRECT;	/* Preference for ICMP redirects */
gw_entry *redirect_gw_list = 0;		/* Active ICMP gateways */
adv_entry *redirect_import_list = 0;	/* List of nets to import from ICMP */
adv_entry *redirect_int_policy = 0;	/* List of interface policy */


static task *redirect_task;
static flag_t redirect_ignore;
static flag_t redirect_ignore_save;
static rt_parms redirect_rtparms = RTPARMS_INIT(1,
					 (metric_t) 0,
					 (flag_t) 0,
					 (pref_t) 0);

void
redirect __PF6(tp, task *,
	       dst, sockaddr_un *,
	       mask, sockaddr_un *,
	       gateway, sockaddr_un *,
	       src, sockaddr_un *,
	       host_redirect, int)
{
    int saveinstall = krt_install;
    rt_entry *rt, *ort = (rt_entry *) 0;
    int interior = 0;
    register if_addr *ifap;
    const char *redirect_type;

    /* XXX - How about installing all ICMP routes, then delete the ones we don't want */
    /* XXX - This will remove need for special interface to kernel delete routines */
    /* XXX - Maybe a flag to indicate that a delete failure is OK */

    rt_open(tp);

    if (mask) {
	/* Apply the given mask to the destination */
	sockmask(dst, mask);
    }

    redirect_rtparms.rtp_dest = dst;
    redirect_rtparms.rtp_dest_mask = mask;
    redirect_rtparms.rtp_router = gateway;
    redirect_rtparms.rtp_preference = redirect_preference;
    redirect_rtparms.rtp_state = RTS_GATEWAY|RTS_NOADVISE|RTS_NETROUTE;

    if (host_redirect) {
	redirect_type = "host";
    } else {
	redirect_type = "net";
    }

    tracef("REDIRECT: %s redirect from %A: %A/%A via %A: ",
	   redirect_type,
	   src,
	   redirect_rtparms.rtp_dest,
	   redirect_rtparms.rtp_dest_mask,
	   redirect_rtparms.rtp_router);

    /* check gateway directly reachable */
    if (!if_withdst(redirect_rtparms.rtp_router)) {
	trace(TR_RT, 0, "source gateway not on same net");
	goto do_log;
    }

    ort = rt_lookup(redirect_rtparms.rtp_state,
		    RTS_DELETE|RTS_HIDDEN,
		    redirect_rtparms.rtp_dest,
		    RTPROTO_BIT_ANY);
    if (ort && !sockaddrcmp(src, ort->rt_router)) {
	trace(TR_RT, 0, "not from router in use");
	goto do_log;
    }

    if (if_withaddr(redirect_rtparms.rtp_router, FALSE)) {
	trace(TR_RT, 0, "redirect to myself");
	goto do_log;
    }

    if (ort && ort->rt_gwp->gw_proto == RTPROTO_DIRECT) {
	trace(TR_RT, 0, "interface route");
	goto do_log;
    }

    /* Ignore if we are the source of this packet */
    if (if_withlcladdr(src, FALSE)) {
	trace(TR_RT, 0, "redirect from myself");
	goto do_log;
    }

    krt_install = FALSE;			/* route already in kernel */

    if (redirect_ignore) {
	trace(TR_RT, 0, "redirects not allowed");
	goto Delete;
    }
    if (!host_redirect && if_withsubnet(redirect_rtparms.rtp_dest)) {
	interior++;
    }
    trace(TR_RT, 0, NULL);

    redirect_rtparms.rtp_gwp = gw_timestamp(&redirect_gw_list,
					    RTPROTO_REDIRECT,
					    redirect_task,
					    (as_t) 0,
					    (as_t) 0,
					    RT_T_EXPIRE,
					    src,
					    (flag_t) 0);

    /* If we have a list of trusted gateways, verify that this gateway is trusted */
    if (redirect_n_trusted && !BIT_TEST(redirect_rtparms.rtp_gwp->gw_flags, GWF_TRUSTED)) {
	BIT_SET(redirect_rtparms.rtp_gwp->gw_flags, GWF_REJECT);
	trace(TR_RT, 0, "not on trustedgateways list");
	goto Delete;
    }

    ifap = if_withdst(redirect_rtparms.rtp_router);
    if (!ifap) {
	trace(TR_ALL, LOG_ERR, "can not find interface for gateway");
	goto Delete;
    }

    if (!import(redirect_rtparms.rtp_dest,
		redirect_import_list,
		ifap->ifa_ps[RTPROTO_REDIRECT].ips_import,
		redirect_rtparms.rtp_gwp ? redirect_rtparms.rtp_gwp->gw_import : (adv_entry *) 0,
		&redirect_rtparms.rtp_preference,
		ifap,
		(void_t) 0)) {
	trace(TR_RT, 0, "not valid");
	BIT_SET(redirect_rtparms.rtp_gwp->gw_flags, GWF_IMPORT);
	goto Delete;
    }

    /* Invalidate any prior redirects to this destination/mask */
    rt = rt_locate(redirect_rtparms.rtp_state,
		   redirect_rtparms.rtp_dest,
		   redirect_rtparms.rtp_dest_mask,
		   RTPROTO_BIT(RTPROTO_REDIRECT));
    if (rt) {
	register rt_entry *rt1;

	RT_ALLRT(rt1, rt->rt_head) {
	    if (rt1->rt_gwp->gw_proto == RTPROTO_REDIRECT &&
		!BIT_TEST(rt1->rt_state, RTS_DELETE)) {
		rt_delete(rt1);
	    }
	} RT_ALLRT_END(rt1, rt->rt_head) ;
    }

    if (!host_redirect) {
	/* Reset the extra bit */
	BIT_RESET(redirect_rtparms.rtp_state, interior ? RTS_EXTERIOR : RTS_INTERIOR);
    }
    rt = rt_add(&redirect_rtparms);
    if (rt) {
	if (rt != rt->rt_active) {
	    rt_delete(rt);
	    goto Delete;
	}
	krt_installed(rt);
    } else {
	trace(TR_RT, 0, "redirect: error from rt_add");
	goto Delete;
    }
    goto do_log;

  Delete:
    /*
     *  Delete the entry from the kernel
     */
    if (ort && !sockaddrcmp(redirect_rtparms.rtp_dest, ort->rt_dest)) {
	/* Redirect is not from route in use */
	ort = (rt_entry *) 0;
    }

    krt_install = saveinstall;
    (void) krt_delete_dst(tp,
			  ort,
			  &redirect_rtparms,
			  src,
			  RTPROTO_REDIRECT,
			  &redirect_gw_list);
    krt_install = FALSE;

  do_log:
    rt_close(tp, redirect_rtparms.rtp_gwp, 0, NULL);
    krt_install = saveinstall;
    return;
}


/* Initialize variables to their defaults */
void
redirect_var_init __PF0(void)
{
    redirect_preference = RTPREF_REDIRECT;
    redirect_ignore = (flag_t) 0;
}


/* Delete all redirects in the routing table */
static void
redirect_delete __PF1(tp, task *)
{
    int changes = 0;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(RTPROTO_REDIRECT));

    rt_open(tp);
    
    RT_LIST(rt, rtl, rt_entry) {
	if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
	    rt_delete(rt);
	    changes++;
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;

    RTLIST_RESET(rtl);

    rt_close(tp, (gw_entry *) 0, changes, NULL);
}


/* Gated shutting down, clean up and exit */
static void
redirect_terminate __PF1(tp, task *)
{
    if (!redirect_ignore) {
	/* Delete any redirects in table */

	redirect_delete(tp);
    }

    task_delete(tp);
}


/* Get ready for protocol processing */
static void
redirect_reinit __PF1(tp, task *)
{
    if (!redirect_ignore_save && redirect_ignore) {
	/* Redirects were enabled but are now disabled */

	redirect_delete(tp);
    } else {
	/* Redirects are (still) enabled.  Re-evaluate preference */
	/* and make sure we don't have any we are not supposed to */

	int entries = 0;
	rt_entry *rt = (rt_entry *) 0;
	rt_list *rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(RTPROTO_REDIRECT));

	rt_open(tp);
    
	RT_LIST(rt, rtl, rt_entry) {
	    if (!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		pref_t preference = redirect_preference;

		if (BIT_TEST(rt->rt_ifap->ifa_ps[RTPROTO_REDIRECT].ips_state, IFPS_NOIN) ||
		    !import(rt->rt_dest,
			    redirect_import_list,
			    rt->rt_ifap->ifa_ps[RTPROTO_REDIRECT].ips_import,
			    rt->rt_gwp->gw_import,
			    &preference,
			    rt->rt_ifap,
			    (void_t) 0)) {
		    /* Get rid of this route */
		    rt_delete(rt);
		} else {
		    if (rt->rt_preference != preference) {
			/* The preference has changed, change the route */
			(void) rt_change(rt,
					 rt->rt_metric,
					 rt->rt_tag,
					 preference,
					 0, (sockaddr_un **) 0);
		    }
		    entries++;
		}
	    }
	} RT_LIST_END(rt, rtl, rt_entry) ;

	RTLIST_RESET(rtl);

	rt_close(tp, (gw_entry *) 0, entries, NULL);
    }

    redirect_ignore_save = (flag_t) 0;
}


/* Cleanup before re-reading configuratino file */
static void
redirect_cleanup __PF1(tp, task *)
{
    /* Save current state */
    redirect_ignore_save = redirect_ignore;

    /* Free policy structures */
    adv_cleanup(
		RTPROTO_REDIRECT,
		&redirect_n_trusted,
		(int *) 0,
		redirect_gw_list,
		&redirect_int_policy,
		&redirect_import_list,
		(adv_entry **) 0);

}


/* Called by protocols to disable redirects */
void
redirect_disable __PF1(proto, proto_t)
{
    if (!redirect_ignore) {
	/* Redirects were enabled */

	redirect_delete(redirect_task);
    }

    BIT_SET(redirect_ignore, RTPROTO_BIT(proto));
}


/* Called by protocols to reenabled redirects */
void
redirect_enable __PF1(proto, proto_t)
{
    BIT_RESET(redirect_ignore, RTPROTO_BIT(proto));
}


/*
 *	Deal with interface policy
 */
static void
redirect_control_reset __PF2(tp, task *,
			     ifap, if_addr *)
{
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];
    
    BIT_RESET(ips->ips_state, IFPS_RESET);
}


static void
redirect_control_set __PF2(tp, task *,
			   ifap, if_addr *)
{
    struct ifa_ps *ips = &ifap->ifa_ps[tp->task_rtproto];
    config_entry **list = config_resolv(redirect_int_policy,
					ifap,
					REDIRECT_CONFIG_MAX);

    /* Init */
    redirect_control_reset(tp, ifap);

    if (list) {
	int type = IF_CONFIG_MAX;
	config_entry *cp;

	/* Fill in the parameters */
	while (--type) {
	    if (cp = list[type]) {
		switch (type) {
		case REDIRECT_CONFIG_NOIN:
		    BIT_SET(ips->ips_state, IFPS_NOIN);
		    break;
		}
	    }
	}

	config_resolv_free(list, REDIRECT_CONFIG_MAX);
    }
}


/* Clean up when an interface changes */
static void
redirect_ifachange __PF2(tp, task *,
			 ifap, if_addr *)
{
    int changes = 0;
    rt_entry *rt = (rt_entry *) 0;
    rt_list *rtl = (rt_list *) 0;

    rt_open(tp);
    
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	    redirect_control_set(tp, ifap);
	}
	/* Don't believe the interface is up until we see packets from it */
	break;

    case IFC_DELETE:
	break;

    case IFC_DELETE|IFC_UPDOWN:
    Delete:
        {
	    rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(RTPROTO_REDIRECT));

	    RT_LIST(rt, rtl, rt_entry) {
		if (rt->rt_ifap == ifap &&
		    !BIT_TEST(rt->rt_state, RTS_DELETE)) {
		    /* Delete any redirects we learned via this interface */

		    rt_delete(rt);
		    changes++;
		}
	    } RT_LIST_END(rt, rtl, rt_entry) ;
	    redirect_control_reset(tp, ifap);
	}
	break;

    default:
	/* Something has changed */
	if (BIT_TEST(ifap->ifa_change, IFC_NETMASK)) {
	    /* The netmask has changed, remove any routes to gateways we can no longer talk to */

	    rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(RTPROTO_REDIRECT));

	    RT_LIST(rt, rtl, rt_entry) {
		if (rt->rt_ifap == ifap &&
		    ifap != if_withdstaddr(rt->rt_router) &&
		    !BIT_TEST(rt->rt_state, RTS_DELETE)) {
		    /* Delete any redirect whose nexthop gateway */
		    /* is no longer on this interface */

		    rt_delete(rt);
		    changes++;
		}
	    } RT_LIST_END(rt, rtl, rt_entry) ;

	}
	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {

	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		/* Transition to up */

		redirect_control_set(tp, ifap);
	    } else {
		/* Transition to down */

		goto Delete;
	    }
	}

	/* BROADCAST - we don't care about */
	/* METRIC - we don't care about */
	/* ADDR - we don't care about */
	/* MTU - we don't care about */
	/* SEL - we don't care aboute */
	break;
    }

    if (rtl) {
	RTLIST_RESET(rtl);
    }
    
    rt_close(tp, (gw_entry *) 0, changes, NULL);
}


static void
redirect_int_dump __PF2(fd, FILE *,
		     list, config_entry *)
{
    register config_entry *cp;

    CONFIG_LIST(cp, list) {
	switch (cp->config_type) {
	case REDIRECT_CONFIG_NOIN:
	    (void) fprintf(fd, " noredirects");
	    break;

	default:
	    assert(FALSE);
	    break;
	}
    } CONFIG_LIST_END(cp, list) ;
}


static void
redirect_dump __PF2(tp, task *,
		    fp, FILE *)
{
    (void) fprintf(fp, "\tRedirects: %s",
		   redirect_ignore ? "off" : "on");
    (void) fprintf(fp, "\tPreference: %d",
		   redirect_preference);

    if (redirect_gw_list) {
	(void) fprintf(fp, "\tGateways providing redirects:\n");
	gw_dump(fp,
		"\t\t",
		redirect_gw_list,
		RTPROTO_REDIRECT);
	(void) fprintf(fp, "\n");
    }
    if (redirect_int_policy) {
	(void) fprintf(fp, "\tInterface policy:\n");
	control_interface_dump(fp, 2, redirect_int_policy, redirect_int_dump);
    }
    control_import_dump(fp, 1, RTPROTO_REDIRECT, redirect_import_list, redirect_gw_list);
    (void) fprintf(fp, "\n\n");
    
}


void
redirect_init __PF0(void)
{
    /* Allocate the routing table task */
    redirect_task = task_alloc("Redirect", TASKPRI_REDIRECT);
    redirect_task->task_cleanup = redirect_cleanup;
    redirect_task->task_reinit = redirect_reinit;
    redirect_task->task_dump = redirect_dump;
    redirect_task->task_terminate = redirect_terminate;
    redirect_task->task_ifachange = redirect_ifachange;
    redirect_task->task_n_timers = RT_TIMER_MAX;
    if (!task_create(redirect_task)) {
	task_quit(EINVAL);
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
