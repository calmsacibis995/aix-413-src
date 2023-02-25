static char sccsid[] = "@(#)03	1.11  src/tcpip/usr/sbin/snmpd/logging.c, snmp, tcpip411, 9434A411a 8/18/94 14:32:04";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: shouldfork(), init_log_struct(), init_prelogging(),
 *            init_logging(), init_log(), start_log(), stop_log(), 
 *            dump_bfr(), advise(), adios(), newlogsize(), 
 *            setmaxlogsize(), rotate_log()
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
 * FILE:	src/tcpip/usr/sbin/snmpd/logging.c
 */

#include <isode/snmp/io.h>
#include "snmpd.h"
#include <varargs.h>
#if	defined(_POWER)
#include <sys/resource.h>
#endif
#if	defined (AIX) && defined (RT)
#include <sys/syslog.h>
#else
#include <syslog.h>
#endif

/* EXTERN */
extern  char  *Myname;
extern	int    debug;
extern	int    debug_default;

extern	char  *strdup ();

/* GLOBAL */
struct logging *logging = NULL;
struct logging  log_stderr = { NULLCP, stderr, OK, 0, 0 };
struct logging  log_cmdline;
struct logging  log_conffile;

struct priority_table {
    char  *name;     /* Embedded in syslog string       */
    int    value;    /* Priority as defined in syslog.h */
};

struct priority_table priority[] = {
    "",		    LOG_ERR,
    "EMERGENCY: ",  LOG_EMERG,
    "ALERT: ", 	    LOG_ALERT,
    "CRITICAL: ",   LOG_CRIT,
    "FATAL: ",	    LOG_ERR,
    "EXCEPTIONS: ", LOG_ERR,
    "WARNING: ",    LOG_WARNING,
    "NOTICE: ",     LOG_NOTICE,
    "PDUS: ", 	    LOG_INFO,
    "TRACE: ", 	    LOG_INFO,
    "DEBUG: ", 	    LOG_DEBUG,
    "DEFAULT: ",    LOG_INFO
};


/* FORWARD */
void	dump_bfr (), advise (), adios ();
void    init_log_struct (), init_log ();
void	start_log (), stop_log ();

#ifdef _POWER
void    rotate_log ();
void    setmaxlogsize ();
#endif /* _POWER */


/*
 * NAME: shouldfork ()
 *
 * FUNCTION:
 *	all the reason's why this daemon should not fork...
 */
int
shouldfork()
{
#ifdef SRC_SUPPORT
    if (init_src() == OK)
	return NOTOK;
#endif

#ifdef DEBUG
    /* "snmpd -f -" case. */
    if (log_cmdline.l_logfp == stderr)
	return NOTOK;
#endif

    /* FALL-THRU... "snmpd" or "snmpd -f <some_file>" case */
    return OK;
}


/* LOGGING functions */


/*
 * NAME: init_log_struct ()
 *
 * FUNCTION: Initialize a logging structure.
 *
 */
void 
init_log_struct (logstruct, filename, on_or_off)
struct logging  *logstruct;
char            *filename;
int              on_or_off;
{
    logstruct -> l_logfp = NULL;
    if (filename == NULLCP)
        logstruct -> l_filename = NULLCP;
    else {
        logstruct -> l_filename = strdup (filename);
#ifdef DEBUG
	if (strlen (filename) == 1 && *filename == '-')
	    logstruct -> l_logfp = stderr;
#endif
    }
    logstruct -> l_level = debug_default;
    logstruct -> l_size = 0;
    logstruct -> l_enabled = on_or_off;
}


/*
 * NAME: init_prelogging
 *
 * FUNCTION: setup syslog, plus all pre-startup initialization
 *	     of the logging functions.
 */
