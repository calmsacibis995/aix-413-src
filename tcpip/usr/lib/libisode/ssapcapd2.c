static char sccsid[] = "@(#)78	1.3  src/tcpip/usr/lib/libisode/ssapcapd2.c, isodelib7, tcpip411, GOLD410 4/5/93 16:47:33";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: SCapdResponse SCapdResponseAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ssapcapd2.c
 */

/* ssapcapd2.c - SPM: read capability data */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapcapd2.c,v 1.2 93/02/05 17:11:56 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapcapd2.c,v 1.2 93/02/05 17:11:56 snmp Exp $
 *
 *
 * $Log:	ssapcapd2.c,v $
 * Revision 1.2  93/02/05  17:11:56  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:45:44  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:24  mrose
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

/*    S-CAPABILITY-DATA.RESPONSE */

int	SCapdResponse (sd, data, cc, si)
int	sd;
char   *data;
int	cc;
struct SSAPindication *si;
{
    SBV	    smask;
    int     result;
    register struct ssapblk *sb;

    missingP (si);

    smask = sigioblock ();

    ssapPsig (sb, sd);
    toomuchP (sb, data, cc, SX_CDASIZE, "capability");

    result = SCapdResponseAux (sb, data, cc, si);

    (void) sigiomask (smask);

    return result;
}

/*  */

static  int  SCapdResponseAux (sb, data, cc, si)
register struct ssapblk *sb;
char   *data;
int	cc;
struct SSAPindication *si;
{
    int     result;

    if (!(sb -> sb_requirements & SR_CAPABILITY))
	return ssaplose (si, SC_OPERATION, NULLCP,
		"capability data exchange service unavailable");
    if (!(sb -> sb_flags & SB_CDA))
	return ssaplose (si, SC_OPERATION, NULLCP,
		"no capability data response in progress");

    if ((result = SWriteRequestAux (sb, SPDU_CDA, data, cc, 0, 0L, 0, NULLSD,
		NULLSD, NULLSR, si)) == NOTOK)
	freesblk (sb);
    else
	sb -> sb_flags &= ~SB_CDA;

    return result;
}
