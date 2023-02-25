static char sccsid[] = "@(#)55	1.7  src/tcpip/usr/sbin/chrctcp/chrctcp.c, tcpip, tcpip411, GOLD410 3/7/94 21:00:36";
/*
 * COMPONENT NAME: (TCPIP) SMIT low-level commands
 *
 * FUNCTIONS: output_disc_format, output_inetd, output_syslogd, parse_syslogd,
 *            output_named, parse_named, output_timed, parse_timed
 *            output_gated, parse_gated, output_routed, parse_routed
 *	      parse_cmdflags, parse_inetd
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
 * chrctcp.c -  Takes parameters from SMIT interface or the command line
 *              and modifies /etc/rc.tcpip to comment or uncomment a TCP/IP   
 *  	        daemon entry.  This program will not physically add or delete
 *		entries from /etc/rc.tcpip.  Although this program can be run
 *		from the command line, it was intended to be run from SMIT.
 */                                                                   

#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/times.h>
#include	"libffdb.h"
#include	"libsmit.h"

#define	RCTCP   "/etc/rc.tcpip"

char 	progname[128],			/* = argv[0] at run time */
	*strstr();
static	char	delim[] = " \t";  /* word delimiters */

extern 	char 	*optarg;
extern 	int 	optind;

#include <nl_types.h>
#include "chrctcp_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_CHRCTCP,n,s)

