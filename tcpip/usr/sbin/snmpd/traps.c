static char sccsid[] = "@(#)19	1.5.1.5  src/tcpip/usr/sbin/snmpd/traps.c, snmp, tcpip411, 9434A411a 8/18/94 14:32:57";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: start_trap(), do_trap(), trap_notify()
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/traps.c
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

#include <isode/snmp/io.h>
#include "snmpd.h"
#include "view.h"
#include <sys/param.h>		/* to get  MAXHOSTNAMELEN */

extern struct snmpstat snmpstat;	/* defined in snmp.c */

static	int	udp = NOTOK;
static struct trap trapque;
static char	*trapType[] = {
    "coldStart",
    "warmStart",
    "linkDown",
    "linkUp",
    "authenticationFailure",
    "egpNeighborLoss",
    "enterpriseSpecific"
};

struct trap *UHead = &trapque ;
struct type_SNMP_Message *trap = NULL;
struct qbuf *loopback_addr = NULL;

/*    TRAPS */

void	
start_trap () 
{
    char    myhost[MAXHOSTNAMELEN+1];
    register struct hostent *hp;
    struct type_SNMP_Message *msg;
    register struct type_SNMP_PDUs *pdu;
    register struct type_SNMP_Trap__PDU *parm;

    /* 
     * Initializes struct type_SNMP_Message, trap, for sending trap messages.
     */
    if ((msg = (struct type_SNMP_Message *) calloc (1, sizeof *msg)) == NULL) {
no_mem: ;
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_1,
		"cannot initialize trap structure: out of memory"));
out: ;
	if (msg)
	    free_SNMP_Message (msg);

	return;
    }
    msg -> version = int_SNMP_version_version__1;

    if ((pdu = (struct type_SNMP_PDUs *) calloc (1, sizeof *pdu)) == NULL)
	goto no_mem;
    msg -> data = pdu;

    pdu -> offset = type_SNMP_PDUs_trap;

    if ((parm = (struct type_SNMP_Trap__PDU *) calloc (1, sizeof *parm))
	    == NULL)
	goto no_mem;
    pdu -> un.trap = parm;

    /* 
     * Fills in agent_addr with the local host name.
     */

    if (gethostname (myhost, MAXHOSTNAMELEN) == NOTOK) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_2,
		"cannot get host name, so no traps"));
	goto out;
    }
    if (hp = (struct hostent *)gethostbyname (myhost)) {
	struct sockaddr_in sin;

	bcopy (hp -> h_addr, (char *) &(sin.sin_addr), hp -> h_length);
	if ((parm -> agent__addr = str2qb ((char *) &sin.sin_addr, 4, 1))
	        == NULL)
	    goto no_mem;
    }
    else {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_3,
		"%s: unknown host, so no traps"), myhost);
	goto out;
    }

    if ((parm -> time__stamp = (struct type_SMI_TimeTicks *)
	 	calloc (1, sizeof *parm -> time__stamp)) == NULL)
	goto no_mem;

    trap = msg;

#ifdef	SMUX

    /* 
     * gets the loopback address for SMUX trap
     */

    if (hp = (struct hostent *)gethostbyname ("localhost")) {
	struct sockaddr_in sin;

	bcopy (hp -> h_addr, (char *) &(sin.sin_addr), hp -> h_length);
	if ((loopback_addr = str2qb ((char *) &sin.sin_addr, 4, 1)) == NULL)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_4,
		    "cannot initialize loopback address: out of memory"));
    }
#endif
}

/*  */

do_trap (generic, specific, bindings)
int	generic,
	specific;
struct type_SNMP_VarBindList *bindings;
{
    struct type_SNMP_Message *msg;
    register struct type_SNMP_Trap__PDU *parm;
    OT	    ot;

    if ((msg = trap) == NULL)
	return;
    parm = msg -> data -> un.trap;

    /* 
     * Fills in enterprise with the type of object generating the trap 
     *  (sysObjectID).     						
     */ 

