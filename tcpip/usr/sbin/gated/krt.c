static char sccsid[] = "@(#)56	1.6  src/tcpip/usr/sbin/gated/krt.c, tcprouting, tcpip411, GOLD410 12/6/93 17:51:17";
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
 *  krt.c,v 1.70.2.6 1993/09/28 18:28:24 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* krt.c
 *
 * Kernel routing table interface routines
 */

#define	INCLUDE_IOCTL
#define	INCLUDE_FILE
#define	INCLUDE_KVM
#define	INCLUDE_NETOPT_POWER
#define	INCLUDE_ROUTE
#define	INCLUDE_IF_TYPES
#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"
#include "krt_var.h"
#ifdef	PROTO_SCRAM
#include "scram.h"
#endif	/* PROTO_SCRAM */

/* What flags mean it was a redirect */
#ifdef	RTF_MODIFIED
#define	RTF_REDIRECT	(RTF_DYNAMIC|RTF_MODIFIED)
#else	/* RTF_MODIFIED */
#define	RTF_REDIRECT	RTF_DYNAMIC
#endif	/* RTF_MODIFIED */

int krt_install = TRUE;			/* if TRUE install route in kernel */
int krt_install_parse = TRUE;		/* What command line parse turned up */
char *krt_version_kernel = 0;		/* OS version of the kernel */

task *krt_task = 0;			/* Task for kernel routing table */
gw_entry *krt_gwp = 0;		/* Gateway structure for routes read from the kernel at startup */

gw_entry *krt_gw_list = 0;		/* List of gateways for static routes */

rt_entry krt_rt = { 0 };
static rt_head krt_rth;

#ifndef	KVM_TYPE_NONE
#ifdef	KVM_WITH_KD
kvm_t	*kd;
#else	/* KVM_WITH_KD */
int	kd;
#endif	/* KVM_WITH_KD */
#endif	/* KVM_TYPE_NONE */


const bits krt_flag_bits[] =
{
    {RTF_UP, "UP"},
    {RTF_GATEWAY, "GW"},
    {RTF_HOST, "HOST"},
#ifdef	RTF_DYNAMIC
    {RTF_DYNAMIC, "DYN"},
#endif	/* RTF_DYNAMIC */
#ifdef	RTF_MODIFIED
    {RTF_MODIFIED, "MOD"},
#endif	/* RTF_MODIFIED */
#ifdef	RTF_DONE
    {RTF_DONE, "DONE"},
#endif	/* RTF_DONE */
#ifdef	RTF_MASK
    {RTF_MASK, "MASK"},
#endif	/* RTF_MASK */
#ifdef	RTF_CLONING
    {RTF_CLONING, "CLONING"},
#endif	/* RTF_CLONING */
#ifdef	RTF_XRESOLVE
    {RTF_XRESOLVE, "XRESOLVE"},
#endif	/* RTF_XRESOLVE */
#ifdef	RTF_LLINFO
    {RTF_LLINFO, "LLINFO"},
#endif	/* RTF_LLINFO */
#ifdef	RTF_REJECT
    {RTF_REJECT, "REJECT"},
#endif	/* RTF_REJECT */
#ifdef	RTF_STATIC
    {RTF_STATIC, "STATIC"},
#endif	/* RTF_STATIC */
#ifdef	RTF_BLACKHOLE
    {RTF_BLACKHOLE, "BLACKHOLE"},
#endif	/* RTF_BLACKHOLE */
#ifdef	RTF_PROTO1
    {RTF_PROTO1, "PROTO1"},
    {RTF_PROTO2, "PROTO2"},
#endif	/* RTF_PROTO1 */
    {0}
};


flag_t
krt_flags_to_state __PF1(flags, flag_t)
{
    register flag_t state = 0;

    if (BIT_TEST(flags, RTF_GATEWAY)) {
	BIT_SET(state, RTS_GATEWAY);
    }
#ifdef	RTF_REJECT
    if (BIT_TEST(flags, RTF_REJECT)) {
	BIT_SET(state, RTS_REJECT);
    }
#endif	/* RTF_REJECT */
#ifdef	RTF_STATIC
    if (BIT_TEST(flags, RTF_STATIC)) {
	BIT_SET(state, RTS_STATIC);
    }
#endif	/* RTF_STATIC */
#ifdef	RTF_BLACKHOLE
    if (BIT_TEST(flags, RTF_BLACKHOLE)) {
	BIT_SET(state, RTS_BLACKHOLE);
    }
#endif	/* RTF_BLACKHOLE */

    return state;
}

flag_t
krt_state_to_flags __PF1(state, flag_t)
{
    register flag_t flags = 0;

    if (BIT_TEST(state, RTS_GATEWAY)) {
	BIT_SET(flags, RTF_GATEWAY);
    }
#ifdef	RTF_REJECT
    if (BIT_TEST(state, RTS_REJECT)) {
	BIT_SET(flags, RTF_REJECT);
    }
#endif	/* RTF_REJECT */
#ifdef	RTF_STATIC
    if (BIT_TEST(state, RTS_STATIC)) {
	BIT_SET(flags, RTF_STATIC);
    }
#endif	/* RTF_STATIC */
#ifdef	RTF_BLACKHOLE
    if (BIT_TEST(state, RTS_BLACKHOLE)) {
	BIT_SET(flags, RTF_BLACKHOLE);
    }
#endif	/* RTF_BLACKHOLE */

    return flags;
}


