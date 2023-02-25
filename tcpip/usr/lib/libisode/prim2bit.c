static char sccsid[] = "@(#)29	1.3  src/tcpip/usr/lib/libisode/prim2bit.c, isodelib7, tcpip411, GOLD410 4/5/93 14:59:57";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: prim2bit
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/prim2bit.c
 */

/* prim2bit.c - presentation element to bit string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2bit.c,v 1.2 93/02/05 17:06:40 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2bit.c,v 1.2 93/02/05 17:06:40 snmp Exp $
 *
 *
 * $Log:	prim2bit.c,v $
 * Revision 1.2  93/02/05  17:06:40  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:20  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:09  mrose
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

PE	prim2bit (pe)
register PE	pe;
{
    int	    i;
    register PElementData bp;
    register PElementLen len;
    register PE	    p;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM:	/* very paranoid... */
	    if ((bp = pe -> pe_prim) && (len = pe -> pe_len)) {
		if ((i = *bp & 0xff) > 7)
		    return pe_seterr (pe, PE_ERR_BITS, NULLPE);
		pe -> pe_nbits = ((len - 1) * 8) - i;
	    }
	    else
		pe -> pe_nbits = 0;
	    break;

	case PE_FORM_CONS: 
	    pe -> pe_nbits = 0;
	    for (p = pe -> pe_cons; p; p = p -> pe_next) {
		if (prim2bit (p) == NULLPE)
		    return NULLPE;
		pe -> pe_nbits += p -> pe_nbits;
	    }
	    break;
    }

    return pe;
}
