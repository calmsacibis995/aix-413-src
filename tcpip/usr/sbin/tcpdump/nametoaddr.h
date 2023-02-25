/* @(#)36	1.1  src/tcpip/usr/sbin/tcpdump/nametoaddr.h, tcpip, tcpip411, GOLD410 8/10/93 17:02:16 */
/*
 *   COMPONENT_NAME: TCPIP
 *
 *   FUNCTIONS: none
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
 *
 * Address to name translation routines.
 */

#ifndef _L_N_NAMETOADDR
#define _L_N_NAMETOADDR

extern u_long **s_nametoaddr();
extern u_long s_nametonetaddr();

extern int s_nametoport();
extern int s_nametoproto();
extern int s_nametoeproto();

extern u_char *ETHER_hostton();
extern u_char *ETHER_aton();

/*
 * If a protocol is unknown, PROTO_UNDEF is returned.
 * Also, s_nametoport() returns the protocol along with the port number.
 * If there are ambiguous entried in /etc/services (i.e. domain
 * can be either tcp or udp) PROTO_UNDEF is returned.
 */
#define PROTO_UNDEF		-1

#endif _L_N_NAMETOADDR
