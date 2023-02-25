static char sccsid[] = "@(#)86	1.9.1.8  src/bos/usr/sbin/netstat/inet.c, cmdnet, bos411, 9435B411a 8/30/94 17:45:41";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: MSGSTR
 *		icmp_stats
 *		igmp_stats
 *		inetname
 *		inetprint
 *		ip_stats
 *		ndx
 *		ndx2
 *		protopr
 *		tcp_stats
 *		udp_stats
 *
 *   ORIGINS: 26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/machine.h>
#include <sys/file.h>

#include <net/nh.h>
#include <net/route.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#ifdef IP_MULTICAST
#include <netinet/igmp_var.h>
#endif
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_seq.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <netdb.h>

#include <stdio.h>
#include <string.h>
#include <nlist.h>
#undef	n_name

#ifndef IP_MULTICAST /* really not needed here */
/* if this changes, change the one in tcp_usrreq.c also */
char *tcpstates[] = {
	"CLOSED",	"LISTEN",	"SYN_SENT",	"SYN_RCVD",
	"ESTABLISHED",	"CLOSE_WAIT",	"FIN_WAIT_1",	"CLOSING",
	"LAST_ACK",	"FIN_WAIT_2",	"TIME_WAIT"
};
#endif
struct	inpcb inpcb;
struct	tcpcb tcpcb;
struct	socket sockb;
extern	int Aflag;
extern	int aflag;
extern	int nflag;
extern	int sflag;
extern	int Zflag;
extern	int kmem;
extern	char *plural();

char	*inetname();
extern struct nlist netstatnl[];
#define N_IPINTRQ	32	/* XXX - must match define in main.c */

#include <nl_types.h>
#include "netstat_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_NETSTAT,n,s) 

/*
 * these are used throughout to provide indicies into
 * the message table
 *
 * ORDER IN THE TABLE IS IMPORTANT: SINGLE FIRST, THEN PLURAL
 */

#define	ndx(f)		((f) == 1 ? 0 : 1)
#define	ndx2(f1,f2)	((f1) == 1 && (f2) == 1 ? 0 : \
			 (f1) != 1 && (f2) == 1 ? 1 : \
			 (f1) == 1 && (f2) != 1 ? 2 : 3)

/*
 * Print a summary of connections related to an Internet
 * protocol.  For TCP, also give state of connection.
 * Listening processes (aflag) are suppressed unless the
 * -a (all) flag is specified.
 */
protopr(off, name)
	off_t off;
	char *name;
{
	struct inpcb cb;
	register struct inpcb *prev, *next;
	int istcp;
	static int first = 1;

	if (off == 0)
		return;
	istcp = strcmp(name, "tcp") == 0;
	kvm_read(off, (char *)&cb, sizeof (struct inpcb));
	inpcb = cb;
	prev = (struct inpcb *)off;
	if (inpcb.inp_next == (struct inpcb *)off)
		return;
	while (inpcb.inp_next != (struct inpcb *)off) {

		next = inpcb.inp_next;
		kvm_read((off_t)next, (char *)&inpcb, sizeof (inpcb));
		if (inpcb.inp_prev != prev) {
			printf("???\n");
			break;
		}
		if (!aflag &&
		  inet_lnaof(inpcb.inp_laddr) == INADDR_ANY) {
			prev = next;
			continue;
		}
		kvm_read((off_t)inpcb.inp_socket,
				(char *)&sockb, sizeof (sockb));
		if (istcp) {
			kvm_read((off_t)inpcb.inp_ppcb,
				(char *)&tcpcb, sizeof (tcpcb));
		}
		if (first) {
			printf(MSGSTR(ACT_INT_CONN, "Active Internet connections"));
			if (aflag)
				printf(MSGSTR(INCL_SRVRS, " (including servers)"));
			putchar('\n');
			if (Aflag)
				printf(MSGSTR(PCB, "PCB/ADDR "));
			if (Aflag)
				printf(MSGSTR(LA_MSG1, "Proto Recv-Q Send-Q  Local Address      Foreign Address    (state)\n"));
			else
				printf(MSGSTR(LA_MSG2, "Proto Recv-Q Send-Q  Local Address          Foreign Address        (state)\n"));
			first = 0;
		}
		if (Aflag)
			if (istcp)
				printf("%8x ", inpcb.inp_ppcb);
			else
				printf("%8x ", next);
		printf("%-5.5s %6d %6d ", name, sockb.so_rcv.sb_cc,
			sockb.so_snd.sb_cc);
		inetprint(&inpcb.inp_laddr, inpcb.inp_lport, name);
		inetprint(&inpcb.inp_faddr, inpcb.inp_fport, name);
		if (istcp) {
			if (tcpcb.t_state < 0 || tcpcb.t_state >= TCP_NSTATES)
				printf(" %d", tcpcb.t_state);
			else
				printf(" %s", tcpstates[tcpcb.t_state]);
		}
		putchar('\n');
		prev = next;
	}
}

