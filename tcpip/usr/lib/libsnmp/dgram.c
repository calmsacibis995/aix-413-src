static char sccsid[] = "@(#)51	1.7  src/tcpip/usr/lib/libsnmp/dgram.c, snmp, tcpip411, 9434A411a 8/18/94 14:18:32";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: dgram_rfx(), dgram_wfx()
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/dgram.c
 */

/* bring in common include files. */

/* system */
#include <stdio.h>
#include <errno.h>
#include <isode/snmp/io.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

extern errno;
extern int debug;

#define	ADVISE		if (o_advise) (*o_advise)
#define	DUMPBITS	if (o_dumpbits) (*o_dumpbits)
IFP	o_dumpbits = NULLIFP;


/* for isode dgram read service.  Read from fd, output into q.
 * NOTE: overwrite sockaddr_in ever time so that dgram_wfx knows
 *	who to send the response to.
 *
 * RETURN:	failure: NOTOK
 *		success: number of bytes read, bytes in qbuf q.
 */
int
dgram_rfx (fd, q)
int	fd;
struct qbuf **q;
{
    register struct qbuf *qb;
    struct fdinfo *fi;
    int size, cc;

    /* match on fd */
    if ((fi = fd2fi (fd)) == NULL) {
	ADVISE (SLOG_DEBUG, 
		MSGSTR(MS_DGRAM, DGRAM_1, 
		       "%s called with %d, but no fdinfo"), "dgram_rfx", fd);
	return NOTOK;
    }

    if ((qb = (struct qbuf *) malloc ((unsigned) (sizeof *qb + fi -> fi_size)))
		== NULL) {
	ADVISE (SLOG_EXCEPTIONS, 
		MSGSTR(MS_DGRAM, DGRAM_2, "dgram_rfx, qbuf malloc failed"));
	return NOTOK;
    }
    qb -> qb_data = qb -> qb_base;	/* point data to buf space */

    size = sizeof (struct sockaddr_in);
    if ((cc = recvfrom (fd, qb -> qb_data, fi -> fi_size, NULL, 
	    fi -> sockaddr_in, &size)) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, 
		MSGSTR(MS_DGRAM, DGRAM_3, "dgram_rfx, recvfrom failed: %s"), 
		strerror(errno));
	free ((char *)qb);
	return NOTOK;
    }
    if (debug)
	ADVISE (SLOG_DEBUG, 
		MSGSTR(MS_DGRAM, DGRAM_4, "received %d bytes from %s"), cc,
		inet_ntoa (fi -> sockaddr_in -> sin_addr));
    if (debug > 1)
	DUMPBITS (qb -> qb_data, cc);

    qb -> qb_len = cc;
    qb -> qb_forw = qb -> qb_back = qb;
    *q = qb;

    return (qb -> qb_len);
}

/* for isode dgram write service.  Write q to fd.
 * NOTE: sends to last address read_from.
 *
 * RETURN:	failure: NOTOK
 *		success: number of bytes wrote.
 */
int
dgram_wfx (fd, qb)
int	fd;
register struct qbuf *qb;
{
    struct fdinfo *fi;
    int size;

    /* match on fd */
    if ((fi = fd2fi (fd)) == NULL) {
	ADVISE (SLOG_DEBUG, 
		MSGSTR(MS_DGRAM, DGRAM_1, 
		       "%s called with %d, but no fdinfo"), "dgram_wfx", fd);
	return NOTOK;
    }
    if (debug)
	ADVISE (SLOG_DEBUG, 
		MSGSTR(MS_DGRAM, DGRAM_5, "sending %d bytes to %s"), 
		qb -> qb_len, inet_ntoa (fi -> sockaddr_in -> sin_addr));
    if (debug > 1)
	DUMPBITS (qb -> qb_data, qb -> qb_len);

    size = sizeof (struct sockaddr_in);
    return (sendto (fd, qb -> qb_data, qb -> qb_len, NULL, fi -> sockaddr_in,
	    size));
}

