static char sccsid[] = "@(#)17	1.66.2.15  src/tcpip/usr/lbin/telnetd/telnetd.c, tcptelnet, tcpip41J, 9510A 2/21/95 21:08:23";
/* 
 * COMPONENT_NAME: TCPIP telnetd.c
 * 
 * FUNCTIONS: MSGSTR, NIACCUM, SB_ACCUM, SB_CLEAR, SB_EOF, SB_GET, 
 *            SB_TERM, SCMPN, SCPYN, askterminal, cleanup, doit, 
 *            dontoption, dooption, expecthups, fatal, fatalperror, 
 *            getopt, getterminaltype, goodterminal, hupcatch, if, 
 *            interrupt, mode, mydebug, mydebugc, netclear, 
 *            netflush, nextitem, ptyflush, rmut, sdebug, sendbrk, 
 *            sequenceIs, settimer, slave_logged_in, slave_state, 
 *            stilloob, suboption, switch, tc_put, telnet, telrcv, 
 *            trace_handler, ttloop, willoption, wontoption 
 *	      gotsak, runtsh, setdefsenv, tpath, check_pty_sak, getamsg
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
#ifdef _AIX
 * _AIX port of 4.3BSD telnet server
#endif _AIX
 */
#undef DEBUG
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef NEVER
static char sccsid[] = "telnetd.c	5.18 (Berkeley) 5/12/86";
#endif not lint
*/

/*
 * Telnet server.
 */
#ifdef _AIX
/*
 * _AIX port of 4.3 server	10/3/87		Mark Carson
 *
 * I had to make a few changes to get this to work with _AIX:
 *
 *	1. The BSD server directly starts up login on the slave side of the
 *	pty.  This is nicer, since that way it can pass the terminal type
 *	in through the environment, and pty's don't have to be tied up for
 *	remote logins.  Unfortunately, this doesn't work right with our
 *	implementation of the SAK, where we assume the login process is
 *	a direct child of init.  So I adopted the old _AIX procedure of
 *	requiring the slave side to be enabled, so getty/login should
 *	already be there.
 *
 *	2. The _AIX server has got some stuff built in for the hft.  This is
 *	a little suboptimal - it seems like everyone (telnet client and server,
 *	remote application, whatever) gets their fingers in on the hft support.
 *	However, it seems unavoidable.
 *
 *	3. I fixed up the terminal type negotiation so that it works properly
 *	(within limits, since we can't pass the type onto login) with the
 *	old _AIX client, the old BSD client, and anyone following RFC 1010.
 *	To do this, I added a snazzy telnet configuration file to translate
 *	terminal names.
 *
 *	4. There were a variety of changes due to the different tty scheme
 *	in BSD and _AIX (SysV).  I made up a header to map as many of these as
 *	possible -- this is what should be in the _AIX sgtty.h, but for some
 *	reason isn't.  I still had to do some ifdef'ing.
 *
#ifdef _CSECURITY
 *	1. For C2 _AIX, I added SAK support.  The client asks about doing SAK by
 *	sending IAC DO TELOPT_SAK, to which we reply IAC WILL TELOPT_SAK.  SAK
 *	is then signalled from the client side by sending IAC SAK.  (We don't
 *	require asking TELOPT_SAK before sending it, I guess.)
 *	2. Some hacking is needed to ensure we live with the hangups we get
 *	from init when it starts the trusted shell. Doing this also revealed
 *	a bug in init, in that it restarts the login shell after the trusted
 *	shell no matter what the cause of trusted shell death was, including
 *	hangups. I'm presuming now this will be fixed in init - I'm fixing it
 *	in the Secure Xenix init.
#endif _CSECURITY
 */
#endif _AIX
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/lockf.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/telnet.h>


#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <utmp.h>
#include <login.h>
#include <usersec.h>
#include <sys/priv.h>
#include <sys/termio.h>
#include <sys/tty.h>
#include <sys/access.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <ctype.h>
/* 
 * The following includes and variable declerations are for support of
 * the System Resource Controller (SRC) and the Object Data Manager (ODM)
 * which were added to AIX v3.1 - 
 */
#include	<sys/ipc.h>
#include	<sys/shm.h>
#include	"libsrc.h"
#include 	<arpa/nameser.h>           /* OCS */
#include        "lm.h"                     /* OCS */
#include    	"ocsvhost.h"               /* OCS */
int	shared_mem_exists;
struct	telnetd_entry *telentry;
key_t	telmemkey;
int	telmemid;
int	telflags = IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | 
	           S_IROTH | S_IWOTH;

#include <nl_types.h>
#include "telnetd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TELNETD,n,s) 
/*  The following are used to retrieve the sys_errlist messages from an NLS
    catalog for the local version of perror */
#define MF_LIBC         "libc.cat"
#define MS_LIBC         1
/* routine to read out of the libc.cat */
char *
getamsg(cat, set, msg, def)
	char *cat;
	long set;
	long msg;
	char *def;
{
	nl_catd catd2;
	char	*message;
	catd2 = catopen(cat, NL_CAT_LOCALE);
	message = catgets(catd2, set, msg, def);
	catclose(catd);
	return(message);
}


#ifdef _CSECURITY
#include <sys/pty.h>
#ifdef	AIX221
#include "tcpipaudit.h"
#else	AIX221
#include "tcpip_audit.h"
#endif	AIX221
#endif _CSECURITY

#define ESC	0x1b
#define NOCHAR  '\377'

#define DEF_FAILAUTH    "Sorry. Failed authentication.\n"
#define DEF_FAILCRED    "Sorry. Failed setting credentials.\n"
#define DEF_FAILEXEC    "Sorry. Failed running login shell.\n"
#define DEF_FAILTERM    "Sorry. Failed setting terminal ownership and mode.\n"
#define DEF_NOTLOWEST   "Sorry. You must \"exec\" login from the lowest login shell."
#define DEF_UTMP	"Active tty entry not found in utmp file.\n"

#define LOGIN_SUCCESSFUL        0               /* login audit flags */
#define LOGIN_FAILED            1

struct utmp utmp;
#define UT_NAMESIZE     sizeof(utmp.ut_user)
#define	UTSIZ		(sizeof (struct utmp))

int     failures, quietlog = 0, stopmotd = 0, timeout = 60;
int	check_pty_sak(char *);
static void     exitlogin(), auditlogin(), loginlog(char *, char *);

#define	OPT_NO			0		/* won't do this option */
#define	OPT_YES			1		/* will do this option */
#define	OPT_YES_BUT_ALWAYS_LOOK	2
#define	OPT_NO_BUT_ALWAYS_LOOK	3
char	hisopts[256];
char	myopts[256];

char	doopt[] = { IAC, DO, 0 };
char	dont[] = { IAC, DONT, 0 };
char	will[] = { IAC, WILL, 0 };
char	wont[] = { IAC, WONT, 0 };

/*
 * I/O data buffers, pointers, and counters.
 */
char	ptyibuf[BUFSIZ], *ptyip = ptyibuf;

char	ptyobuf[BUFSIZ], *pfrontp = ptyobuf, *pbackp = ptyobuf;

char	netibuf[BUFSIZ], *netip = netibuf;
#define	NIACCUM(c)	{   *netip++ = c; \
			    ncc++; \
			}

char	netobuf[BUFSIZ], *nfrontp = netobuf, *nbackp = netobuf;
char	*neturg = 0;		/* one past last bye of urgent data */
	/* the remote system seems to NOT be an old 4.2 */
int	not42 = 1;

#ifdef ADEBUG
FILE *debugfp;
#define	mydebug(a)	fputs(a, debugfp)
#define	mydebugc(a)	putc(a, debugfp)
#endif ADEBUG

char	MSGBANNER[] = "\r\n\r\n telnet (%s)\r\n";
char	MSGBANN2[] = "\r\0\r\n\r\0";

		/* buffer for sub-options */
char	subbuffer[100], *subpointer= subbuffer, *subend= subbuffer;
#define	SB_CLEAR()	subpointer = subbuffer;
#define	SB_TERM()	{ subend = subpointer; SB_CLEAR(); }
#ifdef ADEBUG
#define	SB_ACCUM(c)	if (subpointer < (subbuffer+sizeof subbuffer)) { \
				*subpointer++ = (c); \
				mydebugc(c); \
			}
#else ADEBUG
#define	SB_ACCUM(c)	if (subpointer < (subbuffer+sizeof subbuffer)) { \
				*subpointer++ = (c); \
			}
#endif ADEBUG
#define	SB_GET()	((*subpointer++)&0xff)
#define	SB_EOF()	(subpointer >= subend)

int	pcc, ncc;

int	pty, net;
int	inter;
extern	char **environ;
extern	int errno;
char	*line;      
extern	char *ttyname();
int	SYNCHing = 0;		/* we are in TELNET SYNCH mode */
static int unsafe_on = RAW;	/* on modes to ignore until after login */
static int unsafe_off = RAW;	/* off modes to ignore until after login */
/*
 * The following are some clocks used to decide how to interpret
 * the relationship between various variables.
 */

struct {
    int
	system,			/* what the current time is */
	echotoggle,		/* last time user entered echo character */
	modenegotiated,		/* last time operating mode negotiated */
	didnetreceive,		/* last time we read data from network */
	ttypeopt,		/* ttype will/won't received */
	ttypesubopt,		/* ttype subopt negotiation is FINISHED */
	getterminal,		/* time started to get terminal information */
	gotDM;			/* when did we last see a data mark */
} clocks;

#define	settimer(x)	(clocks.x = ++clocks.system)
#define	sequenceIs(x,y)	(clocks.x < clocks.y)

#ifdef _CSECURITY
ulong remote_addr;
char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
char sak_sequence[2]; /* _AIX assumes a two character SAK sequence ^X^R */
#define	FIRST_SAK_KEY		0x18		/* Ctrl-X = 0x18 */
#define	SECOND_SAK_KEY		0x12		/* Ctrl-R = 0x12 */
#endif _CSECURITY

