static char sccsid[] = "@(#)09	1.3  src/tcpip/usr/lib/libisode/ro2ssthorn.c, isodelib7, tcpip411, GOLD410 4/5/93 15:26:01";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoSetThorn qb2Rpe
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ro2ssthorn.c
 */

/* ro2ssthorn.c - ROPM: interface for THORN */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ro2ssthorn.c,v 1.2 93/02/05 17:08:53 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ro2ssthorn.c,v 1.2 93/02/05 17:08:53 snmp Exp $
 *
 *
 * $Log:	ro2ssthorn.c,v $
 * Revision 1.2  93/02/05  17:08:53  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:24  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:20  mrose
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

/*  */

static PE	qb2Rpe (qb, len, result)
register struct qbuf *qb;
int	len;
int    *result;
{
    return qb2pe (qb, len, 2, result);
}

/*    modify underling service */

int	RoSetThorn (sd, roi)
int	sd;
struct RoSAPindication *roi;
{
    SBV	    smask;
    int	    result;
    register struct assocblk   *acb;

    missingP (roi);

    smask = sigioblock ();

    if ((acb = findacblk (sd)) == NULL) {
	(void) sigiomask (smask);
	return rosaplose (roi, ROS_PARAMETER, NULLCP,
			  "invalid association descriptor");
    }

    if (acb -> acb_flags & ACB_ROS) {
	acb -> acb_getosdu = qb2Rpe;
	result = OK;
    }
    else
	result = rosaplose (roi, ROS_OPERATION, NULLCP,
			    "not an association descriptor for ROS");


    (void) sigiomask (smask);

    return result;
}
