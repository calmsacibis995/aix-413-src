static char sccsid[] = "@(#)20	1.3  src/tcpip/usr/lib/libisode/pe_cmp.c, isodelib7, tcpip411, GOLD410 4/5/93 14:14:21";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: pe_cmp
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pe_cmp.c
 */

/* pe_cmp.c - compare two presentation elements */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe_cmp.c,v 1.2 93/02/05 17:06:07 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe_cmp.c,v 1.2 93/02/05 17:06:07 snmp Exp $
 *
 *
 * $Log:	pe_cmp.c,v $
 * Revision 1.2  93/02/05  17:06:07  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:10  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:01  mrose
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

int	pe_cmp (p, q)
register PE	p,
		q;
{
    register int    i;

    if (p == NULLPE)
	return (q ? 1 : 0);
    if (q == NULLPE
	    || p -> pe_class != q -> pe_class
	    || p -> pe_form != q -> pe_form
	    || p -> pe_id != q -> pe_id)
	return 1;

/* XXX: perhaps compare pe_context ??? */

    switch (p -> pe_form) {
	case PE_FORM_ICONS:
	    if (p -> pe_ilen != q -> pe_ilen)
		return 1;
	    /* else fall */
	case PE_FORM_PRIM: 
	    if (i = p -> pe_len) {
		if (i != q -> pe_len || PEDcmp (p -> pe_prim, q -> pe_prim, i))
		    return 1;
	    }
	    else
		if (q -> pe_len)
		    return 1;
	    return 0;

	case PE_FORM_CONS: 
	    for (p = p -> pe_cons, q = q -> pe_cons;
		    p;
		    p = p -> pe_next, q = q -> pe_next)
		if (pe_cmp (p, q))
		    return 1;
	    return (q ? 1 : 0);

	default:		/* XXX */
	    return 1;
    }
}
