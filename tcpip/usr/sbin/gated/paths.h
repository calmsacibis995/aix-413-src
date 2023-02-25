/* @(#)14	1.1  src/tcpip/usr/sbin/gated/paths.h, tcprouting, tcpip411, GOLD410 12/6/93 17:27:05 */
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
 * paths.h,v 1.11 1993/03/25 12:47:28 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Gated specific Paths from configuration */

#define	_PATH_CONFIG	"/etc/%s.conf"
#define	_PATH_DEFS	"@(_path_defs)"
#define	_PATH_DUMP	"/var/tmp/%s_dump"
#define	_PATH_DUMPDIR	"/var/tmp"
#define	_PATH_PID	"/etc/%s.pid"
#define	_PATH_VERSION	"/etc/%s.version"
#define	_PATH_BINDIR	"/usr/sbin"
#define	_PATH_SBINDIR	"/usr/sbin"

/* System specific paths (not everyone runs BSD 4.4, do they) */
#ifndef	_PATH_DEVNULL
#define	_PATH_DEVNULL		"/dev/null"
#endif	/* _PATH_DEVNULL */

#ifndef	_PATH_DEV
#define	_PATH_DEV	"/dev"
#endif	/* _PATH_DEV */

#ifndef	_PATH_KMEM
#define	_PATH_KMEM	"/dev/kmem"
#endif	/* _PATH_KMEM */

#ifndef	_PATH_TTY
#define	_PATH_TTY	"/dev/tty"
#endif	/* _PATH_TTY */

#ifndef	_PATH_UNIX
#define	_PATH_UNIX	"/unix"
#endif	/* _PATH_UNIX */

#ifndef	_PATH_VARTMP
#define	_PATH_VARTMP	"/var/tmp"
#endif	/* _PATH_VARTMP */

/* Non-path information */
#ifndef	CONFIG_MODE
#define	CONFIG_MODE	0664
#endif	/* CONFIG_MODE */

#ifndef	GDC_GROUP
#define	GDC_GROUP	"gdmaint"
#endif	/* GDC_GROUP */

#ifndef	NAME_GATED
#define	NAME_GATED	"gated"
#endif	/* NAME_GATED */

#ifndef	NAME_GDC
#define	NAME_GDC	"gdc"
#endif	/* NAME_GDC */

#ifndef	NAME_RIPQUERY
#define	NAME_RIPQUERY	"ripquery"
#endif	/* NAME_RIPQUERY */

#ifndef	NAME_OSPF_MONITOR
#define	NAME_OSPF_MONITOR	"ospf_monitor"
#endif	/* NAME_OSPF_MONITOR */


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
