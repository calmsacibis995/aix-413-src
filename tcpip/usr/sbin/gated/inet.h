/* @(#)55	1.6  src/tcpip/usr/sbin/gated/inet.h, tcprouting, tcpip411, GOLD410 12/6/93 17:50:17 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: AS_LIST
 *		AS_LIST_END
 *		PROTOTYPE
 *		inet_class_flags
 *		inet_class_flags_byte
 *		inet_class_of
 *		inet_class_of_byte
 *		inet_class_valid
 *		inet_class_valid_byte
 *		inet_mask_natural
 *		inet_mask_natural_byte
 *		inet_mask_prefix
 *		inet_net_natural
 *		sock2host
 *		sock2in
 *		sock2ip
 *		sock2port
 *		sockaddrcmp_in
 *		sockaddrmask_in
 *		sockclear_in
 *		sockcmp_in
 *		sockmask_in
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
 * inet.h,v 1.19.2.1 1993/05/06 01:06:59 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* macros to select internet address given pointer to a struct sockaddr */

/* clear and init a sockaddr_un for Inet use */
#define	sock2in(x)	((x)->in.sin_addr)
#define	sock2ip(x)	(sock2in(x).s_addr)
#define	sock2port(x)	((x)->in.sin_port)
#define	sock2host(addr, mask) (sock2ip(addr) & (~sock2ip(mask)))
#define	sockclear_in(x)	socktype(x) = AF_INET; \
    			socksize(x) = sizeof ((x)->in); \
    			sock2port(x) = 0; \
    			sock2ip(x) = 0;
#define	sockaddrcmp_in(a, b)	(sock2ip(a) == sock2ip(b))
#define	sockcmp_in(a, b)	(sock2port(a) == sock2port(b) && sockaddrcmp_in(a, b))
#define	sockaddrmask_in(addr, mask)	sock2ip(addr) &= sock2ip(mask)
#define	sockmask_in(addr, mask)	sock2port(addr) &= sock2port(mask); sockaddrmask_in(addr, mask)

/* additional definitions to netinet/in.h */

#ifndef	INADDR_LOOPBACK
#define	INADDR_LOOPBACK	0x7f000001
#endif	/* INADDR_LOOPBACK */

#ifndef	INADDR_HOSTMASK
#define	INADDR_HOSTMASK	0xffffffff
#endif	/* INADDR_HOSTMASK */

#ifndef	INADDR_DEFAULT
#define	INADDR_DEFAULT	0x00000000
#endif	/* INADDR_DEFAULT */


#define	IP_MAXHDRLEN	(sizeof (struct ip) + MAX_IPOPTLEN + IPOPT_MINOFF)
#ifndef	MAX_IPOPTLEN
#define	MAX_IPOPTLEN	40
#endif
#ifndef	IPOPT_MINOFF
#define	IPOPT_MINOFF	4
#endif

/* IP maximum packet size */
#ifndef	IP_MAXPACKET
#define	IP_MAXPACKET	65535
#endif

#ifdef	ROUTER_ID
/* Router ID */
extern if_addr_entry *inet_routerid_entry;
#define	inet_routerid	inet_routerid_entry->ifae_addr
#endif	/* ROUTER_ID */

#ifdef	AUTONOMOUS_SYSTEM
extern as_t inet_autonomous_system;	/* My autonomous system */

/*
 *	Autonomous systems
 */

typedef struct _as_entry {
    struct _as_entry *as_forw;
    struct _as_entry *as_back;
    as_t	as_as;
    u_int	as_refcount;
} as_entry;

#define	AS_LIST(asp)	for ((asp) = inet_as_root.as_forw; (asp) != &inet_as_root; (asp) = (asp)->as_forw)
#define	AS_LIST_END(asp)	if ((asp) == &inet_as_root) (asp) = (as_entry *) 0;

extern as_entry inet_as_root;

PROTOTYPE(inet_as_link,
	  extern as_entry *,
	  (as_t));
PROTOTYPE(inet_as_unlink,
	  extern void,
	  (as_t));
#endif	/* AUTONOMOUS_SYSTEM */


extern int inet_ipforwarding;		/* IP forwarding engine enabled */
extern int inet_udpcksum;		/* UDP checksums enabled */


/* Common addresses */
extern sockaddr_un *inet_addr_default;
extern sockaddr_un *inet_addr_loopback;
extern sockaddr_un *inet_addr_any;
extern sockaddr_un *inet_addr_reject;
extern sockaddr_un *inet_addr_blackhole;

