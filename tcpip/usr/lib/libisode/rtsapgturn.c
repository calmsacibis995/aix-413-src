static char sccsid[] = "@(#)65	1.3  src/tcpip/usr/lib/libisode/rtsapgturn.c, isodelib7, tcpip411, GOLD410 4/5/93 16:15:54";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtGTurnRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsapgturn.c
 */

/* rtsapgturn.c - RTPM: give turn */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapgturn.c,v 1.2 93/02/05 17:10:20 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapgturn.c,v 1.2 93/02/05 17:10:20 snmp Exp $
 *
 *
 * $Log:	rtsapgturn.c,v $
 * Revision 1.2  93/02/05  17:10:20  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:39  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:43:28  mrose
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

/*    RT-TURN-GIVE.REQUEST */

int	RtGTurnRequest (sd, rti)
int	sd;
struct RtSAPindication *rti;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    result = (*acb -> acb_gturnrequest) (acb, rti);

    (void) sigiomask (smask);

    return result;
}
