static char sccsid[] = "@(#)23	1.3  src/tcpip/usr/lib/libisode/pe_expunge.c, isodelib7, tcpip411, GOLD410 4/5/93 14:15:43";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: pe_expunge
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pe_expunge.c
 */

/* pe_expunge.c - expunge a PE */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe_expunge.c,v 1.2 93/02/05 17:06:14 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pe_expunge.c,v 1.2 93/02/05 17:06:14 snmp Exp $
 *
 *
 * $Log:	pe_expunge.c,v $
 * Revision 1.2  93/02/05  17:06:14  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:36:13  mrose
 * Interim 6.8
 * 
 * Revision 7.1  91/02/12  18:32:46  mrose
 * upate
 * 
 * Revision 7.0  89/11/23  22:13:03  mrose
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

PE	pe_expunge (pe, r)
PE	pe,
	r;
{
    if (r) {
	if (pe == r)
	    return r;

	if (pe_extract (pe, r))
	    if (pe -> pe_realbase && !r -> pe_realbase) {
		r -> pe_realbase = pe -> pe_realbase;
		pe -> pe_realbase = NULL;
	    }

	r -> pe_refcnt++;
    }

    pe_free (pe);

    return r;
}
