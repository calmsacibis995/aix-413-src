/* @(#)50       1.4  src/bos/usr/sbin/crash/ndb.h, cmdcrash, bos411, 9434B411a 8/24/94 17:22:02 */
/*
 * COMPONENT_NAME: (CMDCRASH)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
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
int prmenu_ndb(), prmenu_tcb(), prmenu_udb(), prmenu_ifnet(), prmenu_sock(), prmenu_mbuf();
int prhelp(), show_tcb(), show_tcpcb(), show_udb(), show_inpcb();
int show_mbuf(), show_sock(), show_ifnet(), next_ifnet();
int head_tcb(), snd_mbuf(), rcv_mbuf();
int mbuf_next_act(), mbuf_next(), mbuf_head_act(), mbuf_head_next();
int next_tcb(), prev_tcb(), next_udb(), prev_udb();
int top_level(), exit_func();
int ndbmain();

char *makeprintable(), *gettype(), *getflags();

#define readaddr(addr, buf, size) readmem(buf,addr,size,rflag)


caddr_t tcbhd, curtcb, udbhd, curudb, curmbuf, mbufhdnext, mbufhdact,
	curifnet, ifnethd;

int proto;			/* are we doing: TCP or UDP */
int mbufflag = 0;

struct cmdent {
	char *cmd;
	char *help;
	int (*fun)();
};

/* Make sure that these messages  are consistent with prmenu_xxx() in ndb.c */       
struct cmdent ndbtab[] = {
        { "?", "help - you're reading it\n", prmenu_ndb },
	{ "help", "a more lengthy help text\n", prhelp },
	{ "tcb [addr]", "show TCBs, default is HEAD TCB\n", show_tcb },
	{ "udb [addr]", "show UDBs, default is HEAD UDB\n", show_udb },
	{ "sockets addr", "show sockets at given addr\n", show_sock }, 
	{ "mbuf [addr]", "show mbuf at address, default is enter mbuf menu\n", show_mbuf },
	{ "ifnet [addr]", "show ifnet structures, default is \"ifnet\"\n",
		  show_ifnet },
	{ "quit", "exit ndb\n", 0 },
	{ "xit", "exit ndb\n", 0 },
	{ 0 }
};

struct cmdent tcbtab[] = {
	{ "?", "help - you're reading it\n", prmenu_tcb },
	{ "next", "go to next tcb/socket\n", next_tcb },
	{ "previous", "go to previous tcb/socket\n", prev_tcb },
	{ "tcb [addr]", "reread tcb/socket, default is current\n", show_tcb },
	{ "ctrlblk [addr]", "read the control block\n", show_tcpcb },
	{ "sendmbufs", "show the send mbufs\n", snd_mbuf },
	{ "recvmbufs", "show the receive mbufs\n", rcv_mbuf },
	{ "sock addr", "show socket at given address\n", show_sock },
	{ "head", "show the head tcb in the chain\n", head_tcb },
        { "quit", "return to top_level\n", top_level },
        { "xit", "exit ndb\n", 0 },
        { 0 }
};

struct cmdent udbtab[] = {
	{ "?", "help - you're reading it\n", prmenu_udb },
	{ "next", "go to next udb\n", next_udb },
	{ "previous", "go to previous udb\n", prev_udb },
        { "quit", "return to top_level\n", top_level },
        { "xit", "exit ndb\n", 0 },
        { 0 }
};

struct cmdent ifnettab[] = {
	{ "?", "help - you're reading it\n", prmenu_ifnet },
	{ "head", "go to head of ifnet list\n", show_ifnet },
	{ "next", "go to next ifnet struct\n", next_ifnet },
        { "quit", "return to top_level\n", top_level },
        { "xit", "exit ndb\n", 0},
        { 0 }
};

struct cmdent socktab[] = {
	{ "?", "help - you're reading it\n", prmenu_sock },
	{ "socket addr", "show socket at addr\n", show_sock },
        { "quit", "return to top_level\n", top_level },
        { "xit", "exit ndb\n", 0 },
        { 0 }
};

struct cmdent mbuftab[] = {
	{ "?", "help - you're reading it\n", prmenu_mbuf },
	{ "addr", "show mbuf at address\n",
		  show_mbuf },
        { "quit", "return to top_level\n", top_level },
        { "xit", "exit ndb\n", 0 },
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
char *prompt1[6];
char *args[10];
char *unix_path = "/unix";
char *netinet_path = 0;
int rflag = 0;
unsigned long netinet_base = 0;
caddr_t tcbaddr, udbaddr;
