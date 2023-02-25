static char sccsid[] = "@(#)14	1.12.1.16  src/tcpip/usr/sbin/snmpd/snmpd.c, snmp, tcpip411, 9434A411a 8/18/94 14:32:26";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: main(), doit_snmp(), do_operation(), nextinst(), 
 *            count_snmpin(), count_snmpout(), arginit(), 
 *	      Usage(), snmp_receive_signal(), envinit(), snmpd_exit()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/snmpd.c
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

#include <ctype.h>
#include <varargs.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <isode/snmp/io.h>
#include "snmpd.h"
#ifdef SMUX
#  include "smux_g.h"
#endif
#include "view.h"

#ifdef _POWER
#include <nl_types.h>
#endif /* _POWER */

#if     defined (AIX) && defined (RT)
#include <sys/syslog.h>
#else
#include <syslog.h>
#endif

#include "interfaces.h"
#include "routes.h"
#include "fddi.h"

/* EXTERNS */
extern int optind;
extern char *optarg;
nl_catd snmpcatd;
nl_catd catd;

extern struct snmpstat  snmpstat;	/* defined in snmp.c */
extern struct logging  *logging,	/* defined in logging.c */
			log_cmdline;    /* ... */
extern int stimeout,			/* defined in config.c */	
           maxopsize,			/* ... */
           ethernettimeout,		/* ... */
           tokenringtimeout,		/* ... */
           fdditimeout;			/* ... */
extern SMT_impl *SMT_list;		/* defined in fddi.c */
extern int smtnumber;			/* defined in fddi.c */

extern struct community *str2comm ();	/* defined in view.c */
extern void     init_log_struct (),     /* defined in logging.c */
		init_prelogging (),     /* ... */
		init_logging ();        /* ... */
extern void	start_trap (),
		do_trap ();
extern int      under_src ();           /* defined in src.c */
extern void	refresh (),             /* defined in config.c */
		setconfig (),           /* ... */
		readconfig (),          /* ... */
		set_maxpacket ();       /* ... */
extern int	check_routes();		/* Defined in ip.c */
extern void	init_fddi ();		/* Defined in fddi.c */
extern void	free_SMT_frame ();	/* Defined in fddi.c */

#ifdef DEBUG
extern void print_list();
#endif /* DEBUG */

/* FORWARD */
void    arginit (),
	Usage (),
	envinit (),
	count_snmpout (),
	snmpd_exit ();

/* GLOBAL */
char *Myname;
int   quantum,		/* kernel request */
      Nd,	        /* dummy socket fd (actually the udp-161 socket) */
      debug_default = 0,	/* for refresh */
      debug = 0,		/* debug level */
      mypid;
int   OdmInit = NOTOK;			/* For odm initialization status */
static int snmp_signal_mask = 0;	/* signals to block */
char *commname;                         /* The current community name */
int request_id;                         /* The current request id     */
int SNMP_PDU_variable_position = 0;	/* The position in the PDU variable */
					/* bind list                        */

main (argc, argv)
int	argc;
char	**argv;
{
    register int nfds, fd;
    struct fdinfo *fi;
    struct fd_set rfds, rmask;
    struct timeval timeout;
    int n, sigmask_save;
    int oldtimer;

#ifdef _POWER
    setlocale (LC_ALL,""); 			/* Designates native locale */
    snmpcatd = catopen (MF_SNMPD,0);
#endif /* _POWER */

    /* 
     * Initialize the snmpd agent.
     */
    arginit (argc, argv);

    envinit ();

    /* 
     * Send cold start trap.
     */
    do_trap (int_SNMP_generic__trap_coldStart, 0,
	    (struct type_SNMP_VarBindList *) 0);

    /* main loop */
    if ((nfds = fdmask (&rfds) + 1) == 0)
	adios (MSGSTR(MS_SNMPD, SNMPD_19, "no listening fd's"));

    /* setup timeout for interface checking */
    timerclear (&timeout);
    timeout.tv_sec = stimeout;
    oldtimer = stimeout;
    for (;;) {
	rmask = rfds;	/* struct copy */
	if (stimeout != oldtimer)
	{
	    /* setup timeout for interface checking */
	    timerclear (&timeout);
	    timeout.tv_sec = stimeout;
	    oldtimer = stimeout;
        }

	if ((n = select (nfds, &rmask, 0, 0, &timeout)) == NOTOK){
	    if (errno == EINTR)
		continue;		/* caught a signal this time... */
	    adios (MSGSTR(MS_SNMPD, SNMPD_20,
	                  "select failed: %s"), strerror(errno));
	}

	sigmask_save = sigblock (snmp_signal_mask);
	if (n == 0) {
	    if (debug)
		advise(SLOG_DEBUG, "Select interface timeout");
	    quantum++;		/* insure a check */
	    (void) get_interfaces (type_SNMP_PDUs_get__request);
	    sigsetmask (sigmask_save);
	    if (fi_check () && (nfds = fdmask (&rfds) + 1) == 0)
		adios (MSGSTR(MS_SNMPD, SNMPD_19, "no listening fd's"));
            if ((nfds = fdmask (&rfds) + 1) == 0)
		adios (MSGSTR(MS_SNMPD, SNMPD_19, "no listening fd's"));
	    continue;
	}

	/* check out who called us */
	for (fd = 0; fd < nfds; fd++) {
	    if (!FD_ISSET (fd, &rmask)) 
		continue;
	    if ((fi = fd2fi (fd)) == NULL)
	        adios (MSGSTR(MS_SNMPD, SNMPD_21,
	                      "selected fd %d, but no fi"), fd);
	    if ((fi -> fi_flags & FI_UDP) == FI_UDP)
		doit_snmp (fi);
#ifdef SMUX
	    else if ((fi -> fi_flags & FI_SMUX) == FI_SMUX) {
		int tfd;

		/* new smux relationship? */
		if ((tfd = start_smux (fi)) != NOTOK) {
		    if (tfd >= nfds)
			nfds = tfd + 1;
		    FD_SET (tfd, &rfds);
		}
	    }
	    else if ((fi -> fi_flags & FI_SMUX_CLIENT) == FI_SMUX_CLIENT) {
                /* 
                 * Handle multiple simultaneous requests.
                 * Quit looping if the file descriptor has been closed.
                 * The file descriptor can be closed if an operation on
                 * the queue fails.
                 */
		do {
		    (void) doit_smux (fi);
		} while ((fi -> fi_fd != NOTOK) &&
			 (ps_prime (fi -> fi_ps, 1) == DONE));
	    }
#endif /* SMUX */
#ifdef SRC_SUPPORT
	    else if ((fi -> fi_flags & FI_SRC) == FI_SRC)
		doit_src ();
#endif /* SRC_SUPPORT */
	}

	/*
	 * check for fi's that have been marked invalid.
	 * if any are found, redo our fdmask.
	 * Used for SMUX support and device driver support
	 */
	if (fi_check () && (nfds = fdmask (&rfds) + 1) == 0)
	  adios (MSGSTR(MS_SNMPD, SNMPD_19, "no listening fd's"));
        if ((nfds = fdmask (&rfds) + 1) == 0)
	  adios (MSGSTR(MS_SNMPD, SNMPD_19, "no listening fd's"));

	sigsetmask (sigmask_save);
    }
    /* NOTREACHED */
}


