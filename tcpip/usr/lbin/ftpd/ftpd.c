static char sccsid[] = "@(#)15  1.43.1.20  src/tcpip/usr/lbin/ftpd/ftpd.c, tcpfilexfr, tcpip41J, 9524E_all 6/14/95 16:17:05";
/* 
 * COMPONENT_NAME: TCPIP ftpd.c
 * 
 * FUNCTIONS: MSGSTR, Mftpd, ack, cwd, dataconn, delete, dolog, 
 *            dologout, end_login, fatal, getdatasock, gunique, 
 *            lostconn, lreply, makedir, myoob, nack, pass, passive, 
 *            perror_reply, pwd, receive_data, removedir, renamecmd, 
 *            renamefrom, reply, retrieve, send_data, send_file_list, 
 *            setproctitle, sgetpwnam, sgetsave, statcmd, statfilecmd, 
 *            store, trace_handler, user, yyerror, gethdir 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1985, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "ftpd.c	5.27.1.1	(Berkeley) 3/2/89";
#endif  not lint */
/* 
 *
/*
 * FTP server.
 */

/*
 * The following commands have been implemented or enabled according
 * to RFC 1123 (some original options are also shown):
 * 	REIN		(Reinitialize - USER command expected next)
 *	TYPE 	A N	(Ascii type, Non-printing format)
 *		A T	(Ascii type, Telnet - same as TYPE N)
 *		A C	(Ascii type, Carriage-control format)
 *	 	E N	(Ebcdic type, Non-printing format)
 *		E T	(Ebcdic type, Telnet - same as TYPE N)
 *		E C	(Ebcdic type, Carriage-control format)
 *		I	(Image type)
 *		L m	(Logical byte size m)
 *	STRU	F	(File structure)
 *		R	(Record structure)
 *	MODE	B	(Block mode)
 *		S	(Stream mode)
 *	SITE	HELP	(Help for the site command)
 *		UMASK	(Display or set file creation umask value)
 *		CHMOD	(chmod on a file)
 *		IDLE	(Display or set the idle timeout value)
 *	REST	m	(Restart the subsequent transfer with state m)
 *	XMKD		(Same as MKD)
 *	XRMD		(Same as RMD)
 *	XPWD		(Same as PWD)
 *	XCUP		(Same as CDUP)
 *	XCWD		(Same as CWD)
 *
 * Note that the SIZE command is not compatible with the implementation
 * of the REST command.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/dir.h>
#ifdef _CSECURITY
/* Security New Stuff */
#include <sys/id.h>
#include <sys/priv.h>
#endif _CSECURITY

#include <netinet/in.h>

#define	FTP_NAMES
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <setjmp.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include <sys/syslog.h>
#include <varargs.h>
#include <paths.h>
#include <glob.h>

#include <usersec.h>
#include <userpw.h>
#include <userconf.h>

#ifdef _CSECURITY
#include "tcpip_audit.h"
#endif _CSECURITY

#include <nl_types.h>
#include "ftpd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_FTPD,n,s) 

/*
 * File containing login names
 * NOT to be used on this machine.
 * Commonly used to disallow uucp.
 */
#define	FTPUSERS	"/etc/ftpusers"

extern	int errno;
extern	char *sys_errlist[];
extern	int sys_nerr;
extern	char *crypt();
extern	char version[];
char *home;		/* pointer to home directory for glob */
extern	FILE *ftpd_popen();
extern	int  ftpd_pclose();
extern	char *getline();
extern	char cbuf[];

struct	sockaddr_in ctrl_addr;
struct	sockaddr_in data_source;
struct	sockaddr_in data_dest;
struct	sockaddr_in his_addr;
struct	sockaddr_in pasv_addr;
#ifdef _CSECURITY
ulong   remote_addr;
char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
uid_t	saved_uid, effective_uid;
priv_t priv;

/* Restore privs and set the invoker back to uid saved_uid */
#define GET_PRIV(a)		\
        getpriv(PRIV_MAXIMUM, &priv,sizeof(priv_t)); \
        setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t)); \
        setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, (a));

/* Drop privs and set the invoker to uid 100 */
#define DROP_PRIV(a) 		\
        priv.pv_priv[0] = 0; 	\
        priv.pv_priv[1] = 0; 	\
        setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t)); \
        setuidx(ID_EFFECTIVE|ID_REAL|ID_SAVED, (a));

#endif _CSECURITY

int	tracing = 0;
int	data;
jmp_buf	errcatch, urgcatch;
int	logged_in;
struct	passwd *pw;
int	debug;
int	timeout = 900;    /* timeout after 15 minutes of inactivity */
int	maxtimeout = 7200;/* don't allow idle time to be set beyond 2 hours */
int	logging;
int	guest;
int	type;			/* transfer type */
int	bytesz;			/* logical byte size */
int	form;			/* transfer format */
int	stru;			/* transfer structure (avoid C keyword) */
int	mode;			/* transfer mode */
int	usedefault = 1;		/* for data transfers */
int	pdata = -1;		/* for passive mode */
int	transflag;
off_t	file_size;
off_t	byte_count;		/* transfer byte count */
off_t	rest_count;		/* byte count needed for next restart marker */
off_t	rest_mark;		/* seek point to restart transfer */
        int line_state;		/* current state for data formatting */
#define START_OF_FILE 1
#define START_OF_LINE 2
#define IN_LINE 3
#define DID_CR 4
#define DID_ESC 5
        int car_ctl_char;	/* carriage control char to use next */
        int next_bits;		/* no. of bits to the next logical byte */
        int useful_bits;	/* no. of bits to use from the current char */
        int space_bits;		/* no. of available bits in output char */
        int avail_bits;		/* no. of available bits in input char */
        int out_bits;		/* no. of bits used in output char */
        int total_bits;		/* out_bits + useful_bits */
        int out_char;		/* the output char */
#if !defined(CMASK) || CMASK == 0
#undef CMASK
#define CMASK 027
#endif
int	defumask = CMASK;		/* default umask value */
char	tmpline[7];
char	hostname[MAXHOSTNAMELEN];
char	remotehost[MAXHOSTNAMELEN];

char    tempstr[MAXPATHLEN];
char    tempuser[MAXPATHLEN];

/*
 * Timeout intervals for retrying connections
 * to hosts that don't accept PORT cmds.  This
 * is a kludge, but given the problems with TCP...
 */
#define	SWAITMAX	90	/* wait at most 90 seconds */
#define	SWAITINT	5	/* interval between retries */

int	swaitmax = SWAITMAX;
int	swaitint = SWAITINT;

int	keepalive = 0;

int	lostconn();
int	myoob();
FILE	*getdatasock(), *dataconn();

#ifdef SETPROCTITLE
char	**Argv = NULL;		/* pointer to argument vector */
char	*LastArgv = NULL;	/* end of argv */
char	proctitle[BUFSIZ];	/* initial part of title */
#endif /* SETPROCTITLE */
#include <locale.h>
/* bit mask used for TYPE L formatting */
unsigned char mask[] =
{
	0,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff
};
/* Ebcdic new line character */
#define EBCDIC_NL 0x15
/* This is an EBCDIC to ASCII conversion table
*/
unsigned char etoa [] =
{
	0000,0001,0002,0003,0234,0011,0206,0177,
	0227,0215,0216,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0235,0205,0010,0207,
	0030,0031,0222,0217,0034,0035,0036,0037,
	0200,0201,0202,0203,0204,0012,0027,0033,
	0210,0211,0212,0213,0214,0005,0006,0007,
	0220,0221,0026,0223,0224,0225,0226,0004,
	0230,0231,0232,0233,0024,0025,0236,0032,
	0040,0240,0241,0242,0243,0244,0245,0246,
	0247,0250,0325,0056,0074,0050,0053,0174,
	0046,0251,0252,0253,0254,0255,0256,0257,
	0260,0261,0041,0044,0052,0051,0073,0176,
	0055,0057,0262,0263,0264,0265,0266,0267,
	0270,0271,0313,0054,0045,0137,0076,0077,
	0272,0273,0274,0275,0276,0277,0300,0301,
	0302,0140,0072,0043,0100,0047,0075,0042,
	0303,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0304,0305,0306,0307,0310,0311,
	0312,0152,0153,0154,0155,0156,0157,0160,
	0161,0162,0136,0314,0315,0316,0317,0320,
	0321,0345,0163,0164,0165,0166,0167,0170,
	0171,0172,0322,0323,0324,0133,0326,0327,
	0330,0331,0332,0333,0334,0335,0336,0337,
	0340,0341,0342,0343,0344,0135,0346,0347,
	0173,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0350,0351,0352,0353,0354,0355,
	0175,0112,0113,0114,0115,0116,0117,0120,
	0121,0122,0356,0357,0360,0361,0362,0363,
	0134,0237,0123,0124,0125,0126,0127,0130,
	0131,0132,0364,0365,0366,0367,0370,0371,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,0372,0373,0374,0375,0376,0377,
};

/* This is an ASCII to EBCDIC conversion table
*/
unsigned char atoe [] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0232,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0137,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0152,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0112,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0241,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};

/* This is another (slightly different) ASCII to EBCDIC conversion table
*/
unsigned char atoibm[] =
{
	0000,0001,0002,0003,0067,0055,0056,0057,
	0026,0005,0045,0013,0014,0015,0016,0017,
	0020,0021,0022,0023,0074,0075,0062,0046,
	0030,0031,0077,0047,0034,0035,0036,0037,
	0100,0132,0177,0173,0133,0154,0120,0175,
	0115,0135,0134,0116,0153,0140,0113,0141,
	0360,0361,0362,0363,0364,0365,0366,0367,
	0370,0371,0172,0136,0114,0176,0156,0157,
	0174,0301,0302,0303,0304,0305,0306,0307,
	0310,0311,0321,0322,0323,0324,0325,0326,
	0327,0330,0331,0342,0343,0344,0345,0346,
	0347,0350,0351,0255,0340,0275,0137,0155,
	0171,0201,0202,0203,0204,0205,0206,0207,
	0210,0211,0221,0222,0223,0224,0225,0226,
	0227,0230,0231,0242,0243,0244,0245,0246,
	0247,0250,0251,0300,0117,0320,0241,0007,
	0040,0041,0042,0043,0044,0025,0006,0027,
	0050,0051,0052,0053,0054,0011,0012,0033,
	0060,0061,0032,0063,0064,0065,0066,0010,
	0070,0071,0072,0073,0004,0024,0076,0341,
	0101,0102,0103,0104,0105,0106,0107,0110,
	0111,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0142,0143,0144,0145,0146,0147,
	0150,0151,0160,0161,0162,0163,0164,0165,
	0166,0167,0170,0200,0212,0213,0214,0215,
	0216,0217,0220,0232,0233,0234,0235,0236,
	0237,0240,0252,0253,0254,0255,0256,0257,
	0260,0261,0262,0263,0264,0265,0266,0267,
	0270,0271,0272,0273,0274,0275,0276,0277,
	0312,0313,0314,0315,0316,0317,0332,0333,
	0334,0335,0336,0337,0352,0353,0354,0355,
	0356,0357,0372,0373,0374,0375,0376,0377,
};

main(argc, argv, envp)
	int argc;
	char *argv[];
	char **envp;
{
	extern int optind;
	extern char *optarg;
	int addrlen, on = 1;
	int ch, pid;
	struct sigvec sv;
	void trace_handler(int);

#ifdef _CSECURITY
char    tcpipClass[] = "tcpip\0";
#endif _CSECURITY

	setlocale(LC_ALL,"");
	/* New Security Code */
	saved_uid = getuidx(ID_SAVED);
	catd = catopen (MF_FTPD, NL_CAT_LOCALE);
#ifdef _CSECURITY
	if(auditproc(0, AUDIT_EVENTS, tcpipClass, 7) < 0){
                syslog(LOG_ALERT, "ftpd: auditproc: %m");
                exit(1);
	}
        else if ((auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0)) < 0) {
                syslog(LOG_ALERT, "ftpd: auditproc: %m");
                exit(1);
        }
