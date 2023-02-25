/* @(#)27	1.1  src/tcpip/usr/sbin/gated/sockaddr.h, tcprouting, tcpip411, GOLD410 12/6/93 17:27:41 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: PROTOTYPE
 *		sockcopy
 *		sockfree
 *		socksize
 *		socktype
 *		unix_socksize
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
 * sockaddr.h,v 1.20.2.1 1993/08/27 22:28:48 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


typedef union {
    /* Generic address used only for referencing length and family */
    struct {
	byte	sa_len;
	byte	sa_family;
	byte	sa_data[1];
    } a;
    /* Unix domain address */
    struct {
	byte	sun_len;
	byte	sun_family;
	char	sun_path[1];
    } un;
#ifdef	PROTO_INET
    /* IP address.  Note that sin_zero has been removed */
    struct {
	byte	sin_len;
	byte	sin_family;
	u_int16	sin_port;
	struct in_addr sin_addr;
    } in;
#endif	/* PROTO_INET */
#ifdef	SOCKADDR_DL
    struct {
	u_char	sdl_len;	/* Total length of sockaddr */
	u_char	sdl_family;	/* AF_DLI */
	u_short	sdl_index;	/* if != 0, system given index for interface */
	u_char	sdl_type;	/* interface type */
	u_char	sdl_nlen;	/* interface name length, no trailing 0 reqd. */
	u_char	sdl_alen;	/* link level address length */
	u_char	sdl_slen;	/* link layer selector length */
	char	sdl_data[1];	/* work area */
    } dl;
#endif	/* SOCKADDR_DL */
#ifdef	PROTO_ISO
    struct {
	u_char siso_len;
	u_char siso_family;
	u_char siso_addr[1];
    } iso;
#endif	/* PROTO_ISO */
    struct {
	u_char sll_len;
	u_char sll_family;
	u_char sll_type;
	u_char sll_addr[1];
    } ll;
    struct {
	byte	ss_len;
	byte	ss_family;
	char	ss_string[1];
    } s;
} sockaddr_un;

#define	AF_LL		253	/* Link level address */
#ifdef	notdef
#ifndef	AF_LINK
#define	AF_LINK		254	/* Link level interface info */
#endif	/* AF_LINK */
#endif	/* notdef */
#define	AF_STRING	255	/* String hack */

/* For compatibility with BSD 4.4 and later */
#define	socksize(x)	((x)->a.sa_len)
#define	socktype(x)	((x)->a.sa_family)
#define	sockcopy(x, y)	bcopy((caddr_t) (x), (caddr_t) (y), socksize(x))


#ifdef	SOCKET_LENGTHS
#define	unix_socksize(x)	((x)->sa_len)
#else	/* SOCKET_LENGTHS */
#define	unix_socksize(x)	(sizeof (*x))
#endif	/* SOCKET_LENGTHS */


/* Types for AF_LL */
#define	LL_OTHER	0	/* Unknown or Other */
#define	LL_SYSTEMID	1	/* ISO System ID */
#define	LL_8022		2	/* IEEE 802.2 Address */
#define	LL_X25		3	/* X.25 Address */
#define	LL_PRONET	4	/* Proteon Pronet */
#define	LL_HYPER	5	/* NSC Hyperchannel */

extern const bits ll_type_bits[];


extern block_t sock_block_index[256];

#define	sockfree(addr) \
{ \
    register block_t block_index = sock_block_index[socktype(addr)]; \
    if (block_index) { \
	task_block_free(block_index, (void_t) addr); \
    } else { \
	task_mem_free((task *) 0, (caddr_t) addr); \
    } \
}



PROTOTYPE(sockclean,
	  extern void,
	  (sockaddr_un *));
PROTOTYPE(sockaddrcmp2,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(sockaddrcmp,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(sockaddrcmp_mask,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(sockmask,
	  extern void,
	  (sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(sockishost,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(sockhostmask,
	  extern sockaddr_un *,
	  (sockaddr_un *));
PROTOTYPE(sockdup,
	  extern sockaddr_un *,
	  (sockaddr_un *));
PROTOTYPE(mask_locate,
	  extern sockaddr_un *,
	  (sockaddr_un *));
PROTOTYPE(mask_contig,
	  extern int,
	  (sockaddr_un *));
PROTOTYPE(mask_bits,
	  extern int,
	  (sockaddr_un *));
PROTOTYPE(mask_refines,
	  extern int,
	  (sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(mask_dump,
	  extern void,
	  (FILE *));
PROTOTYPE(sock2unix,
	  extern struct sockaddr *,
	  (sockaddr_un *,
	   int *));
PROTOTYPE(sock2gated,
	  extern sockaddr_un *,
	  (struct sockaddr *,
	   int));
PROTOTYPE(sockbuild_un,
	  extern sockaddr_un *,
	  (const char *));
PROTOTYPE(sockbuild_in,
	  extern sockaddr_un *,
	  (u_short,
	   u_int32));
#ifdef	PROTO_ISO
PROTOTYPE(sockbuild_iso,
	  extern sockaddr_un *,
	  (byte *,
	   size_t));
#endif	/* PROTO_ISO */
PROTOTYPE(sockbuild_str,
	  extern sockaddr_un *,
	  (const char *));
PROTOTYPE(sockbuild_byte,
	  extern sockaddr_un *,
	  (u_char *,
	   size_t));
#ifdef	SOCKADDR_DL
PROTOTYPE(sockbuild_dl,
	  extern sockaddr_un *,
	  (int,
	   int,
	   const char *,
	   size_t,
	   byte *,
	   size_t,
	   byte *,
	   size_t));
#endif	/* SOCKADDR_DL */
PROTOTYPE(sockbuild_ll,
	  extern sockaddr_un *,
	  (int,
	   byte *,
	   size_t));
PROTOTYPE(sock_init,
	  extern void,
	  (void));

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
