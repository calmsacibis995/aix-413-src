static char sccsid[] = "@(#)11	1.7.1.13  src/tcpip/usr/sbin/snmpd/smuxserv.c, snmp, tcpip411, 9434A411a 8/18/94 14:32:15";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: init_smuxio(), start_smux(), doit_smux(), smux_process(), 
 *            smux_getfnx(), pb_free(), tb_free(), init_reserved(),
 *            smux_process_unexpected(), smux_register(), 
 *            smux_register_response(), smux_method(), smux_client_auth()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/smuxserv.c
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

#ifdef	SMUX

#include <isode/snmp/io.h>
#include "snmpd.h"
#include "smux_g.h"
#include <isode/snmp/smux.h>

extern  struct type_SNMP_Message *trap;
extern  struct snmpstat snmpstat;		/* defined in snmp.c */
extern	int SNMP_timeout;			/* defined in global.c */
extern	int smuxtimeout;			/* defined in config.c */
extern	int smuxtrapaddr;			/* defined in config.c */

extern  struct qbuf *loopback_addr;

extern  PE  rcv_pe ();

static	int	smux_enabled = 1;
static	int	smux = NOTOK;

static struct smuxReserved {
    char   *rb_text;
    OID	    rb_name;
}    reserved[] = {
    "snmp", NULLOID,
    "view", NULLOID,
    "smux", NULLOID,

    NULL
};


int
init_smuxio (uport)
int	uport;
{
    register struct servent *sp;
    struct fdinfo *fi;

    /* open and initialize smux tcp port */
    if ((fi = addfd()) == NULL) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_1, "addfd failed"));
	return NOTOK;
    }
    fi -> sockaddr_in -> sin_family = AF_INET;
    
    fi -> sockaddr_in -> sin_port = uport ? htons (uport) : 
	    ((sp = getservbyname ("smux", "tcp")) ? sp -> s_port
	    : htons ((u_short) 199));

    if ((fi -> fi_fd = start_tcp_server (fi -> sockaddr_in, SOMAXCONN, 0, 0)) 
	    == NOTOK) {
	remfi (fi);
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_2,
		"start_tcp_server failed for SMUX: %s"), strerror(errno));
	return NOTOK;
    }
    fi -> fi_flags = FI_SMUX;
    if (debug)
	advise (SLOG_DEBUG, MSGSTR(MS_SMUXS, SMUXS_3,
		"bound SMUX fd %d to port %d"), fi -> fi_fd, 
		ntohs (fi -> sockaddr_in -> sin_port));
    return fi -> fi_fd;
}

/* initialize reserved subtrees. */
void
init_reserved ()
{
    register struct smuxReserved *sr;
    OT	ot;

    for (sr = reserved; sr -> rb_text; sr++)
	if (ot = text2obj (sr -> rb_text))
	    sr -> rb_name = ot -> ot_name;
}

/* start a new smux client relationship */
int  
start_smux (smuxfi)
struct fdinfo *smuxfi;
{
    struct fdinfo *fi;
    register struct smuxPeer *pb;
    static int smux_peerno = 0;
    char pb_source[30];

