static char sccsid[] = "@(#)12	1.5  src/tcpip/usr/sbin/snmpd/snmp_g.c, snmp, tcpip411, GOLD410 2/18/94 11:34:22";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: s_snmp_auth_traps(), init_snmp()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/snmp_g.c
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#include "snmpd.h"

/*    SNMP GROUP */

struct snmpstat snmpstat;

/*
 *	FUNCTION: s_snmp_auth_traps ()
 *
 *	PURPOSE: set mib variable snmpEnableAuthTraps
 */
static int  
s_snmp_auth_traps (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    int	rc, etraps;

    if ((rc = s_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    switch (offset) {

	/* decode and check our value */
	case type_SNMP_PDUs_set__request:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    etraps = *((int *)(ot -> ot_save));
	    if (etraps != TRAPS_ENABLED && etraps != TRAPS_DISABLED)
		return int_SNMP_error__status_badValue;
	    break;

	/* perform the set */
	case type_SNMP_PDUs_commit:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    snmpstat.s_enableauthentraps = *((int *)(ot -> ot_save));
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
 *	FUNCTION: init_snmp ()
 *
 *	PURPOSE:  initialize the SNMP mib variables
 */
init_snmp ()
{
    register OT	    ot;

    if (ot = text2obj ("snmpInPkts"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_inpkts;
    if (ot = text2obj ("snmpOutPkts"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outpkts;
    if (ot = text2obj ("snmpInBadVersions"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_badversions;
    if (ot = text2obj ("snmpInBadCommunityNames"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_badcommunitynames;
    if (ot = text2obj ("snmpInBadCommunityUses"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_badcommunityuses;
    if (ot = text2obj ("snmpInASNParseErrs"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_asnparseerrs;
    if (ot = text2obj ("snmpInTotalReqVars"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_totalreqvars;
    if (ot = text2obj ("snmpInTotalSetVars"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_totalsetvars;

    /* input error status */
    if (ot = text2obj ("snmpInTooBigs"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_intoobigs;
    if (ot = text2obj ("snmpInNoSuchNames"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_innosuchnames;
    if (ot = text2obj ("snmpInBadValues"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_inbadvalues;
    if (ot = text2obj ("snmpInReadOnlys"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_inreadonlys;
    if (ot = text2obj ("snmpInGenErrs"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_ingenerrs;

    /* input packet type */
    if (ot = text2obj ("snmpInGetRequests"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_ingetrequests;
    if (ot = text2obj ("snmpInGetNexts"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_ingetnexts;
    if (ot = text2obj ("snmpInSetRequests"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_insetrequests;
    if (ot = text2obj ("snmpInGetResponses"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_ingetresponses;
    if (ot = text2obj ("snmpInTraps"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_intraps;

    /* output error status */
    if (ot = text2obj ("snmpOutTooBigs"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outtoobigs;
    if (ot = text2obj ("snmpOutNoSuchNames"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outnosuchnames;
    if (ot = text2obj ("snmpOutBadValues"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outbadvalues;
    if (ot = text2obj ("snmpOutGenErrs"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outgenerrs;

    /* output packet type */
    if (ot = text2obj ("snmpOutGetRequests"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outgetrequests;
    if (ot = text2obj ("snmpOutGetNexts"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outgetnexts;
    if (ot = text2obj ("snmpOutSetRequests"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outsetrequests;
    if (ot = text2obj ("snmpOutGetResponses"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outgetresponses;
    if (ot = text2obj ("snmpOutTraps"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_info = (caddr_t) &snmpstat.s_outtraps;

    if (ot = text2obj ("snmpEnableAuthenTraps"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_setfnx = s_snmp_auth_traps,
	ot -> ot_info = (caddr_t) &snmpstat.s_enableauthentraps;
    snmpstat.s_enableauthentraps = TRAPS_ENABLED;
}
