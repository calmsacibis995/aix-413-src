static char sccsid[] = "@(#)36	1.3  src/tcpip/usr/lib/libisode/prim2str.c, isodelib7, tcpip411, GOLD410 4/5/93 15:04:31";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: prim2str
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/prim2str.c
 */

/* prim2str.c - presentation element to octet string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2str.c,v 1.2 93/02/05 17:06:56 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2str.c,v 1.2 93/02/05 17:06:56 snmp Exp $
 *
 *
 * $Log:	prim2str.c,v $
 * Revision 1.2  93/02/05  17:06:56  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:27  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:16  mrose
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

/* Similar to pe_pullup.  Returns a newly allocated string, composed of
   of any sub-elements in pe, whereas pe_pullup always reverts "pe" to
   a primitive.  The string is null-terminated, though pe_len specifically
   does NOT reflect this. */

char   *prim2str (pe, len)
register PE	pe;
register int   *len;
{
    register int    i = 0,
                    k;
    int     j;
    register char  *dp = NULL,
                   *ep,
                   *fp;
    register PElementClass class;
    register PElementID id;
    register PE	    p;

    *len = 0;
    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	    if ((dp = (char *)malloc ((unsigned) ((i = pe -> pe_len) + 1))) == NULLCP)
		return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
	    bcopy ((char *) pe -> pe_prim, dp, i);
	    break;

	case PE_FORM_CONS: 
	    if ((p = pe -> pe_cons) == NULLPE) {
		if ((dp = (char *)malloc ((unsigned) ((i = 0) + 1))) == NULLCP)
		    return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
		break;
	    }
	    dp = NULLCP, i = 0;
	    class = p -> pe_class, id = p -> pe_id;
	    for (p = pe -> pe_cons; p; p = p -> pe_next) {
		if ((p -> pe_class != class || p -> pe_id != id)
		        && (p -> pe_class != PE_CLASS_UNIV
			    	|| p -> pe_id != PE_PRIM_OCTS)) {
		    if (dp)
			free (dp);
		    return pe_seterr (pe, PE_ERR_TYPE, NULLCP);
		}

		if ((ep = prim2str (p, &j)) == NULLCP) {
		    if (dp)
			free (dp);
		    return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
		}
		if (dp) {
		    if ((fp = (char *)realloc(dp, (unsigned) ((k = i + j) + 1)))
			    == NULLCP) {
			free (dp);
			return pe_seterr (pe, PE_ERR_NMEM, NULLCP);
		    }
		    bcopy (ep, fp + i, j);
		    dp = fp, i = k;
		}
		else
		    dp = ep, i += j;
	    }
	    break;
    }

    if (dp)
	dp[*len = i] = NULL;

    return dp;
}