void
init_prelogging ()
{
    /* 
     * Set the logging pointer that usage statements go to stderr.
     * Initialize the log file maintained by the syslogd command.  If there 
     * is no entry in /etc/syslog.conf to handle daemon facilities, nothing
     * is written to the log file by the syslogd command.  
     */
    logging = &log_stderr;
    openlog ("snmpd", LOG_PID | LOG_ODELAY, LOG_DAEMON);

    /*
     * Initialize the logging structures to the defaults.  There are two
     * logging structures:  one for command line directed logging and one
     * for configuration file directed logging.  The user can refresh the
     * configuration file logging structure with an src refresh command or
     * with a kill -1 signal.  This ability to refresh the configuration file
     * logging structure forces us to use two logging structures.
     */
    init_log_struct (&log_cmdline, NULLCP, NOTOK);
    init_log_struct (&log_conffile, NULLCP, NOTOK);
}

/*
 * NAME: init_logging
 *
 * FUNCTION: startup logging utilities for snmpd.
 */
void
init_logging ()
{
    /*
     * If the debug level is specified on the command line, then that debug 
     * level becomes the program default.  Reset the debug level in the 
     * log_conffile structure.  This allows a user-settable default debug level 
     * for syslogd.  If level is specified in the configuration file, then
     * the configuration file debug level controls syslog logging.
     */
    log_conffile.l_level = log_cmdline.l_level = debug;

    /* initialize command line logging early */
    if (log_cmdline.l_enabled == OK) {
	logging = &log_cmdline;
	init_log (NULLCP, OK, FALSE);

	/* 
	 * fall back to stderr if init_log failed 
	 * (most likely due to an invalid file name)
	 */
	if (logging -> l_enabled == NOTOK)
	    logging = &log_stderr;
    }

    /* 
     * Read configuration file and parse the logging vectors.  This PASS1
     * through readconfig() only sets up the logging structure.
     */
    readconfig (PASS1);		

    /*
     * If logging was not specified on the command line, then use the
     * logging information in the log_conffile structure from the snmpd.conf 
     * file.  If there is no logging information in the snmpd.conf file,
     * the log_conffile will just have the defaults, so no logging will take
     * place at startup anyway because the filename is NULLCP.
     */
    if (logging == &log_stderr) {
	logging = &log_conffile;
	init_log (NULLCP, OK, FALSE);
    }

    /* 
     * Initialize external logging routines.
     */
    o_advise = (IFP) advise;	  /* for libsnmp.a */
    o_adios = (IFP) adios;	  /* for libsnmp.a */
    o_dumpbits = (IFP) dump_bfr;  /* for io.c in libsnmp.a. */
}

/*
 * NAME: init_log ()
 *
 * FUNCTION: Initializes the logging file specified on the command line
 * or in the configuration file.  It has no effect on the syslog logging.
 *
 */
