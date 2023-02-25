static char sccsid[] = "@(#)85	1.3  src/tcpip/usr/lib/libisode/oid2aei.c, isodelib7, tcpip411, GOLD410 4/5/93 14:06:41";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: oid2aei
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/oid2aei.c
 */

/* oid2aei.c - application entity titles -- OID to AE info  */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid2aei.c,v 1.2 93/02/05 17:05:38 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid2aei.c,v 1.2 93/02/05 17:05:38 snmp Exp $
 *
 *
 * $Log:	oid2aei.c,v $
 * Revision 1.2  93/02/05  17:05:38  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:14:48  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:22:18  mrose
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
#include <isode/isoaddrs.h>

/*  */

AEI	oid2aei (oid)
OID	oid;
{
    static AEInfo aeinfo;
    AEI	    aei = &aeinfo;
    static PE pe = NULLPE;

    if (pe)
	pe_free (pe);

    bzero ((char *) aei, sizeof *aei);
    aei -> aei_ap_title = pe = obj2prim (oid, PE_CLASS_UNIV, PE_PRIM_OID);

    return aei;
}
