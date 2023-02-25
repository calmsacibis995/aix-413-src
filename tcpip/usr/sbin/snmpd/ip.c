static char sccsid[] = "@(#)01	1.20  src/tcpip/usr/sbin/snmpd/ip.c, snmp, tcpip411, 9434A411a 8/18/94 14:31:47";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: read_ipstat(), o_ip(), o_ip_default_ttl(), s_ip_default_ttl(),
 *	      o_ip_forwarding(), s_ip_forwarding(), o_ip_addr(), o_ip_route(),
 *	      s_ip_route(), sr_commit(), sr_dest(), sr_ifindex(), sr_nexthop(),
 *	      sr_type(), sr_mask(), sr_noop(), o_ip_routing_stats(), 
 *	      o_address(), s_address(), sa_commit(), sa_ifindex(), 
 *	      sa_netaddress(), sa_physaddress(), sa_type(), adn_compar(), 
 *	      adm_compar(), ada_compar(), get_arptab(), get_arpent(), init_ip()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/ip.c
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


/* ip.c - MIB realization of the IP (and Address Translation) group */


#include "snmpd.h"
#include "interfaces.h"
#include <sys/ioctl.h>

#ifdef _POWER
#  ifndef NATIVE
#include <net/netopt.h>
#  else
#include "netopt.h"
#  endif /* NATIVE */
#endif /* _POWER */
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include "routes.h" 

extern int routeNumber;
extern int ifNumber;
extern int flush_rt_cache;

/*  */

#define	FORW_GATEWAY	1		/* ipForwarding */
#define	FORW_HOST	2
static int	ipforwarding;

static struct ipstat ipstat;

/*  */

#define	IPFORWARDING	0
#define	IPDEFAULTTTL	1
#if	defined(BSD43) || defined(AIX)
#define	IPINRECEIVES	2
#endif
#define	IPINHDRERRORS	3
#ifdef _POWER
#define	IPINADDRERRORS	4
#else
#undef	IPINADDRERRORS	4		/* NOT IMPLEMENTED */
#endif
#if	defined(BSD43) || defined(AIX)
#define	IPFORWDATAGRAMS	5
#endif
#if	defined(BSD44) || defined(_POWER)
#define	IPINUNKNOWNPROTOS 6
#endif
#ifdef _POWER
#define	IPINDISCARDS	7
#else
#undef	IPINDISCARDS	7		/* NOT IMPLEMENTED */
#endif
#if	defined(BSD44) || defined(_POWER)
#define	IPINDELIVERS	8
#define	IPOUTREQUESTS	9
#define	IPOUTDISCARDS	10
#endif
#if	defined(BSD43) || defined(AIX)
#define	IPOUTNOROUTES	11
#endif	
#define	IPREASMTIMEOUT	12
#if	defined(BSD43) || defined(AIX)
#define	IPREASMREQDS	13
#endif
#if	defined(BSD44) || defined(_POWER)
#define	IPREASMOKS	14
#endif
#if	defined(BSD43) || defined(AIX)
#define	IPREASMFAILS	15
#endif
#ifdef	BSD44
#undef	IPFRAGOKS
#undef	IPFRAGFAILS
#undef	IPFRAGCREATES
#endif
#if	defined(_POWER)
#define	IPFRAGOKS	16
#define	IPFRAGFAILS	17
#define	IPFRAGCREATES	18
#endif
#define	IPROUTINGDISCARDS	19

#if	defined(_POWER) && !defined(BSD44)
#define	ips_noproto	ipInUnknownProtos
#define	ips_delivered	ipInDelivers
#define	ips_localout	ipOutRequests
#define	ips_odropped	ipOutDiscards
#define	ips_reassembled	ipReasmOKs
#define	ips_fragmented	ipFragOKs
#define	ips_cantfrag	ipFragFails
#define	ips_ofragments	ipFragCreates
#endif

#if	defined(_POWER) && defined(BSD44)
extern	int stimeout;
extern	int flush_if_cache;
#endif

static int
read_ipstat ()
{
    register struct ipstat *ips = &ipstat;
    static   int lastq = -1;

    if (quantum != lastq) {
	lastq = quantum;
	if (getkmem (nl + N_IPSTAT, (caddr_t) ips, sizeof *ips) == NOTOK)
	    return int_SNMP_error__status_genErr;
#ifndef _POWER
	if (getkmem (nl + N_IPFORWARDING, (caddr_t) &ipforwarding, 
		sizeof ipforwarding) == NOTOK)
	    return int_SNMP_error__status_genErr;
#endif
    }
    return int_SNMP_error__status_noError;
}

static int  
o_ip (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register struct ipstat *ips = &ipstat;
    register OT	    ot = oi -> oi_type;
    int	rc;

    ifvar = (int) ot -> ot_info;
    if ((rc = o_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    switch (ifvar) {
	case IPDEFAULTTTL:
	case IPREASMTIMEOUT:
	    break;
	    
	default:
	    if ((rc = read_ipstat ()) != int_SNMP_error__status_noError)
		return rc;
	    break;
    }

    switch (ifvar) {
#ifndef _POWER		/* handled separately */
	case IPFORWARDING:
	    return o_integer (oi, v, ipforwarding ? FORW_GATEWAY : FORW_HOST);

	case IPDEFAULTTTL:
	    return o_integer (oi, v, MAXTTL);		/* no kernel info */
#endif

#ifdef	IPINRECEIVES
	case IPINRECEIVES:
#  ifdef _POWER
	    return o_integer (oi, v, ips -> ips_total + ips -> ipInDiscards);
#  else
	    return o_integer (oi, v, ips -> ips_total);
#  endif /* _POWER */
#endif

	case IPINHDRERRORS:
#ifdef _POWER
	    return o_integer (oi, v, ips -> ips_badsum
			           + ips -> ips_tooshort
				   + ips -> ips_toosmall
				   + ips -> ips_badhlen
				   + ips -> ips_badlen
				   /* in addition to other ipstat above */
				   + ips -> ipInHdrErrors);
#else
	    return o_integer (oi, v, ips -> ips_badsum
			           + ips -> ips_tooshort
				   + ips -> ips_toosmall
				   + ips -> ips_badhlen
				   + ips -> ips_badlen);
#endif

#ifdef	IPINADDRERRORS
	case IPINADDRERRORS:
	    return o_integer (oi, v, ips -> ipInAddrErrors);
#endif

#ifdef	IPFORWDATAGRAMS
	case IPFORWDATAGRAMS:
	    return o_integer (oi, v, ips -> ips_forward);
#endif

#ifdef	IPINUNKNOWNPROTOS
	case IPINUNKNOWNPROTOS:
	    return o_integer (oi, v, ips -> ips_noproto);
#endif

#ifdef	IPINDISCARDS
	case IPINDISCARDS:
	    return o_integer (oi, v, ips -> ipInDiscards);
#endif

#ifdef	IPINDELIVERS
	case IPINDELIVERS:
	    return o_integer (oi, v, ips -> ips_delivered);
#endif

#ifdef	IPOUTREQUESTS
	case IPOUTREQUESTS:
	    return o_integer (oi, v, ips -> ips_localout);
#endif

#ifdef	IPOUTDISCARDS
	case IPOUTDISCARDS:
	    return o_integer (oi, v, ips -> ips_odropped);
#endif

#ifdef	IPOUTNOROUTES
	case IPOUTNOROUTES:
	    return o_integer (oi, v, ips -> ips_cantforward);
#endif

	case IPREASMTIMEOUT:
	    return o_integer (oi, v, IPFRAGTTL);

#ifdef	IPREASMREQDS
	case IPREASMREQDS:
	    return o_integer (oi, v, ips -> ips_fragments);
#endif

#ifdef	IPREASMOKS
	case IPREASMOKS:
	    return o_integer (oi, v, ips -> ips_reassembled);
#endif

#ifdef	IPREASMFAILS
	case IPREASMFAILS:
	    return o_integer (oi, v, ips -> ips_fragdropped
			           + ips -> ips_fragtimeout);
#endif

#ifdef	IPFRAGOKS
	case IPFRAGOKS:
	    return o_integer (oi, v, ips -> ips_fragmented);
#endif

#ifdef	IPFRAGFAILS
	case IPFRAGFAILS:
	    return o_integer (oi, v, ips -> ips_cantfrag);
#endif

#ifdef	IPFRAGCREATES
	case IPFRAGCREATES:
	    return o_integer (oi, v, ips -> ips_ofragments);
#endif
	/*
	 * The protocol for discarding routes due to resource problems is
	 * not implemented in the tcpip kernel. So ipRoutingDiscards is hard 
	 * coded to 0.
	 */
	case IPROUTINGDISCARDS:
	    return o_integer (oi, v, 0);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

#ifdef _POWER

/*
 *	FUNCTION: o_ip_default_ttl ()
 *
 *	PURPOSE: get mib variable ipDefaultTTL
 */
static int  
o_ip_default_ttl (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    struct optreq 	optreq;
    int	rc;

    if ((rc = o_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    optreq.getnext = 0;
    strcpy (optreq.name, "maxttl");
    if (ioctl (Nd, SIOCGNETOPT, &optreq) < 0) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_1,
		"GENERR: %s: ioctl (%s) for \"%s\" failed: %s"),
		"o_ip_default_ttl", "SIOCGNETOPT", optreq.name,strerror(errno));
	return int_SNMP_error__status_genErr;
    }

    return o_integer (oi, v, atoi (optreq.data));
}

/*
 *	FUNCTION: s_ip_default_ttl ()
 *
 *	PURPOSE: set mib variable ipDefaultTTL
 */
static int  
s_ip_default_ttl (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    struct optreq 	optreq;
    int	rc, ttl;

    if ((rc = s_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    switch (offset) {
	case type_SNMP_PDUs_set__request:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    break;

	case type_SNMP_PDUs_commit:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    memset (&optreq, 0, sizeof (optreq));
	    strcpy (optreq.name, "maxttl");
	    ttl = *((int *)(ot -> ot_save));
	    bcopy (&ttl, optreq.data, sizeof (int));
	    if (ioctl (Nd, SIOCSNETOPT, &optreq) < 0) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_1,
			"GENERR:%s: ioctl (%s) for \"%s\" failed: %s"),
			"s_ip_default_ttl", "SIOCSNETOPT", optreq.name, 
			strerror(errno));
		return int_SNMP_error__status_genErr;
	    }
	    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;

	case type_SNMP_PDUs_rollback:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;
    }
    return int_SNMP_error__status_noError;
}

/*
 *	FUNCTION: o_ip_forwarding ()
 *
 *	PURPOSE: get mib variable ipForwarding
 */
static int  
o_ip_forwarding (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register struct interface *is;
    register struct ifnet *ifn;
    struct optreq 	optreq;
    int	rc;

    if ((rc = o_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    /* check the kernel variable ipforwarding first */
    optreq.getnext = 0;
    strcpy (optreq.name, "ipforwarding");
    if (ioctl (Nd, SIOCGNETOPT, &optreq) < 0) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_1,
		"GENERR: %s: ioctl (%s) for \"%s\" failed: %s"),
		"o_ip_forwarding", "SIOCGNETOPT", optreq.name, strerror(errno));
	return int_SNMP_error__status_genErr;
    }

    /* stop point 1.  We are a host if ipforwarding == 0 */
    if (atoi (optreq.data) == 0)
	return o_integer (oi, v, FORW_HOST);
    return o_integer (oi, v, FORW_GATEWAY);
}

/*
 *	FUNCTION: s_ip_forwarding ()
 *
 *	PURPOSE: set mib variable ipForwarding
 */
static int  
s_ip_forwarding (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    struct optreq 	optreq;
    int	rc, forw;

    if ((rc = s_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    switch (offset) {

	/* decode and check our value */
	case type_SNMP_PDUs_set__request:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    forw = *((int *)(ot -> ot_save));
	    if (forw != FORW_GATEWAY && forw != FORW_HOST)
		return int_SNMP_error__status_badValue;
	    break;

	/* perform the ioctl */
	case type_SNMP_PDUs_commit:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    memset (&optreq, 0, sizeof (optreq));
	    strcpy (optreq.name, "ipforwarding");
	    forw = *((int *)(ot -> ot_save));
	    forw = forw == FORW_HOST ? 0 : forw;	/* map to kernel */
	    bcopy (&forw, optreq.data, sizeof (int));
	    if (ioctl (Nd, SIOCSNETOPT, &optreq) < 0) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_1,
			"GENERR:%s: ioctl (%s) for \"%s\" failed: %s"),
			"s_ip_forwarding", "SIOCSNETOPT", optreq.name, 
			strerror(errno));
		return int_SNMP_error__status_genErr;
	    }
	    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;

	case type_SNMP_PDUs_rollback:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;
    }
    return int_SNMP_error__status_noError;
}
#endif

