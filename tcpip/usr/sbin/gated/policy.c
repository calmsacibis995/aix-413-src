static char sccsid[] = "@(#)15	1.1  src/tcpip/usr/sbin/gated/policy.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:08";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: _PROTOTYPE
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF5
 *		__PF6
 *		__PF7
 *		__PF8
 *		config_list_add
 *		config_list_alloc1499
 *		control_exterior_dump1319
 *		control_interface_dump1392
 *		control_interior_dump1267
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
 *  policy.c,v 1.30.2.2 1993/07/13 22:02:06 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_CTYPE
#include "include.h"
#include "parse.h"

unsigned int adv_n_allocated = 0;	/* Number of adv's allocated */
static const char *policy_tabs = "\t\t\t\t\t\t\t\t\t\t";

bits const gw_bits[] =
{
    {GWF_SOURCE,	"Source"},
    {GWF_TRUSTED,	"Trusted"},
    {GWF_ACCEPT,	"Accept"},
    {GWF_REJECT,	"Reject"},
    {GWF_QUERY,		"Query"},
    {GWF_IMPORT,	"ImportRestrict"},
    {GWF_FORMAT,	"Format"},
    {GWF_CHECKSUM,	"CheckSum"},
    {GWF_AUXPROTO,	"AuxProto"},
    {GWF_AUTHFAIL,	"AuthFail"},
    {GWF_NEEDHOLD,	"NeedHolddown"},
    {0, NULL}
};


#ifdef	notdef
static const bits adv_flag_bits[] = {
    {ADVF_FIRST,	"first" },
    {ADVF_NO,		"no" },
    {ADVFT_ANY,		"Any" },
    {ADVFT_GW,		"gw_entry" },
    {ADVFT_IFN,		"if_name_entry" },
    {ADVFT_IFAE,	"if_addr_entry" },
    {ADVFT_AS,		"as" },
    {ADVFT_DM,		"dest_mask" },
    {ADVFT_ASPATH,	"as_path" },
    {ADVFT_TAG,		"tag" },
    {ADVFT_PS,		"protocol_specific" },
    {ADVFOT_NONE,	"none" },
    {ADVFOT_METRIC,	"metric_t" },
    {ADVFOT_PREFERENCE,	"pref_t" },
    {ADVFOT_FLAG,	"flag_t" },
    {0,			NULL }
};
#endif	/* notdef */

adv_psfunc *control_psfunc[RTPROTO_MAX] = { 0 }; /* Array of protocol specific functions for dealing with adv data */
static block_t adv_block_index;			/* For allocating adv entries */
static block_t config_entry_index;		/* For allocating config entries */
static block_t config_list_index;


/* Add an entry to the psfunc table for a specific protocol */
void
adv_psfunc_add __PF2(proto, proto_t,
		     psp, adv_psfunc *)
{
    control_psfunc[proto] = psp;
}


/*
 *	Allocate an adv_entry.
 */
adv_entry *
adv_alloc __PF2(flags, flag_t,
		proto, proto_t)
{
    adv_entry *ale;

    if (!adv_block_index) {
	adv_block_index = task_block_init(sizeof (adv_entry));
    }
    
    /* Allocate an adv_list entry and put address into it */
    ale = (adv_entry *) task_block_alloc(adv_block_index);

    ale->adv_refcount = 1;
    ale->adv_flag = flags;
    ale->adv_proto = proto;
#ifdef	TR_ADV
    trace(TR_ADV, 0, "adv_alloc: node %X proto %s flags %X refcount %d",
	  ale,
	  trace_state(rt_proto_bits, ale->adv_proto),
	  ale->adv_flag,
	  ale->adv_refcount);
#endif	/* TR_ADV */
    adv_n_allocated++;
    return ale;
}


/*	Free an adv_entry list	*/
void
adv_free_list __PF1(adv, adv_entry *)
{
    static int level = 0;
#ifdef	TR_ADV
    int allocated = adv_n_allocated;
#endif	/* TR_ADV */
    register adv_entry *advn;

    level++;

    while (adv) {
	advn = adv;
	adv = adv->adv_next;
#ifdef	TR_ADV
	trace(TR_ADV, 0, "adv_free_list:%.*snode %X proto %s flags %X refcount %d",
	      level, policy_tabs,
	      advn,
	      trace_state(rt_proto_bits, advn->adv_proto),
	      advn->adv_flag,
	      advn->adv_refcount);
#endif	/* TR_ADV */
	if (!--advn->adv_refcount) {
	    switch (advn->adv_flag & ADVF_TYPE) {

	        case ADVFT_PS:
		    if (PS_FUNC_VALID(advn, ps_free)) {
			PS_FUNC(advn, ps_free)(advn);
		    }
		    break;

		case ADVFT_ANY:
		case ADVFT_GW:
		case ADVFT_AS:
		case ADVFT_TAG:
		    break;

		case ADVFT_IFN:
		case ADVFT_IFAE:
		    ifae_free(advn->adv_ifae);
		    break;

#ifdef	PROTO_ASPATHS
		case ADVFT_ASPATH:
		    aspath_adv_free(advn->adv_aspath);
		    break;
#endif	/* PROTO_ASPATHS */

		case ADVFT_DM:
		    if (advn->adv_dm.dm_dest) {
			sockfree(advn->adv_dm.dm_dest);
		    }
		    break;

		default:
		    assert(FALSE);
	    }
	    adv_free_list(advn->adv_list);
	    if (advn->adv_flag & ADVFO_TYPE == ADVFOT_CONFIG) {
		config_list_free(advn->adv_config);
	    }
	    task_block_free(adv_block_index, (void_t) advn);
	    adv_n_allocated--;
	}
    }
#ifdef	TR_ADV
    if (allocated != adv_n_allocated) {
	trace(TR_ADV, 0, "adv_free_list:%.*s%d of %d freed",
	      level, policy_tabs,
	      allocated - adv_n_allocated,
	      allocated);
    }
#endif	/* TR_ADV */
    level--;
}


/*
 *	Cleanup for a protocol
 */
