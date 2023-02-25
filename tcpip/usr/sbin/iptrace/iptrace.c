static char sccsid[] = "@(#)36	1.23  src/tcpip/usr/sbin/iptrace/iptrace.c, tcpras, tcpip411, GOLD410 6/15/94 18:54:19";
/*
 * COMPONENT_NAME: TCPIP iptrace.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <netdb.h>		/* must be before nlist.h	*/
#include <nlist.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/device.h>
#include <sys/sysconfig.h>
#include <sys/x25user.h>

#include <net/if_types.h>
#include <net/if.h>
#ifndef _NET_GLOBLAS_H_
#include <net/net_globals.h>
#endif /* _NET_GLOBLAS_H_ */
#include <sys/devinfo.h>	/* after if.h so IFF_UP not redef'd	*/

#include <netinet/in.h>
#include <sys/cdli.h>
#include <net/nd_lan.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <netinet/if_802_5.h>
#include <netinet/if_fddi.h>
#include <aixif/net_if.h>
#include "if_op.h"              /* shipped version clashes */

/* 
 * The following includes and declerations are for support of the System 
 * Resource Controller (SRC) - .
 */
#include <spc.h>
#include "libsrc.h"

static 	struct srcreq srcpacket;
char	progname[128];
int 	cont;
struct 	srchdr *srchdr;

#include <nl_types.h>
#include "iptrace_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_IPTRACE,n,s) 

#define NETDIR  "/usr/lib/drivers/"
#define	TCP_PROTO	6
#define	UDP_PROTO	17
#define	MAXPACKET	8192	/* max packet size */
#define	FLUSH_TIME	5	/* max writed before flush */
u_char	packet[MAXPACKET];
#define	SOCKBUFSIZE	(56 * 1024)

int s;			/* Socket file descriptor 	*/

int 	verbose = 0;	/* debugging messages		*/
int 	trace_arp = 1;	/* trace arp messages? 		*/
int 	debug   = 0;	/* don't fork or detach		*/
int	proto   = 0;	/* protocol number		*/
int	port	= 0;	/* local port number		*/
u_long	sip	= 0;	/* source ip address 		*/
u_long	dip	= 0;	/* destination ip address	*/
int	twoway	= 0;	/* bidirectional traffic 	*/
int	promisc = 0;	/* promiscuous mode		*/
char	*ifname = NULL; /* interface to trace		*/

FILE	*fp;

char version[]	 = "iptrace 2.0";
char fversion[30];
char usage_msg[] = "Usage:\tiptrace [-a] [-e] [-d Host [-b]] [-s Host [-b]] [-p Port]\n\
\t[-P Protocol] [-i Interface] Logfile\n";

#include <locale.h>