extern int optind;
int tracing = 0;

#include <locale.h>

#ifdef _AIX                                                                     
        char *env[8];     /* size of the environment variable */               
        int  nenv = 0;    /* index into env  */                               
#endif

void trace_handler(int);
char *inet_ntoa();                                 /* OCS */
struct OCSconnection *findvhost(struct sockaddr_in *);  /* OCS */

main(argc, argv)
	int argc;
	char **argv;
{
	int keepalive = 1;
	int ch;
	struct sockaddr_in to;     /* local socket w/ destination addr OCS */
        int tolen;                                                  /* OCS */
	struct sockaddr_in from;
	int on = 1, fromlen;
	struct sigvec sv;
#ifdef _CSECURITY
char    tcpipClass[] = "tcpip\0";
#endif _CSECURITY


        setlocale(LC_ALL,"");
	catd = catopen(MF_TELNETD,NL_CAT_LOCALE);

	openlog("telnetd", LOG_PID | LOG_ODELAY, LOG_DAEMON);
#ifdef _CSECURITY
	if(auditproc(0, AUDIT_EVENTS, tcpipClass, 7) < 0){
		syslog(LOG_ALERT, MSGSTR(CS_AUDITPROC,
			"telnetd: auditproc: %m")); /*MSG*/
		exit(1);
	}
	else if ((auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0)) < 0) {
		syslog(LOG_ALERT, MSGSTR(CS_AUDITPROC,
			"telnetd: auditproc: %m")); /*MSG*/
		exit(1);
	}
	sak_sequence[0] = FIRST_SAK_KEY;	/* Ctrl-X = 0x18 */
	sak_sequence[1] = SECOND_SAK_KEY;	/* Ctrl-R = 0x12 */
#endif _CSECURITY

#if	defined(DEBUG)
	{
	    int s, ns, foo;
	    struct servent *sp;
	    static struct sockaddr_in sin = { AF_INET };

	    sp = getservbyname("telnet", "tcp");
	    if (sp == 0) {
		    fprintf(stderr, "telnetd: tcp/telnet: unknown service\n");
		    exit(1);
	    }
	    sin.sin_port = sp->s_port;
	    argc--, argv++;
	    if (argc > 0) {
		    sin.sin_port = atoi(*argv);
		    sin.sin_port = htons((u_short)sin.sin_port);
	    }

	    s = socket(AF_INET, SOCK_STREAM, 0);
	    if (s < 0) {
		    perror("telnetd: socket");
		    exit(1);
	    }
	    if (bind(s, &sin, sizeof sin) < 0) {
		perror("bind");
		exit(1);
	    }
	    if (listen(s, 1) < 0) {
		perror("listen");
		exit(1);
	    }
	    foo = sizeof sin;
	    ns = accept(s, &sin, &foo);
	    if (ns < 0) {
		perror("accept");
		exit(1);
	    }
	    dup2(ns, 0);
	    close(s);
	}
#endif	/* defined(DEBUG) */
#ifdef ADEBUG
	debugfp = fopen("/dev/console", "w");
#endif ADEBUG

        while ( (optind < argc) && ((ch = getopt(argc, argv, "ns")) != EOF))
                switch (ch) {
                case 'n':
                        keepalive = 0;
                        break;
                case 's':
                        tracing = 1;
                        break;
                case '?':
                default:
                        syslog(LOG_ERR, "usage: telnetd [-n] [-s]");
                        break;
                }

        if (keepalive && 
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0) {
	        syslog(LOG_WARNING, "setsockopt (SO_KEEPALIVE): %m");
        }

        if (tracing && 
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0) {
		syslog(LOG_WARNING, MSGSTR(SSO_DEBUG,
			"setsockopt (SO_DEBUG): %m")); /*MSG*/
	}

	/*
         * SO_REUSEADDR so that we don't have to wait for the socket
         * to free up again
         * OCS
         */
        setsockopt(0, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));  /* OCS */
        /*
         * Obtain destination internet address from the current telnet
         * request(local socket) so that we can determine if this address
         * is defined as a virtual host address in the ODM
         * database.
         * OCS
         */
	tolen = sizeof (to);                              /* OCS */
        if (getsockname(0, &to, &tolen) < 0) {            /* OCS */
#ifdef  MSG
                fprintf(stderr, MSGSTR(ERR_GETSOCK, "%s: getsockname"), argv[0])
;
                perror("");
#else   MSG
                fprintf(stderr, "%s: ", argv[0]);
                perror("getsockname");
#endif  MSG
            _exit(1);					 /* OCS */
        } 						 /* OCS */

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
#ifdef	MSG
		fprintf(stderr, MSGSTR(GPEERNM_ERR, "%s: getpeername"), argv[0]);
		perror("");
#else	MSG
		fprintf(stderr, "%s: ", argv[0]);
		perror("getpeername");
#endif	MSG
		_exit(1);
	}
#ifdef _CSECURITY
	remote_addr = from.sin_addr.s_addr;
#endif _CSECURITY


