/* @(#)88	1.1  src/tcpip/usr/sbin/gated/ospf_const.h, tcprouting, tcpip411, GOLD410 12/6/93 17:25:54 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: MORE_RECENT
 *		NEXTNSEQ
 *		NEXTSEQ
 *		ODIFF
 *		OMAX
 *		OMIN
 *		SAME_INSTANCE
 *		SEQCMP
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
 * ospf_const.h,v 1.8 1992/10/08 19:55:55 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Values are on second boundarys */
#ifndef CONST_H
#define CONST_H
#define	SECONDS	1
#define	MINUTES	60
#define HOURS	(60 * MINUTES)

#define	IfChkAlrmTime	(30)
#define DbAgeTime	(900)
#define	AseAgeTime	(61)
#define LSRefreshTime	(1800)
#define	MinLSInterval	(5)
#define MaxAgeDiff	(900)
#define MaxAge		(3600)
#define	CheckAge	(300)
#define MaxAgeDiff	(900)

/*
 * Every AseAgeTime age ASE_AGE_NDX_ADD hash buckets so we don't
 * cause massive flooding
 * This may be adjusted - change with AseAgeTime
 */
#define ASE_AGE_NDX_ADD       2

/* a few offsets so events occur at the same time very infrequently */
#define OFF1	(37)
#define OFF2	(131)
#define OFF3	(199)
#define OFF4	(263)
#define OFF5	(327)
#define OFF6	(391)

#define BaseSeq		htonl(0x80000000)
#define HBaseSeq	0x80000000	/* host order */
#define FirstSeq	htonl((HBaseSeq) + 1)
#define HFirstSeq	((HBaseSeq) + 1)
#define MaxSeqNum	0x7FFFFFFF

/* returns True if A is newer */
#define SEQCMP(A,B)    (A > B)

#define OMAX(A,B) (((A) > (B)) ? (A) : (B))
#define OMIN(A,B) (((A) < (B)) ? (A) : (B))
#define ODIFF(A,B) (OMAX((A),(B)) - OMIN((A),(B)))

/* for these macros, the version that is held in the lsdb should be B */
#define MORE_RECENT(A,B,Elapse) \
	( (SEQCMP(ntohl((A)->ls_seq),ntohl((B)->ls_seq))) ||\
    	  ( ((B)->ls_seq == (A)->ls_seq) &&\
	    ( ((A)->ls_chksum != (B)->ls_chksum &&\
		(ntohs((A)->ls_chksum) > ntohs((B)->ls_chksum))) ||\
	      (((A)->ls_age == MaxAge) && ((B)->ls_age != MaxAge)) ||\
	      ( (((A)->ls_age != MaxAge) && ((B)->ls_age != MaxAge)) &&\
    	        ((ODIFF(((B)->ls_age + Elapse),((A)->ls_age)) > MaxAgeDiff) &&\
		(A)->ls_age < ((B)->ls_age + Elapse)) ) ) ) )

#define SAME_INSTANCE(A,B,Elapse)\
	( (A)->ls_seq == (B)->ls_seq && \
	 !(((A)->ls_age == MaxAge) ^ ((B)->ls_age == MaxAge)) && \
	 ODIFF((B)->ls_age + Elapse, (A)->ls_age) <= MaxAgeDiff && \
	 (A)->ls_chksum == (B)->ls_chksum )

#define NEXTSEQ(S) ((((S) + 1) == HBaseSeq) ? HFirstSeq : ((S) + 1))

#define NEXTNSEQ(S) (htonl(NEXTSEQ(ntohl(S))))	/* from net to host and back */


#define RTRLSInfinity	0xFFFF
#define SUMLSInfinity	0xFFFFFF
#define ASELSInfinity	0xFFFFFF

#define	OSPF_ADDR_ALLSPF	0xe0000005	/* 224.0.0.5 */
#define	OSPF_ADDR_ALLDR		0xe0000006	/* 224.0.0.6 */

/*
 * Default configuration defines
 */
#define OSPF_BC_DFT_HELLO	10
#define OSPF_NBMA_DFT_HELLO	30
#define OSPF_PTP_DFT_HELLO	30
#define OSPF_VIRT_DFT_HELLO	30

#define OSPF_DFT_RETRANS	5
#define OSPF_VIRT_DFT_RETRANS	30
#define OSPF_DFLT_POLL_INT	120
#define	OSPF_DFLT_TRANSDLY	1
#define	OSPF_VIRT_DFLT_TRANSDLY	4
#define OSPF_DFLT_COST		1

#define	OSPF_T_ACK		1

/* Ls_ase are originated every MinASEInterval over a period of LSRefreshTime
 *    - Just do ASEGenLimit at a time
 *    - cur_ase holds current ptr to my_ase_list
 *    - ase_start holds new cycle start time to determine next interval
 * This may be adjusted
*/

#define	ASEGenLimit	100	/* # of LS_ASE generated tq_AseLsa period */
#define	MinASEInterval	(1)	/* Can send ASEGenLimit over this value */

#define MAXOUT  	512	/* Maxmimum tx pkt size less ip hdr stuff */
#define	MAXRXPKTSIZ 	2000	/* Maximum rx pkt size */
#define MaxSortBufSize	2500	/* Buffer for sorting the lsdb */

#endif	/* CONST_H */


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
