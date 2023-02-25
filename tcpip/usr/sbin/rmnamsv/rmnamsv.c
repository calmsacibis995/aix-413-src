static char sccsid[] = "@(#)20	1.5  src/tcpip/usr/sbin/rmnamsv/rmnamsv.c, tcpnaming, tcpip411, GOLD410 3/8/94 18:28:05";
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
 * rmnamsv.c -  Takes parameters from SMIT interface and calls the NAMERSLV
 *              low-level command to modify Name Service related information.
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	"libffdb.h"
#include	"libsmit.h"

#include <nl_types.h>
#include "rmnamsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_RMNAMSV,n,s)

char 	progname[128];			/* = argv[0] at run time */

extern 	char 	*optarg;
extern 	int 	optind;

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	char	*filename = (char *)NULL,
		parm[MAXRECLEN];

    	int 	c = 0, 
		Fflag = 0, 
		fflag = 0; 

	setlocale(LC_ALL, "");
	catd = catopen(MF_RMNAMSV,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && (c = getopt(argc, argv, "fF:")) != EOF) {
		switch (c) {
			case 'f':
				if (Fflag)
					usage(MSGSTR(INCOMP_SW,
					"imcompatible command line switches."));
	    			fflag++;
	    			break;

			case 'F':
				if (fflag)
					usage(MSGSTR(INCOMP_SW,
					"imcompatible command line switches."));
	    			Fflag++;
				filename = optarg;
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

	strcpy(parm, NAMERSLV);
	if (Fflag) {
		strcat(parm, TCPFLG_E);
		strcat(parm, filename);
	} else if (fflag)
		strcat(parm, TCPFLG_e);
	else
		strcat(parm, TCPFLG_X);

	if (system(parm) == -1) {
		fprintf(stderr, 
			MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
			progname, NAMERSLV);
		exit(errno);
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
		MSGSTR(USAGE,"\nusage:	%s -f | -F filename\n"), 
		progname);
    	exit(2);
}
