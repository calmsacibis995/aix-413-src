static char sccsid[] = "@(#)42	1.3  src/tcpip/usr/lib/libisode/ps_free.c, isodelib7, tcpip411, GOLD410 4/5/93 15:07:08";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ps_free
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ps_free.c
 */

/* ps_free.c - free a presentation stream */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_free.c,v 1.2 93/02/05 17:07:16 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_free.c,v 1.2 93/02/05 17:07:16 snmp Exp $
 *
 *
 * $Log:	ps_free.c,v $
 * Revision 1.2  93/02/05  17:07:16  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:34  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:22  mrose
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

void	ps_free (ps)
register PS	ps;
{
    if (ps -> ps_closeP)
	(void) (*ps -> ps_closeP) (ps);

    free ((char *) ps);
}
