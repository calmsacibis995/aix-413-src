/* @(#)83	1.1  src/tcpip/usr/sbin/gated/ospf.h, tcprouting, tcpip411, GOLD410 12/6/93 17:25:41 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ADV_NETNUM
 *		AREA_LIST
 *		AREA_LIST_END
 *		FirstNbr
 *		INTF_LIST
 *		INTF_LIST_END
 *		NBRS_LIST
 *		NBRS_LIST_END
 *		PROTOTYPE
 *		RANGE_LIST
 *		RANGE_LIST_END
 *		REM_NBR_RETRANS
 *		RTR_ADV_NETNUM
 *		VINTF_LIST_END
 *		_PROTOTYPE
 *		findroute
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
 * ospf.h,v 1.15.2.2 1993/08/27 22:28:22 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#include "ospf_gated_mem.h"
#include "ospf_log.h"

/*
 * state transition routines from ospf_state.c
 */

/* state and event ranges - states.c */

#define	NINTF_STATES	7
#define	NINTF_EVENTS	7

#define NNBR_STATES	8
#define NNBR_EVENTS	14

#include "ospf_rtab.h"
#include "ospf_timer_calls.h"	/* timer calls */
#include "ospf_pkts.h"		/* packet formats */
#include "ospf_lsdb.h"		/* link-state database */
#include "ospf_const.h"		/* OSPF constants */

#define OSPF_VERSION 0x02

#include "ospf_gated.h"


/* Flag first for fast flooding functions or rubber baby buggy bumpers */
#define FLOOD		0
#define DONTFLOOD	1

/* default values */
#define 	OSPF_NBMA_DFLT_HELLO	30
#define  	OSPF_BC_DFLT_HELLO	10
#define 	OSPF_PTP_DFLT_HELLO	30
#define 	OSPF_VIRT_DFLT_HELLO	60

#define ADV_NETNUM(A)  	((A)->ls_hdr.ls_id & (A)->net_mask)
#define RTR_ADV_NETNUM(A)  ((A)->lnk_id & (A)->lnk_data)


/***************************************************************************

	   		PROTOCOL DATA STRUCTURES

****************************************************************************/


struct LSDB_SUM {
    struct LSDB_SUM *next;
    struct OSPF_HDR *dbpkt;	/* for dbsum pkts */
    u_int16 len;		/* length of this pkt including ospf hdr size */
    u_int16 cnt;		/* number of lsdb entries in this pkt */
};

#define LSDB_SUM_NULL ((struct LSDB_SUM *)0)

struct LS_REQ {
    struct LS_REQ *ptr[2];
    u_int32 ls_id;
    u_int32 adv_rtr;
    u_int32 ls_seq;
    u_int16 ls_chksum;
    u_int16 ls_age;
};

#define LS_REQ_NULL ((struct LS_REQ *) 0)


/*
 *		Events causing neighbor state changes
 */

#define		HELLO_RX	0
#define		START		1
#define		TWOWAY		2
#define		ADJ_OK		3
#define		NEGO_DONE	4
#define		EXCH_DONE	5
#define		SEQ_MISMATCH	6
#define		BAD_LS_REQ	7
#define		LOAD_DONE	8
#define		ONEWAY		9
#define		RST_ADJ		10
#define		KILL_NBR	11
#define		INACT_TIMER	12
#define		LLDOWN		13

struct NBR {
    struct NBR *next;
    struct INTF *intf;
#ifdef	notdef
    if_addr *ifap;
#endif	/* notdef */
    u_int8 I_M_MS;		/* for passing init, more and mast/slave bits */
    u_int8 mode;		/* master or slave mode */
#define		SLAVE		1
#define		MASTER		2
#define		SLAVE_HOLD	4	/* holding the last dbsum delay */
    u_int state;
#define		NDOWN		0
#define		NATTEMPT 	1
#define		NINIT		2
#define		N2WAY		3
#define		NEXSTART	4
#define		NEXCHANGE	5
#define		NLOADING	6
#define		NFULL		7

