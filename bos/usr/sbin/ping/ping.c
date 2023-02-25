static char sccsid[] = "@(#)32	1.23.1.6  src/bos/usr/sbin/ping/ping.c, cmdnet, bos411, 9428A410j 6/13/94 16:35:50";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: A
 *		B
 *		CLR
 *		MSGSTR
 *		SET
 *		TST
 *		catcher
 *		dump
 *		fill
 *		finish
 *		in_cksum
 *		main
 *		pinger
 *		pr_addr
 *		pr_icmph
 *		pr_iph
 *		pr_pack
 *		pr_retip
 *		tvsub
 *		usage
 *
 *   ORIGINS: 26,27,89
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *			P I N G . C
 *
 * Using the InterNet Control Message Protocol (ICMP) "ECHO" facility,
 * measure round-trip-delays and packet loss across network paths.
 *
 * Author -
 *	Mike Muuss
 *	U. S. Army Ballistic Research Laboratory
 *	December, 1983
 *
 * Status -
 *	Public Domain.  Distribution Unlimited.
 * Bugs -
 *	More statistics could always be gathered.
 *	This program has to run SUID to ROOT to access the ICMP socket.
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/signal.h>

#include <net/nh.h> 		/*pdw*/
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#define	DEFDATALEN	(64 - 8)	/* default data length */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	(65536 - 60 - 8)/* max packet size */
#define	MAXWAIT		10		/* max seconds to wait for response */
#define	NROUTES		9		/* number of record route slots */

#ifdef IP_MULTICAST
#define MULTICAST_NOLOOP 1	/* multicast options */
#define MULTICAST_TTL	 2
#define MULTICAST_IF	 4
int moptions;
#endif IP_MULTICAST

#define	A(bit)		rcvd_tbl[(bit)>>3]	/* identify byte in array */
#define	B(bit)		(1 << ((bit) & 0x07))	/* identify bit in byte */
#define	SET(bit)	(A(bit) |= B(bit))
#define	CLR(bit)	(A(bit) &= (~B(bit)))
#define	TST(bit)	(A(bit) & B(bit))

/* various options */
int options;
#define	F_FLOOD		0x001
#define	F_INTERVAL	0x002
#define	F_NUMERIC	0x004
#define	F_PINGFILLED	0x008
#define	F_QUIET		0x010
#define	F_RROUTE	0x020
#define	F_SO_DEBUG	0x040
#define	F_SO_DONTROUTE	0x080
#define	F_VERBOSE	0x100

/*
 * MAX_DUP_CHK is the number of bits in received table, i.e. the maximum
 * number of received sequence numbers we can keep track of.  Change 128
 * to 8192 for complete accuracy...
 */
#define	MAX_DUP_CHK	(8 * 128)
int mx_dup_ck = MAX_DUP_CHK;
char rcvd_tbl[MAX_DUP_CHK / 8];

struct sockaddr whereto;	/* who to ping */
int datalen = DEFDATALEN;
int s;				/* socket file descriptor */
u_char outpack[MAXPACKET];
char BSPACE = '\b';		/* characters written for flood */
char DOT = '.';
char *hostname;
int ident;			/* process id to identify our packets */

/* counters */
long npackets;			/* max packets to transmit */
long nreceived;			/* # of packets we got back */
long nrepeats;			/* number of duplicates */
long ntransmitted;		/* sequence # for outbound packets = #sent */
int interval = 1;		/* interval between packets */

/* timing */
int timing;			/* flag to do timing */
long tmin = LONG_MAX;		/* minimum round trip time */
long tmax;			/* maximum round trip time */
u_long tsum;			/* sum of all times, for doing average */

int Dflag;
char *pattern;			/* data pattern, if PINGFILLED */

u_long inet_addr();
char *inet_ntoa(), *pr_addr();
void catcher(), finish();

#include <locale.h>
#include <nl_types.h> 
#include "ping_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PING,n,s) 

