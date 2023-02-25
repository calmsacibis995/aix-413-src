static char sccsid[] = "@(#)49	1.3  src/tcpip/usr/lib/libisode/qb2prim.c, isodelib7, tcpip411, GOLD410 4/5/93 15:20:30";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: qb2prim_aux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pb2prim.c
 */

/* qb2prim.c - octet string to primitive */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/qb2prim.c,v 1.2 93/02/05 17:08:19 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/qb2prim.c,v 1.2 93/02/05 17:08:19 snmp Exp $
 *
 *
 * $Log:	qb2prim.c,v $
 * Revision 1.2  93/02/05  17:08:19  snmp
 * ANSI - D67743
 * 
 * Revision 7.3  91/02/22  09:36:43  mrose
 * Interim 6.8
 * 
 * Revision 7.2  90/03/23  11:05:53  mrose
 * typo
 * 
 * Revision 7.1  90/03/22  08:38:15  mrose
 * touch-up
 * 
 * Revision 7.0  89/11/23  22:13:29  mrose
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

/* form: PRIMitive or CONStructor --

	qb2prim  -	octet string (via qbufs) to presentation element

 */

/*  */

PE	qb2prim_aux (qb, class, id, in_line)
register struct qbuf *qb;
PElementClass	class;
PElementID	id;
int	in_line;
{
    register PE	    pe,
		    p;
    register struct qbuf *qp;

    if (qb == NULL)
	return NULLPE;
    
    if ((qp = qb -> qb_forw) == qb || qp -> qb_forw == qb) {
	if ((pe = pe_alloc (class, PE_FORM_PRIM, id)) == NULLPE)
	    return NULLPE;

	if (in_line) {
	    if (pe -> pe_len = qp -> qb_len)
		pe -> pe_prim = (PElementData) qp -> qb_data;
	    pe -> pe_inline = 1;
	}
	else
	    if (pe -> pe_len = qp -> qb_len) {
		if ((pe -> pe_prim = PEDalloc (pe -> pe_len)) == NULLPED)
		    goto no_mem;
		PEDcpy (qp -> qb_data, pe -> pe_prim, pe -> pe_len);
	    }
    }
    else {
	if ((pe = pe_alloc (class, PE_FORM_CONS, id)) == NULLPE)
	    return NULLPE;

	do {
	    if (seq_add (pe, p = pe_alloc (PE_CLASS_UNIV, PE_FORM_PRIM,
					   PE_PRIM_OCTS), -1) == NOTOK) {
no_mem: ;
		pe_free (pe);
		return NULLPE;
	    }

	    p -> pe_len = qp -> qb_len;
	    if (in_line) {
		p -> pe_prim = (PElementData) qp -> qb_data;
		p -> pe_inline = 1;
	    }
	    else {
		if ((p -> pe_prim = PEDalloc (p -> pe_len)) == NULLPED)
		    goto no_mem;
		PEDcpy (qp -> qb_data, p -> pe_prim, p -> pe_len);
	    }

	    qp = qp -> qb_forw;
	}
	while (qp != qb);
    }

    return pe;
}
