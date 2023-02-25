static char sccsid[] = "@(#)28	1.12  src/tcpip/usr/sbin/snmpinfo/snmpinfo.c, snmp, tcpip411, 9438B411a 9/21/94 09:27:30";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: main(), arginit(), mapUsage(), map_arginit(), info_arginit(),
 *            envinit(), Usage(), checkVars(), f_get(), f_get_next(), f_set(),
 *            f_dump(), trapmon_arginit(), TrapUsage(), interactive(), 
 *            getds(), snmploop(), getline(), ncols(), f_help(), f_compile()
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
 * FILE:	src/tcpip/usr/sbin/snmpinfo/snmpinfo.c
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
#include <isode/pepsy/SNMP-types.h>
#include <isode/snmp/objects.h>
#include <stdio.h>
#include <string.h>
#include <isode/snmp/logging.h>
#include "snmpd_msg.h"
#include <sys/param.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

#ifndef VERSION
#define VERSION "unknown"
#endif

#define SNMP_TRAP_PORT 162

/* FORWARD */
void arginit (), info_arginit (), trapmon_arginit (), map_arginit ();

/* EXTERNS */
extern int optind;
extern char *optarg;
extern errno;

extern int Verbose;
extern int max_SNMP_retries;
extern int SNMP_port;

extern void advise ();
extern struct type_SNMP_Message *new_message ();
extern IFP	o_dumpbits;
extern int	dump_bfr ();

extern int SNMP_timeout;

static	char	**op = NULLVP;	/* options (cmd line) */
static int func;		/* dispatch function to use (cmd line) */
int ontty;		/* attached to a tty? (interactive) */
char *prog_name; 	/* my name */

#ifdef LOCAL
int timing;             /* are we doing timing? */
char *odefs;		/* output defs file */
#endif /* LOCAL */
int nvars;		/* number of variables received */

char host[MAXHOSTNAMELEN+1];
static char *community;
static int X_flag = 0;

struct dispatch {
    char	*ds_name;	/* command name (interactive only) */
    IFP		ds_fnx;		/* dispatch */
    char	*ds_help;	/* help string */
};

extern	int	f_dump (), f_get (), f_get_next (), f_set (), f_help (),
		f_compile ();

struct dispatch dispatches[] = {
    "dump", f_dump,     "dump a portion of the MIB",
#define	DS_DUMP	0
    "get",  f_get,      "perform get operation",
#define DS_GET	1
    "help", f_help,     "print help information",
#define DS_HELP 2
    "next", f_get_next, "perform powerful get-next operation",
#define	DS_NEXT	3
    "set", f_set,       "perform set operation",
#define	DS_SET	4
    "compile", f_compile, "write compiled objects file",
#define	DS_COMPILE	5
    NULL
};

main (argc, argv)
int	argc;
char	*argv[];
{
    int rc;

#ifdef _POWER
    setlocale (LC_ALL,"");              /* Designates native locale */
    snmpcatd = catopen (MF_SNMPD,0);
#endif /* _POWER */

    SNMP_timeout = 15;

    arginit (argc, argv);
    envinit ();

    /* 
     * All options provided for on command line... 
     */
    if (op) {
	rc = (*dispatches[func].ds_fnx) (op);
#ifdef LOCAL
	if (timing)
	    stats ();
#endif /* LOCAL */
#ifdef _POWER
	catclose(snmpcatd);
#endif /* _POWER */
	exit (rc);
    }

    exit (interactive ());
}

/*
 * NAME: arginit
 *
 * FUNCTION: Checkout command-line options.
 */
void
arginit (argc, argv)
int argc;
char *argv[];
{
    prog_name = (prog_name = rindex (*argv, '/')) == NULL ? *argv : prog_name+1;

    o_advise = (IFP) advise;    /* for libsnmp.a */
    o_dumpbits = (IFP) dump_bfr;    /* for libsnmp.a */

    /* set isode printing from stdout to stderr */
    vsetfp (stderr, NULLCP);

    /* default is snmpinfo usage */
#ifdef LOCAL
    ontty = isatty (fileno (stdin));

    if (!strcmp (prog_name, "trapmon")) {
	trapmon_arginit (argc, argv);
	return;
    }
    else
#endif
    if (!strcmp (prog_name, "snmp_get") || !strcmp (prog_name, "snmp_next")
	    || !strcmp (prog_name, "snmp_set")) {
	map_arginit (argc, argv);
	return;
    }
    info_arginit (argc, argv);
}

