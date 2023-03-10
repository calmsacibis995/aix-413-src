static char sccsid[] = "@(#)05	1.3  src/tcpip/usr/lib/libisode/obj2prim.c, isodelib7, tcpip411, GOLD410 4/5/93 14:04:00";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: obj2prim
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/obj2prim.c
 */

/* obj2prim.c - object identifier to presentation element */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/obj2prim.c,v 1.2 93/02/05 17:05:28 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/obj2prim.c,v 1.2 93/02/05 17:05:28 snmp Exp $
 *
 *
 * $Log:	obj2prim.c,v $
 * Revision 1.2  93/02/05  17:05:28  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:50  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:46  mrose
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

PE	obj2prim (o, class, id)
register OID	o;
PElementClass	class;
PElementID	id;
{
    register int    i,
		    m,
		    n,
		   *mp,
		   *np;
    register unsigned int j,
			 *ip;
    register PElementData dp,
			  ep;
    register PE	    pe;

    if (o == NULLOID || o -> oid_nelem <= 1)
	return NULLPE;

    if ((pe = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	return NULLPE;

    if ((np = (int *) malloc ((unsigned) (o -> oid_nelem) * sizeof *np))
	    == NULL) {
	pe_free (pe);
	return NULLPE;
    }

    for (i = n = 0, ip = o -> oid_elements, mp = np;
	    i < o -> oid_nelem;
	    i++, ip++) {
	if (ip == o -> oid_elements)
	    j = *ip++ * 40, i++, j+= *ip;
	else
	    j = *ip;
	m = 0;
	do {
	    m++;
	    j >>= 7;
	}
	while (j);
	n += (*mp++ = m);
    }

    if ((pe -> pe_prim = PEDalloc (pe -> pe_len = n)) == NULLPED) {
	free ((char *) np);
	pe_free (pe);
	return NULLPE;
    }

    dp = pe -> pe_prim; 
    for (i = 0, ip = o -> oid_elements, mp = np;
	    i < o -> oid_nelem;
	    i++, ip++) {
	if (ip == o -> oid_elements)
	    j = *ip++ * 40, i++, j += *ip;
	else
	    j = *ip;

	ep = dp + (m = *mp++) - 1;
	for (dp = ep; m-- > 0; j >>= 7)
	    *dp-- = (j & 0x7f) | 0x80;
	*ep &= ~0x80;
	dp = ep + 1;
    }

    free ((char *) np);

    return pe;
}