doit_snmp (fi)
struct fdinfo *fi;
{
    PE pe;
    struct type_SNMP_Message *msg;
    struct community *comm;
    int rt, in_offset, toobig;

    pe = NULLPE;
    msg = NULL;
    commname = NULL;

    /* read an asn packet from our stream */
    if ((pe = ps2pe (fi -> fi_ps)) == NULLPE) {
	advise (SLOG_EXCEPTIONS, "ps2pe: %s: %s (%s)",
		ps_error (fi -> fi_ps -> ps_errno), strerror (errno),
		inet_ntoa (fi -> sockaddr_in -> sin_addr));
	snmpstat.s_inpkts++;
	snmpstat.s_asnparseerrs++;
	return;
    }

    /* expand the packet into a msg, and do some initial checking */
    snmpstat.s_inpkts++;
    if (debug > 2) {	/* need only a PE to print */
        advise (SLOG_TRACE, "");
	print_SNMP_Message (pe, 1, NULLIP, NULLVP, NULLCP);
    }
    if (decode_SNMP_Message (pe, 1, NULLIP, NULLVP, &msg) == NOTOK) {
	advise (SLOG_EXCEPTIONS, "decode_SNMP_Message: %s (%s)", 
		PY_pepy, inet_ntoa (fi -> sockaddr_in -> sin_addr));
	snmpstat.s_asnparseerrs++;
	goto out;
    }
    if (msg -> version != int_SNMP_version_version__1) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_1,
                "bad version: %d (%s)"),
		msg -> version, inet_ntoa (fi -> sockaddr_in -> sin_addr));
	snmpstat.s_badversions++;
	goto out;
    }
    /* Get the community name from the SNMP message */
    if ((commname = qb2str (msg -> community)) == NULLCP) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_2,
                "%s: out of memory (%s)"), "doit_snmp",
		inet_ntoa (fi -> sockaddr_in -> sin_addr));
	goto out;
    }

    /*
     * Check that we have community access.
     * If community access is denied, protocol states an authentication
     * trap may be generated, the datagram is discarded, and the agent
     * takes no further action.
     */
    if ((comm = str2comm (commname, fi -> sockaddr_in)) == NULL) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_3,
                "authentication error: invalid community name: %s"), commname);
	snmpstat.s_badcommunitynames++;
	if (snmpstat.s_enableauthentraps == TRAPS_ENABLED)
	    do_trap(int_SNMP_generic__trap_authenticationFailure, 0,
		(struct type_SNMP_VarBinList *) 0);
	goto out;
    }

    request_id = msg -> data -> un.get__request -> request__id;
    in_offset = msg -> data -> offset;		/* save in-coming offset */

    for (rt = 0; rt < smtnumber; rt++)
      if (SMT_list[rt].msgtosend != NULL)
      {
	free_SMT_frame(SMT_list[rt].msgtosend, 1);
        SMT_list[rt].msgtosend = NULL;
      }

    if ((rt = count_snmpin (fi, msg, comm)) == int_SNMP_error__status_noError) {

	/* Tick off request counter, for kmem diving.
	 * NOTE: use snmpstat.s_inpkts in the future?
	 */
	quantum++;    

	switch (in_offset) {

	    /*
	     *	two step set operation.
	     *		-) set__request "really" means check if these
	     *		   varbinds are setable.
	     *		-) commit actually does the set.
	     *			NO ERROR RECOVERY FOR THIS STEP.
	     *		-) rollback cleans up after a possible unsuccessful
	     *		   set__request.
	     */
	    case type_SNMP_PDUs_set__request:
		if ((rt = do_operation (fi, msg, comm, in_offset)) !=
			int_SNMP_error__status_noError) {
		    rt = do_operation (fi, msg, comm, type_SNMP_PDUs_rollback);
		}
		else {
		    if (check_routes() == NOTOK) {
			struct type_SNMP_PDUs *pdu = msg -> data;
			struct type_SNMP_GetResponse__PDU *parm = pdu -> un.get__response;

		        rt = do_operation(fi, msg, comm, type_SNMP_PDUs_rollback);
			parm -> error__index = 1;
			parm -> error__status = int_SNMP_error__status_genErr;
			rt = int_SNMP_error__status_genErr;
		    }
		    else rt=do_operation(fi, msg, comm, type_SNMP_PDUs_commit);
		}
		break;

	    case type_SNMP_PDUs_get__request:
	    case type_SNMP_PDUs_get__next__request:
	    default:
		rt = do_operation (fi, msg, comm, in_offset);
		break;
	}

    } 
    if (rt == NOTOK)
	goto out;		/* packet dropped */

    /* send out the response */
    pe_free (pe);
    pe = NULLPE;
    toobig = 0;
    msg -> data -> offset = type_SNMP_PDUs_get__response;  /* set outgoing */
    do {
	if (encode_SNMP_Message (&pe, 1, 0, NULLCP, msg) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, "encode_SNMP_Message: %s (%s)",
		    PY_pepy, inet_ntoa (fi -> sockaddr_in -> sin_addr));
	    break;
	}

	/* check if the packet will be too big. */
	if (maxopsize < ps_get_abs (pe)) {
	    struct type_SNMP_PDUs *pdu = msg -> data;
	    struct type_SNMP_GetResponse__PDU *parm = pdu -> un.get__response;

	    if (toobig) {	/* sanity check */
		toobig = 0;
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_4,
		        "too big on a too_big response"));
		continue;	/* DROP out of DO_LOOP */
	    }
	    toobig++;
	    free_SNMP_VarBindList (parm -> variable__bindings);
	    parm -> variable__bindings = NULL;
	    parm -> error__status = int_SNMP_error__status_tooBig;
	    continue;	/* DROP, re-encode msg */
	}
	else
	    toobig = 0;
	if (debug > 2) {	/* need only a PE to print */
	    advise (SLOG_TRACE, "");
	    print_SNMP_Message (pe, 1, NULLIP, NULLVP, NULLCP);
        }

	/* send out the packet */
	if (pe2ps (fi -> fi_ps, pe) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, "pe2ps: %s: %s (%s)",
		    ps_error (fi -> fi_ps -> ps_errno), strerror (errno),
		    inet_ntoa (fi -> sockaddr_in -> sin_addr));

	    /* flush output buffer */
	    (void) dg_flushb (fi -> fi_ps);
	}
	else
	    count_snmpout (fi, msg, in_offset);
    } while (toobig);	/* may need to run this code twice */


