/* @(#)05	1.1  src/bos/usr/sbin/XNSrouted/interface.h, cmdnet, bos411, 9428A410j 3/1/91 12:38:42 */
/*
 * COMPONENT_NAME: (CMDNET) Network commands. 
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)interface.h	5.5 (Berkeley) 6/1/90
 */

/*
 * Routing table management daemon.
 */

/*
 * An ``interface'' is similar to an ifnet structure,
 * except it doesn't contain q'ing info, and it also
 * handles ``logical'' interfaces (remote gateways
 * that we want to keep polling even if they go down).
 * The list of interfaces which we maintain is used
 * in supplying the gratuitous routing table updates.
 * We list only one address for  each interface, the AF_XNS one.
 */
#define NIFADDR 3
struct interface {
	struct	interface *int_next;
	struct	sockaddr int_addr;		/* address on this host */
	union {
		struct	sockaddr intu_broadaddr;
		struct	sockaddr intu_dstaddr;
	} int_intu;
#define	int_broadaddr	int_intu.intu_broadaddr	/* broadcast address */
#define	int_dstaddr	int_intu.intu_dstaddr	/* other end of p-to-p link */
	int	int_metric;			/* init's routing entry */
	int	int_flags;			/* see below */
	struct	ifdebug int_input, int_output;	/* packet tracing stuff */
	int	int_ipackets;			/* input packets received */
	int	int_opackets;			/* output packets sent */
	char	*int_name;			/* from kernel if structure */
	u_short	int_transitions;		/* times gone up-down */
/*XNS Specific entry */
	struct	sameq {
		struct sameq *n;		/* q of other pt-to-pt links */
		struct sameq *p;		/* with same net # */
	}	int_sq;
};

/*
 * 0x1 to 0x10 are reused from the kernel's ifnet definitions,
 * the others agree with the RTS_ flags defined elsewhere.
 */
#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_ROUTE	0x8		/* routing entry installed */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */

#define	IFF_PASSIVE	0x2000		/* can't tell if up/down */
#define	IFF_INTERFACE	0x4000		/* hardware interface */
#define	IFF_REMOTE	0x8000		/* interface isn't on this machine */

struct	interface *if_ifwithaddr();
struct	interface *if_ifwithdstaddr();
struct	interface *if_ifwithnet();
struct	interface *if_iflookup();
