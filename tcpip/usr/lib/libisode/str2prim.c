static char sccsid[] = "@(#)71	1.3  src/tcpip/usr/lib/libisode/str2prim.c, isodelib7, tcpip411, GOLD410 4/5/93 17:00:39";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: str2prim
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2prim.c
 */

/* str2prim.c - octet string to presentation element */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2prim.c,v 1.2 93/02/05 17:12:58 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2prim.c,v 1.2 93/02/05 17:12:58 snmp Exp $
 *
 *
 * $Log:	str2prim.c,v $
 * Revision 1.2  93/02/05  17:12:58  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:06  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:48  mrose
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

PE	str2prim (s, len, class, id)
register char *s;
register int len;
PElementClass	class;
PElementID	id;
{
    register PE	    pe;

    if ((pe = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	return NULLPE;

    if (len && (pe -> pe_prim = PEDalloc (pe -> pe_len = len)) == NULLPED) {
	pe_free (pe);
	return NULLPE;
    }

    if (s)
	PEDcpy (s, pe -> pe_prim, len);

    return pe;
}
