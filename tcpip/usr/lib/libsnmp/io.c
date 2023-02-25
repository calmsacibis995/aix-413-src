static char sccsid[] = "@(#)54	1.8  src/tcpip/usr/lib/libsnmp/io.c, snmp, tcpip411, 9434A411a 8/18/94 14:19:35";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: fdmask(), addfd(), freefi(), remfi(), fd2fi(), init_io(),
 *            fi_check(), type2fi()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/io.c
 */

/* bring in common include files. */

/* system */
#include <stdio.h>
#include <errno.h>
#include <isode/snmp/io.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

extern int dgram_rfx (), dgram_wfx ();
extern errno;
extern int debug;

static struct fdinfo *fdbase;

#define	ADVISE		if (o_advise) (*o_advise)

/*
 * returns the num of fd's, plus the select read bitmask
 */
int
fdmask (mask)
struct fd_set	*mask;
{
    struct fdinfo *fi;
    int high_fd;

    FD_ZERO (mask);
    for (fi = fdbase, high_fd = 0; fi; fi = fi -> fi_next) {
	if (fi -> fi_flags == FI_NOTINUSE)
	    continue;
	FD_SET (fi -> fi_fd, mask);
	if (fi -> fi_fd > high_fd)
	    high_fd = fi -> fi_fd;
    }
    return high_fd;
}

/* add another fd to our linked list of fds.
 * caller is responsible for initializing values.
 */
struct fdinfo *
addfd ()
{
    struct fdinfo *np, *op;

    /* allocate fd info. */
    if ((np = (struct fdinfo *)calloc (1, sizeof *np)) == NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_1,
					"cannot allocate fdinfo structure"));
	return (NULL);
    }
    if ((np -> sockaddr_in = (struct sockaddr_in *)
	    calloc (1, sizeof *(np -> sockaddr_in))) == NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_1,
				"cannot allocate sockaddr_in structure"));
	free (np);
	return (NULL);
    }
    np -> fi_flags = FI_NOTINUSE;

    /* add to list */
    if (fdbase == NULL) 
	return (fdbase = np);

    for (op = fdbase; op -> fi_next != NULL; op = op -> fi_next)
	;
    op -> fi_next = np;
    return (np);
}

static void
freefi (fi)
struct fdinfo *fi;
{
    if (fi -> sockaddr_in)
	free (fi -> sockaddr_in);
    if (fi -> fi_ps)
	ps_free (fi -> fi_ps);
    free (fi);
}

/* remove a fd structure from our list */
void
remfi (fi)
struct fdinfo *fi;
{
    register struct fdinfo *np, *op;

    if (fi == NULL)		/* error check */
	return;

    /* remove from list */
    if (fdbase == NULL) 	/* nothing to do */
	return;

    if (fdbase == fi) {		/* fi is base pointer, special case */
	freefi (fi);
	fdbase = (struct fdinfo *) NULL;
	return;
    }

    /* find fi, and cut out of list */
    for (op = fdbase; op -> fi_next != NULL; op = op -> fi_next) 
	if (op -> fi_next == fi) {
	    np = fi -> fi_next;
	    freefi (fi);
	    op -> fi_next = np;
	    break;		/* break for() */
	}
}

/*
 *	check for, and remove, any invalid fi's.
 *	Invalid entry detected by setting fi_fd to NOTOK.
 */
int
fi_check ()
{
    register struct fdinfo *np, *op;
    int deleted = 0;

    for (op = fdbase; 
	    op != (struct fdinfo *) NULL && 
		op -> fi_next != (struct fdinfo *) NULL; 
	    op = op -> fi_next) 
	if (op -> fi_next -> fi_fd == NOTOK) {
	    deleted++;
	    np = op -> fi_next -> fi_next;
	    freefi (op -> fi_next);
	    op -> fi_next = np;
	}

    /* fdbase is a special case */
    if (fdbase && fdbase -> fi_fd == NOTOK) {
	deleted++;
	np = fdbase -> fi_next;
	freefi (fdbase);
	fdbase = np;
    }
    return deleted;
}

