static char sccsid[] = "@(#)60	1.2  src/tcpip/usr/lib/libsnmp/oid_ocpy.c, snmp, tcpip411, GOLD410 3/2/93 18:24:59";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: oid_ocpy()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/oid_ocpy.c
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* oid_ocpy.c - copy an object identifier, starting at some offset */

/* LINTLIBRARY */

#include <stdio.h>
#include <isode/psap.h>

/*  */

OID
oid_ocpy (q, o)
register OID q;
register int o;
{
    register unsigned int   i,
			   *ip,
			   *jp;
    register OID	oid;

    if (q == NULLOID)
	return NULLOID;
    if ((i = q -> oid_nelem - o) < 1)
	return NULLOID;
    if ((oid = (OID) malloc (sizeof *oid)) == NULLOID)
	return NULLOID;

    if ((ip = (unsigned int *) malloc ((unsigned) (i + 1) * sizeof *ip))
	    == NULL) {
	free ((char *) oid);
	return NULLOID;
    }

    oid -> oid_elements = ip, oid -> oid_nelem = i;

    for (i = 0, jp = q -> oid_elements + o; i < oid -> oid_nelem; i++, jp++)
	*ip++ = *jp;

    return oid;
}
