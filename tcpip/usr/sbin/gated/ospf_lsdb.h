/* @(#)96	1.1  src/tcpip/usr/sbin/gated/ospf_lsdb.h, tcprouting, tcpip411, GOLD410 12/6/93 17:26:13 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ABRTR_ACTIVE
 *		ACK_QUEUE_FULL
 *		ADD_ACK
 *		ADD_ACK_INTF
 *		ADD_DBQ
 *		ADD_Q
 *		ADV_AGE
 *		ADV_RTR
 *		ASBRTR_ACTIVE
 *		ASE_COST_EQUAL
 *		ASE_COST_LESS
 *		AddLSA
 *		DB_AB_RTR
 *		DB_ADV
 *		DB_ASB_RTR
 *		DB_ASE
 *		DB_ASE_FORWARD
 *		DB_ASE_TAG
 *		DB_BDR
 *		DB_BDR_AB
 *		DB_BDR_ASB
 *		DB_CAN_BE_FREED
 *		DB_COUNT
 *		DB_DIRECT
 *		DB_DIST
 *		DB_FREEME
 *		DB_GUTS
 *		DB_LS_HDR
 *		DB_MASK
 *		DB_MYHASH
 *		DB_MY_AREA
 *		DB_NET
 *		DB_NETNUM
 *		DB_NEXT
 *		DB_NH_CNT
 *		DB_NH_NDX
 *		DB_PTR
 *		DB_RERUN
 *		DB_RETRANS
 *		DB_ROUTE
 *		DB_RTR
 *		DB_SEQ_MAX
 *		DB_SUM
 *		DB_TIME
 *		DB_TRANS_AREA
 *		DB_VIRTUAL
 *		DB_WHERE
 *		DEL_DBQ
 *		DEL_Q
 *		EN_DBQ
 *		EN_Q
 *		FindLSA
 *		GOT_A_BDR
 *		GOT_GUTS
 *		LS_AGE
 *		LS_ASE_TAG
 *		LS_CKS
 *		LS_ID
 *		LS_LEN
 *		LS_SEQ
 *		LS_TYPE
 *		NO_GUTS
 *		PROTOTYPE
 *		REM_DBQ
 *		REM_Q
 *		XAddLSA
 *		XFindLSA
 *		XHASH
 *		XHASH_ASE
 *		XHASH_LSA
 *		XHASH_QUEUE
 *		XThash
 *		XXhash
 *		Xkey1
 *		Xkey2
 *		Xtype
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
 * ospf_lsdb.h,v 1.13 1993/03/25 12:47:10 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/*
 *	 STRUCTURES FOR THE SPF ALGORITHM
 */


/* Link State Database */


#define HTBLSIZE	251
#define HTBLMOD		(HTBLSIZE)	/* sb prime (or there aboutst) */
#define XHASH(A1,A2)	((A1) % HTBLMOD)

#define	OSPF_HASH_QUEUE	31
/* Hash size for queues */
#define	XHASH_QUEUE(lsdb)	(LS_ID(lsdb) % OSPF_HASH_QUEUE)
#ifdef	notdef
#define	OSPF_LSA_HASH	31		/* Hash size for LSAs in general */
#define	OSPF_ASE_HASH	251		/* Hash size for ASEs in particular */

#define	XHASH_LSA(A1, A2)	((A1) % OSPF_LSA_HASH)
#define	XHASH_ASE(A1, A2)	((A1) % OSPF_ASE_HASH)
#endif	/* notdef */


union LSA_PTR {			/* advertisements */
    struct RTR_LA_HDR *rtr;
    struct NET_LA_HDR *net;
    struct SUM_LA_HDR *sum;
    struct ASE_LA_HDR *ase;
    struct GM_LA_HDR *gm;
};

#define ADVNULL ((struct RTR_LA_HDR *) 0)

#define DB_CAN_BE_FREED(DB)\
	(DB_FREEME(DB) &&\
	(!(DB_ROUTE(DB) || DB_ASB_RTR(DB))) &&\
	((!DB_RETRANS(DB))) &&\
	(ospf.nbrEcnt == ospf.nbrFcnt))

