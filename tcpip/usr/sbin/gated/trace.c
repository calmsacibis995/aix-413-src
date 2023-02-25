static char sccsid[] = "@(#)79	1.6  src/tcpip/usr/sbin/gated/trace.c, tcprouting, tcpip411, GOLD410 12/6/93 18:04:42";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		trace
 *		tracef
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
 *  trace.c,v 1.39 1993/04/10 12:25:50 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#define	INCLUDE_FILE
#define	INCLUDE_STAT

#include "include.h"
#include "krt.h"

flag_t trace_flags = 0;			/* log errors, route changes &/or packets */
flag_t trace_flags_save = 0;		/* save trace flags */
char *trace_file = NULL;		/* File to trace to */
int trace_opened = FALSE;		/* True if tracing is possible */
static FILE *trace_FILE;		/* Initially open to stderr */
static char trace_buffer[BUFSIZ];
static char *trace_ptr = trace_buffer;
static u_long trace_size;		/* Size of current trace file */
off_t trace_limit_size = 0;		/* Maximum desired file size */
int trace_limit_files = 0;		/* Maximum number of files desired */


const bits trace_types[] =
{
    {TR_ALL,	"all"},
    {TR_INT | TR_EXT | TR_RT, "general"},
    {TR_INT,	"internal"},
    {TR_EXT,	"external"},
    {TR_RT,		"route"},
    {TR_PARSE,	"parse"},
#ifdef	TR_ADV
    {TR_ADV,	"adv"},
#endif	/* TR_ADV */
#ifdef	PROTO_EGP
    {TR_EGP,	"egp"},
#endif	/* PROTO_EGP */
    {TR_UPDATE, "update"},
#ifdef	PROTO_RIP
    {TR_RIP,	"rip"},
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
    {TR_HELLO,	"hello"},
#endif	/* PROTO_HELLO */
#ifdef	PROTO_ICMP
    {TR_ICMP,	"icmp"},
#endif	/* PROTO_ICMP */
    {TR_TASK,	"task"},
    {TR_TIMER,	"timer"},
    {TR_NOSTAMP, "nostamp"},
    {TR_MARK,	"mark"},
    {TR_PROTOCOL, "protocol"},
    {TR_KRT,	"kernel"},
#ifdef	PROTO_OSPF
    {TR_OSPF,	"ospf"},
#endif	/* PROTO_OSPF */
#ifdef	PROTO_IGRP
    {TR_IGRP,	"igrp"},
#endif	/* PROTO_IGRP */
#ifdef	PROTO_BGP
    {TR_BGP,	"bgp"},
#endif	/* PROTO_BGP */
#if	defined(PROTO_SNMP)
    {TR_SNMP,	"snmp"},
#endif	/* defined(PROTO_SNMP) */
#ifdef	PROTO_ISIS
    {TR_ISIS,	"isis"},
#endif	/* PROTO_ISIS */
#ifdef	PROTO_IDPR
    {TR_IDPR,	"idpr"},
#endif	/* PROTO_IDPR */
#ifdef	PROTO_IGMP
    {TR_IGMP, "igmp" },
#endif	/* PROTO_IGMP */
    {0, 0}
};


/*
 *	Convert tracing flags to printable
 */
char *
trace_string __PF1(tr_flags, flag_t)
{
    flag_t seen = 0;
    const bits *p;
    char *buf = (char *) task_block_alloc(task_block_pagesize);
    
    if (tr_flags) {
	*buf = (char) 0;
	for (p = trace_types; p->t_bits; p++) {
	    if (seen && ((seen & p->t_bits) == p->t_bits)) {
		/* We have seen these bits before */
		continue;
	    }
	
	    if ((tr_flags & p->t_bits) == p->t_bits) {
		BIT_SET(seen, p->t_bits);
		(void) strcat(buf, " ");
		(void) strcat(buf, p->t_name);
	    }
	}
    } else {
	(void) strcpy(buf, " none");
    }

    return buf;
}

/*
 *	Display trace options enabled
 */
void
trace_display __PF2(tr_flags, flag_t,
		    level, flag_t)
{
    char *buf = trace_string(tr_flags);

    trace(level, 0, NULL);
    trace(level, 0, "Tracing flags enabled: %s", buf);
    task_block_free(task_block_pagesize, (void_t) buf);
    trace(level, 0, NULL);
}


/*
 * Turn off tracing.
 */