main(argc, argv)
char *argv[];
{
	struct sockaddr 	from;
	extern int 		getopt();
	extern int 		optind;
	int 			c;
	char 			*tracefile;
	int			die();
	int			maxbuf = SOCKBUFSIZE;

	int rc, i, addrsz;
	struct sockaddr srcaddr;
	int src_exists = TRUE;

	setlocale(LC_ALL,"");
	catd = catopen(MF_IPTRACE,NL_CAT_LOCALE);

	strcpy(progname,argv[0]);

	/* make sure file descriptor 0 is a socket */
	addrsz = sizeof(srcaddr);
	if ((rc = getsockname(0, &srcaddr, &addrsz)) < 0) {
		src_exists = FALSE;
	}
	if (src_exists)
		dup2(0,SRC_FD);


	while (( c = getopt(argc, argv, "sdbapPtve?i")) != EOF) {
		switch(c) {
			case 'P' :
				get_proto(argv[optind]);
				optind++;
				break;
			case 'd' :
				dip = get_host(argv[optind]);
				optind++;
				break;
			case 's' :
				sip = get_host(argv[optind]);
				optind++;
				break;
			case 'b' :
				twoway++;
				break;
			case 'i' :
				ifname = argv[optind];
				optind++;
				break;
			case 'p' :
				get_port(argv[optind]);
				optind++;
				break;
			case 'a' :
				trace_arp = 0;
				break;
			case 'v' :
				verbose++;
				break;
			case 't' :
				debug++;
				break;
			case 'e' :
				promisc++;
				break;
			case '?' :
				usage();
				break;
			default :
				usage();
				break;
		}
	}

	if (argv[optind] == NULL)
		usage();

	tracefile = argv[optind++];
	open_tracefile(tracefile);

        (void) signal(SIGTERM, die);
	if (!debug)
		(void) signal(SIGINT, SIG_IGN);
        (void) signal(SIGQUIT, SIG_IGN);
        (void) signal(SIGHUP, die);

	if (intf_cfg(CFG_INIT)) {
 		fprintf(stderr, MSGSTR( CANTLDTREXT,"iptrace: Can't load trace extension\n"));
		exit(-9);
	}

	if ((s = socket(AF_INTF, SOCK_RAW, AF_INTF)) < 0) {
		perror(MSGSTR( ERRSOCK, "iptrace: cannot create socket -"));
		exit(5);
	}
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &maxbuf, sizeof(maxbuf)) ) {
		perror(MSGSTR(SETSOCK, "iptrace: setsockopt -"));
		exit(5);
	}

	set_flags(IFF_DEBUG, ifname);

	if (!debug) {
		if (!src_exists)
			if (fork())
				exit(0);
		{ int tt = open("/dev/tty", O_RDWR);
			if (tt > 0) {
				ioctl(tt, TIOCNOTTY, (char *)0);
				close(tt);
			}
		}
		if (!src_exists)
                	(void) setsid();
                (void) signal(SIGTSTP, SIG_IGN);
                (void) signal(SIGTTIN, SIG_IGN);
                (void) signal(SIGTTOU, SIG_IGN);
	}

	openlog("iptrace", LOG_PID | LOG_NOWAIT, LOG_DAEMON);

	for (;;) {
		int len = sizeof (packet);
		int fromlen = sizeof (from);
		int cc;
 		 
        	fd_set ibits;
		register int n;
	    	int onoff;

		FD_ZERO(&ibits);
		cont=END;
		addrsz=sizeof(srcaddr);
		do {
		        FD_SET(s, &ibits);
			if (src_exists)
		        	FD_SET(SRC_FD, &ibits);

                	n = select(20, &ibits, 0, 0, (struct timeval *)NULL);
			if (n == 0) 
				continue;
			if (n < 0) {
				if (errno != EINTR)
					syslog(LOG_WARNING, "select: %m");
				continue;
			}

			if FD_ISSET(s, &ibits) {
				if ( (cc=recvfrom(s, packet, len, 0, &from, &fromlen)) < 0) {
					if( errno == EINTR )
						continue;
					syslog(LOG_WARNING, "recvfrom %m");
					continue;
				}
				if (verbose)
					printf( MSGSTR(BYTEREV, "\n%d bytes received\n"), cc);

				if (filter(packet,cc))
					fileit(packet, cc);
			}
			rc = -1;
			errno = EINTR;
			if (src_exists)
			    if FD_ISSET(SRC_FD, &ibits)
			        rc = recvfrom(SRC_FD,&srcpacket,SRCMSG,0,&srcaddr,&addrsz);
		} while (rc == -1 && errno == EINTR);
		if (rc < 0) {
			syslog(LOG_ERR,MSGSTR(SRC2,"%s: ERROR: '%d' recvfrom\n"),progname,errno);
			die();
		}

		if (src_exists) {
		    switch(srcpacket.subreq.action) {
			case START:
			case REFRESH:
			case STATUS:
				dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: iptrace does not support this option.\n"),sizeof(struct srcrep));
				break;
			case TRACE:
				if (srcpacket.subreq.object == SUBSYSTEM) { 
					onoff = (srcpacket.subreq.parm2 == TRACEON) ? 1 : 0;
					if (setsockopt(s, SOL_SOCKET, SO_DEBUG, &onoff, sizeof (onoff)) < 0) {
						syslog(LOG_ERR, MSGSTR(SRC4,"setsockopt SO_DEBUG: %m"));
						close(s);
						die();
					}
					dosrcpacket(SRC_OK,progname,NULL,sizeof(struct srcrep)); 
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: iptrace does not support this option.\n"),sizeof(struct srcrep));
				break;
			case STOP:
				if (srcpacket.subreq.object == SUBSYSTEM) {
					dosrcpacket(SRC_OK,argv[0],NULL,sizeof(struct srcrep));
					if (srcpacket.subreq.parm1 == NORMAL)
						die();
					exit(0);
				} else
					dosrcpacket(SRC_SUBMSG,progname,MSGSTR(SRC3,"ERROR: iptrace does not support this option.\n"),sizeof(struct srcrep));
				break;
			default:
				dosrcpacket(SRC_SUBICMD,progname,NULL,sizeof(struct srcrep));
				break;

		    }
		}
	}
	/*NOTREACHED*/
}

