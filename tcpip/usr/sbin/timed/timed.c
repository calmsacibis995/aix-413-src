static char sccsid[] = "@(#)58	1.17.1.1  src/tcpip/usr/sbin/timed/timed.c, tcptimer, tcpip41J, 9517A_all 4/24/95 14:56:16";
/* 
 * COMPONENT_NAME: TCPIP timed.c
 * 
 * FUNCTIONS: MSGSTR, Mtimed, addnetname, casual, checkignorednets, 
 *            date, firstslavenet, lookformaster, makeslave, setstatus 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985 Regents of the University of California.
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
" Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif 

#ifndef lint
static char sccsid[] = "timed.c	2.14 (Berkeley) 6/18/88";
#endif  not lint */

#include "globals.h"
#define TSPTYPES
#include <protocols/timed.h>
#include <net/if.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <spc.h>
#include <setjmp.h>
#include <netdb.h> /*this file was added to allow the use of AF_INET*/

extern char *optarg; /* used by getopt() to return the argument1991 modification*/
#include <nl_types.h>
#include "timed_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd, MS_TIMED,n,s)

#define SRC_FD	13
#define TIMEBUF	50	/* buffer to hold internationalized time string */
char timebuf[TIMEBUF];

int id;
int trace;
int sock, sock_raw = -1;
int status = 0;
int backoff;
int slvcount;				/* no. of slaves controlled by master */
int machup;
u_short sequence;			/* sequence number */
long delay1;
long delay2;
long random();
char hostname[MAXHOSTNAMELEN];
struct host hp[NHOSTS];
char tracefile[] = "/usr/adm/timed.log";
FILE *fd;
jmp_buf jmpenv;
struct netinfo *nettab = NULL;
int nslavenets;		/* Number of networks were I could be a slave */
int nmasternets;	/* Number of networks were I could be a master */
int nignorednets;	/* Number of ignored networks */
int nnets;		/* Number of networks I am connected to */
struct netinfo *slavenet;
struct netinfo *firstslavenet();
int Mflag;
int justquit = 0;
int src_exists = TRUE;

struct nets {
	char *name;
	unsigned long net;
	struct nets *next;
} *nets = (struct nets *)0;

/*
 * The timedaemons synchronize the clocks of hosts in a local area network.
 * One daemon runs as master, all the others as slaves. The master
 * performs the task of computing clock differences and sends correction
 * values to the slaves. 
 * Slaves start an election to choose a new master when the latter disappears 
 * because of a machine crash, network partition, or when killed.
 * A resolution protocol is used to kill all but one of the masters
 * that happen to exist in segments of a partitioned network when the 
 * network partition is fixed.
 *
 * Authors: Riccardo Gusella & Stefano Zatti
 */

#include <locale.h>

main(argc, argv)
int argc;
char **argv;
{
	int on;
	int ret;
        int ret1;
	long seed;
        unsigned long adr;
	int nflag, iflag;
	struct timeval time;
	struct servent *srvp;
	long casual();
	char *date();
	int n;
	int flag;
	char buf[BUFSIZ], *cp, *cplim;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	register struct netinfo *ntp;
	struct netinfo *ntip;
	struct netinfo *savefromnet;
	struct sockaddr_in server;
	u_short port;
	uid_t getuid();
	struct sigvec sv;
	int src_sig_exit();

#ifdef lint
	ntip = NULL;
#endif

/*
 * The declerations and code are for SRC support - .
 */

        int rc, i, addrsz;
        struct sockaddr srcaddr;
	char    progname[128];

	setlocale(LC_ALL,"");
	catd = catopen(MF_TIMED,NL_CAT_LOCALE);
	Mflag = 0;
	on = 1;
	backoff = 1;
	trace = OFF;
	nflag = OFF;
	iflag = OFF;

        strcpy(progname,argv[0]);

        /* make sure file descriptor 0 is a socket */
        addrsz = sizeof(srcaddr);
        if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
                src_exists = FALSE;
        }

        if (src_exists)
                dup2(0,SRC_FD);

	if (getuid() != 0) {
		fprintf(stderr, MSGSTR(NOTROOT,"Timed: not superuser\n"));
		exit(1);
	}

