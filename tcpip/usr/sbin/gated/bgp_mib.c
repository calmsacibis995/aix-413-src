static char sccsid[] = "@(#)64	1.1  src/tcpip/usr/sbin/gated/bgp_mib.c, tcprouting, tcpip411, GOLD410 12/6/93 17:24:49";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: BGP_SORT_LIST
 *		PROTOTYPE
 *		__PF1
 *		__PF3
 *		__PF4
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
 * bgp_mib.c,v 1.16.2.1 1993/08/27 22:28:36 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_ISODE_SNMP
#define	INCLUDE_RT_VAR
#include "include.h"

#if	defined(PROTO_BGP) && defined(PROTO_SNMP)
#include "inet.h"
#include "bgp.h"
#include "snmp_isode.h"


PROTOTYPE(o_bgp,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_bgp_peer,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_bgp_path,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));

static struct object_table bgp_objects[] = {
#define bgpVersion			0
#define bgpLocalAs			1
#define	bgpIdentifier			2
    { "bgpVersion",		o_bgp,		NULL,			bgpVersion },
    { "bgpLocalAs",		o_bgp,		NULL,			bgpLocalAs },
    { "bgpIdentifier",		o_bgp,		NULL,			bgpIdentifier },

#define	bgpPeerIdentifier		3
#define	bgpPeerState			4
#define	bgpPeerAdminStatus		5
#define	bgpPeerNegotiatedVersion	6
#define	bgpPeerLocalAddr		7
#define	bgpPeerLocalPort		8
#define	bgpPeerRemoteAddr		9
#define	bgpPeerRemotePort		10
#define	bgpPeerRemoteAs			11
#define	bgpPeerInUpdates		12
#define	bgpPeerOutUpdates    		13
#define	bgpPeerInTotalMessages		14
#define	bgpPeerOutTotalMessages		15
#define	bgpPeerLastError		16
    { "bgpPeerIdentifier",		o_bgp_peer,	NULL,			bgpPeerIdentifier },
    { "bgpPeerState",			o_bgp_peer,	NULL,			bgpPeerState },
#define	PSTATE_IDLE		1
#define	PSTATE_CONNECT		2
#define	PSTATE_ACTIVE		3
#define	PSTATE_OPENSENT		4
#define	PSTATE_OPENCONFIRM	5
#define	PSTATE_ESTABLISHED	6
    { "bgpPeerAdminStatus",		o_bgp_peer,	NULL,			bgpPeerAdminStatus },
    { "bgpPeerNegotiatedVersion",	o_bgp_peer,	NULL,			bgpPeerNegotiatedVersion },
    { "bgpPeerLocalAddr",		o_bgp_peer,	NULL,			bgpPeerLocalAddr },
    { "bgpPeerLocalPort",		o_bgp_peer,	NULL,			bgpPeerLocalPort },
    { "bgpPeerRemoteAddr",		o_bgp_peer,	NULL,			bgpPeerRemoteAddr },
    { "bgpPeerRemotePort",		o_bgp_peer,	NULL,			bgpPeerRemotePort },
    { "bgpPeerRemoteAs",		o_bgp_peer,	NULL,			bgpPeerRemoteAs },
    { "bgpPeerInUpdates",		o_bgp_peer,	NULL,			bgpPeerInUpdates },
    { "bgpPeerOutUpdates",		o_bgp_peer,	NULL,			bgpPeerOutUpdates },
    { "bgpPeerInTotalMessages",		o_bgp_peer,	NULL,			bgpPeerInTotalMessages },
    { "bgpPeerOutTotalMessages",	o_bgp_peer,	NULL,			bgpPeerOutTotalMessages },
    { "bgpPeerLastError",		o_bgp_peer,	NULL,			bgpPeerLastError },

#define	bgpPathAttrPeer			17
#define	bgpPathAttrDestNetwork		18
#define	bgpPathAttrOrigin		19
#define	bgpPathAttrASPath		20
#define	bgpPathAttrNextHop		21
#define	bgpPathAttrInterASMetric	22
    { "bgpPathAttrPeer",		o_bgp_path,	NULL,			bgpPathAttrPeer },
    { "bgpPathAttrDestNetwork",		o_bgp_path,	NULL,			bgpPathAttrDestNetwork },
    { "bgpPathAttrOrigin",		o_bgp_path,	NULL,			bgpPathAttrOrigin },
#define	PATH_ORIGIN_IGP		1
#define	PATH_ORIGIN_EGP		2
#define	PATH_ORIGIN_INCOMPLETE	3
    { "bgpPathAttrASPath",		o_bgp_path,	NULL,			bgpPathAttrASPath },
    { "bgpPathAttrNextHop",		o_bgp_path,	NULL,			bgpPathAttrNextHop },
    { "bgpPathAttrInterASMetric",	o_bgp_path,	NULL,			bgpPathAttrInterASMetric },

