static char sccsid[] = "@(#)95	1.3  src/tcpip/usr/lib/libisode/bitstr2strb.c, isodelib7, tcpip411, GOLD410 4/5/93 13:44:49";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: bitstr2strb
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/bitstr2strb.c
 */

/* bitstr2strb.c - bit string to string of bits */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/bitstr2strb.c,v 1.2 93/02/05 17:03:44 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/bitstr2strb.c,v 1.2 93/02/05 17:03:44 snmp Exp $
 *
 *
 * $Log:	bitstr2strb.c,v $
 * Revision 1.2  93/02/05  17:03:44  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:32  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:33  mrose
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

char   *bitstr2strb (pe, k)
PE	pe;
int    *k;
{
    register int    i,
		    j,
		    len,
		    bit,
		    mask;
    register char   *dp;
    char   *cp;

    if (pe == NULLPE)
	return NULLCP;

    *k = len = pe -> pe_nbits;
    if ((cp = dp = (char *)calloc (1, (unsigned) (len / 8 + 2))) == NULLCP)
	return NULLCP;

    for (bit = i = 0, mask = 1 << (j = 7); i < len; i++) {
	if (bit_test (pe, i))
	    bit |= mask;
	if (j-- == 0)
	    *dp++ = bit & 0xff, bit = 0, mask = 1 << (j = 7);
	else
	    mask >>= 1;
    }
    if (j != 7)
	*dp = bit & 0xff;

    return cp;
}
