static char sccsid[] = "@(#)97	1.3  src/tcpip/usr/lib/libisode/explode.c, isodelib7, tcpip411, GOLD410 4/5/93 13:48:17";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: explode
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/explode.c
 */

/* explode.c - explode octets into ascii */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/explode.c,v 1.2 93/02/05 17:04:18 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/explode.c,v 1.2 93/02/05 17:04:18 snmp Exp $
 *
 *
 * $Log:	explode.c,v $
 * Revision 1.2  93/02/05  17:04:18  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:09  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:22:59  mrose
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

/*    DATA */

static char nib2hex[0x10] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/*  */

int	explode (a, b, n)
register char  *a;
register u_char *b;
register int    n;
{
    register int    i;
    register u_char c;

    for (i = 0; i < n; i++) {
	c = *b++;
	*a++ = nib2hex[(c & 0xf0) >> 4];
	*a++ = nib2hex[(c & 0x0f)];
    }
    *a = NULL;

    return (n * 2);
}
