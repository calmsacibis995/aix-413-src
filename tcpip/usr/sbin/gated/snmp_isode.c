static char sccsid[] = "@(#)24	1.1  src/tcpip/usr/sbin/gated/snmp_isode.c, tcprouting, tcpip411, GOLD410 12/6/93 17:27:34";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: TREE_LIST
 *		TREE_LIST_END
 *		_PROTOTYPE
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF4
 *		__PF5
 *		snmp_varbinds_build1055
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * snmp_isode.c,v 1.18.2.3 1993/08/27 22:28:42 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_ISODE_SNMP
#include "include.h"

#if	defined(PROTO_ISODE_SNMP)
#include "inet.h"
#include "snmp_isode.h"

/*
 *	NOTES:
 *		general startup flow....
 *		initialize smux tcp port.  When the socket is
 *		ready for writing, kick off the registration
 *		process.  
 */

int debug = 0;
int snmp_debug = 0;
int snmp_quantum = 0;
int doing_snmp;
flag_t snmp_trace_flags;
pref_t snmp_preference;
u_short	snmp_port;
OID snmp_nullSpecific = NULLOID;

static struct smuxEntry *se = NULL;
static char my_desc[LINE_MAX];
static struct type_SNMP_VarBindList *savedVarBinds = NULL;
static task *snmp_task;

static struct snmp_tree snmp_trees = {
    &snmp_trees,
    &snmp_trees
};
static struct snmp_tree *snmp_tree_next;


#define	TREE_LIST(srp, tree)	for (srp = (tree)->r_forw; srp != &snmp_trees; srp = srp->r_forw)
#define	TREE_LIST_END(srp, tree)


static const bits snmp_flag_bits[] = {
    { SMUX_TREE_REGISTER,	"Registered" },
    { SMUX_TREE_ACTION,		"Action" },
    { SMUX_TREE_OBJECTS,	"Objects" },
    { 0,			NULL }
};


static const bits snmp_error_bits[] = {
    { int_SNMP_error__status_noError,		"noError" },
    { int_SNMP_error__status_tooBig,		"tooBig" },
    { int_SNMP_error__status_noSuchName,	"noSuchName" },
    { int_SNMP_error__status_badValue,		"badValue" },
    { int_SNMP_error__status_readOnly,		"readOnly" },
    { int_SNMP_error__status_genErr,		"genErr" }
};


static const bits snmp_close_bits[] = {
    { int_SNMP_ClosePDU_goingDown,		"goingDown" },
    { int_SNMP_ClosePDU_unsupportedVersion,	"unsupportedVersion" },
    { int_SNMP_ClosePDU_packetFormat,		"packetFormat" },
    { int_SNMP_ClosePDU_protocolError,		"protocolError" },
    { int_SNMP_ClosePDU_internalError,		"internalError" },
    { int_SNMP_ClosePDU_authenticationFailure,	"authenticationFailure" },
};

static void
snmp_tree_reset __PF0(void)
{
    register struct snmp_tree *srp;
    
    /* reset registration bits on tree(s) */
    TREE_LIST(srp, &snmp_trees) {
	BIT_RESET(srp->r_flags, SMUX_TREE_REGISTER);
	BIT_SET(srp->r_flags, SMUX_TREE_ACTION);
    } TREE_LIST_END(srp, &snmp_trees) ;

    snmp_tree_next = (struct snmp_tree *) 0;
}


static void
snmp_close __PF2(tp, task *,
		 reason, int)
{
    tracef("smux_close: SEND close %s",
	   trace_state(snmp_close_bits, reason));

    if (smux_close(reason) == NOTOK) {
	trace(TR_SNMP, 0, ": Erorr: %s[%s], %m",
	      smux_error(smux_errno),
	      smux_info);
    } else {
	trace(TR_SNMP, 0, NULL);
    }
}


static void
snmp_terminate __PF1(tp, task *)
{
    if (tp->task_socket >= 0) {
	snmp_close(tp, goingDown);
	task_reset_socket(tp);
    }
    task_delete(tp);
    snmp_task = (task *) 0;

    snmp_tree_reset();

    /* maybe the commit never arrived */
    if (savedVarBinds) {
	free_SNMP_VarBindList(savedVarBinds);
	savedVarBinds = (struct type_SNMP_VarBindList *) 0;
    }
}


