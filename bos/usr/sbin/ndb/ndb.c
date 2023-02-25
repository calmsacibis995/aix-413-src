static char sccsid[] = "@(#)84	1.6.1.8  src/bos/usr/sbin/ndb/ndb.c, cmdnet, bos411, 9428A410j 6/13/94 16:11:19";
/*
 *   COMPONENT_NAME: CMDNET
 *
 *   FUNCTIONS: DELIMITER
 *		PARSE_LIST
 *		SKIPDELIM
 *		SKIPTOKEN
 *		getaddr
 *		getcommand
 *		getflags
 *		gettype
 *		head_tcb
 *		main
 *		makeprintable
 *		mbuf_head_act
 *		mbuf_head_next
 *		mbuf_next
 *		mbuf_next_act
 *		next_ifnet
 *		next_ndd
 *		next_tcb
 *		next_udb
 *		prev_tcb
 *		prev_udb
 *		prhelp
 *		prifnet
 *		prinpcb
 *		prmbuf
 *		prmenu
 *		prndd
 *		prnddp
 *		prnsap
 *		prnsubtype
 *		prpip
 *		prsock
 *		prsock_sb
 *		prtcpcb
 *		rcv_mbuf
 *		readaddr
 *		readsymbol
 *		sayonara_sucker
 *		show_dmx
 *		show_ifnet
 *		show_mbuf
 *		show_ndd
 *		show_sock
 *		show_tcb
 *		show_tcpcb
 *		show_udb
 *		snd_mbuf
 *		top_level
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1988,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* ndb.c - a tool for examining network structures in the kernel 
   for a more descriptive explanation of its features read the long
   help info in prhelp().
*/

#include <storclass.h>

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <netdb.h>		/* hostent struct */
#include <nlist.h>		/* nlist struct */
#define _KERNEL
#include <sys/file.h>
#undef _KERNEL
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/xmem.h>
#include <net/net_malloc.h>
#include <sys/mbuf.h>		/* mbuf struct */
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>	/* socketstruct */
#include <sys/errno.h>
#include <net/if.h>		/* ifnet struct */
#include <sys/ndd.h>		/* ndd struct */
#include <sys/cdli.h>		/* ns stuff */
#include <net/nd_lan.h>
#include <sys/eth_demux.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>	/* inpcb struct */
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_var.h>	/* tcpcb struct */
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <unpack.h>

/*
  In process:
  adding ability to search file table for sockets.
*/
/*
  stuff I may want to put in:
  mbstat struct (mbuf.h).
  routing table/stats (route.h)?
  icmp stats (icmp_var.h)
  arp table (in_netarp.h)
  ipstat/ip_reassembly queue (ip_var.h)
  tcpstat (tcp_var.h)
  udpstat (udp_var.h)
*/

#define  TOP_LEVEL  (cmdtab == ndbtab)
#define  NONE  0
#define  TCP  1
#define  UDP  2

#define	 DELIM          " ,\t\n" /* field delimiters */
#define  SKIPDELIM(bp)  (bp += strspn(bp, DELIM)) /* move past any delimeters */
#define  SKIPTOKEN(bp)  (bp += strcspn(bp, DELIM)) /* move past any non-delimeters */
#define  DELIMITER(ch)  (strchr(DELIM, ch)) /* ch is a delimiter */
#define  PARSE_LIST(list,tokenarray) \
    do { \
	*tokenarray++ = list; \
	SKIPTOKEN(list); \
	if (*list) { \
	    *list++ = '\0'; \
	    SKIPDELIM(list); \
	} \
    } while (*list); \
    *tokenarray = NULL;

/* 
 * some forward declarations, so we can reference these functions in
 * the command tables below.
 */
int prmenu(), prhelp(), show_tcb(), show_tcpcb(), show_udb(), show_inpcb();
int show_mbuf(), show_sock(), show_ifnet(), next_ifnet();
int show_ndd(), next_ndd();
int head_tcb(), snd_mbuf(), rcv_mbuf();
int mbuf_next_act(), mbuf_next(), mbuf_head_act(), mbuf_head_next();
int next_tcb(), prev_tcb(), next_udb(), prev_udb();
int top_level(), sayonara_sucker();
int show_dmx(), next_dmx(), dmx_ndd();

char *makeprintable(), *gettype(), *getflags();

#define readaddr(addr, buf, size) read_memory(kmem, rflag, buf, addr, size)

caddr_t tcbhd, curtcb, udbhd, curudb, curmbuf, mbufhdact, mbufhdnext,
	curifnet, curndd, curdmx, ifnethd, nddhd, dmxhd;

int proto;			/* are we doing: TCP or UDP */

struct cmdent {
	char *cmd;
	char *help;
	int (*fun)();
};

struct cmdent ndbtab[] = {
        { "?", "this help info\n", prmenu },
	{ "help", "a more lengthy help text\n", prhelp },
	{ "tcb [addr]", "show TCBs, default is HEAD TCB\n", show_tcb },
	{ "udb [addr]", "show UDBs, default is HEAD UDB\n", show_udb },
	{ "sockets", "show socket structures\n", show_sock }, 
	{ "mbuf addr", "show mbuf at address\n", show_mbuf },
	{ "ifnet [addr]", "show ifnet structures, default is \"ifnet\"\n",
		  show_ifnet },
	{ "ndd [addr]", "show ndd structures, default is \"ndd\"\n",
		  show_ndd },
	{ "dmx", "show demuxers \"demuxer\"\n", show_dmx },
	{ "quit", "bail out\n", 0 },
	{ "xit", "also bail out\n", 0 },
	{ 0 }
};

struct cmdent tcbtab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "next", "go to next tcb/socket\n", next_tcb },
	{ "previous", "go to previous tcb/socket\n", prev_tcb },
	{ "tcb [addr]", "reread tcb/socket, default is current\n", show_tcb },
	{ "ctrlblk [addr]", "read the control block\n", show_tcpcb },
	{ "sendmbufs", "show the send mbufs\n", snd_mbuf },
	{ "recvmbufs", "show the receive mbufs\n", rcv_mbuf },
	{ "sock addr", "show socket at given address\n", show_sock },
	{ "head", "show the head tcb in the chain\n", head_tcb },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};

struct cmdent udbtab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "next", "go to next udb\n", next_udb },
	{ "previous", "go to previous udb\n", prev_udb },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};