#endif _CSECURITY
	addrlen = sizeof (his_addr);
	if (getpeername(0, (struct sockaddr *)&his_addr, &addrlen) < 0) {
		syslog(LOG_ERR, MSGSTR(PEERNAME, "getpeername (%s): %m"),argv[0]);
		exit(1);
	}
#ifdef _CSECURITY
        remote_addr = his_addr.sin_addr.s_addr;
#endif _CSECURITY
	addrlen = sizeof (ctrl_addr);
	if (getsockname(0, (struct sockaddr *)&ctrl_addr, &addrlen) < 0) {
	    syslog(LOG_ERR, MSGSTR(SOCKNAME,"getsockname (%s): %m"),argv[0]);
		exit(1);
	}
	data_source.sin_port = htons(ntohs(ctrl_addr.sin_port) - 1);
	debug = 0;
	openlog("ftpd", LOG_PID, LOG_DAEMON);
#ifdef SETPROCTITLE
	/*
	 *  Save start and extent of argv for setproctitle.
	 */
	Argv = argv;
	while (*envp)
		envp++;
	LastArgv = envp[-1] + strlen(envp[-1]);
#endif /* SETPROCTITLE */

        while ((ch = getopt(argc, argv, "dklsT:t:vu:")) != EOF)
	    switch((char)ch) {
	    case 'd':
	    case 'v':
		debug = 1;
		break;
		
	    case 'k':
		keepalive++;
		break;
		
	    case 'l':
		logging = 1;
		break;
		
	    case 's':
		tracing = 1;
		break;
		
	    case 't':
		timeout = atoi(optarg);
		if (maxtimeout < timeout)
		    maxtimeout = timeout;
		break;
		
	    case 'T':
		maxtimeout = atoi(optarg);
		if (timeout > maxtimeout)
		    timeout = maxtimeout;
		break;
		
	    case 'u':
	    {
		int n;
		
		/* Verify that all the digits are octal */
		for (n = 0; n < strlen (optarg); n++)
		    if ((optarg[n] < '0') || (optarg[n] > '7'))
			break;
		
		if (n == strlen (optarg))
		    sscanf (optarg, "%o", &defumask);
		else
		    fprintf (stderr, 
			     MSGSTR(BADVALUE, "ftpd: Bad value for -u\n"));
		
	    }
		break;
		
	    default:
		fprintf(stderr, 
			MSGSTR(UNKFLAG, "ftpd: Unknown flag -%c ignored.\n"), (char) ch);
	    }
        argc -= optind;
        argv += optind;
	

	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof(on)) < 0)
		syslog(LOG_ERR, MSGSTR(SOCKOPT, "setsockopt: %m"));

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = (void(*)(int))trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = (void(*)(int))trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);

	(void) freopen("/dev/null", "w", stderr);
	(void) signal(SIGPIPE, lostconn);
	(void) signal(SIGCHLD, SIG_IGN);
	if ((int)signal(SIGURG, myoob) < 0)
		syslog(LOG_ERR, MSGSTR(SIGNAL, "signal: %m"));

	/* handle urgent data inline */
	/* Sequent defines this, but it doesn't work */
#ifdef SO_OOBINLINE
	if (setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)) < 0)
		syslog(LOG_ERR, MSGSTR(SOCKOPT, "setsockopt: %m"));
#endif
	pid = getpid();
	if (ioctl(fileno(stdin), SIOCSPGRP, (char *) &pid) == -1)
		syslog(LOG_ERR, MSGSTR(EIOCTL, "ioctl SIOCSPGRP: %m"));
	dolog(&his_addr);
	/*
	 * Set up default state
	 */
	data = -1;
	type = TYPE_A;
	form = FORM_N;
	stru = STRU_F;
	mode = MODE_S;
        bytesz = NBBY;
        line_state = START_OF_LINE;
        car_ctl_char = ' ';
        next_bits = bytesz;
        useful_bits = 0;
        out_bits = 0;
        avail_bits = 0;
        out_char = '\0';
        space_bits = NBBY;
        rest_count = 0x20000;	/* restart markers at 128 K */
        rest_mark = 0;
	tmpline[0] = '\0';
	(void) gethostname(hostname, sizeof (hostname));
	reply(220, MSGSTR(SERVREADY, "%s FTP server (%s) ready."), hostname, version);
	(void) setjmp(errcatch);
	for (;;)
		(void) yyparse();
	/* NOTREACHED */
}


/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
void
trace_handler(int sig)
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_ERR, MSGSTR(SOCKOPT, "setsockopt: %m"));
}

lostconn()
{

	if (debug)
		syslog(LOG_DEBUG, MSGSTR(LOSTCONN, "lost connection"));
#ifdef _CSECURITY
	GET_PRIV(saved_uid);
        CONNECTION_WRITE(remote_addr, "ftp/tcp", "close",
				MSGSTR(LOSTCONN, "Lost connection"), -1);
	DROP_PRIV(effective_uid);
#endif _CSECURITY
	dologout(-1);
}

static char ttyline[20];

/*
 * Helper function for sgetpwnam().
 */
char *
sgetsave(s)
	char *s;
{
	char *malloc();
	char *new = malloc((unsigned) strlen(s) + 1);

	if (new == NULL) {
		perror_reply(421, MSGSTR(MALLOCERR, "Local resource failure: malloc"));
		dologout(1);
		/* NOTREACHED */
	}
	(void) strcpy(new, s);
	return (new);
}

/*
 * Save the result of a getpwnam.  Used for USER command, since
 * the data returned must not be clobbered by any other command
 * (e.g., globbing).
 */
struct passwd *
sgetpwnam(name)
	char *name;
{
	static struct passwd save;
	register struct passwd *p;
	char *sgetsave();

	if ((p = getpwnam(name)) == NULL)
		return (p);
	if (save.pw_name) {
		free(save.pw_name);
		free(save.pw_passwd);
		free(save.pw_gecos);
		free(save.pw_dir);
		free(save.pw_shell);
	}
	save = *p;
	save.pw_name = sgetsave(p->pw_name);
	save.pw_passwd = sgetsave(p->pw_passwd);
	save.pw_gecos = sgetsave(p->pw_gecos);
	save.pw_dir = sgetsave(p->pw_dir);
	save.pw_shell = sgetsave(p->pw_shell);
	return (&save);
}

int login_attempts;		/* number of failed login attempts */
int askpasswd;			/* had user command, ask for passwd */

/*
 * USER command.
 * Sets global passwd pointer pw if named account exists
 * and is acceptable; sets askpasswd if a PASS command is
 * expected. If logged in previously, need to reset state.
 * If name is "ftp" or "anonymous" and ftp account exists,
 * set guest and pw, then just return.
 * If account doesn't exist, ask for passwd anyway.
 * Otherwise, check user requesting login privileges.
 * Disallow anyone who does not have a standard
 * shell returned by getconfattr()).
 * Disallow anyone mentioned in the file FTPUSERS
 * to allow people such as root and uucp to be avoided.
 */
user(name)
	char *name;
{
	register char *cp = NULL;
	FILE *fd;
	char *shell, *validshells = NULL;
	char line[BUFSIZ];

	if (logged_in) {
		if (guest) {
			reply(530, MSGSTR(NOCHANGE, "Can't change user from guest login."));
			return;
		}
		end_login();
	}

	guest = 0;
	if (strcmp(name, "ftp") == 0 || strcmp(name, "anonymous") == 0) {
		if ((pw = sgetpwnam("ftp")) != NULL) {
			guest = 1;
			askpasswd = 1;
			reply(331, MSGSTR(LOGINOK, "Guest login ok, send ident as password."));
#ifdef _CSECURITY
			NET_ACCESS_WRITE( remote_addr,
			                  "File transfer",
			                  name,
			                  MSGSTR(LOGINOK, "Guest login ok, access restrictions apply."),
			                  0);
#endif _CSECURITY
		} else {
			reply(530, MSGSTR(USRUNK, "User %s unknown."), name);
#ifdef _CSECURITY
			NET_ACCESS_WRITE( remote_addr,
			                  "File transfer",
			                  name,
			                  "unknown user",
			                  -1);
#endif _CSECURITY
		}
		return;
	}
	if (pw = sgetpwnam(name)) {
		if ((shell = pw->pw_shell) == NULL || *shell == 0)
			shell = "/bin/sh";
		if(getconfattr(SC_SYS_LOGIN,SC_SHELLS,&validshells,SEC_LIST)) {
                        reply(530, MSGSTR(DENIED, "User %s access denied."), 
				name);
#ifdef _CSECURITY
			NET_ACCESS_WRITE( remote_addr,
			                  "File transfer",
			                  name,
			                  "access denied",
			                  -1 );
#endif _CSECURITY
                        if (logging)
                               syslog(LOG_NOTICE,
                                    	MSGSTR(GETCONFATTR, 
					"Getconfattr failed with error: %m"));
			pw = (struct passwd *) NULL;
			return;
		}
		while (*(cp = validshells) != NULL) {
			if (strcmp(cp, shell) == 0)
				break;
			else
				validshells += strlen(validshells) + 1;
		}
		if (*cp == NULL) {
		    reply(530, MSGSTR(DENIED, "User %s access denied."), name);
#ifdef _CSECURITY
			NET_ACCESS_WRITE( remote_addr,
			                  "File transfer",
			                  name,
			                  "access denied",
			                  -1 );
#endif _CSECURITY
			if (logging)
				syslog(LOG_NOTICE,
				    MSGSTR(REFUSED, "FTP LOGIN REFUSED FROM %s, %s"),
				    remotehost, name);
			pw = (struct passwd *) NULL;
			return;
		}
		if ((fd = fopen(FTPUSERS, "r")) != NULL) {
		    while (fgets(line, sizeof (line), fd) != NULL) {
			if ((cp = index(line, '\n')) != NULL)
				*cp = '\0';
			if (strcmp(line, name) == 0) {
				reply(530, MSGSTR(DENIED, "User %s access denied."), name);
#ifdef _CSECURITY
			NET_ACCESS_WRITE( remote_addr,
			                  "File transfer",
			                  name,
			                  "access denied",
			                  -1 );
#endif _CSECURITY
				if (logging)
					syslog(LOG_NOTICE,
				    	MSGSTR(REFUSED, "FTP LOGIN REFUSED FROM %s, %s"),
					    remotehost, name);
				pw = (struct passwd *) NULL;
				return;
			}
		    }
		}
		(void) fclose(fd);
	}
	reply(331, MSGSTR(PWDREQ, "Password required for %s."), name);
	askpasswd = 1;
	/*
	 * Delay before reading passwd after first failed
	 * attempt to slow down passwd-guessing programs.
	 */
	if (login_attempts)
		sleep((unsigned) login_attempts);
}

/*
 * Terminate login as previous user, if any, resetting state;
 * used when USER command is given or login fails.
 */
end_login()
{

/* New Security Code (void) seteuid(saved_uid); */
	GET_PRIV(saved_uid);
	if (logged_in)
		logwtmp(ttyline, "", "");
	pw = NULL;
	logged_in = 0;
	guest = 0;
}

