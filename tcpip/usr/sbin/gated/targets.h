/* @(#)33	1.1  src/tcpip/usr/sbin/gated/targets.h, tcprouting, tcpip411, GOLD410 12/6/93 17:27:57 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		TARGET_LIST
 *		TARGET_LIST_END
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
 * targets.h,v 1.12 1993/04/05 04:30:09 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Target definitions.  Here for lack of a better place */

typedef struct _target {
    struct _target *target_next;	/* Pointer to the next one */
    sockaddr_un	**target_dst;		/* Destination */
    sockaddr_un	**target_src;		/* Source */
    if_addr	*target_ifap;		/* Interface */
    gw_entry	*target_gwp;		/* Gateway if available */
    flag_t	target_rtbit;		/* Bit for this target */
    flag_t	target_flags;		/* Protocol dependent flags */
    size_t	target_maxsize;		/* Max packet size */
    void_t	target_packet;		/* Packet for this target */
    void_t	target_fillp;		/* Fill pointer */
    _PROTOTYPE(target_reset,
	       void,
	       (task *,
		struct _target *));	/* Routine to clean up a target being freed */
    
} target;

#define	TARGETF_BROADCAST	0x01		/* This is a broadcast address */
#define	TARGETF_SOURCE		0x02		/* This is a P2P client */
#define	TARGETF_SUPPLY		0x04		/* We supply updates to this client */
#define	TARGETF_ALLINTF		0x08		/* We want to see all interfaces */
#define	TARGETF_POLICY		0x10		/* Initial policy has been run */

#define	TARGETF_USER4		0x1000
#define	TARGETF_USER3		0x2000
#define	TARGETF_USER2		0x4000
#define	TARGETF_USER1		0x8000		/* For protocol use */


#define	TARGET_LIST(tlp, list)	for (tlp = list; tlp; tlp = tlp->target_next)
#define	TARGET_LIST_END(tlp, list)

PROTOTYPE(target_locate,
	  extern target *,
	  (target *,
	   if_addr *,
	   gw_entry *));
PROTOTYPE(target_free,
	  extern target *,
	  (task *,
	   target *));
PROTOTYPE(target_build,
	  extern int,
	  (task *,
	   target **,
	   gw_entry *,
	   flag_t,
	   u_int,
	   _PROTOTYPE(dump,
		      void,
		      (rt_head *,
		       void_t,
		       char *))));
PROTOTYPE(target_dump,
	  extern void,
	  (FILE *,
	   target *,
	   const bits *));



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
