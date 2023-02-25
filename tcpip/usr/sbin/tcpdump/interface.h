/* @(#)31	1.3  src/tcpip/usr/sbin/tcpdump/interface.h, tcpip, tcpip411, GOLD410 4/9/94 15:03:23 */
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: max
 *		min
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1988-1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef _L_H_INTERFACE
#define _L_H_INTERFACE

#ifdef __GNUC__
#define inline __inline
#else
#define inline
#endif

#include "os.h"			/* operating system stuff */
#include "md.h"			/* machine dependent stuff */

#ifndef	_AIX
#ifndef __STDC__
extern char *malloc();
extern char *calloc();
#endif
#endif

extern int dflag;		/* print filter code */
extern int eflag;		/* print ethernet header */
extern int nflag;		/* leave addresses as numbers */
extern int Nflag;		/* remove domains from printed host names */
extern int qflag;		/* quick (shorter) output */
extern int Sflag;		/* print raw TCP sequence numbers */
extern int tflag;		/* print packet arrival time */
extern int vflag;		/* verbose */
extern int xflag;		/* print packet in hex */

extern char *program_name;	/* used to generate self-identifying messages */

extern int snaplen;
/* global pointers to beginning and end of current packet (during printing) */
extern unsigned char *packetp;
extern unsigned char *snapend;

extern long thiszone;			/* gmt to local correction */

extern void ts_print();
extern int clock_sigfigs();

extern char *lookup_device();

extern void error();
extern void warning();

extern char *read_infile();
extern char *copy_argv();

extern void usage();
extern void show_code();
extern void init_addrtoname();

/* The printer routines. */

extern void ether_if_print();
extern void iso88023_if_print();
extern void tr_if_print();
extern void arp_print();
extern void ip_print();
extern void tcp_print();
extern void udp_print();
extern void icmp_print();
extern void default_print();

extern void ntp_print();
extern void nfsreq_print();
extern void nfsreply_print();
extern void ns_print();
extern void ddp_print();
extern void rip_print();
extern void tftp_print();
extern void bootp_print();
extern void snmp_print();
extern void sl_if_print();
extern void ppp_if_print();
extern void fddi_if_print();
extern void null_if_print();
extern void egp_print();

#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((b)>(a)?(b):(a))

/* 
 * The default snapshot length.  This value allows most printers to print
 * useful information while keeping the amount of unwanted data down.
 * In particular, it allows for an ethernet header, tcp/ip header, and
 * 14 bytes of data (assuming no ip options).
 */
#define DEFAULT_SNAPLEN 80

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#define LITTLE_ENDIAN 1234
#endif

struct printer {
	void (*f)();
	int type;
	char *ifname;
};

#endif _L_H_INTERFACE