/*  */

#ifndef	IP_MAXPACKET
#define	IP_MAXPACKET	65535		/* IPADENTREASMMAXSIZE */
#endif


#define	IFN_SIZE	4

/*  */

#define	IPADENTADDR	0
#define	IPADENTIFINDEX	1
#define	IPADENTNETMASK	2
#define	IPADENTBCASTADDR 3
#define	IPADENTREASMMAXSIZE 4


static int  
o_ip_addr (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register int   i;
    int	    ifvar;
    register unsigned int *ip,
			  *jp;
    register struct address   *as;
    register OID    oid = oi -> oi_name;
    OID		new;
    register OT	    ot = oi -> oi_type;

    if (get_interfaces (type_SNMP_PDUs_get__request) == NOTOK)
	return generr (offset);

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + IFN_SIZE)
		return int_SNMP_error__status_noSuchName;
	    if ((as = get_addrent (oid -> oid_elements + oid -> oid_nelem
				   - IFN_SIZE, IFN_SIZE, afs_inet, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if ((i = oid -> oid_nelem - ot -> ot_name -> oid_nelem) != 0
		    && i < IFN_SIZE) {
		for (jp = (ip = oid -> oid_elements + 
			ot -> ot_name -> oid_nelem - 1) + i;
			jp > ip;
			jp--) 
		    if (*jp != 0)
			break;
		if (jp == ip)
		    oid -> oid_nelem = ot -> ot_name -> oid_nelem;
		else {
		    if ((new = oid_normalize (oid, IFN_SIZE - i, 256))
			    == NULLOID)
			return NOTOK;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = oid = new;
		}
	    }
	    else
		if (i > IFN_SIZE)
		    oid -> oid_nelem = ot -> ot_name -> oid_nelem + IFN_SIZE;

	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		if ((as = afs_inet) == NULL)
		    return NOTOK;

		if ((new = oid_extend (oid, IFN_SIZE)) == NULLOID)
		    return int_SNMP_error__status_genErr;
		ip = new -> oid_elements + new -> oid_nelem - IFN_SIZE;
		jp = as -> adr_instance;
		for (i = IFN_SIZE; i> 0; i--)
		    *ip++ = *jp++;
		
		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		if ((as = get_addrent (ip = oid -> oid_elements
				      	    + oid -> oid_nelem - IFN_SIZE,
				       IFN_SIZE, afs_inet, 1)) == NULL)
		    return NOTOK;

		jp = as -> adr_instance;
		for (i = IFN_SIZE; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case IPADENTADDR:
	    return o_ipaddr (oi, v, (struct sockaddr_in *) &as -> adr_address);

	case IPADENTIFINDEX:
	    return o_integer (oi, v, ffs (as -> adr_indexmask));

	case IPADENTNETMASK:
	    return o_ipaddr (oi, v, (struct sockaddr_in *) &as -> adr_netmask);

	case IPADENTBCASTADDR:	/* beyond belief! */
	    {
		u_long a =  (((struct sockaddr_in *) &as -> adr_netmask)
			       				    -> sin_addr.s_addr)
			          & ~(((struct sockaddr_in *) &as
					-> adr_broadaddr) -> sin_addr.s_addr);

		return o_integer (oi, v, a ? 1 : 0);
	    }

	case IPADENTREASMMAXSIZE:
	    return o_integer (oi, v, IP_MAXPACKET);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}
/* #endif */

/*  */

/* BEGIN ROUTE TABLE SECTION */

#define	IPROUTEDEST	0
#define	IPROUTEIFINDEX	1
#define	IPROUTEMETRIC1	2
#define	IPROUTEMETRIC2	3
#define	IPROUTEMETRIC3	4
#define	IPROUTEMETRIC4	5
#define	IPROUTENEXTHOP	6
#define	IPROUTETYPE	7
#define	IPROUTEPROTO	8
#define	IPROUTEAGE	9
#define	IPROUTEMASK	10
#define IPROUTEMETRIC5	11
#define IPROUTEINFO	12
#define	UNIXIPROUTEFLAGS 13
#define	UNIXIPROUTEREFCNT 14
#define	UNIXIPROUTEUSES	15


static int  
o_ip_route (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct rtetab *rt;
    register OID    oid = oi -> oi_name;
    OID	new;
    register OT	    ot = oi -> oi_type;
 
#if 	defined(_POWER) && defined(BSD44) 
    struct timeval current_time;
#endif

    if (get_routes (offset) == NOTOK)
	return generr (offset);

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + IFN_SIZE)
		return int_SNMP_error__status_noSuchName;
	    if ((rt = get_rtent (oid -> oid_elements + oid -> oid_nelem
				 	- IFN_SIZE, IFN_SIZE, rts_inet, 0))
		    == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if ((i = oid -> oid_nelem - ot -> ot_name -> oid_nelem) != 0
		    && i < IFN_SIZE) {
		for (jp = (ip = oid -> oid_elements + 
			ot -> ot_name -> oid_nelem - 1) + i;
			jp > ip;
			jp--) 
		    if (*jp != 0)
			break;
		if (jp == ip)
		    oid -> oid_nelem = ot -> ot_name -> oid_nelem;
		else {
		    if ((new = oid_normalize (oid, IFN_SIZE - i, 256))
			    == NULLOID)
			return NOTOK;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = oid = new;
		}
	    }

	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		if ((rt = rts_inet) == NULL)
		    return NOTOK;

		if ((new = oid_extend (oid, IFN_SIZE)) == NULLOID)
		    return int_SNMP_error__status_genErr;
		ip = new -> oid_elements + new -> oid_nelem - rt -> rt_insize;
		jp = rt -> rt_instance;
		for (i = rt -> rt_insize; i > 0; i--)
		    *ip++ = *jp++;
		
		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int j;

		if ((rt = get_rtent (ip = oid -> oid_elements
					    + ot -> ot_name -> oid_nelem,
				     j = oid -> oid_nelem
					    - ot -> ot_name -> oid_nelem,
				     rts_inet, 1)) == NULL)
		    return NOTOK;

		if ((i = j - rt -> rt_insize) < 0) {
		    if ((new = oid_extend (oid, -i)) == NULLOID)
			return int_SNMP_error__status_genErr;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = oid = new;
		}
		else
		    if (i > 0)
			oid -> oid_nelem -= i;
		
		ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
		jp = rt -> rt_instance;
		for (i = rt -> rt_insize; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case IPROUTEDEST:
	    return o_ipaddr (oi, v, (struct sockaddr_in *) &rt -> rt_dst);

	case IPROUTEIFINDEX:
	    {
		register struct interface *is;

		for (is = ifs; is; is = is -> ifn_next)
		    if ((caddr_t) is -> ifn_offset
			        == (caddr_t) rt -> rt_rt.rt_ifp) {
			if (is -> ifn_ready)
			    return o_integer (oi, v, is -> ifn_index);
			break;
		    }

		if (offset == type_SNMP_PDUs_get__next__request)
		    return NOTOK;
		return int_SNMP_error__status_noSuchName;
	    }
	    
	case IPROUTEMETRIC1:
	case IPROUTEMETRIC2:
	case IPROUTEMETRIC3:
	case IPROUTEMETRIC4:
	case IPROUTEMETRIC5:
	    return o_integer (oi, v, METRIC_NONE);

	case IPROUTENEXTHOP:
	    return o_ipaddr (oi, v, (struct sockaddr_in *) &rt -> rt_gateway);

	case IPROUTETYPE:
	    return o_integer (oi, v, rt -> rt_type);

	case IPROUTEPROTO:
#ifdef	RTF_DYNAMIC
#ifndef	RTF_MODIFIED
	    if (rt -> rt_rt.rt_flags & RTF_DYNAMIC)
#else
	    if (rt -> rt_rt.rt_flags & (RTF_DYNAMIC | RTF_MODIFIED))
#endif
		return o_integer (oi, v, PROTO_ICMP);
	    else
#endif
		return o_integer (oi, v, PROTO_OTHER);

	case IPROUTEAGE:
#if 	defined(_POWER) && defined(BSD44) 
            if (gettimeofday (&current_time, (struct timezone *) 0) == NOTOK) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_3,
			"GENERR: gettimeofday"));
	        return int_SNMP_error__status_genErr;
	    }
	    return o_integer (oi, v, current_time.tv_sec - 
				     rt -> rt_rt.ipRouteAge);
#else
	    return o_integer (oi, v, 0);
#endif

	case IPROUTEMASK:
	    {
		struct sockaddr_in mask;
		struct sockaddr_in *sin = (struct sockaddr_in *) &rt -> rt_dst;
#ifndef BSD44
		register struct interface *is;
		register struct address *as;
#endif

		bzero ((char *) &mask, sizeof mask);
		if (rt -> rt_rt.rt_flags & RTF_HOST)
		    mask.sin_addr.s_addr = (u_long) 0xffffffff;
		else
		    if (sin -> sin_addr.s_addr != 0L) {
#ifndef BSD44
			/* non BSD44, look at netmask on associated interface */
			if (get_interfaces (type_SNMP_PDUs_get__request)
				== NOTOK)
			    return generr (offset);
			
			for (is = ifs; is; is = is -> ifn_next)
			    if ((caddr_t) is -> ifn_offset
				    == (caddr_t) rt -> rt_rt.rt_ifp)
				break;

			if (!is || !is -> ifn_ready)
			    return int_SNMP_error__status_noSuchName;

			for (as = afs_inet; as; as = as -> adr_next)
			    if (is -> ifn_indexmask & as -> adr_indexmask)
				break;

			if (!as)
			    return int_SNMP_error__status_noSuchName;
			
			mask.sin_addr.s_addr = 
				as -> adr_netmask.un_in.sin_addr.s_addr;
#else
			/* BSD44 has a mask associated with each route */

			if (rt -> rt_rmsa)
			    mask.sin_addr.s_addr =
				((struct sockaddr_in *) rt -> rt_rmsa) -> sin_addr.s_addr;

#endif
		}

		return o_ipaddr (oi, v, &mask);
	    }

	case IPROUTEINFO:
	    return o_specific (oi, v, (caddr_t) nullSpecific);

#ifdef UNIXGROUP
	case UNIXIPROUTEFLAGS:
	    return o_integer (oi, v, rt -> rt_rt.rt_flags & 0xffff);

	case UNIXIPROUTEREFCNT:
	    return o_integer (oi, v, rt -> rt_rt.rt_refcnt & 0xffff);

	case UNIXIPROUTEUSES:
	    return o_integer (oi, v, rt -> rt_rt.rt_use);
#endif /* UNIXGROUP */

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/* SET ROUTE TABLE (begin) */

/*
 *	FUNCTION: s_ip_route ()
 *
 *	PURPOSE: set an iproute entry
 */
static int  
s_ip_route (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS     os = ot -> ot_syntax;
    register unsigned int *ip;
    int	    ifvar;
    int		i;
    int	    status = int_SNMP_error__status_noError;
    register struct rtetab *rt;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_set__request:

	    /* only need to query the kernel once */
	    if (get_routes (type_SNMP_PDUs_get__request) == NOTOK) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_4,
			"GENERR: s_ip_route: get_routes failed"));
		return int_SNMP_error__status_genErr;
	    }

	    /* FALL THRU */

	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:

	    /* validate instance */
	    if (oid -> oid_nelem < ot -> ot_name -> oid_nelem + IFN_SIZE)
		return int_SNMP_error__status_noSuchName;
	    if (rt = get_rtent (oid -> oid_elements 
                                      + ot -> ot_name -> oid_nelem,
                                  i = oid -> oid_nelem 
                                      - ot -> ot_name -> oid_nelem,
                                  rts_inet, 0)) {
                if (!(rt -> rt_sflags & ST_INSET))      /* mark in_progress */
                    rt -> rt_sflags |= (ST_EXISTS|ST_INSET|ST_ADDSET|ST_DELETE);
                break;
            }
  
            if (offset != type_SNMP_PDUs_set__request)
                return int_SNMP_error__status_genErr;
  
            switch (i) {
                case IFN_SIZE:
                case IFN_SIZE + 1:
                    break;
  
                default:
                    return int_SNMP_error__status_noSuchName;
            }
            ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
	      
            if ((rt = (struct rtetab *) calloc (1, sizeof *rt)) == NULL) {
              advise(SLOG_EXCEPTIONS, "Out of memory creating routing entry");
              return int_SNMP_error__status_genErr;
            }

            if (i > IFN_SIZE) {
              register struct rtetab *rz;

              for (rz = rts_inet; rz; rz = rz -> rt_next)
                if (rz -> rt_dst.sa.sa_family == AF_INET 
                     && elem_cmp (rz -> rt_instance, rz -> rt_insize,
                                  ip, IFN_SIZE) == 0) {
                  if (ip[i - 1] != rz -> rt_magic + 1) {
bad_magic: ;
                    free((char *) rt);
                    return int_SNMP_error__status_noSuchName;
                  }
                  rz -> rt_magic++;
		  break;
                }
              if (!rz)
                goto bad_magic;
            }
  
            bcopy ((char *) ip, (char *) rt -> rt_instance, 
                   (rt -> rt_insize = i) * sizeof *ip);
  
            rt -> rt_srt.rt_flags |= RTF_UP;
            rt -> rt_srt.rt_flags |= RTF_HOST;
            {
              register u_char *cp;
              register unsigned int *jp,
                                    *kp;
                struct sockaddr_in *sin;
  
              sin = (struct sockaddr_in *) &rt -> rt_dst;
              sin -> sin_family = AF_INET;
              cp = (u_char *) &sin -> sin_addr;
              for (kp = (jp = ip) + IFN_SIZE; jp < kp; )
                *cp++ = *jp++ & 0xff;
              rt -> rt_srt.rt_dst = *((struct sockaddr *) &rt -> rt_dst);
  
              sin = (struct sockaddr_in *) &rt -> rt_gateway;
              sin -> sin_family = AF_INET;
              rt -> rt_srt.rt_gateway = *((struct sockaddr *) 
                                            &rt -> rt_gateway);
            }
  
            rt -> rt_stab.st_forw = rt -> rt_stab.st_back = &rt -> rt_stab;
            rt -> rt_type = RTYPE_OTHER;
            if (!(rt -> rt_sflags & ST_INSET))       /* mark in_progress */
                rt -> rt_sflags |= (ST_INSET | ST_ADDSET | ST_NEWSET);
  
            rt -> rt_next = rts_inet; rts_inet = rt; routeNumber++;
            /* Add sort of rt to make magic calculations easier        */
            /* We do not need to do this.  The kernel does not support */
            /* dest address currently multiple times.                  */
	    break;

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_5,
		    "GENERR in s_ip_route: bad offset %d"), offset);
	    return int_SNMP_error__status_genErr;
    }

    if (os == NULLOS) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_6,
		"GENERR: s_address: no syntax defined for \"%s\""),
		ot -> ot_text);
	return int_SNMP_error__status_genErr;
    }

    /* Mark route as going to change or something */
    rt -> rt_touched = 1;

    switch (ifvar) {
	case IPROUTEDEST:
	    status = sr_dest (oi, v, offset, rt);
	    break;

	case IPROUTEIFINDEX:
	    status = sr_ifindex (oi, v, offset, rt);
	    break;

	case IPROUTENEXTHOP:
	    status = sr_nexthop (oi, v, offset, rt);
	    break;

	case IPROUTETYPE:
	    status = sr_type (oi, v, offset, rt);
	    break;

	case IPROUTEMASK:
	    status = sr_mask (oi, v, offset, rt);
	    break;

	case IPROUTEMETRIC1:
	case IPROUTEMETRIC2:
	case IPROUTEMETRIC3:
	case IPROUTEMETRIC4:
	case IPROUTEAGE:
	    status = sr_noop (oi, v, offset, rt);
	    break;
    }
    return status;
}

