static char sccsid[] = "@(#)17	1.22  src/tcpip/usr/sbin/snmpd/system.c, snmp, tcpip411, 9434A411a 8/18/94 14:32:47";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_system(), s_system(), get_sysDescr_str(), init_system()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/system.c
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

/* INCLUDES */
#include <stdio.h>
#include "snmpd.h"
#include <sys/param.h>
#if	defined(AIX) || defined(_AIX)
#include <sys/utsname.h>
#endif

extern char *configfile;

#define	SYSUPTIME	0
#define SYSCONTACT      1
#define SYSNAME         2
#define SYSLOCATION     3

#define SYSSERVICES	"72"

#define MAXSYS		256

/* sysObjectId.  Built up as follows:
 *
 * IBM.3.A.B.C.D,  where:
 *                 A = Generic Operating System, 
 *                 B = Generic Box, 
 *		   C = Specific Agent, 
 *		   D = Agent Version (on that box/os combination)
 *
==========================================================================
A=OS: 1=AIX, 2=MVS, 3=VM, 4=OS/2, 5=OS/400, 6=AOS4.3,    future
==========================================================================
B=BOX: 1=RT, 2=RISC/6000, 3=370, 4=30XX, 5=PS/2, 6=AS/400,    future
==========================================================================
C=TREE: 1=agents, 2=private,  future
==========================================================================
D=AGENT: 1=snmpd, 2=gated,  future
==========================================================================
 */
#ifdef _POWER		/* defined in /etc/xlc.cfg */
			/* IBM.3.AIX.RISC/6000.1.1.3 (version 3 of the agent) */
#define	SYSOBJECTID	"1.3.6.1.4.1.2.3.1.2.1.1.3"
#define SYSDESCR  "IBM RISC System/6000"
#endif

#ifdef RT		/* defined in isode/config.h */
#define	SYSDESCR	"IBM RT SNMP agent"
#  ifdef AIX		/* defined by compiler on 2.2.1 */
#define	SYSOBJECTID	"1.3.6.1.4.1.2.3.1.1.1.1"
#  else /* !AIX */
#define	SYSOBJECTID	"1.3.6.1.4.1.2.3.6.1.1.1"
#  endif
#endif /* RT */

#ifdef ps2		/* defined in /etc/cc.cfg */
#define	SYSDESCR	"IBM PS/2 SNMP agent"
#define	SYSOBJECTID	"1.3.6.1.4.1.2.3.1.5.1.1"
#endif

#ifdef _AIX370		/* defined in /etc/cc.cfg */
#define	SYSOBJECTID	"1.3.6.1.4.1.2.3.1.3.1.1"
#endif

#ifndef SYSDESCR	/* a generic sysDescr */
#define	SYSDESCR	"IBM SNMP agent"
#endif
#ifndef SYSOBJECTID	/* a generic sysObjectId */
#define	SYSOBJECTID	"1.3.6.1.4.1.2.3.1.3"
#endif

#define OPS_MSG     "Operating System Software:"
#define VER_MSG     "version:"

#ifdef _POWER

#include <sys/cfgdb.h>	    /* cfg db literals */
#include <sys/cfgodm.h>
#ifndef NATIVE
#  include <swvpd.h>
#endif /* NATIVE */

#define TCPIPNAME   "bos.net.tcp.client"	/* tcp/ip lpp name */

/* MESSAGES */
#define HDW_MSG     "Hardware:"
#define TM_MSG      "Machine Type:"
#define SN_MSG      "Serial Number:"
#define PROC_ID_MSG "Processor id:"
#define NET_MSG     "Networking Software:"

#define BOSNAME	    "bos.rte"

/* EXTERNS */
extern int    OdmInit;      			 /* from vpd.c */

/* The parts of the VPD data that we care about. */
static struct key vpd_data[] = {
    { TM_MSG, "TM", NULL, NULL },	/* ascii machine type/model  */
    { SN_MSG, "SN", NULL, NULL },	/* ascii serial number       */
};

#define N_VPDKEYS (sizeof (vpd_data) / sizeof (struct key))

#endif /* _POWER */

/*    SYSTEM GROUP */

