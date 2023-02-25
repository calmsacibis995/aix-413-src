static char sccsid[] = "@(#)07	1.6.1.4  src/tcpip/usr/sbin/snmpd/routes.c, snmp, tcpip411, GOLD410 2/22/94 11:28:11";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: rt_compar(), get_routes(), get_route(), get_radix_nodes(), 
 *            get_radix_node(), get_rtent() 
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/routes.c
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */


/* routes.c - MIB support of the routing tables */


#include "snmpd.h"
#include "interfaces.h"
#include "routes.h"

/*  */

/* FORWARD */

#ifdef BSD44
extern char *inet_ntoa ();
static struct sockaddr *rtmask_lookup();
static void free_masks ();
#endif

/* DATA */

int routeNumber;
static struct rtetab  *rts = NULL;
static struct rtetab **rtp;

struct rtetab *rts_inet = NULL;
#ifdef	BSD44
struct rtetab *rts_iso = NULL;
#endif

static	int	first_time = 1;
int	flush_rt_cache = 0;

/*  */

static int  
rt_compar (a, b)
register struct rtetab **a,
		       **b;
{
    int	    i;

    if ((i = (*a) -> rt_dst.sa.sa_family - (*b) -> rt_dst.sa.sa_family))
	return (i > 0 ? 1 : -1);

    return elem_cmp ((*a) -> rt_instance, (*a) -> rt_insize,
		     (*b) -> rt_instance, (*b) -> rt_insize);
}