/* 
 * The following code creates a shared memory segment that is used to 
 * store information about active telnet connections.  This shared 
 * memory is used by "stinet" to display SRC long status info - 
 */
	shared_mem_exists = TRUE;

	if ((telmemkey = ftok(TELNETD, SRC_FD)) == (key_t) -1) {
		syslog(LOG_ERR, MSGSTR(SRC1,
			"telnetd: ftok for shared memory failed.\n"));
		shared_mem_exists = FALSE;
	}

	if ((telmemid = shmget(telmemkey,sizeof(struct telnetd_entry),telflags)) < 0) {
		syslog(LOG_ERR, MSGSTR(SRC2,
			"telnetd: shmget for shared memory failed.\n"));
		shared_mem_exists = FALSE;
	}

	if ((telentry = (struct telnetd_entry *) shmat(telmemid, 0, 0)) < (struct telnetd_entry *) 0) {
		syslog(LOG_ERR, MSGSTR(SRC3,
			"telnetd: shmat for shared memory failed.\n"));
		shared_mem_exists = FALSE;
	}

	/* initialize attached memory */
	if (shared_mem_exists)
		reset_shared_memory(TRUE);

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = (void (*)(int)) trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = (void (*)(int)) trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);

	doit(0, &from, &to);			   /* OCS */
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
		syslog(LOG_WARNING,MSGSTR(SSO_DEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/
}

/* 
 * Save pid and terminal type for SRC long status.
 */
save_telnetd_termtype(termtype)
	char	*termtype;
{
	int x;

	reset_shared_memory(FALSE);
	for (x = 0; x < MAXCONN; x++) {
	        if (telentry->conn[x].pid == -1) {
	                telentry->conn[x].pid = getpid();
			strcpy(telentry->conn[x].termtype,termtype);
		}
	}
}
/* 
 * Check shared memory pid's to make sure they are still active.
 * If not, reset the pid to -1 to make the memory available for a new      
 * process.  This is for SRC support - .
 */
reset_shared_memory(first_time)
	int	first_time;
{
	int x;

	for (x = 0; x < MAXCONN; x++) {
	        if (telentry->conn[x].pid != -1 || first_time) 
	            if ((kill(telentry->conn[x].pid,0) < 0) || first_time) {
	                telentry->conn[x].pid = -1;
			strcpy(telentry->conn[x].termtype, "");
		    }
	}
}


char	*terminaltype = 0;
char    tbuf[MAXPATHLEN + 2];
struct  hostent *hp;
char	*envinit[2];
void	cleanup(int);

/*
 * ttloop
 *
 *	A small subroutine to flush the network output buffer, get some data
 * from the network, and pass it through the telnet state machine.  We
 * also flush the pty input buffer (by dropping its data) if it becomes
 * too full.
 */

void
ttloop()
{

    if (nfrontp-nbackp) {
	netflush();
    }
    ncc = read(net, netibuf, sizeof netibuf);
    if (ncc < 0) {
	syslog(LOG_INFO, MSGSTR(TTLOOP_READ, "ttloop:  read: %m\n")); /*MSG*/
	exit(1);
    } else if (ncc == 0) {
	syslog(LOG_INFO, MSGSTR(TTLOOP_P_DIED, "ttloop:  peer died: %m\n")); /*MSG*/
#ifdef _CSECURITY
	CONNECTION_WRITE(remote_addr, "telnet/tcp", "close",
		MSGSTR(C_PEER_DIED, "peer died"), -1); /*MSG*/
#endif _CSECURITY
	exit(1);
    }
    netip = netibuf;
    telrcv(0);			/* state machine */ /* OCS */
    if (ncc > 0) {
	pfrontp = pbackp = ptyobuf;
	telrcv(0);		/* OCS */
    }
}

/*
 * getterminaltype
 *
 *	Ask the other end to send along its terminal type.
 * Output is the variable terminaltype filled in.
 */

void
getterminaltype()
{
    static char sbuf[] = { IAC, DO, TELOPT_TTYPE };

    settimer(getterminal);
    bcopy(sbuf, nfrontp, sizeof sbuf);
    nfrontp += sizeof sbuf;
    hisopts[TELOPT_TTYPE] = OPT_YES_BUT_ALWAYS_LOOK;
    while (sequenceIs(ttypeopt, getterminal)) {
	ttloop();
    }
    askterminal();
    /* wait here until we finish negotiating termtype */
    while (sequenceIs(ttypesubopt, getterminal)) {
    	ttloop();
    }
}

askterminal()
{
    if (hisopts[TELOPT_TTYPE] == OPT_YES) {
	static char sbbuf[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };

#ifdef ADEBUG
	mydebug("send IAC SB TTYPE SEND IAC SE\n");
#endif ADEBUG
	bcopy(sbbuf, nfrontp, sizeof sbbuf);
	nfrontp += sizeof sbbuf;
    }
}

pid_t childpid;

/*
 * Get a pty, scan input lines.
 * _AIX version is mostly different.
 */

doit(f, who, local)					       /* OCS */
	int f;
	struct sockaddr_in *who;
	struct sockaddr_in *local;   /* local socket address      OCS */
{
	struct winsize w = {0, 0, 0, 0,};
	char *inet_ntoa();
	int p, t, t1;
	struct termios b;
	priv_t priv;
	int c;
	struct OCSconnection *OCScon_ptr;                       /* OCS */
        LM_VTP_VAR tlm;                                         /* OCS */
        char host[MAXDNAME]; /* OCS */

	if ((p=open("/dev/ptc", O_RDWR)) >= 0)
		line = ttyname(p);
	else
		fatal(f, MSGSTR(PORTS_IN_USE, 
			"All network ports in use")); /*MSG*/

	dup2(f, 0);

	hp = gethostbyaddr(&who->sin_addr, sizeof (struct in_addr),
		who->sin_family);
	if (hp)
                strcpy(host, hp->h_name);  /* OCS */
	else
                strcpy(host, inet_ntoa(who->sin_addr)); /* OCS */

#ifdef _CSECURITY
 	CONNECTION_WRITE(remote_addr, "telnet/tcp", "open", "", 0);
#endif _CSECURITY

	net = f;
	pty = p;

	/*
	* OCS
	* Determine if the destination address found in the
	* local telnet request is a virtual host address
	*/
	OCScon_ptr = findvhost(local);                            /* OCS */
	if (OCScon_ptr) {                                         /* OCS */
		getterminaltype();                                /* OCS */
		if (terminaltype) {
		    strncpy(tlm.u.tv.term_env0, terminaltype,
			      sizeof tlm.u.tv.term_env0);         /* OCS */
		} else {
		    *tlm.u.tv.term_env0 = '\0';
		}
	        strncpy(tlm.u.tv.rhostname, host,
	             sizeof(tlm.u.tv.rhostname));                 /* OCS */
		tlm.vtp_type = TELNETD_VTP;                       /* OCS */
		process_vhost(OCScon_ptr, line, &tlm);            /* OCS */
		(void)setsid();                                   /* OCS */
		telnet(f, p, OCScon_ptr->hostname);               /* OCS */
	} /* End of virtual host check */

	/*
	* OCS
	* Destination address found in the telnet request is
	* NOT a virtual host address.  Proceed with normal
	* telnetd processing.
	*/	

/* we need to set up some initial line disciplines, so we must open the 
   slave side now with the NOCTTY option, set 'em up, and then fork.  
   The child will open the slave side again, close the first open, 
   dup2 the second one to 0,1,2. 
   
   This is all due to the fact that the parent can and will alter these 
   initial line disciplines and must be guaranteed that the slave side is
   open when this is done (see telnet() and mode()). */

	t = open(line, O_RDWR | O_NOCTTY);
	if (t < 0)
		fatalperror(f, line, errno);

	/* Set up pty line disciplines */
#ifdef TXSETLD
	ioctl(t, TXSETLD, "posix");
#endif /* TXSETLD */
	tcgetattr(t,(struct termios *)&b);
	b.c_cflag = (B9600|CS8|CREAD|HUPCL);
	b.c_lflag = (ISIG|ICANON|ECHO|ECHOE|ECHOK|ECHOKE|ECHOCTL|IEXTEN);
	b.c_iflag = (BRKINT|IGNPAR|ICRNL|IMAXBEL);
	/* Do NOT set flow control stuff on PTY since it is taken care of
  	   on the client side */
	b.c_iflag &= ~(IXANY|IXON|IXOFF);
	b.c_oflag = (OPOST|ONLCR|TAB3);
	b.c_cc[VINTR]    = CINTR;
	b.c_cc[VQUIT]    = CQUIT;
	b.c_cc[VERASE]   = CERASE;
	b.c_cc[VKILL]    = CKILL;
	b.c_cc[VEOF]     = CEOF;
	b.c_cc[VEOL]     = 0;
	b.c_cc[VEOL2]    = 0;
	b.c_cc[VSTART]   = CSTART;
	b.c_cc[VSTOP]    = CSTOP;
	b.c_cc[VSUSP]    = 'Z' - '@';
	b.c_cc[VDSUSP]   = 'Y' - '@';
	b.c_cc[VREPRINT] = 'R' - '@';
	b.c_cc[VDISCRD]  = 'O' - '@';
	b.c_cc[VWERSE]   = 'W' - '@';
	b.c_cc[VLNEXT]   = 'V' - '@';
	tcsetattr(t,TCSANOW,(struct termios *)&b);
#ifdef  TIOCSWINSZ
                (void) ioctl(t, TIOCSWINSZ, &w);
#else
#ifdef  TXSETWINSZ
                (void) ioctl(t, TXSETWINSZ, &w);
#endif  /* TXSETWINSZ */
#endif  /* TIOCSWINSZ */
	
	/*
	 * get terminal type.
	 */
	getterminaltype();
	
	if ((childpid = fork()) < 0)
		fatalperror(f, "fork", errno);
	if (childpid) {
		close(t);
		(void)setsid();
		telnet(f, p, 0);		/* OCS */
	}

        /*
         *  Since we're in the child, we need to setup the SRC related signals
         *  to be non-restartable so that further down the line other children
         *  won't get clobbered by the way SIGUSR1 and SIGUSR2 are used
         *  by our parent.
	 *  Also set SIGQUIT handler to SIG_DFL so the ksh and bsh won't block
	 *  SIGQUIT.
         */
        {
        struct sigvec sv;

        bzero((char *)&sv, sizeof(sv));
        sv.sv_handler = SIG_DFL;
        sigvec(SIGQUIT, &sv, (struct sigvec *)0);
        sv.sv_mask = sigmask(SIGUSR2);
        sv.sv_onstack = SV_INTERRUPT;
        sv.sv_handler = (void (*)(int)) trace_handler;
        sigvec(SIGUSR1, &sv, (struct sigvec *)0);
        sv.sv_mask = sigmask(SIGUSR1);
        sv.sv_onstack = SV_INTERRUPT;
        sv.sv_handler = (void (*)(int)) trace_handler;
        sigvec(SIGUSR2, &sv, (struct sigvec *)0);

        }

	(void) setsid();

	t1 = open(line, O_RDWR);
	if (acl_fset(t1, NO_ACC, NO_ACC, NO_ACC))
		fatalperror(f, line, errno);
	frevoke(t1);
	if (t1 < 0)
		fatalperror(f, line, errno);
	close(t);
	close(p);
	close(f);
	dup2(t1, 0);
	dup2(t1, 1);
	dup2(t1, 2);
	close(t1);

	envinit[0] = terminaltype;
	envinit[1] = 0;
	environ = envinit;

	(void) loginlog(line, host);
        /*
         * -h : pass on name of host.
         *              WARNING:  -h is accepted by login if and only if
         *                      getuid() == 0.
         * -p : don't clobber the environment (so terminal type stays set).
         */
        execl("/bin/login", "login", "-h", host,
                                        terminaltype ? "-p" : 0, 0);
        syslog(LOG_ERR, "/bin/login: %m\n");
        fatalperror(2, "/bin/login");
        /*NOTREACHED*/
}


fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];
	char	buf1[BUFSIZ];	/* buffer to hold message since msg may be
				   a pointer to the catalog buffer and the
				   message will be overwritten by the
				   retrieval of FATAL_P_FMT from the catalog. */

	strcpy(buf1, msg);	/* save passed message */
	msg = buf1;		/* reset msg to point to saved message */

	(void) sprintf(buf, MSGSTR(FATAL_FMT, "telnetd: %s.\r\n"), msg); /*MSG*/
	(void) write(f, buf, strlen(buf));
#ifdef _CSECURITY
	CONNECTION_WRITE(remote_addr, "telnet/tcp", "close", msg, -1);
#endif _CSECURITY
	exit(1);
}

fatalperror(f, msg, errno)
	int f;
	char *msg;
	int errno;
{
	char buf[BUFSIZ];
	extern char *sys_errlist[];

	char	buf1[BUFSIZ];	/* buffer to hold message since msg probably
				   is a pointer to the catalog buffer and the
				   message will be overwritten by the
				   retrieval of FATAL_P_FMT from the catalog. */
	char	fmt[81];	/* buffer to hold format for perror type
				   messages */
	char	*c;

	strcpy(buf1, msg);	/* save the passed message */
	msg = buf1;		/* reset msg to point to saved message */
	strcpy(fmt, MSGSTR(FATAL_P_FMT, "%s: %s\r\n")); /*MSG*/
	c = getamsg(MF_LIBC, MS_LIBC, errno, sys_errlist[errno]);
	(void) sprintf(buf, fmt, msg, c);
	fatal(f, buf);
}


/*
 * Check a descriptor to see if out of band data exists on it.
 */


stilloob(s)
int	s;		/* socket number */
{
    static struct timeval timeout = { 0, 0 };
    fd_set	excepts;
    int value;

    do {
	FD_ZERO(&excepts);
	FD_SET(s, &excepts);
	value = select(s+1, (fd_set *)0, (fd_set *)0, &excepts, &timeout);
    } while ((value == -1) && (errno == EINTR));

    if (value < 0) {
	fatalperror(pty, MSGSTR(SELECT_ERR, "select"), errno); /*MSG*/
    }
    if (FD_ISSET(s, &excepts)) {
	return 1;
    } else {
	return 0;
    }
}
#ifdef _CSECURITY
	/* _AIX221 - hupcatch should be declared for all versions,
		    not just 2.2.1 */
#ifdef	_AIX221
	int	hupcatch();
#else
	void	gotsak();
#endif	_AIX221
#endif _CSECURITY

/*
 * Main loop.  Select from pty and network, and
 * hand data to telnet receiver finite state machine.
 */