    /* open and initialize smux client tcp port */
    if ((fi = addfd()) == NULL) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_1, "addfd failed"));
	return NOTOK;
    }
    if ((fi -> fi_fd = join_tcp_client (smuxfi -> fi_fd, fi -> sockaddr_in)) 
	    == NOTOK) {
	if (errno == EWOULDBLOCK)
	    return NOTOK;
	adios ("join_tcp_client: %s", strerror(errno));
    }

    if ((pb = (struct smuxPeer *) calloc (1, sizeof *pb)) == NULL) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_GENERAL, GENERA_1, 
		"%s: Out of Memory"), "doit_smux");
	(void) close_tcp_socket (fi -> fi_fd);
	fi -> fi_fd = NOTOK;
	return NOTOK;
    }
    pb -> pb_fi = fi;			/* fi and pb point at each other */
    fi -> fi_info = (caddr_t) pb;	/* ... */
    fi -> fi_flags = FI_SMUX_CLIENT;

    (void) sprintf (pb_source, "%s+%d",
		    inet_ntoa (fi -> sockaddr_in -> sin_addr),
		    (int) ntohs (fi -> sockaddr_in -> sin_port));
    
    if ((fi -> fi_ps = ps_alloc (fdx_open)) == NULLPS
	    || fdx_setup (fi -> fi_ps, fi -> fi_fd) == NOTOK) {
	if (fi -> fi_ps == NULLPS)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_4,
		    "%s: out of memory (SMUX %s)"), "ps_alloc", pb_source);
	else
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_5,
		    "fdx_setup: %s (SMUX %s)"),
		    ps_error (fi -> fi_ps -> ps_errno), pb_source);

	(void) close_tcp_socket (fi -> fi_fd);
	pb_free (pb);
	return NOTOK;
    }

    /* Insert new smuxPeer structure at the end of the doubly-linked
       list anchored at PHead and assign it the next sequential index
       number for use as smuxPindex in o_smuxPeer() and smuxTindex in
       o_smuxTree().  (EJP) */

    insque (pb, PHead -> pb_back);
    pb -> pb_index = ++smux_peerno;

    /*
     * tack the peer index onto the end of the source identity
     * for easy mapping of peer activity to peerBlock.
     */
    (void) sprintf (pb -> pb_source, "%s+%d", pb_source, pb -> pb_index);

    advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_6, 
	    "SMUX relation started with (%s)"),
	    pb -> pb_source);
    return (fi -> fi_fd);
}

int  
doit_smux (fi)
struct fdinfo	*fi;
{
    PE	    pe;
    register struct smuxPeer *pb;
    struct type_SMUX_PDUs *pdu;
    char    pb_source[60];
    int	    status;

    if ((pb = (struct smuxPeer *) fi -> fi_info) == (struct smuxPeer *) NULL) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_7,
		"lost smuxPeer block for %d"), fi -> fi_fd);

	return NOTOK;
    }

    sprintf (pb_source, " (SMUX %s)", pb -> pb_source);
    SNMP_timeout = smuxtimeout;		/* set timeout in libsnmp */
    if ((pe = rcv_pe (pb -> pb_fi -> fi_ps, pb_source)) == NULLPE) {
	if (/* errno == ENOENT && */ fi -> fi_ps -> ps_errno == 0)
	    advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_8, 
		    "lost peer%s"), pb_source);

	if (pe)
	    pe_free (pe);
	pb_free (pb);
	return NOTOK;
    }

    advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_9,
		"SMUX packet from (%s)"), pb -> pb_source);

    pdu = NULL;

    if (decode_SMUX_PDUs (pe, 1, NULLIP, NULLVP, &pdu) == NOTOK) {
	advise (SLOG_EXCEPTIONS, 
		"decode_SMUX_PDUs: %s%s", PY_pepy, pb_source);

	if (pe)
	    pe_free (pe);
	pb_free (pb);
	return NOTOK;
    }

    if (debug > 2)
	print_SMUX_PDUs (pe, 1, NULLIP, NULLVP, NULLCP);

    if ((status = smux_process (pb, pdu)) == NOTOK)
	pb_free (pb);
    
    if (pdu)
	free_SMUX_PDUs (pdu);
    if (pe)
	pe_free (pe);
    return status;
}


/*  */

static	
smux_process (pb, pdu)
register struct smuxPeer *pb;
struct type_SMUX_PDUs *pdu;
{
    int	    result = OK;
    char    *missing = MSGSTR(MS_SMUXS, SMUXS_31, ", missing identity ");

    switch (pdu -> offset) {
	case type_SMUX_PDUs_simple:
	    if (pb -> pb_identity)
		return smux_process_unexpected (pdu -> offset, 
			MSGSTR(MS_SMUXS,SMUXS_32,", identity already exists "),
			pb -> pb_source);
	    {
		register struct type_SMUX_SimpleOpen *simple =
							    pdu -> un.simple;

		if (simple -> version != int_SMUX_version_version__1) {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_19,
			    "bad version: %d (SMUX %s)"),
			    simple -> version, pb -> pb_source);
		    return NOTOK;
		}