void
init_log (filename, enabled, refresh)
char	*filename;
int      enabled;       /* Current state of logging */
int	 refresh;       /* Refresh rather than initialize log */
{
    int  new_enable = logging -> l_enabled;  /* Temporarily save it */

    /*
     * If there is no filename and no logging -> l_filename, just return.
     * There has been no logging and there will be no logging.
     */
    if ((filename == NULLCP) && (logging -> l_filename == NULLCP)) 
	return;

    /*
     * The debug level may have changed.
     */
    debug = logging -> l_level;

    /*
     * If this is a refresh, the logging configuration could have changed
     * if logging is directed by the configuration file.  We must check for
     * changes in the log filename, the filesize, and if logging is enabled
     * or disabled.  If the old filename (filename) is NULLCP, then do not
     * execute this if statement since no logging is currently taking place.
     * NOTE: If logging is directed by command line specifications, this
     * refresh is a do-nothing operation because the configuration cannot
     * change. The logging pointer points to the logging structure being
     * used.  If logging is based on the command line specifications,
     * the logging structure will not change.
     */
    if (refresh && (filename != NULLCP) && (enabled == OK)) {
	
	/* 
	 * If the user deleted the filename from the configuration file,
	 * stop the logging process and just return.  Logging should not
	 * be restarted.  Reset enablement because stop_log() turns it off.
	 */
	if (logging -> l_filename == NULLCP) {
	    stop_log (filename, TRUE);
	    logging -> l_enabled = new_enable;
	    return;
	}

	/*
	 * Check if the filename, filesize, or enablement status have changed.
	 * If one of them has changed, stop the current logging process.
	 */
	if ((strcmp (filename, logging -> l_filename)) ||
#ifdef _POWER
		(newlogsize ()) || 
#endif /* _POWER */
		(logging -> l_enabled == NOTOK)) {
#ifdef DEBUG
	    syslog (LOG_DEBUG, "file name, size, or enablement change");
	    if (strcmp (filename, logging -> l_filename))
	        syslog (LOG_DEBUG, "file name change: filename = %s l_filename = %s",                        filename, logging -> l_filename);
	    if (logging -> l_enabled == NOTOK)
		syslog (LOG_DEBUG, "logging -> l_enabled = NOTOK");
#endif
	    if (strcmp (filename, logging -> l_filename)) 
	        stop_log (filename, TRUE);
	    else
	        stop_log (logging -> l_filename, TRUE);

	    logging -> l_enabled = new_enable;
        }
	else {
	    /*
	     * No change in logging.  Just continue logging.
	     */
	    free (filename);
	    return;
	}

    }  /* if (refresh) */

    /*
     * Free up the filename space.  This space was allocated by refresh().
     */
    if (refresh && (filename != NULLCP))
	free (filename);

    if (logging -> l_enabled == NOTOK) 
        return;

    /*
     * If the user has specified a maximum log file size in the snmpd.conf
     * file, set the file size limit.  Unlimited filesize is specified by 0 
     * (zero).  Unlimited is the default.  This operation controls the limit
     * of ALL files created by the snmpd agent. 
     */

#ifdef _POWER
    setmaxlogsize ();
#endif /* _POWER */

    /*
     * Start the logging process.  TRUE indicates that a message should
     * be written to the log file stating that logging is starting.
     */
    start_log (TRUE);
}


/*
 * NAME: start_log ()
 *
 * FUNCTION: Opens the logging file.
 */
void
start_log (log_msg)
int    log_msg;       /* Whether or not to call advise() */
{
#ifdef DEBUG
    /* 
     * logfp will always be null at this point, unless writing to stderr.
     */
    if (logging -> l_logfp != stderr)
#endif
	if (logging -> l_filename != NULLCP && *logging -> l_filename != '-')
	    logging -> l_logfp = fopen (logging -> l_filename, "a");

    if (logging -> l_logfp == NULL) {
	logging -> l_enabled = NOTOK;
	if (logging -> l_filename != NULLCP) {
	    if (*logging -> l_filename == '-')
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_LOG, LOG_1,
			"invalid logname: \"%s\""), logging -> l_filename);
	    else
		advise (SLOG_EXCEPTIONS, MSGSTR(MS_LOG, LOG_2,
			"%s cannot be opened for logging: %s"),
			logging -> l_filename, strerror(errno));
	}
	else
	    advise (SLOG_NOTICE, MSGSTR(MS_LOG, LOG_3,
			"%s is not configured for logging"), Myname);
	    
	return;
    }

    /*
     * Set up line buffering so that we get all the messages
     * instead of losing some if the buffer is not full.
     */
    setlinebuf (logging -> l_logfp);

    logging -> l_enabled = OK;

    if (log_msg)
        advise (SLOG_NOTICE, MSGSTR(MS_LOG, LOG_4,
		"logging started at level %d"), debug);

    /*
     * Set isode printing from stdout to logfp for SLOG_TRACE.
     * The isode code provides the English tracing.
     */
    vsetfp (logging -> l_logfp, NULLCP);
}


