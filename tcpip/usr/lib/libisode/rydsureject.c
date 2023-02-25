static char sccsid[] = "@(#)34	1.3  src/tcpip/usr/lib/libisode/rydsureject.c, isodelib7, tcpip411, GOLD410 4/5/93 16:24:57";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RyDsUReject
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rydsureject.c
 */

/* rydsureject.c - ROSY: reject invocation */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydsureject.c,v 1.2 93/02/05 17:10:50 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydsureject.c,v 1.2 93/02/05 17:10:50 snmp Exp $
 *
 *
 * $Log:	rydsureject.c,v $
 * Revision 1.2  93/02/05  17:10:50  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:59  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:51  mrose
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
#include <isode/rosy.h>


#define	missingP(p) \
{ \
    if (p == NULL) \
	return rosaplose (roi, ROS_PARAMETER, NULLCP, \
			    "mandatory parameter \"%s\" missing", "p"); \
}

/*    U-REJECT */

int	RyDsUReject (sd, id, reason, priority, roi)
int	sd;
int	id,
	reason,
	priority;
struct RoSAPindication *roi;
{
    int     result;
    register struct opsblk *opb;

    missingP (roi);

    if ((opb = findopblk (sd, id, OPB_RESPONDER)) == NULLOPB)
	return rosaplose (roi, ROS_PARAMETER, NULLCP,
		"invocation %d not in progress on association %d",
		id, sd);

    if ((result = RoURejectRequest (sd, &id, reason, priority, roi)) != NOTOK)
	freeopblk (opb);

    return result;
}
