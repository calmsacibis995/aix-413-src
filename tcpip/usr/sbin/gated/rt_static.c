static char sccsid[] = "@(#)19	1.1  src/tcpip/usr/sbin/gated/rt_static.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:21";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: RT_STATIC
 *		RT_STATIC_END
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF7
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
 *  rt_static.c,v 1.2 1993/04/10 12:25:40 jch Exp
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


/* Parser code for adding static routes */

struct rt_static {
    struct rt_static *rts_forw;
    struct rt_static *rts_back;
    rt_entry	*rts_route;
    adv_entry	*rts_intfs;
    adv_entry	*rts_gateways;
    rt_parms	rts_parms;
};

static struct rt_static rt_static_list = { &rt_static_list, &rt_static_list };

static block_t rt_static_block_index = (block_t) 0;

#define	RT_STATIC(rtsp)	for (rtsp = rt_static_list.rts_forw; rtsp != &rt_static_list; rtsp = rtsp->rts_forw)
#define	RT_STATIC_END(rtsp)

gw_entry *rt_gw_list = (gw_entry *) 0;			/* List of gateways for static routes */
static gw_entry *rt_static_gwp = (gw_entry *) 0;	/* fill in */

static inline struct rt_static *
rt_static_free __PF2(rtsp, struct rt_static *,
		     delete, int)
{
    struct rt_static *prev_rtsp = rtsp->rts_back;

    /* Delete the route */
    if (rtsp->rts_route && delete) {
	rt_delete(rtsp->rts_route);
    }

    /* Free policy lists and addresses */
    if (rtsp->rts_intfs) {
	adv_free_list(rtsp->rts_intfs);
    }
    if (rtsp->rts_gateways) {
	adv_free_list(rtsp->rts_gateways);
    }
    sockfree(rtsp->rts_parms.rtp_dest);

    remque((struct qelem *) rtsp);

    task_block_free(rt_static_block_index, (void_t) rtsp);

    return prev_rtsp;
}


static inline int
rt_static_compare __PF3(rtp, rt_parms *,
			rt, rt_entry *,
			ifaps, if_addr **)
{
    register int i = rtp->rtp_n_gw;

    while (i--) {
	if (ifaps[i] != rt->rt_ifaps[i]) {
	    return FALSE;
	}
    }

    return TRUE;
}