main(argc, argv)
	int argc;
	char **argv;
{
	extern int errno, optind;
	extern char *optarg;
	struct timeval timeout;
	struct hostent *hp;
	struct sockaddr_in *to;
	struct protoent *proto;
	register int i;
	int ch, fdmask, hold, packlen, preload;
	u_char *datap, *packet;
	char *target, hnamebuf[MAXHOSTNAMELEN], *malloc();
#ifdef IP_OPTIONS
	char rspace[3 + 4 * NROUTES + 1];	/* record route space */
#endif

#ifdef IP_MULTICAST
	struct in_addr ifaddr;
	int loop=1;
	u_char char_loop;
	int ttl=0;
	u_char char_ttl;
#endif IP_MULTICAST 


	setlocale(LC_ALL,"");
	catd = catopen(MF_PING,NL_CAT_LOCALE);
	preload = 0;
	datap = &outpack[8 + sizeof(struct timeval)];
#ifdef IP_MULTICAST
	while ((ch = getopt(argc, argv, "DRc:dfh:i:l:np:qrs:vLT:I:")) != EOF)
#else
	while ((ch = getopt(argc, argv, "DRc:dfh:i:l:np:qrs:v")) != EOF)
#endif /* IP_MULTICAST */
		switch(ch) {
		case 'D':
			Dflag++;
			break;
		case 'c':
			npackets = atoi(optarg);
			if (npackets <= 0) {
				(void)fprintf(stderr,
	    MSGSTR(MSG1, "ping: bad number of packets to transmit.\n"));
				exit(1);
			}
			break;
		case 'd':
			options |= F_SO_DEBUG;
			break;
		case 'f':
			if (getuid()) {
				(void)fprintf(stderr,
	    MSGSTR(MSG2, "ping: you must be root to use the -f option.\n"));
				exit(1);
			}
			options |= F_FLOOD;
			setbuf(stdout, (char *)NULL);
			break;
		case 'i':		/* wait between sending packets */
			interval = atoi(optarg);
			if (interval <= 0) {
				(void)fprintf(stderr,
	    MSGSTR(MSG3, "ping: bad timing interval.\n"));
				exit(1);
			}
			options |= F_INTERVAL;
			break;
		case 'l':
			preload = atoi(optarg);
			if (preload < 0) {
				(void)fprintf(stderr,
	    MSGSTR(MSG4, "ping: bad preload value.\n"));
				exit(1);
			}
			break;
		case 'n':
			options |= F_NUMERIC;
			break;
		case 'p':		/* fill buffer with user pattern */
			options |= F_PINGFILLED;
			pattern = optarg;
			break;
		case 'q':
			options |= F_QUIET;
			break;
		case 'R':
			options |= F_RROUTE;
			break;
		case 'r':
			options |= F_SO_DONTROUTE;
			break;
		case 's':		/* size of packet to send */
			datalen = atoi(optarg);
			if (datalen > MAXPACKET) {
				(void)fprintf(stderr,
	    MSGSTR(MSG5, "ping: packet size too large.\n"));
				exit(1);
			}
			if (datalen <= 0) {
				(void)fprintf(stderr,
	    MSGSTR(MSG6, "ping: illegal packet size.\n"));
				exit(1);
			}
			break;
		case 'v':
			options |= F_VERBOSE;
			break;
#ifdef IP_MULTICAST
		case 'L':
			moptions |= MULTICAST_NOLOOP;
		      	loop = 0;
			break;
		case 'T':
			moptions |= MULTICAST_TTL;
			ttl = atoi(optarg);
			if (ttl > 255) {
			  (void)fprintf(stderr,
	    MSGSTR(MSGTTL, "ping: multicast ttl %u out of range.\n"), ttl);
			  exit(1);
			}
			break;
		case 'I':
			moptions |= MULTICAST_IF;
			target = optarg;
			if (!isinet_addr(target)) {
			    if ((hp = gethostbyname(target))==(struct hostent *) 0 || hp->h_addrtype != AF_INET) {
				(void)fprintf(stderr,
					      MSGSTR(MSGINTF, "ping: bad interface address '%s'\n"), target);
				exit(1);
			    }
			    bcopy(hp->h_addr, &ifaddr.s_addr, hp->h_length);
			} else {
			    if ((ifaddr.s_addr = inet_addr(target)) == (u_int)-1) {
				(void)fprintf(stderr,
					      MSGSTR(MSGINTF, "ping: bad interface address '%s'\n"), target);
				exit(1);
			    }	
			}
			break;
#endif /* IP_MULTICAST */
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc >= 2)
		datalen = atoi(argv[1]);
	if (argc > 2)
		npackets = atoi(argv[2]);

	if ( (argc < 1) || (argc >  3) )
		usage();
	target = *argv;

	bzero((char *)&whereto, sizeof(struct sockaddr));
	to = (struct sockaddr_in *)&whereto;
	to->sin_family = AF_INET;
	to->sin_addr.s_addr = inet_addr(target);
	/* see if it's a name or an address */
	if (!isinet_addr(target)) {
	   if ((hp = gethostbyname(target))==(struct hostent *) 0) {
                    fprintf(stderr,MSGSTR(NME_NT_FND,
                                  "ping: host name %s NOT FOUND\n"), target);
                    exit(1);
                 }
             to->sin_family = hp->h_addrtype;
             bcopy(hp->h_addr, (caddr_t)&to->sin_addr, hp->h_length);
	     (void)strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
             hostname = hnamebuf;
         }
	 else {
	    if (to->sin_addr.s_addr == (u_int)-1) {
		fprintf(stderr, MSGSTR(BADADDR,
                                 "ping: Address %s misformed\n"), target);
                exit(2);
                }
            hostname = target;
	 }

	if (options & F_FLOOD && options & F_INTERVAL) {
		(void)fprintf(stderr,
		    MSGSTR(MSG8, "ping: -f and -i incompatible options.\n"));
		exit(1);
	}

	if (datalen >= sizeof(struct timeval))	/* can we time transfer */
		timing = 1;
	packlen = datalen + MAXIPLEN + MAXICMPLEN;
	if (!(packet = (u_char *)malloc((u_int)packlen))) {
		(void)fprintf(stderr, MSGSTR(MSG9, "ping: out of memory.\n"));
		exit(1);
	}
	if (!(options & F_PINGFILLED))
		for (i = 8; i < datalen; ++i)
			*datap++ = i;
	else
		fill((char *)datap, pattern, datalen - 8);

	ident = getpid() & 0xFFFF;

	if (!(proto = getprotobyname("icmp"))) {
		(void)fprintf(stderr, MSGSTR(PROTOERR,
				"ping: unknown protocol icmp.\n"));
		exit(1);
	}
	if ((s = socket(AF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror(MSGSTR(SOCKERR, "ping: socket"));
		exit(1);
	}
	hold = 1;
	if (options & F_SO_DEBUG)
		(void)setsockopt(s, SOL_SOCKET, SO_DEBUG, (char *)&hold,
		    sizeof(hold));
	if (options & F_SO_DONTROUTE)
		(void)setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (char *)&hold,
		    sizeof(hold));

	/* record route option */
	if (options & F_RROUTE) {
#ifdef IP_OPTIONS
		bzero(rspace, sizeof rspace);
		rspace[IPOPT_OPTVAL] = IPOPT_RR;
		rspace[IPOPT_OLEN] = sizeof(rspace)-1;
		rspace[IPOPT_OFFSET] = IPOPT_MINOFF;
		if (setsockopt(s, IPPROTO_IP, IP_OPTIONS, rspace,
		    sizeof(rspace)) < 0) {
			perror("ping: record route");
			exit(1);
		}
#else
		(void)fprintf(stderr,
		  MSGSTR(MSG12, "ping: record route not available in this implementation.\n"));
		exit(1);
#endif /* IP_OPTIONS */
	}

#ifdef IP_MULTICAST
	if (moptions & MULTICAST_NOLOOP) {
		char_ttl = (u_char)ttl;
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP,
							&char_loop, 1) == -1) {
			perror ("Can't disable multicast loopback");
			exit(1);
		}
	}
	if (moptions & MULTICAST_TTL) {
		char_ttl = (u_char)ttl;
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL,
							&char_ttl, 1) == -1) {
			perror ("Can't set multicast time-to-live");
			exit(1);
		}
	}
	if (moptions & MULTICAST_IF) {
		if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF,
					&ifaddr, sizeof(ifaddr)) == -1) {
			perror ("Can't set multicast source interface");
			exit(1);
		}
	}
