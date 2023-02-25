static char sccsid[] = "@(#)23	1.55.1.23  src/tcpip/usr/lbin/rlogind/rlogind.c, tcprcmds, tcpip41J, 9519A_all 5/4/95 14:45:01";
/* 
 * COMPONENT_NAME: TCPIP rlogind.c
 * 
 * FUNCTIONS: MSGSTR, SUPERUSER, checknologin, cleanup, control, do_getstr,
 *            do_rlogin, doit, fatal, fatalperror, local_domain, loginlog,
 *            protocol, setup_term, trace_handler 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
 * CHANGE_ACTIVITY
 *
 * $L1=OEOCS    HOM???? 950131 PDXO:  Change to add ioctls conditioning
 *			connections for rlogin protocol.  Ioctls were
 *			moved from after parent_only: and so must be
 *			duplicated before the goto parent_only in the
 *			OCS virtual host processing case.
 */
#include <nl_types.h>
#include "rlogind_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_RLOGIND, n, s)

/*
 * remote login server:
 *	\0
 *	remuser\0
 *	locuser\0
 *	terminal_type/speed\0
 *	data
 *
 * Automatic login protocol is done here, using login -f upon success,
 * unless OLD_LOGIN is defined (then done in login, ala 4.2/4.3BSD).
 */
 
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/lockf.h>

#include <netinet/in.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <userpw.h>

#ifdef _AIX
#include <utmp.h>
/* POSIX TERMIOS */
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/acl.h>
#include <sys/access.h>
#else /* _AIX */
#include <sgtty.h>
#endif /* _AIX */

#include <stdio.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <strings.h>
#include <arpa/nameser.h>
#include "lm.h"                  /* OCS */
#include "ocsvhost.h"            /* OCS */

#ifdef _AIX
#define DEF_UTMP	"Active tty entry not found in utmp file.\n"

#define NOLOGIN         "/etc/nologin"

struct utmp utmp;
#define UT_NAMESIZE     sizeof(utmp.ut_user)

static void     loginlog();
#endif /* _AIX */

#ifndef TIOCPKT_WINDOW
#define TIOCPKT_WINDOW 0x80
#endif

char	*env[2];
#define	NMAX 30
char	lusername[NMAX+1], rusername[NMAX+1];
static	char term[64] = "TERM=";
#define	ENVSIZE	(sizeof("TERM=")-1)	/* skip null for concatenation */
int	keepalive = 1;
int	tracing = 0;

#define	SUPERUSER(pwd)	((pwd)->pw_uid == 0)

extern  char    *inet_ntoa();       /* OCS */
struct OCSconnection *findvhost(struct sockaddr_in *);  /* OCS */

struct	passwd *pwd;
char	*ttyname(int);
#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
	extern int opterr, optind, _check_rhosts_file;
	int ch;
	int on = 1, fromlen;
	struct sockaddr_in from;
	int tolen;                                                /* OCS */
    	struct sockaddr_in to;    /* local socket w/destination addr OCS */
	struct sigvec sv;
	void trace_handler(int);

	setlocale(LC_ALL,"");
	catd = catopen(MF_RLOGIND,NL_CAT_LOCALE);
	openlog("rlogind", LOG_PID | LOG_CONS, LOG_AUTH);

	opterr = 0;
	while ((ch = getopt(argc, argv, "lns")) != EOF)
		switch (ch) {
		case 'l':
			_check_rhosts_file = 0;
			break;
		case 'n':
			keepalive = 0;
			break;
		case 's':
			tracing = 1;
			break;
		case '?':
		default:
			syslog(LOG_ERR, "usage: rlogind [-l] [-n] [-s]");
			break;
		}
	argc -= optind;
	argv += optind;

	/*
         * Obtain destination internet address from the current rlogin
         * request(local socket) so that we can determine if this address
         * is defined as a virtual host address in the ODM
         * database.
         * OCS
         */
	tolen = sizeof (to);                                /* OCS */
    	if (getsockname(0, &to, &tolen) < 0) {              /* OCS */
       	    syslog(LOG_ERR,  MSGSTR(SOCKNM,
		"Couldn't get socket name of remote host: %m")); /*MSG*/
            fatalperror(MSGSTR(SOCKNMERR, "getsockname")); /*MSG*/
            }                                               /* OCS */

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		syslog(LOG_ERR,  MSGSTR(PEERNM,
		"Couldn't get peer name of remote host: %m")); /*MSG*/
		fatalperror(MSGSTR(PEERNMERR, "getpeername")); /*MSG*/
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING, MSGSTR(SETSOCKOPT,
			"setsockopt (SO_KEEPALIVE): %m")); /*MSG*/
	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,
			"setsockopt (SO_DEBUG): %m")); /*MSG*/

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = (void(*)(int))trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = (void(*)(int))trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);

	doit(0, &from, &to);                    /* OCS */	
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
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/
}

