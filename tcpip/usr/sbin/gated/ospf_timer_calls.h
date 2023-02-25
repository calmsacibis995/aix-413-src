/* @(#)10	1.1  src/tcpip/usr/sbin/gated/ospf_timer_calls.h, tcprouting, tcpip411, GOLD410 12/6/93 17:26:53 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: NET_LSA_LOCK
 *		OSPF_TQ_HANDLE
 *		RTR_LSA_LOCK
 *		net_lsa_lockout
 *		rem_hold_tmr
 *		rem_inact_tmr
 *		rem_wait_tmr
 *		reset_inact_tmr
 *		reset_net_lock
 *		reset_net_sched
 *		reset_rtr_lock
 *		reset_rtr_sched
 *		rtr_lsa_lockout
 *		set_hold_tmr
 *		set_net_sched
 *		set_rtr_sched
 *		start_wait_tmr
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
 * ospf_timer_calls.h,v 1.9 1992/12/14 22:25:38 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define OSPF_TQ_HANDLE()	/* Null routine for now */

/*
 * Reset the inactivity timer - have seen hello from this neighbor
 */
#define reset_inact_tmr(N) (N)->last_hello = time_sec


/*
 * Set the holding timer (entering slave hold mode)
 */
#define set_hold_tmr(N) (N)->last_exch = time_sec


/*
 * Remove the holding timer (exiting slave hold mode)
 */
#define rem_hold_tmr(N) (N)->last_exch = 0


/*
 * Remove the inactivity - neighbor state has changed to down
 */
#define rem_inact_tmr(N) (N)->last_hello = 0


/*
 * Remove the wait timer - backup seen or interface down
 */
#define rem_wait_tmr(I) (I)->wait_time = 0


/*
 * Start the wait timer - interface up
 */
#define start_wait_tmr(I) (I)->wait_time = time_sec

/*
 * Lock out nets for MinLSInterval
 * - can't generate new LSA with MinLSInterval
 */
#define net_lsa_lockout(I)  ((I)->lock_time = time_sec)

#define set_net_sched(I)    {if ((I)->lock_time == 0) net_lsa_lockout(I);\
				BIT_SET((I)->flags, OSPF_INTFF_NETSCHED);}

#define rtr_lsa_lockout(A)  ((A)->lock_time = time_sec)

#define set_rtr_sched(A)    {if ((A)->lock_time == 0) rtr_lsa_lockout(A);\
				((A)->lsalock |= RTRSCHED);}

#define reset_net_lock(I)   ((I)->lock_time = 0)
#define reset_net_sched(I)  BIT_RESET((I)->flags, OSPF_INTFF_NETSCHED|OSPF_INTFF_BUILDNET);

#define reset_rtr_lock(A)   ((A)->lock_time = 0)
#define reset_rtr_sched(A)  ((A)->lsalock &= ~RTRSCHED)

#define NET_LSA_LOCK(I)     ((I)->lock_time != 0)
#define RTR_LSA_LOCK(A)     ((A)->lock_time != 0)


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