out:
    if (msg)
	free_SNMP_Message (msg);
    msg = NULL;
    if (pe)
	pe_free (pe);
    pe = NULL;
    if (commname)
	free (commname);
    commname = NULL;
    return;
}

/*
 * loop thru our object tree until we find another Object with
 * a get function.  Initialize OI if a good Object is found.
 */
OI
nextinst (oid, ot)
OID	oid;	/* requested instance */
OT	ot;	/* starting point in Object tree */
{
    register OI oi;
    register OT ott;

    /* If current object is being handled by a SMUX sub-agent, then
       step over the entire subtree before proceeding with the
       'get-next' search.  This is needed to avoid descending into
       the portion of the object tree that the SMUX sub-agent has
       'mounted over'. (EJP) */
    if (ot -> ot_smux) {
	int level = ot -> ot_name -> oid_nelem;

	while (ot -> ot_next && ot -> ot_next -> ot_name -> oid_nelem > level &&
		(elem_cmp(oid->oid_elements, level, 
			  ot->ot_next->ot_name->oid_elements, level) == 0))
	    ot = ot -> ot_next;
    }
    for (ot = ot -> ot_next;ot;ot = ot -> ot_next) {

	/* get the next valid instance after our starting point. */
	if ((oi = nextot2inst (oid, ot)) == NULLOI)
	    return NULLOI;

	/* check that we have a valid get function */
	if ((ott = oi -> oi_type) -> ot_getfnx || ott -> ot_smux) 
	    return oi;

	/* increment to a new starting point */
    }
    return NULLOI;
}