struct cmdent ifnettab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "head", "go to head of ifnet list\n", show_ifnet },
	{ "next", "go to next ifnet struct\n", next_ifnet },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};

struct cmdent nddtab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "head", "go to head of ndd list\n", show_ndd },
	{ "next", "go to next ndd struct\n", next_ndd },
	{ "dmx", "display associated demuxer\n", dmx_ndd },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};

struct cmdent dmxtab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "head", "go to head of demuxer list\n", show_dmx },
	{ "next", "go to next demuxer struct\n", next_dmx },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};

struct cmdent socktab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "socket addr", "show socket at addr\n",
		  show_sock },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};

struct cmdent mbuftab[] = {
	{ "?", "help - you're reading it\n", prmenu },
	{ "act", "go to next mbuf in the act chain\n", mbuf_next_act },
	{ "next", "go to next mbuf in message\n", mbuf_next },
	{ "head", "go to head of mbufs in \"next\" list\n",
		  mbuf_head_next },
	{ "first", "go to first mbuf message in \"act\" chain\n",
		  mbuf_head_act },
	{ "mbuf addr", "show mbuf at address\n", show_mbuf },
        { "quit", "return to top_level\n", top_level },
        { "xit", "bail out\n", sayonara_sucker },
        { 0 }
};
	
struct typeent {
	char *name;
	int val;
};

struct typeent tcptimers[] = {
	{ "TCPT_REXMT", TCPT_REXMT },
	{ "TCPT_PERSIST", TCPT_PERSIST },
	{ "TCPT_KEEP", TCPT_KEEP },
	{ "TCPT_2MSL", TCPT_2MSL },
	{ 0 }
};

struct typeent dmxtypes[] = {
	{ "Zero?", 0},
	{ "Other", NDD_OTHER },
	{ "1822", NDD_1822 },
	{ "HDH1822", NDD_HDH1822 },
	{ "X.25 DDN", NDD_X25DDN },
	{ "X.25", NDD_X25 },
	{ "Ethernet", NDD_ETHER },
	{ "802.3 Ethernet", NDD_ISO88023 },
	{ "802.4", NDD_ISO88024 },
	{ "Token Ring", NDD_ISO88025 },
	{ "802.6", NDD_ISO88026 },
	{ "Starlan", NDD_STARLAN },
	{ "P10", NDD_P10 },
	{ "P80", NDD_P80 },
	{ "HY", NDD_HY },
	{ "FDDI", NDD_FDDI },
	{ "LAPB", NDD_LAPB },
	{ 0 }
};

struct typeent filtertypes[] = {
	{ "Zero?", 0},
	{ "DSAP", NS_8022_LLC_DSAP },
	{ "SNAP", NS_8022_LLC_DSAP_SNAP },
	{ "Tap", NS_TAP },
	{ "Ethertype", NS_ETHERTYPE },
	{ 0 }
};

struct typeent mbuftypes[] = {
	{ "FREE", MT_FREE },
	{ "DATA", MT_DATA },
	{ "HEADER", MT_HEADER },
	{ "SOCKET", MT_SOCKET },
	{ "PCB", MT_PCB },
	{ "RTABLE", MT_RTABLE },
	{ "HTABLE", MT_HTABLE },
	{ "ATABLE", MT_ATABLE },
	{ "SONAME", MT_SONAME },
	{ "SOOPTS", MT_SOOPTS },
	{ "FTABLE", MT_FTABLE },
	{ "RIGHTS", MT_RIGHTS },
	{ "IFADDR", MT_IFADDR },
	{ 0 }
};

struct typeent socktypes[] = {
	{ "STREAM", SOCK_STREAM },
	{ "DGRAM", SOCK_DGRAM },
	{ "RAW", SOCK_RAW },
	{ "RDM", SOCK_RDM },
	{ "SEQPACKET", SOCK_SEQPACKET },
	{ 0 }
};

struct typeent tcpstates[] = {
	{ "CLOSED", TCPS_CLOSED },
	{ "LISTEN", TCPS_LISTEN },
	{ "SYN_SENT", TCPS_SYN_SENT },
	{ "SYN_RECEIVED", TCPS_SYN_RECEIVED },
	{ "ESTABLISHED", TCPS_ESTABLISHED },
	{ "CLOSE_WAIT", TCPS_CLOSE_WAIT },
	{ "FIN_WAIT_1", TCPS_FIN_WAIT_1 },
	{ "CLOSING", TCPS_CLOSING },
	{ "LAST_ACK", TCPS_LAST_ACK },
	{ "FIN_WAIT_2", TCPS_FIN_WAIT_2 },
	{ "TIME_WAIT", TCPS_TIME_WAIT },
	{ 0 }
};

struct typeent oobflags[] = {
	{ "HAVEDATA", TCPOOB_HAVEDATA },
	{ "HADDATA", TCPOOB_HADDATA },
	{ 0 }
};

struct flagent {
	char *name;
	int bit;
};

struct flagent sockstates[] = {
	{ "NOFDREF", SS_NOFDREF },
	{ "ISCONNECTED", SS_ISCONNECTED },
	{ "ISCONNECTING", SS_ISCONNECTING },
	{ "ISDISCONNECTING", SS_ISDISCONNECTING },
	{ "CANTSENDMORE", SS_CANTSENDMORE },
	{ "CANTRCVMORE", SS_CANTRCVMORE },
	{ "RCVATMARK", SS_RCVATMARK },
	{ "PRIV", SS_PRIV },
	{ "NBIO", SS_NBIO },
	{ "ASYNC", SS_ASYNC },
	{ "SS_ISCONFIRMING", SS_ISCONFIRMING },
        { 0 }
};

struct flagent ifflags[] = {
	{ "UP", IFF_UP },
	{ "BROADCAST", IFF_BROADCAST },
	{ "DEBUG", IFF_DEBUG },
	{ "LOOPBACK", IFF_LOOPBACK },
	{ "POINTOPOINT", IFF_POINTOPOINT },
	{ "NOTRAILERS", IFF_NOTRAILERS },
	{ "RUNNING", IFF_RUNNING },
	{ "NOARP", IFF_NOARP },
	{ "PROMISC", IFF_PROMISC },
	{ "ALLMULTI", IFF_ALLMULTI },
	{ "BRIDGE", IFF_BRIDGE },
	{ "SNAP", IFF_SNAP },
	{ "ALLCAST", IFF_ALLCAST },
	{ "CANTCHANGE", IFF_CANTCHANGE },
	{ "DO_HW_LOOPBACK", IFF_DO_HW_LOOPBACK },
        { 0 }
};
	
