static char sccsid[] = "@(#)18	1.3  src/tcpip/usr/lib/libisode/rosapselect.c, isodelib7, tcpip411, GOLD410 4/5/93 15:32:55";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoSelectMask
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rosapselect.c
 */

/* rosapselect.c - ROPM: map descriptors */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapselect.c,v 1.2 93/02/05 17:09:29 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapselect.c,v 1.2 93/02/05 17:09:29 snmp Exp $
 *
 * Based on an TCP-based implementation by George Michaelson of University
 * College London.
 *
 *
 * $Log:	rosapselect.c,v $
 * Revision 1.2  93/02/05  17:09:29  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:35  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:29  mrose
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

/*    map association descriptors for select() */

/* ARGSUSED */

int	RoSelectMask (sd, mask, nfds, roi)
int	sd;
fd_set *mask;
int    *nfds;
struct RoSAPindication *roi;
{
    SBV	    smask;
    int     result;
    register struct assocblk   *acb;

    missingP (mask);
    missingP (nfds);
    missingP (roi);

    smask = sigioblock ();

    rosapPsig (acb, sd);

    if (acb -> acb_apdu || (acb -> acb_flags & ACB_CLOSING)) {
	(void) sigiomask (smask);
	return rosaplose (roi, ROS_WAITING, NULLCP, NULLCP);
    }

    result = (*acb -> acb_roselectmask) (acb, mask, nfds, roi);

    (void) sigiomask (smask);

    return result;
}