void cleanup();
pid_t pid;
int	netf;
char	*line;

extern	char	*inet_ntoa();

struct winsize win = { 0, 0, 0, 0 };

doit(f, fromp, local)
	int f;
	struct sockaddr_in *fromp;
	struct sockaddr_in *local;                    /* OCS */
{
	int i, p, t, on = 1;
	struct OCSconnection *OCScon_ptr;             /* OCS */
    	LM_VTP_VAR rlm;                               /* OCS */
    	register char *cp,**cpp;                      /* OCS */
    	char *speed;                                  /* OCS */
#ifndef OLD_LOGIN
	int authenticated = 0, hostok = 0;
#endif
	struct	hostent *hp;
	char remotehost[MAXDNAME];

	char c;

	alarm(60);
	read(f, &c, 1);
	if (c != 0)
		exit(1);

	alarm(0);
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	hp = gethostbyaddr(&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp == 0) {
		strcpy(remotehost, inet_ntoa(fromp->sin_addr));
#ifndef OLD_LOGIN
		hostok++;
#endif
	}
#ifndef OLD_LOGIN
	else {
	    strcpy(remotehost, hp->h_name);
	    if (local_domain(remotehost)) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		hp = gethostbyname(remotehost);
		if (hp)
		    for (; hp->h_addr_list[0]; hp->h_addr_list++)
			if (!bcmp(hp->h_addr_list[0], (caddr_t)&fromp->sin_addr,
			    sizeof(fromp->sin_addr))) {
				hostok++;
				break;
			}
	    } else
		    hostok++;
	}
#endif

	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, MSGSTR(ILLEGALPORT,"Connection from %s on illegal port"),
			inet_ntoa(fromp->sin_addr)); /*MSG*/
		fatal(f, MSGSTR(NOPERMIT,"Permission denied")); /*MSG*/
	}
#ifdef IP_OPTIONS
      {
	u_char optbuf[BUFSIZ/3], *cp;
	char lbuf[BUFSIZ], *lp;
	int optsize = sizeof(optbuf), ipproto;
	struct protoent *ip;

	if ((ip = getprotobyname("ip")) != NULL)
		ipproto = ip->p_proto;
	else
		ipproto = IPPROTO_IP;
	if (getsockopt(0, ipproto, IP_OPTIONS, (char *)optbuf, &optsize) == 0 &&
	    optsize != 0) {
		lp = lbuf;
		for (cp = optbuf; optsize > 0; cp++, optsize--, lp += 3)
			sprintf(lp, " %2.2x", *cp);
		syslog(LOG_NOTICE, MSGSTR(INVIPOPT,
		    "Connection received using IP options (ignored):%s"), lbuf); /*MSG*/
		if (setsockopt(0, ipproto, IP_OPTIONS,
		    (char *)NULL, &optsize) != 0) {
			syslog(LOG_ERR, MSGSTR(NULLIPOPT, "setsockopt IP_OPTIONS NULL: %m")); /*MSG*/
			exit(1);
		}
	}
      }
