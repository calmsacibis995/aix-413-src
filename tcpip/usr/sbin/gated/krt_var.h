/* @(#)81	1.1  src/tcpip/usr/sbin/gated/krt_var.h, tcprouting, tcpip411, GOLD410 12/6/93 17:25:35 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: KVM_CLOSE
 *		KVM_GETERR
 *		KVM_NLIST
 *		KVM_OPENFILES
 *		KVM_OPEN_DEFINE
 *		KVM_OPEN_ERROR
 *		KVM_READ
 *		PROTOTYPE
 *		RTAX_LIST
 *		RTAX_LIST_END
 *		krt_lladdr
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
 * krt_var.h,v 1.3 1993/04/10 12:25:10 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* Kernel interface definitions */

extern const bits krt_flag_bits[];	/* Route flag bits */
extern rt_entry krt_rt;			/* For faking route changes */
extern gw_entry *krt_gwp;

#define        KRT_PACKET_MAX  1024            /* Maximum routing socket packet we expect */

/**/

#if	defined(INCLUDE_KVM) && !defined(KVM_TYPE_NONE)
/* KVM stuff */
#ifdef	KVM_TYPE_RENO
#define	KVM_OPENFILES(nl, core, swap, flags, buf)	((kvm_openfiles(nl, core, swap) < 0) ? NULL : TRUE)
#define	KVM_OPEN_DEFINE(buf)	
#define	KVM_OPEN_ERROR(buf)	kvm_geterr()
#define	KVM_GETERR(kd, string)		kvm_geterr()
#endif	/* KVM_TYPE_RENO */

#ifdef	KVM_TYPE_BSD44
#define	KVM_OPENFILES(nl, core, swap, flags, buf)	kvm_openfiles(nl, core, swap, flags, buf)
#define	KVM_OPEN_DEFINE(buf)	char buf[LINE_MAX]
#define	KVM_OPEN_ERROR(buf)	buf
#define	KVM_WITH_KD
#define	KVM_GETERR(kd, string)		kvm_geterr(kd)
#ifndef	INCLUDE_KVM_WORKS
#define	INCLUDE_KVM_WORKS
#endif
#endif	/* KVM_TYPE_BSD44 */

#ifdef	KVM_TYPE_SUNOS4
#define	KVM_OPENFILES(nl, core, swap, flags, buf)	kvm_open(nl, core, swap, flags, "kvm")
#define	KVM_OPEN_DEFINE(buf)
#define	KVM_OPEN_ERROR(buf)	"kvm_open error"
#define	KVM_WITH_KD
#define	KVM_GETERR(kd, string)		string
#ifndef	INCLUDE_KVM_WORKS
#define	INCLUDE_KVM_WORKS
#endif
#endif	/* KVM_OPEN_SUNOS */

#ifdef	KVM_TYPE_OTHER
#define	KVM_OPENFILES(nl, core, swap, flags, buf)	kvm_openfiles(nl, core, swap, flags, buf)
#define	KVM_OPEN_DEFINE(buf)	char buf[LINE_MAX]
#define	KVM_OPEN_ERROR(buf)	buf
#define	KVM_WITH_KD
#define	KVM_GETERR(kd, string)		kvm_geterr(kd)
typedef struct __kvm kvm_t;

PROTOTYPE(kvm_openfiles,
	  extern kvm_t *,
	  (char *,
	   char *,
	   char *,
	   int,
	   char *));
#ifdef	INCLUDE_NLIST
PROTOTYPE(kvm_nlist,
	  extern int,
	  (kvm_t *,
	   struct nlist *));
#endif	/* INCLUDE_NLIST */
PROTOTYPE(kvm_read,
	  extern int,
	  (kvm_t *,
	   unsigned long,
	   caddr_t,
	   size_t));
PROTOTYPE(kvm_write,
	  extern int,
	  (kvm_t *,
	   off_t,
	   caddr_t,
	   int));
PROTOTYPE(kvm_close,
	  extern int,
	  (kvm_t *));
PROTOTYPE(kvm_geterr,
	  extern char *,
	  (kvm_t *));
#endif	/* KVM_TYPE_OTHER */

#ifdef	INCLUDE_KVM_WORKS
#include <kvm.h>
#endif

#ifdef	KVM_WITH_KD
extern kvm_t	*kd;

#define	KVM_NLIST(kd, nl)		kvm_nlist(kd, nl)
#define	KVM_READ(kd, addr, buf, nbytes)	kvm_read(kd, addr, buf, nbytes)
#define	KVM_CLOSE(kd)			kvm_close(kd)
#else	/* KVM_WITH_KD */
extern int kd;

#define	KVM_NLIST(kd, nl)		kvm_nlist(nl)
#define	KVM_READ(kd, addr, buf, nbytes)	kvm_read(addr, buf, nbytes)
#define	KVM_CLOSE(kd)			kvm_close()
#endif	/* KVM_WITH_KD */

#endif	/* INCLUDE_KVM && !KVM_TYPE_NONE */

/**/
/* Return codes from krt_addrcheck */

#define	KRT_ADDR_OK	1	/* Address is OK */
#define	KRT_ADDR_IGNORE	0	/* Ignore this address */
#define	KRT_ADDR_BOGUS	-1	/* Bogus, delete it */
#define	KRT_ADDR_MC	-2	/* Multicast default specification */

/**/
/* Routing table scanning routines */

#ifdef	KRT_RTREAD_KMEM

extern struct nlist *krt_rthashsize;
extern struct nlist *krt_rthash[2];
#define	KRT_RTHOST	0
#define	KRT_RTNET	1
#endif	/* KRT_RTREAD_KMEM */

