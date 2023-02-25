static char sccsid[] = "@(#)98	1.26  src/tcpip/usr/sbin/named/ns_main.c, tcpnaming, tcpip411, GOLD410 4/13/94 13:34:19";
/* 
 * COMPONENT_NAME: TCPIP ns_main.c
 * 
 * FUNCTIONS: FD_CLR, FD_ISSET, FD_SET, FD_ZERO, MSGSTR,
 *            dosrcpacket, getdtablesize, gettime, maint_alarm, 
 *            net_mask, onhup, onintr, process_src_packet, opensocket,
 *            setIncrDbgFlg, setNoDbgFlg, setchkptflg, setdebug, 
 *            setdumpflg, setproctitle, setstatsflg, sigprof, sqadd, 
 *            sqflush, sqrm, usage, getnetconf, findnetinfo, printnetinfo
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986, 1989, 1990 Regents of the University of California.
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
" Copyright (c) 1986, 1989, 1990 Regents of the University of California.\n\
 All rights reserved.\n";
#endif

#ifndef lint
static char sccsid[] = "ns_main.c	4.51 (Berkeley) 8/15/90";
#endif
*/

/*
* Internet Name server (see rfc883 & others).
*/

#include <sys/param.h>
#if defined(SYSV)
#include <fcntl.h>
#endif /* SYSV */
#include <sys/file.h>
#include <sys/time.h>
#if !defined(SYSV)
#include <sys/wait.h>
#endif /* !SYSV */
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/nameser.h>
#include <arpa/inet.h>
#include <resolv.h>
#undef nsaddr				/* XXX */
#include "ns.h"
#include "db.h"
#include "pathnames.h"

/*
 * The following includes and declerations are for support of the System 
 * Resource Controller (SRC) - .
 */
#include <spc.h>
#include "libffdb.h"
#include "libsrc.h"
static 	struct srcreq srcpacket;
struct	statrep *srcstatus;
short	srcstatsize;
int 	srcreplen;
int 	cont;
struct  srchdr *srchdr;
char	progname[MAXPATHLEN];

#ifdef BOOTFILE 			/* default boot file */
char	*bootfile = BOOTFILE;
#else
char	*bootfile = _PATH_BOOT;
#endif

#ifdef DEBUGFILE 			/* default debug output file */
char	*debugfile = DEBUGFILE;
#else
char	*debugfile = _PATH_DEBUG;
#endif

#ifdef PIDFILE 				/* file to store current named PID */
char	*PidFile = PIDFILE;
#else
char	*PidFile = _PATH_PIDFILE;
#endif

#include <locale.h>
#include <nl_types.h>
nl_catd catd;
#include "named_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_NAMED, n, s) 

#ifndef FD_SET
#define	NFDBITS		32
#define	FD_SETSIZE	32
#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif

FILE	*fp;  				/* file descriptor for pid file */

#ifdef DEBUG
FILE	*ddt;
#endif

int	debug = 0;			/* debugging flag */
int	ds;				/* datagram socket */
int	needreload = 0;			/* received SIGHUP, need to reload db */
int	needmaint = 0;			/* need to call ns_maint()*/
int	needzoneload = 0;		/* need to reload secondary zone(s) */
int     needToDoadump = 0;              /* need to dump database */
int     needToChkpt = 0;	        /* need to checkpoint cache */
int	needStatsDump = 0;		/* need to dump statistics */
#ifdef ALLOW_UPDATES
int     needToExit = 0;                 /* need to exit (may need to doadump
					 * first, if database has changed since
					 * it was last dumped/booted). Gets
					 * set by shutdown signal handler
					 *  (onintr)
					 */
#endif /* ALLOW_UPDATES */

int	priming = 0;			/* is cache being primed */

#ifdef SO_RCVBUF
int	rbufsize = 8 * 1024;		/* UDP recive buffer size */
#endif

struct	qstream *streamq = QSTREAM_NULL; /* list of open streams */
struct	qdatagram *datagramq = QDATAGRAM_NULL; /* list of datagram interfaces */
struct	sockaddr_in nsaddr;
struct	timeval tt;
/*
 * We keep a list of favored networks headed by nettab.
 * There are three (possibly empty) parts to this list, in this order:
 *	1. directly attached (sub)nets.
 *	2. logical networks for directly attached subnetted networks.
 *	3. networks from the sort list.
 * The value (*elocal) points at the first entry in the second part of the list,
 * if any, while (*enettab) points at the first entry in the sort list.
 */
struct	netinfo *nettab = NULL;
struct	netinfo **elocal = &nettab;
struct	netinfo **enettab = &nettab;
struct	netinfo netloop;
struct	netinfo *findnetinfo();
u_long	net_mask();
u_short	ns_port;
struct	sockaddr_in from_addr;		/* Source addr of last packet */
int	from_len;			/* Source addr size of last packet */
time_t	boottime, resettime;		/* Used by ns_stats */
static	fd_set	mask;			/* select mask of open descriptors */
static	int vs;				/* listening TCP socket */

char		**Argv = NULL;		/* pointer to argument vector */
char		*LastArg = NULL;	/* end of argv */
char		**Envp = NULL;		/* pointer to Environment variables */

extern int errno;

#if defined(SYSV)
getdtablesize()
{
	return(FD_SETSIZE);
}
#endif /* SYSV */

