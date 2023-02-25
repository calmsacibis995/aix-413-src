/* @(#)43	1.4  src/tcpip/usr/sbin/gated/egp_param.h, tcprouting, tcpip411, GOLD410 12/6/93 17:41:33 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: none
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
 * egp_param.h,v 1.10 1993/02/08 18:28:10 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* egp_param.h
 *
 * Defines various egp parameters
 */

#ifndef IPPROTO_EGP
#define IPPROTO_EGP 8
#endif

#define EGP_PKTSIZE	8192		/* Maximum EGP packet size we can send and receive */
#define	EGP_PKTSLOP	1024		/* Slop in packet size to make it easier to detect overflow */

/* RFC904 defined parameters */
#define EGP_P1		30		/* Minimum interval for sending and receiving hellos */
#define EGP_P2		120		/* Minimum interval for sending and receiving polls */

#define	EGP_P3		30		/* interval between Request or Cease command retransmissions */
#define	EGP_P4		3600		/* interval during which state variables are maintained in */
					/* the absence of commands or responses in the Down or Up states */
#define	EGP_P5		120		/* interval during which state variables are maintained in */
					/* the absence of responses in the Acquisition and Cease states */

/* Automatic restart timers */
#define	EGP_START_RETRY	EGP_P5		/* Retry if max neighbors already acquired */
#define	EGP_START_SHORT	EGP_P5		/* Retry neighbor in 2 minutes */
#define	EGP_START_LONG	EGP_P4		/* Retry neighbor in an hour */

/* Hello interval constants */

#define MAXHELLOINT	900		/* Maximum hello interval, sec. */
#define HELLOMARGIN	2		/* Margin in hello interval to allow for delay variation in */
					/* the network */
/* Poll interval constants */

#define MAXPOLLINT  	3600		/* Maximum poll interval, sec. */

/* repoll interval is set to the hello interval for the particular neighbor */

/* Reachability test constants */

#define REACH_RATIO	4		/* No. commands sent on which reachability is based */
#define MAXNOUPDATE	3		/* Maximum # successive polls (new id) for which no update */
					/* was received before cease  and try to acquire an alternative */

#define	RATE_WINDOW	4		/* Size of polling rate window */
#define	RATE_MAX	3		/* Number of violations before generating an error message */


/* Limits of gateways per update */
#define	EGP_GATEWAY_MAX		255

/* Limits of EGP distance */
#define	EGP_DISTANCE_MIN	0
#define	EGP_DISTANCE_MAX	255

/* Limits of EGP networks per distance */
#define	EGP_ROUTES_MAX		255


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
