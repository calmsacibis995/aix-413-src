static char sccsid[] = "@(#)52	1.3  src/tcpip/usr/lib/libisode/qb_pullup.c, isodelib7, tcpip411, GOLD410 4/5/93 15:21:24";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: qb_pullup
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/qb_pullup.c
 */

/* qb_pullup.c - "pullup" a list of qbufs */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/qb_pullup.c,v 1.2 93/02/05 17:08:26 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/qb_pullup.c,v 1.2 93/02/05 17:08:26 snmp Exp $
 *
 *
 * $Log:	qb_pullup.c,v $
 * Revision 1.2  93/02/05  17:08:26  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:46  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:31  mrose
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

int	qb_pullup (qb)
register struct qbuf *qb;
{
    register int    len;
    register char  *d;
    register struct qbuf  *p,
			  *qp,
    			  *qpp;

    len = 0;
#ifdef	notdef		/* want null-termination... */
    if ((p = qb -> qb_forw) -> qb_forw == qb)
	return OK;
#endif
    for (p = qb -> qb_forw; p != qb; p = p -> qb_forw)
	len += p -> qb_len;

    if ((p = (struct qbuf *) malloc ((unsigned) (sizeof *p + len + 1)))
	    == NULL)
	return NOTOK;
    d = p -> qb_data = p -> qb_base;
    p -> qb_len = len;

    for (qp = qb -> qb_forw; qp != qb; qp = qpp) {
	qpp = qp -> qb_forw;

	remque (qp);

	bcopy (qp -> qb_data, d, qp -> qb_len);
	d += qp -> qb_len;

	free ((char *) qp);
    }
    *d = NULL;

    insque (p, qb);

    return OK;
}
