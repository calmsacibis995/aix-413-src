static char sccsid[] = "@(#)59	1.10  src/tcpip/usr/sbin/gated/main.c, tcprouting, tcpip411, GOLD410 3/7/94 20:58:08";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: MSGSTR
 *		main
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
 *  main.c,v 1.27 1993/04/10 12:25:14 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "include.h"
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
#include "iso.h"
#endif	/* PROTO_ISO */
#include "krt.h"
#include "parse.h"

#ifdef _POWER
#include "gated_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAIN,n,s)

#include <locale.h>
/*
 * The following includes and declrations are for support of the System
 * Resource Controller (SRC) - .
 */
#include <spc.h>

#include "libffdb.h"
#include "libsrc.h"

char    progname[MAXPATHLEN];
int     src_exists = TRUE;
#else
#define MSGSTR(n,s) s
#endif /* _POWER */

int
main(argc, argv)
int argc;
char *argv[];
{
#ifdef _POWER
    /*
     * SRC local variables -
     */
    int rc, i, addrsz;
    struct sockaddr srcaddr;

    setlocale(LC_ALL,"");               /* set locale */
    catd = catopen(MF_GATED,NL_CAT_LOCALE);         /* open catalog file */

    /*
     * SRC startup.  Save the program name for SRC related error messages. Make
     * sure fd 0 (stdin) is the SRC socket descriptor.  If not, continue,
     * otherwise dup the fd to SRC_FD so that stdin can be used by
     * gated internally - .
     */
    strcpy(progname,argv[0]);
    addrsz = sizeof(srcaddr);
    if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
        src_exists = FALSE;
    }
    if (src_exists == TRUE)
        (void) dup2(0, SRC_FD);
#endif /* _POWER */

    tzsetwall();		/* Init time with the local time zone */
    
    timer_peek();		/* start time */

    srandom((int) time_sec);	/* Prime the random number generator */

    trace_init();		/* Init for tracing */

    task_init1(argv[0]);

    if (src_exists == TRUE)  	/* If under SRC control don't become a daemon */
	task_newstate(TASKS_NODAEMON, 0);

    sock_init();		/* Initialize socket code */

    trace_flags = trace_flags_save = 0;
    if (parse_args(argc, argv)) {
	task_quit(0);
    }

    /* open initialization file */
    if (!task_config_file) {
	char path_config[MAXPATHLEN];

	(void) sprintf(path_config, _PATH_CONFIG, task_progname);
	
	task_config_file = task_mem_strdup((task *) 0, path_config);
#ifndef	FLAT_FS
    } else  if (*task_config_file != '/') {
	/* Make config file name absolute */
	char *file = task_mem_malloc((task *) 0,
				     (size_t) (strlen(task_config_file) + strlen(task_path_start) + 2));

	(void) strcpy(file, task_path_start);
	(void) strcat(file, "/");
	(void) strcat(file, task_config_file);

	task_mem_free((task *) 0, task_config_file);
	task_config_file = file;
#endif	/* FLAT_FS */
    }

    task_init2();

#if	defined(LOG_DAEMON)
    openlog(task_progname, LOG_OPTIONS, LOG_FACILITY);
    (void) setlogmask(LOG_UPTO(LOG_INFO));
#else	/* defined(LOG_DAEMON) */
    openlog(task_progname, LOG_PID);
#endif	/* defined(LOG_DAEMON) */

    if (trace_flags_save) {
	(void) trace_on(trace_file, TRUE);
    }

    if (!BIT_MATCH(task_state, TASKS_TEST|TASKS_NODUMP)) {
	trace(TR_ALL, 0, NULL);
	trace(TR_ALL, LOG_INFO, MSGSTR(MAIN_1,
	      "Start %s[%d] version %s built %s"),
	      task_progname,
	      task_pid,
	      gated_version,
	      build_date);
	trace(TR_ALL, 0, NULL);
#ifdef _POWER
	if (src_exists == FALSE)
	      trace(TR_ALL, LOG_NOTICE, MSGSTR(MAIN_2, "SRC not found, %s continuing without SRC support\n"),progname);
#endif /* _POWER */
    }

    rt_init();				/* initialize routing tables */

#ifdef	PROTO_ASPATHS
    aspath_init();			/* Initialize AS paths */
#endif	/* PROTO_ASPATHS */

#ifdef	PROTO_INET
    inet_family_init();			/* Initialize INET constants */
#endif	/* PROTO_INET */

#ifdef	PROTO_ISO
    iso_family_init();			/* Initialize ISO constants */
#endif	/* FAMILY_ISO */

    if_family_init();			/* initialize interface tables */

    /* The only problem with having the kernel init code before the parse */
    /* is that we can not catch martian nets installed in the kernel.  They */
    /* will time out after a fashion */
    krt_init();				/* Read kernel routing table */

#ifdef	PROTO_INET
    if (!BIT_TEST(task_state, TASKS_TEST)) {
	if (inet_ipforwarding != 1) {
	    trace(TR_ALL, LOG_WARNING, MSGSTR(MAIN_3, "main: *WARNING* IP forwarding disabled!"));
	}

	if (inet_udpcksum != 1) {
	    trace(TR_ALL, LOG_WARNING, MSGSTR(MAIN_4, "main: *WARNING* UDP checksums disabled in kernel"));
	}
    }
#endif	/* PROTO_INET */
    
    trace(TR_TASK, 0, NULL);

    /* Reset all protocol configurations */
    task_proto_var_inits();
    
    /* Read the config file */
    switch (parse_parse(task_config_file)) {
    case -1:
	/* Could not open config file */
#ifdef	PROTO_INET
	if (if_n_link.up == 1) {
	    rt_entry *rt = rt_locate(RTS_NETROUTE,
				     inet_addr_default,
				     inet_mask_default,
				     RTPROTO_BIT(RTPROTO_KERNEL));

	    if (!rt || BIT_TEST(rt->rt_state, RTS_DELETE)) {
		break;
	    }
	    trace(TR_ALL, 0, NULL);
	    trace(TR_ALL, LOG_NOTICE, MSGSTR(MAIN_5, "No config file, one interface and a default route, gated exiting"));
	    trace(TR_ALL, 0, NULL);
	    task_quit(0);
	}
#endif	/* PROTO_INET */
	task_quit(ENOENT);
	break;

    case 0:
	/* Successful */
	if (BIT_MATCH(task_state, TASKS_TEST|TASKS_NODUMP)) {
	    /* Just syntax check */
	    exit(0);
	}
	break;

    default:
	/* Errors in config file */
	if (BIT_MATCH(task_state, TASKS_TEST|TASKS_NODUMP)) {
	    /* Just syntax check */
	    exit(1);
	} else {
	    task_quit(0);
	}
    }

#ifndef vax11c
    if (!BIT_TEST(task_state, TASKS_TEST)) {
	task_pid_open();
    }
#endif	/* vax11c */

    timer_peek();
    
    task_proto_inits();

    (void) timer_create((task *) 0,
			0,
			"Time.Mark",
			0,
			(time_t) TIME_MARK,
			(time_t) 0,
			trace_mark,
			(void_t) 0);

    trace(TR_TASK, 0, NULL);

    trace(TR_ALL, 0, NULL);
    trace(TR_ALL, 0, "***Routes are %sbeing installed in kernel",
	  krt_install ? "" : "not ");
    trace(TR_ALL, 0, NULL);

    task_main();

    return 0;				/* To keep the complier happy */
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