#ifdef	IFT_OTHER
int
krt_type_to_ll __PF1(type, int)
{
    struct if_types {
	int type;
	int ll_type;
    } ;
    static struct if_types tt[] = {
	{ IFT_OTHER,	LL_OTHER },
	{ IFT_1822,	LL_OTHER },
	{ IFT_HDH1822,	LL_OTHER },
	{ IFT_X25DDN,	LL_OTHER },
	{ IFT_X25,	LL_X25 },
	{ IFT_ETHER,	LL_8022 },
	{ IFT_ISO88023,	LL_8022 },
	{ IFT_ISO88024,	LL_8022 },
	{ IFT_ISO88025,	LL_8022 },
	{ IFT_ISO88026,	LL_8022 },
	{ IFT_STARLAN,	LL_8022 },
	{ IFT_P10,	LL_PRONET },
	{ IFT_P80,	LL_PRONET },
	{ IFT_HY,	LL_HYPER },
	{ IFT_FDDI,	LL_8022 },
	{ IFT_LAPB,	LL_OTHER },
	{ IFT_SDLC,	LL_OTHER },
	{ IFT_T1,	LL_OTHER },
	{ IFT_CEPT,	LL_OTHER },
	{ IFT_ISDNBASIC,	LL_OTHER },
	{ IFT_ISDNPRIMARY,	LL_OTHER },
	{ IFT_PTPSERIAL,	LL_OTHER },
	{ IFT_LOOP,	LL_OTHER },
	{ IFT_EON,	LL_OTHER },
	{ IFT_XETHER,	LL_8022 },
	{ IFT_NSIP,	LL_OTHER },
	{ IFT_SLIP,	LL_OTHER },
#ifdef	IFT_ULTRA
	{ IFT_ULTRA,	LL_OTHER },
#endif	/* IFT_ULTRA */
#ifdef	IFT_DS3
	{ IFT_DS3,	LL_OTHER },
#endif	/* IFT_DS3 */
#ifdef	IFT_SIP
	{ IFT_SIP,	LL_OTHER },
#endif	/* IFT_SIP */
#ifdef	IFT_FRELAY
	{ IFT_FRELAY,	LL_OTHER },
#endif	/* IFT_FRELAY */
	{ 0, LL_OTHER }
    };
    register struct if_types *tp = tt;

    do {
	if (tp->type == type) {
	    break;
	}
    } while ((++tp)->type) ;

    return tp->ll_type;
}
#endif	/* IFT_OTHER */


/*
 *		Convert kernel interface flags to gated interface flags
 */
flag_t
krt_if_flags __PF1(k_flags, int)
{
    flag_t flags = 0;

    if (BIT_TEST(k_flags, IFF_UP)) {
	BIT_SET(flags, IFS_UP);
    }
    if (BIT_TEST(k_flags, IFF_BROADCAST)) {
	BIT_SET(flags, IFS_BROADCAST);
    }
    if (BIT_TEST(k_flags, IFF_POINTOPOINT)) {
	BIT_SET(flags, IFS_POINTOPOINT);
    }
#ifdef	IFF_LOOPBACK
    if (BIT_TEST(k_flags, IFF_LOOPBACK)) {
	BIT_SET(flags, IFS_LOOPBACK);
#ifdef	_POWER
	if (BIT_TEST(flags, IFS_BROADCAST)) {
	    /* Some AIX brain damage */

	    BIT_RESET(flags, IFS_BROADCAST);
	}
#endif	/* _POWER */
    }
#endif	/* IFF_LOOPBACK */

#ifdef	IFF_MULTICAST
    if (BIT_TEST(k_flags, IFF_MULTICAST)) {
	BIT_SET(flags, IFS_MULTICAST);
    }
#endif	/* IFF_MULTICAST */

#ifdef	IFF_SIMPLEX
    if (BIT_TEST(k_flags, IFF_SIMPLEX)) {
	BIT_SET(flags, IFS_SIMPLEX);
    }
#endif	/* IFF_SIMPLEX */
    return flags;
}


#ifdef	_AUX_SOURCE
#define	n_name	n_nptr
#endif	/* _AUX_SOURCE */

/* A route has been installed that is already in the kernel.  Flag it as */
/* being announced and remember this gateway */
void
krt_installed __PF1(installed_rt, rt_entry *)
{
    sockaddr_un *router;

    rtbit_set(installed_rt, krt_task->task_rtbit);

    /* Remember this route */
    router = sockdup(installed_rt->rt_router);
    rttsi_set(installed_rt->rt_head, krt_task->task_rtbit, (byte *) &router);
}


/*
 * Verify that the specified address is OK
 * Returns:
 */

