static char sccsid[] = "@(#)72	1.3  src/tcpip/usr/lib/libisode/rtsapwait.c, isodelib7, tcpip411, GOLD410 4/5/93 16:20:49";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtWaitRequest RtWaitRequestAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsapwait.c
 */

/* rtsapwait.c - RTPM: wait for an indication */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapwait.c,v 1.2 93/02/05 17:10:34 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapwait.c,v 1.2 93/02/05 17:10:34 snmp Exp $
 *
 *
 * $Log:	rtsapwait.c,v $
 * Revision 1.2  93/02/05  17:10:34  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:45  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:43:34  mrose
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

/*    RT-WAIT.REQUEST (pseudo) */

int	RtWaitRequest (sd, secs, rti)
int	sd;
int	secs;
struct RtSAPindication *rti;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    result = RtWaitRequestAux (acb, secs, 0, rti);

    (void) sigiomask (smask);

    return result;
}

/*  */

int	RtWaitRequestAux (acb, secs, trans, rti)
register struct assocblk   *acb;
int     secs,
        trans;
register struct RtSAPindication *rti;
{
    if (!trans && (acb -> acb_flags & ACB_PLEASE)) {
	acb -> acb_flags &= ~ACB_PLEASE;

	rti -> rti_type = RTI_TURN;
	{
	    register struct RtSAPturn  *rtu = &rti -> rti_turn;

	    rtu -> rtu_please = 1;
	    rtu -> rtu_priority = acb -> acb_priority;
	}

	return DONE;
    }

    return (*acb -> acb_rtwaitrequest) (acb, secs, trans, rti);
}
