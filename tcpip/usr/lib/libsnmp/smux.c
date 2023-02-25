static char sccsid[] = "@(#)63	1.10  src/tcpip/usr/lib/libsnmp/smux.c, snmp, tcpip411, GOLD410 10/29/93 09:58:45";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: smux_init(), smuxalloc(), smux_simple_open(), smuxsend(), 
 *            smux_close(), smux_register(), smux_wait(), smux_response(),
 *	      smux_trap(), smuxlose(), smux_error(), smux_free_child(),
 *            smux_free_tree()
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/smux.c
 */

/* 
 * Contributed by NYSERNet Inc.  This work was partially supported by the
 * U.S. Defense Advanced Research Projects Agency and the Rome Air Development
 * Center of the U.S. Air Force Systems Command under contract number
 * F30602-88-C-0016.
 *
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

#define DEBUG

/* smux.c - SMUX initiator library */

/* LINTLIBRARY */

#include <stdio.h>
#include <varargs.h>
#include <isode/snmp/smux.h>
#include <isode/snmp/objects.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"


#include <errno.h>
#include <isode/internet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/param.h>

#if defined(RT) && defined(AIX)
#undef PS
#endif

/*    DATA */

integer	smux_errno;
char    smux_info[BUFSIZ];


static	int	sd = NOTOK;
static	PS	ps = NULLPS;

static	struct sockaddr_in in_socket;


static	int	smux_debug = 0;
static	PE	smux_pe = NULL;
static	struct type_SMUX_PDUs *smux_pdu;
static	OID	smux_enterprise = NULL;
static struct type_SMI_NetworkAddress *smux_addr = NULL;
static struct type_SMI_TimeTicks *smux_stamp = NULL;

static	struct timeval my_boottime;

char * snmp_address = "loopback"; /* not necessarily "127.0.0.1" */

extern	int	errno;

/*    INIT */

int
smux_init (debug)
int	debug;
{
    int	    onoff;
    register struct sockaddr_in *isock = &in_socket;
    register struct hostent *hp;
    register struct servent *sp;
    static int inited = 0;

    if (!inited) 
	inited = 1;

    smux_debug = debug;
    if (smux_pe)
	pe_free (smux_pe), smux_pe = NULL;
    if (smux_pdu)
	free_SMUX_PDUs (smux_pdu), smux_pdu = NULL;
    if (smux_enterprise)
	oid_free (smux_enterprise), smux_enterprise = NULL;
    if (smux_addr)
	free_SMI_NetworkAddress (smux_addr), smux_addr = NULL;
    if (smux_stamp == NULL
	    && (smux_stamp = (struct type_SMI_TimeTicks *)
				    calloc (1, sizeof *smux_stamp)) == NULL)
	return smuxlose (congestion, NULLCP, MSGSTR(MS_GENERAL, GENERA_1,
			 "%s: out of memory"), "smux_init");

    bzero ((char *) isock, sizeof *isock);
    if ((hp = gethostbystring (snmp_address)) == NULL) {
	char hostname[MAXHOSTNAMELEN];

	/* in case the nameserver doesn't define "loopback",
	   and the host does not fall back on /etc/hosts,
	   use the hostname of the box.  Do NOT fall back
	   on the local host name if 'snmp_address' has
	   been changed to something other than loopback,
	   ie, they tried to go off the box.  (RLF) */
	if (!strcmp (snmp_address, "loopback") &&
		gethostname (hostname, sizeof hostname) != NOTOK) 
	    hp = gethostbystring (hostname);

	if (hp == NULL) {
	    return smuxlose (youLoseBig, NULLCP, MSGSTR(MS_SMUX, SMUX_1,
			    "%s: unknown host"), 
			    snmp_address);
	}
    }
    isock -> sin_family = hp -> h_addrtype;
    isock -> sin_port = (sp = getservbyname ("smux", "tcp"))
						       ? sp -> s_port
						       : htons ((u_short) 199);
    inaddr_copy (hp, isock);

    if ((sd = start_tcp_client ((struct sockaddr_in *) NULL, 0)) == NOTOK)
	return smuxlose (systemError, MSGSTR(MS_SMUX, SMUX_2, "failed"), 
			"start_tcp_client");

    (void) ioctl (sd, FIONBIO, (onoff = 1, (char *) &onoff));

    if (join_tcp_server (sd, isock) == NOTOK)
	switch (errno) {
	    case EINPROGRESS:
	        return sd;

	    case EISCONN:
		break;

	    default:
		(void) smuxlose (systemError, MSGSTR(MS_SMUX, SMUX_2, "failed"),
				"join_tcp_server");
		(void) close_tcp_socket (sd);
		return (sd = NOTOK);
	}

    if (smuxalloc () == NOTOK)
	return NOTOK;

    (void) gettimeofday (&my_boottime, (struct timezone *) 0);

    return sd;
}

