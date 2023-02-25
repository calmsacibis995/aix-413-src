/* @(#)80       1.3  src/bos/usr/sbin/route/keywords.h, cmdnet, bos411, 9428A410j 5/26/93 14:45:53 */

/*
 * COMPONENT_NAME: (CMDNET) 
 *
 * FUNCTIONS:   keywords.h
 *
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1989 Regents of the University of California.
 * All rights reserved.
 *
 */
/*
 * keywords.h
 *
 * This file used to be made dynamically from "./keywords" with
 * the following script addition to the BSD Makefile:
 *
 * keywords.h: keywords
 * 	sed -e '/^#/d' -e '/^$$/d' keywords > _keywords.tmp
 * 	/usr/ucb/tr a-z A-Z < _keywords.tmp | paste _keywords.tmp - | \
 * 	    awk '{ \
 * 		if (NF > 1) \
 * 			printf "#define\tK_%s\t%d\n\t{\"%s\", K_%s},\n", \
 * 			    $$2, NR, $$1, $$2 }' \
 * 	    > keywords.h
 * 	rm -f _keywords.tmp
 *
 * Making a change to keywords.h is like making a change to keywords,
 * save for a few keystrokes saved.  Big deal.
 * Not doing the above script simplifies the libMakefile.
 *
 * Note further that keywords.h is #included into route.c inside
 * a table.  When changing the code below, be wary of that.
 */

#define	K_ADD	1
	{"add", K_ADD},
#define	K_CHANGE	2
	{"change", K_CHANGE},
#define	K_CLONING	3
	{"cloning", K_CLONING},
#define	K_DELETE	4
	{"delete", K_DELETE},
#define	K_DESTINATION	5
	{"destination", K_DESTINATION},
#define	K_EXPIRE	6
	{"expire", K_EXPIRE},
#define	K_FLUSH	7
	{"flush", K_FLUSH},
#define	K_GATEWAY	8
	{"gateway", K_GATEWAY},
#define	K_GENMASK	9
	{"genmask", K_GENMASK},
#define	K_GET	10
	{"get", K_GET},
#define	K_HOST	11
	{"host", K_HOST},
#define	K_HOPCOUNT	12
	{"hopcount", K_HOPCOUNT},
#define	K_IFACE	13
	{"iface", K_IFACE},
#define	K_INTERFACE	14
	{"interface", K_INTERFACE},
#define	K_IFADDR	15
	{"ifaddr", K_IFADDR},
#define	K_INET	16
	{"inet", K_INET},
#define	K_ISO	17
	{"iso", K_ISO},
#define	K_LINK	18
	{"link", K_LINK},
#define	K_LOCK	19
	{"lock", K_LOCK},
#define	K_LOCKREST	20
	{"lockrest", K_LOCKREST},
#define	K_MASK	21
	{"mask", K_MASK},
#define	K_MONITOR	22
	{"monitor", K_MONITOR},
#define	K_MTU	23
	{"mtu", K_MTU},
#define	K_NET	24
	{"net", K_NET},
#define	K_NETMASK	25
	{"netmask", K_NETMASK},
#define	K_OSI	26
	{"osi", K_OSI},
#define	K_RECVPIPE	27
	{"recvpipe", K_RECVPIPE},
#define	K_RTT	28
	{"rtt", K_RTT},
#define	K_RTTVAR	29
	{"rttvar", K_RTTVAR},
#define	K_SENDPIPE	30
	{"sendpipe", K_SENDPIPE},
#define	K_SSTHRESH	31
	{"ssthresh", K_SSTHRESH},
#define	K_XNS	32
	{"xns", K_XNS},
#define	K_XRESOLVE	33
	{"xresolve", K_XRESOLVE},
#define	K_REJECT	34
	{"reject", K_REJECT},
#define	K_BLACKHOLE	35
	{"blackhole", K_BLACKHOLE},
