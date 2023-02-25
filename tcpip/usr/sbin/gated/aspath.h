/* @(#)63	1.1  src/tcpip/usr/sbin/gated/aspath.h, tcprouting, tcpip411, GOLD410 12/6/93 17:24:46 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: GET_PATH_ATTR
 *		NFA_AND
 *		NFA_ASG
 *		NFA_CLR
 *		NFA_ISSET
 *		NFA_NONZERO
 *		NFA_OR
 *		NFA_SET
 *		NFA_SHIFT
 *		NFA_ZERO
 *		PATH_ATTR_LEN
 *		PATH_ATTR_MINLEN
 *		PATH_ATTR_PTR
 *		PATH_ATTR_SKIP_LEN
 *		PATH_OSPF_ISCOMPLETE
 *		PATH_OSPF_ISTRUSTED
 *		PATH_PTR
 *		PATH_PUT_AS
 *		PATH_PUT_ATTR
 *		PATH_PUT_METRIC
 *		PATH_PUT_NEXTHOP
 *		PATH_SAME
 *		PATH_SHORT_PTR
 *		PROTOTYPE
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
 * aspath.h,v 1.11.2.1 1993/08/28 01:59:53 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */

/*
 * Path attributes are currently used by BGP, but are maintained
 * separately to allow other protocols which may carry the same
 * information to share this data.  In here we record AS path
 * and Origin information as well as unrecognized optional
 * transitive attributes.
 *
 * To reduce the workload on malloc() and free() the variable
 * length data is stored in a fixed length data area, with 32
 * and 128 byte areas being maintained.  Longer data areas are
 * malloc()'d and free()'d as needed.
 *
 * All path attributes are sorted in ascending order by attribute
 * type code.  This allows the use of bcmp() to compare types.
 */

/*
 * Fixed length attribute block.
 */
typedef struct _as_path {
    struct _as_path *path_next;		/* pointer to next in chain */
    u_long path_id;			/* path ID, for pretty printing */
    u_short path_refcount;		/* reference count for this path */
    byte path_size;			/* index into size list, 0 if malloc */
    byte path_origin;		/* path origin type (BGP, IGP, XX) */
    as_t path_as;			/* AS of the recipient of the path */
    u_short path_len;		/* length of the AS path */
    u_short path_attr_len;		/* length of unrecognized data */
} as_path;

/*
 * Origin codes (these should match the BGP codes!)
 */
#define	PATH_ORG_IGP		0	/* route learned from IGP */
#define	PATH_ORG_EGP		1	/* route learned from EGP */
#define	PATH_ORG_XX		2	/* god only knows */

/*
 * Template for an attribute block with data

 */
typedef struct _as_path_data {
    as_path aspd_info;
    union {
    	u_short Xaspd_short_data[2];
	byte Xaspd_data[4];
    } aspd_Xdata;
#define	aspd_data	aspd_Xdata.Xaspd_data
#define	aspd_short_data	aspd_Xdata.Xaspd_short_data
} as_path_data;


/*
 * Fetch pointers to the AS path and attribute data
 */
#define	PATH_PTR(asp) \
    (&(((as_path_data *)(asp))->aspd_data[0]))
#define	PATH_SHORT_PTR(asp) \
    (&(((as_path_data *)(asp))->aspd_short_data[0]))
#define	PATH_ATTR_PTR(asp) \
    (&(((as_path_data *)(asp))->aspd_data[(asp)->path_len]))

/*
 * Compare path attributes to see if they are the same
 */
#define	PATH_SAME(asp1, asp2) \
    (  ((asp1)->path_origin == (asp2)->path_origin) \
     && ((asp1)->path_len == (asp2)->path_len) \
     && ((asp1)->path_attr_len == (asp2)->path_attr_len) \
     && ((asp1)->path_as == (asp2)->path_as) \
     && ((((asp1)->path_len + (asp1)->path_attr_len) == 0) \
     || (bcmp((caddr_t) PATH_PTR((asp1)), (caddr_t) PATH_PTR((asp2)), \
	      (size_t)((asp1)->path_len+(asp1)->path_attr_len)) == 0)))

/*
 * A structure to hold non-transitive AS path info which is of interest.
 * This is used for passing such info back and forth when encoding or
 * decoding AS paths.
 */
typedef struct _as_path_info {
    flag_t api_flags;
    sockaddr_un *api_nexthop;
    metric_t api_metric;
} as_path_info;

/*
 * Flag definitions for the above.
 */
