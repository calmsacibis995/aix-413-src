static char sccsid[] = "@(#)62	1.3  src/tcpip/usr/lib/libisode/set_addon.c, isodelib7, tcpip411, GOLD410 4/5/93 16:39:55";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: set_addon
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/set_addon.c
 */

/* set_addon.c - add member to end of a set */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/set_addon.c,v 1.2 93/02/05 17:11:29 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/set_addon.c,v 1.2 93/02/05 17:11:29 snmp Exp $
 *
 *
 * $Log:	set_addon.c,v $
 * Revision 1.2  93/02/05  17:11:29  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:57  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:39  mrose
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

int	set_addon (pe, last, new)
PE	pe,
        last,
        new;
{
    if (pe == NULLPE)
	return NOTOK;
    if (last == NULLPE)
	return set_add (pe, new);
    new -> pe_offset = pe -> pe_cardinal++;
    last -> pe_next = new;

    return OK;
}
