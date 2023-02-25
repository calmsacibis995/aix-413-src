static char sccsid[] = "@(#)24	1.9  src/tcpip/usr/sbin/mkprtsv/mkprtsv.c, tcpip, tcpip411, GOLD410 3/8/94 18:01:08";
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
 * mkprtsv.c -  Takes parameters from SMIT interface and calls various 
 *              low-level commands to modify Print Service related information.
 *              After modification is completed, SRC is called to reactivate  
 *              the print service server.
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<string.h>
#include  	"cfg04.h"
#include	"libffdb.h"
#include	"libsmit.h"
#include	"smitcfg.h"

#define	MKQUE		"/bin/mkque"
#define	MKQUEDEV	"/bin/mkquedev"
#define STARTLPD	"/bin/startsrc -s lpd"
#define STARTQD		"/bin/startsrc -s qdaemon"

#include <nl_types.h>
#include "mkprtsv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_MKPRTSV,n,s)

extern 	char 	*optarg;
extern 	int 	optind;

char 	progname[128];			/* = argv[0] at run time */

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	CFG__SFT *cfgfile;

	void 	smit_parse_stanza();
	char 	*a_optarray[128],
		*b_optarray[128],
		*hostarray[128],
		*attrlist[128];
	char	**hostname = hostarray,
		**attrpair,*att ,
		*cp = (char *)NULL,
		*filename = (char *)NULL,
		*A_option = (char *)NULL,
		**a_option = a_optarray,
		**b_option = b_optarray,
		*quename = (char *)NULL,
		*devname = (char *)NULL,
		parm[MAXRECLEN],
		recptr[MAXRECLEN],
		d_stanza[MAXATRSZ],
		d_name[MAXRECLEN], 
		d_attributes[MAXRECLEN],
		q_stanza[MAXATRSZ],
		q_name[MAXRECLEN], 
		q_attributes[MAXRECLEN];
	extern char *formatattr();
    	int 	c = 0, 
		Hflag = 0, 
		Aflag = 0, 
		Sflag = 0, 
		aflag = 0, 
		bflag = 0, 
		cflag = 0, 
		hflag = 0, 
		qflag = 0, 
		sflag = 0, 
		vflag = 0; 

	FILE	*fp = (FILE *)NULL,
		*ff_open();
#ifdef DEBUG
	FILE *fdebug, *fopen();
	fdebug = fopen("/tmp/smitdebug","w");
