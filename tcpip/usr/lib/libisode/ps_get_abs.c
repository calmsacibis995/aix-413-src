static char sccsid[] = "@(#)43	1.3  src/tcpip/usr/lib/libisode/ps_get_abs.c, isodelib7, tcpip411, GOLD410 4/5/93 15:07:33";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ps_get_abs ps_get_id ps_get_len
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ps_get_abs.c
 */

/* ps_get_abs.c - get absolute length */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_get_abs.c,v 1.2 93/02/05 17:07:18 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ps_get_abs.c,v 1.2 93/02/05 17:07:18 snmp Exp $
 *
 *
 * $Log:	ps_get_abs.c,v $
 * Revision 1.2  93/02/05  17:07:18  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:35  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:23  mrose
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

/*    DATA */

int	ps_len_strategy = PS_LEN_SPAG;

/*  */

int	ps_get_abs (pe)
register PE	pe;
{
    register    PElementLen len = 0;
    register    PE p;

    switch (pe -> pe_form) {
	case PE_FORM_PRIM: 
	    len = pe -> pe_len;
	    break;

	case PE_FORM_CONS: 
	    len = 0;
	    for (p = pe -> pe_cons; p; p = p -> pe_next)
		len += ps_get_abs (p);

	    switch (ps_len_strategy) {
		case PS_LEN_SPAG: 
		default: 
		    if (len <= PE_LEN_SMAX) {
			pe -> pe_len = len;
			break;
		    }		/* else fall */

		case PS_LEN_INDF: 
		    pe -> pe_len = PE_LEN_INDF;
		    len += 2;	/* for EOC */
		    break;

		case PS_LEN_LONG: 
		    pe -> pe_len = len;
		    break;
	    }
	    break;

	case PE_FORM_ICONS:
	    return pe -> pe_len;
    }

    return (ps_get_id (pe) + ps_get_len (pe) + len);
}

/*  */

static int  ps_get_id (pe)
register PE	pe;
{
    register int    i;
    register PElementID id;

    if ((id = pe -> pe_id) < PE_ID_XTND)
	return 1;

    for (i = 1; id != 0; id >>= PE_ID_SHIFT)
	i++;
    return i;
}

/*  */

static int  ps_get_len (pe)
register PE	pe;
{
    register int    i;
    register PElementLen len;

    if ((len = pe -> pe_len) == PE_LEN_INDF || len <= PE_LEN_SMAX)
	return 1;

    for (i = 1; len > 0; len >>= 8)
	i++;
    return i;
}
