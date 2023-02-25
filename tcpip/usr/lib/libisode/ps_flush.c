static char sccsid[] = "@(#)41	1.3  src/tcpip/usr/lib/libisode/ps_flush.c, isodelib7, tcpip411, GOLD410 4/5/93 15:06:44";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ps_flush
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ps_flush.c
 */

/* ps_flush.c - flush a presentation stream */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_flush.c,v 1.2 93/02/05 17:07:13 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_flush.c,v 1.2 93/02/05 17:07:13 snmp Exp $
 *
 *
 * $Log:	ps_flush.c,v $
 * Revision 1.2  93/02/05  17:07:13  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:33  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:21  mrose
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

int	ps_flush (ps)
register PS	ps;
{
    if (ps -> ps_flushP)
	return (*ps -> ps_flushP) (ps);

    return OK;
}
