static char sccsid[] = "@(#)16	1.6  src/tcpip/usr/sbin/lsnamsv/lsnamsv.c, tcpnaming, tcpip411, GOLD410 3/8/94 17:48:17";
/*
 * COMPONENT NAME: (TCPIP) SMIT hi-level commands
 *
 * FUNCTIONS: usage
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
 * lsnamsv.c -  Takes parameters from SMIT interface and calls the NAMERSLV
 *              low-level command to display Name Service related information.
 *
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	"libffdb.h"
#include	"libsmit.h"

char 	progname[128];			/* = argv[0] at run time */

extern 	char 	*optarg;
extern 	int 	optind;

#include <nl_types.h>
#include "lsnamsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_LSNAMSV,n,s)

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	char	*cp = (char *)NULL,
		*attr_list = (char *)NULL,
    		parm[MAXRECLEN]; 

    	int 	c, 
		Cflag = 0, 
		Sflag = 0, 
		Zflag = 0; 

	setlocale(LC_ALL, "");
	catd = catopen(MF_LSNAMSV,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && (c = getopt(argc, argv, "CS:Z")) != EOF) {
		switch (c) {
			case 'S':
				if (Cflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			Sflag++;
				attr_list = optarg;
	    			break;

			case 'C':
				if (Sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			Cflag++;
	    			break;

			case 'Z':
	    			Zflag++;
	    			break;

			case '?':
	    			usage(0);
	    			break;

			default:
				usage(MSGSTR(UNK_SWITCH,
					"unknown switch: %c"),c);
	    			break;
		}
    	}

    	if (optind != argc)
		usage(MSGSTR(NOT_PARSED,"some parameters were not parsed."));

    	if (!(Cflag || Sflag))
		usage(MSGSTR(SPEC_SW,"you must specify either -C or -S."));

	if (Sflag) {
		cp = attr_list;
		while (*cp != '\0') {
			strcpy(parm, NAMERSLV);
			strcat(parm, TCPFLG_s);
			if (Zflag)
				strcat(parm, TCPFLG_Z);
			SKIPDELIM(cp);
			switch (smit_compare_attr(namerslv_attr, cp, FALSE)) {
				case DOMAIN:
					strcat(parm, TCPFLG_n);
					cp += strlen(namerslv_attr[DOMAIN])-1;
					SKIPTOKEN(cp);
					break;
				case NAMESERVER:
					strcat(parm, TCPFLG_I);
				      cp += strlen(namerslv_attr[NAMESERVER])-1;
					SKIPTOKEN(cp);
					break;
				case NO_ATTR:
					usage(MSGSTR(BAD_ATTR,
						"invalid attribute: %s."),cp);
					break;
			}
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, NAMERSLV);
				exit(errno);
			}
		}    
	}

	if (Cflag) {
		strcpy(parm, NAMERSLV);
		strcat(parm, TCPFLG_s);
		if (Zflag)
			strcat(parm, TCPFLG_Z);
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, NAMERSLV);
			exit(errno);
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
		MSGSTR(USAGE,"\nusage:	%s -C | -S \"attr_val...\" [-Z]\n"), 
		progname);
    	exit(2);
}