/*  */

static int  
smuxalloc ()
{
    int	    len;

    if ((ps = ps_alloc (fdx_open)) == NULLPS || fdx_setup (ps, sd) == NOTOK) {
	if (ps) {
	    ps_free (ps), ps = NULLPS;
	    (void) smuxlose (youLoseBig, NULLCP, "fdx_setup: %s",
			     ps_error (ps -> ps_errno));
	}
	else
	    (void) smuxlose (youLoseBig, NULLCP, "ps_alloc: %s", 
				MSGSTR(MS_SMUX, SMUX_2, "failed"));

you_lose: ;
	(void) close_tcp_socket (sd);
	return (sd = NOTOK);
    }

    if (getsockname (sd, (struct sockaddr *) &in_socket,
		     (len = sizeof in_socket, &len)) == NOTOK)
	bzero ((char *) &in_socket.sin_addr, 4);
    if ((smux_addr = str2qb ((char *) &in_socket.sin_addr, 4, 1)) == NULL) {
	(void) smuxlose (youLoseBig, NULLCP, "str2qb: %s",
				MSGSTR(MS_SMUX, SMUX_2, "failed"));

	ps_free (ps), ps = NULLPS;
	goto you_lose;
    }

    return OK;
}

/*    SIMPLE OPEN */

int
smux_simple_open (identity, description, commname, commlen)
OID	identity;
char   *description;
char   *commname;
int	commlen;
{
    int	    result;
    struct type_SMUX_PDUs pdu;
    register struct type_SMUX_SimpleOpen *simple;

    if (identity == NULL
	    || description == NULL
	    || (commname == NULL && commlen != 0))
	return smuxlose (parameterMissing, NULLCP, MSGSTR(MS_SMUX, SMUX_3,
			"%s: missing parameter"), "smux_simple_open");

    if (sd == NOTOK)
	return smuxlose (invalidOperation, NULLCP, MSGSTR(MS_SMUX, SMUX_4,
			"%s: SMUX not initialized"), "smux_simple_open");
    if (ps == NULLPS) {
	fd_set	mask;
	register struct sockaddr_in *isock = &in_socket;

	FD_ZERO (&mask);
	FD_SET (sd, &mask);
	if (select (sd + 1, NULLFD, &mask, NULLFD, 0) < 1)
	    goto not_yet;

	if (join_tcp_server (sd, isock) == NOTOK)
	    switch (errno) {
	        case EINPROGRESS:
not_yet: ;
    		    return smuxlose (inProgress, NULLCP, NULLCP);

	    case EISCONN:
		break;

	    default:
		(void) smuxlose (systemError, MSGSTR(MS_SMUX, SMUX_2, "failed"),
				"join_tcp_server");
		(void) close_tcp_socket (sd);
		return (sd = NOTOK);
	}

	if (smuxalloc () == NOTOK)
	    return NOTOK;
    }

    bzero ((char *) &pdu, sizeof pdu);

    if ((simple = (struct type_SMUX_SimpleOpen *) calloc (1, sizeof *simple))
	== NULL) {
no_mem: ;
        (void) smuxlose (congestion, NULLCP, MSGSTR(MS_GENERAL, GENERA_1,
			"%s: out of memory"), "smux_simple_open");
	if (simple)
	    free_SMUX_SimpleOpen (simple);

	ps_free (ps), ps = NULLPS;
	(void) close_tcp_socket (sd);
	return (sd = NOTOK);
    }
    pdu.offset = type_SMUX_PDUs_simple;
    pdu.un.simple = simple;

    if ((smux_enterprise = oid_cpy (identity)) == NULL)
	goto no_mem;
	
    simple -> version = int_SMUX_version_version__1;
    if ((simple -> identity = oid_cpy (identity)) == NULLOID
	    || (simple -> description = str2qb (description,
						strlen (description),
						1)) == NULL
	    || (simple -> password = str2qb (commname, commlen, 1)) == NULL)
	goto no_mem;

    result = smuxsend (&pdu);

    free_SMUX_SimpleOpen (simple);

    return result;
}

