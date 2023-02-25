static char sccsid[] = "@(#)28	1.6  src/tcpip/usr/sbin/chprtsv/chprtsv.c, tcpip, tcpip411, GOLD410 3/7/94 20:59:52";
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
 * chprtsv -  Takes parameters from SMIT interface and calls various 
 *            low-level commands to modify Print Service related information.
 *
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include	"libffdb.h"
#include	"libsmit.h"
#include	"smitcfg.h"

#define	CHQUE		"/bin/chque"
#define	CHQUEDEV	"/bin/chquedev"
#define	REFRESHLPD	"/bin/refresh -s lpd"
#define	REFRESHQD 	"/bin/refresh -s qdaemon"

#include <nl_types.h>
#include "chprtsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_CHPRTSV,n,s)

extern 	char 	*optarg;
extern 	int 	optind;

char 	progname[128];			/* = argv[0] at run time */

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	CFG__SFT *cfgfile;

	void 	smit_parse_stanza();

	char	*a_optarray[128],
		*b_optarray[128],
		*h_hostarray[128],
		*x_hostarray[128],
		*attrlist[128];
	char	**h_hostname = h_hostarray ,
		*h_filename = (char *)NULL,
		**x_hostname = x_hostarray ,
		*x_filename = (char *)NULL,
		*qentry = (char *)NULL,
		*devname = (char *)NULL,
		*A_option = (char *)NULL,
		**a_option = a_optarray,
		**b_option = b_optarray,
		*cp = (char *)NULL,
		**attrpair, *att,
		parm[MAXRECLEN],
		recptr[MAXRECLEN],
		d_stanza[MAXATRSZ],
		d_name[MAXRECLEN], 
		d_attributes[MAXRECLEN],
		q_stanza[MAXATRSZ],
		q_name[MAXRECLEN], 
		q_attributes[MAXRECLEN];
    	int 	c = 0, 
		do_queue = FALSE,
		do_queuedev = FALSE,
		Aflag = 0, 
		Hflag = 0, 
		Xflag = 0, 
		aflag = 0, 
		bflag = 0, 
		vflag = 0, 
		cflag = 0, 
		dflag = 0, 
		hflag = 0, 
		iflag = 0, 
		qflag = 0, 
		sflag = 0, 
		xflag = 0; 
	extern char *formatattr();

	FILE	*fp = (FILE *)NULL,
		*ff_open();
#ifdef 	DEBUG
	FILE	*fdebug, *fopen();
	fdebug = fopen("/tmp/smitdebug","w");
