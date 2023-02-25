static char sccsid[] = "@(#)23	1.3  src/tcpip/usr/lib/libisode/servbysel.c, isodelib7, tcpip411, GOLD410 4/5/93 16:38:59";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: getisoserventbyselector
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/servbysel.c
 */

/* servbysel.c - getisoserventbyselector */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/servbysel.c,v 1.2 93/02/05 17:11:25 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/servbysel.c,v 1.2 93/02/05 17:11:25 snmp Exp $
 *
 *
 * $Log:	servbysel.c,v $
 * Revision 1.2  93/02/05  17:11:25  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:50  mrose
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

struct isoservent *getisoserventbyselector (provider, selector, selectlen)
char   *provider,
       *selector;
int	selectlen;
{
    register struct isoservent *is;

    isodetailor (NULLCP, 0);
    DLOG (addr_log, LLOG_TRACE,
	   ("getisoserventbyselector \"%s\" %s",
	    provider, sel2str (selector, selectlen, 1)));

    (void) setisoservent (0);
    while (is = getisoservent ())
	if (selectlen == is -> is_selectlen
		&& bcmp (selector, is -> is_selector, is -> is_selectlen) == 0
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
	      ("lookup of local service %s %s failed",
	       provider, sel2str (selector, selectlen, 1)));

    return is;
}
