static char sccsid[] = "@(#)96	1.10.1.6  src/bos/usr/sbin/renice/renice.c, cmdcntl, bos41B, 9504A 1/4/95 10:13:46";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS:
 *
 * ORIGINS: 26, 27, 71
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * OSF/1 1.1
 */

#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <nl_types.h>
#include "renice_msg.h"
#define  MSGSTR(num,str) catgets(catd,MS_RENICE,num,str)  /*MSG*/
static nl_catd  catd;
#include <locale.h>
static char	*progname;
static int	ob_flag = 0;

/*
 * Change the priority (nice) of processes
 * or groups of processes which are already
 * running.
 */
main(argc, argv)
	char **argv;
{
	int which = -1;
	int who = 0;
	int prio = 0;
	int errs = 0;
	int c;
	char *end_ptr;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_RENICE, NL_CAT_LOCALE);
	progname = argv[0];
	if (argc < 2) 
		usage();
	if (ob_check(argc, argv)) {
		ob_parse(argc, argv);
		/* Does not return */
	}
	while ((c = getopt(argc, argv, "n:gpu")) != EOF) {
	    switch(c) {
		case 'n': 
			prio = strtol(optarg, &end_ptr, 10); /* get the increment */
			if (*end_ptr != '\0') 
				usage();
			break; 
		case 'g':
			if (which == -1) 
   				 which = PRIO_PGRP;
			else
    				usage(); 
			break;
		case 'p':
			if (which == -1) 
    				which = PRIO_PROCESS;
			else
    				usage();
			break;
		case 'u': 
			if (which == -1)
    				which = PRIO_USER;
			else
    				usage();
			break;
		default:
			usage();
		}
            } 
	if (which == -1)
    		which = PRIO_PROCESS;
	if (optind == argc) {
   		fprintf(stderr, MSGSTR(MISSING_ID, "%s: missing IDs...\n"), progname);
		usage();
	}
	for (; optind < argc; optind++) {
		if (get_id(which, &who, argv[optind]))
			errs++;
		else
			errs += donice(which, who, prio);
	} 
	exit(errs != 0);
}
/*
 * NAME: ob_check
 *
 * FUNCTION: To check the command line arguments for obsolescent syntax.
 *           Returns 1 if obsolescent syntax used, 0 otherwise.
 */
static ob_check(argc, argv)
	int argc;
	char **argv;
{
	int i;

	/*
	 * Negative value is probably a 'nice_value'.
	 * Will find:  renice -<nice_value> <ID>
	 */
	if ((argv[1][0] == '-') && isdigit(argv[1][1]))
		return 1;

	/*
	 * -p, -g, or -u come anywhere after an initial numerical value.
	 * Will find: renice <nice_value> [-p|-g|-u] ...
	 *            renice <nice_value> ... [-p|-g|-u] ...
	 */
	if (isdigit(argv[1][0])) {
		for(i=2; i<argc; i++) {
			if ((argv[i][0] == '-') &&
			    (strchr("pgu", argv[i][1]) != NULL))
				return 1;
		}
	}

	/*
	 * Note: Wont find: renice <nice_value> <ID> ...
	 * This is an ambiguous case, so the non-obsolete syntax is used.
	 */
	return 0;
}
/*
 * NAME: ob_parse
 *
 * FUNCTION: To parse the command line arguments using the obsolescent
 *           syntax.  Exits with 0 if no errors occured, >0 otherwise.
 */
static ob_parse(argc, argv)
	int argc;
	char **argv;
{
	int which = PRIO_PROCESS;
	int who, prio, i;
	int errs = 0;
	char *end_ptr, *arg;

	ob_flag = 1;
	prio = strtol(argv[1], &end_ptr, 10); /* get the increment */
	if (*end_ptr != '\0') 
		usage();

	for (i=2; i<argc; i++) {
		if ((argv[i][0] == '-') &&
		    (strchr("pgu", argv[i][1]) != NULL)) {
			switch(argv[i][1]) {
				case 'p':
					which = PRIO_PROCESS;
					break;
				case 'g':
					which = PRIO_PGRP;
					break;
				case 'u':
					which = PRIO_USER;
					break;
			}
			if (argv[i][2] == '\0') {
				/* Flags take at least 1 arg. */
				if (argc == i+1) {
					fprintf(stderr, MSGSTR(MISSING_ID,
					    "%s: missing IDs...\n"), progname);
					usage();
				}
				/* Go to next argument */
				continue;
			}
			arg = argv[i]+2;
		} else
			arg = argv[i];

		if (get_id(which, &who, arg))
			errs++;
		else
			errs += donice(which, who, prio);
	}
	exit(errs != 0);
}
/*
 * NAME: get_id
 * 
 * FUNCTION: To check and get user ID if flag which indicates user. Returns 1
 *           and prints out error message if user unknown. Otherwise, it will
 * 	     check process or process group ID.  Returns 1 and prints out 
 *	     message if process or process group ID parameter is bad. Returns
 *    	     0 if successfully.
 */
static get_id(which, who, oper_str)
	int which, *who;
	char *oper_str;
{
	char *end_ptr;
  	if (which == PRIO_USER) {
       		register struct passwd *pwd = getpwnam(oper_str);
                if (pwd == NULL) {
                       	*who = strtol(oper_str, &end_ptr, 10);
                       	if (*who < 0 || (*end_ptr != '\0')) {
               	        	fprintf(stderr, MSGSTR(UNK_USER,
                                                "%s: User %s is unknown.\n"),
                                                progname, oper_str);
				return(1);
				}
               } else
                       *who = pwd->pw_uid;
          } else {
               *who = strtol(oper_str, &end_ptr, 10);
               if (*who < 0 || (*end_ptr) != '\0') {
	       fprintf(stderr, MSGSTR(BADVALUE,
						"%s: Parameter %s is bad.\n"),
                                        	progname,oper_str);
		return(1);
               }
	}
	return(0);
}

/*
 * NAME: donice
 *
 * FUNCTION: To invoke getpriority() to get old priority of current process.  
 *	     It calculates the new priority. Then it will call setpriority() 
 *	     to set new priority and print out both old and new priorities. 
 *	     Returns 1 if it encounters any errors while getting or setting  
 *           priorities. Returns 0 if successfully
 *
 */
static donice(which, who, prio )
	int which, who, prio;
{
	int oldprio;
	extern int errno;

	if (!ob_flag) {
		errno = 0, oldprio = getpriority(which, who);
		if (oldprio == -1 && errno) {
			fprintf(stderr, MSGSTR(GETPRIO,"%s: %d: "), progname,who);
			perror("getpriority");
			return (1);
		}
		prio += oldprio;
	}
	if (prio > PRIO_MAX)
		prio = PRIO_MAX;
	if (prio < PRIO_MIN)
		prio = PRIO_MIN;
	if (setpriority(which, who, prio) < 0) {
		fprintf(stderr, MSGSTR(SETPRIO,"%s: %d: "), progname ,who);
		perror("setpriority");
		return (1);
	}
	return (0);
}

static usage()
{
	fprintf(stderr,MSGSTR(USAGE2,
	"usage: %s [-n increment] [ -g| -p| -u] ID...\n") ,progname);
	exit(1);
}