#define ADV_RTR(DB) 	((DB)->adv.rtr->ls_hdr.adv_rtr)
#define ADV_AGE(DB)	(LS_AGE(DB) + (time_sec - DB_TIME(DB)))
#define LS_AGE(DB) 	(DB->adv.rtr->ls_hdr.ls_age)
#define LS_ID(DB) 	((DB)->adv.rtr->ls_hdr.ls_id)
#define LS_TYPE(DB) 	((DB)->adv.rtr->ls_hdr.ls_type)
#define LS_SEQ(DB) 	((DB)->adv.rtr->ls_hdr.ls_seq)
#define LS_CKS(DB) 	ntohs((DB)->adv.rtr->ls_hdr.ls_chksum)
#define LS_LEN(DB) 	((DB)->adv.rtr->ls_hdr.length)
#define	LS_ASE_TAG(V)	DB_ASE((V))->tos0.ExtRtTag

#define GOT_GUTS(DB)	((DB_GUTS(DB) != DBGUTSNULL))
#define NO_GUTS(DB)	((DB_GUTS(DB) == DBGUTSNULL))
#define DB_GUTS(DB)	((DB)->un2.dbguts)
#define DB_PTR(DB) 	((DB)->un2.dbguts)->ptr
#define DB_TIME(DB) 	((DB)->un2.dbguts->time_stamp)
#define DB_NEXT(DB)	((DB)->lsdbnext)
#define DB_WHERE(DB) 	((DB)->un2.dbguts->where)
#define	DB_MY_AREA(DB)	((DB)->un2.dbguts->my_area)
#define DB_DIRECT(DB) 	((DB)->un2.dbguts->direct)
#define DB_VIRTUAL(DB) 	((DB)->un2.dbguts->virtual)
#define DB_FREEME(DB) 	((DB)->un2.dbguts->freeme)
#define DB_SEQ_MAX(DB) 	((DB)->un2.dbguts->seq_max)
#define DB_NH_CNT(DB) 	((DB)->un2.dbguts->nhcnt)
#define DB_NH_NDX(DB, I) 	((DB)->un2.dbguts->nhndx[I])
#define DB_DIST(DB) 	((DB)->un2.dbguts->dist)
#define DB_COUNT(DB) 	((DB)->un2.dbhinfo.count)
#define DB_RERUN(DB) 	((DB).un2.dbhinfo.rerunflag)
#define DB_MYHASH(DB) 	((DB)->un2.dbguts->myhash)
#define DB_TRANS_AREA(DB) ((DB)->un2.dbguts->trans_area)
#define DB_ADV(DB) 	((DB)->adv)
#define DB_RTR(DB) 	((DB)->adv.rtr)
#define DB_NET(DB) 	((DB)->adv.net)
#define DB_SUM(DB) 	((DB)->adv.sum)
#define DB_ASE(DB) 	((DB)->adv.ase)
#define DB_LS_HDR(DB)	(DB_RTR(DB)->ls_hdr)
#define DB_MASK(DB) 	(DB_NET(DB)->net_mask)
#define DB_NETNUM(DB)  	(LS_ID(DB) & DB_MASK(DB))
#define DB_RETRANS(DB) 	((DB)->un2.dbguts->retrans)
#define DB_BDR(DB)	((DB)->border)
#define DB_ROUTE(DB) 	((DB)->un1.route)
#define DB_AB_RTR(DB) 	((DB)->un1.ab_rtr)
#define DB_ASB_RTR(DB) 	((DB)->asb_rtr)
#define	DB_BDR_AB(L)	(DB_AB_RTR(DB_BDR(L)))
#define	DB_BDR_ASB(L)	(DB_ASB_RTR(DB_BDR(L)))
#define	DB_ASE_TAG(DB)	(DB_ASE((DB))->tos0.ExtRtTag)
#define	DB_ASE_FORWARD(DB)	(DB_ASE((DB))->tos0.ForwardAddr)

