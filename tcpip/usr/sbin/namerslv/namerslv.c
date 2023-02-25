static char sccsid[] = "@(#)34  1.11  src/tcpip/usr/sbin/namerslv/namerslv.c, tcpnaming, tcpip411, 9434B411a 8/24/94 16:47:29";
/*
 * COMPONENT_NAME:  TCPIP
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989.
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* namerslv.c */

/*
 * namerslv manipulates entries in the /etc/resolv.conf file.  it allows
 * the user to add, delete, and show the nameserver and domain entries in
 * the file.
 */

/* todo - does domain record have to be first? */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "libffdb.h"

#define  DBFILE  "/etc/resolv.conf" /* name of the database file */
#define  DEFFILE "/etc/resolv.conf.sv" /* name of the default save database file */

char progname[128];		/* = argv[0] at run time */

extern char *optarg;
extern int optind;

#include <nl_types.h>
#include "namerslv_msg.h"
#include <locale.h>
nl_catd catd;
extern *malloc();
#define Msg(n,s) catgets(catd,MS_NAMERSLV,n,s)

extern char *colonize();	/* forward declaration */
extern void ff_rename_by_copying();

main(argc,argv)
    int argc;
    char **argv;
{
    char *ipaddr = 0, *domain = 0, recptr[MAXRECLEN], *rp;
    char *ff_delete(), *filename = 0, *(*colonizer)() = 0;
    int c, rc;
    int aflag = 0, bflag = 0, cflag = 0, dflag = 0, Dflag = 0;
    int iflag = 0, eflag = 0, sflag = 0, Xflag = 0, Zflag = 0;
    int Bflag = 0, Iflag = 0, nflag = 0, Eflag = 0;

    setlocale(LC_ALL, "");
    catd = catopen(MF_NAMERSLV,NL_CAT_LOCALE);

    /* save program name, for error messages, etc. */
    strcpy(progname, *argv);

    if (argc < 2)  usage(0);

    /* process command line args */
    while ((c = getopt(argc, argv, "abB:c:dD:eE:i:IsZnX")) != EOF) {
	switch (c) {
	case 'a':
	    if (bflag || cflag || dflag || eflag || sflag || Xflag || Zflag ||
		Eflag || Bflag || Iflag || nflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    aflag++;
	    break;
	case 'b':
	    if (aflag || cflag || dflag || eflag || sflag || Xflag || Zflag ||
		Eflag || Bflag || Iflag || nflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    bflag++;
	    break;
	case 'B':
	    if (aflag || cflag || dflag || iflag || eflag || sflag || bflag ||
		Dflag || Xflag || Zflag || Eflag || Iflag || nflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    Bflag++;
	    filename = optarg;
	    break;
	case 'c':
	    if (aflag || bflag || dflag || iflag || eflag || sflag || nflag ||
		Dflag || Xflag || Zflag || Bflag || Eflag || Iflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    domain = optarg;
	    cflag++;
	    break;
	case 'd':
	    if (aflag || bflag || cflag || eflag || sflag || Xflag ||
		Zflag || Eflag || Bflag || Iflag || Dflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    dflag++;
	    break;
	case 'D':
	    if (eflag || Xflag || Eflag || Bflag || sflag || cflag ||
		dflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    domain = optarg;
	    Dflag++;
	    break;
	case 'e':
	    if (aflag || bflag || cflag || dflag || Dflag || iflag ||
		sflag || Xflag || Zflag || Iflag || nflag || Eflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    eflag++;
	    break;
	case 'i':
	    if (cflag || Dflag || eflag || Xflag || Iflag || Xflag ||
		sflag || Zflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    ipaddr = optarg;
	    iflag++;
	    break;
	case 'I':
	    if (aflag || cflag || Dflag || eflag || iflag ||
		dflag || bflag || Bflag || Eflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    Iflag++;
	    break;
	case 'n':
	    if (aflag || cflag || Dflag || eflag || Xflag || bflag ||
		Bflag || Eflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    nflag++;
	    break;
	case 's':
	    if (aflag || cflag || Dflag || eflag || Xflag || iflag ||
		dflag || bflag || Bflag || Eflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    sflag++;
	    break;
	case 'Z':
	    if (aflag || cflag || Dflag || eflag || Xflag || iflag ||
		dflag || bflag || Bflag || Eflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    Zflag++;
	    colonizer = colonize;
	    break;
	case 'E':
	    if (aflag || cflag || Dflag || eflag || Xflag || iflag ||
		dflag || bflag || Bflag || sflag || Iflag || Zflag ||
		nflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    Eflag++;
	    filename = optarg;
	    break;
	case 'X':
	    if (aflag || cflag || Dflag || eflag || Eflag || nflag ||
		dflag || bflag || Bflag || sflag || Zflag)
		usage(Msg(INCOMP_SW, "incompatible command line switches\n"));
	    Xflag++;
	    break;
	case '?':
	    usage(0);
	default:
	    usage(Msg(UNK_SWITCH, "unknown switch: %c\n"), c);
	}
    }
    if (optind != argc)
	usage(Msg(NOT_PARSED, "some parameters were not parsed.\n"));
    if ((iflag && !(aflag || dflag || bflag)) ||
	(Dflag && !(aflag || bflag)) ||
	(nflag && !(dflag || sflag)) ||
	(Zflag && !sflag))
	usage(Msg(MISSINGFLG, "you forgot some flag(s)\n"));
    
    if (ipaddr)
	    if (well_formed_ipaddr(ipaddr) == FALSE) {
		    fprintf(stderr, Msg(NUM_IPADDR,
		   "IP-addr \"%s\" not well-formed.\n"),
			    ipaddr);
		    exit(1);
	    }
    
    if (aflag) {
	c = 0;
	if (iflag) {
	    if (!ff_exist_key(DBFILE, ipaddr, 2)) {
	    	sprintf(recptr, "nameserver	%s", ipaddr);
	    	c = ff_add(DBFILE, recptr);
	    } else {
		fprintf(stderr,
			Msg(NAMEXIST, "%s: a nameserver already exists:  "),
			progname);
	        ff_show(DBFILE, 2, ipaddr, colonizer);
		exit(1);
	    }
	}
	if (Dflag) {
	    if (!ff_exist_key(DBFILE, "domain", 1)) {
		sprintf(recptr, "domain	%s", domain);
		c = ff_add(DBFILE, recptr) || c;
	    } else {
		fprintf(stderr,
			Msg(DOMEXIST, "%s: a domain already exists:  "),
			progname);
	        ff_show(DBFILE, 1, "domain", colonizer);
		exit(1);
	    }
	}
	if (!iflag && !Dflag) {
	    fprintf(stderr,
		    Msg(SPEC_SW, "%s: you must also specify either -i or -D.\n"),
		    progname);
	    exit(2);
	}
	exit(!c);
    }
    
    if (Xflag)
	if (Iflag) {		/* delete all nameserver entries */
	    rp = ff_delete(DBFILE, 1, "nameserver", 2, 0);
	    exit((rp == 0) ? 1 : 0);
	} else {		/* delete all records */
	    rp = ff_delete(DBFILE, 0, 0, 0, 0);
	    exit((rp == 0) ? 1 : 0);
	}

    if (dflag) {
	rp = 0;
	if (iflag) {
	    rp = ff_delete(DBFILE, 2, ipaddr, 0, 0);
	}
	if (nflag) {
	    rp = ff_delete(DBFILE, 1, "domain", 0, 0) || rp;
	}
        if (!iflag && !nflag) {
	    fprintf(stderr,
		    Msg(SPEC_SW2, "%s: you must also specify either -i or -n.\n"),
		    progname);
	    exit(2);
	}
	exit((rp)?0:1);
    }

    if (cflag) {
	/* delete any existing domain */
	ff_delete(DBFILE, 1, "domain", 0, 0);
	sprintf(recptr, "domain	%s", domain);
	exit(!ff_add(DBFILE, recptr));
    }

    if (bflag) {
	if (Dflag && !iflag)
	    usage(Msg(SPECNS, "You must specify a nameserver address.\n"));
	if (!iflag) {
	    ff_rename_by_copying(DEFFILE, DBFILE);
	} else {			/* iflag is true */
	    /* remove old file if it exists. */
	    unlink(DBFILE);
	    sprintf(recptr, "nameserver	%s", ipaddr);
	    ff_add(DBFILE, recptr);
	    if (Dflag) {
		if (!ff_exist_key(DBFILE, "domain", 1)) {
		    sprintf(recptr, "domain	%s", domain);
		    ff_add(DBFILE, recptr);
		} else {
		    fprintf(stderr, Msg(DOMEXIST,
					"%s: a domain already exists:  "),
			    progname);
		    ff_show(DBFILE, 1, "domain", colonizer);
		    exit(1);
		}
	    }
	}
    }

    if (Bflag)
	ff_rename_by_copying(filename, DBFILE);
	
    if (eflag)
	ff_rename_by_copying(DBFILE, DEFFILE);

    if (Eflag)
	ff_rename_by_copying(DBFILE, filename);

    if (sflag)
	if (Iflag) {
		if (Zflag)
			printf("#nameserver\n");
		ff_show(DBFILE, 1, "nameserver", colonizer);
	        exit(0);
	} else if (nflag) {
		if (Zflag)
			printf("#domain\n");
		ff_show(DBFILE, 1, "domain", colonizer);
	        exit(0);
	} else {
		if (Zflag)
			printf("#all\n");
		ff_show(DBFILE, 1, 0, colonizer);
	        exit(0);
	}

    exit(0);
}

/*
 * colonize()
 *
 * puts a record from file into colon format.  for nameresolv,
 * this really just returns a list of domain/nameserver entries.
 * since there is only one field returned, there is no need for
 * colons.
 *
 * RETURNS:  pointer to modified buffer.
 * 
 */

char *
colonize(bp)
    char *bp;			/* points to source in buffer */
{
    char *orig = bp, *np = bp;	/* points to destination in buffer */

    SKIPDELIM(bp);
    SKIPTOKEN(bp);		/* skip past 'domain' or 'nameserver' */
    SKIPDELIM(bp);

    while (*bp && *bp != '#' && !DELIMITER(*bp))
	*np++ = *bp++;

    *np++ = '\n';		/* terminate record */
    *np = '\0';

    return(orig);
}


/* finite state recognizer for well formed dotted decimal IP addresses */
well_formed_ipaddr(cp)
	char *cp;
{
	int state;

	state = 1;
	for (; *cp; cp++) {
		switch(state) {
		case 1:	state = (isdigit(*cp)) ? 2 : 0;
			break;
		case 2: state = (*cp == '.') ? 3 : ((isdigit(*cp)) ? 2 : 0);
			break;
		case 3:	state = (isdigit(*cp)) ? 2 : 0;
			break;
		default:
			return(0);
		}
	}
       return( (state==2) ? 1 : 0 );
}


usage(a, b, c, d)
    char *a, *b, *c, *d;
{
    if (a)
	fprintf(stderr, a, b, c, d);
    fprintf(stderr,
	    Msg(USAGE1, "\nusage:	%s -a -i IP-addr | -D domainname\n"),
	    progname);
    fprintf(stderr,
	    Msg(USAGE2, "	%s -d -i IP-addr | -n\n"), progname);
    fprintf(stderr,
	    Msg(USAGE3, "	%s -X [-I]\n"), progname);
    fprintf(stderr,
	    Msg(USAGE4, "	%s -c domainname\n"), progname);
    fprintf(stderr,
	    Msg(USAGE5, "	%s -s [ -I | -n ] [-Z]\n"), progname);
    fprintf(stderr,
	    Msg(USAGE6, "	%s -e\n"), progname);
    fprintf(stderr,
	    Msg(USAGE7, "	%s -E filename\n"), progname);
    fprintf(stderr,
	    Msg(USAGE8, "	%s -b [-i IP-addr [-D domainname]]\n"),
	    progname);
    fprintf(stderr,
	    Msg(USAGE9, "	%s -B filename\n"),
	    progname);
    exit(2);
}