static void
rt_static_update __PF1(tp, task *)
{
    struct rt_static *rtsp;

    rt_open(tp);

    RT_STATIC(rtsp) {
	/* For each configured static route */
	
	if (BIT_TEST(rtsp->rts_parms.rtp_state, RTS_DELETE)) {
	    /* Deleted */

	    rtsp = rt_static_free(rtsp, TRUE);
	} else {
	    register int j = 0;
	    register rt_parms *rtp = &rtsp->rts_parms;
	    register rt_entry *rt = rtsp->rts_route;
	    if_addr *ifaps[RT_N_MULTIPATH];

	    rtp->rtp_n_gw = 0;		/* No valid gateways yet */

	    if (BIT_TEST(rtp->rtp_state, RTS_GATEWAY)) {
		/* Static gateway route */
		register adv_entry *adv;

		/* Select the first RT_N_MULTIPATH routers on up interfaces */
		ADV_LIST(rtsp->rts_gateways, adv) {
		    ifaps[j] = if_withroute(rtp->rtp_dest, adv->adv_gwp->gw_addr, rtp->rtp_state);
		    if (ifaps[j]
			&& (!rtsp->rts_intfs
			    || if_policy_match(ifaps[j], rtsp->rts_intfs))) {
			rtp->rtp_routers[j++] = adv->adv_gwp->gw_addr;
			if (j == RT_N_MULTIPATH) {
			    break;
			}
		    }
		} ADV_LIST_END(rtsp->gateways, adv) ;

	    } else {
		/* Static interface route */

		register if_addr *ifap;

		IF_ADDR(ifap) {
		    if (if_policy_match(ifap, rtsp->rts_intfs)) {
#ifdef	P2P_RT_REMOTE
			if (BIT_TEST(ifap->ifa_state, IFS_POINTOPOINT)) {
			    /* A network route to a P2P interface */
			    /* may end up pointing at the wrong */
			    /* interface in the kernel if we use */
			    /* the local address, so use the */
			    /* destination address */
			    rtp->rtp_router = host ? ifap->ifa_addr : ifap->ifa_addr_local;
			} else {
			    rtp->rtp_router = ifap->ifa_addr;
			}
#else	/* P2P_RT_REMOTE */
			rtp->rtp_router = ifap->ifa_addr_local;
#endif	/* P2P_RT_REMOTE */
			ifaps[0] = ifap;
			j++;
			break;
		    }
		} IF_ADDR_END(ifap) ;
	    }

	    rtp->rtp_n_gw = j;

	    if (rtp->rtp_n_gw) {
		int new_if = FALSE;
		int new_state = FALSE;

		/* At least one interface is up */

		if (rt
		    && ((new_if = !rt_static_compare(rtp, rt, ifaps))
			|| (new_state = !BIT_MASK_MATCH(rtp->rtp_state, rt->rt_state, RTS_RETAIN|RTS_REJECT|RTS_BLACKHOLE))
			|| rtp->rtp_preference != rt->rt_preference
			|| rtp->rtp_metric != rt->rt_metric
			|| rtp->rtp_n_gw != rt->rt_n_gw
			|| !rt_routers_compare(rt, rtp->rtp_routers))) {
		    /* Try to change the existing route */

		    switch (new_if) {
		    case FALSE:
			if (!new_state
			    && rt_change(rt,
					 rtp->rtp_metric,
					 rtp->rtp_tag,
					 rtp->rtp_preference,
					 rtp->rtp_n_gw,
					 rtp->rtp_routers)) {
			    /* Change succeded */

			    break;
			}
			/* Fall through to delete and re-add */

		    case TRUE:
			/* Delete the old route */
			rt_delete(rt);
			rt = (rt_entry *) 0;
			break;
		    }
		}

		if (!rt) {
		    /* No old route or change failed, add a new one */

		    rt = rt_add(rtp);
		}
	    } else {
		/* No interfaces up */

		if (rt) {
		    /* Delete the route */
		
		    rt_delete(rt);
		    rt = (rt_entry *) 0;
		}
	    }
	    rtsp->rts_route = rt;
	}
    } RT_STATIC_END(rtsp) ;

    rt_close(tp, (gw_entry *) 0, 0, NULL);

}


static inline int
rt_parse_route_change __PF4(rtp, rt_parms *,
			    intfs, adv_entry *,
			    gateways, adv_entry *,
			    err_msg, char *)
{
    register struct rt_static *rtsp;

    RT_STATIC(rtsp) {
	if (sockaddrcmp(rtp->rtp_dest, rtsp->rts_parms.rtp_dest) &&
	    rtp->rtp_dest_mask == rtsp->rts_parms.rtp_dest_mask) {
	    /* Found an identical route */

	    if (!BIT_TEST(rtsp->rts_parms.rtp_state, RTS_DELETE)) {
		(void) sprintf(err_msg, "duplicate static route to %A",
			       rtp->rtp_dest);
		
		return TRUE;
	    }

	    /* Refresh or change old one */

	    /* Free allocated data */
	    if (rtsp->rts_intfs) {
		adv_free_list(rtsp->rts_intfs);
	    }
	    if (rtsp->rts_gateways) {
		adv_free_list(rtsp->rts_gateways);
	    }
	    sockfree(rtsp->rts_parms.rtp_dest);

	    goto Copy;
	}
    } RT_STATIC_END(rtsp) ;

    rtsp = (struct rt_static *) task_block_alloc(rt_static_block_index);

    insque((struct qelem *) rtsp,
	   (struct qelem *) rt_static_list.rts_back);

 Copy:
    rtsp->rts_parms = *rtp;

    rtsp->rts_parms.rtp_dest = sockdup(rtp->rtp_dest);
    rtsp->rts_intfs = intfs;
    rtsp->rts_gateways = gateways;
   
    return FALSE;
}


int
rt_parse_route __PF7(dest, sockaddr_un *,
		     mask, sockaddr_un *,
		     gateways, adv_entry *,
		     intfs, adv_entry *,
		     pref, pref_t,
		     state, flag_t,
		     err_msg, char *)
{
    adv_entry *adv;
    rt_parms rtparms;

    bzero((caddr_t) &rtparms, sizeof (rtparms));

    rtparms.rtp_dest = dest;
    rtparms.rtp_dest_mask = mask;
    /* Set the task on all the gateways */
    ADV_LIST(gateways, adv) {
	adv->adv_gwp->gw_task = rt_task;
    } ADV_LIST_END(gateways, adv) ;
    rtparms.rtp_gwp = rt_static_gwp;
    rtparms.rtp_state = state | RTS_NOAGE | RTS_INTERIOR;
    rtparms.rtp_preference = pref;
			    
    return rt_parse_route_change(&rtparms, intfs, gateways, err_msg);
}