void
adv_cleanup __PF7(proto, proto_t,
		  n_trusted, int *,
		  n_source, int *,
		  gw_list, gw_entry *,
		  intf_policy, adv_entry **,
		  import_list, adv_entry **,
		  export_list, adv_entry **)
{
    gw_entry *gwp;

    /* Reset trusted and source lists */
    if (n_trusted) {
	*n_trusted = 0;
    }
    if (n_source) {
	*n_source = 0;
    }

    /* Reset gateway list and trusted/source bits */
    GW_LIST(gw_list, gwp) {
	BIT_RESET(gwp->gw_flags, GWF_TRUSTED|GWF_SOURCE);
	adv_free_list(gwp->gw_import);
	gwp->gw_import = (adv_entry *) 0;
	adv_free_list(gwp->gw_export);
	gwp->gw_export = (adv_entry *) 0;
    } GW_LISTEND;

    /* Free import an export lists */
    if (import_list && *import_list) {
	adv_free_list(*import_list);
	*import_list = (adv_entry *) 0;
    }
    if (export_list && *export_list) {
	adv_free_list(*export_list);
	*export_list = (adv_entry *) 0;
    }

    /* Free the interface policy */
    if (intf_policy && *intf_policy) {
	adv_free_list(*intf_policy);
	*intf_policy = (adv_entry *) 0;
    }

    if (proto) {
	/* Free the interface import/export policy */
	if (int_import[proto]) {
	    adv_free_list(int_import[proto]);
	    int_import[proto] = (adv_entry *) 0;
	}
	if (int_export[proto]) {
	    adv_free_list(int_export[proto]);
	    int_export[proto] = (adv_entry *) 0;
	}
    }
}


/*	Look for the specified address in the specified list	*/
adv_entry *
adv_destmask_match __PF2(list, adv_entry *,
			 addr, sockaddr_un *)
{

    ADV_LIST(list, list) {
	dest_mask *dp = &list->adv_dm;

	if (!dp->dm_dest) {
	    /* Match anything */

	    goto Success;
	} else if (socktype(addr) == socktype(dp->dm_dest)) {

	    if (dp->dm_mask) {
		/* Have a mask */

		if (socksize(addr) >= socksize(dp->dm_mask)
		    && socksize(dp->dm_dest) >= socksize(dp->dm_mask)) {
		    register byte *cp = (byte *) dp->dm_dest->a.sa_data;
		    register byte *mp = (byte *) dp->dm_mask->a.sa_data;
		    register byte *ap = (byte *) addr->a.sa_data;
		    byte *lim = (byte *) addr + socksize(dp->dm_mask);

		    while (ap < lim) {
			if ((*ap++ ^ *cp++) & *mp++) {
			    goto Continue;
			}
		    }

		    goto Success;
		}
	    } else {
		/* Exact match */

		if (socksize(addr) == socksize(dp->dm_dest)) {
		    register byte *cp = (byte *) dp->dm_dest->a.sa_data;
		    register byte *ap = (byte *) addr->a.sa_data;
		    byte *lim = (byte *) addr + socksize(addr);

		    while (ap < lim) {
			if (*ap++ != *cp++) {
			    goto Continue;
			}
		    }

		    goto Success;
		}
	    }
	}
    Continue:;
    } ADV_LIST_END(list, list) ;

    return (adv_entry *) 0;

 Success:
#ifdef	ROUTE_AGGREGATION
#endif	/* ROUTE_AGGREGATION */
    return list;
}


/*
 *	Determine if a route is valid to an interior protocol
 */
