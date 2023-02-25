static char sccsid[] = "@(#)04	1.3  src/tcpip/usr/lib/libisode/isohost.c, isodelib7, tcpip411, GOLD410 4/5/93 13:55:03";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: getlocalhost
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/isohost.c
 */

/* isohost.c - getlocalhost */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/isohost.c,v 1.2 93/02/05 17:05:00 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/isohost.c,v 1.2 93/02/05 17:05:00 snmp Exp $
 *
 *
 * $Log:	isohost.c,v $
 * Revision 1.2  93/02/05  17:05:00  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:21  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:08  mrose
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
#include <string.h>
#include <isode/general.h>
#include <isode/manifest.h>
#ifdef	TCP
#include <isode/internet.h>
#endif
#include <isode/tailor.h>
#ifdef	SYS5
#include <sys/utsname.h>
#endif

/*  */

char   *getlocalhost () {
    register char   *cp;
#ifdef	TCP
    register struct hostent *hp;
#endif
#ifdef	SYS5
    struct utsname uts;
#endif
    static char buffer[BUFSIZ];

    if (buffer[0])
	return buffer;

    isodetailor (NULLCP, 0);
    if (*isodename)
	(void) strcpy (buffer, isodename);
    else {
#if	!defined(SOCKETS) && !defined(SYS5)
	(void) strcpy (buffer, "localhost");
#endif
#ifdef	SOCKETS
	(void) gethostname (buffer, sizeof buffer);
#endif
#ifdef	SYS5
	(void) uname (&uts);
	(void) strcpy (buffer, uts.nodename);
#endif

#ifdef	TCP
	if (hp = gethostbyname (buffer))
	    (void) strcpy (buffer, hp -> h_name);
	else
	    SLOG (addr_log, LLOG_EXCEPTIONS, NULLCP,
		  ("%s: unknown host", buffer));
#endif

	if (cp = index (buffer, '.'))
	    *cp = NULL;
    }

    return buffer;
}