/*
 * NAME: mapUsage
 *
 * FUNCTION: Display usage for mapping of snmp_set, snmp_get and snmp_next
 *	     commands.
 */
void
mapUsage ()
{

    if (strcmp (prog_name, "snmp_set")) {
	fprintf (stderr, MSGSTR(MS_SINFO, SINFO_1,
		 "Usage: %s [-d] host community var[.inst]...\n"), 
		 prog_name);
#ifdef _POWER
	catclose(snmpcatd);
#endif /* _POWER */
	exit (1);
    }
    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_2,
		"Usage: %s [-d] host community var[.inst][=value]...\n"), 
		prog_name);
#ifdef _POWER
    catclose(snmpcatd);
#endif /* _POWER */
    exit (1);
}

/*
 * NAME: map_arginit
 *
 * FUNCTION: Check out command line options for mapping of snmp_get, snmp_set, 
 * 	     and snmp_next commands to snmpinfo.
 */
void
map_arginit (argc, argv)
int argc;
char *argv[];
{
    int i, len, port;
    u_long addr;
    char **opt_list;
    char *tmp;
    int count = 3;
    char *pos;
    char *poseq;
    char *var;

    if (argc < 4 || (argv[1][0] == '-' && argc < 5)) 
	mapUsage ();

    /*
     * Save debug flag, host, and community name.
     */
    i = 1;
    if (strcmp (argv[i], "-d") == 0) {
	debug = 3;
	i++;
	count = 4;
    }
    strncpy (host, argv[i++], MAXHOSTNAMELEN);
    community = argv[i++];

    /*
     * Get ip address of the agent.
     */
    if ((addr = lookup_host (host)) == 0) {
	fprintf (stderr, MSGSTR(MS_SINFO, SINFO_3, "%s: unknown host: %s\n"),
		 prog_name, host);
#ifdef _POWER
	catclose(snmpcatd);
#endif /* _POWER */
	exit (1);
    }

    /*
     * Bind a socket to the agent's well-known request port.
     */
    port = 0;
    if ((SNMP_port = init_io (port, addr)) == NOTOK) {
	fprintf (stderr, MSGSTR(MS_SINFO, SINFO_4,
		"%s: could not bind UDP port for requests\n"),
		prog_name);
#ifdef _POWER
	catclose(snmpcatd);
#endif /* _POWER */
	exit (1);
    }

    argv += i;
    opt_list = argv;
    /*
     * Check MIB variables.
     */
    for (i=0; i<argc-count; i++) {
	/*
	 * If numeric, assume it's dotted-numeric and fully qualified;
	 * otherwise, check for instance.
	 */
	if (!(isdigit(opt_list[i][0]))) {
	    /*
	     * Find the final "." if it exists.  If not followed by
	     * an instance tack on an instance of 0. If "." does
	     * not exist, tack on a "." followed by an instance of 0.
	     */
	    len = strlen(opt_list[i]);
	    pos = (char *) strchr (opt_list[i], '.');
	    poseq = (char *) strchr (opt_list[i], '='); 
	    if (pos != NULL) {
		pos++;
		/*
		 * Found "." so check for instance.
		 */
		if ((!(isdigit (*pos))) && (!(isalpha (*pos)))) {
		    --pos;
		    if ((tmp = (char *)malloc((unsigned)(len + 2))) == NULLCP) {
			fprintf (stderr, MSGSTR(MS_GENERAL, GENERA_2,
				 "Out of Memory"));
			fprintf (stderr, "\n");
#ifdef _POWER
			catclose(snmpcatd);
#endif /* _POWER */
			exit (1);
		    }
		    if (poseq == NULL) {
			/*
			 * snmp_next does not get a 0 instance tacked on.
			 * so remove the "." at the end.
			 */
			if (strcmp (prog_name, "snmp_next") == 0)
			    strncpy (tmp, opt_list[i], len - 1);
			else
			    sprintf (tmp, "%s0", opt_list[i]);
		    }
		    else  {
			if ((var = (char *)malloc ((unsigned) 
				(len - strlen (poseq) + 1))) == NULLCP) {
			    fprintf (stderr, "out of memory\n");
#ifdef _POWER
			    catclose(snmpcatd);
#endif /* _POWER */
			    exit (1);
			}
			strncpy (var, opt_list[i], (len - strlen (poseq)));
			sprintf (tmp, "%s0%s\n", var, poseq);
		    }
		    opt_list[i] = tmp;
		}
	    }
	    /*
	     * No ".<instance>" so tack on .0 instance.
	     */
	    else {
		if ((tmp = (char *)malloc ((unsigned) (len + 3))) == NULLCP) {
		    fprintf (stderr, MSGSTR(MS_GENERAL, GENERA_2,
			     "Out of Memory"));
		    fprintf (stderr, "\n");
#ifdef _POWER
		    catclose(snmpcatd);
#endif /* _POWER */
		    exit (1);
		}
		if (poseq == NULL) {
		    /* 
		     * snmp_next does not get a .0 instance tacked on.
		     */
		    if (strcmp (prog_name, "snmp_next") == 0)
			strcpy (tmp, opt_list[i]);
		    else
			sprintf (tmp, "%s.0", opt_list[i]);
		}
		else {
		    if ((var = (char *)malloc((unsigned)(len + 2))) == NULLCP) {
		        fprintf (stderr, MSGSTR(MS_GENERAL, GENERA_2,
			         "Out of Memory"));
		        fprintf (stderr, "\n");
#ifdef _POWER
			catclose(snmpcatd);
#endif /* _POWER */
			exit (1);
		    }
		    strncpy (var, opt_list[i], (len - strlen (poseq)));
		    sprintf (tmp, "%s.0%s", var, poseq);
		}
		opt_list[i] = tmp;
	    }
	} /* if */
    } /* for */

    op = argv;
    if (strcmp (prog_name, "snmp_get") == 0)
	func = DS_GET;
    else 
	if (strcmp (prog_name, "snmp_next") == 0)
	    func = DS_NEXT;
    else
	func = DS_SET;

}

