/* @(#)90	1.1  src/tcpip/usr/sbin/gated/ospf_gated.h, tcprouting, tcpip411, GOLD410 12/6/93 17:25:59 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: AREA_FROM_TIMER
 *		IF_INTF
 *		INTF_ADDR
 *		INTF_FROM_TIMER
 *		INTF_LCLADDR
 *		INTF_MASK
 *		INTF_MTU
 *		INTF_NET
 *		IP_LENGTH
 *		IP_PROTOCOL
 *		IS_HOST
 *		ORT_ADVRTR
 *		ORT_AREA
 *		ORT_CHANGE
 *		ORT_COST
 *		ORT_DTYPE
 *		ORT_ETYPE
 *		ORT_INFO
 *		ORT_INFO_VALID
 *		ORT_IO_NDX
 *		ORT_NH
 *		ORT_NH_CNT
 *		ORT_NH_NDX
 *		ORT_OSPF_PREF
 *		ORT_PTYPE
 *		ORT_REV
 *		ORT_TYPE2COST
 *		ORT_V
 *		OSPF_ADV_TAG
 *		PROTOTYPE
 *		RT_DEST
 *		RT_MASK
 *		RT_NEXTHOP
 *		bgp_routesync_ospf
 *		ospf_checksum
 *		ospf_checksum_bad
 *		ospf_checksum_sum
 *		ospf_get_ctime
 *		ospf_get_sys_time
 *		ospf_ifchk
 *		ospf_path_tag_dump
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
 * ospf_gated.h,v 1.22.2.1 1993/07/01 16:14:02 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#ifndef OSPF_PORT_H
#define OSPF_PORT_H

#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF 89
#endif	/* IPPROTO_OSPF */


/* Convert global_tod to char string */
#define ospf_get_ctime() time_string

#define ospf_get_sys_time()

/* Checksum calculations */
#define	ospf_checksum_sum(cp, len, sum)	sum += iso_cksum((void_t) cp, (size_t) len, (byte *) &(cp)->ls_hdr.ls_chksum)

#define	ospf_checksum(cp, len)	(void) iso_cksum((void_t) cp, (size_t) len, (byte *) &(cp)->ls_hdr.ls_chksum)

#define	ospf_checksum_bad(cp, len)	iso_cksum((void_t) cp, (size_t) len, (byte *) 0)

#define	INTF_MTU(intf)		(((intf)->type == VIRTUAL_LINK) ? MAXOUT : (intf)->ifap->ifa_mtu)
#define	INTF_ADDR(intf)		sock2ip((intf)->ifap->ifa_addr)
#define	INTF_LCLADDR(intf)	sock2ip((intf)->ifap->ifa_addr_local)
#define	INTF_NET(intf)		sock2ip((intf)->ifap->ifa_subnet)
#define	INTF_MASK(intf)		sock2ip((intf)->ifap->ifa_subnetmask)


/* XXX - Maybe we need an intermediate structure? */
#define	ifa_ospf_intf	ifa_ps[RTPROTO_OSPF].ips_datas[0]
#define	ifa_ospf_nh	ifa_ps[RTPROTO_OSPF].ips_datas[1]
#define	ifa_ospf_nh_lcl	ifa_ps[RTPROTO_OSPF].ips_datas[2]

#define	IF_INTF(ifap)	((struct INTF *)(ifap)->ifa_ospf_intf)


#define	OSPF_IFPS_ALLSPF	IFPS_KEEP1	/* Joined All SPF group */
#define	OSPF_IFPS_ALLDR		IFPS_KEEP2	/* Joined All DR group */

#undef	INTF_STATUS_CHANGE

#define	ospf_ifchk(ifap)	BIT_TEST(ifap->ifa_state, IFS_UP)

#define IP_PROTOCOL(IP) (IP)->ip_p
#define IP_LENGTH(IP) 	(IP)->ip_len

#define	OSPF_AUTH_NONE			0	/* No authentication */
#define	OSPF_AUTH_SIMPLE		1	/* Simple password */

/* Export types */
#define	OSPF_EXPORT_TYPE1	0x01
#define	OSPF_EXPORT_TYPE2	0x02

#define	OSPF_EXPORT_TAG		0x04		/* Tag is present */
#define	OSPF_EXPORT_TAG_METRIC2	0x08		/* Tag is in metric2 vs metric */
#define	OSPF_ADV_TAG(adv)	(BIT_TEST((adv)->adv_result.res_flag, OSPF_EXPORT_TAG_METRIC2) ? \
				 (adv)->adv_result.res_metric2 : (adv)->adv_result.res_metric)

