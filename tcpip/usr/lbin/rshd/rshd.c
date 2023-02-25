static char sccsid[] = "@(#)29	1.26.1.13  src/tcpip/usr/lbin/rshd/rshd.c, tcprcmds, tcpip41J, 9515A_all 3/29/95 14:32:49";
/* 
 * COMPONENT_NAME: TCPIP rshd.c
 * 
 * FUNCTIONS: MSGSTR, Mrshd, addenvvar, doit, error, rshd_getstr, 
 *            local_domain, trace_handler, set_logname, copy_pwd
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1988 The Regents of the University of California.
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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "rshd.c	5.17.1.2 (Berkeley) 2/7/89";
#endif  not lint */

/*
 * remote shell server:
 *	[port]\0
 *	remuser\0
 *	locuser\0
 *	command\0
 *	data
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

#include <stdio.h>
#include <usersec.h>
#include <userpw.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/syslog.h>
#include <sys/id.h>
#include <sys/priv.h>

#include <nl_types.h>
nl_catd catd;
#include "rshd_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_RSHD, n, s) 

#define RSHBUFSIZ 8192
#define PATHENV ":/usr/bin:/etc:/usr/sbin:/usr/ucb:/usr/bin/X11:/sbin:/bin"

#include <ctype.h>
#include <login.h>

int	errno;
int	keepalive = 1;
char	*index(), *rindex(), *strncat(), *getenv(), *malloc();
/*VARARGS1*/
int	error();
int	tracing = 0;
void    set_logname(char *);            /* declare function for setting LOGNAME */
                                        /* in environment */
struct passwd *copy_pwd(struct passwd *);     /* declare fcn to copy */
                                              /* password structure */

/*ARGSUSED*/
extern int _check_rhosts_file;

#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
	extern int opterr, optind;
	struct linger linger;
	int ch, on = 1, fromlen;
	struct sockaddr_in from;
	struct sigvec sv;
	void trace_handler();


	setlocale(LC_ALL,"");
	catd = catopen(MF_RSHD,NL_CAT_LOCALE);

	openlog("rshd", LOG_PID | LOG_ODELAY, LOG_DAEMON);

	opterr = 0;
	while ((ch = getopt(argc, argv, "lns")) != EOF)
		switch((char)ch) {
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
			syslog(LOG_ERR, MSGSTR(USAGE, "usage: rshd [-lns]"));
			break;
		}

	argc -= optind;
	argv += optind;

	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, "%s: ", argv[0]);
		perror(MSGSTR(PEERNM, "getpeername")); /*MSG*/
		_exit(1);
	}
	if (keepalive &&
	    setsockopt(0, SOL_SOCKET, SO_KEEPALIVE, (char *)&on,
	    sizeof(on)) < 0)
		syslog(LOG_WARNING, MSGSTR(SETSOCKOPT, "setsockopt (SO_KEEPALIVE): %m")); /*MSG*/
	linger.l_onoff = 1;
	linger.l_linger = 60;			/* XXX */
	if (setsockopt(0, SOL_SOCKET, SO_LINGER, (char *)&linger,
	    sizeof (linger)) < 0)
		syslog(LOG_WARNING, MSGSTR(SETSOCKLING, "setsockopt (SO_LINGER): %m")); /*MSG*/

	if (tracing &&
	    setsockopt(0, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/

	/* set-up signal handler routines for SRC TRACE ON/OFF support */
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = sigmask(SIGUSR2);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR1, &sv, (struct sigvec *)0);
	sv.sv_mask = sigmask(SIGUSR1);
	sv.sv_handler = trace_handler;
	sigvec(SIGUSR2, &sv, (struct sigvec *)0);

	doit(&from);
	return(0);
}

/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
void
trace_handler(sig)
	int	sig;
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(0, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/
}

extern char	**environ;

doit(fromp)
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp;
	char locuser[16], remuser[16];
	struct passwd *pwd, *tmp_pwd;
	int s;
	struct hostent *hp;
	char *p;
	short port;
	int pv[2], pid, cc;
	int nfd;
	fd_set ready, readfrom;
	char buf[RSHBUFSIZ], sig;
	int one = 1;
	int limit ;
	char remotehost[MAXDNAME];
        priv_t  priv;
        char *message = (char *) NULL;
        int passwdexp_rc = 0, logrestrict_rc = 0;
        int no_passwdf;

	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif DEBUG
	fromp->sin_port = ntohs((u_short)fromp->sin_port);
	if (fromp->sin_family != AF_INET ||
	    fromp->sin_port >= IPPORT_RESERVED) {
		syslog(LOG_ERR, MSGSTR(FRMADDRERR, "malformed from address\n")); /*MSG*/
		exit(1);
	}
