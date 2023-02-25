static char sccsid[] = "@(#)96	1.8.1.10  src/tcpip/usr/sbin/snmpd/config.c, snmp, tcpip411, 9434A411a 8/18/94 14:28:49";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: setconfig(), readconfig(), f_variable(), set_variable(), 
 *            set_interface(), refresh(), f_community(), f_trap(),
 *            f_logging(), f_view(), f_smux(), f_snmpd(), set_maxpacket()
 *            f_syscontact(), f_syslocation()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/config.c
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
#include <sys/param.h>
#include <sys/stat.h>
#include "view.h"
#ifdef SMUX
#  include "smux_g.h"		/* for smuxClient struct */
#endif

#if defined(RT) && defined(AIX)
#undef PS
#endif

#ifdef DEBUG
#if defined (AIX) && defined (RT)
#    include <sys/syslog.h>
#else
#    include <syslog.h>
#endif
#endif

/* EXTERN */
extern char *Myname;                     /* defined in snmpd.c */
extern int mypid,                        /* ... */
           debug_default;                /* ... */
extern struct snmpstat   snmpstat;       /* ... */
extern struct logging   *logging,        /* declared in logging.c */
			 log_conffile,   /* ... */
			 log_cmdline;    /* ... */

extern struct hostent *gethostbystring();
extern void init_log_struct ();          /* defined in logging.c */
extern void init_log ();                 /* ... */
extern void do_trap ();                  /* defined in traps.c */
extern int str2int ();                   /* defined in snmpd.c */
extern int dgram_rfx (), dgram_wfx ();

/* FORWARD */
void	setconfig (),
	readconfig (),
	set_variable (),
	set_maxpacket ();

/* GLOBAL */
struct pair {
    char   *p_name;		/* runcom directive */
    IFP	    p_handler;		/* dispatch */
    int	    p_pass;		/* pass through the file */
};

static int viewmask = 0x1;

int  f_community (), f_variable (), f_logging (), f_trap (), f_view ();
int  f_syslocation (), f_syscontact ();
int  f_snmpd ();
#ifdef SMUX
int  f_smux ();
#endif

static struct pair pairs[] = {
    "community",  f_community, 	PASS2,
    "logging",	  f_logging,    PASS1,	/* logging info is needed early */
    "trap", 	  f_trap,       PASS2,	
    "variable",	  f_variable,   PASS2,
    "view",	  f_view,       PASS2,
    "snmpd",      f_snmpd,	PASS2,
    "syslocation", f_syslocation, PASS2,
    "syscontact", f_syscontact, PASS2,
#ifdef SMUX
    "smux",	  f_smux,       PASS2,
#endif
    NULL
};

extern OID    trapview;

char	*configfile;
static int 	trapno;

int stimeout = D_ICHECKTIME;	/* select timeout, for interface check time */
int maxopsize = NOTOK;		/* maximum output packet size */
int smuxtimeout = D_SMUXTIME;	/* smux read timeout */
int smuxtrapaddr = 0;

int ethernettimeout = D_DEVTIME;       /* Default device timeout */
int tokenringtimeout = D_DEVTIME;
int fdditimeout = D_DEVTIME;

/*
 *  NAME: setconfig ()
 *
 *  FUNCTION: Sets the input config file.  Default is /etc/snmpd.conf.
 */
void
setconfig (file)
char *file;
{
    configfile = file;
}


/*
 *    NAME: readconfig ()
 *
 *    FUNCTION: 
 *    Reads the config file (default of /etc/snmpd.conf) for community names,
 *    logging information, trap configuration information, view configuration,
 *    smux configuration information, and variables, then calls the appropriate
 *    handler function.
 */