#endif /* IP_MULTICAST */
	/*
	 * When pinging the broadcast address, you can get a lot of answers.
	 * Doing something so evil is useful if you are trying to stress the
	 * ethernet, or just want to fill the arp cache to get some stuff for
	 * /etc/ethers.
	 */
	hold = 48 * 1024;
	(void)setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&hold,
	    sizeof(hold));

	if (to->sin_family == AF_INET)
		(void)printf(MSGSTR(DATBYTS1,
		    "PING %s (%s): %d data bytes\n"), hostname,
		    inet_ntoa(*(struct in_addr *)&to->sin_addr.s_addr),
		    datalen);
	else
		(void)printf(MSGSTR(DATBYTS2,
		    "PING %s: %d data bytes\n"), hostname, datalen);

	(void)signal(SIGINT, (void(*)(int)) finish);
	(void)signal(SIGALRM, (void(*)(int)) catcher);

	while (preload--)		/* fire off them quickies */
		pinger();

	if ((options & F_FLOOD) == 0)
		catcher(0);		/* start things going */

	for (;;) {
		struct sockaddr_in from;
		register int cc;
		int fromlen;

		if (options & F_FLOOD) {
			pinger();
			timeout.tv_sec = 0;
			timeout.tv_usec = 10000;
			fdmask = 1 << s;
			if (select(s + 1, (fd_set *)&fdmask, (fd_set *)NULL,
			    (fd_set *)NULL, &timeout) < 1)
				continue;
		}
		fromlen = sizeof(from);
		if ((cc = recvfrom(s, (char *)packet, packlen, 0,
		    (struct sockaddr *)&from, &fromlen)) < 0) {
			if (errno == EINTR)
				continue;
			perror(MSGSTR(RECERR, "ping: recvfrom"));
			continue;
		}
		pr_pack((char *)packet, cc, &from);
		if (npackets && nreceived >= npackets)
			break;
	}
	finish(0);
	/* NOTREACHED */
}