static void
snmp_response __PF2(tp, task *,
		    pdu, register struct type_SNMP_GetResponse__PDU *)
{
    if (smux_response(pdu) == NOTOK) {
	trace(TR_SNMP, 0, "snmp_response: smux_response: Error: %s [%s], %m",
	      smux_error(smux_errno),
	      smux_info);
	snmp_restart(tp);
    }
}


static void
snmp_method __PF5(tp, task *,
		  op, const char *,
		  pdu, register struct type_SNMP_GetResponse__PDU *,
		  vp, register struct type_SNMP_VarBindList *,
		  offset, int)
{
    int	idx;
    int	status;
    int done;
    int error__status = int_SNMP_error__status_noError;
    object_instance ois;
    IFP	    method = (IFP) 0;

    idx = 0;
    for (done = 0; vp && !done; vp = vp->next) {
	register OI	oi;
	register OT	ot;
	register struct type_SNMP_VarBind *v = vp->VarBind;

	idx++;

	TRACE_UPDATE(TR_SNMP) {
	    trace(TR_SNMP, 0, "snmp_method: processing %s %s",
		  op,
		  oid2ode(v->name));
	}
	if (offset == type_SNMP_SMUX__PDUs_get__next__request) {
	    if ((oi = name2inst(v->name)) == NULLOI && (oi = next2inst(v->name)) == NULLOI) {
		error__status = int_SNMP_error__status_noSuchName;
		break;		/* fall out of for loop */
	    }

	    if ((ot = oi->oi_type)->ot_getfnx == NULLIFP) {
		goto get_next;
	    }
	} else {
	    if ((oi = name2inst(v->name)) == NULLOI) {
		error__status = int_SNMP_error__status_noSuchName;
		break;		/* fall out of for loop */
	    }
	    ot = oi->oi_type;
	    if (offset == type_SNMP_SMUX__PDUs_get__request) {	/* GET */
		if (ot->ot_getfnx == NULLIFP) {
		    error__status = int_SNMP_error__status_noSuchName;
		    break;		/* fall out of for loop */
		}
	    } else {						/* SET */
		if (ot->ot_setfnx == NULLIFP) {
#ifdef notdef
		    /* internet theory on this case is that if we
		     * do not have WRITE permission on this object,
		     * then it is not in our "VIEW", and therefor,
		     * we must return noSuchName.
		     * We have left this code here for informational
		     * purposes. (RLF)
		     */

		    if (ot->ot_access & OT_RDONLY || ot->ot_access & OT_WRONLY) {
			error__status = int_SNMP_error__status_readOnly;
		    } else {
			error__status = int_SNMP_error__status_noSuchName;
 		    }
#else	/* notdef */
		    error__status = int_SNMP_error__status_noSuchName;
#endif	/* notdef */
		    break;		/* fall out of for loop */
		}
	    }
	}

try_again: ;
	switch (offset) {
	case type_SNMP_SMUX__PDUs_get__request:
	    if (!(ot->ot_access & OT_RDONLY)) {
		error__status = int_SNMP_error__status_noSuchName;
		done = 1;	/* fall out of for loop */
		continue;	/* drop out of do-while */
	    }
	    method = ot->ot_getfnx;
	    break;
		
	case type_SNMP_SMUX__PDUs_get__next__request:
	    if (!(ot->ot_access & OT_RDONLY)) {
		goto get_next;
	    }
	    method = ot->ot_getfnx;
	    break;
		
	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	case type_SNMP_SMUX__PDUs_set__request:
	    if (!(ot->ot_access & OT_WRONLY)) {
#ifdef notdef
		if (!(ot->ot_access & OT_RDONLY)) {
		    error__status = int_SNMP_error__status_noSuchName;
		} else  {
		    error__status = int_SNMP_error__status_readOnly;
 		}
#else	/* notdef */
		error__status = int_SNMP_error__status_readOnly;
#endif	/* notdef */
		done = 1;	/* fall out of for loop */
		continue;	/* drop out of do-while */
	    }
	    method = ot->ot_setfnx;
	    break;
	}

	status = (*method)(oi, v, offset);
	switch (status) {
	case NOTOK:	    /* get-next wants a bump */
	get_next:
	    oi = &ois;
	    for (;;) {
		if ((ot = ot->ot_next) == NULLOT) {
		    error__status = int_SNMP_error__status_noSuchName;
		    done = 1;
		    break;
		}
		oi->oi_name = (oi->oi_type = ot)->ot_name;
		if (ot->ot_getfnx) {
		    goto try_again;
		}
	    }
	    break;

	case int_SNMP_error__status_noError:
	    break;
	    
	default:
	    error__status = status;
	    done = 1;
	    break;
	}

	TRACE_UPDATE(TR_SNMP) {
	    tracef("snmp_method: completed %s",
		   oid2ode(v->name));

	    switch (error__status) {
	    default:
		tracef(": %s",
		       trace_state(snmp_error_bits, error__status));
		break;
	    
	    case int_SNMP_error__status_noError:
		/* XXX - OID and print value */
		break;
	    }
	    trace(TR_SNMP, 0, NULL);
	}
    }

    
    switch (offset) {
    case type_SNMP_PDUs_rollback:
    case type_SNMP_PDUs_commit:
	/* No response expected */
	break;

    default:
	/* Generate a response */
	switch (error__status) {
	case int_SNMP_error__status_noError:
	    pdu->error__index = 0;
	    break;

	default:
	    pdu->error__index = idx;
	}
	pdu->error__status = error__status;

	snmp_response(tp, pdu);
    }

}