/*  */

static int
smuxsend (pdu)
struct type_SMUX_PDUs *pdu;
{
    int	    result;
    PE	    pe;

    pe = NULLPE;
    if (encode_SMUX_PDUs (&pe, 1, 0, NULLCP, pdu) == NOTOK) {
	result = smuxlose (youLoseBig, NULLCP, "encode_SMUX_PDUs: %s",
			   PY_pepy);
	goto out;
    }

#ifdef	DEBUG
    if (smux_debug  > 2)
	(void) print_SMUX_PDUs (pe, 1, NULLIP, NULLVP, NULLCP);
#endif

    if (pe2ps (ps, pe) == NOTOK) {
	result = smuxlose (youLoseBig, NULLCP, "pe2ps: %s",
			   ps_error (ps -> ps_errno));
	goto out;
    }

    result = OK;

out: ;
    if (pe)
	pe_free (pe);

    if (result == NOTOK) {
	ps_free (ps), ps = NULLPS;
	(void) close_tcp_socket (sd);
	return (sd = NOTOK);
    }

    return OK;
}

/*    CLOSE */

int
smux_close (reason)
int	reason;
{
    int	    result;
    struct type_SMUX_PDUs pdu;
    register struct type_SMUX_ClosePDU *close;

    if (ps == NULLPS)
	return smuxlose (invalidOperation, NULLCP, MSGSTR(MS_SMUX, SMUX_5,
			"%s: SMUX not opened"), "smux_close");

    bzero ((char *) &pdu, sizeof pdu);

    if ((close = (struct type_SMUX_ClosePDU *) calloc (1, sizeof *close))
	    == NULL) {
        result = smuxlose (congestion, NULLCP, MSGSTR(MS_GENERAL, GENERA_1,
				"%s: out of memory"), "smux_close");
	if (close)
	    free_SMUX_ClosePDU (close);

	ps_free (ps), ps = NULLPS;
	(void) close_tcp_socket (sd);
	return (sd = NOTOK);
    }
    pdu.offset = type_SMUX_PDUs_close;
    pdu.un.close = close;

    close -> parm = reason;

    result = smuxsend (&pdu);

    free_SMUX_ClosePDU (close);

    ps_free (ps), ps = NULLPS;
    shutdown(sd, 2);
    (void) close_tcp_socket (sd);
    sd = NOTOK;

    if (smux_pe)
	pe_free (smux_pe), smux_pe = NULL;
    if (smux_pdu)
	free_SMUX_PDUs (smux_pdu), smux_pdu = NULL;
    if (smux_enterprise)
	oid_free (smux_enterprise), smux_enterprise = NULL;
    if (smux_addr)
	free_SMI_NetworkAddress (smux_addr), smux_addr = NULL;

    return result;
}

/*    REGISTER */

int
smux_register (subtree, priority, operation)
OID	subtree;
int	priority,
    	operation;
{
    int	    result;
    struct type_SMUX_PDUs pdu;
    register struct type_SMUX_RReqPDU *rreq;

    if (subtree == NULL)
	return smuxlose (parameterMissing, NULLCP, MSGSTR(MS_SMUX, SMUX_3,
			"%s: missing parameter"), "smux_register");

    if (ps == NULLPS)
	return smuxlose (invalidOperation, NULLCP, MSGSTR(MS_SMUX, SMUX_5,
			"%s: SMUX not opened"), "smux_register");

    bzero ((char *) &pdu, sizeof pdu);

    if ((rreq = (struct type_SMUX_RReqPDU *) calloc (1, sizeof *rreq))
	== NULL) {
no_mem: ;
        result = smuxlose (congestion, NULLCP, MSGSTR(MS_GENERAL, GENERA_1,
			"%s: out of memory"), "smux_register");
	if (rreq)
	    free_SMUX_RReqPDU (rreq);

	ps_free (ps), ps = NULLPS;
	(void) close_tcp_socket (sd);
	return (sd = NOTOK);
    }
    pdu.offset = type_SMUX_PDUs_registerRequest;
    pdu.un.registerRequest = rreq;

    if ((rreq -> subtree = oid_cpy (subtree)) == NULLOID)
	goto no_mem;
    rreq -> priority = priority;
    rreq -> operation = operation;

    result = smuxsend (&pdu);

    free_SMUX_RReqPDU (rreq);

    return result;
}