void
readconfig (pass)
int	pass;
{
    register char *cp;
    char    buffer[BUFSIZ],
	    line[BUFSIZ],
	   *vec[NVEC + NSLACK + 1];
    register struct pair *p;
    struct stat    st;
    static FILE   *fp;

#ifdef DEBUG
    syslog (LOG_DEBUG, "readconfig (%d)", pass);
#endif

    /*
     * pass 1 initialization.
     */
    if (pass == PASS1) {

	/*
	 * Open the configuration file for reading.
	 */
        if (configfile == (char *) NULL)
    	    configfile = SNMPDCONF;
	if ((fp = fopen (cp = configfile, "r")) == NULL)
		adios (MSGSTR(MS_CONFIG, CONFIG_29, "cannot read %s: %s"), 
		     configfile, strerror(errno));

	/* 
	 * The configuration file must be owned by the root user.
	 */
	if (getuid () == 0 
		&& fstat (fileno (fp), &st) != NOTOK
		&& st.st_uid != 0) {
	      adios (MSGSTR(MS_CONFIG, CONFIG_30, "%s not owned by root"), cp);
	}
    }

    /*
     * Pass 2 starts here.  Walk through the configuration file, one
     * line at a time.  Each line is converted into a vector.  Each
     * vector is parsed for configuration information.
     */
    else 
    {
	rewind (fp);

	/* 
	 * On the second pass of a readconfig, reset the variables so that 
	 * defaults will be used if they have been removed from the config
	 * file.
	 */
	stimeout = D_ICHECKTIME;	/* select timeout, for if check time */
	maxopsize = NOTOK;		/* maximum output packet size */
	smuxtimeout = D_SMUXTIME;	/* smux read timeout */
	smuxtrapaddr = 0;
	ethernettimeout = D_DEVTIME;    /* Default device timeout */
	tokenringtimeout = D_DEVTIME;
	fdditimeout = D_DEVTIME;
    }

    while (fgets (buffer, sizeof buffer, fp)) {

	/* 
	 * Ignore comments and blank lines and remove newline characters. 
	 */
	if (*buffer == '#')	    	
	    continue;
	if (cp = (char *)index (buffer, '#'))
	    *cp = NULL;
	if (cp = (char *)index (buffer, '\n'))
	    *cp = NULL;
	(void) strcpy (line, buffer);

	/* 
	 * Convert a string to a vector.
	 */
	bzero ((char *) vec, sizeof vec);
	if (str2vec (buffer, vec) < 1)      
	    continue;

	/*
	 * Parse through the vectors.  See the definition of the pairs[]
	 * table to see what vectors are parsed on each pass.
	 */
	for (p = pairs; p -> p_name; p++) {
	    if (lexequ (p -> p_name, vec[0]) == 0) {
		/* calls function */
		if ((p -> p_pass == pass) && (*p -> p_handler) (vec) == NOTOK) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_1,
			    "malformed directive: \"%s\""), line);
		break;
	    }
	}
	if (!p -> p_name)
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_2, 
					    "unknown directive: \"%s\""), line);
    } /* while */

    if (pass == PASS2)
	(void) fclose (fp);
}

/*   
 *    NAME: refresh ()
 *
 *    FUNCTION:
 *    Frees the memory used by CHead and UHead, re-reads the snmpd config
 *    file, and sends warm start trap.
 */
void
refresh()
{ 
    register struct community *c, *comm;
    register struct trap      *t, *trap;
    register struct view      *v, *view;
    char             *log_filename = NULLCP;
    FILE	     *tmp_fp;                /* Saves log_conffile.l_logfp */
    int	 	      enabled;               /* Saves log_conffile.l_enabled */
#ifdef SMUX
    struct smuxClient *sc, *scn;
#endif

    advise (SLOG_NOTICE, MSGSTR(MS_CONFIG, CONFIG_3, "refreshing %s (%d)"), 
	  Myname, mypid);

    /* 
     * Free memory used for the community, trap, and view linked lists.
     */
    for (c = CHead -> c_forw; c != CHead; c = comm) {
	comm = c -> c_forw;
	remque(c);
	if (c -> c_name)
	    free (c-> c_name);
	if (c -> c_instance != NULL)
	    free ((char *)c -> c_instance);
	if (c -> c_vu)
	    oid_free (c-> c_vu);
	free ((char *)c);
    }

    /* views must be free'd before traps, since traps contain
     * a non-alloc'ed view struct.
     */
    for (v = VHead -> v_forw; v != VHead; v = view) {
	register struct subtree *s, *subt, *SHead = & v -> v_subtree;
	int freeit = 1;

	view = v -> v_forw;
	remque (v);
	if (v -> v_name != NULLOID)
	    oid_free (v-> v_name);
	if (v -> v_community != NULL) {
	    qb_free (v -> v_community);

	    /* if v_community has stuff in it, then this view entry
	     * is a part of the trap structure, and was not calloc'ed. 
	     * So dont free it!  (RLF)
	     */
	    freeit = 0;
	}
	if (v -> v_instance != NULL)
	    free ((char *)v -> v_instance);

	/* handle view sub-trees */
	for (s = SHead -> s_forw; s != SHead; s = subt) {
	    subt = s -> s_forw;

	    remque (s);
	    oid_free (s -> s_subtree);
	    if (s -> s_text != NULL)
		free (s -> s_text);
	    free ((char *) s);
	}

	if (freeit)
	    free ((char *)v);
    }

    for (t = UHead -> t_forw; t != UHead; t = trap) {
	trap = t -> t_forw;
	remque(t);
	if (t -> t_name)
	    free (t-> t_name);
	if (t -> t_instance != NULL)
	    free ((char *)t -> t_instance);
	free ((char *)t);
    }

#ifdef SMUX
    /* free smuxClient access list */
    for (sc = SHead -> sc_forw; sc != SHead; sc = scn) {
	scn = sc -> sc_forw;
	remque (sc);
	if (sc -> sc_id != NULLOID)
	    oid_free (sc -> sc_id);
	if (sc -> sc_password != NULLCP)
	    free (sc -> sc_password);
	free ((char *)sc);
    }
#endif

    /*
     * Reinitialize the view table 
     */
    viewmask = 0x1;
    init_def_view ();

    /*
     * The log filename may change.  Save the old one from the logging 
     * structure because readconfig() just writes over log_conffile.l_filename.
     * init_log() frees the space malloc'ed for log_filename.  Enablement may
     * change.  Save the current enablement state.
     */
    if (logging -> l_filename != NULLCP)
	log_filename = strdup (logging -> l_filename);
    enabled = log_conffile.l_enabled;

    /*
     * Reset the logging structure back to the defaults.
     * Do not lose the file pointer to the log file.
     */
    if (log_conffile.l_filename != NULLCP)
	free (log_conffile.l_filename);
    tmp_fp = log_conffile.l_logfp;
    init_log_struct (&log_conffile, NULLCP, NOTOK);
    log_conffile.l_logfp = tmp_fp;

    /* 
     * Re-read the snmpd configuration file.  The first pass fills in the 
     * logging structure and leaves the configuration file open.  The second 
     * pass fills in the community, view, trap, and smux  structures and 
     * closes the configuration file.
     */
    trapno = 0;			/* start traps at ground zero... */
    readconfig (PASS1);		/* logging info... */
    readconfig (PASS2);		/* everything else */
    fin_view ();	        /* finish view, community, trap tables */
    start_view ();	        /* re-export our views */
    set_maxpacket ();		/* maxpacket may have changed */

    /*
     * Refresh the logging.  TRUE tells init_log() that this is a refresh.
     */
    init_log (log_filename, enabled, TRUE);

    /* 
     * Send the warm start trap. 
     */ 
    do_trap (int_SNMP_generic__trap_warmStart, 0, 
	     (struct type_SNMP_VarBindList *) 0);

    return;
}