#ifdef IP_OPTIONS
      {
	u_char optbuf[RSHBUFSIZ/3], *cp;
	char lbuf[RSHBUFSIZ], *lp;
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
		syslog(LOG_NOTICE,MSGSTR(CONN_IP, "Connection received using IP options (ignored):%s"), lbuf);
		if (setsockopt(0, ipproto, IP_OPTIONS,
		    (char *)NULL, &optsize) != 0) {
			syslog(LOG_ERR, MSGSTR(SETSCKIP, "setsockopt IP_OPTIONS NULL: %m"));
			exit(1);
		}
	}
      }
#endif

	if (fromp->sin_port >= IPPORT_RESERVED ||
	    fromp->sin_port < IPPORT_RESERVED/2) {
		syslog(LOG_NOTICE, MSGSTR(ILLEGAL_PORT, "Connection from %s on illegal port"),
			inet_ntoa(fromp->sin_addr));
		exit(1);
	}

	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if ((cc = read(0, &c, 1)) != 1) {
			if (cc < 0)
				syslog(LOG_ERR, MSGSTR(RDERR, "read: %m")); /*MSG*/
			shutdown(0, 1+1);
			exit(1);
		}
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}

	(void) alarm(0);
	if (port != 0) {
		int lport = IPPORT_RESERVED - 1;
		s = rresvport(&lport);
		if (s < 0) {
			syslog(LOG_ERR, MSGSTR(STDERR, "can't get stderr port: %m")); /*MSG*/
			exit(1);
		}
		if (port >= IPPORT_RESERVED) {
			syslog(LOG_ERR, MSGSTR(NO2PORT, "2nd port not reserved\n")); /*MSG*/
			exit(1);
		}
		fromp->sin_port = htons((u_short)port);
		if (connect(s, fromp, sizeof (*fromp)) < 0) {
			syslog(LOG_INFO, MSGSTR(NO2CONN, "connect second port: %m")); /*MSG*/
			exit(1);
		}
	}