int	
get_routes (offset)
int	offset;
{
    register int   i;
    int	    rthashsize,
	    tblsize;
    struct mbuf **rtaddr,
		**rtnet,
		**rthost = NULL;
    register struct rtetab  *rt,
			    *rp;
    struct nlist nzs;
    register struct nlist *nz = &nzs;
    static   int lastq = -1;
    static   int tic = 0;

    if (quantum == lastq)
	return OK;
    if (!flush_rt_cache
	    && offset == type_SNMP_PDUs_get__next__request
	    && quantum == lastq + 1
	    && tic < (IPROUTEENTRIES * routeNumber)) {	/* XXX: caching! */
	tic ++;
	lastq = quantum;
	return OK;
    }
    lastq = quantum, flush_rt_cache = 0, tic = 0;

    for (rt = rts; rt; rt = rp) {
	rp = rt -> rt_next;
	free ((char *) rt);
    }
    rts = rts_inet = NULL;
#ifdef	BSD44
    rts_iso = NULL;

    free_masks ();
#endif

    rtp = &rts, routeNumber = 0;
#ifdef	BSD44
    if (nl[N_RADIX_NODE_HEAD].n_value) {
	if (get_radix_nodes () == NOTOK)
	    goto out1;

	goto sort_routes;
    }
#endif

#ifndef _POWER
    if (getkmem (nl + N_RTHASHSIZE, (caddr_t) &rthashsize, sizeof rthashsize)
	    == NOTOK)
	return NOTOK;
    if (rthashsize == 0)		/* XXX: why is this? */
	rthashsize = 8;
    tblsize = rthashsize * sizeof *rtaddr;
    if ((rtnet = (struct mbuf **) malloc ((unsigned) (tblsize))) == NULL
	    || (rthost = (struct mbuf **) malloc ((unsigned) (tblsize)))
		    == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_routes");
    if (getkmem (nl + N_RTNET, (caddr_t) rtnet, tblsize) == NOTOK
	    || getkmem (nl + N_RTHOST, (caddr_t) rthost, tblsize) == NOTOK)
	goto out2;

    nz -> n_name = "struct route";
    for (rtaddr = rtnet; rtaddr; rtaddr = rthost, rthost = NULL) {
	for (i = 0; i < rthashsize; i++) {
	    register struct mbuf *m;
	    struct mbuf    ms;
	    register struct rtentry *re;

	    for (m = rtaddr[i];
		     nz -> n_value = (unsigned long) m;
		     m = ms.m_next) {
		if (getkmem (nz, (char *) &ms, sizeof ms) == NOTOK)
		    goto out2;

#ifndef	BSD44
		re = mtod (&ms, struct rtentry *);
#else
		re = (struct rtentry *) ms.m_dat;
#endif
		if (get_route (re) == NOTOK)
		    goto out2;
	    }
	}
	free ((char *) rtaddr);
    }
#endif /* _POWER */

#ifdef	BSD44
sort_routes: ;
#endif
    if (routeNumber > 1) {
	register struct rtetab **base,
			       **rte;

	if ((base = (struct rtetab **)
		    	    malloc ((unsigned) (routeNumber * sizeof *base)))
		== NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		"get_routes");

	rte = base;
	for (rt = rts; rt; rt = rt -> rt_next)
	    *rte++ = rt;

#ifdef _ANSI_C_SOURCE 
	qsort ((char *) base, routeNumber, sizeof *base, 
	       (int(*)(const void *, const void *))rt_compar);
#else /* !_ANSI_C_SOURCE */
	qsort ((char *) base, routeNumber, sizeof *base, rt_compar);
#endif /* _ANSI_C_SOURCE */

	rtp = base;
	rt = rts = *rtp++;
	rts_inet = NULL;
#ifdef	BSD44
	rts_iso = NULL;
#endif
	while (rtp < rte) {
	    switch (rt -> rt_dst.sa.sa_family) {
	        case AF_INET:
	            if (rts_inet == NULL)
			rts_inet = rt;
		    break;

#ifdef	BSD44
		case AF_ISO:
		    if (rts_iso == NULL)
			rts_iso = rt;
		    break;
#endif
	    }

	    rt -> rt_next = *rtp;
	    rt = *rtp++;
	}
	switch (rt -> rt_dst.sa.sa_family) {
	    case AF_INET:
	        if (rts_inet == NULL)
		    rts_inet = rt;
		break;

#ifdef	BSD44
	    case AF_ISO:
		if (rts_iso == NULL)
		    rts_iso = rt;
		break;
#endif
	}
	rt -> rt_next = NULL;

	free ((char *) base);
    }

    first_time = 0;
    return OK;

out2: ;
    free ((char *) rtnet);
    free ((char *) rthost);

#ifdef	BSD44
out1: ;
#endif
    for (rt = rts; rt; rt = rp) {
	rp = rt -> rt_next;

	free ((char *) rt);
    }
    rts = rts_inet = NULL;
#ifdef	BSD44
    rts_iso = NULL;
#endif

    return NOTOK;
}

/* */

static int  
get_route (re)
register struct rtentry *re;
{
    register struct rtetab *rt,
			   *rz;
#ifdef	BSD44
    union sockaddr_un rtsock;
    struct nlist nzs;
    register struct nlist *nz = &nzs;
#endif
    OIDentifier	oids;

#ifdef	BSD44
    nz -> n_name = "union sockaddr_un",
    nz -> n_value = (unsigned long) rt_key (re);
    if (getkmem (nz, (caddr_t) &rtsock, sizeof rtsock) == NOTOK)
	return NOTOK;
#endif

    if ((rt = (struct rtetab *) calloc (1, sizeof *rt)) == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_route");
    rt -> rt_stab.st_forw = rt -> rt_stab.st_back = &rt -> rt_stab;
    bcopy ((char *) re, (char *) &rt -> rt_rt, sizeof *re);

#ifndef	BSD44
    rt -> rt_dst.sa = re -> rt_dst;		/* struct copy */

    rt -> rt_gateway.sa = re -> rt_gateway;	/*   .. */
#else
    rt -> rt_dst = rtsock;			/* struct copy */

    nz -> n_name = "union sockaddr_un",
    nz -> n_value = (unsigned long) re -> rt_gateway;
    if (getkmem (nz, (caddr_t) &rt -> rt_gateway, sizeof rt -> rt_gateway)
	    == NOTOK)
	return NOTOK;

    rt -> rt_rmsa = rtmask_lookup (re);
    rt -> rt_type = (rt -> rt_rt.rt_flags & RTF_GATEWAY) ?
                  RTYPE_REMOTE : RTYPE_DIRECT;
    rt -> rt_host = rt -> rt_rt.rt_flags & RTF_HOST;
#endif

    switch (rt -> rt_dst.sa.sa_family) {
	case AF_INET:
	    rt -> rt_insize =
		ipaddr2oid (rt -> rt_instance, &rt -> rt_dst.un_in.sin_addr);
	    if (rts_inet == NULL)	/* in case routeNumber == 1 */
		rts_inet = rt;
	    break;

#ifdef	BSD44
       case AF_ISO:
	    rt -> rt_insize =
		clnpaddr2oid (rt -> rt_instance,
			      &rt -> rt_dst.un_iso.siso_addr);
	    if (rts_iso == NULL)	/* in case routeNumber == 1 */
		rts_iso = rt;
	    break;
#endif

	default:
	    bzero ((char *) rt -> rt_instance, sizeof rt -> rt_instance);
	    rt -> rt_insize = 0;
	    break;
    }
    
    for (rz = rts; rz; rz = rz -> rt_next)
	if (rz -> rt_dst.sa.sa_family == rt -> rt_dst.sa.sa_family
	        && elem_cmp (rz -> rt_instance, rz -> rt_insize,
			     rt -> rt_instance, rt -> rt_insize) == 0)
	    break;
    if (rz) {
	if (first_time) {
	    oids.oid_elements = rt -> rt_instance;
	    oids.oid_nelem = rt -> rt_insize;
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_ROUTE, ROUTE_1,
		    "duplicate routes for destination %d/%s"),
		    rt -> rt_dst.sa.sa_family, sprintoid (&oids));
	}

	rt -> rt_instance[rt -> rt_insize++] = ++rz -> rt_magic;
    }

    *rtp = rt, rtp = &rt -> rt_next, routeNumber++;

    if (debug && first_time) {
	oids.oid_elements = rt -> rt_instance;
	oids.oid_nelem = rt -> rt_insize;
	advise (SLOG_DEBUG, MSGSTR(MS_ROUTE, ROUTE_2,
		"add route: %d/%s%s%s on interface 0x%x with flags %d"),
		rt -> rt_dst.sa.sa_family, sprintoid (&oids), 
#ifdef BSD44
		" w/mask ",
		rt -> rt_rmsa ?
		    inet_ntoa (((struct sockaddr_in *)rt -> rt_rmsa) -> sin_addr)
		    : "NONE",
#else
		"", "",
#endif
		re -> rt_ifp,
		re -> rt_flags);

    }

    /* manually set-up parts of the set structure */
    rt -> rt_srt.rt_flags = rt -> rt_rt.rt_flags;
    rt -> rt_srt.rt_dst = rt -> rt_dst.sa;		/* struct copy */
    rt -> rt_srt.rt_gateway = rt -> rt_gateway.sa;	/*   .. */
    rt -> rt_sflags = ST_NOSET;

    return OK;
}

