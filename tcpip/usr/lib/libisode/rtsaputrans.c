static char sccsid[] = "@(#)70	1.3  src/tcpip/usr/lib/libisode/rtsaputrans.c, isodelib7, tcpip411, GOLD410 4/5/93 16:20:05";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtSetUpTrans
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsaputrans.c
 */

/* rtsaputrans.c - RTPM: set uptrans upcall */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsaputrans.c,v 1.2 93/02/05 17:10:31 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsaputrans.c,v 1.2 93/02/05 17:10:31 snmp Exp $
 *
 *
 * $Log:	rtsaputrans.c,v $
 * Revision 1.2  93/02/05  17:10:31  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:44  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:43:33  mrose
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

/*    set uptrans upcall */

int	RtSetUpTrans (sd, fnx, rti)
int	sd;
IFP	fnx;
struct RtSAPindication *rti;
{
    SBV	    smask;
    register struct assocblk   *acb;

    missingP (rti);

    smask = sigioblock ();

    rtsapPsig (acb, sd);

    acb -> acb_uptrans = fnx;

    (void) sigiomask (smask);

    return OK;
}