struct flagent ddflags[] = {
	{ "UP", NDD_UP },
	{ "BROADCAST", NDD_BROADCAST },
	{ "DEBUG", NDD_DEBUG },
	{ "RUNNING", NDD_RUNNING },
	{ "NOECHO", NDD_SIMPLEX },
	{ "DEAD", NDD_DEAD },
	{ "LIMBO", NDD_LIMBO },
	{ "PROMISCUOUS", NDD_PROMISC },
	{ "ALT ADDRS", NDD_ALTADDRS },
	{ "MULTICAST", NDD_MULTICAST },
        { 0 }
};

struct flagent tcpcbflags[] = {
	{ "ACKNOW", TF_ACKNOW },
	{ "DELACK", TF_DELACK },
	{ "NODELAY", TF_NODELAY },
	{ "NOOPT", TF_NOOPT },
	{ "SENTFIN", TF_SENTFIN },
        { 0 },
};

struct flagent sbflags[] = {
	{ "LOCK", SB_LOCK },
	{ "WANT", SB_WANT },
	{ "WAIT", SB_WAIT },
	{ "SEL", SB_SEL },
	{ "ASYNC", SB_ASYNC },
	{ "COLL", SB_COLL },
	{ "NOINTR", SB_NOINTR },
	{ "SB_WAKEONE", SB_WAKEONE },
	{ "SB_WAITING", SB_WAITING },
	{ "KIODONE", SB_KIODONE },
        { 0 },
};

struct flagent sockoptflags[] = {
	{ "DEBUG", SO_DEBUG },
	{ "ACCEPTCONN", SO_ACCEPTCONN },
	{ "REUSEADDR", SO_REUSEADDR },
	{ "KEEPALIVE", SO_KEEPALIVE },
	{ "DONTROUTE", SO_DONTROUTE },
	{ "BROADCAST", SO_BROADCAST },
	{ "USELOOPBACK", SO_USELOOPBACK },
	{ "LINGER", SO_LINGER },
	{ "OOBINLINE", SO_OOBINLINE },
        { 0 }
};

struct cmdent *cmdtab;
int kmem, lastcommand, level;
char *prompt[6];
char *args[10];
char line[256]; 
char *progname;
char *kmem_path = "/dev/kmem";
char *unix_path = "/unix";
char *netinet_path = 0;
unsigned long netinet_base = 0;
int rflag;				/* running kernel */

/* network debugger */
main(int argc, char *argv[])
{
	int c = 1;

	if (progname = rindex(argv[0], '/'))
		++progname;
	else
		progname = argv[0];
	
	if (!(rflag = (argc == 1))) {	/* not a running kernel */
		if (argc != 5) {
			fprintf(stderr, "Usage: %s [ core unix netinet data_start ]\n",
				progname);
			exit(1);
		}
		kmem_path = argv[1];
		unix_path = argv[2];
		netinet_path = argv[3];
		sscanf(argv[4], "%X", &netinet_base);
	}
	proto = NONE;
	level = 0;
	prompt[0] = "ndb";

	if ((kmem = open(kmem_path, 0)) < 0) {
		fprintf(stderr, "%s: ", progname);
		perror(kmem_path);
		return(1);
	}

	readsymbol("tcb", &tcbhd, sizeof(tcbhd));
	readsymbol("udb", &udbhd, sizeof(udbhd));

	if (!tcbhd) {
		fprintf(stderr,
			"Warning! It looks to me like inet isn't loaded.\n");
	}

	curtcb = tcbhd;
	curudb = udbhd;
	curifnet = ifnethd;

	top_level();

	printf("type ? for help\n");
	for (;;) {
		c = getcommand();
		if (c >= 0)
			if (cmdtab[c].fun) {
				(*cmdtab[c].fun)();
			} else
				break;
	}
	sayonara_sucker();
}

getcommand()
{
	struct cmdent *c = cmdtab;
	char *cp, *ap, *bp, **av = args;
	int cmd = -1, num = 0, i;
	
	printf("\n");
	for (i=0; i<=level; i++)
		printf(prompt[i]);
	printf("> ");
	bzero(line, sizeof(line));
	if (fgets(line, 256, stdin) == NULL) {
		fprintf(stderr, "ndb: error reading input, goodbye.\n");
		exit(1);
	}
	/* nuke the newline */
	line[strlen(line)-1] = '\0';
	cp = line;
	SKIPDELIM(cp);
	ap = cp;
	SKIPTOKEN(ap); /* move past initial command */
	if (*ap)  {
		bp = ap;
		SKIPDELIM(ap);
		*bp = '\0';
	}

	if (*cp == '\0') {	/* NULL input == do last command */
		return(lastcommand);
	}
	while (c->cmd) {
		if (strncmp(cp, c->cmd, strlen(line)) == 0) {
			if (cmd != -1) {
				printf("that wasn't a unique command\n");
				return(-1);
		        } else
				cmd = num;
		}
		c++; num++;
	}
	if (cmd == -1)
		printf("unknown command, dude\n");
	else {	  /* process args */
		lastcommand = cmd;
		memset(args, 0, sizeof(args));
		if (*ap) {
			PARSE_LIST(ap, av);
		}

	}

	return(cmd);
}

show_ifnet()
{
	char **av = args;
	caddr_t addr, ifp;

	level = 1;
	cmdtab = ifnettab;
	lastcommand = 0 ;
	prompt[1] = ".ifnet";

	if (*av == 0) {
		/* the ifnet symbol is a pointer to an ifnet struct.
		   so we have to read the value at that address, then
		   use the value at that address as the pointer to
		   the first ifnet structure. */
		if (!readsymbol("ifnet", &ifp, sizeof(ifp)))
			return;
		curifnet = ifnethd = ifp;
		prifnet(curifnet);
	} else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		curifnet = addr;
		prifnet(curifnet);
	}
}

show_ndd()
{
	char **av = args;
	caddr_t addr, ddp;

	level = 1;
	cmdtab = nddtab;
	lastcommand = 0 ;
	prompt[1] = ".ndd";

	if (*av == 0) {
		/* the ndd symbol is a pointer to an dd struct.
		   so we have to read the value at that address, then
		   use the value at that address as the pointer to
		   the first ndd structure. */

		if (!readsymbol("ndd", &ddp, sizeof(ddp)))
			return;

		if (!ddp) {
			printf("No NDD entries available.\n");
			return;
		}
		else {
			curndd = nddhd = ddp;
			prndd(curndd);
		}
	} else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		curndd = addr;
		prndd(curndd);
	}
}

