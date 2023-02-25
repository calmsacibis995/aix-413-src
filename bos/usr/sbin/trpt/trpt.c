static char sccsid[] = "@(#)25	1.16  src/bos/usr/sbin/trpt/trpt.c, cmdnet, bos411, 9428A410j 11/12/93 13:44:16";
/*
 *   COMPONENT_NAME: SYSXAIXIF
 *
 *   FUNCTIONS: MSGSTR
 *		dotrace
 *		main
 *		numeric
 *		ptime
 *		tcp_trace
 *
 *   ORIGINS: 10,26,27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/*
#ifndef lint
char copyright[] =
"Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "trpt.c	5.2 (Berkeley) 9/18/85";
#endif not lint
*/

#include <stdlib.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#define PRUREQUESTS
#include <sys/protosw.h>

#include <net/route.h>
#include <net/if.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#define TCPSTATES
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#define	TCPTIMERS
#include <netinet/tcp_timer.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_var.h>
#define	TANAMES
#include <netinet/tcp_debug.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <nlist.h>

#include <locale.h>

n_time	ntime;
int	sflag;
int	tflag;
int	jflag;
int	aflag;
int	follow;
int	numeric();

#define NELEM_NLIST 3
int tcp_ndebug=0;

struct	nlist nl[] = {
	{ "tcp_debug" },
	{ "tcp_ndebug" },
	{ "tcp_debx" },
	0
};
struct	tcp_debug *tcpdebug;
caddr_t	*tcp_pcbs;
int	tcp_debx;
int	tcp_debug_size=0;
int	tcp_debugp=0;

#ifdef notdef
/* following definitions have been previously defined in include files */
 
char *tcpstates[] = {
	"CLOSED",	"LISTEN",	"SYN_SENT",	"SYN_RCVD",
	"ESTABLISHED",	"CLOSE_WAIT",	"FIN_WAIT_1",	"CLOSING",
	"LAST_ACK",	"FIN_WAIT_2",	"TIME_WAIT",
};

char	*tanames[] =
    { "input", "output", "user", "respond", "drop" };


char *tcptimers[] =
    { "REXMT", "PERSIST", "KEEP", "2MSL" };

#endif /* notdef */


#include <nl_types.h>
#include "trpt_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TRPT,n,s) 

