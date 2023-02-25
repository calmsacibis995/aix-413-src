static char sccsid[] = "@(#)67	1.5  src/tcpip/usr/sbin/gated/ripquery.c, tcprouting, tcpip411, GOLD410 12/6/93 17:57:10";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		in_addr_ofs
 *		inet_netmask
 *		perror
 *		sock_inaddr
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  ripquery.c,v 1.15 1993/03/27 03:32:33 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#define	INCLUDE_CTYPE

#include "include.h"
#define	RIPCMDS
#include "inet.h"
#include "rip.h"

/* macros to select internet address given pointer to a struct sockaddr */

/* result is u_int32 */
#define sock_inaddr(x) (((struct sockaddr_in *)(x))->sin_addr)

/* result is struct in_addr */
#define in_addr_ofs(x) (((struct sockaddr_in *)(x))->sin_addr)

/* Calculate the natural netmask for a given network */
#define	inet_netmask(net) (IN_CLASSA(net) ? IN_CLASSA_NET :\
			 (IN_CLASSB(net) ? IN_CLASSB_NET :\
			  (IN_CLASSC(net) ? IN_CLASSC_NET : 0)))

#ifdef vax11c
#define perror(s) vmsperror(s)
#endif	/* vax11c */

#define WTIME	5			/* Time to wait for responses */
#define STIME	1			/* Time to wait between packets */

static int s;
static char packet[RIP_PKTSIZE];
static int cmds_request[] = {
    RIPCMD_REQUEST,
    RIPCMD_POLL,
    0 };
static int cmds_poll[] = {
    RIPCMD_POLL,
    RIPCMD_REQUEST,
    0 };
static int *cmds = cmds_poll;
static int dflag;
static int nflag;
const char *version = "1.15";
static int vers = RIP_VERSION_2;
static auth_data[RIP_AUTH_SIZE];
static int domain;

extern int errno;
extern char *optarg;
extern int optind, opterr;


/* Print an IP address as a dotted quad.  We could use inet_ntoa, but */
/* that breaks with GCC on a Sun4 */
static char *
pr_ntoa __PF1(addr, u_int32)
{
    static char buf[18];
    register char *bp = buf;
    register byte *cp = (byte *) &addr;
    register int i = sizeof (struct in_addr);
    register int c;

    while (i--) {
	if (c = *cp++) {
	    register int leading = 0;
	    register int power = 100;

	    do {
		if (c >= power) {
		    *bp++ = '0' + c/power;
		    c %= power;
		    leading = 1;
		} else if (leading || power == 1) {
		    *bp++ = '0';
		}
	    } while (power /= 10) ;
	} else {
	    *bp++ = '0';
	}
	if (i) {
	    *bp++ = '.';
	}
    }
    *bp = (char) 0;
    return buf;
}


/*
 *	Trace RIP packets
 */