void
rt_static_cleanup __PF1(tp, task *)
{
    register struct rt_static *rtsp;

    RT_STATIC(rtsp) {
	BIT_SET(rtsp->rts_parms.rtp_state, RTS_DELETE);
    } RT_STATIC_END(rtsp) ;
}


void
rt_static_reinit __PF1(tp, task *)
{
    rt_static_update(tp);
}


void
rt_static_ifachange __PF1(tp, task *)
{
    rt_static_update(tp);
}

void
rt_static_terminate __PF1(tp, task *)
{
    register struct rt_static *rtsp;

    rt_open(tp);
    
    RT_STATIC(rtsp) {
	int delete = !BIT_TEST(rtsp->rts_parms.rtp_state, RTS_RETAIN);

	rtsp = rt_static_free(rtsp, delete);

    } RT_STATIC_END(rtsp) ;

    rt_close(tp, (gw_entry *) 0, 0, NULL);
}


void
rt_static_dump __PF2(tp, task *,
		     fp, FILE *)
{
    register struct rt_static *rtsp;

    if (rt_static_list.rts_forw == &rt_static_list) {
	/* No static routes */
	return;
    }
    
    fprintf(fp, "\tStatic routes: (* indicates gateway(s) in use)\n");
    
    RT_STATIC(rtsp) {
	register rt_parms *rtp = &rtsp->rts_parms;
	adv_entry *adv;

	fprintf(fp, "\t\t%-15A mask %-15A  preference %d\tstate <%s>\n",
		rtp->rtp_dest,
		rtp->rtp_dest_mask,
		rtp->rtp_preference,
		trace_bits(rt_state_bits, rtp->rtp_state));

	/* Print the Gateways */
	fprintf(fp, "\t\t\tGateway%s\t",
		rtp->rtp_n_gw > 1 ? "s" : "");

	ADV_LIST(rtsp->rts_gateways, adv) {
	    rt_entry *rt = rtsp->rts_route;
	    const char *active = "";

	    if (rt) {
		int j = rt->rt_n_gw;

		while (j--) {
		    if (sockaddrcmp(rt->rt_routers[j], adv->adv_gwp->gw_addr)) {
			active = "*";
			break;
		    }
		} 
	    }
	    
	    fprintf(fp, "@(#) 19 1.1@(#)s ",
		    adv->adv_gwp->gw_addr,
		    active);
	} ADV_LIST_END(rtsp->rts_gateway, adv) ;
	fprintf(fp, "\n");

	/* Print the interfaces */
	if (rtsp->rts_intfs
	    && (rtsp->rts_intfs->adv_flag & ADVF_TYPE) != ADVFT_ANY) {
	    /* Print the interface restrictions */

	    fprintf(fp, "\t\t\tInterfaces ");
	    
	    ADV_LIST(rtsp->rts_intfs, adv) {
		switch (adv->adv_flag & ADVF_TYPE) {
		case ADVFT_IFN:
		    fprintf(fp, "%A ",
			    adv->adv_ifn->ifae_addr);
		    break;
		    
		case ADVFT_IFAE:
		    fprintf(fp, "%A ",
			    adv->adv_ifae->ifae_addr);
		    break;
		}
	    } ADV_LIST_END(rtsp->rts_intfs, adv) ;

	    fprintf(fp, "\n");
	}

	fprintf(fp, "\n");

    } RT_STATIC_END(rtsp) ;

    fprintf(fp, "\n");
}


void
rt_static_init __PF1(tp, task *)
{
    rt_static_gwp = gw_init((gw_entry *) 0,
			    RTPROTO_STATIC,
			    rt_task,
			    (as_t) 0,
			    (as_t) 0,
			    (time_t) 0,
			    (sockaddr_un *) 0,
			    (flag_t) 0);

    rt_static_block_index = task_block_init(sizeof (struct rt_static));
}
