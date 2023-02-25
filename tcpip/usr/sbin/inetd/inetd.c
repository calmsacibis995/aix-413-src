static char sccsid[] = "@(#)22	1.41.1.15  src/tcpip/usr/sbin/inetd/inetd.c, tcpinet, tcpip411, GOLD410 7/21/94 15:38:50";
/* 
 * COMPONENT_NAME: TCPIP inetd.c
 * 
 * FUNCTIONS: MSGSTR, chargen_dg, chargen_stream, config, daytime_dg,
 *		daytime_stream, discard_dg, discard_stream, echo_dg,
 *		echo_stream, endconfig, enter, freeconfig, getconfigent,
 *		initring, machtime, machtime_dg, machtime_stream, reapchild,
 *		skip, setconfig, setproctitle, setup, nextline, retry,
 *		dosrcpacket, process_srcpacket, kill_active_subserver, unregister
 *
 * ORIGINS: 10  24  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983, 1991 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * inetd.c	5.30 (Berkeley) 6/3/91
 */

/*
 * Inetd - Internet super-server
 *
 * This program invokes all internet services as needed.
 * connection-oriented services are invoked each time a
 * connection is made, by creating a process.  This process
 * is passed the connection as file descriptor 0 and is
 * expected to do a getpeername to find out the source host
 * and port.
 *
 * Datagram oriented services are invoked when a datagram
 * arrives; a process is created and passed a pending message
 * on file descriptor 0.  Datagram servers may either connect
 * to their peer, freeing up the original socket for inetd
 * to receive further messages on, or ``take over the socket'',
 * processing all arriving datagrams and, eventually, timing
 * out.	 The first type of server is said to be ``multi-threaded'';
 * the second type of server ``single-threaded''. 
 *
 * Inetd uses a configuration file which is read at startup
 * and, possibly, at some later time in response to a hangup signal.
 * The configuration file is ``free format'' with fields given in the
 * order shown below.  Continuation lines for an entry must being with
 * a space or tab.  All fields must be present in each entry.
 *
 *	service name			must be in /etc/services
 *	socket type			stream/dgram/raw/rdm/seqpacket
 *	protocol			must be in /etc/protocols
 *	wait/nowait			single-threaded/multi-threaded
 *	user				user to run daemon as
 *	server program			full path name
 *	server program arguments	maximum of MAXARGS (20)
 *
 * Comment lines are indicated by a `#' in column 1.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef _SUN
#include <rpc/rpc.h>            /* SUN RPC */
#include <rpc/pmap_prot.h>
#else
#include <netinet/in.h>
#endif

#include <arpa/inet.h>

#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <syslog.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include "pathnames.h"

#ifndef _AIX
#define	TOOMANY		40		/* don't start more than TOOMANY */
#define	CNT_INTVL	60		/* servers in CNT_INTVL sec. */
#endif /* !_AIX */

#define	RETRYTIME	(60*10)		/* retry after bind or server fail */

#define	SIGBLOCK	(sigmask(SIGCHLD)|sigmask(SIGHUP)|sigmask(SIGALRM))

extern	int errno;

void	config(), reapchild(), retry();
char	*index();

int	debug = 0;
int	nsock, maxsock;
fd_set	allsock;
fd_set	readable;
int	options;
int	timingout;
struct	servent *sp;

struct	servtab {
	char	*se_service;		/* name of service */
	short	se_kind;		/* kind of service: socket or rpc */
	short	se_kflag;		/* flags for use by each kind */
	int	se_socktype;		/* type of socket to use */
	char	*se_proto;		/* protocol used */
	int	se_protonum;		/* protocol used (as number) */
	pid_t	se_wait;		/* single threaded server */
	short	se_checked;		/* looked at during merge */
	char	*se_user;		/* user name to run as */
	struct	biltin *se_bi;		/* if built-in, description */
	char	*se_server;		/* server program */
#define	MAXARGV 20
	char	*se_argv[MAXARGV+1];	/* program arguments */
	int	se_fd;			/* open descriptor */
	union	{
		struct	sockaddr_in	ctrladdr ;	/* bound address */
		struct {
			unsigned	prog;
			unsigned	lovers;
			unsigned	hivers;
		} rpcnum;
	} se_un;
	int	se_count;		/* number started since se_time */
	long	se_portnum;		/* port number for SRC - */
	struct	timeval se_time;	/* start of se_count */
	struct	servtab *se_next;
} *servtab;
#define	se_ctrladdr	se_un.ctrladdr
#define	se_rpc		se_un.rpcnum

/*
 * These defines and table split up the kinds of things the inetd
 * handles, into just plain old sockets or sunrpc sockets.  In the
 * future, more kinds can be added.
 */
#define	KIND_SOCKET	0x0001	/* stream|dgram|raw|rdm|seqpacket */
#ifdef _SUN
#define	KIND_SUNRPC	0x0002	/* sunrpc_tcp|sunrpc_udp */
#endif

/* kind table, 0 at END OF TABLE, -1 = inval */
struct kind_table {
	short	kt_kind;
	short	kt_kflag;
	int	kt_socktype;
	char	*kt_name;
} kind_table [] = {
	{ KIND_SOCKET, 0, SOCK_STREAM, "stream" },
	{ KIND_SOCKET, 0, SOCK_DGRAM, "dgram" },
	{ KIND_SOCKET, 0, SOCK_RDM, "rdm" },
	{ KIND_SOCKET, 0, SOCK_SEQPACKET, "seqpacket" },
	{ KIND_SOCKET, 0, SOCK_RAW, "raw" },
#ifdef _SUN
        { KIND_SUNRPC, 0, SOCK_STREAM, "sunrpc_tcp" },
        { KIND_SUNRPC, 0, SOCK_DGRAM, "sunrpc_udp" },
#endif
	{ 0, 0, 0, 0 }
};

int echo_stream(), discard_stream(), machtime_stream();
int daytime_stream(), chargen_stream();
int echo_dg(), discard_dg(), machtime_dg(), daytime_dg(), chargen_dg();

struct biltin {
	char	*bi_service;		/* internally provided service name */
	int	bi_socktype;		/* type of socket supported */
	short	bi_fork;		/* 1 if should fork before call */
	short	bi_wait;		/* 1 if should wait for child */
	int	(*bi_fn)();		/* function which performs it */
} biltins[] = {
	/* Echo received data */
	"echo",		SOCK_STREAM,	1, 0,	echo_stream,
	"echo",		SOCK_DGRAM,	0, 0,	echo_dg,

	/* Internet /dev/null */
	"discard",	SOCK_STREAM,	1, 0,	discard_stream,
	"discard",	SOCK_DGRAM,	0, 0,	discard_dg,

	/* Return 32 bit time since 1970 */
	"time",		SOCK_STREAM,	0, 0,	machtime_stream,
	"time",		SOCK_DGRAM,	0, 0,	machtime_dg,

	/* Return human-readable time */
	"daytime",	SOCK_STREAM,	0, 0,	daytime_stream,
	"daytime",	SOCK_DGRAM,	0, 0,	daytime_dg,

	/* Familiar character generator */
	"chargen",	SOCK_STREAM,	1, 0,	chargen_stream,
	"chargen",	SOCK_DGRAM,	0, 0,	chargen_dg,
	0
};

