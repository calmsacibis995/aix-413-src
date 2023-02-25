static char sccsid[] = "@(#)14	1.6  src/tcpip/usr/sbin/chnamsv/chnamsv.c, tcpnaming, tcpip411, GOLD410 3/7/94 20:59:08";
/*
 * COMPONENT NAME: (TCPIP) SMIT hi-level commands
 *
 * FUNCTIONS: setup_namerslv_parms, usage
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
 * chnamsv - Takes parameters from SMIT interface and calls various low
 *           level commands that modify Nameserver service configurations.
 */           

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include	"libffdb.h"
#include	"libsmit.h"

char 	progname[128];			/* = argv[0] at run time */

extern 	char 	*optarg;
extern 	int 	optind;

#include <nl_types.h>
#include "chnamsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_CHNAMSV,n,s)

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	char	*filename = (char *)NULL,
		*cp = (char *)NULL,
		*attr_list = (char *)NULL,
		recptr[MAXRECLEN],
		parm[MAXRECLEN],
		*setup_namerslv_parms();

    	int 	c = 0, 
		Aflag = 0, 
		aflag = 0; 

	FILE 	*fp = (FILE *)NULL,
		*ff_open();

	setlocale(LC_ALL, "");
	catd = catopen(MF_CHNAMSV,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && (c = getopt(argc, argv, "a:A:")) != EOF) {
		switch (c) {
			case 'a':
				if (Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			aflag++;
				attr_list = optarg;
	    			break;

			case 'A':
				if (aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Aflag++;
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

	if (aflag) {
		cp = attr_list;
		while (*cp != '\0') {
			cp = setup_namerslv_parms(parm, cp);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, NAMERSLV);
				exit(errno);
			}
		}
	} else if (Aflag) {
		fp = ff_open(filename, "r");
		while (fgets(recptr, MAXRECLEN, fp) > 0) {
			if ((cp = strchr(recptr, '\n')) != NULL)
				*cp = '\0';
			cp = recptr;
			cp = setup_namerslv_parms(parm, cp);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, NAMERSLV);
				exit(errno);
			}
		}
		ff_close(filename, fp);
	} else
		usage(MSGSTR(SPEC_SW2,"you must specify either -a or -A."));

	exit(0);
}

char *setup_namerslv_parms(parm, string)
	char	*parm,
		*string;
{
	char	*cp;

	strcpy(parm, NAMERSLV);
	SKIPDELIM(string);
	switch (smit_compare_attr(namerslv_attr, string, TRUE)) {
		case DOMAIN:
			strcat(parm, TCPFLG_c);
			string += strlen(namerslv_attr[DOMAIN]);
			cp = string;
			SKIPTOKEN(cp);
			strncat(parm, string, cp - string);
			break;
		case NAMESERVER:
			strcat(parm, TCPFLG_a);
			strcat(parm, TCPFLG_i);
			string += strlen(namerslv_attr[NAMESERVER]);
			cp = string;
			SKIPTOKEN(cp);
			strncat(parm, string, cp - string);
			break;
		case NO_ATTR:
			usage(MSGSTR(BAD_ATTR,"invalid attribute: %s."),string);
			break;
	}
	return(cp);
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
		MSGSTR(USAGE,"\nusage:	%s -a \"attr_val...\" | -A filename\n"),
		progname);
    	exit(2);
}