/*
 * NAME: info_arginit
 *
 * FUNCTION: Check snmpinfo command line options.
 */
void
info_arginit (argc, argv)
int argc;
char *argv[];
{
    int i, port;
    u_long addr = 0;
#ifdef LOCAL
    char *opts = "VXvd:h:ic:O:o:p:m:st:";
#else /* ! LOCAL */
    char *opts = "vd:h:c:o:m:t:";
#endif

    int m_flag = 0,
	d_flag = 0,
	c_flag = 0,
	h_flag = 0,
	t_flag = 0;
#ifdef LOCAL
    int V_flag = 0,
	p_flag = 0,
	i_flag = 0,
	O_flag = 0;
#endif /* LOCAL */

    community = "public";
    port = 0;
    func = DS_GET;		/* default */

    if (gethostname (host, MAXHOSTNAMELEN+1) == NOTOK) {
	fprintf (stderr, MSGSTR(MS_SINFO, SINFO_5,
		 "%s: cannot determine local hostname\n"), prog_name);
#ifdef _POWER
	catclose(snmpcatd);
#endif /* _POWER */
	exit (1);
    }

    while ((i = getopt (argc, argv, opts)) != EOF) {
	switch (i) {
#ifdef LOCAL
	    /* 
	     * Used for interactive mode
	     */
	    case 'i':  
		if (++i_flag > 1)
		    Usage ();
		break;

	    /* 
	     * Used when xgmon's snmpinfo.g execs snmpinfo.
	     */
	    case 'X':  
		if (++X_flag > 1)
		    Usage ();
		break;

	    /* 
	     * Used for internal timing.
	     */
	     case 's':
		if (++timing > 1)
		    Usage ();
		initTime ();
		break;

	    /*
	     * Used for internal version.
	     */
	    case 'V':
		if (++V_flag > 1)
		    Usage ();
		/*
		 * snmpinfo -V has no other options.
		 */
		if (argc > 2)
		    Usage ();
		printf (MSGSTR(MS_SINFO, SINFO_6, "Version: %s: %s\n"), 
			prog_name, VERSION);
#ifdef _POWER
		catclose(snmpcatd);
#endif /* _POWER */
		exit (0);
#endif /* LOCAL */

	    /*
	     * Verbose mode.
	     */
	    case 'v':
		if (++Verbose > 1)
		    Usage ();
		break;

	    /*
	     * Debug mode. Default is 0.
	     */
	    case 'd':
		if (++d_flag > 1)
		    Usage ();
		debug = str2int (optarg);
		/*
		 * Five levels of debug (0-4).
		 */
	  	if ((debug == NOTOK) ||((debug < 0) || (debug > 4))) {
		    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_7,
			     "Invalid debug level: %s\n"), optarg); 
		    Usage ();
		}
		break;

	    /*
	     * Hostname. Default is the local machine.
	     */
	    case 'h':
		if (++h_flag > 1)
		    Usage ();
		strncpy (host, optarg, MAXHOSTNAMELEN);
		break;

	    /* 
	     * Community name. Default is public.
	     */
	    case 'c':
		if (++c_flag > 1)
		    Usage ();
		community = optarg;
		break;

	    /*
	     * Objects file. Default is /etc/mib.defs .
	     */
	    case 'o':
		readobjects (optarg);
		break;
#if LOCAL
	    /* output defs file. Turns on compile. */
	    case 'O':
		if (++O_flag > 1 || ++m_flag > 1)
		    Usage ();
		odefs = optarg;
		func = DS_COMPILE;
		break;

	    /*
	     * Port number used for internal debugging purposes.
	     */
	    case 'p':
		if (++p_flag > 1)
		    Usage ();
		port = str2int (optarg);
		if (port < 1) {
		     fprintf (stderr, MSGSTR(MS_SINFO, SINFO_8,
				"Invalid port: %s\n"), optarg);
		     Usage ();
		}
		break;
#endif /* LOCAL */

	    /*
	     * Modes. Default is get.
	     */
	    case 'm':
		if (++m_flag > 1)
		    Usage ();

		/* 
		 * Valid func are get, next, set, and dump. The
		 * minimum to make them unique ([g]et, [n]ext, [s]et,
		 * [d]ump is required.
		 */
		switch (isupper (*optarg) ? tolower (*optarg) : *optarg) {

		    case 'g':
			if (strncmp (optarg, "get", strlen (optarg))) {
			    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_9,
					"Invalid func \"%s\"\n"), optarg);
			    Usage ();
			}
			func = DS_GET;
			break;

		    case 'n':
			if (strncmp (optarg, "next", strlen (optarg))) {
			    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_9,
					"Invalid func \"%s\"\n"), optarg);
			    Usage ();
			}
			func = DS_NEXT;
			break;

		    case 's':
			if (strncmp (optarg, "set", strlen (optarg))) {
			    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_9,
					"Invalid func \"%s\"\n"), optarg);
			    Usage ();
			}
			func = DS_SET;
			break;

		    case 'd':
			if (strncmp (optarg, "dump", strlen (optarg))) {
			    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_9,
					"Invalid func \"%s\"\n"), optarg);
			    Usage ();
			}
			func = DS_DUMP;
			break;

		    default:
			fprintf (stderr, MSGSTR(MS_SINFO, SINFO_9, 
				 "Invalid func \"%s\"\n"), optarg);
			Usage ();
		}
		break;

	    /*
	     * Number of tries. Default is 3.
	     */
	    case 't':
	    {
		int tries = str2int (optarg);

		if ((tries == NOTOK) || (tries < 1)) {
		    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_10,
				"Invalid number of tries: %s\n"), optarg); 
		    Usage ();
		}

		if (++t_flag > 1)
		    Usage ();
		 
		max_SNMP_retries = tries;
		break;
	    }

	    default:
		Usage ();
	}
    }

