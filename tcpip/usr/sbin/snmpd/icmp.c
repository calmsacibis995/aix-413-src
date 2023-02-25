static char sccsid[] = "@(#)98	1.5  src/tcpip/usr/sbin/snmpd/icmp.c, snmp, tcpip411, GOLD410 3/3/93 08:24:07";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_icmp(), init_icmp()
 *
 * ORIGINS: 27 60
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/icmp.c
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

/* icmp.c - MIB realization of the ICMP group */


#include "snmpd.h"

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>

/*  */

static struct icmpstat icmpstat;

/*  */

#ifdef _POWER
#define	ICMPINMSGS	0
#endif
#define	ICMPINERRORS	1
#define	ICMPINDESTUNREACHS 2
#define	ICMPINTIMEEXCDS	3
#define	ICMPINPARMPROBS	4
#define	ICMPINSRCQUENCHS 5
#define	ICMPINREDIRECTS	6
#define	ICMPINECHOS	7
#define	ICMPINECHOREPS	8
#define	ICMPINTIMESTAMPS 9
#define	ICMPINTIMESTAMPREPS 10
#define	ICMPINADDRMASKS	11
#define	ICMPINADDRMASKREPS 12
#define	ICMPOUTMSGS	13
#define	ICMPOUTERRORS	14
#define	ICMPOUTDESTUNREACHS 15
#define	ICMPOUTTIMEEXCDS 16
#define	ICMPOUTPARMPROBS 17
#define	ICMPOUTSRCQUENCHS 18
#define	ICMPOUTREDIRECTS 19
#define	ICMPOUTECHOS	20
#define	ICMPOUTECHOREPS	21
#define	ICMPOUTTIMESTAMPS 22
#define	ICMPOUTTIMESTAMPREPS 23
#define	ICMPOUTADDRMASKS 24
#define	ICMPOUTADDRMASKREPS 25


static int  
o_icmp (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register struct icmpstat *icps = &icmpstat;
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

	if (getkmem (nl + N_ICMPSTAT, (caddr_t) icps, sizeof *icps) == NOTOK)
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
#ifdef ICMPINMSGS
	case ICMPINMSGS:
#if 	defined(_POWER) && defined (BSD44)
            return o_integer (oi, v, icps -> icps_badcode
			           + icps -> icps_checksum
				   + icps -> icps_badlen
				   + icps -> icps_tooshort
			     	   + icps -> icps_inhist[ICMP_UNREACH]
				   + icps -> icps_inhist[ICMP_TIMXCEED]
				   + icps -> icps_inhist[ICMP_PARAMPROB]
			           + icps -> icps_inhist[ICMP_SOURCEQUENCH]
				   + icps -> icps_inhist[ICMP_REDIRECT]
				   + icps -> icps_inhist[ICMP_ECHO]
			           + icps -> icps_inhist[ICMP_ECHOREPLY]
				   + icps -> icps_inhist[ICMP_TSTAMP]
			           + icps -> icps_inhist[ICMP_TSTAMPREPLY]
				   + icps -> icps_inhist[ICMP_MASKREQ]
				   + icps -> icps_inhist[ICMP_MASKREPLY]);
#else
	    return o_integer (oi, v, icps -> icps_icmpInMsgs);
#endif
#endif 	/* ICMPINMSGS */

	case ICMPINERRORS:
	    return o_integer (oi, v, icps -> icps_badcode
			           + icps -> icps_checksum
			           + icps -> icps_badlen
				   + icps -> icps_tooshort);

	case ICMPINDESTUNREACHS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_UNREACH]);

	case ICMPINTIMEEXCDS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_TIMXCEED]);

	case ICMPINPARMPROBS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_PARAMPROB]);

	case ICMPINSRCQUENCHS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_SOURCEQUENCH]);

	case ICMPINREDIRECTS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_REDIRECT]);

	case ICMPINECHOS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_ECHO]);

	case ICMPINECHOREPS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_ECHOREPLY]);

	case ICMPINTIMESTAMPS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_TSTAMP]);

	case ICMPINTIMESTAMPREPS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_TSTAMPREPLY]);

	case ICMPINADDRMASKS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_MASKREQ]);

	case ICMPINADDRMASKREPS:
	    return o_integer (oi, v, icps -> icps_inhist[ICMP_MASKREPLY]);

	case ICMPOUTMSGS:
	    return o_integer (oi, v, icps -> icps_error + icps -> icps_reflect);

	case ICMPOUTERRORS:
	    return o_integer (oi, v, icps -> icps_error);

	case ICMPOUTDESTUNREACHS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_UNREACH]);

	case ICMPOUTTIMEEXCDS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_TIMXCEED]);

	case ICMPOUTPARMPROBS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_PARAMPROB]);

	case ICMPOUTSRCQUENCHS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_SOURCEQUENCH]);

	case ICMPOUTREDIRECTS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_REDIRECT]);

	case ICMPOUTECHOS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_ECHO]);

	case ICMPOUTECHOREPS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_ECHOREPLY]);

	case ICMPOUTTIMESTAMPS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_TSTAMP]);

	case ICMPOUTTIMESTAMPREPS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_TSTAMPREPLY]);

	case ICMPOUTADDRMASKS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_MASKREQ]);

	case ICMPOUTADDRMASKREPS:
	    return o_integer (oi, v, icps -> icps_outhist[ICMP_MASKREPLY]);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

