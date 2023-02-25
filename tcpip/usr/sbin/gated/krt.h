/* @(#)74	1.1  src/tcpip/usr/sbin/gated/krt.h, tcprouting, tcpip411, GOLD410 12/6/93 17:25:18 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		if_check
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
 * krt.h,v 1.15 1993/03/22 02:39:25 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Kernel interface definitions */

extern int krt_install;			/* if TRUE install route in kernel */
extern int krt_install_parse;		/* if TRUE install route in kernel */
extern char *krt_version_kernel;
extern gw_entry *krt_gw_list;
extern task *krt_task;

/* Timers */
#define	KRT_TIMER_IFCHECK	0	/* For checking interfaces */
#ifdef	KRT_RT_SOCK
#define	KRT_TIMER_TIMEOUT	1	/* For routing socket retries */
#define	KRT_TIMER_MAX		2
#else	/* KRT_RT_SOCK */
#define	KRT_TIMER_MAX		1
#endif	/* KRT_RT_SOCK */

#ifdef	KRT_IFREAD_KINFO
/* Scan less frequently because we should see notification */
#define	KRT_T_IFCHECK	(time_t) 60
#else	/* KRT_IFREAD_KINFO */
/* Scan often so we notice changes quickly */
#define	KRT_T_IFCHECK	(time_t) 15
#endif	/* KRT_IFREAD_KINFO */
#define	KRT_T_EXPIRE	(time_t) 180

/* For parser */
#define	KRT_LIMIT_SCANTIMER	KRT_T_IFCHECK, 3600

#define	if_check()	(void) krt_ifread(task_state)


PROTOTYPE(krt_init,
	  extern void,
	  (void));		/* Read kernel routing tables and other useful info */
PROTOTYPE(krt_flash,
	  extern void,
	  (rt_list *rtl));		/* Process a flash list */
PROTOTYPE(krt_installed,
	  extern void,
	  (rt_entry *rt));		/* Flag route as being installed */
PROTOTYPE(krt_delete_dst,
	  extern int,
	  (task *,
	   rt_entry *,
	   rt_parms *,
	   sockaddr_un *,
	   proto_t,
	   gw_entry **));				/* Delete a kernel route */
PROTOTYPE(krt_state_to_flags,
	  extern flag_t,
	  (flag_t));
PROTOTYPE(krt_ifread,
	  extern int,
	  (flag_t));
#ifdef	IP_MULTICAST
PROTOTYPE(krt_multicast_add,
	  extern void,
	  (sockaddr_un *));
PROTOTYPE(krt_multicast_delete,
	  extern void,
	  (sockaddr_un *));
#endif	/* IP_MULTICAST */
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