/*
 * The guts of the lsdb structure
 */
struct DBGUTS {
    struct LSDB *ptr[2];	/* for candidate, sum and ase list */
    flag_t where:8,		/* where this vertex is: on candidatelst or
				 * spftree for ls_ase could be on
				 * ase_infinity or ase_list */
#define 	UNINITIALIZED	0
#define	ON_CLIST	1
#define 	ON_SPFTREE	2
#define 	ON_RTAB		2
#define  ON_SUMASB_LIST	3	/* reachable asb from attached area */
#define  ON_SUMNET_LIST	4	/* reachable net from attached area */
#define  ON_INTER_LIST	5	/* on inter-area list - imported from bb */
#define	ON_SUM_INFINITY 6
#define  ON_ASE_LIST	7
#define  ON_ASE_INFINITY 8
	direct:1,		/* net attached to this node */
        freeme:1,		/* flag to free this lsdb entry */
        seq_max:1,		/* note if entry has reached max seq number */
        virtual:1;
    struct AREA *trans_area;	/* For resolution of virtual nbrs */
    struct AREA *my_area;	/* for keeping count of db's in each area */
    u_int16 myhash;		/* this db's hash */
    u_int16 nhcnt;		/* number of in-use parents (< MAXNH) */
    struct NH_BLOCK *nhndx[MAXNH];	/* list of next hops */
    u_int32 dist;		/* distance to root */
    time_t time_stamp;		/* for keeping age - stamped when arrived */
    struct NBR_LIST *retrans;	/* nbrs pointing to this lsdb */
};
/*
 * If greater than 255 possible nexthops, nhndx should be changed to a short
 */

#define DBGUTSNULL ((struct DBGUTS *)0)

struct DBHEAD_INFO {
    u_int16 rerunflag;	/* rerun this row - partial update */
    u_int16 count;		/* count of lsdb elts; used in head */
};

struct LSDB {
    struct LSDB *lsdbnext;	/* LSDB list */
    u_int32 key[2];		/* LSDB keys - ls_id & adv rtr */
    union LSA_PTR adv;		/* advertisement */
    union {
	rt_entry *route;	/* pointer to the routing table entry */
	struct OSPF_ROUTE *ab_rtr;	/* Area bdr rtr */
    }     un1;
    struct OSPF_ROUTE *asb_rtr;	/* if it is ASB Router */
    struct LSDB *border;	/* sum or ase - lsdb of border rtr */
    union UN {
	struct DBHEAD_INFO dbhinfo;
	struct DBGUTS *dbguts;
    }  un2;
};

#define  GOT_A_BDR(L) 	   (GOT_GUTS(L) && DB_BDR(L))
#define  ABRTR_ACTIVE(L)   (GOT_GUTS(L) && DB_BDR_AB(L))
#define  ASBRTR_ACTIVE(L)  (GOT_GUTS(L) && DB_BDR_ASB(L))

#define LSDBNULL	((struct LSDB *)0)

/* structures for keeping track of retransmission lists */

/* lsdb keep track of nbrs pointing to it for tx and retx */
struct NBR_LIST {
    struct NBR_LIST *ptr[2];
    struct NBR *nbr;
};

#define NLNULL	((struct NBR_LIST *) 0)

/* General queue structure for sending advertisements */
struct LSDB_LIST {
    struct LSDB_LIST *ptr[2];
    struct LSDB *lsdb;
    int flood;			/* true if flooding this one */
};

#define LLNULL	((struct LSDB_LIST *) 0)


/* Return values for nbr_ret_req */
#define REQ_NOT_FOUND 		0
#define REQ_LESS_RECENT 	1
#define REQ_SAME_INSTANCE 	2
#define REQ_MORE_RECENT 	3

