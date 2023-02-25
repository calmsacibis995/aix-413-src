static char sccsid[] = "@(#)32	1.15  src/tcpip/usr/sbin/hostent/hostent.c, tcpip, tcpip411, GOLD410 3/8/94 17:33:24";
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: main
 *		ipaddr_chk
 *		usage
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 * hostent.c
 */

/*
 * hostent directly manipulates entries in the /etc/hosts file.  these
 * entries map IP addresses to host names.  this command allows the user
 * to add, delete, and show records in the file.
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "libffdb.h"

#define  DBFILE  "/etc/hosts"	/* name of the database file, like "/etc/hosts" */

char progname[128];		/* = argv[0] at run time */
char ipaddr_db[128];		/* the address representation in memory */

extern char *optarg;
extern int optind;

#include <nl_types.h>
#include "hostent_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define MSGSTR(n,s) catgets(catd,MS_HOSTENT,n,s)

extern char *colonize();	/* forward declaration */

main(argc,argv)
    int argc;
    char **argv;
{
	char *ipaddr = 0, *ipaddr2 = 0, *showarg = 0;
	char recptr[MAXRECLEN], *rp;
	char *ff_delete(), *(*colonizer)() = 0, *hostn = 0;
	int c, i, rc;
	int aflag = 0, dflag = 0, cflag = 0, sflag = 0, hflag = 0, iflag= 0;
	int Xflag = 0, Zflag = 0, Sflag = 0;
	char *incomp_msg = "incompatible command line switches";
	char *cp;

	setlocale(LC_ALL, "");
	catd = catopen(MF_HOSTENT, NL_CAT_LOCALE);

	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);

	if (argc < 2)  usage(0);

	/* process command line args */
	while ((c = getopt(argc, argv, "a:d:c:s:h:i:SZX")) != EOF) {
		switch (c) {
		case 'a':	/* add a record (line) to host file */
			if (dflag || cflag || sflag || Xflag || Sflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			ipaddr = optarg;
			aflag++;
			break;
		case 'd':	/* delete a record by IP addr */
			if (aflag || cflag || sflag || Xflag || Sflag) 
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			ipaddr = optarg;
			dflag++;
			break;
		case 'c':	/* change a record by IP addr */
			if (aflag || dflag || sflag || Xflag || Sflag) 
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			ipaddr = optarg;
			cflag++;
			break;
		case 's':	/* show record(s) in host file */
			if (aflag || cflag || dflag || Xflag || Sflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			showarg = optarg;
			sflag++;
			break;
		case 'h':	/* specify one or more hosts */
			if (dflag || sflag || Xflag || Sflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			/* at least one host name -- may be followed by cmnt */
			rp = optarg;
			SKIPDELIM(rp);

			if (!*rp || *rp == '#') {
			    usage(MSGSTR(ONEHOST,
			       "you must also specify at least one hostname."));
				exit(2);
			}
			hostn = optarg;
			hflag++;
			break;
		case 'i':	/* specify a new IP addr */
			if (aflag || dflag || sflag || Zflag || Xflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			ipaddr2 = optarg;
			iflag++;
			break;
		case 'S':
			Sflag++;
			break;
		case 'X':	/* delete all host entries */
			if (dflag || aflag || cflag || sflag || Zflag || hflag || iflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Xflag++;
			break;
		case 'Z':	/* print in colon format for SMIT */
			if (Xflag || cflag || hflag || iflag || dflag || aflag)
				usage(MSGSTR(INCOMP_SW, incomp_msg));
			Zflag++;
			colonizer = colonize;
			break;
		case '?':	/* show usage message */
			usage(0);
		default:
			usage(MSGSTR(UNK_SWITCH, "unknown switch: %c"), c);
		}
	}
	if (optind != argc)
		usage(MSGSTR(NOT_PARSED, "some parameters were not parsed."));
	if (ipaddr){
		if (strspn(ipaddr, "0123456789.") != strlen(ipaddr))
			usage(MSGSTR(NUM_IPADDR, "IP-addr \"%s\" must be numeric."), ipaddr);
	}
	if (ipaddr2){
		if (strspn(ipaddr2, "0123456789.") != strlen(ipaddr2))
			usage(MSGSTR(NUM_IPADDR, "IP-addr \"%s\" must be numeric."), ipaddr2);
	}
    
	if (aflag) {
		if (!hflag) {
			usage(MSGSTR(ONEHOST, "you must also specify at least one hostname."));
			exit(2);
		}
		if(ipaddr_chk(ipaddr)){
			fprintf(stderr,
				MSGSTR(IPEXIST,
				    "IP address %s already exists.\n"),
				ipaddr);
			exit (1);
		}
		if ((ipaddr = inet_ntoa(inet_addr(ipaddr))) == -1) {
		    fprintf(stderr,
			    MSGSTR(BADADDRESS,
				   "%s is an invalid address.\n"),
			    ipaddr);
		    exit(1);
		}
		sprintf(recptr, "%s\t", ipaddr);
		strcat(recptr, hostn);
		exit(!ff_add(DBFILE, recptr));
	}

	if (dflag) {		/* delete by IP address */
		/* ipaddr_chk will retrieve the exact ip address image from
		 * the data base and put it in ipaddr_db buf
		 */
		if ( ipaddr_chk(ipaddr) &&
		    ( ff_delete(DBFILE, 1, ipaddr_db, 0, 0) == 0 ) ) {
			fprintf(stderr,
				MSGSTR(IPNOEXIST,
				       "IP address %s does not exist.\n"),
				ipaddr);
			exit(1);
		} else
			exit(0);
	}

	if (Xflag)		/* delete all records */
		exit((ff_delete(DBFILE, 0, 0, 0, 0) == 0) ? 1 : 0);
    

	if (cflag) {
		if (!(hflag || iflag))
			usage(MSGSTR(HOST_OR_IP,
				     "must have either hostname(s) or IP address, or both"));
		if( iflag && ipaddr_chk(ipaddr2)){
			fprintf(stderr,
				MSGSTR(IPEXIST,
				    "IP address %s already exists.\n"),
				ipaddr2);
			exit (1);
		}
		if ( ipaddr_chk(ipaddr) &&
		      ((rp = ff_delete(DBFILE, 1, ipaddr_db, 0, 0)) == NULL) ) {
			fprintf(stderr, MSGSTR(IPNOEXIST,
				"IP address %s does not exist.\n"), ipaddr);
			exit(1);
		}
		if (hflag && !iflag) {
		        if ((ipaddr = inet_ntoa(inet_addr(ipaddr))) == -1) {
			    fprintf(stderr,
				    MSGSTR(BADADDRESS,
					   "%s is an invalid address.\n"),
				    ipaddr);
			    exit(1);
			}
			sprintf(recptr, "%s\t", ipaddr);
			strcat(recptr, hostn);
		}
		if (iflag && !hflag) {
		        if ((ipaddr2 = inet_ntoa(inet_addr(ipaddr2))) == -1) {
			    fprintf(stderr,
				    MSGSTR(BADADDRESS,
					   "%s is an invalid address.\n"),
				    ipaddr2);
			    exit(1);
			}

			rp += strspn(rp, "0123456789. \t");
			sprintf(recptr, "%s\t", ipaddr2);
			strcat(recptr, rp);
		}
		if (hflag && iflag) {
		        if ((ipaddr2 = inet_ntoa(inet_addr(ipaddr2))) == -1) {
			    fprintf(stderr,
				    MSGSTR(BADADDRESS,
					   "%s is an invalid address.\n"),
				    ipaddr2);
			    exit(1);
			}
			sprintf(recptr, "%s\t", ipaddr2);
			strcat(recptr, hostn);
		}
		exit(!ff_add(DBFILE, recptr));
	}


	if (sflag) {
		if (strspn(showarg, "0123456789.") == strlen(showarg)) {
			/* key on ip addr */
			if (Zflag)
				printf("#netaddr:hostname:alias:cmnt\n");
			if ( ipaddr_chk (showarg) ) 
				rc = ff_show(DBFILE, 1, ipaddr_db, colonizer);
			else
				rc = 0;
		} else {
			/* key on hostname */
			if (Zflag)
				printf("#netaddr:hostname:alias:cmnt\n");
			rc = ff_show(DBFILE, 0, showarg, colonizer);
		}
		if (rc == 0) {
			fprintf(stderr,	MSGSTR(NOTFOUND,
				       "%s: not found.\n"), showarg);
			exit(1);
		}
	}


	if (Sflag) {
		if (Zflag)
			printf("#netaddr:hostname:alias:cmnt\n");
		exit(!ff_show(DBFILE, 0, 0, colonizer)); /* show ALL records */
	}

	exit(0);
}

/*
 * ipaddr_chk
 *
 * Check existing ip address.
 * This route also put the ipaddr image retrieved from the data base into
 * ipaddr_db.
 *
 * RETURNS:  non-zero if record exists
 * 	     0 otherwise
 */
ipaddr_chk(ipp)
char *ipp;
{
	FILE *fp;
	char buf[BUFSIZ], *bp, *obp;
	u_long  val, val_db;
	int	rc = 0;
    
	/* open a file for reading */
	fp = ff_open(DBFILE, "r");

	if ((val = inet_addr(ipp)) == -1) {
	    /* new error message needed */
	    fprintf(stderr,
		    MSGSTR(BADADDRESS,
			   "%s is an invalid address.\n"),
		    ipp);
	    exit(1);
	}
	while (fgets(buf, BUFSIZ, fp) > 0) {
		bp = buf;	/* point to the beginning of line */
		SKIPDELIM(bp);	/* skip over the initial delim. */
		obp = bp;
		while (*bp && !DELIMITER(*bp)) { /* get ipaddress */
			*bp++;
		}
		*bp = '\0'; /* terminate the ipaddr string */
             	val_db = inet_addr(obp);
                if ( val == val_db ) {
                        rc++;
                        break;
                }
	}
	ff_close ( DBFILE, fp);
	if ( rc ) strcpy ( ipaddr_db, obp ); /* copy into the buffer */
	return (rc);
}
/*
 * colonize()
 *
 * puts a record from file into colon format.
 * the colon format divided into 4 fields -- IPaddr:hostname:aliases:comment
 *
 * RETURNS:  pointer to modified buffer.
 * 
 */

char *
colonize(bp)
    char *bp;			/* points to source in buffer */
{
	int   hostflg=0, i=0;	/* host name flag */
	char  largebuf[4096];

	SKIPDELIM(bp);
	while (*bp && !DELIMITER(*bp)) /* copy ipaddress */
		largebuf[i++] = *bp++;
	SKIPDELIM(bp);
	largebuf[i++] = ':';		/* put colon after address */

	while (*bp && *bp != '#') {
		while (*bp && *bp != '#' && !DELIMITER(*bp))
			largebuf[i++] = *bp++;
		SKIPDELIM(bp);
		if (!hostflg) {	/* separate host name and aliases */
			largebuf[i++] = ':';	/* put colon after hostname */
			hostflg++;
			continue;
		}
		if (*bp && *bp != '#')
			largebuf[i++] = ' ';	/* put space between aliases */
	}
	largebuf[i++] = ':';	/* put colon after alias(es) */
	if (*bp++ == '#') 
		while (*bp)
			largebuf[i++] = *bp++ ;	/* copy comment */
	strcat (largebuf, "\n");	/*  put terminator at the end */
	return(largebuf);
}


usage(a, b, c, d)
    char *a, *b, *c, *d;
{
	if (a)
		fprintf(stderr, a, b, c, d);
	fprintf(stderr,
		MSGSTR(USAGE1, "\nusage:	%s -a IP-addr -h \"hostname ...\"\n"), progname);
	fprintf(stderr,
		MSGSTR(USAGE2, "	%s -d IP-addr\n"), progname);
	fprintf(stderr,
		MSGSTR(USAGE3, "	%s -X\n"), progname);
	fprintf(stderr,
		MSGSTR(USAGE4, "	%s -c IP-addr [ -h \"hostname ...\" | -i new-IP-addr ]\n"),
		progname);
	fprintf(stderr,
		MSGSTR(USAGE5, "	%s -s IP-addr | hostname [-Z]\n"), progname);
	fprintf(stderr,
		MSGSTR(USAGE6, "	%s -S [-Z]\n"), progname);
	exit(2);
}


