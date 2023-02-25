static char sccsid[] = "@(#)32	1.5  src/tcpip/usr/sbin/snmpd/unix_g.c, snmp, tcpip411, GOLD410 10/29/93 10:41:03";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: init_unix(), o_mbuf(), o_mbufType(), init_mbuf()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/unix_g.c
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

#ifdef	UNIXGROUP

#include "snmpd.h"

static	int	lastq = -1;
static	int	unix_netstat = 1;

static struct nlist lnl[] = {
#define	N_MBSTAT	0
#if	defined(_POWER) || defined(ps2)
#define NELEM_NLIST 1
    { "mbstat" },
#else
    { "_mbstat" },
#endif

    NULL
};


init_unix () 
{
    register struct nlist *nz;
    OT ot;

#ifdef _POWER
    if (knlist (lnl, NELEM_NLIST, sizeof (struct nlist)) == NOTOK)
#else
    if (nlist (UNIX, lnl) == NOTOK)
#endif
	adios (MSGSTR(MS_UNIXG, UNIXG_1, "cannot nlist"));
    for (nz = lnl; nz -> n_name; nz++)
	if (nz -> n_value == 0)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_UNIXG, UNIXG_2,
		    "\"%s\" not in /vmunix (warning)"),
		    nz -> n_name);

    init_mbuf ();	    /* UNIX-specific enterprise */

    if (ot = text2obj ("unixNetstat"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &unix_netstat;
}


/*    UNIX */

#if	defined(BSD44) || defined(_POWER)
#include <sys/param.h>
#endif
#include <sys/mbuf.h>

/*  */

static	struct mbstat mbstat;

/*  */

#define	mbufS		0
#define	mbufClusters	1
#define	mbufFreeClusters 2
#define	mbufDrops	3
#if	defined(BSD44) || defined(_POWER)
#define	mbufWaits	4
#define	mbufDrains	5
#endif
#if	!defined(BSD43) && !defined(BSD44) && !defined(RT)
#define	mbufFrees	6
#endif


static int  
o_mbuf (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register struct mbstat *m = &mbstat;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

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

	if (getkmem (lnl + N_MBSTAT, (caddr_t) m, sizeof *m) == NOTOK)
	    return (offset == type_SNMP_PDUs_get__next__request ? NOTOK
					: int_SNMP_error__status_genErr);
    }

    switch (ifvar) {
	case mbufS:
	    return o_integer (oi, v, m -> m_mbufs);

	case mbufClusters:
	    return o_integer (oi, v, m -> m_clusters);

	case mbufFreeClusters:
	    return o_integer (oi, v, m -> m_clfree);

	case mbufDrops:
	    return o_integer (oi, v, m -> m_drops);

#ifdef	mbufWaits
	case mbufWaits:
	    return o_integer (oi, v, m -> m_wait);
#endif

#ifdef	mbufDrains
	case mbufDrains:
	    return o_integer (oi, v, m -> m_drain);
#endif

#ifdef	mbufFrees
	case mbufFrees:
	    return o_integer (oi, v, m -> m_mbfree);
#endif

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

#define	mbufType	0
#define	mbufAllocates	1


static int  
o_mbufType (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifnum,
	    ifvar;
    register struct mbstat *m = &mbstat;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    if ((ifnum = oid -> oid_elements[oid -> oid_nelem - 1])
		    >= sizeof m -> m_mtypes / sizeof m -> m_mtypes[0])
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
again: ;
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		ifnum = 0;

		if ((new = oid_extend (oid, 1)) == NULLOID)
		    return NOTOK;
		new -> oid_elements[new -> oid_nelem - 1] = ifnum;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;

		oid = new;	/* for hack... */
	    }
	    else {
		int	i = ot -> ot_name -> oid_nelem;

		if ((ifnum = oid -> oid_elements[i] + 1)
		        >= sizeof m -> m_mtypes / sizeof m -> m_mtypes[0])
		    return NOTOK;

		oid -> oid_elements[i] = ifnum;
		oid -> oid_nelem = i + 1;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    if (quantum != lastq) {
	lastq = quantum;

	if (getkmem (lnl + N_MBSTAT, (caddr_t) m, sizeof *m) == NOTOK)
	    return (offset == type_SNMP_PDUs_get__next__request ? NOTOK
					: int_SNMP_error__status_genErr);
    }

/* hack to compress table size... */
    if (offset == type_SNMP_PDUs_get__next__request
	    && m -> m_mtypes[ifnum] == 0)
	goto again;

    switch (ifvar) {
	case mbufType:
	    return o_integer (oi, v, ifnum);

	case mbufAllocates:
	    return o_integer (oi, v, m -> m_mtypes[ifnum]);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

init_mbuf () 
{
    register OT	    ot;

    if (ot = text2obj ("mbufS"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufS;
    if (ot = text2obj ("mbufClusters"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufClusters;
    if (ot = text2obj ("mbufFreeClusters"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufFreeClusters;
    if (ot = text2obj ("mbufDrops"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufDrops;
#ifdef	mbufWaits
    if (ot = text2obj ("mbufWaits"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufWaits;
#endif
#ifdef	mbufDrains
    if (ot = text2obj ("mbufDrains"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufDrains;
#endif
#ifdef	mbufFrees
    if (ot = text2obj ("mbufFrees"))
	ot -> ot_getfnx = o_mbuf,
	ot -> ot_info = (caddr_t) mbufFrees;
#endif
    if (ot = text2obj ("mbufType"))
	ot -> ot_getfnx = o_mbufType,
	ot -> ot_info = (caddr_t) mbufType;
    if (ot = text2obj ("mbufAllocates"))
	ot -> ot_getfnx = o_mbufType,
	ot -> ot_info = (caddr_t) mbufAllocates;
}

#endif /* UNIXGROUP */