int
export __PF6(rt, rt_entry *,
	     proto, proto_t,
	     proto_list, adv_entry *,
	     int_list, adv_entry *,
	     gw_list, adv_entry *,
	     result, adv_results *)
{
    int n_list, success, metric, match = FALSE;
    adv_entry *adv = NULL;
    adv_entry *list = NULL;
    adv_entry *sublist = NULL;
    adv_entry *lists[3];

    /* Build an array of lists to ease processing */
    lists[0] = proto_list;
    lists[1] = int_list;
    lists[2] = gw_list;

    if (rt->rt_gwp->gw_proto == RTPROTO_DIRECT) {
	/* Default to always announcing interface routes */

	success = metric = TRUE;
    } else if (rt->rt_gwp->gw_proto == proto) {
	/* If same protocol */

	/* Default to announcing it */
	success = TRUE;

	/* Pass on metric and do not allow it to be tampered with */
	result->res_metric = rt->rt_metric;
	metric = FALSE;
    } else {
	success = FALSE;
	metric = TRUE;
    }

    /* Repeat for each list, gw, int and proto */
    n_list = 2;
    do {
	if (lists[n_list]) {
	    ADV_LIST(lists[n_list], list) {
		/* Null list */
		if (!list->adv_list) {

		    /* Check for global restrict */
		    if (BIT_TEST(list->adv_flag, ADVF_NO)) {
			success = FALSE;
			goto Return;
		    }

		    /* Examine next list */
		    continue;
		}
		ADV_LIST(list->adv_list, sublist) {
		    if (sublist->adv_proto == RTPROTO_ANY
			|| rt->rt_gwp->gw_proto == sublist->adv_proto) {

			switch (sublist->adv_flag & ADVF_TYPE) {
			case ADVFT_ANY:
			    match = TRUE;
			    break;

			case ADVFT_GW:
			    if (rt->rt_gwp == sublist->adv_gwp) {
				match = TRUE;
			    }
			    break;

			case ADVFT_IFN:
			    if (rt->rt_ifap->ifa_nameent == sublist->adv_ifn
				|| rt->rt_ifap->ifa_nameent_wild == sublist->adv_ifn) {
				match = TRUE;
			    }
			    break;

			case ADVFT_IFAE:
			    if (rt->rt_ifap->ifa_addrent == sublist->adv_ifae) {
				match = TRUE;
			    }
			    break;
			    
			case ADVFT_AS:
			    if (rt->rt_gwp->gw_peer_as == sublist->adv_as) {
				match = TRUE;
			    }
			    break;

			case ADVFT_DM:
			    assert(FALSE);
			    break;

#ifdef	PROTO_ASPATHS
			case ADVFT_TAG:
			    if (tag_rt(rt) == sublist->adv_tag) {
				match = TRUE;
			    }
			    break;

			case ADVFT_ASPATH:
			    if (aspath_adv_match(sublist->adv_aspath, rt->rt_aspath)) {
				match = TRUE;
			    }
			    break;
#endif	/* PROTO_ASPATHS */

			case ADVFT_PS:
			    if (PS_FUNC_VALID(sublist, ps_rtmatch) &&
				PS_FUNC(sublist, ps_rtmatch)(sublist->adv_ps, rt)) {
				match = TRUE;
			    }
			    break;
			}
			if (match) {
			    if (sublist->adv_list) {
				if (adv = adv_destmask_match(sublist->adv_list, rt->rt_dest)) {
				    goto Match;
				}
				success = FALSE;
			    } else {
				success = TRUE;
				goto Match;
			    }
			    match = FALSE;
			}
		    }
		} ADV_LIST_END(list->adv_list, sublist) ;
	    } ADV_LIST_END(lists[n_list], list) ;
	    success = FALSE;
	}
    } while (n_list--);

 Match:
    if (match) {
	if ((adv && BIT_TEST(adv->adv_flag, ADVF_NO)) ||
	    BIT_TEST(sublist->adv_flag, ADVF_NO)) {
	    success = FALSE;
	} else {
	    success = TRUE;

	    /* Set metric */
	    if (metric) {
		if (adv && BIT_TEST(adv->adv_flag, ADVFOT_METRIC)) {
		    result->res_metric = adv->adv_result.res_metric;
		} else if (BIT_TEST(sublist->adv_flag, ADVFOT_METRIC)) {
		    result->res_metric = sublist->adv_result.res_metric;
		} else if (BIT_TEST(list->adv_flag, ADVFOT_METRIC)) {
		    result->res_metric = list->adv_result.res_metric;
		}
	    }

	    /* Set second metric */
	    if (adv && BIT_TEST(adv->adv_flag, ADVFOT_METRIC2)) {
		result->res_metric2 = adv->adv_result.res_metric2;
	    } else if (BIT_TEST(sublist->adv_flag, ADVFOT_METRIC2)) {
		result->res_metric2 = sublist->adv_result.res_metric2;
	    } else if (BIT_TEST(list->adv_flag, ADVFOT_METRIC2)) {
		result->res_metric2 = list->adv_result.res_metric2;
	    }

	    /* Set flag */
	    if (adv && BIT_TEST(adv->adv_flag, ADVFOT_FLAG)) {
		result->res_flag = adv->adv_result.res_flag;
	    } else if (BIT_TEST(sublist->adv_flag, ADVFOT_FLAG)) {
		result->res_flag = sublist->adv_result.res_flag;
	    } else if (BIT_TEST(list->adv_flag, ADVFOT_FLAG)) {
		result->res_flag = list->adv_result.res_flag;
	    }
	}
    }

 Return:
    return success;
}


/*
 *  Determine if a route is valid from an interior protocol.  The default
 *  action and preference should be preset into the preference and success
 *  arguments.
 */
int
import __PF7(dst, sockaddr_un *,
	     proto_list, adv_entry *,
	     int_list, adv_entry *,
	     gw_list, adv_entry *,
	     preference, pref_t *,
	     ifap, if_addr *,
	     ps_data, void_t)
{
    int n_list, success = TRUE;
    adv_entry *adv = NULL;
    adv_entry *list = NULL;
    adv_entry *lists[3];

    /* Build an array of lists to ease processing */
    lists[0] = proto_list;
    lists[1] = int_list;
    lists[2] = gw_list;

    /* Repeat for each list, gw, int and proto */
    n_list = 2;
    do {
	if (lists[n_list]) {
	    ADV_LIST(lists[n_list], list) {
		switch (list->adv_flag & ADVF_TYPE) {
#ifdef	PROTO_ASPATHS
		case ADVFT_ASPATH:
		    if (!aspath_adv_match(list->adv_aspath, (as_path *) ps_data)) {
			continue;
		    }
		    goto common;
#endif	/* PROTO_ASPATHS */
		    
		case ADVFT_PS:
		    if (!PS_FUNC_VALID(list, ps_dstmatch) ||
			!PS_FUNC(list, ps_dstmatch)(list->adv_ps, dst, ps_data)) {
			continue;
		    }
		    goto common;

		case ADVFT_IFN:
		case ADVFT_IFAE:
		case ADVFT_AS:
		case ADVFT_ANY:
		common:
		    /* Null list */
		    if (!list->adv_list) {
			/* check for global restrict */

			if (BIT_TEST(list->adv_flag, ADVF_NO)) {
			    success = FALSE;
			    goto finish;
			}
			continue;
		    }

		    if (adv = adv_destmask_match(list->adv_list, dst)) {
			/* Have a match */
			
			if (BIT_TEST(list->adv_flag, ADVF_NO) ||
			    BIT_TEST(adv->adv_flag, ADVF_NO)) {
			    /* Negative match */
			    
			    success = FALSE;
			} else {
			    /* Positive match, check preference */
			    
			    success = TRUE;
			    if (BIT_TEST(adv->adv_flag, ADVFOT_PREFERENCE)) {
				*preference = adv->adv_result.res_preference;
			    } else if (BIT_TEST(list->adv_flag, ADVFOT_PREFERENCE)) {
				*preference = list->adv_result.res_preference;
			    }
			}
			goto finish;
		    }
		    break;

		default:
		    assert(FALSE);
		    break;
		}
	    } ADV_LIST_END(lists[n_list], list);

	    success = FALSE;
	}
    } while (n_list-- && !adv);

 finish:
    if (!success) {
	*preference = -1;
    }
    
    return success;
}


/**/

/* Martians */

adv_entry *martian_list = NULL;		/* List of Martian nets */

