static char sccsid[] = "@(#)69	1.3  src/tcpip/usr/lib/libisode/str2oid.c, isodelib7, tcpip411, GOLD410 4/5/93 16:59:39";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2oid
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2oid.c
 */

/* str2oid.c - string to object identifier */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2oid.c,v 1.2 93/02/05 17:12:54 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2oid.c,v 1.2 93/02/05 17:12:54 snmp Exp $
 *
 *
 * $Log:	str2oid.c,v $
 * Revision 1.2  93/02/05  17:12:54  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:04  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:46  mrose
 * Release 6.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include <isode/psap.h>

/*  */

OID	str2oid (s)
char    *s;
{
    int	    i;
    static struct OIDentifier   oids;
    static unsigned int elements[NELEM + 1];

    if ((i = str2elem (s, elements)) < 1)
	return NULLOID;

    oids.oid_elements = elements, oids.oid_nelem = i;

    return (&oids);
}
