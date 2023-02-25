static char sccsid[] = "@(#)99	1.3  src/tcpip/usr/lib/libisode/flag2prim.c, isodelib7, tcpip411, GOLD410 4/5/93 13:49:06";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: flag2prim
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/flag2prim.c
 */

/* flag2prim.c - boolean to presentation element */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/flag2prim.c,v 1.2 93/02/05 17:04:24 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/flag2prim.c,v 1.2 93/02/05 17:04:24 snmp Exp $
 *
 *
 * $Log:	flag2prim.c,v $
 * Revision 1.2  93/02/05  17:04:24  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:38  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:36  mrose
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

PE	flag2prim (b, class, id)
register int b;
PElementClass	class;
PElementID	id;
{
    register PE	    pe;

    if ((pe = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	return NULLPE;

    if ((pe -> pe_prim = PEDalloc (pe -> pe_len = 1)) == NULLPED) {
	pe_free (pe);
	return NULLPE;
    }

    *pe -> pe_prim = b ? 0xff : 0x00;

    return pe;
}
