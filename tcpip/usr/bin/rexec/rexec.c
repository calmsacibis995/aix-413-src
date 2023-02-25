static char sccsid[] = "@(#)20        1.15.2.3  src/tcpip/usr/bin/rexec/rexec.c, tcprcmds, tcpip411, GOLD410 6/15/94 14:03:42";
/* 
 * COMPONENT_NAME: TCPIP rexec.c
 * 
 * FUNCTIONS: MSGSTR, Mrexec, SYSERR, syserr, usage 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *
 * rexec.c - homebrew version of rexec
 *
 * TO RUN:
 *	rexec <host> <cmd-line>
 *
 *
 * TO COMPILE:
 *	cc -o rexec rexec.c 
 *
 */

#include <stdio.h>
#define _H_BSDtoAIX.h
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/syslog.h>   
#include <sys/ioctl.h>          /* Added to enable reading stdin */
#include <sys/param.h>		/* For NCARGS */
#include <arpa/nameser.h>	/* For MAXDNAME */

#ifdef _CSECURITY
#include "tcpip_audit.h"
#include <netinet/in.h>
#include <unistd.h>
#include <sys/id.h>
#include <sys/priv.h>
uid_t saved_uid, effective_uid;
char *audit_tail[TCPIP_MAX_TAIL_FIELDS];
extern char *sys_errlist[];
#endif _CSECURITY

#include <nl_types.h>
nl_catd catd;
#include "rexec_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_REXEC, n, s) 

#ifdef _CSECURITY		/*  _CSECURITY */
/* These 2 defines for sys_errlist[] messages */
#define MF_LIBC     "libc.cat"
#define MS_LIBC     1

char *
syserr(err)
	int err;
{
	char *tmp;
	extern char *strcpy(),*malloc();
	nl_catd catd2;
	catd2 = catopen(MF_LIBC, NL_CAT_LOCALE);
	tmp = catgets(catd2,MS_LIBC,err,sys_errlist[err]);
	return(strcpy(malloc(strlen(tmp+1)),tmp));
}
#define SYSERR(x)	syserr(x)
#else   _CSECURITY
#define SYSERR(x)	sys_errlist[x]
#endif  _CSECURITY

#define BUFSIZE 4096
char buf[BUFSIZE];

char	*myname;		/* for usage */
char	*nullstring = "";

extern int netrc_restricted;
#include <locale.h>

int	errfd; 		/* Moved out of main to enable reading stdin */
int	sendsig();	/* Added to enable reading stdin */

#define mask(s) (1 << ((s) -1)) /* Added to enable reading stdin */

main( argc, argv, envp )
int	argc;
char	*argv[];
char	*envp[];
{
	char	*hostname;
	int	sockfd;
	int 	pid;
	int 	nfd;
	fd_set	openmask;
	fd_set	selmask;
	fd_set	ready;
	register int n;
	int	scnt = 1;		/* indicator for sockfd - must be one to start */
	int	ecnt = 1;		/* indicator for errfd  - must be one to start */
	struct	servent	*sp;
	int	omask;    	/* added for signal handling */
	struct	sigvec save_old_vec, ign_vec; /* added to read stdin */
	int	dbg = FALSE;
	int	signals = FALSE;
	int	one = 1;

	extern int getopt();
	extern int optind;
	int c;
	char command_line[NCARGS];

#ifdef _CSECURITY
	unsigned long tmpaddr;
	struct hostent *tmphp;
	struct sockaddr_in in_from, err_from;
	int fromlen = sizeof(struct sockaddr_in);
	char in_portbuf[16], err_portbuf[16];
#endif _CSECURITY

	setlocale(LC_ALL,"");
	catd = catopen(MF_REXEC, NL_CAT_LOCALE);

/* Added to handle signals */
	ign_vec.sv_handler=SIG_IGN;
	ign_vec.sv_mask=0;
	ign_vec.sv_onstack=0;

#ifdef _CSECURITY
	if ((auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0)) < 0) {
		syslog(LOG_ALERT, MSGSTR(STAUDIT, "rexec: auditproc: %m")); /*MSG*/
		exit(1);
	}
	saved_uid = getuidx(ID_SAVED);
	effective_uid = getuid();
	/* this is so we can get to /etc/security/config */
	(void) setuid(saved_uid);
	if (is_netrc_restricted("rexec"))
		netrc_restricted = 1;
	else
		netrc_restricted = 0;
	/* Now back to the real user id (the one executing rexec) */
	(void) setuid(effective_uid);
