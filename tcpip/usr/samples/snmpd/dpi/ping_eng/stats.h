/* @(#)39	1.1  src/tcpip/usr/samples/snmpd/dpi/ping_eng/stats.h, snmp, tcpip411, GOLD410 9/13/93 15:09:50 */
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:        /usr/samples/snmpd/dpi/ping_eng/stats.h
 */

/*
 * INTERNATIONAL BUSINESS MACHINES CORP. PROVIDES THIS SOURCE CODE
 * EXAMPLE, ASSOCIATED LIBRARIES AND FILES "AS IS," WITHOUT WARRANTY 
 * OF ANY KIND EITHER EXPRESSED OR IMPLIED INCLUDING BUT NOT LIMITED 
 * TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A 
 * PARTICULAR PURPOSE.  The entire risk as to the quality and performance 
 * of this example is with you.  Should any part of this source code 
 * example prove defective you (and not IBM or an IBM authorized 
 * dealer) assume the entire cost of all necessary servicing, repair, 
 * or correction.
 */

#ifndef _H_PING_ENG_STATS_
#define _H_PING_ENG_STATS_

/*
 * This file contains the include information for the stats table and entries.
 */

/* Round-trip-time statistics */

struct rtt_stats {
	unsigned long		address;
	long			minRTT;
	long			maxRTT;
	long			RTTsum;		/* sum of RTT's */
	long			RTTtries;	/* tries left */
	long			RTTresponses;	/* number of responses */
	unsigned long		timestamp;	/* last update to record */
	struct rtt_stats	*next;
};

extern struct rtt_stats *find_stats_record();
extern void retPINGdata();
extern void queue_PINGs();
extern int next_timeout();
extern unsigned long getValue();

#endif /* _H_PING_ENG_STATS_ */
