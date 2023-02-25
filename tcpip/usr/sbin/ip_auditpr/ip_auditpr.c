static char sccsid[] = "@(#)62        1.8  src/tcpip/usr/sbin/ip_auditpr/ip_auditpr.c, tcpras, tcpip411, GOLD410 3/8/94 17:50:34";
/* 
 * COMPONENT_NAME: TCPIP ip_auditpr.c
 * 
 * FUNCTIONS: Mip_auditpr, hardware_address_print, print_arp_struct, 
 *            print_ifreq_struct, print_indent, print_inpcb_struct, 
 *            print_kchange_configuration, print_kchange_route, 
 *            print_kconnection, print_kimport_kexport, 
 *            print_ksocket_create, print_rawcb_struct, print_rt_struct, 
 *            print_socket_af_inet, print_socket_af_unix, print_tail, 
 *            print_unpcb_struct 
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
 * Auditpr backend to format TCP/IP kernel tail records.
 * All (most?) of the bcopy's are needed to realign int's
 * and structures on word boundaries.  A quick 3 day hack...
 *
*/


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <net/raw_cb.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/in_netarp.h>
#include <sys/un.h>
#include <sys/unpcb.h>
#include <fcntl.h>
#include <sys/mbuf.h>

#include <nl_types.h>
#include "ip_auditpr_msg.h" 
nl_catd catd;
#define MSGSTR(n, s) catgets(catd, MS_ip_auditpr, n, s) 


#define	PROTOCOL_LENGTH 	sizeof("TCP/IP") /* TCP/IP, SNA, UUCP, etc.. */

#define	K_EVENT_CHANGE_CONFIGURATION		"TCPIP_kconfig"
#define	K_CHANGE_CONFIGURATION			0x0001
#define	K_EVENT_CHANGE_ROUTE			"TCPIP_kroute"
#define	K_CHANGE_ROUTE				0x0002
#define	K_EVENT_CONNECTION			"TCPIP_kconnect"
#define	K_CONNECTION				0x0004
#define	K_EVENT_IMPORT				"TCPIP_kdata_in"
#define	K_IMPORT				0x0008
#define	K_EVENT_EXPORT				"TCPIP_kdata_out"
#define	K_EXPORT				0x0010
#define	K_EVENT_SOCKET_CREATE			"TCPIP_kcreate"
#define	K_SOCKET_CREATE				0x0020

#define AUDIT_SIOCSARP			0x0001
#define AUDIT_SIOCDARP			0x0002
#define AUDIT_SIOCSARP_802_5		0x0004
#define AUDIT_SIOCDARP_802_5		0x0008
#define AUDIT_SIOCSARP_X25		0x0010
#define AUDIT_SIOCDARP_X25		0x0020
#define AUDIT_SIOCSIFMETRIC		0x0040
#define AUDIT_SIOCSIFFLAGS		0x0080
#define AUDIT_SIOCSIFNETMASK		0x0100
#define AUDIT_SIOCSIFADDR		0x0200
#define AUDIT_SIOCSIFBRDADDR		0x0400
#define AUDIT_SIOCSIFDSTADDR		0x0800
#define AUDIT_SIOCATTACH		0x1000
#define AUDIT_SIOCDETACH		0x2000
#define AUDIT_SIOCADDRT			0x4000
#define AUDIT_SIOCDELRT			0x8000

#define AUDIT_BIND			0x0001
#define AUDIT_ACCEPT			0x0002
#define AUDIT_CONNECT			0x0004
#define AUDIT_SHUTDOWN			0x0008
#define AUDIT_CLOSE			0x0010


int iflag = 0;
int eflag = 0;
int spaces = 0;

#include <locale.h>