/*
 *    NAME: f_community
 *
 *    FUNCTION:
 *    Converts vector, vec, to community structure, comm, and inserts
 *    into community queue.
 *    The format of a vec is:
 *
 * 	community  <name>  <address>  <netmask>	 <permission>  <view>
 *
 *    An address is either a hostname or an address in dot notation and
 *    permissions is one of:  none, readOnly, writeOnly, readWrite.
 *    The default permission is readOnly; the default address and netmask are
 *    0.0.0.0. unless an address other than 0.0.0.0 is specified.
 *    In that case, the default netmask is 255.255.255.255. If a permission
 *    is specified, both address and netmask must also be specified.
 *
 *    e.g,
 *	community public
 *	community public  0.0.0.0 
 *	community system  0.0.0.0 	  0.0.0.0
 *	community private 129.144.100.5   255.255.255.255 readWrite
 *	community private 129.144.100.5   255.255.255.255 readWrite 1.17.2
 *      ...
 *
 *    RETURNS: NOTOK (failure)
 *	       OK    (success)
 */

int	
f_community (vec)
char  **vec;
{
    register struct community *comm;
    register struct sockaddr_in *na;

    /* get community name */
    vec++;
    if (!(*vec))
	return NOTOK;

    if (((comm = (struct community *) calloc (1, sizeof *comm)) == NULL )
	    || ((comm -> c_name = strdup (*vec)) == NULL)) {
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "community");
    }

    /* set defaults */
    comm -> c_sin.sin_addr.s_addr = inet_addr ("0.0.0.0");
    comm -> c_netmask.sin_addr.s_addr = inet_addr ("0.0.0.0");
    comm -> c_permission = OT_RDONLY;

    /* get community address */
    vec++;
    if (*vec) {
	na = &comm -> c_sin;
	if (strcmp (*vec,"0.0.0.0") != 0) {

	    /*
	     * An address may be either a host name or in dot address 
	     * notation.  
	     */

	    register struct hostent *hp;

	    if (hp = gethostbyname (*vec))  
		bcopy (hp -> h_addr, (char *) &(na -> sin_addr), 
		       hp -> h_length);
	    else {  

		long addr;

		/* See if the parameter is a valid address in dotted decimal
		   notation.  The address is saved and used in case it is validly 
		   formed but not known to the local name server.  (EJP) */
 
		if ((addr = na->sin_addr.s_addr = inet_addr(*vec)) == -1) {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_4,
			    "%s: host address %s mis-formed"),"community",*vec);
		    goto you_lose;
		}
		/*
		 * If an address is defined, change the default netmask to 
		 * allow access only on the defined address 
		 */
		comm -> c_netmask.sin_addr.s_addr = inet_addr ("255.255.255.255");
		if (hp = gethostbyaddr (&addr, sizeof (addr), AF_INET) ) 
		    bcopy (hp -> h_addr, (char *) &(na -> sin_addr),
			    hp -> h_length);
	    }
	}
	vec++;
    }

    /* get community netmask */
    if (*vec) {
	if ((comm -> c_netmask.sin_addr.s_addr = inet_addr (*vec)) == -1) {
	    if (strcmp (*vec, "255.255.255.255") != 0) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_5, 
		        "%s: netmask %s mis-formed"), "community", *vec);
		goto you_lose;
	    }
	}
	vec++;
    }

    /* get community access mode */
    if (*vec) {
	if (lexequ (*vec, "readOnly") == 0)
	    comm -> c_permission = OT_RDONLY;
	else
	    if (lexequ (*vec, "readWrite") == 0)
		comm -> c_permission = OT_RDWRITE;
	    else
		if (lexequ (*vec, "writeOnly") == 0)
		    comm -> c_permission = OT_WRONLY;
		else
		    if (lexequ (*vec, "none") == 0) 
			comm -> c_permission = OT_NONE;
		    else {
			advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_6,
		    		"invalid access mode \"%s\""), *vec);
			goto you_lose;
		    }
	vec++;
    }

    /* set view */
    if (*vec) {
	char    buffer[BUFSIZ];

	(void) strcpy (buffer, *vec);
	if ((comm -> c_vu = text2oid (buffer)) == NULLOID) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_7,
	            "%s: unknown OID \"%s\""), "community", *vec);
	    goto you_lose;
	}

	if (*++vec)
	    goto you_lose;
    }
    else {
	if (comm -> c_permission & OT_WRONLY) {

	    /* be consistent. */
	    if ((comm -> c_vu = text2oid ("defViewWholeRW")) == NULL) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_7,
	                "%s: unknown OID \"%s\""),"community","defViewWholeRW");
		    goto you_lose;
	    }
	}
	else
	    if ((comm -> c_vu = text2oid ("defViewWholeRO")) == NULL) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_7,
	                "%s: unknown OID \"%s\""),"community","defViewWholeRO");
		    goto you_lose;
	    }
    }
    
    insque (comm, CHead -> c_back); /* insert community into comm queue */

    return OK;