#define ASE_COST_LESS(A_ETYPE,A_COST,A_TYPE2COST,B_ETYPE,B_COST,B_TYPE2COST)\
    ( ((!A_ETYPE) && (B_ETYPE)) ||\
     ( ((!A_ETYPE) && (!B_ETYPE)) &&\
      ((A_COST) < (B_COST)) ) ||\
     ( ((A_ETYPE) && (B_ETYPE)) &&\
      ( ((A_TYPE2COST) < (B_TYPE2COST)) ||\
       (((A_TYPE2COST) == (B_TYPE2COST)) && ((A_COST) < (B_COST))))))

#define ASE_COST_EQUAL(A_ETYPE,A_COST,A_TYPE2COST,B_ETYPE,B_COST,B_TYPE2COST)\
	( ((A_ETYPE) == (B_ETYPE)) &&\
	  ((A_COST) == (B_COST)) &&\
	  ((A_TYPE2COST) == (B_TYPE2COST)) )

/* Queue general hdrs */
#define NEXT 0
#define LAST 1

struct Q {			/* an empty shell for general doubly-linked
				 * queueing */
    struct Q *ptr[2];		/* 0 is foward ptr, 1 is back ptr */
};

#define QNULL ((struct Q *)0)

/* Queue of LS_HDRs */

struct LS_HDRQ {
    struct LS_HDRQ *ptr[2];
    struct LS_HDR ls_hdr;
};

/**/
/* Ack list manipulation */

#define	ADD_ACK(qhp, db) \
	{ \
	    struct LS_HDRQ *ack; \
	    HDRQ_ALLOC(ack); \
	    ack->ls_hdr = DB_LS_HDR(db); \
	    GHTONS(ack->ls_hdr.ls_age); \
	    ADD_Q((qhp), ack); \
	}

#define	ADD_ACK_INTF(intf, db) \
	{ \
	    ADD_ACK(&intf->acks, db); \
	    intf->ack_cnt++; \
	}

#define	ACK_QUEUE_FULL(intf)	((OSPF_HDR_SIZE + (intf->ack_cnt + 1) * ACK_PIECE_SIZE) > INTF_MTU(intf))

/**/
/* LSDB Access */

#define Xkey1(A) ((A)->ls_hdr.ls_id)
#define Xkey2(A) ((A)->ls_hdr.adv_rtr)
#define Xtype(A) ((A)->ls_hdr.ls_type)
#define XXhash(A) (XHASH(Xkey1(A),Xkey1(A)))

#define XThash(K1,K2,T) (XHASH(K1,K1))

#define XAddLSA(DB,Area,X,Data) addLSA(DB, Area, Xkey1(X), Xkey2(X), Data, (u_int) Xtype(X))

#define AddLSA(DB,Area,K1,K2,Data,Typ) addLSA(DB, Area, K1, K2, Data, (u_int) Typ)

#define XFindLSA(Area,X)\
     	(struct LSDB *)findLSA(&Area->htbl[Xtype(X)][XXhash(X)],\
		Xkey1(X), Xkey2(X), (u_int) Xtype(X))

#define FindLSA(Area,K1,K2,Typ)\
  	(struct LSDB *)findLSA(&Area->htbl[Typ][XThash(K1,K2,Typ)], K1, K2, (u_int) Typ)

/*
 * Add Q to the queue
 */
#define ADD_Q(PREV,Q) {\
	if ((PREV)->ptr[NEXT]) {\
	    (PREV)->ptr[NEXT]->ptr[LAST] = (Q);\
	}\
	(Q)->ptr[NEXT] = (PREV)->ptr[NEXT];\
	(PREV)->ptr[NEXT] = (Q);\
	(Q)->ptr[LAST] = (PREV);}

/*
 * Add DB to the previous LSDB entry
 */
#define ADD_DBQ(PREV,DB) {\
	if (DB_PTR(PREV)[NEXT]) {\
	    DB_PTR(DB_PTR(PREV)[NEXT])[LAST] = (DB);\
	}\
	DB_PTR(DB)[NEXT] = DB_PTR(PREV)[NEXT];\
	DB_PTR(PREV)[NEXT] = (DB);\
	DB_PTR(DB)[LAST] = (PREV);}


