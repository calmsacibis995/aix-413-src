static char sccsid[] = "@(#)21        1.27  src/tcpip/usr/bin/rlogin/rlogin.c, tcprcmds, tcpip41J, 9513A_all 3/22/95 14:54:35";
/* 
 * COMPONENT_NAME: TCPIP rlogin.c
 * 
 * FUNCTIONS: MSGSTR, Mrlogin, catchild, doit, done, echo, lostpeer, 
 *            mode, oob, prf, reader, sendwindow, sigmask, sigwinch, 
 *            stop, writer, writeroob 
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
" Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "rlogin.c        5.10 (Berkeley) 3/30/86";
#endif not lint
*/

#include <nl_types.h>
nl_catd catd; 
#include "rlogin_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_RLOGIN, n, s) 

/*
 * rlogin - remote login
 */
/*
 * aixbsd signal.h causes failure of vi to recognize terminal on remote
 * and problems exiting (passing quit signal back). Must be included
 * ahead of param.h which also includes signal.h 
 */
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/pty.h>		
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <setjmp.h>
#include <netdb.h>

# ifndef TIOCPKT_WINDOW
# define TIOCPKT_WINDOW 0x80
# endif TIOCPKT_WINDOW

char	*strchr(), *strrchr(), *malloc(), *getenv();
struct	passwd *getpwuid();
char	*name;
int	rem;
char	cmdchar = '~';
int	eight;
int	litout;
char	term[256] = "network";
extern	int errno;
int	lostpeer();
int	dosigwinch = 0;
struct termios deftio;		/* origional termios bits & characters */
struct	winsize winsize;
int	sigwinch();
int	oob();

/* POSIX TERMIOS */
char *speeds(speed) 
	speed_t speed;
{
	switch(speed) {
		case B0 : return("0");
		case B50 : return("50");
		case B75 : return("75");
		case B110 : return("110");
		case B134 : return("134");
		case B150 : return("150");
		case B200 : return("200");
		case B300 : return("300");
		case B600 : return("600");
		case B1200 : return("1200");
		case B1800 : return("1800");
		case B2400 : return("2400");
                case B4800 : return("4800");
		case B9600 : return("9600");
		case B19200 : return("19200");
		case B38400 : return("38400");
		default : return("0");
	}
}

#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
	char *host, *cp;
	struct termios ttyb;
	struct passwd *pwd;
	struct servent *sp;
	int uid, options = 0, oldmask;
	int on = 1;
	char hostnamebuf[MAXHOSTNAMELEN];

	setlocale(LC_ALL,"");
	catd = catopen(MF_RLOGIN, NL_CAT_LOCALE); 
	host = strrchr(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rlogin")) /* arg0 = rlogin or rhost ? */
		host = *argv++, --argc;
another:
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc == 0)
			goto usage;
		name = *argv++; argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		cmdchar = argv[0][2];
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-8")) {
		eight = 1;
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-L")) {
		litout = 1;
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argc > 0)
		goto usage;
{
	long address;
	struct hostent *ha = 0;
       
	/* see if it's a name or an address */
        if (!isinet_addr(host)) {
                if ((ha = gethostbyname (host)) == (struct hostent *) 0) {
                        fprintf(stderr,MSGSTR(NME_NT_FND,
                                            "host: name %s NOT FOUND\n"), host);
                        exit(1);
                }
        }
        else {
		if ((address = inet_addr(host)) == -1) {
			fprintf(stderr, MSGSTR(BADADDR,
					"%s: address misformed\n"),host);
			exit(2);
			}
		if ((ha = gethostbyaddr(&address, sizeof(address),
                                        AF_INET)) == (struct hostent *)0) {
			fprintf(stderr, MSGSTR(BADHOST,
                                               "%s: host not found\n"),host);
			exit(2);
			}
		(void) strncpy(hostnamebuf, ha->h_name, sizeof(hostnamebuf));
		host = hostnamebuf;
	    }	
}
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, MSGSTR(UIDERR, "Who are you?\n")); /*MSG*/
		exit(1);
	}
	sp = getservbyname("login", "tcp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKSERVER, "rlogin: login/tcp: unknown service\n")); /*MSG*/
		exit(2);
	}
	cp = getenv("TERM");
	if (cp)
		strcpy(term, cp);
	if (tcgetattr(0, &ttyb) == 0) {
		strcat(term, "/");
		strcat(term, speeds(cfgetospeed(&ttyb)));
	}
#ifdef TIOCGWINSZ
	(void) ioctl(0, TIOCGWINSZ, &winsize);
#else
	(void) ioctl(0, TIOCGSIZE, &winsize);
