static char sccsid[] = "@(#)11	1.3  src/tcpip/usr/lib/libisode/na2str.c, isodelib7, tcpip411, GOLD410 4/5/93 14:00:59";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: na2str
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/na2str.c
 */

/* na2str.c - pretty-print NSAPaddr */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/na2str.c,v 1.2 93/02/05 17:05:19 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/na2str.c,v 1.2 93/02/05 17:05:19 snmp Exp $
 *
 *
 * $Log:	na2str.c,v $
 * Revision 1.2  93/02/05  17:05:19  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:15:37  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/07/09  14:32:05  mrose
 * sync
 * 
 * Revision 7.0  89/11/23  21:23:19  mrose
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

/*    Network Address to String */

char   *na2str (na)
register struct NSAPaddr *na;
{
    switch (na -> na_stack) {
	case NA_TCP: 
	    return na -> na_domain;

	case NA_X25:
	case NA_BRG: 
	    return na -> na_dte;

	case NA_NSAP:
	default:
	    return sel2str (na -> na_address, na -> na_addrlen, 0);
    }
}