		pb -> pb_identity = simple -> identity;
		simple -> identity = NULL;

		if ((pb -> pb_description = qb2str (simple -> description))
		        == NULL) {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_4,
			    "%s: out of memory (SMUX %s)"), 
			    "qb2str", pb -> pb_source);
		    return NOTOK;
		}

		if (qb_pullup (simple -> password) == NOTOK) {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_4,
			    "%s: out of memory (SMUX %s)"), "qb_pullup",
			    pb -> pb_source);
		    return NOTOK;
		}

		/* should we allow this client to open a connection? */
		if (smux_client_auth (pb, simple -> password) == NOTOK) {
		    if (snmpstat.s_enableauthentraps == TRAPS_ENABLED)
			do_trap (int_SNMP_generic__trap_authenticationFailure,
				 0, (struct type_SNMP_VarBindList *) 0);
		    return NOTOK;
		}
		pb -> pb_priority = 0;

		advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_20,
			"SMUX open: %d %s \"%s\" (%d/ %s)"),
			pb -> pb_index, oid2ode (pb -> pb_identity),
			pb -> pb_description, pb -> pb_fi -> fi_fd,
			pb -> pb_source);
	    }
	    break;

	case type_SMUX_PDUs_close:
	    if (!pb -> pb_identity)
		return smux_process_unexpected (pdu -> offset, 
			missing, pb -> pb_source);
	    advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_21,
		    "SMUX close: %s (%s)"),
		    smux_error (pdu -> un.close -> parm), pb -> pb_source);
	    return NOTOK;

	case type_SMUX_PDUs_registerRequest:
	    if (!pb -> pb_identity)
		return smux_process_unexpected (pdu -> offset, 
			missing, pb -> pb_source);

	    result = smux_register (pb, pdu);
	    break;

	case type_SMUX_PDUs_trap:
	    if (!pb -> pb_identity)
		return smux_process_unexpected (pdu -> offset, 
			missing, pb -> pb_source);
	    {
		struct type_SNMP_Message msgs;
		register struct type_SNMP_Message *msg = &msgs;
		struct type_SNMP_PDUs datas;
		register struct type_SNMP_PDUs *data = &datas;
		register struct type_SNMP_Trap__PDU *parm = pdu -> un.trap;
		struct qbuf *qb = parm -> agent__addr;

		advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_22, 
			"SMUX trap: (%d %d) (%s)"),
			parm -> generic__trap, parm -> specific__trap, 
			pb -> pb_source);

		bzero ((char *) msg, sizeof *msg);
		msg -> version = int_SNMP_version_version__1;
		msg -> data = data;

		bzero ((char *) data, sizeof *data);
		data -> offset = type_SNMP_PDUs_trap;
		data -> un.trap = parm;

                /*
		 * overwrite the smux trap address with the snmp agent
		 * address IFF this trap arrived via loopback, or if
		 * we are explicitly configured to do so.
		 */
		if (smuxtrapaddr ||
			(loopback_addr
			&& qb_pullup (qb) != NOTOK
		        && qb -> qb_len == loopback_addr -> qb_len
		        && bcmp (qb -> qb_forw -> qb_data,
				 loopback_addr -> qb_forw -> qb_data,
				 qb -> qb_len) == 0))
		    parm -> agent__addr = trap -> data -> un.trap->agent__addr;
		trap_notify (msg, parm -> generic__trap, parm->specific__trap);
		parm -> agent__addr = qb;
	    }
	    break;

	case type_SMUX_PDUs_registerResponse:
	case type_SMUX_PDUs_get__request:
	case type_SMUX_PDUs_get__next__request:
	case type_SMUX_PDUs_get__response:
	case type_SMUX_PDUs_set__request:
	    return smux_process_unexpected (pdu -> offset, " ", 
		    pb -> pb_source);

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_23, 
		    "bad operation: %d (SMUX %s)"), pdu -> offset, 
		    pb -> pb_source);
	    return NOTOK;
    }

    return result;
}