#ifdef LOCAL
    /* interactive ignores mode and additional arguments */
    if (!i_flag) {
#endif
    if (func != DS_DUMP && func != DS_COMPILE && argc == optind)
	Usage ();
    argv += optind;
    op = argv;

    if (checkVars (func, argc - optind, argv) == NOTOK)
	Usage ();
#ifdef LOCAL
    }
#endif

    /* try to use localhost first, fall back on actual hostname */
    if (!h_flag)
	addr = lookup_host ("localhost");
    if (addr == 0)
	if ((addr = lookup_host (host)) == 0) {
	    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_3,
			"%s: unknown host: %s\n"), prog_name, host);
#ifdef _POWER
	    catclose(snmpcatd);
#endif /* _POWER */
	    exit (1);
	}

    if ((SNMP_port = init_io (port, addr)) == NOTOK) {
	fprintf (stderr, MSGSTR(MS_SINFO, SINFO_4,
		"%s: could not bind UDP port for requests\n"),
		prog_name);
#ifdef _POWER
	catclose(snmpcatd);
#endif /* _POWER */
	exit (1);
    }

} /* arginit */

int
envinit ()
{
    OT ot;

    /* finish up some initialization */

    /* if mib.objects have not been read in yet, read them now */
    if ((ot = text2obj ("sysdescr")) == NULLOT)		/* an assumption */
	if (readobjects (NULL) == NOTOK) {
	    fprintf (stderr, "%s\n", PY_pepy);

	    /* DO WE REALLY CARE IF THIS FAILS? */
/* 	    return NOTOK; */
	}

    if (Verbose)
	moresyntax ();	/* initialize moresyntax if verbose mode */
    return OK;
}


