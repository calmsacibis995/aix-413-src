static char sccsid[] = "@(#)55	1.3  src/tcpip/usr/lib/libisode/qbuf2ps.c, isodelib7, tcpip411, GOLD410 4/5/93 15:22:21";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: qbuf_close qbuf_open qbuf_read
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/qbuf2ps.c
 */

/* qbuf2ps.c - qbuf-backed abstractions for PStreams */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/qbuf2ps.c,v 1.2 93/02/05 17:08:31 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/qbuf2ps.c,v 1.2 93/02/05 17:08:31 snmp Exp $
 *
 *
 * $Log:	qbuf2ps.c,v $
 * Revision 1.2  93/02/05  17:08:31  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:49  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:33  mrose
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

/* ARGSUSED */

static int  qbuf_read (ps, data, n, in_line)
register PS	ps;
PElementData data;
PElementLen n;
int	in_line;
{
    register int cc,
		 i;
    register struct qbuf *qb,
			 *qp;

    if ((qb = (struct qbuf *) ps -> ps_addr) == NULL)
	return 0;

    for (qp = NULL, cc = 0; n > 0; data += i, cc += i, n -= i) {
	if (qp == NULL && (qp = qb -> qb_forw) == qb)
	    return cc;

	i = min (qp -> qb_len, n);
	bcopy (qp -> qb_data, (char *) data, i);

	qp -> qb_data += i, qp -> qb_len -= i;
	if (qp -> qb_len <= 0) {
	    remque (qp);

	    free ((char *) qp);
	    qp = NULL;
	}
    }

    return cc;
}


static int  qbuf_close (ps)
register PS	ps;
{
    register struct qbuf   *qb;

    if ((qb = (struct qbuf *) ps -> ps_addr) == NULL)
	return(0);

    QBFREE (qb);

    return(0);
}

/*  */

int	qbuf_open (ps)
register PS	ps;
{
    ps -> ps_readP = qbuf_read;
    ps -> ps_closeP = qbuf_close;

    return OK;
}