    { NULL }
};

#define	MIB_ENABLED	1
#define	MIB_DISABLED	2

static struct snmp_tree bgp_mib_tree = {
    NULL, NULL,
    "bgp",
    NULLOID,
    readWrite,
    bgp_objects,
    0
};


/* Since we support multiple connections per peer but the MIB does not, */
/* we only acknowledge the first non-test peer with a given address */

#undef	BGP_SORT_LIST
#undef	BGP_SORT_LIST_END

#define	BGP_SORT_LIST(bnp) \
    for (bnp = bgp_sort_list; bnp; bnp = bnp->bgp_sort_next) { \
	if (bnp->bgp_type != BGPG_TEST)
				
#define	BGP_SORT_LIST_END(bnp) \
	while (bnp->bgp_sort_next && \
	    sockaddrcmp_in(bnp->bgp_addr, bnp->bgp_sort_next->bgp_addr)) { \
	    bnp = bnp->bgp_sort_next; \
	} \
    }

static int
o_bgp __PF3(oi, OI,
	    v, register struct type_SNMP_VarBind *,
	    offset, int)
{
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem != ot->ot_name->oid_nelem + 1
	    || oid->oid_elements[oid->oid_nelem - 1]) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    OID new;

	    if ((new = oid_extend(oid, 1)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }
	    new->oid_elements[new->oid_nelem - 1] = 0;

	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    return NOTOK;
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }

    switch (ot2object(ot)->ot_info) {
    case bgpVersion:
        {
#define	VSTRING_LENGTH	(((BGP_BEST_VERSION - 1) / NBBY) + 1)
	    register int vers;
	    byte versions[VSTRING_LENGTH];

	    for (vers = 0; vers < sizeof versions; vers++) {
		versions[vers] = (byte) 0;
	    }
	    
	    for (vers = BGP_WORST_VERSION; vers <= BGP_BEST_VERSION; vers++) {
		if (BGP_KNOWN_VERSION(vers)) {
		    /* v is a supported version */
		    register int v1 = vers - 1;

		    versions[v1 / NBBY] |= 0x80 >> (v1 % NBBY);
		}
	    }

	    return o_string(oi, v, versions, VSTRING_LENGTH);
#undef	VSTRING_LENGTH
	}

    case bgpLocalAs:
	return o_integer(oi, v, inet_autonomous_system);

    case bgpIdentifier:
	return o_ipaddr(oi,
			v,
			sock2unix(inet_routerid,
				  (int *) 0));
    }

    return int_SNMP_error__status_noSuchName;
}

/**/


static bgpPeer *
o_bgp_get_peer __PF3(ip, register unsigned int *,
		     len, size_t,
		     isnext, int)
{
    static bgpPeer *last_bnp;
    static unsigned int *last;
    static int last_quantum;

    if (last_quantum != snmp_quantum) {
	last_quantum = snmp_quantum;

	if (last) {
	    task_mem_free((task *) 0, (caddr_t) last);
	    last = (unsigned int *) 0;
	}
    }

    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_bnp;
    }

    if (len) {
	u_long bnp_addr;
	register bgpPeer *p;

	oid2ipaddr(ip, &bnp_addr);

	GNTOHL(bnp_addr);

	if (isnext) {
	    register bgpPeer *new = (bgpPeer *) 0;
	    register u_long new_addr = 0;

	    BGP_SORT_LIST(p) {
		register u_long cur_addr = ntohl(sock2ip(p->bgp_addr));

		if (cur_addr > bnp_addr &&
		    (!new || cur_addr < new_addr)) {
		    new = p;
		    new_addr = cur_addr;
		}
	    } BGP_SORT_LIST_END(p) ;

	    last_bnp = new;
	} else {
	    last_bnp = (bgpPeer *) 0;
	    

	    BGP_SORT_LIST(p) {
		register u_long cur_addr = ntohl(sock2ip(p->bgp_addr));
		
		if (cur_addr == bnp_addr) {
		    last_bnp = p;
		    break;
		} else if (cur_addr > bnp_addr) {
		    break;
		}
	    } BGP_SORT_LIST_END(p) ;
	}
    } else {
	last_bnp = bgp_n_peers ? bgp_sort_list : (bgpPeer *) 0;
    }

    return last_bnp;
}


