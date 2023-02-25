/* @(#)39	1.5  src/tcpip/usr/sbin/gated/defs.h, tcprouting, tcpip411, GOLD410 12/6/93 17:36:48 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ABS
 *		BIT
 *		BIT_COMPARE
 *		BIT_FLIP
 *		BIT_MASK_MATCH
 *		BIT_MATCH
 *		BIT_RESET
 *		BIT_SET
 *		BIT_TEST
 *		MAX
 *		MIN
 *		PROTOTYPE
 *		ROUNDUP
 *		assert
 *		defined
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
 * defs.h,v 1.16.2.1 1993/08/27 22:28:08 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/* defs.h
 *
 * Compiler switches and miscellaneous definitions.
 */

#if	!defined(__STDC__) && !defined(const)
#define	const
#endif	/* !defined(__STDC__) && !defined(const) */

#if	!(defined(__GNUC__) && defined(__STDC__))
#define	inline
#endif	/* __GNUC__ */

/* Common types */

/* More generic types */
typedef U_INT8	u_int8;		/* 8 bits unsigned */
typedef	S_INT8	s_int8;		/* 8 bits signed */
typedef U_INT16	u_int16;	/* 16 bits unsigned */
typedef	S_INT16	s_int16;	/* 16 bits signed */
typedef U_INT32	u_int32;	/* 32 bits unsigned */
typedef	S_INT32	s_int32;	/* 32 bits signed */

#ifndef	_PSAP_
/* Define byte if ISODE has not already */
typedef	u_char byte;
#endif	/* _PSAP_ */
typedef u_int16 as_t;		/* 16 bits unsigned */
typedef s_int32 pref_t;		/* 32 bits signed */
typedef u_int32 flag_t;		/* 32 bits unsigned */
typedef u_int16 proto_t;	/* 16 bits unsigned */
typedef u_int16 mtu_t;		/* 16 bits unsigned */
typedef u_int32 metric_t;	/* 32 bits unsigned */
typedef	u_int32 tag_t;		/* 32 bits unsigned */
#ifdef	notdef
typedef int asmatch_t;		/* Temporary for now */
#endif

typedef	struct _if_link if_link;
typedef struct _if_info if_info;
typedef	struct _if_addr if_addr;
typedef struct _if_addr_entry if_addr_entry;
typedef struct _rt_head rt_head;
typedef struct _rt_entry rt_entry;
typedef struct _rt_list rt_list;
typedef struct _task task;
typedef struct _timer timer;
typedef struct _adv_entry adv_entry;
typedef struct task_block *block_t;

/* Gated uses it's own version of *printf */
#define	fprintf	gd_fprintf
#define	sprintf	gd_sprintf
#define	vsprintf	gd_vsprintf

/* Definitions for insque() and remque() */
struct qelem {
    struct qelem *q_forw;
    struct qelem *q_back;
    char data[1];
};


extern const char *gated_version;
extern const char *build_date;

/* general definitions for GATED user process */

#ifndef	TRUE
#define TRUE	 1
#define FALSE	 0
#endif	/* TRUE */

#ifndef NULL
#define NULL	 0
#endif

#define MAXHOSTNAMELENGTH 64		/*used in init_egpngh & rt_dumb_init*/

#undef  MAXPACKETSIZE

#ifndef	MIN
#define	MIN(a, b)	((a) < (b) ? (a) : (b))
#endif	/* MIN */
#ifndef	MAX
#define	MAX(a, b)	((a) > (b) ? (a) : (b))
#endif	/* MAX */
#ifndef	ABS
#define	ABS(a)		((a) < 0 ? -(a) : (a))
#endif	/* ABS */

#ifdef	__STDC__
#define	BIT(b)	b ## ul
#else	/* __STDC__ */
#define	BIT(b)	b
#endif	/* __STDC__ */
#define	BIT_SET(f, b)	((f) |= b)
#define	BIT_RESET(f, b)	((f) &= ~(b))
#define	BIT_FLIP(f, b)	((f) ^= (b))
#define	BIT_TEST(f, b)	((f) & (b))
#define	BIT_MATCH(f, b)	(((f) & (b)) == (b))
#define	BIT_COMPARE(f, b1, b2)	(((f) & (b1)) == b2)
#define	BIT_MASK_MATCH(f, g, b)	(!(((f) ^ (g)) & (b)))

#define ROUNDUP(a, size) (1 + (((a) - 1) | ((size) - 1)))

/* Error message defines */

#ifndef	errno
extern int errno;
#endif	/* errno */

/*
 *	Definitions of descriptions of bits
 */

typedef struct {
    u_int t_bits;
    const char *t_name;
} bits;


/* Our version of assert */
#define assert(ex) \
{ \
    if (!(ex)) { \
	task_assert(__FILE__, __LINE__); \
    } \
}


/*
 *	Routines defined in checksum.c
 */
PROTOTYPE(inet_cksumv,
	  extern u_int16,
	  (struct iovec *v,
	   int,
	   size_t));
PROTOTYPE(inet_cksum,
	  extern u_int16,
	  (void_t,
	   size_t));
#ifdef	FLETCHER_CHECKSUM
PROTOTYPE(iso_cksum,
	  extern u_int32,
	  (void_t,
	   size_t,
	   byte *));
#endif	/* FLETCHER_CHECKSUM */


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