/*
 *	FUNCTION: sr_commit ()
 *
 *	PURPOSE: commit a set on a route entry
 */
static int
sr_commit (rt)
struct rtetab *rt;
{
    struct settab 	*st;
    struct qbuf		*qb;

    if (rt -> rt_sflags & ST_ISSET) {
#ifdef DEBUG
	if (debug > 3) {
	    char buf[20];

	    strcpy (buf, inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_dst) -> sin_addr));
	    /* NOT IN CATALOG, BECAUSE IT IS DEBUG */
	    advise (SLOG_DEBUG,
		    "entry dest: %s, gateway: %s, already set",
		    buf, inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_gateway) -> sin_addr));
	}
#endif
	return int_SNMP_error__status_noError;
    }
    rt -> rt_sflags |= ST_ISSET;
    rt -> rt_touched = 0;

    /* IPROUTEDEST */
    if ((st = find_set (&rt -> rt_stab, IPROUTEDEST)) 
	    != (struct settab *) NULL) {
	qb = ((struct qbuf *) st -> st_value) -> qb_forw;
	bcopy (qb -> qb_data,
	    (char *) &((struct sockaddr_in *) &rt -> rt_srt.rt_dst) -> sin_addr,
	    qb -> qb_len);
    }

    /* IPROUTENEXTHOP */
    if ((st = find_set (&rt -> rt_stab, IPROUTENEXTHOP)) 
	    != (struct settab *) NULL) {
	qb = ((struct qbuf *) st -> st_value) -> qb_forw;
	bcopy (qb -> qb_data,
	    (char *) &((struct sockaddr_in *) &rt -> rt_srt.rt_gateway) -> sin_addr,
	    qb -> qb_len);
    }

    /* IPROUTETYPE */
    if ((st = find_set (&rt -> rt_stab, IPROUTETYPE)) 
	    != (struct settab *) NULL) {
	int type = *((int *)(st -> st_value));
	
	rt -> rt_type = type;
	switch (type) {
	    case RTYPE_DIRECT:
	      rt -> rt_srt.rt_flags &= ~RTF_GATEWAY;
	      break;
	    case RTYPE_REMOTE:
	      rt -> rt_srt.rt_flags |= RTF_GATEWAY;
	      break;
	}
    }

    /* IPROUTEIFNDEX */
    if ((st = find_set (&rt -> rt_stab, IPROUTETYPE)) 
          != (struct settab *) NULL) {
      int i = *((int *)(st -> st_value));
      register struct interface *is;

      for (is = ifs; is; is = is -> ifn_next)
        if (is -> ifn_ready && is -> ifn_index == i)
          break;
      rt -> rt_srt.rt_ifp = (struct ifnet *) is -> ifn_offset;
    }

    /* IPROUTEMASK */
    if ((st = find_set (&rt -> rt_stab, IPROUTEMASK)) 
          != (struct settab *) NULL) {
      struct in_addr mask;

      qb = ((struct qbuf *) st -> st_value) -> qb_forw;
      bcopy (qb -> qb_data,
             (char *) &((struct sockaddr_in *) &mask) -> sin_addr,
             qb -> qb_len);
      if (mask.s_addr == (u_long) 0xffffffff)
      rt -> rt_srt.rt_flags |= RTF_HOST;
      else
      rt -> rt_srt.rt_flags &= ~RTF_HOST;
    }

    /* delete old route */
    if (rt -> rt_sflags & ST_DELETE) {
	struct RTENTRY ort;

#ifdef DEBUG 
	/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
	if (debug > 3)
	    advise (SLOG_DEBUG, "sr_commit: deleting: %s",
		    inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_dst) -> sin_addr));
#endif
	/* XXX: BSD44 should really use routing socket instead... */
	ort.rt_dst = rt -> rt_dst.sa;		/* struct copy */
	ort.rt_gateway = rt -> rt_gateway.sa;	/* ... */
	ort.rt_flags = rt -> rt_host;
	if (ioctl (Nd, SIOCDELRT, (caddr_t) & ort) == NOTOK) {
	    char buf[20];

	    strcpy (buf, 
		    inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_gateway) -> sin_addr));
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_7,
	     "GENERR: sr_commit: SIOCDELRT of dest: %s gateway: %s failed: %s"),
		    inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_dst) -> sin_addr), buf, strerror(errno));
	    return int_SNMP_error__status_genErr;
	}
    }

    /* add new route */
    if (rt -> rt_sflags & ST_ADDSET) {
#ifdef DEBUG
	/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
	if (debug > 3)
	    advise (SLOG_DEBUG, "sr_commit: adding: %s",
		    inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_srt.rt_dst) -> sin_addr));