show_dmx()
{
	char **av = args;
	caddr_t addr, ddp;

	level = 1;
	cmdtab = dmxtab;
	lastcommand = 0 ;
	prompt[1] = ".dmx";

	if (*av == 0) {
		/* the demuxers symbol is a pointer to an ns_demuxer struct
		   so we have to read the value at that address, then
		   use the value at that address as the pointer to
		   the first demuxer structure. */

		if (!readsymbol("demuxers", &ddp, sizeof(ddp)))
			return;

		if (!ddp) {
			printf("No demuxer entries available.\n");
			return;
		}
		else {
			curdmx = dmxhd = ddp;
			prdmx(curdmx, 0);
		}
	} else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		curdmx = addr;
		prdmx(curdmx, 0);
	}
}

dmx_ndd()
{
	struct ndd 		ndd;

	if (!readaddr(curndd, &ndd, sizeof(ndd)))
		return;

	prdmx(ndd.ndd_demuxer, ndd.ndd_nsdemux);
	if (ndd.ndd_nsdemux != NULL)
		prfilters(ndd.ndd_nsdemux);
	if ( (ndd.ndd_specdemux != NULL) && (ndd.ndd_type == NDD_ISO88023) )
		eth_prfilters(ndd.ndd_specdemux);
}

prfilters(kernel_head)
struct ns_8022_user_head *kernel_head;
{
	struct ns_dmx_ctl 	dmx_ctl;
	struct ns_8022_user     *fup;
	struct ns_8022_user     *filter_head;
	struct ns_8022_user     nsuser;
	int			i;

	if (!readaddr(kernel_head, &dmx_ctl, sizeof(dmx_ctl)))
		return;
	printf("\n");
	printf("802.2 Filters:\n");
	printf("type DSAP org  etype  isr");
	printf("      isr_data   queue    fmt  netisr   ifp\n");
	printf("---- ---- ---  -----  ---");
	printf("      --------   -----    ---  ------   ---\n");
	for (i = 0; i < NS_MAX_SAPS; i++) {
		filter_head = (struct ns_8022_user *)
			&(dmx_ctl.dsap_heads[i]);
		fup = filter_head->next;
		if (fup != (struct ns_8022_user *)kernel_head)
			prsubfilters(kernel_head, fup);

		kernel_head++;
	}
}

prsubfilters(kernel_head, fup)
struct ns_8022_user_head *kernel_head;
struct ns_8022_user     *fup;
{
	struct ns_8022_user     nsuser;

	while (fup != (struct ns_8022_user *) kernel_head) {
		if (!readaddr(fup, &nsuser, sizeof(nsuser)))
			return;
		printf("%s %2X   ",
			gettype(nsuser.filter.filtertype, filtertypes,
				 "unknown"), nsuser.filter.dsap);
		printf("%X%X%X %4X", 
				nsuser.filter.orgcode[0],
				nsuser.filter.orgcode[1],
				nsuser.filter.orgcode[2],
				nsuser.filter.ethertype);
		printf("   %8x  %8x  %8x   %x      %d    %8x\n",
			nsuser.user.isr, nsuser.user.isr_data,	
			nsuser.user.protoq, nsuser.user.pkt_format,	
			nsuser.user.netisr, nsuser.user.ifp);
		fup = nsuser.next;
	}
}

eth_prfilters(kernel_head)
struct eth_user *kernel_head;
{
	struct eth_dmx_ctl      eth_dmx_ctl;
	struct eth_user		*fup;
	struct eth_user		*filter_head;
	int			i;

	if (!readaddr(kernel_head, &eth_dmx_ctl, sizeof(eth_dmx_ctl)))
		return;
	printf("\n");
	printf("Ethernet Filters:\n");
	printf("etype  isr       isr_data   queue    fmt  netisr   ifp\n");
	printf("-----  --------  --------   -----    ---  ------   -----\n");
	for (i = 0; i < ETHDMX_HASH ; i++) {
		filter_head = &eth_dmx_ctl.hash_heads[i];
		fup = filter_head->next;
		if (fup != (struct eth_user *)kernel_head)
			eth_prsubfilters(kernel_head, fup);
		kernel_head++;
	}
}

eth_prsubfilters(kernel_head, fup)
struct ns_8022_user_head *kernel_head;
struct eth_user     *fup;
{
	struct eth_user     eth_user;

	while (fup != (struct eth_user *) kernel_head) {
		if (!readaddr(fup, &eth_user, sizeof(eth_user)))
			return;
		printf("%4X", eth_user.filter.ethertype);
		printf("  %8x  %8x  %8x   %x      %d    %8x\n",
			eth_user.user.isr, eth_user.user.isr_data,	
			eth_user.user.protoq, eth_user.user.pkt_format,	
			eth_user.user.netisr, eth_user.user.ifp);
		fup = eth_user.next;
	}
}

head_tcb()
{
	curtcb = tcbhd;
	prinpcb(curtcb);
}

show_mbuf()
{
	char **av = args;
	caddr_t addr = 0;

	if (cmdtab != mbuftab) {
		cmdtab = mbuftab;
		lastcommand = 0 ;
		level++;
		prompt[level] = ".mbuf";
	}
	
	if (*av != 0) {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		curmbuf = addr;
	} else {
		printf("You didn't give me an address to look at\n");
		return;
	}

	prmbuf(curmbuf);
}

show_sock()
{
	char **av = args;
	int addr = 0;
	struct socket sockbuf;

	/* must have an argument */
	if (args[0] != 0) {
		sscanf(args[0], "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", args[0]);
			return;
		}
	} else {
		printf("You didn't give me an address to look at\n");
		return;
	}
	readaddr(addr, &sockbuf, sizeof(sockbuf));
	curtcb = sockbuf.so_pcb;
	prsock(addr);
}

show_tcb()
{
	char **av = args;
	caddr_t addr;

	level = 1;
	cmdtab = tcbtab;
	lastcommand = 0 ;
	prompt[1] = ".tcb";
	proto = TCP;

	if (*av == 0)
		prinpcb(curtcb);
	else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		curtcb = addr;
		prinpcb(curtcb);
	}
}

