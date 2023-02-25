static char sccsid[] = "@(#)22	1.3  src/tcpip/usr/lib/libisode/servbyport.c, isodelib7, tcpip411, GOLD410 4/5/93 16:38:30";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: getisoserventbyport
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/servbyport.c
 */

/* servbyport.c - getisoserventbyport */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/servbyport.c,v 1.2 93/02/05 17:11:23 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/servbyport.c,v 1.2 93/02/05 17:11:23 snmp Exp $
 *
 *
 * $Log:	servbyport.c,v $
 * Revision 1.2  93/02/05  17:11:23  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:49  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:27  mrose
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
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/isoservent.h>
#include <isode/tailor.h>

/*  */

struct isoservent *getisoserventbyport (provider, port)
char   *provider;
unsigned short port;
{
    register struct isoservent *is;

    isodetailor (NULLCP, 0);
    DLOG (addr_log, LLOG_TRACE,
	   ("getisoserventbyport \"%s\" %d", provider, (int) ntohs (port)));

    (void) setisoservent (0);
    while (is = getisoservent ())
	if (sizeof (port) == is -> is_selectlen
		&& port == is -> is_port
		&& strcmp (provider, is -> is_provider) == 0)
	    break;
    (void) endisoservent ();

    if (is) {
#ifdef	DEBUG
	if (addr_log -> ll_events & LLOG_DEBUG)
	    _printsrv (is);
#endif
    }
    else
	SLOG (addr_log, LLOG_EXCEPTIONS, NULLCP,
	      ("lookup of local service %s/%d failed",
	       provider, (int) ntohs (port)));

    return is;
}