static int
o_system (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int 	offset;
{
    register OT		ot = oi -> oi_type;
    int rc, ifvar;

    ifvar = (int) ot -> ot_info;
    if ((rc = o_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return rc;
    
    switch (ifvar) {
	case SYSUPTIME:
	{
	    struct timeval  now;
	    integer diff;

	    if (gettimeofday (&now, (struct timezone *) 0) == NOTOK) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_1,
			"GENERR: gettimeofday"));
		return int_SNMP_error__status_genErr;
	    }
	    diff = (now.tv_sec - my_boottime.tv_sec) * 100
		+ ((now.tv_usec - my_boottime.tv_usec) / 10000);

	    return o_number (oi, v, diff);
	}

	case SYSNAME:
	{
	    char hostname[MAXHOSTNAMELEN];

	    if (gethostname (hostname, sizeof hostname) == NOTOK) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_2,
			"GENERR: gethostname"));
		return int_SNMP_error__status_genErr;
	    }
	    return o_string (oi, v, hostname, strlen (hostname));
	}

#ifdef _POWER
	case SYSLOCATION:
	case SYSCONTACT:
	{
            char answer[MAXSYS];
            char comm[160];
            char filename[L_tmpnam];
            FILE *fp;
            char buffer[300];
            char *p, *q;

            tmpnam(filename);

            sprintf(comm, "fgrep -ie %s %s | egrep -iv \"[#] *[s][y][s][cl][o]\" | tail -1 > %s",
                    ifvar == SYSCONTACT ? "syscontact" : "syslocation" , 
		    configfile, filename);

            system(comm);

            if ((fp = fopen(filename, "r")) == NULL) {
   	      advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_3,
  	          "GENERR in o_system: unable to open config file: error %d"),	
		  errno);
	      return int_SNMP_error__status_genErr;
            }

            /*
	     * Get line from file, and point at the first " 
	     * after point at sysc or sysl 
	     */
            fgets(buffer, 300, fp);

            if (ifvar == SYSCONTACT) {
              if ((p = (char *)strstr(buffer, "syscontact")) == NULL) {
                strcpy(answer, "");
                /* Remove temporary file */
	        fclose(fp);
	        if (unlink(filename) < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_4,
			    "GENERR in o_system: could not remove tmp file"));
	        return o_string (oi, v, answer, strlen (answer));
              }
            }
            else {
              if ((p = (char *)strstr(buffer, "syslocation")) == NULL) {
                strcpy(answer, "");
                /* Remove temporary file */
	        fclose(fp);
	        if (unlink(filename) < 0) 
		    advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_4,
			    "GENERR in o_system: could not remove tmp file"));
	        return o_string (oi, v, answer, strlen (answer));
              }
            }

            if ((p = (char *)strchr(p, '\"')) == NULLCP) {
              strcpy(answer, "");
              /* Remove temporary file */
	      fclose(fp);
	      if (unlink(filename) < 0) 
		  advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_4,
			  "GENERR in o_system: could not remove tmp file"));
	      return o_string (oi, v, answer, strlen (answer));
            }
 
            p++;
            /* Get rid of newline, if it is there */
            if ((q = (char *)strchr(p, '\n')) != NULLCP)
              *q = '\0';
            /* Terminate string at last " if it is there */
            if ((q = (char *)strchr(p, '\"')) != NULLCP)
              *q = '\0';

            /* Copy at most MAXSYS - 1 bytes into answer */
            strncpy(answer, p, MAXSYS - 1);
	    answer[MAXSYS - 1] = '\0';
  
            fclose(fp);

            /* Remove temporary file */
	    if (unlink(filename) < 0) 
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_4,
			"GENERR in o_system: could not remove tmp file"));

	    return o_string (oi, v, answer, strlen(answer));
	}
#endif
    }
    return int_SNMP_error__status_noSuchName;
}

/*  */

/* 
 * NAME:  s_system ()
 *
 * FUNCTION: sets the sysName, sysContact, and sysLocation.
 *
 * RETURNS: int_SNMP_error__status_noError	(success) 
 *	    SNMP_error__status			(failure)
 */
