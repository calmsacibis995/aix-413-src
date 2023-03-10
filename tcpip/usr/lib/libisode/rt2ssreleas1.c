static char sccsid[] = "@(#)58	1.3  src/tcpip/usr/lib/libisode/rt2ssreleas1.c, isodelib7, tcpip411, GOLD410 4/5/93 16:09:52";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtEndRequest RtEndRequestAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rt2ssreleas1.c
 */

/* rt2ssreleas1.c - RTPM: initiate release */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rt2ssreleas1.c,v 1.2 93/02/05 17:10:08 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rt2ssreleas1.c,v 1.2 93/02/05 17:10:08 snmp Exp $
 *
 *
 * $Log:	rt2ssreleas1.c,v $
 * Revision 1.2  93/02/05  17:10:08  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:31  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:43:21  mrose
 * Release 5.0
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
#include <isode/rtpkt.h>

/*    RT-END.REQUEST (X.410 CLOSE.REQUEST) */

int	RtEndRequest (sd, rti)
int	sd;
struct RtSAPindication *rti;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    result = RtEndRequestAux (acb, rti);

    (void) sigiomask (smask);

    return result;
}

/*  */

static int  RtEndRequestAux (acb, rti)
register struct assocblk   *acb;
struct RtSAPindication *rti;
{
    int     result;
    struct SSAPindication   sis;
    register struct SSAPindication *si = &sis;
    register struct SSAPabort  *sa = &si -> si_abort;
    struct SSAPrelease  srs;
    register struct SSAPrelease *sr = &srs;

    if (acb -> acb_flags & ACB_ACS)
	return rtsaplose (rti, RTS_OPERATION, NULLCP,
		    "not an association descriptor for RTS");
    if (!(acb -> acb_flags & ACB_INIT) && (acb -> acb_flags & ACB_TWA))
	return rtsaplose (rti, RTS_OPERATION, NULLCP, "not initiator");
    if (!(acb -> acb_flags & ACB_TURN))
	return rtsaplose (rti, RTS_OPERATION, NULLCP, "turn not owned by you");
    if (acb -> acb_flags & ACB_ACT)
	return rtsaplose (rti, RTS_OPERATION, NULLCP, "transfer in progress");
    if (acb -> acb_flags & ACB_PLEASE)
	return rtsaplose (rti, RTS_WAITING, NULLCP, NULLCP);

    if (SRelRequest (acb -> acb_fd, NULLCP, 0, NOTOK, sr, si) == NOTOK) {
	if (sa -> sa_peer)
	    return ss2rtsabort (acb, sa, rti);

	result = ss2rtslose (acb, rti, "SRelRequest", sa);
    }
    else
	if (!sr -> sr_affirmative)
	    result = rtpktlose (acb, rti, RTS_PROTOCOL, NULLCP,
		    "other side refused to release connection");
	else {
	    acb -> acb_fd = NOTOK;
	    result = OK;
	}

    acb -> acb_flags &= ~ACB_STICKY;
    freeacblk (acb);

    return result;
}