main(argc, argv)
	int argc;
	char **argv;
{
        extern char *optarg;
        extern int optind;
	int ch, i, npcbs = 0;

	setlocale(LC_ALL,"");
	catd = catopen(MF_TRPT,NL_CAT_LOCALE);

	(void) close(0);
	if (open("/dev/kmem", 0) < 0) {
		fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("/dev/kmem");
		exit(2);
	}
	(void) knlist(nl, NELEM_NLIST, sizeof (struct nlist));
	(void) lseek(0, nl[1].n_value, 0);
	if (read(0, &tcp_ndebug, sizeof (tcp_ndebug)) != sizeof (tcp_ndebug)) {
		fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("tcp_ndebug");
		exit(3);
	}
	tcp_pcbs = (caddr_t *)malloc(sizeof(caddr_t) * tcp_ndebug);
	if (tcp_pcbs == (caddr_t *)NULL) {
		perror(MSGSTR(MLC_PCBS, "trpt: no memory for tcp_pcbs: "));
		exit(1);
	}
	tcpdebug = (struct tcp_debug *)
			malloc(sizeof(struct tcp_debug) * tcp_ndebug);
	if (tcpdebug == (struct tcp_debug *)NULL) {
		perror(MSGSTR(MLC_DEBUG, "trpt: no memory for tcpdebug: "));
		exit(1);
	}

        jflag = npcbs = 0;
        while ((ch = getopt(argc, argv, "?afjp:st")) != EOF) {
                switch((char)ch) {
                case 'a':
                        ++aflag;
                        break;
                case 'f':
                        ++follow;
                        break;
                case 'j':
                        ++jflag;
                        break;
                case 'p':
                        if (npcbs >= tcp_ndebug) {
				fprintf (stderr, MSGSTR(TOO_MANY_PCB,
                                             "-p: too many pcb's specified\n"));
                                exit(1);
                        }
                        (void)sscanf(optarg, "%x", (int *)&tcp_pcbs[npcbs++]);
                        break;
                case 's':
                        ++sflag;
                        break;
                case 't':
                        ++tflag;
                        break;
                case '?':
                default:
			fprintf (stderr, MSGSTR(USAGE,
                    "usage: trpt [-afjst] [-p hex-address]\n"));
                        exit(1);
		}
	}

	if (nl[0].n_value == 0) {
		fprintf(stderr, MSGSTR(NO_NMLST, "trpt: no namelist\n")); /*MSG*/
		exit(1);
	}
	(void) lseek(0, nl[2].n_value, 0);
	if (read(0, &tcp_debx, sizeof (tcp_debx)) != sizeof (tcp_debx)) {
		fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("tcp_debx");
		exit(3);
	}
	(void) lseek(0, nl[0].n_value, 0);
	if (read(0, &tcp_debugp, sizeof (tcp_debugp)) != sizeof (tcp_debugp)) {
		fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("tcp_debugp");
		exit(3);
	}
	if (tcp_debugp == 0) {
		fprintf(stderr, "trpt: debugging is off.\n");
		exit(0);
	}
	tcp_debug_size = sizeof(struct tcp_debug) * tcp_ndebug;
	(void) lseek(0, tcp_debugp, 0);
	if (read(0, tcpdebug, tcp_debug_size) != tcp_debug_size) {
		fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("tcpdebug");
		exit(3);
	}
	/*
	 * If no control blocks have been specified, figure
	 * out how many distinct one we have and summarize
	 * them in tcp_pcbs for sorting the trace records
	 * below.
	 */
	if (npcbs == 0) {
		for (i = 0; i < tcp_ndebug; i++) {
			register int j;
			register struct tcp_debug *td = &tcpdebug[i];

			if (td->td_tcb == 0)
				continue;
			for (j = 0; j < npcbs; j++)
				if (tcp_pcbs[j] == td->td_tcb)
					break;
			if (j >= npcbs)
				tcp_pcbs[npcbs++] = td->td_tcb;
		}
	}
	qsort(tcp_pcbs, npcbs, sizeof (caddr_t), numeric);
	if (jflag) {
		char *cp = "";

		for (i = 0; i < npcbs; i++) {
			printf("%s%x", cp, tcp_pcbs[i]);
			cp = ", ";
		}
		if (*cp)
			putchar('\n');
		exit(0);
	}
	for (i = 0; i < npcbs; i++) {
		printf("\n%x:\n", tcp_pcbs[i]);
		dotrace(tcp_pcbs[i]);
	}
	exit(0);
}

dotrace(tcpcb)
	register caddr_t tcpcb;
{
	register int i;
	register struct tcp_debug *td;
	int prev_debx = tcp_debx;

again:
	if (--tcp_debx < 0)
		tcp_debx = tcp_ndebug - 1;
	for (i = prev_debx % tcp_ndebug; i < tcp_ndebug; i++) {
		td = &tcpdebug[i];
		if (tcpcb && td->td_tcb != tcpcb)
			continue;
		ntime = ntohl(td->td_time);
		tcp_trace(td->td_act, td->td_ostate, td->td_tcb, &td->td_cb,
		    &td->td_ti, td->td_req);
		if (i == tcp_debx)
			goto done;
	}
	for (i = 0; i <= tcp_debx % tcp_ndebug; i++) {
		td = &tcpdebug[i];
		if (tcpcb && td->td_tcb != tcpcb)
			continue;
		ntime = ntohl(td->td_time);
		tcp_trace(td->td_act, td->td_ostate, td->td_tcb, &td->td_cb,
		    &td->td_ti, td->td_req);
	}
done:
	if (follow) {
	    prev_debx = tcp_debx + 1;
	    if (prev_debx >= tcp_ndebug)
		prev_debx = 0;
	    do {
		sleep(1);
		(void) lseek(0, nl[2].n_value, 0);
		if (read(0, &tcp_debx, sizeof(tcp_debx)) != sizeof(tcp_debx)) {
		    fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("tcp_debx");
		    exit(3);
		}
	    } while (tcp_debx == prev_debx);
	    (void) lseek(0, tcp_debugp, 0);
	    if (read(0, tcpdebug, tcp_debug_size) != tcp_debug_size) {
		    fprintf(stderr, MSGSTR(TRPT, "trpt: ")); perror("tcpdebug");
		    exit(3);
	    }
	    goto again;
	}
}

