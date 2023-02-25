static char sccsid[] = "@(#)64	1.3  src/tcpip/usr/lib/libisode/rtsaperror.c, isodelib7, tcpip411, GOLD410 4/5/93 16:14:52";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RtErrString
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rtsaperror.c
 */

/* rtsaperror.c - return RtSAP error code in string form */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsaperror.c,v 1.2 93/02/05 17:10:18 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rtsaperror.c,v 1.2 93/02/05 17:10:18 snmp Exp $
 *
 *
 * $Log:	rtsaperror.c,v $
 * Revision 1.2  93/02/05  17:10:18  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:42:38  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/11  07:09:26  mrose
 * jpo
 * 
 * Revision 6.0  89/03/18  23:43:27  mrose
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
#include <isode/rtsap.h>

/*  */


static char *reject_err0[] = {
    "Busy",
    "Cannot recover",
    "Validation failure",
    "Unacceptable dialogue mode",
    "Rejected by responder",
    "Address unknown",
    "Connect request refused on this network connection", 
    "Session disconnect",
    "Protocol error",
    "Congestion at RtSAP",
    "Remote system problem",
    "Presentation disconnect",
    "ACS disconnect",
    "Peer aborted association",
    "Invalid parameter",
    "Invalid operation",
    "Timer expired",
    "Indications waiting",
    "Transfer failure"
};

static int reject_err0_cnt = sizeof reject_err0 / sizeof reject_err0[0];

/*  */

char   *RtErrString (code)
register int code;
{
    static char buffer[50];

    if (code < reject_err0_cnt)
	return reject_err0[code];

    (void) sprintf (buffer, "unknown error code 0x%x", code);
    return buffer;
}