telnet(f, p, ocs_hostname)			    /* OCS */
char *ocs_hostname;                                 /* OCS */
{
	int on = 1;
	char hostname[MAXHOSTNAMELEN];	
	pid_t pid = getpid();
	char	*catbanner;
	int	nfd;


	ioctl(f, FIONBIO, &on);
	ioctl(p, FIONBIO, &on);
#if	defined(SO_OOBINLINE)
	setsockopt(net, SOL_SOCKET, SO_OOBINLINE, &on, sizeof on);
#endif	/* defined(SO_OOBINLINE) */

	(void) signal(SIGTSTP, SIG_IGN);
	(void) signal(SIGCHLD, (void (*)(int))cleanup);
	/* we want to exit if the client disconnects */
	(void) signal(SIGPIPE, (void (*)(int))cleanup);

#ifdef _CSECURITY
#ifdef	_AIX221
	(void) signal(SIGHUP, hupcatch);
#else
	(void) signal(SIGSAK, (void (*)(int))gotsak);
#endif	_AIX221
#endif _CSECURITY

/* Negotiate now for server to echo and suppressing SGA */
	/*
	 * tty should have echo set, plus login has probably already
	 * changed it so just set the states properly and send the
	 * "will" but don't actually change the tty again.
	 */
        if (!myopts[TELOPT_ECHO]) {
	    sprintf(nfrontp, "%s%c", will, TELOPT_ECHO);
	    nfrontp += sizeof doopt;
	    myopts[TELOPT_ECHO] = OPT_YES;
        }
        if (!myopts[TELOPT_SGA]) {
            dooption(TELOPT_SGA);
        }
        if (!hisopts[TELOPT_NAWS]) {
            willoption(TELOPT_NAWS);
        }
#ifdef _CSECURITY
        /* Offer to do SAK if enabled on this pty */
        if (!myopts[TELOPT_SAK]) {
                dooption(TELOPT_SAK);
        }
#endif _CSECURITY

	/*
	 * Is the client side a 4.2 (NOT 4.3) system?  We need to know this
	 * because 4.2 clients are unable to deal with TCP urgent data.
	 *
	 * To find out, we send out a "DO ECHO".  If the remote system
	 * answers "WILL ECHO" it is probably a 4.2 client, and we note
	 * that fact ("WILL ECHO" ==> that the client will echo what
	 * WE, the server, sends it; it does NOT mean that the client will
	 * echo the terminal input).
	 */
	sprintf(nfrontp, "%s%c", doopt, TELOPT_ECHO);
	nfrontp += sizeof doopt;
	hisopts[TELOPT_ECHO] = OPT_YES_BUT_ALWAYS_LOOK;

	/*
	 * Show banner that getty never gave.
	 *
	 * The banner includes some null's (for TELNET CR disambiguation),
	 * so we have to be somewhat complicated.
	 */

	if (ocs_hostname == NULL)                               /* OCS */
	    gethostname(hostname, sizeof (hostname));
	else                                                    /* OCS */
            strncpy(hostname, ocs_hostname, sizeof(hostname));  /* OCS */

	catbanner = MSGSTR(CATBANNER, MSGBANNER);
	sprintf(nfrontp, catbanner, hostname);
	nfrontp += strlen(nfrontp);
	bcopy(MSGBANN2, nfrontp, sizeof MSGBANN2 - 1);
	nfrontp += sizeof MSGBANN2 - 1;

	/*
	 * Call telrcv() once to pick up anything received during
	 * terminal type negotiation.
	 */
	telrcv(hostname);		/* OCS */

	if (f > p)
		nfd = f + 1;
	else
		nfd = p + 1;

	for (;;) {
		fd_set ibits, obits, xbits;
		register int c;

		if (ncc < 0 && pcc < 0)
			break;

		FD_ZERO(&ibits);
		FD_ZERO(&obits);
		FD_ZERO(&xbits);
		/*
		 * Never look for input if there's still
		 * stuff in the corresponding output buffer
		 */
		if (nfrontp - nbackp || pcc > 0) {
			FD_SET(f, &obits);
		} else {
			FD_SET(p, &ibits);
		}
		if (pfrontp - pbackp || ncc > 0) {
			FD_SET(p, &obits);
		} else {
			FD_SET(f, &ibits);
		}
		if (!SYNCHing) {
			FD_SET(f, &xbits);
		}
		if ((c = select(nfd, &ibits, &obits, &xbits,
						(struct timeval *)0)) < 1) {
			if (c == -1) {
				if (errno == EINTR) {
					continue;
				}
			}
#ifdef _CSECURITY
			sleep(1);
#else
			sleep(5);
#endif _CSECURITY
			continue;
		}

		/*
		 * Any urgent data?
		 */
		if (FD_ISSET(net, &xbits)) {
		    SYNCHing = 1;
		}

		/*
		 * Something to read from the network...
		 */
		if (FD_ISSET(net, &ibits)) {
#if	!defined(SO_OOBINLINE)
			/*
			 * In 4.2 (and 4.3 beta) systems, the
			 * OOB indication and data handling in the kernel
			 * is such that if two separate TCP Urgent requests
			 * come in, one byte of TCP data will be overlaid.
			 * This is fatal for Telnet, but we try to live
			 * with it.
			 *
			 * In addition, in 4.2 (and...), a special protocol
			 * is needed to pick up the TCP Urgent data in
			 * the correct sequence.
			 *
			 * What we do is:  if we think we are in urgent
			 * mode, we look to see if we are "at the mark".
			 * If we are, we do an OOB receive.  If we run
			 * this twice, we will do the OOB receive twice,
			 * but the second will fail, since the second
			 * time we were "at the mark", but there wasn't
			 * any data there (the kernel doesn't reset
			 * "at the mark" until we do a normal read).
			 * Once we've read the OOB data, we go ahead
			 * and do normal reads.
			 *
			 * There is also another problem, which is that
			 * since the OOB byte we read doesn't put us
			 * out of OOB state, and since that byte is most
			 * likely the TELNET DM (data mark), we would
			 * stay in the TELNET SYNCH (SYNCHing) state.
			 * So, clocks to the rescue.  If we've "just"
			 * received a DM, then we test for the
			 * presence of OOB data when the receive OOB
			 * fails (and AFTER we did the normal mode read
			 * to clear "at the mark").
			 */
		    if (SYNCHing) {
			int atmark;

			ioctl(net, SIOCATMARK, (char *)&atmark);
			if (atmark) {
			    ncc = recv(net, netibuf, sizeof (netibuf), MSG_OOB);
			    if ((ncc == -1) && (errno == EINVAL)) {
				ncc = read(net, netibuf, sizeof (netibuf));
				if (sequenceIs(didnetreceive, gotDM)) {
				    SYNCHing = stilloob(net);
				}
			    }
			} else {
			    ncc = read(net, netibuf, sizeof (netibuf));
			}
		    } else {
			ncc = read(net, netibuf, sizeof (netibuf));
		    }
		    settimer(didnetreceive);
#else	/* !defined(SO_OOBINLINE)) */
		    ncc = read(net, netibuf, sizeof (netibuf));
#endif	/* !defined(SO_OOBINLINE)) */
		    if (ncc < 0 && errno == EWOULDBLOCK)
			ncc = 0;
		    else {
			if (ncc <= 0) {
			    break;
			}
			netip = netibuf;
		    	FD_SET(p, &obits);		/* try write */
		    }
		}

		/*
		 * Something to read from the pty...
		 */
		if (FD_ISSET(p, &ibits)) {
			pcc = read(p, ptyibuf, BUFSIZ);
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else {
				if (pcc <= 0)
					break;
				ptyip = ptyibuf;
		    		FD_SET(f, &obits);		/* try write */
			}
		}

		while (pcc > 0) {
			if ((&netobuf[BUFSIZ] - nfrontp) < 2)
				break;
			c = *ptyip++ & 0377, pcc--;
			/* Stuff outgoing IAC's */
			if (c == IAC)
				*nfrontp++ = c;
			*nfrontp++ = c;
			/* don't cook cr's if we're in binary mode */
                        if ((c == '\r') && (myopts[TELOPT_BINARY] == OPT_NO)) {
				if (pcc > 0 && ((*ptyip & 0377) == '\n')) {
					*nfrontp++ = *ptyip++ & 0377;
					pcc--;
				} else
					*nfrontp++ = '\0';
			}
		}
		if (FD_ISSET(f, &obits) && (nfrontp - nbackp) > 0)
			netflush();
		if (ncc > 0)
			telrcv(hostname);		/* OCS */
		if (FD_ISSET(p, &obits) && (pfrontp - pbackp) > 0)
			ptyflush();
	}
	cleanup(0);
}
	
/*
 * State for recv fsm
 */
#define	TS_DATA		0	/* base state */
#define	TS_IAC		1	/* look for double IAC's */
#define	TS_CR		2	/* CR-LF ->'s CR */
#define	TS_SB		3	/* throw away begin's... */
#define	TS_SE		4	/* ...end's (suboption negotiation) */
#define	TS_WILL		5	/* will option negotiation */
#define	TS_WONT		6	/* wont " */
#define	TS_DO		7	/* do " */
#define	TS_DONT		8	/* dont " */

