static char sccsid[] = "@(#)12	1.3  src/tcpip/usr/lib/libisode/rosapasync.c, isodelib7, tcpip411, GOLD410 4/5/93 15:29:36";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoSetIndications
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rosapasync.c
 */

/* rosapasync.c - ROPM: set asynchronous events */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapasync.c,v 1.2 93/02/05 17:09:17 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapasync.c,v 1.2 93/02/05 17:09:17 snmp Exp $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log:	rosapasync.c,v $
 * Revision 1.2  93/02/05  17:09:17  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:41:28  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/08/08  14:13:55  mrose
 * update
 * 
 * Revision 6.0  89/03/18  23:42:23  mrose
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

/*    define vectors for INDICATION events */

int	RoSetIndications (sd, indication, roi)
int	sd;
IFP	indication;
struct RoSAPindication *roi;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    _iosignals_set = 1;

    smask = sigioblock ();

    rosapPsig (acb, sd);

    if (acb -> acb_apdu || (acb -> acb_flags & ACB_CLOSING)) {
	(void) sigiomask (smask);
	return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);
    }

    result = (*acb -> acb_rosetindications) (acb, indication, roi);

    (void) sigiomask (smask);

    return result;
}
