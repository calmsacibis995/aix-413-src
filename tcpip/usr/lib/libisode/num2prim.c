static char sccsid[] = "@(#)04	1.4  src/tcpip/usr/lib/libisode/num2prim.c, isodelib7, tcpip411, 9435B411a 8/31/94 12:57:14";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: num2prim
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/num2prim.c
 */

/* num2prim.c - integer to presentation element */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/num2prim.c,v 1.2 93/02/05 17:05:25 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/num2prim.c,v 1.2 93/02/05 17:05:25 snmp Exp $
 *
 *
 * $Log:	num2prim.c,v $
 * Revision 1.2  93/02/05  17:05:25  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:49  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:45  mrose
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

PE	num2prim (i, class, id)
register integer i;
PElementClass	class;
PElementID	id;
{
    register integer mask,
                    sign,
                    n;
    register PElementData dp;
    register PE	    pe;

    if ((pe = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	return NULLPE;

    if ((i & 0x80000000) && (class == 1) && 
	((id == 1) || (id == 2) || (id == 3)))
	n = sizeof(i) + 1;
    else
    {
	sign = i >= 0 ? i : i ^ (-1);
	mask = 0x1ff << (((n = sizeof i) - 1) * 8 - 1);
	while (n > 1 && (sign & mask) == 0)
	    mask >>= 8, n--;
    }

    if ((pe -> pe_prim = PEDalloc (n)) == NULLPED) {
	pe_free (pe);
	return NULLPE;
    }

    for (dp = pe -> pe_prim + (pe -> pe_len = n); n-- > 0; i >>= 8)
	*--dp = i & 0xff;

    if ((i & 0x80000000) && (class == 1) && 
	((id == 1) || (id == 2) || (id == 3)))
	*dp = 0;

    return pe;
}
