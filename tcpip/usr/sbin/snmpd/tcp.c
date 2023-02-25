static char sccsid[] = "@(#)18	1.10  src/tcpip/usr/sbin/snmpd/tcp.c, snmp, tcpip411, GOLD410 5/27/94 10:28:20";
/*
 *   COMPONENT_NAME: SNMP
 *
 *   FUNCTIONS: get_connections
 *		get_tcpent
 *		init_tcp
 *		o_tcp
 *		o_tcp_conn
 *		tt_compar
 *
 *   ORIGINS: 27,60
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   Licensed Materials - Property of IBM
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

/* tcp.c - MIB realization of the TCP group */


#include "snmpd.h"

#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#ifdef BSD44
#include <netinet/ip.h>
#endif
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/ip_var.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>

/*  */

#define	RTOA_VANJ	4		/* tcpRtoAlgorithm */

#define	MXCN_NONE	(-1)		/* tcpMaxConn */


static struct tcpstat tcpstat;

static int tcpConnections;

/*  */

#define	TCPRTOALGORITHM	0
#define	TCPRTOMIN	1
#ifndef AOS
#define	TCPRTOMAX	2
#endif
#define	TCPMAXCONN	3
#ifndef AOS
#define	TCPACTIVEOPENS	4
#define	TCPPASSIVEOPENS	5
#define	TCPATTEMPTFAILS	6
#define	TCPESTABRESETS	7
#endif
#define	TCPCURRESTAB	8
#ifndef AOS
#define	TCPINSEGS	9
#define	TCPOUTSEGS	10
#define	TCPRETRANSSEGS	11
#endif
#if	!defined(SUNOS4) && !defined(AOS)
#define	TCPINERRS	12
#else
#undef	TCPINERRS
#endif
#ifndef AOS
#define	TCPOUTRSTS	13		
#endif


static int  
o_tcp (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register struct tcpstat *tcps = &tcpstat;
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

    switch (ifvar) {
	case TCPCURRESTAB:
	    if (get_connections (offset) == NOTOK)
		return int_SNMP_error__status_genErr;
	    break;

	default:
	    if (quantum != lastq) {
		lastq = quantum;

		if (getkmem (nl + N_TCPSTAT, (caddr_t) tcps, sizeof *tcps)
		        == NOTOK)
		    return int_SNMP_error__status_genErr;
	    }
	    break;
    }

    switch (ifvar) {
	case TCPRTOALGORITHM:
	    return o_integer (oi, v, RTOA_VANJ);

	case TCPRTOMIN:
	    return o_integer (oi, v, TCPTV_MIN * 100);	/* milliseconds */

#ifdef TCPRTOMAX
	case TCPRTOMAX:
	    return o_integer (oi, v, TCPTV_REXMTMAX * 100); /*   .. */
#endif

	case TCPMAXCONN:
	    return o_integer (oi, v, MXCN_NONE);

#ifdef TCPACTIVEOPENS
	case TCPACTIVEOPENS:
	    return o_integer (oi, v, tcps -> tcps_connattempt);
#endif

#ifdef TCPPASSIVEOPENS
	case TCPPASSIVEOPENS:
	    return o_integer (oi, v, tcps -> tcps_accepts);
#endif

#ifdef TCPATTEMPTFAILS
	case TCPATTEMPTFAILS:
	    return o_integer (oi, v, tcps -> tcps_conndrops);
#endif

#ifdef TCPESTABRESETS
	case TCPESTABRESETS:
	    return o_integer (oi, v, tcps -> tcps_drops);
#endif

	case TCPCURRESTAB:
	    return o_integer (oi, v, tcpConnections);

#ifdef TCPINSEGS
	case TCPINSEGS:
	    return o_integer (oi, v, tcps -> tcps_rcvtotal);
#endif

#ifdef TCPOUTSEGS
	case TCPOUTSEGS:
	    return o_integer (oi, v, tcps -> tcps_sndtotal
			           - tcps -> tcps_sndrexmitpack);
#endif

#ifdef TCPRETRANSSEGS
	case TCPRETRANSSEGS:
	    return o_integer (oi, v, tcps -> tcps_sndrexmitpack);
#endif

#ifdef	TCPINERRS
	case TCPINERRS:
	    return o_integer (oi, v, tcps -> tcps_rcvbadsum +
				     tcps -> tcps_rcvbadoff +
				     tcps -> tcps_rcvshort );
#endif

#ifdef TCPOUTRSTS
	case TCPOUTRSTS:
	    return o_integer (oi, v, tcps -> tcps_sndctrl); 
#endif

	default:
	    return int_SNMP_error__status_noSuchName;
    } 
}