#endif
	signal(SIGPIPE, lostpeer);
	signal(SIGURG, oob);
	oldmask = sigblock(sigmask(SIGURG));
        rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    name ? name : pwd->pw_name, term, 0);
        if (rem < 0)
                exit(1);
	if (options & SO_DEBUG &&
	    setsockopt(rem, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		perror(MSGSTR(SETSOCKERR, "rlogin: setsockopt (SO_DEBUG)")); /*MSG*/
	uid = getuid();
	if (setuid(uid) < 0) {
		perror(MSGSTR(SETUIDERR, "rlogin: setuid")); /*MSG*/
		exit(1);
	}
	doit(oldmask);
	/*NOTREACHED*/
usage:
	fprintf(stderr,
	    MSGSTR(USEAGE, "usage: rlogin host [ -ex ] [ -l username ] [ -8 ] \n")); /*MSG*/
	exit(1);
}

#define CRLF "\r\n"

int	child;
int	catchild();
int	writeroob();


doit(oldmask)
{
	int exit();

	/* save origional flags and chars as defaults */
	tcgetattr(0, &deftio);
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, exit);
	signal(SIGQUIT, exit);
	child = fork();
	if (child == -1) {
		perror(MSGSTR(FORKERR, "rlogin: fork")); /*MSG*/
		done(1);
	}
	if (child == 0) {
		mode(1);
		sigsetmask(oldmask);
		if (reader() == 0) {
			prf(MSGSTR(CLSCON, "Connection closed.")); /*MSG*/
			done(0); /* die b4 catchchild setup, call done */
		}
		sleep(1);
		prf(MSGSTR(CLSCON2, "\007Connection closed.")); /*MSG*/
		exit(3);
	}
	signal(SIGURG, writeroob);
	sigsetmask(oldmask);
	signal(SIGCHLD, catchild);
	writer();
	prf(MSGSTR(CLSCON3, "Closed connection.")); /*MSG*/
	done(0);
}

done(status)
	int status;
{

	mode(0);
	if (child > 0 && kill(child, SIGKILL) >= 0)
		wait((int *)0);
	exit(status);
}

/*
 * This is called when the reader process gets the out-of-band (urgent)
 * request to turn on the window-changing protocol.
 */
writeroob()
{

	if (dosigwinch == 0) {
		sendwindow();
		signal(SIGWINCH, sigwinch);
	}
	dosigwinch = 1;
}

catchild()
{
	union wait status;
	int pid;

again:
	pid = wait3(&status, WNOHANG|WUNTRACED, 0);
	if (pid == 0)
		return;
	/*
	 * if the child (reader) dies, just quit
	 */
	if (pid < 0 || pid == child && !WIFSTOPPED(status))
		done(status.w_termsig | status.w_retcode);
	goto again;
}

/*
 * writer: write to remote: 0 -> line.
 * ~.	terminate
 * ~^Z	suspend rlogin process.
 * ~^Y  suspend rlogin process, but leave reader alone.
 */
writer()
{
	char c;
	register n;
	register bol = 1;               /* beginning of line */
	register local = 0;

	for (;;) {
		n = read(0, &c, 1);
		if (n <= 0) {
			if (n < 0 && errno == EINTR)
				continue;
			break;
		}
		/*
		 * If we're at the beginning of the line
		 * and recognize a command character, then
		 * we echo locally.  Otherwise, characters
		 * are echo'd remotely.  If the command
		 * character is doubled, this acts as a 
		 * force and local echo is suppressed.
		 */
		if (bol) {
			bol = 0;
			if (c == cmdchar) {
				bol = 0;
				local = 1;
				continue;
			}
		} else if (local) {
			local = 0;
			/* NOTE: sysV VEOF is same as VMIN, so can't use it!! */
			/*** if (c == '.' || c == deftio.c_cc[VEOF]) { ***/
			if( c == '.' ) {
				echo(c);
				break;
			}
			if (c == deftio.c_cc[VSUSP] ||
			    c == deftio.c_cc[VDSUSP]) {
				bol = 1;
				echo(c);
				stop(c);
				continue;
			}
			if (c != cmdchar)
				write(rem, &cmdchar, 1);
		}
		if (write(rem, &c, 1) == 0) {
			prf(MSGSTR(SENTLN, "line gone")); /*MSG*/
			break;
		}
		/* NOTE: VEOF in sysV is a.k.a. VMIN, so we don't use it! */
		/**bol = c == deftio.c_cc[VKILL] || c == deftio.c_cc[VEOF] ||**/
		bol = c == deftio.c_cc[VKILL] || 
		    c == deftio.c_cc[VINTR] || 
		    c == '\r' || c == '\n';
	}
}

echo(c)
register char c;
{
	char buf[8];
	register char *p = buf;

	c &= 0177;
	*p++ = cmdchar;
	if (c < ' ') {
		*p++ = '^';
		*p++ = c + '@';
	} else if (c == 0177) {
		*p++ = '^';
		*p++ = '?';
	} else
		*p++ = c;
	*p++ = '\r';
	*p++ = '\n';
	write(1, buf, p - buf);
}