    u_int32 seq;
    sockaddr_un *nbr_id;
#define	NBR_ID(nbr)	sock2ip((nbr)->nbr_id)
    sockaddr_un *nbr_addr;
#define	NBR_ADDR(nbr)	sock2ip((nbr)->nbr_addr)
    struct NH_BLOCK *nbr_nh;	/* The NH entry we installed for this neighbor */
    time_t last_hello;		/* time of rx last hello */
    time_t last_exch;		/* time rx last exchange - hold tmr */
    /* for multi-access nets */
    u_int16 pri;		/* 0 means not elig */
    u_int16 rtcnt;		/* retrans queue cnt */
    u_int32 dr;
    u_int32 bdr;
    struct LSDB_LIST retrans[OSPF_HASH_QUEUE];	/* LSAs waiting for acks */
    u_int16 dbcnt;		/* dbsum queue cnt */
    u_int16 reqcnt;		/* ls_req queue cnt */
    struct LSDB_SUM *dbsum;	/* dbsum pkts that make up area db */
    struct LS_REQ *ls_req[6];	/* the ones this rtr wants from this nbr */
    int events;
};

#define	REM_NBR_RETRANS(nbr)	if ((nbr)->rtcnt) rem_nbr_retrans(nbr);

#define	NBRNULL	((struct NBR *) 0)

#define	NBRS_LIST(nbr, intf)	for (nbr = FirstNbr(intf); nbr; nbr = nbr->next)
#define	NBRS_LIST_END(nbr, intf)

#define NO_REQ(N)      ((N)->reqcnt == 0)

/* txpkt defines for NBMA sends */
#define ALL_UP_NBRS 	1
#define ALL_ELIG_NBRS   2
#define ALL_EXCH_NBRS   3
#define DR_and_BDR	4


/*
 * 		Events causing IF state changes
 */
#define		INTF_UP		0
#define		WAIT_TIMER	1
#define		BACKUP_SEEN	2
#define		NBR_CHANGE	3
#define		LOOP_IND	4
#define		UNLOOP_IND	5
#define		INTF_DOWN	6


struct INTF {			/* structure contained within the area */
    struct INTF *next;
    if_addr *ifap;
    struct AREA *area;		/* The area I am in */
    flag_t flags;
#define	OSPF_INTFF_NETSCHED		BIT(0x01)	/* When DR: semaphor for generating LSAs */
#define	OSPF_INTFF_ENABLE		BIT(0x02)	/* Interface is enabled */
#define	OSPF_INTFF_BUILDNET		BIT(0x04)	/* Flag to build_net_lsa */
#define	OSPF_INTFF_NBR_CHANGE		BIT(0x08)	/* Schedule neighbor change */
#define	OSPF_INTFF_MULTICAST		BIT(0x10)	/* Interface is multicast capable */    
#define	OSPF_INTFF_COSTSET		BIT(0x20)	/* Cost was manually configured */
    u_int8 type;
#define		BROADCAST	1
#define		NONBROADCAST	2
#define		POINT_TO_POINT	3
#define		VIRTUAL_LINK	4
    u_int8 state;
#define		IDOWN		0
#define		ILOOPBACK	1
#define		IWAITING	2
#define		IPOINT_TO_POINT	3
#define		IDr		4
#define		IBACKUP		5
#define		IDrOTHER	6
    time_t lock_time;		/* net lock timer */
    u_int16 cost;		/* one for each tos */
    u_int8 pollmod;		/* poll timer is 4 * hello timer */
#define 	STATUS_MOD 4
    u_int8 status_mod;		/* check status 4 * hello timer */
    time_t wait_time;		/* interface is in waiting state */
    time_t hello_timer;		/* interface sends hello (seconds) */
    time_t poll_timer;		/* nbma reduced hello tmr, nbr gone */
    time_t dead_timer;		/* time since last recieved hello */
    time_t retrans_timer;	/* retransmit interval */
    time_t transdly;		/* seconds to transmit a lsu over IF */
    u_int32 authkey[OSPF_AUTH_SIZE / sizeof (u_int32)];		/* 64 bits of auth */
    int nbrIcnt;		/* Count of neighbors > NINIT */
    u_int16 nbrEcnt;		/* Count of neighbors >= EXCHAGE */
    u_int16 nbrFcnt;		/* Count of neighbors == NFULL */
    int events;			/* Cound of state changes */
    struct LS_HDRQ acks;	/* Delayed ack list */
    int ack_cnt;		/* Number of acks queued on this interface */
    struct NBR nbr;		/* linked list of nbrs; if we have to select
				 * dr then head is fake 'this rtr' nbr if
				 * this IF is virtual or point to point use
				 * head of list */
    /* the following are used for interfaces that select dr and bdr */
    u_int16 pri;		/* if priority - if 0 not elig */
    struct AREA *transarea;	/* Virtual link transit area ndx */
    struct NBR *dr;		/* ptr to dr */
    struct NBR *bdr;		/* ptr to bdr */
    task *task;
};
#define INTFNULL ((struct INTF *) 0)