#define NUMINT	(sizeof(intab) / sizeof(struct inent))
char	*CONFIG = _PATH_INETDCONF;
char	**Argv;
char 	*LastArg;

/* The following is included to enable ILS and Message Catalogs */
#include <nl_types.h>
#include "inetd_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_INETD, n, s)
#include <locale.h>

/* The following defines and declarations were added for SRC support */
#include 	<spc.h>
#define SRC_FD		13	/* SRC file descriptor number */
char *progname;

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	extern char *optarg;
	extern int optind;
	register struct servtab *sep;
	register struct passwd *pwd;
	register int tmpint;
	struct sigvec sv;
	int ch, pid, dofork, timewait = 0;
	char buf[50];
	struct timeval tv;
	struct timeval *tvp;

	/* The following declarations were added for SRC support */
	int src_exists = TRUE;
	struct sockaddr_in srcaddr;
	int addrsz = sizeof(srcaddr);
	struct srcreq srcpacket;

	/* stash the program name away for SRC support */
	progname = (char *) malloc(strlen(argv[0]) + 1);
	strcpy(progname, argv[0]);

	/* Set the current locale and open the Message Catalog */
	setlocale(LC_ALL,"");
	catd = catopen(MF_INETD, NL_CAT_LOCALE);

	openlog("inetd", LOG_PID | LOG_NOWAIT, LOG_DAEMON);
	/* 
	 * The following code were added for SRC support.  Stdin is checked
	 * to make sure it is the SRC socket and it is a UNIX domain socket.
	 * Once verified, stdin's file descriptor is duped to a new one
	 * because file descriptor 0 (stdin) is used internally by inetd
	 * and its' subservers.
	 */
	if ((ch = getsockname(0, &srcaddr, &addrsz)) < 0) {
                src_exists = FALSE; /* Continue with out SRC support */
                syslog(LOG_NOTICE, MSGSTR(SRC1,
		"%s: SRC was not found, going on without SRC support\n"),
                progname);
        }
        if (src_exists == TRUE) {
                (void) dup2(0, SRC_FD);
		FD_SET (SRC_FD, &allsock);
		nsock++;
	}

	Argv = argv;
	if (envp == 0 || *envp == 0)
		envp = argv;
	while (*envp)
		envp++;
	LastArg = envp[-1] + strlen(envp[-1]);

	while ((ch = getopt(argc, argv, "dt:")) != EOF)
		switch(ch) {
		case 't':
			timewait = atoi(optarg);
			if (timewait <= 0 || timewait > 999999) {
				(void) fprintf(stderr, MSGSTR(WAITTIME,
					"inetd: invalid wait time: %s\n"),
					optarg);
				(void) fprintf(stderr, MSGSTR(USAGE,
				    "usage: inetd [-d] [ConfigurationFile]\n"));
				exit(1);
			}
			break;
		case 'd':
			debug = 1;
			options |= SO_DEBUG;
			break;
		case '?':
		default:
                        fprintf(stderr, MSGSTR(USAGE,
				"usage: inetd [-d] [ConfigurationFile]\n"));
			exit(1);
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		CONFIG = argv[0];
	if (src_exists == FALSE) /* Don't fork() if under SRC control */ 
		if (debug == 0) /* Don't fork() if under debug mode */
			daemon(0, 0);
	bzero((char *)&sv, sizeof(sv));
	sv.sv_mask = SIGBLOCK;
	sv.sv_handler = retry;
	sigvec(SIGALRM, &sv, (struct sigvec *)0);
	config();
	sv.sv_handler = config;
	sigvec(SIGHUP, &sv, (struct sigvec *)0);
	sv.sv_handler = reapchild;
	sigvec(SIGCHLD, &sv, (struct sigvec *)0);

	{
		/* space for daemons to overwrite environment for ps */
#define	DUMMYSIZE	100
#ifdef _AIX
		char dummy[DUMMYSIZE];
		char buff[DUMMYSIZE + sizeof("inetd_dummy=")];

		(void)memset(dummy, 'x', sizeof(DUMMYSIZE) - 1);
		dummy[DUMMYSIZE - 1] = '\0';
		sprintf(buff, "inetd_dummy=%s", dummy);
		(void)putenv(buff);
#else
		char dummy[DUMMYSIZE];

		(void)memset(dummy, 'x', sizeof(DUMMYSIZE) - 1);
		dummy[DUMMYSIZE - 1] = '\0';
		(void)setenv("inetd_dummy", dummy, 1);
#endif
	}

	/*
	 * This timewait paramter is used to timeout the select() call if
	 * needed.  This allows machines that are doing large number of
	 * wait services, like tftpd, to timeout on the select after a
	 * specified number of seconds, or if another service request arrives.
	 *
	 * This is needed since we could be in select() and not selecting
	 * on the wait service, like tftpd, since one is already going.
	 * Then, while still in select(), we could be interrupted by a
	 * SIGCHLD signal that restores the wait service.  This then puts
	 * us back in select but we are not selecting for new tftpd connections
	 * until any service request arrives or we time out the select call.
	 *
	 * Be careful with the timewait value since the lower the value the
	 * more CPU cycles inetd will use.
	 *
	 * In most cases the timewait value should not be used since any other
	 * inetd service will re-enable the wait service.
	 *
	 * The addition of the timewait value fixes defect 86867 and
	 * defect 102638.
	 */
	if (timewait > 0) {
		tvp = &tv;
		tvp->tv_sec = timewait;
		tvp->tv_usec = 0;
	} else {
		tvp = (struct timeval *) NULL;
	}

	for (;;) {
	    int n, ctrl;

	    if (nsock == 0) {
		(void) sigblock(SIGBLOCK);
		while (nsock == 0)
		    sigpause(0L);
		(void) sigsetmask(0L);
	    }
	    readable = allsock;
	    if ((n = select(maxsock + 1, &readable, (fd_set *)0,
		(fd_set *)0, tvp)) <= 0) {
		    if (n < 0 && errno != EINTR)
			syslog(LOG_WARNING, MSGSTR(SELECTWRN, "select: %m\n"));
		    sleep(1);
		    continue;
	    }
	    /*
	     * First we see if we received a SRC request to process
	     */
	    if (src_exists == TRUE && FD_ISSET (SRC_FD, &readable)) {
		ch = recvfrom(SRC_FD, &srcpacket, sizeof(srcpacket), 0,
							&srcaddr, &addrsz);
		if (ch < 0) {
			syslog(LOG_ERR,MSGSTR(SRC5,
				"%s: '%d' recvfrom\n"), progname, errno);
			exit(1);
		}
		process_srcpacket(&srcpacket);
	    }
	    for (sep = servtab; n && sep; sep = sep->se_next)
	        if (sep->se_fd != -1 && FD_ISSET(sep->se_fd, &readable)) {
		    n--;
		    if (debug)
			    syslog(LOG_DEBUG, MSGSTR(NEEDSERV,
				"someone wants %s\n") , sep->se_service );
		    if (!sep->se_wait && sep->se_socktype == SOCK_STREAM) {
			    ctrl = accept(sep->se_fd, (struct sockaddr *)0,
				(int *)0);
			    if (debug)
                                    syslog(LOG_DEBUG, MSGSTR(ACCEPT,
						"accept, ctrl %d\n"), ctrl);
			    if (ctrl < 0) {
				    if (errno == EINTR)
					    continue;
                                    syslog(LOG_WARNING, MSGSTR(ACCEPTWRN,
					"accept (for %s): %m"),sep->se_service);
				    continue;
			    }
		    } else
			    ctrl = sep->se_fd;
		    (void) sigblock(SIGBLOCK);
		    pid = 0;
		    dofork = (sep->se_bi == 0 || sep->se_bi->bi_fork);
		    if (dofork) {
#ifndef _AIX
			    if (sep->se_count++ == 0)
				(void)gettimeofday(&sep->se_time,
				    (struct timezone *)0);
			    else if (sep->se_count >= TOOMANY) {
				struct timeval now;

				(void)gettimeofday(&now, (struct timezone *)0);
				if (now.tv_sec - sep->se_time.tv_sec >
				    CNT_INTVL) {
					sep->se_time = now;
					sep->se_count = 1;
				} else {
					syslog(LOG_ERR, MSGSTR(LOOPINGERR,
			"%s/%s server failing (looping), service terminated\n"),
					    sep->se_service, sep->se_proto);
					FD_CLR(sep->se_fd, &allsock);
					(void) close(sep->se_fd);
					sep->se_fd = -1;
					sep->se_count = 0;
					nsock--;
					if (!timingout) {
						timingout = 1;
						alarm(RETRYTIME);
					}
				}
			    }
#endif /* !_AIX */
			    pid = fork();
		    }
		    if (pid < 0) {
			    syslog(LOG_ERR, "fork: %m");
			    if (!sep->se_wait && sep->se_socktype == SOCK_STREAM)
				    close(ctrl);
			    sigsetmask(0L);
			    sleep(1);
			    continue;
		    }
		    if (pid && sep->se_wait) {
			    sep->se_wait = pid;
			    if (sep->se_fd >= 0) {
				FD_CLR(sep->se_fd, &allsock);
			        nsock--;
			    }
		    }
		    sigsetmask(0L);
		    if (pid == 0) {
			    if (debug && dofork)
				setsid();
			    if (dofork)
				for (tmpint = maxsock; --tmpint > 2; )
					if (tmpint != ctrl)
						close(tmpint);
			    if (sep->se_bi)
				(*sep->se_bi->bi_fn)(ctrl, sep);
			    else {
				if (debug)
					fprintf(stderr, "%d execl %s\n",
					    getpid(), sep->se_server);
				dup2(ctrl, 0);
				close(ctrl);
				dup2(0, 1);
				dup2(0, 2);
				if ((pwd = getpwnam(sep->se_user)) == NULL) {
					syslog(LOG_ERR, MSGSTR(PWNAM,
					    "getpwnam: %s: No such user"),
					    sep->se_user);
					if (sep->se_socktype != SOCK_STREAM)
						recv(0, buf, sizeof (buf), 0);
#ifdef _SUN
					/* clean up if SUN RPC tcp fails */
					if (sep->se_kind == KIND_SUNRPC &&
					    sep->se_socktype == SOCK_STREAM)
						(void)close(accept(0,
							(struct sockaddr *)NULL,
							(int *)NULL));
#endif
					_exit(1);
				}
				if (pwd->pw_uid) {
					(void) setgid((gid_t)pwd->pw_gid);
					initgroups(pwd->pw_name, pwd->pw_gid);
					(void) setuid((uid_t)pwd->pw_uid);
				}
				execv(sep->se_server, sep->se_argv);
				if (sep->se_socktype != SOCK_STREAM)
					recv(0, buf, sizeof (buf), 0);
#ifdef _SUN
				/* clean up if SUN RPC tcp fails */
				if (sep->se_kind == KIND_SUNRPC &&
				    sep->se_socktype == SOCK_STREAM)
					(void)close(accept(0,
						(struct sockaddr *)NULL,
						(int *)NULL));
#endif
				syslog(LOG_ERR, MSGSTR(EXECV,
					"execv %s: %m"), sep->se_server);
				_exit(1);
			    }
		    }
		    if (!sep->se_wait && sep->se_socktype == SOCK_STREAM) {
			close(ctrl);
			if (sep->se_fd == ctrl) {
				FD_CLR(sep->se_fd, &allsock);
				--nsock;
			}
		    }

		}
	}
}

