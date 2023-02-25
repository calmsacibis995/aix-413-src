/* @(#)16	1.1  src/tcpip/usr/sbin/gated/policy.h, tcprouting, tcpip411, GOLD410 12/6/93 17:27:11 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ADV_LIST
 *		ADV_LIST_END
 *		CONFIG_LIST
 *		CONFIG_LIST_END
 *		GW_LIST
 *		PROTOTYPE
 *		PS_FUNC
 *		PS_FUNC_VALID
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
 * policy.h,v 1.20.2.2 1993/08/27 22:33:14 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


typedef struct _dest_mask {
#ifdef	ROUTE_AGGREGATION
    flag_t dm_flags;
#define	DMF_EXACT	BIT(0x01)	/* Mask must match exactly */
#define	DMF_REFINE	BIT(0x02)	/* Mask must refine our mask */
#endif	/* ROUTE_AGGREGATION */
    sockaddr_un *dm_dest;
    sockaddr_un *dm_mask;
} dest_mask;


/*
 *	Structure describing a gateway
 */
typedef struct _gw_entry {
    struct _gw_entry *gw_next;
    proto_t gw_proto;			/* Protocol of this gateway */
    sockaddr_un *gw_addr;		/* Address of this gateway */
    flag_t gw_flags;			/* Flags for this gateway */
    time_t gw_time;			/* Time this gateway was last heard from */
    time_t gw_timer_max;		/* Maximum timer for this route */
    task *gw_task;			/* The task associated with this gateway */
    as_t gw_peer_as;			/* The AS of this gateway */
    as_t gw_local_as;			/* The AS advertised to this gateway */
    struct _adv_entry *gw_import;	/* What to import from this gateway */
    struct _adv_entry *gw_export;	/* What to export to this gateway */
#ifdef	PROTO_SNMP
    u_int	gw_bad_packets;		/* Bad packets received from this GW */
    u_int	gw_bad_routes;		/* Bad routes received from this GW */
#endif	/* PROTO_SNMP */
} gw_entry;

#define	GWF_SOURCE	BIT(0x01)	/* This is a source gateway */
#define	GWF_TRUSTED	BIT(0x02)	/* This is a trusted gateway */
#define	GWF_ACCEPT	BIT(0x04)	/* We accepted a packet from this gateway */
#define	GWF_REJECT	BIT(0x08)	/* We rejected a packet from this gateway */
#define	GWF_QUERY	BIT(0x10)	/* RIP query packet received */
#define	GWF_IMPORT	BIT(0x20)	/* Rejected a network due to import restrictions */
#define	GWF_FORMAT	BIT(0x40)	/* Packet format error */
#define	GWF_CHECKSUM	BIT(0x80)	/* Bad checksum */
#define	GWF_AUXPROTO	BIT(0x100)	/* This is an auxilary protocol */
#define	GWF_AUTHFAIL	BIT(0x200)	/* Authentication failure */
#define	GWF_NEEDHOLD	BIT(0x400)	/* This protocol requires holddowns */


/*
 *	Structure defining routines to use to process protocol specific data
 */
typedef struct _adv_psfunc {
    _PROTOTYPE(ps_rtmatch,
	       int,
	       (void_t,
		rt_entry *));		/* Routine to match data against route */
    _PROTOTYPE(ps_dstmatch,
	       int,
	       (void_t,
		sockaddr_un *,
		void_t));		/* Routine to match data against destination */
    _PROTOTYPE(ps_compare,
	       int,
	       (void_t,
		void_t));		/* Routine to compare two sets of data */
    _PROTOTYPE(ps_print,
	       char *,
	       (void_t,
		int));			/* Routine to display data */
    _PROTOTYPE(ps_free,
	       void,
	       (adv_entry *));		/* Routine to free data */
} adv_psfunc;

#define	PS_FUNC(adv, func)	control_psfunc[(adv)->adv_proto]->func
#define	PS_FUNC_VALID(adv, func)	(control_psfunc[(adv)->adv_proto] && PS_FUNC(adv, func))
    
extern adv_psfunc *control_psfunc[];


/* Description of results of a policy search */
typedef struct _adv_results {
    union {
	metric_t	resu_metric;
	sockaddr_un 	*resu_mask;
    } res_u1;
#define	res_metric	res_u1.resu_metric
#define	res_mask	res_u1.resu_mask
    union {
	metric_t	resu_metric;
	pref_t		resu_preference;
    } res_u2;
#define	res_metric2	res_u2.resu_metric
#define	res_preference	res_u2.resu_preference
    flag_t	res_flag;
} adv_results;


/* Description of config file info */

typedef struct _config_entry {
    struct _config_entry *config_next;
    short config_type;
    short config_priority;
    void_t config_data;
} config_entry;