/*
 *  NAME: Usage
 *
 *  FUNCTION: Display usage to the user.
 *
 *	LOCAL usage adds in -V and [-p port], 
 *	Xgmon usage adds in [-l num_reps]  and [-T timeout]
 */
Usage()
{
    /* 
     * Posix ordering. 
     */
    char *line1 = MSGSTR(MS_SINFO, SINFO_36, 
			 "[-v] [-c community] [-d level] [-h hostname]");
    char *line2a = MSGSTR(MS_SINFO, SINFO_37, "[-o objectsfile]... ");
    char *line2b = MSGSTR(MS_SINFO, SINFO_38, "[-t tries] ");

#ifdef LOCAL
    char *port = "[-p port] ";
#else
    char *port = "";
#endif

#ifdef LOCAL
	if (X_flag)
#endif
	    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_35, "Usage:  "));
#ifdef LOCAL
	else {
	    fprintf (stderr, "Usage:  %s -V\n\t", prog_name);
	    fprintf (stderr, "%s [-O Output Defs] %s\n\t", 
		    prog_name, line2a);
	}
#endif
	fprintf (stderr, "%s [-m get|next] %s%s%s%s%s%s%s\n", 
		prog_name, line1, 
		X_flag ? "...\n\t\t\t[-l num_reps] " : "\n\t\t\t",
		line2a, X_flag ? "" : port, line2b, 
#ifdef LOCAL
		X_flag ? "[-T timeout]\n\t\t\t" : "\n\t\t\t",
#else
		"",
#endif
		MSGSTR(MS_SINFO, SINFO_39, "variable.instance...")
		);

	fprintf (stderr, "\t%s -m set %s%s%s%s%s%s%s\n",
		prog_name, line1, 
		X_flag ? "...\n\t\t\t[-l num_reps] " : "\n\t\t\t",
		line2a, X_flag ? "" : port, line2b, 
#ifdef LOCAL
		X_flag ? "[-T timeout]\n\t\t\t" : "\n\t\t\t",
#else
		"",
#endif
		MSGSTR(MS_SINFO, SINFO_40, "variable.instance=value")
		);

	fprintf (stderr, "\t%s -m dump %s%s%s%s%s%s[%s]\n", 
		prog_name, line1, 
		X_flag ? "...\n\t\t\t[-l num_reps] " : "\n\t\t\t",
		line2a, X_flag ? "" : port, line2b, 
#ifdef LOCAL
		X_flag ? "[-T timeout]\n\t\t\t" : "\n\t\t\t",
#else
		"",
#endif
		MSGSTR(MS_SINFO, SINFO_39, "variable.instance...")
		);

#ifdef _POWER
    catclose(snmpcatd);
#endif /* _POWER */
    exit (X_flag ? 2 : 1);
}

/*
 * Check out variables for variable=value pairs.
 */
int 
checkVars (func, argc, argv)
int func;
int argc;
char *argv[];
{
    char *cp;
    int i, err;

    if (!argc)		/* dump doesn't require arguments */
	return (NULL);

    for (i = 0, err = 0; i < argc; i++) {
	if (cp = index (argv[i], '=')) {
	    if (func != DS_SET) {
		fprintf (stderr, MSGSTR(MS_SINFO, SINFO_11,
			"value unnecessary for get operation\n"));
#ifdef LOCAL
		*cp = '\0';
#else		
		err++;
#endif
	    }
	}
	else {
	    if (func == DS_SET) {
		fprintf (stderr, MSGSTR(MS_SINFO, SINFO_12,
			"need variable=value for set operation\n"));
		err++;
	    }
	}
    }
    if (err)
	return (NOTOK);
    
    return (OK);
}

int  
f_get (vec)
char  **vec;
{
    return process (new_message (type_SNMP_PDUs_get__request, vec, community));
}