void
reapchild()
{
	int status;
	int pid;
	register struct servtab *sep;

	for (;;) {
		pid = wait3(&status, WNOHANG, (struct rusage *)0);
		if (pid <= 0)
			break;
		if (debug)
                        syslog(LOG_DEBUG, MSGSTR(REAPED, "%d reaped\n"), pid);
		for (sep = servtab; sep; sep = sep->se_next)
			if (sep->se_wait == pid) {
				if ( status )
                                        syslog(LOG_WARNING, MSGSTR(EXITSTAT,
						"%s: exit status 0x%x"),
						sep->se_server, status);
                                if (debug)
                                        syslog(LOG_DEBUG, MSGSTR(RESTORED,
						"restored %s, fd %d\n"),
						sep->se_service, sep->se_fd);
				FD_SET(sep->se_fd, &allsock);
				nsock++;
				sep->se_wait = 1;
			}
	}
	/* The following line was added to fix defect 86867. However, it
	   introduced another problem (refer to defect 102638). Therefore,
	   this fix will not longer be included.			   */
/*	readable = allsock;	*/

}

void
config()
{
	register struct servtab *sep, *cp, **sepp;
	struct servtab *getconfigent(), *enter();
	long omask;
        struct protoent *pp;
	int i;

	if (!setconfig()) {
		syslog(LOG_ERR, "%s: %m", CONFIG);
		return;
	}
	for (sep = servtab; sep; sep = sep->se_next)
		sep->se_checked = 0;
	while (cp = getconfigent()) {
		for (sep = servtab; sep; sep = sep->se_next)
			if (strcmp(sep->se_service, cp->se_service) == 0 &&
			    strcmp(sep->se_proto, cp->se_proto) == 0)
				break;
		if (sep != 0) {
			int i;

			omask = sigblock(SIGBLOCK);
			/*
			 * sep->se_wait may be holding the pid of a daemon
			 * that we're waiting for.  If so, don't overwrite
			 * it unless the config file explicitly says don't
			 * wait.
			 */
			if (cp->se_bi == 0 && 
			    (sep->se_wait == 1 || cp->se_wait == 0))
				sep->se_wait = cp->se_wait;
#define SWAP(a, b) { char *c = a; a = b; b = c; }
			if (cp->se_user)
				SWAP(sep->se_user, cp->se_user);
			if (cp->se_server)
				SWAP(sep->se_server, cp->se_server);
			for (i = 0; i < MAXARGV; i++)
				SWAP(sep->se_argv[i], cp->se_argv[i]);
			sigsetmask(omask);
			freeconfig(cp);
			if (debug)
				print_service(MSGSTR(REDOMSG, "REDO"), sep);
		} else {
			sep = enter(cp);
			if (debug)
				print_service(MSGSTR(ADDMSG, "ADD "), sep);
		}
		sep->se_checked = 1;
                pp = getprotobyname(sep->se_proto);
                if (pp == (struct protoent *)NULL) {
                        syslog(LOG_ERR, MSGSTR(UNKNOWNPROTO,
				"%s: unknown protocol: %s"),
                                sep->se_service, sep->se_proto);
                        continue;
                }
                sep->se_protonum = pp->p_proto;
                switch (sep->se_kind) {
                case KIND_SOCKET:
                        sp = getservbyname(sep->se_service, sep->se_proto);
                        if (sp == (struct servent *)NULL) {
                                syslog(LOG_ERR, MSGSTR(UNKNOWNSERV,
					"%s/%s: unknown service"),
                                        sep->se_service, sep->se_proto);
                                continue;
                        }
                        if (sp->s_port != sep->se_ctrladdr.sin_port) {
                                sep->se_ctrladdr.sin_port = sp->s_port;
                                if (sep->se_fd != -1)
                                        (void) close(sep->se_fd);
                                sep->se_fd = -1;
                        }
                        break;
#ifdef _SUN
                case KIND_SUNRPC:
			unregister(sep);	
	 		if (sep->se_fd != -1) {
				FD_CLR(sep->se_fd, &allsock);
				nsock--;
				(void) close(sep->se_fd);
			}
			sep->se_fd = -1;
			setup_sunrpc(sep);
                        break;
#endif
                default:
                        syslog(LOG_ERR, MSGSTR(UNKNOWNKS,
				"%s: unknown kind for service: %d"),
                                sep->se_service, sep->se_kind);
		}
		if (sep->se_fd == -1)
			setup(sep);
	}
	endconfig();
	/*
	 * Purge anything not looked at above.
	 */
	omask = sigblock(SIGBLOCK);
	sepp = &servtab;
	while (sep = *sepp) {
		if (sep->se_checked) {
			sepp = &sep->se_next;
			continue;
		}
		*sepp = sep->se_next;
		if (sep->se_fd != -1) {
			FD_CLR(sep->se_fd, &allsock);
			nsock--;
			(void) close(sep->se_fd);
		}
#ifdef _SUN
                if (sep->se_kind == KIND_SUNRPC)
                        for (i = sep->se_rpc.lovers;
                                        i <= sep->se_rpc.hivers; i++)
                                pmap_unset(sep->se_rpc.prog, i);
#endif
		if (debug)
			print_service(MSGSTR(FREEMSG, "FREE"), sep);
		freeconfig(sep);
		free((char *)sep);
	}
	(void) sigsetmask(omask);
}