static int
smux_process_unexpected (offset, info, source)
int	offset;
char	*source,
	*info;
{
    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_24,
	    "unexpected operation: %d%s(SMUX %s)"), offset, info, source);
    return NOTOK;
}

/*  */

static int
smux_register (pb, pdu)
register struct smuxPeer *pb;
struct type_SMUX_PDUs *pdu;
{
    int	    result = OK;
    register struct type_SMUX_RReqPDU *rreq = pdu -> un.registerRequest;
    register struct smuxReserved *sr;
    register struct smuxTree *tb = NULL;
    register struct smuxTree *qb;
    register OID	oid = rreq -> subtree;
    OT	ot = NULLOT;
    int found = 0;

    /*
     *	check reserved tree first...
     */
    for (sr = reserved; sr -> rb_text; sr++)
	if (sr -> rb_name
		&& bcmp ((char *) sr -> rb_name -> oid_elements,
			 (char *) oid -> oid_elements,
			 (sr -> rb_name -> oid_nelem
			     <= oid -> oid_nelem
				    ? sr -> rb_name -> oid_nelem
				    : oid -> oid_nelem)
			     * sizeof oid -> oid_elements[0])
			== 0) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_25,
		    "reserved subtree: %s %s %s (SMUX %s)"),
		    oid2ode (oid),
		    sr -> rb_name -> oid_nelem
			<= oid -> oid_nelem
			    ? MSGSTR(MS_SMUXS, SMUXS_10, "under") : 
			      MSGSTR(MS_SMUXS, SMUXS_11, "contains"),
		    sr -> rb_text, pb -> pb_source);
	    return smux_register_response (pb, int_SMUX_RRspPDU_failure);
	}

    /* 
     *	lookup objectType 
     */
    if ((ot = name2obj (oid)) == NULLOT ||
        ot -> ot_name -> oid_nelem < oid -> oid_nelem) { /* part match (rpe) */

	if (rreq -> operation == int_SMUX_operation_delete) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_26,
		    "no such subtree: %s (SMUX %s)"),
		    oid2ode (oid), pb -> pb_source);
	    return smux_register_response (pb, int_SMUX_RRspPDU_failure);
	}
	
	/*
	 *	add new object type.
	 */
	if ((ot = (OT) calloc (1, sizeof *ot)) == NULL
		|| (ot -> ot_text = ot -> ot_id =
			    strdup (sprintoid (oid)))
		    == NULL) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_4,
		    "%s: out of memory (SMUX %s)"), "smux_register", 
		    pb -> pb_source);
	    if (ot)
		free ((char *) ot);
	    return NOTOK;
	}
	ot -> ot_name = rreq -> subtree;
	rreq -> subtree = NULL;
	ot -> ot_access = rreq -> operation;
	ot -> ot_status = OT_OPTIONAL;
	export_view (ot);

	(void) add_objects (ot);
	found = 0;                           /* Not in OT list to begin with */
    }
    else {

	/* 
	 * delete operation 
	 */
	if (rreq -> operation == int_SMUX_operation_delete) {
	    int priority = int_SMUX_RRspPDU_failure;

	    for (tb = (struct smuxTree *) ot -> ot_smux;
		     tb;
		     tb = tb -> tb_next)
		if (tb -> tb_peer == pb
			&& (rreq -> priority < 0
				|| rreq -> priority
					    == tb -> tb_priority))
		    break;
	    if (tb) {
		priority = tb -> tb_priority;
		tb_free (tb);
	    }
	    else {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_27,
			"no such registration: %s (SMUX %s)"),
			oid2ode (oid), pb -> pb_source);
		ot = NULLOT;
	    }
	    result = smux_register_response (pb, priority);
	    if (result != NOTOK && priority != int_SMUX_RRspPDU_failure)
		advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_28,
			"SMUX register: %s %s in = %d out = %d (%s)"),
			MSGSTR(MS_SMUXS, SMUXS_12, "delete"), 
			oid2ode (ot -> ot_name),
			rreq -> priority, priority, pb -> pb_source);
	    return result;
	}

	if (ot -> ot_name -> oid_nelem > oid -> oid_nelem) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_29,
		    "bad subtree: %s (SMUX %s)"),
		    oid2ode (oid), pb -> pb_source);
	    ot = NULL;
	    return smux_register_response (pb, int_SMUX_RRspPDU_failure);
	}

	/* 
	 * if this is an existing node, but no access
	 * has been defined for it yet, do that now.
	 */
	if (ot -> ot_access == OT_NONE)
	    ot -> ot_access = rreq -> operation;

	export_view (ot);
	found = 1;                   /* Found so check if tb already */
    }

    /*
     *	create and fill a new treeBlock 
     */
    if ((tb = (struct smuxTree *) calloc (1, sizeof *tb))
	    == NULL) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_4,
		"%s: out of memory (SMUX %s)"), "smux_register", 
		pb -> pb_source);
	return NOTOK;
    }

    if ((tb -> tb_priority = rreq -> priority) < pb -> pb_priority)
	tb -> tb_priority = pb -> pb_priority;
    for (qb = (struct smuxTree *) ot -> ot_smux; qb; qb = qb -> tb_next)
	if (qb -> tb_priority > tb -> tb_priority)
	    break;
	else
	    if (qb -> tb_priority == tb -> tb_priority)
		tb -> tb_priority++;

    tb -> tb_peer = pb;

    if (found == 0)
      tb -> tb_smux_init = SMUX_INITED;
    else if (ot -> ot_smux == NULL)
      tb -> tb_smux_init = SMUX_NOTINITED;
    else 
      tb -> tb_smux_init = ((struct smuxTree *)(ot -> ot_smux)) -> tb_smux_init;

    /*
     *	a registration request which has passed all checks so far.
     *	Send the response,  and finish the registration work.
     */
    if ((result = smux_register_response (pb, tb -> tb_priority)) != NOTOK) {
	register int    i;
	register unsigned int *ip,
			      *jp;
	register struct smuxTree **qpp;

	advise (SLOG_NOTICE, MSGSTR(MS_SMUXS, SMUXS_28,
		"SMUX register: %s %s in = %d out = %d (%s)"),
		rreq -> operation == int_SMUX_operation_readOnly
		    ? "readOnly" : "readWrite",
		oid2ode (ot -> ot_name),
		rreq -> priority, tb -> tb_priority, pb -> pb_source);

	tb -> tb_subtree = ot;

	/* Insert the new element into the single-linked chain
	   of smuxTree structures for this object type that is
	   anchored in the object tree.  Elements are chained
	   in order of increasing priority.  (EJP) */

	for (qpp = (struct smuxTree **) &ot -> ot_smux;
		 qb = *qpp;
		 qpp = &qb -> tb_next)
	    if (qb -> tb_priority > tb -> tb_priority)
		break;
	tb -> tb_next = qb;
	*qpp = tb;

	/* Fill in the tb_instance field of the new element
	   with the concatenation of the size of the object name
	   followed by the object name itself followed by
	   its priority.  (EJP) */

	ip = tb -> tb_instance;
	jp = ot -> ot_name -> oid_elements;
	*ip++ = ot -> ot_name -> oid_nelem;
	for (i = ot -> ot_name -> oid_nelem; i > 0; i--)
	    *ip++ = *jp++;
	*ip++ = tb -> tb_priority;
	tb -> tb_insize = ip - tb -> tb_instance;

	/* Insert the new element into the doubly-linked chain
	   of all smuxTree structures that is anchored in THead.
	   Elements are chained in lexicographic order of
	   tb_instance so that the get_tbent() function will
	   work correctly for 'get next' operations.  (EJP) */

	for (qb = THead -> tb_forw;
		 qb != THead;
		 qb = qb -> tb_forw)
	  if (elem_cmp (tb -> tb_instance, tb -> tb_insize,
			qb -> tb_instance, qb -> tb_insize)
		  < 0)
	      break;
	insque (tb, qb -> tb_back);
    }

    return result;
}

