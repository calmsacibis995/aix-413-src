static char sccsid[] = "@(#)60	1.8  src/tcpip/usr/sbin/gated/parse.c, tcprouting, tcpip411, GOLD410 12/6/93 17:54:16";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: MSGSTR
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
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
 *  parse.c,v 1.42.2.4 1993/09/28 18:32:21 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_DIRENT
#define	INCLUDE_ROUTE
#include "include.h"
#include "parse.h"
#include "krt.h"
#ifdef	PROTO_RIP
#include "rip.h"
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
#include "hello.h"
#endif	/* PROTO_HELLO */
#ifdef	PROTO_OSPF
#include "ospf.h"
#endif	/* PROTO_OSPF */
#ifdef	PROTO_EGP
#include "egp.h"
#endif	/* PROTO_EGP */
#ifdef	PROTO_BGP
#include "bgp.h"
#endif	/* PROTO_BGP */
#ifdef	PROTO_SLSP
#include "slsp.h"
#endif	/* PROTO_SLSP */
#ifdef	PROTO_ISIS
#include "isis.h"
#endif	/* PROTO_ISIS */
#ifdef	PROTO_DVMRP
#include "dvmrp.h"
#endif	/* PROTO_DVMRP */
#include "parser.h"

#ifdef _POWER
#include "gated_msg.h"
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PARSE,n,s)
#else
#define MSGSTR(n,s) s
#endif /* _POWER */

static int n_keywords;

/*
 *	Table of keywords recognized.  This table is sorted at runtime
 *	to facilitate binary searching.
 */