/*
 * Remove Q from the queue
 */
#define DEL_Q(Q,NK,TYPE) {\
	if ((Q)->ptr[LAST]) (Q)->ptr[LAST]->ptr[NEXT] = (Q)->ptr[NEXT];\
	if ((Q)->ptr[NEXT]) (Q)->ptr[NEXT]->ptr[LAST] = (Q)->ptr[LAST];\
	(Q)->ptr[NEXT] = ((Q)->ptr[LAST] = 0);\
	if (NK) FREE(((char *)(Q)),TYPE);}

/*
 * Remove DB from the queue
 */
#define DEL_DBQ(DB) {\
      if (DB_PTR(DB)[LAST]) DB_PTR(DB_PTR(DB)[LAST])[NEXT] = DB_PTR(DB)[NEXT];\
      if (DB_PTR(DB)[NEXT]) DB_PTR(DB_PTR(DB)[NEXT])[LAST] = DB_PTR(DB)[LAST];\
      DB_PTR(DB)[NEXT] = (DB_PTR(DB)[LAST] = LSDBNULL);}


/*
 * These queue macros assume that the queue doesn't have a head
 */
#define EN_DBQ(QHP,DB) {\
	if (!(QHP)) {\
		(QHP) = DB; DB_PTR(DB)[LAST] = LSDBNULL;\
	} else ADD_DBQ((QHP),DB);}

#define REM_DBQ(QHP,DB) {\
	if ((QHP) == (DB)) {\
		QHP = DB_PTR((DB))[NEXT];\
        if (QHP) DB_PTR((QHP))[LAST] = LSDBNULL;\
  		DB_PTR(DB)[NEXT] = (DB_PTR(DB)[LAST] = LSDBNULL);\
	} else DEL_DBQ((DB));}

#define EN_Q(QHP,Q) {\
	if ((!QHP)){\
		(QHP) = (Q); (Q)->ptr[LAST] = 0;\
	} else ADD_Q((QHP),(Q));}

#define REM_Q(QHP,Q,NK,TYPE) {\
            if ((QHP) == (Q)) {\
                (QHP) = (Q)->ptr[NEXT];\
                if (QHP) (QHP)->ptr[LAST] = 0;\
                if (NK) FREE(((char *)(Q)),TYPE);\
                else (Q)->ptr[NEXT] = ((Q)->ptr[LAST] = 0);\
            } else DEL_Q((Q), (NK), TYPE);}
#ifdef	notdef
#define REM_Q(QHP,Q,NK,TYPE) {\
	    if ((QHP) == (Q)) {\
		(QHP) = (Q)->ptr[NEXT];\
		if (QHP) (QHP)->ptr[LAST] = 0;\
	    } else DEL_Q((Q), TRUE, TYPE);\
	    if (NK) FREE(((char *)(Q)),TYPE);\
      	    else (Q)->ptr[NEXT] = ((Q)->ptr[LAST] = 0);}
#endif

/* Function prototypes */
PROTOTYPE(addLSA,
	  extern int,
	  (struct LSDB **,
	   struct AREA *,
	   u_int32,
	   u_int32,
	   void_t,
	   u_int));
PROTOTYPE(ospf_add_stub_lsa,
	  extern int,
	  (struct LSDB **,
	   struct AREA *,
	   u_int32,
	   u_int32,
	   u_int32));
PROTOTYPE(findLSA,
	  extern struct LSDB *,
	  (struct LSDB *,
	   u_int32,
	   u_int32,
	   u_int));
PROTOTYPE(db_free,
	  extern void,
	  (struct LSDB *));
#ifdef	notdef
PROTOTYPE(add_ack,
	  extern void,
	  (struct LS_HDRQ *,
	   struct LSDB *));
#endif	/* notdef */
PROTOTYPE(nbr_rem_req,
	  extern int,
	  (struct NBR *,
	   union LSA_PTR));


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