/* given a fd, return a fdinfo pointer */
struct fdinfo *
fd2fi (fd)
int	fd;
{
    struct fdinfo *fi;

    /* match on fd */
    for (fi = fdbase; fi; fi = fi -> fi_next)
	if (fi -> fi_fd == fd)
	    return (fi);
    ADVISE (SLOG_DEBUG, MSGSTR(MS_IO, IO_3, 
				"fd2fi called with %d, but no fdinfo"), fd);
    return NULL;
}

struct fdinfo *
type2fi (type, sfi)
int	type;
struct fdinfo *sfi;
{
    struct fdinfo *fi;

    /* match on type */
    for (fi = sfi == (struct fdinfo *) NULL ? fdbase : sfi; fi; 
	    fi = fi -> fi_next)
	if (fi -> fi_flags & type)
	    return (fi);
    return NULL;
}

/* initialize snmp io 
 * Opens udp port for listening or out-going only, dependent on addr.
 * Initializes isode PS routines.
 *
 * RETURNS: 	NOTOK (failure)
 *		fd	(of newly opened socket)
 */
int
init_io (uport, addr)
int	uport;		/* user-defined port */
u_long	addr;
{
    struct fdinfo *fi;
    struct servent *se;
    int size;
    u_short sin_port;

    /* open and initialize snmp dgram port */
    if ((fi = addfd()) == NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_4, "addfd failed"));
	return NOTOK;
    }

    /* assume snmp port unless told otherwise... */
    if (uport)
	fi -> sockaddr_in -> sin_port = htons (uport);
    else {
	se = getservbyname ("snmp", "udp");
	fi -> sockaddr_in -> sin_port = 
		(se == NULL) ? htons (SNMP_PORT) : se -> s_port;
    }

    /* zero out the port for client connection until after the bind */
    sin_port = fi -> sockaddr_in -> sin_port;
    if (addr != INADDR_ANY)
	fi -> sockaddr_in -> sin_port = 0;
    else
	fi -> sockaddr_in -> sin_addr.s_addr = addr;
    fi -> sockaddr_in -> sin_family = AF_INET;
    if ((fi -> fi_fd = socket (AF_INET, SOCK_DGRAM, 0)) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_5,
			"cannot open snmp socket: %s"), strerror(errno));
	return NOTOK;
    }
    if (bind (fi -> fi_fd, fi -> sockaddr_in, sizeof (struct sockaddr_in)) ==
	    NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_6,
			"cannot bind snmp port: %s"), strerror(errno));
	return NOTOK;
    }

    /* assign port and addr after the bind for client connection */
    if (addr != INADDR_ANY) {
	fi -> sockaddr_in -> sin_port = sin_port;
	fi -> sockaddr_in -> sin_addr.s_addr = addr;
    }
    size = sizeof (fi -> fi_size);
    if (getsockopt (fi -> fi_fd, SOL_SOCKET, SO_SNDBUF,
	    (char *)&(fi -> fi_size), &size) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_7,
					"cannot getsockopt SO_SNDBUF"));
	return NOTOK;
    }

    /* initialize isode PS (for asn encoding/decoding) */
    if ((fi -> fi_ps = ps_alloc (dg_open)) == NULLPS) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_8,
					"cannot open PS for snmp port"));
	return NOTOK;
    }
    if (dg_setup (fi -> fi_ps, fi -> fi_fd, fi -> fi_size,
	    dgram_rfx, dgram_wfx) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_IO, IO_9, 
					"cannot setup PS dgram service"));
	return NOTOK;
    }
    fi -> fi_flags = FI_UDP;
    if (debug)
	ADVISE (SLOG_DEBUG, MSGSTR(MS_IO, IO_10,"bound fd %d to port %d %s %s"),
		fi -> fi_fd,
		ntohs (fi -> sockaddr_in -> sin_port), 
		addr ? MSGSTR(MS_IO, IO_12, "host:") : 
		       MSGSTR(MS_IO, IO_11, "listening"),
		addr ? inet_ntoa (addr) : "");

    ps_len_strategy = PS_LEN_LONG;	/* required for snmp packets */

    return (fi -> fi_fd);
}
