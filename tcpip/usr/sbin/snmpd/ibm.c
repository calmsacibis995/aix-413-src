static char sccsid[] = "@(#)97	1.4  src/tcpip/usr/sbin/snmpd/ibm.c, snmp, tcpip411, GOLD410 2/18/94 11:30:38";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: s_debug(), init_ibm()
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/ibm.c
 */

#include "snmpd.h"

extern int debug;

/*    IBM GROUP */

static int  
s_debug (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    int	rc, ldebug;

    if ((rc = s_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    switch (offset) {
	/* decode and check our value */
	case type_SNMP_PDUs_set__request:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    ldebug = *((int *)(ot -> ot_save));
	    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if (ldebug < 0)
		return int_SNMP_error__status_badValue;
	    break;

	/* perform the set */
	case type_SNMP_PDUs_commit:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    debug = *((int *)(ot -> ot_save));
	    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;

	case type_SNMP_PDUs_rollback:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;
    }
    return int_SNMP_error__status_noError;
}


init_ibm ()
{
    register OT	    ot;

#ifdef NATIVE
    /* for debug purposes... let us keep track of the build */
    if (ot = text2obj ("build")) {
	ot -> ot_getfnx = o_generic;
	if (ot -> ot_syntax)
	    (void) (*ot -> ot_syntax -> os_parse) 
		    ((struct qbuf **) &ot -> ot_info, BUILD);
    }
#endif
    if (ot = text2obj ("snmpdDebug"))
	ot -> ot_getfnx = o_generic,
	ot -> ot_setfnx = s_debug,
	ot -> ot_info = (caddr_t) &debug;
}
