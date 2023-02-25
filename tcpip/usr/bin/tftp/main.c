static char sccsid[] = "@(#)26        1.15  src/tcpip/usr/bin/tftp/main.c, tcpfilexfr, tcpip411, GOLD410 3/10/94 18:34:32";
/* 
 * COMPONENT_NAME: TCPIP main.c
 * 
 * FUNCTIONS: MSGSTR, Mmain, command, get, getcmd, getusage, help, 
 *            if, intr,  makeargv, modecmd, put, putusage, 
 *            quit, setascii, setbinary, setjmp, setmode, setpeer, 
 *            settimeout, settrace, setverbose, sizeof, 
 *            status, tail 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "main.c	5.5 (Berkeley) 2/7/86";
#endif
*/
/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

/*
 * TFTP User Program -- Command Interface.
 */

/*
 * The following changes have been made according to RFC 1123:
 *
 * Packets that have a block number for a previous transmission
 * are assumed to have been delayed and are silently ignored.
 * This solves the Sorcerer's Apprentice Syndrome which resulted
 * from transmitting a duplicate packet when a duplicate ACK
 * was received.
 *
 * Adaptive timeout has been implemented with a technique similar
 * to that used by TCP.  When a packet that was not retransmitted
 * is acknowledged, the round trip time is computed and used to 
 * update a smoothed round trip time (srtt = rtt/8 + srtt * 7/8) 
 * and a smoothed round trip time difference (srttd = srttd * 3/4 + 
 * |delta|/4, where delta = rtt - srtt).  When a packet that was
 * retransmitted is acknowledged, the smoothed times are not
 * updated because it is not known which packet is being 
 * acknowledged and thus the round trip time is uncertain.
 * When a packet is transmitted for the first time, the timeout
 * is calculated from the smoothed round trip time and the smoothed
 * round trip time difference (timeout = srtt + 2 * srttd).  When a
 * packet is retransmitted, the timeout is doubled (limited by the
 * maximum timeout).  When a retransmitted packet is acknowledged,
 * the round trip time is used as the best guess for the timeout
 * for the next packet.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in.h>

#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <setjmp.h>
#include <ctype.h>
#include <netdb.h>

#include "tftp_msg.h" 
#include <nl_types.h>
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TFTP,n,s) 


struct	sockaddr_in sin;
int	f;
short   port;
int	trace;
int	verbose;
int	connected;
char	mode[32];
char	line[200];
int	margc;
char	*margv[20];
char	*prompt = "tftp";
jmp_buf	toplevel;
int	intr();
struct	servent *sp;

int	quit(), help(), setverbose(), settrace(), status();
int     get(), put(), setpeer(), modecmd(), settimeout();
int     setbinary(), setascii();

#define HELPINDENT (sizeof("connect"))

struct cmd {
	char	*name;
	int	help_id;
	char	*help;
	int	(*handler)();
};

char	vhelp[] = "toggle verbose mode";
char	thelp[] = "toggle packet tracing";
char	chelp[] = "connect to remote tftp";
char	qhelp[] = "exit tftp";
char	hhelp[] = "print help information";
char	shelp[] = "send file";
char	rhelp[] = "receive file";
char	mhelp[] = "set file transfer mode";
char	sthelp[] = "show current status";
char	ihelp[] = "set total retransmission timeout";
char    ashelp[] = "set mode to netascii";
char    bnhelp[] = "set mode to octet";

struct cmd cmdtab[] = {
	{ "connect",
	HELP_CONN,		chelp,          setpeer },
	{ "mode",
	HELP_FILE_TRANS,	mhelp,          modecmd },
	{ "put",	
	HELP_SEND_FILE,		shelp,          put },
	{ "get",	
	HELP_REC_FILE,		rhelp,          get },
	{ "quit",	
	HELP_EXIT, 		qhelp,          quit },
	{ "verbose",	
	HELP_T_VERBOSE,		vhelp,          setverbose },
	{ "trace",	
	HELP_TRACE,		thelp,          settrace },
	{ "status",	
	HELP_SHOW_STAT,		sthelp,         status },
	{ "binary", 
	HELP_SET_OCTECT,	bnhelp,         setbinary },
	{ "ascii", 
	HELP_SET_ASCII,		ashelp,         setascii },
	{ "timeout",	
	HELP_SET_TIME,		ihelp,          settimeout },
	{ "?",	
	HELP_PRINT,		hhelp,          help },
	0
};

struct	cmd *getcmd();
char	*tail();
char	*index();
char	*rindex();
#include <locale.h>

main(argc, argv)
	int  argc;
	char *argv[];
{
	struct sockaddr_in sin;
	int top;

	setlocale(LC_ALL,"");
	catd = catopen(MF_TFTP,NL_CAT_LOCALE);
	err_load();


	sp = getservbyname("tftp", "udp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKNOWN_SERVICE, "tftp: udp/tftp: unknown service\n")); /*MSG*/
		exit(1);
	}
	f = socket(AF_INET, SOCK_DGRAM, 0);
	if (f < 0) {
		perror("tftp: socket");
		exit(3);
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	if (bind(f, &sin, sizeof (sin)) < 0) {
		perror("tftp: bind");
		exit(1);
	}

	/* Since we're running setuid root and we've finished what
	 * root needs to do, we'll return to real user id.
 	 */
	if (setuid(getuid())) {
		perror("tftp: setuid");
		exit(1);
	}

	strcpy(mode, "netascii");
	signal(SIGINT, intr);
	if (argc > 1) {
		if (setjmp(toplevel) != 0) {
			exit(1);
		}
#ifdef AIX
		/*
		 * if arguments, and first is a - flag, then is funky interface
		 */
		if( *argv[1] == '-' ) {
			/* setup port number to default tftp port */
			port = sp->s_port;
			exit( aix_tftp( argc, argv ) );
		}
#endif AIX
		setpeer(argc, argv);
	}
	top = setjmp(toplevel) == 0;
	for (;;)
		command(top);

}