main(argc,argv)
int argc;
char **argv;
{
	char *progname;
	char buf[BUFSIZ];
	int c;
	int kevent;
	char *event;
	extern char *optarg;

	setlocale(LC_ALL,"");
	catd = catopen(MF_IP_AUDITPR,NL_CAT_LOCALE);
	if ((progname = strrchr(argv[0], '/')) == NULL)
		progname = argv[0];
	else
		progname++;

	while ((c = getopt(argc, argv, "i:e:")) != EOF) {
		switch (c) {
			case 'i':
				iflag++;
				spaces = atoi(optarg);
				break;
			case 'e':
				eflag++;
				event = optarg;
				break;
			default:
				fprintf(stdout,  MSGSTR(M_MSG_1, "usage: %s [-i <spaces>] -e audit_event\n") , argv[0]);
				exit(1);
		}
	}

	if (strcmp(event, K_EVENT_CHANGE_CONFIGURATION) == 0)
		kevent = K_CHANGE_CONFIGURATION;
	else if (strcmp(event, K_EVENT_CHANGE_ROUTE) == 0)
		kevent = K_CHANGE_ROUTE;
	else if (strcmp(event, K_EVENT_CONNECTION) == 0)
		kevent = K_CONNECTION;
	else if (strcmp(event, K_EVENT_IMPORT) == 0)
		kevent = K_IMPORT;
	else if (strcmp(event, K_EVENT_EXPORT) == 0)
		kevent = K_EXPORT;
	else if (strcmp(event, K_EVENT_SOCKET_CREATE) == 0)
		kevent = K_SOCKET_CREATE;
	else {
		fprintf(stdout,  MSGSTR(M_MSG_2, "Non-supported TCP/IP kernel event name!\n") );
		exit(1);
	}

	print_tail(kevent);
	exit(0);

}

print_indent(spaces)
register int spaces;
{
	for(; spaces > 0; spaces--)
		fprintf(stdout, " ");
}

print_tail(kevent)
register int kevent;
{
	char tail_buf[BUFSIZ];
	int tail_buf_size;

	/* read data from auditpr pipe */
	if ((tail_buf_size = read(0, (char *) tail_buf, BUFSIZ)) < 0) {
		perror("read");
		exit(1);
	}

	if (iflag)
		print_indent(spaces);

	switch (kevent) {
		case K_CHANGE_CONFIGURATION:
			print_kchange_configuration(tail_buf);
			break;

		case K_CHANGE_ROUTE:
			print_kchange_route(tail_buf);
			break;

		case K_CONNECTION:
			print_kconnection(tail_buf);
			break;

		case K_IMPORT:
		case K_EXPORT:
			print_kimport_kexport(tail_buf);
			break;

		case K_SOCKET_CREATE:
			print_ksocket_create(tail_buf);
			break;

		default:
			fprintf(stdout,  MSGSTR(M_MSG_4, "Unknown TCP/IP audit event!\n") );
			exit(1);
	}
	fprintf(stdout, "\n");
	exit(0);
}

print_kconnection(tail)
char *tail;
{
	char protocol[64];
	int domain;
	int operation;

	strncpy(protocol, tail, PROTOCOL_LENGTH);
	tail += PROTOCOL_LENGTH;
	fprintf(stdout, "%s ", protocol);

	bcopy(tail, (char *) &domain, sizeof(domain));
	tail += sizeof(int);

	bcopy(tail, (char *) &operation, sizeof(operation));
	tail += sizeof(int);

	switch (domain) {
		case AF_UNIX:
			fprintf(stdout, "AF_UNIX ");
			switch (operation) {
				case AUDIT_BIND:
					fprintf(stdout, "BIND ");
					break;
				case AUDIT_ACCEPT:
					fprintf(stdout, "ACCEPT ");
					break;
				case AUDIT_CONNECT:
					fprintf(stdout, "CONNECT ");
					break;
				case AUDIT_SHUTDOWN:
					fprintf(stdout, "SHUTDOWN ");
					break;
				case AUDIT_CLOSE:
					fprintf(stdout, "CLOSE ");
					break;
				default:
					fprintf(stdout,  MSGSTR(M_MSG_13, "Unknown kernel socket operation!\n") );
					exit(1);
			}
			print_socket_af_unix(tail);
			break;
		case AF_INET:
			fprintf(stdout, "AF_INET ");
			switch (operation) {
				case AUDIT_BIND:
					fprintf(stdout, "BIND ");
					break;
				case AUDIT_ACCEPT:
					fprintf(stdout, "ACCEPT ");
					break;
				case AUDIT_CONNECT:
					fprintf(stdout, "CONNECT ");
					break;
				case AUDIT_SHUTDOWN:
					fprintf(stdout, "SHUTDOWN ");
					break;
				case AUDIT_CLOSE:
					fprintf(stdout, "CLOSE ");
					break;
				default:
					fprintf(stdout,  MSGSTR(M_MSG_20, "Unknown kernel socket operation!\n") );
					exit(1);
			}
			print_socket_af_inet(tail);
			break;
		default:
			fprintf(stdout,  MSGSTR(M_MSG_21, "Unknown kernel protocol domain!\n") );
			exit(1);
	}
}