int
do_operation (fi, msg, comm, offset)
struct fdinfo	*fi;
struct type_SNMP_Message *msg;	/* in-coming message */
struct community *comm;
int offset;			/* type of request */
{
    int		idx,		/* error index pointer */
		status,		/* status returned from the get function */
		done,		/* fall out of varBind for-loop */
		tryagain = 0;	/* get-next ism */
    register struct type_SNMP_PDUs *pdu = msg -> data;
    register struct type_SNMP_VarBindList *vp;
    register struct type_SNMP_GetResponse__PDU *parm = pdu -> un.get__response;
    IFP		method = NULL;
    struct view *vu = comm -> c_view;

    /* 
     * main variable binding processing loop 
     */
    if (offset != type_SNMP_PDUs_rollback) {
        parm -> error__index = 0;
        parm -> error__status = int_SNMP_error__status_noError;
    }

    for (idx = 0, done = 0, vp = pdu -> un.get__request -> variable__bindings; 
	    vp && !done; vp = vp -> next) {
	register OI	oi;
	register OT	ot;
	register struct type_SNMP_VarBind *v = vp -> VarBind;

	idx++;
	SNMP_PDU_variable_position = idx;
	if (offset == type_SNMP_PDUs_get__next__request) {	/* GET_NEXT */
	    if ((oi = name2inst (v -> name)) == NULLOI
		    && (oi = next2inst (v -> name)) == NULLOI) {
		parm -> error__status = int_SNMP_error__status_noSuchName;
		break;		/* FALL OUT OF FOR LOOP */
	    }
	    if ((ot = oi -> oi_type) -> ot_getfnx == NULLIFP
		    && ot -> ot_smux == NULL) {
		goto get_next;
	    }
	}
	else {						     /* GET || SET */
	    if ((oi = name2inst (v -> name)) == NULLOI) {
		parm -> error__status = int_SNMP_error__status_noSuchName;
		break;		/* FALL OUT OF FOR LOOP */
	    }
	    ot = oi -> oi_type;
	    if (offset == type_SNMP_PDUs_get__request) {	/* GET */
		if (ot -> ot_getfnx == NULLIFP && ot -> ot_smux == NULL) {
		    parm -> error__status = int_SNMP_error__status_noSuchName;
		    break;		/* FALL OUT OF FOR LOOP */
		}
	    }
	    else {						/* SET */
		if (ot -> ot_setfnx == NULLIFP && ot -> ot_smux == NULL) {
		    /*
		     *	if we had a getfnx for this varbind (READ ACCESS), 
		     *	or we were suppossed to have WRITE access, but 
		     *	a setfnx does not exist (NOT IMPLEMENTED for
		     *	this varbind), then this var must be readOnly.
		     */
#if 0
		    /*  it is (currently) a protocol error to issue an 
		     *  error of 'readOnly'.   If a variable is not
		     *  writable, then it is not 'in your view', hence
		     *  we must return 'noSuchName'.  (RLF)
		     */
		    if (ot -> ot_access & OT_RDONLY
			    || ot -> ot_access & OT_WRONLY)
			parm -> error__status = int_SNMP_error__status_readOnly;
		    else
#endif
			parm -> error__status = 
				int_SNMP_error__status_noSuchName;
		    break;		/* FALL OUT OF FOR LOOP */
		}
	    }
	}
		
	/*
	 *	do_while for get__next__request cases, 1 pass thru for
	 *	get__request and set__request.
	 */
	do {
	    tryagain = 0;	/* may get bumped by get-next */

	    /* check that we have view access for this object */
	    switch (offset) {
		case type_SNMP_PDUs_get__request:
		    /* view/access check */
		    if (!(vu -> v_mask & ot -> ot_views)
			    || (ot -> ot_smux == NULL 
				&& !(ot -> ot_access & OT_RDONLY))) {
			parm -> error__status = 
				int_SNMP_error__status_noSuchName;
			done = 1;	/* FALL OUT OF FOR LOOP */
			continue;	/* drop out of do-while */
		    }
		    method = ot -> ot_getfnx;
		    break;

		case type_SNMP_PDUs_get__next__request:
		    /* view/access check */
		    if (!(vu -> v_mask & ot -> ot_views)
			    || (ot -> ot_smux == NULL 
				&& !(ot -> ot_access & OT_RDONLY))) 
			goto get_next;
		    method = ot -> ot_getfnx;
		    break;

		case type_SNMP_PDUs_set__request:
		case type_SNMP_PDUs_commit:
		case type_SNMP_PDUs_rollback:
		    /* view/access check */
		    if (!(vu -> v_mask & ot -> ot_views)
			    || (ot -> ot_smux == NULL 
				&& !(ot -> ot_access & OT_WRONLY))) {
			parm -> error__status = 
				int_SNMP_error__status_noSuchName;
			done = 1;	/* FALL OUT OF FOR LOOP */
			continue;	/* drop out of do-while */
		    }
		    method = ot -> ot_setfnx;
		    break;
	    }

	    /* pass request off to Object specific function */
#ifdef SMUX
	    if (ot -> ot_smux)
		status = smux_method (pdu, ot, 
			((struct smuxTree *) ot -> ot_smux) -> tb_peer,
			v, offset);
	    else
#endif
		status = (*method) (oi, v, offset);

	    /* check-out status of object-specific get function */
	    switch (status) {
		case NOTOK:	/* failed get-next at object level routine */
get_next:;
		    if ((oi = nextinst (v -> name, ot)) == NULLOI) {

			/* no dice.  Must be at end of MIB tree */
			parm -> error__status = 
				int_SNMP_error__status_noSuchName;
			done = 1;		/* FALL OUT OF FOR LOOP */
			break;
		    }
		    ot = oi -> oi_type;
		    tryagain = 1;	/* try this get again */
		    break;

		case int_SNMP_error__status_noError:
		    break;
		
		default:
		    if (offset != type_SNMP_PDUs_rollback)
		        parm -> error__status = status;
		    done = 1;		/* FALL OUT OF FOR LOOP */
		    break;
	    }
	} while (tryagain);
    }

    /* set error index, if needed */
    if (parm -> error__status != int_SNMP_error__status_noError &&
		offset != type_SNMP_PDUs_rollback)
	parm -> error__index = SNMP_PDU_variable_position;
    
    return parm -> error__status;
}