telrcv(hostname_ptr)		/* OCS */
char *hostname_ptr;
{
	register int c;
	static int state = TS_DATA;
	int sak_status = TCSAKON;


	while (ncc > 0) {
		if ((&ptyobuf[BUFSIZ] - pfrontp) < 2) {
			return;
		}
		c = *netip++ & 0377, ncc--;
		switch (state) {

		case TS_CR:
			state = TS_DATA;
			if ((c == 0) || (c == '\n')) {
				break;
			}
			/* FALL THROUGH */

		case TS_DATA:
			if (c == IAC) {
				state = TS_IAC;
#ifdef ADEBUG
			mydebug("receive IAC ");
#endif ADEBUG
				break;
			}

			/*
			 * We don't allow certain tty changes until the first
			 * data character comes in, on the assumption that
			 * this gets us by the initial startup period. We
			 * can't, e.g., set the tty to raw mode during this
			 * period since it interferes with the login process.
			 * After the login process, any terminal changes
			 * we do may or may not conflict with what the child
			 * is doing, but the login case is a serious race
			 * condition and definitely needs to be avoided.
			 */
			unsafe_on = unsafe_off = 0;  /* all is safe now */

			if (inter > 0)
				break;
			/*
                         * We now map \r\n ==> \r for pragmatic reasons.
                         * Many client implementations send \r\n when
                         * the user hits the CarriageReturn key.
                         *
                         * We USED to map \r\n ==> \n, since \r\n says
			 * that we want to be in column 1 of the next
			 * printable line, and \n is the standard
			 * unix way of saying that (\r is only good
			 * if CRMOD is set, which it normally is).
			 */
                        if ((c == '\r') && (hisopts[TELOPT_BINARY] == OPT_NO)) {
                                state = TS_CR;
			}
			*pfrontp++ = c;
			break;

		case TS_IAC:
			switch (c) {

			/*
			 * Send the process on the pty side an
			 * interrupt.  Do this with a NULL or
			 * interrupt char; depending on the tty mode.
			 */
			case IP:
#ifdef ADEBUG
			mydebug("IP\n");
#endif ADEBUG
				interrupt();
				break;

			case BREAK:
#ifdef ADEBUG
			mydebug("BREAK\n");
#endif ADEBUG
				sendbrk();
				break;

			/*
			 * Are You There?
			 */
			case AYT:
#ifdef ADEBUG
			mydebug("AYT\n");
#endif ADEBUG
				/* The _AIX message is a little nicer */
				{
				char hostname[MAXHOSTNAMELEN];
				char *here = "\r\n%s here\r\n";
				char *herep;

				herep = MSGSTR(HERESTR, here);
				strncpy(hostname, hostname_ptr,
						sizeof(hostname));  /* OCS */
				sprintf(nfrontp, herep, hostname);
				nfrontp += strlen(nfrontp);
				}
				break;

			/*
			 * Abort Output
			 */
			case AO: {
#ifdef ADEBUG
					mydebug("AO\n");
#endif ADEBUG
					/* For _AIX, use TCFLSH */
					ptyflush();	/* half-hearted */
					tcflush(pty,TCOFLUSH);

/* Since AO is intended to Abort any remaining output, this implies killing
 * whatever process is running.  Otherwise, AO is meaningless.  Case in point,
 * AO sent during a "ls -sailR /usr" needs the interrupt sent or the AO fails
 */
					interrupt();
					netclear();	/* clear buffer back */
					*nfrontp++ = IAC;
					*nfrontp++ = DM;
					neturg = nfrontp-1; /* off by one XXX */
#ifdef ADEBUG
		mydebug("send IAC DM\n");
#endif ADEBUG
					break;
				}

			/*
			 * Erase Character and
			 * Erase Line
			 */
			case EC:
			case EL: {
					struct termios b;
					char ch;

#ifdef ADEBUG
			mydebug(c == EC ? "EC\n" : "EL\n");
#endif ADEBUG
					ptyflush();	/* half-hearted */
					tcgetattr(pty,(struct termios *)&b);
					ch = (c == EC) ?
						b.c_cc[2] : b.c_cc[3];
					/* THIS LINE CHANGED! - Mark */
					if (ch != NOCHAR) {
						*pfrontp++ = ch;
					}
					break;
				}

#ifdef _CSECURITY
			/* SAK support along these lines ... */
			case SAK:
#ifdef ADEBUG
			mydebug("SAK\n");
#endif ADEBUG
				ptyflush();	/* half-hearted */
				/*
				 * Now turn on sak on the pty
				 */
				(void) ioctl(pty, TCSAK, &sak_status);

				write(pty, sak_sequence, 2);
				break;
#endif _CSECURITY
			/*
			 * Check for urgent data...
			 */
			case DM:
#ifdef ADEBUG
			mydebug("DM\n");
#endif ADEBUG
				SYNCHing = stilloob(net);
				settimer(gotDM);
				break;


			/*
			 * Begin option subnegotiation...
			 */
			case SB:
#ifdef ADEBUG
			mydebug("SB ");
#endif ADEBUG
				/* BSD bug!! */
				SB_CLEAR();
				SB_TERM();
				state = TS_SB;
				continue;

			case WILL:
#ifdef ADEBUG
			mydebug("WILL ");
#endif ADEBUG
				state = TS_WILL;
				continue;

			case WONT:
#ifdef ADEBUG
			mydebug("WONT ");
#endif ADEBUG
				state = TS_WONT;
				continue;

			case DO:
#ifdef ADEBUG
			mydebug("DO ");
#endif ADEBUG
				state = TS_DO;
				continue;

			case DONT:
#ifdef ADEBUG
			mydebug("DONT ");
#endif ADEBUG
				state = TS_DONT;
				continue;

			case IAC:
#ifdef ADEBUG
			mydebug("IAC\n");
#endif ADEBUG
				*pfrontp++ = c;
				break;
			}
			state = TS_DATA;
			break;

		case TS_SB:
			if (c == IAC) {
				state = TS_SE;
			} else {
				SB_ACCUM(c);
			}
			break;

		case TS_SE:
			if (c != SE) {
				if (c != IAC) {
					SB_ACCUM(IAC);
				}
				SB_ACCUM(c);
				state = TS_SB;
			} else {
				SB_TERM();
				suboption();	/* handle sub-option */
				state = TS_DATA;
			}
			break;

		case TS_WILL:
			if (hisopts[c] != OPT_YES)
				willoption(c);
			state = TS_DATA;
			continue;

		case TS_WONT:
			if (hisopts[c] != OPT_NO)
				wontoption(c);
			state = TS_DATA;
			continue;

		case TS_DO:
			if (myopts[c] != OPT_YES)
				dooption(c);
			state = TS_DATA;
			continue;

		case TS_DONT:
			if (myopts[c] != OPT_NO) {
				dontoption(c);
			}
			state = TS_DATA;
			continue;

		default:
			syslog(LOG_ERR, MSGSTR(PANIC_ERR,
				"telnetd: panic state=%d\n"), state); /*MSG*/
			printf(MSGSTR(PANIC_ERR, "telnetd: panic state=%d\n"),
				state); /*MSG*/
#ifdef _CSECURITY
			CONNECTION_WRITE(remote_addr, "telnet/tcp", "close",
				MSGSTR(C_PANIC, "panic state"), -1); /*MSG*/
#endif _CSECURITY
			exit(1);
		}
	}
}

willoption(option)
	int option;
{
	char *fmt;


#ifdef ADEBUG
	mydebug(telopts[option]);
	mydebugc('\n');
#endif ADEBUG
	switch (option) {

	case TELOPT_BINARY:
		mode(RAW, 0);
		fmt = doopt;
		break;

        case TELOPT_NAWS:
                fmt = doopt;
                break;

	case TELOPT_ECHO:
		not42 = 0;		/* looks like a 4.2 system */
		/*
		 * Now, in a 4.2 system, to break them out of ECHOing
		 * (to the terminal) mode, we need to send a "WILL ECHO".
		 * Kludge upon kludge!
		 */
		if (myopts[TELOPT_ECHO] == OPT_YES) {
		    dooption(TELOPT_ECHO);
		}
		fmt = dont;
		break;

	case TELOPT_TTYPE:
		settimer(ttypeopt);
		if (hisopts[TELOPT_TTYPE] == OPT_YES_BUT_ALWAYS_LOOK) {
		    hisopts[TELOPT_TTYPE] = OPT_YES;
		    return;
		}
		fmt = doopt;
		break;

	case TELOPT_SGA:
		fmt = doopt;
		break;

	case TELOPT_TM:
		fmt = dont;
		break;

	default:
		fmt = dont;
		break;
	}
	if (fmt == doopt) {
		hisopts[option] = OPT_YES;
	} else {
		hisopts[option] = OPT_NO;
	}
	sprintf(nfrontp, "%s%c", fmt, option);
	nfrontp += sizeof (dont);
#ifdef ADEBUG
	fprintf(debugfp, "send IAC %s %s\n", fmt == doopt? "DO" : "DONT",
		telopts[option]);
#endif ADEBUG
}

wontoption(option)
	int option;
{
	char *fmt;


#ifdef ADEBUG
	mydebug(telopts[option]);
	mydebugc('\n');
#endif ADEBUG
	switch (option) {
	case TELOPT_ECHO:
		not42 = 1;		/* doesn't seem to be a 4.2 system */
		break;

	case TELOPT_BINARY:
		mode(0, RAW);
		break;

	case TELOPT_TTYPE:
	    settimer(ttypeopt);
	    settimer(ttypesubopt);
	    break;
	}

	fmt = dont;
	hisopts[option] = OPT_NO;
	sprintf(nfrontp, "%s%c", fmt, option);
	nfrontp += sizeof (doopt);
#ifdef ADEBUG
	fprintf(debugfp, "send IAC DONT %s\n", telopts[option]);
#endif ADEBUG
}