int  
f_get_next (vec)
char  **vec;
{
    return process (new_message (type_SNMP_PDUs_get__next__request, vec, 
	    community));
}

int  
f_set (vec)
char  **vec;
{
    return process (new_message (type_SNMP_PDUs_set__request, vec, community));
}

int 
f_dump (vec)
char  **vec;
{
    OID		oid;
    char	*nvec[3];
    struct type_SNMP_Message *msg;
    struct type_SNMP_PDU *parm;
    int status;

    msg = NULL;
    oid = NULLOID;

    /* loop thru all arguments in vec */
    do {

	nvec[0] = *vec ? *vec : "0.0";
	nvec[1] = NULL;
	if (msg)
	    free_SNMP_Message (msg);
	if ((msg = new_message (type_SNMP_PDUs_get__next__request, nvec, 
		community)) == NULL)
	    return OK;
	if (*vec++) {
	    if (oid)
		oid_free (oid);
	    if ((oid = oid_cpy (msg -> data -> un.get__next__request ->
		    variable__bindings -> VarBind -> name)) == NULLOID)
		adios (MSGSTR(MS_GENERAL, GENERA_2, "out of memory"));
	}

	/* loop thru get-nexts */
	do {
	    if ((status = sndrcv_msg (&msg)) != OK)
		break;

	    if (msg -> data -> offset != type_SNMP_PDUs_get__response) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SINFO, SINFO_13,
			"unexpected message type %d"),
			msg -> data -> offset);
		status = NOTOK;
		break;
	    }
	    parm = msg -> data -> un.get__response;
	    if (parm -> error__status != int_SNMP_error__status_noError) {
		fprintf (stderr, MSGSTR(MS_SINFO, SINFO_14,
			"%s at position %d\n"),
			snmp_error (parm -> error__status), 
			parm -> error__index);
		status = NOTOK;
		break;
	    }
	    status = process_varbinds (parm -> variable__bindings, oid);
	    msg -> data -> offset = type_SNMP_PDUs_get__next__request;
	    parm -> request__id++;

	} while (status == OK);

    } while (*vec && status != NOTOK);

    if (oid)
	oid_free (oid);
    if (msg)
	free_SNMP_Message (msg);
    return status == NOTOK ? NOTOK : OK;
} 

#ifndef LOCAL

/* STUBS for non-local stuff */

int interactive () 
{
#ifdef _POWER
    catclose(snmpcatd);
#endif /* _POWER */
    return (1); 
}
int f_help () {}
int f_compile () {}

#else

/* LOCAL stuff starts here */

void
trapmon_arginit (argc, argv)
int argc;
char *argv[];
{
    struct servent *serv;
    u_short port;
    int i;

    SNMP_timeout = 0; 		/* never timeout */
    port = ((serv = getservbyname("snmp-trap", "udp")) == NULL)
	    ? SNMP_TRAP_PORT : ntohs (serv -> s_port);

    prog_name = argv[0];
    while ((i = getopt (argc, argv, "d:p:vo:")) != EOF) {
	switch (i) {
	    case 'd':
		debug = atoi (optarg);
		break;
	    case 'p':
		port = str2int (optarg);
		if (port < 1) {
		     fprintf (stderr, MSGSTR(MS_SINFO, SINFO_8,
		    		 "Invalid port: %s\n"), optarg);
		     TrapUsage ();
		}
		break;
	    case 'v':
		if (++Verbose > 1)
		    TrapUsage ();
		break;
#if 0
	    /* COMPILER BUG! */
	    case 'o':
		readobjects (optarg);
		break;
#endif
	    default:
		TrapUsage ();
	}
    }
    envinit ();

    if ((SNMP_port = init_io (port, INADDR_ANY)) == NOTOK) 
	adios (MSGSTR(MS_SINFO, SINFO_15, "init_io failed"));

    while (1) 
	(void) process_trap ();

    /* NOTREACHED */
}

TrapUsage ()
{
    fprintf (stderr, MSGSTR(MS_SINFO, SINFO_16,
	     "Usage: %s [-d level] [-o objectsfile]... [-p port] [-v]\n"),
	     prog_name);
#ifdef _POWER
    catclose(snmpcatd);
#endif /* _POWER */
    exit (1);
}

