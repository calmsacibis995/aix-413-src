static char sccsid[] = "@(#)32	1.3  src/tcpip/usr/lib/libisode/is2taddr.c, isodelib7, tcpip411, GOLD410 4/5/93 13:52:07";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: is2taddr
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/is2taddr.c
 */

/* is2taddr.c - old-style T-ADDR lookup */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/is2taddr.c,v 1.2 93/02/05 17:04:46 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/is2taddr.c,v 1.2 93/02/05 17:04:46 snmp Exp $
 *
 *
 * $Log:	is2taddr.c,v $
 * Revision 1.2  93/02/05  17:04:46  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:14:40  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/03/05  23:02:24  mrose
 * touch-up
 * 
 * Revision 7.0  89/11/23  21:22:11  mrose
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

struct TSAPaddr *is2taddr (host, service, is)
char   *host,
       *service;
struct isoservent *is;
{
    AEI	    aei;
    struct PSAPaddr *pa;
    struct TSAPaddr *ta;

    if ((aei = str2aei (host, service)) == NULLAEI
	    || (pa = aei2addr (aei)) == NULLPA)
	return NULLTA;

    ta = &pa -> pa_addr.sa_addr;
    if (is && strcmp (is -> is_provider, "tsap") == 0) {
	if (is -> is_selectlen > TSSIZE)	/* XXX */
	    return NULLTA;

	bcopy (is -> is_selector, ta -> ta_selector,
		ta -> ta_selectlen = is -> is_selectlen);
    }
	    
    return ta;
}