main(argc, argv, envp)
	int argc;
	char *argv[], *envp[];
{
	register int n, udpcnt;
	register char *arg;
	register struct qstream *sp;
	register struct qdatagram *dqp;
	struct qstream *nextsp;
	int nfds;
	int on = 1;
	int rfd, size;
	u_long lasttime, maxctime;
	char buf[BUFSIZ];
#ifndef SYSV
	struct sigvec vec;
#endif

	fd_set tmpmask;

	struct timeval t, *tp;
	struct qstream *candidate = QSTREAM_NULL;
	extern int onintr(), maint_alarm(), endxfer();
	extern int setdumpflg(), onhup();
	extern int setIncrDbgFlg(), setNoDbgFlg(), sigprof();
	extern int setchkptflg(), setstatsflg();
	extern int loadxfer();
	extern struct qstream *sqadd();
	extern struct qinfo *qhead; 
	extern char Version[];

/*
 * The following code is for SRC support.  The program name is saved
 * for error messages.  fd 0 (stdin) is verified to make sure it is 
 * the SRC socket descriptor and it is a UNIX domain socket.  Once verified,
 * this descriptor is duped to a new descriptor (SRC_FD) so that stdin
 * can be used internally by named.
 */
        int rc, addrsz;
        struct sockaddr srcaddr;
	int src_exists = TRUE;

	setlocale(LC_ALL,"");
	catd = catopen(MF_NAMED, NL_CAT_LOCALE);
	strcpy(progname,argv[0]);
        addrsz = sizeof(srcaddr);
        if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
		src_exists = FALSE;
        }
	if (src_exists)
		(void) dup2(0, SRC_FD);

	ns_port = htons(NAMESERVER_PORT);

	/*
	**  Save start and extent of argv for setproctitle.
	*/

	Argv = argv;
	Envp = envp;
	if (envp == 0 || *envp == 0)
		envp = argv;
	while (*envp)
		envp++;
	LastArg = envp[-1] + strlen(envp[-1]);

	(void) umask(022);
	while (--argc > 0) {
		arg = *++argv;
		if (*arg == '-') {
			while (*++arg)
				switch (*arg) {
				case 'b':
					if (--argc <= 0)
						usage();
					bootfile = *++argv;
					break;

  				case 'd':
 					if (*(arg+1) != 0) {
 					    debug = atoi(arg+1);
					    while (*(arg+1) != 0)
						arg++;
					} else {
						++argv;

						if (*argv != 0) {
						    if (**argv == '-') {
							argv--;
							break;
						    }
						    debug = atoi(*argv);
						    --argc;
						}
					}
					if (debug <= 0)
						debug = 1;
					setdebug(1);
					break;

				case 'p':
					if (--argc <= 0)
						usage();
					ns_port = htons((u_short)atoi(*++argv));
					break;

				default:
					usage();
				}
		} else
			bootfile = *argv;
	}

	if (!debug)
		for (n = getdtablesize() - 1; n > 2; n--) {
		    if (!src_exists)
			(void) close(n);
                    else if (n != SRC_FD)
			(void) close(n);
		}
#ifdef DEBUG
	else {
		fprintf(ddt,"Debug turned ON, Level %d\n",debug);
		fprintf(ddt,"Version = %s\t",Version);
		fprintf(ddt,"bootfile = %s\n",bootfile);
	}		
#endif

#ifdef LOG_DAEMON
	openlog("named", LOG_PID|LOG_CONS|LOG_NDELAY, LOG_DAEMON);
#else
	openlog("named", LOG_PID);
