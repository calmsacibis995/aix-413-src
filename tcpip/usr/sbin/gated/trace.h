/* @(#)80	1.4  src/tcpip/usr/sbin/gated/trace.h, tcprouting, tcpip411, GOLD410 12/6/93 18:05:03 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		PROTOTYPEV
 *		TRACE_ON
 *		TRACE_PROTO
 *		TRACE_UPDATE
 *		trace_state
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
 * trace.h,v 1.12.2.1 1993/08/27 22:28:57 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/*
 * trace.h
 */

#define	TIME_MARK	60*10		/* Duration between marks in seconds */

 /* tracing levels */
#define	TR_INT		BIT(0x01)	/* internal errors */
#define TR_EXT		BIT(0x02)	/* external changes resulting from egp */
#define TR_RT		BIT(0x04)	/* routing changes */
#define TR_EGP		BIT(0x08)	/* all egp packets sent and received */
#define TR_UPDATE	BIT(0x10)	/* trace update info sent */
#define TR_RIP		BIT(0x20)	/* trace update info sent */
#define TR_HELLO	BIT(0x40)	/* trace update info sent */
#define	TR_TASK		BIT(0x80)	/* trace task dispatching */
#define	TR_MARK		BIT(0x0100)	/* time mark every TIME_MARK seconds */
#define	TR_SNMP		BIT(0x0200)
#define	TR_ICMP		BIT(0x0400)	/* ICMP packet */
#define	TR_PROTOCOL	BIT(0x0800)	/* trace a protocol */
#define	TR_BGP		BIT(0x1000)	/* BGP protocol */
#define	TR_OSPF		BIT(0x2000)	/* OSPF protocol */
#define	TR_IDPR		BIT(0x4000)	/* IDPR protocol */
#define	TR_KRT		BIT(0x8000)	/* Kernel routes */
#define	TR_SLSP		BIT(0x010000)	/* SLSP protocol */
#define	TR_PARSE	BIT(0x020000)	/* Trace Parser */
#define	TR_TIMER	BIT(0x040000)	/* Trace timers */
#define	TR_ISIS		BIT(0x080000)	/* IS-IS protocol */
#define	TR_IGMP		BIT(0x100000)	/* IGMP protocol */

#define	TR_USR1		BIT(0x400000)
#define	TR_USR2		BIT(0x800000)
#define	TR_USR3		BIT(0x01000000)
#define	TR_USR4		BIT(0x02000000)
#define	TR_USR5		BIT(0x04000000)
#define	TR_USR6		BIT(0x08000000)
#define	TR_USR7		BIT(0x10000000)
#define	TR_USR8		BIT(0x20000000)
#define	TR_USR9		BIT(0x40000000)

#define TR_NOSTAMP	BIT(0x80000000)	/* no timestamp requested */

#define	TR_ALL		~TR_NOSTAMP	/* trace everything but the time stamp */
#define	TR_GEN		TR_INT | TR_EXT | TR_RT	/* General flags */

#define	TRACE_ON(flag)		if (BIT_TEST(trace_flags, (flag)))
#define	TRACE_PROTO(flag)	if (BIT_MATCH(trace_flags, (flag)|TR_PROTOCOL))
#define	TRACE_UPDATE(flag)	if (BIT_MATCH(trace_flags, (flag)|TR_UPDATE))

#define	TRACE_LIMIT_FILE_SIZE	(u_int) 10*1024, (u_int) -1
#define	TRACE_LIMIT_FILE_COUNT	2, (u_int) -1

/*
 *	Trace routines
 */

PROTOTYPE(trace_init,
	  extern void,
	  (void));
PROTOTYPE(trace_string,
	  extern char *,
	  (flag_t));
PROTOTYPE(trace_display,
	  extern void,
	  (flag_t,
	   flag_t));
PROTOTYPE(trace_off,
	  extern void,
	  (void));
PROTOTYPE(trace_close,
	  extern void,
	  (void));
PROTOTYPE(trace_on,
	  extern void,
	  (char *,
	   int));
PROTOTYPE(trace_args,
	  extern flag_t,
	  (char *));
PROTOTYPE(trace_value,
	  extern const char *,
	  (const bits *,
	   int));
PROTOTYPE(trace_bits,
	  extern char *,
	  (const bits *,
	   flag_t));
PROTOTYPE(trace_bits2,
	  extern char *,
	  (const bits *,
	   const bits *,
	   flag_t));
PROTOTYPE(trace_dump,
	  extern void,
	  (int));
PROTOTYPE(trace_mark,
	  extern void,
	  (timer *,
	   time_t));
PROTOTYPEV(trace,
	   extern void,
	   (flag_t,
	    int,
	    const char *,
	    ...));
PROTOTYPEV(tracef,
	   extern void,
	   (const char *,
	    ...));

#define trace_state(bits, mask) bits[mask].t_name

extern flag_t trace_flags;		/* trace packets and route changes */
extern flag_t trace_flags_save;		/* save tracing flags */
extern const bits trace_types[];	/* Names for the tracing flags */
extern char *trace_file;		/* File we are tracing to */
extern int trace_opened;		/* True of trace_on() has been called */
extern off_t trace_limit_size;		/* Maximum file size */
extern int trace_limit_files;		/* Maximum number of files */


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