/*
 * Count the type of incoming message.  
 * Returns error if bad type, or community access doesn't match up.
 */
int
count_snmpin (fi, msg, comm)
struct fdinfo *fi;
struct type_SNMP_Message *msg;	/* in-coming message */
struct community *comm;
{
    int status = int_SNMP_error__status_noError;
    register struct type_SNMP_PDUs *pdu = msg -> data;
    register struct type_SNMP_GetResponse__PDU *parm = pdu -> un.get__response;
    struct view *vu = comm -> c_view;

    /*
     * Count the input error status.
     * the purist view... although illegal protocol, it *is* possible
     * to get an input packet that has the error__status field set. 
     */
    switch (pdu -> offset) {
	case type_SNMP_PDUs_get__request:
	case type_SNMP_PDUs_get__next__request:
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_get__response:

	    switch (parm -> error__status) {

		case int_SNMP_error__status_tooBig:
		    snmpstat.s_intoobigs++;
		    break;

		case int_SNMP_error__status_noSuchName:
		    snmpstat.s_innosuchnames++;
		    break;

		case int_SNMP_error__status_badValue:
		    snmpstat.s_inbadvalues++;
		    break;

		case int_SNMP_error__status_readOnly:
		    snmpstat.s_inreadonlys++;
		    break;

		case int_SNMP_error__status_genErr:
		    snmpstat.s_ingenerrs++;
		    break;
	    }
    }

    /* Count the input packet type */
    switch (pdu -> offset) {

	case type_SNMP_PDUs_get__request:
	case type_SNMP_PDUs_get__next__request:

	    if (pdu -> offset == type_SNMP_PDUs_get__request)
		snmpstat.s_ingetrequests++;
	    else
		snmpstat.s_ingetnexts++;

	    /* 
	     * If we don't have at least read permission for our
	     * community, return 'noSuchName'.
	     */
	    if (vu == NULL || !(comm -> c_permission & OT_RDONLY)) {
		snmpstat.s_badcommunityuses++;
		status = parm -> error__status = int_SNMP_error__status_noSuchName;
	    }
	    break;

	case type_SNMP_PDUs_set__request:
	    snmpstat.s_insetrequests++;

	    /* 
	     * If we don't have at least write permission for our
	     * community, return 'readOnly'.
	     */
	    if (vu == NULL || !(comm -> c_permission & OT_WRONLY)) {
		snmpstat.s_badcommunityuses++;
		status = parm -> error__status = 
#if 0
			/* see above comments about readOnly. (RLF) */
			int_SNMP_error__status_readOnly;
#else
			int_SNMP_error__status_noSuchName;
#endif
	    }
	    break;

	case type_SNMP_PDUs_get__response:
	    snmpstat.s_ingetresponses++;

	    /* fall thru ... */
	case type_SNMP_PDUs_trap:
	    if (pdu -> offset == type_SNMP_PDUs_trap)
		snmpstat.s_intraps++;

	    /* fall thru ... */
	default:
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_5,
	            "bad operation: %d (%s)"),
		    pdu -> offset, inet_ntoa (fi -> sockaddr_in -> sin_addr));
	    return NOTOK;	/* packet dropped !!! */
    }
    if (status != int_SNMP_error__status_noError)
	/* construct a return message */
	pdu -> offset = type_SNMP_PDUs_get__response;

    return status;
}

/*
 * count the outgoing message type.
 */