static int
s_system (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    register OT		ot = oi -> oi_type;
    register OS		os = ot -> ot_syntax;
    register char	*s = NULLCP;
    int 		ifvar;
    int	status = int_SNMP_error__status_noError;

    ifvar = (int) ot -> ot_info;

    if ((status = s_icheck (oi, v, offset)) != int_SNMP_error__status_noError)
	return status;

    /* initial checking... */
    switch (offset) {
	case type_SNMP_PDUs_rollback:
	    return status;	/* rollback need not continue */

	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    if ((*os -> os_decode) (&ot -> ot_save, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;

	    if ((s = qb2str ((struct qbuf *) ot -> ot_save)) == NULLCP) {
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_5,
			"GENERR in s_system: qb2str failed"));
		if (ot -> ot_save)
		    (*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
		return int_SNMP_error__status_genErr;
	    }

	    if (ot -> ot_save)
		(*os -> os_free) (ot -> ot_save), ot -> ot_save = NULL;
	    break;
	
	default:
	    return int_SNMP_error__status_genErr;
    }

    /* length checks, plus the commit stage */
    switch (ifvar) {

	case SYSNAME:
	    switch (offset) {
		case type_SNMP_PDUs_set__request:
		    if (strlen (s) > MAXHOSTNAMELEN)
			status = int_SNMP_error__status_badValue;
		    break;

		case type_SNMP_PDUs_commit:
#ifdef DEBUG
		    /* NOT NEED IN CATALOG, BECAUSE DEGUB */
		    if (debug > 3)
			advise (SLOG_DEBUG, 
				"s_system: setting hostname to: \"%s\"", s);
#endif
		    if (sethostname (s, strlen (s)) == NOTOK) {
			advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_6,
			    "GENERR: sethostname failed: %s"), strerror(errno));
			status = int_SNMP_error__status_genErr;
		    }
		    break;
	    }
	    break;

#ifdef _POWER
	case SYSLOCATION:
	case SYSCONTACT:
	    switch (offset) {
		case type_SNMP_PDUs_set__request:
		    if (strlen (s) > MAXDISPLAYLEN)
			status = int_SNMP_error__status_badValue;
		    break;
		case type_SNMP_PDUs_commit:
                {
                    FILE *fp;
                    char output[300];
		    char filename[L_tmpnam];

		    tmpnam(filename);
		    sprintf(output, "egrep -iv %s %s > %s",
		       ifvar == SYSCONTACT ? "\\^syscontact" : "\\^syslocation",
			    configfile, filename);
		    system(output);

		    sprintf(output, "mv %s %s", filename, configfile);
		    system(output);

                    if ((fp = fopen(configfile, "a")) == NULL) {
   	              advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_7,
              "GENERR in s_system: unable to open config file: error %d"), 
		              errno);
		      status = int_SNMP_error__status_genErr;
                    }
		    else {
                      sprintf(output, "%s \"%s\"", 
		           ifvar == SYSCONTACT ? "syscontact" : "syslocation",
		           s);
                      if (fprintf(fp, "%s\n", output) < 0) {
   	                advise (SLOG_EXCEPTIONS, MSGSTR(MS_SYSTEM, SYSTEM_8,
               "GENERR in s_system: unable to write config file: error %d"), 
				errno);
		        status = int_SNMP_error__status_genErr;
	              }	
                      fclose(fp);
                    }
		    break;
                }
	    }
	    break;
#endif /* _POWER */

	default:
	    status = int_SNMP_error__status_noSuchName;
    }

    if (s != NULLCP)
	free (s);
    
    return status;
}

/*  */

/*
 * NAME:  get_sysDescr_str ()
 *
 * FUNCTION:  
 * Gets the information required for sysDescr.  This includes the 
 * operating system, the system hardware, and the networking
 * software.  
 *
 * NOTE: I use CSSERVER_TABLE to distinguish between old swvpd and
 *	 new swvpd.  It was never used in 3.1, and deleted going into
 *	 3.2.
 */

#ifndef CSSERVER_TABLE
#define SWVPD3_2
#endif