#ifdef notdef
	/* from inetd, socket is already on 0, 1, 2 */
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
#endif
	hp = gethostbyaddr((char *)&fromp->sin_addr, sizeof (struct in_addr),
		fromp->sin_family);
	if (hp) {
		/*
		 * If name returned by gethostbyaddr is in our domain,
		 * attempt to verify that we haven't been fooled by someone
		 * in a remote net; look up the name and check that this
		 * address corresponds to the name.
		 */
		strcpy(remotehost, hp->h_name);
		if (local_domain(remotehost)) {
			hp = gethostbyname(remotehost);
			if (hp == NULL) {
				syslog(LOG_INFO,
				    MSGSTR(NOADDR, "Couldn't look up address for %s"),
				    remotehost);
				error(MSGSTR(NOADDR2, "Couldn't look up address for your host"));
				exit(1);
			} else for (; ; hp->h_addr_list++) {
				if (!bcmp(hp->h_addr_list[0],
				    (caddr_t)&fromp->sin_addr,
				    sizeof(fromp->sin_addr)))
					break;
				if (hp->h_addr_list[0] == NULL) {
					syslog(LOG_NOTICE,MSGSTR(HOSTADDR1,
					"Host addr %s not listed for host %s"),
					    inet_ntoa(fromp->sin_addr),
					    hp->h_name);
					error(MSGSTR(HOSTADDR2, "Host address mismatch"));
					exit(1);
				}
			}
		}
	} else {
		error(MSGSTR(HOSTUNK, "Host name for your address unknown.\n"));
		exit(1);
	}

	rshd_getstr(remuser, sizeof(remuser), "remuser");
	rshd_getstr(locuser, sizeof(locuser), "locuser");
	rshd_getstr(cmdbuf, sizeof(cmdbuf), "command");

	setpwent();
	pwd = getpwnam(locuser);
	if (pwd == NULL) {
		error(MSGSTR(LOGINERR, "Login incorrect.\n")); /*MSG*/
		exit(1);
	}
	tmp_pwd = copy_pwd(pwd);                /* make a local copy */
	endpwent();
	pwd = tmp_pwd;          /* the easy way to make the change */
	if (chdir(pwd->pw_dir) < 0) {
		(void) chdir("/");
#ifdef notdef
		error(MSGSTR(NODIR, "No remote directory.\n")); /*MSG*/
		exit(1);
#endif
	}
        /* pwd is a pointer to static data in libc and might get overwritten
         * by the call to loginrestrictions() which calls getuserattr() ).
         * The changes made for libs.a  ( defect 110769 ) makes getuserattr()
         * get passwd from /etc/passwd only , not the real passwd from /etc/
         * security; which changes pwd->pw_passwd to ! ; other fields are
         * restored. Hence, check the no passwd condition before calling
         * loginrestrictions().
         */
         no_passwdf = ((pwd->pw_passwd == 0) || (*pwd->pw_passwd == '\0')) ;

	/*
         * If the SYSTEM authentication is not being used (i.e.,
         * "SYSTEM = NONE"), then the passwords should not
         * be checked.
         */
        if (!SYSTEM_is_NONE(pwd->pw_name)) {
        	/* Check to see if the password expired */
        	passwdexp_rc = passwdexpired(pwd->pw_name, &message);
        	if (message) {
               	 	if (passwdexp_rc != 0) {
               	         	error(message);
               	 	}
               	 	free(message);
               	 	message = (char *) NULL;
        	}
	} else {
         	no_passwdf = 1;
	}
	/* Check to see if there are any login restrictions for */
	/* the user. This includes checking if the user account */
	/* expired.                                             */
	logrestrict_rc = loginrestrictions(pwd->pw_name,0,"RSH",&message);
	if (message) {
		if (logrestrict_rc != 0) {
			error(message);
		}
		free(message);
		message = (char *) NULL;
	}
	if (passwdexp_rc ||             /* password expired? */
	    logrestrict_rc ||           /* account expired? */
	    (( !no_passwdf ) &&         /* no password? */
	    (ruserok(remotehost, pwd->pw_uid == 0, remuser, locuser) < 0))
	   ) {
		error(MSGSTR(NOPERMIT, "Permission denied.\n")); /*MSG*/
		exit(1);
	}

	if (pwd->pw_uid && !access("/etc/nologin", F_OK)) {
		error(MSGSTR(LOGINDISABLE, "Logins currently disabled.\n"));
		exit(1);
	}

        /* need to set LOGNAME */
        set_logname( pwd->pw_name );

        /* need to set user's ulimit, else system default */
	if (setpcred(pwd->pw_name, NULL) < 0)
		error(MSGSTR(USERCRED,"Can't set user credentials.\n"));/*MSG*/

	(void) write(2, "\0", 1);

	if (port) {
		if (pipe(pv) < 0) {
			error(MSGSTR(PIPERR, "Can't make pipe.\n")); /*MSG*/
			exit(1);
		}
		pid = fork();
		if (pid == -1)  {
			error(MSGSTR(RETRY, "Try again.\n")); /*MSG*/
			exit(1);
		}
		if (pv[0] > s)
			nfd = pv[0];
		else
			nfd = s;
		nfd++;
		if (pid) {
			(void) close(0); (void) close(1); (void) close(2);
			(void) close(pv[1]);
			FD_ZERO(&readfrom);
			FD_SET(s, &readfrom);
			FD_SET(pv[0], &readfrom);
			ioctl(pv[0], FIONBIO, (char *)&one);
			/* should set s nbio! */
			do {
				ready = readfrom;
				if (select(nfd, &ready, (fd_set *)0,
				    (fd_set *)0, (struct timeval *)0) < 0)
					break;
				if (FD_ISSET(s, &ready)) {
					if (read(s, &sig, 1) <= 0)
						FD_CLR(s, &readfrom);
					else
						killpg(pid, sig);
				}
				if (FD_ISSET(pv[0], &ready)) {
					errno = 0;
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						shutdown(s, 1+1);
						FD_CLR(pv[0], &readfrom);
					} else
						(void) write(s, buf, cc);
				}
			} while (FD_ISSET(s, &readfrom) ||
			    FD_ISSET(pv[0], &readfrom));
			exit(0);
		}
		setsid();
		(void) close(s); (void) close(pv[0]);
		dup2(pv[1], 2);
		if (pv[1] > 2)
			(void) close(pv[1]);
	}
	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";
	(void) setgid((gid_t)pwd->pw_gid);
	initgroups(pwd->pw_name, pwd->pw_gid);

	(void) setuid((uid_t)pwd->pw_uid);
        priv.pv_priv[0] = 0;
        priv.pv_priv[1] = 0;
        setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));

        /**
         **     Set up environment and exec a shell.
         **/

        addenvvar("HOME",    pwd->pw_dir);
        addenvvar("SHELL",   pwd->pw_shell);
        addenvvar("PATH",    PATHENV);
        addenvvar("LOGNAME", pwd->pw_name);
        addenvvar("USER",    pwd->pw_name);

	/* Check shell type and make the jump to lightspeed. */
	cp = rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
	execl(pwd->pw_shell, cp, "-c", cmdbuf, 0);

	/* fatal, numbing error */
	sprintf(buf,"rshd, %s",pwd->pw_shell);
	perror(buf);
	exit(1);
}

