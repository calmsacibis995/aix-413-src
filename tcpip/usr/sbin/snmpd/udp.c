static char sccsid[] = "@(#)20	1.8  src/tcpip/usr/sbin/snmpd/udp.c, snmp, tcpip411, GOLD410 5/27/94 10:29:02";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_udp(), o_udp_listen(), ut_compar(), get_listeners(), 
 *            get_udpent(), init_udp()
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/udp.c
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

/* udp.c - MIB realization of the UDP group */


#include "snmpd.h"

#include <sys/socket.h>
#include <sys/socketvar.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

/*  */

static struct udpstat udpstat;

/*  */

#if	defined(BSD44) || defined(_POWER)
#define	UDPINDATAGRAMS	1
#define	UDPNOPORTS	2
#endif
#define	UDPINERRORS	3
#if	defined(BSD44) || defined(_POWER)
#define	UDPOUTDATAGRAMS 4
#endif

#if	defined(_POWER) && !defined(BSD44)
/* re-map BSD44 udpstat struct names */
#define	udps_ipackets	udpInDatagrams
#define	udps_noport	udpNoPorts
#define	udps_opackets	udpOutDatagrams
#endif

static int  
o_udp (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register struct udpstat *udps = &udpstat;
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
	    if (oid -> oid_nelem > ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((new = oid_extend (oid, 1)) == NULLOID)
		    return int_SNMP_error__status_genErr;
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

	if (getkmem (nl + N_UDPSTAT, (caddr_t) udps, sizeof *udps) == NOTOK)
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
#ifdef	UDPINDATAGRAMS
	case UDPINDATAGRAMS:
	    return o_integer (oi, v, udps -> udps_ipackets);
#endif

#ifdef	UDPNOPORTS
	case UDPNOPORTS:
	    return o_integer (oi, v, udps -> udps_noport);
#endif

	case UDPINERRORS:
	    return o_integer (oi, v, udps -> udps_hdrops
			           + udps -> udps_badsum
			           + udps -> udps_badlen);

#ifdef	UDPOUTDATAGRAMS
	case UDPOUTDATAGRAMS:
	    return o_integer (oi, v, udps -> udps_opackets);
#endif

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

struct udptab {
#define	UT_SIZE	5			/* object instance */
    unsigned int   ut_instance[UT_SIZE];

    struct inpcb   ut_pcb;		/* protocol control block */

    struct socket  ut_socb;		/* socket info */

    struct udptab *ut_next;
};

static struct udptab *uts = NULL;

static  int     flush_udp_cache = 0;
static	int	udpListeners;

struct udptab *get_udpent ();

/*  */

#define	UDPLOCALADDRESS 0
#define	UDPLOCALPORT 1
#define	UNIXUDPREMADDRESS 2
#define	UNIXUDPREMPORT 3
#define	UNIXUDPSENDQ 4
#define	UNIXUDPRECVQ 5


static int  o_udp_listen (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct udptab *ut;
    struct sockaddr_in netaddr;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    if (get_listeners (offset) == NOTOK)
	return int_SNMP_error__status_genErr;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + UT_SIZE)
		return int_SNMP_error__status_noSuchName;
	    if ((ut = get_udpent (oid -> oid_elements + oid -> oid_nelem
				      - UT_SIZE, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((ut = uts) == NULL)
		    return NOTOK;

		if ((new = oid_extend (oid, UT_SIZE)) == NULLOID)
		    return int_SNMP_error__status_genErr;
		ip = new -> oid_elements + new -> oid_nelem - UT_SIZE;
		jp = ut -> ut_instance;
		for (i = UT_SIZE; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		OID new;

                if (oid->oid_nelem < UT_SIZE + ot -> ot_name -> oid_nelem)
		{
		    int i;

		    if ((new = oid_extend (oid,
		        UT_SIZE+ot->ot_name->oid_nelem-oid->oid_nelem))==
			NULLOID)
			return int_SNMP_error__status_genErr;

		    for(i=0;i<UT_SIZE+ot->ot_name->oid_nelem-oid->oid_nelem;i++)
			new -> oid_elements[i + oid -> oid_nelem] = 0;
		}
		else if (oid->oid_nelem > UT_SIZE + ot -> ot_name -> oid_nelem)
		{
		    if ((new = oid_extend(ot -> ot_name, UT_SIZE)) == NULLOID)
			return int_SNMP_error__status_genErr;

		    for(i=0; i < UT_SIZE; i++)
			new -> oid_elements[i + ot -> ot_name -> oid_nelem] = 0;
		}
		else if ((new = oid_cpy(oid)) == NULLOID)
		    return int_SNMP_error__status_genErr;

		if ((ut = get_udpent (ip = new -> oid_elements
				         + new -> oid_nelem - UT_SIZE, 1))
		        == NULL)
		{
		    oid_free(new);
		    return NOTOK;
		}

		jp = ut -> ut_instance;
		for (i = UT_SIZE; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {

	case UDPLOCALADDRESS:
	    netaddr.sin_addr = ut -> ut_pcb.inp_laddr;	/* struct copy */
	    return o_ipaddr (oi, v, &netaddr);
	    
	case UDPLOCALPORT:
	    return o_integer (oi, v, ut -> ut_pcb.inp_lport & 0xffff);

#ifdef UNIXGROUP
	case UNIXUDPREMADDRESS:
	    netaddr.sin_addr = ut -> ut_pcb.inp_faddr;	/* struct copy */
	    return o_ipaddr (oi, v, &netaddr);

	case UNIXUDPREMPORT:
	    return o_integer (oi, v, ntohs (ut -> ut_pcb.inp_fport) & 0xffff);

	case UNIXUDPSENDQ:
	    return o_integer (oi, v, ut -> ut_socb.so_snd.sb_cc);
	
	case UNIXUDPRECVQ:
	    return o_integer (oi, v, ut -> ut_socb.so_rcv.sb_cc);
#endif /* UNIXGROUP */

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

static int  
ut_compar (a, b)
struct udptab **a,
	      **b;
{
    return elem_cmp ((*a) -> ut_instance, UT_SIZE,
		     (*b) -> ut_instance, UT_SIZE);
}


static int  
get_listeners (offset) 
int	offset;
{
    register unsigned int  *cp;
    register struct udptab *us,
			   *up,
			  **usp;
    register struct inpcb  *ip;
    struct inpcb *head,
		  udb,
		  zdb;
    struct nlist nzs;
    register struct nlist *nz = &nzs;
    static   int first_time = 1;
    static   int lastq = -1;
    static   int tic = 0;

    if (quantum == lastq)
	return OK;
    if (!flush_udp_cache
	    && offset == type_SNMP_PDUs_get__next__request
	    && quantum == lastq + 1
	    && tic < (UDPENTRIES * udpListeners)) {	/* XXX: caching! */
	tic ++;
	lastq = quantum;
	return OK;
    }
    lastq = quantum, flush_udp_cache = 0, tic = 0;

    for (us = uts; us; us = up) {
	up = us -> ut_next;

	free ((char *) us);
    }
    uts = NULL;

    if (getkmem (nl + N_UDB, (char *) &udb, sizeof udb) == NOTOK)
	return NOTOK;
    head = (struct inpcb *) nl[N_UDB].n_value;

    usp = &uts, udpListeners = 0;
    ip = &udb;
    while (ip -> inp_next != head) {
	register struct udptab *uz;
	OIDentifier	oids;

	if ((us = (struct udptab *) calloc (1, sizeof *us)) == NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "get_listeners");
	nz -> n_name = "struct inpcb",
	nz -> n_value = (unsigned long) ip -> inp_next;
	if (getkmem (nz, (caddr_t) &us -> ut_pcb, sizeof us -> ut_pcb)
	        == NOTOK)
	    return NOTOK;
	ip = &us -> ut_pcb;

	nz ->n_name = "struct socket",
	nz -> n_value = (unsigned long) ip -> inp_socket;
	if (getkmem (nz, (caddr_t) &us -> ut_socb, sizeof us -> ut_socb)
	        == NOTOK)
	    return NOTOK;

	cp = us -> ut_instance;
	cp += ipaddr2oid (cp, &us -> ut_pcb.inp_laddr);
	*cp++ = us -> ut_pcb.inp_lport & 0xffff;

	for (uz = uts; uz; uz = uz -> ut_next)
	    if (elem_cmp (uz -> ut_instance, UT_SIZE,
			  us -> ut_instance, UT_SIZE) == 0)
		break;
	if (uz) {
	    if (first_time) {
		oids.oid_elements = us -> ut_instance;
		oids.oid_nelem = UT_SIZE;
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_UDP, UDP_1,
			"duplicate listeners: %s"), sprintoid (&oids));
	    }

	    *(ip = &zdb) = us -> ut_pcb;	/* struct copy */
	    free ((char *) us);
	    continue;
	}
	*usp = us, usp = &us -> ut_next, udpListeners++;
	if (debug && first_time) {
	    oids.oid_elements = us -> ut_instance;
	    oids.oid_nelem = UT_SIZE;
	    advise (SLOG_DEBUG, MSGSTR(MS_UDP, UDP_2,
		    "error:  add listener: %s"), sprintoid (&oids));
	}
    }
    first_time = 0;
    if (udpListeners > 1) {
	register struct udptab **base,
			       **use;

	if ((base = (struct udptab **) 
		malloc ((unsigned) (udpListeners * sizeof *base))) == NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "get_listeners");

	use = base;
	for (us = uts; us; us = us -> ut_next)
	    *use++ = us;

#ifdef _ANSI_C_SOURCE
	qsort ((char *) base, udpListeners, sizeof *base, 
	       (int(*)(const void *, const void *))ut_compar);
#else /* !_ANSI_C_SOURCE */
	qsort ((char *) base, udpListeners, sizeof *base, ut_compar);
#endif /* _ANSI_C_SOURCE */

	usp = base;
	us = uts = *usp++;

	while (usp < use) {
	    us -> ut_next = *usp;
	    us = *usp++;
	}
	us -> ut_next = NULL;

	free ((char *) base);
    }
    return OK;    
}

/*  */

static struct udptab *
get_udpent (ip, isnext)
register unsigned int *ip;
int	isnext;
{
    register struct udptab *ut;

    for (ut = uts; ut; ut = ut -> ut_next)
	switch (elem_cmp (ut -> ut_instance, UT_SIZE, ip, UT_SIZE)) {
	    case 0:
		if (!isnext)
		    return ut;
		if ((ut = ut -> ut_next) == NULL) {
		    flush_udp_cache = 1;
		    return NULL;
		}
		/* else fall... */

	    case 1:
		return (isnext ? ut : NULL);
	}

    flush_udp_cache = 1;
    return NULL;
}

/*  */

init_udp () 
{
    register OT	    ot;

#ifdef	UDPINDATAGRAMS
    if (ot = text2obj ("udpInDatagrams"))
	ot -> ot_getfnx = o_udp,
	ot -> ot_info = (caddr_t) UDPINDATAGRAMS;
#endif
#ifdef	UDPNOPORTS
    if (ot = text2obj ("udpNoPorts"))
	ot -> ot_getfnx = o_udp,
	ot -> ot_info = (caddr_t) UDPNOPORTS;
#endif
    if (ot = text2obj ("udpInErrors"))
	ot -> ot_getfnx = o_udp,
	ot -> ot_info = (caddr_t) UDPINERRORS;
#ifdef	UDPOUTDATAGRAMS
    if (ot = text2obj ("udpOutDatagrams"))
	ot -> ot_getfnx = o_udp,
	ot -> ot_info = (caddr_t) UDPOUTDATAGRAMS;
#endif
    if (ot = text2obj ("udpLocalAddress"))
	ot -> ot_getfnx = o_udp_listen,
	ot -> ot_info = (caddr_t) UDPLOCALADDRESS;

    if (ot = text2obj ("udpLocalPort"))
	ot -> ot_getfnx = o_udp_listen,
	ot -> ot_info = (caddr_t) UDPLOCALPORT;

#ifdef UNIXGROUP
    if (ot = text2obj ("unixUdpRemAddress"))
	ot -> ot_getfnx = o_udp_listen,
	ot -> ot_info = (caddr_t) UNIXUDPREMADDRESS;
    if (ot = text2obj ("unixUdpRemPort"))
	ot -> ot_getfnx = o_udp_listen,
	ot -> ot_info = (caddr_t) UNIXUDPREMPORT;
    if (ot = text2obj ("unixUdpSendQ"))
	ot -> ot_getfnx = o_udp_listen,
	ot -> ot_info = (caddr_t) UNIXUDPSENDQ;
    if (ot = text2obj ("unixUdpRecvQ"))
	ot -> ot_getfnx = o_udp_listen,
	ot -> ot_info = (caddr_t) UNIXUDPRECVQ;
#endif /* UNIXGROUP */
}
