static char sccsid[] = "@(#)84	1.2  src/bos/usr/sbin/netstat/ns.c, cmdnet, bos411, 9428A410j 3/11/91 16:09:53";

/*
 * COMPONENT_NAME: (CMDNET) Network commands. 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: ?
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985, 1988 Regents of the University of California.
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

#include <stdio.h>
#include <errno.h>
#include <nlist.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/tcp_fsm.h>

#include <netns/ns.h>
#include <netns/ns_pcb.h>
#include <netns/idp.h>
#include <netns/idp_var.h>
#include <netns/ns_error.h>
#include <netns/sp.h>
#include <netns/spidp.h>
#include <netns/spp_timer.h>
#include <netns/spp_var.h>
#define SANAMES
#include <netns/spp_debug.h>


struct	nspcb nspcb;
struct	sppcb sppcb;
struct	socket sockb;
extern	int Aflag;
extern	int aflag;
extern	int nflag;
extern	char *plural();
char *ns_prpr();

static	int first = 1;

#ifdef MSG
#include "netstat_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) NLcatgets(catd,MS_NETSTAT,n,s) 
#else
#define MSGSTR(n,s) s
#endif

/*
 * Print a summary of connections related to a Network Systems
 * protocol.  For SPP, also give state of connection.
 * Listening processes (aflag) are suppressed unless the
 * -a (all) flag is specified.
 */

nsprotopr(off, name)
	off_t off;
	char *name;
{
	struct nspcb cb;
	register struct nspcb *prev, *next;
	int isspp;

	if (off == 0)
		return;
	isspp = strcmp(name, "spp") == 0;
	kvm_read(off, (char *)&cb, sizeof (struct nspcb));
	nspcb = cb;
	prev = (struct nspcb *)off;
	if (nspcb.nsp_next == (struct nspcb *)off)
		return;
	for (;nspcb.nsp_next != (struct nspcb *)off; prev = next) {
		off_t ppcb;

		next = nspcb.nsp_next;
		kvm_read((off_t)next, (char *)&nspcb, sizeof (nspcb));
		if (nspcb.nsp_prev != prev) {
			printf("???\n");
			break;
		}
		if (!aflag && ns_nullhost(nspcb.nsp_faddr) ) {
			continue;
		}
		kvm_read((off_t)nspcb.nsp_socket,
				(char *)&sockb, sizeof (sockb));
		ppcb = (off_t) nspcb.nsp_pcb;
		if (ppcb) {
			if (isspp) {
				kvm_read(ppcb, (char *)&sppcb, sizeof (sppcb));
			} else continue;
		} else
			if (isspp) continue;
		if (first) {
			printf(MSGSTR(NS_ACT_CONN, "Active NS connections"));
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
			printf("%8x ", ppcb);
		printf("%-5.5s %6d %6d ", name, sockb.so_rcv.sb_cc,
			sockb.so_snd.sb_cc);
		printf("  %-22.22s", ns_prpr(&nspcb.nsp_laddr));
		printf(" %-22.22s", ns_prpr(&nspcb.nsp_faddr));
		if (isspp) {
			extern char *tcpstates[];
			if (sppcb.s_state >= TCP_NSTATES)
				printf(" %d", sppcb.s_state);
			else
				printf(" %s", tcpstates[sppcb.s_state]);
		}
		putchar('\n');
		prev = next;
	}
}

#define ndx(f) ((f) == 1 ? 0 : 1)
#define ANY(f,i,m) ((f) ? printf("\t%d ", f),\
			  printf(MSGSTR(i+ndx(f), m), plural(f)),\
			  printf(" -- x\n") \
		        : 0)
#define XANY(f,i,m) ((f) ? printf("\t%d ", f),\
			   printf(MSGSTR(i, m)),\
			   printf(" -- x\n") \
		         : 0)

/*
 * Dump SPP statistics structure.
 */