show_tcpcb()
{
	char **av = args;
	caddr_t addr;
	struct inpcb inpcb;
	struct tcpcb tcpcbuf;

	level = 1;
	cmdtab = tcbtab;
	lastcommand = 0 ;
	prompt[1] = ".tcpcb";
	proto = TCP;
	
	if (*av == 0) {
		readaddr(curtcb, &inpcb, sizeof(inpcb));
		prtcpcb(inpcb.inp_ppcb);
	} else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		readaddr(addr, &tcpcbuf, sizeof(tcpcbuf));
		curtcb = (caddr_t)tcpcbuf.t_inpcb;
		prtcpcb(addr);
	}
}

show_udb()
{
	char **av = args;
	caddr_t addr;

	level = 1;
	cmdtab = udbtab;
	lastcommand = 0 ;
	prompt[1] = ".udb";
	proto = UDP;

	if (*av == 0)
		prinpcb(curudb);
	else {
		sscanf(*av, "%x", &addr);
		if (addr <= (caddr_t)0) {
			printf("Yoicks! Invalid argument: %s\n", *av);
			return;
		}
		prinpcb(addr);
	}
}

snd_mbuf()
{
	struct inpcb inpcb;
	struct socket sockbuf;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	readaddr(inpcb.inp_socket, &sockbuf, sizeof(sockbuf));

	if (sockbuf.so_snd.sb_mb) {
		curmbuf = (caddr_t)sockbuf.so_snd.sb_mb;
		mbufhdnext = mbufhdact = curmbuf;
		show_mbuf();
	} else
		printf("the send buffer has no mbufs\n");
}

rcv_mbuf()
{
	struct inpcb inpcb;
	struct socket sockbuf;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	readaddr(inpcb.inp_socket, &sockbuf, sizeof(sockbuf));

	if (sockbuf.so_rcv.sb_mb)
		prmbuf(sockbuf.so_rcv.sb_mb);
	else
		printf("the receive buffer has no mbufs\n");
}

mbuf_next_act()
{
	struct mbuf mb;

	readaddr(curmbuf, &mb, sizeof(mb));
	if (mb.m_act) {
		curmbuf = (caddr_t)mb.m_act;
		mbufhdnext = curmbuf;
		prmbuf(curmbuf);
	} else
		printf("at end of act chain\n");
}

mbuf_next()
{
	struct mbuf mb;

	readaddr(curmbuf, &mb, sizeof(mb));
	if (mb.m_next) {
		curmbuf = (caddr_t)mb.m_next;
		prmbuf(curmbuf);
	} else
		printf("at end of mbuf list\n");
}

mbuf_head_act()
{
	curmbuf = mbufhdact;
	prmbuf(curmbuf);
}

mbuf_head_next()
{
	curmbuf = mbufhdnext;
	prmbuf(curmbuf);
}

char *
getflags(val, flaglist)
	int val;
	struct flagent *flaglist;
{
	static char buf[128];
	struct flagent *f = flaglist;

	buf[0] = '\0';
	while (f->name) {
		if (val & f->bit) {
			if (buf[0]) strcat(buf, "|");
			strcat(buf, f->name);
		}
		f++;
	}
	return(buf);
}

char *
gettype(val, typelist, dflt)
	int val;
	struct typeent *typelist;
	char *dflt;
{
	struct typeent *t = typelist;

	while (t->name) {
		if (val == t->val) {
			return(t->name);
		}
		t++;
	}
	return(dflt);
}

next_tcb()
{
	static struct inpcb inpcb;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	curtcb = (caddr_t)inpcb.inp_next;
	prinpcb(curtcb);
}

prev_tcb()
{
	static struct inpcb inpcb;
	
	readaddr(curtcb, &inpcb, sizeof(inpcb));
	curtcb = (caddr_t)inpcb.inp_prev;
	prinpcb(curtcb);
}
	
next_udb()
{
	static struct inpcb inpcb;
	
	readaddr(curudb, &inpcb, sizeof(inpcb));
	curudb = (caddr_t)inpcb.inp_next;
	prinpcb(curudb);
}
	
prev_udb()
{
	static struct inpcb inpcb;
	
	readaddr(curudb, &inpcb, sizeof(inpcb));
	curudb = (caddr_t)inpcb.inp_prev;
	prinpcb(curudb);
}
	
top_level()
{
	level = 0;
	cmdtab = ndbtab;
}

sayonara_sucker()
{
	printf("\nExiting.\n");
	exit(0);
}

prhelp()
{
	printf(
"\tWelcome to ndb, a crash-like network debugger for network kernel\n\
\tstructures.  At any prompt type a \"?\" for context sensitive help.\n\
\tThere are 2 main functions currently supported by ndb:\n\n\
\t    * show sockets and related control blocks and buffers, allowing\n\
\t      you to move back and forth along the chain of control blocks.\n\n\
\t    * show mbuf structures, following either the next pointer, or\n\
\t      the m_act pointer.\n\n\
\t    * show interface structures, following the chain from the head\n\
\t      of the interface list.\n\n\
\tThings to remember:\n\
\t    - When you just hit ENTER, the last command will be redone.\n\n\
\tfor suggestions, comments and help, send mail to doug@robo\n\n"
	       );

}

prmenu()
{
	struct cmdent *c = cmdtab;
	
	printf("\n\nHelp stuff:\n\n");
	while (c->cmd) {
		printf("  %-15s  %s", c->cmd, c->help);
		c++;
	}
}

prinpcb(inp_addr)
	caddr_t inp_addr;
{
	struct inpcb tcbbuf, *inp;
	struct hostent *hp;
	char lhost[128], fhost[128];

	readaddr(inp_addr, &tcbbuf, sizeof(tcbbuf));
	inp = &tcbbuf;

	hp = gethostbyaddr(&inp->inp_faddr, sizeof(inp->inp_faddr), AF_INET);
	strcpy(fhost, (hp) ? hp->h_name : "NONE");
	
	hp = gethostbyaddr(&inp->inp_laddr, sizeof(inp->inp_laddr), AF_INET);
	strcpy(lhost, (hp) ? hp->h_name : "NONE");

	printf("    (%s%s----------- INPCB  INFO -------------------\n",
	       (proto == TCP) ? "TCB" : "UDB",
	       ((proto == TCP && curtcb == tcbhd) ||
		(proto == UDP && curudb == udbhd)) ? "-HEAD)" : ")-----");

	printf("    next:0x%08x    prev:  0x%08x    head: 0x%08x\n",
	       inp->inp_next, inp->inp_prev, inp->inp_head);
	printf("    ppcb:0x%08x    socket:0x%08x    flags:0x%08x\n\n",
	       inp->inp_ppcb, inp->inp_socket, inp->inp_flags);
	printf("    lport:%5d   laddr:0x%08x (%s)\n",
	       inp->inp_lport, inp->inp_laddr, lhost);
	printf("    fport:%5d   faddr:0x%08x (%s)\n\n",
	       inp->inp_fport, inp->inp_faddr, fhost);

	prsock(inp->inp_socket);
}

