/* @(#)03	1.1  src/tcpip/usr/sbin/gated/ospf_rtab.h, tcprouting, tcpip411, GOLD410 12/6/93 17:26:31 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: INTER_ACTIVE
 *		NH_LIST
 *		NH_LIST_END
 *		PROTOTYPE
 *		PTYPE_BIT
 *		RRT_ADVRTR
 *		RRT_AREA
 *		RRT_CHANGE
 *		RRT_COST
 *		RRT_DEST
 *		RRT_DTYPE
 *		RRT_ETYPE
 *		RRT_INFO
 *		RRT_IO_NDX
 *		RRT_LAST
 *		RRT_MASK
 *		RRT_NEXT
 *		RRT_NH
 *		RRT_NH_NDX
 *		RRT_PTYPE
 *		RRT_REV
 *		RRT_TYPE2COST
 *		RRT_V
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
 * ospf_rtab.h,v 1.12 1992/11/30 20:55:08 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#ifndef RTAB_H
#define RTAB_H

/* setable maximum number of equal-cost parents - LSDB sb <= 32 bytes */
#define MAXNH		RT_N_MULTIPATH


/*
 * For accessing router route information
 */

#define RRT_INFO(R)	((R)->ospf_rt_info)
#define RRT_DEST(R) 	((R)->dest)
#define RRT_MASK(R) 	((R)->mask)
#define RRT_DTYPE(R) 	((R)->ospf_rt_info.dtype)
#define RRT_ETYPE(R) 	((R)->ospf_rt_info.etype)
#define RRT_CHANGE(R) 	((R)->ospf_rt_info.change)
#define RRT_PTYPE(R) 	((R)->ospf_rt_info.ptype)
#define RRT_REV(R) 	((R)->ospf_rt_info.revision)
#define RRT_AREA(R) 	((R)->ospf_rt_info.area)
#define RRT_COST(R)	((R)->ospf_rt_info.cost)
#define RRT_TYPE2COST(R) ((R)->ospf_rt_info.type2cost)
#define RRT_NH(R) 	((R)->ospf_rt_info.nh_ndx[0]->nh_addr)
#define RRT_IO_NDX(R) 	((R)->ospf_rt_info.nh_ndx[0]->nh_ifap)
#define RRT_NH_NDX(R) 	((R)->ospf_rt_info.nh_ndx[0])
#define RRT_ADVRTR(R) 	((R)->ospf_rt_info.advrtr)
#define RRT_V(R) 	((R)->ospf_rt_info.v)
#define RRT_NEXT(R) 	((R)->ptr[NEXT])
#define RRT_LAST(R) 	((R)->ptr[LAST])


typedef struct ospf_rt_info {
    struct NH_BLOCK *nh_ndx[MAXNH];	/* Better all around if an odd number like 3 */
    u_int8 nh_cnt;
    u_int8 etype;		/* external metric type */
    u_int8 dtype;		/* destination type */
    u_int8 change;		/* this entry has been changed flag */
    u_int8 ptype;		/* LS_RTR and LS_NET are internal, LS_SUM
			           are inter area and LS_ASE are external */
    struct LSDB *v;		/* vertex */
    struct AREA *area;		/* associatiated area */
    u_int32 cost;
    u_int32 type2cost;
    u_int32 advrtr;
    u_int32 revision;
} OSPF_RT_INFO;


#define ORT_INFO_NULL 	((OSPF_RT_INFO *) 0)

/*
 * Type router entries - Area border, AS border and AS border summary
 */
struct OSPF_ROUTE {
    struct OSPF_ROUTE *ptr[2];
    u_int32 dest;
    u_int32 mask;
    OSPF_RT_INFO ospf_rt_info;
};

/*
 * Cache for forwarding address
 */
#define FORWARD_CACHE_SIZE 5


/*
 * OSPF next-hop list
 */
#define NH_DIRECT		1
#define NH_NBR			2
#define NH_DIRECT_FORWARD	3
#define NH_VIRTUAL		4
#define	NH_LOCAL		5

#define MAXNHCOUNT	256	/* Maximum number of unique next hops */

/* May want to include link address here... */
struct NH_BLOCK {
    struct NH_BLOCK *nh_forw;
    struct NH_BLOCK *nh_back;
    u_short	nh_type;	/* type of next hop */
    u_short	nh_refcount;	/* Number of references */
    if_addr *nh_ifap;		/* pointer to interface */
#define	NH_NET(nhp)	sock2ip((nhp)->nh_ifap->ifa_subnet)
    u_int32 nh_addr;		/* next hop ip address */
};

#define	NH_LIST(np)	for (np = ospf.nh_list.nh_forw; np != &ospf.nh_list; np = np->nh_forw)
#define	NH_LIST_END(np)

/* Destination types */
#define DTYPE_NET  	0
#define DTYPE_ASBR 	1	/* autonomous system border rtr */
#define DTYPE_ABR  	2	/* area border rtr */
#define DTYPE_VIRT 	3
#define DTYPE_ASE  	4