#define	CONFIG_LIST(cp, list)	for (cp = list; cp; cp = cp->config_next)
#define	CONFIG_LIST_END(cp, list)


typedef struct _config_list {
    int conflist_refcount;
    config_entry *conflist_list;
    _PROTOTYPE(conflist_free,
	       void,
	       (config_entry *));
} config_list;


#define	CONFIG_PRIO_ANY		1
#define	CONFIG_PRIO_WILD	2
#define	CONFIG_PRIO_NAME	3
#define	CONFIG_PRIO_ADDR	4
#define	CONFIG_PRIO_MAX		5



/*
 *	Structure used for all control lists.  Nested unions are used
 *	to minimize unused space.
 */
struct _adv_entry {
    struct _adv_entry *adv_next;	/* Pointer to next entry in list */
    int adv_refcount;			/* Number of references */
    flag_t adv_flag;			/* Flags */
    proto_t adv_proto;			/* Protocol for this match */

    union {
	adv_results	advru_result;		/* Result of the lookup */
#define	adv_result	adv_ru.advru_result
	config_list	*advru_config;		/* Config list */
#define	adv_config	adv_ru.advru_config
    } adv_ru;

    struct _adv_entry *adv_list;	/* List of functions to match */

    union adv_union {
	dest_mask advu_dm;
#define	adv_dm		adv_u.advu_dm
	gw_entry *advu_gwp;		/* Match a gateway address */
#define	adv_gwp		adv_u.advu_gwp
	if_addr *advu_ifap;		/* Match an interface */
#define	adv_ifap		adv_u.advu_ifap
	if_addr_entry *advu_ifn;	/* Match an interface name */
#define	adv_ifn		adv_u.advu_ifn
	if_addr_entry *advu_ifae;	/* Match on interface address */
#define	adv_ifae	adv_u.advu_ifae
	as_t advu_as;			/* Match an AS */
#define	adv_as		adv_u.advu_as
	tag_t advu_tag;			/* Match on tag */
#define	adv_tag		adv_u.advu_tag
	void_t	advu_aspath;		/* Match with AS path pattern */
#define	adv_aspath	adv_u.advu_aspath
	void_t advu_ps;			/* Protocol specific data */
#define	adv_ps		adv_u.advu_ps
    } adv_u;
};

#define	ADVF_TYPE		BIT(0x0f)	/* Type to match */
#define	ADVFT_ANY		BIT(0x00)	/* No type specified */
#define	ADVFT_GW		BIT(0x01)	/* Match gateway address */
#define	ADVFT_IFN		BIT(0x02)	/* Match on interface name */
#define	ADVFT_IFAE		BIT(0x03)	/* Match on interface address */
#define	ADVFT_AS		BIT(0x04)	/* Match on AS */
#define	ADVFT_DM		BIT(0x05)	/* Match on dest/mask pair */
#define	ADVFT_ASPATH		BIT(0x06)	/* Match on AS path */
#define	ADVFT_TAG		BIT(0x07)	/* Match on tag */
#define	ADVFT_PS		BIT(0x08)	/* Match on protocol specific data */

#define	ADVFO_TYPE		BIT(0xf0)	/* Option type */
#define	ADVFOT_NONE		BIT(0x00)	/* No option specified */
#define	ADVFOT_METRIC		BIT(0x10)	/* Result Metric option */
#define	ADVFOT_PREFERENCE	BIT(0x20)	/* Result Preference option */
#define	ADVFOT_METRIC2		ADVFOT_PREFERENCE
#define	ADVFOT_FLAG		BIT(0x40)	/* Result Flag option */
#define	ADVFOT_CONFIG		BIT(0x80)	/* Config structure */

#define	ADVF_NO			BIT(0x1000)	/* Negative (i.e. noannounce, nolisten, nopropogate) */
#define	ADVF_FIRST		BIT(0x2000)	/* First entry in a sequence (of gateways or interfaces) */


#define	GW_LIST(list, gwp)	for (gwp = list; gwp; gwp = gwp->gw_next)
#define	GW_LISTEND

#define	ADV_LIST(list, adv)	for (adv = list; adv; adv = adv->adv_next)
#define	ADV_LIST_END(list, adv)


extern adv_entry *martian_list;
extern unsigned int adv_n_allocated;

PROTOTYPE(control_dump,
	  extern void,
	  (FILE *));
PROTOTYPE(control_import_dump,
	  extern void,
	  (FILE *,
	   int,
	   proto_t,
	   adv_entry *,
	   gw_entry *));
PROTOTYPE(control_export_dump,
	  extern void,
	  (FILE *,
	   int,
	   proto_t,
	   adv_entry *,
	   gw_entry *));