static int
smux_register_response (pb, priority)
register struct smuxPeer *pb;
int priority;
{
    int	    result = OK;
    struct type_SMUX_RRspPDU rrsp;
    struct type_SMUX_PDUs rsp;
    PE	pe;

    bzero ((char *) &rsp, sizeof rsp);
    rsp.offset = type_SMUX_PDUs_registerResponse;
    rsp.un.registerResponse = &rrsp;

    bzero ((char *) &rrsp, sizeof rrsp);
    rrsp.parm = priority;

    pe = NULLPE;

    if (encode_SMUX_PDUs (&pe, 1, 0, NULLCP, &rsp) != NOTOK) {
	if (debug > 2)
	    print_SMUX_PDUs (pe, 1, NULLIP, NULLVP, NULLCP);
	if (debug)
	    advise (SLOG_DEBUG, MSGSTR(MS_SMUXS, SMUXS_13,
		    "sending register response to (SMUX %s)"),
		    pb -> pb_source);

	if (pe2ps (pb -> pb_fi -> fi_ps, pe) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, "pe2ps: %s (SMUX %s)",
		    ps_error (pb -> pb_fi -> fi_ps -> ps_errno), 
		    pb -> pb_source);
	    result = NOTOK;
	}
    }
    else {
	advise (SLOG_EXCEPTIONS, "encode_SMUX_PDUs: %s (SMUX %s)",
		PY_pepy, pb -> pb_source);
	result = NOTOK;
    }

    if (pe)
	pe_free (pe);

    return result;
}

