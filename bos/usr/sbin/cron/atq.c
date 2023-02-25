static char sccsid[] = "@(#)57	1.16.1.2  src/bos/usr/sbin/cron/atq.c, cmdcntl, bos41B, 9504A 1/4/95 10:13:17";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS: atq
 *
 * ORIGINS: 26, 27
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <pwd.h>
#include "cron.h"

/*
 * NAME: atq
 *                                                                    
 * FUNCTION: command line processing to call the correct atq functions
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * 	This function will compute the command line arguments and
 *	call the correct "at" function based on the user flags.
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *	    what bits / data structures, etc it manipulates. 
 *
 */

#include "cron_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
#define ROOT            0       /* user-id of super-user */
#define	INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)"
nl_catd catd;

main(argc,argv)
int argc;
char *argv[];
{
	register int i;			/* loop variable 	*/
	struct passwd   *nptr;
	uid_t	user;			/* current user id	*/
	char	*pp, login[UNAMESIZE];	/* current user name	*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_CRON,NL_CAT_LOCALE);

	/* get the current user id/name	*/
	if ((nptr=getpwuid(user=getuid())) == NULL) {
		fprintf(stderr, "atq: %s\n",
			MSGSTR(MS_INVALIDUSER, INVALIDUSER));
		exit(1);
	} else {
		pp = nptr->pw_name;
	}
	strcpy(login, pp);

	if (argc == 1) {			/* atq */
		/* list in execution order */
		if (user == ROOT)
			list_aj(CRON_SORT_M | CRON_USER, NULL);
		else
			list_aj(CRON_SORT_M | CRON_USER, login);
		exit(0);
	}

	/* 
 	 * If flags are specified but no user names are
	 * listed, then display at jobs for the current users
	 * If a privileged user invokes, atq displays the list for
	 * all users.
	 */

	if (argv[1][0] == '-') {
		switch (argv[1][1]) {
		   case 'c':		/* list in order of submission */
			if (argc == 2) {
			    if (user == ROOT)
				list_aj(CRON_SORT_M | CRON_USER, NULL);
			    else
				list_aj(CRON_SORT_E | CRON_USER, login);
			} else {
			    for (i=2; i<argc; i++) {
				if ( (user == ROOT) ||
					!(strcmp(login, argv[i])))
				    list_aj(CRON_SORT_E | CRON_USER, argv[i]);
			    }
			}
			exit(0);
		   case 'n':		/* count total number of at jobs */
			if (argc == 2) {
			    if (user == ROOT)
				list_aj(CRON_COUNT_JOBS | CRON_USER, NULL);
			    else
				list_aj(CRON_COUNT_JOBS | CRON_USER, login);
			} else {
			    for (i=2; i<argc; i++) {
				if ( (user == ROOT) ||
					!(strcmp(login, argv[i])))
			            list_aj(CRON_COUNT_JOBS | CRON_USER, 
						argv[i]);
			    }
			}
			exit(0);
		   case '?':
		   case 'h':
		   default:
			usage();
		}
	}
	else {
		for (i=1; i<argc; i++) {	/* list in execution order */
			if ( (user == ROOT) ||
				!(strcmp(login, argv[i])))
			    list_aj(CRON_SORT_M | CRON_USER, argv[i]);
		}
		exit(0);
	}

}

static usage()
{
	fprintf(stderr, MSGSTR(MS_ATQUSAGE,
		"Usage: atq [-c|-n] [username...]\n"));
	exit(1);
}