void
trace_off __PF0(void)
{
    if (!trace_flags) {
	return;
    }
    if (trace_FILE) {
	trace(TR_ALL, 0, NULL);
	trace(TR_ALL, LOG_INFO, "Tracing to \"%s\" suspended",
	      trace_file ? trace_file : "(stderr)");
	trace(TR_ALL, 0, NULL);
	trace_flags = 0;
	if (trace_FILE != stderr) {
	    (void) fclose(trace_FILE);
	}
	trace_FILE = NULL;
    }
    return;
}


/*
 * Close trace file
 */
void
trace_close __PF0(void)
{
    if (trace_FILE) {
	trace_flags = 0;
	if (trace_FILE != stderr) {
	    (void) fclose(trace_FILE);
	}
	trace_FILE = NULL;
    }
    return;
}


/*
 * Turn on tracing.
 */
void
trace_on __PF2(file, char *,
	       append, int)
{

    if (!file) {
	trace_FILE = stderr;
    } else {
	int fd;
#ifndef	NO_STAT
	struct stat stbuf;
#endif	/* NO_STAT */

#ifndef	FLAT_FS
	if (*file != '/') {
	    /* Make trace file name absolute */
	    char *fn = task_mem_malloc((task *) 0,
				       (size_t) (strlen(file) + strlen(task_path_start) + 2));

	    (void) strcpy(fn, task_path_start);
	    (void) strcat(fn, "/");
	    (void) strcat(fn, file);

	    if (file != trace_file) {
		task_mem_free((task *) 0, file);
	    }
	    file = fn;
	}
#endif	/* FLAT_FS */

	if (trace_file
	    && !strcmp(file, trace_file)
	    && BIT_TEST(task_state, TASKS_RECONFIG)) {
	    /* Don't replace if reconfiguring to the same file */
	    append = TRUE;
	}
	
#ifndef	NO_STAT
	if (stat(file, &stbuf) < 0) {
	    switch (errno) {
	    case ENOENT:
		trace_size = 0;
		break;

	    default:
		syslog(LOG_ERR, "trace_on: stat(%s): %m",
		       file);
		return;
	    }
	} else {
	    /* Indicate current file size.  Will be reset later if not appending */
	    trace_size = stbuf.st_size;

	    switch (stbuf.st_mode & S_IFMT) {
	    default:
		/* This may be a pipe, where we won't be able to seek */
		append = FALSE;
		if (trace_limit_size) {
		    /* Don't want to try to rename a pipe! */

		    trace_limit_size = 0;
		    trace(0, LOG_WARNING, "trace_on: \"%s\" is not a regular file, ignoring size limit",
			  file);
		}
		/* Fall through */

	    case S_IFREG:
		break;

	    case S_IFDIR:
	    case S_IFBLK:
	    case S_IFLNK:
		trace(TR_ALL, LOG_ERR, "trace_on: \"%s\" is not a regular file",
		      file);
		return;
	    }
	}
#endif	/* NO_STAT */

	/* First try to open the file */
	fd = open(file, O_RDWR | O_CREAT | (append ? O_APPEND : O_TRUNC), 0644);
	if (fd < 0) {
	    trace(TR_ALL, LOG_ERR, "Could not open \"%s\": %m",
		  file);
	    return;
	}
	/* Then try to lock the file */
	/* Lock the file to insure only one gated writes to it at a time */
	if (flock(fd, LOCK_EX|LOCK_NB) < 0) {
	    int error = errno;

	    switch (error) {
	    case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	    case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
		trace(TR_ALL, LOG_ERR, "trace_on: trace file \"%s\" appears to be in use",
		      file);
		break;

	    default:
		trace(TR_ALL, LOG_ERR, "trace_on: Could not obtain lock on \"%s\": %m",
		       file);
		break;
	    }
	    (void) close(fd);
	    return;
	}
	if (!append) {
	    /* Indicate the file is empty */
	    trace_size = 0;
	}

	/* Now close the old one */
	if (trace_FILE) {
	    trace_close();
	}

	/* And finally open the stream */
	trace_FILE = fdopen(fd, append ? "a" : "w");
	if (!trace_FILE) {
	    syslog(LOG_ERR, "trace_on: can not open \"%s\" for writing: %m",
		   file);
	    return;
	}
    }
    syslog(LOG_INFO, "tracing to \"%s\" started",
	   file ? file : "(stderr)");
#ifndef vax11c
    setvbuf(trace_FILE, NULL, _IOLBF, 0);
#endif	/* vax11c */

    trace_flags = trace_flags_save;
    trace_display(trace_flags, TR_INT);
    trace_opened = TRUE;

    if (file != trace_file) {
	if (trace_file) {
	    task_mem_free((task *) 0, (void_t) trace_file);
	}
	trace_file = file;
    }
}