/*  */

#ifdef	BSD44

/* XXX: should really use route socket instead of kmem reads...*/

static int  
get_radix_nodes () 
{
    struct radix_node_head *rnh,
			    head;
    struct nlist nzs;
    register struct nlist *nz = &nzs;

    if (getkmem (nl + N_RADIX_NODE_HEAD, (caddr_t) &rnh, sizeof rnh) == NOTOK)
	return NOTOK;

    while (rnh) {
	nz -> n_name = "struct radix_node_head",
	nz -> n_value = (unsigned long) rnh;
	if (getkmem (nz, (caddr_t) &head, sizeof head) == NOTOK)
	    return NOTOK;
	rnh = head.rnh_next;

	if (get_radix_node (head.rnh_treetop, head.rnh_af) == NOTOK)
	    return NOTOK;
    }

    return OK;
}

/*  */

static int  
get_radix_node (rn, rnh_af)
struct radix_node *rn;
int rnh_af;
{
    struct radix_node rnode;
    struct rtentry rtentry;
    struct nlist nzs;
    register struct nlist *nz = &nzs;

    for (;;) {
	nz -> n_name = "struct radix_node",
	nz -> n_value = (unsigned long) rn;
	if (getkmem (nz, (caddr_t) &rnode, sizeof rnode) == NOTOK)
	    return NOTOK;

	if (rnode.rn_b < 0) {
	    if (!(rnode.rn_flags & RNF_ROOT)) {

		switch (rnh_af) {
		    case AF_UNSPEC:
			if (get_mask (&rnode) == NOTOK)
			    return NOTOK;
			break;

		    default:
			nz -> n_name = "struct rtentry",
			nz -> n_value = (unsigned long) rn;
			if (getkmem (nz, (caddr_t) &rtentry, sizeof rtentry) 
				== NOTOK)
			    return NOTOK;
#ifdef DEBUG
			if (first_time && debug > 2)
			    /* NOT IN CATALOG, BECAUSE DEBUG */
			    advise (SLOG_DEBUG, 
				    "get_radix_node: rtentry.netmask = %x",  
				    rt_mask (&rtentry));
#endif
			if (get_route (&rtentry) == NOTOK)
			    return NOTOK;
			break;
		}
	    }

	    if (rn = rnode.rn_dupedkey)
		continue;
	}
	else {
	    if (get_radix_node (rnode.rn_l, rnh_af) == NOTOK
		    || get_radix_node (rnode.rn_r, rnh_af) == NOTOK)
		return NOTOK;
	}

	return OK;
    }
}