#endif
	if (ioctl (Nd, SIOCADDRT, (caddr_t) & rt -> rt_srt) == NOTOK) {
	    char buf[20];

	    strcpy (buf, 
		    inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_srt.rt_gateway) -> sin_addr));
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_8,
	     "GENERR: sr_commit: SIOCADDRT of dest: %s gateway: %s failed: %s"),
		    inet_ntoa (((struct sockaddr_in *) 
			&rt -> rt_srt.rt_dst) -> sin_addr),buf,strerror(errno));
	    return int_SNMP_error__status_genErr;
	}
    }
    flush_rt_cache = 1;
    return int_SNMP_error__status_noError;
}

/*
 *	FUNCTION: sr_dest ()
 *
 *	PURPOSE: set route_table destination field.
 */
static int
sr_dest (oi, v, offset, rt)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct rtetab *rt;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct settab 	*st;
    int 		status = int_SNMP_error__status_noError;
    int			type;
    struct qbuf		*qb;

    switch (offset) {

	case type_SNMP_PDUs_set__request:

	    /* check for conflicts with delete */
	    if ((st = find_set (&rt -> rt_stab, IPROUTETYPE)) 
		    != (struct settab *) NULL) {
		type = *((int *)(st -> st_value));
		if (type == RTYPE_INVALID)
		    return int_SNMP_error__status_badValue;
	    }

	    /* parse */
	    if ((status = 
		    parse_set (&st, &rt -> rt_stab, IPROUTEDEST, os, v) )
		    != int_SNMP_error__status_noError) 
		return status;

	    /* ipAddress limited to 4 bytes. */
	    if ((qb = ((struct qbuf *) st -> st_value) -> qb_forw)
		    -> qb_len != 4)
		return int_SNMP_error__status_badValue;
	    
#ifdef DEBUG
	    if (debug) {
		char buf[20];

		strcpy (buf, 
			inet_ntoa (((struct sockaddr_in *) 
			    &rt -> rt_dst) -> sin_addr));
		/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
		advise (SLOG_DEBUG,
			"dest change request from %s to %s",
			buf, inet_ntoa (*((unsigned long *) qb -> qb_data)) );
	    }
#endif
	    break;

	case type_SNMP_PDUs_commit:
	    status = sr_commit (rt);
	    free_set (&rt -> rt_stab, IPROUTEDEST, os);
	    break;

	case type_SNMP_PDUs_rollback:
	    free_set (&rt -> rt_stab, IPROUTEDEST, os);
	    rt -> rt_touched = 0;
	    break;
    }
    return status;
}

/*
 *	FUNCTION: sr_ifindex ()
 *
 *	PURPOSE: set route_table ifindex field.
 */
static int
sr_ifindex (oi, v, offset, rt)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct rtetab *rt;
{
  register OT         ot = oi -> oi_type;
  register OS         os = ot -> ot_syntax;
  register struct interface *is;
  int i;
  int type;
  struct settab       *st;
  int                 status = int_SNMP_error__status_noError;

  switch (offset) {
    case type_SNMP_PDUs_set__request:

          /* check for conflicts with delete */
          if ((st = find_set (&rt -> rt_stab, IPROUTETYPE)) 
                  != (struct settab *) NULL) {
              type = *((int *)(st -> st_value));
              if (type == RTYPE_INVALID)
                  return int_SNMP_error__status_badValue;
          }

          /* parse */
          if ((status = 
                  parse_set (&st, &rt -> rt_stab, IPROUTEIFINDEX, os, v) )
                  != int_SNMP_error__status_noError) 
              return status;

          i = *((int *)(st -> st_value));

            for (is = ifs; is; is = is -> ifn_next)
              if (is -> ifn_ready && is -> ifn_index == i)
                break;
            if (!is)
              return int_SNMP_error__status_badValue;
            break;

    case type_SNMP_PDUs_commit:
          status = sr_commit (rt);
          free_set (&rt -> rt_stab, IPROUTEIFINDEX, os);
          break;

    case type_SNMP_PDUs_rollback:
          free_set (&rt -> rt_stab, IPROUTEIFINDEX, os);
          rt -> rt_touched = 0;
          break;
  }
  return status;
}

/*
 *	FUNCTION: sr_nexthop ()
 *
 *	PURPOSE: set route_table gateway field.
 */
static int
sr_nexthop (oi, v, offset, rt)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct rtetab *rt;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct settab 	*st;
    int 		status = int_SNMP_error__status_noError;
    int			type;
    struct qbuf		*qb;
    struct sockaddr_in	sin;

    switch (offset) {

	case type_SNMP_PDUs_set__request:

	    /* check for conflicts with delete */
	    if ((st = find_set (&rt -> rt_stab, IPROUTETYPE)) 
		    != (struct settab *) NULL) {
		type = *((int *)(st -> st_value));
		if (type == RTYPE_INVALID)
		    return int_SNMP_error__status_badValue;
	    }

	    /* parse */
	    if ((status = 
		    parse_set (&st, &rt -> rt_stab, IPROUTENEXTHOP, os, v) )
		    != int_SNMP_error__status_noError) 
		return status;

	    /* ipAddress limited to 4 bytes. */
	    if ((qb = ((struct qbuf *) st -> st_value) -> qb_forw)
		    -> qb_len != 4)
		return int_SNMP_error__status_badValue;

	    /* check if this address is reachable form here */
	    bcopy (qb -> qb_data, (char *) &sin.sin_addr, qb -> qb_len);
	    if (ifa_ifwithnet (sin.sin_addr) == (struct address *) NULL) {
#ifdef DEBUG
		/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
		advise (SLOG_DEBUG, "NextHop %s not reachable",
			inet_ntoa (*((unsigned long *) qb -> qb_data)) );
#endif
		return int_SNMP_error__status_badValue;
	    }
#ifdef DEBUG
	    if (debug) {
		char buf[20];

		strcpy (buf, 
			inet_ntoa (((struct sockaddr_in *) 
			    &rt -> rt_gateway) -> sin_addr));
		/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
		advise (SLOG_DEBUG,
			"NextHop change request from %s to %s",
			buf, inet_ntoa (*((unsigned long *) qb -> qb_data)) );
	    }
#endif
	    break;

	case type_SNMP_PDUs_commit:
	    status = sr_commit (rt);
	    free_set (&rt -> rt_stab, IPROUTENEXTHOP, os);
	    break;

	case type_SNMP_PDUs_rollback:
	    free_set (&rt -> rt_stab, IPROUTENEXTHOP, os);
	    rt -> rt_touched = 0;
	    break;
    }

    return status;
}

/*
 *	FUNCTION: sr_type ()
 *
 *	PURPOSE: set route_table type field.
 */
static int
sr_type (oi, v, offset, rt)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct rtetab *rt;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct settab *st;
    int status = int_SNMP_error__status_noError;
    int type;

    switch (offset) {

	case type_SNMP_PDUs_set__request:

	    /* parse */
	    if ((status = parse_set (&st, &rt -> rt_stab, IPROUTETYPE, os, v) )
		    != int_SNMP_error__status_noError) 
		return status;

	    /* check desired value */
	    type = *((int *)(st -> st_value));
	    if (type < RTYPE_MIN || type > RTYPE_MAX)
		return int_SNMP_error__status_badValue;

	    if (type == RTYPE_INVALID) {

		/* if a delete, this must be the only variable set */
		if (st -> st_forw != st -> st_back)
		    return int_SNMP_error__status_badValue;
		if (rt -> rt_sflags & ST_NEWSET)
		    return int_SNMP_error__status_badValue;
		rt -> rt_sflags &= ~ST_ADDSET;	/* dont add new */
	    }

            if (type == RTYPE_OTHER) 
              return int_SNMP_error__status_badValue;

	    break;

	case type_SNMP_PDUs_commit:
	    status = sr_commit (rt);
	    free_set (&rt -> rt_stab, IPROUTETYPE, os);
	    break;

	case type_SNMP_PDUs_rollback:
	    free_set (&rt -> rt_stab, IPROUTETYPE, os);
            rt -> rt_touched = 0;
	    break;
    }
    return status;
}

/*
 *	FUNCTION: sr_mask ()
 *
 *	PURPOSE: set route_table mask field.
 */
static int
sr_mask (oi, v, offset, rt)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct rtetab *rt;
{
  register OT         ot = oi -> oi_type;
  register OS         os = ot -> ot_syntax;
  struct settab       *st;
  int                 status = int_SNMP_error__status_noError;
  int type;

  switch (offset) {
    case type_SNMP_PDUs_set__request:

          /* check for conflicts with delete */
          if ((st = find_set (&rt -> rt_stab, IPROUTETYPE)) 
                  != (struct settab *) NULL) {
              type = *((int *)(st -> st_value));
              if (type == RTYPE_INVALID)
                  return int_SNMP_error__status_badValue;
          }

          /* parse */
          if ((status = 
                  parse_set (&st, &rt -> rt_stab, IPROUTEMASK, os, v) )
                  != int_SNMP_error__status_noError)
              return status;
            break;

    case type_SNMP_PDUs_commit:
          status = sr_commit (rt);
          free_set (&rt -> rt_stab, IPROUTEMASK, os);
          break;

    case type_SNMP_PDUs_rollback:
          free_set (&rt -> rt_stab, IPROUTEMASK, os);
          rt -> rt_touched = 0;
          break;
  }
  return status;
}

/*
 *	FUNCTION: sr_noop ()
 *
 *	PURPOSE: routeEntry noop set function.  Called for:
 *		IPROUTEMETRIC1, IPROUTEMETRIC2, IPROUTEMETRIC3,
 *		IPROUTEMETRIC4, IPROUTEAGE.
 */
static int
sr_noop (oi, v, offset, rt)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct rtetab *rt;
{
    return int_SNMP_error__status_noError;
}

/* SET ROUTE TABLE (end) */

/* END ROUTE TABLE SECTION */

/*  */

#ifdef UNIXGROUP
static struct rtstat rtstat;


#define	UNIXROUTEBADREDIRECTS 0
#define	UNIXROUTECREATEDBYREDIRECTS 1
#define	UNIXROUTEMODIFIEDBYREDIRECTS 2
#define	UNIXROUTELOOKUPFAILS 3
#define	UNIXROUTEWILDCARDUSES 4