/*
 * NAME:  stop_log ()
 *
 * FUNCTION: Closes the logging file and sets logging -> l_enabled to NOTOK.
 *
 * The logging file name has to be passed in because the log file may not
 * be the same one in logging -> l_filename as the user can cause snmpd
 * to reread the configuration file at any time.
 */
void
stop_log (filename, log_msg)
char  *filename;      /* Log filename to close */
int    log_msg;       /* Whether or not to call advise() */
{
    if (logging -> l_logfp != NULL
#ifdef DEBUG
	    && logging -> l_logfp != stderr
#endif
	    ) {
        if (log_msg) {
	    /*
	     * Temporarily turn on logging.  The enablement may have been
	     * overwritten by NOTOK if the configuration file was just reread.
	     */
	    logging -> l_enabled = OK;
    	    advise (SLOG_NOTICE, MSGSTR(MS_LOG, LOG_5,
		    "logging to %s is terminating"), filename);
	}
	fclose (logging -> l_logfp);
	logging -> l_logfp = NULL;
    }
    else
	advise (SLOG_NOTICE, MSGSTR(MS_LOG, LOG_6,
		"%s is not configured for logging"), Myname);

    logging -> l_enabled = NOTOK;
}


/* 
 * NAME: dump_bfr ()
 *
 * FUNCTION: Dump bytes in a buffer if debug > 1.
 */
void
dump_bfr (buf, buf_len)
char  *buf;
int    buf_len;
{
    register int i, n;
    char	 buffer[MAXMSGSIZE];
    char         hex_string[4];
    char        *bp;
    int          len;
    int          p_level;          /* Priority level */
    int          maxmsg = 320;     /* Bytes to be converted to hex per loop */

    if (debug <= 1)
	return;

    p_level = SLOG_PDUS;

    /*
     * Each byte read in takes up three bytes in the buffer string
     * because each byte is converted to a 2 digit hex number and
     * a blank or '\n' is tacked on the end.  maxmsg limits the number
     * of bytes read from buf so that buffer[] does not overflow.
     */
    if (buf_len  > maxmsg) 
	len = maxmsg;
    else
	len = buf_len;

    bp = buffer;

    /*
     * Walk through buf[], converting each byte in buf to three bytes
     * and storing the result in buffer[].  buffer[] is limited to
     * MAXMSGSIZE by requirements for syslog() via advise().  Limit
     * buffer[] so that each line has 16 hex numbers, totaling 48 bytes,
     * with a maximum of 20 lines.  This totals 960 bytes.  The first
     * buffer contains the SLOG_PDUS priority name string.  All continuation
     * buffers have no priority name string.
     */

    do {

	n = 0;
        for (i = 0; i < len; i++) {
	    if (n == 0)
		*bp++ = '\n';
	    sprintf (hex_string, " %2.2x", *buf++);
	    *bp++ = hex_string[0];
	    *bp++ = hex_string[1];
	    *bp++ = hex_string[2];
	    if (++n > 15)
		n = 0;
        }
        *bp++ = '\0';

        advise (p_level, buffer);

        bp = buffer;

	p_level = SLOG_NONE;
	buf_len -= maxmsg;
	if (buf_len < maxmsg)
	    len = buf_len;

    } while (buf_len > 0);
}


/*
 * NAME:  advise ()
 *
 * FUNCTION: builds and prints a log message to both syslogd and to the
 * snmpd log file if such a log file is specified.  If the log message has 
 * the priority SLOG_FATAL, advise() does *NOT* exit() snmpd.
 */