prsock(sockaddr)
	caddr_t sockaddr;
{
	struct socket sockbuf, *so;

	if (sockaddr == 0)
		return;

	readaddr(sockaddr, &sockbuf, sizeof(sockbuf));
	so = &sockbuf;
	
	printf("    --------------------- SOCKET INFO -------------------\n");
	printf("    type:0x%04x (%s)    opts:0x%04x  (%s)\n",
	       so->so_type, gettype(so->so_type, socktypes, "BOGUS"),
	       so->so_options, getflags(so->so_options, sockoptflags));
	printf("    state:0x%04x  (%s)\n",
	       so->so_state, getflags(so->so_state, sockstates));
	printf("    linger:0x%04x  pcb:0x%08x  proto:0x%08x\n",
	       so->so_linger, so->so_pcb, so->so_proto);
	printf("    q0:0x%08x  q0len:%5d q:0x%08x\n", 
	       so->so_q0, so->so_q0len, so->so_q);
	printf("    qlen:%5d  qlimit:%5u  head:0x%08x\n",
	       so->so_qlen, so->so_qlimit, so->so_head);
	printf("    timeo:%5d  error:%5u  oobmark:%5u  pgid:%5u\n\n",
	       so->so_timeo, so->so_error, so->so_oobmark,
	       so->so_pgid);
	prsock_sb("snd", &(so->so_snd));
	printf("\n");
	prsock_sb("rcv", &(so->so_rcv));
}

prsock_sb( which, sb )
	char *which;		/* say which one you are */
	struct sockbuf *sb;
{
	printf("    %s: cc:%5u  hiwat:%5u  mbcnt:%5u  mbmax:%5u\n", which,
	    	sb->sb_cc, sb->sb_hiwat, sb->sb_mbcnt, sb->sb_mbmax);
#ifdef _5A
	printf("    lowat:%5u  mb:0x%08x\n",
	    	sb->sb_lowat, sb->sb_mb);
#else
	printf("    lowat:%5u  mb:0x%08x  events:0x%4x\n",
	    	sb->sb_lowat, sb->sb_mb, sb->sb_reqevents);
#endif
	printf("    iodone:0x%08x  ioargs:0x%08x ",
		sb->sb_iodone,sb->sb_ioarg);
	printf("    flags:0x%04x  (%s)\n",
	       sb->sb_flags, getflags(sb->sb_flags, sbflags));
}

prtcpcb(tcpcbaddr)
	caddr_t tcpcbaddr;
{
	struct tcpcb tcpcb, *tp;
	int i;

	readaddr(tcpcbaddr, &tcpcb, sizeof(tcpcb));
	tp = &tcpcb;
	
	printf("  ------------------------ TCPCB ----------------------\n");
	printf("  seg_next 0x%08x  seg_prev 0x%08x  t_state 0x%02x  (%s)\n",
	       tp->seg_next, tp->seg_prev, tp->t_state,
	       gettype(tp->t_state, tcpstates, ""));
	printf("  timers:");
	for (i=0; i<TCPT_NTIMERS; i++)
		printf("   %s:%d", gettype(i, tcptimers, ""), tp->t_timer[i]);
	printf("\n");
	printf("  t_txtshift %d  t_txtcur %d  t_dupacks %d  t_maxseg %d  t_force %1d\n",
	       tp->t_rxtshift, tp->t_rxtcur, tp->t_dupacks,
	       tp->t_maxseg, tp->t_force);
	printf("  flags:0x%04x  (%s)\n",
	       tp->t_flags, getflags(tp->t_flags, tcpcbflags));
	printf("  t_template 0x%08x  inpcb 0x%08x  snd_cwnd:%05d  snd_ssthresh:%05d\n",
	       tp->t_template, tp->t_inpcb, tp->snd_cwnd, tp->snd_ssthresh);
	printf("  snd_una=%05d  snd_nxt=%05d  snd_up=%05d  snd=wl1=%05d  snd_wl2=%05d  iss=%05d\n",
	       tp->snd_una, tp->snd_nxt, tp->snd_up, tp->snd_wl1, tp->snd_wl2,
	       tp->iss);
	printf("  snd_wnd:%6d  rcv_wnd:%6d\n", tp->snd_wnd, tp->rcv_wnd);
	printf("  t_idle=%05d  t_rtt=%05d t_rtseq=%05d  t_srtt=%05d  t_rttvar=%05d\n",
	       tp->t_idle, tp->t_rtt, tp->t_rtseq, tp->t_srtt, tp->t_rttvar);
	printf("  max_sndwnd=%05d  t_iobc=0x%02x  t_oobflags=0x%02x (%s)\n",
	       tp->max_sndwnd, tp->t_iobc, tp->t_oobflags,
	       gettype(tp->t_oobflags, oobflags, ""));
}

long getaddr(sym)
{
	struct nlist nl[2];

	nl[0].n_name = sym;
	nl[1].n_name = 0;

	if (!rflag) {
		nlist(unix_path, nl);
		if (nl[0].n_value == 0) {
			nlist(netinet_path, nl);
			if (nl[0].n_value)
				nl[0].n_value += netinet_base;
		}
	}
#ifndef _5A
	else
		knlist(&nl, 1, sizeof(struct nlist));
#endif
	if (nl[0].n_value == 0) {
		fprintf(stderr, "can not find %s\n", sym);
		return 0;
	}

#ifndef _5A
	if (!rflag && nl[0].n_sclass == C_HIDEXT) {
		if (!readaddr(nl[0].n_value, &nl[0].n_value, sizeof(nl[0].n_value))) {
			perror("readsymbol: read fail");
			return 0;
		}
	}
#endif
		
	printf("address of %s is 0x%08x\n", sym, nl[0].n_value);
	return nl[0].n_value;
}