static int  o_ip_routing_stats (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register struct rtstat *rts = &rtstat;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    static   int lastq = -1;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1
		    || oid -> oid_elements[oid -> oid_nelem - 1] != 0)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((new = oid_extend (oid, 1)) == NULLOID)
		    return NOTOK;
		new -> oid_elements[new -> oid_nelem - 1] = 0;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else
		return NOTOK;
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    if (quantum != lastq) {
	lastq = quantum;

	if (getkmem (nl + N_RTSTAT, (caddr_t) rts, sizeof *rts) == NOTOK)
	    return generr (offset);
    }

    switch (ifvar) {
	case UNIXROUTEBADREDIRECTS:
	    return o_integer (oi, v, rts -> rts_badredirect & 0xffff);

	case UNIXROUTECREATEDBYREDIRECTS:
	    return o_integer (oi, v, rts -> rts_dynamic & 0xffff);

	case UNIXROUTEMODIFIEDBYREDIRECTS:
	    return o_integer (oi, v, rts -> rts_newgateway & 0xffff);

	case UNIXROUTELOOKUPFAILS:
	    return o_integer (oi, v, rts -> rts_unreach & 0xffff);

	case UNIXROUTEWILDCARDUSES:
	    return o_integer (oi, v, rts -> rts_wildcard & 0xffff);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}
#endif /* UNIXGROUP */

/*  */

/* BEGIN ARP TABLE SECTION */

struct adrtab {
#define	ADN_SIZE	(IFN_SIZE + 1)		/* IpAddress instance */
    unsigned int    adn_instance[ADN_SIZE];	
    int	    adn_insize;

    struct in_addr adn_address;			/* IpAddress */


#define	ADM_SIZE	ADR_SIZE		/* PhysAddress instance */
    unsigned int    adm_instance[ADM_SIZE];	
    int	    adm_insize;

    u_char	adm_address[ADM_SIZE];		/* PhysAddress */
    u_char	adm_addrlen;			/*   .. */


#define	ADA_SIZE	(IFN_SIZE + 2)		/* AtEntry instance */
    unsigned int    ada_instance[ADA_SIZE];	
    int	    ada_insize;


    int	    adr_index;				/* ifIndex */

    int	    adr_type;				/* IPNETTOMEDIATYPE */
#define	OTHER_MAPPING	1
#define DELETE_MAPPING	2
#define	DYNAMIC_MAPPING	3
#define	STATIC_MAPPING	4
#define MAPPING_MIN	1
#define MAPPING_MAX	4

    struct arptab *ad_arptab;			/* set support    */
    struct arpreq ad_arpold,			/*    ...         */
    		  ad_arpnew;			/*    ...         */
    int		  ad_sflags;			/*    ...         */
    struct settab ad_stab;			/*    ...         */
    struct adrtab *ad_snext;			/*    ...         */

    struct adrtab *adn_next;			/* next IpAddress */
    struct adrtab *adm_next;    		/* next PhysAddress */
    struct adrtab *ada_next;			/* next AtEntry */
};

static	struct adrtab *ada = NULL;
static	struct adrtab *adn = NULL;
static	struct adrtab *adm = NULL;
static  struct arptab *arptab = NULL;	/* need to keep this for sets */

static  int     flush_arp_cache = 0;
static	int	arptab_size;

static struct adrtab *get_arpent ();

/*  */

#define	ATIFINDEX	0
#define	ATPHYSADDRESS	1
#define	ATNETADDRESS	2

#define	IPNETTOMEDIAIFINDEX 3
#define	IPNETTOMEDIAPHYSADDRESS 4
#define	IPNETTOMEDIANETADDRESS 5
#define	IPNETTOMEDIATYPE 6


static int  
o_address (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register int    i = 0;
    int	    ifvar,
	    isnpa;
    register unsigned int *ip,
			  *jp = NULL;
    register struct adrtab *at = NULL;
    struct sockaddr_in netaddr;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    if (get_arptab (offset) == NOTOK)
	return generr (offset);

    switch (ifvar = (int) ot -> ot_info) {
	case IPNETTOMEDIAIFINDEX:
	case IPNETTOMEDIAPHYSADDRESS:
	case IPNETTOMEDIANETADDRESS:
	case IPNETTOMEDIATYPE:
	    isnpa = 0;
	    break;

	case ATIFINDEX:
	case ATPHYSADDRESS:
	case ATNETADDRESS:
	    isnpa = -1;
	    break;
	    
	default:
	    return int_SNMP_error__status_noSuchName;
    }
    
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((at = get_arpent (oid -> oid_elements
				  	+ ot -> ot_name -> oid_nelem,
				  oid -> oid_nelem
				  	- ot -> ot_name -> oid_nelem,
				  isnpa, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		switch (isnpa) {
		    case 0:
		        if (at = adn)
			    jp = at -> adn_instance, i = at -> adn_insize;
			break;

		    case 1:
		        if (at = adm)
			    jp = at -> adm_instance, i = at -> adm_insize;
			break;

		    case -1:
		        if (at = ada)
			    jp = at -> ada_instance, i = at -> ada_insize;
			break;
		}
		if (at == NULL)
		    return NOTOK;

		if ((new = oid_extend (oid, i)) == NULLOID)
		    return int_SNMP_error__status_genErr;
		ip = new -> oid_elements + new -> oid_nelem - i;
		for (; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	j;

		if ((at = get_arpent (oid -> oid_elements
				          + ot -> ot_name -> oid_nelem,
				      j = oid -> oid_nelem
				      	      - ot -> ot_name -> oid_nelem,
				      isnpa, 1)) == NULL)
		    return NOTOK;
		i = isnpa > 0 ? at -> adm_insize
		    	      : isnpa == 0 ? at -> adn_insize
				  	   : at -> ada_insize;

		if ((i = j - i) < 0) {
		    OID	    new;

		    if ((new = oid_extend (oid, -i)) == NULLOID)
			return int_SNMP_error__status_genErr;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = new;

		    oid = new;
		}
		else
		    if (i > 0)
			oid -> oid_nelem -= i;

		ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
		switch (isnpa) {
		    case 0:
			jp = at -> adn_instance, i = at -> adn_insize;
			break;

		    case 1:
			jp = at -> adm_instance, i = at -> adm_insize;
			break;

		    case -1:
		        jp = at -> ada_instance, i = at -> ada_insize;
			break;

		}
		for (; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case ATIFINDEX:
	case IPNETTOMEDIAIFINDEX:
	    return o_integer (oi, v, at -> adr_index);

	case ATPHYSADDRESS:
	case IPNETTOMEDIAPHYSADDRESS:
	    return o_string (oi, v, (char *) at -> adm_address,
			     (int) at -> adm_addrlen);

	case ATNETADDRESS:
	case IPNETTOMEDIANETADDRESS:
	    netaddr.sin_addr = at -> adn_address;	/* struct copy */
	    return o_ipaddr (oi, v, &netaddr);

	case IPNETTOMEDIATYPE:
	    return o_integer (oi, v, at -> adr_type);
    }
}

/* 	SET ARP TABLE (begin) */

/*
 *	FUNCTION: s_address ()
 *
 *	PURPOSE: set an arp table entry
 */
static int  
s_address (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID	oid = oi -> oi_name;
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct arpreq *arn, *aro;
    struct arptab *ac;
    int     status = int_SNMP_error__status_noError;
    int	    ifvar,
	    isnpa;
    register struct adrtab *at;
    struct sockaddr_in *sin;

    switch (ifvar = (int) ot -> ot_info) {
	case IPNETTOMEDIAIFINDEX:
	case IPNETTOMEDIAPHYSADDRESS:
	case IPNETTOMEDIANETADDRESS:
	case IPNETTOMEDIATYPE:
	    isnpa = 0;
	    break;

	case ATIFINDEX:
	case ATPHYSADDRESS:
	case ATNETADDRESS:
	    isnpa = -1;
	    break;
	
	default:
	    return int_SNMP_error__status_genErr;
    }

    /* check that this interface instance exists */
    switch (offset) {
	case type_SNMP_PDUs_set__request:

	    /* 
	     * only need to query the kernel once, protected by
	     * quantum for multiple requests in one snmp packet.
	     */
	    if (get_arptab (type_SNMP_PDUs_get__request) == NOTOK) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_9,
			"GENERR: s_address: get_arptab failed"));
		return int_SNMP_error__status_genErr;
	    }

	    /* need some interface information up front for some variables */
	    switch (ifvar) {
		case IPNETTOMEDIAIFINDEX:
		case IPNETTOMEDIANETADDRESS:
		case ATIFINDEX:
		case ATNETADDRESS:
		    if (get_interfaces (type_SNMP_PDUs_get__request) == NOTOK){
			advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_2,
			    "GENERR: %s: get_interfaces failed"),"s_address");
			return int_SNMP_error__status_genErr;
		    }
		    break;
	    }
	    /* FALL THRU */

	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:

	    /* validate instance */
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((at = get_arpent (oid -> oid_elements
				  	+ ot -> ot_name -> oid_nelem,
				  oid -> oid_nelem
				  	- ot -> ot_name -> oid_nelem,
				  isnpa, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_11,
		    "GENERR in s_address: bad offset %d"), offset);
	    return int_SNMP_error__status_genErr;
    }

    if (os == NULLOS) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_12,
		"GENERR: s_address: no syntax defined for \"%s\""),
		ot -> ot_text);
	return int_SNMP_error__status_genErr;
    }

    /* clear request struct */
    if (offset == type_SNMP_PDUs_set__request && !(at -> ad_sflags & ST_INSET)){

	aro = &at -> ad_arpold;
	arn = &at -> ad_arpnew;
	ac = at -> ad_arptab;
	at -> ad_sflags |= (ST_INSET | ST_ADDSET);	/* set in_progress */
	memset (aro, 0, sizeof *aro); 
	memset (arn, 0, sizeof *arn); 

	/* fill arpreq */
	sin = (struct sockaddr_in *) &aro -> arp_pa;
	sin -> sin_family = AF_INET;
	sin -> sin_addr = ac -> at_iaddr;	/* struct copy */
#if	defined(_POWER) && !defined(BSD44)
	bcopy ((char *)ac -> at_enaddr, (char *)aro -> arp_ha.sa_data, 
		ac -> at_length);
	aro -> at_length = ac -> at_length;
	aro -> ifType = ac -> ifType;
#else
	bcopy ((char *)ac -> at_enaddr, (char *)aro -> arp_ha.sa_data, 
		sizeof ac -> at_enaddr);
#endif
	aro -> arp_flags = ac -> at_flags & 
		(ATF_PERM | ATF_PUBL | ATF_USETRAILERS);
	bcopy ((char *)aro, (char *)arn, sizeof *aro);
    }

    switch (ifvar) {
	case ATIFINDEX:
	case IPNETTOMEDIAIFINDEX:
	    status = sa_ifindex (oi, v, offset, at);
	    break;

	case ATPHYSADDRESS:
	case IPNETTOMEDIAPHYSADDRESS:
	    status = sa_physaddress (oi, v, offset, at);
	    break;

	case ATNETADDRESS:
	case IPNETTOMEDIANETADDRESS:
	    status = sa_netaddress (oi, v, offset, at);
	    break;

	case IPNETTOMEDIATYPE:
	    status = sa_type (oi, v, offset, at);
	    break;
    }

    return status;
}

/*
 *	FUNCTION: sa_commit ()
 *
 *	PURPOSE: commit a set on an arp table entry
 */
static int
sa_commit (at)
struct adrtab *at;
{
    struct settab 	*st;
    struct arpreq 	*arn = &at -> ad_arpnew,
			*aro = &at -> ad_arpold;
    struct qbuf		*qb;