static bits keywords[] =
{
    {T_INTERFACE, "interface"},
    {T_INTERFACE, "intf"},
    {T_INTERFACES, "interfaces"},
    {T_INTERFACES, "intfs"},
    {T_LCLADDR, "localaddress"},
    {T_LCLADDR, "local-address"},
    {T_LCLADDR, "lcladdr"},
    {T_LCLADDR, "lcl-addr"},
    {T_DIRECT, "direct"},
    {T_DOWN, "down"},
    {T_PROTO, "protocol"},
    {T_PROTO, "proto"},
    {T_METRIC, "metric"},
    {T_METRIC, "distance"},
    {T_METRICIN, "metricin"},
    {T_METRICOUT, "metricout"},
    {T_INFINITY, "infinity"},
    {T_INFINITY, "unreachable"},
    {T_PARSE, "parse"},
    {T_ADV,	"adv"},
    {T_ADV,	"adv_entry"},
    {T_DEFAULT, "default"},
#ifdef	PROTO_INET
    {T_INET,	"inet"},
#endif	/* PROTO_INET */
#ifdef	PROTO_ISO
    {T_ISO,	"iso"},
#endif	/* PROTO_ISO */
    {T_DEFAULTS, "defaults"},
    {T_DEBUG, "debug"},
#if	YYDEBUG != 0
    {T_YYDEBUG, "yydebug"},
    {T_YYSTATE, "yystate"},
    {T_YYQUIT, "yyquit"},
#endif	/* YYDEBUG */

    {T_SYSLOG, "syslog"},
    {T_UPTO,	"upto"},
    {T_EMERG, "emerg" },
    {T_ALERT, "alert"},
    {T_CRIT, "crit"},
    {T_ERR, "err"},
    {T_WARNING, "warning"},
    {T_NOTICE, "notice"},
    {T_INFO, "info"},
/*    {T_DEBUG, "debug"},	Defined above */
    
    {T_ON, "on"},
    {T_ON, "yes"},
    {T_OFF, "off"},
    {T_OFF, "no"},
    {T_GATEWAY, "gateway"},
    {T_GATEWAY, "gw"},
    {T_GATEWAY, "router"},
    {T_GATEWAY, "rtr"},
    {T_PREFERENCE, "preference"},
    {T_PREFERENCE, "pref"},
    {T_DEFAULTMETRIC, "defaultmetric"},
    {T_BROADCAST, "broadcast"},
    {T_NONBROADCAST, "nobroadcast"},
    {T_NONBROADCAST, "no-broadcast"},
    {T_NONBROADCAST, "nonbroadcast"},
    {T_NONBROADCAST, "non-broadcast"},
    {T_NONBROADCAST, "nbma"},
    {T_POINTOPOINT,	"pointtopoint"},
    {T_POINTOPOINT,	"pointopoint"},
    {T_POINTOPOINT,	"point2point"},
    {T_POINTOPOINT,	"p2p"},
#ifdef	IP_MULTICAST
    {T_MULTICAST,	"multicast" },
#endif	/* IP_MULTICAST */
    {T_NETMASK,		"netmask" },
    {T_NETMASK,		"net-mask" },
    {T_NETMASK,		"subnetmask" },
    {T_DEFINE,		"define" },
    {T_TYPE, "type"},
    /* Authentication */
    {T_AUTHTYPE, "authenticationtype"},
    {T_AUTHTYPE, "authentication-type"},
    {T_AUTHTYPE, "authtype"},
    {T_AUTHTYPE, "auth-type"},
    {T_AUTHKEY, "authenticationkey"},
    {T_AUTHKEY, "authentication-key"},
    {T_AUTHKEY, "authkey"},
    {T_AUTHKEY, "auth-key"},
    {T_NONE, "none"},
    {T_SIMPLE, "simple"},
    {T_SIMPLE, "simplepassword"},
    {T_SIMPLE, "simple-password"},
#if	defined(PROTO_EGP) || defined(PROTO_BGP)
    {T_NEIGHBOR, "neighbor"},
    {T_NEIGHBOR, "peer"},
    {T_GROUP, "group"},
    {T_PEERAS, "asin"},
    {T_PEERAS, "peeras" },
    {T_LOCALAS, "asout"},
    {T_LOCALAS, "localas" },
    {T_VERSION, "version"},
#ifdef	PROTO_INET
    {T_GENDEFAULT, "gendefault"},
    {T_GENDEFAULT, "originatedefault"},
    {T_GENDEFAULT, "originate-default"},
    {T_NOGENDEFAULT, "nogendefault"},
    {T_DEFAULTIN,	"importdefault"},
    {T_DEFAULTOUT,	"exportdefault"},
#endif	/* PROTO_INET */
#endif				/* defined(PROTO_EGP) || defined(PROTO_BGP) */
#ifdef	PROTO_EGP
    {T_MAXUP, "maxup"},
    {T_SOURCENET, "sourcenet"},
    {T_P1, "p1"},
    {T_P1, "minhello"},
    {T_P2, "p2"},
    {T_P2, "minpoll"},
    {T_PKTSIZE, "packetsize"},
    {T_PKTSIZE, "packet-size"},
#endif	/* PROTO_EGP */
#if	defined(PROTO_EGP) || defined(PROTO_ASPATHS)
    {T_EGP, "egp"},
#endif	/* defined(PROTO_EGP) || defined(PROTO_ASPATHS) */
#ifdef	PROTO_BGP
    {T_BGP, "bgp"},
    {T_HOLDTIME, "holdtime"},
    {T_TEST, "test" },
    {T_KEEPALL, "keepall"},
    {T_KEEPALL, "keep-all"},
    {T_SENDBUF, "sendbuf"},
    {T_SENDBUF, "sendbuffer"},
    {T_SENDBUF, "send-buffer"},
    {T_RECVBUF, "recvbuf"},
    {T_RECVBUF, "recvbuffer"},
    {T_RECVBUF, "recv-buffer"},
    {T_SPOOLBUF, "spoolbuf"},
    {T_SPOOLBUF, "spoolbuffer"},
    {T_SPOOLBUF, "spool-buffer"},
#endif	/* PROTO_BGP */
#ifdef	PROTO_ASPATHS
    {T_ASPATH,	"aspath"},
    {T_ASPATH,	"as-path"},
    {T_ORIGIN,	"origin"},
    {T_INCOMPLETE, "incomplete"},
    {T_ANY,	"any"},
#endif	/* PROTO_ASPATHS */
#if	defined(PROTO_RIP) || defined(PROTO_HELLO)
    {T_TRUSTEDGATEWAYS, "trustedgateways"},
    {T_SOURCEGATEWAYS, "sourcegateways"},
#endif				/* defined(PROTO_RIP) || defined(PROTO_HELLO) */
#ifdef	PROTO_RIP
    {T_RIP, "rip"},
    {T_NORIPOUT, "noripout"},
    {T_NORIPIN, "noripin"},
    {T_NOCHECKZERO, "nocheckzero"},
    {T_NOCHECKZERO, "no-check-zero"},
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
    {T_HELLO, "hello"},
    {T_NOHELLOOUT, "nohelloout"},
    {T_NOHELLOIN, "nohelloin"},
#endif	/* PROTO_HELLO */
#ifdef	PROTO_ICMP
    {T_ICMP, "icmp"},
#endif	/* PROTO_ICMP */
#ifdef	PROTO_IGMP
    {T_IGMP, "igmp"},
#endif	/* PROTO_IGMP */
#if	defined(PROTO_ICMP) || defined(KRT_RT_SOCK)
    {T_NOREDIRECTS, "noredirects"},
    {T_REDIRECT, "redirect"},
    {T_REDIRECT, "redirects"},
#endif	/* defined(PROTO_ICMP) || defined(KRT_RT_SOCK) */
#ifdef	PROTO_SNMP
    {T_SNMP, "snmp"},
    {T_PORT, "port"},
#endif	/* PROTO_SNMP */
#ifdef	PROTO_OSPF
    {T_OSPF, "ospf"},
    {T_OSPF_ASE, "ospfase"},
    {T_OSPF_ASE, "ospf-ase"},
    {T_TAG, "tag"},
    {T_STUB, "stub"},
    {T_STUBHOSTS, "stubhosts"},
    {T_STUBHOSTS, "stub-hosts"},
    {T_MONITORAUTHKEY,	"monitorauthenticationkey"},
    {T_MONITORAUTHKEY,	"monitor-authentication-key"},
    {T_MONITORAUTHKEY,	"monauthkey"},
    {T_MONITORAUTHKEY,	"mon-auth-key"},
    {T_TRANSITAREA, "transitarea"},
    {T_TRANSITAREA, "transit-area"},
    {T_ROUTERS, "routers"},
    {T_NEIGHBORID, "neighborid"},
    {T_NEIGHBORID, "neighbor-id"},
    {T_NETWORKS, "networks"},
    {T_ELIGIBLE, "eligible"},
    {T_ENABLE, "enable"},
    {T_ENABLE, "enabled"},
    {T_DISABLE, "disable"},
    {T_DISABLE, "disabled"},
    {T_VIRTUALLINK, "virtuallink"},
    {T_VIRTUALLINK, "virtual-link"},
    {T_BACKBONE, "backbone"},
    {T_INFTRANSDELAY, "inftransdelay"},
    {T_INFTRANSDELAY, "transitdelay"},			/* Alias */
    {T_INFTRANSDELAY, "transit-delay"},			/* Alias */
    {T_POLLINTERVAL, "pollinterval"},
    {T_POLLINTERVAL, "poll-interval"},			/* Alias */
    {T_RXMITINTERVAL, "rxmtinterval"},
    {T_RXMITINTERVAL, "retransmitinterval"},
    {T_RXMITINTERVAL, "retransmit-interval"},
    {T_DD, "dd"},
    {T_REQUEST, "request"},
    {T_REQUEST, "req"},					/* Alias */
    {T_LSU, "lsu"},
    {T_LSU, "linkstateupdate"},				/* Alias */
    {T_LSU, "link-state-update"},			/* Alias */
    {T_ACK, "acknowledgement"},
    {T_ACK, "ack"},					/* Alias */
    {T_RECEIVE, "receive"},
    {T_RECEIVE, "rx"},					/* Alias */
    {T_LSA_BLD, "lsabuild"},
    {T_LSA_BLD, "lsa-build"},				/* Alias */
    {T_LSA_TX, "lsatransmit"},
    {T_LSA_TX, "lsa-transmit"},				/* Alias */
    {T_LSA_TX, "lsatx"},				/* Alias */
    {T_LSA_TX, "lsa-tx"},				/* Alias */
    {T_LSA_RX, "lsareceive"},
    {T_LSA_RX, "lsa-receive"},				/* Alias */
    {T_LSA_RX, "lsarx"},				/* Alias */
    {T_LSA_RX, "lsa-rx"},				/* Alias */
    {T_TRAP, "trap"},
    {T_EXPORTINTERVAL, "exportinterval"},
    {T_EXPORTINTERVAL, "export-interval"},
    {T_EXPORTLIMIT, "exportlimit"},
    {T_EXPORTLIMIT, "export-limit"},
#endif	/* PROTO_OSPF */
#if	defined(PROTO_OSPF) || defined(PROTO_BGP)
    {T_ROUTERID, "routerid"},
    {T_ROUTERID, "router-id"},
#endif	/* defined(PROTO_OSPF) || defined(PROTO_BGP) */
#if	defined(PROTO_OSPF) || defined(PROTO_ISIS)
    {T_AREA, "area"},
    {T_SPF, "spf"},
#endif	/* defined(PROTO_OSPF) || defined(PROTO_ISIS) */
#if	defined(PROTO_OSPF) || defined(PROTO_SLSP)
    {T_METRIC, "cost"},
    {T_HELLOINTERVAL, "hellointerval"},
    {T_HELLOINTERVAL, "hello-interval"},		/* Alias */
    {T_ROUTERDEADINTERVAL, "routerdeadinterval"},
    {T_ROUTERDEADINTERVAL, "router-dead-interval"},	/* Alias */
#endif	/* defined(PROTO_OSPF) || defined(PROTO_SLSP) */
#if	defined(PROTO_OSPF) || defined(PROTO_ISIS) || defined(PROTO_SLSP)
    {T_PRIORITY,	"priority"},
#endif	/* defined(PROTO_OSPF) || defined(PROTO_ISIS) || defined(PROTO_SLSP) */
#ifdef	PROTO_ISIS
    {T_ISIS,	"isis"},
    {T_ISIS,	"is-is"},
    {T_IP,	"ip"},
    {T_DUAL,	"dual"},
    {T_CIRCUIT,	"circuit"},
    {T_SYSTEMID,	"systemid"},
    {T_SYSTEMID,	"system-id"},
    {T_SNPA,	"snpa"},
    {T_LEVEL,	"level"},
    {T_IPREACH,	"ipreachability"},
    {T_IPREACH,	"ip-reachability"},
    {T_TROLL,	"troll"},
    {T_SET,	"set"},
    {T_PREFIX,	"prefix"},
    {T_MODE,	"mode"},
    {T_INTDOMINFO,	"internal-domain-info"},
    {T_INTDOMINFO,	"internaldomaininfo"},
#endif	/* PROTO_ISIS */
#ifdef	PROTO_IDPR
    {T_IDPR, "idpr"},
#endif	/* PROTO_IDPR */
#ifdef	PROTO_SLSP
    {T_SLSP, "slsp"},
#endif	/* PROTO_SLSP */
    {T_SIMPLEX, "simplex"},
    {T_PASSIVE, "passive"},
    {T_SCANINTERVAL, "scaninterval"},
    {T_SCANINTERVAL, "scan-interval"},
    {T_STRICTIFS, "strictintfs"},
    {T_STRICTIFS, "strict-intfs"},
    {T_STRICTIFS, "strictinterfaces"},
    {T_STRICTIFS, "strict-interfaces"},
    {T_STATIC, "static"},
#ifdef	notdef
    {T_ANNOUNCE, "announce"},
    {T_NOANNOUNCE, "noannounce"},
    {T_LISTEN, "listen"},
    {T_NOLISTEN, "nolisten"},
#endif	/* notdef */
    {T_MARTIANS, "martians"},
    {T_RETAIN, "retain"},
    {T_IGP,	"igp"},
    {T_AS, "as"},
    {T_AS, "autonomoussystem"},
    {T_AS, "autonomous-system"},
    {T_EXPORT, "export"},
    {T_IMPORT, "import"},
#ifdef	ROUTE_AGGREGATION
    {T_AGGREGATE, "aggregate"},
    {T_BRIEF, "brief"},
#endif	/* ROUTE_AGGREGATION */
    {T_MASK, "mask"},
    {T_MASKLEN, "masklength"},
    {T_MASKLEN, "mask-length"},
    {T_MASKLEN, "masklen"},
    {T_MASKLEN, "mask-len"},
    {T_HOST, "host"},
    {T_REJECT, "reject"},
    {T_BLACKHOLE, "blackhole"},
    {T_RESTRICT, "restrict"},
    {T_ALLOW, "allow" },
    {T_OPTIONS, "options"},
    {T_NOINSTALL, "noinstall"},
    {T_NOSEND, "nosend"},
    {T_NORESOLV, "noresolv"},
    {T_TRACEOPTIONS, "traceoptions"},
    {T_EXCEPT, "except"},
    {T_TRACEFILE, "tracefile"},
    {T_REPLACE, "replace"},
    {T_SIZE, "size"},
    {T_K, "k"},
    {T_M, "m"},
    {T_FILES, "files"},
    {T_ALL, "all"},
    {T_GENERAL, "general"},
    {T_INTERNAL, "internal"},
    {T_EXTERNAL, "external"},
    {T_ROUTE, "route"},
    {T_UPDATE, "update"},
    {T_KERNEL, "kernel"},
    {T_TASK, "task"},
    {T_TIMER, "timer"},
    {T_NOSTAMP, "nostamp"},
    {T_MARK, "mark"},
#if	defined(PROTO_ISIS)
    {TISIS_IIH, "iih"},
    {TISIS_DUMPLSP, "dumplsp"},
    {TISIS_EVENTS, "events"},
    {TISIS_LANADJ, "lanadj"},
    {TISIS_FLOODING, "flooding"},
    {TISIS_BUILDLSP, "buildlsp"},
    {TISIS_CSNP, "csnp"},
    {TISIS_PSNP, "psnp"},
    {TISIS_LSPINPUT, "lspinput"},
    {TISIS_P2PADJ, "p2padj"},
    {TISIS_LSPDB, "lspdb"},
    {TISIS_PATHS, "paths"},
    {TISIS_LSPCONTENT, "lspcontent"},
    {TISIS_SUMMARY, "summary"},
#endif	/* defined(PROTO_ISIS) */
#ifdef	PROTO_DVMRP
    {T_DVMRP, "dvmrp"},
    {T_IGNORE, "ignore"},
    {T_THRESHOLD, "threshold"},
    {T_TUNNEL, "tunnel"},
#endif	/* PROTO_DVMRP */
    {0, NULL}
};