int
krt_addrcheck __PF1(rtp, rt_parms *)
{
    switch (socktype(rtp->rtp_dest)) {
#ifdef	PROTO_INET
    case AF_INET:
	switch (inet_class_of_byte((byte *) &rtp->rtp_dest->in.sin_addr.s_addr)) {
	case INET_CLASSC_A:
	case INET_CLASSC_B:
	case INET_CLASSC_C:
	case INET_CLASSC_E:
	case INET_CLASSC_C_SHARP:
	    break;

#ifdef	IP_MULTICAST
	case INET_CLASSC_MULTICAST:
	    if (rtp->rtp_dest_mask == inet_mask_host) {
		/* This is a default interface specification for this group */

		return KRT_ADDR_MC;
	    } else {
		/* This is the default multicast specification */
		
		return KRT_ADDR_IGNORE;
	    }
	    break;
#endif	/* IP_MULTICAST */

	default:
	    /* Bogus, delete it */
	    return KRT_ADDR_BOGUS;
	}
	break;
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
    case AF_ISO:
 	if (sockaddrcmp(rtp->rtp_dest, sockbuild_iso(iso_default_prefix, ISO_MAXADDRLEN))) {
	    /* yank routes to an all zeros (default) prefix */

	    trace(TR_ALL,0,"YANKING ISO DEFAULT ROUTE");
	    return KRT_ADDR_BOGUS;
 	}
	break;
#endif	/* PROTO_ISO */

    default:
	/* Unknown address family */
	return KRT_ADDR_IGNORE;
    }

    return KRT_ADDR_OK;
}


/* Common routine to add kernel routes to the gated routing table */
/*
 * Returns:
 *	0	Delete this route
 *	1	OK
 */
int
krt_rtadd __PF2(rtp, rt_parms *,
		flags, short)
{
    rt_entry *rt;
    
    switch (socktype(rtp->rtp_dest)) {
#ifdef	PROTO_INET
    case AF_INET:
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
	case AF_ISO:
#endif	/* PROTO_ISO */
#if	defined(PROTO_INET) || defined(PROTO_ISO)
	if (BIT_TEST(flags, RTF_REDIRECT)) {
	    rtp->rtp_gwp = gw_timestamp(&redirect_gw_list,
					RTPROTO_REDIRECT,
					rt_task,
					(as_t) 0,
					(as_t) 0,
					RT_T_EXPIRE,
					(sockaddr_un *) 0,
					(flag_t) 0);
	    rtp->rtp_preference = RTPREF_REDIRECT;
	    BIT_SET(rtp->rtp_state, RTS_NOADVISE);
	} else {
	    rtp->rtp_gwp = krt_gwp;

	    if (BIT_TEST(rtp->rtp_state, RTS_STATIC)) {
		/* Intended to be a static route */

		rtp->rtp_preference = RTPREF_KERNEL;
		BIT_SET(rtp->rtp_state, RTS_NOAGE | RTS_RETAIN);
	    } else {
		/* Assume left over from a previous run */

		rtp->rtp_preference = RTPREF_KERNEL_REMNANT;
		BIT_SET(rtp->rtp_state, RTS_NOADVISE);
	    }
	}
	break;
#endif	/* defined(PROTO_INET) || defined(PROTO_ISO) */

    default:
	assert(FALSE);
	break;
    }

    /*
     *	If Kernel route already exists, delete this one, the kernel uses the
     *	first one
     */
    rt = rt_locate(rtp->rtp_state,
		   rtp->rtp_dest,
		   rtp->rtp_dest_mask,
		   RTPROTO_BIT(rtp->rtp_gwp->gw_proto));
    if (rt && !BIT_TEST(rt->rt_state, RTS_DELETE)) {
	return 0;
    }
    /*
     *	If there was a problem adding the route, delete the kernel route
     */
    rt = rt_add(rtp);
    if (!rt) {
	/* Could not add */

	return 0;
    } else if (rt != rt->rt_active) {
	/* Already a better route (probably an interface) */
	rt_head *rth = rt->rt_head;

	/* Delete our route */
	rt_delete(rt);

	/* Remember the active route */
	rt = rth->rth_active;

	if (!rt
	    || BIT_TEST(rt->rt_state, RTS_NOTINSTALL)) {
	    /* Should not be in kernel, delete it */

	    return 0;
	}
    }

    /* Indicate it is installed in the kernel */
    krt_installed(rt);

    return 1;
}


#ifdef	PROTO_SCRAM
int
krt_change __PF3(old_rt, rt_entry *,
		 old_router, sockaddr_un *,
		 new_rt, rt_entry *)
{
    int error = 0;

    if (old_rt) {
	error = krt_request(RTM_DELETE, old_rt, old_router);
	if (!error) {
	    scram_request(RTM_DELETE, old_rt);
	}
    }

    if (!error && new_rt) {
#if	defined(PROTO_INET)
	if (inet_addr_reject
	    && BIT_TEST(new_rt->rt_state, RTS_REJECT)) {
	    error = krt_request(RTM_ADD, new_rt, inet_addr_reject);
	} else 
#endif	/* defined(PROTO_INET) */
	error = krt_request(RTM_ADD, new_rt, new_rt->rt_router);
	if (!error) {
	    scram_request(RTM_ADD, new_rt);
	}
    }

    return error;
}