/* Multi-access nets use intf nbr structure for electing dr */
#define FirstNbr(I) ( (((I)->type == BROADCAST) ||\
		      ((I)->type == NONBROADCAST)) ?\
			 (I)->nbr.next : &((I)->nbr) )

#define	INTF_LIST(i, a) for (i = a->intf; i; i = i->next) 
#define	INTF_LIST_END(i, a)

#define	VINTF_LIST(vi)	for (vi = ospf.vl; vi; vi = vi->next)
#define	VINTF_LIST_END(vi)

/*
 *  list of configured hosts
 */
struct OSPF_HOSTS {
    struct OSPF_HOSTS *ptr[2];
    u_int32 if_addr;
    u_int32 cost;
};

#define HOSTSNULL ((struct OSPF_HOSTS *) 0)

/*
 *  list of nets associated with an area
 */
struct NET_RANGE {
    struct NET_RANGE *ptr[2];
    u_int32 net, mask;
    u_int32 cost;
    rt_entry *rt;
};

#define NRNULL ((struct NET_RANGE *) 0)

#define	RANGE_LIST(nrp, area)	for (nrp = (area)->nr.ptr[NEXT]; nrp; nrp = nrp->ptr[NEXT])
#define	RANGE_LIST_END(nr, area)

struct AREA {
    struct AREA *next;
    u_int32 area_id;
    u_int16 nrcnt;		/* count of net ranges defined for this area */
    flag_t	area_flags;	/* Various state bits */
#define	OSPF_AREAF_TRANSIT	BIT(0x01)	/* This is a transit area */
#define	OSPF_AREAF_VIRTUAL_UP	BIT(0x02)	/* One or more virtual links in this area are up */
#define	OSPF_AREAF_STUB		BIT(0x04)	/* This is a stub area (NO ASEs) */
#define	OSPF_AREAF_NSSA		BIT(0x08)	/* This is a not so stubby area */
#define	OSPF_AREAF_STUB_DEFAULT	BIT(0x10)	/* Inject default into this stub area */
#define	OSPF_AREAF_SPF_RUN	BIT(0x20)	/* An SPF needs to be run for this area */
#define	OSPF_AREAF_SPF_FULL	BIT(0x40)	/* A full SPF is required */

