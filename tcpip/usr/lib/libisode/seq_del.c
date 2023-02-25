static char sccsid[] = "@(#)59	1.3  src/tcpip/usr/lib/libisode/seq_del.c, isodelib7, tcpip411, GOLD410 4/5/93 16:36:15";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: seq_del
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/seq_del.c
 */

/* seq_del.c - delete a member from a sequence */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/seq_del.c,v 1.2 93/02/05 17:11:16 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/seq_del.c,v 1.2 93/02/05 17:11:16 snmp Exp $
 *
 *
 * $Log:	seq_del.c,v $
 * Revision 1.2  93/02/05  17:11:16  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:53  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:37  mrose
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

int	seq_del (pe, i)
register PE	pe;
register int	i;
{
    int	    offset;
    register PE	   *p,
		    q;

    for (p = &pe -> pe_cons, offset = 0;
	    q = *p;
	    p = &q -> pe_next, offset = q -> pe_offset)
	if (q -> pe_offset == i) {
	    if (((*p) = q -> pe_next) == NULLPE)
		pe -> pe_cardinal = offset + 1;
	    pe_free (q);
	    return OK;
	}
	else
	    if (q -> pe_offset > i)
		break;
	
    return pe_seterr (pe, PE_ERR_MBER, NOTOK);
}
