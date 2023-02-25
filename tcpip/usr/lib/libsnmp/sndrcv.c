static char sccsid[] = "@(#)66	1.11  src/tcpip/usr/lib/libsnmp/sndrcv.c, snmp, tcpip411, 9438B411a 9/20/94 14:59:40";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: sndrcv_msg(), snd_msg(), snd_pe(), rcv_pe(), rcv_msg()
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
 * FILE:	src/tcpip/usr/lib/libsnmp/sndrcv.c
 */

#include <isode/pepsy/SNMP-types.h>
#include <isode/snmp/io.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"

extern errno;
extern int max_SNMP_retries;
extern int SNMP_timeout;
extern int last_SNMP;
extern int debug;		/* in objects.c */
extern jmp_buf Sjbuf;		/* in alarm.c */

extern int alarmtr ();

int timedout;

#define	ADVISE		if (o_advise) (*o_advise)

int  
sndrcv_msg (omsg)
struct type_SNMP_Message **omsg;
{
    struct type_SNMP_Message *msg = *omsg;
    int	    request_id, status = OK, tries = 0, send_again;
    PE	    pe;
    struct fdinfo *fi;
    register struct type_SNMP_PDU *parm;

    if (msg == NULL)
	return OK;

    /* NOTE: move this outbound? */
    if ((fi = type2fi (FI_UDP, (struct fdinfo *) NULL)) 
	    == (struct fdinfo *) NULL) {
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_SNDRCV, SNDRCV_1, 
					"no active UDP port"));
	return NOTOK;
    }

    request_id = (parm = msg -> data -> un.get__request) -> request__id;

    if ((status = encode_SNMP_Message (&pe, 1, 0, NULLCP, msg)) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, "encode_SNMP_Message: %s", PY_pepy);
	goto out;
    }

    if (debug > 2)
	(void) print_SNMP_Message (pe, 1, NULLIP, NULLVP, NULLCP);

    send_again = 1;
    while (tries < max_SNMP_retries) {

	/* send SNMP message */
	if (send_again)
	    if ((status = snd_pe (pe, fi -> fi_ps, NULL)) != OK)
		break;

	free_SNMP_Message (*omsg);
	*omsg = msg = NULL;

	/* receive SNMP message */
	if ((status = rcv_msg (omsg, fi -> fi_ps)) != OK) {
	    if (timedout) {
		tries++;
		send_again++;
		continue;
	    }
	    break;
	}
	msg = *omsg;

	if ((last_SNMP = (parm = msg -> data -> un.get__response) 
		-> request__id) == request_id) 
	    break;
	ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_SNDRCV, SNDRCV_2,
		"request-id mismatch (received %d, expected %d)"),
		parm -> request__id, request_id);
	send_again = 0;		/* just hang in receive */
    } 

    if (tries == max_SNMP_retries) {
	ADVISE (SLOG_NOTICE, MSGSTR(MS_SNDRCV, SNDRCV_3, "No response"));
	status = DONE;
    }
out:;
    if (pe)
	pe_free (pe);
    if (status == NOTOK && msg) {
	free_SNMP_Message (msg);
	*omsg = msg = NULL;
    }
    return status;
}

int
snd_msg (msg, ps)
struct type_SNMP_Message *msg;
PS ps;
{
    int status = OK;
    PE	    pe;

    if ((status = encode_SNMP_Message (&pe, 1, 0, NULLCP, msg)) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, "encode_SNMP_Message: %s", PY_pepy);
	status = NOTOK;
	goto out;
    }

    if (debug > 2)
	(void) print_SNMP_Message (pe, 1, NULLIP, NULLVP, NULLCP);

    status = snd_pe (pe, ps, NULL);
out:;
    if (pe)
	pe_free (pe);
    return status;
}

int
snd_pe (pe, ps, info)
PE	pe;
PS	ps;
char	*info;		/* additional info to print upon error */
{
    /* send SNMP message */
    if (pe2ps (ps, pe) == NOTOK) {
	ADVISE (SLOG_EXCEPTIONS, "pe2ps: %s: %s%s", 
		ps_error (ps -> ps_errno), strerror (errno),
		info == NULL ? "" : info);
	return NOTOK;
    }
    return OK;
}

PE 
rcv_pe (ps, info)
PS	ps;
char	*info;		/* additional info to print upon error */
{
    PE pe = NULLPE;
    unsigned int oldsigmask = 0;
    unsigned int oldalarm = 0;
    void (*oldptr)() = NULL;

    /* set a place for longjump to go to */
    timedout = 0;
    if (setjmp (Sjbuf)) {
#ifdef _ANSI_C_SOURCE 
	signal (SIGALRM, (void (*)(int))oldptr);
#else /* !_ANSI_C_SOURCE */
	signal (SIGALRM, oldptr);
#endif /* _ANSI_C_SOURCE */
	sigsetmask(oldsigmask);
	alarm(oldalarm);

	timedout++;
	if (debug)
	    ADVISE (SLOG_EXCEPTIONS, MSGSTR(MS_SNDRCV, SNDRCV_4,
		    "receive timed out%s"), 
		    info == NULL ? "" : info);
	return NULLPE;
    }

    if (SNMP_timeout) {
	oldsigmask = sigsetmask(~sigmask(SIGALRM));
	sigsetmask(oldsigmask & ~sigmask(SIGALRM));
	oldalarm = alarm(0);
#ifdef _ANSI_C_SOURCE 
	oldptr = signal (SIGALRM, (void (*)(int))alarmtr);
#else /* !_ANSI_C_SOURCE */
	oldptr = signal (SIGALRM, alarmtr);
#endif /* _ANSI_C_SOURCE */
	alarm (SNMP_timeout);
    }

    /* receive SNMP message */
    if ((pe = ps2pe (ps)) == NULLPE && ps -> ps_errno) 
	ADVISE (SLOG_EXCEPTIONS, "ps2pe: %s: %s%s", 
		ps_error (ps -> ps_errno), strerror (errno),
		info == NULL ? "" : info);

    if (SNMP_timeout)
    {
	alarm (0);
	sigsetmask(oldsigmask);
#ifdef _ANSI_C_SOURCE 
	signal (SIGALRM, (void (*)(int))oldptr);
#else /* !_ANSI_C_SOURCE */
	signal (SIGALRM, oldptr);
#endif /* _ANSI_C_SOURCE */
	alarm(oldalarm);
    }

    return pe;
}

int
rcv_msg (msg, ps)
struct type_SNMP_Message **msg;
PS ps;
{
    int status = OK;
    PE	    pe;

    /* receive SNMP message */
    if ((pe = rcv_pe (ps, NULL)) == NULLPE)
	return NOTOK;

    if ((status = decode_SNMP_Message (pe, 1, NULLIP, NULLVP, msg)) == 
	    NOTOK){
	ADVISE (SLOG_EXCEPTIONS, "decode_SNMP_Message: %s", PY_pepy);
	status = NOTOK;
    }

    if (debug > 2)
	(void) print_SNMP_Message (pe, 1, NULLIP, NULLVP, NULLCP);

    if (pe)
	pe_free (pe);
    return status;
}
