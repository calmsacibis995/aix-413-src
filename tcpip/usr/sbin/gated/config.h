/* @(#)38	1.8  src/tcpip/usr/sbin/gated/config.h, tcprouting, tcpip411, GOLD410 12/6/93 17:35:29 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ALIGN
 *		GHTONL
 *		GHTONS
 *		GNTOHL
 *		GNTOHS
 *		HTONL
 *		HTONS
 *		NON_INTR
 *		NTOHL
 *		NTOHS
 *		PROTOTYPE
 *		PROTOTYPEV
 *		WEXITSTATUS
 *		WIFCOREDUMP
 *		WSTOPSIG
 *		WTERMSIG
 *		_PROTOTYPE
 *		__PF0
 *		__PF1
 *		__PF10
 *		__PF11
 *		__PF12
 *		__PF2
 *		__PF3
 *		__PF4
 *		__PF5
 *		__PF6
 *		__PF7
 *		__PF8
 *		__PF9
 *		defined
 *		waitpid
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
 * config.h,v 1.34 1993/04/06 03:47:26 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* compiler switches */

/* How about a standard AIX define? */
#if	(defined(_AIX) || defined(__AIX)) && !defined(AIX)
#define	AIX
#endif

/* RS/6000 hacks */
#if	defined(_POWER)
#ifdef	_WINT_T
#define	AIX3_2
#else	/* _WINT_T */
#define	AIX3_1
#endif
#endif

/* For AIX systems */
#if	defined(_BSD) && !defined(BSD)
#define	BSD	_BSD
#endif

/* IBM's HighC supports varargs and prototypes if -Hnocpp is used, but this breaks ioctls and -Hpcc does not fix it. */

/* Systems that support ANSI varargs */
#if	defined(__STDC__) && \
    (!defined(ibm032) || (__GNUC__ > 1))
#define	STDARG
#endif

/* Systems where function prototypes work */
#if	defined(__STDC__) || defined(_POWER)
#define	USE_PROTOTYPES
#define	PROTOTYPE(func, return, proto)	return func proto
#define	_PROTOTYPE(func, return, proto)	return (*func) proto
#ifdef	STDARG
#define	PROTOTYPEV(func, return, proto)	return func proto
#else	/* STDARG */
#define	PROTOTYPEV(func, return, proto) return func()
#endif	/* STDARG */
#define	__PF0(void)	(void)
#define	__PF1(arg1, type1) (type1 arg1)
#define	__PF2(arg1, type1, \
	      arg2, type2) \
    (type1 arg1, \
     type2 arg2)
#define	__PF3(arg1, type1, \
	      arg2, type2, \
	      arg3, type3) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3)
#define	__PF4(arg1, type1, \
	      arg2, type2, \
	      arg3, type3, \
	      arg4, type4) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4)
#define	__PF5(arg1, type1, \
	      arg2, type2, \
	      arg3, type3, \
	      arg4, type4, \
	      arg5, type5) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5)
#define	__PF6(arg1, type1, \
	      arg2, type2, \
	      arg3, type3, \
	      arg4, type4, \
	      arg5, type5, \
	      arg6, type6) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6)
#define	__PF7(arg1, type1, \
	      arg2, type2, \
	      arg3, type3, \
	      arg4, type4, \
	      arg5, type5, \
	      arg6, type6, \
	      arg7, type7) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6, \
     type7 arg7)
#define	__PF8(arg1, type1, \
	      arg2, type2, \
	      arg3, type3, \
	      arg4, type4, \
	      arg5, type5, \
	      arg6, type6, \
	      arg7, type7, \
	      arg8, type8) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6, \
     type7 arg7, \
     type8 arg8)
#define	__PF9(arg1, type1, \
	      arg2, type2, \
	      arg3, type3, \
	      arg4, type4, \
	      arg5, type5, \
	      arg6, type6, \
	      arg7, type7, \
	      arg8, type8, \
	      arg9, type9) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6, \
     type7 arg7, \
     type8 arg8, \
     type9 arg9)
#define	__PF10(arg1, type1, \
	       arg2, type2, \
	       arg3, type3, \
	       arg4, type4, \
	       arg5, type5, \
	       arg6, type6, \
	       arg7, type7, \
	       arg8, type8, \
	       arg9, type9, \
	       arg10, type10) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6, \
     type7 arg7, \
     type8 arg8, \
     type9 arg9, \
     type10 arg10)
#define	__PF11(arg1, type1, \
	       arg2, type2, \
	       arg3, type3, \
	       arg4, type4, \
	       arg5, type5, \
	       arg6, type6, \
	       arg7, type7, \
	       arg8, type8, \
	       arg9, type9, \
	       arg10, type10, \
	       arg11, type11) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6, \
     type7 arg7, \
     type8 arg8, \
     type9 arg9, \
     type10 arg10, \
     type11 arg11)