/*
 * SRC packet processing - .
 */
int dosrcpacket(msgno, subsys, txt, len)
	int msgno;
	char *subsys;
	char *txt;
	int len;
{
	struct srcrep reply;

	reply.svrreply.rtncode = msgno;
	strcpy(reply.svrreply.objname, "iptrace");
	strcpy(reply.svrreply.rtnmsg, txt);
	srchdr = srcrrqs(&srcpacket);
	srcsrpy(srchdr, &reply, len, cont);
}

usage() {
	fprintf(stderr, MSGSTR(USAGE, usage_msg));
	exit(1);
}

filter(packet, cc)
char		*packet;
int		cc;
{
	u_short		type;
	u_char		hlen;
	struct ip	*ipp;
	struct tcphdr	*th;
	struct udphdr	*uh;
	char		packet_ifname[IFNAMSIZ];
	int		unit;
	struct packet_trace_header	*pth;
	u_char		iftype;
	int		xmit_len;
	int		srcmatch, dstmatch, sip_src, dip_src, sip_dst, dip_dst;

	srcmatch = dstmatch = sip_src = dip_src = sip_dst = dip_dst = 0;

	pth = (struct packet_trace_header *)packet;
	type = pth->type;
	hlen = pth->hlen;
	iftype = pth->iftype;
	bcopy(pth->ifname, packet_ifname, IFNAMSIZ);
	unit = pth->unit;
	packet += sizeof(struct packet_trace_header);
	packet += hlen;
	if (type == 0) {
		type = header_to_type(packet, iftype, &xmit_len, packet_ifname);
		packet += xmit_len;
	}
	if (type == 0)
		return(0);
	if (ifname) {
		packet_ifname[strlen(packet_ifname) + 1] = '\0';
		packet_ifname[strlen(packet_ifname)] = '0' + unit;
		if (strcmp(ifname, packet_ifname))
			return(0);
	}
	if (type != ETHERTYPE_IP) {
		if (trace_arp)
			return(1);
		else
			return(0);
	}
	ipp = (struct ip *) packet;
	if (proto) {
		if (ipp->ip_p != proto)
			return(0);
	}


	sip_src = (sip == ipp->ip_src.s_addr);
	sip_dst = (sip == ipp->ip_dst.s_addr);

	dip_dst = (dip == ipp->ip_dst.s_addr);
	dip_src = (dip == ipp->ip_src.s_addr);

	srcmatch = (sip_src || dip_src);
	dstmatch = (sip_dst || dip_dst);

	if (sip) {
		switch (dip) {
			case 0:
				/* If -b is not specified, sip must 	*/
				/* match src IP. If -b specified, sip 	*/
				/* must match either the dest or src IP	*/
				if (!twoway) {
					if (!sip_src)
						return 0;
					else
						break;
				}
				else {
					if (sip_src || sip_dst)
						break;
					else
						return 0;
				}
			default:
				/* If -b is not specified, sip and dip	*/
				/* must match src and dst IP respectively*/
				/* If -b is specified, capture packet	*/
				/* between sip and dip.			*/
				if (!twoway) {
					if (sip_src && dip_dst)
						break;
					else
						return 0;
				}
				else {
					if (srcmatch && dstmatch)
						break;
					else
						return 0;
				}
		}
	}
	if (dip) {
		switch (sip) {
			case 0:
				/* If -b is not specified, dip must 	*/
				/* match dest IP. If -b specified, dip 	*/
				/* must match either the dest or src IP	*/
				if (!twoway) {
					if (!dip_dst)
						return 0;
					else
						break;
				}
				else {
					if (dip_src || dip_dst)
						break;
					else
						return 0;
				}
			default:
				/* If -b is not specified, sip and dip	*/
				/* must match src and dst IP respectively*/
				/* If -b is specified, capture packet	*/
				/* between sip and dip.			*/
				if (!twoway) {
					if (sip_src && dip_dst)
						break;
					else
						return 0;
				}
				else {
					if (srcmatch && dstmatch)
						break;
					else
						return 0;
				}
		}
	}
	if (port) {
		hlen = ipp->ip_hl << 2;
		if (ipp->ip_p == TCP_PROTO) {
			th = (struct tcphdr *) ((char *)ipp + hlen);
			if (th->th_sport != port && th->th_dport != port)
				return(0);
		}
		else if (ipp->ip_p == UDP_PROTO) {
			uh = (struct udphdr *) ((char *)ipp + hlen);
			if (uh->uh_sport != port && uh->uh_dport != port)
				return(0);
		}
		else
			return(0);
	}
	return(1);
}

