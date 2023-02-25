/* @(#)38	1.1  src/tcpip/usr/sbin/gated/str.h, tcprouting, tcpip411, GOLD410 12/6/93 18:02:29 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		PROTOTYPEV
 *		PickUp
 *		PickUpStr
 *		PutDown
 *		PutDownStr
 *		gd_lower
 *		gd_upper
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
 * str.h,v 1.8.2.1 1993/08/27 18:27:45 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Definitions for putting data into and getting data out of packets */
/* in a machine dependent manner. */
#define	PickUp(s, d)	bcopy((caddr_t) s, (caddr_t)&d, sizeof(d));	s += sizeof(d);
#define	PutDown(s, d)	bcopy((caddr_t)&d, (caddr_t) s, sizeof(d));	s += sizeof(d);
#define	PickUpStr(s, d, l)	bcopy((caddr_t) s, (caddr_t) d, l);	s += l;
#define	PutDownStr(s, d, l)	bcopy((caddr_t) d, (caddr_t) s, l);	s += l;

#ifdef	notdef
#define	PickUp(s, d)	{ \
    register int PickUp_i = sizeof(d); \
    d = 0; \
    while (PickUp_i--) { \
	d <<= 8; \
        d |= *s++;
    } \
}
#define	PutDown(s, d)	{
    register int i = sizeof(d);
    register long ii = d;
    register caddr_t cp;

    cp = (s += i);
    while (i--) {
	*--cp = (ii & 0xff);
	ii >>= 8;
    }
}
#define	PickUpStr(s, d, l) {
    register int i = l;
    register char *cp = d;

    while (i--) {
	*cp++ = *s++;
    }
}
#define	PutDownStr(s, d, l) {
    register int i = l;
    register char *cp = s;

    while (i--) {
	*s++ = *cp++;
    }
}
#endif	/* notdef */

PROTOTYPE(gd_uplow,
	  extern char *,
	  (const char *,
	   int));
#define	gd_upper(str)	gd_uplow(str, TRUE)
#define	gd_lower(str)	gd_uplow(str, FALSE)
PROTOTYPEV(fprintf,
	   extern int,
	   (FILE *,
	    const char *,
	    ...));
PROTOTYPEV(vsprintf,
	   extern int,
	   (char *,
	    const char *,
	    va_list *));
PROTOTYPEV(sprintf,
	   extern int,
	   (char *,
	    const char *,
	    ...));
PROTOTYPE(strcasecmp,
	  extern int,
	  (const char *,
	   const char *));
PROTOTYPE(strncasecmp,
	  extern int,
	  (const char *,
	   const char *,
	   size_t));


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
