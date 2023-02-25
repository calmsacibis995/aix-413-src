static char sccsid[] = "@(#)93	1.1  src/tcpip/usr/sbin/gated/ospf_log.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:07";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF1
 *		__PF3
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
 * ospf_log.c,v 1.12 1993/04/10 12:25:19 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"

const char *ospf_logtype[] =
{
    "Monitor request",
    "Hello",
    "DB Description",
    "Link-State Req",
    "Link-State Update",
    "Link-State Ack",
    "Monitor response",
    "Hello",
    "DB Description",
    "Link-State Req",
    "Link-State Update",
    "Link-State Ack",
    "IP: Bad OSPF pkt type",
    "IP: Bad IP Dest",
    "IP: Bad IP proto id",
    "IP: Pkt src = my IP addr",
    "OSPF: Bad OSPF version",
    "OSPF: Bad OSPF checksum",
    "OSPF: Bad intf area id",
    "OSPF: Area mismatch",
    "OSPF: Bad virt link info",
    "OSPF: Auth type != area type",
    "OSPF: Auth key != area key",
    "OSPF: Packet is too small",
    "OSPF: Packet size > IP length",
    "OSPF: Transmit bad",
    "OSPF: Received on down IF",
    "Hello: IF mask mismatch",
    "Hello: IF hello timer mismatch",
    "Hello: IF dead timer mismatch",
    "Hello: Extern option mismatch",
    "Hello: Nbr Id/IP addr confusion",
    "Hello: Unknown Virt nbr",
    "Hello: Unknown NBMA nbr",
    "DD: Unknown nbr",
    "DD: Nbr state low",
    "DD: Nbr's rtr = my rtrid",
    "DD: Extern option mismatch",
    "Ack: Unknown nbr",
    "Ack: Nbr state low",
    "Ls Req: Nbr state low",
    "Ls Req: Unknown nbr",
    "Ls Req: Empty request",
    "LS Req: Bad pkt",
    "LS Update: Nbr state low",
    "Ls Update: Unknown nbr",
    "Ls Update: Newer self-gen LSA",
    "Ls Update: Bad LS chksum",
    "Ls Update: less recent rx",
    "Ls Update: Unknown type"};

long ospf_cumlog[LASTLOG + 1] = { 0 };	/* should be even */

void
ospf_log_rx __PF3(type, int,
		  src, sockaddr_un *,
		  dst, sockaddr_un *)
{
    ospf_cumlog[type]++;

    if (type > GOOD_LSA
	&& BIT_TEST(ospf.trace_flags, TR_OSPF_TRAP)) {
	trace(TR_OSPF, 0, "OSPF RECV %-15A -> %-15A: %s",
	      src,
	      dst,
	      ospf_logtype[type]);
    }
}


/*
 * Change dest addr to string
 */
sockaddr_un *
ospf_addr2str __PF1(to, sockaddr_un *)
{
    switch (sock2ip(to)) {
    case ALL_UP_NBRS:
	return sockbuild_str("All_up_nbrs");

    case ALL_ELIG_NBRS:
	return sockbuild_str("All_elig_nbrs");

    case ALL_EXCH_NBRS:
	return sockbuild_str("All_exch_nbrs");

    case DR_and_BDR:
	return sockbuild_str("Dr_and_Bdr");
    }

    return to;
}


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
 * ------------------------------------------------------------------------
 * 
 *                 U   U M   M DDDD     OOOOO SSSSS PPPPP FFFFF
 *                 U   U MM MM D   D    O   O S     P   P F
 *                 U   U M M M D   D    O   O  SSS  PPPPP FFFF
 *                 U   U M M M D   D    O   O     S P     F
 *                  UUU  M M M DDDD     OOOOO SSSSS P     F
 * 
 *     		          Copyright 1989, 1990, 1991
 *     	       The University of Maryland, College Park, Maryland.
 * 
 * 			    All Rights Reserved
 * 
 *      The University of Maryland College Park ("UMCP") is the owner of all
 *      right, title and interest in and to UMD OSPF (the "Software").
 *      Permission to use, copy and modify the Software and its documentation
 *      solely for non-commercial purposes is granted subject to the following
 *      terms and conditions:
 * 
 *      1. This copyright notice and these terms shall appear in all copies
 * 	 of the Software and its supporting documentation.
 * 
 *      2. The Software shall not be distributed, sold or used in any way in
 * 	 a commercial product, without UMCP's prior written consent.
 * 
 *      3. The origin of this software may not be misrepresented, either by
 *         explicit claim or by omission.
 * 
 *      4. Modified or altered versions must be plainly marked as such, and
 * 	 must not be misrepresented as being the original software.
 * 
 *      5. The Software is provided "AS IS". User acknowledges that the
 *         Software has been developed for research purposes only. User
 * 	 agrees that use of the Software is at user's own risk. UMCP
 * 	 disclaims all warrenties, express and implied, including but
 * 	 not limited to, the implied warranties of merchantability, and
 * 	 fitness for a particular purpose.
 * 
 *     Royalty-free licenses to redistribute UMD OSPF are available from
 *     The University Of Maryland, College Park.
 *       For details contact:
 * 	        Office of Technology Liaison
 * 		4312 Knox Road
 * 		University Of Maryland
 * 		College Park, Maryland 20742
 * 		     (301) 405-4209
 * 		FAX: (301) 314-9871
 * 
 *     This software was written by Rob Coltun
 *      rcoltun@ni.umd.edu
 */