you_lose: 
    free (comm -> c_name);
    free ((char *) comm);

    return NOTOK;
}


/*
 *    NAME: f_variable ()
 *
 *    FUNCTION:
 *    sets variables (interface,snmpEnableAuthenTraps) given by vector, vec.  
 *   
 *    RETURNS: NOTOK (failure)
 *	       OK    (success)
 */

static int  
f_variable (vec)
char  **vec;
{
    if (*++vec == NULL)
	return NOTOK;

#ifndef	_POWER
    if (lexequ (*vec, "interface") == 0) {
	char   *name;

	if ((name = *++vec) == NULL)
	    return NOTOK;
	for (vec++; *vec; vec++)
	    if (index (*vec, '='))
		set_interface (name, *vec);
	    else
		return NOTOK;

	return OK;	
    }
#endif	/* !_POWER */
    
    if (lexequ (*vec, "snmpEnableAuthenTraps") == 0) {
	++vec;

	if (lexequ (*vec, "enabled") == 0)
	    snmpstat.s_enableauthentraps = TRAPS_ENABLED;
	else
	    if (lexequ (*vec, "disabled") == 0)
		snmpstat.s_enableauthentraps = TRAPS_DISABLED;

	return OK;
    }

    if (!vec[0] || !vec[1] || vec[2])
	return NOTOK;

    set_variable (vec[0], vec[1]);

    return OK;
}


/*
 * NAME: set_variable ()
 *
 * FUNCTION: Does the set for f_variable().
 */
void
set_variable (name, newvalue)
char   *name,
       *newvalue;
{
    caddr_t  value;
    register OT	    ot = text2obj (name);
    register OS	    os;

    if (ot == NULLOT) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_8, 
                                        "unknown object \"%s\""), name);
	return;
    }
    if (ot -> ot_getfnx == NULLIFP) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_9,
                                        "no getfnx for object \"%s\""),
		ot -> ot_text);
	return;
    }
    if (ot -> ot_getfnx != o_igeneric) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_10,
                "non-initializable getfnx for object \"%s\""), ot -> ot_text);
	return;
    }
    if ((os = ot -> ot_syntax) == NULLOS) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_11,
                                        "no syntax defined for object \"%s\""),
		ot -> ot_text);
	return;
    }
    if ((*os -> os_parse) (&value, newvalue) == NOTOK) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_12,
                "invalid value for variable \"%s.0\": \"%s\""), 
		ot -> ot_text, newvalue);
	return;
    }
    if (ot -> ot_info) {
	(*os -> os_free) (ot -> ot_info);
	ot -> ot_info = NULL;
    }
    ot -> ot_info = value;
}


/*
 *    NAME: f_logging ()
 *
 *    FUNCTION:
 *    Converts vector, vec, into logging configuration information.
 *    The possible formats of vec are:
 *
 *	logging		file=<filename>  <enabled | disabled> size=<#> 
 *	logging		<level>=<#>
 */
