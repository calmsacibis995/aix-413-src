static char sccsid[] = "@(#)14	1.26  src/tcpip/usr/sbin/routed/main.c, tcprouting, tcpip41J, 9511A_all 3/3/95 10:06:41";
/* 
 * COMPONENT_NAME: TCPIP main.c
 * 
 * FUNCTIONS: MSGSTR, Mmain, dosrcpacket, getsocket, process, 
 *            timevaladd, timevalsub 
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
 * Copyright (c) 1983, 1988 Regents of the University of California.
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
" Copyright (c) 1983, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "@(#)main.c	5.21 (Berkeley) 6/29/90";
#endif  not lint */

#include <nl_types.h>
nl_catd catd;
#include "routed_msg.h" 
#define MSGSTR(n,s) catgets(catd, MS_ROUTED, n, s) 

/*
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <sys/ioctl.h>
#include <sys/file.h>

#include <net/if.h>

#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include "pathnames.h"

/*
 * The following includes and declerations are for SRC support.
 */
#include <spc.h>
#include "libffdb.h"
#include "libsrc.h"
static  struct srcreq srcpacket;
int     cont;
struct  srchdr *srchdr;
struct  statrep *srcstatus;
short   srcstatsize;
int     srcreplen;
char	progname[128];


int	supplier = -1;		/* process should supply updates */
int	gateway = 0;		/* 1 if we are a gateway to parts beyond */
int	debug = 0;
int	bufspace = 127*1024;	/* max. input buffer size to request */

struct	rip *msg = (struct rip *)packet;
int	hup(), rtdeleteall(), sigtrace();

/*
 * The following is the maximum time to hold on to a route that needs to
 * needs to be deleted via garbage collection.
 */

int	garbage_time = GARBAGE_TIME;

#include <locale.h>

main(argc, argv)
	int argc;
	char *argv[];
{
	int n, cc, nfd, omask, tflags = 0, errflag = 0;
	struct sockaddr from;
	struct timeval *tvp, waittime;
	struct itimerval itval;
	struct itimerval otval;
	fd_set ibits;
	u_char retry;
	extern int optind;
	extern char *optarg;

/* 
 * The following declerations and code are for SRC support.  Stdin is 
 * checked to make sure it is the SRC socket.  If so, the file descriptor for 
 * stdin (0) is duped to a new file descriptor so that 0 can be used 
 * internally by routed.
 */
        int rc, i, addrsz;
        struct sockaddr srcaddr;
	int srctraceflag = 0;
	int src_exists = TRUE;

	setlocale(LC_ALL,"");
	catd = catopen(MF_ROUTED, NL_CAT_LOCALE);
	strcpy(progname,argv[0]);
        addrsz = sizeof(srcaddr);
        if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
		src_exists = FALSE;
        }
	if (src_exists)
        	(void) dup2(0, SRC_FD);

	
	argv0 = argv;