/*
 * catcher --
 *	This routine causes another PING to be transmitted, and then
 * schedules another SIGALRM for 1 second from now.
 * 
 * bug --
 *	Our sense of time will slowly skew (i.e., packets will not be
 * launched exactly at 1-second intervals).  This does not affect the
 * quality of the delay and loss statistics.
 */
void
catcher(int s)
{
	int waittime;

	pinger();
	(void)signal(SIGALRM, (void(*)(int)) catcher);
	if (!npackets || ntransmitted < npackets)
		alarm((u_int)interval);
	else {
		if (nreceived) {
			waittime = 2 * tmax / 1000;
			if (!waittime)
				waittime = 1;
		} else
			waittime = MAXWAIT;
		(void)signal(SIGALRM, (void(*)(int)) finish);
		(void)alarm((u_int)waittime);
	}
}

/*
 * pinger --
 * 	Compose and transmit an ICMP ECHO REQUEST packet.  The IP packet
 * will be added on by the kernel.  The ID field is our UNIX process ID,
 * and the sequence number is an ascending integer.  The first 8 bytes
 * of the data portion are used to hold a UNIX "timeval" struct in VAX
 * byte-order, to compute the round-trip time.
 */
pinger()
{
	register struct icmp *icp;
	register int cc;
	int i;

	icp = (struct icmp *)outpack;
	icp->icmp_type = ICMP_ECHO;
	icp->icmp_code = 0;
	icp->icmp_cksum = 0;
	icp->icmp_seq = ntransmitted++;
	icp->icmp_id = ident;			/* ID */

	CLR(icp->icmp_seq % mx_dup_ck);

	if (timing)
		(void)gettimeofday((struct timeval *)&outpack[8],
		    (struct timezone *)NULL);

	cc = datalen + 8;			/* skips ICMP portion */

	/* compute ICMP checksum here */
	icp->icmp_cksum = in_cksum((u_short *)icp, cc);

	i = sendto(s, (char *)outpack, cc, 0, &whereto,
	    sizeof(struct sockaddr));

	if (i < 0 || i != cc)  {
		if (i < 0)
			perror(MSGSTR(SENDTOERR, "ping: sendto"));
	(void)printf(MSGSTR(WRTBYTS, "ping: wrote %s %d chars, ret=%d\n"),
		    hostname, cc, i);
	}
	if (options & F_FLOOD)
		(void)write(STDOUT_FILENO, &DOT, 1);
}

dump(p, c)
	char *p;
	int c;
{
	int i;
	char buf[17];

	buf[16] = 0;
	for (i = 0; i < c; p++, i++) {
		if (i % 16 == 0 && i != 0)
			printf("* %s\n", buf);
		printf("%02x ", *p & 0xff);
		buf[i % 16] = (*p > 31 && *p < 127) ? *p : '.';
	}
	for (; i % 16; i++) {
		printf("   ");
		buf[i % 16] = ' ';
	}
	printf("* %s\n", buf);
}

/*
 * pr_pack --
 *	Print out the packet, if it came from us.  This logic is necessary
 * because ALL readers of the ICMP socket get a copy of ALL ICMP packets
 * which arrive ('tis only fair).  This permits multiple copies of this
 * program to be run without having intermingled output (or statistics!).
 */