void
retry()
{
	register struct servtab *sep;

	timingout = 0;
	for (sep = servtab; sep; sep = sep->se_next)
		if (sep->se_fd == -1)
			setup(sep);
}

setup(sep)
	register struct servtab *sep;
{
	int on = 1;

	switch (sep->se_kind) {
        case KIND_SOCKET:
		if ((sep->se_fd = socket(AF_INET, sep->se_socktype, 0)) < 0) {
			syslog(LOG_ERR, MSGSTR(SOCKERR, "%s/%s: socket: %m"),
				sep->se_service, sep->se_proto);
			return;
		}
#define	turnon(fd, opt) \
		setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))
		if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
		    turnon(sep->se_fd, SO_DEBUG) < 0)
			syslog(LOG_ERR, MSGSTR(SETSOCK,
						"setsockopt (SO_DEBUG): %m"));
		if (turnon(sep->se_fd, SO_REUSEADDR) < 0)
			syslog(LOG_ERR, MSGSTR(SETSOCKOPT,
					"setsockopt (SO_REUSEADDR): %m"));
#undef turnon
		sep->se_ctrladdr.sin_family = AF_INET;
		sep->se_ctrladdr.sin_addr.s_addr = 0;
		if (bind(sep->se_fd, &sep->se_ctrladdr,
		    sizeof (sep->se_ctrladdr)) < 0) {
			syslog(LOG_ERR, MSGSTR(BIND, "%s/%s: bind: %m"),
				sep->se_service, sep->se_proto);
			(void) close(sep->se_fd);
			sep->se_fd = -1;
			if (!timingout) {
				timingout = 1;
				alarm(RETRYTIME);
			}
			return;
		}
		if (sep->se_socktype == SOCK_STREAM)
			listen(sep->se_fd, 10);
                break;
#ifdef _SUN
        case KIND_SUNRPC:
                if ((sep->se_fd = setup_sunrpc(sep)) < 0) {
                        syslog(LOG_ERR, MSGSTR(SUNSETUP,
				"%s/%s: setup_sunrpc: %s: %m"), /*MSG*/
                                sep->se_service, sep->se_proto, sep->se_server);
                        return;
                }
                break;
#endif
        default:
                syslog(LOG_ERR, MSGSTR(UNKSETUP,"setup: unknown kind: %d "),
								sep->se_kind);
                return;
	}
	FD_SET(sep->se_fd, &allsock);
	nsock++;
	if (sep->se_fd > maxsock)
		maxsock = sep->se_fd;
}

struct servtab *
enter(cp)
	struct servtab *cp;
{
	register struct servtab *sep;
	long omask;

	sep = (struct servtab *)malloc(sizeof (*sep));
	if (sep == (struct servtab *)0) {
                syslog(LOG_ERR, MSGSTR(MEMERR, "Out of memory."));
		exit(-1);
	}
	*sep = *cp;
	sep->se_fd = -1;
	omask = sigblock(SIGBLOCK);
	sep->se_next = servtab;
	servtab = sep;
	sigsetmask(omask);
	return (sep);
}

FILE	*fconfig = NULL;
struct	servtab serv;
char	line[256];
char	*skip(), *nextline();

setconfig()
{

	if (fconfig != NULL) {
		fseek(fconfig, 0L, L_SET);
		return (1);
	}
	fconfig = fopen(CONFIG, "r");
	return (fconfig != NULL);
}

endconfig()
{
	if (fconfig) {
		(void) fclose(fconfig);
		fconfig = NULL;
	}
}