/*
 *	send a smux register request.
 *	returns:
 *		OK 	== a register request was sent
 *		NOTOK	== nothing left to register, <or>
 *			problems sending the request.
 *			Check smux_state to know which failed.
 *			smux_state & SMUX_CONNECTED == nothing to register
 */

static int
snmp_register __PF2(tp, task *,
		    failure, int)
{
    int mode = delete;
    const char *cmode = "delete";

 try_again:
    /* Find something that needs to be registered */
    if (snmp_tree_next) {
	if (failure) {
	    /* Registration failed, remove it from the queue */
	    register struct snmp_tree *drp = snmp_tree_next;

	    snmp_tree_next = drp->r_back;
	    remque((struct qelem *) drp);
	    drp->r_forw = drp->r_back = (struct snmp_tree *) 0;
	} else {
	    BIT_RESET(snmp_tree_next->r_flags, SMUX_TREE_ACTION);
	    BIT_FLIP(snmp_tree_next->r_flags, SMUX_TREE_REGISTER);
	}
    } else {
	snmp_tree_next = &snmp_trees;
    }

    /* Find some work to do */
    TREE_LIST(snmp_tree_next, snmp_tree_next) {
	if (BIT_TEST(snmp_tree_next->r_flags, SMUX_TREE_ACTION)) {
	    break;
	}
    } TREE_LIST_END(snmp_tree_next, snmp_tree_next) ;
    
    if (!snmp_tree_next->r_text) {
	/* No more work to do */
	snmp_tree_next = (struct snmp_tree *) 0;
	return OK;
    }

    /* How is this being registered? */
    if (!BIT_TEST(snmp_tree_next->r_flags, SMUX_TREE_REGISTER)) {
	mode = snmp_tree_next->r_mode;
	cmode = snmp_tree_next->r_mode == readWrite ? "readWrite" : "readOnly";
    }

    tracef("snmp_register: REGISTER %s %s",
	   cmode,
	   oid2ode(snmp_tree_next->r_name));

    if (BIT_TEST(task_state, TASKS_TEST)) {
	trace(TR_SNMP, 0, NULL);
	goto try_again;
    }
    
    if (smux_register(snmp_tree_next->r_name,
		      -1,
		      mode) == NOTOK) {
	trace(TR_ALL, LOG_ERR, ": %s [%s]",
	      smux_error(smux_errno),
	      smux_info);

	snmp_restart(tp);
	return NOTOK;
    }

    trace(TR_SNMP, 0, NULL);
    return OK;
}