/*  */

int  
smux_method (pdu, ot, pb, v, offset)
struct type_SNMP_PDUs *pdu;
OT	ot;
register struct smuxPeer *pb;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    status,
	    orig_id = 0;
    struct type_SNMP_VarBindList *orig_bindings = NULL,
				 vps;
    struct type_SMUX_PDUs  	req,
				*rsp;
    struct type_SMUX_SOutPDU cor;
    register struct type_SNMP_GetResponse__PDU *get;
    PE	    pe;
    char    pb_source[60];

    status = int_SNMP_error__status_noError;
    bzero ((char *) &req, sizeof req);
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    req.offset = type_SMUX_PDUs_get__request;
	    goto stuff_pdu;

	case type_SNMP_PDUs_get__next__request:
	    req.offset = type_SMUX_PDUs_get__next__request;
	    goto stuff_pdu;

	case type_SNMP_PDUs_set__request:
	    req.offset = type_SMUX_PDUs_set__request;
stuff_pdu:;
	    orig_id = pdu -> un.get__request -> request__id;
	    orig_bindings = pdu -> un.get__request -> variable__bindings;

	    req.un.get__request = pdu -> un.get__request;
	    bzero ((char *) &vps, sizeof vps);
	    vps.VarBind = v;
    
	    pdu -> un.get__request -> request__id = quantum;
	    pdu -> un.get__request -> variable__bindings = &vps;
	    break;

	case type_SNMP_PDUs_commit:
	    cor.parm = int_SMUX_SOutPDU_commit;
	    goto stuff_cor;

	case type_SNMP_PDUs_rollback:
	    cor.parm = int_SMUX_SOutPDU_rollback;