dooption(option)
	int option;
{
	char *fmt;
#ifdef _CSECURITY
#ifdef _AIX221
	int sak_status = TCSAKOFF;
#else
	int sak_status;
#endif _AIX221
#endif _CSECURITY


#ifdef ADEBUG
	mydebug(telopts[option]);
	mydebugc('\n');
#endif ADEBUG
	switch (option) {

	case TELOPT_TM:
		fmt = wont;
		break;

	case TELOPT_ECHO:
		mode(ECHO|CRMOD, 0);
		fmt = will;
		break;

	case TELOPT_BINARY:
		mode(RAW, 0);
		fmt = will;
		break;

	case TELOPT_SGA:
		fmt = will;
		break;
	
#ifdef _CSECURITY
	case TELOPT_SAK:
#ifdef	_AIX221
		(void) ioctl(pty, TCQSAK, &sak_status);
#else
		if (check_pty_sak(line))
			sak_status = TCSAKON;
		else
			sak_status = TCSAKOFF;
#endif	_AIX221
		if (sak_status == TCSAKON)
			fmt = will;
		else
			fmt = wont;
		break;
#endif _CSECURITY

	default:
		fmt = wont;
		break;
	}
	if (fmt == will) {
	    myopts[option] = OPT_YES;
	} else {
	    myopts[option] = OPT_NO;
	}
	sprintf(nfrontp, "%s%c", fmt, option);
	nfrontp += sizeof (doopt);
#ifdef ADEBUG
	fprintf(debugfp, "send IAC %s %s\n", fmt == will? "WILL" : "WONT",
		telopts[option]);
#endif ADEBUG
}


dontoption(option)
int option;
{
    char *fmt;


#ifdef ADEBUG
	mydebug(telopts[option]);
	mydebugc('\n');
#endif ADEBUG
    switch (option) {
    case TELOPT_ECHO:		/* we should stop echoing */
	mode(0, ECHO);
	fmt = wont;
	break;

    default:
	fmt = wont;
	break;
    }

    if (fmt = wont) {
	myopts[option] = OPT_NO;
    } else {
	myopts[option] = OPT_YES;
    }
    sprintf(nfrontp, "%s%c", fmt, option);
    nfrontp += sizeof (wont);
#ifdef ADEBUG
	fprintf(debugfp, "send IAC %s %s\n", fmt == wont? "WONT" : "WILL",
		telopts[option]);
#endif ADEBUG
}

/*
 * suboption()
 *
 *	Look at the sub-option buffer, and try to be helpful to the other
 * side.
 *
 *	Currently we recognize:
 *
 *	Terminal type is
 */

suboption()
{

    switch (SB_GET()) {

    case TELOPT_NAWS: 
	{	
		unsigned char *c;
		struct winsize w;
		int width, height;

		c = (unsigned char *) subpointer;
		width = (*c << 8) | (*(c+1));
		c+=2;
		height = (*c << 8) | (*(c+1));
		c+=2;

		/* Network byte order. */
		w.ws_row=ntohs(height);
	  	w.ws_col=ntohs(width);

		w.ws_xpixel=0;
		w.ws_ypixel=0;

		/* now do the ioctl to inform the pty of the new window size
		   so the pty will SIGWINCH everyone. */
#ifdef  TIOCSWINSZ
		(void) ioctl(pty, TIOCSWINSZ, &w);
#else
#ifdef  TXSETWINSZ
		(void) ioctl(pty, TXSETWINSZ, &w);
#endif  /* TXSETWINSZ */
#endif  /* TIOCSWINSZ */
		break;
	}

    case TELOPT_TTYPE: {		/* Yaaaay! */
#define MAX_TERM 41
	static char termenv[] = "TERM=";
	static char terminalname[MAX_TERM + sizeof(termenv) - 1];
	static char term[MAX_TERM];
	static char lastguess[MAX_TERM];
	int i;

	if (SB_GET() != TELQUAL_IS) {
	    return;		/* ??? XXX but, this is the most robust */
	}
	for (i = 0; i < sizeof(term) - 1 && !SB_EOF(); ++i) {
	    term[i] = SB_GET();
	}
	term[i] = '\0';

#ifdef ADEBUG
	fprintf(debugfp, "suboption: terminal type '%s'\n", term);
#endif ADEBUG

	/* Check for supported terminals */
	if (!goodterminal(term)) {
#ifdef ADEBUG
		mydebug("suboption: terminal type rejected\n");
#endif ADEBUG
		/* RFC 930 says we have to shut up and accept it if we
		   get the same terminal two times in a row.  */
		if (strcmp(term, lastguess)) {
			strcpy(lastguess, term);
			askterminal();
			return;
		} else {
#ifdef ADEBUG
			mydebug("suboption: end of negotiation loop\n");
#endif ADEBUG
		}
	} 
	/* set global terminaltype and notify getterminaltype() */
	sprintf(terminalname, "%s%s", termenv, term);
	terminaltype = terminalname;
	settimer(ttypesubopt);
/*
 * save terminal type in shared memory for SRC long status
 */
	if (shared_mem_exists)
		save_telnetd_termtype(terminaltype);

#ifdef ADEBUG
	fprintf(debugfp, "suboption: accepted %s\n", terminaltype);
#endif ADEBUG

	break;
    }

    default:
	;
    }
}

goodterminal(terminaltype)
char *terminaltype;	/* can be altered */
{
	static char terminfo[100] = "/usr/lib/terminfo/x/";
#define TERMINFODIR	sizeof("/usr/lib/terminfo/") - 1
#define	TERMINFOMAX	20		/* maximum number of terminfo guesses */
	char *guess[TERMINFOMAX];
	register int i;

	if (!terminaltype || !*terminaltype)
		return 0;
	/* If BSD is talking to us, we've got the terminfo name already */
	terminfo[TERMINFODIR] = *terminaltype;	/* indexing char */
	strcpy(terminfo+TERMINFODIR+2, terminaltype);
	if (!accessx(terminfo, E_ACC, ACC_SELF))	/* found it */
		return 1;
	/* If old _AIX or RFC 1010 guys are talking to us, we have a telnet
	 * standard name, so convert it to a terminfo one.
	 */
	(void) telnettoterminfo(terminaltype, guess);
	for (i=0; guess[i]; ++i) {	/* keep on guessing */
		terminfo[TERMINFODIR] = *guess[i];
		strcpy(terminfo+TERMINFODIR+2, guess[i]);
		if (!accessx(terminfo,E_ACC, ACC_SELF)) {	/* found it */
		/* update this in the hope we'll someday pass it on */
			strcpy(terminaltype, guess[i]);
			return 1;
		}
	}
	return 0;	/* reject it */
}


mode(on, off)
	int on, off;
{
	struct termios b;
	static int didraw = 0;
	static struct termios prerawb;

#ifdef ADEBUG
	fprintf(debugfp, "on %s %s %s off %s %s %s\n",
		(on&CRMOD) ? "CRMOD" : "",
		(on&ECHO) ? "ECHO" : "",
		(on&RAW) ? "RAW" : "",
		(off&CRMOD) ? "CRMOD" : "",
		(off&ECHO) ? "ECHO" : "",
		(off&RAW) ? "RAW" : "");
#endif ADEBUG

	/* turn off unsafe bits */
	on &= ~unsafe_on;
	off &= ~unsafe_off;

	ptyflush();
	tcgetattr(pty,(struct termios *)&b);
	/* For sys V termio, things map a little strangely */
	if (on&CRMOD) {
		b.c_iflag |= ICRNL;
		b.c_oflag |= (OPOST|ONLCR);
		b.c_oflag &= ~ONLRET;
	}
	if (on&ECHO) {
		if (didraw)
			b = prerawb;
		b.c_lflag |= (ICANON|ECHO|ECHOE|ECHOK);
	}
	if (on&RAW) {
		/* the VMIN and VTIME values overlap with VEOF and VEOL
		 * (what a kludge!), plus all kinds of things change, so
		 * save the old struct. Also I believe BSD RAW mode doesn't
		 * do timeout itself, so turn off timeout.  Also,
		 * in RAW mode, a break condition on input is reported as
		 * a null character, which we can't do - so skip that.
		 */
		if (!didraw) {	/* haven't gone raw before */
			prerawb = b;
			didraw = 1;
		}
		b.c_iflag = 0;
		b.c_oflag &= ~OPOST;
		b.c_cflag |= CS8; b.c_cflag &= ~PARENB;
		b.c_lflag &= ~(ISIG|ICANON);
		b.c_cc[VTIME] = 0;	/* no timeout */
		b.c_cc[VMIN] = 1;	/* each character */
	}
	if (off&CRMOD) {
		b.c_iflag &= ~ICRNL;
		b.c_oflag &= ~ONLCR;
	}
	if (off&ECHO) {
		b.c_lflag &= ~(ECHO|ECHOE|ECHOK);
	}
	if (off&RAW) {
		if (didraw)		/* sure hope so */
			b = prerawb;
		else {		/* oops, do our best ... */
			b.c_iflag |= BRKINT|IGNPAR|IXON|IXANY|IMAXBEL;
			b.c_iflag &= ~(ISTRIP);
			b.c_oflag |= OPOST;
			b.c_cflag &= ~CS8; 
			b.c_cflag |= CS8|PARENB;
			b.c_lflag |= ISIG|ICANON;
			b.c_cc[VEOL] = '\0'; 
			b.c_cc[VEOF] = '\004';
		}
	}
	tcsetattr(pty,TCSANOW,&b);
}

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
interrupt()
{
	struct termios b;
	struct tchars tchars;


	ptyflush();	/* half-hearted */
	/* What if RAW mode??? */
	*pfrontp++ = tcgetattr(pty,(struct termios *)&b) < 0 ?
		'\177' : b.c_cc[0];
}

/*
 * Send quit to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write quit char.
 */
sendbrk()
{
	struct termios b;
	struct tchars tchars;


	ptyflush();	/* half-hearted */
	tcgetattr(pty,(struct termios *)&b);
	/* What if RAW mode??? */
	*pfrontp++ = tcgetattr(pty,(struct termios *)&b) < 0 ?
		CTRL(V) : b.c_cc[1];
}

ptyflush()
{
	int n;

	if ((n = pfrontp - pbackp) > 0)
		n = write(pty, pbackp, n);
	if (n < 0)
		return;
	pbackp += n;
	if (pbackp == pfrontp)
		pbackp = pfrontp = ptyobuf;
}

/*
 * nextitem()
 *
 *	Return the address of the next "item" in the TELNET data
 * stream.  This will be the address of the next character if
 * the current address is a user data character, or it will
 * be the address of the character following the TELNET command
 * if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */

char *
nextitem(current)
char	*current;
{

	if ((*current&0xff) != IAC) {
		return current+1;
	}
	switch (*(current+1)&0xff) {
	      case DO:
	      case DONT:
	      case WILL:
	      case WONT:
		return current+3;
	      case SB:		/* loop forever looking for the SE */
		{
			register char *look = current+2;

			for (;;) {
				if ((*look++&0xff) == IAC) {
					if ((*look++&0xff) == SE) {
						return look;
					}
				}
			}
		}
	      default:
		return current+2;
	}
}


/*
 * netclear()
 *
 *	We are about to do a TELNET SYNCH operation.  Clear
 * the path to the network.
 *
 *	Things are a bit tricky since we may have sent the first
 * byte or so of a previous TELNET command into the network.
 * So, we have to scan the network buffer from the beginning
 * until we are up to where we want to be.
 *
 *	A side effect of what we do, just to keep things
 * simple, is to clear the urgent data pointer.  The principal
 * caller should be setting the urgent data pointer AFTER calling
 * us in any case.
 */

netclear()
{
	register char *thisitem, *next;
	char *good;
#define	wewant(p)	((nfrontp > p) && ((*p&0xff) == IAC) && \
			 ((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))


	thisitem = netobuf;

	while ((next = nextitem(thisitem)) <= nbackp) {
		thisitem = next;
	}

    /* Now, thisitem is first before/at boundary. */

    good = netobuf;	/* where the good bytes go */

    while (nfrontp > thisitem) {
	if (wewant(thisitem)) {
	    int length;

	    next = thisitem;
	    do {
		next = nextitem(next);
	    } while (wewant(next) && (nfrontp > next));
	    length = next-thisitem;
	    bcopy(thisitem, good, length);
	    good += length;
	    thisitem = next;
	} else {
	    thisitem = nextitem(thisitem);
	}
    }

    nbackp = netobuf;
    nfrontp = good;		/* next byte to be sent */
    neturg = 0;
}

/*
 *  netflush
 *		Send as much data as possible to the network,
 *	handling requests for urgent data.
 */


netflush()
{
	int n;


	if ((n = nfrontp - nbackp) > 0) {
		/*
		 * if no urgent data, or if the other side appears to be an
		 * old 4.2 client (and thus unable to survive TCP urgent data),
		 * write the entire buffer in non-OOB mode.
		 */
		if ((neturg == 0) || (not42 == 0)) {
			n = write(net, nbackp, n); /* normal write */
		} else {
			n = neturg - nbackp;
			/*
			 * In 4.2 (and 4.3) systems, there is some question about
			 * what byte in a sendOOB operation is the "OOB" data.
			 * To make ourselves compatible, we only send ONE byte
			 * out of band, the one WE THINK should be OOB (though
			 * we really have more the TCP philosophy of urgent data
			 * rather than the Unix philosophy of OOB data).
			 */
			if (n > 1) {
				n = send(net, nbackp, n-1, 0); /* send URGENT all by itself */
			} else {
				n = send(net, nbackp, n, MSG_OOB); /* URGENT data */
			}
		}
	}
	if (n < 0)
		switch (errno) {

			case EWOULDBLOCK:
			case ENOBUFS:
				/* give it a second */
				sleep(1);
				return;

			case EINTR:
				return;
			
			default:
				/* all others fatal: log and die */
				syslog(LOG_ERR, "error writing to client: %m");

				cleanup(0);
		}
	nbackp += n;
	if (nbackp >= neturg) {
		neturg = 0;
	}
	if (nbackp == nfrontp) {
		nbackp = nfrontp = netobuf;
	}
}

void
cleanup(int sig)
{
	char *p;
	struct utmp cutmp, *cptr, *getutline(), *pututline();
	int status;
	sigset_t newset;

	close(pty);
	close(net);
	while (waitpid(childpid, &status, 0) == -1 && errno == EINTR);

	p = line;
	if (!strncmp(p, "/dev/", 5))
		p += 5;
	bzero(&cutmp, sizeof(cutmp));
	strncpy(cutmp.ut_id, p, sizeof(cutmp.ut_id));
	cutmp.ut_type = USER_PROCESS;
	setutent();
	if (cptr = getutid(&cutmp)) {
		bzero(cptr->ut_user, sizeof(cptr->ut_user));
		bzero(cptr->ut_host, sizeof(cptr->ut_host));
		cptr->ut_time = time((time_t *)0);
		cptr->ut_type = DEAD_PROCESS;
		cptr->ut_exit.e_termination = status & 0xff;
		cptr->ut_exit.e_exit = (status >> 8) & 0xff;
                sigfillset(&newset);
                sigdelset(&newset,SIGSEGV);
                sigprocmask(SIG_BLOCK,&newset,NULL);
		(void) pututline(cptr);
                sigprocmask(SIG_UNBLOCK,&newset,NULL);
		append_wtmp(WTMP_FILE, cptr);
		endutent();
	}
	/* Release floating iFOR/LS license (sigh) */
	_FloatingReleaseLicense(childpid);

#ifdef _CSECURITY
	CONNECTION_WRITE(remote_addr, "telnet/tcp", "close", "", 0);
#endif _CSECURITY
#ifdef _CSECURITY
	/* Ok, let's say someone did kill -20 0. Then init will think it's
	 * supposed to start up the trusted shell.  But if we've exited out,
	 * it will start it up for the next person using telnet to get hold of.
	 * This bug isn't fixed by the fix I made to init.  That one covered
	 * the case where someone exited telnet while in the trusted shell.
	 * I fixed init so it wouldn't recreate the login shell (after we had
	 * already exited out).
	 * Is there any fix short of redoing init??
	 */
/*	frevoke(pty);	*/	/* don't frevoke */
#endif _CSECURITY
#ifdef _AIX
        (void) acl_set(line, R_ACC|W_ACC, R_ACC|W_ACC, R_ACC|W_ACC);
#else
        (void)chmod(line, 0666);
#endif /* _AIX */
        (void)chown(line, 0, 0);
	shutdown(net, 2);
	exit(1);
}

#ifdef _CSECURITY
#ifdef	_AIX221
static int hups = 0;
expecthups(number)
int number;
{
	hups = number;
}

hupcatch()
{
	if (--hups < 0)
		cleanup(0);
}
#endif	_AIX221

/*
 * slave_logged_in() - return 1 if slave is logged in (in a user state),
 * 0 if still in getty or login.
 */
#include <utmp.h>

slave_logged_in()
{
	switch (slave_state()) {
	case USER_PROCESS:
/* This is from the Gaithersburg implementation....
	case TSH_PROCESS:
*/
		return 1;
	default:
		return 0;
	}
}

extern struct utmp *getutent();

slave_state()
{
	char slaveline[12];
	register struct utmp *thisline;
	int type;

	strcpy(slaveline, line + sizeof("/dev/")-1);
	slaveline[sizeof("pt")-1] = 's';
	setutent();
	while (thisline = getutent()) {
		if (!strncmp(thisline->ut_line, slaveline, 12)) {
			type = thisline->ut_type;
			endutent();
			return type;
		}
	}
	/* if we reach here, something's screwed up, but oh well */
	endutent();
	return EMPTY;
}
#endif _CSECURITY


tc_put(c)	/* for compatibility */
int c;
{
	*nfrontp++ = c;
}

/*
 * NAME: exitlogin
 *
 * FUNCTION: audit failure and exit
 *
 * RETURNS: void
 *
 */
static void
exitlogin (uname, ttyn, msg)
char    *uname;         /* user logging in */
char    *ttyn;          /* Terminal name */
char    *msg;           /* error message or NULL if successful */
{
        auditlogin (uname, msg);
        if (!errno)
                errno = EACCES;
	cleanup(0);
}

/*
 * NAME: auditlogin
 *
 * FUNCTION: Print a msg if specified and Audit the result of login
 *
 * RETURNS: void
 *
 */
static void
auditlogin (uname, msg)
char    *uname;         /* user logging in */
char    *msg;           /* error message or NULL if successful */
{
int     result;         /* numeric result of login:
                         * 0 successful and 1 failure 
                         */

        if (msg) {
                pmsg (msg);
                result = 1;
        } else {
                result = 0;
        }
        (void) auditwrite ("telnet", result, uname, strlen (uname) + 1, msg,
                                                        strlen (msg) + 1, NULL);        return;
}

/*
 * NAME: pmsg
 *                                                                    
 * FUNCTION: print a message to the port
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *
 *                                                                   
 * RETURNS: int
 */  

#define PROGNAME "rlogind"

int
pmsg (args)
char    *args;
{
/*
int     fd;
char    **argv;
char    buf[BUFSIZ];
static  char    prefix[] = ": ";
static  char    suffix[] = "\n";

        fd = open (line, O_WRONLY);
        if (fd <= -1)
        {
                DPRINTF (("can't open %s\n", line));
                return;
        }
        write (fd, PROGNAME, strlen (PROGNAME));
        write (fd, prefix, sizeof (prefix));
        argv = &args;
        vsprintf (buf, argv[0], &argv[1]);
        write (fd, buf, strlen (buf));
        write (fd, suffix, sizeof (suffix) - 1);
        close (fd);
*/
}

/*
 * NAME: loginlog
 *                                                                    
 * FUNCTION: log the login
 *                                                                    
 * EXECUTION ENVIRONMENT: static
 *
 *		record login in utmp and wtmp files
 *                                                                   
 * RETURNS: void
 */  

static void
loginlog (char *tty, char *hostname)
{
	struct utmp utmp;

	bzero(&utmp, sizeof(utmp));
	if (!strncmp(tty, "/dev/", 5))
		tty += 5;
	strncpy(utmp.ut_id, tty, sizeof(utmp.ut_id));
	strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
	utmp.ut_type = LOGIN_PROCESS;
	utmp.ut_pid = getpid();
	utmp.ut_time = time((time_t *)0);
	if (hostname)
		strncpy(utmp.ut_host, hostname, sizeof(utmp.ut_host));
	pututline(&utmp);
	append_wtmp(WTMP_FILE, &utmp);
	endutent();
}