print_socket_af_unix(tail)
char *tail;
{
	struct sockaddr_un *sun;
	int buf_addr_len;
	struct mbuf mbuf;

	bcopy(tail, (char *) &buf_addr_len, sizeof(int));
	if (buf_addr_len == 0) {
		fprintf(stdout,  MSGSTR(M_MSG_22, "No path name") );
		return(0);
	}
	tail += sizeof(int);
	sun = mtod(((struct mbuf *) &mbuf), struct sockaddr_un *);
	bcopy(tail, (char *) &mbuf, buf_addr_len);
	fprintf(stdout, "%s", sun->sun_path);
}

print_socket_af_inet(tail)
char *tail;
{
	struct sockaddr_in sin;
	int buf_addr_len;

	bcopy(tail, (char *) &buf_addr_len, sizeof(int));
	if (buf_addr_len == 0) {
		fprintf(stdout,  MSGSTR(M_MSG_24, "No IP address available") );
		return(0);
	}
	tail += sizeof(int);
	bcopy(tail, (char *) &sin, buf_addr_len);
	fprintf(stdout, "%s ", inet_ntoa(sin.sin_addr));
	fprintf(stdout, "%d", sin.sin_port);
}

print_kchange_route(tail)
register char *tail;
{
	char protocol[64];
	int socket_ioctl;

	strncpy(protocol, tail, PROTOCOL_LENGTH);
	tail += PROTOCOL_LENGTH;
	fprintf(stdout, "%s ", protocol);

	bcopy(tail, (char *) &socket_ioctl, sizeof(int));
	tail += sizeof(int);

	switch (socket_ioctl) {
		case AUDIT_SIOCADDRT:
			fprintf(stdout, "SIOCADDRT ");
			print_rt_struct(tail);
			break;
		case AUDIT_SIOCDELRT:
			fprintf(stdout, "SIOCDELRT ");
			print_rt_struct(tail);
			break;
		default:
			fprintf(stdout,  MSGSTR(M_MSG_30, "Unknown kernel TCP/IP ioctl!\n") );
			exit(1);
	}
}

