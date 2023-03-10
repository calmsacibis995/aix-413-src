static char sccsid[] = "@(#)06	1.3  src/tcpip/usr/lib/libisode/ro2ssreleas1.c, isodelib7, tcpip411, GOLD410 4/5/93 15:25:08";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoEndRequest RoEndRequestAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ro2ssreleas1.c
 */

/* ro2ssreleas1.c - initiate release */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ro2ssreleas1.c,v 1.2 93/02/05 17:08:47 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ro2ssreleas1.c,v 1.2 93/02/05 17:08:47 snmp Exp $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log:	ro2ssreleas1.c,v $
 * Revision 1.2  93/02/05  17:08:47  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:21  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:17  mrose
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
#include <isode/ropkt.h>

/*    RO-END.REQUEST */

int	RoEndRequest (sd, priority, roi)
int	sd;
int	priority;
struct RoSAPindication *roi;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (roi);

    smask = sigioblock ();

    rosapPsig (acb, sd);

    result = RoEndRequestAux (acb, priority, roi);

    (void) sigiomask (smask);

    return result;

}

/*  */

static int  RoEndRequestAux (acb, priority, roi)
register struct assocblk   *acb;
int	priority;
struct RoSAPindication *roi;
{
    int     result;
    struct SSAPindication   sis;
    register struct SSAPindication *si = &sis;
    register struct SSAPabort *sa = &si -> si_abort;
    struct SSAPrelease  srs;
    register struct SSAPrelease *sr = &srs;

    if (acb -> acb_apdu)	/* ACB_CLOSING tested earlier in rosapPsig */
	return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);

    if (!(acb -> acb_flags & ACB_ROS))
	return rosaplose (roi, ROS_OPERATION, NULLCP,
		"not an association descriptor for ROS");

    if (!(acb -> acb_flags & ACB_INIT))
	return rosaplose (roi, ROS_OPERATION, NULLCP, "not initiator");

    if (acb -> acb_ready
	    && !(acb -> acb_flags & ACB_TURN)
	    && (*acb -> acb_ready) (acb, priority, roi) == NOTOK)
	return NOTOK;

    if (SRelRequest (acb -> acb_fd, NULLCP, 0, NOTOK, sr, si) == NOTOK) {
	if (sa -> sa_peer)
	    return ss2rosabort (acb, sa, roi);

	result = ss2roslose (acb, roi, "SRelRequest", sa);
    }
    else
	if (!sr -> sr_affirmative)
	    result = ropktlose (acb, roi, ROS_PROTOCOL, NULLCP,
		    "other side refused to release connection");
	else {
	    acb -> acb_fd = NOTOK;
	    result = OK;
	}

    freeacblk (acb);

    return result;
}