static void
get_sysDescr_str (outbuf)
char *outbuf;
{
#if	defined(AIX) || defined(_AIX)
    struct utsname utsname;
    int urc;
    char *na = NA_MSG,
	 *ops_msg = MSGSTR(MS_SYSTEM, SYSTEM_12, OPS_MSG),
	 *ver_msg = MSGSTR(MS_SYSTEM, SYSTEM_13, VER_MSG);
    char opsys[128];
#endif	/* AIX || _AIX */
#ifdef _POWER
#ifndef NATIVE
#ifndef SWVPD3_2
    struct history history;
#endif
    struct lpp lpp;
    struct CuAt *CuAt;
    struct CuVPD *CuVPD;
    struct listinfo lst_info;
    int vpd_stat;
    char criteria[128];
    char *tcpipName = TCPIPNAME,
	 *machtype_msg = MSGSTR(MS_SYSTEM, SYSTEM_9, TM_MSG),
	 *proc_id_msg = MSGSTR(MS_SYSTEM, SYSTEM_10, PROC_ID_MSG),
	 *net_msg = MSGSTR(MS_SYSTEM, SYSTEM_11, NET_MSG);
    char hardware[128], network[128];
    char *bosName = BOSNAME;
    char *odm_path = -1;
#endif /* NATIVE */ 
#endif /* _POWER */

#if	defined(AIX) || defined(_AIX)
    /* 
     * Get operating system name and version/release from uname().
     */
    bzero ((char *) &utsname, sizeof (utsname));
    if ((urc = uname (&utsname) == NOTOK))
	strcpy (opsys, na);
    else 
	sprintf (opsys, "%s %s %s %s.%s", ops_msg, utsname.sysname, 
		                 ver_msg, utsname.version, utsname.release);

#endif	/* AIX || _AIX */

#ifdef _POWER
#ifndef NATIVE
    /* initialize ODM */
    if (OdmInit == NOTOK) {
	sprintf (hardware, "%s %s %s %s", machtype_msg, na, 
		    proc_id_msg, (urc == NOTOK) ? na : utsname.machine);
	sprintf (network, "%s %s", net_msg, na);
    }
    else {
	/* 
	 * Get system hardware type.
	 * First search CuVPD for user entered "sysunit" information.
	 * If that fails, then use CuAt for machine model code 
	 * and uname for processor id.
	 * The assumption here is that if the user enters in VPD data,
	 * it will be correct, so that is what we will give back, verbatim.
         */
	sprintf (criteria, "name = sysunit0 and vpd_type = %d", USER_VPD);
	CuVPD = get_CuVPD_list (CuVPD_CLASS, criteria, &lst_info, 1, 1);

	/* parse sysunit VPD info */
	if (lst_info.num >= 1) {
	    parse_VPD (CuVPD -> vpd, hardware, vpd_data, N_VPDKEYS);
	    odm_free_list (CuVPD, &lst_info); 
	}
	else {  /* no VPD info.  Try CuAt instead */	
	    strcpy (criteria, "name = sys0 and attribute = modelcode");
	    CuAt = get_CuAt_list (CuAt_CLASS, criteria, &lst_info, 1, 1);
	    sprintf (hardware, "%s %s %s %s", machtype_msg,
			 (lst_info.num < 1) ? na : CuAt -> value, proc_id_msg,
			 (urc == NOTOK) ? na : utsname.machine);
	    odm_free_list (CuAt, &lst_info); 
	}
	
	/*
	 * Get BOS information from the VPD for local purposes and
	 * use the full version id including modification and fix
	 * history.
	 */
	strcpy (lpp.name, bosName);
	vpd_stat = vpdget (LPP_TABLE, LPP_NAME, &lpp);
	if (vpd_stat == VPD_OK) {
#ifdef SWVPD3_2		/* new swvpd format */
	    /* 
	     * NOTES: (as of 9119.  Revisit this code closer to ship!)
	     *	      -) currently, lpp.description may not exist
	     *	      -) currently, bos.obj has no versioning information.
	     *		 atoi's make an *Assumption* that we will always
	     *		 have digits in utsname.version and release
	     */
	    sprintf (opsys, "%s %s %s %02d.%02d.%04d.%04d",
		    strlen (lpp.description) ? lpp.description : ops_msg,
		    utsname.sysname, ver_msg, 
		    lpp.ver ? lpp.ver : atoi (utsname.version),
		    lpp.rel ? lpp.rel : atoi (utsname.release),
		    lpp.mod, lpp.fix);
#else
	    strcpy (history.lpp_name, bosName);
	    history.state = HIST_ACTIVE;
	    vpd_stat = vpdget (HISTORY_TABLE, HIST_LPP_NAME | 
				HIST_STATE, &history);
	    if (vpd_stat == VPD_OK) 
		sprintf (opsys, "%s %s %s %s.%s.%s.%s",
			strlen (lpp.description) ? lpp.description : ops_msg,
			utsname.sysname, 
			ver_msg, history.ver, history.rel, history.mod, 
			history.fix);
#endif
	}

	/* 
	 * Get networking software.  TCP/IP information is retrieved
	 * from the VPD.
	 */
	strcpy (lpp.name, tcpipName);
	vpd_stat = vpdget (LPP_TABLE, LPP_NAME, &lpp);
	if (vpd_stat != VPD_OK) {
	    sprintf (network, "%s %s", net_msg, na);
	}
	else {
#ifdef SWVPD3_2		/* new swvpd format */
	    sprintf (network, "%s %s %02d.%02d.%04d.%04d",
		    strlen (lpp.description) ? lpp.description : net_msg,
		    ver_msg, 
		    lpp.ver, lpp.rel, lpp.mod, lpp.fix);
#else
	    strcpy (history.lpp_name, tcpipName);
	    history.state = HIST_ACTIVE;
	    vpd_stat = vpdget (HISTORY_TABLE, HIST_LPP_NAME | 
				HIST_STATE, &history);
	    if (vpd_stat == VPD_OK) {
		/* 
		 * Use full version id, including modification and 
		 * fix history, for local purposes.
		 */
		sprintf (network, "%s %s %s.%s.%s.%s", 
			strlen (lpp.description) ? lpp.description : net_msg,
			ver_msg, 
			history.ver, history.rel, history.mod, history.fix);
	    }
	    else
		sprintf (network, "%s %s", net_msg, lpp.description);
#endif
	}

	/* 
	 * this code only gets run once.  After runing thru vpd*() routines,
	 * we have re-set ODMDIR.  Correct this now (once), so that 
	 * raw odm routines in interfaces.c will get thr correct information.
	 */
	odm_path = odm_set_path ("/etc/objrepos");
	if (odm_path != -1)
	    free(odm_path);
    }

    /* 
     * Combine all of our parts into one sysDescr string.
     */
    sprintf (outbuf, "%s\n%s\n%s\n%s", SYSDESCR, hardware, opsys, network);
#else /* NATIVE */
    sprintf (outbuf, "%s\n%s", SYSDESCR, opsys);    /* sysDescr + utsname */
#endif /* NATIVE */
#else /* !_POWER */
#if	defined(AIX) || defined(_AIX)
    sprintf (outbuf, "%s\n%s", SYSDESCR, opsys);    /* sysDescr + utsname */
#else
    strcpy (outbuf, SYSDESCR);			/* very generic sysDescr */
#endif	/* AIX || _AIX */
#endif /* _POWER */
}