stuff_cor:;
	    req.offset = type_SMUX_PDUs_commitOrRollback;
	    req.un.commitOrRollback = &cor;
	    break;
    }

    sprintf (pb_source, " (SMUX %s)", pb -> pb_source);
    pe = NULLPE;

    if (encode_SMUX_PDUs (&pe, 1, 0, NULLCP, &req) != NOTOK) {
	if (debug > 2)
	    print_SMUX_PDUs (pe, 1, NULLIP, NULLVP, NULLCP);

	if (debug)
	    advise (SLOG_DEBUG, MSGSTR(MS_SMUXS, SMUXS_14,
		    "sending request to %s"), pb_source);
	if (snd_pe (pe, pb -> pb_fi -> fi_ps, pb_source) == NOTOK) {

lost_peer: ;
	    pb_free (pb);
	    status = int_SNMP_error__status_genErr;
	}
    }
    else {
	advise (SLOG_EXCEPTIONS, 
		"encode_SMUX_PDUs: %s%s", PY_pepy, pb_source);
	status = int_SNMP_error__status_genErr;
    }

    if (pe)
	pe_free (pe);

    switch (offset) {
	case type_SNMP_PDUs_get__request:
	case type_SNMP_PDUs_get__next__request:
	case type_SNMP_PDUs_set__request:
	    pdu -> un.get__request -> request__id = orig_id;
	    pdu -> un.get__request -> variable__bindings = orig_bindings;
	    break;
	
	default:
	    break;
    }

    /* Don't bother to wait for a response from the subagent if the
       message wasn't sent successfully, or if it was a commit or 
       rollback message. (EJP) */

    if (status != int_SNMP_error__status_noError
	    || offset == type_SNMP_PDUs_rollback
	    || offset == type_SNMP_PDUs_commit)
	return status;

get_response_again:;
    SNMP_timeout = smuxtimeout;		/* set timeout in libsnmp */
    if ((pe = rcv_pe (pb -> pb_fi -> fi_ps, pb_source)) == NULLPE) {

	/* log this timeout (EJP 5/20/92) */
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_15,
		"noResponse after %d seconds %s"), smuxtimeout, pb_source);

	goto lost_peer;
    }

    rsp = NULL;

    if (decode_SMUX_PDUs (pe, 1, NULLIP, NULLVP, &rsp) == NOTOK) {
	advise (SLOG_EXCEPTIONS, "decode_SMUX_PDUs: %s%s", 
		PY_pepy, pb_source);

lost_peer_again: ;
	pb_free (pb);
	status = int_SNMP_error__status_genErr;
	goto out;
    }

    if (debug)
	advise (SLOG_DEBUG, MSGSTR(MS_SMUXS, SMUXS_16,
		"received response from %s"), pb_source);
    if (debug > 2)
	print_SMUX_PDUs (pe, 1, NULLIP, NULLVP, NULLCP);

    switch (rsp -> offset) {
	case type_SMUX_PDUs_get__response:
	    get = rsp -> un.get__response;
	    break;

	/* allow traps, registerRequests, and close messages */
	case type_SMUX_PDUs_trap:
	case type_SMUX_PDUs_registerRequest:
	case type_SMUX_PDUs_close:
	/* note: smux_process of a trap (currently) will never fail.
	         registerRequest may fail, and close always fails  */
	    if (smux_process (pb, rsp) == OK) {
		free_SMUX_PDUs (rsp);
		if (pe)
			pe_free(pe);
		goto get_response_again;  /* still expect a get__response */
	    }
	    else 
		goto lost_peer_again;
	    break;

	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_30,
		    "unexpected response: %d%s"), rsp -> offset, pb_source);

	    goto lost_peer_again;
	    break;
    }

    switch (status = get -> error__status) {
	case int_SNMP_error__status_noError:
	    {
		register struct type_SNMP_VarBindList *vp;
		register struct type_SNMP_VarBind *v2;

		if ((vp = get -> variable__bindings) == NULL) {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_17,
			    "missing variable in get response %s"), pb_source);

		    goto lost_peer_again;
		}
		v2 = vp -> VarBind;

		if (offset == type_SNMP_PDUs_get__next__request
		        && (ot -> ot_name -> oid_nelem
			    	    > v2 -> name -> oid_nelem
			        || bcmp ((char *) ot -> ot_name ->oid_elements,
					 (char *) v2 -> name -> oid_elements,
					 ot -> ot_name -> oid_nelem
				           * sizeof ot -> ot_name ->
							oid_elements[0]))) {
			status = NOTOK;
			break;
		}
		free_SMI_ObjectName (v -> name);
		v -> name = v2 -> name;
		v2 -> name = NULL;
		free_SMI_ObjectSyntax (v -> value);
		v -> value = v2 -> value;
		v2 -> value = NULL;
	    }
	    break;

	case int_SNMP_error__status_noSuchName:
	    if (offset == type_SNMP_PDUs_get__next__request) {
		status = NOTOK;
		break;
	    }
	    /* else fall */

        default:
	    break;
    }