/* Defines for the parser */
#define	OSPF_LIMIT_COST			0, RTRLSInfinity
#define	OSPF_LIMIT_METRIC		0, ASELSInfinity
#define	OSPF_LIMIT_AREA			1, 0xffffffff
#define	OSPF_LIMIT_RETRANSMITINTERVAL	0, 0xffff
#define	OSPF_LIMIT_ROUTERDEADINTERVAL	0, 0xffff
#define	OSPF_LIMIT_HELLOINTERVAL	0, 0xff
#define	OSPF_LIMIT_POLLINTERVAL		0, 0xff
#define	OSPF_LIMIT_TRANSITDELAY		0, 0xffff
#define	OSPF_LIMIT_DRPRIORITY		0, 0xff
#define	OSPF_LIMIT_ACKTIMER		0, 0xffff
#define	OSPF_LIMIT_AUTHKEY		8
#define	OSPF_LIMIT_AUTHTYPE		OSPF_AUTH_NONE, OSPF_AUTH_SIMPLE
#define	OSPF_LIMIT_TAG			0, 0xffffffff
#define	OSPF_LIMIT_EXPORTTYPE		OSPF_EXPORT_TYPE1, OSPF_EXPORT_TYPE2

#define	OSPF_CONFIG_TYPE	1	/* Interface type */
#define	OSPF_CONFIG_COST	2	/* Interface cost */
#define	OSPF_CONFIG_ENABLE	3	/* Enable/disable */
#define	OSPF_CONFIG_RETRANSMIT	4	/* Retransmit interval */
#define	OSPF_CONFIG_TRANSIT	5	/* Transit delay */
#define	OSPF_CONFIG_PRIORITY	6	/* Priority */
#define	OSPF_CONFIG_HELLO	7	/* Hello interval */
#define	OSPF_CONFIG_ROUTERDEAD	8	/* Router dead interval */
#define	OSPF_CONFIG_AUTHKEY	9	/* Authentication */
#define	OSPF_CONFIG_POLL	10	/* NBMA Poll interval */
#define	OSPF_CONFIG_ROUTERS	11	/* NBMA routers */
#define	OSPF_CONFIG_MAX		12

typedef struct _ospf_config_router {
    struct _ospf_config_router *ocr_next;
    struct in_addr ocr_router;
    u_int ocr_priority;
} ospf_config_router ;

PROTOTYPE(ospf_config_free,
	  extern void,
	  (config_entry *));
PROTOTYPE(ospf_parse_router_alloc,
	  extern ospf_config_router *,
	  (struct in_addr,
	   u_int));

extern block_t ospf_router_index;

/* Defaults for ASE imports */
#define	OSPF_DEFAULT_METRIC	1
#define	OSPF_DEFAULT_TAG	PATH_OSPF_TAG_TRUSTED
#define	OSPF_DEFAULT_TYPE	OSPF_EXPORT_TYPE2

#ifdef	PROTO_ASPATHS
#define	ospf_path_tag_dump(as, tag)	sockbuild_str(aspath_tag_dump(as, tag))
#else	/* PROTO_ASPATHS */
#define	ospf_path_tag_dump(as, tag)	sockbuild_in(0, htonl(tag))
#endif	/* PROTO_ASPATHS */

#define	bgp_routesync_ospf(x)	0

#define	OSPF_HOP			1	/* Value to add to ifa_metric to get default interface cost */

extern adv_psfunc ospf_adv_psfunc;

#ifdef	IP_MULTICAST
extern sockaddr_un *ospf_addr_allspf;
extern sockaddr_un *ospf_addr_alldr;
#endif	/* IP_MULTICAST */

PROTOTYPE(ospf_parse_area_alloc,
	  extern struct AREA *,
	  (u_int32,
	   char *));
PROTOTYPE(ospf_parse_area_check,
	  extern int,
	  (struct AREA *,
	   char *));
PROTOTYPE(ospf_parse_intf_alloc,
	  extern struct INTF *,
	  (struct AREA *,
	   int,
	   if_addr *));
PROTOTYPE(ospf_parse_virt_parse,
	  extern struct INTF *,
	  (struct AREA *,
	   sockaddr_un *,
	   u_int32,
	   config_list *,
	   char *));
PROTOTYPE(ospf_parse_intf_check,
	  extern void,
	  (struct INTF *intf));
PROTOTYPE(ospf_parse_valid_check,
	  extern int,
	  (char *));
