static char sccsid[] = "@(#)84	1.3  src/tcpip/usr/lib/libisode/ssapmajor1.c, isodelib7, tcpip411, GOLD410 4/5/93 16:50:57";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: SMajSyncRequest SMajSyncRequestAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ssapmajor1.c
 */

/* ssapmajor1.c - SPM: initiate majorsyncs */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapmajor1.c,v 1.2 93/02/05 17:12:09 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapmajor1.c,v 1.2 93/02/05 17:12:09 snmp Exp $
 *
 *
 * $Log:	ssapmajor1.c,v $
 * Revision 1.2  93/02/05  17:12:09  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:45:51  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:30  mrose
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

/*    S-MAJOR-SYNC.REQUEST */

int	SMajSyncRequest (sd, ssn, data, cc, si)
int	sd;
long   *ssn;
char   *data;
int	cc;
struct SSAPindication *si;
{
    SBV	    smask;
    int     result;
    register struct ssapblk *sb;

    missingP (ssn);
    missingP (si);

    smask = sigioblock ();

    ssapPsig (sb, sd);
    toomuchP (sb, data, cc, SN_SIZE, "majorsync");

    result = SMajSyncRequestAux (sb, ssn, data, cc, MAP_SYNC_NOEND, si);

    (void) sigiomask (smask);

    return result;
}

/*  */

int	SMajSyncRequestAux (sb, ssn, data, cc, opts, si)
register struct ssapblk *sb;
long   *ssn;
char   *data;
int	cc,
	opts;
register struct SSAPindication *si;
{
    int     result;

    if (SDoActivityAux (sb, si, 0, 0) == NOTOK)
	return NOTOK;

    if ((sb -> sb_requirements & SR_ACTIVITY)
	    && !(sb -> sb_flags & SB_Vact))
	return ssaplose (si, SC_OPERATION, NULLCP, "no activity in progress");

    if (sb -> sb_flags & SB_MAA)
	return ssaplose (si, SC_OPERATION, "awaiting your majorsync response");

    if ((result = SWriteRequestAux (sb, SPDU_MAP, data, cc, opts,
	    *ssn = sb -> sb_V_M, 0, NULLSD, NULLSD, NULLSR, si)) == NOTOK)
	freesblk (sb);
    else {
	sb -> sb_flags |= SB_MAP;
	if (opts & MAP_SYNC_NOEND) {
	    if (sb -> sb_requirements & SR_ACTIVITY)
		sb -> sb_flags |= SB_Vnextact;
	}
	else
	    sb -> sb_flags |= SB_AE, sb -> sb_flags &= ~SB_Vnextact;
	if (sb -> sb_flags & SB_Vsc) {
	    sb -> sb_V_A = sb -> sb_V_M;
	    sb -> sb_flags &= ~SB_Vsc;
	}
	sb -> sb_V_M++;
    }

    return result;
}