    struct NET_RANGE nr;	/* list of component networks */
    u_int32 spfcnt;		/* # times spf has been run for this area */
    u_short db_int_cnt; 	/* Intra + inter LSDB entry count */
    u_short db_cnts[5];		/* Counts for each type of LSDB entry */
    u_int32 db_chksumsum;	/* Checksum sum */
    u_int16 asbr_cnt;		/* count of as bdr rtrs local to this area */
    u_int16 abr_cnt;		/* count of area bdr rtrs local to this area */
    struct OSPF_ROUTE asbrtab;	/* as bdr rtrs within this area */
    struct OSPF_ROUTE abrtab;	/* area bdr rtr within this area */
    u_int16 nbrIcnt;		/* Count of neighbors >= NINIT (area) */
    u_int16 nbrEcnt;		/* neighbors >= EXCHANGE (area) */
    u_int16 nbrFcnt;		/* neighbors == NFULL (area) */
    u_int16 ifcnt;		/* will allocate an array at config */
    adv_entry *intf_policy;	/* Interface matching */
    struct INTF *intf;		/* setup at config time  */
    struct LSDB *htbl[6];	/* the lsdb - 0 is for stub nets */
    u_int16 authtype;		/* authentication type */
    u_int16 ifUcnt;           	/* count of up INTFs include virt lnks for BB */
    struct LSDB spf;		/* area's spf tree; head is this rtr */
    struct LSDB candidates;	/* area'scandidate list for dijkstra */
    struct LSDB asblst;		/* reachable asbs (connected areas) */
    struct LSDB sumnetlst;	/* reachable nets from attached areas */
    struct LSDB interlst;	/* reahcable inter-area routes from backbone */
    struct LSDB dflt_sum;	/* used if ABRtr and stub area */
    u_int32 dflt_metric;	/* metric for default route */
    struct LSDB_LIST *txq;	/* for building and sending sum lsa */
    time_t lock_time;		/* rtr lock timer */
    struct OSPF_HOSTS hosts;
    u_int8 hostcnt;
    u_int8 lsalock;		/* MinLsInterval semaphore for LSA
				 * origination */
    u_int8 spfsched;		/* Schedule flags for spf algorithm */
    u_int8 spfsched_save;	/* Saved flags */
    u_int8 build_rtr;		/* Schedule build_rtr_lsa */

#define 	RTRSCHED 	0x02
#define 	NETSCHED 	0x04
#define 	INTRASCHED 	0x07
#define 	SUMNETSCHED 	0x08
#define 	SUMASBSCHED 	0x10
#define 	SUMSCHED 	(SUMNETSCHED | SUMASBSCHED)
#define 	INTSCHED 	(INTRASCHED | SUMSCHED)
#define 	ASESCHED 	0x20
#define 	SUMASESCHED 	(SUMSCHED | ASESCHED)
#define 	ALLSCHED 	(INTSCHED | ASESCHED)
#define		FLAG_NO_PROBLEM	0x0
#define		FLAG_BUILD_RTR	0x40
#define		FLAG_BUILD_NET	0x80
#define		FLAG_LOAD_DONE	0x100
#define 	FLAG_FOUND_REQ	0x400
#define		FLAG_NO_BUFS	0x800
#define		FLAG_BAD_REQ	0x1000
#define 	SCHED_BIT(T) 	(1 << (T))
#define 	RTRLOCK  	0x40
#define 	NETLOCK  	0x80
};

#define AREANULL ((struct AREA *)0)

#define	AREA_LIST(a)	for (a = ospf.area; a; a = a->next)
#define	AREA_LIST_END(a)

/* GLOBAL FOR THE PROTOCOL */

struct OSPF {
    sockaddr_un *router_id;	/* My router ID */
#define	MY_ID	sock2ip(ospf.router_id)
    int ospf_admin_stat;	/* Enabled or Disabled */
#define OSPF_ENABLED 	1
#define OSPF_DISABLED 	0
    int nintf;			/* number of ospf interfaces */
    int nbrcnt;			/* number of neighbors known to this router */
    int nbrIcnt;		/* number of neighbors >= Init state */
    int nbrEcnt;		/* number of neighbors >= Exchange state */
    int nbrFcnt;		/* number of neighbors == Full state */
    int acnt;			/* number of areas, 0 will allways be bacbone */
    struct AREA backbone;	/* Backbone area */
    struct AREA *area;		/* areas connected to this router - an array
				 * which will be allocated at init time, area
				 * 0 is the backbone */
    int vcnt;			/* number of virtual links */
    int vUPcnt;
    struct INTF *vl;		/* list of configured virtal links */
    struct LSDB ase[HTBLSIZE];	/* external ls advertisements */
    time_t ase_start;		/* time started tq_AseLsa */
    int ase_age_ndx;		/* starting index of next dbage */
    struct LSDB *cur_ase;	/* current ptr */
    struct LSDB my_ase_list;	/* self generated ase list */
    struct LSDB db_free_list;	/* list of LSAs to be freed */
    int asbr;			/* as border rtr flag */

