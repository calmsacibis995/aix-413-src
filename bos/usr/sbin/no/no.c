static char sccsid[] = "@(#)75	1.11  src/bos/usr/sbin/no/no.c, cmdnet, bos411, 9428A410j 11/12/93 13:43:59";
/* 
 * COMPONENT_NAME: CMDNET Network commands.
 * 
 * FUNCTIONS: main, printallopts, printopt, setdefltopt, setopt, usage 
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
 * no allows the user to change kernel data structures relating to network
 * operation.
 */


#include <stdio.h>
#include <unistd.h>
#include <nlist.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <string.h>
#include <fcntl.h>
#include <net/netopt.h>


#include <nl_types.h>
#include "no_msg.h"
#define MSG_SET MS_NO
#include <locale.h>
extern *malloc();
nl_catd catd;
#define Msg(n,s) catgets(catd,MSG_SET,n,s)


/* New security stuff */
#include <sys/id.h>

char progname[128];		/* = argv[0] at run time */

extern char *optarg;
extern int optind;

#define  DELIM  '='		/* delimits option from value on command line */

int s;				/* dummy socket */


/*
 * NAME: main
 *
 * RETURNS: 
 *     0 upon normal completion.
 */
main(argc,argv)
	int argc;
	char **argv;
{
	int c;
	char *p, *cp;
	
	setlocale(LC_ALL, "");
	catd = catopen(MF_NO,NL_CAT_LOCALE);
	
	/* save program name, for error messages, etc. */
	strcpy(progname, (cp = strrchr(*argv, '/')) ? ++cp : *argv);
	
	if (argc < 2)  usage(0);
	
	/* open the dummy socket for ioctl's */
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr,
			Msg(BADSOCK,
	    "0821-055 no: Cannot create a socket. The error number is %d.\n"),
			errno);
		exit(1);
	}
	
	
	/* process command line args */
	while ((c = getopt(argc, argv, "ao:d:")) != EOF) {
		switch (c) {
		case 'a':
			printallopts();
			break;
		case 'o':
			if (p = strchr(optarg, DELIM)) {
				*p = '\0';
				setopt(optarg, p+1);
			} else
				printopt(optarg);
			break;
		case 'd':
			setdefltopt(optarg);
			break;
		case '?':
			usage(0);
		default:
			usage("unknown switch: %c\n", c);
		}
	}
	if (optind != argc)
		usage("some parameters were not parsed.\n");
	
	return(0);
}


/*
 * NAME: printopt
 *                                                                    
 * FUNCTION: prints value of specified option.
 *                                                                    
 * NOTES:
 *
 *
 * RETURNS:  nothing.
 */
printopt(option)
	char *option;
{
	struct optreq optreq;

	optreq.getnext = 0;
	strcpy(optreq.name, option);
	if (ioctl(s, SIOCGNETOPT, &optreq) == -1) {
		fprintf(stderr,	Msg(GETFAIL,
	    "0821-057 no: The ioctl SIOCGNETOPT system call failed.\n"));
		exit(1);
	}
	printf("%s = %s\n", optreq.name, optreq.data);
}


/*
 * NAME: printallopts
 *                                                                    
 * FUNCTION: prints value of all options.
 *                                                                    
 * NOTES:
 *
 *
 * RETURNS:  nothing.
 */
printallopts()
{
	struct optreq optreq;

	optreq.name[0] = '\0';
	optreq.getnext = 1;
	while (ioctl(s, SIOCGNETOPT, &optreq) != -1)
		printf("%25s = %s\n", optreq.name, optreq.data);
}

/*
 * NAME: setopt
 *                                                                    
 * FUNCTION: sets value of specified option to specified value.
 *                                                                    
 * NOTES:
 *
 *
 * RETURNS:  nothing.
 */
setopt(option, optval)
	char *option;
	char *optval;
{
	int i;
	struct optreq optreq;

        /* only superuser can change network options. */
        if (getuidx(ID_REAL))
		fprintf(stderr,
			Msg(NOTROOT,
	    "0821-058 no: Only the root user can set network options.\n"));
        else {
		strcpy(optreq.name, option);
		if (isnumber(optval)) {
			i = atoi(optval);
			bcopy(&i, optreq.data, sizeof(int));
		} else
			strcpy(optreq.data, optval);

		if (ioctl(s, SIOCSNETOPT, &optreq) == -1)
			fprintf(stderr,
				Msg(SETFAIL,
	    "0821-059 no: The ioctl SIOCSNETOPT system call failed.\n"));
	}
}


/*
 * NAME: setdefltopt
 *                                                                    
 * FUNCTION: sets value of specified option to default value.
 *                                                                    
 * NOTES:
 *
 *
 * RETURNS:  nothing.
 */
setdefltopt(option)
	char *option;
{
	struct optreq optreq;

        /* only superuser can change network options. */
        if (getuidx(ID_REAL))
		fprintf(stderr,
			Msg(NOTROOT,
	    "0821-058 no: Only the root user can set network options.\n"));
        else {
		strcpy(optreq.name, option);
		if (ioctl(s, SIOCDNETOPT, &optreq) == -1) {
			fprintf(stderr,
				Msg(DEFFAIL,
	    "0821-056 no: The ioctl SIOCGNETOPT system call failed.\n"));
		}
	}
}


/*
 * NAME: isnumber
 *                                                                    
 * FUNCTION: examines input string to determine if it is a number.
 *                                                                    
 * NOTES:
 *
 *
 * RETURNS:
 *     1  if input string is determined to be a number.
 *     0  if input string is NAN.
 */
isnumber(option)
	char *option;
{
	return( strspn(option, "-0123456789") );
}


/*
 * NAME: usage
 *                                                                    
 * FUNCTION: prints out the usage message when user make a mistake on 
 *           the command line.
 *                                                                    
 * RETURNS:  nothing, exits.
 */
usage(a, b, c, d)
	char *a, *b, *c, *d;
{
	if (a)  fprintf(stderr, a, b, c, d);
	fprintf(stderr,
		Msg(USAGE1,
		    "\nusage:\t%1$s -o option[%2$cnewvalue] [ -o ... ]\n"),
		progname, DELIM);
	fprintf(stderr, Msg(USAGE2, "\t%s -d option\n"), progname);
	fprintf(stderr, Msg(USAGE3, "\t%s -a\n"), progname);
	exit(2);
}
