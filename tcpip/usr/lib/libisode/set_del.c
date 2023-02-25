static char sccsid[] = "@(#)63	1.3  src/tcpip/usr/lib/libisode/set_del.c, isodelib7, tcpip411, GOLD410 4/5/93 16:40:31";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: set_del
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/set_del.c
 */

/* set_del.c - remove member from set */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/set_del.c,v 1.2 93/02/05 17:11:31 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/set_del.c,v 1.2 93/02/05 17:11:31 snmp Exp $
 *
 *
 * $Log:	set_del.c,v $
 * Revision 1.2  93/02/05  17:11:31  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:58  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:40  mrose
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

int	set_del (pe, class, id)
register PE	pe;
register PElementClass class;
register PElementID id;
{
    register int    pe_id;
    register PE	   *p,
		    q;

    pe_id = PE_ID (class, id);
    for (p = &pe -> pe_cons; q = *p; p = &q -> pe_next)
	if (PE_ID (q -> pe_class, q -> pe_id) == pe_id) {
	    (*p) = q -> pe_next;
	    pe_free (q);
	    return OK;
	}

    return pe_seterr (pe, PE_ERR_MBER, NOTOK);
}