pass(passwd)
	char *passwd;
{
	char *xpasswd, *salt;
	int limit, reenter;
        char *message = (char *) NULL;
        int passexp_rc = 0;
        int logrestrict_rc = 0;
        int authenticate_rc = 0;


	if (logged_in || askpasswd == 0) {
		reply(503, MSGSTR(LOGINUSER, "Login with USER first."));
#ifdef _CSECURITY
                NET_ACCESS_WRITE(remote_addr, "File transfer", "",
                        MSGSTR(LOGINUSER, "Login with USER first."), -1);
#endif _CSECURITY
		return;
	}
	askpasswd = 0;
	if (!guest) {		/* "ftp" is only account allowed no password */
                if (pw) {

                        /* Check to see if the user's password expired */
                        passexp_rc = passwdexpired(pw->pw_name, &message);
                        if (message) {
                                free(message);
                                message = (char *) NULL;
                        }

                        /* Check to see if there are any login restrictions for
                         * the user. This includes a check to see if the
                         * user's account expired.
                         */
                        logrestrict_rc = loginrestrictions(pw->pw_name,0,0,&message);
                        if (message) {
                                free(message);
                                message = (char *) NULL;
                        }
			authenticate_rc = authenticate(pw->pw_name,passwd,&reenter, &message);
                        if (message) {
                                free(message);
                                message = (char *) NULL;
                        }
                } else {
			/*
			 * No pwd struct, so no user.
			 */
			authenticate_rc = 1;
		}

                /* The strcmp does not catch null passwords! */
		if ( passexp_rc ||                       /* password expired? */
                    logrestrict_rc ||                   /* account expired? */
		    authenticate_rc ) {                /* not authenticated? */

			reply(530, MSGSTR(LOGINERR,"Login incorrect."));
#ifdef _CSECURITY
                        NET_ACCESS_WRITE(remote_addr,
			                 "File transfer",
			                 pw ? pw->pw_name : "",
			                 MSGSTR(LOGINERR,"Login incorrect."),
			                 -1);
#endif _CSECURITY
			pw = NULL;
			if (login_attempts++ >= 5) {
				syslog(LOG_NOTICE,
				    MSGSTR(LOGINFAIL, "repeated login failures from %s"),
				    remotehost);
				exit(0);
			}
			return;
		}
	}
	login_attempts = 0;		/* this time successful */
	(void) setgid((gid_t)pw->pw_gid);
	(void) initgroups(pw->pw_name, pw->pw_gid);

	/* open wtmp before chroot */
	(void)sprintf(ttyline, "ftp%d", getpid());
	logwtmp(ttyline, pw->pw_name, remotehost);
	logged_in = 1;

	if(getuserattr(pw->pw_name,S_UFSIZE,&limit,SEC_INT) >= 0){
		if(limit > 0) 
			ulimit(2,limit);
	}

	if (guest) {
		/*
		 * We MUST do a chdir() after the chroot. Otherwise
		 * the old current directory will be accessible as "."
		 * outside the new root!
		 */
		if (chroot(pw->pw_dir) < 0 || chdir("/") < 0) {
			reply(550, "Can't set guest privileges.");
			goto bad;
		}
	} else if (chdir(pw->pw_dir) < 0) {
		if (chdir("/") < 0) {
			reply(530, MSGSTR(DIRERR, "User %s: can't change directory to %s."),
			    pw->pw_name, pw->pw_dir);
			goto bad;
		} else
			lreply(230, MSGSTR(NODIR, "No directory! Logging in with home=/"));
	}

/* New Security Code	if (seteuid((uid_t)pw->pw_uid) < 0) */
	effective_uid = pw->pw_uid;
        priv.pv_priv[0] = 0;
        priv.pv_priv[1] = 0;
        setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv,sizeof(priv_t));
	if (setuidx(ID_REAL|ID_EFFECTIVE|ID_SAVED, (uid_t)pw->pw_uid) < 0) {
		reply(550, MSGSTR(NOSEETUID, "Can't set uid."));
		goto bad;
	}
	if (guest) {
		reply(230, MSGSTR(GUESTLOGIN, "Guest login ok, access restrictions apply."));
#ifdef SETPROCTITLE
		sprintf(proctitle, "%s: anonymous/%.*s", remotehost,
		    sizeof(proctitle) - sizeof(remotehost) -
		    sizeof(": anonymous/"), passwd);
		setproctitle(proctitle);
#endif /* SETPROCTITLE */
		if (logging)
			syslog(LOG_INFO, MSGSTR(ANONYMLOGIN, "ANONYMOUS FTP LOGIN FROM %s, %s"),
			    remotehost, passwd);
	} else {
		reply(230, MSGSTR(LOGIN, "User %s logged in."), pw->pw_name);
#ifdef SETPROCTITLE
		sprintf(proctitle, "%s: %s", remotehost, pw->pw_name);
		setproctitle(proctitle);
#endif /* SETPROCTITLE */
		if (logging)
			syslog(LOG_INFO, MSGSTR(LOGINFROM, "FTP LOGIN FROM %s, %s"),
			    remotehost, pw->pw_name);
	}
#ifdef _CSECURITY
	GET_PRIV(saved_uid);
        NET_ACCESS_WRITE(remote_addr, "File transfer", pw->pw_name, "", 0);
	DROP_PRIV(effective_uid);
#endif _CSECURITY

	home = pw->pw_dir;		/* home dir for globbing */
	(void) umask(defumask);
	return;
bad:
        /* Forget all about it... after doing an audit write */
#ifdef _CSECURITY
        NET_ACCESS_WRITE(remote_addr,"File transfer","",
				MSGSTR(LOGINERR,"Login incorrect."),-1);
#endif _CSECURITY
	end_login();
}

retrieve(cmd, name)
	char *cmd, *name;
{
	FILE *fin, *dout;
	struct stat st;
	int (*closefunc)();
#ifdef _CSECURITY
        char *local, *remote;

        local = "";
        remote = name;
#endif _CSECURITY
	if (cmd == 0) {
		fin = fopen(name, "r"), closefunc = fclose;
		st.st_size = 0;
	} else {
		char line[BUFSIZ];

		(void) sprintf(line, cmd, name), name = line;
		fin = ftpd_popen(line, "r"), closefunc = ftpd_pclose;
		st.st_size = -1;
		st.st_blksize = BUFSIZ;
	}
	if (fin == NULL) {
		if (errno != 0) {
			perror_reply(550, name);
#ifdef _CSECURITY
			GET_PRIV(saved_uid);
                        EXPORT_DATA_WRITE(remote_addr, local, remote,
                                sys_errlist[errno], errno);
			DROP_PRIV(effective_uid);
                } else {
			GET_PRIV(saved_uid);
                        EXPORT_DATA_WRITE(remote_addr, local, remote,
                                "Can't open remote file", -1);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
		}
		return;
	}
	if (cmd == 0 &&
	    (fstat(fileno(fin), &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG)) {
		reply(550, MSGSTR(NOTPLAINF, "%s: not a plain file."), name);
#ifdef _CSECURITY
		GET_PRIV(saved_uid);
                EXPORT_DATA_WRITE(remote_addr, local, remote,
                        "Remote file is not plain", -1);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		goto done;
	}
	dout = dataconn(name, st.st_size, "w", 0);
	if (dout == NULL) {
#ifdef _CSECURITY
		GET_PRIV(saved_uid);
                EXPORT_DATA_WRITE(remote_addr, local, remote,
                        "Data connection not established", -1);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		goto done;
	}
	send_data(fin, dout, st.st_blksize);
#ifdef _CSECURITY
	if (logging)
                syslog(LOG_INFO,"FTPD: EXPORT file local %s, remote %s",
                                                                local, remote);
	GET_PRIV(saved_uid);
	EXPORT_DATA_WRITE(remote_addr, local, remote, "", 0);
	DROP_PRIV(effective_uid);
#endif _CSECURITY
	(void) fclose(dout);
	data = -1;
	pdata = -1;
done:
	(*closefunc)(fin);
}

store(name, mode, unique)
	char *name, *mode;
	int unique;
{
	FILE *fout, *din;
	struct stat st;
	int (*closefunc)(), fdout, oflag;
	char *gunique();

#ifdef _CSECURITY
        char *remote;

	remote = "";
#endif _CSECURITY

	if (unique && stat(name, &st) == 0 &&
	    (name = gunique(name)) == NULL)
		return;
	fout = fopen(name, mode);
	if (strcmp(mode, "a") == 0)
		lseek(fileno(fout), (off_t)0, SEEK_END);
	closefunc = fclose;
	if (fout == NULL) {
		perror_reply(553, name);
#ifdef _CSECURITY
		GET_PRIV(saved_uid);
                IMPORT_DATA_WRITE(remote_addr, name, remote,
                        sys_errlist[errno], errno);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		return;
	}
	din = dataconn(name, (off_t)-1, "r", unique);
	if (din == NULL) {
#ifdef _CSECURITY
		GET_PRIV(saved_uid);
                IMPORT_DATA_WRITE(remote_addr, name, remote,
                        MSGSTR(NODATACONN, "Can't build data connection"), -1);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		(*closefunc)(fout);
	}
	if (receive_data(din, fout) == 0) {
		if ((*closefunc)(fout) < 0 ) {
                        perror_reply(452, "Error writing file");
                        return;
                }
		if (unique) {
			reply(226, MSGSTR(TRANSCOMP, "Transfer complete (unique file name:%s)."), name);
#ifdef _CSECURITY
			if (logging)
                                syslog(LOG_INFO,
                                        "FTPD: IMPORT file local %s, remote %s",
                                                                name, remote);
			GET_PRIV(saved_uid);
                        IMPORT_DATA_WRITE(remote_addr, name, remote,
                                sys_errlist[errno], errno);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
		} else {
			reply(226, MSGSTR(TRANSOK, "Transfer complete."));
#ifdef _CSECURITY
			if (logging)
                                syslog(LOG_INFO,
                                        "FTPD: IMPORT file local %s, remote %s",
                                                                name, remote);
			GET_PRIV(saved_uid);
                        IMPORT_DATA_WRITE(remote_addr, name, remote,
				MSGSTR(TRANSOK, "Transfer complete."), 0);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
		}
	}
	(void) fclose(din);
	data = -1;
	pdata = -1;
}

FILE *
getdatasock(mode)
	char *mode;
{
	int s, on = 1;
	int bindretry = 0;

	if (data >= 0)
		return (fdopen(data, mode));
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (NULL);
/* New Security Code (void) seteuid(saved_uid); */
	GET_PRIV(saved_uid);
	if (keepalive) {
	    if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (char *) &on, sizeof (on)) < 0)
		goto bad;
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on)) < 0)
	    goto bad;
	/* anchor socket to avoid multi-homing problems */
	data_source.sin_family = AF_INET;
	data_source.sin_addr = ctrl_addr.sin_addr;
	while (bind(s, (struct sockaddr *)&data_source, sizeof (data_source)) < 0) {
		if ( bindretry > 20 )
			goto bad;

		bindretry++;
		sleep(swaitint);
	}
/* New Security Code (void) seteuid((uid_t)pw->pw_uid); */
	DROP_PRIV(effective_uid);
	return (fdopen(s, mode));
bad:
/* New Security Code (void) seteuid((uid_t)pw->pw_uid); */
	DROP_PRIV(effective_uid);
	(void) close(s);
	return (NULL);
}

