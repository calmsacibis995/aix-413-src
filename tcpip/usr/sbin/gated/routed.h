/* static char sccsid[] = "@(#)68	1.3  src/tcpip/usr/sbin/gated/routed.h, tcprouting, tcpip411, GOLD410 10/9/91 14:43:46"; */
/*
 * COMPONENT_NAME: TCPIP
 *
 * ORIGINS: 26  36  39
 *
 * FILE:	../com/sockcmd/gated2/routed.h
 */

/*
 * $Header: /users/jch/src/gated/src/RCS/routed.h,v 2.0 90/04/16 16:53:30 jch Exp $
 */

/********************************************************************************
*										*
*	GateD, Release 2							*
*										*
*	Copyright (c) 1990 by Cornell University				*
*	    All rights reserved.						*
*										*
*	    Royalty-free licenses to redistribute GateD Release 2 in		*
*	    whole or in part may be obtained by writing to:			*
*										*
*	    Center for Theory and Simulation in Science and Engineering		*
*	    Cornell University							*
*	    Ithaca, NY 14853-5201.						*
*										*
*	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR		*
*	IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED		*
*	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.	*
*										*
*	GateD is based on Kirton's EGP, UC Berkeley's routing daemon		*
*	(routed), and DCN's HELLO routing Protocol.  Development of Release	*
*	2 has been supported by the National Science Foundation.		*
*										*
*	The following acknowledgements and thanks apply:			*
*										*
*	    Mark Fedor (fedor@psi.com) for the development and maintenance	*
*	    up to release 1.3.1 and his continuing advice.			*
*										*
*********************************************************************************
*      Portions of this software may fall under the following			*
*      copyrights: 								*
*										*
*	Copyright (c) 1988 Regents of the University of California.		*
*	All rights reserved.							*
*										*
*	Redistribution and use in source and binary forms are permitted		*
*	provided that the above copyright notice and this paragraph are		*
*	duplicated in all such forms and that any documentation,		*
*	advertising materials, and other materials related to such		*
*	distribution and use acknowledge that the software was developed	*
*	by the University of California, Berkeley.  The name of the		*
*	University may not be used to endorse or promote products derived	*
*	from this software without specific prior written permission.		*
*	THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR		*
*	IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED		*
*	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.	*
********************************************************************************
*
*       The origin of this software will be explicitly stated as developed 
*       by Cornell University and its collaborators.
*
*       You agree to use reasonable efforts to provide Cornell University with 
*       all modifications, enhancements and documentation you make to GateD, 
*       including documentation related to your distribution of GateD.  Cornell 
*       University shall have the rights to redistribute the modifications,   
*       enhancements, and documentation without accounting to you.
*
*******************************************************************************/


/*
 * Routing Information Protocol
 *
 * Derived from Xerox NS Routing Information Protocol
 * by changing 32-bit net numbers to sockaddr's and
 * padding stuff to 32-bit boundaries.
 */
#define	RIPVERSION	1

struct netinfo {
    struct {
	u_short rip_family;
	u_char rip_zero1[2];
	struct in_addr rip_addr;
	u_char rip_zero2[8];
    } rip_dst;
    u_long rip_metric;			/* cost of route */
};

struct rip {
    u_char rip_cmd;			/* request/response */
    u_char rip_vers;			/* protocol version # */
    u_short rip_res;			/* pad to 32-bit boundary */
    union {
	struct netinfo ru_nets[1];	/* variable length... */
	char ru_tracefile[1];		/* ditto ... */
    } ripun;
#define	rip_nets	ripun.ru_nets
#define	rip_tracefile	ripun.ru_tracefile
};

struct entryinfo {
    struct sockaddr rtu_dst;
    struct sockaddr rtu_router;
    u_short rtu_flags;
    short rtu_state;
    int rtu_timer;
    int rtu_metric;
    u_int int_flags;
    char int_name[16];
};

/*
 * Packet types.
 */
#define	RIPCMD_REQUEST		1	/* want info */
#define	RIPCMD_RESPONSE		2	/* responding to request */
#define	RIPCMD_TRACEON		3	/* turn tracing on */
#define	RIPCMD_TRACEOFF		4	/* turn it off */
#define	RIPCMD_POLL		5	/* like request, but anyone answers */
#define	RIPCMD_POLLENTRY	6	/* like poll, but for entire entry */

#define	RIPCMD_MAX		7
#ifdef RIPCMDS
const char *ripcmds[RIPCMD_MAX] =
{"#0", "Request", "Response", "TraceOn", "TraceOff", "Poll", "PollEntry"};

#endif

#ifndef HOPCNT_INFINITY
#define	HOPCNT_INFINITY		16	/* per Xerox NS */
#endif				/* HOPCNT_INFINITY */
#ifndef MAXPACKETSIZE
#define	MAXPACKETSIZE		512	/* max broadcast size */
#endif				/* MAXPACKETSIZE */

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
