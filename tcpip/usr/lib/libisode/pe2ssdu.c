static char sccsid[] = "@(#)16	1.3  src/tcpip/usr/lib/libisode/pe2ssdu.c, isodelib7, tcpip411, GOLD410 4/5/93 14:12:15";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: pe2ssdu
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pe2ssdu.c
 */

/* pe2ssdu.c - write a PE to a SSDU */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe2ssdu.c,v 1.2 93/02/05 17:05:57 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe2ssdu.c,v 1.2 93/02/05 17:05:57 snmp Exp $
 *
 *
 * $Log:	pe2ssdu.c,v $
 * Revision 1.2  93/02/05  17:05:57  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:36:04  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/24  14:50:19  mrose
 * update
 * 
 * Revision 7.0  89/11/23  22:12:57  mrose
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
#include <isode/tailor.h>

/*  */

int	pe2ssdu (pe, base, len)
register PE pe;
char  **base;
int    *len;
{
    register int plen, ret;

    *len = 0;
    plen  = ps_get_abs (pe);
    Qcp = (char *)malloc(plen);
    *base = Qcp;

    if (Qcp == NULL)
        return NOTOK;

    Len = 0;
    Ecp = Qcp + plen;
    if ((ret = pe2qb_f(pe)) != plen) {
        printf("pe2ssdu: bad length returned %d should be %d\n",
                ret, plen);
	return NOTOK;
    }
    *len = plen;

#ifdef	DEBUG
    if (psap_log -> ll_events & LLOG_PDUS)
	pe2text (psap_log, pe, 0, *len);
#endif

    return OK;
}
