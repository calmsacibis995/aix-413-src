static char sccsid[] = "@(#)07	1.3  src/tcpip/usr/lib/libisode/ro2ssreleas2.c, isodelib7, tcpip411, GOLD410 4/5/93 15:25:27";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoEndResponse RoEndResponseAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ro2ssreleas2.c
 */

/* ro2ssreleas2.c - respond to release */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ro2ssreleas2.c,v 1.2 93/02/05 17:08:49 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ro2ssreleas2.c,v 1.2 93/02/05 17:08:49 snmp Exp $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log:	ro2ssreleas2.c,v $
 * Revision 1.2  93/02/05  17:08:49  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:22  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:18  mrose
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

/*    RO-END.RESPONSE */

int	RoEndResponse (sd, roi)
int	sd;
struct RoSAPindication *roi;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (roi);

    smask = sigioblock ();

    rosapFsig (acb, sd);

    result = RoEndResponseAux (acb, roi);

    (void) sigiomask (smask);

    return result;

}

/*  */

static int  RoEndResponseAux (acb, roi)
register struct assocblk   *acb;
struct RoSAPindication *roi;
{
    int     result;
    struct SSAPindication   sis;
    register struct SSAPindication *si = &sis;
    register struct SSAPabort *sa = &si -> si_abort;

    if (SRelResponse (acb -> acb_fd, SC_ACCEPT, NULLCP, 0, si) == NOTOK)
	result = ss2roslose (acb, roi, "SRelResponse", sa);
    else {
	acb -> acb_fd = NOTOK;
	result = OK;
    }

    freeacblk (acb);

    return result;
}
