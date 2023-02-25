static char sccsid[] = "@(#)29	1.3  src/tcpip/usr/lib/libisode/str2saddr.c, isodelib7, tcpip411, GOLD410 4/5/93 17:02:25";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2saddr
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2saddr.c
 */

/* str2saddr.c - string value to SSAPaddr */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2saddr.c,v 1.2 93/02/05 17:13:05 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2saddr.c,v 1.2 93/02/05 17:13:05 snmp Exp $
 *
 *
 * $Log:	str2saddr.c,v $
 * Revision 1.2  93/02/05  17:13:05  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:56  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:36  mrose
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

struct SSAPaddr *str2saddr (str)
char   *str;
{
    register struct PSAPaddr *pa;

    if (pa = str2paddr (str))
	return (&pa -> pa_addr);

    return NULLSA;
}
