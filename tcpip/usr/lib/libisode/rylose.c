static char sccsid[] = "@(#)37	1.3  src/tcpip/usr/lib/libisode/rylose.c, isodelib7, tcpip411, GOLD410 4/5/93 16:27:21";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RyLose
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rylose.c
 */

/* rylose.c - ROSY: clean-up after association termination */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rylose.c,v 1.2 93/02/05 17:10:56 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rylose.c,v 1.2 93/02/05 17:10:56 snmp Exp $
 *
 *
 * $Log:	rylose.c,v $
 * Revision 1.2  93/02/05  17:10:56  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:02  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:54  mrose
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

/*    clean-up after association termination */

int	RyLose (sd, roi)
int	sd;
struct RoSAPindication *roi;
{
    missingP (roi);

    loseopblk (sd, ROS_DONE);
    losedsblk (sd);

    return OK;
}