#endif
	write(f, "", 1);
	/*
         * Determine if the destination address found in the
         * local rlogin request is a virtual host address
         * OCS
         */
        OCScon_ptr = findvhost(local);                            /* OCS */

#ifndef OLD_LOGIN
	if (do_rlogin(remotehost, OCScon_ptr, f) == 0) {		  /* OCS */
		if (hostok)
		    authenticated++;
		else
		    write(f, MSGSTR(ADMISMATCH, "rlogind: Host address mismatch.\r\n"), strlen(MSGSTR(ADMISMATCH, "rlogind: Host address mismatch.\r\n")) - 1); /*MSG*/
	}
#endif
#ifdef _AIX
	(void) setsid();
        if ((p = open("/dev/ptc", O_RDWR)) >= 0){
                line = ttyname(p);
	}
        else {
                fatal(f, MSGSTR(NOPTY, "Out of ptys")); /*MSG*/
        }

#else
	for (c = 'p'; c <= 's'; c++) {
		struct stat stb;
		line = "/dev/ptyXX";
		line[strlen("/dev/pty")] = c;
		line[strlen("/dev/ptyp")] = '0';
		if (stat(line, &stb) < 0)
			break;
		for (i = 0; i < 16; i++) {
			line[sizeof("/dev/ptyp") - 1] = "0123456789abcdef"[i];
			p = open(line, O_RDWR);
			if (p > 0)
				goto gotpty;
		}
	}
	fatal(f, MSGSTR(NOPTY, "Out of ptys")); /*MSG*/
	/*NOTREACHED*/
gotpty:
#endif /* _AIX */
#ifdef	TIOCSWINSZ
	(void) ioctl(p, TIOCSWINSZ, &win);
#else
#ifdef	TXSETWINSZ
	(void) ioctl(p, TXSETWINSZ, &win);
#endif	/* TXSETWINSZ */
#endif	/* TIOCSWINSZ */
	netf = f;
	
        /*
         * For OCS Support:
         * If the local rlogin request is a virtual host address,
         * then bypass normal rlogind processing dealing with
         * opening the slave side of the pty and a child process
         * exec-ing login.
         *
         * Instead, set up rlogind variables for the Login Monitor
         * to perform authentication on AIX/ESA, invoke
         * process_vhost(), and then put up a select() on the
         * master side of the pty.
         *
         * OCS
         */
	if (OCScon_ptr) {                                     /* OCS */
		rlm.vtp_type = RLOGIND_VTP;                   /* OCS */
		strncpy(rlm.u.rv.rusername, rusername,
		    sizeof(rlm.u.rv.rusername));              /* OCS */
		strncpy(rlm.u.rv.lusername, lusername,
		    sizeof(rlm.u.rv.lusername));              /* OCS */
		strncpy(rlm.u.rv.rhostname, remotehost,
		    sizeof(rlm.u.rv.rhostname));              /* OCS */
		/*
		* Since term = "TERM=terminaltype/speed\0",
		* must break it up for the AIX/ESA Login Monitor
		* OCS
		*/
		if ((cp = index(term, '/')) != NULL){
		*cp++ = '\0';     /* null terminate "TERM=" environ variable */
		speed = cp;       /* set ASCII speed */
		if ((cp = index(speed, '/')) != NULL)
		    *cp++ = '\0';  /* null terminate ASCII speed */
		}			                      /* OCS */
		strncpy(rlm.u.rv.term_env0, term,
		    sizeof(rlm.u.rv.term_env0));              /* OCS */
		strncpy(rlm.u.rv.speed, speed,
		    sizeof(rlm.u.rv.speed));                  /* OCS */
		process_vhost(OCScon_ptr, line, &rlm);        /* OCS */
		pid = -1;           /* no child process for OCS support  OCS */
		/* following ioctls were moved out of the common     @L1A */
		/* path after parent_only: thus must be duplicated   @L1A */
		/* here						     @L1A */
		ioctl(f, FIONBIO, &on);  /* @L1A */
		ioctl(p, FIONBIO, &on);  /* @L1A */
		ioctl(p, TIOCPKT, &on);  /* @L1A */
		goto parent_only;                             /* OCS */
	} /* End of virtual host processing */
	
