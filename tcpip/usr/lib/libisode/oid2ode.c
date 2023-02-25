static char sccsid[] = "@(#)09	1.3  src/tcpip/usr/lib/libisode/oid2ode.c, isodelib7, tcpip411, GOLD410 4/5/93 14:07:18";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: oid2ode_aux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/oid2ode.c
 */

/* oid2ode.c - object identifier to object descriptor  */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid2ode.c,v 1.2 93/02/05 17:05:40 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid2ode.c,v 1.2 93/02/05 17:05:40 snmp Exp $
 *
 *
 * $Log:	oid2ode.c,v $
 * Revision 1.2  93/02/05  17:05:40  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:35:54  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/07/09  14:43:47  mrose
 * sync
 * 
 * Revision 7.0  89/11/23  22:12:49  mrose
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
#include <isode/psap.h>
#include <isode/tailor.h>

/*  */

char   *oid2ode_aux (identifier, quoted)
OID	identifier;
int	quoted;
{
    int	    events;
    register struct isobject *io;
    static char buffer[BUFSIZ];
    
    events = addr_log -> ll_events;
    addr_log -> ll_events = LLOG_FATAL;

    io = getisobjectbyoid (identifier);

    addr_log -> ll_events = events;

    if (io) {
	(void) sprintf (buffer, quoted ? "\"%s\"" : "%s",
			io -> io_descriptor);
	return buffer;
    }

    return sprintoid (identifier);
}