/*
 *  Rotate the trace files
 */
static void
trace_rotate __PF1(file, char *)
{
    int i = trace_limit_files - 2;
    char file1[MAXPATHLEN];
    char file2[MAXPATHLEN];
    char *cp = file1;

    syslog(LOG_INFO, "trace_rotate: rotating %s",
	   file);

    while (i >= 0) {
	(void) sprintf(file2, "%s.%d", file, i--);
	if (i < 0) {
	    cp = file;
	} else {
	    (void) sprintf(file1, "%s.%d", file, i);
	}

	if (rename(cp, file2) < 0) {
	    switch (errno) {
	    case ENOENT:
		break;

	    default:
		syslog(LOG_ERR, "trace_rotate: rename(%s, %s): %m",
		       file1,
		       file2);
	    }
	}
    }
}


/*
 *  Parse trace flags specified on the command line
 */
flag_t
trace_args __PF1(flags, char *)
{
    int i = 1;
    flag_t tr_flags = 0;
    char *cp = flags;
    const bits *p;

    while (cp = (char *) index(cp, ',')) {
	    i++;
	    *cp++ = (char) 0;
    }

    for (cp = flags; i--; cp += strlen(cp) + 1) {
	if (!strcasecmp(cp, "none")) {
	    tr_flags = (flag_t) 0;
	} else {
	    for (p = trace_types; p->t_bits; p++) {
		    if (!strcasecmp(cp, p->t_name)) {
		        BIT_SET(tr_flags, p->t_bits);
		        break;
		    }
	    }
		
	    if (!p->t_bits) {
		trace(TR_ALL, LOG_ERR, "%s: unknown trace flag: %s",
		      task_progname,
		      cp);
	    }
	}
    }

    return tr_flags;
}

#define	BITS_N_STRINGS	4
#define	BITS_NEW_INDEX	(bits_index = ((bits_index + 1) % BITS_N_STRINGS))
#define	BITS_STRING	bits_strings[bits_index]
static int bits_index;
static char bits_strings[BITS_N_STRINGS][LINE_MAX];

/*
 *	Return pointer to static string with current trace flags.
 */
char *
trace_bits __PF2(bp, const bits *,
		 mask, flag_t)
{
    char *s = BITS_STRING;
    register char *dp = s ;
    const bits *p;
    flag_t seen = 0;

    *dp = (char) 0;

    for (p = bp; p->t_bits; p++) {
	if (BIT_MATCH(mask, p->t_bits) && !BIT_MATCH(seen, p->t_bits)) {
	    register const char *sp = p->t_name;

	    BIT_SET(seen, p->t_bits);
	    
	    if (dp > s) {
		*dp++ = ' ';
	    }
			
	    while (*sp) {
		*dp++ = *sp++;
	    }
	}
    }

    *dp = (char) 0;

    BITS_NEW_INDEX;

    return s;
}


/*
 *	Return pointer to static string with current trace flags.
 */
char *
trace_bits2 __PF3(bp1, const bits *,
		  bp2, const bits *,
		  mask, flag_t)
{
    char *s = BITS_STRING;
    register char *dp = s;
    const bits *p, *bp = bp1;
    flag_t seen = 0;

    *dp = (char) 0;

    do {
	for (p = bp; p && p->t_bits; p++) {
	    if (BIT_MATCH(mask, p->t_bits) && !BIT_MATCH(seen, p->t_bits)) {
		register const char *sp = p->t_name;

		BIT_SET(seen, p->t_bits);
	    
		if (dp > s) {
		    *dp++ = ' ';
		}
			
		while (*sp) {
		    *dp++ = *sp++;
		}
	    }
	}
    } while (bp != bp2 && (bp = bp2)) ;

    *dp = (char) 0;

    BITS_NEW_INDEX;
	
    return s;
}


const char *
trace_value __PF2(bp, const bits *,
		  value, int)
{
    const bits *p;

    for (p = bp; p->t_name; p++) {
	if (p->t_bits == (u_int) value) {
	    return p->t_name;
	}
    }

    return (char *) 0;
}


#ifndef	trace_state
char *
trace_state __PF2(bp, bits *,
		  mask, flag_t)
{

    return bp[mask].t_name;
}

#endif	/* trace_state */

