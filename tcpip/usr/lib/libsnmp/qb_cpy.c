static char sccsid[] = "@(#)61	1.2  src/tcpip/usr/lib/libsnmp/qb_cpy.c, snmp, tcpip411, GOLD410 3/2/93 18:25:20";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: qb_cpy()
 *
 * ORIGINS: 27 60
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/qb_cpy.c
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* same as qb_pullup, does not insque/remque, nor free any elements */

#include <stdio.h>
#include <isode/psap.h>

struct qbuf *
qb_cpy (qb, head)
register struct qbuf *qb;
int head;
{
    register int    len;
    register char  *d;
    register struct qbuf  *p,
			  *rp,
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
	return NULL;

    /* from str2qb.c ... */
    if (head) {
	if ((rp = (struct qbuf *) malloc (sizeof *rp)) == NULL) {
	    free ((char *) p);
	    return NULL;
	}
	rp -> qb_forw = rp -> qb_back = rp;
	rp -> qb_data = NULL, qb -> qb_len = len;
	insque (p, rp);
    } else {
	p -> qb_forw = p -> qb_back = p;
	rp = p;
    }

    d = p -> qb_data = p -> qb_base;
    p -> qb_len = len;
    for (qp = qb -> qb_forw; qp != qb; qp = qpp) {
	qpp = qp -> qb_forw;

	bcopy (qp -> qb_data, d, qp -> qb_len);
	d += qp -> qb_len;
    }
    *d = NULL;

    return rp;
}