/* the following while loop was modified from bsd by the use of getopt() in
   order to get null terminated string used by inet_addr() that was added
*/
	while ((ret1 = getopt(argc, argv, "Mtn:i:S")) != EOF) {
			switch (ret1) {

			case 'M':
				Mflag = 1; 
				break;
			case 't':
				trace = ON; 
				break;
			case 'n':
				if (iflag) {
					fprintf(stderr,
				    MSGSTR(NOTIN,"timed: -i and -n make no sense together\n"));
				} else {
					nflag = ON;
					addnetname(optarg);
				}
				break;
			case 'i':
				if (nflag) {
					fprintf(stderr,
				    MSGSTR(NOTIN,"timed: -i and -n make no sense together\n"));
				} else {
					iflag = ON;
					addnetname(optarg);
				}
				break;
                        case 'S':
                                src_exists = TRUE;
		} 
	}

/*
 * signal handlers must be set-up for signal processing
 */
		bzero((char *)&sv, sizeof(sv));
		sv.sv_mask = sigmask(SIGHUP) | sigmask(SIGTERM);
		sv.sv_handler = src_sig_exit;
		sigvec(SIGINT, &sv, (struct sigvec *)0);
		sv.sv_mask = sigmask(SIGINT) | sigmask(SIGTERM);
		sv.sv_handler = src_sig_exit;
		sigvec(SIGHUP, &sv, (struct sigvec *)0);
		sv.sv_mask = sigmask(SIGINT) | sigmask(SIGHUP);
		sv.sv_handler = src_sig_exit;
		sigvec(SIGTERM, &sv, (struct sigvec *)0);

/*
 * fork only if SRC does not exist.
 */
	if (!src_exists) {
 		if (fork())
 			exit(0);
	}

	/* make sure that no other timed is already running */
	if (!timed_lock()) {
		syslog(LOG_ERR,"timed: could not acquire lock - there is another time daemon already running"); 
		exit(1);
	}

	{ int s;
	  for (s = getdtablesize(); s >= 0; --s)
		(void) close(s);
	  (void) open("/dev/null", 0);
	  (void) dup2(0, 1);
	  (void) dup2(0, 2);
	  s = open("/dev/tty", 2);
	  if (s >= 0) {
		(void) ioctl(s, TIOCNOTTY, (char *)0);
		(void) close(s);
	  }
	}
	if (!src_exists) 
		setsid();
	openlog("timed", LOG_CONS|LOG_PID, LOG_DAEMON);

	if (!src_exists)
		syslog(LOG_ERR,MSGSTR(SRCNO,"timed: SRC not found, continuing without SRC support\n"));

	if (trace == ON) {
		fd = fopen(tracefile, "w+");
		setlinebuf(fd);
		fprintf(fd, MSGSTR(TSTART,"Tracing started on: %s\n\n"), 
					date());
	}

	srvp = getservbyname("timed", "udp");
	if (srvp == 0) {
		syslog(LOG_CRIT, MSGSTR(UNKSERVICE,"unknown service 'timed/udp'"));
		exit(1);
	}
	port = srvp->s_port;
	server.sin_port = srvp->s_port;
	server.sin_family = AF_INET;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		syslog(LOG_ERR, MSGSTR(SOCKET, "socket: %m"));
		exit(1);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, 
							sizeof(on)) < 0) {
		syslog(LOG_ERR, MSGSTR(SETSOCK,"setsockopt: %m"));
		exit(1);
	}
	if (bind(sock, &server, sizeof(server))) {
		if (errno == EADDRINUSE)
		        syslog(LOG_ERR, MSGSTR(SRVRUNNING,"server already running"));
		else
		        syslog(LOG_ERR, MSGSTR(BIND,"bind: %m"));
		exit(1);
	}

	/* choose a unique seed for random number generation */
	(void)gettimeofday(&time, (struct timezone *)0);
	seed = time.tv_sec + time.tv_usec;
	srandom(seed);

	sequence = random();     /* initial seq number */

	/* rounds kernel variable time to multiple of 5 ms. */
	time.tv_sec = 0;
	time.tv_usec = -((time.tv_usec/1000) % 5) * 1000;
	(void)adjtime(&time, (struct timeval *)0);

	id = getpid();

	if (gethostname(hostname, sizeof(hostname) - 1) < 0) {
		syslog(LOG_ERR, MSGSTR(GETHOST,"gethostname: %m"));
		exit(1);
	}
	hp[0].name = hostname;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0) {
		syslog(LOG_ERR, MSGSTR(GETCONF, "get interface configuration: %m"));
		exit(1);
	}
	ntp = NULL;
