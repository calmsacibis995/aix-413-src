static char sccsid[] = "@(#)28	1.23.1.13  src/tcpip/usr/lbin/rexecd/rexecd.c, tcprcmds, tcpip41J, 9515A_all 3/29/95 14:30:41";
/* 
 * COMPONENT_NAME: TCPIP rexecd.c
 * 
 * FUNCTIONS: MSGSTR, Mrexecd, SAVMSG, SYSERR, doit, error, rexec_getstr, 
 *            msgmalloc, syserr, set_logname
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "rexecd.c	5.4 (Berkeley) 5/9/86";
#endif not lint
*/
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/syslog.h>

#include <netinet/in.h>

#include <stdio.h>
#include <usersec.h>
#include <userpw.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <netdb.h>

#ifdef _CSECURITY
#include <sys/id.h>
#include <sys/priv.h>
#include "tcpip_audit.h"
#endif _CSECURITY

extern	errno;
struct	passwd *getpwnam();
char	*crypt(), *rindex(), *strncat();
/*VARARGS1*/
int	error();
int	tracing = 0;
static void     set_logname(char *);		/* set environment variable */

#ifdef _CSECURITY
char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
extern char *sys_errlist[];
struct sockaddr_in from;
char portbuf[16];
#endif _CSECURITY

#include <nl_types.h>
#include "rexecd_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_REXECD, n, s) 

#ifdef _CSECURITY			/* _CSECURITY */
#define MF_LIBC	"libc.cat"
#define MS_LIBC	1
char *syserr();
char *msgmalloc();
#define SYSERR(x)	syserr(x)
#define SAVMSG(x)	strcpy(msgmalloc(strlen(x)+1),x)
#else  !_CSECURITY	
#define SYSERR(x)	sys_errlist[x]
#endif _CSECURITY
/*
 * remote execute server:
 *	username\0
 *	password\0
 *	command\0
 *	data
 */
/*ARGSUSED*/

#include <locale.h>

main(argc, argv)
	int argc;
	char **argv;
{
#ifndef _CSECURITY
	struct sockaddr_in from;
#endif !_CSECURITY
	int fromlen;
	int on = 1;
	struct sigvec sv;
	void trace_handler();

#ifdef _CSECURITY
	char tcpipClass[] = "tcpip\0";
	int ch;
#endif _CSECURITY 

	setlocale(LC_ALL,"");
	catd = catopen(MF_REXECD,NL_CAT_LOCALE);

#ifdef _CSECURITY
	if(auditproc(0, AUDIT_EVENTS, tcpipClass, 7) < 0){
		syslog(LOG_ALERT, MSGSTR(STAUDIT, "rexecd: auditproc: %m")); /*MSG*/
		exit(1);
	}
	else if ((auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0)) < 0) {
		syslog(LOG_ALERT, MSGSTR(STAUDIT, "rexecd: auditproc: %m")); /*MSG*/
		exit(1);
	}

        while ((ch = getopt(argc,argv,"s")) != EOF)
                switch((char)ch) {
                case 's':       tracing = 1;
                                break;
                default:        fprintf(stderr,"usage: rexecd [-s]\n");
                                break;
                }

	strcpy(portbuf, "exec/tcp");
#endif _CSECURITY
	fromlen = sizeof (from);
	if (getpeername(0, &from, &fromlen) < 0) {
		fprintf(stderr, MSGSTR(GPEERERR, "%s: "), argv[0]); /*MSG*/
#ifdef _CSECURITY
		{		/* copy msg to new buff before 2nd call to getamsg */
			char *unk;

			unk = catgets(catd,MS_REXECD,UNK,"Unknown");

			CONNECTION_WRITE(0,SAVMSG(unk),
				MSGSTR(SECLSE,"close"), SYSERR(errno), errno); /*MSG*/
		}
#endif _CSECURITY  
		perror(MSGSTR(PEERERR, "getpeername")); /*MSG*/
		exit(1);
	}
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

	doit(0, &from);
}

int s;

/*
 * trace_handler - SRC TRACE ON/OFF signal handler
 */