/*
 * Dump TCP statistics structure.
 */
tcp_stats(off, name)
	off_t off;
	char *name;
{
	struct tcpstat tcpstat;

	if (off == 0)
		return;
	if (Zflag) {
		bzero(&tcpstat, sizeof(tcpstat));
		(void) lseek(kmem, off, 0);
		if (write(kmem, &tcpstat, sizeof(tcpstat)) != sizeof(tcpstat))
			perror("write");
		return;
	}
	printf ("%s:\n", name);
	kvm_read(off, (char *)&tcpstat, sizeof (tcpstat));

#define	p(f, i, m) if (tcpstat.f || sflag <= 1) \
    printf(MSGSTR(i+ndx(tcpstat.f),m), \
	tcpstat.f, plural(tcpstat.f))
#define p2(f1, f2, i, m) if (tcpstat.f1 || tcpstat.f2 || sflag <= 1) \
    printf(MSGSTR(i+ndx2(tcpstat.f1,tcpstat.f2),m), \
	tcpstat.f1, plural(tcpstat.f1), \
	tcpstat.f2, plural(tcpstat.f2))

	p(tcps_sndtotal, PACK_SENT, "\t%u packet%s sent\n");
	p2(tcps_sndpack,tcps_sndbyte, PACK_BYTE,
		"\t\t%u data packet%s (%u byte%s)\n");
	p2(tcps_sndrexmitpack, tcps_sndrexmitbyte, PACK_RXMIT,
		"\t\t%u data packet%s (%u byte%s) retransmitted\n");
	p2(tcps_sndacks, tcps_delack, ACK_PACK,
		"\t\t%u ack-only packet%s (%u delayed)\n");
	p(tcps_sndurg, URG_PACK, "\t\t%u URG only packet%s\n");
	p(tcps_sndprobe, WPROBE_PACK, "\t\t%u window probe packet%s\n");
	p(tcps_sndwinup, WUPDT_PACK, "\t\t%u window update packet%s\n");
	p(tcps_sndctrl, CTRL_PACK, "\t\t%u control packet%s\n");
	p(tcps_rcvtotal, PACK_RCV, "\t%u packet%s received\n");
	p2(tcps_rcvackpack, tcps_rcvackbyte, ACK4_BYTE, "\t\t%u ack%s (for %u byte%s)\n");
	p(tcps_rcvdupack, DUP_ACK, "\t\t%u duplicate ack%s\n");
	p(tcps_rcvacktoomuch, UNSENT_ACK, "\t\t%u ack%s for unsent data\n");
	p2(tcps_rcvpack, tcps_rcvbyte, RCV_INSEQ,
		"\t\t%u packet%s (%u byte%s) received in-sequence\n");
	p2(tcps_rcvduppack, tcps_rcvdupbyte, DUP_PACK,
		"\t\t%u completely duplicate packet%s (%u byte%s)\n");
	p2(tcps_rcvpartduppack, tcps_rcvpartdupbyte, DUP_DATA,
		"\t\t%u packet%s with some dup. data (%u byte%s duped)\n");
	p2(tcps_rcvoopack, tcps_rcvoobyte, OUT_ORDER,
		"\t\t%u out-of-order packet%s (%u byte%s)\n");
	p2(tcps_rcvpackafterwin, tcps_rcvbyteafterwin, DATA_AFTWIN,
		"\t\t%u packet%s (%u byte%s) of data after window\n");
	p(tcps_rcvwinprobe, WIN_PROBE, "\t\t%u window probe%s\n");
	p(tcps_rcvwinupd, WIN_UPD, "\t\t%u window update packet%s\n");
	p(tcps_rcvafterclose, AFTER_CLOSE, "\t\t%u packet%s received after close\n");
	p(tcps_rcvbadsum, BAD_SUM, "\t\t%u discarded for bad checksum%s\n");
	p(tcps_rcvbadoff, BAD_OFF, "\t\t%u discarded for bad header offset field%s\n");
	p(tcps_rcvshort, BAD_SHORT, "\t\t%u discarded because packet too short\n");
	p(tcps_connattempt, CONN_REQ, "\t%u connection request%s\n");
	p(tcps_accepts, CONN_ACC, "\t%u connection accept%s\n");
	p(tcps_connects, CONN_EST, "\t%u connection%s established (including accepts)\n");
	p2(tcps_closed, tcps_drops, CONN_CLOSE,
		"\t%u connection%s closed (including %u drop%s)\n");
	p(tcps_conndrops, CONN_DRP, "\t%u embryonic connection%s dropped\n");
	p2(tcps_rttupdated, tcps_segstimed, UPD_RTT,
		"\t%u segment%s updated rtt (of %u attempt%s)\n");
	p(tcps_rexmttimeo, XMIT_TOUT, "\t%u retransmit timeout%s\n");
	p(tcps_timeoutdrop, CONN_DRP_XMIT, "\t\t%u connection%s dropped by rexmit timeout\n");
	p(tcps_persisttimeo, PERS_TOUT, "\t%u persist timeout%s\n");
	p(tcps_keeptimeo, KEEP_TOUT, "\t%u keepalive timeout%s\n");
	p(tcps_keepprobe, KEEP_PROBE, "\t\t%u keepalive probe%s sent\n");
	p(tcps_keepdrops, KEEP_DROP, "\t\t%u connection%s dropped by keepalive\n");
#undef p
#undef p2
}