int
interactive ()
{
    int	    eof,
	    vecp,
	    status;
    char    buffer[BUFSIZ],
	    *vec[NVEC + 1];

    /* interactive (or file-fed) stuff here (in-house only) */
    status = eof = 0;
    for (;;) {
	if (getline ("%s> ", buffer) == NOTOK) {
	    if (eof)
		break;
	    eof = 1;
	    continue;
	}
	eof = 0;
	bzero ((char *) vec, sizeof vec);
	if ((vecp = str2vec (buffer, vec)) < 1)
	    continue;
	
	switch (snmploop (vec, OK)) {
	    case NOTOK:
		status = 1;
		break;

	    case OK:
	    default:
		status = 0;
		continue;
	    
	    case DONE:
		status = 0;
		break;
	}
	break;
    }

#ifdef _POWER
    catclose(snmpcatd);
#endif /* _POWER */
    return status;
}

static struct dispatch *
getds (name)
char   *name;
{
    register int    longest,
                    nmatches;
    register char  *p,
                   *q;
    char    buffer[BUFSIZ];
    register struct dispatch   *ds,
                               *fs = NULL;

    longest = nmatches = 0;
    for (ds = dispatches; p = ds -> ds_name; ds++) {
	for (q = name; *q == *p++; q++)
	    if (*q == NULL)
		return ds;

	if (*q == NULL)
	    if (q - name > longest) {
		longest = q - name;
		nmatches = 1;
		fs = ds;
	    }
	    else
		if (q - name == longest)
		    nmatches++;
    }

    switch (nmatches) {
	case 0: 
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SINFO, SINFO_17,
			"unknown operation \"%s\""), name);
	    return NULL;

	case 1: 
	    return fs;

	default: 
	    for (ds = dispatches, p = buffer; q = ds -> ds_name; ds++)
		if (strncmp (q, name, longest) == 0) {
		    (void) sprintf (p, "%s \"%s\"", p != buffer ? "," : "", q);
		    p += strlen (p);
		}
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SINFO, SINFO_18,
		        "ambiguous operation, it could be one of:%s"),
			buffer);
	    return NULL;
    }
}

static int
snmploop (vec, error)
char **vec;
int error;
{
    register struct dispatch *ds;

    if ((ds = getds (strcmp (*vec, "?") ? *vec : "help")) == NULL)
	return error;
    switch ((*ds -> ds_fnx) (++vec)) {
	case NOTOK:
	    return error;

	case OK:
	default:
	    return OK;
	
	case DONE:
	    return DONE;
    }
    /* NOTREACHED */
}

static int  
getline (prompt, buffer)
char   *prompt,
       *buffer;
{
    register int    i;
    register char  *cp,
                   *ep;
    static int  sticky = 0;

    if (sticky) {
	sticky = 0;
	return NOTOK;
    }

    if (ontty) {
	printf (prompt, prog_name);
	(void) fflush (stdout);
    }

    for (ep = (cp = buffer) + BUFSIZ - 1; (i = getchar ()) != '\n';) {
	if (i == EOF) {
	    if (ontty)
		printf ("\n");
	    clearerr (stdin);
	    if (cp == buffer) {
		if (ontty)
		    printf ("\n");	/* and fall */
		return NOTOK;
	    }

	    sticky++;
	    break;
	}

	if (cp < ep)
	    *cp++ = i;
    }
    *cp = NULL;

    return OK;
}

static int  
ncols (fp)
FILE *fp;
{
#ifdef	TIOCGWINSZ
    int	    i;
    struct winsize ws;

    if (ioctl (fileno (fp), TIOCGWINSZ, (char *) &ws) != NOTOK
	    && (i = ws.ws_col) > 0)
	return i;
#endif

    return 80;
}