void
trace_handler(sig)
	int	sig;
{
	int	onoff;

	onoff = (sig == SIGUSR1) ? 1 : 0;
	if (setsockopt(s, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0)
		syslog(LOG_WARNING,MSGSTR(SETDEBUG,"setsockopt (SO_DEBUG): %m")); /*MSG*/
}


char	username[20] = "USER=";
char	homedir[64] = "HOME=";
char	shell[64] = "SHELL=";
char	time_zone[20] = "TZ=";
char	*envinit[] =
	    {homedir, shell, "PATH=:/bin:/usr/bin:/usr/bin/X11", username, time_zone, 0};
extern char	**environ;
#define PATHENV ":/usr/ucb:/bin:/usr/bin:/usr/bin/X11"

struct	sockaddr_in asin;

doit(f, fromp)
	int f;
	struct sockaddr_in *fromp;
{
	char cmdbuf[NCARGS+1], *cp, *namep;
	char user[16], pass[16];
	struct passwd *pwd;
	u_short port;
	int pv[2], pid, ready, readfrom, cc;
	char buf[BUFSIZ], sig, *getenv(), *tzp;
	int one = 1, reenter;
	char **penvlist;
        int passwdexp_rc = 0, logrestrict_rc = 0, authenticate_rc = 0;
        char *message = (char *) NULL;
	struct sockaddr_in *to;
#ifdef _CSECURITY
	priv_t  priv;
#endif _CSECURITY


	(void) signal(SIGINT, SIG_DFL);
	(void) signal(SIGQUIT, SIG_DFL);
	(void) signal(SIGTERM, SIG_DFL);
#ifdef TCP_DEBUG
	{ int t = open("/dev/tty", 2);
	  if (t >= 0) {
		ioctl(t, TIOCNOTTY, (char *)0);
		(void) close(t);
	  }
	}
#endif
	dup2(f, 0);
	dup2(f, 1);
	dup2(f, 2);
	(void) alarm(60);
	port = 0;
	for (;;) {
		char c;
		if (read(f, &c, 1) != 1)
			exit(1);
		if (c == 0)
			break;
		port = port * 10 + c - '0';
	}
	(void) alarm(0);
	if (port != 0) {
		s = socket(AF_INET, SOCK_STREAM, 0);
		if (s < 0)
			exit(1);
		if (getsockname(0, &to, sizeof(to)) == 0) 
			asin.sin_addr.s_addr=to->sin_addr.s_addr;
		asin.sin_family=AF_INET;
		if (bind(s, &asin, sizeof (asin)) < 0)
			exit(1);
		(void) alarm(60);
		fromp->sin_port = htons((u_short)port);
#ifdef _CSECURITY
		sprintf(portbuf, "%d", htons((u_short)port));
#endif _CSECURITY
		if (connect(s, fromp, sizeof (*fromp)) < 0) {
#ifdef _CSECURITY
			CONNECTION_WRITE(fromp->sin_addr.s_addr, portbuf,
				MSGSTR(SECOPEN,"open"), SYSERR(errno), errno);/*MSG*/
#endif _CSECURITY 
			exit(1);
		}
		(void) alarm(0);
	}
	rexec_getstr(user, sizeof(user), "username");
	rexec_getstr(pass, sizeof(pass), "password");
	rexec_getstr(cmdbuf, sizeof(cmdbuf), "command");
	setpwent();
	pwd = getpwnam(user);
	if (pwd == NULL) {
#ifdef _CSECURITY
		char *log;

		log = catgets(catd,MS_REXECD,LOGINERR,"Login incorrect.");

		NET_ACCESS_WRITE(fromp->sin_addr.s_addr,
			MSGSTR(REMEXEC,"Remote exec"), "", SAVMSG(log), -1); /*MSG*/
#endif _CSECURITY 
		error(MSGSTR(LOGINERR2, "Login incorrect.\n")); /*MSG*/
		exit(1);
	}
	endpwent();

	/*
         * If the SYSTEM authentication is not being used (i.e.,
         * "SYSTEM = NONE"), then the passwords should not
         * be checked.
         */
        if (!SYSTEM_is_NONE(pwd->pw_name)) {
		/* Check to see if the user's password expired */
		passwdexp_rc = passwdexpired(user, &message);
		if (message) {
			if (passwdexp_rc != 0)
				error(message);
			free(message);
			message = (char *) NULL;
		}
	}
	/* Check to see if there are any login restrictions for */
	/* the user. This includes checking if the user account */
	/* expired.                                             */
	logrestrict_rc = loginrestrictions(user,0,"REXEC",&message);
	if (message) {
		if (logrestrict_rc != 0)
			error(message);
		free(message);
		message = (char *) NULL;
	}
	/* Authenticate user through the authenticate() security routine */
	authenticate_rc = authenticate(user,pass,&reenter,&message);
	if (message) {
		if (authenticate_rc != 0)
			error(message);
		free(message);
		message = (char *) NULL;
	}

	if (passwdexp_rc ||             /* password expired? */
	    logrestrict_rc ||           /* account expired? */
	    authenticate_rc ) {          /* authenticated? */

#ifdef _CSECURITY
		char *log;

		log = catgets(catd,MS_REXECD,LOGINERR,"Login incorrect.");
		NET_ACCESS_WRITE(fromp->sin_addr.s_addr, MSGSTR(REMEXEC,"Remote exec"),
			user, SAVMSG(log),-1); /*MSG*/
#endif _CSECURITY 
		error(MSGSTR(LOGINERR2, "Login incorrect.\n")); /* password incorrect!! */ /*MSG*/
		exit(1);	/* SEE YA! */
	}
	if (chdir(pwd->pw_dir) < 0) {
#ifdef _CSECURITY
		char *norem;

		norem = catgets(catd,MS_REXECD,REMEXEC,"Remote exec");

		NET_ACCESS_WRITE(fromp->sin_addr.s_addr,SAVMSG(norem),
			pwd->pw_name,MSGSTR(NODIR,"No remote directory"),-1); /*MSG*/
#endif _CSECURITY 
		(void) chdir("/");
	}

	(void) write(2, "\0", 1);
	if (port) {
		(void) pipe(pv);
		pid = fork();
		if (pid == -1)  {
#ifdef _CSECURITY
		char *remex;

		remex = catgets(catd,MS_REXECD,REMEXEC,"Remote exec");
		NET_ACCESS_WRITE(fromp->sin_addr.s_addr,SAVMSG(remex),
			pwd->pw_name,MSGSTR(NOFORK, "Can't fork remote exec"), -1); 
#endif _CSECURITY 
			error(MSGSTR(RETRY, "Try again.\n")); /*MSG*/
			exit(1);
		}
		if (pid) {
			(void) close(0); (void) close(1); (void) close(2);
			(void) close(f); (void) close(pv[1]);
			readfrom = (1<<s) | (1<<pv[0]);
			ioctl(pv[1], FIONBIO, (char *)&one);
			/* should set s nbio! */
			do {
				ready = readfrom;
				(void) select(16, &ready, (fd_set *)0,
				    (fd_set *)0, (struct timeval *)0);
				if (ready & (1<<s)) {
					if (read(s, &sig, 1) <= 0)
						readfrom &= ~(1<<s);
					else
						killpg(pid, sig);
				}
				if (ready & (1<<pv[0])) {
					cc = read(pv[0], buf, sizeof (buf));
					if (cc <= 0) {
						shutdown(s, 1+1);
						readfrom &= ~(1<<pv[0]);
					} else
						(void) write(s, buf, cc);
				}
			} while (readfrom);
#ifdef _CSECURITY
			CONNECTION_WRITE(fromp->sin_addr.s_addr, portbuf,
				MSGSTR(SECLSE,"close"), "", 0);
#endif _CSECURITY 
			exit(0);
		}
		setpgrp(0, getpid());
		(void) close(s); (void)close(pv[0]);
		dup2(pv[1], 2);
	}

#ifdef _CSECURITY
	NET_ACCESS_WRITE(fromp->sin_addr.s_addr,MSGSTR(REMEXEC,"Remote exec"),
		pwd->pw_name, "", 0); /*MSG*/
#endif _CSECURITY 

        /* need to set LOGNAME */
        set_logname( pwd->pw_name );

        /*
	 * Need to set user's ulimit, else system default
	 */
        if (setpcred(pwd->pw_name, NULL) < 0) {
              error(MSGSTR(USERCRED,"Can't set user credentials.\n"));/*MSG*/
	}

	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = "/bin/sh";
	if (f > 2)
		(void) close(f);

	(void) setgid((gid_t)pwd->pw_gid);
	initgroups(pwd->pw_name, pwd->pw_gid);

#ifdef _CSECURITY
	/*
	 * need to drop privs here so that we only have user privs
	 */
	priv.pv_priv[0] = 0;
	priv.pv_priv[1] = 0;
	setpriv(PRIV_SET|PRIV_EFFECTIVE|PRIV_BEQUEATH, &priv, sizeof(priv_t));
	(void) setuid((uid_t)pwd->pw_uid);
#endif _CSECURITY

	/* Now we need to TZ= from the environment */
	if ((tzp = getenv("TZ")) != NULL)
		strncat(time_zone, tzp, sizeof(time_zone)-4);

        penvlist = getpenv(PENV_USR); 
	environ = penvlist +1 ;
        addenvvar("HOME",    pwd->pw_dir);
        addenvvar("SHELL",   pwd->pw_shell);
        addenvvar("PATH",    PATHENV);
        addenvvar("LOGNAME", pwd->pw_name);
        addenvvar("USER",    pwd->pw_name);
	cp = rindex(pwd->pw_shell, '/');
	if (cp)
		cp++;
	else
		cp = pwd->pw_shell;
	execl(pwd->pw_shell, cp, "-c", cmdbuf, 0);
	perror(pwd->pw_shell);
#ifdef _CSECURITY
	CONNECTION_WRITE(fromp->sin_addr.s_addr, portbuf,
			MSGSTR(SECLSE,"close"), SYSERR(errno), errno);
#endif _CSECURITY 
	exit(1);
}

/*VARARGS1*/
error(fmt, a1, a2, a3)
	char *fmt;
	int a1, a2, a3;
{
	char buf[BUFSIZ];

	buf[0] = 1;
	(void) sprintf(buf+1, fmt, a1, a2, a3);
	(void) write(2, buf, strlen(buf));
}

rexec_getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1) {
#ifdef _CSECURITY
			char *badrd;

			badrd = catgets(catd,MS_REXECD,BADRD,"Bad read");

			CONNECTION_WRITE(from.sin_addr.s_addr, portbuf,
				MSGSTR(SECLSE,"close"),SAVMSG(badrd), -1); 
#endif _CSECURITY 
			exit(1);
		}
		*buf++ = c;
		if (--cnt == 0) {
#ifdef _CSECURITY
			char *lngstr;

			lngstr = catgets(catd,MS_REXECD,LNGSTR,"rexec_getstr: too long");
			CONNECTION_WRITE(from.sin_addr.s_addr, portbuf,
		 		MSGSTR(SECLSE,"close"),SAVMSG(lngstr),-1);
#endif _CSECURITY 
			error(MSGSTR(TOOLNG, "%s too long\n"), err); /*MSG*/
			exit(1);
		}
	} while (c != 0);
}

