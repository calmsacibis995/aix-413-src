/* @(#)94	1.1  src/tcpip/usr/sbin/gated/ospf_log.h, tcprouting, tcpip411, GOLD410 12/6/93 17:26:09 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: LOG_RTAB
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
 * ospf_log.h,v 1.10 1993/01/15 20:33:13 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


extern const char *ospf_con_types[];
extern const char *ospf_logtype[];
extern long ospf_cumlog[];
extern const char *ospf_intf_states[];
extern const char *ospf_nbr_states[];
extern const bits ospf_ls_type_bits[];

/**************************************************************************
	
			OLOG CODES

***************************************************************************/

#define	TR_OSPF_HELLO	TR_HELLO
#define	TR_OSPF_DD	TR_USR1
#define	TR_OSPF_REQ	TR_USR2
#define	TR_OSPF_LSU	TR_USR3
#define	TR_OSPF_ACK	TR_USR4
#define	TR_OSPF_LSA_BLD	TR_USR5
#define	TR_OSPF_SPF	TR_USR6
#define	TR_OSPF_LSA_TX	TR_USR7
#define	TR_OSPF_LSA_RX	TR_USR8
#define	TR_OSPF_TRAP	TR_USR9
#define	TR_OSPF_TRANS	TR_PROTOCOL
#define	TR_OSPF_PKT	TR_UPDATE

#define	TR_OSPF_RX	(TR_OSPF_HELLO | TR_OSPF_DD | TR_OSPF_REQ | TR_OSPF_LSU | TR_OSPF_ACK)
#define	TR_OSPF_DEFAULT	(TR_OSPF_RX | TR_OSPF_TRANS | TR_OSPF_PKT)

#define	LOG_RTAB(TYPE,ID,DEST)
#define	LOG_LSDB(TYPE,ID,LSTYPE,K0,K1)

#define GOOD_IPSUM	0xFFFF	/* good checksum could be -0 on some machines */

#define GOOD_RX		0

/* receive types */
#define	GOOD_MON	0
#define	GOOD_HELLO	1
#define	GOOD_DB_DESCRIPT	2
#define	GOOD_LSR	3
#define	GOOD_LSU	4
#define	GOOD_LSA	5
/* send types
#define		O_MON
#define		O_HELLO
#define		O_DB_DESCRIPT
#define		O_LSR
#define		O_LSU
#define		O_LSA
*/

#define	BAD_OSPF_TYPE	12
#define BAD_IP_DEST	13
#define BAD_IP_PROTOID	14
#define	MY_IP_SRC	15
#define BAD_OSPF_VERSION	16
#define	BAD_OSPF_CHKSUM	17
#define BAD_IF_AREAID	18
#define RXPACKET_TO_ABR	19
#define BAD_VL		20
#define	BAD_AUTH_TYPE	21
#define	BAD_AUTH_KEY	22
#define	PKT_TOO_SMALL	23
#define BAD_OSPF_LENGTH	24
#define	TX_SENDTO_BAD	25
#define	RX_ON_DOWN_IF   26
#define BAD_HELLO_MASK  27
#define BAD_HELLO_TIMER 28
#define BAD_DEAD_TIMER 	29
#define BAD_HELLO_E_bit 30
#define HELLO_ID_CONFUSION 31
#define UNKNOWN_VIRT_NBR 32
#define UNKNOWN_NBMA_NBR 33
#define CANT_FIND_NBR1  34
#define LOW_NBR_STATE1	35
#define BAD_RTRID  	36
#define BAD_DD_E_bit	37
#define NO_ACK_NBR	38
#define LOW_NBR_STATE3	39
#define LOW_NBR_STATE4	40
#define CANT_FIND_NBR4	41
#define EMPTY_LS_REQ	42
#define BOGUS_REQ       43
#define LOW_NBR_STATE2	44
#define CANT_FIND_NBR2	45
#define NEWER_LSU	46
#define BAD_LS_CHKSUM   47
#define UNUSUAL_INSTANCE 48
#define BAD_LSA_TYPE 	49

#define LASTLOG		50	/* best if even for err_dump() */


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
