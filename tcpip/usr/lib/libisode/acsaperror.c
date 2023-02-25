static char sccsid[] = "@(#)75	1.3  src/tcpip/usr/lib/libisode/acsaperror.c, isodelib7, tcpip411, GOLD410 4/5/93 13:39:05";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AcErrString
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/acsaperror.c
 */

/* acsaperror.c - return AcSAP error code in string form */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/acsaperror.c,v 1.2 93/02/05 17:03:02 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/acsaperror.c,v 1.2 93/02/05 17:03:02 snmp Exp $
 *
 *
 * $Log:	acsaperror.c,v $
 * Revision 1.2  93/02/05  17:03:02  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:14:07  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/11  07:09:04  mrose
 * jpo
 * 
 * Revision 7.0  89/11/23  21:21:48  mrose
 * Release 6.0
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
#include <isode/acsap.h>

/*  */

static char *reject_err0[] = {
    "Accepted",
    "Permanent",
    "Transient",
    "Rejected by service-user: null",
    "Rejected by service-user: no reason given",
    "Application context name not supported",
    "Calling AP title not recognized",
    "Calling AP invocation-ID not recognized",
    "Calling AE qualifier not recognized",
    "Calling AE invocation-ID not recognized",
    "Called AP title not recognized",
    "Called AP invocation-ID not recognized",
    "Called AE qualifier not recognized",
    "Called AE invocation-ID not recognized",
    "Rejected by service-provider: null",
    "Rejected by service-provider: no reason given",
    "No common acse version",
    "Address unknown",
    "Connect request refused on this network connection",
    "Local limit exceeded",
    "Presentation disconnect",
    "Protocol error",
    "Peer aborted association",
    "Invalid parameter",
    "Invalid operation",
    "Timer expired"
};

static int reject_err0_cnt = sizeof reject_err0 / sizeof reject_err0[0];

/*  */

char   *AcErrString (code)
register int code;
{
    static char buffer[50];

    if (code < reject_err0_cnt)
	return reject_err0[code];

    (void) sprintf (buffer, "unknown error code %d", code);
    return buffer;
}
