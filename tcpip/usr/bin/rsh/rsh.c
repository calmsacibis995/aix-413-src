static char sccsid[] = "@(#)32	1.21  src/tcpip/usr/bin/rsh/rsh.c, tcprcmds, tcpip411, GOLD410 6/15/94 14:01:44";
/* 
 * COMPONENT_NAME: TCPIP rsh.c
 * 
 * FUNCTIONS: MSGSTR, Mrsh, mask, sendsig 
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
 * Copyright (c) 1983 The Regents of the University of California.
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
" Copyright (c) 1983 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "rsh.c	5.7 (Berkeley) 9/20/88";
#endif  not lint */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/param.h>

#include <netinet/in.h>

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <pwd.h>
#include <netdb.h>

#include <nl_types.h>
nl_catd catd;
#include "rsh_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_RSH, n, s) 

/*
 * rsh - remote shell
 */
/* VARARGS */
int	error();
char	*index(), *rindex(), *malloc(), *getpass(), *strcpy();

struct	passwd *getpwuid();

extern int	errno;
int	options;
int	rfd2;
int	nflag;
int	sendsig();

#define	mask(s)	(1 << ((s) - 1))

#include <locale.h>

main(argc, argv0)
	int argc;
	char **argv0;
{
	int rem, pid;
	char *host, *cp, **ap, buf[BUFSIZ], *args, **argv = argv0, *user = 0;
	register int cc;
	int asrsh = 0;
	struct passwd *pwd;
	int one = 1;
	struct servent *sp;
	int omask;
	struct sigvec save_old_vec, ign_vec;
	char hostnamebuf[MAXHOSTNAMELEN];
	fd_set rembits, ready, readfrom;
	int nfd;

	setlocale(LC_ALL,"");
	catd = catopen(MF_RSH, NL_CAT_LOCALE);
	ign_vec.sv_handler=SIG_IGN;
	ign_vec.sv_mask=0;
	ign_vec.sv_onstack=0;

	host = rindex(argv[0], '/');
	if (host)
		host++;
	else
		host = argv[0];
	argv++, --argc;
	if (!strcmp(host, "rsh") || !strcmp(host, "remsh")) {
		host = *argv++, --argc;
		asrsh = 1;
	}
another:
	if (argc > 0 && !strcmp(*argv, "-l")) {
		argv++, argc--;
		if (argc > 0)
			user = *argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-n")) {
		argv++, argc--;
		nflag++;
		goto another;
	}
	if (argc > 0 && !strcmp(*argv, "-d")) {
		argv++, argc--;
		options |= SO_DEBUG;
		goto another;
	}
	/*
	 * Ignore the -L, -w, -e and -8 flags to allow aliases with rlogin
	 * to work
	 *
	 * There must be a better way to do this! -jmb
	 */
	if (argc > 0 && !strncmp(*argv, "-L", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-w", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-e", 2)) {
		argv++, argc--;
		goto another;
	}
	if (argc > 0 && !strncmp(*argv, "-8", 2)) {
		argv++, argc--;
		goto another;
	}
	if (host == 0)
		goto usage;
	if (argv[0] == 0) {
		if (asrsh)
			*argv0 = "rlogin";
		execv("/usr/bin/rlogin", argv0);
		perror(MSGSTR(BLOGIN, "/usr/bin/rlogin")); /*MSG*/
		exit(1);
	}
	pwd = getpwuid(getuid());
	if (pwd == 0) {
		fprintf(stderr, MSGSTR(UIDERR, "who are you?\n")); /*MSG*/
		exit(1);
	}
	cc = 0;
	for (ap = argv; *ap; ap++)
		cc += strlen(*ap) + 1;
	cp = args = malloc(cc);
	for (ap = argv; *ap; ap++) {
		(void) strcpy(cp, *ap);
		while (*cp)
			cp++;
		if (ap[1])
			*cp++ = ' ';
	}
	sp = getservbyname("shell", "tcp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKSERVER, "rsh: shell/tcp: unknown service\n")); /*MSG*/
		exit(1);
	}

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
					"%s: address misformed\n"), host);
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

        rem = rcmd(&host, sp->s_port, pwd->pw_name,
	    user ? user : pwd->pw_name, args, &rfd2);
        if (rem < 0)
                exit(1);
	if (rfd2 < 0) {
		fprintf(stderr, MSGSTR(NOSTDERR, "rsh: can't establish stderr\n")); /*MSG*/
		exit(2);
	}
	if (options & SO_DEBUG) {
		if (setsockopt(rem, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror(MSGSTR(SETSOCK, "setsockopt (stdin)")); /*MSG*/
		if (setsockopt(rfd2, SOL_SOCKET, SO_DEBUG, &one, sizeof (one)) < 0)
			perror(MSGSTR(SETSOCK2, "setsockopt (stderr)")); /*MSG*/
	}
	(void) setuid(getuid());
	omask = sigblock(mask(SIGHUP)|mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	(void) sigvec(SIGHUP, &ign_vec, &save_old_vec);
	if (save_old_vec.sv_handler != SIG_IGN)
		(void) signal(SIGHUP, sendsig);
	(void) sigvec(SIGINT, &ign_vec, &save_old_vec);
	if (save_old_vec.sv_handler != SIG_IGN)
		(void) signal(SIGINT, sendsig);
	(void) sigvec(SIGQUIT, &ign_vec, &save_old_vec);
	if (save_old_vec.sv_handler != SIG_IGN)
		(void) signal(SIGQUIT, sendsig);
	(void) sigvec(SIGTERM, &ign_vec, &save_old_vec);
	if (save_old_vec.sv_handler != SIG_IGN)
		(void) signal(SIGTERM, sendsig); 
	if (nflag == 0) {
		pid = fork();
		if (pid < 0) {
			perror(MSGSTR(FORK, "fork")); /*MSG*/
			exit(1);
		}
	}
	ioctl(rfd2, FIONBIO, &one);
	ioctl(rem, FIONBIO, &one);
        if (nflag == 0 && pid == 0) {
		char *bp; 
		int wc;
		(void) close(rfd2);
	reread:
		errno = 0;
		cc = read(0, buf, sizeof buf);
		if (cc <= 0)
			goto done;
		bp = buf;
	rewrite:
		FD_ZERO(&rembits);
		FD_SET(rem,&rembits);
		/* select only on the file descriptors that we are interested
		 * in since selecting on FD_SETSIZE causes poor performance
	         */
		if (select(rem+1, 0, &rembits, 0, 0) < 0) {
			if (errno != EINTR) {
				perror(MSGSTR(SELECT, "select")); /*MSG*/
				exit(1);
			}
			goto rewrite;
		}
		if (!FD_ISSET(rem,&rembits))
			goto rewrite;
		wc = write(rem, bp, cc);
		if (wc < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				goto rewrite;
			goto done;
		}
		cc -= wc; bp += wc;
		if (cc == 0)
			goto reread;
		goto rewrite;
	done:
		(void) shutdown(rem, 1);
		exit(0);
	}
	sigsetmask(omask);
	FD_ZERO(&readfrom);
	FD_SET(rfd2,&readfrom);
	FD_SET(rem,&readfrom);
	nfd = (rfd2 > rem ? rfd2 : rem) + 1;
	do {
		ready = readfrom;
		/* select only on the file descriptors that we are interested
		 * in since selecting on FD_SETSIZE causes poor performance
	         */
		if (select(nfd, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror(MSGSTR(SELECT, "select")); /*MSG*/
				exit(1);
			}
			continue;
		}
		if (FD_ISSET(rfd2,&ready)) {
			errno = 0;
			cc = read(rfd2, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK || errno == EAGAIN)
					FD_CLR(rfd2,&readfrom);
			} else
				(void) write(2, buf, cc);
		}
		if (FD_ISSET(rem,&ready)) {
			errno = 0;
			cc = read(rem, buf, sizeof buf);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK)
					FD_CLR(rem,&readfrom);
			} else
				(void) write(1, buf, cc);
		}
        } while (FD_ISSET(rfd2,&readfrom) || FD_ISSET(rem,&readfrom));
	if (nflag == 0)
		(void) kill(pid, SIGKILL);
	exit(0);
usage:
	fprintf(stderr,
	    MSGSTR(USE, "usage: rsh host [ -l login ] [ -n ] command\n")); /*MSG*/
	exit(1);
}

sendsig(signo)
	char signo;
{

	(void) write(rfd2, &signo, 1);
}