static void
snmp_recv __PF1(tp, task *)
{
    struct type_SNMP_SMUX__PDUs *event;

    if (smux_wait(&event, NOTOK) == NOTOK) {
	if (smux_errno == inProgress) {
	    return;
	}
	
	trace(TR_ALL, LOG_ERR, "snmp_recv: smux_wait: %s [%s] (%m)",
	      smux_error(smux_errno),
	      smux_info);
	snmp_restart(tp);
	return;
    }

    switch (event->offset) {
    case type_SNMP_SMUX__PDUs_registerResponse:
	{
	    struct type_SNMP_RRspPDU *rsp = event->un.registerResponse;

	    tracef("snmp_recv: registerResponse");
	    if (snmp_tree_next && snmp_tree_next->r_name) {
		tracef(" (%s)",
		       oid2ode(snmp_tree_next->r_name));
	    }
	    trace(TR_SNMP, 0, ": %s",
		  trace_state(snmp_error_bits, rsp->parm));

	    snmp_register(tp, rsp->parm == int_SNMP_RRspPDU_failure);
	}
	break;

    case type_SNMP_SMUX__PDUs_get__request:
	snmp_quantum = event->un.get__request->request__id;
	snmp_method(tp,
		    "get",
		    event->un.get__response, 
		    event->un.get__request->variable__bindings,
		    event->offset);
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	snmp_quantum = event->un.get__next__request->request__id;
	snmp_method(tp,
		    "getNext",
		    event->un.get__response, 
		    event->un.get__next__request->variable__bindings,
		    event->offset);
	break;

    case type_SNMP_SMUX__PDUs_set__request:
	snmp_quantum = event->un.set__request->request__id;
	snmp_method(tp,
		    "set",
		    event->un.get__response, 
		    event->un.set__request->variable__bindings,
		    event->offset);

	/* maybe the commit never arrived */
	if (savedVarBinds) {
	    free_SNMP_VarBindList(savedVarBinds);
	    savedVarBinds = (struct type_SNMP_VarBindList *) 0;
	}

	/* save the varBinds till the commit/rollback arrives */
	savedVarBinds = event->un.get__request->variable__bindings;

	/* dont let smux_wait() free these varBinds */
	event->un.get__request->variable__bindings = (struct type_SNMP_VarBindList *) 0;
	break;

    case type_SNMP_SMUX__PDUs_commitOrRollback:
	{
	    int commitOrRollback = event->un.commitOrRollback->parm;

	    snmp_method(tp,
			commitOrRollback == int_SNMP_SOutPDU_commit ? "commit" : "rollback",
			NULL, savedVarBinds,
			commitOrRollback == int_SNMP_SOutPDU_commit ?
			type_SNMP_PDUs_commit : type_SNMP_PDUs_rollback);
	    free_SNMP_VarBindList(savedVarBinds);
	    savedVarBinds = NULL;
	    break;
	}
	    
    case type_SNMP_SMUX__PDUs_close:
	trace(TR_SNMP, 0, "snmp_recv: close: %s",
	      trace_state(snmp_close_bits, event->un.close->parm));
	snmp_restart(tp);
	break;

    case type_SNMP_SMUX__PDUs_simple:
    case type_SNMP_SMUX__PDUs_registerRequest:
    case type_SNMP_SMUX__PDUs_get__response:
    case type_SNMP_SMUX__PDUs_trap:
	trace(TR_SNMP, 0, "snmp_recv: unexpectedOperation: %d",
	      event->offset);
	snmp_close(tp, protocolError);
	snmp_restart(tp);
	break;

    default: 
	trace(TR_SNMP, 0, "snmp_recv: badOperation: %d",
	      event->offset);
	snmp_close(tp, protocolError);
	snmp_restart(tp);
	break;
    }
}


/*
 *	startup the registration process, stop paying attention to
 *	writes on the socket.
 */
static void
snmp_startup __PF1(tp, task *)
{
    tracef("snmp_startup: OPEN %s \"%s\"", 
	   oid2ode(&se->se_identity),
	   se->se_name);

    if (smux_simple_open(&se->se_identity,
			 my_desc,
			 se->se_password,
			 strlen(se->se_password)) == NOTOK) {
	trace(TR_ALL, LOG_ERR, ": smux_simple_open: %s [%s]",
	      smux_error(smux_errno),
	      smux_info);
	snmp_restart(tp);
	return;
    }

    trace(TR_SNMP, 0, NULL);

    if (snmp_register(tp, FALSE) == NOTOK) {
	return;
    }

    tp->task_connect = (void (*) ()) 0;
    BIT_RESET(tp->task_flags, TASKF_CONNECT);

    task_set_socket(tp, tp->task_socket);
}