spp_stats(off, name)
	off_t off;
	char *name;
{
	struct spp_istat spp_istat;
#define sppstat spp_istat.newstats

	if (off == 0)
		return;
	kvm_read(off, (char *)&spp_istat, sizeof (spp_istat));
	printf("%s:\n", name);
	ANY(spp_istat.nonucn, NS_NONUCN, "connection%s dropped due to no new sockets");
	ANY(spp_istat.gonawy, NS_GONAWY, "connection%s terminated due to our end dying");
	ANY(spp_istat.noconn, NS_NOCONN, "connection%s dropped due to inability to connect");
	ANY(spp_istat.notme, NS_NOTME, "connection%s incompleted due to mismatched id's");
	ANY(spp_istat.wrncon, NS_WRNCON, "connection%s dropped due to mismatched id's");
	ANY(spp_istat.bdreas, NS_BDREAS, "packet%s dropped out of sequence");
	ANY(spp_istat.lstdup, NS_LSTDUP, "packet%s duplicating the highest packet");
	ANY(spp_istat.notyet, NS_NOTYET, "packet%s refused as exceeding allocation");
	ANY(sppstat.spps_connattempt, NS_CONNATTEMPT, "connection%s initiated");
	ANY(sppstat.spps_accepts, NS_ACCEPTS, "connection%s accepted");
	ANY(sppstat.spps_connects, NS_CONNECTS, "connection%s established");
	ANY(sppstat.spps_drops, NS_DROPS, "connection%s dropped");
	ANY(sppstat.spps_conndrops, NS_CONNDROPS, "embryonic connection%s dropped");
	ANY(sppstat.spps_closed, NS_CLOSED, "connection%s closed (includes drops)");
	ANY(sppstat.spps_segstimed, NS_SEGSTIMED, "packet%s where we tried to get rtt");
	ANY(sppstat.spps_rttupdated, NS_RTTUPDATED, "time%s we got rtt");
	ANY(sppstat.spps_delack, NS_DELACK, "delayed ack%s sent");
	ANY(sppstat.spps_timeoutdrop, NS_TIMEOUTDROP, "connection%s dropped in rxmt timeout");
	XANY(sppstat.spps_rexmttimeo, NS_REXMTTIMEO, "retransmit timeout");
	XANY(sppstat.spps_persisttimeo, NS_PERSISTTIMEO, "persist timeout");
	XANY(sppstat.spps_keeptimeo, NS_KEEPTIMEO, "keepalive timeout");
	ANY(sppstat.spps_keepprobe, NS_KEEPPROBE, "keepalive probe%s sent");
	ANY(sppstat.spps_keepdrops, NS_KEEPDROPS, "connection%s dropped in keepalive");
	ANY(sppstat.spps_sndtotal, NS_SNDTOTAL, "total packet%s sent");
	ANY(sppstat.spps_sndpack, NS_SNDPACK, "data packet%s sent");
	ANY(sppstat.spps_sndbyte, NS_SNDBYTE, "data byte%s sent");
	ANY(sppstat.spps_sndrexmitpack, NS_SNDREXMITPACK, "data packet%s retransmitted");
	ANY(sppstat.spps_sndrexmitbyte, NS_SNDREXMITBYTE, "data byte%s retransmitted");
	ANY(sppstat.spps_sndacks, NS_SNDACKS, "ack-only packet%s sent");
	ANY(sppstat.spps_sndprobe, NS_SNDPROBE, "window probe%s sent");
	ANY(sppstat.spps_sndurg, NS_SNDURG, "packet%s sent with URG only");
	ANY(sppstat.spps_sndwinup, NS_SNDWINUP, "window update-only packet%s sent");
	ANY(sppstat.spps_sndctrl, NS_SNDCTRL, "control (SYN|FIN|RST) packet%s sent");
	ANY(sppstat.spps_sndvoid, NS_SNDVOID, "request%s to send a non-existant packet");
	ANY(sppstat.spps_rcvtotal, NS_RCVTOTAL, "total packet%s received");
	ANY(sppstat.spps_rcvpack, NS_RCVPACK, "packet%s received in sequence");
	ANY(sppstat.spps_rcvbyte, NS_RCVBYTE, "byte%s received in sequence");
	ANY(sppstat.spps_rcvbadsum, NS_BADSUM, "packet%s received with ccksum errs");
	ANY(sppstat.spps_rcvbadoff, NS_RCVBADOFF, "packet%s received with bad offset");
	ANY(sppstat.spps_rcvshort, NS_RCVSHORT, "packet%s received too short");
	ANY(sppstat.spps_rcvduppack, NS_RCVDUPPACK, "duplicate-only packet%s received");
	ANY(sppstat.spps_rcvdupbyte, NS_RCVDUPBYTE, "duplicate-only byte%s received");
	ANY(sppstat.spps_rcvpartduppack, NS_RCVPARTDUPPACK, "packet%s with some duplicate data");
	ANY(sppstat.spps_rcvpartdupbyte, NS_RCVPARTDUPBYTE, "dup. byte%s in part-dup. packet");
	ANY(sppstat.spps_rcvoopack, NS_RCVOOPACK, "out-of-order packet%s received");
	ANY(sppstat.spps_rcvoobyte, NS_RCVOOBYTE, "out-of-order byte%s received");
	ANY(sppstat.spps_rcvpackafterwin, NS_RCVPACKAFTERWIN, "packet%s with data after window");
	ANY(sppstat.spps_rcvbyteafterwin, NS_RCVBYTEAFTERWIN, "byte%s rcvd after window");
	ANY(sppstat.spps_rcvafterclose, NS_RCVAFTERCLOSE, "packet%s rcvd after 'close'");
	XANY(sppstat.spps_rcvwinprobe, NS_RCVWINPROBE, "rcvd window probe packet");
	XANY(sppstat.spps_rcvdupack, NS_RCVDUPACK, "rcvd duplicate ack");
	ANY(sppstat.spps_rcvacktoomuch, NS_RCVACKTOOMUCH, "rcvd ack%s for unsent data");
	XANY(sppstat.spps_rcvackpack, NS_RCVACKPACK, "rcvd ack packet");
	ANY(sppstat.spps_rcvackbyte, NS_RCVACKBYTE, "byte%s acked by rcvd acks");
	XANY(sppstat.spps_rcvwinupd, NS_RCVWINUPD, "rcvd window update packet");
}
#undef ANY
#undef XANY
#define ANY(f,i,m) ((f) ? printf("\t%d ", f),\
			  printf(MSGSTR(i+ndx(f), m), plural(f)),\
			  printf("\n") \
		        : 0)