#define	__PF12(arg1, type1, \
	       arg2, type2, \
	       arg3, type3, \
	       arg4, type4, \
	       arg5, type5, \
	       arg6, type6, \
	       arg7, type7, \
	       arg8, type8, \
	       arg9, type9, \
	       arg10, type10, \
	       arg11, type11, \
	       arg12, type12) \
    (type1 arg1, \
     type2 arg2, \
     type3 arg3, \
     type4 arg4, \
     type5 arg5, \
     type6 arg6, \
     type7 arg7, \
     type8 arg8, \
     type9 arg9, \
     type10 arg10, \
     type11 arg11, \
     type12 arg12)
#else	/* USE_PROTOTYPES */
#define	PROTOTYPE(func, return, proto)	return func()
#define	_PROTOTYPE(func, return, proto)	return (*func) ()
#define	PROTOTYPEV(func, return, proto) return func()
#define	__PF0(void) ()
#define	__PF1(arg1, type1) (arg1) \
    type1 arg1;
#define	__PF2(arg1, type1, arg2, type2) \
    (arg1, arg2) \
    type1 arg1; \
    type2 arg2;
#define	__PF3(arg1, type1, arg2, type2, arg3, type3) \
    (arg1, arg2, arg3) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3;
#define	__PF4(arg1, type1, arg2, type2, arg3, type3, arg4, type4) \
    (arg1, arg2, arg3, arg4) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4;
#define	__PF5(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5) \
    (arg1, arg2, arg3, arg4, arg5) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5;
#define	__PF6(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6) \
    (arg1, arg2, arg3, arg4, arg5, arg6) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6;