static int
o_bgp_peer_info __PF4(oi, OI,
		      v, register struct type_SNMP_VarBind *,
		      ifvar, int,
		      vp, void_t)
{
    register bgpPeer *bnp = (bgpPeer *) vp;

    switch (ifvar) {
    case bgpPeerIdentifier:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, bnp->bgp_id),
				  (int *) 0));
	
    case bgpPeerState:
    {
	int state;

	switch (bnp->bgp_state) {
	case BGPSTATE_IDLE:
	    state = PSTATE_IDLE;
	    break;
		
	case BGPSTATE_CONNECT:
	    state = PSTATE_CONNECT;
	    break;
		
	case BGPSTATE_ACTIVE:
	    state = PSTATE_ACTIVE;
	    break;
		
	case BGPSTATE_OPENSENT:
	    state = PSTATE_OPENSENT;
	    break;
		
	case BGPSTATE_OPENCONFIRM:
	    state = PSTATE_OPENCONFIRM;
	    break;
		
	case BGPSTATE_ESTABLISHED:
	    state = PSTATE_ESTABLISHED;
	    break;

	default:
	    state = -1;
	}

	return o_integer(oi, v, state);
    }
	
    case bgpPeerAdminStatus:
	return o_integer(oi, v, MIB_ENABLED/*XXX*/);
	
    case bgpPeerNegotiatedVersion:
	return o_integer(oi, v, bnp->bgp_version);

    case bgpPeerLocalAddr:
	return o_ipaddr(oi,
			v,
			sock2unix(bnp->bgp_myaddr ? bnp->bgp_myaddr : inet_addr_default,
				  (int *) 0));

    case bgpPeerLocalPort:
	return o_integer(oi, v, bnp->bgp_myaddr ? ntohs(sock2port(bnp->bgp_myaddr)) : 0);

    case bgpPeerRemoteAddr:
	return o_ipaddr(oi,
			v,
			sock2unix(bnp->bgp_addr,
				  (int *) 0));

    case bgpPeerRemotePort:
	return o_integer(oi, v, ntohs(sock2port(bnp->bgp_addr)));

    case bgpPeerRemoteAs:
	return o_integer(oi, v, bnp->bgp_peer_as);
	
    case bgpPeerInUpdates:
	return o_integer(oi, v, bnp->bgp_in_updates);
	
    case bgpPeerOutUpdates:
	return o_integer(oi, v, bnp->bgp_out_updates);
	
    case bgpPeerInTotalMessages:
	return o_integer(oi, v, bnp->bgp_in_updates + bnp->bgp_in_notupdates);
	
    case bgpPeerOutTotalMessages:
	return o_integer(oi, v, bnp->bgp_out_updates + bnp->bgp_out_notupdates);
	
    case bgpPeerLastError:
	return o_string(oi, v, &bnp->bgp_lasterror, sizeof bnp->bgp_lasterror);
    }

    return int_SNMP_error__status_noSuchName;
}


static int
o_bgp_peer __PF3(oi, OI,
		 v, register struct type_SNMP_VarBind *,
		 offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register bgpPeer *bnp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;

    /* INDEX { bgpPeerRemoteAddr } */
#define	NDX_SIZE	(sizeof (struct in_addr))

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem != ot->ot_name->oid_nelem + NDX_SIZE) {
		return int_SNMP_error__status_noSuchName;
	    }
	bnp = o_bgp_get_peer(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			     sizeof (struct in_addr),
			     0);
	if (!bnp) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    bnp = o_bgp_get_peer((unsigned int *) 0,
				 0,
				 TRUE);
	    if (!bnp) {
		return NOTOK;
	    }

	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &sock2ip(bnp->bgp_addr), sizeof (sock2ip(bnp->bgp_addr)));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    bnp = o_bgp_get_peer(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				 j = oid->oid_nelem - ot->ot_name->oid_nelem,
				 TRUE);
	    if (!bnp) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &sock2ip(bnp->bgp_addr), sizeof (sock2ip(bnp->bgp_addr)));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    return o_bgp_peer_info(oi, v, ot2object(ot)->ot_info, (void_t) bnp);
}