#define	APIF_UNREACH		BIT(0x1)	/* unreachable attribute present */
#define	APIF_NEXTHOP		BIT(0x2)	/* next hop present */
#define	APIF_METRIC		BIT(0x4)	/* metric present */

#define	APIF_INTERNAL		BIT(0x100)	/* internal BGP AS path discipline */


/*
 * Definitions for decoding path attributes.  These come from
 * the BGP protocol definition, RFC1163.
 *
 * Each attribute consists of a flag byte, followed by an attribute
 * type code, followed by a one- or two-byte length, followed by
 * the data.
 */

/*
 * Bit definitions for the attribute flags byte
 */
#define	PA_FLAG_OPT	BIT(0x80)	/* attribute is optional */
#define	PA_FLAG_TRANS	BIT(0x40)	/* attribute is transitive */
#define	PA_FLAG_PARTIAL	BIT(0x20)	/* incomplete optional, transitive attribute */
#define	PA_FLAG_EXTLEN	BIT(0x10)	/* extended length flag */

#define	PA_FLAG_ALL  (PA_FLAG_OPT|PA_FLAG_TRANS|PA_FLAG_PARTIAL|PA_FLAG_EXTLEN)


/*
 * Attribute type codes we know about
 */
#define	PA_TYPE_INVALID		0
#define	PA_TYPE_ORIGIN		1
#define	PA_TYPE_ASPATH		2
#define	PA_TYPE_NEXTHOP		3
#define	PA_TYPE_UNREACH		4
#define	PA_TYPE_METRIC		5
#define	PA_MAXTYPE		5	/* highest known type code */

/*
 * Lengths for a few of the attributes (the fixed length ones)
 */
#define	PA_LEN_ORIGIN		1
#define	PA_LEN_NEXTHOP		4
#define	PA_LEN_UNREACH		0
#define	PA_LEN_METRIC		2

/*
 * Path error codes.  These are essentially the UPDATE subcodes from
 * the BGP spec.  BGP knows this.
 */
#define	PA_ERR_MALFORMED	1	/* Malformed attribute list */
#define	PA_ERR_UNKNOWN		2	/* Unknown well-known attribute */
#define	PA_ERR_MISSING		3	/* Missing well-known attribute */
#define	PA_ERR_FLAGS		4	/* Flags in error */
#define	PA_ERR_LENGTH		5	/* Goofy length */
#define	PA_ERR_ORIGIN		6	/* Unrecognized ORIGIN attr */
#define	PA_ERR_ASLOOP		7	/* AS appeared twice in path */
#define	PA_ERR_NEXTHOP		8	/* NEXTHOP screwed up */
#define	PA_ERR_OPTION		9	/* optional attribute error */
#define	PA_ERR_NETWORK		10	/* network field screwed */

/*
 * Macro for retrieving attribute information from a byte stream
 */
#define	GET_PATH_ATTR(flags, code, len, cp) \
    do { \
        register unsigned tmp; \
        tmp = *(cp)++; \
        (flags) = (tmp & ~PA_FLAG_EXTLEN); \
        (code) = *(cp)++; \
        if (tmp & PA_FLAG_EXTLEN) { \
	    tmp = (*(cp)++) << 8; \
	    tmp |= *(cp)++; \
	    (len) = tmp; \
	} else { \
	     (len) = *(cp)++; \
	} \
    } while (0)

/*
 * Macro for determining the full length of an attribute given
 * the length of the data portion.
 */
#define	PATH_ATTR_LEN(len)	(((len) > 255) ? ((len)+4) : ((len)+3))

/*
 * Macro for determining the minimum length of an attribute given
 * the flags field.
 */
#define	PATH_ATTR_MINLEN(flags)	(((flags) & PA_FLAG_EXTLEN) ? 4 : 3)

/*
 * Macro for skipping over total attributes length field.
 */
#define	PATH_ATTR_TOTAL_LEN_SIZE	(2)
#define	PATH_ATTR_SKIP_LEN(cp)	((cp) += PATH_ATTR_TOTAL_LEN_SIZE)

/*
 * Macro for writing an AS into the message.
 */
#define	PATH_ATTR_AS_SIZE	(2)
#define	PATH_PUT_AS(as, cp) \
    do { \
	register u_short tmp = (u_short)(as); \
	*(cp)++ = tmp >> 8; \
	*(cp)++ = tmp & 0xff; \
    } while (0)

/*
 * Writing a metric into the message
 */