FILE *
dataconn(name, size, mode, unique)
	char *name;
	off_t size;
	char *mode;
	int unique;	/* RFC 1123 requires specific format for STOU reply */
{
	char sizebuf[32];
	FILE *file;
	int retry = 0;
#ifdef _CSECURITY
        char portbuf[8];
#endif _CSECURITY

	file_size = size;
	byte_count = 0;
	if (size != (off_t) -1)
		(void) sprintf (sizebuf, MSGSTR(BYTES, " (%ld bytes)"), size);
	else
		(void) strcpy(sizebuf, "");
	if (pdata >= 0) {
		struct sockaddr_in from;
		int s, fromlen = sizeof(from);

		s = accept(pdata, (struct sockaddr *)&from, &fromlen);
		if (s < 0) {
			reply(425, MSGSTR(NOCONN, "Can't open data connection."));
			(void) close(pdata);
			pdata = -1;
#ifdef _CSECURITY
			GET_PRIV(saved_uid);
                        CONNECTION_WRITE(remote_addr, "", "open",
			MSGSTR(NOCONN, "Can't open data connection."), errno);
			DROP_PRIV(effective_uid);
#endif _CSECURITY
			return(NULL);
		}
		(void) close(pdata);
		pdata = s;
		if (unique)
		    reply(150, 
		    	MSGSTR(UNIQUE_CONN, "FILE:  %s"), name);
		else
		    reply(150, 
		    	MSGSTR(OPENCONN, "Opening data connection for %s%s."),
		    	name, sizebuf);
#ifdef _CSECURITY
                sprintf(portbuf, "%d", ntohs(from.sin_port));
		GET_PRIV(saved_uid);
                CONNECTION_WRITE(from.sin_addr, portbuf, "open", "", 0);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		return(fdopen(pdata, mode));
	}
	if (data >= 0) {
		if (unique)
			reply(125, 
			    MSGSTR(UNIQUE_CONN, "FILE:  %s"), name); 
		else
			reply(125, 
			    MSGSTR(USECONN, 
			    "Using existing data connection for %s%s."), 
			    name, sizebuf); 
		usedefault = 1;
		return (fdopen(data, mode));
	}
	if (usedefault)
		data_dest = his_addr;
	usedefault = 1;
	file = getdatasock(mode);
	if (file == NULL) {
		reply(425, MSGSTR(NODATASOCK, "Can't create data socket (%s,%d): %s."),
		    inet_ntoa(data_source.sin_addr),
		    ntohs(data_source.sin_port),
		    errno < sys_nerr ? sys_errlist[errno] : "unknown error");
#ifdef _CSECURITY
                sprintf(portbuf, "%d", ntohs(data_source.sin_port));
		GET_PRIV(saved_uid);
                CONNECTION_WRITE(data_source.sin_addr, portbuf, "open",
                        "Can't create data socket", -1);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		return (NULL);
	}
	data = fileno(file);
	while (connect(data, (struct sockaddr *)&data_dest,
	    sizeof (data_dest)) < 0) {
		if (errno == EADDRINUSE && retry < swaitmax) {
			sleep((unsigned) swaitint);
			retry += swaitint;
			continue;
		}
		perror_reply(425, MSGSTR(NODATACONN, "Can't build data connection"));
		(void) fclose(file);
		data = -1;
#ifdef _CSECURITY
                sprintf(portbuf, "%d", ntohs(data_dest.sin_port));
		GET_PRIV(saved_uid);
                CONNECTION_WRITE(data_dest.sin_addr, portbuf, "open",
                MSGSTR(NODATACONN, "Can't build data connection"), errno);
		DROP_PRIV(effective_uid);
#endif _CSECURITY
		return (NULL);
	}
	if (unique)
	    reply(150, 
	    	MSGSTR(UNIQUE_CONN, "FILE:  %s"), name);
	else
	    reply(150, 
		MSGSTR(OPENCONN, "Opening data connection for %s%s."),
		name, sizebuf);
#ifdef _CSECURITY
	sprintf(portbuf, "%d", ntohs(data_dest.sin_port));
	GET_PRIV(saved_uid);
	CONNECTION_WRITE(data_dest.sin_addr, portbuf, "open", "", 0);
	DROP_PRIV(effective_uid);
#endif _CSECURITY
	return (file);
}

/*
 * Tranfer the contents of "instr" to
 * "outstr" peer using the appropriate
 * encapsulation of the data subject
 * to Mode, Structure, Type and Form.
 */
send_data(instr, outstr, blksize)
	FILE *instr, *outstr;
	off_t blksize;
{
	int c,cnt,d ;
	register char *buf, *hbuf, *bufp;
	int netfd, filefd;
	int buf_char;
	int block_count;
	char *block_ptr;
	int read_count;
	int mark_count;
#define DOPUTC(ch) \
	(void) putc(ch, outstr); \
	if (ferror(outstr)) \
		goto data_err; \
	byte_count++;

	mark_count = 0;
	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}

	switch (mode) {

	case MODE_S:
	    switch (stru) {

	    case STRU_F:
		switch (type) {
	
		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
                        if ((buf = malloc((u_int)blksize)) == NULL ||
                                (hbuf = malloc((u_int)blksize*2)) == NULL) {
                                transflag = 0;
                                perror_reply(451,
                                    MSGSTR(MALLOCERR,
                                    "Local resource failure: malloc"));
                                return;
                        }
                        netfd = fileno(outstr);
                        filefd = fileno(instr);
                        while ((c = read(filefd, buf, (u_int)blksize)) > 0) {
                            /* do translation */
                            for (d = cnt = 0; d < c; ) {
                                if ((hbuf[cnt++] = buf[d++]) == '\n') {
                                    hbuf[cnt-1] = '\r';
                                    hbuf[cnt++] = '\n';
                                }
                            }
                            byte_count += cnt;
                            for (bufp = hbuf; cnt > 0; cnt -= d, bufp += d)
                                if ((d = write(netfd, bufp, cnt)) <= 0)
                                    break;
                            if (d <= 0)
                                break;
                        }
                        transflag = 0;
                        (void)free(buf);
                        (void)free(hbuf);
                        if (c != 0) {
                                if (d < 0)
                                        goto file_err;
                                goto data_err;
                        }
                        reply(226, MSGSTR(TRANSOK, "Transfer complete."));
                        return;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC('1');
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(car_ctl_char);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC('\r');
					DOPUTC('\n');
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC('\r');
					DOPUTC('\n');
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				DOPUTC(c);
				line_state = IN_LINE;
			}
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }
		    break;
	
		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(instr)) != EOF) {
				if (c == '\n') {
					DOPUTC(EBCDIC_NL);
				}
				else {
					DOPUTC(atoe[c]);
				}
			}
			break;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC(atoe['1']);
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(atoe[car_ctl_char]);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC(EBCDIC_NL);
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC(EBCDIC_NL);
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				DOPUTC(atoe[c]);
				line_state = IN_LINE;
			}
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }
		    break;

		case TYPE_I:
			if ((buf = malloc((u_int)blksize)) == NULL) {
				transflag = 0;
				perror_reply(451,
				    MSGSTR(MALLOCERR,
				    "Local resource failure: malloc"));
				return;
			}
			netfd = fileno(outstr);
			filefd = fileno(instr);
			while ((cnt = read(filefd, buf, (u_int)blksize)) > 0 &&
			    write(netfd, buf, cnt) == cnt)
				byte_count += cnt;
			transflag = 0;
			(void)free(buf);
			if (cnt != 0) {
				if (cnt < 0)
					goto file_err;
				goto data_err;
			}
			reply(226, MSGSTR(TRANSOK, "Transfer complete."));
			return;

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
			next_bits = bytesz;
			useful_bits = 0;
			out_bits = 0;
			out_char = '\0';
			
			while ((c = getc(instr)) != EOF) {
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesz;
				}
				c &= mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits == NBBY) {
					DOPUTC(out_char);
					out_char = '\0';
					out_bits = 0;
				}
				else if (total_bits > NBBY) {
					DOPUTC(out_char);
					out_char = c << (NBBY - out_bits);
					out_bits = total_bits - NBBY;
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesz) {
				while (next_bits > 0) {
					DOPUTC(out_char);
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
				}
			}
			else if (out_bits > 0) {
				DOPUTC(out_char);
			}
			break;

		default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "TYPE", type);
			return;

		}
		break;

	    case STRU_R:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			line_state = START_OF_LINE;
			while ((c = getc(instr)) != EOF) {
				line_state = IN_LINE;
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
				}
				else if (c == REC_ESC) {
					DOPUTC(REC_ESC);
					DOPUTC(REC_ESC);
				}
				else {
					DOPUTC(c);
				}
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC('1');
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(car_ctl_char);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				line_state = IN_LINE;
				if (c == REC_ESC) {
					DOPUTC(REC_ESC);
				}
				DOPUTC(c);
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }
		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			line_state = START_OF_LINE;
			while ((c = getc(instr)) != EOF) {
				line_state = IN_LINE;
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
				}
				else if (c == REC_ESC) {
					DOPUTC(REC_ESC);
					DOPUTC(REC_ESC);
				}
				else {
					DOPUTC(atoe[c]);
				}
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    case FORM_C:
			line_state = START_OF_LINE;
			car_ctl_char = ' ';
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						DOPUTC(atoe['1']);
						line_state = IN_LINE;
						continue;
					}
					DOPUTC(atoe[car_ctl_char]);
					line_state = IN_LINE;
				}
				if (c == '\n') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					continue;
				}
				if (c == '\r') {
					DOPUTC(REC_ESC);
					DOPUTC(REC_EOR);
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					continue;
				}
				line_state = IN_LINE;
				if (c == REC_ESC) {
					DOPUTC(REC_ESC);
				}
				DOPUTC(atoe[c]);
			}
			DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }
		    break;

		case TYPE_I:
		    line_state = START_OF_LINE;
		    block_count = 2;
		    while ((c = getc(instr)) != EOF) {
			line_state = IN_LINE;
			if (c == REC_ESC) {
				DOPUTC(REC_ESC);
				++block_count;
			}
			DOPUTC(c);
			++block_count;
			if (block_count >= blksize) {
				DOPUTC(REC_ESC);
				DOPUTC(REC_EOR);
				block_count = block_count % blksize + 2;
				line_state = START_OF_LINE;
			}
		    }
		    DOPUTC(REC_ESC);
		    if (line_state != START_OF_LINE) {
			DOPUTC(REC_EOR | REC_EOF);
		    }
		    else {
			DOPUTC(REC_EOF);
		    }
		    break;

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
			line_state = START_OF_LINE;
			block_count = 2;
			next_bits = bytesz;
			useful_bits = 0;
			out_bits = 0;
			out_char = '\0';
			
			while ((c = getc(instr)) != EOF) {
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesz;
				}
				c &= mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits >= NBBY) {
					line_state = IN_LINE;
					out_char &= 0xff;
					if (out_char == REC_ESC) {
						DOPUTC(REC_ESC);
						++block_count;
					}
					DOPUTC(out_char);
					++block_count;
					if (block_count >= blksize) {
						DOPUTC(REC_ESC);
						DOPUTC(REC_EOR);
						block_count = 
						    block_count % blksize + 2;
						line_state = START_OF_LINE;
					}
					if (total_bits == NBBY) {
						out_char = '\0';
					}
					else {
						out_char = 
						    c << (NBBY - out_bits);
					}
					out_bits = total_bits - NBBY;
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesz) {
				line_state = IN_LINE;
				while (next_bits > 0) {
					if (out_char == REC_ESC) {
						DOPUTC(REC_ESC);
					}
					DOPUTC(out_char);
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
				}
			}
			else if (out_bits > 0) {
				line_state = IN_LINE;
				if (out_char == REC_ESC) {
					DOPUTC(REC_ESC);
				}
				DOPUTC(out_char);
			}
		    	DOPUTC(REC_ESC);
			if (line_state != START_OF_LINE) {
				DOPUTC(REC_EOR | REC_EOF);
			}
			else {
				DOPUTC(REC_EOF);
			}
			break;

		default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "TYPE", type);
			return;
		}
		break;

	    default:
		transflag = 0;
		reply(501,
		    MSGSTR(UNIMPSEND, 
		    "Unimplemented %s %d in send_data"), "STRU", stru);
		return;

	    }
	    break;

	case MODE_B:

#define DOBUF(ch,d,sp)\
	*buf++ = ch;\
	++block_count;\
	DOENDBUF(d,sp)\