/*
 * Dump UDP statistics structure.
 */
udp_stats(off, name)
	off_t off;
	char *name;
{
	struct udpstat udpstat;
	u_long delivered;

	if (off == 0)
		return;
	if (Zflag) {
		bzero(&udpstat, sizeof(udpstat));
		(void) lseek(kmem, off, 0);
		if (write(kmem, &udpstat, sizeof(udpstat)) != sizeof(udpstat))
			perror("write");
		return;
	}
	kvm_read(off, (char *)&udpstat, sizeof (udpstat));
	printf("%s:\n", name);

#define	p(f, m) if (udpstat.f || sflag <= 1) \
    printf(m, udpstat.f, plural(udpstat.f))

	p(udps_ipackets, MSGSTR(UDP_RECV+ndx(udpstat.udps_ipackets),
		"\t%u datagram%s received\n"));
	if (udpstat.udps_hdrops || sflag <= 1)
		printf(MSGSTR(INCMPLT_HDR+ndx(udpstat.udps_hdrops),
		"%s\t%u incomplete header%s\n"), "", udpstat.udps_hdrops, 
		plural(udpstat.udps_hdrops));
	p(udps_badlen, MSGSTR(BAD_DATA_LEN+ndx(udpstat.udps_badlen),
		"\t%u bad data length field%s\n"));
	p(udps_badsum, MSGSTR(BAD_CHKSUM+ndx(udpstat.udps_badsum),
		"\t%u bad checksum%s\n"));
	p(udps_noport, MSGSTR(UDP_DROPPED, "\t%u dropped due to no socket\n"));
	p(udps_noportbcast,
	   MSGSTR(UDP_DROPPED_MULTI+ndx(udpstat.udps_noportbcast),
	   "\t%u broadcast/multicast datagram%s dropped due to no socket\n"));
	p(udps_fullsock, MSGSTR(UDP_OVERFLOW, "\t%u dropped due to full socket buffers\n"));
	delivered = udpstat.udps_ipackets -
		    udpstat.udps_hdrops -
		    udpstat.udps_badlen -
		    udpstat.udps_badsum -
		    udpstat.udps_noport -
		    udpstat.udps_noportbcast -
		    udpstat.udps_fullsock;
	if (delivered || sflag <= 1)
		printf(MSGSTR(UDP_DELIVERED, "\t%u delivered\n"), delivered);
	p(udps_opackets, MSGSTR(UDP_OUTPUT+ndx(udpstat.udps_opackets),
		"\t%u datagram%s output\n"));
#undef p
}

/*
 * Dump IP statistics structure.
 */
