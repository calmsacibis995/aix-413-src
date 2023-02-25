static char sccsid[] = "@(#)26	1.5  src/tcpip/usr/sbin/lsprtsv/lsprtsv.c, tcpip, tcpip411, GOLD410 3/8/94 18:03:52";
/*
 * COMPONENT NAME: (TCPIP) SMIT hi-level commands
 *
 * FUNCTIONS:  usage
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * lsprtsv.c -  Takes parameters from SMIT interface and calls the various 
 *              low-level commands to dsiplay Print Service related information.
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	"libffdb.h"
#include	"libsmit.h"

#define	LSQUE		"/bin/lsque"
#define	LSALLQDEV	"/bin/lsallqdev"

#include <nl_types.h>
#include "lsprtsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_LSPRTSV,n,s)

char 	progname[128];			/* = argv[0] at run time */

extern 	char 	*optarg;
extern 	int 	optind;

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	char	*cp = (char *)NULL,
		*cp2 = (char *)NULL,
		*qentry = (char *)NULL,
		parm[MAXRECLEN];

    	int 	c = 0, 
		Zflag = 0, 
		cflag = 0, 
		hflag = 0, 
		pflag = 0, 
		qflag = 0; 

	setlocale(LC_ALL, "");
	catd = catopen(MF_LSPRTSV,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && (c = getopt(argc, argv, "chpq:Z")) != EOF) {
		switch (c) {
			case 'c':
				if (pflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			cflag++;
	    			break;

			case 'Z':
	    			Zflag++;
	    			break;

			case 'h':
	    			hflag++;
	    			break;

			case 'q':
	    			qflag++;
				qentry = optarg;
	    			break;

			case 'p':
				if (cflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			pflag++;
	    			break;

			case '?':
	    			usage(0);

			default:
				usage(MSGSTR(UNK_SWITCH,
					"unknown switch: %c."),c);
		}
    	}

    	if (optind != argc)
		usage(MSGSTR(NOT_PARSED,"some parameters were not parsed."));

    	if (!(cflag || pflag))
		usage(MSGSTR(SPEC_SW,"you must specify either -c or -p."));

    	if (cflag) {
	}

    	if (pflag) {
	}

	if (hflag) {
		strcpy(parm, RUSER);
		strcat(parm, TCPFLG_s);
 		strcat(parm, TCPFLG_P);
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, RUSER);
			exit(errno);
		}
	}

	if (qflag) {
		cp = cp2 = qentry;
		while (*cp != '\0') {
			SKIPDELIM(cp2);
			SKIPTOKEN(cp2);
			if (*cp2 != '\0')
				*cp2++ = '\0';
			strcpy(parm, LSQUE);
			strcat(parm, TCPFLG_q);
			strcat(parm, cp);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, LSQUE);
				exit(errno);
			}
			strcpy(parm, LSALLQDEV);
			strcat(parm, TCPFLG_q);
			strcat(parm, cp);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, LSALLQDEV);
				exit(errno);
			}
			cp = cp2;
		}
	}

	exit(0);
}

int usage(a, b, c, d)
	char	*a,
		*b, 
		*c, 
		*d;
{
	if (a)
    		fprintf(stderr, a, b, c, d);
    	fprintf(stderr, 
		MSGSTR(USAGE,
		"\nusage:	%s -c | -p [-h] [-q \"qentry...\"] [-Z]\n"), 
		progname);
    	exit(2);
}