#define	__PF7(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6, arg7, type7) \
    (arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6; \
    type7 arg7;
#define	__PF8(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6, arg7, type7, arg8, type8) \
    (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6; \
    type7 arg7; \
    type8 arg8;
#define	__PF9(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6, arg7, type7, arg8, type8, arg9, type9) \
    (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6; \
    type7 arg7; \
    type8 arg8; \
    type9 arg9;
#define	__PF10(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6, arg7, type7, arg8, type8, arg9, type9, arg10, type10) \
    (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6; \
    type7 arg7; \
    type8 arg8; \
    type9 arg9; \
    type10 arg10;    
#define	__PF11(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6, arg7, type7, arg8, type8, arg9, type9, arg10, type10, arg11, type11) \
    (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6; \
    type7 arg7; \
    type8 arg8; \
    type9 arg9; \
    type10 arg10; \
    type11 arg11;
#define	__PF12(arg1, type1, arg2, type2, arg3, type3, arg4, type4, arg5, type5, arg6, type6, arg7, type7, arg8, type8, arg9, type9, arg10, type10, arg11, type11, arg12, type12) \
    (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) \
    type1 arg1; \
    type2 arg2; \
    type3 arg3; \
    type4 arg4; \
    type5 arg5; \
    type6 arg6; \
    type7 arg7; \
    type8 arg8; \
    type9 arg9; \
    type10 arg10; \
    type11 arg11; \
    type12 arg12;    
#endif	/* USE_PROTOTYPES */


#if	defined(SOCKADDR_DL) && !defined(INCLUDE_IF_DL)
/*
 * Structure of a Link-Level sockaddr:
 */
struct sockaddr_dl {
	u_char	sdl_len;	/* Total length of sockaddr */
	u_char	sdl_family;	/* AF_DLI */
	u_short	sdl_index;	/* if != 0, system given index for interface */
	u_char	sdl_type;	/* interface type */
	u_char	sdl_nlen;	/* interface name length, no trailing 0 reqd. */
	u_char	sdl_alen;	/* link level address length */
	u_char	sdl_slen;	/* link layer selector length */
	char	sdl_data[12];	/* minimum work area, can be larger;
				   contains both if name and ll address */
};
#endif


/* Some systems require this */
#ifndef	ALIGN
#define	ALIGN(p)	(((u_int)(p) + (sizeof(int) - 1)) &~ (sizeof(int) - 1))
#endif


/* Systems where the kernel treats all redirects as host redirects */
#if	!defined(KRT_RT_SOCK) && \
    defined(notdef)
#define	HOST_REDIRECTS_ONLY
#endif

/* Systems that have MULTICAST support */
#if	defined(IFF_MULTICAST) && defined(PROTO_INET)
#define	IP_MULTICAST
#endif

/* Make sure that ICMP is included if we don't have the routing socket */
#if	defined(PROTO_INET) \
    && (!defined(KRT_RT_SOCK) \
	&& !defined(PROTO_ICMP))
Fatal error - PROTO_ICMP not defined
#endif

    
/* POSIX compatible waitpid() */
#ifdef	HAVE_WAITPID
#ifdef  _POWER
#define	WAIT_T	union wait
#define	WIFCOREDUMP(statusp)	(statusp.w_coredump)
#else   /* _POWER */
#define	WAIT_T	int
#define	WIFCOREDUMP(statusp)	((statusp & 0x80) == 0x80)
#endif  /* _POWER */
#else	/* HAVE_WAITPID */
#define	waitpid(pid, statusp, options)	wait3(statusp, options, NULL)
#define	WAIT_T	union wait
#ifndef	WEXITSTATUS		
#define	WEXITSTATUS(statusp)	(statusp.w_retcode)
#endif	/* WEXITSTATUS */
#ifndef	WTERMSIG
#define	WTERMSIG(statusp)	(statusp.w_termsig)
#endif	/* WTERMSIG */
#ifndef	WSTOPSIG
#define	WSTOPSIG(statusp)	(statusp.w_stopsig)
#endif	/* WSTOPSIG */
#define	WIFCOREDUMP(statusp)	(statusp.w_coredump)
#endif	/* HAVE_WAITPID */


/* For now lets assume that the overhead of being non-interruptable is minimal */ 
#define	NON_INTR(rc, syscall)	while ((rc = (syscall)) == -1 && errno == EINTR)


/* Systems that do not support recvmsg() */
#ifdef	NEED_RECVMSG

struct msghdr {
    caddr_t msg_name;			/* optional address */
    int msg_namelen;			/* size of address */
    struct iovec *msg_iov;		/* scatter/gather array */
    int msg_iovlen;			/* # elements in msg_iov */
    caddr_t msg_accrights;		/* access rights sent/received */
    int msg_accrightslen;
};

#endif


/* BSD 4.4 defines UIO_MAXIOV */
#ifndef	MSG_MAXIOVLEN
#ifdef	UIO_MAXIOV
#define	MSG_MAXIOVLEN	UIO_MAXIOV
#else	/* UIO_MAXIOV */
#define	MSG_MAXIOVLEN	16
#endif	/* UIO_MAXIOV */
#endif	/* MSG_MAXIOVLEN */


#ifndef	LINE_MAX
#ifdef	_POSIX2_LINE_MAX
#define	LINE_MAX	_POSIX2_LINE_MAX
#else	/* _POSIX2_LINE_MAX */
#define	LINE_MAX	256
#endif
#endif	/* LINE_MAX */


/* Void type */
typedef	VOID_T	void_t;


/* Systems where routines place calls to stdout or stderr */
#if	defined(KVM_TYPE_SUNOS4) \
    || defined(PROTO_ISODE_SNMP)
#define	STDIO_HACK
#endif


/* Assume that if nothl()... are defined they are defined as null macros */

#ifndef	LITTLE_ENDIAN
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax) */
#define BIG_ENDIAN      4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN      3412    /* LSB first in word, MSW first in long (pdp) */
#endif	/* LITTLE_ENDIAN */

#if	defined(BYTE_ORDER) && BYTE_ORDER != BIG_ENDIAN
#if	!defined(NTOHL)
#define NTOHL(x)        (x) = ntohl(x)
#define NTOHS(x)        (x) = ntohs(x)
#define HTONL(x)        (x) = htonl(x)
#define HTONS(x)        (x) = htons(x)
#endif	/* !defined(NTOHL) */
#define	GNTOHL(x)	NTOHL(x);
#define	GNTOHS(x)	NTOHS(x);
#define	GHTONL(x)	HTONL(x);
#define	GHTONS(x)	HTONS(x);
#else	/* !BIG_ENDIAN */
#if	!defined(NTOHL)
#define NTOHL(x)        (x)
#define NTOHS(x)        (x)
#define HTONL(x)        (x)
#define HTONS(x)        (x)
#endif	/* !defined(NTOHL) */
#define	GNTOHL(x)
#define	GNTOHS(x)
#define	GHTONL(x)
#define	GHTONS(x)
#endif	/* !BIG_ENDIAN */

/* Protocols that need AS path support */
#if	(defined(PROTO_BGP) || defined(PROTO_OSPF)) && !defined(PROTO_ASPATHS)
Fatal error - AS paths not configured
#endif


/* Protocols that need routerid */
#if	(defined(PROTO_BGP) || defined(PROTO_OSPF)) && !defined(ROUTER_ID)
Fatal error - Router ID not configured
#endif

/* Protocols that need autonomous system */
#if	(defined(PROTO_BGP) || defined(PROTO_EGP) || defined(PROTO_OSPF)) && !defined(AUTONOMOUS_SYSTEM)
Fatal error - Autonomous systems not defined
#endif	

/* To prevent a warning if -Wmissing-prototypes is used */
#if	__GNUC__ > 1
PROTOTYPE(main,
	  int,
	  (int,
	   char *argv[]));
#endif


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