#if _BSD >= 43
	openlog("routed", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	setlogmask(LOG_UPTO(LOG_WARNING));
#else
	openlog("routed", LOG_PID);
#define LOG_UPTO(x) (x)
#define setlogmask(x) (x)
#endif
	sp = getservbyname("router", "udp");
	if (sp == NULL) {
		fprintf(stderr, MSGSTR(UNKSERV, "routed: router/udp: unknown service\n")); /*MSG*/
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = sp->s_port;
	s = getsocket(AF_INET, SOCK_DGRAM, &addr);
	if (s < 0)
		exit(1);

	while ((rc = getopt(argc, argv, "sqtdgh:")) != EOF) {
		switch (rc) {
		case 's':
			supplier = 1;
			break;
		case 'q':
			supplier = 0;
			break;
		case 't':
			tflags++;
			srctraceflag++;
			setlogmask(LOG_UPTO(LOG_DEBUG));
			break;
		case 'd':
			debug++;
			setlogmask(LOG_UPTO(LOG_DEBUG));
			break;
		case 'g':
			gateway = 1;
			break;
		case 'h':
			garbage_time = atoi(optarg);
			if (garbage_time == 0) {
				fprintf(stderr,
				    MSGSTR(INVALID_HOLD,
				     "%s: invalid route hold down time: %s\n"),
							progname, optarg);
				errflag = 1;
			}
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (errflag) {
		fprintf(stderr,
			MSGSTR(USAGE_NEW, "usage: routed [ -s ] [ -q ] [ -t ] [ -d ] [ -g ] [ -h hold_time ]\n")); /*MSG*/
		exit(1);
	}

	if (debug == 0 && tflags == 0) {
		int t;

/* 
 * Fork only if SRC is not present. Also, file descriptor 13 (SRC_FD)
 * can't be closed because SRC packets are processed via this descriptor.
 */
		if (!src_exists)
 			if (fork())
 				exit(0);
		for (t = 0; t < 20; t++) {
			if (!src_exists) {
				if (t != s)
					(void) close(t);
			} else if (t != s && t != SRC_FD)
				(void) close(t);
		}
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		t = open("/dev/tty", 2);
		if (t >= 0) {
			ioctl(t, TIOCNOTTY, (char *)0);
			(void) close(t);
		}
	}
	/*
	 * Any extra argument is considered
	 * a tracing log file.
	 */
	if (tflags > 0)
		traceon(*argv);
	while (tflags-- > 0)
		bumploglevel();

	(void) gettimeofday(&now, (struct timezone *)NULL);
	/*
	 * Collect an initial view of the world by
	 * checking the interface configuration and the gateway kludge
	 * file.  Then, send a request packet on all
	 * directly connected networks to find out what
	 * everyone else thinks.
	 */
	rtinit();
	ifinit();
	gwkludge();
	if (gateway > 0)
		rtdefault();
	if (supplier < 0)
		supplier = 0;
	msg->rip_cmd = RIPCMD_REQUEST;
	msg->rip_vers = RIPVERSION;
	if (sizeof(msg->rip_nets[0].rip_dst.sa_family) > 1)	/* XXX */
		msg->rip_nets[0].rip_dst.sa_family = htons((u_short)AF_UNSPEC);
	else
		msg->rip_nets[0].rip_dst.sa_family = AF_UNSPEC;
	msg->rip_nets[0].rip_metric = htonl((u_long)HOPCNT_INFINITY);
	toall(Sendmsg, RTS_CHANGED, (struct interface *) NULL);
	signal(SIGALRM, timer);
	signal(SIGHUP, hup);
	signal(SIGTERM, hup);
	signal(SIGINT, rtdeleteall);
	signal(SIGUSR1, sigtrace);
	signal(SIGUSR2, sigtrace);
	itval.it_interval.tv_sec = TIMER_RATE;
	itval.it_value.tv_sec = TIMER_RATE;
	itval.it_interval.tv_usec = 0;
	itval.it_value.tv_usec = 0;
	otval.it_interval.tv_sec = 0;
	otval.it_value.tv_sec = 0;
	otval.it_interval.tv_usec = 0;
	otval.it_value.tv_usec = 0;
	srandom(getpid());
	if (setitimer(ITIMER_REAL, &itval, &otval) < 0)
		syslog(LOG_ERR, MSGSTR(SETTIME,"setitimer: %m\n"));

	if (!src_exists)
                syslog(LOG_NOTICE, MSGSTR(SRC1,
		  "%s: ERROR: SRC not found, continuing without SRC support\n"),
		  progname);

	FD_ZERO(&ibits);
	nfd = MAX(s, SRC_FD) + 1;			/* 1 + max(fd's) */

	for (;;) {
		cont=END;
		addrsz=sizeof(srcaddr);
	
		FD_SET(s, &ibits);
		if (src_exists) 
			FD_SET(SRC_FD, &ibits);

		/*
		 * If we need a dynamic update that was held off,
		 * needupdate will be set, and nextbcast is the time
		 * by which we want select to return.  Compute time
		 * until dynamic update should be sent, and select only
		 * until then.  If we have already passed nextbcast,
		 * just poll.
		 */
		if (needupdate) {
			waittime = nextbcast;
			timevalsub(&waittime, &now);
			if (waittime.tv_sec < 0) {
				waittime.tv_sec = 0;
				waittime.tv_usec = 0;
			}
			if (traceactions)
				fprintf(ftrace,
				 MSGSTR(SELECTUDT,"select until dynamic update %d/%d sec/usec\n"),
				    waittime.tv_sec, waittime.tv_usec);
			tvp = &waittime;
		} else
			tvp = (struct timeval *)NULL;
		n = select(nfd, &ibits, 0, 0, tvp);
		if (n <= 0) {
			/*
			 * Need delayed dynamic update if select returned
			 * nothing and we timed out.  Otherwise, ignore
			 * errors (e.g. EINTR).
			 */
			if (n < 0) {
				if (errno == EINTR)
					continue;
				syslog(LOG_ERR, MSGSTR(SOCKERR, "socket: %m")); /*MSG*/
			}
			omask = sigblock(sigmask(SIGALRM));
			if (n == 0 && needupdate) {
				if (traceactions)
					fprintf(ftrace,
					MSGSTR(SENDDELAY,"send delayed dynamic update\n"));
				(void) gettimeofday(&now,
					    (struct timezone *)NULL);
				toall(supply, RTS_CHANGED,
				    (struct interface *)NULL);
				lastbcast = now;
				needupdate = 0;
				nextbcast.tv_sec = 0;
			}
			sigsetmask(omask);
			continue;
		}
		(void) gettimeofday(&now, (struct timezone *)NULL);
		omask = sigblock(sigmask(SIGALRM));
#ifdef doesntwork
/*
printf(MSGSTR(FORMAT,"s %d, ibits %x index %d, mod %d, sh %x, or %x &ibits %x\n"),
	s,
	ibits.fds_bits[0],
	(s)/(sizeof(fd_mask) * 8),
	((s) % (sizeof(fd_mask) * 8)),
	(1 << ((s) % (sizeof(fd_mask) * 8))),
	ibits.fds_bits[(s)/(sizeof(fd_mask) * 8)] & (1 << ((s) % (sizeof(fd_mask) * 8))),
	&ibits
	);
*/
#endif
		if (FD_ISSET(s, &ibits))
			process(s);
		/* handle ICMP redirects */
		sigsetmask(omask);

		if (src_exists && (FD_ISSET(SRC_FD, &ibits)))
			src_recv(SRC_FD, srctraceflag);
	}
}

/*
 * src_recv -	receive requests on the SRC socket
 *
 * Input:
 *	fd	-	file descriptor alleged to represent SRC
 *	trace	-	flag: !0 => trace
 *
 * Note:
 *	An attempt is made to clean this mess up, but my time here is
 *	limited.
 */
src_recv(fd, trace) {
	register rc;
	int addrsz = sizeof (struct sockaddr);
	char buf[128], *cp;
	int onoff, x;
	FILE *stfp, *popen();
        struct sockaddr srcaddr;

	rc = recvfrom(fd, &srcpacket,SRCMSG, 0, &srcaddr, &addrsz);

	if (rc < 0) {
		syslog(LOG_ERR
		       , MSGSTR(SRC2, "%s: ERROR: '%d' recvfrom\n")
		       , progname, errno);
		exit(1);
	}

	/*
	 * process the SRC request packet
	 */
	switch (srcpacket.subreq.action) {
	    case START:
	    case REFRESH:
		dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: routed does not support this option.\n"),sizeof(struct srcrep));
		break;

	    case STOP:
		if (srcpacket.subreq.object == SUBSYSTEM) {
			dosrcpacket(SRC_OK,progname,NULL,sizeof(struct srcrep));
			if (srcpacket.subreq.parm1 == NORMAL)
				hup();
			exit(0);
		} else
			dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: routed does not support this option.\n"),sizeof(struct srcrep));
		break;

	    case STATUS:
		if (srcpacket.subreq.object == SUBSYSTEM) {
		    	srcreplen = srcstatsize = sizeof(struct srchdr) + (sizeof(struct statcode)*SRCSTATNDX);
		    	srcstatus = (struct statrep *)malloc(srcstatsize);
		    	memset(srcstatus,'\0',srcreplen);
			cont = NEWREQUEST;
		    	srcstat("","",getpid(),&srcstatsize,srcstatus, &cont);
		    	srchdr = srcrrqs(&srcpacket);
			sprintf(srcstatus->statcode[3].objname,
				"%-12s",MSGSTR(SRC4,"Trace"));
			if (trace) 
				sprintf(srcstatus->statcode[3].objtext,
					" %s", MSGSTR(SRC5,"Active"));
			else
				sprintf(srcstatus->statcode[3].objtext,
					" %s", MSGSTR(SRC6,"Inactive"));
			sprintf(srcstatus->statcode[4].objname,
				"%-12s",MSGSTR(SRC7,"Debug"));
			if (debug) 
				sprintf(srcstatus->statcode[4].objtext,
					" %s", MSGSTR(SRC5,"Active"));
			else
				sprintf(srcstatus->statcode[4].objtext,
					" %s", MSGSTR(SRC6,"Inactive"));
			sprintf(srcstatus->statcode[5].objname,
				"%-12s",MSGSTR(SRC22,"Supplier"));
			if (supplier == 1) 
				sprintf(srcstatus->statcode[5].objtext,
					" %s", MSGSTR(SRC5,"Active"));
			else
				sprintf(srcstatus->statcode[5].objtext,
					" %s", MSGSTR(SRC6,"Inactive"));
		    	srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
		    	memset(srcstatus,'\0',srcreplen);
			sprintf(srcstatus->statcode[1].objname,
				"%-12s",MSGSTR(SRC8,"Signal"));
			sprintf(srcstatus->statcode[1].objtext,
				" %s",MSGSTR(SRC9,"Purpose"));
			sprintf(srcstatus->statcode[2].objname,
				" %-12s",MSGSTR(SRC10,"SIGALRM"));
			sprintf(srcstatus->statcode[2].objtext,
				"%s", MSGSTR(SRC11,"Manages timers on routing table entries"));
			sprintf(srcstatus->statcode[3].objname,
				" %-12s",MSGSTR(SRC12,"SIGHUP"));
			sprintf(srcstatus->statcode[3].objtext,
				"%s", MSGSTR(SRC13,"Terminates routed"));
			sprintf(srcstatus->statcode[4].objname,
				" %-12s",MSGSTR(SRC14,"SIGINT"));
			sprintf(srcstatus->statcode[4].objtext,
				"%s", MSGSTR(SRC15,"Deletes all existing routes"));
			sprintf(srcstatus->statcode[5].objname,
				" %-12s",MSGSTR(SRC16,"SIGTERM"));
			sprintf(srcstatus->statcode[5].objtext,
				"%s", MSGSTR(SRC13,"Terminates routed"));
		    	srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
		    	memset(srcstatus,'\0',srcreplen);
			sprintf(srcstatus->statcode[0].objname,
				" %-12s",MSGSTR(SRC17,"SIGUSR1"));
			sprintf(srcstatus->statcode[0].objtext,
				"%s", MSGSTR(SRC18,"Toggles packet tracing on"));
			sprintf(srcstatus->statcode[1].objname,
				" %-12s",MSGSTR(SRC19,"SIGUSR2"));
			sprintf(srcstatus->statcode[1].objtext,
				"%s", MSGSTR(SRC20,"Toggles packet tracing off"));
			x = 3;
			stfp = popen("netstat -r","r");
			while (fgets(buf,sizeof(buf),stfp) > 0) {
			    if ((cp = strchr(buf,'\n')) != NULL)
				*cp = '\0';
			    cp = buf;
			    cp += strspn(cp, DELIM);
			    cp += strcspn(cp, DELIM);
			    *cp++ = '\0';
			    cp += strspn(cp, DELIM);
			    if (x < SRCSTATNDX) {
				sprintf(srcstatus->statcode[x].objname,
				    "%-12s",buf);
				sprintf(srcstatus->statcode[x].objtext,
				    "%s", cp);
				x++;
			    } else {
		    		srcsrpy(srchdr,srcstatus,srcreplen,
					STATCONTINUED);
				x = 0;
		    		memset(srcstatus,'\0',srcreplen);

			    }
			}
			pclose(stfp);
			
			if (x > 0) 
		    	    srcsrpy(srchdr,srcstatus,srcreplen,STATCONTINUED);
		    	srcsrpy(srchdr,srcstatus,sizeof(struct srchdr),END);
		    	free(srcstatus);
		} else
			dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: routed does not support this option.\n"),sizeof(struct srcrep));
		break;

	    case TRACE:
		if (srcpacket.subreq.object == SUBSYSTEM) { 
			onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
			if (setsockopt(s, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0) {
				syslog(LOG_ERR, MSGSTR(SRC21,"setsockopt SO_DEBUG: %m"));
				close(s);
				exit (1);
			}
			dosrcpacket(SRC_OK,progname,NULL,sizeof(struct srcrep)); 
		} else
			dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: routed does not support this option.\n"),sizeof(struct srcrep));
		break;

	    default:
		dosrcpacket(SRC_SUBICMD,progname,NULL,sizeof(struct srcrep));
		break;

	}
}

int dosrcpacket(msgno, subsys, txt, len)
	int msgno;
	char *subsys;
	char *txt;
	int len;
{
	struct srcrep reply;

	reply.svrreply.rtncode = msgno;
	strcpy(reply.svrreply.objname, "routed");
	strcpy(reply.svrreply.rtnmsg, txt);
	srchdr = srcrrqs(&srcpacket);
	srcsrpy(srchdr, &reply, len, cont);
}

timevaladd(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec += t2->tv_sec;
	if ((t1->tv_usec += t2->tv_usec) > 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}

timevalsub(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec -= t2->tv_sec;
	if ((t1->tv_usec -= t2->tv_usec) < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
}

process(fd)
	int fd;
{
	struct sockaddr from;
	int fromlen, cc;
	union {
		char	buf[MAXPACKETSIZE+1];
		struct	rip rip;
	} inbuf;

	for(;;) {
		fromlen = sizeof (from);
		cc = recvfrom(fd, &inbuf, sizeof (inbuf), 0, &from, &fromlen);
		if (cc <= 0) {
			if (cc < 0 && errno != EWOULDBLOCK)
			  perror(MSGSTR(RECERR, "recvfrom")); /*MSG*/
			break;
		}
		if (fromlen != sizeof (struct sockaddr_in))
			break;
		rip_input(&from, &inbuf.rip, cc);
	}
}

getsocket(domain, type, sin)
	int domain, type;
	struct sockaddr_in *sin;
{
	int sock, on = 1;

	if ((sock = socket(domain, type, 0)) < 0) {
		perror(MSGSTR(SOCKET, "socket")); /*MSG*/
		syslog(LOG_ERR, MSGSTR(SOCKERR,"socket: %m"));
		return (-1);
	}
#ifdef SO_BROADCAST
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		syslog(LOG_ERR, MSGSTR(SKTOPT, "setsockopt SO_BROADCAST: %m")); /*MSG*/
		close(sock);
		return (-1);
	}
#endif
#ifdef SO_RCVBUF
	for (on = bufspace; ; on -= 1024) {
		if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
		    &on, sizeof (on)) == 0)
			break;
		if (on <= 8*1024) {
		    syslog(LOG_ERR, MSGSTR(SKTOPT1,"setsockopt SO_RCVBUF: %m"));
			break;
		}
	}
	if (traceactions)
		fprintf(ftrace, MSGSTR(REVBUF,"recv buf %d\n"), on);
#endif
	if (bind(sock, sin, sizeof (*sin)) < 0) {
		perror(MSGSTR(BINDERR, "bind")); /*MSG*/
		syslog(LOG_ERR, MSGSTR(BINDERR2, "bind: %m")); /*MSG*/
		close(sock);
		return (-1);
	}
	on = 1;
	if (ioctl(sock, FIONBIO, &on) == -1)
		syslog(LOG_ERR, "ioctl(FIONBIO): %m");
	return (sock);
}