PROTOTYPE(control_entry_dump,
	  extern void,
	  (FILE *,
	   int,
	   adv_entry *));
PROTOTYPE(control_dmlist_dump,
	  extern void,
	  (FILE *,
	   int,
	   adv_entry *,
	   adv_entry *,
	   adv_entry *));
PROTOTYPE(control_interior_dump,
	  extern void,
	  (FILE *,
	   int,
	   _PROTOTYPE(func,
		      void,
		      (FILE *,
		       int,
		       proto_t,
		       adv_entry *,
		       gw_entry *)),
	   adv_entry * list));
PROTOTYPE(control_exterior_dump,
	  extern void,
	  (FILE *,
	   int,
	   _PROTOTYPE(func,
		      void,
		      (FILE *,
		       int,
		       proto_t,
		       adv_entry *,
		       gw_entry *)),
	   adv_entry * list));
PROTOTYPE(control_interface_dump,
	  extern void,
	  (FILE *,
	   int,
	   adv_entry *list,
	   _PROTOTYPE(func,
		      void,
		      (FILE *,
		       config_entry *))));
PROTOTYPE(control_interface_import_dump,
	  extern void,
	  (FILE *,
	   int,
	   adv_entry *));
PROTOTYPE(control_interface_export_dump,
	  extern void,
	  (FILE *,
	   int,
	   adv_entry *));
PROTOTYPE(control_exterior_locate,
	  extern adv_entry *,
	  (adv_entry * list,
	   as_t as));
PROTOTYPE(import,
	  extern int,
	  (sockaddr_un *,
	   adv_entry *,
	   adv_entry *,
	   adv_entry *,
	   pref_t *,
	   if_addr *,
	   void_t));
PROTOTYPE(export,
	  extern int,
	  (struct _rt_entry *,
	   proto_t,
	   adv_entry *,
	   adv_entry *,
	   adv_entry *,
	   adv_results *));
PROTOTYPE(is_martian,
	  extern int,
	  (sockaddr_un * dst));
PROTOTYPE(martian_add,
	  extern void,
	  (sockaddr_un *,
	   sockaddr_un *,
	   flag_t));
PROTOTYPE(martian_cleanup,
	  extern void,
	  (void));

PROTOTYPE(adv_alloc,
	  extern adv_entry *,
	  (flag_t,
	   proto_t));
PROTOTYPE(adv_free_list,
	  extern void,
	  (adv_entry * adv));
PROTOTYPE(adv_cleanup,
	  extern void,
	  (proto_t,
	   int *,
	   int *,
	   gw_entry *,
	   adv_entry **,
	   adv_entry **,
	   adv_entry **));
PROTOTYPE(adv_psfunc_add,
	  extern void,
	  (proto_t,
	   adv_psfunc *));
PROTOTYPE(adv_destmask_match,
	  extern adv_entry *,
	  (adv_entry *,
	   sockaddr_un *));

PROTOTYPE(gw_locate,
	  extern gw_entry *,
	  (gw_entry **,
	   proto_t,
	   task *,
	   as_t,
	   as_t,
	   time_t,
	   sockaddr_un *,
	   flag_t));
PROTOTYPE(gw_timestamp,
	  extern gw_entry *,
	  (gw_entry **,
	   proto_t,
	   task *,
	   as_t,
	   as_t,
	   time_t,
	   sockaddr_un *,
	   flag_t));
PROTOTYPE(gw_init,
	  extern gw_entry *,
	  (gw_entry *,
	   proto_t,
	   task *,
	   as_t,
	   as_t,
	   time_t,
	   sockaddr_un *,
	   flag_t));
PROTOTYPE(gw_dump,
	  extern void,
	  (FILE *,
	   const char *,
	   gw_entry *,
	   proto_t));
PROTOTYPE(gw_freelist,
	  extern void,
	  (gw_entry *));

/* Config info */
PROTOTYPE(config_alloc,
	  extern config_entry *,
	  (int,
	   void_t));
PROTOTYPE(config_append,
	  extern config_entry *,
	  (config_entry *,
	   config_entry *));
PROTOTYPE(config_resolv,
	  extern config_entry **,
	  (adv_entry *,
	   if_addr *,
	   int));
PROTOTYPE(config_resolv_free,
	  extern void,
	  (config_entry **,
	   int));
PROTOTYPE(config_list_alloc,
	  extern config_list *,
	  (config_entry *,
	   _PROTOTYPE(entry_free,
		      void,
		      (config_entry *))));
PROTOTYPE(config_list_free,
	  extern void,
	  (config_list *));
PROTOTYPE(config_list_add,
	  extern config_list *,
	  (config_list *,
	   config_entry *,
	   _PROTOTYPE(entry_free,
		      void,
		      (config_entry *))));

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