pr_pack(buf, cc, from)
	char *buf;
	int cc;
	struct sockaddr_in *from;
{
	register struct icmp *icp;
	register u_long l;
	register int i, j;
	register u_char *cp,*dp;
	static int old_rrlen;
	static char old_rr[MAX_IPOPTLEN];
	struct ip *ip;
	struct timeval tv, *tp;
	long triptime;
	int hlen, dupflag;

	(void)gettimeofday(&tv, (struct timezone *)NULL);

	/* Check the IP header */
	ip = (struct ip *)buf;
	hlen = ip->ip_hl << 2;
	if (cc < hlen + ICMP_MINLEN) {
		if (options & F_VERBOSE)
			(void)fprintf(stderr,
			  MSGSTR(SHRTPKT, "ping: packet too short (%d bytes) from %s\n"), cc,
			  inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr));
		return;
	}

	/* Now the ICMP part */
	cc -= hlen;
	icp = (struct icmp *)(buf + hlen);
	if (icp->icmp_type == ICMP_ECHOREPLY) {
		if (icp->icmp_id != ident)
			return;			/* 'Twas not our ECHO */
		++nreceived;
		if (Dflag)
			dump(buf, cc + hlen);
		if (timing) {
#ifndef icmp_data
			tp = (struct timeval *)&icp->icmp_ip;
#else
			tp = (struct timeval *)icp->icmp_data;
#endif
			tvsub(&tv, tp);
			triptime = tv.tv_sec * 1000 + (tv.tv_usec / 1000);
			tsum += triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
		}

		if (TST(icp->icmp_seq % mx_dup_ck)) {
			++nrepeats;
			--nreceived;
			dupflag = 1;
		} else {
			SET(icp->icmp_seq % mx_dup_ck);
			dupflag = 0;
		}

		if (options & F_QUIET)
			return;

		if (options & F_FLOOD)
			(void)write(STDOUT_FILENO, &BSPACE, 1);
		else {
			(void)printf(MSGSTR(MSG14, "%d bytes from %s: icmp_seq=%u"), cc,
			   inet_ntoa(*(struct in_addr *)&from->sin_addr.s_addr),
			   icp->icmp_seq);
			(void)printf(MSGSTR(MSG15, " ttl=%d"), ip->ip_ttl);
			if (timing)
				(void)printf(MSGSTR(MSG16, " time=%ld ms"), triptime);
			if (dupflag)
				(void)printf(MSGSTR(MSG17, " (DUP!)"));
			/* check the data */
			cp = (u_char*)&icp->icmp_data[8];
			dp = &outpack[8 + sizeof(struct timeval)];
			for (i = 8; i < datalen; ++i, ++cp, ++dp) {
				if (*cp != *dp) {
	(void)printf(MSGSTR(MSG18, "\nwrong data byte #%d should be 0x%x but was 0x%x"),
	    i, *dp, *cp);
					cp = (u_char*)&icp->icmp_data[0];
					for (i = 8; i < datalen; ++i, ++cp) {
						/* MSG - okay */
						if ((i % 32) == 8)
							(void)printf("\n\t");
						(void)printf("%x ", *cp);
					}
					break;
				}
			}
		}
	} else {
		/* We've got something other than an ECHOREPLY */
		if (!(options & F_VERBOSE))
			return;
		(void)printf(MSGSTR(BYTRECV, "%d bytes from %s: "), cc,
		    pr_addr(from->sin_addr.s_addr));
		pr_icmph(icp);
	}

	/* Display any IP options */
	cp = (u_char *)buf + sizeof(struct ip);

	/* ANSI C will force hlen to unsigned! */
	for (; hlen > (int)sizeof(struct ip); --hlen, ++cp)
		switch (*cp) {
		case IPOPT_EOL:
			hlen = 0;
			break;
		case IPOPT_LSRR:
			(void)printf("\nLSRR: ");	/* MSG - okay */
			j = *++cp;
			++cp;
			hlen -= (j-1);
			if (j > IPOPT_MINOFF)	/* MSG - okay */
				for (;;) {
					l = *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					l = (l<<8) + *++cp;
					if (l == 0)
						(void)printf("\t0.0.0.0");
				else
					(void)printf("\t%s", pr_addr(ntohl(l)));
				j -= 4;
				if (j <= IPOPT_MINOFF)
					break;
				(void)putchar('\n');
			}
			cp += j;
			break;
		case IPOPT_RR:
			j = *++cp;		/* get length */
			i = *++cp;		/* and pointer */
			hlen -= (j-1);		/* --hlen in loop head */
			j -= 3;			/* now only data bytes left */
			if (i > j)
				i = j;
			i -= IPOPT_MINOFF;
			if (i <= 0) {
				cp += j;
				break;
			}
			if (i == old_rrlen
			    && cp == (u_char *)buf + sizeof(struct ip) + 2
			    && !bcmp((char *)cp, old_rr, i)
			    && !(options & F_FLOOD)) {
				(void)printf(MSGSTR(MSG19, "\t(same route)"));
				cp += j;
				break;
			}
			old_rrlen = i;
			bcopy((char *)cp, old_rr, i);
			(void)printf("\nRR: ");	/* MSG - okay */
			for (;;) { /* MSG - okay */
				l = *++cp;
				l = (l<<8) + *++cp;
				l = (l<<8) + *++cp;
				l = (l<<8) + *++cp;
				if (l == 0)
					(void)printf("\t0.0.0.0");
				else
					(void)printf("\t%s", pr_addr(ntohl(l)));
				j -= 4;
				i -= 4;
				if (i <= 0)
					break;
				(void)putchar('\n');
			}
			cp += j;
			break;
		case IPOPT_NOP:
			(void)printf("\nNOP");	/* MSG - okay */
			break;
		default:
			(void)printf(MSGSTR(MSG20, "\nunknown option %x"), *cp);
			break;
		}
	if (!(options & F_FLOOD)) {
		(void)putchar('\n');	/* MSG - okay */
		(void)fflush(stdout);
	}
}