/* Remove entries added from config file */
void
martian_cleanup __PF0(void)
{
    if (martian_list) {
	adv_free_list(martian_list);

	martian_list = (adv_entry *) 0;
    }
}


/* Add an entry - used only at init time */
void
martian_add __PF3(dest, sockaddr_un *,
		  mask, sockaddr_un *,
		  flag, flag_t)
{
    adv_entry *adv, *list;
    
    adv = adv_alloc(ADVFT_DM | flag, (proto_t) 0);
    adv->adv_dm.dm_dest = sockdup(dest);
    adv->adv_dm.dm_mask = mask;
    list = parse_adv_dm_append(martian_list, adv);
    if (list) {
	martian_list = list;
    }
}


/*  Return true if said network is a martian */
int
is_martian __PF1(addr, sockaddr_un *)
{
    register adv_entry *adv = adv_destmask_match(martian_list, addr);

    return (adv && BIT_TEST(adv->adv_flag, ADVF_NO)) ? TRUE : FALSE;
}


/**/

static block_t gw_block_index;

/*
 *	Lookup a gateway entry
 */
static inline gw_entry *
gw_lookup __PF3(lookup_list, gw_entry **,
		lookup_proto, proto_t,
		lookup_addr, sockaddr_un *)
{
    register gw_entry *lookup_gwp;

    GW_LIST(*lookup_list, lookup_gwp) {
	if ((lookup_gwp->gw_proto == lookup_proto) &&
	    ((lookup_gwp->gw_addr && lookup_addr && sockaddrcmp(lookup_gwp->gw_addr, lookup_addr)) ||
	     (lookup_gwp->gw_addr == lookup_addr))) {
	    break;
	}
    } GW_LISTEND;

    return lookup_gwp;
}


/*
 *	Add a gateway entry to end of the list
 */
static inline gw_entry *
gw_add __PF8(add_list, gw_entry **,
	     add_proto, proto_t,
	     add_tp, task *,
	     add_peer_as, as_t,
	     add_local_as, as_t,
	     add_timer_max, time_t,
	     add_addr, sockaddr_un *,
	     add_flags, flag_t)
{
    gw_entry *add_gwp, *gwp_new;

    if (!gw_block_index) {
	gw_block_index = task_block_init(sizeof (gw_entry));
    }

    gwp_new = (gw_entry *) task_block_alloc(gw_block_index);

    if (*add_list) {
	GW_LIST(*add_list, add_gwp) {
	    if (!add_gwp->gw_next) {
		add_gwp->gw_next = gwp_new;
		break;
	    }
	} GW_LISTEND;
    } else {
	*add_list = gwp_new;
    }
    if (add_addr) {
	gwp_new->gw_addr = sockdup(add_addr);
	sockclean(gwp_new->gw_addr);
    }
    gwp_new->gw_proto = add_proto;
    gwp_new->gw_task = add_tp;
    gwp_new->gw_peer_as = add_peer_as;
    gwp_new->gw_local_as = add_local_as;
    gwp_new->gw_timer_max = add_timer_max;
    gwp_new->gw_flags |= add_flags;
    return gwp_new;
}


/*
 *	Find an existing gw_entry or create a new one
 */
gw_entry *
gw_locate __PF8(locate_list, gw_entry **,
		locate_proto, proto_t,
		locate_tp, task *,
		locate_peer_as, as_t,
		locate_local_as, as_t,
		locate_timer_max, time_t,
		locate_addr, sockaddr_un *,
		locate_flags, flag_t)
{
    gw_entry *locate_gwp;

    locate_gwp = gw_lookup(locate_list, locate_proto, locate_addr);
    if (locate_gwp) {
	if (locate_timer_max &&
	    locate_timer_max != locate_gwp->gw_timer_max) {
	    locate_gwp->gw_timer_max = locate_timer_max;
	    locate_gwp->gw_flags |= locate_flags;
	}
    } else {
	locate_gwp = gw_add(locate_list,
			    locate_proto,
			    locate_tp,
			    locate_peer_as,
			    locate_local_as,
			    locate_timer_max,
			    locate_addr,
			    locate_flags);
    }
    return locate_gwp;
}


/*
 *	Update last heard from timer for a gateway
 */
gw_entry *
gw_timestamp __PF8(list, gw_entry **,
		   proto, proto_t,
		   tp, task *,
		   peer_as, as_t,
		   local_as, as_t,
		   timer_max, time_t,
		   addr, sockaddr_un *,
		   flags, flag_t)
{
    gw_entry *gwp;

    gwp = gw_locate(list, proto, tp, peer_as, local_as, timer_max, addr, flags);
    gwp->gw_time = time_sec;
    return gwp;
}


/*
 *	Initialize a gateway structure
 */
gw_entry *
gw_init __PF8(gwp, gw_entry *,
	      proto, proto_t,
	      tp, task *,
	      peer_as, as_t,
	      local_as, as_t,
	      timer_max, time_t,
	      addr, sockaddr_un *,
	      flags, flag_t)
{
    gw_entry *list = (gw_entry *) 0;
    
    if (gwp) {
	gwp->gw_proto = proto;
	gwp->gw_task = tp;
	gwp->gw_peer_as = peer_as;
	gwp->gw_local_as = local_as;
	gwp->gw_timer_max = timer_max;
	gwp->gw_flags |= flags;
	if (addr) {
	    gwp->gw_addr = sockdup(addr);
	}
    } else {
	gwp = gw_add(&list, proto, tp, peer_as, local_as, timer_max, addr, flags);
    }

    return gwp;
}


/*
 *	Free a gw list
 */
void
gw_freelist __PF1(gwp, gw_entry *)
{
    while (gwp) {
	gw_entry *next_gwp = gwp->gw_next;

	if (gwp->gw_addr) {
	    sockfree(gwp->gw_addr);
	}
	task_block_free(gw_block_index, (void_t) gwp);

	gwp = next_gwp;
    }
}


/*
 *	Dump gateway information
 */