#ifndef _AIX
	line[strlen("/dev/")] = 't';
#endif _AIX
 	t = open(line, O_NOCTTY|O_RDWR);
	if (t < 0)
		fatalperror(f, line);
	if (fchmod(t, 0))
		fatalperror(f, line);
#ifndef _AIX 
	(void)signal(SIGHUP, SIG_IGN);
  	vhangup();
	(void)signal(SIGHUP, SIG_DFL);
	t = open(line, O_RDWR);
	if (t < 0)
		fatalperror(f, line);
#else
	frevoke(t);
#endif _AIX
	setup_term(t);

	ioctl(f, FIONBIO, &on);
	ioctl(p, FIONBIO, &on);
	ioctl(p, TIOCPKT, &on);

	pid = fork();
	if (pid < 0)
		fatalperror(f, "");
	if (pid == 0) {
#ifdef _AIX
		(void) setsid();
		t = open(line, O_RDWR);
		if (t < 0)
			fatalperror(f, line);
		if (acl_fset(t, NO_ACC, NO_ACC, NO_ACC))
			fatalperror(f, line);
		/* frevoke(t); */

                {
                struct sigvec sv;
                bzero((char *)&sv, sizeof(sv));
                sv.sv_handler = SIG_DFL;
                sigvec(SIGQUIT, &sv, (struct sigvec *)0);
                sv.sv_handler = SIG_DFL;
                sigvec(SIGHUP, &sv, (struct sigvec *)0);
                }

#endif /* _AIX */
		close(f), close(p);
		dup2(t, 0), dup2(t, 1), dup2(t, 2);
		close(t);

                /*
                 * Reset SIGUSR1 and SIGUSR2 to non-restartable so children
                 * of rlogind do not get clobbered by the way rlogind handles
                 * these signals.
                 */
                {
                struct sigvec sv;
                void trace_handler(int);

                bzero((char *)&sv, sizeof(sv));
                sv.sv_mask = sigmask(SIGUSR2);
                sv.sv_handler = (void(*)(int))trace_handler;
                sv.sv_onstack = SV_INTERRUPT;
                sigvec(SIGUSR1, &sv, (struct sigvec *)0);
                sv.sv_mask = sigmask(SIGUSR1);
                sv.sv_handler = (void(*)(int))trace_handler;
                sv.sv_onstack = SV_INTERRUPT;
                sigvec(SIGUSR2, &sv, (struct sigvec *)0);
                }

		(void) loginlog(lusername, line, remotehost);

#ifdef OLD_LOGIN
		execl("/bin/login", "login", "-r", remotehost, 0);
#else /* OLD_LOGIN */

		if (authenticated)
			execl("/bin/login", "login", "-p", "-h", remotehost,
			    "-f", "--", lusername, 0);
		else
			execl("/bin/login", "login", "-p", "-h", remotehost,
			    "--", lusername, 0);
#endif /* OLD_LOGIN */
		fatalperror(2, MSGSTR(LOGINERR, "/bin/login")); /*MSG*/
		/*NOTREACHED*/
	}
	close(t);

parent_only:                                    /* OCS */
	(void)signal(SIGTSTP, SIG_IGN);
	(void)signal(SIGCHLD, (void (*)(int))cleanup);
	set_nocldstop(); /* Do not receive SIGCHLD if child stops */
#ifndef _AIX
	(void) setpgrp(0,0);
#endif /* _AIX */
	protocol(f, p,pid);
	(void)signal(SIGCHLD, SIG_IGN);
	close(p);
	close(f);
	cleanup();
}