/* 
 * connect to the SMUX tcp port.
 * Initializing tcp port, and smux library internals are
 * intermixed here... so we let the smux library handle everything.
 * { instead of using task_get_socket() to open the port, which
 *   would be the proper way to do things.... }.
 */
static int
snmp_connect __PF1(tp, task *)
{
    tracef("snmp_connect: CONNECT");
    if (snmp_debug) {
	tracef(" (debug)");
    }

    if (BIT_TEST(task_state, TASKS_TEST)) {
	tp->task_socket = task_get_socket(PF_INET, SOCK_STREAM, 0);
	assert(tp->task_socket >= 0);
    } else {
	if ((tp->task_socket = smux_init(debug = snmp_debug)) < 0) {
	    trace(TR_ALL, 0, ": smux_init: %s [%s] (%m)",
		  smux_error(smux_errno),
		  smux_info);
	    return NOTOK;
	}
	tp->task_recv = snmp_recv;
	tp->task_connect = snmp_startup;
	BIT_SET(tp->task_flags, TASKF_CONNECT);
    }

    trace(TR_SNMP, 0, NULL);
    
    task_set_socket(tp, tp->task_socket);

    if (TASK_TIMER(tp, SMUX_TIMER_STARTUP)) {
	(void) timer_delete(TASK_TIMER(tp, SMUX_TIMER_STARTUP));
    }

    return OK;
}


static void
snmp_job __PF2(tip, timer *,
	       interval, time_t)
{
    snmp_connect(tip->timer_task);
}


static void
snmp_cleanup __PF1(tp, task *)
{
    /* Nothing really to do here */
}


void
snmp_restart __PF1(tp, task *)
{
    trace(TR_SNMP, 0, "snmp_restart: RESTART");

    /* Reset things */
    if (tp->task_socket != -1) {
	task_reset_socket(tp);
    }
    tp->task_recv = tp->task_connect = (void (*) ()) 0;
    BIT_RESET(tp->task_flags, TASKF_CONNECT);

    snmp_tree_reset();

    /* a constant 60sec timer, to try to startup smux again */
    if (TASK_TIMER(tp, SMUX_TIMER_STARTUP)) {
	timer_set(TASK_TIMER(tp, SMUX_TIMER_STARTUP),
		  (time_t) 60,
		  (time_t) 0);
    } else {
	(void) timer_create(tp,
			    SMUX_TIMER_STARTUP,
			    "Startup",
			    TIMERF_ABSOLUTE, 
			    (time_t) 60,
			    (time_t) 0,
			    snmp_job,
			    (void_t) 0);
    }
}


static void
snmp_dump __PF2(tp, task *,
		fp, FILE *)
{
    struct snmp_tree *tree;

    (void) fprintf(fp,
		   "\tDebugging: %d\tPort: %d\tPreference: %d\n",
		   snmp_debug,
		   ntohs(snmp_port),
		   snmp_preference);

    (void) fprintf(fp,
		   "\tMIB trees active:\n\n");

    TREE_LIST(tree, &snmp_trees) {
	(void) fprintf(fp,
		       "\t%s\t%s\tflags: <%s>\n",
		       tree->r_mode == readWrite ? "readWrite" : "readOnly",
		       tree->r_text,
		       trace_bits(snmp_flag_bits, tree->r_flags));
	if (BIT_TEST(tree->r_flags, SMUX_TREE_OBJECTS)) {
	    register const struct object_table *op = tree->r_table;
	    
	    while (op->ot_object) {
		(void) fprintf(fp,
			       "\t\t%s\t%s\n",
			       op->ot_setfunc ? "readWrite" : "readOnly",
			       op->ot_object);
		op++;
	    }
	    (void) fprintf(fp, "\n");
	}
    } TREE_LIST_END(tree, &snmp_trees) ;
}


void
snmp_var_init __PF0(void)
{
    doing_snmp = TRUE;
    snmp_preference = RTPREF_SNMP;
    snmp_debug = FALSE;
    snmp_port = 0;
}


/*
 *	from-the-top initialization of the SMUX task.
 *	should be called 'smux_init', but conflicts with the smux 
 *	library initialization routine...
 */