/*VARARGS1*/
error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	char buf[RSHBUFSIZ];

	buf[0] = 1;
        (void) sprintf(buf+1, fmt, a1, a2, a3);
        (void) write(2, buf, strlen(buf));

}

rshd_getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		*buf++ = c;
		if (--cnt == 0) {
			error(MSGSTR(TOOLONG, "%s too long\n"), err); /*MSG*/
			exit(1);
		}
	} while (c != 0);
}

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

	(void) gethostname(localhost, sizeof(localhost));
	p1 = index(localhost, '.');
	if (p1 == NULL || p2 == NULL || !strcasecmp(p1, p2))
		return(1);
	return(0);
}

/* Build an environment table entry of the form 'tag=value' */

addenvvar(tag,value)
  char *tag,*value;

{       char    *penv;
        unsigned int len = strlen(tag)+1;       /* allow for '=' */

        if (!tag)
        {       errno = EINVAL;
                perror("rshd, addenvvar");
                exit(1);
        }
        if (value) len+= strlen(value);

        penv = malloc(len+1);
        strcpy(penv,tag);
        strcat(penv,"=");
        if (value) strcat(penv,value);

        if (putenv(penv)<0)
        {       perror("rshd, putenv");
                exit(1);
        }
        return;
}

#include <uinfo.h>

/*
 * set_logname sets LOGNAME= to the usrName by calling usrinfo().
 *            If LOGNAME was previously set, it is deleted and the
 *            new value is appended after the other strings.
 */
static void
set_logname( char *usrName )
{
   char  *logname_str = "LOGNAME=",
         buf[UINFOSIZ+1],
         *ip = buf,
         *op = buf;
   int   len;

   buf[UINFOSIZ] = 0;                 /* defensive programming */

   if (usrinfo(GETUINFO, buf, UINFOSIZ) != -1) {
       len = strlen(logname_str);
       /* copy buf into buf omitting LOGNAME */
       while (*ip)
           if ( strncmp( ip, logname_str, len) ) {
               if ( ip == op )
                   op = (ip += (strlen(ip) + 1)); /* advance ip/op */
               else
                   while (*op++ = *ip++);         /* copy string past NUL */
           } else
                   ip += (strlen(ip) + 1);        /* skip LOGNAME=... and NUL */

       bzero(op, sizeof(buf) - (op -buf));       /* a clean buf */

       /* buf should not overflow, need 2 NUL's after last string */
       if ( ((op - buf) + strlen(logname_str) + 1) < (UINFOSIZ) ) {
           while ( *op++ = *logname_str++);
           if ( usrName && ((--op - buf) + strlen(usrName)+1) < (UINFOSIZ) )
               strcpy(op, usrName);
       }

       (void) usrinfo(SETUINFO, buf, UINFOSIZ);
   }
}

/******************************************************************************
** copy_pwd
**      allocate storage for all the strings pointed to by the char ptrs
**      in the passwd structure, allocate storage for the structure,
**      copy all the data and pointers over to our local memory
**      input is a pointer to a passwd structure
**      function returns a pointer to a new passwd structure
******************************************************************************/
static struct passwd *copy_pwd(struct passwd *s_ptr)
{
  /* declare variable local to this function */
  struct passwd *d_ptr;         /* destination pointer */

  /* allocate storage for our copy of the passwd structure */
  d_ptr = (struct passwd *)malloc(sizeof(struct passwd));

  /* allocate memory for each of the strings + null char */
  d_ptr->pw_name = malloc(strlen(s_ptr->pw_name)+1);
  d_ptr->pw_passwd = malloc(strlen(s_ptr->pw_passwd)+1);
  d_ptr->pw_gecos = malloc(strlen(s_ptr->pw_gecos)+1);
  d_ptr->pw_dir = malloc(strlen(s_ptr->pw_dir)+1);
  d_ptr->pw_shell = malloc(strlen(s_ptr->pw_shell)+1);

  /* copy the unsigned longs uid_t and gid_t */
  d_ptr->pw_uid = s_ptr->pw_uid;
  d_ptr->pw_gid = s_ptr->pw_gid;

  /* copy the "strings" over to our memory */
  strcpy(d_ptr->pw_name, s_ptr->pw_name);
  strcpy(d_ptr->pw_passwd, s_ptr->pw_passwd);
  strcpy(d_ptr->pw_gecos, s_ptr->pw_gecos);
  strcpy(d_ptr->pw_dir, s_ptr->pw_dir);
  strcpy(d_ptr->pw_shell, s_ptr->pw_shell);

  return(d_ptr);
}