readsymbol(sym, buf, bytes)
	char *sym, *buf;
	int bytes;
{
	long symval;

	if (!(symval = getaddr(sym)) ||
	    !readaddr(symval, buf, bytes)) {
		perror("readsymbol: read fail");
		return 0;
	}

	return 1;
}

prmbuf(addr)
	caddr_t addr;
{
	struct	mbuf	mbuf;
	char		*type;
	int		dptr;
	int		buf[4];
	register int	i,j,size;
	register char 	*p;
	
#ifndef _5A
	if ((int)addr & (sizeof(struct mbuf) - 1)) {
		printf("invalid mbuf ptr:%8x, must be on %d byte boundary!\n",
		       addr, sizeof(struct mbuf));
		return;
	}
#endif
	if (!readaddr(addr, &mbuf, sizeof(mbuf)))
		return;
	
	dptr = mbuf.m_data;
	printf("\n    addr:0x%08x    dptr:0x%08x     m_nextpkt:0x%08x\n",
	       addr, dptr, mbuf.m_nextpkt);
	printf("    m_next:0x%08x  data:0x%08x  len:%5d  type:%s\n",
	       mbuf.m_next, mbuf.m_data, mbuf.m_len,
	       gettype(mbuf.m_type, mbuftypes, "weird"),
	       mbuf.m_nextpkt);
	printf("    ---------------------------------------\n");

	if (mbuf.m_len > 0 && mbuf.m_len <= MLEN)
		size = mbuf.m_len;
	else
		size = MLEN;
	
	for (; size>0; dptr += 4*sizeof(int)) {
		p = (char *)buf;
		
		printf("    ");
		if (!readaddr(dptr, buf, sizeof(buf)))
			return;
		
		for (i=j=0; j<16;j++) {
			if (size) {
				size--; 
				i++;
				printf("%02x",*p++);
			} else
				printf("  ");
			if (!((j+1)%4))
				printf(" ");
		}
		printf(" |%-16s|\n",
		       makeprintable(buf, size>0 ? 16 : i));
	}
}

char *
makeprintable(buf, len)
	unsigned char *buf;
	int len;
{
	int i;
	static unsigned char outbuf[128];

	for (i=0; i<len; i++) {
		if (buf[i]< 0x20 || buf[i] > 0xFE)
			outbuf[i] = '.';
		else
			outbuf[i] = buf[i];
	}
	outbuf[i] = '\0';
	return(outbuf);
}

next_ifnet()
{
	struct ifnet ifnet;
	
	readaddr(curifnet, &ifnet, sizeof(ifnet));
	if (ifnet.if_next) {
		curifnet = (caddr_t)ifnet.if_next;
		prifnet(curifnet);
	} else
		printf("at end of ifnet chain\n");
}

next_ndd()
{
	struct ndd ndd;
	
	readaddr(curndd, &ndd, sizeof(ndd));
	if (ndd.ndd_next) {
		curndd = (caddr_t)ndd.ndd_next;
		prndd(curndd);
	} else
		printf("at end of ndd chain\n");
}

next_dmx()
{
	struct ns_demuxer dmx;
	
	readaddr(curdmx, &dmx, sizeof(dmx));
	if (dmx.nd_next) {
		curdmx = (caddr_t)dmx.nd_next;
		prdmx(curdmx, 0);
	} else
		printf("at end of demuxer chain\n");
}

prifnet(addr)
	caddr_t addr;
{
	struct ifaddr *ifaddrp, ifaddr;
	struct ifnet interface, *i;
	char name[12], *cp, host[64];
	struct sockaddr_in *sin;
	struct hostent *hp;
	int n;
	struct sockaddr sa;

	i = &interface;
	ifaddrp = &ifaddr;
	readaddr(addr, &interface, sizeof(interface));
	if (i->if_name && readaddr(i->if_name, name, sizeof(name)-1)) {
		name[sizeof(name)-2] = '\0';
		cp = strchr(name, '\0');
		*cp++ = i->if_unit + '0';
		if ((i->if_flags&IFF_UP) == 0)
			*cp++ = '*';
		*cp = '\0';
	} else
		name[0] = '\0';

	printf("    ---------------------- IFNET INFO -----(@0x%08x)----------\n",
	       addr);
	printf("    name: %-8.8s  unit:%01d  mtu:%10lu  if_next:0x%08x\n",
	       name, i->if_unit, i->if_mtu, i->if_next);
	printf("    flags:0x%08x \n\t(%s)\n",
	       i->if_flags, getflags(i->if_flags, ifflags));
	printf("    timer:%05d  metric:%02d\n\n", i->if_timer, i->if_metric);
        if (!readaddr(i->if_addrlist,ifaddrp,sizeof(struct ifaddr)))
             return;

	while ((ifaddrp->ifa_addr) && 
	    readaddr(ifaddrp->ifa_addr, &sa, sizeof(struct sockaddr)) ) {
		switch (sa.sa_family) {
		case AF_UNSPEC:
			printf("\tUNIX family socket, no address\n");
			break;
		case AF_INET:
			sin = (struct sockaddr_in *)&sa;
			printf("\t    address: %s  ",
			       inet_ntoa(sin->sin_addr));
			hp = gethostbyaddr(&sin->sin_addr,
					   sizeof(sin->sin_addr),
					   AF_INET);
			strcpy(host, (hp) ? hp->h_name : "NONE");
			printf("hostname: %s\n", host);
			break;
		}
		if ((ifaddrp->ifa_next == NULL) || 
		   !(readaddr(ifaddrp->ifa_next,ifaddrp,sizeof(struct ifaddr))))
                        break;
	}
	printf("\n");
	printf("    ifq_head:0x%08x  if_init():0x%08x      ipackets:%08d\n",
	       i->if_snd.ifq_head, i->if_init, i->if_ipackets);
	printf("    ifq_tail:0x%08x  if_output():0x%08x    ierrors: %05d\n",
	       i->if_snd.ifq_tail, i->if_output, i->if_ierrors);
	printf("    ifq_len:%05d        if_ioctl():0x%08x     opackets:%08d\n",
	       i->if_snd.ifq_len, i->if_ioctl, i->if_opackets);
	printf("    ifq_maxlen:%05d     if_reset():0x%08x     oerrors: %05d\n",
	       i->if_snd.ifq_maxlen, i->if_reset, i->if_oerrors);
	printf("    ifq_drops:%05d      if_watchdog():0x%08x\n",
	       i->if_snd.ifq_drops, i->if_watchdog);
	
}