void
snmp_init __PF0(void)
{
    static struct servent *sp;
    static int once_only = TRUE;

    /* 
     * should really be more graceful here.
     * for now, if a re-init, shutdown smux first, 
     * then start all over.
     */
    if (snmp_task) {
	snmp_terminate(snmp_task);	/* shut it down */
    }

    if (!doing_snmp) {
	/* We've been turned off! */
	return;
    }
    
    /* lookup gated in snmpd.peers. */
    if ((se = getsmuxEntrybyname("gated")) == NULL) {
	trace(TR_SNMP, 0, "no SMUX entry for %s.  Shutting down SMUX",
	      "gated");
	doing_snmp = 0;
	return;
    }

    /* one-time configuration */
    if (once_only) {
	once_only = FALSE;
#if	defined(IBM_LIBSMUX)
	/* init internal database */
	if (init_objects() == NOTOK) {
	    trace(TR_SNMP, 0, 
		    "snmp_init: init_objects: %s.  Shutting down SMUX", 
		    PY_pepy);
	    doing_snmp = 0;
	    return;
	}
#else	/* defined(IBM_LIBSMUX) */
#ifdef	ISODE_SNMP_NODEFS
	if (readobjects("./gated.defs") == NOTOK &&
	    readobjects(_PATH_DEFS) == NOTOK &&
	    readobjects("gated.defs") == NOTOK) {
	    trace(TR_SNMP, 0,
		  "snmp_init: readobjects: %s.  Shutting down SMUX",
		  PY_pepy);
	    doing_snmp = 0;
	    return;
	}
#else	/* ISODE_SNMP_NODEFS */
	if (loadobjects((char *) 0) == NOTOK) {
	    trace(TR_SNMP, 0,
		  "snmp_init: loadobjects: %s.  Shutting down SMUX",
		  PY_pepy);
	    doing_snmp = 0;
	    return;
	}
#endif	/* ISODE_SNMP_NODEFS */
#endif	/* defined(IBM_LIBSMUX) */

	if ((snmp_nullSpecific = text2oid("0.0")) == NULLOID) {
	    trace(TR_SNMP, 0, "snmp_init: text2oid(\"0.0\") failed!");
	    doing_snmp = 0;
	    return;
	}

	sprintf(my_desc, "SMUX GATED version %s, built %s",
		gated_version, 
		build_date);
    }

    /* setup a task to handle smux */
    snmp_task = task_alloc("SMUX", TASKPRI_NETMGMT);
    snmp_task->task_addr = sockdup(inet_addr_loopback);
    if (snmp_port) {
	sock2port(snmp_task->task_addr) = snmp_port;
    } else if (sp = getservbyname("smux", "tcp")) {
	sock2port(snmp_task->task_addr) = sp->s_port;
    } else {
	trace(TR_ALL, LOG_ERR, "No service for SMUX available, using %d",
	      SMUX_PORT);
	sock2port(snmp_task->task_addr) = htons(SMUX_PORT);
    }
    snmp_task->task_terminate = snmp_terminate;
    snmp_task->task_dump = snmp_dump;
    snmp_task->task_trace_flags = snmp_trace_flags;
    snmp_task->task_rtproto = RTPROTO_SNMP;
    snmp_task->task_n_timers = SMUX_TIMER_MAX;
    snmp_task->task_cleanup = snmp_cleanup;

    if (!task_create(snmp_task)) {
	task_quit(errno);
    }

    if (snmp_connect(snmp_task) == NOTOK) {
	snmp_restart(snmp_task);
    }
}


void
snmp_trap __PF5(name, const char *,
		enterprise, OID,
		generic, int,
		specific, int,
		bindings, struct type_SNMP_VarBindList *)
{
    if (!snmp_task || snmp_task->task_socket == -1) {
	return;
    }

    if (!enterprise) {
	if (generic == int_SNMP_generic__trap_enterpriseSpecific) {
	    /* Can not generate enterprise specified trap with no enterprise */
	    return;
	}
	enterprise = &se->se_identity;
#if	!defined(SMUX_TRAP_HAS_ENTERPRISE) && !defined(IBM_6611)
    } else {
	/* Can not generate an enterprise specific trap */
	return;
#endif	/* !defined(SMUX_TRAP_HAS_ENTERPRISE) && !defined(IBM_6611) */
    }
    
    tracef("snmp_trap: TRAP enterprise %s type %s (generic %u specific %u)",
	   oid2ode(enterprise),
	   name,
	   generic,
	   specific);

#if	defined(SMUX_TRAP_HAS_ENTERPRISE)
#define	SMUX_TRAP(e, g, s, b)	smux_trap(e, g, s, b)
#else	/* SMUX_TRAP_HAS_ENTERPRISE */	   
#ifdef	IBM_6611
#define	SMUX_TRAP(e, g, s, b)	smux_trap_enterprise(g, s, b, e)
#else	/* IBM_6611 */
#define	SMUX_TRAP(e, g, s, b)	smux_trap(g, s, b)
#endif	/* IBM_6611 */
#endif	/* SMUX_TRAP_HAS_ENTERPRISE */

    if (SMUX_TRAP(enterprise, generic, specific, bindings) == NOTOK) {
	trace(TR_SNMP, LOG_ERR, " smux_trap: %s [%s] %m",
	      smux_error(smux_errno),
	      smux_info);
	snmp_restart(snmp_task);
	return;
    }

    trace(TR_SNMP, 0, NULL);
}


