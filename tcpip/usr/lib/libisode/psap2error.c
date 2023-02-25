static char sccsid[] = "@(#)71	1.3  src/tcpip/usr/lib/libisode/psap2error.c, isodelib7, tcpip411, GOLD410 4/5/93 15:09:03";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PErrString
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/psap2error.c
 */

/* psap2error.c - return PSAP error code in string form */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psap2error.c,v 1.2 93/02/05 17:07:23 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psap2error.c,v 1.2 93/02/05 17:07:23 snmp Exp $
 *
 *
 * $Log:	psap2error.c,v $
 * Revision 1.2  93/02/05  17:07:23  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:37:27  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/11  07:09:14  mrose
 * jpo
 * 
 * Revision 7.0  89/11/23  22:14:16  mrose
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
#include <isode/psap2.h>

/*  */

static char *reject_err0[] = {
    "Rejected by peer",
    "Reason not specified",
    "Temporary congestion",
    "Local limit exceeded",
    "Called presentation address unknown",
    "Protocol version not supported",
    "Default context not supported",
    "User-data not readable",
    "No PSAP available",
    "Unrecognized PPDU",
    "Unexpected PPDU",
    "Unexpected session service primitive",
    "Unrecognized PPDU parameter",
    "Unexpected PPDU parameter",
    "Invalid PPDU parameter value",
    "Abstract syntax not supported",
    "Proposed transfer syntaxes not supported",
    "Local limit on DCS exceeded",
    "Connect request refused on this network connection",
    "Session disconnect",
    "Protocol error",
    "Peer aborted connection",
    "Invalid parameter",
    "Invalid operation",
    "Timer expired",
    "Indications waiting"
};

static int reject_err0_cnt = sizeof reject_err0 / sizeof reject_err0[0];

/*  */

char   *PErrString (code)
register int code;
{
    static char buffer[50];

    if (code < reject_err0_cnt)
	return reject_err0[code];

    (void) sprintf (buffer, "unknown error code %d", code);
    return buffer;
}
