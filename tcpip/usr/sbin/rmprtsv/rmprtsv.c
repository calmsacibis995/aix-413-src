static char sccsid[] = "@(#)22	1.8  src/tcpip/usr/sbin/rmprtsv/rmprtsv.c, tcpip, tcpip411, GOLD410 3/8/94 18:27:14";
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
 * rmprtsv.c -  Takes parameters from SMIT interface and calls various 
 *              low-level command to modify Print Service related information.
 *              After modification is completed, SRC is called to reactivate  
 *              the print service server.
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include	"libffdb.h"
#include	"libsmit.h"

#define	RMQUE		"/bin/rmque"
#define	RMQUEDEV	"/bin/rmquedev"
#define	STOPLPD		"/bin/stopsrc -s lpd"
#define	STOPQD		"/bin/stopsrc -s qdaemon"

#include <nl_types.h>
#include "rmprtsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_RMPRTSV,n,s)

char 	progname[128];			/* = argv[0] at run time */

extern 	char 	*optarg;
extern 	int 	optind;

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	char	*hostarray[128];
	char	*filename = (char *)NULL,
		*cp = (char *)NULL,
		**hostname = hostarray,
		*qentry = (char *)NULL,
		*devname = (char *)NULL,
		parm[MAXRECLEN],
		recptr[MAXRECLEN];

    	int 	c = 0, 
		Hflag = 0, 
		Aflag = 0, 
		Uflag = 0, 
		Tflag = 0, 
		cflag = 0, 
		vflag = 0, 
		hflag = 0, 
		qflag = 0, 
		sflag = 0; 

	FILE	*fp,
		*ff_open();

	setlocale(LC_ALL, "");
	catd = catopen(MF_RMPRTSV,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && (c = getopt(argc,argv,"cv:h:sq:H:ATU")) != EOF) {
		switch (c) {
			case 'H':
				if (hflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Hflag++;
	 			filename = optarg;
	    			break;

			case 'A':
				if (Uflag | Tflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Aflag++;
	    			break;

			case 'T':
				if (Uflag | Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Tflag++;
	    			break;

			case 'U':
				if (Tflag | Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Uflag++;
	    			break;

			case 'c':
				if (sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			cflag++;
	    			break;

			case 'h':
				if (Hflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	 			hflag++;
	 			PARSE_LIST(optarg,hostname);
	    			break;

			case 'v':
	 			vflag++;
	 			devname = optarg;
	    			break;

			case 'q':
	 			qflag++;
	 			qentry=optarg;
	    			break;

			case 's':
				if (cflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			sflag++;
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

    	if (!(sflag || cflag))
		usage(MSGSTR(SPEC_SW,"you must specify either -c or -s."));

	if (vflag)
		if (!qflag)
			usage(MSGSTR(SPEC_SW2,
				"you must specify both -q and -v."));

	if (qflag) {
		if (vflag) {
			strcpy(parm, RMQUEDEV);
		 	strcat(parm, TCPFLG_q);
		 	strcat(parm, qentry);
		 	strcat(parm, TCPFLG_d);
		 	strcat(parm, devname);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, RMQUEDEV);
				exit(errno);
			}
		} else {
			strcpy(parm, RMQUE);
			strcat(parm, TCPFLG_q);
		 	strcat(parm, qentry);
			if (system(parm) == -1) {
				fprintf(stderr, 
				MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, RMQUE);
				exit(errno);
			}
		}
	}

    	if (sflag) {
		if (hflag || Hflag) {
			strcpy(parm, RUSER);
		 	strcat(parm, TCPFLG_d);
			if (hflag) {
				for(hostname=hostarray; *hostname; hostname++){
		 			strcat(parm, TCPFLG_p);
		 			strcat(parm, *hostname);
				}
			} else {
				fp = ff_open(filename, "r");
				while (fgets(recptr, MAXRECLEN, fp) > 0) {
					if ((cp = strchr(recptr, '\n')) != NULL)
						*cp = '\0';
					cp = recptr;
					SKIPDELIM(cp);
					strcat(parm, TCPFLG_p);
					strcat(parm, cp);
				}
				ff_close(filename, fp);
			}
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, RUSER);
				exit(errno);
			}
		}
		if (Tflag) {
		 	strcpy(parm, STOPQD);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, STOPQD);
				exit(errno);
			}
		 	strcpy(parm, STOPLPD);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, STOPLPD);
				exit(errno);
			}
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
		MSGSTR(USAGE1,
"\nusage:	%s -c | -s [-A | -T | -U] [-h \"hostname...\" | -H filename]\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE2,
"		[-q \"qentry...\"] [-q qentry -v \"devname...\"]\n"));
    	exit(2);
}