#define max(a, b) (a > b ? a : b)
#ifdef AF_LINK
#define size(p) max((p).sa_len, sizeof(p))
#else
#define size(p) (sizeof(p))
#endif
        cplim = buf + ifc.ifc_len; /* skip over if's with big ifr_addr's */
        for (cp = buf; cp < cplim; cp += max(sizeof(struct ifreq), (sizeof(ifr->
ifr_name) + size(ifr->ifr_addr))) ) {
		ifr = (struct ifreq *) cp;
                ifreq = *ifr;
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		if (ntp == NULL)
			ntp = (struct netinfo *)malloc(sizeof(struct netinfo));
		ntp->my_addr = 
			((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
		if (ioctl(sock, SIOCGIFFLAGS, 
					(char *)&ifreq) < 0) {
			syslog(LOG_ERR, MSGSTR(GETFLSAG,"get interface flags: %m"));
			continue;
		}
		if ((ifreq.ifr_flags & IFF_UP) == 0 ||
			((ifreq.ifr_flags & IFF_BROADCAST) == 0 &&
			(ifreq.ifr_flags & IFF_POINTOPOINT) == 0)) {
			continue;
		}
		if (ifreq.ifr_flags & IFF_BROADCAST)
			flag = 1;
		else
			flag = 0;
		if (ioctl(sock, SIOCGIFNETMASK, 
					(char *)&ifreq) < 0) {
			syslog(LOG_ERR, MSGSTR(GETMASK, "get netmask: %m"));
			continue;
		}
		ntp->mask = ((struct sockaddr_in *)
			&ifreq.ifr_addr)->sin_addr.s_addr;
		if (flag) {
			if (ioctl(sock, SIOCGIFBRDADDR, 
						(char *)&ifreq) < 0) {
				syslog(LOG_ERR, MSGSTR(GETADDR, "get broadaddr: %m"));
				continue;
			}
			ntp->dest_addr = *(struct sockaddr_in *)&ifreq.ifr_broadaddr;
		} else {
			if (ioctl(sock, SIOCGIFDSTADDR, 
						(char *)&ifreq) < 0) {
				syslog(LOG_ERR, MSGSTR(GETDSTADDR,"get destaddr: %m"));
				continue;
			}
			ntp->dest_addr = *(struct sockaddr_in *)&ifreq.ifr_dstaddr;
		}
		ntp->dest_addr.sin_port = port;
		if (nflag || iflag) {
			u_long addr, mask;
			struct nets *tpn;

			addr = ntohl(ntp->dest_addr.sin_addr.s_addr);
			mask = ntohl(ntp->mask);
			while ((mask & 0xFF) == 0) {
				addr >>= 8;
				mask >>= 8;
			}
			for (tpn = nets ; tpn ; tpn = tpn->next)
				if ( (addr & mask) == tpn->net )
					break;
			if (nflag && !tpn || iflag && tpn)
				continue;
		}
		ntp->net = ntp->mask & ntp->dest_addr.sin_addr.s_addr;
		ntp->next = NULL;
		if (nettab == NULL) {
			nettab = ntp;
		} else {
			ntip->next = ntp;
		}
		ntip = ntp;
		ntp = NULL;
	}
	if (ntp)
		(void) free((char *)ntp);
	if (nettab == NULL) {
		syslog(LOG_ERR, MSGSTR(NONET,"No network usable"));
		exit(1);
	}

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		lookformaster(ntp);
	setstatus();
	/*
	 * Take care of some basic initialization.
	 */
	/* us. delay to be used in response to broadcast */
	delay1 = casual((long)10000, 200000);	

	/* election timer delay in secs. */
	delay2 = casual((long)MINTOUT, (long)MAXTOUT);

	if (Mflag) {
		/*
		 * number (increased by 1) of slaves controlled by master: 
		 * used in master.c, candidate.c, networkdelta.c, and 
		 * correct.c 
		 */
		slvcount = 1;
		ret = setjmp(jmpenv);

		switch (ret) {

		case 0: 
			makeslave(firstslavenet());
			setstatus();
			break;
		case 1: 
			/* Just lost our master */
			setstatus();
			slavenet->status = election(slavenet);
			checkignorednets();
			setstatus();
			if (slavenet->status == MASTER)
				makeslave(firstslavenet());
			else
				makeslave(slavenet);
			setstatus();
			break;
		case 2:
			/* Just been told to quit */
			fromnet->status = SLAVE;
			setstatus();
			savefromnet = fromnet;
			rmnetmachs(fromnet);
			checkignorednets();
			if (slavenet)
				makeslave(slavenet);
			else
				makeslave(savefromnet);
			setstatus();
			justquit = 1;
			break;
			
		default:
			/* this should not happen */
			syslog(LOG_ERR, MSGSTR(INVALIDSTAT,"Attempt to enter invalid state"));
			break;
		}
			
		if (status & MASTER) {
			/* open raw socket used to measure time differences */
			if (sock_raw == -1) {
			    sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
			    if (sock_raw < 0)  {
				    syslog(LOG_ERR, MSGSTR(RAWSOCK,"opening raw socket: %m"));
				    exit (1);
			    }
			}
		} else {
			/* sock_raw is not being used now */
			if (sock_raw != -1) {
			    (void)close(sock_raw);
			    sock_raw = -1;
			}
		}

		if (status == MASTER) 
			master();
		else 
			slave();
	} else {
		/* if Mflag is not set timedaemon is forced to act as a slave */
		status = SLAVE;
		if (setjmp(jmpenv)) {
			setstatus();
			checkignorednets();
		}
		makeslave(firstslavenet());
		for (ntp = nettab; ntp != NULL; ntp = ntp->next)
			if (ntp->status == MASTER)
				ntp->status = IGNORE;
		setstatus();
		slave();
	}
}

/*
 * timed must acquire a lock before starting.
 *	returns TRUE if timed was able to acquire the lock
 *	returns FALSE if timed wasn't able to acquire the lock.
 */
timed_lock()
{
	char *timelf;

	timelf = "timed";

	/* lock is available, try and acquire it */
	if (ttylock(timelf) < 0)
		return (FALSE);

	/* got it! */
	return (TRUE);
}

/*
 * unlock the timed lock file
 */
timed_unlock()
{
	char *timelf;

	timelf = "timed";

	/* check and see if locked */
	if (ttylocked(timelf))
		ttyunlock(timelf);
}

/*
 * Try to become master over ignored nets..
 */
checkignorednets()
{
	register struct netinfo *ntp;
	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == IGNORE)
			lookformaster(ntp);
}

