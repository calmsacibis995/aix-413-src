static char sccsid[] = "@(#)58	1.3  src/tcpip/usr/lib/libisode/seq_addon.c, isodelib7, tcpip411, GOLD410 4/5/93 16:35:34";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: seq_addon
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/seq_addon.c
 */

/* seq_addon.c - add a member to the end of a sequence (efficiency hack) */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/seq_addon.c,v 1.2 93/02/05 17:11:14 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/seq_addon.c,v 1.2 93/02/05 17:11:14 snmp Exp $
 *
 *
 * $Log:	seq_addon.c,v $
 * Revision 1.2  93/02/05  17:11:14  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:52  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:36  mrose
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

int	seq_addon (pe, last, new)
PE	pe,
        last,
        new;
{
    if (pe == NULLPE)
	return NOTOK;
    if (last == NULLPE)
	return seq_add (pe, new, -1);
    new -> pe_offset = pe -> pe_cardinal++;
    last -> pe_next = new;
    return OK;
}
