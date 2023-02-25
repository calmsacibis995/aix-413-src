/* @(#)08	1.1  src/bos/usr/sbin/XNSrouted/protocol.h, cmdnet, bos411, 9428A410j 3/1/91 12:39:21 */
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
 * Copyright (c) 1985 The Regents of the University of California.
 * All rights reserved.
 *
 * This file includes significant work done at Cornell University by
 * Bill Nesheim.  That work included by permission.
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
 *	@(#)protocol.h	5.6 (Berkeley) 6/1/90
 */

/*
 * Xerox Routing Information Protocol
 *
 */

struct netinfo {
	union ns_net	rip_dst;		/* destination net */
	u_short		rip_metric;		/* cost of route */
};

struct rip {
	u_short	rip_cmd;		/* request/response */
	struct netinfo rip_nets[1];	/* variable length */
};
 
/*
 * Packet types.
 */
#define	RIPCMD_REQUEST		1	/* want info */
#define	RIPCMD_RESPONSE		2	/* responding to request */

#define	RIPCMD_MAX		3
#ifdef RIPCMDS
char *ripcmds[RIPCMD_MAX] =
  { "#0", "REQUEST", "RESPONSE" };
#endif

#define	HOPCNT_INFINITY		16		/* per Xerox NS */
#define	DSTNETS_ALL		0xffffffff	/* per Xerox NS */
#define	MAXPACKETSIZE		512		/* max broadcast size */

extern union ns_net ns_anynet;
extern union ns_net ns_zeronet;

/*
 * Timer values used in managing the routing table.
 * Every update forces an entry's timer to be reset.  After
 * EXPIRE_TIME without updates, the entry is marked invalid,
 * but held onto until GARBAGE_TIME so that others may
 * see it "be deleted".
 */
#define	TIMER_RATE		30	/* alarm clocks every 30 seconds */

#define	SUPPLY_INTERVAL		30	/* time to supply tables */

#define	EXPIRE_TIME		180	/* time to mark entry invalid */
#define	GARBAGE_TIME		240	/* time to garbage collect */