lookformaster(ntp)
	register struct netinfo *ntp;
{
	struct tsp resp, conflict, *answer, *readmsg(), *acksend();
	struct timeval time;
	char mastername[MAXHOSTNAMELEN];
	struct sockaddr_in masteraddr;

	ntp->status = SLAVE;
	/* look for master */
	resp.tsp_type = TSP_MASTERREQ;
	(void)strcpy(resp.tsp_name, hostname);
	answer = acksend(&resp, &ntp->dest_addr, (char *)ANYADDR, 
	    TSP_MASTERACK, ntp);
	if (answer == NULL) {
		/*
		 * Various conditions can cause conflict: race between
		 * two just started timedaemons when no master is
		 * present, or timedaemon started during an election.
		 * Conservative approach is taken: give up and became a
		 * slave postponing election of a master until first
		 * timer expires.
		 */
		time.tv_sec = time.tv_usec = 0;
		answer = readmsg(TSP_MASTERREQ, (char *)ANYADDR,
		    &time, ntp);
		if (answer != NULL) {
			ntp->status = SLAVE;
			return;
		}

		time.tv_sec = time.tv_usec = 0;
		answer = readmsg(TSP_MASTERUP, (char *)ANYADDR,
		    &time, ntp);
		if (answer != NULL) {
			ntp->status = SLAVE;
			return;
		}

		time.tv_sec = time.tv_usec = 0;
		answer = readmsg(TSP_ELECTION, (char *)ANYADDR,
		    &time, ntp);
		if (answer != NULL) {
			ntp->status = SLAVE;
			return;
		}
		ntp->status = MASTER;
	} else {
		(void)strcpy(mastername, answer->tsp_name);
		masteraddr = from;

		/*
		 * If network has been partitioned, there might be other
		 * masters; tell the one we have just acknowledged that 
		 * it has to gain control over the others. 
		 */
		time.tv_sec = 0;
		time.tv_usec = 300000;
		answer = readmsg(TSP_MASTERACK, (char *)ANYADDR, &time,
		    ntp);
		/*
		 * checking also not to send CONFLICT to ack'ed master
		 * due to duplicated MASTERACKs
		 */
		if (answer != NULL && 
		    strcmp(answer->tsp_name, mastername) != 0) {
			conflict.tsp_type = TSP_CONFLICT;
			(void)strcpy(conflict.tsp_name, hostname);
			if (acksend(&conflict, &masteraddr, mastername,
			    TSP_ACK, (struct netinfo *)NULL) == NULL) {
				syslog(LOG_ERR, 
				    MSGSTR(ERRSEND,"error on sending TSP_CONFLICT"));
				exit(1);
			}
		}
	}
}
/*
 * based on the current network configuration, set the status, and count
 * networks;
 */