#define DOENDBUF(d,sp)\
	if ((block_count >= blksize - sp) || d) {\
		block_ptr[0] = d;\
		block_ptr[1] = (block_count - 3) >> 8;\
		block_ptr[2] = (block_count - 3) & 0xff;\
		if (fwrite(block_ptr, block_count, 1, outstr) == 0)\
			goto data_err;\
		byte_count += block_count;\
		if (!(d & BLK_EOF)) {\
			DORESTMARK\
		}\
		block_count = 3;\
		buf = block_ptr;\
		buf += block_count;\
	}\

#define DORESTMARK\
	if (read_count > rest_count) {\
		mark_count += read_count;\
		block_ptr[0] = BLK_RESTART;\
		block_ptr[1] = 0;\
		block_ptr[2] = 21;\
		if (next_bits < 0)\
			++block_ptr[2];\
		sprintf(&block_ptr[3], "%08d%02d%03d%03d%02d%03d",\
			mark_count, line_state, car_ctl_char,\
			next_bits, out_bits, out_char & 0xff);\
		if (fwrite(block_ptr, block_ptr[2]+3, 1, outstr) == 0)\
			goto data_err;\
		byte_count += block_ptr[2]+3;\
		read_count = 0;\
	}\

	    /* rest_mark and other state vars. are set by the REST command */
	    if (rest_mark > 0) {

		if (fseek(instr, rest_mark, 0) != 0) {
		    reply(554, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid REST parameter"));
		    return(-1);
		}
	        mark_count = rest_mark;
	    }
            else {
                line_state = START_OF_LINE;
                car_ctl_char = ' ';
		next_bits = bytesz;
                out_bits = 0;
                out_char = '\0';
            }

	    if ((buf = malloc((u_int)blksize)) == NULL) {
	    	transflag = 0;
	    	perror_reply(451,
	    	    MSGSTR(MALLOCERR,
	    	    "Local resource failure: malloc"));
	    	return;
	    }
	    block_ptr = buf;
	    block_count = 3;
	    buf += block_count;
	    read_count = 0;

	    switch (stru) {

	    case STRU_F:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:

			while ((c = getc(instr)) != EOF) {
				if ((c == '\n') && (line_state != DID_CR)) {
					line_state = DID_CR;
					DOBUF('\r', 0, 0)
				}
				++read_count;
				line_state = 0;
				DOBUF(c, 0, 0)
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF('1', 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(car_ctl_char, 0, 0);
				}
				if ((c == '\n') || (c == '\r')) {
					if (line_state != DID_CR) {
						line_state = DID_CR;
						DOBUF('\r', 0, 0);
					}
					++read_count;
					line_state = START_OF_LINE;
					if (c == '\n') {
						car_ctl_char = ' ';
					}
					if (c == '\r') {
						car_ctl_char = '+';
					}
					DOBUF('\n', 0, 0);
					continue;
				}
				++read_count;
				line_state = IN_LINE;
				DOBUF(c, 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }

		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(instr)) != EOF) {
				++read_count;
				if (c == '\n') {
					DOBUF(EBCDIC_NL, 0, 0);
				}
				else {
					DOBUF(atoe[c], 0, 0);
				}
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF(atoe['1'], 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(atoe[car_ctl_char], 0, 0);
				}
				++read_count;
				if (c == '\n') {
					line_state = START_OF_LINE;
					car_ctl_char = ' ';
					DOBUF(EBCDIC_NL, 0, 0);
					continue;
				}
				if (c == '\r') {
					line_state = START_OF_LINE;
					car_ctl_char = '+';
					DOBUF(EBCDIC_NL, 0, 0);
					continue;
				}
				line_state = IN_LINE;
				DOBUF(atoe[c], 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }

		    break;

		case TYPE_I:
		    filefd = fileno(instr);
		    while ((cnt = read(filefd, buf, (u_int)blksize-3)) > 0){
			read_count += cnt;
			block_count += cnt;
			DOENDBUF(0, blksize)
		    }
		    DOENDBUF(BLK_EOF, 0)
		    break;

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
			while ((c = getc(instr)) != EOF) {
				++read_count;
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesz;
				}
				c &= mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits == NBBY) {
					buf_char = out_char;
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
				else if (total_bits > NBBY) {
					buf_char = out_char;
					out_char = c << (NBBY - out_bits);
					out_bits = total_bits - NBBY;
					DOBUF(buf_char, 0, 0);
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesz) {
				while (next_bits > 0) {
					buf_char = out_char;
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
			}
			else if (out_bits > 0) {
				DOBUF(out_char, 0, 0)
			}
			DOENDBUF(BLK_EOF, 0)

			break;

		default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "TYPE", type);
			return;
		}
		break;

	    case STRU_R:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(instr)) != EOF) {
				++read_count;
				if (c == '\n') {
					DOENDBUF(BLK_EOR, 0)
				}
				else {
					DOBUF(c, 0, 0)
				}
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF('1', 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(car_ctl_char, 0, 0);
				}
				if ((c == '\n') || (c == '\r')) {
					++read_count;
					line_state = START_OF_LINE;
					if (c == '\n') {
						car_ctl_char = ' ';
					}
					if (c == '\r') {
						car_ctl_char = '+';
					}
					DOENDBUF(BLK_EOR, 0)
					continue;
				}
				++read_count;
				line_state = IN_LINE;
				DOBUF(c, 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }

		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(instr)) != EOF) {
				++read_count;
				if (c == '\n') {
					DOENDBUF(BLK_EOR, 0)
				}
				else {
					DOBUF(atoe[c], 0, 0);
				}
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    case FORM_C:
			while ((c = getc(instr)) != EOF) {
				if (line_state == START_OF_LINE) {
					if (c == '\f') {
						++read_count;
						line_state = IN_LINE;
						DOBUF(atoe['1'], 0, 0);
						continue;
					}
					line_state = IN_LINE;
					DOBUF(atoe[car_ctl_char], 0, 0);
				}
				++read_count;
				if ((c == '\n') || (c == '\r')) {
					line_state = START_OF_LINE;
					if (c == '\n') {
						car_ctl_char = ' ';
					}
					if (c == '\r') {
						car_ctl_char = '+';
					}
					DOENDBUF(BLK_EOR, 0)
					continue;
				}
				line_state = IN_LINE;
				DOBUF(atoe[c], 0, 0);
			}
			DOENDBUF(BLK_EOF, 0)
			break;

		    default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "FORM", form);
			return;

		    }

		    break;

		case TYPE_I:
		    filefd = fileno(instr);
		    while ((cnt = read(filefd, buf, (u_int)blksize-3)) > 0){
			read_count += cnt;
			block_count += cnt;
			DOENDBUF(BLK_EOR, 0)
		    }
		    DOENDBUF(BLK_EOF, 0)
		    break;

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
			while ((c = getc(instr)) != EOF) {
				++read_count;
				if (next_bits > NBBY) {
					useful_bits = NBBY;
					next_bits -= NBBY;
				}
				else {
					useful_bits = next_bits;
					next_bits = bytesz;
				}
				c &= mask[useful_bits];
				out_char |= c >> out_bits;
				total_bits = out_bits + useful_bits;
				if (total_bits == NBBY) {
					buf_char = out_char;
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
				else if (total_bits > NBBY) {
					buf_char = out_char;
					out_char = c << (NBBY - out_bits);
					out_bits = total_bits - NBBY;
					DOBUF(buf_char, 0, 0);
				}
				else out_bits = total_bits;
			}
			if (next_bits != bytesz) {
				while (next_bits > 0) {
					buf_char = out_char;
					next_bits -= (NBBY - out_bits);
					out_char = '\0';
					out_bits = 0;
					DOBUF(buf_char, 0, 0);
				}
			}
			else if (out_bits > 0) {
				DOBUF(out_char, 0, 0)
			}
			DOENDBUF(BLK_EOR | BLK_EOF, 0)

			break;

		default:
			transflag = 0;
			reply(501,
			    MSGSTR(UNIMPSEND, 
			    "Unimplemented %s %d in send_data"), "TYPE", type);
			return;
		}
		break;

	    default:
		transflag = 0;
		reply(501,
		    MSGSTR(UNIMPSEND, 
		    "Unimplemented %s %d in send_data"), "STRU", stru);
		return;

	    }
	    (void)free(block_ptr);
	    break;

	default:
	    transflag = 0;
	    reply(501,
	        MSGSTR(UNIMPSEND, 
	        "Unimplemented %s %d in send_data"), "MODE", mode);
	    return;

	}
	fflush(outstr);
	transflag = 0;
	if (ferror(instr))
		goto file_err;
	if (ferror(outstr))
		goto data_err;
	reply(226, MSGSTR(TRANSOK, "Transfer complete."));
	return;

data_err:
	if (block_ptr)
	    (void)free(block_ptr);
	transflag = 0;
	perror_reply(426, MSGSTR(DATACONNERR, "Data connection"));
	return;

file_err:
	transflag = 0;
	perror_reply(551, MSGSTR(ERRINPUT, "Error on input file"));
}

/*
 * Transfer data from peer to
 * "outstr" using the appropriate
 * encapulation of the data subject
 * to Mode, Structure, Type and Form.
 */