char    hostname[100];

setpeer(argc, argv)
	int argc;
	char *argv[];
{
	struct hostent *host;


	if (argc < 2) {
		strcpy(line, "Connect ");
		printf(MSGSTR(CONN_TO, "(to) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc > 3) {
		printf(MSGSTR(USAGE, "usage: %s host-name [port]\n"), argv[0]); /*MSG*/
		return;
	}

	/* see if it's a name or an address */
        if (!isinet_addr(argv[1])) {
                if ((host = gethostbyname (argv[1])) == (struct hostent *) 0) {
                        printf(MSGSTR(NME_NT_FND,
                                         "tftp: Unknown host %s\n"),argv[1]);
                        return;
                }
		sin.sin_family = host->h_addrtype;
                bcopy(host->h_addr, &sin.sin_addr, host->h_length);
                strcpy(hostname, host->h_name);
        }
        else {
		sin.sin_family = AF_INET;
                sin.sin_addr.s_addr = inet_addr(argv[1]);
                if (sin.sin_addr.s_addr == -1) {
		     connected = 0;
                     printf(MSGSTR(UNKNOWN_HOST, 
				"tftp: Address %s misformed\n"), argv[1]);
                     return;
               	  }
		strcpy(hostname, argv[1]);
	}
	port = sp->s_port;
	if (argc == 3) {
		port = atoi(argv[2]);
		if (port < 0) {
			printf(MSGSTR(BAD_PORT_NO, "%s: bad port number\n"), argv[2]); /*MSG*/
			connected = 0;
			return;
		}
		port = htons(port);
	}
	connected = 1;
}

struct	modes {
	char *m_name;
	char *m_mode;
} modes[] = {
	{ "ascii",	"netascii" },
	{ "netascii",   "netascii" },
	{ "binary",     "octet" },
	{ "image",      "octet" },
	{ "octet",     "octet" },
/*      { "mail",       "mail" },       */
	{ 0,		0 }
};

modecmd(argc, argv)
	char *argv[];
{
	register struct modes *p;
	char *sep;

	if (argc < 2) {
		printf(MSGSTR(USING_MODE, "Using %s mode to transfer files.\n"), mode); /*MSG*/
		return;
	}
	if (argc == 2) {
		for (p = modes; p->m_name; p++)
			if (strcmp(argv[1], p->m_name) == 0)
				break;
		if (p->m_name) {
			setmode(p->m_mode);
			return;
		}
		printf(MSGSTR(UNKNOWN_MODE, "%s: unknown mode\n"), argv[1]); /*MSG*/
		/* drop through and print usage message */
	}

	printf(MSGSTR(USAGE_2, "usage: %s ["), argv[0]); /*MSG*/
	sep = " ";
	for (p = modes; p->m_name; p++) {
		printf("%s%s", sep, p->m_name);
		if (*sep == ' ')
			sep = " | ";
	}
	printf(" ]\n");
	return;
}

setbinary(argc, argv)
char *argv[];
{       setmode("octet");
}

setascii(argc, argv)
char *argv[];
{       setmode("netascii");
}

setmode(newmode)
char *newmode;
{
	strcpy(mode, newmode);
	if (verbose)
		printf(MSGSTR(MODE_SET, "mode set to %s\n"), mode); /*MSG*/
}


/*
 * Send file(s).
 */
put(argc, argv)
	char *argv[];
{
	int fd;
	register int n;
	register char *cp, *targ;


	if (argc < 2) {
		strcpy(line, "send ");
		printf(MSGSTR(PUT_FILE, "(file) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		putusage(argv[0]);
		return;
	}
	targ = argv[argc - 1];
	if (index(argv[argc - 1], ':')) {
		char *cp;
		struct hostent *hp;

		for (n = 1; n < argc - 1; n++)
			if (index(argv[n], ':')) {
				putusage(argv[0]);
				return;
			}
		cp = argv[argc - 1];
		targ = index(cp, ':');
		*targ++ = 0;

		/* see if it's a name or an address */
        	if (!isinet_addr(cp)) {
                   if ((hp = gethostbyname (cp)) == (struct hostent *) 0) {
			printf(MSGSTR(NME_NT_FND,
                                         "tftp: Unknown host %s\n"),cp);
			herror((char *)NULL);
                        return;
                	}
                	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
                	sin.sin_family = hp->h_addrtype;
			port = sp->s_port;
			connected = 1;
                	strcpy(hostname, hp->h_name);
        	}
        	else {
		    sin.sin_addr.s_addr = inet_addr(cp);
		    if (sin.sin_addr.s_addr == -1) {
                       connected = 0;
                       printf(MSGSTR(UNKNOWN_HOST,
				"tftp: Address %s misformed\n"), cp);
                       return;
                       }
	        }
	}
	if (!connected) {
		printf(MSGSTR(NO_TARGET, "No target machine specified.\n")); /*MSG*/
		return;
	}
	if (argc < 4) {
		cp = argc == 2 ? tail(targ) : argv[1];
		fd = open(cp, O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(cp); /*MSG*/
			return;
		}
		if (verbose)
			printf(MSGSTR(PUTTING, "putting %s to %s:%s [%s]\n"), /*MSG*/
				cp, hostname, targ, mode);
		sin.sin_port = port;
		sendfile(fd, targ, mode);
		return;
	}
				/* this assumes the target is a directory */
				/* on a remote unix system.  hmmmm.  */
	cp = index(targ, '\0'); 
	*cp++ = '/';
	for (n = 1; n < argc - 1; n++) {
		strcpy(cp, tail(argv[n]));
		fd = open(argv[n], O_RDONLY);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(argv[n]); /*MSG*/
			continue;
		}
		if (verbose)
			printf(MSGSTR(PUTTING, "putting %s to %s:%s [%s]\n"), /*MSG*/
				argv[n], hostname, targ, mode);
		sin.sin_port = port;
		sendfile(fd, targ, mode);
	}
}

putusage(s)
	char *s;
{
	printf(MSGSTR(USAGE3, "usage: %s file ... host:target, or\n"), s); /*MSG*/
	printf(MSGSTR(USAGE_4, "       %s file ... target (when already connected)\n"), s); /*MSG*/
}

/*
 * Receive file(s).
 */
get(argc, argv)
	char *argv[];
{
	int fd;
	register int n;
	register char *cp;
	char *src;


	if (argc < 2) {
		strcpy(line, "get ");
		printf(MSGSTR(GET_FILES, "(files) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc < 2) {
		getusage(argv[0]);
		return;
	}
	if (!connected) {
		for (n = 1; n < argc ; n++)
			if (index(argv[n], ':') == 0) {
				getusage(argv[0]);
				return;
			}
	}
	for (n = 1; n < argc ; n++) {
		src = index(argv[n], ':');
		if (src == NULL)
			src = argv[n];
		else {
			struct hostent *hp;

			*src++ = 0;

			/* see if it's a name or an address */
                       if (!isinet_addr(argv[n])) {
                          if ((hp = gethostbyname(argv[n])) == 
					(struct hostent *) 0) {
                       		 printf(MSGSTR(NME_NT_FND,
                                         "tftp: Unknown host %s\n"),argv[n]);
                        	 herror((char *)NULL);
                        	 continue;
                        	}
			bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
			sin.sin_family = hp->h_addrtype;
			port = sp->s_port;
			connected = 1;
			strcpy(hostname, hp->h_name);
			}
			else {
                    	   sin.sin_addr.s_addr = inet_addr(argv[n]);
                           if (sin.sin_addr.s_addr == -1) {
                       		connected = 0;
                       		printf(MSGSTR(UNKNOWN_HOST,
				     "tftp: Address %s misformed\n"),argv[n]);
                       		return;
                       		}
                	}
		}
		if (argc < 4) {
			cp = argc == 3 ? argv[2] : tail(src);
			fd = creat(cp, 0644);
			if (fd < 0) {
				fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(cp); /*MSG*/
				return;
			}
			if (verbose)
				printf(MSGSTR(GET_GETTING, "getting from %s:%s to %s [%s]\n"), /*MSG*/
					hostname, src, cp, mode);
			sin.sin_port = port;
			recvfile(fd, src, mode, cp);
			break;
		}
		cp = tail(src);         /* new .. jdg */
		fd = creat(cp, 0644);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(TFTP, "tftp: ")); perror(cp); /*MSG*/
			continue;
		}
		if (verbose)
			printf(MSGSTR(GET_GETTING, "getting from %s:%s to %s [%s]\n"), /*MSG*/
				hostname, src, cp, mode);
		sin.sin_port = port;
		recvfile(fd, src, mode, cp);
	}
}

getusage(s)
char * s;
{
	printf(MSGSTR(USAGE_5, "usage: %s host:file host:file ... file, or\n"), s); /*MSG*/
	printf(MSGSTR(GET_USAGE, "       %s file file ... file if connected\n"), s); /*MSG*/
}

int	rexmtval;

int	maxtimeout = 25;

settimeout(argc, argv)
	char *argv[];
{
	int t;

	if (argc < 2) {
		strcpy(line, "Maximum-timeout ");
		printf(MSGSTR(SETREXMT_VALUE, "(value) ")); /*MSG*/
		gets(&line[strlen(line)]);
		makeargv();
		argc = margc;
		argv = margv;
	}
	if (argc != 2) {
		printf(MSGSTR(USAGE_6, "usage: %s value\n"), argv[0]); /*MSG*/
		return;
	}
	t = atoi(argv[1]);
	if (t < 0)
		printf(MSGSTR(BAD_VAL, "%s: bad value\n"), argv[1]); /*MSG*/
	else
		maxtimeout = t;
}

char *
onoff(bool)
        int bool;
{
        return (bool ? MSGSTR(ONOFF_ON, "on") : MSGSTR(ONOFF_OFF, "off"));
}

status(argc, argv)
	char *argv[];
{
	if (connected)
		printf(MSGSTR(CONNECTED_TO, "Connected to %s.\n"), hostname); /*MSG*/
	else 
		printf(MSGSTR(NOT_CONNECTED, "Not connected.\n")); /*MSG*/
	printf(MSGSTR(MODE_V_T, "Mode: %s Verbose: %s Tracing: %s\n"), mode,
		onoff(verbose), onoff(trace));
	printf(MSGSTR(MAX_TIMEOUT, "Max-timeout: %d seconds\n"), /*MSG*/
		maxtimeout);
	
}

intr()
{
	signal(SIGALRM, SIG_IGN);
	alarm(0);
	longjmp(toplevel, -1);
}

char *
tail(filename)
	char *filename;
{
	register char *s;
	
	while (*filename) {
		s = rindex(filename, '/');
		if (s == NULL)
			break;
		if (s[1])
			return (s + 1);
		*s = '\0';
	}
	return (filename);
}

/*
 * Command parser.
 */
command(top)
	int top;
{
	register struct cmd *c;

	if (!top)
		putchar('\n');
	for (;;) {
		printf("%s> ", prompt);
		if (gets(line) == 0) {
			if (feof(stdin)) {
				quit();
			} else {
				continue;
			}
		}
		if (line[0] == 0)
			continue;
		makeargv();
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf(MSGSTR(AMBIGUOUS, "?Ambiguous command\n")); /*MSG*/
			continue;
		}
		if (c == 0) {
			printf(MSGSTR(INVALID_CMD, "?Invalid command\n")); /*MSG*/
			continue;
		}
		(*c->handler)(margc, margv);
	}
}

struct cmd *
getcmd(name)
	register char *name;
{
	register char *p, *q;
	register struct cmd *c, *found;
	register int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; p = c->name; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */
makeargv()
{
	register char *cp;
	register char **argp = margv;

	margc = 0;
	for (cp = line; *cp;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*argp++ = cp;
		margc += 1;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}
	*argp++ = 0;
}

/*VARARGS*/
quit()
{
	exit(0);
}

/*
 * Help command.
 */
help(argc, argv)
	int argc;
	char *argv[];
{
	register struct cmd *c;

	if (argc == 1) {
		printf(MSGSTR(HELP_TABLE, "Commands may be abbreviated.  Commands are:\n\n")); /*MSG*/
		for (c = cmdtab; c->name; c++)
			printf(MSGSTR(c->help_id, "%-*s\t%s\n"), HELPINDENT,
				c->name, c->help); /*MSG*/
			printf("%-*s\t%s\n", HELPINDENT, c->name, c->help);
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf(MSGSTR(AMB_HELP, "?Ambiguous help command %s\n"), arg); /*MSG*/
		else if (c == (struct cmd *)0)
			printf(MSGSTR(INVALID_HELP, "?Invalid help command %s\n"), arg); /*MSG*/
		else
			printf("%s\n", c->help);
	}
}

/*VARARGS*/
settrace()
{
	trace = !trace;
	if (trace)
		printf(MSGSTR(PACKET_TRACE_ON, "Packet tracing on.\n"));
	else
		printf(MSGSTR(PACKET_TRACE_OFF, "Packet tracing off.\n"));
}

/*VARARGS*/
setverbose()
{
	verbose = !verbose;
	if (verbose)
		printf(MSGSTR(VERBOSE_MODE_ON, "Verbose mode on.\n"));
	else
		printf(MSGSTR(VERBOSE_MODE_OFF, "Verbose mode off.\n"));
}
