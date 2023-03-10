static char sccsid[] = "@(#)29	1.3  src/tcpip/usr/lib/libisode/rydiscard.c, isodelib7, tcpip411, GOLD410 4/5/93 16:21:24";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RyDiscard do_response missingP
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rydiscard.c
 */

/* rydiscard.c - ROSY: discard invocation in progress */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydiscard.c,v 1.2 93/02/05 17:10:38 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rydiscard.c,v 1.2 93/02/05 17:10:38 snmp Exp $
 *
 *
 * $Log:	rydiscard.c,v $
 * Revision 1.2  93/02/05  17:10:38  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:53  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:46  mrose
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

/*  */

int	do_response ();

/*    DISCARD */

int	RyDiscard (sd, id, roi)
int	sd,
	id;
struct RoSAPindication *roi;
{
    register struct opsblk *opb;

    missingP (roi);

    if ((opb = findopblk (sd, id, OPB_INITIATOR)) == NULLOPB)
	return rosaplose (roi, ROS_PARAMETER, NULLCP,
			  "invocation %d not in progress on association %d",
			  id, sd);

    opb -> opb_resfnx = opb -> opb_errfnx = do_response;

    return OK;
}

/*  */

/* ARGSUSED */

static int  do_response (sd, id, dummy, value, roi)
int	sd,
	id,
	dummy;
caddr_t value;
struct RoSAPindication *roi;
{
    return OK;
}