#ifdef _CSECURITY
/*
 *  NAME:       gotsak
 *
 *  FUNCTION:   Catch SIGSAK
 *
 *  EXECUTION ENVIRONMENT:
 *
 *      SAK Signal handler
 *
 *  RETURNS: void
 */

void
gotsak ()
{
	struct passwd *getpwnam(), *pwd;
	char *findloginlog();
	char user[UT_NAMESIZE +1];
	char *userp = user;
	pid_t pid;
	pid_t runtsh(char *, char*);

#ifdef ADEBUG
        mydebug("gotsak called\n");
#endif ADEBUG
	/* 
	 * First we need to get the user from the utmp file
	 * If we cannot get the user we must exit.
	 */
	userp = findloginlog(user, line);
	if ((pwd = (struct passwd *)getpwnam(userp)) == NULL)
		exitlogin(user, line, MSGSTR(M_FAILEXEC, DEF_FAILEXEC));
#ifdef ADEBUG
        fprintf(debugfp, "gotsak: userp %s.\n", userp);
#endif ADEBUG
	/*
	 * Insure the child is killed.  wait() until the
	 * child pid is seen or -1.
	 */
#ifdef ADEBUG
        mydebug("gotsak: about to kill the child.\n");
#endif ADEBUG
	(void) signal(SIGCHLD, SIG_IGN);
	kill (childpid, SIGKILL);
	while ((pid = wait (0)) != childpid && pid != -1)
		;
	/*
	 * Kill all other processes on the slave side of the pty.
	 */
#ifdef ADEBUG
        mydebug("gotsak: about revoke the line.\n");
#endif ADEBUG
        if (revoke(line) < 0)
		perror("revoke(line)");
	(void) signal(SIGCHLD, (void (*)(int))cleanup);

	/*
	 * Now run the trusted shell
	 */
	childpid = runtsh (line, user);
}

/*
 * NAME: check_pty_sak
 *
 * FUNCTION: Check if the tty supports the sak key.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: Returns 1 if sak is enabled on pty, otherwise return 0.
 */
int
check_pty_sak(char *ptyname)
{
	AFILE_t af;
	ATTR_t ar;
	char *sak;

        if ((af = afopen("/etc/security/login.cfg")) == NULL) {
		syslog(LOG_DEBUG, "afopen failed to open /etc/security/login.cfg: %m");
		return 0;
	}

        ar = afgetrec(af, ptyname);
	if (ar != NULL) {
		sak = afgetatr(ar, "sak_enabled");
		if (strcmp(sak, "true") == 0)
			return(1);
	}
	return(0);
}

typedef enum { TPATH_NOSAK, TPATH_NOTSH, TPATH_ALWAYS, TPATH_ON } tpath_t;
tpath_t   user_tpath;     /* Enumerated type indicated SAK processing */
/*
 * NAME: runtsh
 *
 * FUNCTION: Execute trusted shell on current login port.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process.  Creates a sub-process invoking the trusted
 *      shell.
 *
 * RETURNS: Process ID of new child, or -1 if an error occured.
 */

pid_t
runtsh (char *sakport, char *user)
{
        int     i, t1;
        int     pid = 0;
        int     parent = getpid ();
        char    term[PATH_MAX];
	struct 	termios b;
        void	setdefsenv (char **, int *, char *);

        /*
         * This portion of code implements the security policy that
         * a user with "tpath = notsh" can not execute the Trusted
         * Shell.
         */

        (void) tpath(user);
        if (user_tpath == TPATH_NOTSH || user_tpath == TPATH_NOSAK)
		exitlogin(user, sakport, MSGSTR(M_FAILEXEC, DEF_FAILEXEC));

        if (pid = fork ())
                return pid;

	(void) setsid();
	t1 = open(sakport, O_RDWR);
	if (t1 < 0)
		fatalperror(t1, sakport, errno);
	dup2(t1, 0);
	dup2(t1, 1);
	dup2(t1, 2);
	close(t1);

        /* Set up pty line disciplines */
        tcgetattr(0,(struct termios *)&b);
        b.c_cflag = (B9600|CS8|CREAD|HUPCL);
        b.c_lflag = (ISIG|ICANON|ECHO|ECHOE|ECHOK);
        b.c_iflag = (BRKINT|IGNPAR|ICRNL|IXON|IXANY|IMAXBEL);
        b.c_oflag = (OPOST|ONLCR|TAB3);
        b.c_cc[VINTR]  = CINTR;
        b.c_cc[VQUIT]  = CQUIT;
        b.c_cc[VERASE] = CERASE;
        b.c_cc[VKILL]  = CKILL;
        b.c_cc[VEOF]   = CEOF;
        tcsetattr(0,TCSANOW,(struct termios *)&b);

#if 0					/* just a huge no-op */
        /*
         * Re-initialize terminal modes and run trusted shell
         */
        setpgid(getpid (), getpid ());
        tcsetpgrp (0, getpid ());

        /* Set up pty line disciplines */
        tcgetattr(0,(struct termios *)&b);
        b.c_cflag = (B9600|CS8|CREAD|HUPCL);
        b.c_lflag = (ISIG|ICANON|ECHO|ECHOE|ECHOK);
        b.c_iflag = (BRKINT|IGNPAR|ICRNL|IXON|IXANY|IMAXBEL);
        b.c_oflag = (OPOST|ONLCR|TAB3);
        b.c_cc[VINTR]  = CINTR;
        b.c_cc[VQUIT]  = CQUIT;
        b.c_cc[VERASE] = CERASE;
        b.c_cc[VKILL]  = CKILL;
        b.c_cc[VEOF]   = CEOF;
        tcsetattr(0,TCSANOW,(struct termios *)&b);
#endif

        env[nenv++] = PENV_USRSTR;
        env[nenv++] = "SHELL=/bin/tsh";
        setdefsenv (env, &nenv, user);
	/*
	 * Set up credentials for this user and exec the trusted shell.
	 */
#ifdef ADEBUG
        fprintf(debugfp, "runtsh: about call setpcred(%s).\n", user);
#endif ADEBUG
	if (setpcred(user, NULL) == 0) {
		if (setpenv(user, PENV_INIT, env, "/bin/tsh") < 0)
			perror("setpenv():");
	}
	exitlogin(user, sakport, MSGSTR(M_FAILEXEC, DEF_FAILEXEC));
}

/*
 * NAME: setdefsenv
 *
 * FUNCTION: set default security environment
 *
 * EXECUTION ENVIRONMENT:
 *
 *      User process.  Sets default security environment for call
 * 	to setpenv().
 *
 * RETURNS: void
 */
void
setdefsenv (char **env ,int *nenv ,char *user)
{
        char     buf[128];
        char    *lognmenv;       /* Static (malloced) location to hold
                                   LOGNAME system env variable setting */
        char    *loginenv;       /* Static (malloced) location to hold
                                   LOGIN system env variable setting */
	extern	char *strdup();
        

#ifdef ADEBUG
        mydebug("setdefsenv: entered.\n");
#endif ADEBUG
        /*
         * Tell setpenv these are SYSTEM variables
        */

        env[(*nenv)++] = PENV_SYSSTR;
       /*
        * Place LOGNAME into system environment so that
        * logname will function correctly on hft shells
        * Duplicate the data area in case the env variable
        * is modified somewhere else
       */

       strcpy (buf , "LOGNAME=");
       strcat (buf , user);
       lognmenv = strdup (buf);
       env[(*nenv)++] = lognmenv;

     /*
      *Place LOGIN into system environment
      * Duplicate the data area in case the env variable
      * is modified somewhere else
     */

         strcpy (buf , "LOGIN=");
         strcat (buf , user);
         loginenv = strdup (buf);
         env[(*nenv)++] = loginenv;
                
        /*
        * End of environment variables
        */

        env[(*nenv) ++] = (char *) 0;

}

/*
 *  NAME:       tpath
 *
 *  FUNCTION:   Determine if user requested sak processing
 *
 *  EXECUTION ENVIRONMENT:
 *
 *      User process
 *
 *  RETURNS:
 *      If trusted path is requested for this user return non-zero.
 */

int
tpath (user)
char    *user;
{
        char    *tpathval = NULL;

        if (getuserattr (user, S_TPATH, (void *) &tpathval, (int) NULL))
                tpathval = "on";

        if (strcmp (tpathval, "nosak") == 0)
                user_tpath = TPATH_NOSAK;
        else if (strcmp (tpathval, "notsh") == 0)
                user_tpath = TPATH_NOTSH;
        else if (strcmp (tpathval, "always") == 0)
                user_tpath = TPATH_ALWAYS;
        else if (strcmp (tpathval, "on") == 0)
                user_tpath = TPATH_ON;
        else    /* Default case ... */
                user_tpath = TPATH_NOSAK;

        return user_tpath == TPATH_ALWAYS;
}
/*
 * NAME: findloginlog
 *                                                                    
 * FUNCTION: find the log entry for a tty
 *                                                                    
 * EXECUTION ENVIRONMENT: 
 *
 *		find record for tty in utmp and wtmp files
 *                                                                   
 * RETURNS: pointer to the username on that tty
 */  

char *
findloginlog (user, tty)
char	*user;
char	*tty;
{
	struct utmp outmp, *uptr;

	if (!strncmp(tty, "/dev/", 5))
		tty += 5;
	strncpy(outmp.ut_id, tty, sizeof(outmp.ut_id));
	setutent();
	if (uptr = getutid(&outmp))
		strcpy(user, uptr->ut_user);
	else
		user = 0;
	endutent();
	return user;
}
#endif _CSECURITY