#endif

	/* tuck my process id away */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}
	syslog(LOG_NOTICE, MSGSTR(RESTRT, "restarted\n"));

	if (!src_exists)
                syslog(LOG_NOTICE, MSGSTR(SRC1_NEW,
		  "%s: SRC not found, continuing without SRC support\n"),
		  progname);

	_res.options &= ~(RES_DEFNAMES | RES_DNSRCH | RES_RECURSE);

	nsaddr.sin_family = AF_INET;
	nsaddr.sin_addr.s_addr = INADDR_ANY;
	nsaddr.sin_port = ns_port;

	/*
	** Open stream port.
	*/
	if ((vs = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		syslog(LOG_ERR, MSGSTR(SOCKSTRM, "socket(SOCK_STREAM): %m"));
		exit(1);
	}	
	(void)setsockopt(vs, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	if (bind(vs, (struct sockaddr *)&nsaddr, sizeof(nsaddr))) {
		syslog(LOG_ERR, MSGSTR(BINDERR, "bind(vs, %s[%d]): %m"),
			inet_ntoa(nsaddr.sin_addr), ntohs(nsaddr.sin_port));
		exit(1);
	}
	(void) listen(vs, 5);

	/*
	 * Get list of local addresses and set up datagram sockets.
	 */
	getnetconf();

	/*
	** Initialize and load database.
	*/
	gettime(&tt);
	buildservicelist();
	buildprotolist();
	ns_init(bootfile);
#ifdef DEBUG
	if (debug) {
		fprintf(ddt, "Network and sort list:\n");
		printnetinfo(nettab);
	}
#endif /* DEBUG */

	time(&boottime);
	resettime = boottime;

	(void) signal(SIGHUP, onhup);
#if defined(SYSV)
	(void) signal(SIGCLD, endxfer);
	(void) signal(SIGALRM, maint_alarm);
#else
	bzero((char *)&vec, sizeof(vec));
	vec.sv_handler = maint_alarm;
	vec.sv_mask = sigmask(SIGCHLD);
	(void) sigvec(SIGALRM, &vec, (struct sigvec *)NULL);

	vec.sv_handler = endxfer;
	vec.sv_mask = sigmask(SIGALRM);
	(void) sigvec(SIGCHLD, &vec, (struct sigvec *)NULL);
#endif /* SYSV */
	(void) signal(SIGPIPE, SIG_IGN);
	(void) signal(SIGSYS, sigprof);
	(void) signal(SIGINT, setdumpflg);
	(void) signal(SIGQUIT, setchkptflg);
	(void) signal(SIGIOT, setstatsflg);

#ifdef ALLOW_UPDATES
        /* Catch SIGTERM so we can dump the database upon shutdown if it
           has changed since it was last dumped/booted */
        (void) signal(SIGTERM, onintr);
#endif /* ALLOW_UPDATES */

#if defined(SIGUSR1) && defined(SIGUSR2)
	(void) signal(SIGUSR1, setIncrDbgFlg);
	(void) signal(SIGUSR2, setNoDbgFlg);
#else	SIGUSR1&&SIGUSR2
	(void) signal(SIGEMT, setIncrDbgFlg);
	(void) signal(SIGFPE, setNoDbgFlg);
#endif /* SIGUSR1 && SIGUSR2 */

#ifdef DEBUG
	if (debug) {
		fprintf(ddt,"database initialized\n");
	}
#endif

	t.tv_usec = 0;

	/*
	 * Fork and go into background now that
	 * we've done any slow initialization
	 * and are ready to answer queries.
	 */
	if (!debug) {
/* daemon() not defined in AIX v3.2.5 only in AIX v4.1.
   but additional checking for SRC requires us to do it the old way. (sigh!)
#if defined(BSD) && BSD >= 199006
		daemon(1, 0);
#else
*/
		/* 
		 * fork only if SRC not present.
		 */
		if (!src_exists) {
			if (fork() > 0)
				exit(0);
		}
		n = open(_PATH_DEVNULL, O_RDONLY);
		(void) dup2(n, 0);
		(void) dup2(n, 1);
		(void) dup2(n, 2);
		if (n > 2)
			(void) close(n);
#ifdef SYSV
		if (!src_exists)
			setpgrp();
#else
		{
			struct itimerval ival;

			/*
			 * The open below may hang on pseudo ttys if the person
			 * who starts named logs out before this point.
			 *
			 * needmaint may get set inapropriately if the open
			 * hangs, but all that will happen is we will see that
			 * no maintenance is required.
			 */
			bzero((char *)&ival, sizeof(ival));
			ival.it_value.tv_sec = 120;
			(void) setitimer(ITIMER_REAL, &ival,
				    (struct itimerval *)NULL);
			n = open(_PATH_TTY, O_RDWR);
			ival.it_value.tv_sec = 0;
			(void) setitimer(ITIMER_REAL, &ival,
				    (struct itimerval *)NULL);
			if (n > 0) {
				(void) ioctl(n, TIOCNOTTY, (char *)NULL);
				if (!src_exists)
					setsid();
				(void) close(n);
			}
		}
#endif /* SYSV */
/*
#endif BSD > 199006 */

	}
	/* tuck my process id away again */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}

#ifdef DEBUG
	if (debug)
		fprintf(ddt,"Ready to answer queries.\n");
#endif
	prime_cache();
	nfds = getdtablesize();       /* get the number of file descriptors */
	if (nfds > FD_SETSIZE) {
		nfds = FD_SETSIZE;	/* Bulletproofing */
		syslog(LOG_ERR, MSGSTR(SETFDSZ,
				"Return from getdtablesize() > FD_SETSIZE"));
#ifdef DEBUG
		if (debug)
		      fprintf(ddt,"Return from getdtablesize() > FD_SETSIZE\n");
#endif
	}
	FD_ZERO(&mask);
	FD_SET(vs, &mask);
	if (src_exists)
		FD_SET(SRC_FD, &mask);
	for (dqp = datagramq; dqp != QDATAGRAM_NULL; dqp = dqp->dq_next)
		FD_SET(dqp->dq_dfd, &mask);
	for (;;) {
#ifdef DEBUG
		if (ddt && debug == 0) {
			fprintf(ddt,"Debug turned OFF\n");
			(void) fclose(ddt);
			ddt = 0;
		}
#endif
#ifdef ALLOW_UPDATES
                if (needToExit) {
			struct zoneinfo *zp;
			sigblock(~0);   /*
					 * Block all blockable signals
					 * to ensure a consistant
					 * state during final dump
					 */
#ifdef DEBUG
			if (debug)
				fprintf(ddt, "Received shutdown signal\n");                     
#endif /* DEBUG */
			for (zp = zones; zp < &zones[nzones]; zp++) {
				if (zp->hasChanged)
					zonedump(zp);
                        }
                        exit(0);
                }
#endif /* ALLOW_UPDATES */
		if (needreload) {
			needreload = 0;
			db_reload();
		}
#ifdef STATS
		if (needStatsDump) {
			needStatsDump = 0;
			ns_stats();
		}
#endif /* STATS */
		if (needzoneload) {
			needzoneload = 0;
			loadxfer();
		}
		if (needmaint) {
                        needmaint = 0;
                        ns_maint();
                }
	        if(needToChkpt) {
                        needToChkpt = 0;
                        doachkpt();
	        }
                if(needToDoadump) {
                        needToDoadump = 0;
                        doadump();
                }
		/*
		** Wait until a query arrives
		*/
		if (retryqp != NULL) {
			gettime(&tt);
			t.tv_sec = (long) retryqp->q_time - tt.tv_sec;
			if (t.tv_sec <= 0) {
				retry(retryqp);
				continue;
			}
			tp = &t;
		} else
			tp = NULL;
		tmpmask = mask;
		n = select(nfds, &tmpmask, (fd_set *)NULL, (fd_set *)NULL, tp);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			syslog(LOG_ERR, "select: %m");
#ifdef DEBUG
			if (debug)
				fprintf(ddt,"select error\n");
#endif
			;
		}
		if (n == 0)
			continue;

	        if (src_exists && (FD_ISSET(SRC_FD, &tmpmask))) {
			process_src_packet();
		}
		for (dqp = datagramq; dqp != QDATAGRAM_NULL;
		    dqp = dqp->dq_next) {
		    if (FD_ISSET(dqp->dq_dfd, &tmpmask))
		        for(udpcnt = 0; udpcnt < 25; udpcnt++) {
			    from_len = sizeof(from_addr);
			    if ((n = recvfrom(dqp->dq_dfd, buf, sizeof(buf), 0,
				(struct sockaddr *)&from_addr, &from_len)) < 0)
			    {
				if ((n == -1) && (errno == EWOULDBLOCK))
					break;
				syslog(LOG_WARNING, MSGSTR(RCVERR,
							"recvfrom: %m"));
				break;
			    }
#ifdef STATS
			    stats[S_INPKTS].cnt++;
#endif
#ifdef DEBUG
			    if (debug)
				fprintf(ddt,
				  "\ndatagram from %s port %d, fd %d, len %d\n",
				    inet_ntoa(from_addr.sin_addr),
				    ntohs(from_addr.sin_port), dqp->dq_dfd, n);
			    if (debug >= 10)
				fp_query(buf, ddt);
#endif
			    /*
			     * Consult database to get the answer.
			     */
			    gettime(&tt);
			    ns_req(buf, n, PACKETSZ, QSTREAM_NULL, &from_addr,
				    dqp->dq_dfd);
		        }
		}
		/*
		** Process stream connection
		*/
		if (FD_ISSET(vs, &tmpmask)) {
			from_len = sizeof(from_addr);
			rfd = accept(vs, (struct sockaddr *)&from_addr, &from_len);
			gettime(&tt);
			if (rfd < 0 && errno == EMFILE && streamq != NULL) {
				maxctime = 0;
				candidate = QSTREAM_NULL;
				for (sp = streamq; sp != QSTREAM_NULL;
				   sp = nextsp) {
					nextsp = sp->s_next;
					if (sp->s_refcnt != 0)
					    continue;
					lasttime = tt.tv_sec - sp->s_time;
					if (lasttime >= 900)
					    sqrm(sp);
					else if (lasttime > maxctime) {
					    candidate = sp;
					    maxctime = lasttime;
					}
				}
				rfd = accept(vs, (struct sockaddr *)&from_addr, &from_len);
				if ((rfd < 0) && (errno == EMFILE) &&
				    candidate != QSTREAM_NULL) {
					sqrm(candidate);
					rfd = accept(vs, (struct sockaddr *)&from_addr, &from_len);
				}
			}
			if (rfd < 0) {
				syslog(LOG_WARNING, MSGSTR(ACCERR,
								"accept: %m"));
				continue;
			}
			(void) fcntl(rfd, F_SETFL, FNDELAY);
			(void) setsockopt(rfd, SOL_SOCKET, SO_KEEPALIVE,
				(char *)&on, sizeof(on));
			if ((sp = sqadd()) == QSTREAM_NULL) {
				(void) close(rfd);
				continue;
			}
			sp->s_rfd = rfd;	/* stream file descriptor */
			sp->s_size = -1;	/* amount of data to receive */
			gettime(&tt);
			sp->s_time = tt.tv_sec;	/* last transaction time */
			sp->s_from = from_addr;	/* address to respond to */
			sp->s_bufp = (char *)&sp->s_tempsize;
			FD_SET(rfd, &mask);
			FD_SET(rfd, &tmpmask);
#ifdef DEBUG
			if (debug) {
				fprintf(ddt,
				   "\nTCP connection from %s port %d (fd %d)\n",
					inet_ntoa(sp->s_from.sin_addr),
					ntohs(sp->s_from.sin_port), rfd);
			}
#endif
		}
#ifdef DEBUG
		if (debug > 2 && streamq)
			fprintf(ddt,"streamq  = x%x\n",streamq);
#endif
		if (streamq != NULL) {
			for (sp = streamq; sp != QSTREAM_NULL; sp = nextsp) {
			    nextsp = sp->s_next;
			    if (FD_ISSET(sp->s_rfd, &tmpmask)) {
#ifdef DEBUG
				if (debug > 5) {
				    fprintf(ddt,
					"sp x%x rfd %d size %d time %d ",
					sp, sp->s_rfd, sp->s_size,
					sp->s_time );
				    fprintf(ddt," next x%x \n", sp->s_next );
				    fprintf(ddt,"\tbufsize %d",sp->s_bufsize);
				    fprintf(ddt," buf x%x ",sp->s_buf);
				    fprintf(ddt," bufp x%x\n",sp->s_bufp);
				}
#endif /* DEBUG */
			    if (sp->s_size < 0) {
			        size = sizeof(u_short) -
				   (sp->s_bufp - (char *)&sp->s_tempsize);
			        while (size > 0 &&
			           (n = read(sp->s_rfd, sp->s_bufp, size)) > 0){
					    sp->s_bufp += n;
					    size -= n;
			        }
			        if ((n == -1) && (errno == EWOULDBLOCK))
					    continue;
			        if (n <= 0) {
					    sqrm(sp);
					    continue;
			        }
			        if ((sp->s_bufp - (char *)&sp->s_tempsize) ==
					sizeof(u_short)) {
					sp->s_size = htons(sp->s_tempsize);
					if (sp->s_bufsize == 0) {
					    if ( (sp->s_buf = malloc(BUFSIZ))
						== NULL) {
						    sp->s_buf = buf;
						    sp->s_size  = sizeof(buf);
					    } else {
						    sp->s_bufsize = BUFSIZ;
					    }
					}
					if (sp->s_size > sp->s_bufsize &&
					  sp->s_bufsize != 0) {
					    if ((sp->s_buf = realloc(
						(char *)sp->s_buf,
						(unsigned)sp->s_size)) == NULL){
						    sp->s_buf = buf;
						    sp->s_bufsize = 0;
						    sp->s_size  = sizeof(buf);
					   } else {
						    sp->s_bufsize = sp->s_size;
					   }
					}
					sp->s_bufp = sp->s_buf;	
				    }
			    }
			    gettime(&tt);
			    sp->s_time = tt.tv_sec;
			    while (sp->s_size > 0 &&
			      (n = read(sp->s_rfd, sp->s_bufp, sp->s_size)) > 0)
			    {
				    sp->s_bufp += n;
				    sp->s_size -= n;
			    }
			    /*
			     * we don't have enough memory for the query.
			     * if we have a query id, then we will send an
			     * error back to the user.
			     */
			    if (sp->s_bufsize == 0 &&
				(sp->s_bufp - sp->s_buf > sizeof(u_short))) {
				    HEADER *hp;

				    hp = (HEADER *)sp->s_buf;
				    hp->qr = 1;
				    hp->ra = 1;
				    hp->ancount = 0;
				    hp->qdcount = 0;
				    hp->nscount = 0;
				    hp->arcount = 0;
				    hp->rcode = SERVFAIL;
				    (void) writemsg(sp->s_rfd, sp->s_buf,
					sizeof(HEADER));
				    continue;
			    }
			    if ((n == -1) && (errno == EWOULDBLOCK))
				    continue;
			    if (n <= 0) {
				    sqrm(sp);
				    continue;
			    }
			    /*
			     * Consult database to get the answer.
			     */
			    if (sp->s_size == 0) {
				    sq_query(sp);
				    ns_req(sp->s_buf,
					sp->s_bufp - sp->s_buf,
					sp->s_bufsize, sp,
					&sp->s_from, -1);
				    sp->s_bufp = (char *)&sp->s_tempsize;
				    sp->s_size = -1;
				    continue;
			    }
			}
		    }
		}
	}
	/* NOTREACHED */
}