char	magic[2] = { 0377, 0377 };
char	oobdata[] = {TIOCPKT_WINDOW};

/*
 * Handle a "control" request (signaled by magic being present)
 * in the data stream.  For now, we are only willing to handle
 * window size changes.
 */
control(pty, cp, n)
	int pty;
	char *cp;
	int n;
{
	struct winsize w;

	if (n < 4+sizeof (w) || cp[2] != 's' || cp[3] != 's')
		return (0);
	oobdata[0] &= ~TIOCPKT_WINDOW;	/* we know he heard */
	bcopy(cp+4, (char *)&w, sizeof(w));
	w.ws_row = ntohs(w.ws_row);
	w.ws_col = ntohs(w.ws_col);
	w.ws_xpixel = ntohs(w.ws_xpixel);
	w.ws_ypixel = ntohs(w.ws_ypixel);

#ifdef	TIOCSWINSZ
	(void) ioctl(pty, TIOCSWINSZ, &w);
#else
	(void) ioctl(pty, TXSETWINSZ, &w);
#endif	
	return (4+sizeof (w));
}

/*
 * rlogin "protocol" machine.
 */
protocol(f, p,pid)
	int f, p;
	pid_t pid ;
{
	char pibuf[1024], fibuf[1024], *pbp, *fbp;
	register pcc = 0, fcc = 0;
	int cc, nfd, pmask, fmask;
	char cntl;

	/*
	 * Must ignore SIGTTOU, otherwise we'll stop
	 * when we try and set slave pty's window shape
	 * (our controlling tty is the master pty).
	 */
	(void) signal(SIGTTOU, SIG_IGN);
	send(f, oobdata, 1, MSG_OOB);	/* indicate new rlogin */
	if (f > p)
		nfd = f + 1;
	else
		nfd = p + 1;
	fmask = 1 << f;
	pmask = 1 << p;
	for (;;) {
		int ibits, obits, ebits;

		ibits = 0;
		obits = 0;
		if (fcc)
			obits |= pmask;
		else
			ibits |= fmask;
		if (pcc >= 0)
			if (pcc)
				obits |= fmask;
			else
				ibits |= pmask;
		ebits = pmask;
		if (select(nfd, &ibits, obits ? &obits : (int *)NULL,
		    &ebits, 0) < 0) {
			if (errno == EINTR)
				continue;
			close(p);
			close(f);
			cleanup();
			fatalperror(f, MSGSTR(SELERR, "select")); /*MSG*/
		}
		if (ibits == 0 && obits == 0 && ebits == 0) {
			/* shouldn't happen... */
			sleep(5);
			continue;
		}
#define	pkcontrol(c)	((c)&(TIOCPKT_FLUSHWRITE|TIOCPKT_NOSTOP|TIOCPKT_DOSTOP))
		if (ebits & pmask) {
			cc = read(p, &cntl, 1);
			if (cc == 1 && pkcontrol(cntl)) {
				cntl |= oobdata[0];
				send(f, &cntl, 1, MSG_OOB);
				if (cntl & TIOCPKT_FLUSHWRITE) {
					pcc = 0;
					ibits &= ~pmask;
				}
			}
		}
		if (ibits & fmask) {
			fcc = read(f, fibuf, sizeof(fibuf));
			if (fcc < 0 && errno == EWOULDBLOCK)
				fcc = 0;
			else {
				register char *cp;
				int left, n;

				if (fcc <= 0)
					break;
				fbp = fibuf;

			top:
				for (cp = fibuf; cp < fibuf+fcc-1; cp++)
					if (cp[0] == magic[0] &&
					    cp[1] == magic[1]) {
						left = fcc - (cp-fibuf);
						n = control(p, cp, left);
						if (n) {
							left -= n;
							if (left > 0)
								bcopy(cp+n, cp, left);
							fcc -= n;
							goto top; /* n^2 */
						}
					}
				obits |= pmask;		/* try write */
			}
		}

		if ((obits & pmask) && fcc > 0) {
			cc = write(p, fbp, fcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/* also shouldn't happen */
				sleep(1);
				continue;
			}
			if (cc > 0) {
				fcc -= cc;
				fbp += cc;
			}
		}

		if (ibits & pmask) {
			pcc = read(p, pibuf, sizeof (pibuf));
			pbp = pibuf;
			if (pcc < 0 && errno == EWOULDBLOCK)
				pcc = 0;
			else if (pcc <= 0)
				break;
			else if (pibuf[0] == 0) {
				pbp++, pcc--;
				obits |= fmask;	/* try a write */
			} else {
				if (pkcontrol(pibuf[0])) {
					pibuf[0] |= oobdata[0];
					send(f, &pibuf[0], 1, MSG_OOB);
				}
				pcc = 0;
			}
		}
		if ((obits & fmask) && pcc > 0) {
			cc = write(f, pbp, pcc);
			if (cc < 0 && errno == EWOULDBLOCK) {
				/* also shouldn't happen */
				sleep(1);
				continue;
			}
			if (cc > 0) {
				pcc -= cc;
				pbp += cc;
			}
		}
	}
}