void
gw_dump __PF4(fd, FILE *,
	      name, const char *,
	      list, gw_entry *,
	      proto, proto_t)
{
    gw_entry *gwp;

    GW_LIST(list, gwp) {
	(void) fprintf(fd, name);
	if (gwp->gw_addr) {
	    (void) fprintf(fd, " %#-20A",
			   gwp->gw_addr);
	}
	if (gwp->gw_proto != proto) {
	    (void) fprintf(fd, " proto %s",
			   trace_state(rt_proto_bits, gwp->gw_proto));
	}
	if (gwp->gw_time) {
	    (void) fprintf(fd, " last update: %T",
			   gwp->gw_time);
	}
	if (gwp->gw_flags) {
	    (void) fprintf(fd, " flags: <%s>",
			   trace_bits(gw_bits, gwp->gw_flags));
	}
	if (gwp->gw_task) {
	    (void) fprintf(fd, " task: %s",
			   task_name(gwp->gw_task));
	}
	if (gwp->gw_peer_as) {
	    (void) fprintf(fd, " peer AS: %d",
			   gwp->gw_peer_as);
	}
	if (gwp->gw_local_as) {
	    (void) fprintf(fd, " local AS: %d",
			   gwp->gw_local_as);
	}
	if (gwp->gw_timer_max) {
	    (void) fprintf(fd, " Timer max: %#T",
			   gwp->gw_timer_max);
	}
	(void) fprintf(fd, "\n");
    } GW_LISTEND;
}


/*
 *	Dump a dest/mask list displaying metric and preference if present
 */
void
control_dmlist_dump __PF5(fd, FILE *,
			  level, int,
			  list, adv_entry *,
			  dummy1, adv_entry *,
			  dummy2, adv_entry *)
{
    adv_entry *adv;

    ADV_LIST(list, adv) {
	dest_mask *dp = &adv->adv_dm;

	if (dp->dm_dest) {
	    if (dp->dm_mask) {
		(void) fprintf(fd, "%.*s%-15A  mask %-15A",
			       level, policy_tabs,
			       dp->dm_dest,
			       dp->dm_mask);
	    } else {
		(void) fprintf(fd, "%.*shost %-15A",
			       level, policy_tabs,
			       dp->dm_dest);
	    }
	} else {
	    (void) fprintf(fd, "%.*sall",
			   level, policy_tabs);
	}
	if (BIT_TEST(adv->adv_flag, ADVF_NO)) {
	    (void) fprintf(fd, " restrict\n");
	} else {
	    switch (adv->adv_flag & ADVFO_TYPE) {
	    case ADVFOT_NONE:
		(void) fprintf(fd, "\n");
		break;

	    case ADVFOT_METRIC:
		(void) fprintf(fd, "  metric %d\n",
			       adv->adv_result.res_metric);
		break;

	    case ADVFOT_PREFERENCE:
		(void) fprintf(fd, "  preference %d\n",
			       adv->adv_result.res_preference);
		break;
	    }
	}
    } ADV_LIST_END(list, adv) ;
}


/*
 *	Dump the policy database
 */
void
control_dump __PF1(fd, FILE *)
{
    /* Martian networks */
    (void) fprintf(fd, "\tMartians:\n");
    control_dmlist_dump(fd, 2, martian_list, (adv_entry *) 0, (adv_entry *) 0);
    (void) fprintf(fd, "\n");
}


void
control_interface_import_dump __PF3(fd, FILE *,
				    level, int,
				    list, adv_entry *)
{
    ADV_LIST(list, list) {
	switch (list->adv_flag & ADVF_TYPE) {
	case ADVFT_ANY:
	    break;

	case ADVFT_IFN:
	{
	    const char *type = "*";
	    register char *cp = list->adv_ifn->ifae_addr->s.ss_string;

	    do {
		if (isdigit(*cp)) {
		    type = "";
		    break;
		}
	    } while (*cp++) ;
	    (void) fprintf(fd,
			   "%.*sInterface @(#) 15 1.1@(#)s\n",
			   level++, policy_tabs,
			   list->adv_ifn->ifae_addr,
			   type);
	}
	    break;

	case ADVFT_IFAE:
	    (void) fprintf(fd,
			   "%.*sInterface %A\n",
			   level++, policy_tabs,
			   list->adv_ifae->ifae_addr);
	    break;
			
	default:
	    assert(FALSE);
	}
	if (BIT_TEST(list->adv_flag, ADVF_NO)) {
	    (void) fprintf(fd, "%.*sRestrict\n",
			   level, policy_tabs);
	} else if (BIT_TEST(list->adv_flag, ADVFOT_PREFERENCE)) {
	    (void) fprintf(fd, "%.*sPreference %d:\n",
			   level, policy_tabs,
			   list->adv_result.res_preference);
	    level++;
	}
	control_dmlist_dump(fd, level, list->adv_list, (adv_entry *) 0, (adv_entry *) 0);
    } ADV_LIST_END(list, list) ;
}


void
control_import_dump __PF5(fd, FILE *,
			  level, int,
			  proto, proto_t,
			  proto_list, adv_entry *,
			  gw_list, gw_entry *)
{
    int lower;
    adv_entry *adv;
    gw_entry *gwp;

    if (proto_list ||
	proto ||
	gw_list) {
	(void) fprintf(fd, "%.*sImport controls:\n",
		       level++, policy_tabs);
    }
    if (proto_list) {
	ADV_LIST(proto_list, adv) {
	    lower = level;
	    if (BIT_TEST(adv->adv_flag, ADVF_NO)) {
		(void) fprintf(fd, "%.*sRestrict\n",
			       level, policy_tabs);
	    } else if (BIT_TEST(adv->adv_flag, ADVFOT_PREFERENCE)) {
		(void) fprintf(fd, "%.*sPreference %d:\n",
			       level, policy_tabs,
			       adv->adv_result.res_preference);
		lower++;
	    }
	    control_dmlist_dump(fd, lower, adv->adv_list, (adv_entry *) 0, (adv_entry *) 0);
	} ADV_LIST_END(proto_list, adv) ;
    }
    if (proto && int_import[proto]) {
	control_interface_import_dump(fd, level, int_import[proto]);
    }
    if (gw_list) {
	GW_LIST(gw_list, gwp) {
	    adv = gwp->gw_import;
	    if (!adv) {
		continue;
	    }
	    lower = level + 1;
	    (void) fprintf(fd, "%.*sGateway %A:\n",
			   level, policy_tabs,
			   gwp->gw_addr);
	    if (BIT_TEST(adv->adv_flag, ADVF_NO)) {
		(void) fprintf(fd, "%.*sRestrict\n",
			       level + 1, policy_tabs);
	    } else if (BIT_TEST(adv->adv_flag, ADVFOT_PREFERENCE)) {
		(void) fprintf(fd, "%.*sPreference %d:\n",
			       level + 1, policy_tabs,
			       adv->adv_result.res_preference);
		lower++;
	    }
	    control_dmlist_dump(fd, lower, adv->adv_list, (adv_entry *) 0, (adv_entry *) 0);
	} GW_LISTEND;
    }
}