print_kchange_configuration(tail)
register char *tail;
{
	char protocol[64];
	int socket_ioctl;

	strncpy(protocol, tail, PROTOCOL_LENGTH);
	tail += PROTOCOL_LENGTH;
	fprintf(stdout, "%s ", protocol);

	bcopy(tail, (char *) &socket_ioctl, sizeof(int));
	tail += sizeof(int);

	switch (socket_ioctl) {
		case AUDIT_SIOCSARP:
			fprintf(stdout, "SIOCSARP ");
			print_arp_struct(tail);
			break;
		case AUDIT_SIOCDARP:
			fprintf(stdout, "SIOCDARP ");
			print_arp_struct(tail);
			break;
		case AUDIT_SIOCSARP_802_5:
			fprintf(stdout, "SIOCSARP_802_5 ");
			print_arp_struct(tail);
			break;
		case AUDIT_SIOCDARP_802_5:
			fprintf(stdout, "SIOCDARP_802_5 ");
			print_arp_struct(tail);
			break;
		case AUDIT_SIOCSARP_X25:
			fprintf(stdout, "SIOCSARP_X25 ");
			print_arp_struct(tail);
			break;
		case AUDIT_SIOCDARP_X25:
			fprintf(stdout, "SIOCDARP_X25 ");
			print_arp_struct(tail);
			break;
		case AUDIT_SIOCSIFMETRIC:
			fprintf(stdout, "SIOCSIFMETRIC ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCSIFFLAGS:
			fprintf(stdout, "SIOCSIFFLAGS ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCSIFNETMASK:
			fprintf(stdout, "SIOCSIFNETMASK ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCSIFADDR:
			fprintf(stdout, "SIOCSIFADDR ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCSIFBRDADDR:
			fprintf(stdout, "SIOCSIFBRDADDR ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCSIFDSTADDR:
			fprintf(stdout, "SIOCSIFDSTADDR ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCATTACH:
			fprintf(stdout, "SIOCATTACH ");
			print_ifreq_struct(tail);
			break;
		case AUDIT_SIOCDETACH:
			fprintf(stdout, "SIOCDETACH ");
			print_ifreq_struct(tail);
			break;
		default:
			fprintf(stdout,  MSGSTR(M_MSG_46, "Unknown kernel TCP/IP ioctl!\n") );
			exit(1);
	}
}

print_rt_struct(rt_data)
register caddr_t rt_data;
{
	struct rtentry rtentry;
	struct sockaddr_in *sin_dst;
	struct sockaddr_in *sin_gateway;

	bcopy(rt_data, (char *) &rtentry, sizeof(struct rtentry));
	sin_dst = rt_key(&rtentry);
	sin_gateway = (struct sockaddr_in *) &rtentry.rt_gateway;

	fprintf(stdout, "%s ", inet_ntoa(sin_dst->sin_addr));
	fprintf(stdout, "%s ", inet_ntoa(sin_gateway->sin_addr));

	if (rtentry.rt_flags & RTF_UP)
		fprintf(stdout,  MSGSTR(M_MSG_49, "UP ") );
	else
		fprintf(stdout,  MSGSTR(M_MSG_50, "DOWN ") );

	if (rtentry.rt_flags & RTF_HOST)
		fprintf(stdout,  MSGSTR(M_MSG_51, "HOST ROUTE") );
	else
		fprintf(stdout,  MSGSTR(M_MSG_52, "NET ROUTE") );
}

print_ifreq_struct(ifreq_tail)
register caddr_t ifreq_tail;
{
	struct ifreq ifreq;
	struct sockaddr_in *sin;

	bcopy(ifreq_tail, (char *) &ifreq, sizeof(struct ifreq));
	sin = (struct sockaddr_in *) &(ifreq.ifr_addr);

	fprintf(stdout, "%s ", ifreq.ifr_name);
	fprintf(stdout, "%s", inet_ntoa(sin->sin_addr));
}

print_arp_struct(arp_data)
register caddr_t arp_data;
{
	struct arpreq ar;
	struct sockaddr_in *sin;

	bcopy(arp_data, (char *) &ar, sizeof(struct arpreq));
	sin = (struct sockaddr_in *)&ar.arp_pa;

	fprintf(stdout, "%s ", inet_ntoa(sin->sin_addr));
	hardware_address_print((char *) ar.arp_ha.sa_data);

}

hardware_address_print(cp)
caddr_t cp;
{
	fprintf(stdout, "%x:%x:%x:%x:%x:%x",
		 cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
}

print_kimport_kexport(tail)
register caddr_t tail;
{
	char protocol[64];
	int domain;
	short type;

	strncpy(protocol, tail, PROTOCOL_LENGTH);
	tail += PROTOCOL_LENGTH;
	fprintf(stdout, "%s ", protocol);

	bcopy(tail, (char *) &domain, sizeof(domain));
	tail += sizeof(int);

	bcopy(tail, (char *) &type, sizeof(short));
	tail += sizeof(short);

	switch (domain) {
	case AF_INET:
		fprintf(stdout, "AF_INET ");
		switch (type) {
			case SOCK_STREAM:
				fprintf(stdout, "SOCK_STREAM ");
				print_inpcb_struct(tail);
				break;
			case SOCK_DGRAM:
				fprintf(stdout, "SOCK_DGRAM ");
				print_inpcb_struct(tail);
				break;
			case SOCK_RAW:
				fprintf(stdout, "SOCK_RAW ");
				print_rawcb_struct(tail);
				break;
			default:
				fprintf(stdout,  MSGSTR(M_MSG_62, "Unknown kernel socket type!\n") );
				exit(1);
		}
		break;
	case AF_UNIX:
		fprintf(stdout, "AF_UNIX ");
		switch (type) {
			case SOCK_STREAM:
			case SOCK_DGRAM:
				print_unpcb_struct(tail);
				break;
			default:
				fprintf(stdout, MSGSTR(M_MSG_64, "Unknown kernel socket type!\n") );
				exit(1);
		}
		break;
	default:
		fprintf(stdout,  MSGSTR(M_MSG_65, "Unknown kernel protocol domain!\n") );
		exit(1);
	}
}

print_unpcb_struct(tail)
register caddr_t tail;
{
	struct unpcb unpcb;
	struct sockaddr_un *sun;

	bcopy(tail, (char *) &unpcb, sizeof(struct unpcb));
	/* Should something be printed???? */
	/* No ports with raw sockets */
}

print_rawcb_struct(tail)
register caddr_t tail;
{
	struct rawcb rawcb;
	struct sockaddr_in *sin;

	bcopy(tail, (char *) &rawcb, sizeof(struct rawcb));
	sin = (struct sockaddr_in *) &rawcb.rcb_faddr;
	fprintf(stdout, "%s", inet_ntoa(sin->sin_addr));
	/* No ports with raw sockets */
}

print_inpcb_struct(tail)
register caddr_t tail;
{
	struct inpcb inpcb;

	bcopy(tail, (char *) &inpcb, sizeof(struct inpcb));
	fprintf(stdout, "%s %d %d", inet_ntoa(inpcb.inp_faddr),
			inpcb.inp_fport, inpcb.inp_lport);
}

print_ksocket_create(tail)
register caddr_t tail;
{
	char protocol[64];
	int domain;
	int type;
	int proto;

	strncpy(protocol, tail, PROTOCOL_LENGTH);
	tail += PROTOCOL_LENGTH;
	fprintf(stdout, "%s ", protocol);

	bcopy(tail, (char *) &domain, sizeof(domain));
	tail += sizeof(int);

	bcopy(tail, (char *) &type, sizeof(type));
	tail += sizeof(int);

	bcopy(tail, (char *) &proto, sizeof(proto));
	tail += sizeof(int);

	switch (domain) {
	case AF_INET:
		fprintf(stdout, "AF_INET ");
		break;
	case AF_UNIX:
		fprintf(stdout, "AF_UNIX ");
		break;
	default:
		fprintf(stdout,  MSGSTR(M_MSG_71, "Unknown kernel protocol domain!\n") );
		exit(1);
	}

	switch (type) {
	case SOCK_STREAM:
		fprintf(stdout, "SOCK_STREAM ");
		break;
	case SOCK_DGRAM:
		fprintf(stdout, "SOCK_DGRAM ");
		break;
	case SOCK_RAW:
		fprintf(stdout, "SOCK_RAW ");
		break;
	default:
		fprintf(stdout,  MSGSTR(M_MSG_75, "Unknown kernel socket type!\n") );
		exit(1);
	}

	switch (proto) {
	case PF_UNSPEC:
		fprintf(stdout, "UNSPEC");
		break;
	case PF_INET:
		fprintf(stdout, "PF_INET");
		break;
	case PF_UNIX:
		fprintf(stdout, "PF_UNIX");
		break;
	default:
		fprintf(stdout,  MSGSTR(M_MSG_79, "Unknown kernel socket protocol!\n") );
		exit(1);
	}
}