/* Change types */
#define E_UNCHANGE              0
#define E_NEW                   1       /* New route */
#define E_NEXTHOP               2       /* Next hop has changed */
#define E_METRIC                3       /* Metric has changed */
#define E_WAS_INTER_NOW_INTRA   4       /* Was sum now intra */
#define E_WAS_INTRA_NOW_INTER   5       /* Was intra now sum */
#define E_ASE_METRIC            6       /* ASE metric has changed */
#define E_WAS_ASE               7       /* Was ase now internal */
#define E_WAS_INTRA_NOW_ASE     8       /* Was intra now ase */
#define E_WAS_INTER_NOW_ASE     9       /* Was inter now ase */
#define E_ASE_TYPE              10      /* From type 2 to type 1 or visa versa*/
#define E_ASE_TAG               11      /* Tag has changed */
#define E_DELETE                12

/* Path types */
#define PTYPE_ANY    	0x3F	/* internal and external routes */
#define PTYPE_INTRA  	0x7	/* intra route */
#define PTYPE_INTER  	0x18	/* inter route */
#define PTYPE_INT    	0x1F	/* intra or inter routes */
#define	PTYPE_INTNET	0x0f	/* inter area network routes */
#define PTYPE_EXT    	0x20	/* external routes */
#define PTYPE_LEAVES 	0x38	/* inter or ext */
#define PTYPE_BIT(T) 	(1 << (T))	/* T is lsa type */

extern struct OSPF_ROUTE sum_asb_rtab;		/* summary as bdr rtr table */

/*
 * Check to see if the route is an active inter area route
 * - pass the level that has just been run, the area that
 *   is running the spf algorithm, and the route
 */
#define  INTER_ACTIVE(FROM,A,R)\
	   ( (PTYPE_BIT(ORT_PTYPE(R)) & PTYPE_INTER) &&\
	     ( (!((FROM) & PTYPE_INTER)) ||\
	       ( (((FROM) & PTYPE_INTER) &&\
	         ((ORT_REV(R) == RTAB_REV))) ||\
	         (ORT_AREA(R) != (A)) ) ) )

/* Function prototypes */
PROTOTYPE(addroute,
	  extern int,
	  (struct AREA *,
	   struct LSDB *,
	   int,
	   struct AREA *));
PROTOTYPE(rtr_findroute,
	  extern struct OSPF_ROUTE *,
	  (struct AREA *,
	   u_int32,
	   int,
	   int));
PROTOTYPE(ospf_int_active,
	  extern int,
	  (int,
	   struct AREA *,
	   rt_entry *));

#endif	/* RTAB_H */


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
 * ------------------------------------------------------------------------
 * 
 *                 U   U M   M DDDD     OOOOO SSSSS PPPPP FFFFF
 *                 U   U MM MM D   D    O   O S     P   P F
 *                 U   U M M M D   D    O   O  SSS  PPPPP FFFF
 *                 U   U M M M D   D    O   O     S P     F
 *                  UUU  M M M DDDD     OOOOO SSSSS P     F
 * 
 *     		          Copyright 1989, 1990, 1991
 *     	       The University of Maryland, College Park, Maryland.
 * 
 * 			    All Rights Reserved
 * 
 *      The University of Maryland College Park ("UMCP") is the owner of all
 *      right, title and interest in and to UMD OSPF (the "Software").
 *      Permission to use, copy and modify the Software and its documentation
 *      solely for non-commercial purposes is granted subject to the following
 *      terms and conditions:
 * 
 *      1. This copyright notice and these terms shall appear in all copies
 * 	 of the Software and its supporting documentation.
 * 
 *      2. The Software shall not be distributed, sold or used in any way in
 * 	 a commercial product, without UMCP's prior written consent.
 * 
 *      3. The origin of this software may not be misrepresented, either by
 *         explicit claim or by omission.
 * 
 *      4. Modified or altered versions must be plainly marked as such, and
 * 	 must not be misrepresented as being the original software.
 * 
 *      5. The Software is provided "AS IS". User acknowledges that the
 *         Software has been developed for research purposes only. User
 * 	 agrees that use of the Software is at user's own risk. UMCP
 * 	 disclaims all warrenties, express and implied, including but
 * 	 not limited to, the implied warranties of merchantability, and
 * 	 fitness for a particular purpose.
 * 
 *     Royalty-free licenses to redistribute UMD OSPF are available from
 *     The University Of Maryland, College Park.
 *       For details contact:
 * 	        Office of Technology Liaison
 * 		4312 Knox Road
 * 		University Of Maryland
 * 		College Park, Maryland 20742
 * 		     (301) 405-4209
 * 		FAX: (301) 314-9871
 * 
 *     This software was written by Rob Coltun
 *      rcoltun@ni.umd.edu
 */
