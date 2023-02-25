static char sccsid[] = "@(#)00	1.3  src/tcpip/usr/lib/libisode/str2spkt.c, isodelib7, tcpip411, GOLD410 4/5/93 17:03:54";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: spkt2str str2spkt
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/str2spkt.c
 */

/* str2spkt.c - read/write a SPDU thru a string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2spkt.c,v 1.2 93/02/05 17:13:10 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/str2spkt.c,v 1.2 93/02/05 17:13:10 snmp Exp $
 *
 *
 * $Log:	str2spkt.c,v $
 * Revision 1.2  93/02/05  17:13:10  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:46:12  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:53  mrose
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
#include <isode/spkt.h>
#include <isode/tailor.h>

/*  */

char   *spkt2str (s)
struct ssapkt *s;
{
    int     i,
	    len;
    char   *base,
	   *dp;
    static char buffer[(CONNECT_MAX + BUFSIZ) * 2 + 1];

    if (spkt2tsdu (s, &base, &len) == NOTOK)
	return NULLCP;
    if (s -> s_udata)
	switch (s -> s_code) {
	    case SPDU_DT:
		if (s -> s_mask & SMASK_SPDU_GT)
		    break;	/* else fall */
	    case SPDU_EX:
	    case SPDU_TD:
		if ((dp = (char *)realloc (base, (unsigned) (i = len + s -> s_ulen)))
			== NULL) {
		    free (base);
		    return NULLCP;
		}
		bcopy (s -> s_udata, (base = dp) + len, s -> s_ulen);
		len = i;
		break;

	    default:
		break;
	}

    buffer[explode (buffer, (u_char *) base, len)] = NULL;
    if (len > 0)
	free (base);

#ifdef	DEBUG
    if (ssap_log -> ll_events & LLOG_PDUS) {
	LLOG (ssap_log, LLOG_PDUS,
	      ("write %d bytes, \"%s\"", strlen (buffer), buffer));
	spkt2text (ssap_log, s, 0);
    }
#endif

    return buffer;
}

/*  */

struct ssapkt *str2spkt (buffer)
char  *buffer;
{
    int	    cc;
    char    packet[CONNECT_MAX + BUFSIZ];
    register struct ssapkt *s;
    struct qbuf qbs;
    register struct qbuf *qb = &qbs,
			 *qp;

    bzero ((char *) qb, sizeof *qb);
    qb -> qb_forw = qb -> qb_back = qb;

    cc = implode ((u_char *) packet, buffer, strlen (buffer));
    if ((qp = (struct qbuf *) malloc (sizeof *qp + (unsigned) cc)) == NULL)
	s = NULLSPKT;
    else {
	bcopy (packet, qp -> qb_data = qp -> qb_base, qp -> qb_len = cc);
	insque (qp, qb -> qb_back);
	s = tsdu2spkt (qb, cc, NULLIP);
	QBFREE (qb);
    }

#ifdef	DEBUG
    if (ssap_log -> ll_events & LLOG_PDUS) {
	LLOG (ssap_log, LLOG_PDUS,
	      ("read %d bytes, \"%s\"", strlen (buffer), buffer));
	spkt2text (ssap_log, s, 1);
    }
#endif

    return s;
}