receive_data(instr, outstr)
	FILE *instr, *outstr;
{
	int c;
	int cnt,d;
#define DOPUTC2(ch)\
	(void) putc (ch, outstr);\
	if (ferror(outstr))\
		goto file_err;\
	write_count++;
#define MYMIN(a,b) ((a) < (b) ? (a) : (b))

	int block_count;
	int block_code;
	int read_count;
	int write_count;
	char buf[BUFSIZ],hbuf[BUFSIZ*2],*bufp;
	char mark_s[40];
	char mark_r[40];
	c = 0;
	block_code = 0;

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return (-1);
	}

	errno = 0;
	switch (mode) {

	case MODE_S:
	    switch (stru) {

	    case STRU_F:
		switch (type) {

		case TYPE_A:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
                        line_state = 0;
                        while ((cnt = read(fileno(instr), buf, sizeof buf)) > 0) {
                            int hind, i;

                            buf[cnt] = buf[cnt + 1] = '\n';
                            byte_count += cnt;
                            hind = 0;
                            if(line_state == DID_CR) {
                                hbuf[0] = '\r';
                                hind = 1;
                            }

                            for (i = 0; i < cnt; ) {
                                if(((hbuf[hind++] = buf[i++])== '\n')
                                         && (hbuf[hind - 2] == '\r')) {
                                    hbuf[hind-- - 2] = '\n';
                                }
                            }
                            if(hbuf[hind - 1] == '\r') {
                                hind--; /* save for possible overwrite */
                                line_state = DID_CR;
                            }
                            else line_state = 0;

                            if ((d = write(fileno(outstr), hbuf, hind)) <= 0)
                                goto file_err;

                            if (d < hind) {
                                goto file_err;
                            }
                        }  /* end of file */
                        if (cnt < 0)
                            goto data_err;
                        transflag = 0;
                        return (0);

		    case FORM_C:
			c = getc(instr);
			byte_count++;
			line_state = IN_LINE;
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				switch (line_state) {
				case START_OF_LINE:
				    switch (c) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == '\r') {
						line_state = DID_CR;
						continue;
					}
					break;
				case DID_CR:
					if (c == '\n') {
						line_state = START_OF_LINE;
						continue;
					}
					else if (c != '\r') {
						DOPUTC2('\r')
						line_state = IN_LINE;
					}
					break;
				default:
					break;
				}
				DOPUTC2(c)
			}
			if (errno) {
			    goto data_err;
			}

			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_E:
		    switch (form) {

		    case FORM_N:
		    case FORM_T:
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				if (c == EBCDIC_NL) {
					DOPUTC2('\n')
				}
				else {
					DOPUTC2(etoa[c])
				}
			}
			if (errno) {
			    goto data_err;
			}

			break;

		    case FORM_C:
			c = getc(instr);
			byte_count++;
			line_state = IN_LINE;
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				switch (line_state) {
				case START_OF_LINE:
				    switch (etoa[c]) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == EBCDIC_NL) {
						line_state = START_OF_LINE;
						continue;
					}
					break;
				default:
					break;
				}
				DOPUTC2(etoa[c])
			}
			if (errno) {
			    goto data_err;
			}

			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_I:
			errno = d= 0 ;
			while ((cnt = read(fileno(instr), buf, sizeof buf)) 
					> 0) {
				byte_count += cnt;
				for(bufp=buf;cnt > 0; cnt -=d, bufp +=d){
				if ((d=write(fileno(outstr), bufp, cnt)) <= 0)
						goto file_err;
				}
			}
			if (cnt < 0)
				goto data_err;
			transflag = 0;
			return (0);

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
                        next_bits = bytesz;
                        out_bits = 0;
                        out_char = '\0';
			while ((c = getc(instr)) != EOF) {
			    byte_count++;
			    space_bits = NBBY - out_bits;
			    avail_bits = NBBY;
			    do {
				useful_bits = MYMIN(space_bits, avail_bits);
				useful_bits = MYMIN(useful_bits, next_bits);
				out_char |= (c & mask[useful_bits]) >> out_bits;
				out_bits += useful_bits;
				avail_bits -= useful_bits;
				next_bits -= useful_bits;
				if (out_bits == NBBY || next_bits == 0) {
				    DOPUTC2(out_char);
				    out_char = '\0';
				    out_bits = 0;
				}
				c = (c << useful_bits);
				if (next_bits <= 0)
				    next_bits = bytesz;
			    } while (avail_bits >= next_bits);
			    out_bits = avail_bits;
			    next_bits -= out_bits;
			    out_char = c & mask[out_bits];
			}
			if (errno) {
			    goto data_err;
			}

		    break;
	
		default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "TYPE", type);
			transflag = 0;
			return (-1);
		}
		break;

	    case STRU_R:
		switch (type) {
		case TYPE_A:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			line_state = 0;
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				if (line_state == DID_ESC) {
					line_state = 0;
					if (c == REC_ESC) {
						DOPUTC2(c)
						continue;
					}
					if (c & REC_EOR) {
						DOPUTC2('\n')
					}
					if (c & REC_EOF) {
						break;
					}
				}
				else {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
				}
			}
			if (errno) {
			    goto data_err;
			}

			break;

		    case FORM_C:
			c = getc(instr);
			byte_count++;
			line_state = IN_LINE;
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				if (line_state == START_OF_LINE) {
				    switch (c) {
				    case REC_ESC:
					DOPUTC2('\n')
					line_state = DID_ESC;
					continue;
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				}
				else if (line_state == IN_LINE) {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
					continue;
				}
				else if (line_state == DID_ESC) {
					if (c == REC_ESC) {
						DOPUTC2(c)
						line_state = IN_LINE;
						continue;
					}
					if (c & REC_EOR) {
						line_state = START_OF_LINE;
					}
					if (c & REC_EOF) {
						break;
					}
					continue;
				}
			}
			if (errno) {
			    goto data_err;
			}

			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_E:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			line_state = 0;
			while ((c = getc(instr)) != EOF) {
				c = etoa[c];
				byte_count++;
				if (line_state == DID_ESC) {
					line_state = 0;
					if (c == REC_ESC) {
						DOPUTC2(c)
						continue;
					}
					if (c & REC_EOR) {
						DOPUTC2('\n')
					}
					if (c & REC_EOF) {
						break;
					}
				}
				else {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
				}
			}
			if (errno) {
			    goto data_err;
			}

			break;

		    case FORM_C:
			c = getc(instr);
			byte_count++;
			line_state = IN_LINE;
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				c = etoa[c];
				if (line_state == START_OF_LINE) {
				    switch (c) {
				    case REC_ESC:
					DOPUTC2('\n')
					line_state = DID_ESC;
					continue;
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				}
				else if (line_state == IN_LINE) {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
					continue;
				}
				else if (line_state == DID_ESC) {
					if (c == REC_ESC) {
						DOPUTC2(c)
						line_state = IN_LINE;
						continue;
					}
					if (c & REC_EOR) {
						line_state = START_OF_LINE;
					}
					if (c & REC_EOF) {
						break;
					}
					continue;
				}
			}
			if (errno) {
			    goto data_err;
			}

			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_I:
			line_state = 0;
			while ((c = getc(instr)) != EOF) {
				byte_count++;
				if (line_state == DID_ESC) {
					line_state = 0;
					if (c == REC_ESC) {
						DOPUTC2(c)
						continue;
					}
					if (c & REC_EOF) {
						break;
					}
				}
				else {
					if (c == REC_ESC) {
						line_state = DID_ESC;
						continue;
					}
					DOPUTC2(c)
				}
			}
			if (errno) {
			    goto data_err;
			}

			break;

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
                        next_bits = bytesz;
                        out_bits = 0;
                        out_char = '\0';
			while ((c = getc(instr)) != EOF) {
			    byte_count++;
			    if (c == REC_ESC) {
				c = getc(instr);
			        byte_count++;
				if (c == EOF)
				    break;
				if (c != REC_ESC) {
				    if (c & REC_EOF)
					break;
				    continue;
				}
			    }
			    space_bits = NBBY - out_bits;
			    avail_bits = NBBY;
			    do {
				useful_bits = MYMIN(space_bits, avail_bits);
				useful_bits = MYMIN(useful_bits, next_bits);
				out_char |= (c & mask[useful_bits]) >> out_bits;
				out_bits += useful_bits;
				avail_bits -= useful_bits;
				next_bits -= useful_bits;
				if (out_bits == NBBY || next_bits == 0) {
				    DOPUTC2(out_char);
				    out_char = '\0';
				    out_bits = 0;
				}
				c = (c << useful_bits);
				if (next_bits <= 0)
				    next_bits = bytesz;
			    } while (avail_bits >= next_bits);
			    out_bits = avail_bits;
			    next_bits -= out_bits;
			    out_char = c & mask[out_bits];
			}
			if (errno) {
			    goto data_err;
			}

			break;

		default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "TYPE", type);
			transflag = 0;
			return (-1);
		}
		break;

	    default:
		reply(550,
		    MSGSTR(UNIMPRECV,
		    "Unimplemented %s %d in receive_data"), "STRU", stru);
		transflag = 0;
		return (-1);
	    }
	    break;

	case MODE_B:

#define GETBLKHDR\
	if (fread(buf, 1, 3, instr) != 3)\
		goto data_err;\
	byte_count += 3;\
	block_code = buf[0];\
	block_count = buf[1] << NBBY | buf[2];

#define DORESTART\
	if (block_code & BLK_RESTART) {\
		if (fread(buf, 1, block_count, instr) != block_count)\
		goto data_err;\
		byte_count += block_count;\
		if (fsync(fileno(outstr)) < 0)\
			goto file_err;\
		buf[block_count] = '\0';\
                sprintf(mark_r,\
			"%08d%02d%03d%03d%02d%03d",\
                       	write_count, line_state, car_ctl_char,\
                       	next_bits, out_bits, out_char & 0xff);\
		reply(110, MSGSTR(MARK,\
			"MARK %s = %s"), buf, mark_r);\
		continue;\
	}

	    /* rest_mark and other state vars. are set by the REST command */
	    if (rest_mark > 0) {

		if (fseek(outstr, rest_mark, 0) != 0) {
		    reply(554, MSGSTR(BAD_REST,
		   	"Requested action not taken:  invalid REST parameter"));
		    return(-1);
		}
	    }
            else {
                line_state = START_OF_FILE;
                car_ctl_char = ' ';
                next_bits = bytesz;
                out_bits = 0;
                out_char = '\0';
            }
	    write_count = ftell(outstr);

	    switch (stru) {
	    case STRU_F:
		switch (type) {
		case TYPE_A:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				if (line_state == DID_CR) {
					if (c == '\n')
						line_state = 0;
					else if ( c != '\r') {
						line_state = 0;
						DOPUTC2('\r')
					}
				}
				else {
					if (c == '\r') {
						line_state = DID_CR;
						continue;
					}
				}
				DOPUTC2(c)
			    }
			    if (errno) {
				goto data_err;
			    }
			}
                        if (line_state == DID_CR) {
                                DOPUTC2('\r')
                        }
			break;

		    case FORM_C:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (c) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == '\r') {
						line_state = DID_CR;
						continue;
					}
					break;
				case DID_CR:
					if (c == '\n') {
						line_state = START_OF_LINE;
						continue;
					}
					else if (c != '\r') {
						DOPUTC2('\r')
						line_state = IN_LINE;
					}
					break;
				default:
					break;
				}
				DOPUTC2(c)
			    }
			    if (errno) {
				goto data_err;
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_E:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				if (c == EBCDIC_NL) {
					DOPUTC2('\n')
				}
				else {
					DOPUTC2(etoa[c])
				}
			    }
			    if (errno) {
				goto data_err;
			    }
			}
			break;

		    case FORM_C:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (etoa[c]) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				case IN_LINE:
					if (c == EBCDIC_NL) {
						line_state = START_OF_LINE;
						continue;
					}
					break;
				default:
					break;
				}
				DOPUTC2(etoa[c])
			    }
			    if (errno) {
				goto data_err;
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_I:
			errno = d= 0 ;
			while ((block_code & BLK_EOF) != BLK_EOF) {
			    GETBLKHDR
			    DORESTART
			    while (block_count > 0) {
			        read_count = MYMIN(block_count, sizeof buf);
				if ((cnt = fread(buf, 1, read_count, instr)) 
					> 0) {
				    block_count -= cnt;
				    byte_count += cnt;
				    write_count += cnt;
				    for(bufp=buf;cnt > 0; cnt -=d, bufp +=d) {
					if ((d = write(fileno(outstr), 
						bufp, cnt)) <= 0)
					goto file_err;
				    }
				}
				else
				    goto data_err;
			    }
			}
			transflag = 0;
			return (0);

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				space_bits = NBBY - out_bits;
				avail_bits = NBBY;
				do {
				    useful_bits = MYMIN(space_bits, avail_bits);
				    useful_bits = MYMIN(useful_bits, next_bits);
				    out_char |= (c & mask[useful_bits]) 
					>> out_bits;
				    out_bits += useful_bits;
				    avail_bits -= useful_bits;
				    next_bits -= useful_bits;
				    if (out_bits == NBBY || next_bits == 0) {
					DOPUTC2(out_char);
					out_char = '\0';
					out_bits = 0;
				    }
				    c = (c << useful_bits);
				    if (next_bits <= 0)
					next_bits = bytesz;
				} while (avail_bits >= next_bits);
				out_bits = avail_bits;
				next_bits -= out_bits;
				out_char = c & mask[out_bits];
			    }
			    if (errno) {
				goto data_err;
			    }			    
			}
		    break;

		default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "TYPE", type);
			transflag = 0;
			return (-1);
		}
		break;

	    case STRU_R:
		switch (type) {
		case TYPE_A:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				DOPUTC2(c)
			    }
			    if (errno) {
				goto data_err;
			    }
			    if (block_code & BLK_EOR) {
				DOPUTC2('\n')
			    }
			}
			break;

		    case FORM_C:
			while ((block_code != BLK_EOF) && (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (c) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				default:
					break;
				}
				DOPUTC2(c)
			    }
			    if (errno) {
				goto data_err;
			    }
			    if (block_code & BLK_EOR) {
				line_state = START_OF_LINE;
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_E:
		    switch (form) {
		    case FORM_N:
		    case FORM_T:
			while ((block_code != BLK_EOF) && (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				DOPUTC2(etoa[c])
			    }
			    if (errno) {
				goto data_err;
			    }
			    if (block_code & BLK_EOR) {
				DOPUTC2('\n')
			    }
			}
			break;

		    case FORM_C:
			while ((block_code != BLK_EOF) && (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				switch (line_state) {
				case START_OF_FILE:
				    line_state = IN_LINE;
				    continue;
				case START_OF_LINE:
				    switch (etoa[c]) {
				    case '1':
					DOPUTC2('\n')
					DOPUTC2('\f')
					break;
				    case ' ':
					DOPUTC2('\n')
					break;
				    case '+':
					DOPUTC2('\r')
					break;
				    default:
					DOPUTC2('\n')
					break;
				    }
				    line_state = IN_LINE;
				    continue;
				default:
					break;
				}
				DOPUTC2(etoa[c])
			    }
			    if (errno) {
				goto data_err;
			    }
			    if (block_code & BLK_EOR) {
				line_state = START_OF_LINE;
			    }
			}
			if (line_state == START_OF_LINE) {
				DOPUTC2('\n')
			}
			break;

		    default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "FORM", form);
			transflag = 0;
			return (-1);
		    }
		    break;

		case TYPE_I:
			errno = d= 0 ;
			while ((block_code & BLK_EOF) != BLK_EOF) {
			    GETBLKHDR
			    DORESTART
			    while (block_count > 0) {
			        read_count = MYMIN(block_count, sizeof buf);
				if ((cnt = fread(buf, 1, read_count, instr)) 
					> 0) {
				    block_count -= cnt;
				    byte_count += cnt;
				    write_count += cnt;
				    for(bufp=buf;cnt > 0; cnt -=d, bufp +=d) {
					if ((d = write(fileno(outstr), 
						bufp, cnt)) <= 0)
					goto file_err;
				    }
				}
				else
				    goto data_err;
			    }
			}
			transflag = 0;
			return (0);

		case TYPE_L:
			/*
			 * Data is assumed to be stored so that each logical
			 * byte takes an integral number of storage bytes.
			 * For example, if the logical byte size were 36,
			 * each logical byte would take 5 storage bytes with
			 * the last 4 bits of the 5th byte unused.
			 */
			while (((block_code & BLK_EOF) != BLK_EOF) 
				&& (c != EOF)) {
			    GETBLKHDR
			    DORESTART
			    while ((block_count-- > 0) && 
				((c = getc(instr)) != EOF)) {
				byte_count++;
				space_bits = NBBY - out_bits;
				avail_bits = NBBY;
				do {
				    useful_bits = MYMIN(space_bits, avail_bits);
				    useful_bits = MYMIN(useful_bits, next_bits);
				    out_char |= (c & mask[useful_bits]) 
					>> out_bits;
				    out_bits += useful_bits;
				    avail_bits -= useful_bits;
				    next_bits -= useful_bits;
				    if (out_bits == NBBY || next_bits == 0) {
					DOPUTC2(out_char);
					out_char = '\0';
					out_bits = 0;
				    }
				    c = (c << useful_bits);
				    if (next_bits <= 0)
					next_bits = bytesz;
				} while (avail_bits >= next_bits);
				out_bits = avail_bits;
				next_bits -= out_bits;
				out_char = c & mask[out_bits];
			    }
			    if (errno) {
				goto data_err;
			    }
			}
		    break;

		default:
			reply(550,
			    MSGSTR(UNIMPRECV,
			    "Unimplemented %s %d in receive_data"), "TYPE", type);
			transflag = 0;
			return (-1);
		}
		break;

	    default:
		reply(550,
		    MSGSTR(UNIMPRECV,
		    "Unimplemented %s %d in receive_data"), "STRU", stru);
		transflag = 0;
		return (-1);
	    }
	    break;

	default:
		reply(550,
		    MSGSTR(UNIMPRECV,
		    "Unimplemented %s %d in receive_data"), "MODE", mode);
		transflag = 0;
		return (-1);
	}
	fflush(outstr);
	if (ferror(instr))
		goto data_err;
	if (ferror(outstr))
		goto file_err;
	transflag = 0;
	return (0);