#else	/* PROTO_SCRAM */

int
krt_change __PF3(old_rt, rt_entry *,
		 old_router, sockaddr_un *,
		 new_rt, rt_entry *)
{
    int error = 0;
    sockaddr_un *new_router = (sockaddr_un *) 0;

    if (new_rt) {
#ifdef	PROTO_INET
	if (inet_addr_reject
	    && BIT_TEST(new_rt->rt_state, RTS_REJECT)) {
	    /* Reject routes need to be installed via the loopback interface */
	    new_router = inet_addr_reject;
	} else
#endif	/* PROTO_INET */
	    new_router = new_rt->rt_router;
    }

    if (old_rt && new_rt) {
	if (BIT_MASK_MATCH(old_rt->rt_state, new_rt->rt_state, RTS_GATEWAY|RTS_REJECT|RTS_BLACKHOLE|RTS_STATIC)) {
	    /* Flags are the same, we may be able to do a change */
	    
	    if (sockaddrcmp(old_router, new_router)) {
		/* Same router */
	    
		if (new_rt->rt_ifap == old_rt->rt_ifap) {
		    /* If nothing has changed, there isn't anything to do */
	    
		    return error;
		}

		/* Fall through to delete, then add */
#if	defined(KRT_RT_SOCK) && !defined(KRT_RT_NOCHANGE)
	    } else if (socksize(new_router) <= socksize(old_router)) {
		/* With the routing socket we can do a change, but only if the */
		/* length of the new gateway is equal to or less than the */
		/* length of the old one, and the flags and interface have */
		/* not changed.  Otherwise we need a delete followed */
		/* by an add. */

		return krt_request(RTM_CHANGE, new_rt, new_router);
#endif	/* defined(KRT_RT_SOCK) && !defined(KRT_RT_NOCHANGE) */
	    }
	}

	error = krt_request(RTM_DELETE, old_rt, old_router);
	switch (error) {
	case ESRCH:
	    /* This should not happen, but if this means that the */
	    /* route is really not there then a failure deleting it */
	    /* isn't too big a problem */
	    /* Fall through */
	case 0:
	    error = krt_request(RTM_ADD, new_rt, new_router);
	    break;

	default:
	    /* Do not try to add the new route because of the error */
	    /* deleting the old route */
	    break;
	}

	return error;
    }

    if (new_rt) {
	error = krt_request(RTM_ADD, new_rt, new_router);
    }

    if (old_rt) {
	switch (error) {
	case EEXIST:
	    /* This should not happen.  Try deleting and re-adding */
	    /* the route to make sure all the attributes are correct */

	    switch (krt_request(RTM_DELETE, new_rt, new_router)) {
	    case ESRCH:
		/* Very confusing, we were just told it was present. */
		/* Try to add it anyway. */
		/* Fall through */

	    case 0:
		(void) krt_request(RTM_ADD, new_rt, new_router);
		break;

	    default:
		break;
	    }

	    /* This should not happen, but if this means that the */
	    /* route is already there, that should not be too much */
	    /* of a problem */
	    /* Fall through */
	case 0:
	    error = krt_request(RTM_DELETE, old_rt, old_router);
	    break;

	default:
	    /* Do not try to delete the old route because of the error */
	    /* adding the new route */
	    break;
	}
    }

    return error;
}
#endif	/* PROTO_SCRAM */


/*	Delete a route given dest, gateway and flags	*/
/* XXX - This is a hack, is there a better way? */
/*ARGSUSED*/
int
krt_delete_dst __PF6(tp, task *,
		     rt, rt_entry *,
		     rtp, rt_parms *,
		     author, sockaddr_un *,
		     proto, proto_t,
		     gw_list, gw_entry **)
{
    int i;

    krt_rt.rt_dest = rtp->rtp_dest;
    krt_rt.rt_dest_mask = rtp->rtp_dest_mask;
    krt_rt.rt_n_gw = rtp->rtp_n_gw;
    for (i = 0; i < rtp->rtp_n_gw; i++) {
	krt_rt.rt_routers[i] = rtp->rtp_routers[i];
    }
    krt_rt.rt_state = rtp->rtp_state | RTS_GATEWAY;
    krt_rt.rt_gwp = gw_timestamp(gw_list,
				 proto,
				 tp,
				 (as_t) 0,
				 (as_t) 0,
				 RT_T_EXPIRE,
				 author,
				 (flag_t) 0);

    (void) krt_change(&krt_rt, rtp->rtp_router, rt);
    
    return 0;
}