open_tracefile(tracefile)
char			*tracefile;
{
	struct stat		statbuf;

	if ((fp = fopen(tracefile, "a+")) == (FILE *)NULL) {
 		fprintf(stderr, MSGSTR( OPEN,"iptrace: Can't open %s.\n"),
			tracefile);
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-1);
	}
	if (fstat(fileno(fp), &statbuf)) {
 		fprintf(stderr, MSGSTR(NOSTAT,"iptrace: Can't stat %s.\n"),
			tracefile);
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-1);
	}
	if (statbuf.st_size == 0) {
		if (fwrite(version,1, strlen(version), fp) != strlen(version)) {
 			fprintf(stderr,
				MSGSTR(CANTWRITE,"iptrace: Error writing to %s.\n"),
				tracefile);
			perror( MSGSTR( ERRORF, "\tError"));
			exit(-2);
		}
		fflush(fp);
	} else {
		fseek(fp, (long)0, SEEK_SET);
		if (fread(fversion,1, strlen(version), fp) != strlen(version)) {
 			fprintf(stderr, MSGSTR( CANTREAD, "iptrace: Error reading %s.\n"), 
				tracefile);
			perror( MSGSTR( ERRORF, "\tError"));
			exit(-2);
		}
		fseek(fp, (long)0, SEEK_END);
		if (strcmp(version, fversion)) {
			fprintf(stderr, 
 				MSGSTR(NOTTRACEF, "iptrace: %s is not a trace file.\n"), 
				tracefile);
			exit(-5);
		}
	}
}

fileit(packet, cc) 
u_char	*packet;
int	cc;
{
	time_t		curtime;
	static  int	flusher = 1;
	
	curtime = time(NULL);
	if (fwrite(&cc, sizeof(cc), 1, fp) != 1) {
 		fprintf(stderr, MSGSTR(CANTWRITET, "iptrace: Error writing to trace file.\n"));
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-2);
	}
	if (fwrite(&curtime, sizeof(curtime), 1, fp) != 1) {
 		fprintf(stderr, MSGSTR(CANTWRITET, "iptrace: Error writing to trace file.\n"));
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-2);
	}
	if (fwrite(packet, sizeof(u_char), cc, fp) != cc) {
 		fprintf(stderr, MSGSTR(CANTWRITET, "iptrace: Error writing to trace file.\n"));
		perror( MSGSTR( ERRORF, "\tError"));
		exit(-3);
	}
	if (flusher++ >= FLUSH_TIME) {
		fflush(fp);
		flusher = 1;
	}
}