/*
 *	A string of alpha characters which is either a keyword or a host/network name.
 *	First do a binary search (Knuth 6.2.1) to lookup up a keyword, if it is not found
 *	assume it is a host/network name.  The proper thing to do is a REJECT, but flex
 *	won't optimize if we use a REJECT.
 */
bits *
parse_keyword __PF1(yytext, char *)
{
    register int c;
    register bits *i, *l, *u;

    l = keywords;
    u = &keywords[n_keywords - 1];
    do {
	i = (u - l) / 2 + l;
	c = strcasecmp(yytext, i->t_name);
	if (!c) {
	    return i;
	} else if (c < 0) {
	    u = i - 1;
	} else {
	    l = i + 1;
	}
    } while (u >= l);

    return (bits *) 0;
}


/*
 *	Look up a token name given it's ID in the keyword table.  There is no way to select
 *	between multiple keywords mapping to the same token once the table is sorted.
 */
bits *
parse_keyword_lookup __PF1(token, int)
{
    bits *p;

    for (p = keywords; p->t_name; p++) {
	if (token == p->t_bits) {
	    return p;
	}
    }

    return (bits *) 0;
}


/*
 *	A comparison routine used by the sorting routine that follows
 */
static int
parse_keyword_sort_compare __PF2(p1, const void_t,
				 p2, const void_t)
{
    return strcasecmp(((bits *) p1)->t_name,
		      ((bits *) p2)->t_name);
}