/*
 * in_cksum --
 *	Checksum routine for Internet Protocol family headers (C Version)
 */
in_cksum(addr, len)
	u_short *addr;
	int len;
{
	register int nleft = len;
	register u_short *w = addr, tmp;
	register int sum = 0;
	u_short answer = 0;

	/*
	 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
	 * sequential 16 bit words to it, and at the end, fold back all the
	 * carry bits from the top 16 bits into the lower 16 bits.
	 */
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		tmp = *(u_char *)w;
		sum += (tmp << 8);
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

/*
 * tvsub --
 *	Subtract 2 timeval structs:  out = out - in.  Out is assumed to
 * be >= in.
 */
tvsub(out, in)
	register struct timeval *out, *in;
{
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}

/*
 * finish --
 *	Print out statistics, and give up.
 */
void
finish(int s)
{
	(void)signal(SIGINT, SIG_IGN);
	(void)putchar('\n');	/* MSG - okay */
	(void)fflush(stdout);
	(void)printf(MSGSTR(PINGHDR, "--- %s ping statistics ---\n"), hostname);
	(void)printf(MSGSTR(XMTPKTS, "%ld packets transmitted, "), ntransmitted);
	(void)printf(MSGSTR(RECPKTS, "%ld packets received, "), nreceived);
	if (nrepeats)
		(void)printf(MSGSTR(MSG24, "+%ld duplicates, "), nrepeats);
	if (ntransmitted)
		if (nreceived > ntransmitted)
			(void)printf(MSGSTR(MSG25, "-- somebody's printing up packets!"));
		else
			(void)printf(MSGSTR(LOSTPKTS, "%d%% packet loss"),
			    (int) (((ntransmitted - nreceived) * 100) /
			    ntransmitted));
	(void)putchar('\n');
	if (nreceived && (datalen < 8))
               exit(0);

	if (nreceived && timing) {
		(void)printf(MSGSTR(RDTRIPTM, "round-trip min/avg/max = %ld/%lu/%ld ms\n"),
		    tmin, tsum / (nreceived + nrepeats), tmax);
		exit(0);
	}
	else
		exit(1);
}

#ifdef notdef
static char *ttab[] = {
	"Echo Reply",		/* ip + seq + udata */
	"Dest Unreachable",	/* net, host, proto, port, frag, sr + IP */
	"Source Quench",	/* IP */
	"Redirect",		/* redirect type, gateway, + IP  */
	"Echo",
	"Time Exceeded",	/* transit, frag reassem + IP */
	"Parameter Problem",	/* pointer + IP */
	"Timestamp",		/* id + seq + three timestamps */
	"Timestamp Reply",	/* " */
	"Info Request",		/* id + sq */
	"Info Reply"		/* " */
};
#endif

/*
 * pr_icmph --
 *	Print a descriptive string about an ICMP header.
 */
pr_icmph(icp)
	struct icmp *icp;
{
	switch(icp->icmp_type) {
	case ICMP_ECHOREPLY:
		(void)printf(MSGSTR(MSG28, "Echo Reply\n"));
		/* XXX ID + Seq + Data */
		break;
	case ICMP_UNREACH:
		switch(icp->icmp_code) {
		case ICMP_UNREACH_NET:
			(void)printf(MSGSTR(MSG29, "Destination Net Unreachable\n"));
			break;
		case ICMP_UNREACH_HOST:
			(void)printf(MSGSTR(MSG30, "Destination Host Unreachable\n"));
			break;
		case ICMP_UNREACH_PROTOCOL:
			(void)printf(MSGSTR(MSG31, "Destination Protocol Unreachable\n"));
			break;
		case ICMP_UNREACH_PORT:
			(void)printf(MSGSTR(MSG32, "Destination Port Unreachable\n"));
			break;
		case ICMP_UNREACH_NEEDFRAG:
			(void)printf(MSGSTR(MSG33, "frag needed and DF set\n"));
			break;
		case ICMP_UNREACH_SRCFAIL:
			(void)printf(MSGSTR(MSG34, "Source Route Failed\n"));
			break;
		default:
			(void)printf(MSGSTR(MSG35, "Dest Unreachable, Bad Code: %d\n"),
			    icp->icmp_code);
			break;
		}
		/* Print returned IP header information */
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_SOURCEQUENCH:
		(void)printf(MSGSTR(MSG36, "Source Quench\n"));
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_REDIRECT:
		switch(icp->icmp_code) {
		case ICMP_REDIRECT_NET:
			(void)printf(MSGSTR(MSG37, "Redirect Network"));
			break;
		case ICMP_REDIRECT_HOST:
			(void)printf(MSGSTR(MSG38, "Redirect Host"));
			break;
		case ICMP_REDIRECT_TOSNET:
			(void)printf(MSGSTR(MSG39, "Redirect Type of Service and Network"));
			break;
		case ICMP_REDIRECT_TOSHOST:
			(void)printf(MSGSTR(MSG40, "Redirect Type of Service and Host"));
			break;
		default:
			(void)printf(MSGSTR(MSG41, "Redirect, Bad Code: %d"), icp->icmp_code);
			break;
		}
		(void)printf(MSGSTR(MSG42, "(New addr: 0x%08lx)\n"), icp->icmp_gwaddr.s_addr);
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_ECHO:
		(void)printf(MSGSTR(MSG43, "Echo Request\n"));
		/* XXX ID + Seq + Data */
		break;
	case ICMP_TIMXCEED:
		switch(icp->icmp_code) {
		case ICMP_TIMXCEED_INTRANS:
			(void)printf(MSGSTR(MSG44, "Time to live exceeded\n"));
			break;
		case ICMP_TIMXCEED_REASS:
			(void)printf(MSGSTR(MSG45, "Frag reassembly time exceeded\n"));
			break;
		default:
			(void)printf(MSGSTR(MSG46, "Time exceeded, Bad Code: %d\n"),
			    icp->icmp_code);
			break;
		}
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_PARAMPROB:
		(void)printf(MSGSTR(MSG47, "Parameter problem: pointer = 0x%02x\n"),
		    icp->icmp_hun.ih_pptr);
#ifndef icmp_data
		pr_retip(&icp->icmp_ip);
#else
		pr_retip((struct ip *)icp->icmp_data);
#endif
		break;
	case ICMP_TSTAMP:
		(void)printf(MSGSTR(MSG48, "Timestamp\n"));
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_TSTAMPREPLY:
		(void)printf(MSGSTR(MSG49, "Timestamp Reply\n"));
		/* XXX ID + Seq + 3 timestamps */
		break;
	case ICMP_IREQ:
		(void)printf(MSGSTR(MSG50, "Information Request\n"));
		/* XXX ID + Seq */
		break;
	case ICMP_IREQREPLY:
		(void)printf(MSGSTR(MSG51, "Information Reply\n"));
		/* XXX ID + Seq */
		break;
#ifdef ICMP_MASKREQ
	case ICMP_MASKREQ:
		(void)printf(MSGSTR(MSG52, "Address Mask Request\n"));
		break;
#endif
#ifdef ICMP_MASKREPLY
	case ICMP_MASKREPLY:
		(void)printf(MSGSTR(MSG53, "Address Mask Reply\n"));
		break;
#endif
	default:
		(void)printf(MSGSTR(MSG54, "Bad ICMP type: %d\n"), icp->icmp_type);
	}
}

/*
 * pr_iph --
 *	Print an IP header with options.
 */
pr_iph(ip) /* MSG - okay : table headers are abbrev. proto names */
	struct ip *ip;
{
	int hlen;
	u_char *cp;

	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + 20;		/* point to options */

	(void)printf("Vr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst Data\n");
	(void)printf(" %1x  %1x  %02x %04x %04x",
	    ip->ip_v, ip->ip_hl, ip->ip_tos, ip->ip_len, ip->ip_id);
	(void)printf("   %1x %04x", ((ip->ip_off) & 0xe000) >> 13,
	    (ip->ip_off) & 0x1fff);
	(void)printf("  %02x  %02x %04x", ip->ip_ttl, ip->ip_p, ip->ip_sum);
	(void)printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_src.s_addr));
	(void)printf(" %s ", inet_ntoa(*(struct in_addr *)&ip->ip_dst.s_addr));
	/* dump and option bytes */
	while (hlen-- > 20) {
		(void)printf("%02x", *cp++);
	}
	(void)putchar('\n');
}