void
advise (va_alist)
va_dcl
{
    int		event;
    char       *format;
    char       *prior;
    char       *msg_format;
    va_list	ap;
    long    	clock;
    register struct tm *tm;
    char        buffer[MAXMSGSIZE];    /* syslog() buffer limit      */
    char        time_str[20];        /* Time stamp for log message */

    va_start (ap);

    /* advise is always called like: 
     *   advise (EVENT, printf_format, args)
     * Parse out the EVENT first.  The event specifies the priority of 
     * the message.
     */
    event = va_arg (ap, int);
    if (event > SLOG_DEFAULT) 
	event = SLOG_DEFAULT;

    /* 
     * Print out a time stamp if logging is enabled and the EVENT is a
     * valid priority.  syslog() puts its own time stamp in its logfile,
     * so we only deal with the snmpd log file here.  The logging pointer
     * may be NULL!
     */
    *time_str = '\0';
    if ((logging -> l_logfp != NULL) && (event != SLOG_NONE)) {
	(void) time (&clock);
	tm = localtime (&clock);
	strftime(time_str, 19, "%D %T ", tm);
    }

    /*
     * Build the log message.  The message format is:
     *   PRIORITY: message
     * Neither vsprintf() nor _doprnt() check buffer limitations.  It assumes 
     * you have passed a pointer to an adequate sized buffer.  This is a 
     * potential problem area.  The coder can stomp on memory since there is 
     * no boundary checking done.
     */
    format = va_arg (ap, char *);
    prior = MSGSTR(MS_LOG, LOG_12 + event, priority[event].name);
    if ((msg_format = (char *)malloc (strlen (format) + strlen (prior) + 2)) ==
	NULL) {
	adios (MSGSTR(MS_GENERAL, GENERA_1, "%s: Out of Memory"), "advise");
    }

    sprintf (msg_format, "%s%s", prior, format);

#ifndef VSPRINTF
    FILE  strbuf;

    strbuf._flag = _IOWRT+_IOSTRG;
    strbuf._ptr = buffer;
    strbuf._cnt = 32767;
    _doprnt (msg_format, ap, &strbuf);
    putc('\0', &strbuf);
#else
    vsprintf (buffer, msg_format, ap);
#endif  /* VSPRINTF */

    free (msg_format);

    /*
     * Write this entry to syslogd and to the snmpd log file.  syslogd provides
     * its own time stamp.  If the snmpd log file has reached its size limit, 
     * rotate the log file.  Then rewrite the message again.  No logging is
     * allowed if logging == NULL!
     */

    if (event != SLOG_TRACE)
        syslog (priority[event].value, "%s", buffer);

    if ((logging -> l_enabled == OK) && (logging -> l_logfp != NULL)) {
	if (fprintf (logging -> l_logfp, "%s%s\n", time_str, buffer) < 0) {
#ifdef _POWER
            if (errno == EFBIG) {
	        rotate_log();
                fprintf (logging -> l_logfp, "%s%s\n", time_str, buffer);
	    }
	    else {
#endif /* _POWER */
                syslog (priority[SLOG_EXCEPTIONS].value, 
	                MSGSTR(MS_LOG, LOG_10, 
				"%scannot write to log file %s: %s"),
	                MSGSTR(MS_LOG, LOG_17, priority[SLOG_EXCEPTIONS].name), 
	                logging -> l_filename, strerror(errno));
#ifdef _POWER
	    }
#endif /* _POWER */
	} 
    }

    va_end (ap);
}


/*
 * NAME: adios ()
 *
 * FUNCTION: Terminates snmpd on the event of a fatal error.
 *           MAXMSGSIZE limit imposed by syslogd.
 */
void
adios (va_alist)
va_dcl
{
    char        buffer[MAXMSGSIZE];
    char       *format;
    va_list	ap;
    
    va_start (ap);

    /* adios is always called like: 
     *   adios (printf_format, args)
     * Build the exit message string in buffer[] for advise().
     * Neither vsprintf() nor _doprnt() check buffer limitations.  It assumes 
     * you have passed a pointer to an adequate sized buffer.  This is a 
     * potential problem area.  The coder can stomp on memory since there is 
     * no boundary checking done.
     */
    format = va_arg (ap, char *);

#ifndef VSPRINTF
    FILE  strbuf;

    strbuf._flag = _IOWRT+_IOSTRG;
    strbuf._ptr = buffer;
    strbuf._cnt = 32767;
    _doprnt (format, ap, &strbuf);
    putc('\0', &strbuf);
#else
    vsprintf (buffer, format, ap);
#endif  /* VSPRINTF */

    va_end (ap);

    /*
     * Write the killer message and exit.  The FALSE tells snmpd_exit()
     * that snmpd_exit() is not called by doit_src().
     */
    advise (SLOG_FATAL, buffer);
    snmpd_exit (1, FALSE);
}