/*
 *	Quicksort the table of keywords to insure that they are in order, easier
 *	than trying to keep the source sorted and less prone to mistakes.
 *
 *	The sort is only done the first time we are called, i.e. when n_keyword is
 *	zero.
 */
static void
parse_keyword_sort __PF0(void)
{
    bits *p;

    if (!n_keywords) {
	/* Calculate size of table */
	for (p = keywords; p->t_name; p++, n_keywords++) ;

	qsort((caddr_t) keywords, n_keywords, sizeof(bits), parse_keyword_sort_compare);
    }
}


/*
 *	Front end for yyparse().  Exit if any errors
 */
int
parse_parse __PF1(file, const char *)
{
    int errors;
    extern int yynerrs;
    static int first_parse = TRUE;

    if (first_parse) {
	parse_keyword_sort();		/* Sort the keyword table */

    }
#ifndef	vax11c
    parse_directory = task_mem_strdup((task *) 0, task_path_start);
#endif	/* vax11c */

    if (!BIT_TEST(task_state, TASKS_NORESOLV)) {
	sethostent(1);
	setnetent(1);
    }

    if (parse_open(task_mem_strdup((task *) 0, file))) {
	/* Indicate problem opening file */
	
	errors = -1;
    } else {
	protos_seen = (proto_t) 0;
	parse_state = PS_INITIAL;
	switch (errors = yyparse()) {
	case 0:
	    errors = yynerrs;
	    break;

	default:
	    errors = yynerrs ? yynerrs : 1;
	}
    }

    if (!BIT_TEST(task_state, TASKS_NORESOLV)) {
	endhostent();
	endnetent();
    }

    if (parse_directory) {
	task_mem_free((task *) 0, parse_directory);
	parse_directory = (char *) 0;
    }
	
    if (errors) {
	int errs = ABS(errors);
	
	trace(TR_ALL, 0, NULL);
	trace(TR_ALL, LOG_ERR, "parse_parse: %d parse error%s",
	      errs,
	      errs > 1 ? "s" : "");
	trace(TR_ALL, 0, NULL);
    }
    first_parse = FALSE;

    return errors;
}