ip_stats(off, name)
	off_t off;
	char *name;
{
	struct ipstat ipstat;
	struct ifqueue ipintrq;

	if (off == 0)
		return;
	if (Zflag) {
		bzero(&ipstat, sizeof(ipstat));
		(void) lseek(kmem, off, 0);
		if (write(kmem, &ipstat, sizeof(ipstat)) != sizeof(ipstat))
			perror("write");
		return;
	}
	kvm_read(off, (char *)&ipstat, sizeof (ipstat));
	printf("%s:\n", name);

#define	p(f, m) if (ipstat.f || sflag <= 1) \
    printf(m, ipstat.f, plural(ipstat.f))

	if (ipstat.ips_total || sflag <= 1)
		printf(MSGSTR(TOT_PACK_RCVD, "%s\t%u total packets received\n"),
		"", ipstat.ips_total, plural(ipstat.ips_total));
	p(ips_badsum, MSGSTR(BAD_HDR_CHKSUM+ndx(ipstat.ips_badsum),
		"\t%u bad header checksum%s\n"));
	p(ips_tooshort, MSGSTR(SZ_TOO_SMALL, "\t%u with size smaller than minimum\n"));
	p(ips_toosmall, MSGSTR(DSIZE_LESS_LEN, "\t%u with data size < data length\n"));
	p(ips_badhlen, MSGSTR(HLEN_LESS_DSIZE, "\t%u with header length < data size\n"));
	p(ips_badlen, MSGSTR(DLEN_LESS_HLEN, "\t%u with data length < header length\n"));
	p(ips_badoptions, MSGSTR(IP_BAD_OPT, "\t%u with bad options\n"));
	p(ips_badvers, MSGSTR(IP_BAD_VER, "\t%u with incorrect version number\n"));
	p(ips_fragments, MSGSTR(FRAG_RCVD+ndx(ipstat.ips_fragments), "\t%u fragment%s received\n"));
	p(ips_fragdropped, MSGSTR(OUT_OF_SP+ndx(ipstat.ips_fragdropped), "\t%u fragment%s dropped (dup or out of space)\n"));
	p(ips_fragtimeout, MSGSTR(FRAG_DRPD_TOUT+ndx(ipstat.ips_fragtimeout), "\t%u fragment%s dropped after timeout\n"));
	p(ips_reassembled, MSGSTR(IP_REASS+ndx(ipstat.ips_reassembled),
		"\t%u packet%s reassembled ok\n"));
	p(ips_delivered, MSGSTR(IP_FOR_US+ndx(ipstat.ips_delivered),
		"\t%u packet%s for this host\n"));
	p(ips_noproto, MSGSTR(IP_BAD_PROTO+ndx(ipstat.ips_noproto),
		"\t%u packet%s for unknown/unsupported protocol\n"));
	p(ips_forward, MSGSTR(PACKET_FWD+ndx(ipstat.ips_forward), "\t%u packet%s forwarded\n"));
	p(ips_cantforward, MSGSTR(PACKET_NO_FWD+ndx(ipstat.ips_cantforward), "\t%u packet%s not forwardable\n"));
	p(ips_redirectsent, MSGSTR(REDIRECT_SNT+ndx(ipstat.ips_redirectsent), "\t%u redirect%s sent\n"));
	p(ips_localout, MSGSTR(IP_FROM_US+ndx(ipstat.ips_localout),
		"\t%u packet%s sent from this host\n"));
	p(ips_rawout, MSGSTR(IP_RAWOUT+ndx(ipstat.ips_rawout),
		"\t%u packet%s sent with fabricated ip header\n"));
	p(ips_odropped, MSGSTR(IP_ODROPPED+ndx(ipstat.ips_odropped),
		"\t%u output packet%s dropped due to no bufs, etc.\n"));
	p(ips_noroute, MSGSTR(IP_ODISCARD+ndx(ipstat.ips_noroute),
		"\t%u output packet%s discarded due to no route\n"));
	p(ips_fragmented, MSGSTR(IP_FRAGED+ndx(ipstat.ips_fragmented),
		"\t%u output datagram%s fragmented\n"));
	p(ips_ofragments, MSGSTR(IP_OFRAG+ndx(ipstat.ips_ofragments),
		"\t%u fragment%s created\n"));
	p(ips_cantfrag, MSGSTR(IP_CANT_FRAG+ndx(ipstat.ips_cantfrag),
		"\t%u datagram%s that can't be fragmented\n"));
	p(ipInMAddrErrors, MSGSTR(IP_MULTI_DROP+ndx(ipstat.ipInMAddrErrors),
		"\t%u IP Multicast packet%s dropped due to no receiver\n"));
	off = netstatnl[N_IPINTRQ].n_value;
	kvm_read(off, (char *)&ipintrq, sizeof (ipintrq));
	if (ipintrq.ifq_drops || sflag <= 1)
		printf(MSGSTR(IP_OVERFLOW+ndx(ipintrq.ifq_drops),
			"\t%u ipintrq overflow%s\n"), ipintrq.ifq_drops, 
			plural(ipintrq.ifq_drops));
#undef p
}

