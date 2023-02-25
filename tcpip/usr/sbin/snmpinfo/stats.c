static char sccsid[] = "@(#)29	1.4  src/tcpip/usr/sbin/snmpinfo/stats.c, snmp, tcpip411, GOLD410 3/3/93 08:32:58";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: initTime(), displayTime(), stats(), tvsub(), dgram_rfx(),
 *            dgram_wfx()
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/sbin/snmpinfo/stats.c
 */

#ifdef LOCAL

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <isode/snmp/logging.h>
#include <isode/snmp/io.h>

extern errno;
extern int debug;

#define	ADVISE		if (o_advise) (*o_advise)
#define	DUMPBITS	if (o_dumpbits) (*o_dumpbits)
IFP	o_dumpbits = NULLIFP;

extern char *prog_name, host[];

/* borrowed from ping.c */
extern int timing;		/* are we doing timing? */
extern int nvars;		/* number of variables received */
int nreceived = 0;		/* number of packets received */
int nsent = 0;			/* number of packets sent */
int tmin = 999999999;
int tmax = 0;
int tsum = 0;
int triptime;
struct timeval	tz, 		/* time zero */
		to,		/* time out */
		tr,		/* time returned */
		td;		/* time completely done */

initTime ()
{
    gettimeofday (&tz, (struct timezone *) 0);
}

displayTime ()
{
    fprintf (stdout, "  time=%d. ms\n", triptime);
}

stats ()
{
    gettimeofday (&td, (struct timezone *) 0);
    tvsub (&td, &tz);

    fprintf (stderr, "\n----%s SNMP Statistics ----\n", host); 
    fprintf (stderr, "Total time in %s: %d (ms)\n", prog_name, 
	    td.tv_sec*1000+(td.tv_usec/1000));
    fprintf (stderr, "%d/%d packets sent/received.\n", nsent, nreceived);
    fprintf (stderr, "round-trip (ms) min/avg/max = %d/%d/%d\n",
	tmin, tsum / nreceived, tmax);
    fprintf (stderr, "%d snmp variables received in %d (ms)\n", nvars, tsum);

    /* for checking multi-variable packets */
    if (nvars > nreceived) {
	fprintf (stderr, "Multi-variable requests:\n");
	fprintf (stderr, "(ms)/variable avg = %d\n", tsum / nvars);
	if ((tsum / 1000) > 0)	/* ran for at least 1 second? */
	    fprintf (stderr, "variables/second avg %d\n", 
		    nvars / (tsum / 1000));
    }
}

tvsub (out, in)
register struct timeval *out, *in;
{
    if ((out -> tv_usec -= in -> tv_usec) < 0) {
	out -> tv_sec--;
	out -> tv_usec += 1000000;
    }
    out -> tv_sec -= in -> tv_sec;
}

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
	ADVISE (SLOG_DEBUG, "dgram_rfx called with %d, but no fdinfo", fd);
	return NOTOK;
    }

    if ((qb = (struct qbuf *) malloc ((unsigned) (sizeof *qb + fi -> fi_size)))
		== NULL) {
	ADVISE (SLOG_EXCEPTIONS, "dgram_rfx, qbuf malloc failed");
	return NOTOK;
    }
    qb -> qb_data = qb -> qb_base;	/* point data to buf space */

    size = sizeof (struct sockaddr_in);
    if ((cc = recvfrom (fd, qb -> qb_data, fi -> fi_size, NULL, 
	    fi -> sockaddr_in, &size)) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, "dgram_rfx, recvfrom failed, errno %d", 
		errno);
	free ((char *)qb);
	return NOTOK;
    }
    if (timing) {
	gettimeofday (&tr, (struct timezone *) 0);
	tvsub (&tr, &to);
	triptime = tr.tv_sec*1000+(tr.tv_usec/1000);
	tsum += triptime;
	if (triptime < tmin)
	    tmin = triptime;
	if (triptime > tmax)
	    tmax = triptime;
	nreceived++;
    }
    if (debug)
	ADVISE (SLOG_NOTICE, "received %d bytes from %s", cc,
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
	ADVISE (SLOG_DEBUG, "dgram_rfx called with %d, but no fdinfo", fd);
	return NOTOK;
    }
    if (debug)
	ADVISE (SLOG_NOTICE, "sending %d bytes to %s", qb -> qb_len,
		inet_ntoa (fi -> sockaddr_in -> sin_addr));
    if (debug > 1)
	DUMPBITS (qb -> qb_data, qb -> qb_len);

    size = sizeof (struct sockaddr_in);
    if (timing) {
	nsent++;
	gettimeofday (&to, (struct timezone *) 0);
    }
    return (sendto (fd, qb -> qb_data, qb -> qb_len, NULL, fi -> sockaddr_in,
	    size));
}

#else
stats () {}
initTime () {}
displayTime () {}
#endif
