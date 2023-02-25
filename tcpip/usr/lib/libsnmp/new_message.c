static char sccsid[] = "@(#)58	1.6  src/tcpip/usr/lib/libsnmp/new_message.c, snmp, tcpip411, GOLD410 10/29/93 09:58:40";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: new_message(), get_ava()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/new_message.c
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

#include <isode/pepsy/SNMP-types.h>
#include <isode/snmp/objects.h>
#include <stdio.h>
#include <string.h> 
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

extern int SNMP_requests;

#define	ADVISE		if (o_advise) (*o_advise)
#define	ADIOS		if (o_adios) (*o_adios)

struct type_SNMP_Message *
new_message (offset, vec, community)
int	offset;
char  **vec;
char *community;
{
    register struct type_SNMP_Message *msg;
    register struct type_SNMP_PDUs *pdu;
    register struct type_SNMP_PDU *parm;
    register struct type_SNMP_VarBindList **vp;

    if ((msg = (struct type_SNMP_Message *) calloc (1, sizeof *msg)) == NULL)
	ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "new_message");

    msg -> version = int_SNMP_version_version__1;

    if ((msg -> community = str2qb (community, strlen (community), 1)) == NULL)
	ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "new_message");

    if ((pdu = (struct type_SNMP_PDUs *) calloc (1, sizeof *pdu)) == NULL)
	ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "new_message");
    msg -> data = pdu;

    pdu -> offset = offset;
    
/* for now, always a PDU... */

    if ((parm = (struct type_SNMP_PDU *) calloc (1, sizeof *parm)) == NULL)
	ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "new_message");
    pdu -> un.get__request = parm;

    parm -> request__id = ++SNMP_requests;

    vp = &parm -> variable__bindings;
    for (vec; *vec; vec++) {
	register struct type_SNMP_VarBindList *bind;
	register struct type_SNMP_VarBind *v;

	if ((bind = (struct type_SNMP_VarBindList *) calloc (1, sizeof *bind))
	    	    == NULL)
	    ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
		   "new_message");
	*vp = bind, vp = &bind -> next;

	if ((v = (struct type_SNMP_VarBind *) calloc (1, sizeof *v)) == NULL)
	    ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), 
			  "new_message");
	bind -> VarBind = v;

	if (get_ava (v, *vec, offset) == NOTOK) {
	    free_SNMP_Message (msg);
	    return NULL;
	}
    }

    return msg;
}

/*  */

static int  get_ava (v, ava, offset)
register struct type_SNMP_VarBind *v;
char   *ava;
int	offset;
{
    int	    result;
    caddr_t value;
    register char *cp;
    register OI	   oi;
    register OT	   ot;
    register OS	   os;
    OID	    oid = NULL;

    if (cp = index (ava, '=')) {
	if (offset != type_SNMP_PDUs_set__request)
	    ADVISE (SLOG_NOTICE, MSGSTR(MS_NEWMESS, NEWMESS_1, 
					"value unnecessary for get operation"));
	*cp++ = NULL;
    }
    else
	if (offset == type_SNMP_PDUs_set__request) {
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_NEWMESS, NEWMESS_2,
				"need variable=value for set operation"));
	    return NOTOK;
	}

    if ((oi = text2inst (ava)) == NULL) {
	if (cp || (oid = text2oid (ava)) == NULL) {
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_NEWMESS, NEWMESS_3,
					"unknown variable \"%s\""), ava);
	    return NOTOK;
	}

	ot = NULLOT;
    }
    else
	ot = oi -> oi_type;

    if ((v -> name = oid_cpy (oi ? oi -> oi_name : oid)) == NULLOID)
	ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_ava");

    if (cp == NULL) {			/* get request */
	if ((v -> value = alloc_SMI_ObjectSyntax ()) == NULL) 
	    ADIOS (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "get_ava");
    }
    else {				/* set request */
	if ((os = ot -> ot_syntax) == NULL) {
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_NEWMESS, NEWMESS_4,
		    "no syntax defined for object \"%s\""), ava);
	    return NOTOK;
	}

	if ((*os -> os_parse) (&value, cp) == NOTOK) {
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_NEWMESS, NEWMESS_5,
		    "invalid value for variable \"%s\": \"%s\""),
		    ava, cp);
	    return NOTOK;
	}
	result = (*os -> os_encode) (value, &v -> value);
	(*os -> os_free) (value);

	if (result == NOTOK) {
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_NEWMESS, NEWMESS_6, 
				"encoding error for variable \"%s\""), ava);
	    return NOTOK;
	}	
    }

    if (oi == NULL)
	oid_free (oid);

    return OK;
}