#define	inet_mask_default	inet_masks[INET_MASK_DEFAULT]
#define	inet_mask_classa	inet_masks[INET_MASK_CLASSA]
#define	inet_mask_classb	inet_masks[INET_MASK_CLASSB]
#define	inet_mask_classc	inet_masks[INET_MASK_CLASSC]
#ifdef	INET_CLASS_E
#define	inet_mask_classe	inet_masks[INET_MASK_CLASSE]
#endif	/* INET_CLASS_E */
#ifdef	INET_CLASS_C_SHARP
#define	inet_mask_classc_sharp	inet_masks[INET_MASK_CLASSC_SHARP]    
#endif	/* INET_CLASS_C_SHARP */
#define	inet_mask_host		inet_masks[INET_MASK_HOST]

extern	sockaddr_un *inet_masks[];		/* Table of all possible *contiguous* masks */

#define	INET_LIMIT_MASKLEN	0, 32

/* Classes */
struct inet_class {
    byte	inetc_class;
    byte	inetc_flags;
    byte	inetc_mask;
};

/* Network classes */
#define	INET_CLASSC_A			1
#define	INET_CLASSC_B			2
#define	INET_CLASSC_C			3
#define	INET_CLASSC_D			4
#define	INET_CLASSC_E			5
#define	INET_CLASSC_A_SHARP		INET_CLASSC_RESERVED
#define	INET_CLASSC_C_SHARP		7
#define	INET_CLASSC_RESERVED		0
#define	INET_CLASSC_MULTICAST		INET_CLASSC_D

#define	INET_CLASSF_NETWORK		0x01
#define	INET_CLASSF_LOOPBACK		0x02
#define	INET_CLASSF_DEFAULT		0x04
#define	INET_CLASSF_RESERVED		0x08
#define	INET_CLASSF_MULTICAST		0x10

/* Masks */
#define	INET_MASK_DEFAULT	0		/* 0.0.0.0 */
#define	INET_MASK_CLASSA	8		/* 255.0.0.0 */
#define	INET_MASK_CLASSB	16		/* 255.255.0.0 */
#define	INET_MASK_CLASSC	24		/* 255.255.255.0 */
#define	INET_MASK_CLASSD	32		/* 255.255.255.255 */
#define	INET_MASK_CLASSE	20		/* 255.255.240.0 */
#define	INET_MASK_CLASSC_SHARP	20		/* 255.255.240.0 */
#define	INET_MASK_MULTICAST	INET_MASK_CLASSD
#define	INET_MASK_HOST		32		/* 255.255.255.255 */
#define	INET_MASK_LOOPBACK	INET_MASK_HOST
#define	INET_MASK_INVALID	33		/* Oops! */

extern struct inet_class inet_classes[];	/* Class and mask info for the various networks */

#define	inet_class_of_byte(x)		inet_classes[*(x)].inetc_class
#define	inet_class_valid_byte(x)	BIT_MATCH(inet_classes[*(x)].inetc_flags, INET_CLASSF_NETWORK)
#define	inet_class_flags_byte(x)	inet_classes[*(x)].inetc_flags
#define	inet_mask_natural_byte(x)	(inet_masks[inet_classes[*(x)].inetc_mask])

/* Returns class number */
#define	inet_class_of(s)	inet_class_of_byte((byte *) &sock2ip(s))

/* Returns true if a valid class */
#define	inet_class_valid(s)	inet_class_valid_byte((byte *) &sock2ip(s))

/* Returns flags */
#define	inet_class_flags(s)	inet_class_flags_byte((byte *) &sock2ip(s))

/* Returns the natural mask, zero for experimental */
#define	inet_mask_natural(s)	inet_mask_natural_byte((byte *) &sock2ip(s))

/* Returns the natural network (as a u_int32), dumps core for invalid nets */
#define	inet_net_natural(s)	(sock2ip(s) & sock2ip(inet_masks[inet_classes[*((byte *) &sock2ip(s))].inetc_mask]))

/* Locate an inet mask given it's prefix length */
#define	inet_mask_prefix(pfx)	((pfx < 0 || pfx > 32) ? (sockaddr_un *) 0 : inet_masks[pfx])

#define INET_CLASSE_NET           htonl(0xfffff000)


PROTOTYPE(inet_mask_locate,
	  extern sockaddr_un *,
	  (u_int32));
PROTOTYPE(inet_mask_withif,
	  extern sockaddr_un *,
	  (sockaddr_un *,
	   if_addr *,
	   flag_t *));
PROTOTYPE(inet_ifsubnetmask,
	  extern sockaddr_un *,
	  (struct in_addr));	/* Subnet mask of an address if known */
#ifdef	ROUTER_ID
PROTOTYPE(inet_parse_routerid,
	  extern int,
	  (sockaddr_un *,
	   char *));
#endif	/* ROUTER_ID */
PROTOTYPE(inet_family_init,
	  extern void,
	  (void));
PROTOTYPE(inet_init,
	  extern void,
	  (void));


#ifdef	IP_MULTICAST
PROTOTYPE(inet_allrouters_join,
	  extern void,
	  (if_addr *));
PROTOTYPE(inet_allrouters_drop,
	  extern void,
	  (if_addr *));
#endif	/* IP_MULTICAST */

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
