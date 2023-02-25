/* @(#)53	1.5  src/tcpip/usr/sbin/gated/include.h, tcprouting, tcpip411, GOLD410 12/6/93 17:49:32 */
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
 * include.h,v 1.26 1993/04/05 04:24:42 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#ifndef	_GATED_INCLUDE_H
#define	_GATED_INCLUDE_H

#include "defines.h"

#ifdef	vax11c
#include "[.vms]gated_named.h"
#endif	/* vax11c */

#if	defined(_POWER) && !defined(_BSD)
#define	_BSD	1
#endif

#include <sys/param.h>			/* Was types */
#ifdef	INCLUDE_TYPES
#include <sys/types.h>
#endif	/* INCLUDE_TYPES */
#ifdef	INCLUDE_BSDTYPES
#include <sys/bsdtypes.h>
#endif	/* INCLUDE_TYPES */
#ifdef	USE_STREAMIO
#include <sys/stream.h>
#endif	/* USE_STREAMIO */
#ifdef	vax11c
#include <sys/ttychars.h>
#include <sys/ttydev.h>
#endif	/* vax11c */
#include <sys/uio.h>

#include <sys/socket.h>
#if	defined(SYSV)
#undef	SO_RCVBUF
#endif

#include <stdio.h>
#include <netdb.h>
#include <sys/errno.h>
#ifdef	SYSV
#undef ENAMETOOLONG
#undef ENOTEMPTY
#include <net/errno.h>
#endif
#ifdef	INCLUDE_STRING
#include <string.h>
#else
#include <strings.h>
#endif

#ifdef	INCLUDE_TIME
#if	defined(_POWER) || defined(_AUX_SOURCE) || defined(__sgi)
#include <time.h>
#endif
#include <sys/time.h>
#endif

#ifndef	NO_MBUF_H
#include <sys/mbuf.h>
#endif	/* NO_MBUF_H */
#ifdef vax11c
#define DONT_INCLUDE_IF_ARP
#endif	/* vax11c */
#include <net/if.h>

#ifdef	PROTO_UNIX
#include <sys/un.h>
#endif

#if	defined(PROTO_INET)
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#ifndef	HPUX7_X
#include <netinet/in_var.h>
#include <arpa/inet.h>
#endif	/* HPUX7_X */
#ifdef	INCLUDE_ETHER
#if	defined(AIX) && !defined(_POWER)
#include <sys/if_ieee802.h>
#else
#if	defined(_POWER) && !defined(IPPROTO_TP)	/* AIX 3.1 */
#include <netinet/in_netarp.h>
#else	/* defined(_POWER) && !defined(IPPROTO_TP) */
#include <netinet/if_ether.h>
#endif	/* defined(_POWER) && !defined(IPPROTO_TP) */
#endif	/* defined(AIX) && !define(_POWER) */
#endif	/* INCLUDE_ETHER */
#if	defined(INCLUDE_MROUTE) && defined(IFF_MULTICAST) && defined(PROTO_INET) && defined(IP_MULTICAST_ROUTING)
#include <netinet/ip_mroute.h>
#endif

#ifdef	INCLUDE_UDP
#include <netinet/udp.h>
#endif
#endif	/* PROTO_INET */

#if	defined(PROTO_ISO)
#include <netiso/iso.h>
#include <netiso/iso_var.h>
#ifdef	INCLUDE_CLNP
#include <netiso/clnp.h>
#endif
#ifdef	INCLUDE_ESIS
#include <netiso/esis.h>
#endif
#ifdef	INCLUDE_SNPA
#include <netiso/iso_snpac.h>
#endif
#endif	/* PROTO_ISO */

#ifdef	INCLUDE_ROUTE
#include <net/route.h>
#undef	KERNEL
#endif

#ifdef	INCLUDE_PATHS
#include <paths.h>
#endif
#include "paths.h"

#ifdef	INCLUDE_WAIT
#include <sys/wait.h>
#ifdef	notdef
#include <sys/resource.h>
#endif
#endif

#include "config.h"

#ifdef	INCLUDE_UNISTD
#include <unistd.h>
#endif

#ifdef	INCLUDE_IF_DL
#include <net/if_dl.h>
#endif	/* INCLUDE_IF_DL */
#if	defined(INCLUDE_IF_TYPES) && defined(SOCKADDR_DL)
#include <net/if_types.h>
#endif

#if	defined(AIX)
#include <sys/syslog.h>
#else				/* defined(AIX) */
#include <syslog.h>
#endif				/* defined(AIX) */

#ifdef	INCLUDE_SIGNAL
#include <signal.h>
#endif

#ifdef	INCLUDE_FILE
#include <sys/file.h>
#ifdef	INCLUDE_FCNTL
#include <sys/fcntl.h>
#endif
#endif

#if	!defined(NO_STAT) && defined(INCLUDE_STAT)
#include <sys/stat.h>
#endif

#ifdef	INCLUDE_IOCTL
#ifdef	INCLUDE_SOCKIO
#include <sys/sockio.h>
#endif	/* INCLUDE_SOCKIO */
#ifdef	INCLUDE_SIOCTL
#include <sys/sioctl.h>
#endif	/* INCLUDE_SIOCTL */
#ifdef	USE_STREAMIO
#include <sys/stropts.h>
#else	/* USE_STREAMIO */
#include <sys/ioctl.h>
#endif	/* USE_STREAMIO */
#endif	/* INCLUDE_IOCTL */

#ifdef	INCLUDE_NLIST
#include <nlist.h>
#endif

#ifdef	INCLUDE_KINFO
#ifdef	USE_SYSCTL
#include <sys/sysctl.h>
#else	/* USE_SYSCTL */
#if	(defined(KRT_RTREAD_KINFO) \
	 || defined(KRT_IFREAD_KINFO)) \
    && !defined(KINFO_RT_DUMP)
#include <sys/kinfo.h>
#endif
#endif	/* USE_SYSCTL */
#endif	/* INCLUDE_KINFO */

#if	defined(INCLUDE_NETOPT_POWER) && defined(_POWER) && !defined(SIOCGNETOPT)
#include <net/netopt.h>
#endif

#ifdef	INCLUDE_CTYPE
#include <ctype.h>
#endif

#if	defined(INCLUDE_DIRENT) && defined(HAVE_DIRENT)
#include <dirent.h>
#endif

#ifdef	STDARG
#ifndef	va_arg
#include <stdarg.h>
#endif	/* va_arg */
#else	/* STDARG */
#include <varargs.h>
#endif	/* STDARG */

#ifdef	INCLUDE_ISODE_SNMP
#ifdef _POWER
#include <snmp/objects.h>
#include <snmp/smux.h>
#else
#include "objects.h"
#include "smux.h"
#endif /* _POWER */
#undef	sprintf
#endif

#include "defs.h"
#include "sockaddr.h"
#include "str.h"
#ifdef	notdef
#include "inet.h"
#endif	/* notdef */
#include "policy.h"
#ifdef	PROTO_ASPATHS
#include "aspath.h"
#endif
#ifdef	INCLUDE_RT_VAR
#include "rt_var.h"
#endif
#include "rt_table.h"
#include "if.h"
#include "task.h"
#include "trace.h"
#ifndef	INCLUDE_UNISTD
#include "unix.h"
#endif

#endif	/* _GATED_INCLUDE_H */


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