static void
rip_trace __PF4(dir, const char *,
		who, struct sockaddr_in *,
		cp, void_t,
		size, register int)
{
    register struct rip *rpmsg = (struct rip *) cp;
    register struct netinfo *n;
    register const char *cmd = "Invalid";

    /* XXX - V2 extensions */

    if (rpmsg->rip_cmd && rpmsg->rip_cmd < RIPCMD_MAX) {
	cmd = trace_state(rip_cmd_bits, rpmsg->rip_cmd);
    }
    (void) fprintf(stderr, "RIP %s %s+%d vers %d, cmd %s, domain %d, length %d",
		   dir,
		   pr_ntoa(who->sin_addr.s_addr),
		   ntohs(who->sin_port),
		   rpmsg->rip_vers,
		   cmd,
		   rpmsg->rip_domain,
		   size);

    switch (rpmsg->rip_cmd) {
#ifdef	RIPCMD_POLL
	case RIPCMD_POLL:
#endif	/* RIPCMD_POLL */
	case RIPCMD_REQUEST:
	case RIPCMD_RESPONSE:
	    (void) fprintf(stderr, "\n");
	    size -= 4 * sizeof(char);
	    n = (struct netinfo *) ((void_t) (rpmsg + 1));
	    for (; size > 0; n++, size -= sizeof(struct netinfo)) {
		u_int family = ntohs(n->rip_family);
		metric_t metric = ntohl((u_int32) n->rip_metric);
		
		if (size < sizeof(struct netinfo)) {
		    break;
		}
		switch (family) {
		case RIP_AF_INET:
		    if (rpmsg->rip_vers > 1) {
			(void) fprintf(stderr,
				       "\tnet %15s/",
				       pr_ntoa(n->rip_dest));
			(void) fprintf(stderr,
				       "%-15s  ",
				       pr_ntoa(n->rip_dest_mask));
			(void) fprintf(stderr,
				       "router %-15s  metric %2d  tag %#04X\n",
				       pr_ntoa(n->rip_router),
				       metric,
				       ntohs((u_int16) n->rip_tag));
		    } else {
			(void) fprintf(stderr,
				       "\tnet %-15s  metric %2d\n",
				       pr_ntoa(n->rip_dest),
				       metric);
		    }
		    break;

		case RIP_AF_UNSPEC:
		    if (metric == RIP_METRIC_UNREACHABLE) {
			(void) fprintf(stderr,
				       "\trouting table request\n");
			break;
		    }
		    goto bogus;

		case RIP_AF_AUTH:
		    if (rpmsg->rip_vers > 1
			&& n == (struct netinfo *) ((void_t) (rpmsg + 1))) {
			struct authinfo *auth = (struct authinfo *) n;
			int auth_type = ntohs(auth->auth_type);

			switch (auth_type) {
			case RIP_AUTH_NONE:
			    (void) fprintf(stderr,
					   "\tAuthentication: None\n");
			    break;

			case RIP_AUTH_SIMPLE:
			    (void) fprintf(stderr,
					   "\tAuthentication: %.*s\n",
					   (int) sizeof auth->auth_data,
					   (char *) auth->auth_data);
			    break;

			default:
			    (void) fprintf(stderr,
					   "\tInvalid auth type: %d\n",
					   auth_type);
			}
			break;
		    }
		    /* Fall through */

		default:
		bogus:
		    (void) fprintf(stderr,
				   "\tInvalid family: %d",
				   family);
		}
	    }
	    (void) fprintf(stderr,
			   "RIP %s end of packet", dir);
	    break;
	case RIPCMD_TRACEON:
	    (void) fprintf(stderr, ", file %*s", size, (char *) (rpmsg + 1));
	    break;
#ifdef	RIPCMD_POLLENTRY
	case RIPCMD_POLLENTRY:
	    n = (struct netinfo *) ((void_t) (rpmsg + 1));
	    (void) fprintf(stderr, ", net %s",
			   pr_ntoa(n->rip_dest));
	    break;
#endif	/* RIPCMD_POLLENTRY */
	default:
	    (void) fprintf(stderr, "\n");
	    break;
    }
    (void) fprintf(stderr, "\n");
}