PROTOTYPE(ospf_parse_add_net,
	  void,
	  (struct AREA *,
	   sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(ospf_parse_add_host,
	  extern void,
	  (struct AREA *,
	   u_int32,
	   metric_t));
PROTOTYPE(ospf_init,
	  extern void,
	  (void));
PROTOTYPE(ospf_var_init,
	  extern void,
	  (void));
PROTOTYPE(ospf_txpkt,
	  extern void,
	  (struct OSPF_HDR *,
	   struct INTF *,
	   u_int,
	   size_t,
	   u_int32,
	   int));
PROTOTYPE(ospf_policy_init,
	  extern void,
	  (task *));
PROTOTYPE(ospf_policy_cleanup,
	  extern void,
	  (task *));
PROTOTYPE(ospf_freeRangeList,
	  extern void,
 	  (struct AREA *));


/**/
/* Routing table */

#define IS_HOST(R) 		(RT_MASK(R) == HOST_NET_MASK)

/*
 * References for the the routing table access
 */

#define ORT_INFO(rt)	((OSPF_RT_INFO * ) (rt)->rt_data->rtd_data)
#define	ORT_INFO_VALID(rt)	((rt) && (rt)->rt_data && (rt)->rt_data->rtd_data)
#define ORT_DTYPE(rt) 	(ORT_INFO(rt)->dtype)
#define ORT_ETYPE(rt) 	(ORT_INFO(rt)->etype)
#define ORT_CHANGE(rt) 	(ORT_INFO(rt)->change)
#define ORT_PTYPE(rt) 	(ORT_INFO(rt)->ptype)
#define ORT_REV(rt) 	(ORT_INFO(rt)->revision)
#define ORT_AREA(rt) 	(ORT_INFO(rt)->area)
#define ORT_COST(rt)	(ORT_INFO(rt)->cost)
#define ORT_TYPE2COST(rt) (ORT_INFO(rt)->type2cost)
#define ORT_NH(rt,I) 	 (ORT_INFO(rt)->nh_ndx[I]->nh_addr)
#define ORT_IO_NDX(rt,I) (ORT_INFO(rt)->nh_ndx[I]->nh_ifap)
#define ORT_NH_NDX(rt,I) (ORT_INFO(rt)->nh_ndx[I])
#define ORT_NH_CNT(rt) 	(ORT_INFO(rt)->nh_cnt)
#define ORT_ADVRTR(rt) 	(ORT_INFO(rt)->advrtr)
#define ORT_OSPF_PREF(rt) (ORT_INFO(rt)->preference)
#define ORT_V(rt) 	(ORT_INFO(rt)->v)

/* OSPF's routing table structure */

#define RT_DEST(rt) 	sock2ip((rt)->rt_dest)
#define	RT_MASK(rt)	sock2ip((rt)->rt_dest_mask)
#define	RT_NEXTHOP(rt)	sock2ip((rt)->rt_router)

/*
 *  For exporting gated routes to OSPF.
 */
typedef struct _ospf_export_entry {
    struct _ospf_export_entry *forw;
    struct _ospf_export_entry *back;
    rt_entry *old_rt;		/* points at route with bit set, if any */
    struct LSDB *db;		/* points to LS db entry, if any */
    rt_entry *new_rt;		/* points at exportable active route, if any */
    metric_t metric;
    u_int32 tag;
    struct in_addr forward;
} ospf_export_entry;


/**/
/* Timers */

#define	OTIMER	timer
#define	INTF_FROM_TIMER(tip)	(struct INTF *) tip->timer_task->task_data
#define	AREA_FROM_TIMER(tip)	(struct AREA *) tip->timer_data

/*
 * task timer index defines
 */
#define	INTF_TIMER_HELLO	0 	/* interface timers */
#define INTF_TIMER_RXMIT	1
#define INTF_TIMER_NETLOCK	2
#define	INTF_TIMER_MAX		3

#define SYS_TIMER_INT_LSA	0	/* generate lsa timers */
#define SYS_TIMER_SUM_LSA	1
#define SYS_TIMER_ASE_LSA	2
#define SYS_TIMER_INT_AGE	3	/* lsdb age timers */
#define SYS_TIMER_SUM_AGE	4
#define SYS_TIMER_ASE_AGE	5
#define	SYS_TIMER_ASE_QUEUE	6
#define	SYS_TIMER_ACK		7
#define	SYS_TIMER_MAX		8

PROTOTYPE(ospf_ifdown,
	  extern void,
	  (struct INTF *));
PROTOTYPE(ospf_ifup,
	  extern void,
	  (struct INTF *));
PROTOTYPE(tq_hellotmr,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_lsa_lock,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_IntLsa,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_SumLsa,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_AseLsa,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_retrans,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_ack,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_int_age,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_sum_age,
	  extern void,
	  (OTIMER *,
	   time_t));
PROTOTYPE(tq_ase_age,
	  extern void,
	  (OTIMER *,
	   time_t));

/* SNMP support */
#ifdef	PROTO_SNMP
PROTOTYPE(ospf_init_mib,
	  extern void,
	  (int));
PROTOTYPE(o_intf_get,
	  extern void,
	  (void));
PROTOTYPE(o_vintf_get,
	  extern void,
	  (void));
#endif	/* PROTO_SNMP */

#endif	/* OSPF_PORT_H */


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