data_err:
	transflag = 0;
	perror_reply(426, MSGSTR(DATACONNERR, "Data connection"));
	return (-1);

file_err:
	transflag = 0;
	perror_reply(452, MSGSTR(ERRWRITE, "Error writing file"));
	return (-1);
}

statfilecmd(filename)
	char *filename;
{
	struct stat st;
	char line[BUFSIZ];
	FILE *fin;
	int c, rcode;

        if (stat(filename, &st) < 0) {
                perror_reply(550, filename);
                return;
        }
        if ((st.st_mode&S_IFMT) == S_IFDIR)
		rcode = 212;
        else
		rcode = 213;
	(void) sprintf(line, "/bin/ls -lgA %s", filename);
	fin = ftpd_popen(line, "r");
	lreply(rcode, "status of %s:", filename);
	while ((c = getc(fin)) != EOF) {
		if (c == '\n') {
			if (ferror(stdout)){
				perror_reply(421, MSGSTR(CNTRCONN, "control connection"));
				(void) ftpd_pclose(fin);
				dologout(1);
				/* NOTREACHED */
			}
			if (ferror(fin)) {
				perror_reply(551, filename);
				(void) ftpd_pclose(fin);
				return;
			}
			(void) putc('\r', stdout);
		}
		(void) putc(c, stdout);
	}
	(void) ftpd_pclose(fin);
	reply(rcode, MSGSTR(ENDSTATUS,"End of Status"));
}

