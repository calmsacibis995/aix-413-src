static char sccsid[] = "@(#)26	1.6  src/tcpip/usr/sbin/snmpinfo/process.c, snmp, tcpip411, GOLD410 10/29/93 10:06:45";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: snmp_error(), process(), process_varbinds()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpinfo/process.c
 */

#include <isode/pepsy/SNMP-types.h>
#include <isode/snmp/objects.h>
#include <stdio.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

extern int Verbose;	/* defined in syntax.c */
extern int nvars; 	/* defined in snmpinfo.c */
#ifdef LOCAL
extern int timing;	/* defined in snmpinfo.c */
#endif /* LOCAL */

static char *errors[] = {
    "noError", "tooBig", "noSuchName", "badValue", "readOnly", "genErr"
};


char   *
snmp_error (i)
integer	i;
{
    static char buffer[BUFSIZ];

    if (0 < i && i < sizeof errors / sizeof errors[0])
	return errors[i];
    (void) sprintf (buffer, MSGSTR(MS_SINFO, SINFO_28, "error %d"), i);

    return buffer;
}

int  
process (msg)
struct type_SNMP_Message *msg;
{
    int		status;
    register struct type_SNMP_PDU *parm;

    if (msg == NULL)
	return OK;

    if ((status = sndrcv_msg (&msg)) != OK)
	return status;

    switch (msg -> data -> offset) {
	case type_SNMP_PDUs_get__response:
	    parm = msg -> data -> un.get__response;
	    break;
	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SINFO, SINFO_13,
		    "unexpected message type %d"),
		    msg -> data -> offset);
	    status = NOTOK;
	    goto out;
    }

    if (parm -> error__status != int_SNMP_error__status_noError) {
	fprintf (stderr, MSGSTR(MS_SINFO, SINFO_14, "%s at position %d\n"),
		snmp_error (parm -> error__status), parm -> error__index);
	status = NOTOK;
	goto out;
    }
    status = process_varbinds (parm -> variable__bindings, NULLOID);

 out: ;
    if (msg)
	free_SNMP_Message (msg);
    return (status);
}

int  
process_varbinds (vp, oid)
struct type_SNMP_VarBindList *vp;
OID	oid;
{
    for (; vp; vp = vp -> next) {
	caddr_t	 value;
	register OI	oi;
	register OS	os;
	register struct type_SNMP_VarBind *v = vp -> VarBind;

	nvars++;

	/* check stop point (dump) */
	if (oid
	    && (oid -> oid_nelem > v -> name -> oid_nelem
		|| bcmp ((char *) oid -> oid_elements,
			(char *) v -> name -> oid_elements,
			oid -> oid_nelem
			    * sizeof oid -> oid_elements[0]))) {
	    return DONE;
	}

	printf ("%s = ", Verbose ? oid2ode (v -> name) : sprintoid (v -> name));
	if ((oi = name2inst (v -> name)) == NULL
		|| (os = oi -> oi_type -> ot_syntax) == NULL
		|| (*os -> os_decode) (&value, v -> value) == NOTOK)
	    generic_print (v -> value);
	else {
	    (*os -> os_print) (value, os);

	    (*os -> os_free) (value);
	}
	printf ("\n");
    }

#ifdef LOCAL
    if (timing)
	displayTime ();
#endif /* LOCAL */
    return OK;
}
