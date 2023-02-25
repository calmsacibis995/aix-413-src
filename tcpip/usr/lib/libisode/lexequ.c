static char sccsid[] = "@(#)06	1.3  src/tcpip/usr/lib/libisode/lexequ.c, isodelib7, tcpip411, GOLD410 4/5/93 13:57:05";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: lexequ
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/lexequ.c
 */

/* lexequ.c - Compare two strings ignoring case */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/lexequ.c,v 1.2 93/02/05 17:05:10 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/lexequ.c,v 1.2 93/02/05 17:05:10 snmp Exp $
 *
 *
 * $Log:	lexequ.c,v $
 * Revision 1.2  93/02/05  17:05:10  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:15:25  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/10/15  18:19:55  mrose
 * sync
 * 
 * Revision 7.0  89/11/23  21:23:12  mrose
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

lexequ (str1, str2)
register char   *str1,
		*str2;
{
    if (str1 == NULL)
	if (str2 == NULL)
		return (0);
	else
		return (-1);

    if (str2 == NULL)
	return (1);

    while (chrcnv[*str1] == chrcnv[*str2]) {
	if (*str1++ == NULL)
	    return (0);
	str2++;
    }

    if (chrcnv[*str1] > chrcnv[*str2])
	return (1);
    else
	return (-1);
}