    if (at -> ad_sflags & ST_ISSET) {
#ifdef DEBUG
	/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
	if (debug > 3) 
	    advise (SLOG_DEBUG,
		    "Arp Entry %s already set",
		    inet_ntoa (((struct sockaddr_in *) 
			&aro -> arp_pa) -> sin_addr));
#endif
	return int_SNMP_error__status_noError;
    }
    at -> ad_sflags |= ST_ISSET;	/* mark this entry as already set */

    /* IPNETTOMEDIAPHYSADDRESS */
    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIAPHYSADDRESS)) 
	    != (struct settab *) NULL) {
	qb = ((struct qbuf *) st -> st_value) -> qb_forw;
#if	defined(_POWER)
	bcopy (qb -> qb_data, (char *)arn -> arp_ha.sa_data, qb -> qb_len);
#  if	!defined(BSD44)
	arn -> at_length = qb -> qb_len;
#  endif
#else
	bcopy (qb -> qb_data, (char *)arn -> arp_ha.sa_data, 
		sizeof (struct in_addr));
#endif
    }

    /* IPNETTOMEDIANETADDRESS */
    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIANETADDRESS)) 
	    != (struct settab *) NULL) {
	qb = ((struct qbuf *) st -> st_value) -> qb_forw;
	bcopy (qb -> qb_data,
	    (char *) &((struct sockaddr_in *) &arn -> arp_pa) -> sin_addr,
	    qb -> qb_len);
    }

    /* IPNETTOMEDIATYPE */
    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIATYPE)) 
	    != (struct settab *) NULL) {
	int type = *((int *)(st -> st_value));

	switch (type) {
	    case OTHER_MAPPING:
		arn -> arp_flags &= ~ATF_PERM;
		arn -> arp_flags |= ATF_PUBL;
		break;

	    case STATIC_MAPPING:
		arn -> arp_flags |= ATF_PERM;
		break;

	    case DYNAMIC_MAPPING:
		arn -> arp_flags &= ~(ATF_PERM & ATF_PUBL);
		break;
	}
    }

    /* delete old arp table entry */
    if (at -> ad_sflags & ST_DELETE) {
#ifdef DEBUG
	/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
	if (debug > 3)
	    advise (SLOG_DEBUG, "sa_commit: deleting: %s",
		    inet_ntoa (((struct sockaddr_in *) 
			&aro -> arp_pa) -> sin_addr));
#endif
	if (ioctl (Nd, SIOCDARP, (caddr_t) aro) == NOTOK) {

	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_13,
		    "GENERR: sa_commit: %s of %s failed: %s"), "SIOCDARP",
		    inet_ntoa (((struct sockaddr_in *) 
			&aro -> arp_pa) -> sin_addr), strerror(errno));
	    return int_SNMP_error__status_genErr;
	}
    }

    /* add/set arp table entry */
    if (at -> ad_sflags & ST_ADDSET) {
#ifdef DEBUG
	/* NOT IN CATALOG, BECAUSE IT IS DEBUG */
	if (debug > 3)
	    advise (SLOG_DEBUG, "sa_commit: setting: %s",
		    inet_ntoa (((struct sockaddr_in *) 
			&arn -> arp_pa) -> sin_addr));
#endif
	if (ioctl (Nd, SIOCSARP, (caddr_t) arn) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_13,
		    "GENERR: sa_commit: %s of %s failed: %s"), "SIOCSARP",
		    inet_ntoa (((struct sockaddr_in *) 
			&arn -> arp_pa) -> sin_addr), strerror(errno));
	    return int_SNMP_error__status_genErr;
	}
    }
    flush_arp_cache = 1;
    return int_SNMP_error__status_noError;
}

/*
 *	FUNCTION: sa_ifindex ()
 *
 *	PURPOSE: set arp_table interface index field.
 */
static int  
sa_ifindex (oi, v, offset, at)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct adrtab *at;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct arpreq 	*aro = &at -> ad_arpold;
    struct settab	*st;
    int			status = int_SNMP_error__status_noError;
    int			type, index;
#if 0
    struct qbuf 	*qb;
    struct address	*as;
    struct sockaddr_in  sin;
#endif

    switch (offset) {

	/* decode and check our value */
	case type_SNMP_PDUs_set__request:

	    /* check that we aren't already tagged to do a delete */
	    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIATYPE))
		    != (struct settab *) NULL) {
		type = *((int *)(st -> st_value));
		if (type == DELETE_MAPPING)
		    return int_SNMP_error__status_badValue;
	    }

	    /* parse */
	    if ((status = 
		    parse_set (&st, &at -> ad_stab, IPNETTOMEDIAIFINDEX, os, v))
		    != int_SNMP_error__status_noError)
		return status;

	    /* check that our index is between 1 and numinterfaces */
	    index = *((int *)(st -> st_value));
	    if (index < 1 || index > ifNumber)
		return int_SNMP_error__status_badValue;

#if 0
/* sure, we can check all this.  But since we aren't doing any
   real work, do we really care?
 */
	    /* check that our host can be reached via this interface */
	    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIANETADDRESS))
		    != (struct settab *) NULL) {
		qb = ((struct qbuf *)st -> st_value) -> qb_forw;
		bcopy (qb -> qb_data, (char *) &sin.sin_addr, qb -> qb_len);
	    }
	    else
		sin.sin_addr = ((struct sockaddr_in *) 
			&aro -> arp_pa) -> sin_addr;	/* struct copy */

	    if ((as = ifa_ifwithnet (sin.sin_addr)) == (struct address *)NULL){
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_14,
			"sa_ifindex: ifa_ifwithnet failed on %s"),
			inet_ntoa (sin.sin_addr));
		return int_SNMP_error__status_genErr;
	    }
	    if (index != ffs (as -> adr_indexmask)) {
#ifdef DEBUG
		/* NOT IN CATALOG, BECAUSE DEBUG */
		advise (SLOG_DEBUG,
			"NetAddress %s not reachable on interface %d",
			inet_ntoa (sin.sin_addr),
			index);
#endif
		return int_SNMP_error__status_badValue;
	    }
#endif /* 0 */
	    break;

	/* perform the ioctl (NOT IMPLEMENTED) */
	case type_SNMP_PDUs_commit:

	    free_set (&at -> ad_stab, IPNETTOMEDIAIFINDEX, os);
	    break;

	case type_SNMP_PDUs_rollback:

	    free_set (&at -> ad_stab, IPNETTOMEDIAIFINDEX, os);
	    break;
    }
    return status;
}

/*
 *	FUNCTION: sa_physaddress ()
 *
 *	PURPOSE: set arp_table physical address field.
 */
static int  
sa_physaddress (oi, v, offset, at)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct adrtab *at;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct settab	*st;
    int			status = int_SNMP_error__status_noError;
    int			type;
    struct qbuf 	*qb;

    switch (offset) {

	/* decode and check our value */
	case type_SNMP_PDUs_set__request:

	    /* check that we aren't already tagged to do a delete */
	    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIATYPE))
		    != (struct settab *) NULL) {
		type = *((int *)(st -> st_value));
		if (type == DELETE_MAPPING)
		    return int_SNMP_error__status_badValue;
	    }

	    /* parse */
	    if ((status = parse_set (&st, &at -> ad_stab, 
		    IPNETTOMEDIAPHYSADDRESS, os, v))
		    != int_SNMP_error__status_noError)
		return status;

	    /* PhysAddress length limited to ?? bytes */
	    /* XXX: may have to re-visit this for table adds */
	    if ((qb = ((struct qbuf *)st -> st_value) -> qb_forw) 
		    -> qb_len != at -> adm_addrlen) 
		return int_SNMP_error__status_badValue;

#ifdef DEBUG
	    if (debug > 3) {
		char tbuf[20];
		register char *cp, *ep, *p;

		p = "";
		*tbuf = '\0';
		for (ep = (cp = qb -> qb_data) + qb -> qb_len; 
			cp < ep; cp++) {
		    sprintf (tbuf, "%s%s%02x", tbuf, p, *cp & 0xff);
		    p = ":";
		}
		/* NOT IN CATALOG, BECAUSE DEBUG */
		advise (SLOG_DEBUG, "setting physaddress to %s", tbuf);
	    }
#endif
	    break;

	/* perform the ioctl */
	case type_SNMP_PDUs_commit:

	    status = sa_commit (at);
	    free_set (&at -> ad_stab, IPNETTOMEDIAPHYSADDRESS, os);
	    break;

	case type_SNMP_PDUs_rollback:

	    free_set (&at -> ad_stab, IPNETTOMEDIAPHYSADDRESS, os);
	    break;
    }
    return status;
}

/*
 *	FUNCTION: sa_netaddress ()
 *
 *	PURPOSE: set arp_table net (host) address field.
 */
static int  
sa_netaddress (oi, v, offset, at)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct adrtab *at;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct settab	*st;
    int			status = int_SNMP_error__status_noError;
    int			type;
    struct qbuf 	*qb;
    struct sockaddr_in  sin;

    switch (offset) {

	/* decode and check our value */
	case type_SNMP_PDUs_set__request:

	    /* check that we aren't already tagged to do a delete */
	    if ((st = find_set (&at -> ad_stab, IPNETTOMEDIATYPE))
		    != (struct settab *) NULL) {
		type = *((int *)(st -> st_value));
		if (type == DELETE_MAPPING)
		    return int_SNMP_error__status_badValue;
	    }

	    /* parse */
	    if ((status = parse_set (&st, &at -> ad_stab, 
		    IPNETTOMEDIANETADDRESS, os, v))
		    != int_SNMP_error__status_noError)
		return status;

	    /* ipAddress length limited to 4 bytes */
	    if ((qb = ((struct qbuf *)st -> st_value) -> qb_forw) 
		    -> qb_len != 4) 
		return int_SNMP_error__status_badValue;

	    /* check if this address is reachable from here */
	    bcopy (qb -> qb_data, (char *) &sin.sin_addr, qb -> qb_len);
	    if (ifa_ifwithnet (sin.sin_addr) == (struct address *)NULL) {
#ifdef DEBUG
		/* NOT IN CATALOG, BECAUSE DEBUG */
		advise (SLOG_DEBUG, "NetAddress %s not reachable",
			inet_ntoa(*((unsigned long *)qb -> qb_data)) );
#endif
		return int_SNMP_error__status_badValue;
	    }
#ifdef DEBUG
	    if (debug)
		/* NOT IN CATALOG, BECAUSE DEBUG */
		advise (SLOG_DEBUG, "NetAddress change request to: %s", 
			inet_ntoa(*((unsigned long *)qb -> qb_data)) );
#endif
	    if (!(at -> ad_sflags & ST_NEWSET))		/* if not tableset.. */
		at -> ad_sflags |= ST_DELETE;		/* ..must delete old */
	    break;

	/* perform the ioctl */
	case type_SNMP_PDUs_commit:

	    status = sa_commit (at);
	    free_set (&at -> ad_stab, IPNETTOMEDIANETADDRESS, os);
	    break;

	case type_SNMP_PDUs_rollback:

	    free_set (&at -> ad_stab, IPNETTOMEDIANETADDRESS, os);
	    break;
    }
    return status;
}

