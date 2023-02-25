static char sccsid[] = "@(#)74	1.3  src/tcpip/usr/lib/libisode/strb2bitstr.c, isodelib7, tcpip411, GOLD410 4/5/93 17:07:03";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: strb2bitstr
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2bitstr.c
 */

/* strb2bitstr.c - string of bits to bit string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/strb2bitstr.c,v 1.2 93/02/05 17:13:19 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/strb2bitstr.c,v 1.2 93/02/05 17:13:19 snmp Exp $
 *
 *
 * $Log:	strb2bitstr.c,v $
 * Revision 1.2  93/02/05  17:13:19  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:11  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:50  mrose
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

PE	strb2bitstr (cp, len, class, id)
register char  *cp;
register int     len;
PElementClass class;
PElementID id;
{
    register int    i,
		    j,
                    bit,
		    mask;
    register PE	    p;

    if ((p = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	return NULLPE;

    p = prim2bit (p);
    if (len > 0 && bit_off (p, len - 1) == NOTOK) {
no_mem: ;
        pe_free (p);
        return NULLPE;
    }

    for (bit = (*cp & 0xff), i = 0, mask = 1 << (j = 7); i < len; i++) {
	if ((bit & mask) && bit_on (p, i) == NOTOK)
	    goto no_mem;
	if (j-- == 0)
	    bit = *++cp & 0xff, mask = 1 << (j = 7);
	else
	    mask >>= 1;
    }

    return p;
}