struct servtab *
getconfigent()
{
	register struct servtab *sep = &serv;
        register struct kind_table      *kt;
	struct servent *sp;
	int argc;
	char *cp, *arg, *newstr();

more:
	while ((cp = nextline(fconfig)) && *cp == '#')
		;
	if (cp == NULL)
		return ((struct servtab *)0);
	sep->se_service = newstr(skip(&cp));
	arg = skip(&cp);
        /*
         * use the new-fangled kind_table to get both kind and socktype!
         */
        sep->se_socktype = -1;
        sep->se_kind = -1;
        for (kt = kind_table; kt->kt_kind; kt++)
                if (strcmp(arg, kt->kt_name) == 0) {
                        sep->se_socktype = kt->kt_socktype;
                        sep->se_kind = kt->kt_kind;
                        break;
                }
        if (kt->kt_kind == 0) { /* not found */
                syslog(LOG_ERR, MSGSTR(UNKNOWNKIND,
			"unknown kind %s for service %s, skipping\n"),
                        arg, sep->se_service);
                goto more;
        }
	sep->se_proto = newstr(skip(&cp));
	switch (sep->se_kind) {
	case KIND_SOCKET:
		sp = getservbyname(sep->se_service, sep->se_proto);
		if (sp == (struct servent *)NULL) {
			syslog(LOG_ERR, MSGSTR(UNKNOWNSERV,
				"%s/%s: unknown service"),
				sep->se_service, sep->se_proto);
		}
		/*
		 * We stash the port number in the servtab
		 * entry for SRC lookups
		 */
		sep->se_portnum = (long) sp->s_port;
		break;
#ifdef _SUN
	case KIND_SUNRPC:
		sp = getservbyname("sunrpc", sep->se_proto);
		if (sp == (struct servent *)NULL) {
			syslog(LOG_ERR, MSGSTR(UNKNOWNSERV,
				"%s/%s: unknown service"),
				"sunrpc", sep->se_proto);
		}
		/*
		 * We stash the port number in the servtab
		 * entry for SRC lookups
		 */
		sep->se_portnum = (long) sp->s_port;
		break;
#endif
	default:
		syslog(LOG_ERR, MSGSTR(UNKNOWNKS,
			"%s: unknown kind for service: %d"),
			sep->se_service, sep->se_kind);
	}
	arg = skip(&cp);
	sep->se_wait = strcmp(arg, "wait") == 0;
	sep->se_user = newstr(skip(&cp));
	sep->se_server = newstr(skip(&cp));
	if (strcmp(sep->se_server, "internal") == 0) {
		register struct biltin *bi;

		for (bi = biltins; bi->bi_service; bi++)
			if (bi->bi_socktype == sep->se_socktype &&
			    strcmp(bi->bi_service, sep->se_service) == 0)
				break;
		if (bi->bi_service == 0) {
			syslog(LOG_ERR, "internal service %s unknown\n",
				sep->se_service);
			goto more;
		}
		sep->se_bi = bi;
		sep->se_wait = bi->bi_wait;
	} else
		sep->se_bi = NULL;
	argc = 0;
	for (arg = skip(&cp); cp; arg = skip(&cp))
		if (argc < MAXARGV)
			sep->se_argv[argc++] = newstr(arg);
	while (argc <= MAXARGV)
		sep->se_argv[argc++] = NULL;
        /*
         * For SUNRPC, we now parse the version and rpc number.
         *      argv[1] is the rpc number
         *      argv[2] is the version number or range of versions
         */
#ifdef _SUN
        if (sep->se_kind == KIND_SUNRPC) {
                if (argc < 3) {
                        syslog(LOG_ERR, MSGSTR(NORPCVER,
        "sunrpc service %s does not have BOTH rpc and version specified\n"),
                                sep->se_service);
                        goto more;
                }
                sep->se_rpc.prog = atoi(sep->se_argv[1]);
                sep->se_rpc.lovers = atoi(sep->se_argv[2]);
                /* WARNING: reusing "cp" */
                cp = strchr(sep->se_argv[2], '-');
                if (cp != NULL)
                        sep->se_rpc.hivers = atoi(cp + 1);
                else
                        sep->se_rpc.hivers = sep->se_rpc.lovers;
        }
#endif
	return (sep);
}

freeconfig(cp)
	register struct servtab *cp;
{
	int i;

	if (cp->se_service)
		free(cp->se_service);
	if (cp->se_proto)
		free(cp->se_proto);
	if (cp->se_user)
		free(cp->se_user);
	if (cp->se_server)
		free(cp->se_server);
	for (i = 0; i < MAXARGV; i++)
		if (cp->se_argv[i])
			free(cp->se_argv[i]);
}

char *
skip(cpp)
	char **cpp;
{
	register char *cp = *cpp;
	char *start;

again:
	while (*cp == ' ' || *cp == '\t')
		cp++;
	if (*cp == '\0') {
		int c;

		c = getc(fconfig);
		(void) ungetc(c, fconfig);
		if (c == ' ' || c == '\t')
			if (cp = nextline(fconfig))
				goto again;
		*cpp = (char *)0;
		return ((char *)0);
	}
	start = cp;
	while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	if (*cp != '\0')
		*cp++ = '\0';
	*cpp = cp;
	return (start);
}

char *
nextline(fd)
	FILE *fd;
{
	char *cp;

	if (fgets(line, sizeof (line), fd) == NULL)
		return ((char *)0);
	cp = index(line, '\n');
	if (cp)
		*cp = '\0';
	return (line);
}

char *
newstr(cp)
	char *cp;
{
	if (cp = strdup(cp ? cp : ""))
		return(cp);
	syslog(LOG_ERR, MSGSTR(MEMERR, "Out of memory."));
	exit(-1);
}

setproctitle(a, s)
	char *a;
	int s;
{
	int size;
	register char *cp;
	struct sockaddr_in sin;
	char buf[80];

	cp = Argv[0];
	size = sizeof(sin);
	if (getpeername(s, (struct sockaddr *)&sin, &size) == 0)
		(void) sprintf(buf, "-%s [%s]", a, inet_ntoa(sin.sin_addr)); 
	else
		(void) sprintf(buf, "-%s", a); 
	strncpy(cp, buf, LastArg - cp);
	cp += strlen(cp);
	while (cp < LastArg)
		*cp++ = ' ';
}

/*
 * Internet services provided internally by inetd:
 */
#define	BUFSIZE	8192

/* ARGSUSED */
echo_stream(s, sep)		/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];
	int i;

	setproctitle(sep->se_service, s);
	while ((i = read(s, buffer, sizeof(buffer))) > 0 &&
	    write(s, buffer, i) > 0)
		;
	exit(0);
}

/* ARGSUSED */
echo_dg(s, sep)			/* Echo service -- echo data back */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];
	int i, size;
	struct sockaddr sa;

	size = sizeof(sa);
	if ((i = recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size)) < 0)
		return;
	(void) sendto(s, buffer, i, 0, &sa, sizeof(sa));
}

/* ARGSUSED */
discard_stream(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	int ret;
	char buffer[BUFSIZE];

	setproctitle(sep->se_service, s);
	while (1) {
		while ((ret = read(s, buffer, sizeof(buffer))) > 0)
			;
		if (ret == 0 || errno != EINTR)
			break;
	}
	exit(0);
}

/* ARGSUSED */
discard_dg(s, sep)		/* Discard service -- ignore data */
	int s;
	struct servtab *sep;
{
	char buffer[BUFSIZE];

	(void) read(s, buffer, sizeof(buffer));
}

#include <ctype.h>
#define LINESIZ 72
char ring[128];
char *endring;

initring()
{
	register int i;

	endring = ring;

	for (i = 0; i <= 128; ++i)
		if (isprint(i))
			*endring++ = i;
}