f_logging (vec)
char	**vec;
{
    char *key;

    /*
     * Just skip this if we are logging based on specifications from the 
     * command line.  If logging is directed from the command line, the
     * logging configuration information in the configuration file is
     * ignored.
     */
    if (log_cmdline.l_filename != NULLCP && log_cmdline.l_enabled == OK)
	return OK;

    if ((key = *++vec) == NULL)
	return NOTOK;

    while (key != NULL) {
	char *sub;

	/* handle keyword=value */
	if ((sub = (char *)index (key, '=')) != NULL) {
	    *sub++ = '\0';
	    if (lexequ (key, "file") == 0) {
		log_conffile.l_filename = strdup (sub);
	    }
	    else if (lexequ (key, "size") == 0) {
		log_conffile.l_size = str2int (sub);
		if (log_conffile.l_size == NOTOK) {
		    log_conffile.l_size = 0;
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_13,
"Log file \"size\" value, %s, cannot be converted into an integer.\nThe default value 0 (unlimited) will be used."),
			   sub);
		}
		if (*sub == NULL) {
		    log_conffile.l_size = 0;
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_14,
"Log file \"size\" value was not specified.  The default value 0 (unlimited) will be used."));
		}
	    }
	    else if (lexequ (key, "level") == 0) {
		log_conffile.l_level = str2int (sub);
		if (log_conffile.l_level == NOTOK 
#ifndef DEBUG
		        || log_conffile.l_level > MAXDEBUGLEVEL
#endif
		        ) {
		    log_conffile.l_level = 0;
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_15,
"Invalid debug level in %s: %s. The default value 0 (minimal) will be used."),
			    configfile, sub); 
		}
		if (*sub == NULL) {
		    log_conffile.l_level = 0;
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_16, 
"Log file \"level\" value was not specified.  The default value 0 (minimal) will be used."));
		}
	    }
	    else
		return NOTOK;
	}

	/* handle keyword */
	else {
	    if (lexequ (key, "enabled") == 0)
		log_conffile.l_enabled = OK;
	    else if (lexequ (key, "disabled") == 0)
		log_conffile.l_enabled = NOTOK;
	    else
		return NOTOK;
	}
	key = *++vec;
    }
     
#ifdef DEBUG
    syslog (LOG_DEBUG, "log_conffile.l_filename = %s", log_conffile.l_filename);
    syslog (LOG_DEBUG, "log_conffile.l_size = %d", log_conffile.l_size);
    syslog (LOG_DEBUG, "log_conffile.l_level = %d", log_conffile.l_level);
    syslog (LOG_DEBUG, "log_conffile.l_enabled = %s", 
	    (log_conffile.l_enabled == OK) ? "OK" : "NOTOK");
#endif

    return OK;
}


/*
 *    NAME: f_trap ()
 *
 *    FUNCTION:
 *    Converts vector, vec, to trap structure, t, and inserts
 *    into trap queue.
 *    The format of a vec is:
 *
 * 	trap     traps    address
 *
 *    An address is either a hostname or an address in dot notation. 
 *
 *    RETURNS: NOTOK (failure)
 *	       OK    (success)
 */

int 
f_trap (vec)
char **vec;
{
    register struct trap *t;
    register struct view *v;
    register struct sockaddr_in *na;
    static int traport = 162; 	/* trap port */     

    vec++;
    if (!(*vec))
	return NOTOK;
	 
    /* get the community name */
    if (( t = (struct trap *) calloc (1, sizeof *t)) == NULL 
	|| (t -> t_name = strdup (*vec)) == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "trap");

    v = t -> t_view = & t -> t_vu;
    v -> v_subtree.s_forw = v -> v_subtree.s_back = & v -> v_subtree;
    t -> t_generics = 0xfe;

    /* get the IP address */
    vec++;
    if (!(*vec)) 
	goto you_lose;

    trapview -> oid_elements[trapview -> oid_nelem++] = trapno + 1;
    v -> v_name = oid_cpy (trapview);
    trapview -> oid_nelem--;
    if (v -> v_name == NULLOID)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "trap");
    if ((v -> v_community = str2qb (t -> t_name, strlen (t -> t_name), 1))
	    == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "trap");

    na = (struct sockaddr_in *) &v -> v_sa;

    /*
    * An address may be either a host name or in dot address 
    * notation.  
    */
    {
	register struct hostent *hp;

	if (hp = gethostbyname (*vec)) 
	    bcopy (hp -> h_addr, (char *) &(na -> sin_addr), hp -> h_length);
	else {  

	    long addr;

	    /* See if the parameter is a valid address in dotted decimal
	       notation.  The address is saved and used in case it is validly 
	       formed but not known to the local name server.  (EJP) */

	    if ((addr = na->sin_addr.s_addr = inet_addr(*vec)) == -1) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_4, 
	                "%s: host address %s mis-formed"),"trap",*vec);
		goto you_lose;
	    }
	    if (hp = gethostbyaddr (*vec, strlen(*vec), AF_INET) )
	        bcopy (hp -> h_addr, (char *) &(na -> sin_addr),
		        hp -> h_length);
	}
	vec++;
    }
    na -> sin_family = AF_INET;
    na -> sin_port = htons (traport);

    if (*vec) {
	char	buffer[BUFSIZ];
	OID	name;
	register struct view *u;

	(void) strcpy (buffer, *vec);
	if ((name = text2oid (buffer)) == NULL) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_7,
	                "%s: unknown OID \"%s\""), "trap", *vec);
	    goto you_lose;
	}
	oid_free (v -> v_name);
	v -> v_name = name;

	for (u = VHead -> v_forw; u != VHead; u = u -> v_forw)
	    if (oid_cmp (u -> v_name, v -> v_name) == 0) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_17,
	                "duplicate view \"%s\" for trap sink \"%s\""),
			*vec, t -> t_name);
		goto you_lose;
	    }
	vec++;
    }
    else
	trapno++;

    if (*vec) {
	if (sscanf (*vec, "%lx", &t -> t_generics) != 1)	/* ??? */
	    goto you_lose;

	if (*++vec)
	    goto you_lose;
    }

    insque (t, UHead -> t_back); /* insert trap into trap queue */
    insque (v, VHead -> v_back);

    return OK;