void
control_entry_dump __PF3(fd, FILE *,
			 level, int,
			 list, adv_entry *)
{
    int first = TRUE;
    adv_entry *adv;

    if (list) {
	(void) fprintf(fd, "%.*s", level, policy_tabs);
	if (list->adv_proto != RTPROTO_ANY) {
	    (void) fprintf(fd, "Protocol %s ",
			   trace_state(rt_proto_bits, list->adv_proto));
	}
	adv = list;
	if (adv) {
	    do {
		switch (adv->adv_flag & ADVF_TYPE) {

		case ADVFT_DM:
		    assert(FALSE);
		    break;

		case ADVFT_ANY:
		    break;

		case ADVFT_GW:
		    (void) fprintf(fd, " %s%A",
				   first ? "gateway " : "",
				   adv->adv_gwp->gw_addr);
		    break;

		case ADVFT_IFN:
		    (void) fprintf(fd, " %s@(#) 15 1.1@(#)s",
				   first ? "interface " : "",
				   adv->adv_ifn->ifae_addr,
				   ifn_wildcard(adv->adv_ifn) ? "" : "*");
		    break;

		case ADVFT_IFAE:
		    (void) fprintf(fd, " %s%A",
				   first ? "interface " : "",
				   adv->adv_ifae->ifae_addr);
		    break;
		    
		case ADVFT_AS:
		    (void) fprintf(fd, " %s%u",
				   first ? "as " : "",
				   adv->adv_as);
		    break;	

#ifdef	PROTO_ASPATHS
		case ADVFT_TAG:
		    (void) fprintf(fd, " %s%A",
				   first ? "tag " : "",
				   sockbuild_in(0, adv->adv_tag));
		    break;

		case ADVFT_ASPATH:
		    (void) fprintf(fd, " %s%s",
				   first ? "aspath " : "",
				   aspath_adv_print(adv->adv_aspath));
		    break;
#endif	/* PROTO_ASPATHS */

		case ADVFT_PS:
		    if (PS_FUNC_VALID(adv, ps_print)) {
			(void) fprintf(fd, " %s",
				       PS_FUNC(adv, ps_print)(adv->adv_ps, first));
		    }
		    break;
		}
		first = FALSE;
	    } while ((adv = adv->adv_next) && !BIT_TEST(adv->adv_flag, ADVF_FIRST));
	}
	if (BIT_TEST(list->adv_flag, ADVF_NO)) {
 	    (void) fprintf(fd, " restrict");
	} else {
	    switch (list->adv_flag & ADVFO_TYPE) {
	    case ADVFOT_NONE:
		break;

	    case ADVFOT_METRIC:
		(void) fprintf(fd, " metric %d", list->adv_result.res_metric);
		break;

	    case ADVFOT_PREFERENCE:
		if (list->adv_result.res_preference < 0) {
		    (void) fprintf(fd, " restrict");
		} else {
		    (void) fprintf(fd, " preference %d",
				   list->adv_result.res_preference);
		}
		break;
	    }
	}
    }
    (void) fprintf(fd, "\n");
}


static void
control_export_list_dump __PF3(fd, FILE *,
			       level, int,
			       list, adv_entry *)
{
    int lower;
    adv_entry *adv;

    if (list) {
	ADV_LIST(list, list) {
	    lower = level;
	    if (BIT_TEST(list->adv_flag, ADVFOT_METRIC)) {
		(void) fprintf(fd, "%.*sMetric %d:\n",
			       level, policy_tabs,
			       list->adv_result.res_metric);
		lower++;
	    }
	    adv = list->adv_list;
	    if (adv) {
		do {
		    control_entry_dump(fd, lower, adv);
		    if (adv->adv_list) {
			control_dmlist_dump(fd, lower + 1, adv->adv_list, (adv_entry *) 0, (adv_entry *) 0);
		    }
		    do {
			adv = adv->adv_next;
		    } while (adv && !BIT_TEST(adv->adv_flag, ADVF_FIRST));
		} while (adv);
	    }
	} ADV_LIST_END(list, list) ;
    }
}


void
control_interface_export_dump __PF3(fd, FILE *,
				    level, int,
				    list, adv_entry *)
{
    ADV_LIST(list, list) {
	switch (list->adv_flag & ADVF_TYPE) {
	case ADVFT_ANY:
	    break;

	case ADVFT_IFN:
	    (void) fprintf(fd,
			   "%.*sInterface @(#) 15 1.1@(#)s\n",
			   level++, policy_tabs,
			   list->adv_ifn->ifae_addr,
			   ifn_wildcard(list->adv_ifn) ? "" : "*");
	    break;

	case ADVFT_IFAE:
	    (void) fprintf(fd,
			   "%.*sInterface %A\n",
			   level++, policy_tabs,
			   list->adv_ifae->ifae_addr);
	    break;
			
	default:
	    assert(FALSE);
	}
	if (BIT_TEST(list->adv_flag, ADVF_NO)) {
	    (void) fprintf(fd, "%.*sRestrict\n",
			   level, policy_tabs);
	} else if (BIT_TEST(list->adv_flag, ADVFOT_METRIC)) {
	    (void) fprintf(fd, "%.*sMetric %d:\n",
			   level, policy_tabs,
			   list->adv_result.res_metric);
	    level++;
	}
	control_export_list_dump(fd, level, list);
	level--;
    } ADV_LIST_END(list, list) ;
}


