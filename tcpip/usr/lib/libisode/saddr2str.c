static char sccsid[] = "@(#)17	1.3  src/tcpip/usr/lib/libisode/saddr2str.c, isodelib7, tcpip411, GOLD410 4/5/93 16:32:57";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: saddr2str
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/saddr2str.c
 */

/* saddr2str.c - SSAPaddr to string value */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/saddr2str.c,v 1.2 93/02/05 17:11:07 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/saddr2str.c,v 1.2 93/02/05 17:11:07 snmp Exp $
 *
 *
 * $Log:	saddr2str.c,v $
 * Revision 1.2  93/02/05  17:11:07  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:43  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:22  mrose
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

char   *saddr2str (sa)
register struct SSAPaddr *sa;
{
    struct PSAPaddr pas;
    register struct PSAPaddr *pa = &pas;

    bzero ((char *) pa, sizeof *pa);
    pa -> pa_addr = *sa;		/* struct copy */

    return paddr2str (pa, NULLNA);
}
