static char sccsid[] = "@(#)97	1.3  src/tcpip/usr/lib/libisode/ssaptyped.c, isodelib7, tcpip411, GOLD410 4/5/93 16:57:16";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: STypedRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ssaptyped.c
 */

/* ssaptyped.c - SPM: write typed data */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssaptyped.c,v 1.2 93/02/05 17:12:42 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssaptyped.c,v 1.2 93/02/05 17:12:42 snmp Exp $
 *
 *
 * $Log:	ssaptyped.c,v $
 * Revision 1.2  93/02/05  17:12:42  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:46:09  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:51  mrose
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
#include <signal.h>
#include <isode/spkt.h>

/*    S-TYPED-DATA.REQUEST */

int	STypedRequest (sd, data, cc, si)
int	sd;
char   *data;
int	cc;
struct SSAPindication *si;
{
    SBV	    smask;
    int     result;
    struct udvec uvs[2];
    register struct udvec *uv = uvs;
    register struct ssapblk *sb;

    missingP (data);
    if (cc <= 0)
	return ssaplose (si, SC_PARAMETER, NULLCP,
		    "illegal value for TSSDU length (%d)", cc);
    missingP (si);

    smask = sigioblock ();

    ssapPsig (sb, sd);

    uv -> uv_base = data, uv -> uv_len = cc, uv++;
    uv -> uv_base = NULL;

    result = SDataRequestAux (sb, SPDU_TD, uvs, 1, 1, si);

    (void) sigiomask (smask);

    return result;
}