/*
 *  Dump everything
 */
static void
trace_do_dump __PF1(tp, task *)
{
    int tries = 10;
    FILE *fp = NULL;
    char path_dump[MAXPATHLEN];

    (void) sprintf(path_dump, _PATH_DUMP,
		   task_progname);

    while (fp == NULL && tries--) {
	int fd;
	int can_seek = TRUE;
#ifndef	NO_STAT
	struct stat stbuf;

	if (stat(path_dump, &stbuf) < 0) {
	    switch (errno) {
	    case ENOENT:
		break;

	    default:
		syslog(LOG_ERR, "trace_do_dump: stat(%s): %m",
		       path_dump);
		return;
	    }
	} else {
	    switch (stbuf.st_mode & S_IFMT) {
	    default:
		/* Might be a FIFO (pipe) Indicate we can't seek */
		can_seek = FALSE;
		/* Fall through */

	    case S_IFREG:
		break;

	    case S_IFDIR:
	    case S_IFBLK:
	    case S_IFLNK:
		trace(TR_ALL, LOG_ERR, "trace_do_dump: \"%s\" is not a regular file",
		      path_dump);
		return;
	    }
	}
#endif	/* NO_STAT */

	/* First try to open the file */
	fd = open(path_dump, O_RDWR | O_CREAT | (can_seek ? O_APPEND : 0), 0644);
	if (fd < 0) {
	    trace(TR_ALL, LOG_ERR, "Could not open \"%s\": %m",
		  path_dump);
	    return;
	}
	/* Then try to lock the file */
	/* Lock the file to insure only one gated writes to it at a time */
	if (flock(fd, LOCK_EX|LOCK_NB) < 0) {
	    int error = errno;

	    switch (error) {
	    case EWOULDBLOCK:
#if	defined(EAGAIN) && EAGAIN != EWOULDBLOCK
	    case EAGAIN:		/* System V style */
#endif	/* EAGAIN */
#ifdef	NO_FORK
		trace(TR_ALL, LOG_ERR, "trace_do_dump: dump file \"%s\" in use",
		      path_dump);
		break;
#else	/* NO_FORK */
		trace(TR_ALL, LOG_ERR, "trace_do_dump: dump file \"%s\" in use, waiting...",
		      path_dump);
		(void) close(fd);
		sleep(15);
		continue;
#endif	/* NO_FORK */

	    default:
		trace(TR_ALL, LOG_ERR, "trace_do_dump: Could not obtain lock on \"%s\": %m",
		       path_dump);
		break;
	    }
	    (void) close(fd);
	    return;
	}

	/* And finally open the stream */
	fp = fdopen(fd, can_seek ? "a" : "w");
	if (!fp) {
	    syslog(LOG_ERR, "trace_do_dump: can not open \"%s\" for writing: %m",
		   path_dump);
	    return;
	}
    }
    
#ifndef vax11c
    setvbuf(fp, NULL, _IOLBF, 0);
#endif	/* vax11c */
    trace(TR_ALL, LOG_INFO, "trace_dump: processing dump to %s",
	  path_dump);
    (void) fprintf(fp, "\f\n\t%s[%d] version %s memory dump on %s at %s\n",
		   task_progname,
		   task_mpid,
		   gated_version,
		   task_hostname,
		   time_full);

    if (krt_version_kernel) {
	(void) fprintf(fp, "\t\t%s\n\n", krt_version_kernel);
    }

    (void) fprintf(fp, "\t\tStarted at %s\n",
		   timer_time_start.gt_ctime);

    /* Task_dump dumps all protocols */
    task_dump(fp);

    (void) fprintf(fp, "\nDone\n");
    
    (void) fflush(fp);
    (void) fclose(fp);

    timer_peek();
    
    trace(TR_ALL, LOG_INFO, "trace_dump: dump completed to %s",
	  path_dump);
}


#ifndef	NO_FORK
static task *trace_dump_task;		/* Pointer to the dump task */

static void
trace_dump_done __PF1(tp, task *)
{
    trace_dump_task = (task *) 0;
    task_delete(tp);
}

#endif	/* NO_FORK */


void
trace_dump __PF1(now, int)
{

#ifndef	NO_FORK
    if (trace_dump_task) {
	trace(TR_ALL, LOG_ERR, "trace_dump: %s already active",
	      task_name(trace_dump_task));
    } else if (now) {
	trace_do_dump((task *) 0);
    } else {
	trace_dump_task = task_alloc("TraceDump", TASKPRI_PROTO);
	trace_dump_task->task_child = trace_dump_done;
	trace_dump_task->task_process = trace_do_dump;
	if (!task_fork(trace_dump_task)) {
	    task_quit(EINVAL);
	}
    }
#else	/* NO_FORK */
    trace_do_dump((task *) 0);
#endif	/* NO_FORK */
}