/* ARGSUSED */
chargen_stream(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	register char *rs;
	int len;
	char text[LINESIZ+2];

	setproctitle(sep->se_service, s);

	if (!endring) {
		initring();
		rs = ring;
	}

	text[LINESIZ] = '\r';
	text[LINESIZ + 1] = '\n';
	for (rs = ring;;) {
		if ((len = endring - rs) >= LINESIZ)
			bcopy(rs, text, LINESIZ);
		else {
			bcopy(rs, text, len);
			bcopy(ring, text + len, LINESIZ - len);
		}
		if (++rs == endring)
			rs = ring;
		if (write(s, text, sizeof(text)) != sizeof(text))
			break;
	}
	exit(0);
}

/* ARGSUSED */
chargen_dg(s, sep)		/* Character generator */
	int s;
	struct servtab *sep;
{
	struct sockaddr sa;
	static char *rs;
	int len, size;
	char text[LINESIZ+2];

	if (endring == 0) {
		initring();
		rs = ring;
	}

	size = sizeof(sa);
	if (recvfrom(s, text, sizeof(text), 0, &sa, &size) < 0)
		return;

	if ((len = endring - rs) >= LINESIZ)
		bcopy(rs, text, LINESIZ);
	else {
		bcopy(rs, text, len);
		bcopy(ring, text + len, LINESIZ - len);
	}
	if (++rs == endring)
		rs = ring;
	text[LINESIZ] = '\r';
	text[LINESIZ + 1] = '\n';
	(void) sendto(s, text, sizeof(text), 0, &sa, sizeof(sa));
}

/*
 * Return a machine readable date and time, in the form of the
 * number of seconds since midnight, Jan 1, 1900.  Since gettimeofday
 * returns the number of seconds since midnight, Jan 1, 1970,
 * we must add 2208988800 seconds to this figure to make up for
 * some seventy years Bell Labs was asleep.
 */

long
machtime()
{
	struct timeval tv;

	if (gettimeofday(&tv, (struct timezone *)0) < 0) {
		fprintf(stderr, "Unable to get time of day\n");
		return (0L);
	}
	return (htonl((long)tv.tv_sec + 2208988800));
}

/* ARGSUSED */
machtime_stream(s, sep)
	int s;
	struct servtab *sep;
{
	long result;

	result = machtime();
	(void) write(s, (char *) &result, sizeof(result));
}

/* ARGSUSED */
machtime_dg(s, sep)
	int s;
	struct servtab *sep;
{
	long result;
	struct sockaddr sa;
	int size;

	size = sizeof(sa);
	if (recvfrom(s, (char *)&result, sizeof(result), 0, &sa, &size) < 0)
		return;
	result = machtime();
	(void) sendto(s, (char *) &result, sizeof(result), 0, &sa, sizeof(sa));
}

/* ARGSUSED */
daytime_stream(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	char *ctime();

	clock = time((time_t *) 0);

	(void) sprintf(buffer, "%.24s\r\n", ctime(&clock));
	(void) write(s, buffer, strlen(buffer));
}

/* ARGSUSED */
daytime_dg(s, sep)		/* Return human-readable time of day */
	int s;
	struct servtab *sep;
{
	char buffer[256];
	time_t time(), clock;
	struct sockaddr sa;
	int size;
	char *ctime();

	clock = time((time_t *) 0);

	size = sizeof(sa);
	if (recvfrom(s, buffer, sizeof(buffer), 0, &sa, &size) < 0)
		return;
	(void) sprintf(buffer, "%.24s\r\n", ctime(&clock));
	(void) sendto(s, buffer, strlen(buffer), 0, &sa, sizeof(sa));
}

/*
 * print_service:
 *	Dump relevant information to syslog
 */
print_service(action, sep)
	char *action;
	struct servtab *sep;
{
	syslog(LOG_DEBUG, MSGSTR(PRNTSERV,
		"%s: %s proto=%s, wait=%d, user=%s builtin=%x server=%s\n"),
		action, sep->se_service, sep->se_proto,
		sep->se_wait, sep->se_user, (int)sep->se_bi, sep->se_server);
}


#ifdef _SUN
/*
 * setup_sunrpc sets up a socket for SUN's rpc.
 * Returns valid fd or -1 for error.
 */
setup_sunrpc( sep )
	register struct servtab	*sep;
{
	int			s, i, on = 1;
	struct sockaddr_in	addr;
	int			len = sizeof(struct sockaddr_in);
	static int		first = 1;
	static int		ok = 1;
	/* these were args to getrpcsock */
	int			prognum;	/* sep->se_rpc.prog; */
	int			lovers;		/* sep->se_rpc.lovers; */
	int			hivers;		/* sep->se_rpc.hivers; */
	int			kind;		/* sep->se_kind; */
	
	prognum = sep->se_rpc.prog;
	lovers = sep->se_rpc.lovers;
	hivers = sep->se_rpc.hivers;
	kind = sep->se_kind;

	/* 
	 * Do unsets.
	 */
	unregister(sep);

	if ((s = socket(AF_INET, sep->se_socktype, 0)) < 0) {
                syslog(LOG_ERR, MSGSTR(SUNSOCK,
			"setup_sunrpc of %s: socket(): %m"),
                        sep->se_service);
		return(-1);
	}
	/* set the socket options; if errors occur, just log them */
#define	turnon(fd, opt) \
	setsockopt(fd, SOL_SOCKET, opt, (char *)&on, sizeof (on))

	if (strcmp(sep->se_proto, "tcp") == 0 && (options & SO_DEBUG) &&
            turnon(s, SO_DEBUG) < 0)
                syslog(LOG_ERR, MSGSTR(SETSOCK, "setsockopt (SO_DEBUG): %m"));
        if (turnon(s, SO_REUSEADDR) < 0)
                syslog(LOG_ERR, MSGSTR(SETSOCKOPT,
					"setsockopt (SO_REUSEADDR): %m"));
#undef turnon
	/* this bind gets any old address, as assigned by the system */
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = 0;
	if (bind(s, &addr, sizeof(addr)) < 0) {
                syslog(LOG_ERR, MSGSTR(SUNBIND,
			"setup_sunrpc of %s: bind(): %m"), sep->se_service);
		(void)close(s);
		return(-1);
	}
	if (getsockname(s, &addr, &len) != 0) {
                syslog(LOG_ERR, MSGSTR(SUNGETSOCK,
			"setup_sunrpc of %s: getsockname(): %m"),
                        sep->se_service);
		(void)close(s);
		return(-1);
	}
	/* this registers the socket address with the RPC port mapper */
	for (i = lovers; i <= hivers; i++)
		pmap_set(prognum, i, sep->se_protonum, htons(addr.sin_port));
	if (sep->se_protonum == IPPROTO_TCP) {
		if (listen(s, 10) < 0) {
			perror("inetd setup_rpc");
			(void)close(s);
			return(-1);
		}
	}
	return(s);
}
#endif

dosrcpacket(msgno, subsys, txt, len, srcpacket)
int msgno;
char *subsys;
char *txt;
int len;
struct srcreq *srcpacket;
{
	struct srcrep reply;
	struct srchdr *srchdr;

	reply.svrreply.rtncode = msgno;
	strncpy(reply.svrreply.objname, subsys, SRCNAMESZ);
	strncpy(reply.svrreply.rtnmsg, txt, 256);
	srchdr = srcrrqs(srcpacket);
	srcsrpy(srchdr, &reply, len, END);
}
/* 
 * In the case of a forced stop of inetd via SRC,
 * all active subservers must be killed.
 */