/* Process the change list generated by routing table changes. */
void
krt_flash __PF1(rtl, rt_list *)
{
    rt_head *rth;

    /* Open the routing table in case we make a change */
    rt_open(krt_task);
    
    RT_LIST(rth, rtl, rt_head) {
	rt_entry *new_rt;
	rt_entry *old_rt = (rt_entry *) 0;
#ifdef	PROTO_INET
	const char *msg = (char *) 0;
#endif	/* PROTO_INET */

	/* See if we installed another route */
	if (rth->rth_n_announce) {
	    RT_ALLRT(old_rt, rth) {
		if (rtbit_isset(old_rt, krt_task->task_rtbit)) {
		    break;
		}
	    } RT_ALLRT_END(old_rt, rth) ;
	}

	if (new_rt = rth->rth_active) {
	    /* There is an active route */

	    if (!BIT_TEST(new_rt->rt_state, RTS_NOTINSTALL)) {
		/* We are not announcing it, this will be our new route */

#ifdef	PROTO_INET
#ifndef	VARIABLE_MASKS
	        if (new_rt->rt_dest_mask != inet_mask_host
		    && new_rt->rt_dest_mask != inet_mask_natural(new_rt->rt_dest)) {
		    if_addr *ifap = if_withnet(rth->rth_dest);

		    if (!ifap
			|| ifap->ifa_subnetmask != rth->rth_dest_mask) {
			/* We can not install subnets unless we have an interface on that */
			/* network and the netmask is the same */

			msg = "variable subnet masks not supported";
			goto no_announce;
		    }
		}
#endif	/* VARIABLE_MASKS */
		if (BIT_TEST(new_rt->rt_state, RTS_REJECT)
		     && !inet_addr_reject) {
		    /* Homey don't do reject routes */

		    msg = "reject routes not supported";
		    goto no_announce;
		}
		if (BIT_TEST(new_rt->rt_state, RTS_BLACKHOLE)
		    && !inet_addr_blackhole) {
		    /* Homey don't do blackhole routes */

		    msg = "blackhole routes not supported";
		    goto no_announce;
		}
#endif	/* PROTO_INET */

		if (new_rt != old_rt) {
		    rtbit_set(new_rt, krt_task->task_rtbit);
		}
	    } else {
		/* Not allowed to announce it */

#ifdef	PROTO_INET
	    no_announce:
		if (msg) {
		    trace(TR_KRT, LOG_INFO,
			  "krt_flash: Can not install %A/%A gateway %A: %s",
			  rth->rth_dest,
			  rth->rth_dest_mask,
			  new_rt->rt_router,
			  msg);
		}
#endif	/* PROTO_INET */
		new_rt = (rt_entry *) 0;
	    }
	}

	if (new_rt || old_rt) {
	    sockaddr_un *new_router;
	    sockaddr_un *old_router;
	    /* Something has changed */

	    if (old_rt) {
		/* Get the old router address */

		rttsi_get(rth, krt_task->task_rtbit, (byte *) &old_router);
	    } else {
		/* Indicate no old router */

		old_router = (sockaddr_un *) 0;
	    }

	    /* Change the kernel */
	    (void) krt_change(old_rt, old_router, new_rt);

	    /* Set TSI to remember the current router */
	    if (new_rt) {
		/* See if we can make use of the old sockaddr */
		if (old_router && sockaddrcmp(old_router, new_rt->rt_router)) {
		    /* Use old router */
		    new_router = old_router;
		} else if (BIT_TEST(new_rt->rt_state, RTS_REJECT)) {
		    /* Reject route, set router */

		    new_router = sockdup(inet_addr_reject);
		} else if (BIT_TEST(new_rt->rt_state, RTS_BLACKHOLE)) {
		    /* Reject route, set router */

		    new_router = sockdup(inet_addr_blackhole);
		} else {
		    /* Set new router */
		    new_router = sockdup(new_rt->rt_router);
		}
	    } else {
		/* Indicate no old router */
		new_router = (sockaddr_un *) 0;
	    }

	    if (new_router != old_router) {
		/* Change the TSI to point at new router */
		rttsi_set(rth, krt_task->task_rtbit, (byte *) &new_router);
		if (old_router) {
		    /* Free old router */
		    sockfree(old_router);
		}
	    }

	    if (old_rt && old_rt != new_rt) {
		/* Reset the announcement bit on the old route */
		(void) rtbit_reset(old_rt, krt_task->task_rtbit);
		if (old_rt->rt_gwp == krt_gwp
		    && BIT_COMPARE(old_rt->rt_state, RTS_DELETE|RTS_NOADVISE, RTS_NOADVISE)) {
		    /* A kernel route has been overridden, delete it */

		    rt_delete(old_rt);
		}
	    }
	}
    } RT_LIST_END(rth, rtl, rt_head) ;

    /* Close the routing table */
    rt_close(krt_task, (gw_entry *) 0, 0, NULL);
}



/*
 *	If we support the routing socket, remove any static routes we
 *	installed. 
 *
 *	Reset the announcement bits on any routes installed in the kernel.
 *	If they don't have RTS_RETAIN set they get removed from the kernel.
 */