/*
 * Dump IDP statistics structure.
 */
idp_stats(off, name)
	off_t off;
	char *name;
{
	struct idpstat idpstat;

	if (off == 0)
		return;
	kvm_read(off, (char *)&idpstat, sizeof (idpstat));
	printf("%s:\n", name);
	ANY(idpstat.idps_toosmall, IDP_TOOSMALL, "packet%s smaller than a header");
	ANY(idpstat.idps_tooshort, IDP_TOOSHORT, "packet%s smaller than advertised");
	ANY(idpstat.idps_badsum, IDP_BADSUM, "packet%s with bad checksums");
}

static	struct {
	u_short code;
	int msgid;
	char *name;
} ns_errnames[] = {
	{0,	NS_UNSPEC_DEST, "Unspecified Error%s at Destination"},
	{1,	NS_CHKSUM_DEST, "Bad Checksum%s at Destination"},
	{2,	NS_NOLSTN_SOCK, "No Listener%s at Socket"},
	{3,	NS_PKT_DEST_SP, "Packet%s Refused due to lack of space at Destination"},
	{01000,	NS_UNSPEC_GATE, "Unspecified Error%s while gatewayed"},
	{01001,	NS_CHKSUM_GATE, "Bad Checksum%s while gatewayed"},
	{01002,	NS_PKT_TOOFWD,  "Packet%s forwarded too many times"},
	{01003,	NS_PKT_TOOBIG,  "Packet%s too large to be forwarded"},
	{-1, 0, 0},
};

/*
 * Dump NS Error statistics structure.
 */
/*ARGSUSED*/
nserr_stats(off, name)
	off_t off;
	char *name;
{
	struct ns_errstat ns_errstat;
	register int j;
	register int histoprint = 1;
	int z;

	if (off == 0)
		return;
	kvm_read(off, (char *)&ns_errstat, sizeof (ns_errstat));
	printf(MSGSTR(NSERR_STATS, "NS error statistics:\n"));
	ANY(ns_errstat.ns_es_error, NSERR_ERROR, "call%s to ns_error");
	ANY(ns_errstat.ns_es_oldshort, NSERR_OLDSHORT, "error%s ignored due to insufficient addressing");
	ANY(ns_errstat.ns_es_oldns_err, NSERR_OLDNSERR, "error request%s in response to error packets");
	ANY(ns_errstat.ns_es_tooshort, NSERR_TOOSHORT, "error packet%s received incomplete");
	ANY(ns_errstat.ns_es_badcode, NSERR_BADCODE, "error packet%s received of unknown type");
	for(j = 0; j < NS_ERR_MAX; j ++) {
		z = ns_errstat.ns_es_outhist[j];
		if (z && histoprint) {
			printf(MSGSTR(NSERR_OHIST, "Output Error Histogram:\n"));
			histoprint = 0;
		}
		ns_erputil(z, ns_errstat.ns_es_codes[j]);

	}
	histoprint = 1;
	for(j = 0; j < NS_ERR_MAX; j ++) {
		z = ns_errstat.ns_es_inhist[j];
		if (z && histoprint) {
			printf(MSGSTR(NSERR_IHIST, "Input Error Histogram:\n"));
			histoprint = 0;
		}
		ns_erputil(z, ns_errstat.ns_es_codes[j]);
	}
}
static
ns_erputil(z, c)
{
	int j;
	int i;
	char codebuf[50];
	char *name;
	for(j = 0;; j ++) {
		if ((name = ns_errnames[j].name) == 0)
			break;
		if (ns_errnames[j].code == c)
			break;
	}
	if (name == 0)  {
		if (c > 01000) {
			i = NS_UNK_INTRANS;
			name = "Unknown XNS error code 0%o in transit";
		} else {
			i = NS_UNK_ATDEST;
			name = "Unknown XNS error code 0%o at destination";
		}
		if (z) {
			/* modified (no plurals) ANY(z, i, name) */
			printf("\t%d ", z);
			printf(MSGSTR(i, name), c);
			printf("\n");
		}
		return;
	} else 
		i = ns_errnames[j].msgid;
	ANY(z, i, name);
}
static struct sockaddr_ns ssns = {AF_NS};

char *ns_prpr(x)
struct ns_addr *x;
{
	extern char *ns_print();
	struct sockaddr_ns *sns = &ssns;
	sns->sns_addr = *x;
	return(ns_print(sns));
}