void
cleanup()
{
	char *p;
	struct utmp cutmp, *cptr, *getutline(), *pututline();
	int status;
        sigset_t newset;

	/*
         * For OCS support:
         * Since there is no child process, then
         * just shutdown and exit
         * OCS
         */
        if (pid == -1) {                            /* OCS */
		shutdown(netf, 2);                  /* OCS */
		exit(1);                            /* OCS */
        }                                           /* OCS */	 

	while (waitpid(pid, &status, 0) == -1 && errno == EINTR);

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
	} else {
		kill(pid, SIGKILL);
		syslog(LOG_WARNING, MSGSTR(BAD_UTMP, DEF_UTMP));
	}
	/*
	if (logout(p))
		logwtmp(p, "", "");
	*/

	/* Release floating iFOR/LS license (sigh) */
	_FloatingReleaseLicense(pid);

#ifdef _AIX
        (void) acl_set(line, R_ACC|W_ACC, R_ACC|W_ACC, R_ACC|W_ACC);
#else
	(void)chmod(line, 0666);
#endif /* _AIX */
	(void)chown(line, 0, 0);
	shutdown(netf, 2);
	exit(1);
}

fatal(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];

	buf[0] = '\01';		/* error indicator */
	(void) sprintf(buf + 1, MSGSTR(RLOGD, "rlogind: %s.\r\n"), msg); /*MSG*/
	(void) write(f, buf, strlen(buf));
	exit(1);
}

fatalperror(f, msg)
	int f;
	char *msg;
{
	char buf[BUFSIZ];
	extern int sys_nerr;
	extern char *sys_errlist[];

	if ((unsigned)errno < sys_nerr)
		(void) sprintf(buf, MSGSTR(FATAL, "%s: %s"), msg, sys_errlist[errno]); /*MSG*/
	else
		(void) sprintf(buf, MSGSTR(FATAL2, "%s: Error %d"), msg, errno); /*MSG*/
	fatal(f, buf);
}