intf_cfg(cmd)
int		cmd;
{
	int			error;
	char 			path[128];
	char 			module_symbol[128];
	struct device_req	device;
	struct cfg_load 	load;
	struct cfg_kmod 	kmod = { 0, CFG_INIT, 0, 0, };
	struct iftrace		info;


	(void) bzero(&info, sizeof(info));
	kmod.cmd = cmd;
	load.path = path;
	strcpy(path, NETDIR);
	strcat(path, "netintf");
	strcpy(module_symbol, "netintf");
	strcat(module_symbol, "_kmid");
	if ((kmod.kmid = check_load(module_symbol)) == 0) {
		if ((error = sysconfig(SYS_KLOAD, &load, sizeof(load))) 
                     == CONF_FAIL)
			return(error); 
		kmod.kmid = load.kmid; 
	}
	info.promisc = promisc;
	info.kmid = kmod.kmid;
	kmod.mdiptr = (caddr_t)(&info);
	kmod.mdilen = sizeof(info);
	
	/* call sysconfig() to config (or unconfig) the driver */
	error = sysconfig(SYS_CFGKMOD, &kmod, sizeof(kmod));

	return(error);
}

check_load(mod)
	char *mod;
{
	struct nlist nl;
	int kmid, kmem;

	nl.n_name = mod;
	knlist(&nl, 1, sizeof(struct nlist));
	if ((kmem = open("/dev/kmem", 0)) < 0) {
		perror(MSGSTR( NOOPENKMEM,"iptrace: Cannot open /dev/kmem -"));
		return(0);
	}
	if (nl.n_value == 0) {
		close(kmem);
		return(0);
	}
	if (lseek(kmem, nl.n_value, 0) == -1) {
		perror(MSGSTR( NOLSEEKKMEM, "iptrace: Cannot lseek on /dev/kmem -"));
		close(kmem);
		return(0);
	}
	if (read(kmem, &kmid, sizeof(kmid)) == -1) {
		perror(MSGSTR(NORDKMEM, "iptrace: Cannot read /dev/kmem -"));
		close(kmem);
		return(0);
	}
	close(kmem);
	return(kmid);
}

get_proto(proto_arg)
char		*proto_arg;
{
	struct protoent		*protop;

	if (isalpha(*proto_arg)) {
		protop = getprotobyname(proto_arg);
		if (protop == (struct protoent *)NULL) {
			fprintf(stderr, 
 				MSGSTR( NOPROTO,"iptrace : protocol \"%s\" not in /etc/protocols\n"), 
				proto_arg);
			exit(9);
		} else
			proto = protop->p_proto;
	} else
		proto = atoi(proto_arg);
}

get_host(name)
char 		*name;
{
        register struct hostent *hp;
        long    addr;

        if( (hp = gethostbyname( name )) == (struct hostent *)0 ) {
                if (isalpha(*name)) {
			fprintf(stderr,MSGSTR( NOHOSTN,"iptrace: hostname \"%s\" not found\n"), 
				name);
                        exit(1);
                }
                if( (addr = inet_addr( name )) == -1 ) {
			fprintf(stderr, MSGSTR( ILLGADDR,"iptrace: address \"%s\" misformed\n"),
				name );
                        exit(1);
                }
		return(addr);
        }
	return(*(long *)hp->h_addr);
}

get_port(port_arg)
char		*port_arg;
{
	struct servent		*sp;
	char			protoname[16];

	
	if (isalpha(*port_arg)) {
		sp = getservbyname(port_arg, NULL);
		if (sp == (struct servent *)NULL) {
			fprintf(stderr, 
 			MSGSTR( NOSERV,"iptrace : service \"%s\" not in /etc/services\n"), 
			port_arg);
			exit(9);
		} else
			port = sp->s_port;
	} else
		port = atoi(port_arg);
}

die() 
{
	set_flags(-IFF_DEBUG, ifname);
	fflush(fp);
	if (intf_cfg(CFG_TERM)) {
 		fprintf(stderr, MSGSTR( CANTOFFTRACE,"iptrace: Can't turn off tracing\n"));
		exit(-9);
	}
	exit(0);
}

