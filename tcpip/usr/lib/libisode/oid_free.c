static char sccsid[] = "@(#)12	1.3  src/tcpip/usr/lib/libisode/oid_free.c, isodelib7, tcpip411, GOLD410 4/5/93 14:09:45";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: oid_free
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/oid_free.c
 */

/* oid_free.c - free an object identifier */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid_free.c,v 1.2 93/02/05 17:05:45 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid_free.c,v 1.2 93/02/05 17:05:45 snmp Exp $
 *
 *
 * $Log:	oid_free.c,v $
 * Revision 1.2  93/02/05  17:05:45  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:58  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:52  mrose
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

int	oid_free (oid)
register OID oid;
{
    if (oid == NULLOID)
	return(0);

    if (oid -> oid_elements)
	free ((char *) oid -> oid_elements);

    free ((char *) oid);

    return(0);
}