void
count_snmpout (fi, msg, in_offset)
struct fdinfo *fi;
struct type_SNMP_Message *msg;	/* out-going message */
int in_offset;			/* in-coming offset */
{
    register struct type_SNMP_PDUs *pdu = msg -> data;
    register struct type_SNMP_GetResponse__PDU *parm = pdu -> un.get__response;

    /* count out-going packet-types */
    switch (pdu -> offset) {
	case type_SNMP_PDUs_get__response:
	    snmpstat.s_outgetresponses++;

	    /* count out-going errors */
	    switch (parm -> error__status) {
		register struct type_SNMP_VarBindList *vp;
		int cnt;

		case int_SNMP_error__status_noError:

		    /* count number of variables retrieved or set */
		    for (cnt = 0, vp = parm -> variable__bindings; 
			    vp; vp = vp -> next)
			cnt++;
		    switch (in_offset) {
			case type_SNMP_PDUs_get__request:
			case type_SNMP_PDUs_get__next__request:
			    snmpstat.s_totalreqvars += cnt;
			    break;

			case type_SNMP_PDUs_set__request:
			    snmpstat.s_totalsetvars += cnt;
			    break;
		    }
		    break;

		case int_SNMP_error__status_tooBig:
		    snmpstat.s_outtoobigs++;
		    break;

		case int_SNMP_error__status_noSuchName:
		    snmpstat.s_outnosuchnames++;
		    break;

		case int_SNMP_error__status_badValue:
		    snmpstat.s_outbadvalues++;
		    break;

		case int_SNMP_error__status_genErr:
		    snmpstat.s_outgenerrs++;
		    break;

		case int_SNMP_error__status_readOnly:
		    		/* no longer tracked... */
		default:
		    break;
	    }

	    break;

	case type_SNMP_PDUs_trap:
	    snmpstat.s_outtraps++;
	    break;

	/* 
	 * the following 3 cases are not implemented, but are included
	 * for the 'purist' point of view.
	 */
	case type_SNMP_PDUs_get__request:
	    snmpstat.s_outgetrequests++;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    snmpstat.s_outgetnexts++;
	    break;

	case type_SNMP_PDUs_set__request:
	    snmpstat.s_outsetrequests++;
	    break;

	default:
	    break;
    }
    snmpstat.s_outpkts++;
}

/* globalized for envinit () */
static int udp_port = 0, smux_port = 0;
#ifdef DEBUG
static int smux_enabled = OK;
#endif

/* 
 * NAME: arginit ()
 *
 * FUNCTION: Checkout command-line options 
 */
void
arginit (argc, argv)
int	argc;
char	**argv;
{
    int i;
    int d_flag = 0,
	c_flag = 0,
	f_flag = 0;

#ifdef DEBUG
    int p_flag = 0, 
        P_flag = 0, 
	s_flag = 0;
    char *options = "d:p:c:P:sf:";
#else
    char *options = "d:c:f:";
#endif

    init_prelogging ();		/* setup syslog */
    Myname = (Myname = rindex (*argv, '/')) == NULL ? *argv : Myname+1;

    while ((i = getopt (argc, argv, options)) != EOF) {
	switch (i) {
	    case 'd':
		/*
		 * No more than one -d flag.
		 */
		d_flag++;
		if (d_flag > 1) 
		    Usage ();
		debug = str2int (optarg);
		/* 
		 * MAXDEBUGLEVEL levels of debug.
		 */
		if (debug < 0
#ifndef DEBUG
			|| debug > MAXDEBUGLEVEL
#endif
			) {
		    advise (SLOG_NONE, MSGSTR(MS_SNMPD, SNMPD_6,
		                "invalid debug level: %s"), optarg);
		    Usage ();
		}
		debug_default = debug;
		break;

#ifdef DEBUG
	    case 'p':
		/*
		 * No more than one -p flag.
		 */
		p_flag++;
		if (p_flag > 1) 
		    Usage ();
		udp_port = str2int (optarg);
		if (udp_port < 1) {
		    advise (SLOG_NONE, MSGSTR(MS_SNMPD, SNMPD_7,
		                              "invalid port: %s"), optarg);
		    Usage ();
		}
		break;

	    case 'P':
		/*
		 * No more than one -P flag.
		 */
		P_flag++;
		if (P_flag > 1) 
		    Usage ();
		smux_port = str2int (optarg);
		if (smux_port < 1) {
		    advise (SLOG_NONE, MSGSTR(MS_SNMPD, SNMPD_8,
		                              "invalid smux_port: %s"), optarg);
		    Usage ();
		}
		break;

	    case 's':
		/*
		 * No more than one -s flag.
		 */
		s_flag++;
		if (s_flag > 1) 
		    Usage ();
		smux_enabled = NOTOK;
		break;
#endif  /* DEBUG */

	    case 'c':
		/*
		 * No more than one -c flag.
		 */
		c_flag++;
		if (c_flag > 1) 
		    Usage ();
		if (*optarg == '-') {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_9,
		                "invalid configfile: %s"), optarg);
		    Usage ();
		}
		setconfig (optarg);
		break;

	    case 'f':
		/*
		 * No more than one -f flag.
		 */
		f_flag++;
		if (f_flag > 1) 
		    Usage ();
		init_log_struct (&log_cmdline, optarg, OK);
		break;

	    default:
		Usage ();
	}
    }
    if (optind < argc)
	Usage ();
}


/*
 * NAME: Usage
 *
 * FUNCTION: Display snmpd usage
 */
void
Usage ()
{
    /*
     * Call advise() even if under SRC control because syslogd may be logging.
     * advise() messages will NOT be displayed by SRC.
     */

#ifdef DEBUG
    /*
     * For internal debugging purposes.
     */
    advise (SLOG_NONE, "Usage: %s [-c config_file] [-d level] [-f logfile] [-p SNMP_port]\n\t\t\t[-P SMUX_port] [-s]", Myname);
#else
    /*
     * The real usage.
     */
     advise (SLOG_NONE, MSGSTR(MS_SNMPD, SNMPD_10,
	   "Usage: %s [-c config_file] [-d level] [-f logfile]"), 
	   Myname);
#endif

    snmpd_exit (1, FALSE);
}

/*
 *    FUNTCION: snmp_receive_signal
 *		snmpd's signal handler.
 *    
 *    NOTE:  Data type SFD is defined in ../com/inc/isode/manifest.h.
 */