static void
query __PF2(host, char *,
	    cmd, int)
{
    struct sockaddr_in router;
    struct rip *msg = (struct rip *) ((void_t) packet);
    struct netinfo *nets = (struct netinfo *) ((void_t) (msg + 1));
    struct servent *sp;
    int rc;

    bzero((char *) &router, sizeof(router));
    if (!inet_aton(host, &router.sin_addr)) {
	struct hostent *hp = gethostbyname(host);

	if (!hp) {
	    (void) printf("%s: unknown\n", host);
	    exit(1);
	} else {
	    (void) bcopy(hp->h_addr, (char *) &router.sin_addr, (size_t) hp->h_length);
	}
    }
    sp = getservbyname("router", "udp");
    if (sp == NULL) {
	(void) fprintf(stderr, "No service for router available\n");
	exit(1);
    }
    router.sin_family = AF_INET;
    router.sin_port = sp->s_port;
    msg->rip_cmd = cmd;
    msg->rip_domain = domain;
    msg->rip_vers = vers;
    if (vers > 1
	&& *auth_data) {
	struct authinfo *auth = (struct authinfo *) nets++;

	auth->auth_family = RIP_AF_AUTH;
	auth->auth_type = RIP_AUTH_SIMPLE;
	bcopy((caddr_t) auth_data, (caddr_t) auth->auth_data, RIP_AUTH_SIZE);
    }
    bzero((caddr_t) nets, sizeof *nets);
    nets->rip_family = htons(AF_UNSPEC);
    nets->rip_metric = htonl((u_int32) RIP_METRIC_UNREACHABLE);

    if (dflag) {
	rip_trace("SEND", &router, (void_t) msg, (caddr_t) (nets + 1) - packet);
    }
    NON_INTR(rc,
	     sendto(s,
		    packet,
		    (caddr_t) (nets  + 1) - packet,
		    0,
		    (struct sockaddr *) & router,
		    sizeof(router)));
    if (rc < 0)
	perror(host);
}


/*
 * Handle an incoming routing packet.
 */
static void
rip_input __PF3(from, struct sockaddr_in *,
		to, struct in_addr *,
		size, int)
{
    register struct rip *msg = (struct rip *) ((void_t) packet);
    struct netinfo *n;
    struct hostent *hp;

    if (dflag) {
	rip_trace("RECV", from, (void_t) msg, size);
    }
    if (msg->rip_cmd != RIPCMD_RESPONSE)
	return;
    if (nflag
	|| !(hp = gethostbyaddr((char *) &from->sin_addr, sizeof(struct in_addr), AF_INET))) {
	(void) printf("%d bytes from %s",
		      size,
		      pr_ntoa(from->sin_addr.s_addr));
    } else {
	(void) printf("%d bytes from %s(%s)",
		      size,
		      hp->h_name,
		      pr_ntoa(from->sin_addr.s_addr));
    }
    if (to) {
	(void) printf(" to %s",
		      pr_ntoa(to->s_addr));
    }
    printf(":\n");
    size -= sizeof(int);
    n = (struct netinfo *) ((void_t) (msg + 1));
    while (size > 0) {
	if (size < sizeof(struct netinfo))
	    break;
	switch (msg->rip_vers) {
	case 0:
	    break;

	default:
	    GNTOHS(n->rip_tag);
	    /* Fall through */

	case 1:
	    GNTOHS(n->rip_family);
	    GNTOHL(n->rip_metric);
	    break;
	}
#ifdef	notdef
	sin = (struct sockaddr_in *) & n->rip_dst;
	if (sin->sin_port) {
	    (void) printf("**Non-zero port (%d) **",
			  sin->sin_port & 0xFFFF);
	}
	for (i = 0; i < 8; i++)
	    if (n->rip_dst.rip_zero2[i]) {
		(void) printf("sockaddr = ");
		for (i = 0; i < 8; i++)
		    (void) printf("%d ", n->rip_dst.rip_zero2[i] & 0xFF);
		break;
	    }
#endif	/* notdef */
	switch (n->rip_family) {
	case RIP_AF_INET:
	    if (msg->rip_vers > 1) {
		(void) printf("\t%15s/",
			      pr_ntoa(n->rip_dest));
		(void) printf("%-15s  ",
			      pr_ntoa(n->rip_dest_mask));
		(void) printf("router %-15s  metric %2d  tag %#04X\n",
			      pr_ntoa(n->rip_router),
			      n->rip_metric,
			      n->rip_tag);
	    } else {
		/* XXX - Print a couple per line */
		(void) printf("\t%-15s  metric %2d\n",
			      pr_ntoa(n->rip_dest),
			      n->rip_metric);
	    }
	    break;

	case RIP_AF_AUTH:
	    break;

	default:
	    (void) printf("\tInvalid family: %d\n",
			  n->rip_family);
	    break;
	}
	size -= sizeof(struct netinfo), n++;
    }
}


