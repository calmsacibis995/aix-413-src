static char sccsid[] = "@(#)62	1.3  src/tcpip/usr/lib/libisode/rtsapasync.c, isodelib7, tcpip411, GOLD410 4/5/93 16:13:13";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtSetIndications
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsapasync.c
 */

/* rtsapasync.c - RTPM: set asynchronous events */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapasync.c,v 1.2 93/02/05 17:10:15 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapasync.c,v 1.2 93/02/05 17:10:15 snmp Exp $
 *
 *
 * $Log:	rtsapasync.c,v $
 * Revision 1.2  93/02/05  17:10:15  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:42:36  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/08/08  14:13:59  mrose
 * update
 * 
 * Revision 6.0  89/03/18  23:43:26  mrose
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

/*    define vectors for INDICATION events */

int	RtSetIndications (sd, indication, rti)
int	sd;
IFP	indication;
struct RtSAPindication *rti;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    _iosignals_set = 1;

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    if (acb -> acb_flags & ACB_PLEASE) {
	(void) sigiomask (smask);

	return rtsaplose (rti, RTS_WAITING, NULLCP, NULLCP);
    }

    result = (*acb -> acb_rtsetindications) (acb, indication, rti);

    (void) sigiomask (smask);

    return result;
}