stop(cmdc)
	char cmdc;
{
	mode(0);
	signal(SIGCHLD, SIG_IGN);
	kill(cmdc == deftio.c_cc[VSUSP] ? 0 : getpid(), SIGTSTP);
	signal(SIGCHLD, catchild);
	mode(1);
	sigwinch();			/* check for size changes */
}

#ifdef TIOCGWINSZ
sigwinch()
{
        struct winsize ws;

        if (dosigwinch && ioctl(0, TIOCGWINSZ, &ws) == 0 &&
            bcmp(&ws, &winsize, sizeof (ws))) {
                winsize = ws;
                sendwindow();
        }
}
#else
sigwinch()
{
        struct ttysize ws;

        if (dosigwinch && ioctl(0, TIOCGSIZE, &ws) == 0 &&
            bcmp(&ws, &winsize, sizeof (ws))) {
                winsize = ws;
                sendwindow();
        }
}
#endif

/*
 * Send the window size to the server via the magic escape
 */
sendwindow()
{
	char obuf[4 + sizeof (struct winsize)];
	struct winsize *wp = (struct winsize *)(obuf+4);

	obuf[0] = 0377;
	obuf[1] = 0377;
	obuf[2] = 's';
	obuf[3] = 's';
#ifdef TIOCGWINSZ
        wp->ws_row = htons(winsize.ws_row);
        wp->ws_col = htons(winsize.ws_col);
        wp->ws_xpixel = htons(winsize.ws_xpixel);
        wp->ws_ypixel = htons(winsize.ws_ypixel);
#else
        wp->ws_row = htons(winsize.ts_lines);
        wp->ws_col = htons(winsize.ts_cols);
        wp->ws_xpixel = 0;
        wp->ws_ypixel = 0;
#endif 
	(void) write(rem, obuf, sizeof(obuf));
}

/*
 * reader: read from remote: line -> 1
 */
#define	READING	TCIFLUSH
#define	WRITING	TCOFLUSH

char	rcvbuf[8 * 1024];
int	rcvcnt;
int	rcvstate;
int	ppid;
jmp_buf	rcvtop;

oob()
{
	int out = WRITING, atmark, n;
	int rcvd = 0;
	char waste[BUFSIZ], mark;
	struct termios tio;

	while (recv(rem, &mark, 1, MSG_OOB) < 0) {
		switch (errno) {
		
		case EWOULDBLOCK:
			/*
			 * Urgent data not here yet.
			 * It may not be possible to send it yet
			 * if we are blocked for output
			 * and our input buffer is full.
			 */
			if (rcvcnt < sizeof(rcvbuf)) {
				n = read(rem, rcvbuf + rcvcnt,
					sizeof(rcvbuf) - rcvcnt);
				if (n <= 0)
					return;
				rcvd += n;
				/*
				 * need to add to rcvcnt so that the reader()
				 * function will know how much to write.
				 */
				rcvcnt += n;
			} else {
				n = read(rem, waste, sizeof(waste));
				if (n <= 0)
					return;
			}
			continue;
				
		default:
			return;
		}
	}
	if (mark & TIOCPKT_WINDOW) {
		/*
		 * Let server know about window size changes
		 */
		kill(ppid, SIGURG);
	}
	if (!eight && (mark & TIOCPKT_NOSTOP)) {
		tcgetattr(0, &tio);
		/* net effect is raw 8 bits */
		tio.c_iflag &= ~(ISTRIP|IXON);		/* clear "CBREAK" */
		tio.c_lflag &= ~(ICANON|ISIG);		/* set "RAW" */
		tio.c_cflag &= ~(CS7|PARENB);
		tio.c_cflag |= CS8;
		tio.c_iflag &= 
		    ~(BRKINT|IGNPAR|ISTRIP|INLCR|ICRNL|IGNCR|IXON|IXANY|IUCLC);
		tio.c_oflag &= ~(OPOST);
		tio.c_cc[VTIME] = 0;
		tio.c_cc[VMIN]  = 1;
		tcsetattr(0, TCSANOW, &tio);
		tio.c_cc[VSTOP] = -1;
		tio.c_cc[VSTART]  = -1;
	}
	if (!eight && (mark & TIOCPKT_DOSTOP)) {
		tcgetattr(0, &tio);
		tio.c_lflag |= (ICANON|ISIG);	/* clear "RAW" */
		tio.c_iflag |= (IXON|IGNPAR);
		tio.c_oflag |= (OPOST);	/* TAB3 */
/* gsj		tio.c_cflag &= ~(CS8);				*/
/* gsj		tio.c_cflag |= (CS7|PARENB);			*/
		tio.c_lflag &= ~(ICANON|ISIG);	/* set "CBREAK" */
		tio.c_cc[VTIME] = 0;
		tio.c_cc[VMIN]  = 1;
		tio.c_cc[VSTOP] = deftio.c_cc[VSTOP];
		tio.c_cc[VSTART]  = deftio.c_cc[VSTART];
		tcsetattr(0, TCSANOW, &tio);
		/* sysV/AIX has no settable stop/start chars */
	}
	if (mark & TIOCPKT_FLUSHWRITE) {
		tcflush(1, out);
		for (;;) {
			if (ioctl(rem, SIOCATMARK, &atmark) < 0) {
				perror(MSGSTR(IOCTLERR, "ioctl")); /*MSG*/
				break;
			}
			if (atmark)
				break;
			n = read(rem, waste, sizeof (waste));
			if (n <= 0)
				break;
		}
		/*
		 * Don't want any pending data to be output,
		 * so clear the recv buffer.
		 * If we were hanging on a write when interrupted,
		 * don't want it to restart.  If we were reading,
		 * restart anyway.
		 */
		rcvcnt = 0;
		longjmp(rcvtop, 1);
	}
	/*
	 * If we filled the receive buffer while a read was pending,
	 * longjmp to the top to restart appropriately.  Don't abort
	 * a pending write, however, or we won't know how much was written.
	 */
	if (rcvd && rcvstate == READING)
		longjmp(rcvtop, 1);
}