out: ;
    if (rsp)
	free_SMUX_PDUs (rsp);
    if (pe)
	pe_free (pe);

    return status;
}

/*  */

void
pb_free (pb)
register struct smuxPeer *pb;
{
    register struct smuxTree *tb,
			     *ub;

    if (pb == NULL)
	return;

    for (tb = THead -> tb_forw; tb != THead; tb = ub) {
	ub = tb -> tb_forw;
	
	if (tb -> tb_peer == pb)
	    tb_free (tb);
    }

    if (pb -> pb_fi -> fi_fd != NOTOK) {
	shutdown (pb -> pb_fi -> fi_fd, 2);
	(void) close_tcp_socket (pb -> pb_fi -> fi_fd);
    }

    if (pb -> pb_identity)
	oid_free (pb -> pb_identity);
    if (pb -> pb_description)
	free (pb -> pb_description);

    pb -> pb_fi -> fi_fd = NOTOK;		/* delete from fi list */

    remque (pb);

    free ((char *) pb);
}

/*  */

void
tb_free (tb)
register struct smuxTree *tb;
{
    register struct smuxTree *tp,
			    **tpp;

    if (tb == NULL)
	return;

    for (tpp = (struct smuxTree **) &tb -> tb_subtree -> ot_smux;
	     tp = *tpp;
	     tpp = &tp -> tb_next)
	if (tp == tb) {
	    *tpp = tb -> tb_next;
	    break;
	}

    if ((tb -> tb_smux_init == SMUX_INITED) && 
	(tb -> tb_subtree -> ot_smux == NULL)) {
      delete_objects(tb -> tb_subtree);
    }

    remque (tb);

    free ((char *) tb);
}

/*  */

/* 
 * authenticate this smux client.
 */
int
smux_client_auth (pb, password)
register struct smuxPeer *pb;
struct qbuf *password;
{
    register struct smuxClient *sc;
    register struct sockaddr_in *na = pb -> pb_fi -> sockaddr_in;
    char *errStr;

    /* Look through all of the SMUX client structures (built from the
       'smux' entries in the snmp.conf configuration file) for a match
       with the identifier of the subagent trying to open a session.
       Only the common root part of the identifiers is compared.  This 
       allows the subagent to append its version number to its identifier
       without having to change the configuration file.  (EJP) */

    for (sc = SHead -> sc_forw; sc != SHead; sc = sc -> sc_forw) 
	if (pb->pb_identity->oid_nelem>=sc->sc_id->oid_nelem &&
	    elem_cmp(pb->pb_identity->oid_elements, sc->sc_id->oid_nelem,
		     sc->sc_id->oid_elements, sc->sc_id->oid_nelem) == 0)
	    break;

    if (sc == SHead 
	    || (sc -> sc_password != NULL &&
		strcmp (sc -> sc_password, password -> qb_forw -> qb_data))) {
	errStr = sc == SHead ? "badIdentity" : "badPassword";
	goto you_lose;
    }

    /*
     * The netmask is &'d with the address to check if a 
     * particular network has access.
     */
    if (((sc -> sc_sin.sin_addr.s_addr & sc -> sc_netmask.sin_addr.s_addr) == 
	    (na -> sin_addr.s_addr & sc -> sc_netmask.sin_addr.s_addr)) == 0) { 
	errStr = "invalid host/network address";
	goto you_lose;
    }
    return OK;

you_lose:;
    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXS, SMUXS_18,
		"simpleOpen rejected (%s): %s (SMUX %s)"),
		errStr, oid2ode (pb -> pb_identity), pb -> pb_source);
    return NOTOK;
}
#endif	/* SMUX */

