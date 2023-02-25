static char sccsid[] = "@(#)30	1.3  src/tcpip/usr/lib/libisode/is2paddr.c, isodelib7, tcpip411, GOLD410 4/5/93 13:51:21";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: is2paddr
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/is2paddr.c
 */

/* is2paddr.c - old-style P-ADDR lookup */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/is2paddr.c,v 1.2 93/02/05 17:04:43 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/is2paddr.c,v 1.2 93/02/05 17:04:43 snmp Exp $
 *
 *
 * $Log:	is2paddr.c,v $
 * Revision 1.2  93/02/05  17:04:43  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:14:38  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/03/05  23:02:18  mrose
 * touch-up
 * 
 * Revision 7.0  89/11/23  21:22:10  mrose
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
#include <isode/isoservent.h>

/*  */

struct PSAPaddr *is2paddr (host, service, is)
char   *host,
       *service;
struct isoservent *is;
{
    AEI	    aei;
    struct PSAPaddr *pa;

    if ((aei = str2aei (host, service)) == NULLAEI
	    || (pa = aei2addr (aei)) == NULLPA)
	return NULLPA;

    if (is && strcmp (is -> is_provider, "psap") == 0) {
	if (is -> is_selectlen > PSSIZE)	/* XXX */
	    return NULLPA;

	bcopy (is -> is_selector, pa -> pa_selector,
		pa -> pa_selectlen = is -> is_selectlen);
    }

    return pa;
}