you_lose: 
    free (t -> t_name);
    if (v -> v_name != NULLOID)
	oid_free (v -> v_name);
    if (v -> v_community != NULL)
	qb_free (v -> v_community);
    free ((char *) t);

    return NOTOK;
}


/*
 *  NAME: f_view ()
 *
 *  FUNCTION:
 *    The format of a vec is:
 *
 *	view     1.17.2          system unix
 *
 *	where arg[1] 		is the view name (an OID)
 *	      arg[2] -> arg[n]	are subtrees that this view governs.
 */
int  
f_view (vec)
char  **vec;
{
    char    buffer[BUFSIZ];
    register struct subtree *s,
			    *x,
			    *y;
    register struct view *v,
			 *u;

    if (viewmask == 0) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_18, 
                "too many views starting with \"%s\""), *vec);
	return NOTOK;
    }

    if ((v = (struct view *) calloc (1, sizeof *v)) == NULL)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "view");
    s = &v -> v_subtree;
    v -> v_subtree.s_forw = v -> v_subtree.s_back = s;
    vec++;

    (void) strcpy (buffer, *vec);
    if ((v -> v_name = text2oid (buffer)) == NULL) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_7, 
                        "%s: unknown OID \"%s\""), "view", *vec);
	goto you_lose;
    }
    if (trapview && inSubtree (trapview, v -> v_name)) {
        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_19,
                        "view \"%s\" is for traps"), *vec);
	goto you_lose;
    }
    for (u = VHead -> v_forw; u != VHead; u = u -> v_forw)
	if (oid_cmp (u -> v_name, v -> v_name) == 0) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_20,
	                "duplicate view \"%s\""), *vec);
	    goto you_lose;
	}

    for (vec++; *vec; vec++) {
	register struct subtree *z;
	OID	name;

	(void) strcpy (buffer, *vec);
	if ((name = text2oid (buffer)) == NULLOID) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_7,
	                "%s: unknown OID \"%s\""), "view", *vec);
	    goto you_lose;
	}

	for (x = s -> s_forw; x != s; x = y) {
	    register int    i,
			    j;
	    y = x -> s_forw;

	    if (bcmp ((char *) x -> s_subtree -> oid_elements,
		      (char *) name -> oid_elements,
		      ((i = x -> s_subtree -> oid_nelem)
		           <= (j = name -> oid_nelem) ? i : j)
		          * sizeof name -> oid_elements[0]) == 0) {
		advise (SLOG_EXCEPTIONS, "%s %s %s",
			*vec,
			i <= j ? MSGSTR(MS_CONFIG, CONFIG_21, "already under") :
			         MSGSTR(MS_CONFIG, CONFIG_22, "superceding"),
			oid2ode (x -> s_subtree));
		if (i <= j)
		    goto another;

		remque (x);
		oid_free (x -> s_subtree);
		free ((char *) x);
	    }
	}

	if ((z = (struct subtree *) calloc (1, sizeof *z)) == NULL)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "view");
	z -> s_subtree = name;
	z -> s_text = strdup (buffer);

	insque (z, s -> s_back);
another: ;
    }

    v -> v_mask = viewmask;
    viewmask <<= 1;
    insque (v, VHead -> v_back);

    return OK;

you_lose: ;
    for (x = s -> s_forw; x != s; x = y) {
	y = x -> s_forw;

	remque (x);
	oid_free (x -> s_subtree);
	if (x -> s_text != NULL)
	    free (x -> s_text);
	free ((char *) x);
    }
    if (v -> v_name)
	oid_free (v -> v_name);
    free ((char *) v);

    return NOTOK;
}