/*
 *	Format a message indicating the current line number and return
 *	a pointer to it.
 */
char *
parse_where __PF0(void)
{
    static char where[LINE_MAX];

    if (parse_filename) {
	(void) sprintf(where, "%s:%d",
		       parse_filename,
		       yylineno);
    } else {
	(void) sprintf(where, "%d",
		       yylineno);
    }

    return where;
}


/*
 *	Limit check a number
 */
int
parse_limit_check __PF4(type, const char *,
			value, u_int,
			lower, u_int,
			upper, u_int)
{
    if ((value < lower) || ((upper != (u_int) -1) && (value > upper))) {
	(void) sprintf(parse_error, "invalid %s value at '%u' not in range %u to %u",
		       type,
		       value,
		       lower,
		       upper);
	return 1;
    }
    trace(TR_PARSE, 0, "parse: %s %s: %u",
	  parse_where(),
	  type,
	  value);
    return 0;
}


/*
 *	Look up a string as a host or network name, returning it's IP address
 *	in normalized network byte order.
 */
sockaddr_un *
parse_addr_hostname __PF1(hname, char *)
{
    struct hostent *hostent;
    const char *errmsg = 0;
#ifdef	HOST_NOT_FOUND
#ifndef h_errno
    extern int h_errno;
#endif /* !h_errno */
    extern int h_nerr;
    extern char *h_errlist[];

#endif	/* HOST_NOT_FOUND */

    trace(TR_PARSE, 0, "parse_addr_hostname: resolving %s",
	  hname);

    hostent = gethostbyname(hname);
    if (hostent) {
	switch (hostent->h_addrtype) {
#ifdef	PROTO_INET
	case AF_INET:
	{
	    struct in_addr addr;
#ifdef	h_addr
	    if (hostent->h_addr_list[1]) {
		/* XXX - For a gateway we could use just the address on our network if there was only one */
		(void) sprintf(parse_error, "host has multiple addresses at '%s'",
			       hname);
		return (sockaddr_un *) 0;
	    }
#endif	/* h_addr */

	    bcopy(hostent->h_addr, (caddr_t) &addr.s_addr, (size_t) hostent->h_length);
	    return sockbuild_in(0, addr.s_addr);
	}
#endif	/* PROTO_INET */

	default:
	    /* XXX - Should we pretend it does not exist if it is not an INET name? */
	    (void) sprintf(parse_error, "unsupported host family at '%s'",
			   hname);
	    return (sockaddr_un *) 0;
	}
    } else {
#ifdef	HOST_NOT_FOUND
	if (h_errno && (h_errno < h_nerr)) {
	    errmsg = h_errlist[h_errno];
	} else {
	    errmsg = "Unknown host";
	}
#else	/* HOST_NOT_FOUND */
	errmsg = "Unknown host";
#endif	/* HOST_NOT_FOUND */
    }
    (void) sprintf(parse_error, "error resolving '%s': %s",
		   hname,
		   errmsg);
    return (sockaddr_un *) 0;
}


/*
 *	Look up a string as a host or network name, returning it's IP address
 *	in normalized network byte order.
 */
sockaddr_un *
parse_addr_netname __PF1(hname, char *)
{
    struct netent *netent;

    trace(TR_PARSE, 0, "parse_addr_netname: resolving %s",
	  hname);

    netent = getnetbyname(hname);
    if (netent) {

	switch (netent->n_addrtype) {
#ifdef	PROTO_INET
	case AF_INET:
	{
	    struct in_addr network;

	    network.s_addr = netent->n_net;
	    if (network.s_addr) {
		while (!(network.s_addr & 0xff000000)) {
		    network.s_addr <<= 8;
		}
	    }
	    return sockbuild_in(0, network.s_addr);
	}
#endif	/* PROTO_INET */

	default:
	    (void) sprintf(parse_error, "unsupported network family at '%s'",
			   hname);
	    return (sockaddr_un *) 0;
	}
    } else {
	(void) sprintf(parse_error, "error resolving '%s': network unknown",
		       hname);
    }
    return (sockaddr_un *) 0;
}


/*
 *	Append an advlist to another advlist
 */
