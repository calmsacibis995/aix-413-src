static char sccsid[] = "@(#)65	1.3  src/tcpip/usr/lib/libisode/sprintoid.c, isodelib7, tcpip411, GOLD410 4/5/93 16:43:39";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: sprintoid
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/sprintoid.c
 */

/* sprintoid.c - object identifier to string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintoid.c,v 1.2 93/02/05 17:11:45 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintoid.c,v 1.2 93/02/05 17:11:45 snmp Exp $
 *
 *
 * $Log:	sprintoid.c,v $
 * Revision 1.2  93/02/05  17:11:45  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:37:00  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/08/18  00:44:34  mrose
 * touch-up
 * 
 * Revision 7.0  89/11/23  22:13:42  mrose
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

/*  */

char   *sprintoid (oid)
register OID oid;
{
    register int    i;
    register unsigned int  *ip;
    register char  *bp,
		   *cp;
    static char buffer[BUFSIZ];

    if (oid == NULLOID || oid -> oid_nelem < 1)
	return "";

    bp = buffer;

    for (ip = oid -> oid_elements, i = oid -> oid_nelem, cp = "";
	    i-- > 0;
	    ip++, cp = ".") {
	(void) sprintf (bp, "%s%u", cp, *ip);
	bp += strlen (bp);
    }

    return buffer;
}