static int  
f_help (vec)
char  **vec;
{
#if 0
    register int    i,
                    j,
                    w;
    int     columns,
            width,
            lines;
    register struct dispatch   *ds,
                               *es;

    for (es = dispatches; es -> ds_name; es++)
	continue;
    width = helpwidth;

    if (*++vec == NULL) {
	if ((columns = ncols (stdout) / (width = (width + 8) & ~7)) == 0)
	    columns = 1;
	lines = ((es - dispatches) + columns - 1) / columns;

	printf (MSGSTR(MS_SINFO, SINFO_19, "Operations:\n"));
	for (i = 0; i < lines; i++)
	    for (j = 0; j < columns; j++) {
		ds = dispatches + j * lines + i;
		printf ("%s", ds -> ds_name);
		if (ds + lines >= es) {
		    printf ("\n");
		    break;
		}
		for (w = strlen (ds -> ds_name); w < width; w = (w + 8) & ~7)
		    (void) putchar ('\t');
	    }
	printf ("\n");

	return OK;
    }

    if (strcmp (*vec, "-help") == 0) {
	printf (MSGSTR(MS_SINFO, SINFO_20, "help [commands ...]\n"));
	printf (MSGSTR(MS_SINFO, SINFO_21,
		"  with no arguments, lists operations which may be invoked\n");
	printf (MSGSTR(MS_SINFO, SINFO_22,
		"  otherwise prints help for each operation given\n");
	return OK;
    }
    
    for (i = 0; *vec; i++, vec++)
	if (strcmp (*vec, "?") == 0) {
	    for (ds = dispatches; ds -> ds_name; ds++)
		printf ("%-*s\t- %s\n", width, ds -> ds_name, 
			MSGSTR(MS_SINFO, SINFO_29 + i, ds -> ds_help));

	    break;
	}
	else
	    if (ds = getds (*vec))
		printf ("%-*s\t- %s\n", width, ds -> ds_name,
			MSGSTR(MS_SINFO, SINFO_29 + i, ds -> ds_help));

#endif
    return OK;
}

/*  */

static char *access_t[] = { "not-accessible",
			    "read-only",
			    "write-only",
			    "read-write"};

static char *status_t[] = { "obsolete",
			    "mandatory",
			    "optional",
			    "deprecated" };
			     

static int  f_compile (vec)
char  **vec;
{
    register int    i = 0;
    int	    fast;
    char   *cp,
	   *file;
    register OS	    os;
    register OT	    ot;
    FILE   *fp;

    fast = 0;
    file = odefs ? odefs : "./objects.defs";
    if (*vec != NULL && strcmp (*vec, "-help") == 0) {
	printf (MSGSTR(MS_SINFO, SINFO_23, "compile [-f] [file]\n"));
	printf (MSGSTR(MS_SINFO, SINFO_24, 
		"    -f:   brief output (default to stdout)\n"));
	printf (MSGSTR(MS_SINFO, SINFO_25,
		"    file: output file (default %s)\n"), file);

	return OK;
    }
    while (cp = *vec++) {
	if (strcmp (cp, "-f") == 0) {
	    fast = 1, file = NULLCP;
	    continue;
	}
	else if (strcmp (cp, "-F") == 0) {
	    fast = 2, file = NULLCP;
	    continue;
	}

	file = cp;
    }

    if (file) {
	if ((fp = fopen (file, "w")) == NULL) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SINFO, SINFO_26,
			"unable to write: %s"), file);
	    return OK;
	}
	if (!fast) {
	    register int j,
			 k;

	    i = j = k = 0;
	    for (ot = text2obj ("ccitt"); ot; ot = ot -> ot_next) {
		i++;
		j += strlen (ot -> ot_text)
		   + strlen (sprintoid (ot -> ot_name));
		k += ot -> ot_name -> oid_nelem;
	    }
	    j += i << 1, k += i;
	    fprintf (fp, "--* compiled %d %d %d\n", i, j, k);
	}
    }
    else
	fp = stdout;

    for (ot = text2obj ("ccitt"); ot; ot = ot -> ot_next) {

	switch (fast) {
	    case 0:
		fprintf (fp, "%-20s %-20s", ot -> ot_text, 
			sprintoid (ot -> ot_name));
		if ((os = ot -> ot_syntax) || ot -> ot_status)
		    fprintf (fp, " %-15s %-15s %s",
			     os ? os -> os_name : "Aggregate",
			     access_t[ot -> ot_access & OT_RDWRITE],
			     status_t[ot -> ot_status & OT_DEPRECATED]);
		fprintf (fp, "\n");
		break;
	    case 1:
		fprintf (fp, "%s=%s\n", ot -> ot_text, 
			sprintoid (ot -> ot_name));
		break;
	    case 2:
		fprintf (fp, "%s\n", ot -> ot_ltext);
		break;
	}
    }

    if (file)
	(void) fclose (fp);

    if (!fast)
	advise (SLOG_NONE, MSGSTR(MS_SINFO, SINFO_27,
		"%d objects written to %s"), i, file);

    return OK;
}

#endif