/*  */

/*
 * NAME:  init_system ()
 *
 * FUNCTION:  Initialize system mib variables.
 */
init_system ()
{
    register OT	    ot;

    if (ot = text2obj ("sysDescr")) {
	ot -> ot_getfnx = o_generic;
	if (ot -> ot_syntax) {
	    char    buffer[BUFSIZ];

	    get_sysDescr_str (buffer);
	    (void) (*ot -> ot_syntax -> os_parse) ((struct qbuf **)
			&ot -> ot_info, buffer);
	}
    }
    if (ot = text2obj ("sysObjectID")) {
	ot -> ot_getfnx = o_generic;
	if (ot -> ot_syntax)
	    (void) (*ot -> ot_syntax -> os_parse) 
		    ((struct qbuf **) &ot -> ot_info, SYSOBJECTID);
    }
    if (ot = text2obj ("sysUpTime")) 
	ot -> ot_info = (caddr_t) SYSUPTIME,
	ot -> ot_getfnx = o_system;
    if (ot = text2obj ("sysContact")) {
#ifdef _POWER
	ot -> ot_info = (caddr_t) SYSCONTACT;
	ot -> ot_getfnx = o_system;
	ot -> ot_setfnx = s_system;		/* setable! */
#else
	ot -> ot_getfnx = o_igeneric;		/* initializable */
	ot -> ot_setfnx = s_generic;		/* setable! */

	/* just in case it's not in snmpd.conf - init to NULL */
	if (ot -> ot_syntax)
	    (void) (*ot -> ot_syntax -> os_parse) 
		    ((struct qbuf **) &ot -> ot_info, "");
#endif /* _POWER */
    }
    if (ot = text2obj ("sysName")) 
	ot -> ot_info = (caddr_t) SYSNAME,
	ot -> ot_getfnx = o_system,
	ot -> ot_setfnx = s_system;		/* setable! */
    if (ot = text2obj ("sysLocation")) {
#ifdef _POWER
	ot -> ot_info = (caddr_t) SYSLOCATION;
	ot -> ot_getfnx = o_system;
	ot -> ot_setfnx = s_system;		/* setable! */
#else
	ot -> ot_getfnx = o_igeneric;		/* initializable */
	ot -> ot_setfnx = s_generic;		/* setable! */

	/* just in case it's not in snmpd.conf - init to NULL */
	if (ot -> ot_syntax)
	    (void) (*ot -> ot_syntax -> os_parse) 
		    ((struct qbuf **) &ot -> ot_info, "");
#endif /* _POWER */
    }
    if (ot = text2obj ("sysServices")) {
	ot -> ot_getfnx = o_generic;
	if (ot -> ot_syntax)
	    (void) (*ot -> ot_syntax -> os_parse)
		    ((struct qbuf **) &ot -> ot_info, SYSSERVICES);
    }
}