#ifndef OLD_LOGIN
do_rlogin(host, OCScon_ptr, f)                                     /* OCS */
	char *host;
	struct OCSconnection *OCScon_ptr;                       /* OCS */
	int f;
{
        char *message = (char *)NULL;  /* message from passwdexpired */
        int passwdexp_rc;              /* return code from passwdexpired */



	do_getstr(rusername, sizeof(rusername), MSGSTR(REMUSER, "remuser too long")); /*MSG*/
	do_getstr(lusername, sizeof(lusername), MSGSTR(LOCUSER, "locuser too long")); /*MSG*/
	do_getstr(term+ENVSIZE, sizeof(term)-ENVSIZE, MSGSTR(TTTOOLONG, "Terminal type too long")); /*MSG*/

	/*
         * If the destination internet address is found in
         * the ODM database (OCSvhost object class), then
         * bypass authentication here since the AIX/ESA host
         * will perform this task.
         * OCS
         */
        if (OCScon_ptr != NULL)                             /* OCS */
            return(-1);                                     /* OCS */

	if (getuid())
		return(-1);
	pwd = getpwnam(lusername);
	if (pwd == NULL)
		return(-1);
#ifdef _AIX
        if (*pwd->pw_shell == '\0')
                pwd->pw_shell = "/bin/sh";
	/* check for disabled logins before */
	/* authenticating based on .rhosts  */
	if (pwd->pw_uid)
		if (checknologin() == 0)
		    return(-1);
#endif /* _AIX */
	/*
         * If the SYSTEM authentication is not being used (i.e.,
         * "SYSTEM = NONE"), then the passwords should not
         * be checked.
         */
        if (!SYSTEM_is_NONE(pwd->pw_name)) {
        	/* Check to see if the user's password expired */
        	passwdexp_rc = passwdexpired(pwd->pw_name, &message);
	}
        if (message) {
		if ((passwdexp_rc != 0)&&(passwdexp_rc != 1)) {
                	write(f, message, strlen(message));
		}
                free(message);
                message = (char *) NULL;
        }
        if (passwdexp_rc != 0)		/* password expired ? */
                return(-1);

	return(ruserok(host, SUPERUSER(pwd), rusername, lusername));
}


do_getstr(buf, cnt, errmsg)
	char *buf;
	int cnt;
	char *errmsg;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0)
			fatal(1, errmsg);
		*buf++ = c;
	} while (c != 0);
}

extern	char **environ;

#ifdef _AIX
/* POSIX TERMIOS */
speed_t speeds(speed)
	char *speed;
{
	if (strcmp(speed, "38400") == 0) return(B38400);
	if (strcmp(speed, "19200") == 0) return(B19200);
	if (strcmp(speed, "9600") == 0) return(B9600);
	if (strcmp(speed, "4800") == 0) return(B4800);
	if (strcmp(speed, "2400") == 0) return(B2400);
	if (strcmp(speed, "1800") == 0) return(B1800);
	if (strcmp(speed, "1200") == 0) return(B1200);
	if (strcmp(speed, "600") == 0) return(B600);
	if (strcmp(speed, "300") == 0) return(B300);
	if (strcmp(speed, "200") == 0) return(B200);
	if (strcmp(speed, "150") == 0) return(B150);
	if (strcmp(speed, "134") == 0) return(B134);
	if (strcmp(speed, "110") == 0) return(B110);
	if (strcmp(speed, "75") == 0) return(B75);
	if (strcmp(speed, "50") == 0) return(B50);
	if (strcmp(speed, "0") == 0) return(B0);
	return(B0);
}
#else
char *speeds[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600",
	"1200", "1800", "2400", "4800", "9600", "19200", "38400",
};
#define	NSPEEDS	(sizeof(speeds) / sizeof(speeds[0]))
#endif