usage()
{
fprintf(stderr, MSGSTR(USAGE,
			"Usage: named [-d #] [-p port] [{-b} bootfile]\n"));
	exit(1);
}

getnetconf()
{
	register struct netinfo *ntp;
	struct netinfo *ontp;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	struct	qdatagram *dqp;
	static int first = 1;
	char buf[BUFSIZ], *cp, *cplim;
	u_long nm;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(vs, SIOCGIFCONF, (char *)&ifc) < 0) {
		syslog(LOG_ERR, MSGSTR(IFCONF,
					"get interface configuration: %m"));
		exit(1);
	}
	ntp = NULL;
#ifdef AF_LINK
#define max(a, b) (a > b ? a : b)
#define size(p)	max((p).sa_len, sizeof(p))
#else
#define size(p) (sizeof (p))
#endif
	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim;
			cp += sizeof (ifr->ifr_name) + size(ifr->ifr_addr)) {
#undef size
		ifr = (struct ifreq *)cp;
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		ifreq = *ifr;
		/*
		 * Don't test IFF_UP, packets may still be received at this
		 * address if any other interface is up.
		 */
		if (ioctl(vs, SIOCGIFADDR, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, MSGSTR(IFADDR,
						"get interface addr: %m"));
			continue;
		}
		/* build datagram queue */
		/* 
		 * look for an already existing source interface address.
		 * This happens mostly when reinitializing.  Also, if
		 * the machine has multiple point to point interfaces, then 
		 * the local address may appear more than once.
		 */		   
		for (dqp=datagramq; dqp != NULL; dqp = dqp->dq_next)
		    if (((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr.s_addr
		        == dqp->dq_addr.s_addr) {
#ifdef DEBUG
			  if (debug)
			      fprintf(ddt, "dup interface address %s on %s\n",
				    inet_ntoa(dqp->dq_addr), ifreq.ifr_name);
#endif		      
			  break;
		    }
		if (dqp != NULL)
			continue;

		if ((dqp = (struct qdatagram *)calloc(1,
		    sizeof(struct qdatagram))) == NULL) {
#ifdef DEBUG
			if (debug >= 5)
				fprintf(ddt,"getnetconf: malloc error\n");
#endif
			syslog(LOG_ERR, MSGSTR(MMEMERR1,
						"getnetconf: Out Of Memory"));
			exit(12);
		}
		dqp->dq_next = datagramq;
		datagramq = dqp;
		dqp->dq_addr = ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
		opensocket(dqp);

		/*
		 * Add interface to list of directly-attached (sub)nets
		 * for use in sorting addresses.
		 */
		if (ntp == NULL)
			ntp = (struct netinfo *)malloc(sizeof(struct netinfo));
		ntp->my_addr = 
		    ((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
#ifdef SIOCGIFNETMASK
		if (ioctl(vs, SIOCGIFNETMASK, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, MSGSTR(NETMSK, "get netmask: %m"));
			ntp->mask = net_mask(ntp->my_addr);
		} else
			ntp->mask = ((struct sockaddr_in *)
			    &ifreq.ifr_addr)->sin_addr.s_addr;
#else
		/* 4.2 does not support subnets */
		ntp->mask = net_mask(ntp->my_addr);
#endif
		if (ioctl(vs, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			syslog(LOG_ERR, MSGSTR(IFFLGS,
						"get interface flags: %m"));
			continue;
		}
#ifdef IFF_LOOPBACK
		if (ifreq.ifr_flags & IFF_LOOPBACK)
#else
		/* test against 127.0.0.1 (yuck!!) */
		if (ntp->my_addr.s_addr == htonl(0x7F000001))
#endif
		{
		    if (netloop.my_addr.s_addr == 0) {
			netloop.my_addr = ntp->my_addr;
			netloop.mask = 0xffffffff;
			netloop.net = ntp->my_addr.s_addr;
#ifdef DEBUG
			if (debug) 
			    fprintf(ddt,"loopback address: x%lx\n",
				    netloop.my_addr.s_addr);
#endif /* DEBUG */
		    }
		    continue;
		} else if ((ifreq.ifr_flags & IFF_POINTOPOINT)) {
			if (ioctl(vs, SIOCGIFDSTADDR, (char *)&ifreq) < 0) {
		    	    syslog(LOG_ERR, MSGSTR(DESTADDR,
							"get dst addr: %m"));
		            continue;
			}
			ntp->mask = 0xffffffff;
			ntp->net = ((struct sockaddr_in *)
		    	    &ifreq.ifr_addr)->sin_addr.s_addr;
		} else {
			ntp->net = ntp->mask & ntp->my_addr.s_addr;
		}
		/*
		 * Place on end of list of locally-attached (sub)nets,
		 * but before logical nets for subnetted nets.
		 */
		ntp->next = *elocal;
		*elocal = ntp;
		if (elocal == enettab)
		    enettab = &ntp->next;
		elocal = &ntp->next;
		ntp = NULL;
	}
	if (ntp)
		(void) free((char *)ntp);

	/*
	 * Create separate qdatagram structure for socket
	 * wildcard address.
	 */
	if (first) {
		if ((dqp = (struct qdatagram *)calloc(1, sizeof(*dqp))) == NULL) {
#ifdef DEBUG
			if (debug >= 5)
				fprintf(ddt,"getnetconf: malloc error\n");
#endif
			syslog(LOG_ERR, MSGSTR(MMEMERR1,
						"getnetconf: Out Of Memory"));
			exit(12);
		}
		dqp->dq_next = datagramq;
		datagramq = dqp;
		dqp->dq_addr.s_addr = INADDR_ANY;
		opensocket(dqp);
		ds = dqp->dq_dfd;	/* used externally */
	}

	/*
	 * Compute logical networks to which we're connected
	 * based on attached subnets;
	 * used for sorting based on network configuration.
	 */
	for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
		nm = net_mask(ntp->my_addr);
		if (nm != ntp->mask) {
			if (findnetinfo(ntp->my_addr))
				continue;
			ontp = (struct netinfo *)malloc(sizeof(struct netinfo));
			if (ontp == NULL) {
#ifdef DEBUG
				if (debug >= 5)
				    fprintf(ddt,"getnetconf: malloc error\n");
#endif
				syslog(LOG_ERR, MSGSTR(MMEMERR1,
						"getnetconf: Out Of Memory"));
				exit(12);
			}
			ontp->my_addr = ntp->my_addr;
			ontp->mask = nm;
			ontp->net = ontp->my_addr.s_addr & nm;
			ontp->next = *enettab;
			*enettab = ontp;
			enettab = &ontp->next;
		}
	}
	first = 0;
}

/*
 * Find netinfo structure for logical network implied by address "addr",
 * if it's on list of local/favored networks.
 */
struct netinfo *
findnetinfo(addr)
	struct in_addr addr;
{
	register struct netinfo *ntp;
	u_long net, mask;

	mask = net_mask(addr);
	net = addr.s_addr & mask;
	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->net == net && ntp->mask == mask)
			return (ntp);
	return ((struct netinfo *) NULL);
}

#ifdef DEBUG
printnetinfo(ntp)
	register struct netinfo *ntp;
{
	for ( ; ntp != NULL; ntp = ntp->next) {
		fprintf(ddt,"net x%lx mask x%lx", ntp->net, ntp->mask);
		fprintf(ddt," my_addr x%lx", ntp->my_addr.s_addr);
		fprintf(ddt," %s\n", inet_ntoa(ntp->my_addr));
	}
}
#endif

opensocket(dqp)
	register struct qdatagram *dqp;
{
	int on = 1;

	/*
	 * Open datagram sockets bound to interface address.
	 */
	if ((dqp->dq_dfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		syslog(LOG_ERR, MSGSTR(DGRAMSOCK, "socket(SOCK_DGRAM): %m"));
		exit(1);
	}	
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"dqp->dq_addr %s d_dfd %d\n",
		    inet_ntoa(dqp->dq_addr), dqp->dq_dfd);
#endif /* DEBUG */
	(void)setsockopt(dqp->dq_dfd, SOL_SOCKET, SO_REUSEADDR,
	    (char *)&on, sizeof(on));
#ifdef SO_RCVBUF
	(void)setsockopt(dqp->dq_dfd, SOL_SOCKET, SO_RCVBUF,
	    (char *)&rbufsize, sizeof(rbufsize));
#endif /* SO_RCVBUF */
	(void) fcntl(dqp->dq_dfd, F_SETFL, FNDELAY);
	/*
	 *   NOTE: Some versions of SunOS have problems with the following
	 *   call to bind.  Bind still seems to function on these systems
	 *   if you comment out the exit inside the if.  This may cause
	 *   Suns with multiple interfaces to reply strangely.
	 */
	nsaddr.sin_addr = dqp->dq_addr;
	if (bind(dqp->dq_dfd, (struct sockaddr *)&nsaddr, sizeof(nsaddr))) {
		syslog(LOG_ERR, MSGSTR(DGBIND, "bind(dfd %d, %s[%d]): %m"),
			dqp->dq_dfd, inet_ntoa(nsaddr.sin_addr),
			ntohs(nsaddr.sin_port));
#if !defined(sun)
		exit(1);
#endif
	}
}

/*
** Set flag saying to reload database upon receiving SIGHUP.
** Must make sure that someone isn't walking through a data
** structure at the time.
*/

onhup()
{
#if defined(SYSV)
	(void)signal(SIGHUP, onhup);
#endif /* SYSV */
	needreload = 1;
}

/*
** Set flag saying to call ns_maint()
** Must make sure that someone isn't walking through a data
** structure at the time.
*/

maint_alarm()
{
#if defined(SYSV)
	(void)signal(SIGALRM, maint_alarm);
#endif /* SYSV */
	needmaint = 1;
 }


#ifdef ALLOW_UPDATES
/*
 * Signal handler to schedule shutdown.  Just set flag, to ensure a consistent
 * state during dump.
 */
onintr()
{
        needToExit = 1;
}
#endif /* ALLOW_UPDATES */

/*
 * Signal handler to schedule a data base dump.  Do this instead of dumping the
 * data base immediately, to avoid seeing it in a possibly inconsistent state
 * (due to updates), and to avoid long disk I/O delays at signal-handler
 * level
 */
setdumpflg()
{
#if defined(SYSV)
	(void)signal(SIGINT, setdumpflg);
#endif /* SYSV */
        needToDoadump = 1;
}

/*
** Turn on or off debuging by open or closeing the debug file
*/

setdebug(code)
int code;
{
#if defined(lint) && !defined(DEBUG)
	code = code;
#endif
#ifdef DEBUG

	if (code) {
		ddt = freopen(debugfile, "w+", stderr);
		if ( ddt == NULL) {
			syslog(LOG_WARNING, "can't open debug file %s: %m",
			    debugfile);
			debug = 0;
		} else {
#if defined(SYSV)
			setvbuf(ddt, NULL, _IOLBF, BUFSIZ);
#else
			setlinebuf(ddt);
#endif
			(void) fcntl(fileno(ddt), F_SETFL, FAPPEND);
		}
	} else
		debug = 0;
		/* delay closing ddt, we might interrupt someone */
#endif
}

/*
** Catch a special signal and set debug level.
**
**  If debuging is off then turn on debuging else increment the level.
**
** Handy for looking in on long running name servers.
*/

setIncrDbgFlg()
{
#if defined(SYSV)
	(void)signal(SIGUSR1, setIncrDbgFlg);
#endif /* SYSV */
#ifdef DEBUG
	if (debug == 0) {
		debug++;
		setdebug(1);
	}
	else {
		debug++;
	}
	fprintf(ddt,"Debug turned ON, Level %d\n",debug);
#endif
}

/*
** Catch a special signal to turn off debugging
*/

setNoDbgFlg()
{
#if defined(SYSV)
	(void)signal(SIGUSR2, setNoDbgFlg);
#endif /* SYSV */
	setdebug(0);
}

/*
** Set flag for statistics dump
*/
setstatsflg()
{
#if defined(SYSV)
	(void)signal(SIGIOT, setstatsflg);
#endif /* SYSV */
	needStatsDump = 1;
}

setchkptflg()
{
#if defined(SYSV)
	(void)signal(SIGQUIT, setchkptflg);
#endif /* SYSV */
	needToChkpt = 1;
}

/*
** Catch a special signal SIGSYS
**
**  this is setup to fork and exit to drop to /usr/tmp/gmon.out
**   and keep the server running
*/

sigprof()
{
#if defined(SYSV)
	(void)signal(SIGSYS, sigprof);
#endif /* SYSV */
#ifdef DEBUG
	if (debug)
		fprintf(ddt,"sigprof()\n");
#endif
	if ( fork() == 0)
	{
		(void) chdir(_PATH_TMPDIR);
		exit(1);
	}
}

/*
** Routines for managing stream queue
*/

struct qstream *
sqadd()
{
	register struct qstream *sqp;

	if ((sqp = (struct qstream *)calloc(1, sizeof(struct qstream)))
	    == NULL ) {
#ifdef DEBUG
		if (debug >= 5)
			fprintf(ddt,"sqadd: malloc error\n");
#endif
		syslog(LOG_ERR, MSGSTR(SQADDMEM, "sqadd: Out Of Memory"));
		return(QSTREAM_NULL);
	}
#ifdef DEBUG
	if (debug > 3)
		fprintf(ddt,"sqadd(x%x)\n", sqp);
#endif

	sqp->s_next = streamq;
	streamq = sqp;
	return(sqp);
}

/*
 * Remove stream queue structure.
 * No current queries may refer to this stream when it is removed.
 */
sqrm(qp)
	register struct qstream *qp;
{
	register struct qstream *qsp;

#ifdef DEBUG
	if (debug > 1) {
		fprintf(ddt,"sqrm(%#x, %d ) rfcnt=%d\n",
		    qp, qp->s_rfd, qp->s_refcnt);
	}
#endif

	if (qp->s_bufsize != 0)
		(void) free(qp->s_buf);
	FD_CLR(qp->s_rfd, &mask);
	(void) close(qp->s_rfd);
	if (qp == streamq) {
		streamq = qp->s_next;
	} else {
		for (qsp = streamq; qsp->s_next != qp; qsp = qsp->s_next)
			;
		qsp->s_next = qp->s_next;
	}
	(void)free((char *)qp);
}

sqflush()
{
	register struct qstream *sp, *spnext;

	for (sp = streamq; sp != QSTREAM_NULL; sp = spnext) {
		spnext = sp->s_next;
		sqrm(sp);
	}
}

/*
 * Initiate query on stream;
 * mark as referenced and stop selecting for input.
 */
sq_query(sp)
	register struct qstream *sp;
{
	sp->s_refcnt++;
#ifdef DEBUG
	if (debug > 1) {
		fprintf(ddt,"sq_query(%#x, %d ) rfcnt=%d\n",
		    sp, sp->s_rfd, sp->s_refcnt);
	}
#endif
	FD_CLR(sp->s_rfd, &mask);
}

/*
 * Note that the current request on a stream has completed,
 * and that we should continue looking for requests on the stream.
 */
sq_done(sp)
	register struct qstream *sp;
{

	sp->s_refcnt = 0;
	sp->s_time = tt.tv_sec;
#ifdef DEBUG
	if (debug > 1) {
		fprintf(ddt,"sq_done(%#x, %d ) rfcnt=%d, s_time=%d\n",
		    sp, sp->s_rfd, sp->s_refcnt, sp->s_time);
	}
#endif
	FD_SET(sp->s_rfd, &mask);
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
	else {
		syslog(LOG_DEBUG, MSGSTR(PEERNMERR, "getpeername: %m"));
		(void) sprintf(buf, "-%s", a);
	}
	(void) strncpy(cp, buf, LastArg - cp);
	cp += strlen(cp);
	while (cp < LastArg)
		*cp++ = ' ';
}

u_long
net_mask(in)
struct in_addr in;
{
	register u_long i = ntohl(in.s_addr);

	if (IN_CLASSA(i))
		return (htonl(IN_CLASSA_NET));
	else if (IN_CLASSB(i))
		return (htonl(IN_CLASSB_NET));
	else
		return (htonl(IN_CLASSC_NET));
}

gettime(ttp)
struct timeval *ttp;
{
	if (gettimeofday(ttp, (struct timezone *)0) < 0)
		syslog(LOG_ERR, MSGSTR(TODERR, "gettimeofday failed: %m"));
	return;
}

/*
 * SRC packet processing - .
 */
int
dosrcpacket(msgno, subsys, txt, len)
        int msgno;
        char *subsys;
        char *txt;
        int len;
{
        struct srcrep reply;

        reply.svrreply.rtncode = msgno;
        strcpy(reply.svrreply.objname, "named");
        strcpy(reply.svrreply.rtnmsg, txt);
        srchdr = srcrrqs(&srcpacket);
        srcsrpy(srchdr, &reply, len, cont);
}

process_src_packet()
{
	int onoff;
	char buf[BUFSIZ], *cp;
	int x;
	FILE *fp;
        int rc, addrsz;
        struct sockaddr srcaddr;

	cont = END;
	addrsz = sizeof(srcaddr);

	rc = recvfrom(SRC_FD, &srcpacket, SRCMSG, 0, &srcaddr, &addrsz);
	if (rc < 0) {
	    syslog(LOG_ERR,MSGSTR(SRC2_NEW,"%s: '%d' recvfrom\n"),
							progname,errno);
	    exit(1);
	}

	/* process the request packet */
	switch(srcpacket.subreq.action) {
		case START:
		    dosrcpacket(SRC_SUBMSG, progname, MSGSTR(SRC3,
			"ERROR: named does not support this option.\n"),
							sizeof(struct srcrep));
		    break;
		case STOP:
		    if (srcpacket.subreq.object == SUBSYSTEM) {
			    dosrcpacket(SRC_OK, progname, NULL,
							sizeof(struct srcrep));
			    if (srcpacket.subreq.parm1 == NORMAL) {
#ifdef ALLOW_UPDATES
				    needToExit = 1;
#else
				    exit(0);
#endif /* ALLOW_UPDATES */
			    } else
				    exit(0);
		    } else
			    dosrcpacket(SRC_SUBMSG, progname, MSGSTR(SRC3,
				"ERROR: named does not support this option.\n"),
							sizeof(struct srcrep));
		    break;
		case REFRESH:
		    needreload = 1;
		    dosrcpacket(SRC_OK, progname, NULL, sizeof(struct srcrep));
		    break;
		case STATUS:
		    if (srcpacket.subreq.object == SUBSYSTEM) {
			    srcreplen = srcstatsize = sizeof(struct srchdr)
					+ (sizeof(struct statcode)*SRCSTATNDX);
			    srcstatus = (struct statrep *)malloc(srcstatsize);
			    memset(srcstatus, '\0', srcreplen);
			    cont = NEWREQUEST;
			    srcstat("","", getpid(), &srcstatsize, srcstatus, &cont);
			    srchdr = srcrrqs(&srcpacket);
			    sprintf(srcstatus->statcode[3].objname,
					    "%-12s",MSGSTR(SRC4_NEW,"Debug"));
			    if (debug) 
				    sprintf(srcstatus->statcode[3].objtext,
					    " %s", MSGSTR(SRC5_NEW,"Active"));
			    else
				    sprintf(srcstatus->statcode[3].objtext,
					    " %s", MSGSTR(SRC6_NEW,"Inactive"));
			    if ((fp = fopen(bootfile, "r")) == NULL) {
				    syslog(LOG_ERR, MSGSTR(BOOTFILERR,
						"%s: %m"), bootfile);
				    exit(1);
			    }
			    sprintf(srcstatus->statcode[5].objname,
					    "%-12s",MSGSTR(SRC8_NEW,"Type"));
			    sprintf(srcstatus->statcode[5].objtext,
					    "%-30s%-30s", MSGSTR(SRC9_NEW,"Domain"),
			    MSGSTR(SRC10_NEW,"Source File or Host"));
			    srcsrpy(srchdr, srcstatus, srcreplen,
								STATCONTINUED);
			    memset(srcstatus, '\0', srcreplen);
			    x = 0;
			    while (fgets(buf, sizeof(buf), fp) > 0) {
				    if (buf[0] == '\n' || buf[0] == ';')
					    continue;
				    if ((cp = strchr(buf,'\n')) != NULL)
					    *cp = '\0';
				    cp = buf;
				    cp += strspn(cp, DELIM);
				    cp += strcspn(cp, DELIM);
				    *cp++ = '\0';
				    cp += strspn(cp, DELIM);
				    if (x < SRCSTATNDX) {
					    sprintf(srcstatus->statcode[x].objname," %-12s",buf);
					    sprintf(srcstatus->statcode[x].objtext,"%s",cp);
					    x++;
				    } else {
					    srcsrpy(srchdr, srcstatus,
						srcreplen, STATCONTINUED);
					    x = 0;
					    memset(srcstatus, '\0', srcreplen);
				    }
			    }
			    (void) fclose(fp);
			    if (x > 0)
				    srcsrpy(srchdr, srcstatus, srcreplen,
								STATCONTINUED);
			    srcsrpy(srchdr, srcstatus, sizeof(struct srchdr),
									END);
			    free(srcstatus);
		    } else
			    dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,
				"ERROR: named does not support this option.\n"),
							sizeof(struct srcrep));
		    break;
		case TRACE:
		    if (srcpacket.subreq.object == SUBSYSTEM) { 
			    onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
			    if (setsockopt(vs, SOL_SOCKET, SO_DEBUG, &onoff,
							sizeof (onoff)) < 0) {
				    syslog(LOG_ERR, MSGSTR(SRC7,
						   "setsockopt SO_DEBUG: %m"));
				    close(vs);
#ifdef ALLOW_UPDATES
				    needToExit = 1;
#else
				    exit(1);
#endif /* ALLOW_UPDATES */
			    }
			    dosrcpacket(SRC_OK, progname, NULL,
							sizeof(struct srcrep)); 
		    } else
			    dosrcpacket(SRC_SUBMSG, progname, MSGSTR(SRC3,
				"ERROR: named does not support this option.\n"),
							sizeof(struct srcrep));
		    break;
		default:
		    dosrcpacket(SRC_SUBICMD, progname, NULL,
							sizeof(struct srcrep));
		    break;

	}
}