    task *task;			/* task for lsa generataion and lsdb aging */
    pref_t preference;		/* Preference for intra and inter area routes */
    pref_t preference_ase;	/* Preference for ASE routes */
    adv_entry *import_list;	/* Networks to import */
    adv_entry *export_list;	/* Networks to export */
    gw_entry *gwp;		/* Gateway structure for AS Internal routes */
    gw_entry *gwp_ase;		/* Gateway structure for ASE routes */
    metric_t export_metric;	/* Default metric for external routes */
    metric_t export_tag;		/* Default tag for external routes (in host byte order) */
    metric_t export_type;	/* Default type for external routes */
    flag_t trace_flags;		/* Trace options for OSPF */
    gw_entry *gw_list;		/* List of gateways */
    u_int32 rtab_rev;		/* Rev number of ospf's portion of the rtab */
    time_t export_interval;	/* Minimum interval between ASE exports into OSPF */
    int export_limit;		/* Maximum number of ASEs to import into OSPF per interval */
    int export_queue_size;	/* Number in queue */
    ospf_export_entry export_queue;	/* List of routes queued for exportation into OSPF */
    ospf_export_entry *export_queue_delete; /* Place to insert deleted routes */
    ospf_export_entry *export_queue_change; /* Place to insert changed routes */
#define  OSPF_BACKBONE	0
#define  SPFCNT  	(ospf.rtab_rev)
#define  RTAB_REV	(ospf.rtab_rev)
#define 	GOTBACKBONE	(ospf.vcnt || ospf.backbone.intf_policy)
#define 	IAmBorderRtr	(GOTBACKBONE && ospf.acnt > 1)
#define 	IAmASBorderRtr  (ospf.asbr)

    struct NH_BLOCK nh_list;

    flag_t mon_authtype;	/* Authentication for monitor packets */
    u_int32 mon_authkey[OSPF_AUTH_SIZE / sizeof (u_int32) ];	/* 64 bits of auth for monitor packets */
    /* A few stats for the MIB */
    int db_cnt;			/* Total number of entries in the LSDB */
    u_int32 db_chksumsum;	/* Checksum sum of external ASEs */
    int db_ase_cnt;		/* Number of ASEs in the LSDB */
    int rx_new_lsa;		/* Number of new LSAs received */
    int orig_new_lsa;		/* Number of self originated LSAs */

    /*
     * A single trap is generated per event where an event is a 
     * timer expiring or a packet being received 
     */
#define TRAP_REF_LEN		2
#define TRAP_REF_UPDATE \
	ospf.trap_ref[1] += !++(ospf.trap_ref[0]) ? 1 : 0
#define TRAP_REF_CURRENT(T) \
	( ((T)[0] == ospf.trap_ref[0]) && ((T)[1] == ospf.trap_ref[1]) )
	/* Set T to equal ospf.trap_ref */
#define TRAP_REF_SET(T) \
	{ (T)[0] = ospf.trap_ref[0]; (T)[1] = ospf.trap_ref[1]; }
    u_int32	trap_ref[TRAP_REF_LEN];	

    /* LSDB limits */
    int		lsdb_limit;	/* Configured upper limit of LSDBs */
    int		lsdb_overflow;	/* We're in lsdb overflow mode */
    int		lsdb_hiwater;	/* Hi water mark for LSDB. 95% of lsdb_limit */
    int		lsdb_hiwater_exceeded;

#ifdef	notdef
    /* Sort block for INTFs */
    struct IF_SB *if_sb;
    int 	if_sb_nel;

    /* Sort block for virtual INTFs */
    struct IF_SB *virt_if_sb;
    int 	virt_if_sb_nel;

    /* Sort block for NBRs */
    struct NBR 	**nbr_sb;
    int 	nbr_sb_nel;
    u_int16	nbr_sb_not_valid;
    u_int16	nbr_sb_size;

    /* Sort block for lsdb */
    struct LSDB **ls_sb;	/* ptr to array of LSDB ptrs for MIB sorting */
    int	sb_size;		/* Size of allocated ls_sb */
    int	sb_nel;			/* Number of elements in sb */
    struct AREA *sb_area;	/* Area and ls type sorted in ls_sb */
    u_int8	sb_ls_type;
    u_int8	sb_not_valid;	/* If a new db entry has been added within
				   the range of area and type mark sb invalid */
#endif	/* notdef */
}; 

extern struct OSPF ospf;


/**/

/* Prototype */
PROTOTYPE(build_dbsum,
	  extern int,
	  (struct INTF *,
	   struct NBR *));