main(argc, argv)
	int 	argc;
	char 	**argv;
{
	char	*start, *cp = (char *)NULL,
		*daemon = (char *)NULL,
		*cmdflags[MAXRECLEN],
    	 	tmpfile[128],
		startparms[MAXRECLEN], 
    		inbuf[MAXRECLEN], 
		outbuf[MAXRECLEN],		
    		parm[MAXRECLEN]; 

    	int 	c, 
		rc = 0,
		got_parms = FALSE,
		aflag = 0, 
		cflag = 0, 
		dflag = 0, 
		fflag = 0, 
		sflag = 0, 
		Sflag = 0; 

	FILE	*fp = (FILE *)NULL,
		*tfp = (FILE *)NULL,
	 	*ff_open();

	struct	stat	sb;

	setlocale(LC_ALL, "");
	catd = catopen(MF_CHRCTCP,NL_CAT_LOCALE);

    	/* save program name, for error messages, etc. */
    	strcpy(progname, *argv);

    	if (argc < 2)  
		usage(0);

    	/* process command line args */
    	while (optind < argc && (c = getopt(argc, argv, "a:c:d:f:s:S")) != EOF) {
		switch (c) {
			case 'S':
				if (sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			Sflag++;
	    			break;

			case 'a':
				if (cflag || dflag || sflag || fflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			aflag++;
				daemon = optarg;
	    			break;

			case 'c':
				if (aflag || dflag || sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			cflag++;
				daemon = optarg;
	    			break;

			case 'd':
				if (cflag || aflag || sflag || fflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			dflag++;
				daemon = optarg;
	    			break;

			case 'f':
				if (aflag || dflag || sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
				cmdflags[fflag++] = optarg;
	    			break;

			case 's':
				if (aflag || cflag || fflag || dflag || Sflag)
					usage(MSGSTR(INCOMP_SW, 
					"incompatible command line switches"));
	    			sflag++;
				daemon = optarg;
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

    	if (!(aflag || cflag || dflag || sflag))
		usage(MSGSTR(SPEC_SW,
			"you must specify either -a, -c, -d, or -s."));

	if (fflag)
		got_parms = parse_cmdflags(startparms, daemon, cmdflags, fflag);

	if (aflag || dflag || cflag) {
    		if (stat(RCTCP, &sb) != 0) {
			fprintf(stderr,
				MSGSTR(ERR_STAT, "%s: %s does not exist.\n"),
				progname, RCTCP);
			exit(errno);
    		}

    		strcpy(tmpfile, tmpnam(NULL));
    		if ((tfp = fopen(tmpfile, "w")) == 0) {
			fprintf(stderr,
				MSGSTR(ERR_OPEN2,"%s: can't open temp file.\n"),
				progname);
			exit(errno);
    		}
		fp = ff_open(RCTCP, "r");

		while (fgets(inbuf, MAXRECLEN, fp) > 0) {

		    /* strip trailing newline */
		    inbuf[strlen(inbuf) - 1] = '\0';

		    /* check if first word is "start" */
		    for (cp = start = inbuf; *cp == '#';
			++cp); /* skip comment chars */
		    if (! strncmp(cp, "start ", 6)) {  /* found it */

			/* check if second word is daemon pathname */
			cp += 5;
			cp += strspn(cp, delim);  /* skip to next word */
			cp += strcspn(cp, delim);  /* skip to end of word */
			if (! strncmp(cp - strlen(daemon), daemon,
			    strlen(daemon))) {
			
			    /* found our line: now massage it */
			    if (aflag || cflag) {  /* uncomment it */

				/* strip leading comments */
				while (*start == '#')
				    ++start;;

				if (fflag) {
				    /* find fourth word (optional args) */
				    cp += strspn(cp, delim);
				    cp += strcspn(cp, delim);
				    *cp = '\0';  /* strip args */
				    if (got_parms)  /* add new args */
					 strcat(start, startparms);
				}

			    /* dflag: insert comment char */
			    } else if (*start != '#')
				fputs("#", tfp);
			}
		    }
		    fputs(start, tfp);
		    fputc('\n', tfp);
		}

		ff_close(RCTCP, fp);
    		fclose(tfp);

    		if (rename(tmpfile, RCTCP) == -1) {
			if (errno == EXDEV)
	    			ff_rename_by_copying(tmpfile, RCTCP);
			else {
				fprintf(stderr, MSGSTR(ERR_RENAME,
				     "%s: can't move temp file back to %s.\n"),
					progname, RCTCP);
	    			exit(errno);
			}
    		}
		if (chmod(RCTCP, sb.st_mode) == -1) {
	    		fprintf(stderr, MSGSTR(ERRACC, 
				"%s: error changing access mode of file: %s\n"),
		    		progname, RCTCP);
	    		exit(errno);
		}
	}

	if (sflag) {
		fp = ff_open(RCTCP, "r");
		while (fgets(inbuf, MAXRECLEN, fp) > 0) {

			/* strip trailing newline */
			inbuf[strlen(inbuf) - 1] = '\0';

			/* check if first word is "start" */
			cp += strspn(cp = inbuf, " \t#");  /* get first word */
			if (! strncmp(cp, "start ", 6)) {  /* found it */

			    /* check if second word is daemon pathname */
			    cp += 5;
			    cp += strspn(cp, delim);  /* skip to next word */
			    cp += strcspn(cp, delim);  /* skip to end of word */
			    if (! strncmp(cp - strlen(daemon), daemon,
				strlen(daemon)))
				    output_disc_format(daemon, cp);
			}
		}
		ff_close(RCTCP, fp);
	}

	if (Sflag) {
		if (cflag) {
			strcpy(outbuf, "/bin/stopsrc -cs ");
			strcat(outbuf, daemon);
			system(outbuf);
		}
		cp = (dflag) ? "/bin/stopsrc" : "/bin/startsrc";
		strcpy(outbuf, cp);
		strcat(outbuf, " -s ");
		strcat(outbuf, daemon);
                if (got_parms) {
			strcat(outbuf, " -a");
			strcat(outbuf, startparms);
		}
                if ((rc = system(outbuf)) < 0) {
                        fprintf(stderr,
                                MSGSTR(ERR_EXEC,"error executing %s."),outbuf);
                        exit(-1);
                }
	}

	exit(0);
}


int parse_cmdflags(startparms, daemon, cmdflags, fflag)
	char	*daemon,
		*cmdflags[],
		*startparms;
	int	fflag;
{
	if (!strcmp(daemon, "inetd"))
		parse_inetd(startparms, cmdflags, fflag);
	else if (!strcmp(daemon, "syslogd"))
		parse_syslogd(startparms, cmdflags, fflag);
	else if (!strcmp(daemon, "routed"))
		parse_routed(startparms, cmdflags, fflag);
	else if (!strcmp(daemon, "gated"))
		parse_gated(startparms, cmdflags, fflag);
	else if (!strcmp(daemon, "named"))
		parse_named(startparms, cmdflags, fflag);
	else if (!strcmp(daemon, "timed"))
		parse_timed(startparms, cmdflags, fflag);
	if (strlen(startparms) <= 3)
		return(FALSE);
	return(TRUE);
}


int parse_inetd(startparms, cmdflags, fflag)
	char	*startparms,
		*cmdflags[];
	int	fflag;
{
	char 	*cp;
	int	x = 0;
	
	strcpy(startparms, " \"");
	while (x < fflag) {
		if (!strncmp(cmdflags[x], "debug=", strlen("debug="))) {
			cp = cmdflags[x];
			cp += strlen("debug=");
			if (*cp == 'y')
				strcat(startparms, " -d ");
		} else if (!strncmp(cmdflags[x], "file=", strlen("file="))) {
			cp = cmdflags[x];
			cp += strlen("file=");
			strcat(startparms, cp);
		} else {
                        fprintf(stderr, MSGSTR(BAD_ATTR,
				"The attribute %s is not valid.\n"),
				cmdflags[x]);
                        exit(1);
                }
		x++;
	}
	strcat(startparms, "\"");
}


int parse_named(startparms, cmdflags, fflag)
	char	*startparms,
		*cmdflags[];
	int	fflag;
{
	char 	*cp;
	int	x = 0;
	
	strcpy(startparms, " \"");
	while (x < fflag) {
		if (!strncmp(cmdflags[x],"debug_level=",strlen("debug_level="))) {
			cp = cmdflags[x];
			cp += strlen("debug_level=");
			if (*cp != '0') {
				strcat(startparms, " -d ");
				strcat(startparms, cp);
			}
		} else if(!strncmp(cmdflags[x],"portnum=",strlen("portnum="))) {
			cp = cmdflags[x];
			cp += strlen("portnum=");
			strcat(startparms, " -p ");
			strcat(startparms, cp);
		} else if (!strncmp(cmdflags[x], "boot_file=", strlen("boot_file="))) {
			cp = cmdflags[x];
			cp += strlen("boot_file=");
			strcat(startparms, " -b ");
			strcat(startparms, cp);
		} else {
                        fprintf(stderr, MSGSTR(BAD_ATTR,
				"The attribute %s is not valid.\n"),
				cmdflags[x]);
                        exit(1);
                }
		x++;
	}
	strcat(startparms, "\"");
}


int parse_timed(startparms, cmdflags, fflag)
	char	*startparms,
		*cmdflags[];
	int	fflag;
{
	char 	*cp;
	int	x = 0,
		iflag = FALSE,
		nflag = FALSE;
	
	strcpy(startparms, " \"");
	while (x < fflag) {
		if (!strncmp(cmdflags[x],"trace=",strlen("trace="))) {
			cp = cmdflags[x];
			cp += strlen("trace=");
			if (*cp == 'y')
				strcat(startparms, " -t ");
		} else if(!strncmp(cmdflags[x],"master=",strlen("master="))) {
			cp = cmdflags[x];
			cp += strlen("master=");
			if (*cp == 'y')
				strcat(startparms, " -M ");
		} else if(!strncmp(cmdflags[x],"ignore_net=",strlen("ignore_net="))) {
			iflag = TRUE;
			cp = cmdflags[x];
			cp += strlen("ignore_net=");
			strcat(startparms, " -i ");
			strcat(startparms, cp);
		} else if (!strncmp(cmdflags[x], "valid_net=", strlen("valid_net="))) {
			nflag = TRUE;
			cp = cmdflags[x];
			cp += strlen("valid_net=");
			strcat(startparms, " -n ");
			strcat(startparms, cp);
		} else {
                        fprintf(stderr, MSGSTR(BAD_ATTR,
				"The attribute %s is not valid.\n"),
				cmdflags[x]);
                        exit(1);
                }
		x++;
	}
	if (iflag && nflag) {
		fprintf(stderr, MSGSTR(BAD_TIMED,
		"The Valid and Ignore Network options are incompitable.\n"));
               	exit(1);
	}
	strcat(startparms, "\"");
}


int parse_routed(startparms, cmdflags, fflag)
	char	*startparms,
		*cmdflags[];
	int	fflag;
{
	char 	*cp;
	int	x = 0,
		quiet = FALSE,
		supply = FALSE;
	
	strcpy(startparms, " \"");
	while (x < fflag) {
		if (!strncmp(cmdflags[x], "debug=", strlen("debug="))) {
			cp = cmdflags[x];
			cp += strlen("debug=");
			if (*cp == 'y')
				strcat(startparms, " -d");
		} else if(!strncmp(cmdflags[x],"gateway=",strlen("gateway="))) {
			cp = cmdflags[x];
			cp += strlen("gateway=");
			if (*cp == 'y')
				strcat(startparms, " -g");
		} else if (!strncmp(cmdflags[x],"quiet=",strlen("quiet="))) {
			cp = cmdflags[x];
			cp += strlen("quiet=");
			if (*cp == 'y') {
				if (supply) {
                        		fprintf(stderr, MSGSTR(BAD_ROUTE,
					"Suppress routing and Supply routing options incompatible.\n"));
                        		exit(1);
				}
				strcat(startparms, " -q");
				quiet = TRUE;
			}
		} else if (!strncmp(cmdflags[x],"supply=",strlen("supply="))) {
			cp = cmdflags[x];
			cp += strlen("supply=");
			if (*cp == 'y') {
				if (quiet) {
                        		fprintf(stderr, MSGSTR(BAD_ROUTE,
					"Suppress routing and Supply routing options incompatible.\n"));
                        		exit(1);
				}
				strcat(startparms, " -s");
				supply = TRUE;
			}
		} else if (!strncmp(cmdflags[x],"trace=",strlen("trace="))) {
			cp = cmdflags[x];
			cp += strlen("trace=");
			if (*cp == 'y')
				strcat(startparms, " -t");
		} else if (!strncmp(cmdflags[x], "file=", strlen("file="))) {
			cp = cmdflags[x];
			cp += strlen("file=");
			strcat(startparms, " ");
			strcat(startparms, cp);
		} else {
                        fprintf(stderr, MSGSTR(BAD_ATTR,
				"The attribute %s is not valid.\n"),
				cmdflags[x]);
                        exit(1);
                }
		x++;
	}
	strcat(startparms, "\"");
}


int parse_gated(startparms, cmdflags, fflag)
	char	*startparms,
		*cmdflags[];
	int	fflag;
{
	int	i;
	static struct {
		char *flag;
		char *letter;
	} *optp, options[] = {  /* trace options */
		{ "external", "external," },
		{ "hello", "hello," },
		{ "internal", "internal," },
		{ "snmp", "snmp," },
		{ "egp", "egp," },
		{ "rip", "rip," },
		{ "changes", "route,kernel," },
		{ "updates", "update," },
		{ NULL },
	};
	char flag[40], val[40], logfile[256], traceopts[10];
	
	*traceopts = *logfile = '\0';
	for (i = 0; i < fflag; ++i) {

		if (sscanf(cmdflags[i], "%[^=]=%s", flag, val) != 2)
			break;

		if (!strcmp(flag, "file"))
			strcpy(logfile, val);

		else {  /* parse trace options */
			for (optp = options; optp->flag; ++optp)
				if (!strcmp(optp->flag, flag))
					break;
			if (!optp->flag)  /* invalid flag */
				break;
			if (*val == 'y') {
				strcat(traceopts, optp->letter);
			}
		}
	}
	/* Now remove comma at end of traceopts if needed */
	if (*traceopts)
		if (traceopts[strlen(traceopts)] == ',')
			traceopts[strlen(traceopts)] = '\0';

	if (i < fflag) {
		fprintf(stderr, MSGSTR(BAD_ATTR,
			"The attribute %s is not valid.\n"), cmdflags[i]);
		exit(1);
	}

	if (*traceopts || *logfile)
		sprintf(startparms, " \"%s%s %s\"", *traceopts ? "-t" : "",
		    traceopts, logfile);
	else
		*startparms = '\0';
}


int parse_syslogd(startparms, cmdflags, fflag)
	char	*startparms,
		*cmdflags[];
	int	fflag;
{
	char 	*cp;
	int	x = 0;
	
	strcpy(startparms, " \"");
	while (x < fflag) {
		if (!strncmp(cmdflags[x], "debug=", strlen("debug="))) {
			cp = cmdflags[x];
			cp += strlen("debug=");
			if (*cp == 'y')
				strcat(startparms, " -d");
		} else if (!strncmp(cmdflags[x], "file=", strlen("file="))) {
			cp = cmdflags[x];
			cp += strlen("file=");
			strcat(startparms, " -f");
			strcat(startparms, cp);
		} else if (!strncmp(cmdflags[x],"minutes=",strlen("minutes="))){
			cp = cmdflags[x];
			cp += strlen("minutes=");
			strcat(startparms, " -m");
			strcat(startparms, cp);
		} else {
                        fprintf(stderr, MSGSTR(BAD_ATTR,
				"The attribute %s is not valid.\n"),
				cmdflags[x]);
                        exit(1);
                }
		x++;
	}
	strcat(startparms, "\"");
}


int output_disc_format(daemon, inbuf)
	char	*daemon,
		*inbuf;  /* points to daemon name in cmd string */
{
	char	*cp;

	/* remove quotes */
	for (cp = inbuf; *cp; ++cp)
		if (*cp == '"')
			*cp = ' ';

	/* skip to (optional) arg field */
	inbuf += strcspn(inbuf, delim);
	inbuf += strspn(inbuf, delim);  /* third word (src_running) */
	inbuf += strcspn(inbuf, delim);
	inbuf += strspn(inbuf, delim);  /* point to fourth word (args) */

	if (!strcmp(daemon, "inetd"))
		output_inetd(inbuf);
	else if (!strcmp(daemon, "syslogd"))
		output_syslogd(inbuf);
	else if (!strcmp(daemon, "routed"))
		output_routed(inbuf);
	else if (!strcmp(daemon, "gated"))
		output_gated(inbuf);
	else if (!strcmp(daemon, "named"))
		output_named(inbuf);
	else if (!strcmp(daemon, "timed"))
		output_timed(inbuf);
	else
		puts(inbuf);
}


int output_inetd(inbuf)
	char	*inbuf;
{
	char	*cp, 
		outbuf[MAXRECLEN];

	fprintf(stdout,"#debug_mode:config_file\n");
	if ((cp = strstr(inbuf, "-d")) != (char *)NULL) {
		strcpy(outbuf, "yes:");
		cp += strlen("-d");
	} else {
		strcpy(outbuf,"no:");
		cp = inbuf;
	}
	while (*cp == ' ' | *cp == '\t')
		cp++;
	strcat(outbuf,cp);
	puts(outbuf);
}


int output_routed(inbuf)
	char	*inbuf;
{
	char	*cp, *cp2,
		outbuf[MAXRECLEN];

	fprintf(stdout,"#debug_mode:gateway:quiet:supply:trace:log_file\n");
	/* first get the debug flag */
	if ((cp = strstr(inbuf, "-d")) != (char *)NULL)
		strcpy(outbuf, "yes:");
	else
		strcpy(outbuf,"no:");
	/* get the gateway flag */
	if ((cp = strstr(inbuf, "-g")) != (char *)NULL)
		strcat(outbuf, "yes:");
	else
		strcat(outbuf,"no:");
	/* get the quiet flag */
	if ((cp = strstr(inbuf, "-q")) != (char *)NULL)
		strcat(outbuf, "yes:");
	else
		strcat(outbuf,"no:");
	/* get the supply flag */
	if ((cp = strstr(inbuf, "-s")) != (char *)NULL)
		strcat(outbuf, "yes:");
	else
		strcat(outbuf,"no:");
	/* get the log-trace flag */
	if ((cp = strstr(inbuf, "-t")) != (char *)NULL)
		strcat(outbuf, "yes:");
	else
		strcat(outbuf,"no:");
	/* get the logfile name */
	cp = inbuf;
	while (*cp != '\0') {
		SKIPDELIM(cp);
		if (*cp == '-') {
			SKIPTOKEN(cp);
		} else if (*cp == '\0')
			break;
		else {
			cp2 = cp;
			SKIPTOKEN(cp2);
			strncat(outbuf, cp, (cp2 - cp));
			break;
		}
	}
	puts(outbuf);
}


int output_gated(inbuf)
	char	*inbuf;
{
	char	*cp, *cp2,
		outbuf[MAXRECLEN],
		traceopts[9];
	static char yes[] = "yes:", no[] = "no:",
	    notrace[] = "no:no:no:no:no:no:no:no:",
	    deftrace[] = "yes:no:yes:no:no:no:yes:no:";

	fprintf(stdout,"#external:hello:internal:snmp:egp:rip:changes:updates:log_file\n");

	/* first get the debug flag */
	if (cp = strstr(inbuf, "-t")) {
		/* check for trace options */
		if (!*(cp += 2) || isspace(*cp))  /* no options: use defaults */
			strcpy(outbuf, deftrace);
		else {  /* check each option */
			sscanf(cp, "%8s", traceopts);  /* parse opts */
			strcpy(outbuf, strstr(traceopts, "external") ? yes : no);
			strcat(outbuf, strstr(traceopts, "hello") ? yes : no);
			strcat(outbuf, strstr(traceopts, "internal") ? yes : no);
			strcat(outbuf, strstr(traceopts, "snmp") ? yes : no);
			strcat(outbuf, strstr(traceopts, "egp") ? yes : no);
			strcat(outbuf, strstr(traceopts, "rip") ? yes : no);
			strcat(outbuf, strstr(traceopts, "route,kernel") ? yes : no);
			strcat(outbuf, strstr(traceopts, "update") ? yes : no);
		}
	} else  /* no trace flag */
		strcpy(outbuf, notrace);

	/* get the logfile name */
	cp = inbuf;
	while (*cp != '\0') {
		SKIPDELIM(cp);
		if (*cp == '-') {
			SKIPTOKEN(cp);
		} else if (*cp == '\0')
			break;
		else {
			cp2 = cp;
			SKIPTOKEN(cp2);
			strncat(outbuf, cp, (cp2 - cp));
			break;
		}
	}
	puts(outbuf);
}


int output_syslogd(inbuf)
	char	*inbuf;
{
	char	*cp, *cp2,
		outbuf[MAXRECLEN];

	fprintf(stdout,"#debug_mode:config_file:minutes\n");
	/* first get the debug flag */
	if ((cp = strstr(inbuf, "-d")) != (char *)NULL)
		strcpy(outbuf, "yes:");
	else
		strcpy(outbuf,"no:");
	/* second get the alternate config file */
	if ((cp = strstr(inbuf, "-f")) != (char *)NULL) {
		cp += strlen("-f");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	} 
	strcat(outbuf,":");
	/* third get the marked minutes parmaeter */
	if ((cp = strstr(inbuf, "-m")) != (char *)NULL) {
		cp += strlen("-m");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	}
	puts(outbuf);
}


int output_named(inbuf)
	char	*inbuf;
{
	char	*cp, *cp2,
		outbuf[MAXRECLEN];

	fprintf(stdout,"#debug_level:portnum:boot_file\n");
	strcpy(outbuf,"");
	/* get the debug level */
	if ((cp = strstr(inbuf, "-d")) != (char *)NULL) {
		cp += strlen("-d");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	} else {
		strcat(outbuf,"0");
	}
	strcat(outbuf,":");
	/* get the alternate port number */
	if ((cp = strstr(inbuf, "-p")) != (char *)NULL) {
		cp += strlen("-p");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	} 
	strcat(outbuf,":");
	/* get the alternate boot file */
	if ((cp = strstr(inbuf, "-b")) != (char *)NULL) {
		cp += strlen("-b");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	}
	puts(outbuf);
}


int output_timed(inbuf)
	char	*inbuf;
{
	char	*cp, *cp2,
		outbuf[MAXRECLEN];

	fprintf(stdout,"#trace:master:valid_net:ignore_net\n");
	/* get the trace flag */
	if ((cp = strstr(inbuf, "-t")) != (char *)NULL)
		strcpy(outbuf, "yes:");
	else
		strcpy(outbuf,"no:");
	/* get the master flag */
	if ((cp = strstr(inbuf, "-M")) != (char *)NULL)
		strcat(outbuf, "yes:");
	else
		strcat(outbuf,"no:");
	/* get the valid network file */
	if ((cp = strstr(inbuf, "-n")) != (char *)NULL) {
		cp += strlen("-n");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	} 
	strcat(outbuf,":");
	/* get the ignore network parmaeter */
	if ((cp = strstr(inbuf, "-i")) != (char *)NULL) {
		cp += strlen("-i");
		SKIPDELIM(cp);
		cp2 = cp;
		SKIPTOKEN(cp2);
		strncat(outbuf, cp, (cp2 - cp));
	}
	puts(outbuf);
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
		MSGSTR(USAGE,"\nusage:	%s [-S] {-a|-d|-s} Daemon | {-c Daemon [-f \"Attribute=Value...\"]}\n"), 
		progname);
    	exit(2);
}