header_to_type(packet, maybe_type, xmit_len, ifname)
char	*packet;
int	maybe_type;
int	*xmit_len;
char	*ifname;
{
	struct ether_header		*eh;
	struct ie5_mac_hdr		*mac5;
	struct ie3_mac_hdr		*mac3;
	struct ie2_llc_snaphdr		*llc;
	struct op_hdr                   *op;
	struct fddi_mac_hdr             *macf;

	switch(maybe_type) {
		case IFT_ETHER : 
			eh = (struct ether_header *)packet;
			*xmit_len = sizeof(struct ether_header);
			return(eh->ether_type);
			break;
		case IFT_ISO88025 :
			mac5 = (struct ie5_mac_hdr *)packet;
			llc = snd_mac_to_llc(mac5);
			*xmit_len = mac_size(mac5) + sizeof(*llc);
			return(llc->type);
			break;
		case IFT_ISO88023 :
			mac3 = (struct ie3_mac_hdr *)packet;
			llc =  (struct ie2_llc_snaphdr *)((mac3) + 1);
			*xmit_len = sizeof(*mac3) + sizeof(*llc);
			return(llc->type);
			break;
		case IFT_X25:
			*xmit_len = offsetof(struct x25_buffer, 
				body.user_data[0]);
			return(ETHERTYPE_IP);
			break;
		case IFT_FDDI:
			macf = (struct fddi_mac_hdr *)packet;
			llc = (struct ie2_llc_snaphdr *)mac_to_llc_f(macf);
			*xmit_len = mac_size_f(macf) + sizeof(*llc);
			return(llc->type);
			break;
		case IFT_PTPSERIAL:
		case 0:
			*xmit_len = 0;
			return(ETHERTYPE_IP);
			break; 
		case IFT_OTHER:
		default:
			if (!strncmp (ifname, "so", 2)) {
			    op = (struct op_hdr *)packet;
			    llc = (struct ie2_llc_snaphdr *)(&op -> snap);
			    *xmit_len = (int)sizeof(*op);
			    return(llc->type);
			}
			*xmit_len = 0;
			return(ETHERTYPE_IP);
			break;
	}
	/* NOTREACHED */
}

#define	BUFSIZE	4000
set_flags(value, name)
int		value;
char		*name;
{
	int s;
	char buf[BUFSIZE], *cp, *cplim;
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)	
		return(1);
	
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0)
		return(1);
	
	ifr = ifc.ifc_req;

#define size(p)	MAX((p).sa_len, sizeof(p))

	cplim = buf + ifc.ifc_len; /*skip over if's with big ifr_addr's */
	for (cp = buf; cp < cplim;
		            cp += sizeof(ifr->ifr_name) + size(ifr->ifr_addr)) {
		ifr = (struct ifreq *)cp;
		ifreq = *ifr;
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		if (name == NULL || (!strcmp(name, ifr->ifr_name)) )
			setifflag(s, &ifreq, value);
	}
	close(s);
}

setifflag(s, ifreqp, value)
int 		s;
struct ifreq	*ifreqp;
int		value;
{
	int	flags;

	if (ioctl(s, SIOCGIFFLAGS, (char *)ifreqp) < 0)
		return(1);

	flags = ifreqp->ifr_flags;
	
	if (value < 0) {
		value = -value;
		flags &= ~value;
	} else
		flags |= value;
	ifreqp->ifr_flags = flags;
	if (ioctl(s, SIOCSIFFLAGS, (char *)ifreqp) < 0)
		return(1);
}

#ifdef DEBUG
hex_dump(bp, cc)
u_char *bp;
int  cc;
{
        int     i;

        for ( i = 0 ; i < cc ; bp += 16, i += 16) {
                printf("%8.8x    ", i);
                dump_line(bp, MIN(16, cc - i));
        }
        printf("\n");
}

dump_line(bp, byte_count)
u_char          *bp;
int             byte_count;
{
        int     j;

        for ( j = 0 ; j < 16 ; j++) {
                if (j >= byte_count)
                        break;
                if (j % 4 == 0)
                        printf(" ");
                printf("%2.2x", bp[j]);
        }
        while ( j < 16 ) {
                if (j % 4 == 0)
                        printf(" ");
                printf("  ");
		j++;
        }
        printf("   \t|");
        for ( j = 0 ; j < 16 ; j++) {
                if (j >= byte_count)
                        break;
                printf("%c", isprint(bp[j]) ? bp[j] : '.');
        }
        while ( j++ < 16 )
                printf(" ");
        printf("|\n");
}

#endif DEBUG