#endif _CSECURITY

	myname = argv[0];

	while (( c = getopt(argc, argv, "dns")) != EOF) {
		switch(c){
			case 'd' :
				dbg = TRUE;
				break;

			case 'n' :
				netrc_restricted = 1;
				break;
			
			case 's' :
				signals = TRUE;
				break;

			case '?' :
				usage();
				break;
		}
	}

	if ((argv[optind] == NULL) ||  (argv[optind+1] == NULL))  
		usage();

	hostname = argv[optind++];

#ifdef TCP_DEBUG
		printf("%s: rexec test, host name: %s\n", myname, hostname );
		fflush(stdout);
#endif TCP_DEBUG

	/* start the rexec */
	sp = getservbyname("exec", "tcp");
	if (sp == 0) {
		fprintf(stderr, MSGSTR(UNKSERVER, "rexec: exec/tcp: unknown service\n")); /*MSG*/
		exit(1);
	}

	/* Fix up command line into one buffer */
	strcpy(command_line, argv[optind++]);
	while (optind < argc) {
		strcat(command_line, " ");
		strcat(command_line, argv[optind++]);
	}
	sockfd = rexec( &hostname, sp->s_port, 0, 0, command_line, &errfd );
	if (signals && errfd < 0) {
		fprintf(stderr, MSGSTR(NOSTDERR, "rexec: can't establish stderr\n"));
		exit(2);
	}
#ifdef _CSECURITY
	tmpaddr = inet_addr(hostname);
	if (tmpaddr == -1) {
		tmphp = gethostbyname(hostname);
		if (!tmphp) {
			herror(hostname);
			tmpaddr = 0;
		}
		else {
		  memcpy(&tmpaddr, tmphp->h_addr, sizeof(tmpaddr));
		}
	}
#endif _CSECURITY
	if( sockfd == -1 ) {
#ifdef _CSECURITY
		CONNECTION_WRITE(tmpaddr,sp->s_port,
		                 command_line, "remote execution failed", -1);
#endif _CSECURITY

		exit(1);
	}
#ifdef _CSECURITY
	CONNECTION_WRITE(tmpaddr,sp->s_port, command_line, "OK",0);
#endif _CSECURITY

#ifndef _CSECURITY
	if (setuid(getuid())) {
		perror(MSGSTR(REXECID, "rexec: setuid")); /*MSG*/
		exit(1);
	}
#else
	{
		char *tmp;

		if (getpeername(sockfd, &in_from, &fromlen) < 0) {
			tmp = MSGSTR(UNK,"Unknown");
			CONNECTION_WRITE(0,strcpy(malloc(strlen(tmp)+1),tmp),
		    MSGSTR(CLOSE,"close"),SYSERR(errno),errno);
			perror(MSGSTR(REXECPEER, "getpeername")); /*MSG*/
			exit(1);
		}
		if (getpeername(errfd, &err_from, &fromlen) < 0) {
			tmp = MSGSTR(UNK,"Unknown");
			CONNECTION_WRITE(0,strcpy(malloc(strlen(tmp)+1),tmp),
		    MSGSTR(CLOSE,"close"),SYSERR(errno),errno);
			perror(MSGSTR(REXECPEER, "getpeername")); /*MSG*/
			exit(1);
		}
		sprintf(in_portbuf, "%d", in_from.sin_port);
		sprintf(err_portbuf, "%d", err_from.sin_port);

		privilege(PRIV_LAPSE);
	}
#endif _CSECURITY
 
	if (dbg) {
		if (setsockopt(sockfd,SOL_SOCKET,SO_DEBUG,&one,sizeof(one)) < 0)
			perror(MSGSTR(INSOCK, "setsockopt (stdin)")); /*MSG*/
		if (setsockopt(errfd,SOL_SOCKET,SO_DEBUG,&one,sizeof(one)) < 0)
			perror(MSGSTR(ERRSOCK, "setsockopt (stderr)")); /*MSG*/
	}