setup_term(fd)
	int fd;
{
	register char *cp = index(term, '/'), **cpp;
#ifdef _AIX
	/* POSIX TERMIOS */
	struct termios termios;
#else /* _AIX */
	struct sgttyb sgttyb;
#endif /* _AIX */
	char *speed;

#ifdef _AIX
	/* POSIX TERMIOS */
	tcgetattr(fd, &termios);
#else /* _AIX */
	(void)ioctl(fd, TIOCGETP, &sgttyb);
#endif /* _AIX */
	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
#ifdef _AIX
		/* POSIX TERMIOS */
		/* Setup PTY with some reasonable defaults */
		termios.c_cc[VINTR]    = CINTR;
		termios.c_cc[VQUIT]    = CQUIT;
		termios.c_cc[VERASE]   = CERASE;
		termios.c_cc[VKILL]    = CKILL;
		termios.c_cc[VEOF]     = CEOF;
		termios.c_cc[VEOL]     = 0;
		termios.c_cc[VEOL2]    = 0;
		termios.c_cc[VSTART]   = CSTART;
		termios.c_cc[VSTOP]    = CSTOP;
		termios.c_cc[VSUSP]    = 'Z' - '@';
		termios.c_cc[VDSUSP]   = 'Y' - '@';
		termios.c_cc[VREPRINT] = 'R' - '@';
		termios.c_cc[VDISCRD]  = 'O' - '@';
		termios.c_cc[VWERSE]   = 'W' - '@';
		termios.c_cc[VLNEXT]   = 'V' - '@';
		/* For NLS environments, we need 8 bit data stream. */
		termios.c_iflag = IXON|BRKINT|IGNPAR|ICRNL|IMAXBEL;
		termios.c_cflag = CS8|HUPCL|CREAD;    

		/* original 7 bit ascii code */
/*		termios.c_iflag = IXON|BRKINT|IGNPAR|ISTRIP|ICRNL;   */
/*		termios.c_cflag = PARENB|CS7|HUPCL|CREAD;   */ 

		termios.c_oflag = OPOST|ONLCR|TAB3;
		termios.c_lflag = ISIG|ICANON|
				  ECHO|ECHOE|ECHOK|ECHOKE|ECHOCTL|
				  IEXTEN;
		cfsetispeed(&termios, speeds(speed));
		cfsetospeed(&termios, speeds(speed));
#else /* _AIX */
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
		    if (strcmp(*cpp, speed) == 0) {
			sgttyb.sg_ispeed = sgttyb.sg_ospeed = cpp - speeds;
			break;
		    }
#endif /* _AIX */
	}
#ifdef _AIX
#ifdef TXSETLD
	/* POSIX TERMIOS */
	ioctl(fd, TXSETLD, "posix");
#endif /* TXSETLD */
	tcsetattr(fd, TCSANOW, &termios);
#else /* _AIX */
	sgttyb.sg_flags = ECHO|CRMOD|ANYP|XTABS;
	(void)ioctl(fd, TIOCSETP, &sgttyb);
#endif /* _AIX */

	env[0] = term;
	env[1] = 0;
	environ = env;
}

#include <resolv.h>
extern struct state _res;
/*
 * Check whether host h is in our local domain,
 * as determined by the part of the name following
 * the first '.' in its name and in ours.
 * If either name is unqualified (contains no '.'),
 * assume that the host is local, as it will be
 * interpreted as such.
 */
local_domain(h)
	char *h;
{
	char localhost[MAXDNAME];
	char *p1, *p2 = index(h, '.');

	/*
	 * If _res.defdname is NULL then we are not using DNS,
	 * so the additional call to gethostbyname() is silly.
	 */
	if (!(_res.options & RES_INIT)) {
		return(0);
	}
	(void) gethostname(localhost, sizeof(localhost));
	p1 = index(localhost, '.');
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2))
		return(1);
	return(0);
}
#endif /* OLD_LOGIN */

#ifdef _AIX

checknologin()
{
        register int fd;

        if ((fd = open(NOLOGIN, O_RDONLY, 0)) >= 0) {
		close(fd);
		return(0);
        }
	else return(-1);
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
loginlog (user, tty, hostname)
char	*user;
char	*tty;
char	*hostname;
{
	struct utmp utmp;
 
	bzero(&utmp, sizeof(utmp));
	if (!strncmp(tty, "/dev/", 5))
		tty += 5;
	strncpy(utmp.ut_user, user, sizeof(utmp.ut_user));
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
#endif /* _AIX */

set_nocldstop()
{
struct sigaction oact;
	if(sigaction(SIGCHLD,NULL,&oact) == -1)
		return(-1);
	oact.sa_flags |= SA_NOCLDSTOP;
	sigaction(SIGCHLD,&oact,NULL);
}

