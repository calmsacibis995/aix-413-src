static char sccsid[] = "@(#)31	1.3  src/tcpip/usr/lib/libisode/str2taddr.c, isodelib7, tcpip411, GOLD410 4/5/93 17:04:50";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2taddr
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2taddr.c
 */

/* str2taddr.c - string value to TSAPaddr */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2taddr.c,v 1.2 93/02/05 17:13:12 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2taddr.c,v 1.2 93/02/05 17:13:12 snmp Exp $
 *
 *
 * $Log:	str2taddr.c,v $
 * Revision 1.2  93/02/05  17:13:12  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:16:00  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:38  mrose
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
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/isoaddrs.h>

/*  */

struct TSAPaddr *str2taddr (str)
char   *str;
{
    register struct PSAPaddr *pa;

    if (pa = str2paddr (str))
	return (&pa -> pa_addr.sa_addr);

    return NULLTA;
}