int
parse_adv_append __PF3(list, adv_entry **,
		       new, adv_entry *,
		       freeit, int)
{
    adv_entry *alo, *aln, *last = NULL;

    /* Add this network to the end of the list */
    if (*list) {
	ADV_LIST(*list, alo) {
	    if (!alo->adv_next) {
		last = alo;
	    }
	    /* Scan list for duplicates */
	    for (aln = new; aln; aln = aln->adv_next) {
		if (aln->adv_proto != alo->adv_proto
		    || (aln->adv_flag & ADVF_TYPE) != (alo->adv_flag & ADVF_TYPE)) {
		    continue;
		}
		switch (aln->adv_flag & ADVF_TYPE) {
		    case ADVFT_ANY:
			break;

		    case ADVFT_GW:
			if (aln->adv_gwp == alo->adv_gwp) {
			    (void) sprintf(parse_error, "duplicate gateway in list at '%A'",
					   aln->adv_gwp->gw_addr);
			    return TRUE;
			}
			break;

		    case ADVFT_IFN:
			if (aln->adv_ifn == alo->adv_ifae) {
			    (void) sprintf(parse_error, "duplicate interface in list at '%A'",
					   aln->adv_ifn->ifae_addr);
			    return TRUE;
			}
			break;

		    case ADVFT_IFAE:
			if (aln->adv_ifae == alo->adv_ifae) {
			    (void) sprintf(parse_error, "duplicate interface address in list at '%A'",
					   aln->adv_ifae->ifae_addr);
			    return TRUE;
			}
			break;

		    case ADVFT_AS:
			if (aln->adv_as == alo->adv_as) {
			    (void) sprintf(parse_error, "duplicate autonomous-system in list at '%u'",
					   aln->adv_as);
			    return TRUE;
			}
			break;

		    case ADVFT_DM:
			if (aln->adv_dm.dm_dest
			    && alo->adv_dm.dm_dest
			    && socktype(aln->adv_dm.dm_dest) != socktype(alo->adv_dm.dm_dest)) {
			    (void) sprintf(parse_error, "conflicting address family in dest/mask list at '%A mask %A'",
					   aln->adv_dm.dm_dest,
					   aln->adv_dm.dm_mask);
			    return TRUE;
			}
			if ((aln->adv_dm.dm_dest == alo->adv_dm.dm_dest
			     || sockaddrcmp(aln->adv_dm.dm_dest, alo->adv_dm.dm_dest))
			    && aln->adv_dm.dm_mask == alo->adv_dm.dm_mask) {
			    (void) sprintf(parse_error, "duplicate dest and mask in list at '%A mask %A'",
					   aln->adv_dm.dm_dest,
					   aln->adv_dm.dm_mask);
			    return TRUE;
			}
			break;

#ifdef	PROTO_ASPATHS
		    case ADVFT_TAG:
			if (aln->adv_tag == alo->adv_tag) {
			    (void) sprintf(parse_error, "duplicate tag in last at '%A'",
					   sockbuild_in(0, aln->adv_tag));
			    return TRUE;
			}
			break;
			
		    case ADVFT_ASPATH:
			if (aspath_adv_compare(alo->adv_aspath, aln->adv_aspath)) {
			    (void) sprintf(parse_error, "duplicate AS path pattern in list at '%s'",
					   aspath_adv_print(aln->adv_aspath));
			}
			break;
#endif	/* PROTO_ASPATHS */

		    case ADVFT_PS:
			if (PS_FUNC_VALID(aln, ps_compare) && PS_FUNC(aln, ps_compare)(alo->adv_ps, aln->adv_ps)) {
			    (void) sprintf(parse_error, "duplicate protocol specific data in list");
			    if (PS_FUNC_VALID(aln, ps_print)) {
				(void) sprintf(&parse_error[strlen(parse_error)],
					       "at '%s'",
					       PS_FUNC(aln, ps_print)(aln->adv_ps, TRUE));
			    }
			    return TRUE;
			}
			break;
		}
	    }
	} ADV_LIST_END(*list, alo) ;
	last->adv_next = new;
    } else {
	*list = new;
    }
    if (new) {
	/* XXX - bump the refcount, then free it? */
	ADV_LIST(new, aln) {
	    aln->adv_refcount++;
	} ADV_LIST_END(new, aln) ;
	if (freeit) {
	    adv_free_list(new);
	}
    }
    return FALSE;
}


/*
 *	Insert a dest/mask pair into list with longest mask first
 */
adv_entry *
parse_adv_dm_append __PF2(list, adv_entry *,
			  new, adv_entry *)
{
    adv_entry *adv = list;

    if (list) {
	register int new_bits = new->adv_dm.dm_mask ? mask_bits(new->adv_dm.dm_mask) : 0;
	register sockaddr_un *dest = new->adv_dm.dm_dest;
	register adv_entry *last = (adv_entry *) 0;

	/* Find where the mask gets inserted */
	do {
	    register int old_bits = list->adv_dm.dm_mask ? mask_bits(list->adv_dm.dm_mask) : 0;

	    if (new_bits == old_bits) {
		/* Same mask */

		if (!dest) {
		    /* Match all */
		    
		    if (!list->adv_dm.dm_dest) {
			/* Duplicate! */

		    Duplicate:
			(void) sprintf(parse_error, "duplicate entry in list at '%A mask %A'",
				       dest,
				       new->adv_dm.dm_mask);
			return (adv_entry *) 0;
		    }

		    /* Insert here */
		    break;
		}
		    
		switch (sockaddrcmp2(dest, list->adv_dm.dm_dest)) {
		case -1:
		    /* Insert here */
		    goto Insert;

		case 0:
		    /* Duplicate entry */
		    goto Duplicate;

		case 1:
		    /* Still looking */
		    continue;
		}
	    } else if (new_bits > old_bits) {
		/* Longer mask */
		break;
	    } else {
		/* Shorter mask */

		/* XXX - check for ambiguous entry */
	    }
	} while (last = list, list = list->adv_next);

    Insert:
	new->adv_next = list;
	if (last) {
	    last->adv_next = new;
	} else {
	    adv = new;
	}
    } else {
	adv = new;
    }

    return adv;
}


/*
 *	Set a flag in the gw structure for each element in a list
 */
int
parse_gw_flag __PF3(list, adv_entry *,
		    proto, proto_t,
		    flag, flag_t)
{
    int n = 0;
    adv_entry *adv;

    ADV_LIST(list, adv) {
	BIT_SET(adv->adv_gwp->gw_flags, flag);
	adv->adv_gwp->gw_proto = proto;
	n++;
    } ADV_LIST_END(list, adv) ;
    return n;
}


/*
 *	Switch to a new state if it is a valid progression from
 *	the current state
 */
