static char sccsid[] = "@(#)70	1.1.1.4  src/tcpip/usr/lib/libsnmp/values.c, snmp, tcpip411, GOLD410 10/29/93 09:58:57";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_igeneric(), o_generic(), s_generic(), o_number(),
 *            o_string(), o_qbstring(), o_specific(), o_ipaddr(), 
 *            mediaddr2oid(), oid_extend(), oid_normalize(), o_icheck(), 
 *            s_icheck()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/values.c
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

/* values.c - encode values */

#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <isode/pepsy/SNMP-types.h>
#include <isode/pepsy/SMI-types.h>
#include <isode/snmp/objects.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"
	    
/*  */

#define	ADVISE	if (o_advise) (*o_advise)

IFP	o_advise = NULLIFP;
IFP	o_adios = NULLIFP;

/*  */
/*
 * initializable generic function.
 */
int	
o_igeneric (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    return o_generic (oi, v, offset);
}

int	
o_generic (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    int	rc;

    if ((rc = o_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;

    if (os == NULLOS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return (offset == type_SNMP_PDUs_get__next__request ? NOTOK
		: int_SNMP_error__status_genErr);
    }
    if (ot -> ot_info == NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_2,
		"no value defined for object \"%s\""), ot -> ot_text);

	return (offset == type_SNMP_PDUs_get__next__request ? NOTOK
		: int_SNMP_error__status_noSuchName);
    }

    if (v -> value)
	free_SMI_ObjectSyntax (v -> value), v -> value = NULL;
    if ((*os -> os_encode) (ot -> ot_info, &v -> value) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_3,
		"encoding error for variable \"%s\""), oid2ode (oi -> oi_name));

	return int_SNMP_error__status_genErr;
    }

    return int_SNMP_error__status_noError;
}

/*  */
/* generic set function */
int
s_generic (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    int	rc;

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
	    if (ot -> ot_info)
		(*os -> os_free) (ot -> ot_info);
	    ot -> ot_info = ot -> ot_save, ot -> ot_save = NULL;
	    break;

	case type_SNMP_PDUs_rollback:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;
    }

    return int_SNMP_error__status_noError;
}
	    
/*  */

int
o_number (oi, v, number)
OI	oi;
register struct type_SNMP_VarBind *v;
integer	number;
{
    int	    result;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;

    if (os == NULLOS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }

    if (v -> value)
	free_SMI_ObjectSyntax (v -> value), v -> value = NULL;
    result = (*os -> os_encode) (&number, &v -> value);

    if (result == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_3,
		"encoding error for variable \"%s\""), oid2ode (oi -> oi_name));

	return int_SNMP_error__status_genErr;
    }

    return int_SNMP_error__status_noError;
}
	    
/*  */

int	
o_string (oi, v, base, len)
OI	oi;
register struct type_SNMP_VarBind *v;
char   *base;
int	len;
{
    int	    result;
    struct qbuf *value;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;

    if (os == NULLOS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }

    if ((value = str2qb (base, len, 1)) == NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1,
					"%s: Out of Memory"), "o_string");
	return int_SNMP_error__status_genErr;
    }

    if (v -> value)
	free_SMI_ObjectSyntax (v -> value), v -> value = NULL;
    result = (*os -> os_encode) (value, &v -> value);

    /* free the memory used by value */
    qb_free (value);

    if (result == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_3, 
		"encoding error for variable \"%s\""), oid2ode (oi -> oi_name));

	return int_SNMP_error__status_genErr;
    }

    return int_SNMP_error__status_noError;
}

/*  */

int	
o_qbstring (oi, v, value)
OI	oi;
register struct type_SNMP_VarBind *v;
struct qbuf *value;
{
    int	    result;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;

    if (os == NULLOS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }

    if (v -> value)
	free_SMI_ObjectSyntax (v -> value), v -> value = NULL;
    result = (*os -> os_encode) (value, &v -> value);

    if (result == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_3,
		"encoding error for variable \"%s\""), oid2ode (oi -> oi_name));

	return int_SNMP_error__status_genErr;
    }

    return int_SNMP_error__status_noError;
}