/*
 * Tcp debug routines
 */
tcp_trace(act, ostate, atp, tp, ti, req)
	short act, ostate;
	struct tcpcb *atp, *tp;
	struct tcpiphdr *ti;
	int req;
{
	tcp_seq seq, ack;
	int len, flags, win, timer;
	char *cp;

	ptime(ntime);
	printf("%s:%s ", tcpstates[ostate], tanames[act]);
	switch (act) {

	case TA_INPUT:
	case TA_OUTPUT:
	case TA_DROP:
		if (aflag) {
			printf(MSGSTR(PRINT_SRC, "(src=%s,%d, "), inet_ntoa(ti->ti_src), /*MSG*/
				ntohs(ti->ti_sport));
			printf(MSGSTR(PRINT_DEST, "dst=%s,%d)"), inet_ntoa(ti->ti_dst), /*MSG*/
				ntohs(ti->ti_dport));
		}
		seq = ti->ti_seq;
		ack = ti->ti_ack;
		len = ti->ti_len;
		win = ti->ti_win;
		if (act == TA_OUTPUT) {
			seq = ntohl(seq);
			ack = ntohl(ack);
			len = ntohs(len);
			win = ntohs(win);
		}
		if (act == TA_OUTPUT)
			len -= sizeof (struct tcphdr);
		if (len)
			printf("[%x..%x)", seq, seq+len);
		else
			printf("%x", seq);
		printf("@%x", ack);
		if (win)
			printf(MSGSTR(PRINT_WIN, "(win=%x)"), win); /*MSG*/
		flags = ti->ti_flags;
		if (flags) {
			char *cp = "<";
#define pf(f) { if (ti->ti_flags&TH_/**/f) { printf("%s%s", cp, "f"); cp = ","; } }
			pf(SYN); pf(ACK); pf(FIN); pf(RST); pf(PUSH); pf(URG);
			printf(">");
		}
		break;

	case TA_USER:
		timer = req >> 8;
		req &= 0xff;
		printf("%s", prurequests[req]);
		if (req == PRU_SLOWTIMO || req == PRU_FASTTIMO)
			printf("<%s>", tcptimers[timer]);
		break;
	}
	printf(" -> %s", tcpstates[tp->t_state]);
	/* print out internal state of tp !?! */
	printf("\n");
	if (sflag) {
		printf(MSGSTR(PR1, "\trcv_nxt %x rcv_wnd %x snd_una %x snd_nxt %x snd_max %x\n"),
		    tp->rcv_nxt, tp->rcv_wnd, tp->snd_una, tp->snd_nxt,
		    tp->snd_max);
		printf(MSGSTR(PR2, "\tsnd_wl1 %x snd_wl2 %x snd_wnd %x\n"), tp->snd_wl1,
		    tp->snd_wl2, tp->snd_wnd);
	}
	/* print out timers? */
	if (tflag) {
		char *cp = "\t";
		register int i;

		for (i = 0; i < TCPT_NTIMERS; i++) {
			if (tp->t_timer[i] == 0)
				continue;
			printf("%s%s=%d", cp, tcptimers[i], tp->t_timer[i]);
			if (i == TCPT_REXMT)
				printf(MSGSTR(RXTSHFT, " (t_rxtshft=%d)"), tp->t_rxtshift); /*MSG*/
			cp = ", ";
		}
		if (*cp != '\t')
			putchar('\n');
	}
}

ptime(ms)
	int ms;
{

	printf("%03d ", (ms/10) % 1000);
}

numeric(c1, c2)
	caddr_t *c1, *c2;
{
	
	return (*c1 - *c2);
}
