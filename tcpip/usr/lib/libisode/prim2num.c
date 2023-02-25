static char sccsid[] = "@(#)31	1.4  src/tcpip/usr/lib/libisode/prim2num.c, isodelib7, tcpip411, 9435B411a 8/31/94 12:57:37";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: prim2num
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/prim2num.c
 */

/* prim2num.c - presentation element to integer */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2num.c,v 1.2 93/02/05 17:06:45 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2num.c,v 1.2 93/02/05 17:06:45 snmp Exp $
 *
 *
 * $Log:	prim2num.c,v $
 * Revision 1.2  93/02/05  17:06:45  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:22  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:11  mrose
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

integer	prim2num (pe)
register PE	pe;
{
    register integer    i;
    register PElementData dp,
			  ep;

    if (pe -> pe_form != PE_FORM_PRIM || pe -> pe_prim == NULLPED)
	return pe_seterr (pe, PE_ERR_PRIM, NOTOK);
    if (pe -> pe_len > sizeof (i) + 1)
	return pe_seterr (pe, PE_ERR_OVER, NOTOK);

    pe -> pe_errno = PE_ERR_NONE;/* in case integer is NOTOK-valued */
    i = (*(dp = pe -> pe_prim) & 0x80) ? (-1) : 0;
    for (ep = dp + pe -> pe_len; dp < ep;)
	i = (i << 8) | (*dp++ & 0xff);

    return i;
}