void    
control_export_dump __PF5(fd, FILE *,
			  level, int,
			  proto, proto_t,
			  proto_list, adv_entry *,
			  gw_list, gw_entry *)
{
    adv_entry *adv;
    gw_entry *gwp;

    if (proto_list ||
	proto ||
	gw_list) {
	(void) fprintf(fd, "%.*sExport controls:\n",
		       level++, policy_tabs);
    }
    control_export_list_dump(fd, level, proto_list);

    if (proto && int_export[proto]) {
	control_interface_export_dump(fd, level, int_export[proto]);
#ifdef	notdef
	if_addr *ifap;

	IF_ADDR(ifap) {
	    adv = ifap->ifa_ps[proto].ips_export;
	    if (adv) {
		(void) fprintf(fd, "%.*sInterface %s  Address %A:\n",
			       level, policy_tabs,
			       ifap->ifa_name,
			       ifap->ifa_addr);
		control_export_list_dump(fd, level + 1, adv);
	    }
	} IF_ADDR_END(ifap) ;
#endif	/* notdef */
    }
    if (gw_list) {
	GW_LIST(gw_list, gwp) {
	    adv = gwp->gw_export;
	    if (adv) {
		(void) fprintf(fd, "%.*sGateway %A:\n",
			       level, policy_tabs,
			       gwp->gw_addr);
		control_export_list_dump(fd, level + 1, adv);
	    }
	} GW_LISTEND;
    }
}


void
control_interior_dump(fd, level, func, list)
FILE *fd;
int level;
_PROTOTYPE(func,
	   void,
	   (FILE *,
	    int,
	    proto_t,
	    adv_entry *,
	    gw_entry *));
adv_entry *list;
{
    adv_entry *adv;

    ADV_LIST(list, adv) {
	switch (adv->adv_flag & ADVF_TYPE) {
	case ADVFT_AS:
	    (void) fprintf(fd, "%.*sAS %u:\n",
			   level, policy_tabs,
			   adv->adv_as);
	    break;

#ifdef	PROTO_ASPATHS
	case ADVFT_TAG:
	    (void) fprintf(fd, "%.*sTAG %A:\n",
			   level, policy_tabs,
			   sockbuild_in(0, adv->adv_tag));
	    break;

	case ADVFT_ASPATH:
	    (void) fprintf(fd, "%.*sAS Path %s:\n",
			   level, policy_tabs,
			   aspath_adv_print(adv->adv_aspath));
	    break;
#endif	/* PROTO_ASPATHS */
			   
	case ADVFT_PS:
	    if (PS_FUNC_VALID(adv, ps_print)) {
		(void) fprintf(fd, "%.*s%s:\n",
			       level, policy_tabs,
			       PS_FUNC(adv, ps_print)(adv->adv_ps, TRUE));
	    }
	    break;
	}

	func(fd, level + 1, (proto_t) 0, adv, (gw_entry *) 0);
	(void) fprintf(fd, "\n");
    } ADV_LIST_END(list, adv) ;
}


void
control_exterior_dump(fd, level, func, list)
FILE *fd;
int level;
_PROTOTYPE(func,
	   void,
	   (FILE *,
	    int,
	    proto_t,
	    adv_entry *,
	    gw_entry *));
adv_entry *list;
{
    adv_entry *adv;

    ADV_LIST(list, adv) {
	switch (adv->adv_flag & ADVF_TYPE) {
	case ADVFT_AS:
	    (void) fprintf(fd, "%.*sAS %u:\n",
			   level, policy_tabs,
			   adv->adv_as);
	    break;

#ifdef	PROTO_ASPATHS
	case ADVFT_TAG:
	    (void) fprintf(fd, "%.*sTag %A:\n",
			   level, policy_tabs,
			   sockbuild_in(0, adv->adv_tag));
	    break;

	case ADVFT_ASPATH:
	    (void) fprintf(fd, "%.*sAS Path %s:\n",
			   level, policy_tabs,
			   aspath_adv_print(adv->adv_aspath));
	    break;
#endif	/* PROTO_ASPATHS */
			   
	case ADVFT_PS:
	    if (PS_FUNC_VALID(adv, ps_print)) {
		(void) fprintf(fd, "%.*s%s:\n",
			       level, policy_tabs,
			       PS_FUNC(adv, ps_print)(adv->adv_ps, TRUE));
	    }
	    break;
	}
			   
	func(fd, level + 1, (proto_t) 0, adv->adv_list, (gw_entry *) 0);
	(void) fprintf(fd, "\n");
    } ADV_LIST_END(list, adv) ;
}


adv_entry *
control_exterior_locate __PF2(list, adv_entry *,
			      as, as_t)
{
    adv_entry *exterior = (adv_entry *) 0;
    
    if (list) {
	ADV_LIST(list, list) {
	    if (list->adv_as == as) {
		exterior = list->adv_list;
		if (exterior) {
		    break;
		}
	    }
	} ADV_LIST_END(list, list) ;
    }

    return exterior;
}