static SFD
snmp_receive_signal (sig, code, scp)
int sig, code;
struct sigcontext *scp;
{
    if (debug)
        advise (SLOG_DEBUG, MSGSTR(MS_SNMPD, SNMPD_11,
                "processing signal %d"), sig);

    switch (sig) {

	/* attempt a graceful shutdown */
	case SIGTERM:
	case SIGINT:
	    snmpd_exit (0, 1);
	    /* NOTREACHED */

	/* toggle logging */
	case SIGUSR1:
	    if (logging -> l_filename == NULLCP) {
	        advise (SLOG_NOTICE, MSGSTR(MS_SNMPD, SNMPD_12,
	                "%s is not configured for logging"), 
			Myname);
		break;
	    }
	    if (logging -> l_enabled == OK)
		stop_log (logging -> l_filename, TRUE);
	    else
		start_log (TRUE);
	    break;

	/* re-initialize */
	case SIGHUP:
	    refresh ();
	    break;

        case SIGALRM:
	    {
              struct interface *is;
              struct interface *is2;
	      int i;

	      is2 = is = iftimers; 
	      while (is -> time_next && is -> time_next -> timeout > 0)
		is = is -> time_next;

              while (is2 != is) {
                is2 -> timeout -= is -> timeout;
                is2 = is2 -> time_next; 
              }
	      is -> timeout = 0;
	      is -> flush_cache = 1;
	      is -> flush_rcv = 1;
	      if (is -> datarcvtable)
		  free (is -> datarcvtable);
	      is -> datarcvtable = NULL;
               
              is = iftimers;
	      while (is -> time_next && is -> time_next -> timeout > 0)
		is = is -> time_next;

              if ((i = alarm(is -> timeout)) == NOTOK)
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_27,
		       "snmp_receive_signal: Set timer failed."));
              else if (i != 0)
		advise(SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_28, 
       "snmp_receive_signal: Set timer is reseting an existing timer."));

              if (is -> timeout != 0) is = is -> time_next;
              while (is && is != is2) {
		is -> flush_cache = 1;
		is -> flush_rcv = 1;
	        if (is -> datarcvtable)
		    free (is -> datarcvtable);
	        is -> datarcvtable = NULL;
		is -> timeout = 0;
		is = is -> time_next;
              }
	    }
	    signal(SIGALRM, snmp_receive_signal);
	    break;
    }
#ifdef SFDINT
    return(0);
#endif
}

/* 
 * NAME: envinit ()
 *
 * FUNCTION: Initial agent's working environment.
 */
void
envinit ()
{
    int sd, t;
    FILE *fp;
    char file[40];
    struct sigvec vec, ovec;
    int *ip;
    static int signals[] = {
	SIGTERM,
	SIGUSR1,
	SIGINT,
	SIGHUP,
	SIGALRM,
	0
    };
    char *odm_path = -1;

    if (shouldfork () == OK) {
	switch (fork ()) {
	    case NOTOK:
		adios (MSGSTR(MS_SNMPD, SNMPD_22, "cannot fork"));
	    case OK:
		break;
	    default:
		sleep (1); 	 /* timing... */
		_exit (0);
	}

	(void) chdir ("/");
	if ((sd = open ("/dev/null", O_RDWR)) == NOTOK) {
	    adios (MSGSTR(MS_SNMPD, SNMPD_23, "cannot open /dev/null"));
	}
	if (sd != 0)
	    (void) dup2 (sd, 0), (void) close (sd);
	(void) dup2 (0, 1);
#ifdef DEBUG
	if (log_cmdline.l_logfp != stderr)
#endif
	(void) dup2 (0, 2);

	/* Remove our association with a controling tty */
	if ((t = open("/dev/tty", O_RDWR)) >= 0) {
	    (void) ioctl(t, TIOCNOTTY, (char *) NULL);
	    (void) close(t);
	}

#ifdef _POWER
	setsid ();
#else
#if	defined(RT) && defined(AIX)
	setpgrp ();
#else
	setpgrp (0,0);
#endif	 /* defined(RT) && defined(AIX) */
#endif	 /* _POWER */
    }

    init_logging ();	/* initialize *real* logging */

    mypid = getpid ();

    if (init_objects () == NOTOK)	/* init internal database */
	adios (MSGSTR(MS_SNMPD, SNMPD_24, "cannot initialize mib objects"));

#if	defined(_POWER) && !defined(SNMPSD)
    if ((OdmInit = odm_initialize ()) == NOTOK)
      advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_13,
              "odm_initialize failed"));
    else  {
      odm_path = odm_set_path ("/etc/objrepos");

      if (odm_path != -1)
          free(odm_path);
    }
#endif

    /* MIB variable initialization */

    init_mib ();	/* init kernel mib structure pointers */

    /* Internet-standard MIB groups */
    init_view ();	/* experimental view group...(plus internal usage of) */
    init_snmp ();	/* snmp group */
    init_smux ();	/* smux extension group */

#ifndef SNMPSD
    init_system ();	/* system group */
    init_icmp ();	/* icmp group */
    init_tcp ();	/* tcp group */
    init_udp ();	/* udp group */

    init_generic();  
    init_ethernet();  
    init_tokenring();  
    init_fddi();	
    init_interfaces ();	

    init_ip ();
