static char sccsid[] = "@(#)10	1.3  src/tcpip/usr/lib/libisode/oid_cmp.c, isodelib7, tcpip411, GOLD410 4/5/93 14:52:20";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: elem_cmp oid_cmp
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/oid_cmp.c
 */

/* oid_cmp.c - compare two object identifiers */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid_cmp.c,v 1.2 93/02/05 17:05:42 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/oid_cmp.c,v 1.2 93/02/05 17:05:42 snmp Exp $
 *
 *
 * $Log:	oid_cmp.c,v $
 * Revision 1.2  93/02/05  17:05:42  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:56  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:50  mrose
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

int	oid_cmp (p, q)
register OID	p,
		q;
{
    if (p == NULLOID)
	return (q ? -1 : 0);

    return elem_cmp (p -> oid_elements, p -> oid_nelem,
		     q -> oid_elements, q -> oid_nelem);
}

/*  */

int	elem_cmp (ip, i, jp, j)
register int   i,
	       j;
register unsigned int *ip,
		      *jp;
{
    while (i > 0) {
	if (j == 0)
	    return 1;
	if (*ip > *jp)
	    return 1;
	else
	    if (*ip < *jp)
		return (-1);

	ip++, i--;
	jp++, j--;
    }
    return (j == 0 ? 0 : -1);
}