/*  */
static  int     tcp_states[TCP_NSTATES];


struct tcptab {
#define	TT_SIZE	10			/* object instance */
    unsigned int   tt_instance[TT_SIZE];

    struct inpcb   tt_pcb;		/* protocol control block */

    struct socket  tt_socb;		/* socket info */

    struct tcpcb   tt_tcpb;		/* TCP info */

    struct tcptab *tt_next;
};

static struct tcptab *tts = NULL;
static  int     flush_tcp_cache = 0;


struct tcptab *get_tcpent ();

/*  */

#define	TCPCONNSTATE	0
#define	TCPCONNLOCALADDRESS 1
#define	TCPCONNLOCALPORT 2
#define	TCPCONNREMADDRESS 3
#define	TCPCONNREMPORT	4
#define	UNIXTCPCONNSENDQ 5
#define	UNIXTCPCONNRECVQ 6


static int  
o_tcp_conn (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct tcptab *tt;
    struct sockaddr_in netaddr;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    if (get_connections (offset) == NOTOK)
	return int_SNMP_error__status_genErr;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + TT_SIZE)
		return int_SNMP_error__status_noSuchName;
	    if ((tt = get_tcpent (oid -> oid_elements + oid -> oid_nelem
				      - TT_SIZE, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((tt = tts) == NULL)
		    return NOTOK;

		if ((new = oid_extend (ot -> ot_name, TT_SIZE)) == NULLOID)
		    return int_SNMP_error__status_genErr;
		ip = new -> oid_elements + new -> oid_nelem - TT_SIZE;
		jp = tt -> tt_instance;
		for (i = TT_SIZE; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		OID	new;

		if (oid->oid_nelem < TT_SIZE + ot -> ot_name -> oid_nelem)
		{
		    int i;

		    if ((new = oid_extend (oid, 
			  TT_SIZE+ot->ot_name->oid_nelem-oid->oid_nelem))==
			NULLOID)
			return int_SNMP_error__status_genErr;

		    for(i=0;i<TT_SIZE+ot->ot_name->oid_nelem-oid->oid_nelem;i++)
			new -> oid_elements[i + oid -> oid_nelem] = 0;
		}
		else if (oid->oid_nelem > TT_SIZE + ot -> ot_name -> oid_nelem)
		{
		    if ((new = oid_extend(ot -> ot_name, TT_SIZE)) == NULLOID)
			return int_SNMP_error__status_genErr;

		    for(i=0; i < TT_SIZE; i++)
			new -> oid_elements[i + ot -> ot_name -> oid_nelem] = 0;
		}
		else if ((new = oid_cpy(oid)) == NULLOID)
		    return int_SNMP_error__status_genErr;

		if ((tt = get_tcpent (ip = new -> oid_elements
				         + new -> oid_nelem - TT_SIZE, 1))
		        == NULL)
		{
		    oid_free(new);
		    return NOTOK;
		}

		jp = tt -> tt_instance;
		for (i = TT_SIZE; i > 0; i--)
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
	case TCPCONNSTATE:
	    if ((i = tt -> tt_tcpb.t_state) < 0
			|| i >= sizeof tcp_states / sizeof tcp_states[0])
		return generr (offset);
	    return o_integer (oi, v, tcp_states[i]);


	case TCPCONNLOCALADDRESS:
	    netaddr.sin_addr = tt -> tt_pcb.inp_laddr;	/* struct copy */
	    return o_ipaddr (oi, v, &netaddr);

	case TCPCONNLOCALPORT:
	    return o_integer (oi, v, tt -> tt_pcb.inp_lport & 0xffff);

	case TCPCONNREMADDRESS:
	    netaddr.sin_addr = tt -> tt_pcb.inp_faddr;	/* struct copy */
	    return o_ipaddr (oi, v, &netaddr);

	case TCPCONNREMPORT:
	    return o_integer (oi, v, tt -> tt_pcb.inp_fport & 0xffff);

#ifdef UNIXGROUP
	case UNIXTCPCONNSENDQ:
	    return o_integer (oi, v, tt -> tt_socb.so_snd.sb_cc);
	
	case UNIXTCPCONNRECVQ:
	    return o_integer (oi, v, tt -> tt_socb.so_rcv.sb_cc);
#endif /* UNIXGROUP */

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

static int  
tt_compar (a, b)
struct tcptab **a,
	      **b;
{
    return elem_cmp ((*a) -> tt_instance, TT_SIZE,
		     (*b) -> tt_instance, TT_SIZE);
}


static int  
get_connections (offset) 
int	offset;
{
    register int    i;
    register unsigned int  *cp;
    register struct tcptab *ts,
			   *tp,
			  **tsp;
    register struct inpcb  *ip;
    struct inpcb *head,
		  tcb;
    struct nlist nzs;
    register struct nlist *nz = &nzs;
    static   int first_time = 1;
    static   int lastq = -1;
    static   int tic = 0;

    if (quantum == lastq)
	return OK;
    if (!flush_tcp_cache
	    && offset == type_SNMP_PDUs_get__next__request
	    && quantum == lastq + 1
	    && tic < (TCPCONNENTRIES * tcpConnections)) { /* XXX: caching! */
	tic ++;
	lastq = quantum;
	return OK;
    }
    lastq = quantum, flush_tcp_cache = 0, tic = 0;

    for (ts = tts; ts; ts = tp) {
	tp = ts -> tt_next;

	free ((char *) ts);
    }
    tts = NULL;

    if (getkmem (nl + N_TCB, (char *) &tcb, sizeof tcb) == NOTOK)
	return NOTOK;
    head = (struct inpcb *) nl[N_TCB].n_value;

    tsp = &tts, i = 0;
    ip = &tcb;
    while (ip -> inp_next != head) {
	if ((ts = (struct tcptab *) calloc (1, sizeof *ts)) == NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		"get_connections");
			    /* no check needed for duplicate connections... */
	*tsp = ts, tsp = &ts -> tt_next, i++;

	nz -> n_name = "struct inpcb",
	nz -> n_value = (unsigned long) ip -> inp_next;
	if (getkmem (nz, (caddr_t) &ts -> tt_pcb, sizeof ts -> tt_pcb)
	        == NOTOK)
	    return NOTOK;
	ip = &ts -> tt_pcb;

	nz ->n_name = "struct socket",
	nz -> n_value = (unsigned long) ip -> inp_socket;
	if (getkmem (nz, (caddr_t) &ts -> tt_socb, sizeof ts -> tt_socb)
	        == NOTOK)
	    return NOTOK;

	nz ->n_name = "struct tcb",
	nz -> n_value = (unsigned long) ip -> inp_ppcb;
	if (getkmem (nz, (caddr_t) &ts -> tt_tcpb, sizeof ts -> tt_tcpb)
	        == NOTOK)
	    return NOTOK;

	cp = ts -> tt_instance;
	cp += ipaddr2oid (cp, &ts -> tt_pcb.inp_laddr);
	*cp++ = ts -> tt_pcb.inp_lport & 0xffff;
	cp += ipaddr2oid (cp, &ts -> tt_pcb.inp_faddr);
	*cp++ = ts -> tt_pcb.inp_fport & 0xffff;

	if (debug && first_time) {
	    OIDentifier	oids;

	    oids.oid_elements = ts -> tt_instance;
	    oids.oid_nelem = TT_SIZE;
	    advise (SLOG_DEBUG, MSGSTR(MS_TCP, TCP_1,
		    "add connection: %s"), sprintoid (&oids));
	}
    }
    first_time = 0;

    if ((tcpConnections = i) > 1) {
	register struct tcptab **base,
			       **tse;

	if ((base = (struct tcptab **) malloc ((unsigned) (i * sizeof *base)))
	        == NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		"get_connections");

	tse = base;
	for (ts = tts; ts; ts = ts -> tt_next)
	    *tse++ = ts;

#ifdef _ANSI_C_SOURCE
	qsort ((char *) base, i, sizeof *base, 
	       (int(*)(const void *, const void *))tt_compar);
#else /* !_ANSI_C_SOURCE */
	qsort ((char *) base, i, sizeof *base, tt_compar);
#endif /* _ANSI_C_SOURCE */

	tsp = base;
	ts = tts = *tsp++;

	while (tsp < tse) {
	    ts -> tt_next = *tsp;
	    ts = *tsp++;
	}
	ts -> tt_next = NULL;

	free ((char *) base);
    }
    return OK;    
}

/*  */

static struct tcptab *
get_tcpent (ip, isnext)
register unsigned int *ip;
int	isnext;
{
    register struct tcptab *tt;

    for (tt = tts; tt; tt = tt -> tt_next)
	switch (elem_cmp (tt -> tt_instance, TT_SIZE, ip, TT_SIZE)) {
	    case 0:
		if (!isnext)
		    return tt;
		if ((tt = tt -> tt_next) == NULL) {
		    flush_tcp_cache = 1;
		    return NULL;
		}
		/* else fall... */

	    case 1:
		return (isnext ? tt : NULL);
	}

    flush_tcp_cache = 1;
    return NULL;
}

/*  */

init_tcp () 
{
    register OT	    ot;

    if (ot = text2obj ("tcpRtoAlgorithm"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPRTOALGORITHM;
    if (ot = text2obj ("tcpRtoMin"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPRTOMIN;
#ifdef TCPRTOMAX
    if (ot = text2obj ("tcpRtoMax"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPRTOMAX;
#endif
    if (ot = text2obj ("tcpMaxConn"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPMAXCONN;
#ifdef TCPACTIVEOPENS
    if (ot = text2obj ("tcpActiveOpens"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPACTIVEOPENS;
#endif
#ifdef TCPPASSIVEOPENS
    if (ot = text2obj ("tcpPassiveOpens"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPPASSIVEOPENS;
#endif
#ifdef TCPATTEMPTFAILS
    if (ot = text2obj ("tcpAttemptFails"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPATTEMPTFAILS;
#endif
#ifdef TCPESTABRESETS
    if (ot = text2obj ("tcpEstabResets"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPESTABRESETS;
#endif
    if (ot = text2obj ("tcpCurrEstab"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPCURRESTAB;
#ifdef TCPINSEGS
    if (ot = text2obj ("tcpInSegs"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPINSEGS;
#endif
#ifdef TCPOUTSEGS
    if (ot = text2obj ("tcpOutSegs"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPOUTSEGS;
#endif
#ifdef TCPRETRANSSEGS
    if (ot = text2obj ("tcpRetransSegs"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPRETRANSSEGS;
#endif
#ifdef	TCPINERRS
    if (ot = text2obj ("tcpInErrs"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPINERRS;
#endif
#ifdef	TCPOUTRSTS
    if (ot = text2obj ("tcpOutRsts"))
	ot -> ot_getfnx = o_tcp,
	ot -> ot_info = (caddr_t) TCPOUTRSTS;
#endif

    if (ot = text2obj ("tcpConnState"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) TCPCONNSTATE;

    if (ot = text2obj ("tcpConnLocalAddress"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) TCPCONNLOCALADDRESS;

    if (ot = text2obj ("tcpConnLocalPort"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) TCPCONNLOCALPORT;

    if (ot = text2obj ("tcpConnRemAddress"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) TCPCONNREMADDRESS;

    if (ot = text2obj ("tcpConnRemPort"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) TCPCONNREMPORT;

#ifdef UNIXGROUP
    if (ot = text2obj ("unixTcpConnSendQ"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) UNIXTCPCONNSENDQ;
    if (ot = text2obj ("unixTcpConnRecvQ"))
	ot -> ot_getfnx = o_tcp_conn,
	ot -> ot_info = (caddr_t) UNIXTCPCONNRECVQ;
#endif /* UNIXGROUP */
    tcp_states[TCPS_CLOSED] = 1;
    tcp_states[TCPS_LISTEN] = 2;
    tcp_states[TCPS_SYN_SENT] = 3;
    tcp_states[TCPS_SYN_RECEIVED] = 4;
    tcp_states[TCPS_ESTABLISHED] = 5;
    tcp_states[TCPS_CLOSE_WAIT] = 8;
    tcp_states[TCPS_FIN_WAIT_1] = 6;
    tcp_states[TCPS_CLOSING] = 10;
    tcp_states[TCPS_LAST_ACK] = 9;
    tcp_states[TCPS_FIN_WAIT_2] = 7;
    tcp_states[TCPS_TIME_WAIT] = 11;
}
