static char sccsid[] = "@(#)79	1.3  src/tcpip/usr/lib/libisode/ts2ps.c, isodelib7, tcpip411, GOLD410 4/5/93 17:16:06";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ts_read ts_write
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ts2ps.c
 */

/* ts2ps.c - TSDU-backed abstraction for PStreams
 		(really just a refinement of datagram-backed PStreams) */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ts2ps.c,v 1.2 93/02/05 17:13:53 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ts2ps.c,v 1.2 93/02/05 17:13:53 snmp Exp $
 *
 *
 * $Log:	ts2ps.c,v $
 * Revision 1.2  93/02/05  17:13:53  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:16  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:55  mrose
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
#include <isode/tsap.h>
#include <isode/tailor.h>

/*  */

int	ts_read (fd, q)
int	fd;
struct qbuf **q;
{
    register struct qbuf *qb;
    struct TSAPdata  txs;
    register struct TSAPdata *tx = &txs;
    struct TSAPdisconnect  tds;
    register struct TSAPdisconnect *td = &tds;

    if (TReadRequest (fd, tx, NOTOK, td) == NOTOK) {
	if (td -> td_reason == DR_NORMAL) {
	    *q = NULL;
	    return OK;
	}

	SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
	      (td -> td_cc > 0 ? "ts_read: [%s] %*.*s" : "ts_read: [%s]",
	       TErrString (td -> td_reason), td -> td_cc, td -> td_cc,
	        td -> td_data));

	return NOTOK;
    }

    qb = &tx -> tx_qbuf;
    if (qb -> qb_forw -> qb_forw != qb && qb_pullup (qb) == NOTOK) {
	SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
	      ("ts_read: qb_pullup fails"));
	TXFREE (tx);

	return NOTOK;
    }

    remque (qb = tx -> tx_qbuf.qb_forw);
    qb -> qb_forw = qb -> qb_back = qb;

    *q = qb;

    TXFREE (tx);

    return qb -> qb_len;
}


int	ts_write (fd, qb)
int	fd;
register struct qbuf *qb;
{
    struct TSAPdisconnect  tds;
    register struct TSAPdisconnect *td = &tds;
    
    if (TDataRequest (fd, qb -> qb_data, qb -> qb_len, td) == NOTOK) {
	SLOG (psap_log, LLOG_EXCEPTIONS, NULLCP,
	      (td -> td_cc > 0 ? "ts_write: [%s] %*.*s" : "ts_write: [%s]",
	       TErrString (td -> td_reason), td -> td_cc, td -> td_cc,
	        td -> td_data));

	return NOTOK;
    }

    return qb -> qb_len;
}