#endif	DEBUG
	setlocale(LC_ALL, "");
	catd = catopen(MF_CHPRTSV,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc &&
	   (c = getopt(argc, argv, "a:b:v:cdh:iq:sx:H:A:X:")) != EOF) {
		switch (c) {
			case 'H':
				if (hflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Hflag++;
	 			h_filename = optarg;
	    			break;

			case 'A':
				if (aflag | bflag | qflag | vflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Aflag++;
	 			A_option = optarg;
	    			break;

			case 'X':
				if (xflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Xflag++;
				x_filename = optarg;
	    			break;

			case 'a':
				if (Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			aflag++;
	 			PARSE_LIST(optarg,a_option);
	    			break;

			case 'b':
				if (Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			bflag++;
	 			PARSE_LIST(optarg,b_option);
	    			break;

			case 'v':
				if (Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			vflag++;
	 			devname = optarg;
	    			break;

			case 'c':
				if (sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			cflag++;
	    			break;

			case 'd':
				if (iflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			dflag++;
	    			break;

			case 'h':
				if (Hflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
				hflag++;
				PARSE_LIST(optarg,h_hostname);
	    			break;

			case 'i':
				if (dflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			iflag++;
	    			break;

			case 'q':
				if (Aflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
				qflag++;
				qentry = optarg;
	    			break;

			case 's':
				if (cflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			sflag++;
	    			break;

			case 'x':
				if (Xflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
				xflag++;
				PARSE_LIST(optarg,x_hostname);
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

    	if (!(cflag || sflag))
		usage(MSGSTR(SPEC_SW,"you must specify either -c or -s."));

	if (qflag)
		if (!(aflag || bflag || vflag))
			usage(MSGSTR(SPEC_SW2,
			"you must specify the -a, -b, -q, and -v options"));

	if (aflag) {
		if (!qflag)
			usage(MSGSTR(SPEC_SW3,
			"you must specify the -a and -q options"));
		else
			do_queue = TRUE;
	}

	if (vflag) {
		if (!(qflag && bflag))
			usage(MSGSTR(SPEC_SW4,
			"you must specify the -b, -q, and -v options"));
		else
			do_queuedev = TRUE;
	}

	if (bflag) {
		if (!(vflag && qflag))
			usage(MSGSTR(SPEC_SW4,
			"you must specify the -b, -q, and -v options"));
		else
			do_queuedev = TRUE;
	}

	if (qflag) {
		if (do_queue) {
			strcpy(parm, CHQUE);
			strcat(parm, TCPFLG_q);
			strcat(parm, qentry);
			for(a_option=a_optarray; *a_option; a_option++){
				strcat(parm,TCPFLG_a);
				strcat(parm, TCP_QUOTE);
				strcat(parm,formatattr(*a_option));
				strcat(parm, TCP_QUOTE);
			}
#ifdef	DEBUG
	fprintf(fdebug,"command= %s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, CHQUE);
				exit(errno);
			}
		}
		if (do_queuedev) {
			strcpy(parm, CHQUEDEV);
			strcat(parm, TCPFLG_q);
			strcat(parm, qentry);
			strcat(parm, TCPFLG_d);
			strcat(parm, devname);
			for(b_option=b_optarray; *b_option; b_option++){
				strcat(parm,TCPFLG_a);
				strcat(parm, TCP_QUOTE);
				strcat(parm,formatattr(*b_option));
				strcat(parm, TCP_QUOTE);
			}
#ifdef	DEBUG
	fprintf(fdebug,"command= %s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, CHQUEDEV);
				exit(errno);
			}
		}
	}

	if (Aflag) {
		cfgfile = cfgcopsf(A_option);
		if (cfgfile == (CFG__SFT *)NULL) {
			fprintf(stderr, 
				MSGSTR(ERR_OPEN, "%s: error opening %s.\n"), 
				progname, A_option);
			exit(errno);
		}

		if (cfgcrdsz(cfgfile,q_stanza,sizeof(q_stanza),QUEUE) 
		    != CFG_SUCC) {
			fprintf(stderr, 
				MSGSTR(ERR_READ, 
				"%s: error reading %s stanza in %s.\n"), 
				progname, QUEUE, A_option);
			exit(errno);
		}

		if (cfgcrdsz(cfgfile,d_stanza,sizeof(d_stanza),DEVICE) 
		    != CFG_SUCC) 
			do_queuedev = FALSE;
		else
			do_queuedev = TRUE;

 		cfgcclsf(cfgfile);

		smit_parse_stanza(q_stanza, q_name, q_attributes);
		if (!strlen(q_name)) {
			fprintf(stderr, 
				MSGSTR(ERR_STANZA, 
				"%s: invalid %s stanza format in %s.\n"), 
				progname, QUEUE, a_option);
			exit(2);
		}

		if (strlen(q_attributes))
			do_queue = TRUE;
		else
			do_queue = FALSE;

		if (do_queuedev) {
			smit_parse_stanza(d_stanza, d_name, d_attributes);
			if (!strlen(d_name) || !strlen(d_attributes)) {
				fprintf(stderr, 
					MSGSTR(ERR_STANZA, 
				      "%s: invalid %s stanza format in %s.\n"), 
					progname, DEVICE, A_option);
				exit(2);
			}
		}

		if (do_queue) {
			strcpy(parm, CHQUE);
			strcat(parm, TCPFLG_q);
			strcat(parm, q_name);
			attrpair=attrlist;
			att = q_attributes;
			PARSE_LIST(att,attrpair);
			for(attrpair=attrlist; *attrpair; attrpair++){
				strcat(parm,TCPFLG_a);
				strcat(parm, TCP_QUOTE);
				strcat(parm,formatattr(*attrpair));
				strcat(parm, TCP_QUOTE);
			}
#ifdef	DEBUG
	fprintf(fdebug,"command= %s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, CHQUE);
				exit(errno);
			}
		}
		if (do_queuedev) {
			strcpy(parm, CHQUEDEV);
			strcat(parm, TCPFLG_q);
			strcat(parm, q_name);
			strcat(parm, TCPFLG_d);
			strcat(parm, d_name);
			attrpair=attrlist;
			att = d_attributes;
			PARSE_LIST(att,attrpair);
			for(attrpair=attrlist; *attrpair; attrpair++){
				strcat(parm,TCPFLG_a);
				strcat(parm, TCP_QUOTE);
				strcat(parm,formatattr(*attrpair));
				strcat(parm, TCP_QUOTE);
			}
#ifdef	DEBUG
	fprintf(fdebug,"command= %s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, CHQUEDEV);
				exit(errno);
			}
		}
	}


    	if (sflag) {
		if (hflag || Hflag) {
			strcpy(parm, RUSER);
		 	strcat(parm, TCPFLG_a);
			if (hflag) {
			 	for(h_hostname=h_hostarray; *h_hostname;h_hostname++){
					strcat(parm,TCPFLG_p);
					strcat(parm, *h_hostname);
				}
			} else {
				fp = ff_open(h_filename, "r");
				while (fgets(recptr, MAXRECLEN, fp) > 0) {
					if ((cp = strchr(recptr, '\n')) != NULL)
						*cp = '\0';
					cp = recptr;
					cp += strspn(cp, DELIM);
					strcat(parm, TCPFLG_p);
					strcat(parm, cp);
				}
				ff_close(h_filename, fp);
			}
#ifdef	DEBUG
	fprintf(fdebug,"command = %s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, RUSER);
				exit(errno);
			}
		}
		if (xflag || Xflag) {
			strcpy(parm, RUSER);
		 	strcat(parm, TCPFLG_d);
			if (xflag) {
			 	for(x_hostname=x_hostarray; *x_hostname;x_hostname++){
					strcat(parm,TCPFLG_p);
					strcat(parm, *x_hostname);
				}
			} else {
				fp = ff_open(x_filename, "r");
				while (fgets(recptr, MAXRECLEN, fp) > 0) {
					if ((cp = strchr(recptr, '\n')) != NULL)
						*cp = '\0';
					cp = recptr;
					cp += strspn(cp, DELIM);
					strcat(parm, TCPFLG_p);
					strcat(parm, cp);
				}
				ff_close(x_filename, fp);
			}
#ifdef	DEBUG
	fprintf(fdebug,"command = %s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, RUSER);
				exit(errno);
			}
		}
		strcpy(parm, REFRESHQD);
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, REFRESHQD);
			exit(errno);
		}
		strcpy(parm, REFRESHLPD);
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, REFRESHLPD);
			exit(errno);
		}
	}
#ifdef	DEBUG
	fclose(fdebug);
#endif	DEBUG
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
"\nusage:	%s -c | -s [-d | -i] [-h \"hostname...\" | -H filename]\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE2,
"	           [-x \"hostname...\" | -X filename] [-q \"qentry\" -v \"devname\"\n")); 
    	fprintf(stderr, 
		MSGSTR(USAGE3,
"	           -a \"attr=val...\" -b \"attr=val...\" | -A filename]\n"));
    	
    	exit(2);
}
char *formatattr(attrval)
char *attrval;
{
	char result[80];
	int i;
	i = strcspn(attrval,"=");
	strncpy(result,attrval,i);
	result[i]='\0';
	strcat(result," = ");
	strcat(result,(attrval+i+1));
	return(result);
}