/*    WAIT */

int
smux_wait (event, isecs)
struct type_SMUX_PDUs **event;
int	isecs;
{
    fd_set  mask;
    PE	    pe;
    struct timeval timeout; 
    struct timeval *secs = &timeout;

    /* initialize microseconds field to zero  (EJP) */
    bzero ((char *) &timeout, sizeof timeout);
    if (event == NULL)
	return smuxlose (parameterMissing, NULLCP, MSGSTR(MS_SMUX, SMUX_3,
			"%s: missing parameter"), "smux_wait");
	
    if (ps == NULLPS)
	return smuxlose (invalidOperation, NULLCP, MSGSTR(MS_SMUX, SMUX_5,
			"%s: SMUX not opened"), "smux_wait");

    FD_ZERO (&mask);
    FD_SET (sd, &mask);

    /* If no time limit was specified, wait for the next SMUX request to arrive.
       If a time limit was specified, wait only that long.  If a time limit of
       zero was specified, just check for requests that have already arrived.  (EJP) */

    if (isecs < 0)
	secs = (struct timeval *) NULL;
    else
	secs -> tv_sec = isecs;

    if (ps_prime (ps, 1) == OK
	    && select (sd + 1, &mask, NULLFD, NULLFD, secs) <= OK) {
	errno = EWOULDBLOCK;
	return smuxlose (inProgress, NULLCP, NULLCP);
    }

    if ((pe = ps2pe (ps)) == NULLPE) {
	(void) smuxlose (youLoseBig, NULLCP, "ps2pe: %s",
			 ps_error (ps -> ps_errno));
	goto out;
    }

    if (decode_SMUX_PDUs (pe, 1, NULLIP, NULLVP, event) == NOTOK) {
	(void) smuxlose (youLoseBig, NULLCP, "encode_SMUX_PDUs: %s",
			 PY_pepy);
	goto out;
    }

#ifdef	DEBUG
    if (smux_debug > 2)
	(void) print_SMUX_PDUs (pe, 1, NULLIP, NULLVP, NULLCP);
#endif

    if (smux_pe)
	pe_free (smux_pe);
    smux_pe = pe;
    if (smux_pdu)
	free_SMUX_PDUs (smux_pdu);
    smux_pdu = *event;

    if (smux_pdu -> offset == type_SMUX_PDUs_close) {
	ps_free (ps), ps = NULLPS;
	(void) close_tcp_socket (sd);
	sd = NOTOK;
    }
    return OK;

out: ;
    if (pe)
	pe_free (pe);

    ps_free (ps), ps = NULLPS;
    (void) close_tcp_socket (sd);
    return (sd = NOTOK);
}

/*    RESPONSE */

int
smux_response (event)
struct type_SNMP_GetResponse__PDU *event;
{
    struct type_SMUX_PDUs pdu;

    if (event == NULL)
	return smuxlose (parameterMissing, NULLCP, MSGSTR(MS_SMUX, SMUX_3,
			"%s: missing parameter"), "smux_response");

    if (ps == NULLPS)
	return smuxlose (invalidOperation, NULLCP, MSGSTR(MS_SMUX, SMUX_5,
			"%s: SMUX not opened"), "smux_response");

    bzero ((char *) &pdu, sizeof pdu);

    pdu.offset = type_SMUX_PDUs_get__response;
    pdu.un.get__response = event;

    return smuxsend (&pdu);
}

/*    TRAP */

int
smux_trap (generic, specific, bindings)
int	generic,
	specific;