/*
 * pr_addr --
 *	Return an ascii host address as a dotted quad and optionally with
 * a hostname.
 */
char *
pr_addr(l)
	u_long l;
{
	struct hostent *hp;
	static char buf[80];

	if ((options & F_NUMERIC) ||
	    !(hp = gethostbyaddr((char *)&l, 4, AF_INET)))
		(void)sprintf(buf, "%s", inet_ntoa(*(struct in_addr *)&l));
	else
		(void)sprintf(buf, "%s (%s)", hp->h_name,
		    inet_ntoa(*(struct in_addr *)&l));
	return(buf);
}

/*
 * pr_retip --
 *	Dump some info on a returned (via ICMP) IP packet.
 */
pr_retip(ip)
	struct ip *ip;
{
	int hlen;
	u_char *cp;

	pr_iph(ip);
	hlen = ip->ip_hl << 2;
	cp = (u_char *)ip + hlen;

	if (ip->ip_p == 6)
		(void)printf(MSGSTR(MSG55, "TCP: from port %u, to port %u (decimal)\n"),
		    (*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
	else if (ip->ip_p == 17)
		(void)printf(MSGSTR(MSG56, "UDP: from port %u, to port %u (decimal)\n"),
			(*cp * 256 + *(cp + 1)), (*(cp + 2) * 256 + *(cp + 3)));
}

fill(bp, patp, datalen)
	char *bp, *patp;
	int datalen;
{
	register int ii, jj, kk;
	int pat[16];
	char *cp;

	for (cp = patp; *cp; cp++)
		if (!isxdigit(*cp)) {
			(void)fprintf(stderr,
	    MSGSTR(MSG57, "ping: patterns must be specified as hex digits.\n"));
			exit(1);
		}
	ii = sscanf(patp,
	    "%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",
	    &pat[0], &pat[1], &pat[2], &pat[3], &pat[4], &pat[5], &pat[6],
	    &pat[7], &pat[8], &pat[9], &pat[10], &pat[11], &pat[12],
	    &pat[13], &pat[14], &pat[15]);

	if (ii > 0)
		for (kk = 0; kk <= datalen; kk++)
			bp[kk] = pat[kk % ii];

	if (!(options & F_QUIET)) {
		(void)printf(MSGSTR(MSG58, "PATTERN: 0x"));
		for (jj = 0; jj < ii; ++jj)
			(void)printf("%02x", bp[jj] & 0xFF);	/* MSG - okay */
		(void)printf("\n");
	}
}

usage()
{
	(void)fprintf(stderr,
#ifdef IP_MULTICAST
	    MSGSTR(USAGE, "usage: ping [-RDdfnqrvL] [-c count] [-i wait] [-l preload]\n\t[-I a.b.c.d] [-T ttl]\n\t[-p pattern] [-s packetsize] host [data size] [npackets]\n"));
#else
	    MSGSTR(USAGE, "usage: ping [-RDdfnqrv] [-c count] [-i wait] [-l preload]\n\t[-p pattern] [-s packetsize] host [data size] [npackets]\n"));
#endif /* IP_MULTICAST */
	exit(1);
}