static rt_entry *
o_bgp_get_path __PF3(ip, register unsigned int *,
		     len, size_t,
		     isnext, int)    
{
    static rt_entry *last_rt;
    static unsigned int *last;
    static int last_quantum;

    if (last_quantum != snmp_quantum) {
	last_quantum = snmp_quantum;

	if (last) {
	    task_mem_free((task *) 0, (caddr_t) last);
	    last = (unsigned int *) 0;
	}
    }
    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_rt;
    }

    if (len) {
	u_long peer;
	sockaddr_un *dest;
	register bgpPeer *p;

	dest = sockbuild_in(0, 0);
	oid2ipaddr(ip, &sock2ip(dest));

	oid2ipaddr(&ip[sizeof (struct in_addr)], &peer);

	if (isnext) {
	    rt_head *rth;
	    /* Find the next peer in the next valid route */

	    GNTOHL(peer);

	    /* First look up this route and see if there is a next peer */
	    rth = rt_table_locate(dest, (sockaddr_un *) 0);

	    BGP_SORT_LIST(p) {
		if (ntohl(sock2ip(p->bgp_addr)) > peer) {
		    register gw_entry *gwp = &p->bgp_gw;
		    register rt_entry *rt;
		    
		    RT_ALLRT(rt, rth) {
			if (!BIT_TEST(rt->rt_state, RTS_DELETE) &&
			    rt->rt_gwp == gwp) {
			    last_rt = rt;
			    goto Return;
			}
		    } RT_ALLRT_END(rt, rth) ;
		}
	    } BGP_SORT_LIST_END(p) ;

	    /* That didn't work, find the first peer in the next route */
	    for (;
		 rth = rt_table_getnext(dest,
					AF_INET,
					(flag_t) 0,
					RTPROTO_BIT(RTPROTO_BGP));
		 dest = rth->rth_dest) {

		BGP_SORT_LIST(p) {
		    register rt_entry *rt;
		    register gw_entry *gwp = &p->bgp_gw;

		    RT_ALLRT(rt, rth) {
			if (!BIT_TEST(rt->rt_state, RTS_DELETE) &&
			    rt->rt_gwp == gwp) {
			    last_rt = rt;
			    goto Return;
			}
		    } RT_ALLRT_END(rt, rth) ;
		} BGP_SORT_LIST_END(p) ;
	    }
	} else {
	    /* Find the route with this peer */

	    /* First find the peer */
	    BGP_SORT_LIST(p) {
		if (peer == sock2ip(p->bgp_addr)) {

		    /* Now look up the route */
		    last_rt = rt_locate_gw(RTS_NETROUTE,
					   dest,
					   (sockaddr_un *) 0,
					   &p->bgp_gw);
		    goto Return;
		}
	    } BGP_SORT_LIST_END(p) ;
	}
    } else {
	/* Find the first peer of the first route */
	sockaddr_un *dest;
	rt_head *rth;

	for (dest = (sockaddr_un *) 0;
	     rth = rt_table_getnext(dest,
				    AF_INET,
				    (flag_t) 0,
				    RTPROTO_BIT(RTPROTO_BGP));
	     dest = rth->rth_dest) {
	    register bgpPeer *p;

	    BGP_SORT_LIST(p) {
		register rt_entry *rt;
		register gw_entry *gwp = &p->bgp_gw;

		RT_ALLRT(rt, rth) {
		    if (!BIT_TEST(rt->rt_state, RTS_DELETE) &&
			rt->rt_gwp == gwp) {
			last_rt = rt;
			goto Return;
		    }
		} RT_ALLRT_END(rt, rth) ;
	    } BGP_SORT_LIST_END(p) ;
	}
    }

    last_rt = (rt_entry *) 0;

 Return:
    return last_rt;
}


