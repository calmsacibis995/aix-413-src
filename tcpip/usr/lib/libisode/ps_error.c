static char sccsid[] = "@(#)40	1.1.1.3  src/tcpip/usr/lib/libisode/ps_error.c, isodelib7, tcpip411, GOLD410 4/5/93 15:06:12";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ps_error
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ps_error.c
 */

/* ps_error.c - presentation stream error to string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_error.c,v 1.3 93/02/09 13:06:14 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_error.c,v 1.3 93/02/09 13:06:14 snmp Exp $
 *
 *
 * $Log:	ps_error.c,v $
 * Revision 1.3  93/02/09  13:06:14  snmp
 * Race condition for smux. D60188
 * 
 * Revision 7.3  91/02/22  09:36:32  mrose
 * Interim 6.8
 * 
 * Revision 7.2  91/01/11  07:09:10  mrose
 * jpo
 * 
 * Revision 7.1  91/01/07  12:40:35  mrose
 * update
 * 
 * Revision 7.0  89/11/23  22:13:20  mrose
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
#include <errno.h>

/*  */

static char *ps_errorlist[] = {
    "Error 0",
    "Overflow in ID",
    "Overflow in length",
    "Out of memory",
    "End of file",
    "End of file reading extended ID",
    "End of file reading extended length",
    "Length Mismatch",
    "Truncated",
    "Indefinite length in primitive form",
    "I/O error",
    "Extraneous octets",
    "XXX"
};

static int ps_maxerror = sizeof ps_errorlist / sizeof ps_errorlist[0];

/*  */

char   *ps_error (c)
int	c;
{
    register char  *bp;
    static char buffer[80];

    if (c==PS_ERR_IO) /* return system error code for I/O errors */
    {
	sprintf(buffer, "I/O Error, %s", strerror(errno));
	return buffer;
    }

    if (c < ps_maxerror && (bp = ps_errorlist[c]))
	return bp;

    (void) sprintf (buffer, "Error %d", c);
    return buffer;
}