#ifdef TCP_DEBUG
/*DDD*/	printf("%s: rexec test, sockfd %d, errfd %d\n", myname, sockfd, errfd );
/*DDD*/	fflush(stdout);
#endif


	/* New code added to enable rexec to read from stdin. */
	/* This code creates a child process whose only function is to
	 * read from stdin and send the information to the remote host. 
	*/
	if (signals) {
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
	}

        pid = fork();
        if (pid < 0) {
                 perror(MSGSTR(FORK, "fork")); /*MSG*/
                 exit(1);
        }
        ioctl(errfd, FIONBIO, &one);
        ioctl(sockfd, FIONBIO, &one);
        if (pid == 0) {
                char *bp;
                int wc;

                (void) close(errfd);
        reread:
                errno = 0;
                scnt = read(0, buf, sizeof buf);
                if (scnt <= 0)
                        goto done;
                bp = buf;
        rewrite:
                FD_ZERO(&selmask);
                FD_SET(sockfd,&selmask);
		/* select only on the file descriptors that we are
		 * interested in since selecting on FD_SETSIZE causes
	         * poor performance.
		 */
                if (select(sockfd+1, 0, &selmask, 0, 0) < 0) {
                        if (errno != EINTR) {
                                perror(MSGSTR(SELECT, "select")); /*MSG*/
                                exit(1);
                        }
                        goto rewrite;
                }
                if (!FD_ISSET(sockfd,&selmask))
                        goto rewrite;
                wc = write(sockfd, bp, scnt);
                if (wc < 0) {
                        if (errno == EWOULDBLOCK || errno == EAGAIN)
                                goto rewrite;
                        goto done;
                }
                scnt -= wc; bp += wc;
                if (scnt == 0)
                        goto reread;
                goto rewrite;
        done:
                (void) shutdown(sockfd, 1);
                exit(0);
        }

	if (signals) {
		/* set the mask back to its original state */
		sigsetmask(omask);
	}
	/* New code ended  */

	FD_ZERO(&selmask); /* zero out our mask */
	FD_SET(errfd,&selmask);
	FD_SET(sockfd,&selmask);
	nfd = (errfd > sockfd ? errfd : sockfd) + 1;
	do {
		ready = selmask;
		/* select only on the file descriptors that we are
		 * interested in since selecting on FD_SETSIZE causes
	         * poor performance.
		 */
		if (select(nfd, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
#ifdef _CSECURITY
				(void) setuid(saved_uid);
				CONNECTION_WRITE(in_from.sin_addr.s_addr, in_portbuf,
					MSGSTR(CLOSE,"close"), SYSERR(errno), errno);
				CONNECTION_WRITE(err_from.sin_addr.s_addr, err_portbuf,
					MSGSTR(CLOSE,"close"), SYSERR(errno), errno); 
				(void) setuid(effective_uid);
#endif _CSECURITY
				perror("select: ");
				exit(1);
			}
			continue;
		}
		if (FD_ISSET(errfd,&ready)) {
			errno = 0;
			ecnt = read(errfd, buf, sizeof buf);
			if (ecnt <= 0) {
				if (errno != EWOULDBLOCK || errno == EAGAIN)
					FD_CLR(errfd,&selmask);
			} else
				(void) write(2, buf, ecnt);
		}
		if (FD_ISSET(sockfd,&ready)) {
			errno = 0;
			scnt = read(sockfd, buf, sizeof buf);
			if (scnt <= 0) {
				if (errno != EWOULDBLOCK)
					FD_CLR(sockfd,&selmask);
			} else
				(void) write(1, buf, scnt);
		}
        } while (FD_ISSET(errfd,&selmask) || FD_ISSET(sockfd,&selmask));
#ifdef _CSECURITY
	(void) setuid(saved_uid);
	CONNECTION_WRITE(in_from.sin_addr.s_addr, in_portbuf,MSGSTR(CLOSE, "close"),
		 ((scnt < 0) ? SYSERR(errno) : ""), ((scnt < 0) ? -1 : 0));
	CONNECTION_WRITE(err_from.sin_addr.s_addr, err_portbuf,MSGSTR(CLOSE, "close"),
		 ((ecnt < 0) ? SYSERR(errno) : ""), ((ecnt < 0) ? -1 : 0));
	(void) setuid(effective_uid);
#endif _CSECURITY
	(void) kill(pid, SIGKILL);
	exit(0);
}

/*
 * usage - print usage message
 */
usage()
{
	printf("%s: \n", myname );
	printf(MSGSTR(USEAGE, "    usage: %s [-d] [-n] [-s] <hostname> <cmd-line>\n"), myname ); /*MSG*/
	exit(1);
}
/* end of rexec.c */

/* sendsig - sends the signal to the remote side
 *           added as part of the code needed to enable rexec
 *	     to read stdin.
 */
sendsig(signo)
        char signo;
{

        (void) write(errfd, &signo, 1);
}