static void
krt_terminate __PF1(tp, task *)
{
    int changes = 0;
    rt_entry *rt;
    rt_head *rth;
    rt_list *rtl;

    rt_open(tp);

#ifdef	KRT_RT_SOCK
    rtl = rtlist_gw(krt_gwp);

    RT_LIST(rt, rtl, rt_entry) {
	if (!BIT_TEST(rt->rt_state, RTS_DELETE|RTS_STATIC|RTS_NOADVISE)) {
	    /* Remove a route we installed */
	    rt_delete(rt);
	    changes++;
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;

    RTLIST_RESET(rtl);
#endif	/* KRT_RT_SOCK */

    rtl = rthlist_all(AF_UNSPEC);

    RT_LIST(rth, rtl, rt_head) {
	if (((rt = rth->rth_active) || (rt = rth->rth_holddown))
	    && rtbit_isset(rt, tp->task_rtbit)) {
	    sockaddr_un *router = (sockaddr_un *) 0;

	    /* Get the old router */
	    rttsi_get(rth, tp->task_rtbit, (byte *) &router);

	    if (!BIT_TEST(rt->rt_state, RTS_RETAIN)) {
		/* Remove it from the kernel */
		(void) krt_change(rt, router, (rt_entry *) 0);
	    }

	    /* Reset router and free the socket */
	    sockfree(router);
	    rttsi_reset(rth, tp->task_rtbit);
	    changes++;

	    /* Reset the bit */
	    (void) rtbit_reset(rt, tp->task_rtbit);
	}
    } RT_LIST_END(rth, rtl, rt_head);

    RTLIST_RESET(rtl);
    
    rt_close(tp, (gw_entry *) 0, changes, NULL);
    
    rtbit_reset_all(tp, tp->task_rtbit, krt_gwp);

#ifndef	KVM_TYPE_NONE
    if (kd && (KVM_CLOSE(kd) < 0)) {
	trace(TR_ALL, LOG_ERR, "kvm_terminate: %s",
	      KVM_GETERR(kd, "kvm_close error"));
    }
#endif	/* KVM_TYPE_NONE */

    task_delete(tp);
}


/*ARGSUSED*/
static void
krt_ifcheck __PF2(tip, timer *,
		  interval, time_t)
{
    /* Read the interface configuration */
    if_check();
}


/* Figure out what the maximum value for a kernel socket buffer is */
static int
krt_get_maxpacket __PF1(tp, task *)
{
#ifdef	SO_RCVBUF
    u_int I;
    u_int U = (u_int) -1 >> 1;
    u_int L = 0;

    if (tp->task_socket < 0) {
	/* No socket, just wing it */

	task_maxpacket = 32767;
	return 0;
    }

    do {
	I = (U + L) / 2;
	if (setsockopt(tp->task_socket,
		       SOL_SOCKET,
		       SO_RCVBUF,
		       (caddr_t) &I,
		       (int) (sizeof (I))) < 0) {
	    /* Failed, reduce the upper limit */

	    U = I - 1;
	} else if (L == (u_int) -1) {
	    /* As big as we can get */

	    break;
	} else {
	    /* Success, increase the lower limit */

	    L = I + 1;
	}
    } while (U >= L);

    if (setsockopt(tp->task_socket,
		   SOL_SOCKET,
		   SO_RCVBUF,
		   (caddr_t) &I,
		   (int) (sizeof (I))) < 0) {
	/* Failed - reduce by one */

	I--;
    }

    task_maxpacket = I;

    trace(TR_INT, 0, "krt_get_maxpacket: Maximum kernel receive/send packet size is %u",
	  task_maxpacket);
    trace(TR_INT, 0, NULL);
#else	/* SO_RCVBUF */
    task_maxpacket = 32767;
#endif	/* SO_RCVBUF */
    
    return 0;
}


/*
 *	Deal with an interface state change
 */
static void
krt_ifachange __PF2(tp, task *,
		    ifap, if_addr *)
{
#ifdef	KRT_RT_SOCK
    int changes = 0;
    rt_list *rtl = (rt_list *) 0;
    
    rt_entry *rt;

    rt_open(tp);
#endif	/* KRT_RT_SOCK */
    
    switch (ifap->ifa_change) {
    case IFC_NOCHANGE:
    case IFC_ADD:
	if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
	Up:;
#ifdef	IP_MULTICAST
	    inet_allrouters_join(ifap);
#endif	/* IP_MULTICAST */
#ifdef	KRT_RT_SOCK
	    rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(tp->task_rtproto));

	    RT_LIST(rt, rtl, rt_entry) {
		if (rt->rt_ifap == ifap) {
		    /* Restore static routes by making their preference positive again */
		    if (rt->rt_preference < 0) {
			(void) rt_change(rt,
					 rt->rt_metric,
					 rt->rt_tag,
					 -rt->rt_preference,
					 0, (sockaddr_un **) 0);
		    }
		}
	    } RT_LIST_END(rt, rtl, rt_entry) ;
#endif	/* KRT_RT_SOCK */
	}
	break;

    case IFC_DELETE:
	break;
	
    case IFC_DELETE|IFC_UPDOWN:
#ifdef	KRT_RT_SOCK
	rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(tp->task_rtproto));

	RT_LIST(rt, rtl, rt_entry) {
	    if (rt->rt_ifap == ifap &&
		!BIT_TEST(rt->rt_state, RTS_DELETE)) {
		/* Delete any static routes we learned via this interface */
		rt_delete(rt);
	    }
	} RT_LIST_END(rt, rtl, rt_entry) ;
#endif	/* KRT_RT_SOCK */
#ifdef	IP_MULTICAST
    Down:
	inet_allrouters_drop(ifap);
#endif	/* IP_MULTICAST */
	break;

    default:
	/* Something has changed */

	if (BIT_TEST(ifap->ifa_change, IFC_UPDOWN)) {
	    /* Up <-> Down transition */

	    if (BIT_TEST(ifap->ifa_state, IFS_UP)) {
		/* It's up now */

		goto Up;
	    } else {
		/* It's down now */

#ifdef	KRT_RT_SOCK
		rtl = rtlist_proto(AF_UNSPEC, RTPROTO_BIT(tp->task_rtproto));

		RT_LIST(rt, rtl, rt_entry) {
		    if (rt->rt_ifap == ifap) {
			/* Hide any static routes by giving them negative preferences */
			if (rt->rt_preference > 0) {
			    (void) rt_change(rt,
					     rt->rt_metric,
					     rt->rt_tag,
					     -rt->rt_preference,
					     0, (sockaddr_un **) 0);
			}
		    }
		} RT_LIST_END(rt, rtl, rt_entry) ;
#endif	/* KRT_RT_SOCK */
#ifdef	IP_MULTICAST
		goto Down;
#endif	/* IP_MULTICAST */
	    }
	}

	/* METRIC - We don't care about */
	/* BROADCAST - We don't care about */
	/* LOCALADDR - I don't think we care about */

#ifdef	KRT_RT_SOCK
	if (BIT_TEST(ifap->ifa_change, IFC_NETMASK)) {
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
#endif	/* KRT_RT_SOCK */
	break;
    }

#ifdef	KRT_RT_SOCK
    if (rtl) {
	RTLIST_RESET(rtl);
    }

    rt_close(tp, (gw_entry *) 0, changes, NULL);
#endif	/* KRT_RT_SOCK */
}