int	
oid2mediaddr __PF4(ip, register unsigned int *,
		   addr, register byte *,
		   len, int,
		   islen, int)
{
    register int i;

    if (islen) {
	len = *ip++;
    } else {
	len = len ? len : 4;		/* len, else ipaddress, is default */
    }

    for (i = len; i > 0; i--) {
	*addr++ = *ip++;
    }

    return len + (islen ? 1 : 0);
}


static int
snmp_tree_resolve __PF1(tree, struct snmp_tree *)
{
    register int fails = 0;
    register int count = 0;
    register OT ot;
    register struct object_table *op = tree->r_table;

    /* Translate tree name */
    ot = text2obj(tree->r_text);
    if (ot) {
	tree->r_name = ot->ot_name;

	TRACE_PROTO(TR_SNMP) {
	    trace(TR_SNMP, 0, "snmp_tree_resolve: RESOLVE %s OID %s",
		  tree->r_text,
		  sprintoid(tree->r_name));
	}
    } else {
	trace(TR_ALL, LOG_ERR, "snmp_tree_resolve: tree %s unable to resolve root",
	      tree->r_text);
	return TRUE;
    }

    /* Translate object names */
    while (op->ot_object) {
	count++;
	ot = op->ot_type = text2obj(op->ot_object);
	if (ot) {
	    ot->ot_getfnx = op->ot_getfunc;
	    if (op->ot_setfunc) {
		ot->ot_setfnx = op->ot_setfunc;
	    }
	    ot->ot_info = (caddr_t) op;

	    TRACE_PROTO(TR_SNMP) {
		trace(TR_SNMP, 0, "snmp_tree_resolve: RESOLVE %s OID %s",
		      op->ot_object,
		      sprintoid(ot->ot_name));
	    }
	} else {
	    trace(TR_ALL, LOG_ERR, "snmp_tree_resolve: tree %s unable to resolve object %s",
		  tree->r_text,
		  op->ot_object);
	    fails++;
	}
	    
	op++;
    }

    if (count > fails) {
	BIT_SET(tree->r_flags, SMUX_TREE_OBJECTS);
	TRACE_PROTO(TR_SNMP) {
	    trace(TR_SNMP, 0, "snmp_tree_resolve: RESOLVE tree %s %d of %d objects",
		  tree->r_text,
		  count - fails,
		  count);
	}
    } else {
	trace(TR_ALL, LOG_ERR, "snmp_tree_resolve: tree %s could not resolve any of %d objects",
	      tree->r_text,
	      count);

	return TRUE;
    }

    return FALSE;
}


void
snmp_tree_register __PF1(tree, struct snmp_tree *)
{
    if (!BIT_TEST(tree->r_flags, SMUX_TREE_OBJECTS)) {
	/* If this is the first reference to this tree, translate the names into OID's */

	if (snmp_tree_resolve(tree)) {
	    /* Could not resolve tree, ignore it */
	    return;
	}
    }

    if (BIT_TEST(tree->r_flags, SMUX_TREE_REGISTER)) {
	return;
    }
    
    trace(TR_SNMP, 0, "snmp_tree_register: ADD tree %s  mode %s",
	  tree->r_text,
	  tree->r_mode == readOnly ? "readOnly" : "readWrite");

    BIT_RESET(tree->r_flags, SMUX_TREE_REGISTER);
    BIT_SET(tree->r_flags, SMUX_TREE_ACTION);

    if (!tree->r_forw) {
	/* Insert into queue */
	insque((struct qelem *) tree, snmp_trees.r_back);
    }

    if (snmp_task && snmp_task->task_socket != -1 && !snmp_task->task_connect && !snmp_tree_next) {
	/* Start registration process */
	snmp_register(snmp_task, FALSE);
    }
}

