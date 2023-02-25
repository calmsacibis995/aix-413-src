static char sccsid[] = "@(#)63	1.3  src/tcpip/usr/lib/libisode/rtsapdtrans.c, isodelib7, tcpip411, GOLD410 4/5/93 16:14:06";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtSetDownTrans
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsapdtrans.c
 */

/* rtsapdtrans.c - RTPM: set downtrans upcall */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapdtrans.c,v 1.2 93/02/05 17:10:17 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsapdtrans.c,v 1.2 93/02/05 17:10:17 snmp Exp $
 *
 *
 * $Log:	rtsapdtrans.c,v $
 * Revision 1.2  93/02/05  17:10:17  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:37  mrose
 * Interim 6.8
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

/*    set downtrans upcall */

int	RtSetDownTrans (sd, fnx, rti)
int	sd;
IFP	fnx;
struct RtSAPindication *rti;
{
    SBV	    smask;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    acb -> acb_downtrans = fnx;

    (void) sigiomask (smask);

    return OK;
}