struct {
	int	msgid;
	char	*name;
} icmpnames[] = {
	ECHO_REPLY,	"echo reply",
	ONE,		"#1",
	TWO,		"#2",
	DEST_UNRCH,	"destination unreachable",
	SRC_QUENCH,	"source quench",
	RT_REDIRECT,	"routing redirect",
	SIX,		"#6",
	SEVEN,		"#7",
	ECHO,		"echo",
	NINE,		"#9",
	TEN,		"#10",
	TIME_EXCEED,	"time exceeded",
	PARAM_PROB,	"parameter problem",
	TIMESTAMP,	"time stamp",
	TIMEST_REPLY,	"time stamp reply",
	INFO_REQ,	"information request",
	INFO_REQ_REPLY,	"information request reply",
	ADDR_REQ,	"address mask request",
	ADDR_REPLY,	"address mask reply",
};

/*
 * Dump ICMP statistics.
 */
icmp_stats(off, name)
	off_t off;
	char *name;
{
	struct icmpstat icmpstat;
	register int i, first;

	if (off == 0)
		return;
	if (Zflag) {
		bzero(&icmpstat, sizeof(icmpstat));
		(void) lseek(kmem, off, 0);
		if (write(kmem, &icmpstat, sizeof(icmpstat)) != sizeof(icmpstat))
			perror("write");
		return;
	}
	kvm_read(off, (char *)&icmpstat, sizeof (icmpstat));

#define	p(f, m) if (icmpstat.f || sflag <= 1) \
    printf(m, icmpstat.f, plural(icmpstat.f))

	printf(MSGSTR(CALLS_TO_ICMP+ndx(icmpstat.icps_error), "%s:\n\t%u call%s to icmp_error\n"), name,
		icmpstat.icps_error, plural(icmpstat.icps_error));
	p(icps_oldicmp, MSGSTR(ERR_NO_GEN+ndx(icmpstat.icps_oldicmp), 
		"\t%u error%s not generated 'cuz old message was icmp\n"));
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_outhist[i] != 0) {
			if (first) {
				printf(MSGSTR(OTPT_HISTOGRAM, "\tOutput histogram:\n"));
				first = 0;
			}
			printf("\t\t%s: %u\n", MSGSTR(icmpnames[i].msgid, icmpnames[i].name),
				icmpstat.icps_outhist[i]);
		}
	p(icps_badcode, MSGSTR(BAD_CODE_FIELD+ndx(icmpstat.icps_badcode), 
		"\t%u message%s with bad code fields\n"));
	p(icps_tooshort, MSGSTR(MSG_LESS_MINLN+ndx(icmpstat.icps_tooshort),
		"\t%u message%s < minimum length\n"));
	p(icps_checksum, MSGSTR(BAD_CHKSUM+ndx(icmpstat.icps_checksum), 
		"\t%u bad checksum%s\n"));
	p(icps_badlen, MSGSTR(MSG_BAD_LEN+ndx(icmpstat.icps_badlen), 
		"\t%u message%s with bad length\n"));
	for (first = 1, i = 0; i < ICMP_MAXTYPE + 1; i++)
		if (icmpstat.icps_inhist[i] != 0) {
			if (first) {
				printf(MSGSTR(INPT_HISTOGRAM, "\tInput histogram:\n"));
				first = 0;
			}
			printf("\t\t%s: %u\n", MSGSTR(icmpnames[i].msgid, icmpnames[i].name),
				icmpstat.icps_inhist[i]);
		}
	p(icps_reflect, MSGSTR(MSG_RESP_GEN+ndx(icmpstat.icps_reflect), 
		"\t%u message response%s generated\n"));
#undef	p
}

#ifdef IP_MULTICAST
/*
 * Dump IGMP statistics.
 */
