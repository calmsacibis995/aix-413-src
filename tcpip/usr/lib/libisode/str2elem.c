static char sccsid[] = "@(#)28	1.3  src/tcpip/usr/lib/libisode/str2elem.c, isodelib7, tcpip411, GOLD410 4/5/93 16:59:15";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2elem
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2elem.c
 */

/* str2elem.c - string to list of integers */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2elem.c,v 1.2 93/02/05 17:12:53 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2elem.c,v 1.2 93/02/05 17:12:53 snmp Exp $
 *
 *
 * $Log:	str2elem.c,v $
 * Revision 1.2  93/02/05  17:12:53  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:15:55  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/11/21  11:29:57  mrose
 * sun
 * 
 * Revision 7.0  89/11/23  21:23:35  mrose
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

#include <ctype.h>
#include <stdio.h>
#include <isode/general.h>
#include <isode/manifest.h>

/*  */

int	str2elem (s, elements)
char   *s;
unsigned int elements[];
{
    register int    i;
    register unsigned int  *ip;
    register char  *cp,
                   *dp;

    ip = elements, i = 0;
    dp = s;
    for (cp = s; *cp && i <= NELEM; cp = ++dp) {
	for (dp = cp; isdigit ((u_char) *dp); dp++)
	    continue;
	if ((cp == dp) || (*dp && *dp != '.'))
	    break;
	*ip++ = (unsigned int) atoi (cp), i++;
	if (*dp == NULL)
	    break;
    }
    if (*dp || i >= NELEM)
	return NOTOK;
    *ip = 0;

    return i;
}