/*
 *	FUNCTION: sa_type ()
 *
 *	PURPOSE: set arp_table flags field.
 */
static int  
sa_type (oi, v, offset, at)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
struct adrtab *at;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    struct settab	*st;
    int			status = int_SNMP_error__status_noError;
    int			type;

    switch (offset) {

	/* decode and check our value */
	case type_SNMP_PDUs_set__request:

	    /* parse */
	    if ((status = parse_set (&st, &at -> ad_stab, 
		    IPNETTOMEDIATYPE, os, v))
		    != int_SNMP_error__status_noError)
		return status;

	    /* check desired value */
	    type = *((int *)(st -> st_value));
	    if (type < MAPPING_MIN || type > MAPPING_MAX) 
		return int_SNMP_error__status_badValue;

	    /* can't delete and change the same entry */
	    if (type == DELETE_MAPPING) {

		/* if a delete, this must be the only variable set */
		if (st -> st_forw != st -> st_back)
		    return int_SNMP_error__status_badValue;

		/* can't delete a table-add entry */
		if (at -> ad_sflags & ST_NEWSET)
		    return int_SNMP_error__status_badValue;
		at -> ad_sflags |= ST_DELETE;	/* delete old... */
		at -> ad_sflags &= ~ST_ADDSET;	/* ...but dont add new */
	    }
	    break;

	/* perform the ioctl */
	case type_SNMP_PDUs_commit:

	    status = sa_commit (at);
	    free_set (&at -> ad_stab, IPNETTOMEDIATYPE, os);
	    break;

	case type_SNMP_PDUs_rollback:

	    free_set (&at -> ad_stab, IPNETTOMEDIATYPE, os);
	    break;
    }
    return status;
}
/* SET ARP TABLE (END) */

/*  */

static int  
adn_compar (a, b)
register struct adrtab **a,
		       **b;
{
    return elem_cmp ((*a) -> adn_instance, (*a) -> adn_insize,
		     (*b) -> adn_instance, (*b) -> adn_insize);
}


static int  
adm_compar (a, b)
register struct adrtab **a,
		       **b;
{
    return elem_cmp ((*a) -> adm_instance, (*a) -> adm_insize,
		     (*b) -> adm_instance, (*b) -> adm_insize);
}

static int  
ada_compar (a, b)
register struct adrtab **a,
		       **b;
{
    return elem_cmp ((*a) -> ada_instance, (*a) -> ada_insize,
		     (*b) -> ada_instance, (*b) -> ada_insize);
}


static int  
get_arptab (offset) 
int	offset;
{
    int	    adrNumber = 0,
	    tblsize;
    register struct arptab *ac,
			   *ae;
    register struct adrtab *at,
			   *ap,
			  **base,
			  **afe,
			  **afp;
    register struct address *as;
#if	defined(_POWER) && defined(BSD44)
    struct interface *is;
    static  int itic = 0;
#endif
    static  int first_time = 1;
    static  int lastq = -1;
    static  int tic = 0;
    struct nlist                nzs;
    register struct nlist *nz = &nzs;
    caddr_t     arptabp;

    if (quantum == lastq)
	return OK;
    if (!flush_arp_cache
	    && offset == type_SNMP_PDUs_get__next__request
	    && quantum == lastq + 1
	    && tic < (IPNETTOMEDIAENTRIES * arptab_size)) {/* XXX: caching! */
	lastq = quantum;
	tic ++;
	return OK;
    }

#if	defined(_POWER) && defined(BSD44)
    /*
     * If we are config'ed to not automatically check the interfaces
     * table, then we must insure that we have a good interface table here,
     * since arp entries contain ifnet pointers.  We cache abit here, 
     * to prevent the overhead of reading the interface table every 
     * time we want an arp table entry.
     */
    if (!stimeout)
	if (!flush_arp_cache 
		&& !flush_if_cache
		&& quantum == lastq + 1
		&& itic < (IPNETTOMEDIAENTRIES * arptab_size)) {
	    itic ++;
	}
	else {
	    itic = 0;
	    if (get_interfaces (type_SNMP_PDUs_get__request) == NOTOK){
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_2, 
			"%s: get_interfaces failed"), "get_arptab");
		return NOTOK;
	    }
	}

#endif	/* defined(_POWER) && defined(BSD44) */

    lastq = quantum, flush_arp_cache = 0, tic = 0;

    /* cleanup */
    if (arptab != NULL) {
	free ((char *) arptab);
	arptab = NULL;
    }
    for (at = adn; at; at = ap) {
	ap = at -> adn_next;

	free ((char *) at);
    }
    adn = adm = ada = NULL;

    /* fetch arptab from kernel */
    if (getkmem (nl + N_ARPTAB_SIZE, (caddr_t) &arptab_size,
		 sizeof arptab_size) == NOTOK)
	return NOTOK;
    tblsize = arptab_size * sizeof *arptab;
    if ((arptab = (struct arptab *) malloc ((unsigned) (tblsize))) == NULL)
	adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_arptab");
    if (getkmem (nl + N_ARPTAB, (caddr_t) &arptabp, sizeof arptabp) == NOTOK) {
	free ((char *) arptab);
	arptab = NULL;
	return NOTOK;
    }
    nz -> n_name = "struct arptab", nz -> n_value = arptabp;
    if (getkmem (nz, (caddr_t) arptab, tblsize) == NOTOK) {
	free ((char *) arptab);
	arptab = NULL;
	return NOTOK;
    }

    afp = &adn;
    for (ae = (ac = arptab) + arptab_size; ac < ae; ac++) {
	int	type;

	if (!(ac -> at_iaddr.s_addr) || !(ac -> at_flags & ATF_COM))
	    continue;
	type = ac -> at_flags & ATF_PERM ? STATIC_MAPPING
	     : ac -> at_flags & ATF_PUBL ? OTHER_MAPPING
	     : DYNAMIC_MAPPING;

	/* find the interface that this arp table addr would go over.  */
#if	defined(_POWER) && defined(BSD44)

	/* match ifnet pointers */
	for (is = ifs; is; is = is -> ifn_next) 
	    if (is -> ifn_offset == (unsigned long)ac -> at_ifp)
		break;
	    
	/*
	 * Ideally this will only occur after an IFDETACH until
	 * arp_entry time_out.
	 */
	if (is == (struct interface *) NULL) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_10,
		    "no interface for arp entry %s"),
		    inet_ntoa (ac -> at_iaddr));
	    continue;
	}

	/* locate address for this interfaces */
	for (as = afs_inet; as; as = as -> adr_next)
	    if (as -> adr_indexmask & is -> ifn_indexmask)
		break;

	if (as == (struct address *) NULL) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_15,
		    "cannot locate address for arp entry %s"),
		    inet_ntoa (ac -> at_iaddr));
	    continue;
	}

#else
	if ((as = ifa_ifwithnet (ac -> at_iaddr)) == (struct address *)NULL) {
	    /*
	     * Ideally this will only occur on a 6000, with valid arp
	     * table entries, after an IFDETACH up till arp_entry time_out.
	     */
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_IP, IP_10,
		    "no interface for arp entry %s"),
		    inet_ntoa (ac -> at_iaddr));
	    continue;		/* ignore it. */
	}
#endif	/* defined(_POWER) && defined(BSD44) */

	/* fill up internal version of arptab */
	if ((at = (struct adrtab *) calloc (1, sizeof *at)) == NULL)
	    adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		  "get_arptab");
	*afp = at, afp = &at -> adn_next, adrNumber++;

	at -> ad_arptab = ac;		/* set support */
	at -> ad_sflags = ST_NOSET;	/*     ...     */
	at -> ad_stab.st_forw = at -> ad_stab.st_back = &at -> ad_stab;
	at -> adr_index = ffs (as -> adr_indexmask);
	at -> adr_type = type;

	at -> adn_address = ac -> at_iaddr;	/* struct copy */
	at -> adn_instance[0] = at -> adr_index, at -> adn_insize = 1;
	at -> adn_insize += ipaddr2oid (at -> adn_instance + 1,
					    &at -> adn_address);

#if	defined(_POWER)
#  if	defined(BSD44)
	at -> adm_addrlen = is -> ifn_interface -> ac_if.if_addrlen;
#  else
	at -> adm_addrlen = ac -> at_length;
#  endif	/* defined(BSD44) */
#else
	at -> adm_addrlen = sizeof ac -> at_enaddr;
#endif		/* defined(_POWER) */

	bcopy ((char *) ac -> at_enaddr, (char *) at -> adm_address,
		at -> adm_addrlen);

	at -> adm_instance[0] = at -> adr_index, at -> adm_insize = 1;
	at -> adm_insize += mediaddr2oid (at -> adm_instance + 1,
					  at -> adm_address,
					  (int) at -> adm_addrlen, 0);

	at -> ada_instance[0] = at -> adr_index;
	at -> ada_instance[1] = 1;
	at -> ada_insize = 2;
	at -> ada_insize += ipaddr2oid (at -> ada_instance + 2,
					&at -> adn_address);

#ifdef DEBUG
	if (debug && first_time) {
	    char    buffer[BUFSIZ];
	    OIDentifier	oids;

	    oids.oid_elements = at -> adn_instance;
	    oids.oid_nelem = at -> adn_insize;
	    (void) strcpy (buffer, sprintoid (&oids));
	    oids.oid_elements = at -> adm_instance;
	    oids.oid_nelem = at -> adm_insize;
	    /* NOT IN CATALOG, BECAUSE DEBUG */
	    advise (SLOG_DEBUG, 
		    "ARPTAB: add mapping on interface %d: %s -> %s", 
		    at -> adr_index,
		    buffer, sprintoid (&oids));
	}
#endif
    }
    first_time = 0;

    if (adrNumber <= 1) {
	adm = ada = adn;
	return OK;
    }

    if ((base = (struct adrtab **)
		    malloc ((unsigned) (adrNumber * sizeof *base))) == NULL)
	adios(MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
	      "get_arptab");

    afe = base;
    for (at = adn; at; at = at -> adn_next)
	*afe++ = at;

#ifdef _ANSI_C_SOURCE 
    qsort ((char *) base, adrNumber, sizeof *base, 
	   (int(*)(const void *, const void *))adn_compar);
#else /* !_ANSI_C_SOURCE */
    qsort ((char *) base, adrNumber, sizeof *base, adn_compar);
#endif /* _ANSI_C_SOURCE */

    afp = base;
    at = adn = *afp++;
    while (afp < afe) {
	at -> adn_next = *afp;
	at = *afp++;
    }
    at -> adn_next = NULL;

#ifdef _ANSI_C_SOURCE 
    qsort ((char *) base, adrNumber, sizeof *base, 
           (int(*)(const void *, const void *))adm_compar);
#else /* !_ANSI_C_SOURCE */
    qsort ((char *) base, adrNumber, sizeof *base, adm_compar);