/* ospf_build_ls.c */
PROTOTYPE(build_rtr_lsa,
	  extern int,
	  (struct AREA *,
	   struct LSDB_LIST **,
	   int));
PROTOTYPE(build_net_lsa,
	  extern int,
	  (struct INTF *,
	   struct LSDB_LIST **,
	   int));
PROTOTYPE(build_sum_net,
	  extern int,
	  (struct AREA *));
PROTOTYPE(build_sum_asb,
	  extern int,
	  (struct AREA *,
	   struct OSPF_ROUTE *,
	   struct AREA *));
PROTOTYPE(build_sum,
	  extern int,
	  (void));
PROTOTYPE(build_inter,
	  extern int,
	  (struct LSDB *,
	   struct AREA *,
	   int));
PROTOTYPE(build_sum_dflt,
	  extern void,
	  (struct AREA *));
PROTOTYPE(beyond_max_seq,
	  extern int,
	  (struct AREA *,
	   struct INTF *,
	   struct LSDB *,
	   struct LSDB_LIST **,
	   struct LSDB_LIST **,
	   int));

/* ospf_choose_dr.c */
PROTOTYPE(ospf_choose_dr,
	  extern void,
	  (struct INTF *));

/* ospf_flood.c */
PROTOTYPE(area_flood,
	  extern void,
	  (struct AREA *,
	   struct LSDB_LIST *,
	   struct INTF *,
	   struct NBR *,
	   int));
PROTOTYPE(self_orig_area_flood,
	  extern int,
	  (struct AREA *,
	   struct LSDB_LIST *,
	   int));

/* ospf_gated_rxmon.c */
PROTOTYPE(ospf_rx_mon,
	  extern int,
	  (struct MON_HDR *,
	   struct INTF *,
	   sockaddr_un *,
	   sockaddr_un *,
	   size_t));

/* ospf_rxlinkup.c */
PROTOTYPE(ospf_rx_lsupdate,
	  extern int,
	  (struct LS_UPDATE_HDR *,
	   struct INTF *,
	   sockaddr_un *,
	   sockaddr_un *,
	   size_t));

/* ospf_rxpkt.c */
PROTOTYPE(ospf_rxpkt,
	  extern void,
	  (struct ip *,
	   struct OSPF_HDR *,
	   sockaddr_un *,
	   sockaddr_un *));

/* ospf_gated_trace.c */
PROTOTYPE(ospf_trace,
	  extern void,
	  (struct OSPF_HDR *,
	   size_t,
	   u_int,
	   int,
	   struct INTF *,
	   sockaddr_un *,
	   sockaddr_un *));

PROTOTYPE(ospf_trace_build,
	  extern void,
	  (struct AREA *,
	   struct AREA *,
	   union LSA_PTR,
	   int));


#define	findroute(rt, dest, mask) \
{ \
    rt = rt_locate(RTS_NETROUTE, \
		   sockbuild_in(0, (dest)), \
		   inet_mask_locate((mask)), \
		   (flag_t) (RTPROTO_BIT(RTPROTO_OSPF)|RTPROTO_BIT(RTPROTO_OSPF_ASE))); \
    if (rt && BIT_TEST(rt->rt_state, RTS_DELETE)) { \
	rt = (rt_entry *) 0; \
    } \
}

PROTOTYPE(rvbind,
	  extern void,
	  (rt_entry *rt,
	   struct LSDB *v,
	   struct AREA *a));
PROTOTYPE(ospf_route_update,
	  extern void,
	  (rt_entry *rt,
	   struct AREA *a,
	   int level));
PROTOTYPE(ntab_update,
	  extern void,
	  (struct AREA *,
	   int));
PROTOTYPE(build_route,
	  extern int,
	  (struct AREA *,
	   struct LSDB *,
	   rt_data *));

/* ospf_states.c */
_PROTOTYPE(if_trans[NINTF_EVENTS][NINTF_STATES],
	  extern void,
	  (struct INTF *));
_PROTOTYPE(nbr_trans[NNBR_EVENTS][NNBR_STATES],
	  extern void,
	  (struct INTF *,
	   struct NBR *));