#ifdef UNIXGROUP
    init_unix ();	/* unix netstat mib extensions, in unix_g.c */
#endif /* UNIXGROUP */

#ifdef LOCAL
    init_ibm ();	/* ibm agent build_info extensions */
#endif /* LOCAL */
#endif /* SNMPSD */

    readconfig (PASS2);	/* read snmpd.conf config file */

    fin_view ();

    fin_mib ();		/* check mib variables and set boot time */

    start_trap ();

    start_view ();

    /* initialize io functions */
    if ((Nd = init_io (udp_port, INADDR_ANY)) == NOTOK)
	adios(MSGSTR(MS_SNMPD, SNMPD_25, "init_io failed: %s"),strerror(errno));
    set_maxpacket ();

#ifdef SMUX
#ifdef DEBUG
    if (smux_enabled == NOTOK)
	advise (SLOG_NOTICE, "SMUX disabled");
    else
#endif	/* DEBUG */
    if (init_smuxio (smux_port) == NOTOK) 
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_SNMPD, SNMPD_14,
                "init_smuxio failed: %s"), strerror(errno));
#endif	/* SMUX */

    /* Set up signals to block */
    for (ip = signals; *ip; ip++) 
	snmp_signal_mask |= sigmask(*ip);

    /* Setup signal processing */
    bzero((char *)&vec, sizeof(vec));
    vec.sv_mask = snmp_signal_mask;
#ifdef _ANSI_C_SOURCE
    vec.sv_handler = (void (*)(int))snmp_receive_signal;
#else /* !_ANSI_C_SOURCE */
    vec.sv_handler = snmp_receive_signal;
#endif /* _ANSI_C_SOURCE */

    for (ip = signals; *ip; ip++) 
	if (sigvec(*ip, &vec, &ovec)) 
	    adios (MSGSTR(MS_SNMPD, SNMPD_26, "sigvec failed for sig %d: %s"), 
	           *ip, strerror(errno));

    /* only do this for production level work. */
    (void) sprintf (file, "/etc/%s.pid", Myname);
    if (fp = fopen (file, "w")) {
	(void) fprintf (fp, "%d\n", mypid);
	(void) fclose (fp);
    }

    advise (SLOG_NOTICE, MSGSTR(MS_SNMPD, SNMPD_15, 
            "%s (%d) is starting"), Myname, mypid);

}

#ifdef	SNMPSD
/* stub */
set_interface () {}
#endif


/*
 * NAME: snmpd_exit ()
 *
 * FUNCTION: cleans up and exits snmpd:
 *           * shuts down smux clients,
 *           * closes log file if it is open,
 */
void
snmpd_exit (exit_code, write_msg) 
int  exit_code;  /* The exit code on which to exit snmpd.       */
int  write_msg;  /* Whether or not to write the termination message. */ 
{
    struct fdinfo *fi;
#ifdef SMUX
#ifndef RT
#ifdef _POWER
    struct linger l = {1, 0};   /* blow away the connection after close */
#else 
    linger l = {1, 0};	        /* blow away the connection after close */
#endif /* POWER */
#else /* RT */
    struct linger l;	        /* blow away the connection after close */
    l.l_onoff = 1;
    l.l_linger = 0;
#endif /* RT */
#endif

#if	defined(_POWER) && !defined(SNMPSD)
    odm_terminate ();	/* close odm. */
#endif

#ifdef SMUX
    /*
     * closeout any SMUX peers
     */
    fi = type2fi (FI_SMUX_CLIENT, (struct fdinfo *) NULL);
    while (fi) {
	if (setsockopt(fi -> fi_fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) 
		&& debug)
	    advise (SLOG_DEBUG, MSGSTR(MS_SNMPD, SNMPD_16,
		    "setsockopt SO_LINGER failed on SMUX_CLIENT %d, errno %d"),
		    fi -> fi_fd, errno);
	shutdown (fi -> fi_fd, 2);
	close (fi -> fi_fd);

	fi = fi -> fi_next ? 
		type2fi (FI_SMUX_CLIENT, fi -> fi_next) : fi -> fi_next;
    }

    /*
     * closeout the main SMUX port (199)
     */
    if ((fi = type2fi (FI_SMUX, (struct fdinfo *) NULL)) != NULL) {
	if (setsockopt(fi -> fi_fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l))
		&& debug)
	    advise (SLOG_DEBUG, MSGSTR(MS_SNMPD, SNMPD_17,
		    "setsockopt SO_LINGER failed on SMUX %d, errno %d"),
		    fi -> fi_fd, errno);
	shutdown (fi -> fi_fd, 2);
	close (fi -> fi_fd);
    }
#endif /* SMUX */

    /*
     * The final cleanup and grande finale.
     */
    if (write_msg)
	advise (SLOG_NOTICE, MSGSTR(MS_SNMPD, SNMPD_18,
		"%s (%d) is terminating"), Myname, mypid);
    if (logging -> l_logfp != stderr && logging -> l_enabled == OK)
	stop_log (logging -> l_filename, TRUE);

#ifdef _POWER
    catclose(snmpcatd);
#endif /* _POWER */

    exit (exit_code);
}