int
main __PF2(argc, int,
	   argv, char **)
{
    int c, cc, fdbits, errflg = 0, *cmd = cmds;
    static struct timeval *wait_time, long_time = {WTIME, 0}, short_time = {STIME, 0};
    static struct sockaddr_in from;
    static struct iovec iovec = { (caddr_t) packet, sizeof (packet) };
#ifdef	SCM_RIGHTS
    int on = 1;
    struct in_addr *to;
    static byte control[BUFSIZ];
    static struct msghdr msghdr = {
	(caddr_t) &from, sizeof (from),
	&iovec, 1,
	(caddr_t) control, sizeof(control),
	0 } ;
#else	/* SCM_RIGHTS */
    static struct msghdr msghdr = {
	(caddr_t) &from, sizeof (from),
	&iovec, 1 } ;
#endif	/* SCM_RIGHTS */

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	perror("socket");
#ifdef vax11c
	exit(0x10000002);
#else	/* vax11c */
	exit(2);
#endif	/* vax11c */
    }
#ifdef	SCM_RIGHTS
    if (setsockopt(s, IPPROTO_IP, IP_RECVDSTADDR, &on, sizeof (on)) < 0) {
	perror("setsockopt(IP_RECVDSTADDR)");
	exit (2);
    }
#endif	/* SCM_RIGHTS */
    while ((c = getopt(argc, argv, "a:dnprvw:12D:")) != EOF) {
	switch (c) {
	case 'a':
	    {
		int len;
		char *cp = optarg;
		char *dp = (caddr_t) auth_data;

		for (len = 0; *cp; len++) {
		    if (!isprint(*cp)) {
			break;
		    }
		    *dp++ = *cp++;
		}
		if (*cp || !len || len > RIP_AUTH_SIZE) {
		    (void) fprintf(stderr,
				   "Invalid Authentication: %s\n",
				   optarg);
		    errflg++;
		    break;
		}
		while (len++ < RIP_AUTH_SIZE) {
		    *dp++ = (char) 0;
		}
	    }
	    break;

	case 'd':
	    dflag++;
	    break;

	case 'n':
	    nflag++;
	    break;
	    
	case 'p':
	    cmd = cmds_poll;
	    break;

	case 'r':
	    cmd = cmds_request;
	    break;

	case 'v':
	    (void) fprintf(stderr, "RIPQuery %s\n", version);
	    break;

	case 'w':
	    long_time.tv_sec = atoi(optarg);
	    (void) fprintf(stderr, "Wait time set to %d\n", long_time.tv_sec);
	    break;

	case '1':
	    vers = RIP_VERSION_1;
	    break;

	case '2':
	    vers = RIP_VERSION_2;
	    break;

	case 'D':
	    domain = atoi(optarg);
	    break;
	    
	case '?':
	    errflg++;
	    break;
	}
    }

    if (errflg || (optind >= argc)) {
	(void) printf("usage: ripquery [ -d ] [ -n ] [ -p ] [ -r ] [ -v ] [ -1 ] [ -2 ] [-D domain] [ -w time] hosts...\n");
	exit(1);
    }
    setnetent(1);

    for (; optind < argc; optind++) {
      retry:
	query(argv[optind], *cmd);
	fdbits = 1 << s;
	wait_time = &long_time;
	for (;;) {
#ifndef vax11c
	    cc = select(s + 1, (struct fd_set *) & fdbits, (struct fd_set *) 0, (struct fd_set *) 0, wait_time);
#else	/* vax11c */
	    cc = Socket_Ready(s, wait_time->tv_sec);
#endif	/* vax11c */
	    if (cc == 0) {
		if (wait_time == &short_time) {
		    break;
		}
		if (*(++cmd)) {
		    goto retry;
		} else {
		    break;
		}
	    } else if (cc < 0) {
		perror("select");
		(void) close(s);
		exit(1);
	    } else {
		wait_time = &short_time;
		msghdr.msg_namelen = sizeof (from);
#ifdef	SCM_RIGHTS
		msghdr.msg_controllen = sizeof (control) ;
		msghdr.msg_flags = 0;
#endif	/* SCM_RIGHTS */
		NON_INTR(cc, recvmsg(s, &msghdr, 0));
		if (cc <= 0) {
		    if (cc < 0) {
			perror("recvmsg");
			(void) close(s);
			exit(1);
		    }
		    continue;
		}
#ifdef	SCM_RIGHTS
#define	ENOUGH_CMSG(cmsg, size)	((cmsg)->cmsg_len >= ((size) + sizeof(struct cmsghdr)))

		if (msghdr.msg_flags & MSG_TRUNC) {
		    (void) fprintf(stderr, "message from %s truncated to %d bytes\n",
				   pr_ntoa(from.sin_addr.s_addr),
				   cc);
#ifdef	notdef
		    continue;
#endif	/* notdef */
		}

		to = (struct in_addr *) 0;
		if (msghdr.msg_controllen >= sizeof (struct cmsghdr)
		    && !(msghdr.msg_flags & MSG_CTRUNC)) {
		    struct cmsghdr *cmsg;

		    for (cmsg = CMSG_FIRSTHDR(&msghdr);
			 cmsg && cmsg->cmsg_len >= sizeof (struct cmsghdr);
			 cmsg = CMSG_NXTHDR(&msghdr, cmsg)) {

			if (cmsg->cmsg_level == IPPROTO_IP
			    && cmsg->cmsg_type == IP_RECVDSTADDR
			    && ENOUGH_CMSG(cmsg, sizeof (struct in_addr))) {
			    to = (struct in_addr *) CMSG_DATA(cmsg);
			}
		    }
		}

		rip_input(&from, to, cc);
#else	/* SCM_RIGHTS */
		rip_input(&from, (struct in_addr *) 0, cc);
#endif	/* SCM_RIGHTS */
	    }
	}
    }

    endnetent();
    return 0;
}