/* Build an environment table entry of the form 'tag=value' */

addenvvar(tag,value)
  char *tag,*value;

{       char    *penv;
        unsigned int len = strlen(tag)+1;       /* allow for '=' */

        if (!tag) {
		errno = EINVAL;
                perror("rexecd, addenvvar");
                exit(1);
        }
        if (value) len+= strlen(value);

        penv = malloc(len+1);
        strcpy(penv,tag);
        strcat(penv,"=");
        if (value) strcat(penv,value);

        if (putenv(penv)<0) {
		perror("rexecd, putenv");
                exit(1);
        }
        return;
}

#ifdef _CSECURITY

char *syserr(errn)
int errn;
{
	char *tmp;
	nl_catd catd2;

	catd2 = catopen(MF_LIBC,NL_CAT_LOCALE);
	tmp = catgets(catd2,MS_LIBC,errno,sys_errlist[errno]);
	catclose(catd2);
	return strcpy(msgmalloc(strlen(tmp)+1),tmp);
}

char *msgmalloc(sz)
unsigned sz;
{
	char *tmp;

	if(tmp=malloc(sz))
		return tmp;

	error("%s",catgets(catd,MS_REXECD,MSGMAL,
		"Could not malloc memory for message string."));
	exit(1);
}

#endif _CSECURITY

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

