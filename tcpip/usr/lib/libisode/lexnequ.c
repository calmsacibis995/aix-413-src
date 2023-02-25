static char sccsid[] = "@(#)07	1.3  src/tcpip/usr/lib/libisode/lexnequ.c, isodelib7, tcpip411, GOLD410 4/5/93 13:57:43";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: lexnequ
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/lexnequ.c
 */

/* lexnequ.c - Compare two strings ignoring case upto n octets */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/lexnequ.c,v 1.2 93/02/05 17:05:11 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/lexnequ.c,v 1.2 93/02/05 17:05:11 snmp Exp $
 *
 *
 * $Log:	lexnequ.c,v $
 * Revision 1.2  93/02/05  17:05:11  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:26  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:13  mrose
 * Release 6.0
 * 
 */

/*
 *                                NOTICE
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

/*  */

lexnequ (str1, str2, len)
register char   *str1,
		*str2;
int             len;
{
    register int count = 1;

    if (str1 == NULL)
	if (str2 == NULL)
		return (0);
	else
		return (1);

    if (str2 == NULL)
	return (-1);

    while (chrcnv[*str1] == chrcnv[*str2++]) {
	if (count++ >= len)
	    return (0);
	if (*str1++ == NULL)
	    return (0);
    }

    str2--;
    if (chrcnv[*str1] > chrcnv[*str2])
	return (1);
    else
	return (-1);
}