#ifdef	vax11c
/*
 *	See if a socket is ready for reading (waiting "n" seconds)
 */
static int
Socket_Ready __PF2(Socket, int,
		   Wait_Time, int)
{
#include <vms/iodef.h>
#define EFN_1 20
#define EFN_2 21
    int Status;
    int Timeout_Delta[2];
    int Dummy;
    unsigned short int IOSB[4];

    /*
     *	Check for data (using MSG_PEEK)
     */
    Status = SYS$QIO(EFN_1,
		     Socket,
		     IO$_READVBLK,
		     IOSB,
		     0,
		     0,
		     &Dummy,
		     sizeof(Dummy),
		     MSG_PEEK,
		     0,
		     0,
		     0);
    /*
     *	Check for completion
     */
    if (IOSB[0] != 0) {
	return 1;
    }
    /*
     *	Setup timer
     */
    if (Wait_Time) {
	Timeout_Delta[0] = -(Wait_Time * 10000000);
	Timeout_Delta[1] = -1;
	SYS$SETIMR(EFN_2, Timeout_Delta, 0, Socket_Ready);
	SYS$WFLOR(EFN_1, (1 << EFN_1) | (1 << EFN_2));
	if (IOSB[0] != 0) {
	    SYS$CANTIM(Socket_Ready, 0);
	    return 1;
	}
    }
    /*
     *	Last chance
     */
    if (IOSB[0] == 0) {
	/*
	 *	Lose:
	 */
	SYS$CANCEL(Socket);
	return 0;
    }
    return 1;
}

#endif	/* vax11c */


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
