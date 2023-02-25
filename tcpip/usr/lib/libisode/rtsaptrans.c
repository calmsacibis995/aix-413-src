static char sccsid[] = "@(#)69	1.3  src/tcpip/usr/lib/libisode/rtsaptrans.c, isodelib7, tcpip411, GOLD410 4/5/93 16:19:03";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtTransferRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsaptrans.c
 */

/* rtsaptrans.c - RTPM: transfer */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsaptrans.c,v 1.2 93/02/05 17:10:29 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsaptrans.c,v 1.2 93/02/05 17:10:29 snmp Exp $
 *
 *
 * $Log:	rtsaptrans.c,v $
 * Revision 1.2  93/02/05  17:10:29  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:43  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:43:32  mrose
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

/*    RT-TRANSFER.REQUEST */

int	RtTransferRequest (sd, data, secs, rti)
int	sd;
PE	data;
int	secs;
struct RtSAPindication *rti;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    if (data == NULLPE && acb -> acb_downtrans == NULLIFP) {
	(void) sigiomask (smask);
	return rtsaplose (rti, RTS_PARAMETER, NULLCP,
			  "mandatory parameter \"data\" missing");
    }

    result = (*acb -> acb_transferequest)  (acb, data, secs, rti);

    (void) sigiomask (smask);

    return result;
}