void
control_interface_dump(fd, level, list, entry_dump)
FILE *fd;
int level;
adv_entry *list;
_PROTOTYPE(entry_dump,
	   void,
	   (FILE *,
	    config_entry *));
{

    if (list) {
	ADV_LIST(list, list) {
	    adv_entry *adv;
	    
	    /* Start the line */
	    (void) fprintf(fd,
			   "%.*s Interface",
			   level, policy_tabs);

	    /* Dump the interface names */
	    ADV_LIST(list, adv) {

		switch (adv->adv_flag & ADVF_TYPE) {
		case ADVFT_ANY:
		    (void) fprintf(fd, " all");
		    break;

		case ADVFT_IFN:
		    (void) fprintf(fd, " @(#) 15 1.1@(#)s",
				   adv->adv_ifn->ifae_addr,
				   ifn_wildcard(adv->adv_ifn) ? "" : "*");
		    break;

		case ADVFT_IFAE:
		    (void) fprintf(fd, " %A",
				   adv->adv_ifae->ifae_addr);
		    break;
			
		default:
		    assert(FALSE);
		}
		if (adv->adv_next && BIT_TEST(adv->adv_next->adv_flag, ADVF_FIRST)) {
		    break;
		}
	    } ADV_LIST_END(list, adv) ;

	    if (entry_dump && list->adv_config && list->adv_config->conflist_list) {

		entry_dump(fd, list->adv_config->conflist_list);
	    }
	    
	    /* Dump the contents of the first entry */
	    if (BIT_TEST(list->adv_flag, ADVFOT_FLAG)) {
		(void) fprintf(fd,
			       " <%s>",
			       trace_bits(if_state_bits, list->adv_result.res_flag));
	    }
	    if (BIT_TEST(list->adv_flag, ADVFOT_METRIC)) {
		(void) fprintf(fd,
			       " metric %d",
			       list->adv_result.res_metric);
	    }
	    if (BIT_TEST(list->adv_flag, ADVFOT_PREFERENCE)) {
		if (list->adv_result.res_preference < 0) {
		    (void) fprintf(fd,
				   " restrict");
		} else {
		    (void) fprintf(fd,
				   " preference %d",
				   list->adv_result.res_preference);
		}
	    }

	    /* and end the line */
	    (void) fprintf(fd, "\n");

	    if (!(list = adv)) {
		break;
	    }
	} ADV_LIST_END(list, list) ;

	(void) fprintf(fd, "\n");
    }
}


/**/

config_entry *
config_alloc __PF2(type, int,
		   data, void_t)
{
    config_entry *cp;

    if (!config_entry_index) {
	config_entry_index = task_block_init(sizeof *cp);
    }
    
    cp = (config_entry *) task_block_alloc(config_entry_index);
    cp->config_type = type;
    cp->config_data = data;

    return cp;
}


config_list *
config_list_alloc(cp, entry_free)
config_entry *cp;
_PROTOTYPE(entry_free,
	   void,
	   (config_entry *));
{
    config_list *list;

    if (!config_list_index) {
	config_list_index = task_block_init(sizeof *list);
    }

    list = (config_list *) task_block_alloc(config_list_index);
    list->conflist_refcount++;
    list->conflist_free = entry_free;
    list->conflist_list = cp;

    return list;
}


config_list *
config_list_add(list, cp, entry_free)
config_list *list;
config_entry *cp;
_PROTOTYPE(entry_free,
	   void,
	   (config_entry *));
{
    if (!list) {
	list = config_list_alloc(cp, entry_free);
    } else {
	list->conflist_list = config_append(list->conflist_list,
					    cp);
    }

    return list->conflist_list ? list : (config_list *) 0;
}


void
config_list_free __PF1(list, config_list *)
{
    if (list && !--list->conflist_refcount) {
	config_entry *next = list->conflist_list;

	while (next) {
	    config_entry *cp = next;

	    next = next->config_next;

	    list->conflist_free(cp);
	    task_block_free(config_entry_index, (void_t) cp);
	}

	task_block_free(config_list_index, (void_t) list);
    }
}


config_entry *
config_append __PF2(list, config_entry *,
		    new, config_entry *)
{

    if (list) {
	config_entry *last = (config_entry *) 0;
	register config_entry *cp;

	CONFIG_LIST(cp, list) {
	    if (new->config_type < cp->config_type) {
		/* Insert before this one */
		break;
	    } else if (cp->config_type == new->config_type) {
		/* XXX - Duplicate entry */
		return (config_entry *) 0;
	    }
	    last = cp;
	} CONFIG_LIST_END(cp, list) ;

	if (last) {
	    new->config_next = last->config_next;
	    last->config_next = new;
	} else {
	    new->config_next = list;
	    list = new;
	}
    } else {
	list = new;
    }

    return list;
}


void
config_resolv_free __PF2(list, config_entry **,
			 size, int)
{
    /* Free the array */
    task_block_reclaim((size_t) (size * sizeof (config_entry *)),
		       (void_t) list);
}


config_entry **
config_resolv __PF3(policy, adv_entry *,
		    ifap, if_addr *,
		    size, int)
{
    int entries = 0;
    register adv_entry *adv;
    config_entry **list;

    /* Allocate a block */
    list = (config_entry **) task_block_malloc((size_t) (size * sizeof (config_entry)));

    ADV_LIST(policy, adv) {
	int prio = 0;
	
	switch (adv->adv_flag & ADVF_TYPE) {
		
	case ADVFT_ANY:
	    prio = CONFIG_PRIO_ANY;
	    break;

	case ADVFT_IFN:
	    if (ifap->ifa_nameent == adv->adv_ifn) {
		prio = CONFIG_PRIO_NAME;
	    } else if (ifap->ifa_nameent_wild == adv->adv_ifn) {
		prio = CONFIG_PRIO_WILD;
	    }
	    break;

	case ADVFT_IFAE:
	    if (ifap->ifa_addrent == adv->adv_ifae) {
		prio = CONFIG_PRIO_ADDR;
	    }
	    break;

	default:
	    assert(FALSE);
	}

	if (prio) {
	    register config_entry *cp;

	    /* We have a match */
	    entries++;

	    if (adv->adv_config) {

		/* Merge entries according to priority */

		CONFIG_LIST(cp, adv->adv_config->conflist_list) {
		    if (!list[cp->config_type] ||
			list[cp->config_type]->config_priority < prio) {
			(list[cp->config_type] = cp)->config_priority = prio;
		    }		
		} CONFIG_LIST_END(cp, adv->adv_config->conflist_list) ;
	    }
	}
    } ADV_LIST(policy, adv) ;

    if (!entries) {
	config_resolv_free(list, size);
	list = (config_entry **) 0;
    }
    
    return list;
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