setstatus()
{
	register struct netinfo *ntp;

	status = 0;
	nmasternets = nslavenets = nnets = nignorednets = 0;
	if (trace)
		fprintf(fd, MSGSTR(NETSTAT,"Net status:\n"));
	for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
		switch ((int)ntp->status) {
		  case MASTER:
			nmasternets++;
			break;
		  case SLAVE:
			nslavenets++;
			break;
		  case IGNORE:
			nignorednets++;
			break;
		}
		if (trace) {
			fprintf(fd, "\t%-16s", inet_ntoa(ntp->net));
			switch ((int)ntp->status) {
			  case MASTER:
				fprintf(fd, MSGSTR(MASTER0,"MASTER\n"));
				break;
			  case SLAVE:
				fprintf(fd, MSGSTR(SLAVE,"SLAVE\n"));
				break;
			  case IGNORE:
				fprintf(fd, MSGSTR(IGN,"IGNORE\n"));
				break;
			  default:
				fprintf(fd, MSGSTR(INVALIDSTATE,"invalid state %d\n"),(int)ntp->status);
				break;
			}
		}
		nnets++;
		status |= ntp->status;
	}
	status &= ~IGNORE;
	if (trace)
		fprintf(fd,
		  MSGSTR(TRACE2,"\tnets = %d, masters = %d, slaves = %d, ignored = %d\n"),
		      nnets, nmasternets, nslavenets, nignorednets);
}

makeslave(net)
	struct netinfo *net;
{
	register struct netinfo *ntp;

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == SLAVE && ntp != net)
			ntp->status = IGNORE;
	slavenet = net;
}
	
struct netinfo *
firstslavenet()
{
	register struct netinfo *ntp;

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == SLAVE)
			return (ntp);
	return ((struct netinfo *)0);
}

/*
 * `casual' returns a random number in the range [inf, sup]
 */

long
casual(inf, sup)
long inf;
long sup;
{
	float value;

	value = (float)(random() & 0x7fffffff) / 0x7fffffff;
	return(inf + (sup - inf) * value);
}

char *
date()
{
	char    *ctime();
	struct	timeval tv;
	struct  tm *tmptr;
	struct	tm *localtime();

	(void)gettimeofday(&tv, (struct timezone *)0);
	tmptr = localtime(&tv.tv_sec);
	if (strftime(timebuf, TIMEBUF, "%a %sD %H:%M", tmptr))
		return(timebuf);
	else
		return (ctime(&tv.tv_sec));
}

addnetname(name)
	char *name;
{
	register struct nets **netlist = &nets;
	struct netent *n;

	while (*netlist)
		netlist = &((*netlist)->next);
	*netlist = (struct nets *)malloc(sizeof **netlist);
	if (*netlist == (struct nets *)0) {
		syslog(LOG_ERR, MSGSTR(MALLOC,"malloc failed"));
		exit(1);
	}
	bzero((char *)*netlist, sizeof(**netlist));
	(*netlist)->name = name;

/* The following lines were added to allow the use of several network  */
/*  names or addresses in dotted decimal notation. In addition, this   */
/*  allows the user to not have to set up /etc/networks file           */
/* Getnetbyname is able to resolve both the network names and ip       */
/*  address in /etc/networks, since they are both seen as character    */
/*  strings. Therefore, I did not use getnetbyaddr.                    */

	n = getnetbyname(name);
	if (n == NULL)
		(*netlist)->net = inet_network(name);
	else 
		(*netlist)->net = n->n_net;
		
}

/* 
 * src_sig_exit - signal handler for an SRC stop call 
 */
src_sig_exit(sig)
	int	sig;
{
	char	*cp;

	if (src_exists) {
		if (sig == SIGHUP) cp = MSGSTR(SRCNORM,"normal");
		if (sig == SIGINT) cp = MSGSTR(SRCFORCE,"force");
		if (sig == SIGTERM) cp = MSGSTR(SRCCANCEL,"cancel");
		syslog(LOG_ERR,MSGSTR(SRCEXIT,
			"timed exiting: SRC stop %s received"),cp);
	}
	timed_unlock();	
	exit(0);
}