struct type_SNMP_VarBindList *bindings;
{
    int	    result;
    struct timeval now;
    struct type_SMUX_PDUs pdu;
    register struct type_SNMP_Trap__PDU *trap;

    if (ps == NULLPS)
	return smuxlose (invalidOperation, NULLCP, MSGSTR(MS_SMUX, SMUX_5,
			"%s: SMUX not opened"), "smux_trap");

    bzero ((char *) &pdu, sizeof pdu);

    if ((trap = (struct type_SNMP_Trap__PDU *) calloc (1, sizeof *trap))
	    == NULL) {
        result = smuxlose (congestion, NULLCP, MSGSTR(MS_GENERAL, GENERA_1,
			"%s: out of memory"), "smux_trap");
	if (trap)
	    free_SNMP_Trap__PDU (trap);

	ps_free (ps), ps = NULLPS;
	(void) close_tcp_socket (sd);
	return (sd = NOTOK);
    }
    pdu.offset = type_SMUX_PDUs_trap;
    pdu.un.trap = trap;

    trap -> enterprise = smux_enterprise;
    trap -> agent__addr = smux_addr;
    trap -> generic__trap = generic;
    trap -> specific__trap = specific;
    trap -> time__stamp = smux_stamp;
    (void) gettimeofday (&now, (struct timezone *) 0);
    trap -> time__stamp -> parm = (now.tv_sec - my_boottime.tv_sec) * 100
	    				+ ((now.tv_usec - my_boottime.tv_usec)
								      / 10000);
    trap -> variable__bindings = bindings;

    result = smuxsend (&pdu);

    trap -> enterprise = NULL;
    trap -> agent__addr = NULL;
    trap -> time__stamp = NULL;
    trap -> variable__bindings = NULL;

    free_SNMP_Trap__PDU (trap);

    return result;
}

/*    LOSE */

#ifndef	lint
static int
smuxlose (va_alist)
va_dcl
{
    va_list ap;

    va_start (ap);

    smux_errno = va_arg (ap, int);

    asprintf (smux_info, ap);

    va_end (ap);

    return NOTOK;
}
#else
/* VARARGS3 */

static int
smuxlose (reason, what, fmt)
int	reason;
char   *what,
       *fmt;
{
    return smuxlose (reason, what, fmt);
}
#endif

/*  */

static char *errors_up[] = {
    "goingDown",
    "unsupportedVersion",
    "packetFormat",
    "protocolError",
    "internalError",
    "authenticationFailure"
};

static char *errors_down[] = {
    "SMUX error 0",
    "invalidOperation",
    "parameterMissing",
    "systemError",
    "youLoseBig",
    "congestion",
    "inProgress"
};

char   *
smux_error (i)
integer	i;
{
    int	    j;
    char  **ap;
    static char buffer[BUFSIZ];

    if (i < 0) {
	ap = errors_down, j = sizeof errors_down / sizeof errors_down[0];
	i = -i;
    }
    else
	ap = errors_up, j = sizeof errors_up / sizeof errors_up[0];
    if (0 <= i && i < j)
	return ap[i];

    (void) sprintf (buffer, MSGSTR(MS_SMUX, SMUX_6, "SMUX error %s%d"), 
		    ap == errors_down ? "-" : "",i);

    return buffer;
}

void smux_free_child(op)
OT op;
{
	OT		k = op->ot_children, sk;

	while (k)
	{
		if (k->ot_children)
		{
			smux_free_child(k);
		}
		sk = k->ot_sibling;
		object_remove(k);
		k = sk;
	}
}

void smux_free_tree(parent, self)
char *parent;
char *self;
{
	OT		k, dk;

	if (!self)
		return;
	k = text2obj(self);

        if (!parent)
		return;
	dk = text2obj(parent);

	if (k)
	{
		if (dk->ot_children == k)
		{
			dk->ot_children = k->ot_sibling;
		}
		else
		{
			dk = dk->ot_children;
			while (dk)
			{
				if (dk->ot_sibling == k)
				{
					dk->ot_sibling = k->ot_sibling;
					break;
				}
				else 
					dk = dk->ot_sibling;
			}
		}
		smux_free_child(k);
		object_remove(k);
	}
}