#endif DEBUG

	setlocale(LC_ALL, "");
	catd = catopen(MF_MKPRTSV,NL_CAT_LOCALE);
    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && 
	      (c = getopt(argc,argv,"a:b:ch:q:sv:SH:A:")) != EOF) {
		switch (c) {
			case 'H':
				if (hflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Hflag++;
	 			filename = optarg;
	    			break;

			case 'A':
				if (aflag | bflag | qflag | vflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			Aflag++;
	 			A_option = optarg;
	    			break;

			case 'a':
				if (Aflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			aflag++;
	 			PARSE_LIST(optarg,a_option) ;
	    			break;

			case 'b':
				if (Aflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			bflag++;
	 			PARSE_LIST(optarg,b_option) ;
	    			break;

			case 'q':
				if (Aflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			qflag++;
	 			quename = optarg;
	    			break;

			case 'v':
				if (Aflag) 
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches."));
	    			vflag++;
	 			devname = optarg;
	    			break;

			case 'S':
	    			Sflag++;
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
	 			PARSE_LIST(optarg,hostname) ;
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

    	if (!(cflag || sflag))
		usage(MSGSTR(SPEC_SW,"you must specify either -c or -s."));

	if (qflag)
		if (!(bflag && vflag))
			usage(MSGSTR(SPEC_SW2,
			"you must specify the -b, -q, and -v options"));

	if (aflag)
		if (!(qflag && bflag && vflag))
			usage(MSGSTR(SPEC_SW2,
			"you must specify the -a, -b, -q, and -v options"));

	if (bflag)
		if (!(qflag && vflag))
			usage(MSGSTR(SPEC_SW2,
			"you must specify the -a, -b, -q, and -v options"));

	if (vflag)
		if (!(bflag && qflag))
			usage(MSGSTR(SPEC_SW2,
			"you must specify the -a, -b, -q, and -v options"));

	if (qflag) {
		strcpy(parm, MKQUE);
		strcat(parm, TCPFLG_q);
		strcat(parm, quename);
		for(a_option=a_optarray; *a_option; a_option++){
			strcat(parm,TCPFLG_a);
			strcat(parm, TCP_QUOTE);
			strcat(parm,formatattr(*a_option));
			strcat(parm, TCP_QUOTE);
		}
#ifdef DEBUG
	fprintf(fdebug,"command = %s\n",parm);
#endif DEBUG
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, MKQUE);
			exit(errno);
		}
		strcpy(parm, MKQUEDEV);
		strcat(parm, TCPFLG_q);
		strcat(parm, quename);
		strcat(parm, TCPFLG_d);
		strcat(parm, devname);
		for(b_option=b_optarray; *b_option; b_option++){
			strcat(parm,TCPFLG_a);
			strcat(parm, TCP_QUOTE);
			strcat(parm,formatattr(*b_option));
			strcat(parm, TCP_QUOTE);
		}
#ifdef DEBUG
	fprintf(fdebug,"command = %s\n",parm);
#endif DEBUG
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, MKQUEDEV);
			exit(errno);
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
		    != CFG_SUCC) {
			fprintf(stderr, 
				MSGSTR(ERR_READ, 
				"%s: error reading %s stanza in %s.\n"), 
				progname, DEVICE, A_option);
			exit(errno);
		}

 		cfgcclsf(cfgfile);

		smit_parse_stanza(q_stanza, q_name, q_attributes);
		if (!strlen(q_name) || !strlen(q_attributes)) {
			fprintf(stderr, 
				MSGSTR(ERR_STANZA, 
				"%s: invalid %s stanza format in %s.\n"), 
				progname, QUEUE, A_option);
			exit(2);
		}

		smit_parse_stanza(d_stanza, d_name, d_attributes);
		if (!strlen(d_name) || !strlen(d_attributes)) {
			fprintf(stderr, 
				MSGSTR(ERR_STANZA, 
				"%s: invalid %s stanza format in %s.\n"), 
				progname, DEVICE, A_option);
			exit(2);
		}

		strcpy(parm, MKQUE);
		strcat(parm, TCPFLG_q);
		strcat(parm, q_name);
		attrpair= attrlist ;
		att = q_attributes ;
		PARSE_LIST(att,attrpair);	
		for(attrpair=attrlist  ; *attrpair; attrpair++){
			strcat(parm,TCPFLG_a);
			strcat(parm, TCP_QUOTE);
			strcat(parm,formatattr(*attrpair));
			strcat(parm, TCP_QUOTE);
		}
#ifdef DEBUG
	fprintf(fdebug,"command = %s\n",parm);
#endif DEBUG
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, MKQUE);
			exit(errno);
		}
		strcpy(parm, MKQUEDEV);
		strcat(parm, TCPFLG_q);
		strcat(parm, q_name);
		strcat(parm, TCPFLG_d);
		strcat(parm, d_name);
		attrpair= attrlist ;
		att = d_attributes ;
		PARSE_LIST(att,attrpair);	
		for(attrpair=attrlist  ; *attrpair; attrpair++){
			strcat(parm,TCPFLG_a);
			strcat(parm, TCP_QUOTE);
			strcat(parm,formatattr(*attrpair));
			strcat(parm, TCP_QUOTE);
		}
#ifdef DEBUG
	fprintf(fdebug,"command = %s\n",parm);
#endif DEBUG
		if (system(parm) == -1) {
			fprintf(stderr, 
				MSGSTR(ERR_EXEC, "%s: error executing %s.\n"), 
				progname, MKQUEDEV);
			exit(errno);
		}
	}

    	if (sflag) {
		if (hflag || Hflag) {
			strcpy(parm, RUSER);
		 	strcat(parm, TCPFLG_a);
			if (hflag) {
				for(hostname=hostarray; *hostname; hostname++){
					strcat(parm,TCPFLG_p);
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
#ifdef	DEBUG
	fprintf(fdebug,"command=%s\n",parm);
#endif	DEBUG
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC, 
					"%s: error executing %s.\n"), 
					progname, RUSER);
				exit(errno);
			}
		}
		if (Sflag) {
			strcpy(parm, STARTQD);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, STARTQD);
				exit(errno);
			}
			strcpy(parm, STARTLPD);
			if (system(parm) == -1) {
				fprintf(stderr, 
					MSGSTR(ERR_EXEC,
					"%s: error executing %s.\n"), 
					progname, STARTLPD);
				exit(errno);
			}
		}
	}
#ifdef DEBUG
	fclose(fdebug);
#endif DEBUG
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
"\nusage:	%s -c [-S] [-q queue_name -v devname -b \"attr=val...\"\n"), 
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE2,"	-a \"attr=val...\" | -A filename]\n")); 
    	fprintf(stderr, 
		MSGSTR(USAGE3,
"\nusage:	%s -s [-S] [-q queue_name -v devname -b \"attr=val...\"\n"),
		progname);
    	fprintf(stderr, 
		MSGSTR(USAGE2,"	-a \"attr=val...\" | -A filename]\n")); 
    	fprintf(stderr, 
		MSGSTR(USAGE4,"	[-h \"hostname...\" | -H filename]\n"));
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

