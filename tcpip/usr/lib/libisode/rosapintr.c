static char sccsid[] = "@(#)14	1.3  src/tcpip/usr/lib/libisode/rosapintr.c, isodelib7, tcpip411, GOLD410 4/5/93 15:30:31";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoIntrRequest intrser
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rosapintr.c
 */

/* rosapintr.c - ROPM: invoke (interruptable) */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapintr.c,v 1.2 93/02/05 17:09:21 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rosapintr.c,v 1.2 93/02/05 17:09:21 snmp Exp $
 *
 *
 * $Log:	rosapintr.c,v $
 * Revision 1.2  93/02/05  17:09:21  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:41:30  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:25  mrose
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
#include <isode/rosap.h>

/*  */

static int interrupted;
SFD	intrser ();

/*    RO-INVOKE.REQUEST (interruptable) */

int	RoIntrRequest (sd, op, args, invokeID, linkedID, priority, roi)
int	sd;
int	op,
    	invokeID,
       *linkedID,
	priority;
PE	args;
struct RoSAPindication *roi;
{
    int	    nfds,
	    result;
    fd_set  rfds;
    SFP    istat;

    if (RoInvokeRequest (sd, op, ROS_ASYNC, args, invokeID, linkedID, priority,
			 roi) == NOTOK)
	return NOTOK;

    interrupted = 0;
    istat = signal (SIGINT, intrser);

    for (;;) {
	nfds = 0;
	FD_ZERO (&rfds);

						/* interrupt causes EINTR */
	if (RoSelectMask (sd, &rfds, &nfds, roi) == OK)
	    (void) xselect (nfds, &rfds, NULLFD, NULLFD, NOTOK);

	if (interrupted) {
	    result = rosaplose (roi, ROS_INTERRUPTED, NULLCP, NULLCP);
	    break;
	}

	if ((result = RoWaitRequest (sd, OK, roi)) != NOTOK
	        || roi -> roi_preject.rop_reason != ROS_TIMER)
	    break;
    }

    (void) signal (SIGINT, istat);

    return result;
}

/*  */

/* ARGSUSED */

static  SFD intrser (sig)
int	sig;
{
#ifndef	BSDSIGS
    (void) signal (SIGINT, intrser);
#endif

    interrupted++;

#ifdef SFDINT
    return(0);
#endif

}