int
parse_new_state __PF1(state, int)
{
    static const bits states[] = {
        { PS_INITIAL,	"initial" },
        { PS_OPTIONS,	"options" },
        { PS_INTERFACE,	"interface" },
	{ PS_DEFINE,	"define" },
	{ PS_PROTO,	"protocol" },
	{ PS_ROUTE,	"route" },
	{ PS_CONTROL,	"control" },
	{ 0, NULL }
    } ;

    assert(state >= PS_MIN && state <= PS_MAX);

    if (state < parse_state) {
	(void) sprintf(parse_error, "statement out of order");
	return TRUE;
    } else if (state > parse_state) {
	trace(TR_PARSE, 0, "parse_new_state: %s old %s new %s",
	      parse_where(),
	      trace_state(states, parse_state),
	      trace_state(states, state));
	parse_state = state;
    }

    return FALSE;
}


int
parse_metric_check __PF2(proto, proto_t,
			 metric, pmet_t *)
{
    int rc = 1;
    
    struct limit {
	proto_t proto;
	metric_t low;
	metric_t high;
	const char *desc;
    };
    static struct limit limits[] = {
#ifdef	PROTO_RIP
        { RTPROTO_RIP, RIP_LIMIT_METRIC, "RIP metric" },
#endif	/* PROTO_RIP */
#ifdef	PROTO_HELLO
        { RTPROTO_HELLO, HELLO_LIMIT_DELAY, "HELLO delay" },
#endif	/* PROTO_HELLO */
#ifdef	PROTO_OSPF
        { RTPROTO_OSPF, OSPF_LIMIT_COST, "OSPF cost" },
#endif	/* PROTO_OSPF */
#ifdef	PROTO_OSPF
        { RTPROTO_OSPF_ASE, OSPF_LIMIT_METRIC, "OSPF metric" },
#endif	/* PROTO_OSPF */
#ifdef	PROTO_EGP
        { RTPROTO_EGP, EGP_LIMIT_DISTANCE, "EGP distance" },
#endif	/* PROTO_EGP */
#ifdef	PROTO_BGP
        { RTPROTO_BGP, BGP_LIMIT_METRIC, "BGP metric" },
#endif	/* PROTO_BGP */
#ifdef	PROTO_SLSP
	{ RTPROTO_SLSP, SLSP_LIMIT_COST, "SLSP metric" },
#endif	/* PROTO_SLSP */
#ifdef  PROTO_ISIS
        { RTPROTO_ISIS, ISIS_LIMIT_METRIC, "IS-IS metric" },
#endif  /* PROTO_ISIS */
#ifdef	PROTO_DVMRP
        { RTPROTO_DVMRP, DVMRP_LIMIT_METRIC, "DVMRP metric" },
#endif  /* PROTO_DVMRP */
        { RTPROTO_DIRECT, IF_LIMIT_METRIC, "interface metric" },
        { 0, 0, 0, NULL }
    }, *limit = limits;

    for (limit = limits; limit->proto; limit++) {
	if (limit->proto == proto) {
	    break;
	}
    }

    if (limit->proto) {
	switch (metric->state) {
	    case PARSE_METRICS_UNSET:
		(void) sprintf(parse_error, "parse_metric_check: %s metric not set",
			       trace_state(rt_proto_bits, proto));
		break;

	    case PARSE_METRICS_INFINITY:
		PARSE_METRIC_SET(metric, limit->high);
		/* Fall Thru */

	    case PARSE_METRICS_SET:
		rc = parse_limit_check(limit->desc,
				       (u_int) metric->metric,
				       (u_int) limit->low,
				       (u_int) limit->high);
		break;

	    default:
		(void) sprintf(parse_error, "parse_metric_set: %s metric in unknown state: %d",
			       trace_state(rt_proto_bits, proto),
			       metric->state);
	}
    } else {
	(void) sprintf(parse_error, "parse_metric_check: invalid protocol %s (%x)",
		       trace_state(rt_proto_bits, proto),
		       proto);
    }

    return rc;
}


/*
 *	Set metric for each element in list that does not have one
 */
adv_entry *
parse_adv_propagate_metric __PF4(list, adv_entry *,
				 proto, proto_t,
				 metric, pmet_t *,
				 advlist, adv_entry *)
{
    adv_entry *adv;

    ADV_LIST(list, adv) {
	adv->adv_proto = proto;
	if (parse_adv_append(&adv->adv_list, advlist, FALSE)) {
	    return (adv_entry *) 0;
	}
	if (PARSE_METRIC_ISRESTRICT(metric)) {
	    BIT_SET(adv->adv_flag, ADVF_NO);
	} else if (PARSE_METRIC_ISSET(metric)) {
	    BIT_SET(adv->adv_flag, ADVFOT_METRIC);
	    adv->adv_result.res_metric = metric->metric;
	}
    } ADV_LIST_END(list, adv) ;

    return list;
}


/*
 *	Set metric for each element in list that does not have one
 */
adv_entry *
parse_adv_propagate_preference __PF4(list, adv_entry *,
				     proto, proto_t,
				     preference, pmet_t *,
				     advlist, adv_entry *)
{
    adv_entry *adv;

    ADV_LIST(list, adv) {
	adv->adv_proto = proto;
	if (parse_adv_append(&adv->adv_list, advlist, FALSE)) {
	    return (adv_entry *) 0;
	}
	if (PARSE_METRIC_ISRESTRICT(preference)) {
	    BIT_SET(adv->adv_flag, ADVF_NO);
	} else if (PARSE_METRIC_ISSET(preference)) {
	    BIT_SET(adv->adv_flag, ADVFOT_METRIC);
	    adv->adv_result.res_preference = preference->metric;
	}
    } ADV_LIST_END(list, adv) ;

    return list;
}


/*
 *	Set config list for each element
 */