/*  */

int	
o_specific (oi, v, value)
OI	oi;
register struct type_SNMP_VarBind *v;
caddr_t	value;
{
    int	    result;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;

    if (os == NULLOS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }

    if (v -> value)
	free_SMI_ObjectSyntax (v -> value), v -> value = NULL;
    result = (*os -> os_encode) (value, &v -> value);

    if (result == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_3,
		"encoding error for variable \"%s\""), oid2ode (oi -> oi_name));

	return int_SNMP_error__status_genErr;
    }

    return int_SNMP_error__status_noError;
}

/*  */

int	
o_ipaddr (oi, v, netaddr)
OI	oi;
register struct type_SNMP_VarBind *v;
struct sockaddr_in *netaddr;
{
    int	ret;
    struct qbuf *value = str2qb (&netaddr -> sin_addr, 4, 1);	/* len == 4 */

    if (value == NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_3,
		"encoding error for variable \"%s\""), oid2ode (oi -> oi_name));

	return int_SNMP_error__status_genErr;
    }
    ret = o_qbstring (oi, v, value);	/* ipaddr syntax expects a qbuf */
    qb_free (value);			/* qbuf gets copied in syntax.c */
    return ret;
}

/*  */

int	
mediaddr2oid (ip, addr, len, islen)
register unsigned int *ip;
register u_char *addr;
int	len,
	islen;
{
    register int   i;

    if (islen)
	*ip++ = len & 0xff;

    for (i = len; i > 0; i--)
	*ip++ = *addr++ & 0xff;

    return (len + (islen ? 1 : 0));
}

/*  */

OID	
oid_extend (q, howmuch)
register OID	q;
int	howmuch;
{
    register unsigned int   i,
			   *ip,
			   *jp;
    OID	    oid;

    if (q == NULLOID)
	return NULLOID;
    if ((i = q -> oid_nelem) < 1)
	return NULLOID;
    if ((oid = (OID) malloc (sizeof *oid)) == NULLOID)
	return NULLOID;

    if ((ip = (unsigned int *)
	 	    calloc ((unsigned) (i + howmuch + 1), sizeof *ip))
		== NULL) {
	free ((char *) oid);
	return NULLOID;
    }

    oid -> oid_elements = ip, oid -> oid_nelem = i + howmuch;

    for (i = 0, jp = q -> oid_elements; i < oid -> oid_nelem; i++, jp++)
	*ip++ = *jp;

    return oid;
}

/*  */

OID	
oid_normalize (q, howmuch, bigvalue)
register OID	q;
int	howmuch,
	bigvalue;
{
    register int	i;
    register unsigned int   *ip,
			    *jp;
    OID	    oid;

    if ((oid = oid_extend (q, howmuch)) == NULL)
	return NULLOID;

    for (jp = (ip = oid -> oid_elements + q -> oid_nelem) - 1;
	     jp >= oid -> oid_elements;
	     jp--)
	if (*jp > 0) {
	    *jp -= 1;
	    break;
	}
    for (i = howmuch; i > 0; i--)
	*ip++ = (unsigned int) bigvalue;

    return oid;
}

/*
 *	FUNCTION: o_icheck
 *
 *	PURPOSE: check that our instance for get is .0, or that our
 *		calculated next is .0
 */
int
o_icheck (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

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
    return int_SNMP_error__status_noError;
}

/*
 *	FUNCTION: s_icheck
 *
 *	PURPOSE: check that our instance for set is .0
 */
int
s_icheck (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;

    switch (offset) {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1
		    || oid -> oid_elements[oid -> oid_nelem - 1] != 0)
		return int_SNMP_error__status_noSuchName;
	    break;

	default:
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_4,
		    "GENERR in s_icheck: bad offset %d"), offset);
	    return int_SNMP_error__status_genErr;
    }
	
    if (os == NULLOS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_VALUES, VALUES_5,
		"GENERR in s_icheck: no syntax defined for object \"%s\""), 
		ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }
    return int_SNMP_error__status_noError;
}