igmp_stats(off, name)
	off_t off;
	char *name;
{
	struct igmpstat igmpstat;
	register int i, first;

	if (off == 0)
		return;
	if (Zflag) {
		bzero(&igmpstat, sizeof(igmpstat));
		(void) lseek(kmem, off, 0);
		if (write(kmem, &igmpstat, sizeof(igmpstat)) != sizeof(igmpstat))
			perror("write");
		return;
	}
	
	kvm_read(off, (char *)&igmpstat, sizeof (igmpstat));
	printf("%s:\n", name);

#define	p(f, m) if (igmpstat.f || sflag <= 1) \
    printf(m, igmpstat.f, plural(igmpstat.f))
#define	py(f, m) if (igmpstat.f || sflag <= 1) \
    printf(m, igmpstat.f, igmpstat.f != 1 ? "ies" : "y")
	p(igps_rcv_total, MSGSTR(IGMP_RECV+ndx(igmpstat.igps_rcv_total),
		"\t%u message%s received\n"));
        p(igps_rcv_tooshort, MSGSTR(IGMP_TOO_SHORT+ndx(igmpstat.igps_rcv_tooshort),
		"\t%u message%s received with too few bytes\n"));
        p(igps_rcv_badsum, MSGSTR(IGMP_BAD_CKSUM+ndx(igmpstat.igps_rcv_badsum),
		"\t%u message%s received with bad checksum\n"));
        py(igps_rcv_queries, MSGSTR(IGMP_RCV_QUERIES+ndx(igmpstat.igps_rcv_queries),
		"\t%u membership quer%s received\n"));
        py(igps_rcv_badqueries, MSGSTR(IGMP_RCV_BADQUERIES+ndx(igmpstat.igps_rcv_badqueries),
		"\t%u membership quer%s received with invalid field(s)\n"));
        p(igps_rcv_reports, MSGSTR(IGMP_RCV_RPTS+ndx(igmpstat.igps_rcv_reports),
		"\t%u membership report%s received\n"));
        p(igps_rcv_badreports, MSGSTR(IGMP_RCV_BADRPTS+ndx(igmpstat.igps_rcv_badreports),
		"\t%u membership report%s received with invalid field(s)\n"));
        p(igps_rcv_ourreports, MSGSTR(IGMP_RCV_OURRPTS+ndx(igmpstat.igps_rcv_ourreports),
		"\t%u membership report%s received for groups to which we belong\n"));
        p(igps_snd_reports, MSGSTR(IGMP_SND_RPTS+ndx(igmpstat.igps_snd_reports),
		"\t%u membership report%s sent\n"));
#undef p
#undef py
}
#endif IP_MULTICAST

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */
inetprint(in, port, proto)
	register struct in_addr *in;
	u_short port; 
	char *proto;
{
	struct servent *sp = 0;
	char line[80], *cp, *index();
	int width;

	sprintf(line, "%.*s.", (Aflag && !nflag) ? 12 : 16, inetname(*in));
	cp = index(line, '\0');
	if (!nflag && port)
		sp = getservbyport((int)port, proto);
	if (sp || port == 0)
		sprintf(cp, "%.8s", sp ? sp->s_name : "*");
	else
		sprintf(cp, "%d", ntohs((u_short)port));
	width = Aflag ? 18 : 22;
	printf(" %-*.*s", width, width, line);
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give 
 * numeric value, otherwise try for symbolic name.
 */
char *
inetname(in)
	struct in_addr in;
{
	register char *cp;
	static char line[50];
	struct hostent *hp;
	struct netent *np;
	static char domain[MAXHOSTNAMELEN + 1];
	static int first = 1;

	if (first && !nflag) {
		first = 0;
		if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
		    (cp = index(domain, '.')))
			(void) strcpy(domain, cp + 1);
		else
			domain[0] = 0;
	}
	cp = 0;
	if (!nflag && in.s_addr != INADDR_ANY) {
		int net = inet_netof(in);
		int lna = inet_lnaof(in);

		if (lna == INADDR_ANY) {
			np = getnetbyaddr(net, AF_INET);
			if (np)
				cp = np->n_name;
		}
		if (cp == 0) {
			hp = gethostbyaddr((char *)&in, sizeof (in), AF_INET);
			if (hp) {
				if ((cp = index(hp->h_name, '.')) &&
				    !strcmp(cp + 1, domain))
					*cp = 0;
				cp = hp->h_name;
			}
		}
	}
	if (in.s_addr == INADDR_ANY)
		strcpy(line, "*");
	else if (cp)
		strcpy(line, cp);
	else {
		in.s_addr = ntohl(in.s_addr);
#define C(x)	((x) & 0xff)
		sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
			C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
	}
	return (line);
}