/*  */

init_icmp () 
{
    register OT	    ot;

#ifdef ICMPINMSGS
    if (ot = text2obj ("icmpInMsgs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINMSGS;
#endif
    if (ot = text2obj ("icmpInErrors"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINERRORS;
    if (ot = text2obj ("icmpInDestUnreachs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINDESTUNREACHS;
    if (ot = text2obj ("icmpInTimeExcds"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINTIMEEXCDS;
    if (ot = text2obj ("icmpInParmProbs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINPARMPROBS;
    if (ot = text2obj ("icmpInSrcQuenchs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINSRCQUENCHS;
    if (ot = text2obj ("icmpInRedirects"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINREDIRECTS;
    if (ot = text2obj ("icmpInEchos"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINECHOS;
    if (ot = text2obj ("icmpInEchoReps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINECHOREPS;
    if (ot = text2obj ("icmpInTimestamps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINTIMESTAMPS;
    if (ot = text2obj ("icmpInTimestampReps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINTIMESTAMPREPS;
    if (ot = text2obj ("icmpInAddrMasks"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINADDRMASKS;
    if (ot = text2obj ("icmpInAddrMaskReps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPINADDRMASKREPS;
    if (ot = text2obj ("icmpOutMsgs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTMSGS;
    if (ot = text2obj ("icmpOutErrors"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTERRORS;
    if (ot = text2obj ("icmpOutDestUnreachs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTDESTUNREACHS;
    if (ot = text2obj ("icmpOutTimeExcds"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTTIMEEXCDS;
    if (ot = text2obj ("icmpOutParmProbs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTPARMPROBS;
    if (ot = text2obj ("icmpOutSrcQuenchs"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTSRCQUENCHS;
    if (ot = text2obj ("icmpOutRedirects"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTREDIRECTS;
    if (ot = text2obj ("icmpOutEchos"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTECHOS;
    if (ot = text2obj ("icmpOutEchoReps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTECHOREPS;
    if (ot = text2obj ("icmpOutTimestamps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTTIMESTAMPS;
    if (ot = text2obj ("icmpOutTimestampReps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTTIMESTAMPREPS;
    if (ot = text2obj ("icmpOutAddrMasks"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTADDRMASKS;
    if (ot = text2obj ("icmpOutAddrMaskReps"))
	ot -> ot_getfnx = o_icmp,
	ot -> ot_info = (caddr_t) ICMPOUTADDRMASKREPS;
}