#define	PS_CMD "/bin/ps -e"
#define	GREP_CMD "/bin/egrep"
#define	AWK_CMD "/bin/awk '{ print $1 }'"
#define	XARGS_CMD "/bin/xargs"
#define	KILL_CMD "/bin/kill -9"

kill_active_subserver(service, server, srcpacket)
char	*service;
char	*server;
struct srcreq *srcpacket;
{
	char buffer[256];
	FILE *stfp;
	
	sprintf(buffer, "%s | %s \" %s$\" | %s | %s %s", PS_CMD, GREP_CMD,
		server, AWK_CMD, XARGS_CMD, KILL_CMD);
	stfp = popen(buffer,"r");
	while (fgets(buffer, sizeof(buffer), stfp) > 0) {
	   dosrcpacket(SRC_SUBMSG, service, buffer,
				sizeof(struct srcrep), srcpacket);
	}
	pclose(stfp);
}

char *
get_serv_name(port, service, proto)
short port;
char *service;
char *proto;
{
        struct servent *serv;
	char return_buf[256];
        int i = 0;

        if ((serv = getservbyport (port, NULL)) != NULL) {
		sprintf(return_buf,"%s", serv->s_name);
		strncpy(service, serv->s_name, strlen(serv->s_name) + 1);
		strncpy(proto, serv->s_proto, strlen(serv->s_proto) + 1);
		return(return_buf);
        } else	{
		return(NULL);
	}
}

#define SRCSTATNDX	6

process_srcpacket(srcpacket)
struct srcreq *srcpacket;
{
	register struct servtab *sep;
	struct	 statrep *srcstatus;
	struct	 srchdr *srchdr;
	short	 srcstatsize;
	int 	 srcreplen, x;
	char 	 buffer[256];
	FILE	 *stfp;
	int	 cont;