#ifdef SMUX
/* 
 *    NAME: f_smux ()
 *
 *    FUNCTION:
 *    Converts vector, vec, into smuxClient structure, sc, and inserts
 *    into smuxClient list.
 *    The format of a vec is:
 *
 *	smux <client OIDentifier> <password> <address> <netmask>
 *
 *    An address is either a hostname or an address in dot notation
 *    The default address and netmask are 127.0.0.1 and 255.255.255.255,
 *    (localhost) unless specified.
 *
 *    e.g (one of),
 *	smux 1.3.6.1.4.1.2.3.1.2.2
 *	smux 1.3.6.1.4.1.2.3.1.2.2  private
 *	smux 1.3.6.1.4.1.2.3.1.2.2  private  0.0.0.0 	    0.0.0.0
 *	smux 1.3.6.1.4.1.2.3.1.2.2  private  192.144.100.5  255.255.255.255
 *
 *	FUTURES: allow double entries, if they reference different hosts?
 */

f_smux (vec)
char	**vec;
{
    struct sockaddr_in *na;
    struct smuxClient *sc, *scc;
    unsigned int elements[NELEM + 1];
    OIDentifier ooid;
    int i;
    char * loopback = "loopback"; /* not necessarily 127.0.0.1 */
    struct hostent * hp;

    if (*++vec == NULL)
	return NOTOK;
    
    if ((sc = (struct smuxClient *) calloc (1, sizeof *sc)) == NULL) 
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "smux");

    /* decode client OIDentifier.  No namelookup is attempted here. */
    if ((i = str2elem (*vec, elements)) <= 1)
	return NOTOK;
    ooid.oid_elements = elements;
    ooid.oid_nelem = i;
    if ((sc -> sc_id = oid_cpy (&ooid)) == NULLOID)
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "smux");

    /* check for duplicates */
    for (scc = SHead -> sc_forw; scc != SHead; scc = scc -> sc_forw) 
	if (oid_cmp (sc -> sc_id, scc -> sc_id) == 0) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_23,
	            "f_smux: duplicate smux peer entry: %s"),
		    sprintoid (sc -> sc_id));
	    goto you_lose;
	}

    /* save passwd, if it exists */
    if (*++vec) {
	if ((sc -> sc_password = strdup (*vec)) == NULLCP)
	    adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "smux");
	vec++;
    }

    /* Set the default SMUX subagent address to the loopback address for
       this host.  This is usually but not always 127.0.0.1. (EJP) */

    if ((hp = gethostbystring (loopback)) == NULL) {
	char hostname[MAXHOSTNAMELEN];

	/* in case the nameserver doesn't define "loopback",
	   and the host does not fall back on /etc/hosts,
	   use the hostname of the box. (RLF) */
	if (gethostname (hostname, sizeof hostname) != NOTOK) 
	    hp = gethostbystring (hostname);
	if (hp == NULL) {
	    advise(SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_24,
	                "%s: host address %s unknown"), "smux", loopback);
	    goto you_lose;
	}
    }
    bcopy(hp->h_addr, &sc->sc_sin.sin_addr.s_addr, hp->h_length);
    sc -> sc_netmask.sin_addr.s_addr = inet_addr ("255.255.255.255");

    /* get address (if it exists) */
    if (*vec) {
	na = &sc -> sc_sin;
	if (strcmp (*vec,"0.0.0.0") != 0) {

	    /*
	     * An address may be either a host name or in dot address 
	     * notation.  
	     */

	    register struct hostent *hp;

	    if (hp = gethostbyname (*vec))  
		bcopy (hp -> h_addr, (char *) &(na -> sin_addr), 
		       hp -> h_length);
	    else {  

		long addr;

		/* See if the parameter is a valid address in dotted decimal
		   notation.  The address is saved and used in case it is validly 
		   formed but not known to the local name server.  (EJP) */

		if ((addr = na->sin_addr.s_addr = inet_addr(*vec)) == -1) {
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_4,
		            "%s: host address %s mis-formed"), "smux", *vec);
		    goto you_lose;
		}
		if (hp = gethostbyaddr (&addr, sizeof (addr), AF_INET) ) {
		    bcopy (hp -> h_addr, (char *) &(na -> sin_addr),
			    hp -> h_length);
		}
	    }
	}
	vec++;
    }

    /* get netmask (if it exists) */
    if (*vec) {
	if ((sc -> sc_netmask.sin_addr.s_addr = inet_addr (*vec)) == -1) {
	    if (strcmp (*vec, "255.255.255.255") != 0) {
	        advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_5,
	                "%s: netmask %s mis-formed"), "smux", *vec);
		goto you_lose;
	    }
	}
	vec++;
    }

    /* insert into smuxClient list */
    insque (sc, SHead -> sc_back);
    return OK;

you_lose:;
    if (sc -> sc_password != NULLCP)
	free (sc -> sc_password);
    if (sc -> sc_id != NULLOID)
	oid_free (sc -> sc_id);
    free ((char *) sc);
    return NOTOK;
}
#endif /* SMUX */


/*
 *    NAME: f_syscontact ()
 *
 *    FUNCTION: No op so that readconfig ignores syscontact
 *
 */
int f_syscontact(char **vec) {
  return OK;
}