#ifdef _POWER
/*
 * NAME: newlogsize ()
 *
 * FUNCTION: Determines if the log file size parameter in the configuration
 * file has changed value.
 *
 * RETURNS:   TRUE if the file size parameter has changed.
 *            FALSE if the file size parameter has not changed.
 */
static int
newlogsize ()
{
    struct rlimit   rl;

    if (getrlimit (RLIMIT_FSIZE, &rl) != 0) 
	adios (MSGSTR(MS_LOG, LOG_8, "getrlimit failed: %s"), strerror(errno));

    /*
     * If the requested size is 0 (zero), it means unlimited file size.
     */
    if ((logging -> l_size == 0) && (rl.rlim_cur == rl.rlim_max))
	return FALSE;
    
    if (logging -> l_size == rl.rlim_cur)
	return FALSE;
    
    return TRUE;
}


/*
 * NAME: setmaxlogsize ()
 *
 * FUNCTION: Sets the log file size parameter as specified in the configuration
 * file.
 */
void
setmaxlogsize ()
{
    struct rlimit   rl;

    if (getrlimit (RLIMIT_FSIZE, &rl) != 0)
	adios (MSGSTR(MS_LOG, LOG_8, "getrlimit failed: %s"), strerror(errno));

    if (rl.rlim_max < logging -> l_size) {
	advise (SLOG_NOTICE, MSGSTR(MS_LOG, LOG_7,
	    "current system max limit on the log file (%d) is less than the requested max size (%d)"), 
	    rl.rlim_max, logging -> l_size);
        logging -> l_size = rl.rlim_max;
    }

    /*
     * If the requested size is 0 (zero), it means unlimited file size.
     */
    if (logging -> l_size == 0) 
        rl.rlim_cur = rl.rlim_max;
    else
        rl.rlim_cur = logging -> l_size;
    if (setrlimit(RLIMIT_FSIZE, &rl) != 0)
        adios (MSGSTR(MS_LOG, LOG_9, "setrlimit failed: %s"), strerror(errno));
}


/*
 * NAME: rotate_log()
 *
 * FUNCTION: Moves the log files as follows:
 *  logfile.2 --> logfile.3
 *  logfile.1 --> logfile.2
 *  logfile.0 --> logfile.1
 *  logfile   --> logfile.0
 *
 * The old logfile.3 is lost.
 * The logging process is temporarily stopped while the log files are
 * being rotated.
 */

void
rotate_log() 
{
    char  cmd[512];
    int	  i;

    syslog (LOG_NOTICE, MSGSTR(MS_LOG, LOG_11,
		"%srotating log files"), MSGSTR(MS_LOG, LOG_19, 
		priority[SLOG_NOTICE].name));

    /*
     * Temporarily stop the logging process while files are being rotated.
     * FALSE indicates that no message should be written to the log file
     * stating that the logging process is stopping.
     */
    stop_log (logging -> l_filename, FALSE);

    for (i = 3; i > 0; i--) {
	sprintf(cmd, "mv -f %s.%d %s.%d 2>/dev/null", logging -> l_filename, 
	              i - 1, logging -> l_filename, i);
	system(cmd);
    }
 
    sprintf(cmd, "mv -f %s %s.0", logging -> l_filename, logging -> l_filename);
    system(cmd);

    /*
     * Restart the logging process.  FALSE indicates that no message should be 
     * written to the log file stating that the logging process is starting.
     */
    start_log (FALSE);
}
#endif /* _POWER */