statcmd()
{
	struct sockaddr_in *sin;
	u_char *a, *p;

	lreply(211, MSGSTR(SERVSTATUS, "%s FTP server status:"), hostname, version);
	printf("     %s\r\n", version);
	printf(MSGSTR(CONN, "     Connected to %s"), remotehost);
	if (isdigit(remotehost[0]))
		printf(" (%s)", inet_ntoa(his_addr.sin_addr));
	printf("\r\n");
	if (logged_in) {
		if (guest)
		    printf(MSGSTR(ANONYM, "     Logged in anonymously\r\n"));
		else
		    printf(MSGSTR(USERLOG,"     Logged in as %s\r\n"), pw->pw_name);
	} else if (askpasswd)
		printf(MSGSTR(WAITPWD, "     Waiting for password\r\n"));
	else
		printf(MSGSTR(WAITNAME, "     Waiting for user name\r\n"));
#ifdef never
	printf("     TYPE: %s", typenames[type]);
	if (type == TYPE_A || type == TYPE_E)
		printf(", FORM: %s", formnames[form]);
	if (type == TYPE_L)
#if NBBY == 8
		printf(" %d", NBBY);
#else
		printf(" %d", bytesize);	/* need definition! */
#endif
	printf("; STRUcture: %s; transfer MODE: %s\r\n",
	    strunames[stru], modenames[mode]);
#endif /* never */
	if (data != -1)
		printf(MSGSTR(DATACONN1,"     Data connection open\r\n"));
	else if (pdata != -1) {
		printf(MSGSTR(PASSIVE, "     in Passive mode"));
		sin = &pasv_addr;
		goto printaddr;
	} else if (usedefault == 0) {
		printf("     PORT");
		sin = &data_dest;
printaddr:
		a = (u_char *) &sin->sin_addr;
		p = (u_char *) &sin->sin_port;
#define UC(b) (((int) b) & 0xff)
		printf(" (%d,%d,%d,%d,%d,%d)\r\n", UC(a[0]),
			UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
#undef UC
	} else
		printf(MSGSTR(NODATACONN, "     No data connection\r\n"));
	reply(211, MSGSTR(ENDSTATUS, "End of status"));
}

fatal(s)
	char *s;
{
	reply(451, MSGSTR(ERRSERV, "Error in server: %s\n"), s);
	reply(221, MSGSTR(CLOSECONN,"Closing connection due to server error."));
#ifdef _CSECURITY
	GET_PRIV(saved_uid);
        CONNECTION_WRITE(remote_addr, "ftp/tcp", "close",
	    MSGSTR(CLOSECONN,"Closing connection due to server error."), -1);
	DROP_PRIV(effective_uid);
#endif _CSECURITY
	dologout(0);
	/* NOTREACHED */
}

/* VARARGS2 */
reply(n, fmt, p0, p1, p2, p3, p4, p5)
	int n;
	char *fmt;
{
	printf("%d ", n);
	printf(fmt, p0, p1, p2, p3, p4, p5);
	printf("\r\n");
	(void)fflush(stdout);
	if (debug) {
		syslog(LOG_DEBUG, "<--- %d ", n);
		syslog(LOG_DEBUG, fmt, p0, p1, p2, p3, p4, p5);
	}
}

/* VARARGS2 */
lreply(n, fmt, p0, p1, p2, p3, p4, p5)
	int n;
	char *fmt;
{
	printf("%d- ", n);
	printf(fmt, p0, p1, p2, p3, p4, p5);
	printf("\r\n");
	(void)fflush(stdout);
	if (debug) {
		syslog(LOG_DEBUG, "<--- %d- ", n);
		syslog(LOG_DEBUG, fmt, p0, p1, p2, p3, p4, p5);
	}
}

ack(s)
	char *s;
{
	reply(250, MSGSTR(CMDOK, "%s command successful."), s);
}

nack(s)
	char *s;
{
	reply(502, MSGSTR(NOCMD, "%s command not implemented."), s);
}

/* ARGSUSED */
yyerror(s)
        char *s;
{
        char *cp;
        char    tmp_cbuf[512];

        strcpy(tmp_cbuf, cbuf);
        if (cp = index(tmp_cbuf,'\n')) {
            *cp = '\0';
        }

        reply(500, MSGSTR(CMDUNK, "'%s': command not understood."), tmp_cbuf);
}

delete(name)
	char *name;
{
	struct stat st;
	int fd;
 
	/* First we check that the file is not busy
	 * before we delet it.
	 */
	if ((fd = open(name, O_RDONLY|O_NSHARE)) < 0) {
		if (errno == ETXTBSY)
			perror_reply(450, name);
		else
			perror_reply(550, name);
		close(fd);
		return;
	}
	close(fd);
 
	if (stat(name, &st) < 0) {
		perror_reply(550, name);
		return;
	}
	if ((st.st_mode&S_IFMT) == S_IFDIR) {
		if (rmdir(name) < 0) {
			perror_reply(550, name);
			return;
		}
		goto done;
	}
	if (unlink(name) < 0) {
		perror_reply(550, name);
		return;
	}
done:
	ack("DELE");
}

cwd(path)
	char *path;
{
	if (chdir(path) < 0)
		perror_reply(550, path);
	else
		ack("CWD");
}

makedir(name)
	char *name;
{
	if (mkdir(name, 0777) < 0)
		perror_reply(550, name);
	else
		reply(257, MSGSTR(MKD_OK, "MKD command successful."));
}

removedir(name)
	char *name;
{
	if (rmdir(name) < 0)
		perror_reply(550, name);
	else
		ack("RMD");
}

pwd()
{
	char path[MAXPATHLEN + 1];
	extern char *getwd();

	if (getwd(path) == (char *)NULL)
		reply(550, "%s.", path);
	else
		reply(257, MSGSTR(CURRDIR, "\"%s\" is current directory."), path);
}

char *
renamefrom(name)
	char *name;
{
	struct stat st;
	int fd;
 
	/* We first check to see that the file is not busy before
	 * renaming the file.
	 */
	if ((fd = open(name, O_RDONLY|O_NSHARE)) < 0) {
		if (errno == ETXTBSY)
			perror_reply(450, name);
		else
			perror_reply(550, name);
		close(fd);
		return ((char *)0);
	}
	close(fd);

	if (stat(name, &st) < 0) {
		perror_reply(550, name);
		return ((char *)0);
	}
	reply(350,MSGSTR(FILEEXIST, "File exists, ready for destination name"));
	return (name);
}

renamecmd(from, to)
	char *from, *to;
{
	int fd;
 
	/* We first check to see that the files are not busy before
	 * renaming the files.
	 */
	if ((fd = open(from, O_RDONLY|O_NSHARE)) < 0) {
		if (errno == ETXTBSY)
			perror_reply(450, from);
		else
			perror_reply(550, from);
		close(fd);
		return;
	}
	close(fd);
	if (rename(from, to) < 0)
		perror_reply(550, "rename");
	else
		ack("RNTO");
}

dolog(sin)
	struct sockaddr_in *sin;
{
	struct hostent *hp = gethostbyaddr((char *)&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);
	time_t t, time();
	extern char *ctime();

	if (hp)
		(void) strncpy(remotehost, hp->h_name, sizeof (remotehost));
	else
		(void) strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
#ifdef SETPROCTITLE
	sprintf(proctitle, "%s: connected", remotehost);
	setproctitle(proctitle);
#endif /* SETPROCTITLE */

	if (logging) {
		t = time((time_t *) 0);
		syslog(LOG_INFO, "connection from %s at %s",
		    remotehost, ctime(&t));
	}
}

/*
 * Record logout in wtmp file
 * and exit with supplied status.
 */
dologout(status)
	int status;
{
	if (logged_in) {
/* New Security Code (void) seteuid(saved_uid); */
		GET_PRIV(saved_uid);
		logwtmp(ttyline, "", "");
	}
	/* beware of flushing buffers after a SIGPIPE */
	_exit(status);
}

myoob()
{
	char *cp;

	/* only process if transfer occurring */
	if (!transflag)
		return;
	cp = tmpline;
	if (getline(cp, 7, stdin) == NULL) {
		reply(221, MSGSTR(GOODBYE, "You could at least say goodbye."));
#ifdef _CSECURITY
	GET_PRIV(saved_uid);
        CONNECTION_WRITE(remote_addr, "ftp/tcp", "close",
                 MSGSTR(GOODBYE, "You could at least say goodbye."), -1);
	DROP_PRIV(effective_uid);
#endif _CSECURITY
		dologout(0);
	}
	upper(cp);
	if (strcmp(cp, "ABOR\r\n") == 0) {
		tmpline[0] = '\0';
		reply(426, MSGSTR(CONNABORT, "Transfer aborted. Data connection closed."));
		reply(226, MSGSTR(ABORT, "Abort successful"));
		longjmp(urgcatch, 1);
	}
	if (strcmp(cp, "STAT\r\n") == 0) {
		if (file_size != (off_t) -1)
			reply(213, MSGSTR(STATUS1, "Status: %lu of %lu bytes transferred"),
			    byte_count, file_size);
		else
			reply(213, MSGSTR(STATUS2, "Status: %lu bytes transferred"), byte_count);
	}
}

/*
 * Note: a response of 425 is not mentioned as a possible response to
 * 	the PASV command in RFC959. However, it has been blessed as
 * 	a legitimate response by Jon Postel in a telephone conversation
 *	with Rick Adams on 25 Jan 89.
 */
passive()
{
	int len;
	register char *p, *a;

	pdata = socket(AF_INET, SOCK_STREAM, 0);
	if (pdata < 0) {
		perror_reply(425, MSGSTR(NOPASSCONN, "Can't open passive connection"));
		return;
	}
	pasv_addr = ctrl_addr;
	pasv_addr.sin_port = 0;
/* New Security Code (void) seteuid((uid_t)0); */
	GET_PRIV(saved_uid);
	if (bind(pdata, (struct sockaddr *)&pasv_addr, sizeof(pasv_addr)) < 0) {
/* New Security Code (void) seteuid((uid_t)pw->pw_uid); */
		DROP_PRIV(effective_uid);
		goto pasv_error;
	}
/* New Security Code (void) seteuid((uid_t)pw->pw_uid); */
	DROP_PRIV(effective_uid);
	len = sizeof(pasv_addr);
	if (getsockname(pdata, (struct sockaddr *) &pasv_addr, &len) < 0)
		goto pasv_error;
	if (listen(pdata, 1) < 0)
		goto pasv_error;
	a = (char *) &pasv_addr.sin_addr;
	p = (char *) &pasv_addr.sin_port;

#define UC(b) (((int) b) & 0xff)

	reply(227, MSGSTR(ENTRPASS, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)"), UC(a[0]),
		UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
	return;

pasv_error:
	(void) close(pdata);
	pdata = -1;
	perror_reply(425, MSGSTR(NOPASSCONN, "Can't open passive connection"));
	return;
}

/*
 * Generate unique name for file with basename "local".
 * The file named "local" is already known to exist.
 * Generates failure reply on error.
 */
char *
gunique(local)
	char *local;
{
	static char new[MAXPATHLEN];
	struct stat st;
	char *cp = rindex(local, '/');
	int count = 0;

	if (cp)
		*cp = '\0';
	if (stat(cp ? local : ".", &st) < 0) {
		perror_reply(553, cp ? local : ".");
		return((char *) 0);
	}
	if (cp)
		*cp = '/';
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	for (count = 1; count < 100; count++) {
		(void) sprintf(cp, "%d", count);
		if (stat(new, &st) < 0)
			return(new);
	}
	reply(452, MSGSTR(NOFILENAME, "Unique file name cannot be created."));
	return((char *) 0);
}

/*
 * Format and send reply containing system error number.
 */
perror_reply(code, string)
	int code;
	char *string;
{
	if (errno < sys_nerr)
		reply(code, "%s: %s.", string, sys_errlist[errno]);
	else
		reply(code, MSGSTR(UNKERR, "%s: unknown error %d."), string, errno);
}

static char *onefile[] = {
	"",
	0
};

send_file_list(whichfiles)
	char *whichfiles;
{
	struct stat st;
	DIR *dirp = NULL;
	struct direct *dir;
	FILE *dout = NULL;
	char **dirlist, *dirname;
	char *strpbrk();
	int simple=0;

	if (strpbrk(whichfiles, "~{[*?") != NULL) {
		char *cp;
		glob_t globbuf;
		int j;
                /*
                 * First we need to do tilde expansion since glob()
                 * in libc.a is to dumb to do it for us.
                 */
                cp = whichfiles;
                if (*cp == '~') {       /* we found a tilde */
                        cp++;
                        if (*cp == '\0' || *cp == '/') {
                                /*
                                 * we found a null,
                                 * or / after the tilde.
                                 */
                                strcpy(tempstr, home);
                                strcat(tempstr, cp);
                        } else {
                                /* we might of found a user after the tilde */
                                for (j = 0; j < sizeof(tempuser) &&
                                                 *cp != '/' && *cp != '\0'; j++)
                                        tempuser[j] = *cp++;
                                tempuser[j] = '\0';
                                if (gethdir(tempuser)) {
					reply(550, MSGSTR(UNKNOWN_USER,
                                               "Unknown user name after ~\n"));
					return;
                                } else {
                                        strcpy(tempstr, tempuser);/* user dir */
                                        strcat(tempstr, cp); /* rest of path */
                                }
                        }
                } else
                        strcpy(tempstr, cp);

		if (strpbrk(tempstr, "*?[{")) {
			/* we have glob characters in tempstr */
			if (glob(tempstr, GLOB_QUOTE, NULL,&globbuf) != 0) {
				/* globbing failed */
				reply(550, MSGSTR(BAD_DIR,
						"Bad directory components"));
				return;
			}
			if (globbuf.gl_pathc == 0) {
				/* globbing failed */
				errno = ENOENT;
				perror_reply(550, whichfiles);
				return;
			}
			dirlist = (char **) malloc((unsigned) sizeof(dirlist)
                                                            * globbuf.gl_pathc);
			for (j = 0; j < globbuf.gl_pathc &&
					globbuf.gl_pathv[j] != NULL; j++) {
				if ((dirlist[j] = (char *) malloc((unsigned)
                                       strlen(globbuf.gl_pathv[j])+1)) != NULL)
                                strcpy(dirlist[j], globbuf.gl_pathv[j]);
			}
			dirlist[j] = NULL;
		} else {
		        dirlist = (char **) malloc((unsigned) (sizeof(char*) * 2));
			dirlist[0] = tempstr;
			dirlist[1] = NULL;
		}
	} else {
		onefile[0] = whichfiles;
		dirlist = onefile;
		simple=1;
	}

	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}
	while (dirname = *dirlist++) {
		if (stat(dirname, &st) < 0) {
			/*
			 * If user typed "ls -l", etc, and the client
			 * used NLST, do what the user meant.
			 */
			if (dirname[0] == '-' && *dirlist == NULL &&
			    transflag == 0) {
				retrieve("/bin/ls %s", dirname);
				return;
			}
			perror_reply(550, whichfiles);
			if (dout != NULL) {
				(void) fclose(dout);
				transflag = 0;
				data = -1;
				pdata = -1;
			}
			return;
		}

		if ((st.st_mode&S_IFMT) == S_IFREG) {
			if (dout == NULL) {
				dout = dataconn(whichfiles, (off_t)-1, "w", 0);
				if (dout == NULL)
					return;
				transflag++;
			}
			fprintf(dout, "%s\r\n", dirname);
			byte_count += strlen(dirname) + 1;
			continue;
		} else if ((st.st_mode&S_IFMT) != S_IFDIR)
			continue;

		if ((dirp = opendir(dirname)) == NULL)
			continue;

		while ((dir = readdir(dirp)) != NULL) {
			char nbuf[MAXPATHLEN];

			if (dir->d_name[0] == '.' && dir->d_namlen == 1)
				continue;
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' &&
			    dir->d_namlen == 2)
				continue;

			sprintf(nbuf, "%s/%s", dirname, dir->d_name);

			/*
			 * We have to do a stat to insure it's
			 * not a directory or special file.
			 */
			if (simple || stat(nbuf, &st) == 0 &&
			    (st.st_mode&S_IFMT) == S_IFREG) {
				if (dout == NULL) {
					dout = dataconn(whichfiles, (off_t)-1,
						"w", 0);
					if (dout == NULL)
						return;
					transflag++;
				}
				if (nbuf[0] == '.' && nbuf[1] == '/')
					fprintf(dout, "%s\n", &nbuf[2]);
				else
					fprintf(dout, "%s\n", nbuf);
				byte_count += strlen(nbuf) + 1;
			}
		}
		(void) closedir(dirp);
	}

	if (dout == NULL)
		reply(550, "No files found.");
	else if (ferror(dout) != 0)
		perror_reply(550, MSGSTR(DATACONNERR, "Data connection"));
	else
		reply(226, MSGSTR(TRANSOK, "Transfer complete."));

	transflag = 0;
	if (dout != NULL)
		(void) fclose(dout);
	data = -1;
	pdata = -1;
}

#ifdef SETPROCTITLE
/*
 * clobber argv so ps will show what we're doing.
 * (stolen from sendmail)
 * warning, since this is usually started from inetd.conf, it
 * often doesn't have much of an environment or arglist to overwrite.
 */

/*VARARGS1*/
setproctitle(fmt, a, b, c)
char *fmt;
{
	register char *p, *bp, ch;
	register int i;
	char buf[BUFSIZ];

	(void) sprintf(buf, fmt, a, b, c);

	/* make ps print our process name */
	p = Argv[0];
	*p++ = '-';

	i = strlen(buf);
	if (i > LastArgv - p - 2) {
		i = LastArgv - p - 2;
		buf[i] = '\0';
	}
	bp = buf;
	while (ch = *bp++)
		if (ch != '\n' && ch != '\r')
			*p++ = ch;
	while (p < LastArgv)
		*p++ = ' ';
}
#endif /* SETPROCTITLE */
/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 */
int
gethdir(char *home)
{
        register struct passwd *pp = getpwnam(home);

        if (pp == 0)
                return (1);
        (void) strcpy(home, pp->pw_dir);
        return (0);
}