#define	PATH_PUT_METRIC(metric, cp) \
    do { \
	register u_short tmp = (u_short)(metric); \
	*(cp)++ = tmp >> 8; \
	*(cp)++ = tmp & 0xff; \
    } while (0)

/*
 * Writing a next hop into the message.
 */
#define	PATH_PUT_NEXTHOP(nexthop, cp) \
    do { \
	register byte *hp = (byte *)&sock2ip(nexthop); \
	*(cp)++ = *hp++; \
	*(cp)++ = *hp++; \
	*(cp)++ = *hp++; \
	*(cp)++ = *hp++; \
    } while (0);

/*
 * Macro for inserting a path attribute header into a buffer.  The
 * extended length bit is set in the flag as appropriate.
 */
#define	PATH_PUT_ATTR(flag, code, len, cp) \
    do { \
        register unsigned tmp; \
        tmp = (len); \
        if (tmp > 255) { \
	    *(cp)++ = (byte)((flag) | PA_FLAG_EXTLEN); \
	    *(cp)++ = (byte)(code); \
	    *(cp)++ = (byte) (tmp >> 8); \
	    *(cp)++ = (byte) tmp; \
	} else { \
	    *(cp)++ = (byte)((flag) & ~PA_FLAG_EXTLEN); \
	    *(cp)++ = (byte)(code); \
	    *(cp)++ = (byte) tmp; \
	} \
    } while (0)

/*
 * Protocol-specific attribute-related processing
 */

/*
 * OSPF uses a tag format which can be translated as path attributes.
 * We maintain a knowlege of this here since this is about the only
 * place which needs to understand this.
 */
#define	PATH_OSPF_TAG_TRUSTED	BIT(0x80000000)	/* tag set to standard format */
#define	PATH_OSPF_TAG_COMPLETE	BIT(0x40000000)	/* attributes are complete */

#define	PATH_OSPF_TAG_LEN_MASK	BIT(0x30000000)	/* mask for the path length */
#define	PATH_OSPF_TAG_LEN_0	  BIT(0x00000000)	/* zero length path */
#define	PATH_OSPF_TAG_LEN_1	  BIT(0x10000000)	/* path length 1 */
#define	PATH_OSPF_TAG_LEN_2	  BIT(0x20000000)	/* path length 2 */
#define	PATH_OSPF_TAG_LEN_X	  BIT(0x30000000)	/* invalid */

#define	PATH_OSPF_TAG_USR_MASK	BIT(0x0fff0000)	/* arbitrary user bits */
#define	PATH_OSPF_TAG_USR_SHIFT	16		/* shift to normalize user */
#define	PATH_OSPF_TAG_AS_MASK	BIT(0x0000ffff)	/* mask for AS */
#define	PATH_OSPF_TAG_USR_LIMIT	0, (PATH_OSPF_TAG_USR_MASK >> PATH_OSPF_TAG_USR_SHIFT)
#define	PATH_OSPF_TAG_LIMIT	0, ~PATH_OSPF_TAG_TRUSTED

#define PATH_OSPF_ISTRUSTED(tag)	(((tag) & PATH_OSPF_TAG_TRUSTED) != 0)
#define	PATH_OSPF_ISCOMPLETE(tag)	(((tag) & PATH_OSPF_TAG_COMPLETE) != 0)


/*
 * Entries into the path module
 */
PROTOTYPE(aspath_init,
	  extern void,
	  (void));
PROTOTYPE(aspath_dump,
	  extern void,
	  (FILE *,
	   as_path *,
	   const char *));
PROTOTYPE(aspath_trace,
	  extern void,
	  (flag_t,
	   const char *,
	   byte *,
	   int));
PROTOTYPE(aspath_alloc,
	  extern as_path *,
	  (int));
PROTOTYPE(aspath_insert,
	  extern as_path *,
	  (as_path *));
PROTOTYPE(aspath_rt_free,
	  extern void,
	  (rt_entry *));
PROTOTYPE(aspath_unlink,
	  extern void,
	  (as_path *));
PROTOTYPE(aspath_rt_build,
	  extern void,
	  (rt_entry *,
	   as_path *));
PROTOTYPE(aspath_attr, extern as_path *,
	 (byte *,
	  int,
	  as_t,
	  as_path_info *,
	  int *,
	  byte **));
#ifdef	PROTO_OSPF
PROTOTYPE(aspath_tag_ospf,
	  extern u_int32,
	  (as_t,
	   rt_entry *,
	   metric_t));
PROTOTYPE(aspath_tag_dump,
	  extern char *,
	  (as_t,
	   u_long));
