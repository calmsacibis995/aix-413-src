static char sccsid[] = "@(#)67	1.3  src/tcpip/usr/lib/libisode/rtsappturn.c, isodelib7, tcpip411, GOLD410 4/5/93 16:17:32";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtPTurnRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsappturn.c
 */

/* rtsappturn.c - RTPM: turn please */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsappturn.c,v 1.2 93/02/05 17:10:24 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsappturn.c,v 1.2 93/02/05 17:10:24 snmp Exp $
 *
 *
 * $Log:	rtsappturn.c,v $
 * Revision 1.2  93/02/05  17:10:24  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:41  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:43:30  mrose
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

/*    RT-TURN-PLEASE.REQUEST */

int	RtPTurnRequest (sd, priority, rti)
int	sd;
int	priority;
struct RtSAPindication *rti;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    result = (*acb -> acb_pturnrequest) (acb, priority, rti);

    (void) sigiomask (smask);

    return result;
}