static void
krt_tsi_dump __PF3(rth, rt_head *,
		   data, void_t,
		   buf, char *)
{
    sockaddr_un *gw;

    rttsi_get(rth, krt_task->task_rtbit, (byte *) &gw);

    if (gw) {
	(void) sprintf(buf, "KERNEL gateway %-15A",
		       gw);
    }
		   
    return;
}


/*
 *	Dump the routing socket queue
 */
static void
krt_dump __PF2(tp, task *,
	       fd, FILE *)
{
    /*
     * Dump state
     */

    (void) fprintf(fd, "\tInstall routes in kernel: %s\n\n",
		   krt_install ? "yes" : "no");

    /*
     * Dump the static gateways
     */
    if (krt_gw_list) {
	(void) fprintf(fd, "\tGateways referenced by kernel routes:\n");

	gw_dump(fd,
		"\t\t",
		krt_gw_list,
		tp->task_rtproto);

	(void) fprintf(fd, "\n");
    }

#ifdef	IP_MULTICAST
    krt_multicast_dump(tp, fd);
#endif	/* IP_MULTICAST */
#ifdef	KRT_RT_SOCK
    krt_rtsock_dump(tp, fd);
#endif	/* KRT_RT_SOCK */
}


 /*  Initilize the kernel routing table function.  First, create a	*/
 /*  task to hold the socket used in manipulating the kernel routing	*/
 /*  table.  Second, read the initial kernel routing table into		*/
 /*  gated's routing table.						*/