	/* process the request packet */
	switch(srcpacket->subreq.action) {
	    case START:
		    if (srcpacket->subreq.object == SUBSYSTEM) {
			    dosrcpacket(SRC_SUBMSG, progname, MSGSTR(SRC6,
				"inetd does not support this option.\n"),
					sizeof(struct srcrep), srcpacket);
		    } else {
			    char *tmp_cp, tmpbuf[256];
			    char tmpbuf2[256];
			    /*
			     * Now we call the CHSUBSERVER script to
			     * uncomment the service to start.
			     * get_serv_name() returns the service name
			     * associated with the port number in
			     * srcpacket->subreq.object.
			     */
			    if ((tmp_cp =
				get_serv_name(srcpacket->subreq.object,
						    tmpbuf, tmpbuf2)) != NULL) {
				sprintf(buffer,"%s -a -v \"%s\" -p \"%s\" -C %s",
					_PATH_CHSUBSERVER, tmpbuf, tmpbuf2, CONFIG);
				if ((stfp = popen(buffer,"r")) != NULL) {
					while (fgets(buffer,sizeof(buffer),
								    stfp) > 0) {
					   dosrcpacket(SRC_SUBMSG, tmpbuf,
						buffer, sizeof(struct srcrep),
								     srcpacket);
					}
					pclose(stfp);
					config();
					dosrcpacket(SRC_OK, tmpbuf, NULL,
					      sizeof(struct srcrep), srcpacket);
				} else
					syslog(LOG_WARNING,
						"popen(%s,\"r\"): %m", buffer);
			    } else {
				dosrcpacket(SRC_SUBICMD, tmpbuf, NULL,
					sizeof(struct srcrep), srcpacket);
			    }
		    }
		    break;
	    case STOP:
		    if (srcpacket->subreq.object == SUBSYSTEM) {
			    /* Simply have inetd exit gracefully */
			    dosrcpacket(SRC_OK, progname, NULL,
					sizeof(struct srcrep), srcpacket);
			    exit(0);
		    } else {
			    char tmpbuf[256];
			    char tmpbuf2[256];
			    short found = FALSE;
			    /*
			     * Find the service name using the port number
			     * supplied in srcpacket->subreq.object.
			     */
			    for (sep = servtab; sep; sep = sep->se_next) {
				if (srcpacket->subreq.object
							== sep->se_portnum) {
				    found = TRUE;
				    /*
				     * Now we call the CHSUBSERVER script to
				     * comment out the service to stop.
				     */
				    sprintf(buffer,
				       "%s -d -v \"%s\" -p \"%s\" -g \"%s\" -C %s",
					_PATH_CHSUBSERVER, sep->se_service,
					sep->se_proto, sep->se_server, CONFIG);
				    if ((stfp = popen(buffer, "r")) != NULL) {
					   while (fgets(buffer, sizeof(buffer),
								    stfp) > 0) {
					       dosrcpacket(SRC_SUBMSG,
							sep->se_service,
							buffer, sizeof(struct
							srcrep), srcpacket);
					   }
					   pclose(stfp);
					   strncpy(tmpbuf, sep->se_service,
								sizeof(tmpbuf));
					   if (srcpacket->subreq.parm1 !=
									NORMAL){
						 /* Kill all the daemons running
						 * on a particular service.
						 * Where x is the pid of the
						 * object to stop.
						 */
					    strncpy(tmpbuf2, sep->se_server,
							sizeof(tmpbuf2));
					    kill_active_subserver(tmpbuf,
							tmpbuf2, srcpacket);
					}
					config();
				        dosrcpacket(SRC_OK, tmpbuf, NULL,
						sizeof(struct srcrep),
							srcpacket);
				    } else
					syslog(LOG_WARNING,
						"popen(%s,\"r\"): %m", buffer);
				break;
				}
			    }
			    if (found == FALSE) {
			       char *tmp_cp2;
			       char serv_name_buf[256], dummy[256];

			       if ((tmp_cp2 =
				  get_serv_name(srcpacket->subreq.object,
						    serv_name_buf, dummy)) != NULL) {
				        dosrcpacket(SRC_OK, serv_name_buf, NULL,
					      sizeof(struct srcrep), srcpacket);
				}
			    }
		    }
		    break;
	    case REFRESH:
		    config();
		    dosrcpacket(SRC_OK, progname, NULL, sizeof(struct srcrep),
								srcpacket);
		    break;
	    case STATUS:
		   srcreplen = srcstatsize = sizeof(struct srchdr) + 
			(sizeof(struct statcode) * SRCSTATNDX);
		    srcstatus = (struct statrep *)malloc(srcstatsize);
		    if (srcpacket->subreq.object == SUBSYSTEM) {
			memset(srcstatus, '\0', srcreplen);
			cont = NEWREQUEST;
			srcstat("", "", getpid(), &srcstatsize, srcstatus, &cont);
			srchdr = srcrrqs(srcpacket);
			sprintf(srcstatus->statcode[3].objname,
				"%-12s",MSGSTR(SRC7,"Debug"));
			if (debug) 
				sprintf(srcstatus->statcode[3].objtext,
					" %s", MSGSTR(SRC8,"Active"));
			else
				sprintf(srcstatus->statcode[3].objtext,
					" %s", MSGSTR(SRC9,"Inactive"));
			sprintf(srcstatus->statcode[5].objname,
				"%-12s",MSGSTR(SRC10,"Signal"));
			sprintf(srcstatus->statcode[5].objtext,
				" %s", MSGSTR(SRC11,"Purpose"));
			srcsrpy(srchdr, srcstatus, srcreplen, STATCONTINUED);
			memset(srcstatus, '\0', srcreplen);
			sprintf(srcstatus->statcode[0].objname,
				" %-12s",MSGSTR(SRC12,"SIGALRM"));
			sprintf(srcstatus->statcode[0].objtext,
				"%s", MSGSTR(SRC13,"Establishes socket connections for failed services"));
			sprintf(srcstatus->statcode[1].objname,
				" %-12s",MSGSTR(SRC14,"SIGHUP"));
			sprintf(srcstatus->statcode[1].objtext,
				"%s", MSGSTR(SRC15,"Rereads configuration database and reconfigures services"));
			sprintf(srcstatus->statcode[3].objname,
				" %-12s",MSGSTR(SRC17,"SIGCHLD"));
			sprintf(srcstatus->statcode[3].objtext,
				"%s", MSGSTR(SRC18,"Restarts service in case the service dies abnormally"));
			sprintf(srcstatus->statcode[5].objname,
				"%-12s",MSGSTR(SRC19,"Service"));
			sprintf(srcstatus->statcode[5].objtext,
				" %-24s %-24s %s",
				MSGSTR(SRC20,"Command"),
				MSGSTR(SRC21,"Arguments"),
				MSGSTR(SRC22,"Status"));
			srcsrpy(srchdr,srcstatus, srcreplen, STATCONTINUED);

			memset(srcstatus, '\0', srcreplen);
			for (x = 0, sep = servtab; sep; sep = sep->se_next) {
			    if (x < SRCSTATNDX) {
				int se_argc = 0;
				char tmpstr[24], argstr[24];

				sprintf(srcstatus->statcode[x].objname,
					" %-12s", sep->se_service);
					if (strlen(sep->se_server) <= 48) {
						tmpstr[0] = NULL;
						argstr[0] = NULL;
						while (sep->se_argv[se_argc] !=
						  NULL && se_argc < MAXARGV+1) {
							strncpy(tmpstr,
							  sep->se_argv[se_argc],
								sizeof(tmpstr));
							if (strlen(tmpstr) <
							(sizeof(argstr) -
							      strlen(argstr))) {
							   strncat(argstr,
							   tmpstr,
							   sizeof(argstr));
							   strncat(argstr,
									" ", 1);
							   se_argc++;
							} else
							   break;
						}
						sprintf(srcstatus->statcode[x].objtext,
						"%-24s %-24s",
						sep->se_server,
						argstr);
					} else {
						sprintf(srcstatus->statcode[x].objtext,
						"%-49s",
						sep->se_server);
					}
				srcstatus->statcode[x].status = 1; 
				x++;
			    } else {
				srcsrpy(srchdr, srcstatus, srcreplen,
					STATCONTINUED);
				x = 0;
				memset(srcstatus,'\0',srcreplen);
			    }
			}
			if (x > 0) 
			    srcsrpy(srchdr, srcstatus, srcreplen,STATCONTINUED);
			srcsrpy(srchdr, srcstatus, sizeof(struct srchdr), END);
		    } else { 		/* subserver status request */
			memset(srcstatus,'\0',srcreplen);
			srchdr = srcrrqs(srcpacket);
			x = 0;
			sprintf(srcstatus->statcode[x].objname,
				"%-12s",MSGSTR(SRC19,"Service"));
			sprintf(srcstatus->statcode[x++].objtext,
				" %-24s %-24s %s",
				MSGSTR(SRC20,"Command"),
				MSGSTR(SRC21,"Arguments"),
				MSGSTR(SRC22,"Status"));
			srcsrpy(srchdr, srcstatus, srcreplen, STATCONTINUED);
			memset(srcstatus, '\0', srcreplen);
			for (sep = servtab; sep; sep = sep->se_next) {
			  if (srcpacket->subreq.object == sep->se_portnum) {
			    if (x < SRCSTATNDX) {
				int se_argc = 0;
				char tmpstr[24], argstr[24];

				sprintf(srcstatus->statcode[x].objname,
					" %-12s", sep->se_service);
					if (strlen(sep->se_server) <= 48) {
						tmpstr[0] = NULL;
						argstr[0] = NULL;
						while (sep->se_argv[se_argc] !=
						  NULL && se_argc < MAXARGV+1) {
							strncpy(tmpstr, sep->se_argv[se_argc], 24);
							if (strlen(tmpstr) <
							(sizeof(argstr) -
							      strlen(argstr))) {
								strncat(argstr,
								    tmpstr, 24);
								strncat(argstr,
									" ", 1);
								se_argc++;
							} else
								break;
						}
						sprintf(srcstatus->statcode[x].objtext,
						"%-24s %-24s",
						sep->se_server,
						argstr);
					} else {
						sprintf(srcstatus->statcode[x].objtext,
						"%-49s",
						sep->se_server);
					}
				srcstatus->statcode[x].status = 1; 
				x++;
			    } else {
				srcsrpy(srchdr, srcstatus, srcreplen,
					STATCONTINUED);
				x = 0;
				memset(srcstatus,'\0',srcreplen);
			    }
			  break;
			  }
		      }
		      if (x > 0) 
			srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
		      srcsrpy(srchdr,srcstatus,sizeof(struct srchdr),END);
		    }
		    free(srcstatus);
		    break;
	    case TRACE:
		    if (srcpacket->subreq.object == SUBSYSTEM) {
			debug = (srcpacket->subreq.parm2 == TRACEON) ? 1 : 0;
			if (debug)
				options |= SO_DEBUG;
			else
				options = 0;
			config();
			dosrcpacket(SRC_OK, progname, NULL,
					sizeof(struct srcrep), srcpacket);
		    } else {
			dosrcpacket(SRC_NOSUPPORT, progname, NULL,
					sizeof(struct srcrep), srcpacket);
		    }
		    break;
	    default:
		    dosrcpacket(SRC_SUBICMD, progname, NULL,
					sizeof(struct srcrep), srcpacket);
		    break;

	}
}

/*
 * Unregister an RPC service.
 */

unregister(sep)
register struct servtab *sep;
{
    register int i;
    register struct servtab *s;
    unsigned prog;

    /*
     * Unregister the service only if there were no rpc programnum
     * earlier, because then you would pmap_unset that too!
     * Added to support multiple transports for the same prognum.
     */
    prog = sep->se_rpc.prog;
    for (s = servtab; s; s = s->se_next) {
        if (s == sep)   /* Ignore this one */
            continue;
        if ((s->se_checked == 0) || !sep->se_kind == KIND_SUNRPC ||
            (prog != s->se_rpc.prog)) {
            continue;
        }
        /* Found an existing entry for that prog number */
        return;
    }
    for (i = sep->se_rpc.lovers; i <= sep->se_rpc.hivers; i++)
        pmap_unset(sep->se_rpc.prog, i);
}