/*
 * reader: read from remote: line -> 1
 */
reader()
{
	int pid = getpid();
	int n, remaining;
	char *bufp = rcvbuf;

	n =   ioctl(rem, SIOCSPGRP, &pid);
	ppid = getppid();
	(void) setjmp(rcvtop);
	for (;;) {
		while ((remaining = rcvcnt - (bufp - rcvbuf)) > 0) {
			rcvstate = WRITING;
			n = write(1, bufp, remaining);
			if (n < 0) {
				if (errno != EINTR)
					return (-1);
				continue;
			}
			bufp += n;
		}
		bufp = rcvbuf;
		rcvcnt = 0;
		rcvstate = READING;
		rcvcnt = read(rem, rcvbuf, sizeof (rcvbuf));
		if (rcvcnt == 0)
			return (0);
		if (rcvcnt < 0) {
			if (errno == EINTR)
				continue;
			perror(MSGSTR(RDERR, "read")); /*MSG*/
			return (-1);
		}
	}
}

mode(f)
{
	struct termios tio;
	register int i;

	tcgetattr(0, &tio);

	switch (f) {
	case 0:
		/* restore origional state */
		tio = deftio;
		break;

	case 1:
		/* set state for rlogin useage */
		if( eight ) {			/* set "RAW" */
			tio.c_cflag &= ~(CSIZE);  /* gsj 01-08-88	*/
			tio.c_cflag |= CS8;
			tio.c_lflag &= ~(ICANON|ISIG|ECHO|ECHOE|ECHOK);
			tio.c_iflag &= ~(IGNBRK|BRKINT|INLCR|ICRNL|IXON);
			tio.c_oflag &= ~(OPOST);
		} else {			/* set "CBREAK" */
/* gsj 01-08-88		tio.c_cflag |= CS7;			*/
			tio.c_lflag &= ~(ICANON|ISIG|ECHO|ECHOE|ECHOK);
/* dab 9/29/88 */	tio.c_iflag |= (IXON);
			tio.c_iflag &= ~(IGNBRK|INLCR|ICRNL);
			tio.c_oflag = 0;
			tio.c_oflag |= (OPOST);	/* TAB3 */
		}
		tio.c_cc[VMIN] = 1;
		tio.c_cc[VTIME] = 0;

		/* preserve tab delays, but turn off XTABS (TAB3) */
		if ((tio.c_oflag & TABDLY) == TAB3)
			tio.c_oflag &= ~TABDLY;
		tio.c_cc[VKILL] = -1;
		tio.c_cc[VERASE] = -1;
/*XXX NYD MAYBE we don't support -L flag on rlogin , ok ??? */
/***		if (litout) ***/
/***			lflags |= LLITOUT; ***/
		break;

	default:
		return;
	}
	tcsetattr(0, TCSANOW, &tio);
}

/*VARARGS*/
prf(f, a1, a2, a3, a4, a5)
	char *f;
{
	fprintf(stderr, f, a1, a2, a3, a4, a5);
	fprintf(stderr, CRLF);
}

lostpeer()
{
	signal(SIGPIPE, SIG_IGN);
	prf(MSGSTR(CLSCONPEER, "\007Connection closed, lost peer.")); /*MSG*/
	done(1);
}