#ifdef	KRT_RTREAD_RADIX
extern struct nlist *krt_radix_head;
#endif	/* KRT_RTREAD_RADIX */

/**/
/* Routing table manipulation routines */

#ifdef	KRT_RT_IOCTL
#define	RTM_ADD		SIOCADDRT
#define	RTM_DELETE	SIOCDELRT
#endif	/* KRT_RT_IOCTL */

/**/
/* Symbols */

PROTOTYPE(krt_symbols,
	  extern int,
	  (void));
#ifdef	SIOCGNETOPT
PROTOTYPE(krt_netopts,
	  extern int,
	  (void));
#endif	/* SIOCGNETOPT */

/**/

/* Routing socket */

PROTOTYPE(krt_recv,
	  extern void,
	  (task *));
PROTOTYPE(krt_timeout,
	  extern void,
	  (timer *,
	   time_t));
PROTOTYPE(krt_rtsock_dump,
	  extern void,
	  (task *,
	   FILE *));

/**/

/* LL addr */

#ifdef	KRT_LLADDR_NONE
#define	krt_lladdr(ifr)	((sockaddr_un *) 0)
#else	/* KRT_LLADDR_NONE */
#ifdef	KRT_LLADDR_KMEM
extern struct nlist *krt_ifnet;
#endif

PROTOTYPE(krt_lladdr,
	  extern sockaddr_un *,
	  (struct ifreq *));
#endif	/* KRT_LLADDR_NONE */

/**/

/* Routing socket and kerninfo support */

#if	defined(KRT_RT_SOCK) && defined(INCLUDE_ROUTE)
/*
 *	Support code for use with the getkerninfo() call and the routing socket.
 */

#ifndef	RTAX_DST
/*
 * Index offsets for sockaddr array for alternate internal encoding.
 */
#define RTAX_DST	0	/* destination sockaddr present */
#define RTAX_GATEWAY	1	/* gateway sockaddr present */
#define RTAX_NETMASK	2	/* netmask sockaddr present */
#define RTAX_GENMASK	3	/* cloning mask sockaddr present */
#define RTAX_IFP	4	/* interface name sockaddr present */
#define RTAX_IFA	5	/* interface addr sockaddr present */
#define RTAX_AUTHOR	6	/* sockaddr for author of redirect */
#define	RTAX_BRD	7	/* broadcast address */
#define RTAX_MAX	8	/* size of array to allocate */
#endif	/* RTAX_DST */

typedef struct {
	flag_t rti_addrs;
	sockaddr_un *rti_info[RTAX_MAX];
} krt_addrinfo;

#define	RTAX_LIST(i)	for (i = 0; i < RTAX_MAX; i++)
#define	RTAX_LIST_END(i)

#define	RTM_ADDR(ap)	ap = (struct sockaddr *) \
    ((caddr_t) ap + (ap->sa_len ? ROUNDUP(ap->sa_len, sizeof (u_long)) : sizeof(u_long)))


extern const bits rtm_type_bits[];


PROTOTYPE(krt_xaddrs,
	  extern krt_addrinfo *,
	  (register struct rt_msghdr *,
	   size_t));
PROTOTYPE(krt_trace,
	  void,
	  (task *,
	   const char *,
	   struct rt_msghdr *,
	   size_t,
	   krt_addrinfo *,
	   int));
PROTOTYPE(krt_trace_disp,
	  void,
	  (const char *,
	   krt_addrinfo *,
	   flag_t,
	   int));
#ifdef	KRT_IFREAD_KINFO
PROTOTYPE(krt_ifaddr,
	  extern void,
	  (task *,
	   struct ifa_msghdr *,
	   krt_addrinfo *,
	   if_link *));
#endif	/* KRT_IFREAD_KINFO */
#ifdef	KRT_RTREAD_KINFO
PROTOTYPE(krt_rtaddrs,
	  int,
	  (krt_addrinfo *,
	   rt_parms *,
	   sockaddr_un **,
	   flag_t));
#endif	/* KRT_RTREAD_KINFO */
#endif	/* KRT_RT_SOCK && INCLUDE_ROUTE */

/**/

/* General prototypes */


#ifdef	IP_MULTICAST
PROTOTYPE(krt_multicast_install,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *));
#ifdef	RTM_CHANGE
PROTOTYPE(krt_multicast_change,
	  extern void,
	  (int,
	   rt_parms *));
#endif	/* RTM_CHANGE */
PROTOTYPE(krt_multicast_dump,
	  extern void,
	  (task *,
	   FILE *));
#endif	/* IP_MULTICAST */
#ifdef	IFT_OTHER
PROTOTYPE(krt_type_to_ll,
	  extern int,
	  (int));
#endif	/* IFT_OTHER */
PROTOTYPE(krt_rtread,
	  extern int,
	  (void));
PROTOTYPE(krt_rtadd,
	  extern int,
	  (rt_parms *,
	   short));
PROTOTYPE(krt_addrcheck,
	  extern int,
	  (rt_parms *));
PROTOTYPE(krt_if_flags,
	  extern flag_t,
	  (int));
PROTOTYPE(krt_flags_to_state,
	  extern flag_t,
	  (flag_t));
PROTOTYPE(krt_request,
	  extern int,
	  (u_int,
	   rt_entry *,
	   sockaddr_un *));
PROTOTYPE(krt_change,
	  int,
	  (rt_entry *,
	   sockaddr_un *,
	   rt_entry *));

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
