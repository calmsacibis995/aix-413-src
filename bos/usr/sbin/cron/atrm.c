static char sccsid[] = "@(#)63	1.12.1.3  src/bos/usr/sbin/cron/atrm.c, cmdcntl, bos41J, 9521B_all 5/26/95 10:58:50";
/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
 *
 * FUNCTIONS: atrm
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
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
#include <dirent.h>
#include "cron.h"

/*
 * NAME: atrm
 *                                                                    
 * FUNCTION: command line processing to call the correct atrm functions
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 * 	This function will compute the command line arguments and
 *	call the correct "at" function based on the user flags.
 *	The correct at command flag arguments are added and then
 *	at is called.
 *                                                                   
 * (NOTES:) More detailed description of the function, down to
 *	    what bits / data structures, etc it manipulates. 
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: nothing
 */  

#include "cron_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_CRON,Num,Str)
nl_catd catd;

extern char *strcpy();
extern char *strcat();
extern char *strchr();
extern char *setlocale(int category, const char *locale);
extern struct passwd *getpwuid(uid_t uid);

#define	ATERR_SIGNAL	0x000000ff
#define ATRMDEL "atrm: only jobs belonging to user: %s may be deleted\n"
#define isajob(name)  (getpwnam(name) == NULL)

main(argc,argv)
int argc;
char *argv[];
{
	char	argbuf[BUFSIZ];
	int	allflag = 0;
	int	iflag = 0;
	int	fflag = 0;
	uid_t	user;
	struct passwd *pwinfo;	/* struct for pwd name info 		*/
	int	rec;		/* return value from system("at -r")	*/

	(void ) setlocale(LC_ALL,"");
	catd = catopen(MF_CRON,NL_CAT_LOCALE);

	/* 
	 * If job number, user name, or "-" is not specified, just print
	 * usage info and exit.
	 */
	if (argc < 2)
		usage ();
	--argc; ++argv;

	while (argc > 0 && **argv == '-') {
		if (*(++(*argv)) == '\0') {
			++allflag;
		} else while (**argv)
			 switch (*(*argv)++) {
				case 'f':	++fflag;
						break;
				case 'i':	++iflag;
						break;
				default:
						usage ();
			}
		++argv; --argc;
	}
	/*
	 * If all jobs are to be removed and extra command line arguments
	 * are given, print usage info and exit.
	 */
	if (allflag && argc)
		usage();

	/*
	 * If only certain jobs are to be removed and no job #'s or user
	 * names are specified, print usage info and exit.
	 */
	if (!allflag && !argc)
		usage();

	/*
	 * If interavtive removal and quiet removal are requexted, override
	 * quiet removal and run interactively.
	 */
	if (iflag && fflag)
		fflag = 0;

	/*
	 * Move to spooling area and get user name of person requesting 
 	 * removal.
	 */
	user = getuid();
	pwinfo = getpwuid(user);

	if (!user) { /* super user */
		if (allflag) {
			if (iflag)
				remove_aj(CRON_PROMPT, NULL);
			else if (fflag)
				remove_aj(CRON_QUIET, NULL);
			else remove_aj(NULL, NULL);
		}
		else {
			strcpy (argbuf, "/usr/bin/at -r ");
			if (iflag)
				strcat(argbuf, "-i ");
			else if (fflag)
				strcat(argbuf, "-f ");
			for (; argc--; argv++) {
				if (isajob(*argv)) {
					strcat(argbuf, *argv);
					strcat(argbuf, " ");
				} else {	/* users name */
					strcat(argbuf,"-u ");
					strcat(argbuf, *argv);
					strcat(argbuf, " ");
				}
			}
			rec = system(argbuf);
			if ( !(rec & ATERR_SIGNAL) )
				/* at returned by calling exit()	*/
				rec >>= 8;
			exit(rec);
		}
	}
	else {	/* normal user */
		strcpy (argbuf, "/usr/bin/at -r ");
		if (iflag)
			strcat(argbuf, "-i ");
		else if (fflag)
			strcat(argbuf, "-f ");
		if (allflag) {	/* "-" remove only user's jobs */
			strcat(argbuf,"-u ");
			strcat(argbuf, pwinfo->pw_name);
		}
		for (; argc--; argv++) {
			if (isajob(*argv)) {
				strcat(argbuf, *argv);
				strcat(argbuf, " ");
			} else {	/* users  name */
				strcat(argbuf,"-u ");
				strcat(argbuf, *argv);
				strcat(argbuf, " ");
			}
		}
		rec = system(argbuf);	/* call "at -r"	*/
		if ( !(rec & ATERR_SIGNAL) )
			/* at returned by calling exit()	*/
			rec >>= 8;
		exit(rec);
	}
}

static usage()
{
	fprintf(stderr, MSGSTR(MS_ATRMUSAGE,
	"Usage: atrm [-f|-i] [-] [job #|username]\n"));
	exit(1);
}