/*  */

/* Route Mask */

/*  route mask table */
struct rmtab {
    caddr_t rm_rnkey;		/* from radix node... */
    struct sockaddr *rm_sa;

    struct rmtab *rm_next;
};

struct rmtab *rm_tab = NULL;

/*  */

static int
get_mask (rnode)
register struct radix_node *rnode;
{
    register struct rmtab *rm;
    struct sockaddr sa;
    struct nlist nzs;
    register struct nlist *nz = &nzs;

    if ((rm = (struct rmtab *) calloc (1, sizeof *rm)) == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_mask");
    rm -> rm_rnkey = rnode -> rn_key;	/* key used for lookup */

    /*
     * get netmask sockaddr struct.  Note that sockaddr's may vary
     * in length (think osi), so the netmask may not fit in the size
     * of a normal sockaddr.
     */
    nz -> n_name = "struct sockaddr",
    nz -> n_value = (unsigned long) rnode -> rn_key;
    if (getkmem (nz, (caddr_t) &sa, sizeof sa) == NOTOK)
	return NOTOK;
    if ((rm -> rm_sa = 
	    (struct sockaddr *) calloc (1, max (sa.sa_len, sizeof (sa))))
	    == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_mask");
    if (sa.sa_len > sizeof sa) {
	if (getkmem (nz, (caddr_t) rm -> rm_sa, sa.sa_len) == NOTOK)
	    return NOTOK;
    }
    else
	bcopy ((char *) &sa, rm -> rm_sa, sizeof (sa));

    /* insert in routemask table */
    rm -> rm_next = rm_tab;
    rm_tab = rm;

#ifdef DEBUG
    if (debug > 2 && first_time) {
	/* NOT IN CATALOG, BECAUSE DEBUG */
	advise (SLOG_DEBUG, "get_mask: rn_key = %x, sa_len = %d", rnode -> rn_key,
		sa.sa_len);
    }
#endif
}

static struct sockaddr *
rtmask_lookup (re)
struct rtentry *re;
{
    register struct rmtab *rm;

    for (rm = rm_tab; rm; rm = rm -> rm_next) 
	if (rm -> rm_rnkey == (caddr_t) rt_mask (re))
	    break;

    return (rm ? rm -> rm_sa : (struct sockaddr *) NULL);
}

static void
free_masks ()
{
    register struct rmtab *rm,
			  *rmp;

    for (rm = rm_tab; rm; rm = rmp) {
	rmp = rm -> rm_next;
	free ((char *) rm -> rm_sa);
	free ((char *) rm);
    }
    rm_tab = NULL;
}
#endif

/*  */

struct rtetab *
get_rtent (ip, len, head, isnext)
register unsigned int *ip;
int	len;
struct rtetab *head;
int	isnext;
{
    int	    family = 0;
    register struct rtetab *rt;

    if (head)
	family = head -> rt_dst.sa.sa_family;
    for (rt = head; rt; rt = rt -> rt_next)
	if (rt -> rt_dst.sa.sa_family != family)
	    break;
	else
	    switch (elem_cmp (rt -> rt_instance, rt -> rt_insize, ip, len)) {
	        case 0:
		    if (!isnext)
			return rt;
		    if ((rt = rt -> rt_next) == NULL
			    || rt -> rt_dst.sa.sa_family != family) {
			flush_rt_cache = 1;
			return NULL;
		    }
		    /* else fall... */

		case 1:
		    return (isnext ? rt : NULL);
	    }

    flush_rt_cache = 1;
    return NULL;
}
