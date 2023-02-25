static char sccsid[] = "@(#)24	1.3  src/tcpip/usr/lib/libisode/pe_extract.c, isodelib7, tcpip411, GOLD410 4/5/93 14:16:13";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: pe_extract
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pe_extract.c
 */

/* pe_extract.c - extract a PE */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe_extract.c,v 1.2 93/02/05 17:06:18 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe_extract.c,v 1.2 93/02/05 17:06:18 snmp Exp $
 *
 *
 * $Log:	pe_extract.c,v $
 * Revision 1.2  93/02/05  17:06:18  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:36:14  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/01/24  14:50:25  mrose
 * update
 * 
 * Revision 7.0  89/11/23  22:13:04  mrose
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

/* assumes that q appears at most once directly under p... */

int	pe_extract (pe, r)
PE	pe,
	r;
{
    register PE	   *p,
		    q;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	case PE_FORM_ICONS: 
	    break;

	case PE_FORM_CONS: 
	    for (p = &pe -> pe_cons; q = *p; p = &q -> pe_next)
		if (q == r) {
		    (*p) = q -> pe_next;
		    q -> pe_next = NULLPE;
		    if (r->pe_refcnt > 0)
			    r->pe_refcnt--;
		    return 1;
		}
		else
		    if (pe_extract (q, r))
			return 1;
	    break;
    }

    return 0;
}
