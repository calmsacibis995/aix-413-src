static char sccsid[] = "@(#)35	1.3  src/tcpip/usr/lib/libisode/prim2set.c, isodelib7, tcpip411, GOLD410 4/5/93 15:04:04";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: prim2set
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/prim2set.c
 */

/* prim2flag.c - presentation element to set */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2set.c,v 1.2 93/02/05 17:06:54 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2set.c,v 1.2 93/02/05 17:06:54 snmp Exp $
 *
 *
 * $Log:	prim2set.c,v $
 * Revision 1.2  93/02/05  17:06:54  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:26  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:15  mrose
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

PE	prim2set (pe)
register PE	pe;
{
    register int    i;
    register PE	    p;

    if (pe -> pe_form != PE_FORM_CONS)
	return pe_seterr (pe, PE_ERR_CONS, NULLPE);

    for (i = 0, p = pe -> pe_cons; p; p = p -> pe_next)
	p -> pe_offset = i++;

    pe -> pe_cardinal = i;

    return pe;
}
