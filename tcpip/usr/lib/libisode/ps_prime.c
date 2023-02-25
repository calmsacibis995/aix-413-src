static char sccsid[] = "@(#)45	1.3  src/tcpip/usr/lib/libisode/ps_prime.c, isodelib7, tcpip411, GOLD410 4/5/93 15:08:34";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ps_prime
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ps_prime.c
 */

/* ps_prime.c - prime a presentation stream */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_prime.c,v 1.2 93/02/05 17:07:21 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_prime.c,v 1.2 93/02/05 17:07:21 snmp Exp $
 *
 *
 * $Log:	ps_prime.c,v $
 * Revision 1.2  93/02/05  17:07:21  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:36:39  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/07  12:40:37  mrose
 * update
 * 
 * Revision 7.0  89/11/23  22:13:25  mrose
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

int	ps_prime (ps, waiting)
register PS	ps;
int	waiting;
{
    if (ps -> ps_primeP)
	return (*ps -> ps_primeP) (ps, waiting);

    return OK;
}