void
snmp_tree_unregister __PF1(tree, struct snmp_tree *)
{
    trace(TR_SNMP, 0, "snmp_tree_unregister: DELETE tree %s",
	  tree->r_text);
    
    if (!BIT_TEST(tree->r_flags, SMUX_TREE_REGISTER)) {
	return;
    }

    BIT_SET(tree->r_flags, SMUX_TREE_ACTION);

    if (!tree->r_forw) {
	/* Insert into queue */
	insque((struct qelem *) tree, snmp_trees.r_back);
    }

    if (snmp_task && snmp_task->task_socket != -1 && !snmp_task->task_connect && !snmp_tree_next) {
	/* Start registration process */
	snmp_register(snmp_task, FALSE);
    }
}


void
snmp_last_free __PF1(last, unsigned int **)
{
	task_mem_free((task *) 0, (caddr_t) *last);
	*last = (unsigned int *) 0;
}


int
snmp_last_match __PF4(last, unsigned int **,
		      oid, register unsigned int *,
		      len, size_t,
		      isnext, int)
{
    register unsigned int *lp = *last;
    register unsigned int *ip = oid;
    size_t last_len;
    int last_isnext;
    unsigned int *llp;

    if (*last) {
	last_len = *lp++;

	if (last_len == len) {
	    last_isnext = *lp++;

	    if (last_isnext == isnext) {
		llp = lp + last_len;
		
		while (lp < llp) {
		    if (*lp++ != *ip++) {
			ip = oid;
			goto free_up;
		    }
		}

		return TRUE;
	    }
	}

    free_up:
	snmp_last_free(last);
    }

    /* XXX - Could we figure out the max size? */
    *last = lp = (unsigned int *) task_mem_malloc((task *) 0, (len + 2) * sizeof (int));
    llp = lp + len + 2;
    
    *lp++ = len;
    *lp++ = isnext;

    while (lp < llp) {
	*lp++ = *ip++;
    }

    return FALSE;
}


/**/

int
snmp_varbinds_build (n, vb, v, var_list, tree, build, vp)
int n;
struct type_SNMP_VarBindList *vb;
struct type_SNMP_VarBind * v;
int *var_list;
struct snmp_tree *tree;
_PROTOTYPE(build,
	   int,
	   (OI,
	    struct type_SNMP_VarBind *,
	    int,
	    void_t));
void_t vp;
{
    register int i;
    struct object_instance oia;
    OI oi = &oia;

    /* Clear the VarBinds and VarBindLists */
    for (i = 0; i < n; i++) {
	v[i].name = (struct type_SNMP_ObjectName *) 0;
	v[i].value = (struct type_SNMP_ObjectSyntax *) 0;
	vb[i].VarBind = (struct type_SNMP_VarBind *) 0;
	vb[i].next = (struct type_SNMP_VarBindList *) 0;
    }
    
    if (!BIT_TEST(tree->r_flags, SMUX_TREE_OBJECTS|SMUX_TREE_REGISTER)) {
	/* Objects not resolved or tree not registered */
	return NOTOK;
    }
    
    for (i = 0; i < n; i++, vb++, v++) {
	register const struct object_table *ot = &tree->r_table[var_list[i]];

	if (!ot->ot_type) {
	    /* MIB not inited */
	    return NOTOK;
	}
	oi->oi_type = ot->ot_type;
	oi->oi_name = oi->oi_type->ot_name;

	v->name = oi->oi_name;

	if (build(oi, v, ot->ot_info, vp) == NOTOK) {
	    return NOTOK;
	}

	vb->VarBind = v;
	if (i) {
	    vb[-1].next = vb;
	}
    }

    return OK;
}


void
snmp_varbinds_free __PF1(vb, register struct type_SNMP_VarBindList *)
{
    for (;
	 vb && vb->VarBind;
	 vb = vb->next) {
	free_SNMP_ObjectSyntax(vb->VarBind->value);
    }
}
#endif		/* defined(PROTO_ISODE_SNMP) */


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