static int
o_bgp_path __PF3(oi, OI,
		 v, register struct type_SNMP_VarBind *,
		 offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    register rt_entry *rt;
    bgpPeer *bnp;

    /* INDEX { bgpPathAttrDestNetwork, bgpPathAttrPeer } */
#define	NDX_SIZE	(sizeof (struct in_addr) + sizeof (struct in_addr))
    
    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	rt = o_bgp_get_path(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			    NDX_SIZE,
			    0);
	if (!rt) {
	    return int_SNMP_error__status_noSuchName;
	}
	bnp = (bgpPeer *) rt->rt_gwp->gw_task->task_data;
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* Next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    rt = o_bgp_get_path((unsigned int *) 0,
				0,
				TRUE);
	    if (!rt) {
		return NOTOK;
	    }
	    bnp = (bgpPeer *) rt->rt_gwp->gw_task->task_data;
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &sock2ip(rt->rt_dest), sizeof (sock2ip(rt->rt_dest)));
	    STR_OID(ip, &sock2ip(bnp->bgp_addr), sizeof (sock2ip(bnp->bgp_addr)));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    rt = o_bgp_get_path(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				j = oid->oid_nelem - ot->ot_name->oid_nelem,
				TRUE);
	    if (!rt) {
		return NOTOK;
	    }
	    bnp = (bgpPeer *) rt->rt_gwp->gw_task->task_data;

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &sock2ip(rt->rt_dest), sizeof (sock2ip(rt->rt_dest)));
	    STR_OID(ip, &sock2ip(bnp->bgp_addr), sizeof (sock2ip(bnp->bgp_addr)));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case bgpPathAttrPeer:
	return o_ipaddr(oi,
			v,
			sock2unix(bnp->bgp_addr,
				  (int *) 0));

    case bgpPathAttrDestNetwork:
	return o_ipaddr(oi,
			v,
			sock2unix(rt->rt_dest,
				  (int *) 0));
    
    case bgpPathAttrOrigin:
        {
	    int origin;

	    switch (rt->rt_aspath->path_origin) {
	    case PATH_ORG_IGP:
		origin = PATH_ORIGIN_IGP;
		break;

	    case PATH_ORG_EGP:
		origin = PATH_ORIGIN_EGP;
		break;

	    case PATH_ORG_XX:
		origin = PATH_ORIGIN_INCOMPLETE;
		break;

	    default:
		origin = -1;
	    }
	    
	    return o_integer(oi, v, origin);
	}

    case bgpPathAttrASPath:
	/* The path is stored in network byte order and that is basically how the MIB wants to see it */
	/* XXX - what about a zero length AS path? */
	return o_string(oi, v, PATH_PTR(rt->rt_aspath), rt->rt_aspath->path_len);

    case bgpPathAttrNextHop:
	return o_ipaddr(oi,
			v,
			sock2unix(rt->rt_router,
				  (int *) 0));
	
    case bgpPathAttrInterASMetric:
	return o_integer(oi, v, rt->rt_metric);
    }

    return int_SNMP_error__status_noSuchName;
}


/**/
/* Traps */

#define	bgpEstablished		1
#define	bgpBackwardTransition	2

#define	TRAP_NUM_VARS	3

static int bgp_trap_varlist[TRAP_NUM_VARS] = {
    bgpPeerRemoteAddr,
    bgpPeerLastError,
    bgpPeerState
};


void
bgp_trap_established __PF1(bnp, bgpPeer *)
{
    struct type_SNMP_VarBindList bind[TRAP_NUM_VARS];
    struct type_SNMP_VarBind var[TRAP_NUM_VARS];

    if (snmp_varbinds_build(TRAP_NUM_VARS,
			    bind,
			    var,
			    bgp_trap_varlist,
			    &bgp_mib_tree,
			    o_bgp_peer_info,
			    (void_t) bnp) == NOTOK) {
	trace(TR_SNMP,
	      LOG_ERR,
	      "bgp_trap_established: snmp_varbinds_build failed");
	goto out;
    }

    snmp_trap("bgpEstablished",
	      bgp_mib_tree.r_name,
	      int_SNMP_generic__trap_enterpriseSpecific,
	      bgpEstablished,
	      bind);

out:;
    snmp_varbinds_free(bind);
}


void
bgp_trap_backward __PF1(bnp, bgpPeer *)
{
    struct type_SNMP_VarBindList bind[TRAP_NUM_VARS];
    struct type_SNMP_VarBind var[TRAP_NUM_VARS];

    if (snmp_varbinds_build(TRAP_NUM_VARS,
			    bind,
			    var,
			    bgp_trap_varlist,
			    &bgp_mib_tree,
			    o_bgp_peer_info,
			    (void_t) bnp) == NOTOK) {
	trace(TR_SNMP,
	      LOG_ERR,
	      "bgp_trap_backward: snmp_varbinds_build failed");
	goto out;
    }

    snmp_trap("bgpBackwardTransition",
	      bgp_mib_tree.r_name,
	      int_SNMP_generic__trap_enterpriseSpecific,
	      bgpBackwardTransition,
	      bind);

out:;
    snmp_varbinds_free(bind);
}

/**/

void
bgp_init_mib __PF1(enabled, int)
{
    if (enabled) {
	snmp_tree_register(&bgp_mib_tree);
    } else {
	snmp_tree_unregister(&bgp_mib_tree);
    }
}

#endif	/* defined(PROTO_BGP) && defined(PROTO_SNMP) */


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