#endif /* _ANSI_C_SOURCE */

    afp = base;
    at = adm = *afp++;
    while (afp < afe) {
	at -> adm_next = *afp;
	at = *afp++;
    }
    at -> adm_next = NULL;

#ifdef _ANSI_C_SOURCE 
    qsort ((char *) base, adrNumber, sizeof *base, 
	   (int(*)(const void *, const void *))ada_compar);
#else /* !_ANSI_C_SOURCE */
    qsort ((char *) base, adrNumber, sizeof *base, ada_compar);
#endif /* _ANSI_C_SOURCE */

    afp = base;
    at = ada = *afp++;
    while (afp < afe) {
	at -> ada_next = *afp;
	at = *afp++;
    }
    at -> ada_next = NULL;

    free ((char *) base);

    return OK;
}

/*  */

static struct adrtab *
get_arpent (ip, len, isnpa, isnext)
register unsigned int *ip;
int	len;
int	isnpa,
	isnext;
{
    register struct adrtab *at;

    switch (isnpa) {
	case 0:
	    for (at = adn; at; at = at -> adn_next)
		switch (elem_cmp (at -> adn_instance, at -> adn_insize,
				  ip, len)) {
		    case 0:
			if (!isnext)
			    return at;
			if ((at = at -> adn_next) == NULL) {
			    flush_arp_cache = 1;
			    return NULL;
			}
			/* else fall... */

		    case 1:
			return (isnext ? at : NULL);
		}
	    break;

	case 1:
	    for (at = adm; at; at = at -> adm_next)
		switch (elem_cmp (at -> adm_instance, at -> adm_insize,
				  ip, len)) {
		    case 0:
			if (!isnext)
			    return at;
			if ((at = at -> adm_next) == NULL) {
			    flush_arp_cache = 1;
			    return NULL;
			}
			/* else fall... */

		    case 1:
			return (isnext ? at : NULL);
		}
	    break;

	case -1:
	    for (at = ada; at; at = at -> ada_next)
		switch (elem_cmp (at -> ada_instance, at -> ada_insize,
				  ip, len)) {
		    case 0:
			if (!isnext)
			    return at;
			if ((at = at -> ada_next) == NULL) {
			    flush_arp_cache = 1;
			    return NULL;
			}
			/* else fall... */

		    case 1:
			return (isnext ? at : NULL);
		}
	    break;
    }

    flush_arp_cache = 1;
    return NULL;
}

/* END ARP TABLE SECTION */

/*  */

init_ip () 
{
    register OT	    ot;

    if (ot = text2obj ("ipForwarding"))
#ifdef _POWER
	ot -> ot_getfnx = o_ip_forwarding,
	ot -> ot_setfnx = s_ip_forwarding;
#else
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPFORWARDING;
#endif
    if (ot = text2obj ("ipDefaultTTL"))
#ifdef _POWER
	ot -> ot_getfnx = o_ip_default_ttl,
	ot -> ot_setfnx = s_ip_default_ttl;
#else
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPDEFAULTTTL;
#endif
#ifdef	IPINRECEIVES
    if (ot = text2obj ("ipInReceives"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPINRECEIVES;
#endif
    if (ot = text2obj ("ipInHdrErrors"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPINHDRERRORS;
#ifdef	IPINADDRERRORS
    if (ot = text2obj ("ipInAddrErrors"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPINADDRERRORS;
#endif
#ifdef	IPFORWDATAGRAMS
    if (ot = text2obj ("ipForwDatagrams"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPFORWDATAGRAMS;
#endif
#ifdef	IPINUNKNOWNPROTOS
    if (ot = text2obj ("ipInUnknownProtos"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPINUNKNOWNPROTOS;
#endif
#ifdef	IPINDISCARDS
    if (ot = text2obj ("ipInDiscards"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPINDISCARDS;
#endif
#ifdef	IPINDELIVERS
    if (ot = text2obj ("ipInDelivers"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPINDELIVERS;
#endif
#ifdef	IPOUTREQUESTS
    if (ot = text2obj ("ipOutRequests"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPOUTREQUESTS;
#endif
#ifdef	IPOUTDISCARDS
    if (ot = text2obj ("ipOutDiscards"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPOUTDISCARDS;
#endif
#ifdef	IPOUTNOROUTES
    if (ot = text2obj ("ipOutNoRoutes"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPOUTNOROUTES;
#endif
    if (ot = text2obj ("ipReasmTimeout"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPREASMTIMEOUT;
#ifdef	IPREASMREQDS
    if (ot = text2obj ("ipReasmReqds"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPREASMREQDS;
#endif
#ifdef	IPREASMOKS
    if (ot = text2obj ("ipReasmOKs"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPREASMOKS;
#endif
#ifdef	IPREASMFAILS
    if (ot = text2obj ("ipReasmFails"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPREASMFAILS;
#endif
#ifdef	IPFRAGOKS
    if (ot = text2obj ("ipFragOKs"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPFRAGOKS;
#endif
#ifdef	IPFRAGFAILS
    if (ot = text2obj ("ipFragFails"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPFRAGFAILS;
#endif
#ifdef	IPFRAGCREATES
    if (ot = text2obj ("ipFragCreates"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPFRAGCREATES;
#endif
    if (ot = text2obj ("ipRoutingDiscards"))
	ot -> ot_getfnx = o_ip,
	ot -> ot_info = (caddr_t) IPROUTINGDISCARDS;

    if (ot = text2obj ("ipAdEntAddr"))
	ot -> ot_getfnx = o_ip_addr,
	ot -> ot_info = (caddr_t) IPADENTADDR;
    if (ot = text2obj ("ipAdEntIfIndex"))
	ot -> ot_getfnx = o_ip_addr,
	ot -> ot_info = (caddr_t) IPADENTIFINDEX;
    if (ot = text2obj ("ipAdEntNetMask"))
	ot -> ot_getfnx = o_ip_addr,
	ot -> ot_info = (caddr_t) IPADENTNETMASK;
    if (ot = text2obj ("ipAdEntBcastAddr"))
	ot -> ot_getfnx = o_ip_addr,
	ot -> ot_info = (caddr_t) IPADENTBCASTADDR;
    if (ot = text2obj ("ipAdEntReasmMaxSize"))
	ot -> ot_getfnx = o_ip_addr,
	ot -> ot_info = (caddr_t) IPADENTREASMMAXSIZE;

    if (ot = text2obj ("ipRouteDest"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_setfnx = s_ip_route,		/* setable! */
	ot -> ot_info = (caddr_t) IPROUTEDEST;
    if (ot = text2obj ("ipRouteIfIndex"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEIFINDEX;
    if (ot = text2obj ("ipRouteMetric1"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEMETRIC1;
    if (ot = text2obj ("ipRouteMetric2"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEMETRIC2;
    if (ot = text2obj ("ipRouteMetric3"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEMETRIC3;
    if (ot = text2obj ("ipRouteMetric4"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEMETRIC4;
    if (ot = text2obj ("ipRouteNextHop"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_setfnx = s_ip_route,		/* setable! */
	ot -> ot_info = (caddr_t) IPROUTENEXTHOP;
    if (ot = text2obj ("ipRouteType"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_setfnx = s_ip_route,		/* setable! */
	ot -> ot_info = (caddr_t) IPROUTETYPE;
    if (ot = text2obj ("ipRouteProto"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEPROTO;
    if (ot = text2obj ("ipRouteAge"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEAGE;
    if (ot = text2obj ("ipRouteMask"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEMASK;
    if (ot = text2obj ("ipRouteMetric5"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEMETRIC5;
    if (ot = text2obj ("ipRouteInfo"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) IPROUTEINFO;
    
#ifdef UNIXGROUP
    if (ot = text2obj ("unixIpRouteFlags"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) UNIXIPROUTEFLAGS;
    if (ot = text2obj ("unixIpRouteRefCnt"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) UNIXIPROUTEREFCNT;
    if (ot = text2obj ("unixIpRouteUses"))
	ot -> ot_getfnx = o_ip_route,
	ot -> ot_info = (caddr_t) UNIXIPROUTEUSES;

    if (ot = text2obj ("unixRouteBadRedirects"))
	ot -> ot_getfnx = o_ip_routing_stats,
	ot -> ot_info = (caddr_t) UNIXROUTEBADREDIRECTS;
    if (ot = text2obj ("unixRouteCreatedByRedirects"))
	ot -> ot_getfnx = o_ip_routing_stats,
	ot -> ot_info = (caddr_t) UNIXROUTECREATEDBYREDIRECTS;
    if (ot = text2obj ("unixRouteModifiedByRedirects"))
	ot -> ot_getfnx = o_ip_routing_stats,
	ot -> ot_info = (caddr_t) UNIXROUTEMODIFIEDBYREDIRECTS;
    if (ot = text2obj ("unixRouteLookupFails"))
	ot -> ot_getfnx = o_ip_routing_stats,
	ot -> ot_info = (caddr_t) UNIXROUTELOOKUPFAILS;
    if (ot = text2obj ("unixRouteWildcardUses"))
	ot -> ot_getfnx = o_ip_routing_stats,
	ot -> ot_info = (caddr_t) UNIXROUTEWILDCARDUSES;
#endif /* UNIXGROUP */
    
    if (ot = text2obj ("atIfIndex"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) ATIFINDEX;
    if (ot = text2obj ("atPhysAddress"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) ATPHYSADDRESS;
    if (ot = text2obj ("atNetAddress"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) ATNETADDRESS;

    if (ot = text2obj ("ipNetToMediaIfIndex"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) IPNETTOMEDIAIFINDEX;
    if (ot = text2obj ("ipNetToMediaPhysAddress"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) IPNETTOMEDIAPHYSADDRESS;
    if (ot = text2obj ("ipNetToMediaNetAddress"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) IPNETTOMEDIANETADDRESS;
    if (ot = text2obj ("ipNetToMediaType"))
	ot -> ot_getfnx = o_address,
	ot -> ot_setfnx = s_address,		/* setable! */
	ot -> ot_info = (caddr_t) IPNETTOMEDIATYPE;
}

/* 
 *  Procedure: check_routes()
 *
 *  Purpose: To check that all the touched routes have the required values.
 */
int 
check_routes()
{
  struct rtetab *ptr = rts_inet;
  struct settab *ptrs = NULL;
  int count;

  for ( ;ptr; ptr = ptr -> rt_next) {
    if (ptr -> rt_touched == 0)
      continue; 

    count = 0;
    if (find_set (&ptr -> rt_stab, IPROUTEDEST) != (struct settab *) NULL)
      count++;

    if (find_set (&ptr -> rt_stab, IPROUTENEXTHOP) != (struct settab *) NULL)
      count++;

    if ((ptrs = find_set (&ptr -> rt_stab, IPROUTETYPE)) != 
	(struct settab *) NULL)
      count++;

    if ((count != 3) && !(ptr -> rt_sflags & ST_EXISTS))
	return (NOTOK);

    if (ptrs && *((int *)(ptrs -> st_value)) == 2 && 
	ptrs -> st_forw != ptrs -> st_back)
	return (NOTOK);
  }

  return(OK);
}