prndd(addr)
	caddr_t addr;
{
	struct ndd interface, *i;
	char name[12], *cp, host[64];
	char alias[12];
	int n;
	int a;
	char physaddr[64];

	i = &interface;
	readaddr(addr, &interface, sizeof(interface));
	readaddr(i->ndd_name, name, sizeof(name)-1);
	name[sizeof(name)-2] = '\0';
	cp = strchr(name, '\0');
	if ((i->ndd_flags&NDD_UP) == 0)
		*cp++ = '*';
	*cp = '\0';

	if (i->ndd_alias && readaddr(i->ndd_alias, alias, sizeof(alias)-1)) {
		alias[sizeof(alias)-1] = '\0';
	} else
		alias[0] = '\0';

	printf("    ---------------------- NDD INFO -----(@0x%08x)----------\n",
	       addr);
	printf("    name: %-8.8s\t alias: %s	ndd_next:0x%08x\n",
	       name,  alias[0] ? alias : "none", i->ndd_next);
	printf("    flags:0x%08x \t(%s) \n",
	       i->ndd_flags, getflags(i->ndd_flags, ddflags));
	printf("\n");

	printf("    ndd_open():  0x%08x  ndd_close():0x%08x  ndd_output():0x%08x\n",
	       i->ndd_open, i->ndd_close, i->ndd_output);
	printf("    ndd_ctl():   0x%08x  ndd_stat(): 0x%08x  receive():   0x%08x\n\n",
	       i->ndd_ctl, i->nd_status, i->nd_receive);

	printf("    ndd_correlator: 0x%08x   ndd_refcnt:%10d \n", 
		i->ndd_correlator, i->ndd_refcnt);
	printf("    ndd_mtu:        %8d     ndd_mintu:    %8d\n",
	       i->ndd_mtu, i->ndd_mintu);
	printf("    ndd_addrlen:    %8d     ndd_hdrlen:   %8d \n",
	       i->ndd_addrlen, i->ndd_hdrlen);
	readaddr(i->ndd_physaddr, physaddr, 64);
	printf("    ndd_physaddr: ");
	for (a = 0 ; a < i->ndd_addrlen ; a++) 
		printf("%02x", physaddr[a]);
	printf("   ndd_type:    %8d (%s)\n",
	       i->ndd_type, gettype(i->ndd_type, dmxtypes, "unknown"));
	printf("\n");
	printf("    ndd_demuxer:    0x%08x\tndd_nsdemux:     0x%08x \n", 
		i->ndd_demuxer, i->ndd_nsdemux);
	printf("    ndd_specdemux:  0x%08x\tndd_demuxsource: %d\n", 
		i->ndd_specdemux, i->ndd_demuxsource);
	printf("    ndd_demux_lock: 0x%08x\tndd_lock:        0x%08x \n", 
		i->ndd_demux_lock, i->ndd_lock);
	printf("    ndd_trace:      0x%08x\tndd_trace_arg:   0x%08x \n", 
		i->ndd_trace_arg, i->ndd_trace_arg);
	printf("    ndd_specstats:  0x%08x\tndd_speclen:     %d \n", 
		i->ndd_specstats, i->ndd_speclen);
	printf("\n");
	printf("    ndd_ipackets:   %12u    ndd_opackets:     %12u\n",
	       i->ndd_genstats.ndd_ipackets_lsw, i->ndd_genstats.ndd_opackets_lsw);
	printf("    ndd_ierrors:    %12u    ndd_oerrors:      %12u\n",
	       i->ndd_genstats.ndd_ierrors, i->ndd_genstats.ndd_oerrors);
	printf("    ndd_ibytes:     %12u    ndd_obytes:       %12u\n",
	       i->ndd_genstats.ndd_ibytes_lsw, i->ndd_genstats.ndd_obytes_lsw);
	printf("    ndd_recvintr:   %12u    ndd_xmitintr:     %12u\n",
	       i->ndd_genstats.ndd_recvintr_lsw, i->ndd_genstats.ndd_xmitintr_lsw);
	printf("    ndd_ipackets_drop: %9u    ndd_nobufs:       %12u\n",
	       i->ndd_genstats.ndd_ipackets_drop, i->ndd_genstats.ndd_nobufs);
	printf("    ndd_xmitque_max:%12u    ndd_xmitque_ovf:  %12u",
	       i->ndd_genstats.ndd_xmitque_max,i->ndd_genstats.ndd_xmitque_ovf);
}

prdmx(addr, ctl_addr)
	caddr_t addr;
	caddr_t ctl_addr;
{
	int i;
	struct ns_demuxer d;
	struct ns_dmx_ctl dmx_ctl;

	if (!readaddr(addr, &d, sizeof(d)))
		return;

	printf("    ------------------ DEMUXER INFO -----(@0x%08x)----------\n",
	       addr);
	printf("    type: %d (%s)\tnd_next:0x%08x\n\n",
	       d.nd_type, gettype(d.nd_type, dmxtypes, "unknown"), d.nd_next);
	printf("    nd_add_filter():  0x%08x  nd_del_filter():0x%08x\n",
	       d.nd_add_filter, d.nd_del_filter);
	printf("    nd_add_status():  0x%08x  nd_del_status():0x%08x\n",
	       d.nd_add_status, d.nd_del_status);
	printf("    nd_receive():     0x%08x  nd_status():    0x%08x\n",
	       d.nd_receive, d.nd_status);
	printf("    nd_response():    0x%08x\n", d.nd_response);
	printf("    nd_speclen:	        %d  nd_specstats:   0x%08x\n",
		d.nd_speclen, d.nd_specstats);
	printf("\n");

	if (!ctl_addr)
		return;

	if (!readaddr(ctl_addr, &dmx_ctl, sizeof(dmx_ctl)))
		return;
	printf("    nd_nofilter:   %12u    ndd_nobufs:     %12u\n",
	       dmx_ctl.nd_dmxstats.nd_nofilter, dmx_ctl.nd_dmxstats.nd_nobufs);
	printf("    nd_bcast:      %12u    ndd_mcast:      %12u\n",
	       dmx_ctl.nd_dmxstats.nd_bcast, dmx_ctl.nd_dmxstats.nd_mcast);

}
