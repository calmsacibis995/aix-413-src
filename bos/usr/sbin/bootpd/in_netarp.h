/*
 * @(#)07        1.5  src/bos/usr/sbin/bootpd/in_netarp.h, cmdnet, bos411, 9428A410j 10/17/91 16:08:08
 *
 * COMPONENT_NAME: SOCKET Socket Services
 * 
 * FUNCTIONS: ARPTAB_HASH, ARPTAB_LOOK 
 *
 * ORIGINS: 26,27
 *
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright,
 * permission and disclaimer notice appear on all copies and supporting
 * documentation; the name of IBM not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission; and notice be given in supporting documentation that copying
 * and distribution is by permission of IBM.  IBM makes no representations
 * about the suitability of this software for any purpose.  This program
 * is provided "as is" without any express or implied warranties, including,
 * without limitation, the implied warranties of merchantability and fitness
 * for a particular purpose.
 */

/**
 ** #define ATF_INUSE 1
 ** #define ATF_COM   2
 ** #define ATF_PERM  4
 **/



union if_dependent {
	struct   token  {
		u_short	u_rcf;		/* route control field */
		u_short	u_seg[8];	/* routing info */
	} token;
	struct  x25 {
		u_short	u_session_id;	/* x.25 session id */
		u_short	u_unit;		/* unit number     */
	} x25;
};

#define  MAX_HWADDR		32

/*
 * Internet to ethernet address resolution table.
 */

struct  arptab {
	struct  in_addr	at_iaddr;	/* internet address */
	u_char 		hwaddr[MAX_HWADDR];	/* hardware address */
	u_short		at_length;	/* length of hardware address */
	u_char		at_timer;	/* minutes since last reference */
	u_char		at_flags;	/* flags */
	struct  mbuf	*at_hold;	/* last pkt until resolved/timeout */
	union if_dependent if_dependent;/* hdwr dependent info */
	u_long		ifType;		/* interface type */
};



#define	at_enaddr	hwaddr
#define	at_traddr	hwaddr
#define	at_ie3addr	hwaddr
#define	at_x25addr	hwaddr

#define	at_rcf		if_dependent.token.u_rcf
#define	at_seg		if_dependent.token.u_seg
#define	at_session_id	if_dependent.x25.u_session_id
#define	at_x25unit	if_dependent.x25.u_unit

/*
 * ARP ioctl request
 */
struct	arpreq {
	struct	sockaddr	arp_pa;		/* protocol address */
	struct  sockaddr	arp_ha;		/* hardware address */
	int			arp_flags;	/* flags */
	u_short		at_length;		/* length of hardware address */
	union if_dependent ifd;			/* hdwr dependent info */
	u_long	ifType;				/* interface type */
};

#define	arp_rcf		ifd.token.u_rcf
#define	arp_seg		ifd.token.u_seg
#define	arp_session_id	ifd.x25.u_session_id
#define	arp_x25unit	ifd.x25.u_unit


#define IN_USE  0x0001          /* interface entry in use when bit = = 1 */

/*
 * Structure shared between the ethernet driver modules and
 * the address resolution code.  For example, each ec_softc or il_softc
 * begins with this structure.
 */
struct  arpcom {
	struct  ifnet	ac_if;			/* network visible interface */
	u_char		ac_hwaddr[MAX_HWADDR];	/* hardware address */
	u_short		ac_hwlen;		/* length of hw address	*/
	struct  in_addr	ac_ipaddr;		/* copy of IP address */
	u_long		ifType;			/* interface type */
};

struct snmpindex_table {
	int     *softc;         /* pointer to softc structures */
	u_long  ifType;         /* type of interface, per RFC 1066 */
	u_short flags;          /* flags */
};

#define ac_enaddr	ac_hwaddr
#define ac_traddr	ac_hwaddr

#ifdef		GATEWAY
#define	ARPTAB_BSIZ	25		/* bucket size */
#define	ARPTAB_NB	37		/* number of buckets */
#else
#define	ARPTAB_BSIZ	18		/* bucket size */
#define	ARPTAB_NB	27		/* number of buckets */
#endif

#define	ARPTAB_SIZE	(ARPTAB_BSIZ * ARPTAB_NB)


#define	ARPTAB_HASH(a)		\
		((u_long)(a) % ARPTAB_NB)

#define	ARPTAB_LOOK(at, addr) {	\
		register n;	\
		at = &arptab[ARPTAB_HASH(addr) * ARPTAB_BSIZ];	\
		for (n = 0 ; n < ARPTAB_BSIZ ; n++, at++)	\
			if (at->at_iaddr.s_addr == addr)	\
				break;		\
		if  (n >= ARPTAB_BSIZ)		\
			at = 0; \
}

/* timer values */
#define	ARPT_AGE	(60 * 1)	/* aging timer, 1 min. */
#define	ARPT_KILLC	20		/* kill completed entry in 20 minutes */
#define	ARPT_KILLI	3		/* kill incomplete entry in 3 minutes */