/* ospf_spf.c */
PROTOTYPE(add_nh_entry,
	  extern struct NH_BLOCK *,
	  (if_addr *,
	   u_int32,
	   int));
PROTOTYPE(alloc_nh_entry,
	  extern struct NH_BLOCK *,
	  (struct NH_BLOCK *));
PROTOTYPE(free_nh_entry,
	  extern void,
	  (struct NH_BLOCK **));
PROTOTYPE(freeplist,
	  extern void,
	  (struct LSDB *));
PROTOTYPE(ospf_add_parent,
	  extern int,
	  (struct LSDB *,
	   struct LSDB *,
	   u_int32,
	   struct AREA *,
	   u_int32,
	   struct NH_BLOCK *,
	   struct AREA *));
PROTOTYPE(run_spf,
	  extern void,
	  (struct AREA *,
	   int));

/* ospf_spf_leaves.c */
PROTOTYPE(netsum,
	  extern int,
	  (struct AREA *,
	   int,
	   struct AREA *,
	   int));
PROTOTYPE(asbrsum,
	  extern int,
	  (struct AREA *,
	   int,
	   struct AREA *,
	   int));
PROTOTYPE(ase,
	  extern int,
	  (struct AREA *,
	   int,
	   int));

/* ospf_gated_rxmon.c */
PROTOTYPE(ospf_log_rx,
	  extern void,
	  (int,
	   sockaddr_un *,
	   sockaddr_un *));
PROTOTYPE(ospf_addr2str,
	  extern sockaddr_un *,
	  (sockaddr_un *));

/* ospf_newq.c */
PROTOTYPE(rem_db_ptr,
	  extern int,
	  (struct NBR *,
	   struct LSDB *));
PROTOTYPE(rem_nbr_ptr,
	  extern int,
	  (struct LSDB *,
	   struct NBR *));
PROTOTYPE(ospf_freeq,
	  extern void,
	  (struct Q **,
	   int type));
PROTOTYPE(add_nbr_retrans,
	  extern void,
	  (struct NBR *,
	   struct LSDB *));
PROTOTYPE(add_db_retrans,
	  extern void,
	  (struct LSDB *,
	   struct NBR *));
PROTOTYPE(rem_db_retrans,
	  extern void,
	  (struct LSDB *));
PROTOTYPE(rem_nbr_retrans,
	  extern void,
	  (struct NBR *));
PROTOTYPE(freeDbSum,
	  extern void,
	  (struct NBR *));
PROTOTYPE(freeLsReq,
	  extern void,
	  (struct NBR *));
PROTOTYPE(freeAckList,
	  extern void,
	  (struct INTF *));

/* ospf_txpkt.c */
PROTOTYPE(send_hello,
	  extern void,
	  (struct INTF *,
	   struct NBR *,
	   int));
PROTOTYPE(send_exstart,
	  extern void,
	  (struct INTF *,
	   struct NBR *,
	   int));
PROTOTYPE(send_dbsum,
	  extern void,
	  (struct INTF *,
	   struct NBR *,
	   int));
PROTOTYPE(send_req,
	  extern void,
	  (struct INTF *,
	   struct NBR *,
	   int));
PROTOTYPE(send_ack,
	  extern int,
	  (struct INTF *,
	   struct NBR *,
	   struct LS_HDRQ *));
PROTOTYPE(send_lsu,
	  extern int,
	  (struct LSDB_LIST *,
	   int hash,
	   struct NBR *,
	   struct INTF *,
	   int));
PROTOTYPE(send_ase,
	  extern void,
	  (struct LSDB *,
	   struct LSDB *));

PROTOTYPE(nbr_enq,
	  extern void,
	  (struct INTF *,
	   struct NBR *));
PROTOTYPE(range_enq,
	  extern void,
	  (struct AREA *,
	   struct NET_RANGE *));
PROTOTYPE(host_enq,
	  extern void,
	  (struct AREA *,
	   struct OSPF_HOSTS *));

#ifdef	notdef
PROTOTYPE(ospf_discard_delete,
	  extern void,
	  (struct NET_RANGE *));
PROTOTYPE(ospf_discard_add,
	  extern int,
	  (struct NET_RANGE *));
#endif	/* notdef */


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
