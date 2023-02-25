static char sccsid[] = "@(#)30	1.3  src/tcpip/usr/lib/libisode/rydispatch.c, isodelib7, tcpip411, GOLD410 4/5/93 16:21:58";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RyDispatch
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rydispatch.c
 */

/* rydispatch.c - ROSY: dispatch  */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydispatch.c,v 1.2 93/02/05 17:10:40 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydispatch.c,v 1.2 93/02/05 17:10:40 snmp Exp $
 *
 *
 * $Log:	rydispatch.c,v $
 * Revision 1.2  93/02/05  17:10:40  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:55  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:47  mrose
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

/*    DISPATCH */

int	RyDispatch (sd, ryo, op, fnx, roi)
int	sd;
register struct RyOperation *ryo;
int	op;
IFP	fnx;
struct RoSAPindication *roi;
{
    register struct dspblk *dsb;

    missingP (roi);

    if ((dsb = finddsblk (sd, op)) == NULLDSB) {
	missingP (ryo);
	missingP (fnx);

	for (; ryo -> ryo_name; ryo++)
	    if (ryo -> ryo_op == op)
		break;
	if (!ryo -> ryo_name)
	    return rosaplose (roi, ROS_PARAMETER, NULLCP,
		    "unknown operation code %d", op);

	if ((dsb = newdsblk (sd, ryo)) == NULLDSB)
	    return rosaplose (roi, ROS_CONGEST, NULLCP, NULLCP);
    }
    else
	if (ryo)
	    dsb -> dsb_ryo = ryo;

    if ((dsb -> dsb_vector = fnx) == NULLIFP)
	freedsblk (dsb);

    return OK;
}