/*ARGSUSED*/
void
trace_mark __PF2(tip, timer *,
		 interval, time_t)
{
    trace(TR_MARK | TR_NOSTAMP, 0, "%s MARK", time_string);
}


void
trace_init __PF0(void)
{
    trace_FILE = stderr;
}


/*
 *	Trace to the log and syslog
 */
#ifdef	STDARG
/*VARARGS2*/
void
trace(flag_t flags, int pri, const char *fmt,...)
#else	/* STDARG */
/*ARGSUSED*/
/*VARARGS0*/
void
trace(va_alist)
va_dcl
#endif	/* STDARG */
{
    int do_trace, do_syslog;
    char time_buffer[LINE_MAX];
    va_list args;
#ifdef	STDARG

    va_start(args, fmt);
#else	/* STDARG */
    flag_t flags;
    int pri;
    byte *fmt;

    va_start(args);

    flags = va_arg(args, flag_t);
    pri = va_arg(args, int);
    fmt = va_arg(args, byte *);

#endif	/* STDARG */
    do_trace = trace_FILE
	&& (flags == TR_ALL
	    || ((flags & trace_flags) & ~TR_NOSTAMP));
    do_syslog = pri && !BIT_TEST(task_state, TASKS_TEST);

    if (do_trace || do_syslog) {
	if (fmt && *fmt) {
	    trace_ptr += vsprintf(trace_ptr, fmt, &args);
	}

	if (do_trace) {
	    register char *dp = time_buffer;
	    register char *sp;
	    int size;

	    if (!((flags | trace_flags) & TR_NOSTAMP)) {
		sp = time_string;
		while (*sp) {
		    *dp++ = *sp++;
		}
		*dp++ = ' ';
#ifdef	notdef
		/* XXX - begin testing */
		if (task_active) {
		    sp = task_name(task_active);
		    while (*sp) {
			*dp++ = *sp++;
		    }
		    *dp++ = ' ';
		}
		/* XXX - end testing */
#endif	/* notdef */
	    }

	    if (task_mpid != task_pid) {
		*dp++ = '[';
		sp = task_pid_str;
		while (*sp) {
		    *dp++ = *sp++;
		}
		*dp++ = ']';
		*dp++ = ' ';
	    }
	    *dp = (char) 0;

	    size = (dp - time_buffer) + (trace_ptr - trace_buffer) + 1;
	    if (trace_FILE != stderr &&
		trace_limit_size &&
		(trace_size + size) > trace_limit_size) {
		/* Time to rotate files */

 		/* Inform our audience */
 		if (*time_buffer) {
 		    (void) fputs(time_buffer, trace_FILE);
 		}
 		(void) fputs("Rotating trace files\n", trace_FILE);
 
		/* Close this one */
		trace_close();

		/* Rotate them around */
		trace_rotate(trace_file);

		/* And open new file */
		trace_on(trace_file, FALSE);
	    }

	    trace_size += size;

	    if (*time_buffer) {
		(void) fputs(time_buffer, trace_FILE);
	    }
	    (void) fputs((char *) trace_buffer, trace_FILE);
	    (void) fputc('\n', trace_FILE);
#ifdef	MUST_FFLUSH
	    (void) fflush(trace_FILE);
#endif
	}
	if (do_syslog) {
	    syslog(pri, (char *) trace_buffer);
	}
    }
	
    *(trace_ptr = trace_buffer) = (char) 0;
	
    va_end(args);
	
}


/*
 *	Prefill the trace buffer
 */
#ifdef	STDARG
/*VARARGS2*/
void
tracef(const char *fmt,...)
#else	/* STDARG */
/*ARGSUSED*/
/*VARARGS0*/
void
tracef(va_alist)
va_dcl
#endif	/* STDARG */
{
    va_list args;
#ifdef	STDARG
    va_start(args, fmt);
#else	/* STDARG */
    byte *fmt;

    va_start(args);

    fmt = va_arg(args, byte *);
#endif	/* STDARG */

    if (fmt && *fmt) {
	trace_ptr += vsprintf(trace_ptr, fmt, &args);
    }
    va_end(args);
}


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