    if ((ot = text2obj ("sysObjectID")) == NULLOT) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_5,
		"cannot send trap: no such object: \"%s\""),
		"sysObjectID");
	return;
    }
    if ((parm -> enterprise = (OID) ot -> ot_info) == NULLOID) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_6,
		"cannot send trap: no value defined for object \"%s\""),
		"sysObjectID");
	return;
    }

    /* Fills in trap type */
    parm -> generic__trap = generic;
    parm -> specific__trap = specific;

    /* 
     * Fills time_stamp with time elapsed between the last (re)initialization
     * of the agent and the generation of the trap. 			
     */

    {
	struct timeval now;

	if (gettimeofday (&now, (struct timezone *) 0) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_TRAP, TRAP_7, "gettimeofday"));
	    return;
	}

	parm -> time__stamp -> parm = (now.tv_sec - my_boottime.tv_sec) * 100
	    				+ ((now.tv_usec - my_boottime.tv_usec)
								      / 10000);
    }
    parm -> variable__bindings = bindings;

    trap_notify (msg, (integer) generic, (integer) specific);
}


/*  */
trap_notify (msg, generic, specific)
register struct type_SNMP_Message *msg;
integer	generic,
	specific;
{
    int	    mask = 1 << 7 - generic;
    register struct trap *t;
    struct fdinfo *fi;
    struct sockaddr_in *s;

    /* 
     * get the fdinfo pointer for udp port, and save the old address
     */

    fi = fd2fi(Nd);
    s = fi -> sockaddr_in;

    /* 
     * Sends trap to each host defined in config file.
     */

    for (t = UHead -> t_forw; t != UHead; t = t -> t_forw) {
	register struct view *v = t -> t_view;
	PE	pe;

	if (specific == 0 && !(t -> t_generics & mask)) {
#ifdef DEBUG
	    /* NOT IN CATALOG, BECAUSE DEBUG STATEMENT */
	    if (debug)
		advise (SLOG_DEBUG,
			"trap blocked by t_generics %d and mask %d",
			t -> t_generics, mask);
#endif
	    continue;
	}

	/* 
	 * assign the destination IP address and udp port to be 162
	 */

        fi -> sockaddr_in = (struct sockaddr_in *) &(v -> v_sa);

	/* 
	 * assign the community name 
	 */

	msg -> community =  t -> t_view -> v_community;

	pe = NULLPE;
	if (encode_SNMP_Message (&pe, 1, 0, NULLCP, msg) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, "encode_SNMP_Message: %s", PY_pepy);
	    if (pe)
		pe_free (pe);
	    continue;
	}

        if (debug > 2) {	/* need only a PE to print */
	    advise (SLOG_TRACE, "");
	    print_SNMP_Message (pe, 1, NULLIP, NULLVP, NULLCP);
	}

	/* 
	 * sends the trap
	 */

	if (pe2ps (fi -> fi_ps, pe) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, "pe2ps: %s: %s (%s)",
		    ps_error (fi -> fi_ps -> ps_errno), strerror (errno),
		    inet_ntoa (fi -> sockaddr_in -> sin_addr));
	    
	    /* flush output buffers */
	    (void) dg_flushb (fi -> fi_ps);
	}
	else {
	    snmpstat.s_outpkts++; 
	    snmpstat.s_outtraps++;
	    if (debug) {
		if (generic == int_SNMP_generic__trap_enterpriseSpecific)
		    advise (SLOG_DEBUG, MSGSTR(MS_TRAP, TRAP_8,
			    "sent trap (%s:%d) to %s"), 
			    trapType[generic], specific,
			    inet_ntoa(fi -> sockaddr_in -> sin_addr.s_addr));
		else
		    advise (SLOG_DEBUG, MSGSTR(MS_TRAP, TRAP_9,
			    "sent trap (%s) to %s"), 
			    trapType[generic], 
			    inet_ntoa(fi -> sockaddr_in -> sin_addr.s_addr));
	    }
	}
	/* 
	 * store the old address back 
	 */
        fi -> sockaddr_in = s;
	pe_free (pe);
    }
}