void
krt_init __PF0(void)
{
    int saveinstall = krt_install;
    flag_t save_task_state = task_state;
#ifndef	KVM_TYPE_NONE
    KVM_OPEN_DEFINE(open_msg);
#endif	/* KVM_TYPE_NONE */

#if	RT_N_MULTIPATH > 1
    if (!BIT_MATCH(task_state, TASKS_TEST|TASKS_NODUMP)) {
	trace(TR_ALL, LOG_WARNING, "krt_init: Configured for %d multipath routes, kernel only supports one!",
	      RT_N_MULTIPATH);
    }
#endif	/* RT_N_MULTIPATH > 1 */

    krt_rt.rt_head = &krt_rth;

    krt_task = task_alloc("KRT", TASKPRI_KERNEL);
    krt_task->task_flags = TASKF_LAST;
    krt_task->task_proto = IPPROTO_RAW;
    krt_task->task_rtproto = RTPROTO_KERNEL;
    krt_task->task_terminate = krt_terminate;
    BIT_RESET(task_state, TASKS_INIT|TASKS_TEST);
    krt_task->task_ifachange = krt_ifachange;
    krt_task->task_dump = krt_dump;
#ifdef	KRT_RT_SOCK
    krt_task->task_recv = krt_recv;
    krt_task->task_socket = task_get_socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
#else	/* KRT_RT_SOCK */
#ifdef	USE_STREAMIO
    krt_task->task_socket = task_get_socket(PF_INET, SOCK_DGRAM, 0);
#else	/* USE_STREAMIO */
    krt_task->task_socket = task_get_socket(PF_UNIX, SOCK_DGRAM, 0);
#endif	/* USE_STREAMIO */
#endif	/* KRT_RT_SOCK */
    krt_task->task_n_timers = KRT_TIMER_MAX;
    task_state = save_task_state;

    /* Insure we have a socket */
    assert(BIT_TEST(task_state, TASKS_TEST) || krt_task->task_socket >= 0);

    krt_task->task_rtbit = rtbit_alloc(krt_task,
				       sizeof(sockaddr_un *),
				       (caddr_t) 0,
				       krt_tsi_dump);	/* Allocate a bit */
    if (!task_create(krt_task)) {
	task_quit(EINVAL);
    }

    krt_gwp = gw_init((gw_entry *) 0,
		      krt_task->task_rtproto,
		      krt_task,
		      (as_t) 0,
		      (as_t) 0,
		      KRT_T_EXPIRE,
		      (sockaddr_un *) 0,
		      (flag_t) 0);
    
#ifdef	KRT_RT_SOCK
    (void) timer_create(krt_task,
			KRT_TIMER_TIMEOUT,
			"Timeout",
			TIMERF_ABSOLUTE,
			(time_t) 0,
			(time_t) 0,
			krt_timeout,
			(void_t) 0);
#endif	/* KRT_RT_SOCK */

    (void) timer_create(krt_task,
			KRT_TIMER_IFCHECK,
			"IfCheck",
			(flag_t) 0,
			KRT_T_IFCHECK,
			KRT_T_IFCHECK,	/* Prevent immediate firing */
			krt_ifcheck,
			(void_t) 0);

#ifdef	PROTO_INET
#ifdef	RTF_REJECT
    if (inet_addr_reject) {
	sockfree(inet_addr_reject);
    }
    inet_addr_reject = sockdup(inet_addr_loopback);
#endif	/* RTF_REJECT */
#ifdef	RTF_BLACKHOLE
    if (inet_addr_blackhole) {
	sockfree(inet_addr_blackhole);
    }
    inet_addr_blackhole = sockdup(inet_addr_loopback);
#endif	/* RTF_BLACKHOLE */
#endif	/* PROTO_INET */

#ifndef	KVM_TYPE_NONE
    kd = KVM_OPENFILES(NULL, NULL, NULL, O_RDONLY, open_msg);
    if (!kd) {
	if (!BIT_MATCH(task_state, TASKS_TEST|TASKS_NODUMP)) {
	    trace(TR_ALL, LOG_ERR, "krt_init: %s",
		  KVM_OPEN_ERROR(open_msg));
	}

	if (!BIT_TEST(task_state, TASKS_TEST)) {
	    task_quit(errno);
	}
    }
#endif	/* KVM_TYPE_NONE */

    errno = krt_symbols();
    if (errno
	&& !BIT_TEST(task_state, TASKS_TEST)) {
	task_quit(errno);
    }

#ifdef	SIOCGNETOPT
    /* Check some kernel variables */
    errno = krt_netopts();
    if (errno
	&& !BIT_TEST(task_state, TASKS_TEST)) {
	task_quit(errno);
    }
#endif	/* SIOCGNETOPT */

    /* Figure out the maximum packet size we can use */
    errno = krt_get_maxpacket(krt_task);
    if (errno
	&& !BIT_TEST(task_state, TASKS_TEST)) {
	task_quit(errno);
    }
    
#ifdef	PROTO_SCRAM    
    /* Hack */
    scram_init();
#endif	/* PROTO_SCRAM */

    /* Read the interface configuration */
    trace(TR_KRT, 0, NULL);
    trace(TR_KRT, 0, "krt_init: Read kernel interface list");
    (void) krt_ifread(save_task_state);
    trace(TR_KRT, 0, NULL);

    rt_open(krt_task);

#ifdef	KRT_RT_SOCK
    /* Allocate buffer space */
    task_alloc_recv(krt_task, KRT_PACKET_MAX);
    task_alloc_send(krt_task, KRT_PACKET_MAX);

    if (task_set_option(krt_task,
			TASKOPTION_NONBLOCKING,
			TRUE) < 0) {
	task_quit(errno);
    }

    /* Set our receive buffer as high as possible so we don't miss any packets */
    if (task_set_option(krt_task,
			TASKOPTION_RECVBUF,
			task_maxpacket) < 0) {
	task_quit(errno);
    }
    
    /* Indicate we do not want to see our packets */
    if (task_set_option(krt_task,
			TASKOPTION_USELOOPBACK,
			FALSE) < 0) {
	task_quit(errno);
    }
#endif	/* KRT_RT_SOCK */
    
    /* Read the kernel's routing table */
    errno = krt_rtread();
    if (errno
	&& !BIT_TEST(task_state, TASKS_TEST)) {
	task_quit(errno);
    }

    rt_close(krt_task, (gw_entry *) 0, 0, NULL);

    krt_install = saveinstall;
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
