static char sccsid[] = "@(#)00	1.3  src/tcpip/usr/lib/libisode/implode.c, isodelib7, tcpip411, GOLD410 4/5/93 13:50:14";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: implode
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/implode.c
 */

/* implode.c - explode ascii into octets */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/implode.c,v 1.2 93/02/05 17:04:36 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/implode.c,v 1.2 93/02/05 17:04:36 snmp Exp $
 *
 *
 * $Log:	implode.c,v $
 * Revision 1.2  93/02/05  17:04:36  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:12  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:02  mrose
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

static char hex2nib[0x80] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

/*  */
int	implode (a, b, n)
register u_char *a;
register char  *b;
register int    n;
{
    register int    i;

    for (i = 0; i < n; i += 2) {
	*a++ = (hex2nib[b[0] & 0x7f] << 4) | (hex2nib[b[1] & 0x7f]);
	b += 2;
    }

    return (n / 2);
}