adv_entry *
parse_adv_propagate_config __PF3(list, adv_entry *,
				 config, config_list *,
				 proto, proto_t)
{
    adv_entry *adv;

    if (config) {
	ADV_LIST(list, adv) {
	    adv->adv_config = config;
	    adv->adv_proto = proto;
	    config->conflist_refcount++;
	} ADV_LIST_END(list, adv) ;
    }

    return list;
}


/*
 *	Set preference for each elmit in list that does not have one
 */
void
parse_adv_preference __PF3(list, adv_entry *,
			   proto, proto_t,
			   preference, pref_t)
{
    ADV_LIST(list, list) {
	if ((list->adv_flag & ADVFO_TYPE) == ADVFOT_NONE) {
	    list->adv_proto = proto;
	    BIT_SET(list->adv_flag, ADVFOT_PREFERENCE);
	    list->adv_result.res_preference = preference;
	}
    } ADV_LIST_END(list, list) ;
}


/*
 *	Return a pointer to a duplicate of this adv_entry
 */
adv_entry *
parse_adv_dup __PF1(old, adv_entry *)
{
    adv_entry *new = NULL, *adv, *root;

    root = NULL;

    ADV_LIST(old, old) {
	adv = adv_alloc(old->adv_flag, old->adv_proto);
	bcopy((caddr_t) old, (caddr_t) adv, sizeof(*adv));
	if (!root) {
	    root = adv;
	} else {
	    new->adv_next = adv;
	}
	if (adv->adv_list) {
	    adv->adv_list->adv_refcount++;
	}
	new = adv;
    } ADV_LIST_END(old, old) ;
    return root;
}


/*
 *	Lookup the entry in the list for the exterior protocol and append this list to it.
 */
int
parse_adv_as __PF2(advlist, adv_entry **,
		   adv, adv_entry *)
{
    adv_entry *list;

    ADV_LIST(*advlist, list) {
	if (adv->adv_as == list->adv_as) {
	    break;
	}
    } ADV_LIST_END(*advlist, list) ;

    if (!list) {
	list = adv_alloc(ADVFT_AS, adv->adv_proto);
	list->adv_as = adv->adv_as;
	if (parse_adv_append(advlist, list, TRUE)) {
	    return 0;
	}
    }
    if (parse_adv_append(&list->adv_list, adv, TRUE)) {
	return 0;
    }
    return 1;
}


/*
 *	Parse the config file options
 */
int
parse_args __PF2(argc, int,
		 argv, char **)
{
    int arg_n, err_flag = 0;
    char *arg, *cp, *ap;
    char seen[MAXHOSTNAMELENGTH];

    trace_file = (char *) 0;
    bzero(seen, MAXHOSTNAMELENGTH);

    for (arg_n = 1; arg_n < argc; arg_n++) {
	arg = argv[arg_n];
	if (*arg == '-') {
	    cp = arg + 1;
	    if (index(seen, (int) *cp)) {
		trace(TR_ALL, LOG_ERR, MSGSTR(PARSE_1,
		      "%s: duplicate switch: %s\n"),
		      task_name,
		      arg);
		err_flag++;
		continue;
	    }
	    seen[strlen(seen)] = *cp;
	    switch (*cp++) {
	    case 'C':
		/* Test configuration */
		task_newstate(TASKS_TEST|TASKS_NOSEND|TASKS_NODUMP, 0);
		trace_flags_save = trace_flags = TR_NOSTAMP;
		break;

	    case 'c':
		/* Test configuration */
		task_newstate(TASKS_TEST|TASKS_NOSEND, 0);
		trace_flags_save = trace_flags = TR_GEN | TR_KRT | TR_NOSTAMP;
		break;

	    case 'n':
		/* Don't install in kernel */
		krt_install = krt_install_parse = FALSE;
		break;

	    case 'N':
		/* Don't daemonize */
		task_newstate(TASKS_NODAEMON, 0);
		break;
		
	    case 't':
		/* Set trace flags */
		if (*cp) {
		    if (!(trace_flags_save = trace_args(cp))) {
			err_flag++;
		    }
		} else {
		    trace_flags_save = TR_GEN;
		}
		break;

	    case 'f':
		/* Specify config file */
		ap = arg + 2;
		if (!*ap) {
		    if (((arg_n + 1) < argc) && (*argv[arg_n + 1] != '-')) {
			ap = argv[++arg_n];
		    }
		}
		if (!*ap) {
		    trace(TR_ALL, LOG_ERR, MSGSTR(PARSE_2,
			  "%s: missing argument for switch: %s\n"),
			  task_progname,
			  arg);
		    err_flag++;
		    break;
		}
		if (task_config_file) {
		    task_mem_free((task *) 0, task_config_file);
		}
		task_config_file = task_mem_strdup((task *) 0, ap);
		break;

	    default:
		trace(TR_ALL, LOG_ERR, MSGSTR(PARSE_3,
		"%s: invalid switch: %s\n"), task_progname, arg);
		err_flag++;
	    }
	} else if (!trace_file) {
	    trace_file = task_mem_strdup((task *) 0, arg);
	} else {
	    trace(TR_ALL, LOG_ERR, MSGSTR(PARSE_4,
	    "%s: extraneous information on command line: %s\n"),
	    task_progname, arg);
	    err_flag++;
	}
    }
    if (err_flag) {
	trace(TR_ALL, LOG_ERR, MSGSTR(PARSE_5,
   "Usage: %s [-c] [-C] [-n] [-N] [-t[flags]] [-f config-file] [trace-file]\n"),
	      task_progname);
    }
    return err_flag;
}


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