#endif	/* PROTO_OSPF */
#ifdef	PROTO_BGP
PROTOTYPE(aspath_adv_ibgp,
	  extern int,
	  (as_t,
	   proto_t,
	   rt_entry *));
#endif	/* PROTO_BGP */
PROTOTYPE(aspath_free,
	  void,
	  (as_path *));
PROTOTYPE(aspath_find,
	  as_path *,
	  (as_path *));
PROTOTYPE(aspath_format,
	  byte *,
	 (as_t,
	  as_path *,
	  as_path_info *,
	  byte **,
	  byte *));
PROTOTYPE(aspath_prefer,
	  int,
	  (rt_entry *,
	   rt_entry *));

/* Tag support */
PROTOTYPE(tag_rt,
	  extern tag_t,
	  (rt_entry *));

/* Routines to handle control information */
PROTOTYPE(aspath_adv_match,
	  extern int,
	  (void_t,
	   as_path *));
PROTOTYPE(aspath_adv_compare,
	  extern int,
	  (void_t,
	   void_t));
PROTOTYPE(aspath_adv_print,
	  extern char *,
	  (void_t));
PROTOTYPE(aspath_adv_free,
	  extern void,
	  (void_t));
PROTOTYPE(aspath_adv_origins,
	  extern char *,
	  (flag_t));

#define	ASP_WORD	(sizeof(flag_t) * NBBY)		/* bytes in a word */
#define	ASP_BITS	30				/* max state #, beginning from 0 */
#define	ASP_ACC		(ASP_BITS + 1)			/* accept state */

typedef	struct {
    as_t	as;
    flag_t	next[1 + ASP_ACC / ASP_WORD];
} asp_table;

typedef	struct {
    flag_t	origin_mask;
    flag_t	start[1 + ASP_ACC / ASP_WORD];
    asp_table	nfa[ASP_ACC];
} asmatch_t;

extern block_t aspath_match_index;

#define	NFA_ZERO(x)		{ \
	register int macvar = ASP_ACC / ASP_WORD; \
	do { \
	    x[macvar] = 0; \
	} while (macvar--) ; \
    }
#define	NFA_ASG(x, y)	{ \
    	register int macvar = ASP_ACC / ASP_WORD; \
	do { \
	    x[macvar] = y[macvar]; \
	} while (macvar--) ; \
    }
#define	NFA_OR(x, y, z)	{ \
    	register int macvar = ASP_ACC / ASP_WORD; \
	do { \
	    x[macvar] = y[macvar] | z[macvar]; \
	} while (macvar--) ; \
    }
#define	NFA_AND(x,y,z)	{ \
    	register int macvar = ASP_ACC / ASP_WORD; \
	do { \
	    x[macvar] = y[macvar] & z[macvar]; \
	} while (macvar--) ; \
    }
#define	NFA_NONZERO(x, i)	{ \
    	register int macvar = ASP_ACC / ASP_WORD; \
	i = FALSE; \
	do { \
	    if (x[macvar]) { \
		i = TRUE; \
		break; \
	    } \
	} while (macvar--) ; \
    }
#define	NFA_SET(x,y)	(x[(y) / ASP_WORD] |= (1 << ((y) % ASP_WORD)))
#define	NFA_CLR(x,y)	(x[(y) / ASP_WORD] &= ~(1 << ((y) % ASP_WORD)))
#define	NFA_ISSET(x,y)	(x[(y) / ASP_WORD] & (1 << ((y) % ASP_WORD)))

#define	NFA_SHIFT(x,y)	{ \
	register int macvar; \
	if (NFA_ISSET(x, ASP_ACC)) { \
	    NFA_SET(x, ASP_ACC- ((y) % ASP_ACC)); \
	} \
	for (macvar= ASP_ACC / ASP_WORD; macvar>(y) / ASP_WORD; macvar--) { \
	    x[macvar] = x[macvar-(y) / ASP_WORD]<<((y) % ASP_WORD); \
	    x[macvar]|=(x[macvar-1-(y) / ASP_WORD]>>(ASP_WORD - ((y) % ASP_WORD))); \
	} \
	x[macvar] = x[0]<<((y) % ASP_WORD); \
	while (macvar--) { \
            x[macvar] = 0; \
	} \
	x[ASP_ACC / ASP_WORD] <<= (ASP_WORD - 1 - ASP_ACC % ASP_WORD); \
	x[ASP_ACC / ASP_WORD] >>= (ASP_WORD - 1 - ASP_ACC % ASP_WORD); \
    }



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