/*
 *    NAME: f_syslocation ()
 *
 *    FUNCTION: No op so that readconfig ignores syslocation
 *
 */
int f_syslocation(char **vec) {
  return OK;
}


/*
 *    NAME: f_snmpd ()
 *
 *    FUNCTION:
 *    Converts vector, vec, into snmpd configuration information.
 *    The possible formats of vec are:
 *
 *	snmpd		maxpacket=<#>  querytimeout=<#>  smuxtimeout=<#>
 */
f_snmpd (vec)
char	**vec;
{
    char *key;
    int n;

    if ((key = *++vec) == NULL)
	return NOTOK;

    while (key != NULL) {
	char *sub;

	/* handle keyword=value */
	if ((sub = (char *)index (key, '=')) != NULL) {
	    *sub++ = '\0';

	    if (lexequ (key, "maxpacket") == 0) {
		n = str2int (sub);
		if (n < MINPACKET) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_25,
			    "maxpacket must be larger than %d"), MINPACKET);
		else 
		    maxopsize = n;
	    }

	    else if (lexequ (key, "querytimeout") == 0) {
		n = str2int (sub);
		if (n < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_26,
			    "%s: invalid value %s"), "querytimeout", sub);
		else if (n > 0 && n < MINTIMEOUT)
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_27,
		            "%s: value must be 0, or greater than %d"),
		            "querytimeout", MINTIMEOUT);
		else
		    stimeout = n;
	    }
	    else if (lexequ (key, "smuxtimeout") == 0) {
		n = str2int (sub);
		if (n < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_26,
			    "%s: invalid value %s"), "smuxtimeout", sub);
		else
		    smuxtimeout = n;
	    }
	    else if (lexequ (key, "smuxtrapaddr") == 0) {
		n = str2int (sub);

		if (n < 0 || n > 1)
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_26,
			    "%s: invalid value %s"), "smuttrapaddr", sub);
		else
		    smuxtrapaddr = n;
	    }
	    else if (lexequ (key, "ethernettimeout") == 0) {
		n = str2int (sub);
		if (n < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_26,
			    "%s: invalid value %s"), "ethernettimeout", sub);
		else
		    ethernettimeout = n;
	    }
	    else if (lexequ (key, "tokenringtimeout") == 0) {
		n = str2int (sub);
		if (n < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_26,
			    "%s: invalid value %s"), "tokenringtimeout", sub);
		else
		    tokenringtimeout = n;
	    }
	    else if (lexequ (key, "fdditimeout") == 0) {
		n = str2int (sub);
		if (n < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_26,
			    "%s: invalid value %s"), "fdditimeout", sub);
		else
		    fdditimeout = n;
	    }
	    else
		return NOTOK;
	}

	else 
	    return NOTOK;
	key = *++vec;
    }
     
    return OK;
}

/*
 *    NAME: set_maxpacket
 *
 *    FUNCTION: configure output packet size
 */
void
set_maxpacket ()
{
    struct fdinfo *fi;

    /* maxopsize is initialized by system limits on UDP send buffer */
    if ((fi = fd2fi (Nd)) == NULL)
	adios (MSGSTR(MS_CONFIG, CONFIG_31, "lost fi for fd %d"), Nd);

    if (maxopsize == fi -> fi_size)
	goto show_it;				/* no change */

    if (maxopsize == NOTOK)			/* first call, not set yet...*/
	maxopsize = fi -> fi_size;

    else if (maxopsize > fi -> fi_size) {	/* increase request */
	caddr_t ptr = (caddr_t) &maxopsize;
	int len = sizeof (maxopsize);

	if (setsockopt (Nd, SOL_SOCKET, SO_SNDBUF, ptr, len) == NOTOK) {
	    advise (SLOG_EXCEPTIONS, MSGSTR(MS_CONFIG, CONFIG_28,
	                "setsockopt(SO_SNDBUF) failed: %s"), strerror(errno));
	    maxopsize = fi -> fi_size;
	}
	else {

	    /* re-initialize isode PS (for asn encoding/decoding) */
	    fi -> fi_size = maxopsize;
	    ps_free (fi -> fi_ps);
	    if ((fi -> fi_ps = ps_alloc (dg_open)) == NULLPS) 
	        adios (MSGSTR(MS_CONFIG, CONFIG_32,
	                      "cannot open PS for snmp port"));
	    if (dg_setup (fi -> fi_ps, fi -> fi_fd, fi -> fi_size,
			dgram_rfx, dgram_wfx) == NOTOK) 
                adios (MSGSTR(MS_CONFIG, CONFIG_33, 
                              "cannot setup PS dgram service"));
	}
    }

show_it:;
    if (debug)
    /* NO CATALOG MESSAGE becasue nothing to translate */
	advise (SLOG_DEBUG, 
		"maxpacketsize = %d, querytimeout = %d, smuxtimeout = %d", 
		maxopsize, stimeout, smuxtimeout);
}
