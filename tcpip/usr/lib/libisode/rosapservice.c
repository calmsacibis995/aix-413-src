static char sccsid[] = "@(#)19	1.3  src/tcpip/usr/lib/libisode/rosapservice.c, isodelib7, tcpip411, GOLD410 4/5/93 15:34:00";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoSetService
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rosapservice.c
 */

/* rosapservice.c - ROPM: hack loader */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapservice.c,v 1.2 93/02/05 17:09:31 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapservice.c,v 1.2 93/02/05 17:09:31 snmp Exp $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log:	rosapservice.c,v $
 * Revision 1.2  93/02/05  17:09:31  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:36  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:30  mrose
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

/*    bind underlying service */

int	RoSetService (sd, bfunc, roi)
int	sd;
IFP	bfunc;
struct RoSAPindication *roi;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (bfunc);
    missingP (roi);

    smask = sigioblock ();

    if ((acb = findacblk (sd)) == NULL) {
	(void) sigiomask (smask);
	return rosaplose (roi, ROS_PARAMETER, NULLCP,
		"invalid association descriptor");
    }

    result = (*bfunc) (acb, roi);

    (void) sigiomask (smask);

    return result;
}
