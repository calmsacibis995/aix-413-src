/* @(#)91	1.1  src/tcpip/usr/sbin/gated/ospf_gated_mem.h, tcprouting, tcpip411, GOLD410 12/6/93 17:26:01 */
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ADV_ALLOC
 *		ADV_COPY
 *		ADV_FREE
 *		ADV_REPLACE
 *		AREA_ALLOC
 *		AREA_FREE
 *		ASE_HDR_ALLOC
 *		ASE_TOS_CMP
 *		BufCheckAck
 *		BufCheckAddLSDB
 *		BufCheckAse
 *		BufCheckBuildNet
 *		BufCheckBuildRtr
 *		BufCheckDbParse
 *		BufCheckLL
 *		BufCheckNet
 *		BufCheckNewADV
 *		BufCheckNotIntra
 *		BufCheckOldADV
 *		BufCheckOldNotIntra
 *		BufCheckReBuildNet
 *		BufCheckRetrans
 *		BufCheckRtrAdv
 *		BufCheckStub
 *		BufCheckSumHdr
 *		CALLOC
 *		CLEAR_BUF
 *		DBADV_FREE
 *		DBBLOCK_ALLOC
 *		DBGUTS_ALLOC
 *		DBGUTS_FREE
 *		DBS_ALLOC
 *		DB_ALLOC
 *		DB_FREE_PKT
 *		FREE
 *		GET_BLOCK_SIZE
 *		HDRQ_ALLOC
 *		HOST_ALLOC
 *		INTF_ALLOC
 *		INTF_FREE
 *		LL_ALLOC
 *		LS_REQ_ALLOC
 *		MALLOC
 *		MY_NET_ADV_SIZE
 *		MY_RTR_ADV_SIZE
 *		NBR_ALLOC
 *		NBR_INIT_ALLOC
 *		NET_ATTRTR_CMP
 *		NET_HDR_ALLOC
 *		NET_INIT_ALLOC
 *		NL_ALLOC
 *		NR_ALLOC
 *		ORT_COPY
 *		ORT_INFO_ALLOC
 *		OSPF_PKT_ALLOC
 *		OTIMER_ALLOC
 *		PROTOTYPE
 *		REALLOC
 *		RRT_ALLOC
 *		RTR_HDR_ALLOC
 *		RTR_LINK_CMP
 *		RT_CNT
 *		STATIC_INFO_ALLOC
 *		STUB_HDR_ALLOC
 *		SUM_HDR_ALLOC
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
 * ospf_gated_mem.h,v 1.10 1993/02/09 18:57:42 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


/*
 * Mem types to be managed
 */
#define OMEM_ADV800	1
#define OMEM_ADV128	2
#define OMEM_ADV64	3
#define OMEM_ADV36	6
#define OMEM_RTRHDR	1	/* adv */
#define OMEM_NETHDR	1
#define OMEM_NBR	4	/* for nbr struct */
#define OMEM_DBGUTS	5	/* for dbguts part of lsdb entry */
#define OMEM_RRT	5
#define OMEM_ORT_INFO	5
#define OMEM_STUBHDR	6	/* 36 bytes */
#define OMEM_SUMNETHDR	6
#define OMEM_SUMASBHDR	6
#define OMEM_ASEHDR	6
#define OMEM_ASEDFLTHDR	6
#define OMEM_NET	6
#define	OMEM_LS_REQ	6
#define OMEM_HOST	6
#define OMEM_TIMER	6
#define OMEM_ACKLST	7	/* 16 bytes; most of the hdr queues used */
#define OMEM_LL		7
#define OMEM_LSDB	7
#define OMEM_NL		8	/* 12 bytes; smallest hdr queues */
#define OMEM_DBSUM	8


#define OMEM_PKT	10
#define OMEM_AREA	11
#define OMEM_INTF	12

PROTOTYPE(dbsum_alloc,
	  extern struct LSDB_SUM *,
	  (int));
/*
 * CLEAR BUFFER CALL
 */
#define CLEAR_BUF(B,LEN) bzero((caddr_t) B, LEN)

#define RT_CNT(A,T) (((T) == LS_ASE) ? ospf.nbrcnt : (A)->nbrEcnt)

/*
 *	General alloc and free routine
 */
#undef	FREE
#define FREE(PTR, TYPE)	(task_mem_free(ospf.task, (caddr_t) PTR))

#undef	REALLOC
#define	REALLOC(PTR, LEN)	task_mem_realloc(ospf.task, PTR, (size_t) (LEN));

#undef	CALLOC
#define	CALLOC(NUM, LEN)	task_mem_calloc(ospf.task, NUM, (size_t) (LEN));

#undef	MALLOC
#define	MALLOC(LEN)		task_mem_malloc(ospf.task, (size_t) (LEN));


#define	DB_FREE_PKT(N) FREE((N)->dbpkt, OMEM_PKT)


/*
 * sizeof this area's router advertisement
 */
#define MY_RTR_ADV_SIZE(A)\
	(RTR_LA_HDR_SIZE +\
	  (((A)->ifcnt * RTR_LA_PIECES_SIZE) * 2) +\
	  (((A)->hostcnt * RTR_LA_PIECES_SIZE)) +\
	  (((A)->area_id == OSPF_BACKBONE) ? ((ospf.vcnt * RTR_LA_PIECES_SIZE)) : 0))

/*
 * sizeof this area's network advertisement
 */
#define MY_NET_ADV_SIZE(I)\
	 (NET_LA_HDR_SIZE + (((I)->nbrIcnt + 1) * NET_LA_PIECES_SIZE))

/*
 * Get block size for adv based on type and length - currently only do TOS0
 */
#define GET_BLOCK_SIZE(TYPE,LEN) 1

/*
 * OSPF PACKET
 */
#define OSPF_PKT_ALLOC(P,LEN) P = (struct OSPF_HDR *) CALLOC(1, (LEN));

/*
 * LSDB structs
 */
#define DBGUTS_ALLOC(DB)	DB_GUTS(DB) = (struct DBGUTS *) CALLOC(1, sizeof(struct DBGUTS));

#define DBGUTS_FREE(DB)	{FREE(DB_GUTS(DB), OMEM_DBGUTS); DB_GUTS(DB) = DBGUTSNULL;}

#define DB_ALLOC(DB)	DB = (struct LSDB *) CALLOC(1, sizeof(struct LSDB));


/*
 *	Header queues
 */

#define	HDRQ_ALLOC(A) A = (struct LS_HDRQ *) CALLOC(1, sizeof (*(A)))

/*
 * Link State Advertisements
 */
#define ADV_ALLOC(DB,TYPE,LEN) \
		if ((TYPE) == LS_NET || (TYPE) == LS_RTR) { \
			DB_NET(DB) = (struct NET_LA_HDR *) CALLOC(1, LEN); \
		} else {DB_NET(DB) = (struct NET_LA_HDR *) CALLOC(1 ,LEN); \
		}

#define DBADV_FREE(DB,TYPE) FREE(DB_RTR(DB), TYPE)

#define ADV_FREE(A,TYPE) FREE(A, TYPE)

/*
 * COPY CALLS
 */
#define ADV_COPY(FROM,TO,LEN) bcopy((caddr_t) FROM, (caddr_t) TO, (size_t) (LEN))

#define ORT_COPY(FROM,TO) bcopy(FROM, TO, sizeof(OSPF_RT_INFO))

/*
 * REPLACE CALL
 */
#define ADV_REPLACE(DB,ADV,LEN) {\
		if ((LEN) > ntohs(LS_LEN(DB))) {\
			DBADV_FREE((DB),LS_TYPE(DB));\
	     		ADV_ALLOC((DB), LS_TYPE(DB),(LEN));\
		}\
	     	ADV_COPY((ADV).rtr,DB_RTR(DB),(LEN));}

/*
 * COMPARE CALLS
 */
#define RTR_LINK_CMP(R1,R2,LEN)\
	bcmp((caddr_t) &((R1)->link), (caddr_t) &((R2)->link), (size_t) (LEN))

#define NET_ATTRTR_CMP(N1,N2,LEN)\
	bcmp((caddr_t) &((N1)->att_rtr), (caddr_t) &((N2)->att_rtr), (size_t) (LEN))

#define ASE_TOS_CMP(T1,T2) bcmp((caddr_t) (T1), (caddr_t) (T2), ASE_LA_PIECES_SIZE)


/*
 * LSDB List
 */
#define LL_ALLOC(LL)	LL = (struct LSDB_LIST *) CALLOC(1 ,sizeof(struct LSDB_LIST));

/*
 * Self-originated LSA allocs
 */

/*
 * STUBNET LSA HDR
 */
#define STUB_HDR_ALLOC(N,LEN) N = (struct NET_LA_HDR *) CALLOC(1, LEN);

/*
 * RTR LSA HDR
 */
#define	RTR_HDR_ALLOC(R,LEN) R = (struct RTR_LA_HDR *) CALLOC(1, LEN);

/*
 * NET LSA HDR
 */
#define NET_HDR_ALLOC(N,LEN) N = (struct NET_LA_HDR *) CALLOC(1, LEN);

/*
 * SUM LSA HDR
 */
#define SUM_HDR_ALLOC(S,LEN) S = (struct SUM_LA_HDR *) CALLOC(1, LEN);

/*
 * ASE LSA HDR
 */
#define ASE_HDR_ALLOC(A,LEN) A = (struct ASE_LA_HDR *) CALLOC(1, LEN);

/*
 * DB SUM
 */
#define DBS_ALLOC(D)	D = (struct LSDB_SUM *) CALLOC(1, sizeof(struct LSDB_SUM));

/*
 * OTIMER STRUCTURE
 */
#define OTIMER_ALLOC(T)	T = (struct OTIMER *) CALLOC(1, sizeof(struct OTIMER));

/*
 * OSPF ROUTE INFO
 */
#define ORT_INFO_ALLOC(R)	(ORT_INFO(R)) = (OSPF_RT_INFO *) CALLOC(1, sizeof(OSPF_RT_INFO));

/*
 * STATIC ROUTE INFO
 */
#define STATIC_INFO_ALLOC(R)	(STATIC_INFO(R)) = (STATIC_RT_INFO *) CALLOC(1, sizeof(STATIC_RT_INFO));

/*
 * OSPF ROUTER ROUTE ENTRY
 */
#define RRT_ALLOC(R)	R = (struct OSPF_ROUTE *) CALLOC(1, sizeof(struct OSPF_ROUTE));

/*
 * NBR's LS REQ
 */
#define	LS_REQ_ALLOC(R)	R = (struct LS_REQ *) CALLOC(1, sizeof(struct LS_REQ));

/*
 * LSDB's NBR LIST
 */
#define NL_ALLOC(NL)	NL = (struct NBR_LIST *) CALLOC(1, sizeof(struct NBR_LIST));

/*
 * AREA's NET list ALLOC
 */
#define NR_ALLOC(N)	N = (struct NET_RANGE *) CALLOC(1, sizeof(struct NET_RANGE));

/*
 * NBR STRUCT ALLOC
 */
#define NBR_ALLOC(N)	N = (struct NBR *) CALLOC(1, sizeof(struct NBR));

/*
 * BufChecks allow a port with limited memory (relative to a virtual machine)
 * to determine if there is enough memory to complete an event
 */

/* LSDB and GUTS */
#define BufCheckAddLSDB(CNT)\
	TRUE
 /* (add_stash(OMEM_LSDB,(CNT)) & add_stash(OMEM_DBGUTS,(CNT))) */

/* An ack */
#define BufCheckAck(CNT) 	TRUE	/* (add_stash(OMEM_ACKLST,(CNT))) */

/* LL structs */
#define BufCheckLL(CNT)		TRUE	/* (add_stash(OMEM_LL,(CNT))) */

#define BufCheckRetrans(CNT)	TRUE	/* (add_stash(OMEM_NL,((CNT) * 2))) */

/*
 * Check for rtr advertisement flooding queue, LL and retrans queues
 */
#define BufCheckBuildRtr(A,LEN)\
	TRUE
 /*
  * ((add_stash(adv_stash_type((LEN) + 12),1)) &&\ (BufCheckLL(1)) &&\
  * (BufCheckRetrans((A)->nbrEcnt)))
  */

#define BufCheckRtrAdv(A) TRUE	/* (stash_check_rtr_lsa(A)) */


/*
 * Used for !found Net LSAs and Sum LSAs
 * - check to have enough for an LL
 * - check for enough retrans ques
 * - check buffers for other areas if border rtr
 */
#define BufCheckBuildNet(A,LEN)\
	TRUE			/* (((IAmBorderRtr) &&\
				 * ((add_stash(adv_stash_type(LEN),1)) &&\
				 * (BufCheckLL(ospf.acnt)) &&\
				 * (BufCheckRetrans(ospf.nbrcnt)) &&\
				 * (BufCheckAddLSDB(ospf.acnt - 1)) &&\
				 * (BufCheckSumHdr(ospf.acnt - 1)))) ||\
				 * ((!IAmBorderRtr) &&\
				 * ((add_stash(adv_stash_type(LEN),1)) &&\
				 * (BufCheckLL(1)) &&\
				 * (BufCheckRetrans((A)->nbrEcnt))))) */

/*
 * Used for stub nets
 */
#define BufCheckStub(A,CNT)\
	TRUE			/* (((IAmBorderRtr) &&\
				 * ((add_stash(adv_stash_type(NET_LA_HDR_SIZE)
				 * ,(CNT))) &&\ (BufCheckLL((ospf.acnt - 1) *
				 * (CNT))) &&\ (BufCheckRetrans((ospf.nbrcnt) *
				 * CNT)) &&\ (BufCheckAddLSDB((ospf.acnt) *
				 * (CNT))) &&\ (BufCheckSumHdr((ospf.acnt -
				 * 1) * (CNT))))) ||\ ((!IAmBorderRtr) &&\
				 * ((add_stash(adv_stash_type(NET_LA_HDR_SIZE)
				 * ,(CNT))) &&\ (BufCheckAddLSDB(1 *
				 * (CNT)))))) */
/*
 * Check for rebuilding newwork advertisement by tqIntLsa
 */
#define BufCheckReBuildNet(A,LEN)\
	TRUE			/* ((add_stash(adv_stash_type((LEN)),1)) &&\
				 * (BufCheckLL(1)) &&\
				 * (BufCheckRetrans((A)->nbrEcnt))) */

/*
 * tq_IntLsa check for enough to re build internal LSAs
 */
#define BufCheckIntLsa TRUE	/* (stash_check_IntLsa()) */


#define BufCheckSumHdr(CNT) TRUE/* add_stash(OMEM_SUMNETHDR,(CNT)) */

/*
 * Used for newer instances of Net LSAs and Sum LSAs
 * - check to have enough for an LL
 * - check for enough retrans ques
 * - check buffers for other areas if border rtr
 */
#define BufCheckNet(A,LEN,CNT)\
	TRUE			/* (((IAmBorderRtr) &&\
				 * ((add_stash(adv_stash_type(LEN),(CNT)))
				 * &&\ (BufCheckLL(ospf.acnt)) &&\
				 * (BufCheckRetrans(ospf.nbrcnt * (CNT))))
				 * &&\ (BufCheckAddLSDB(ospf.acnt * (CNT)))
				 * &&\ (BufCheckSumHdr(ospf.acnt * (CNT))))
				 * ||\ ((!IAmBorderRtr) &&\
				 * ((add_stash(adv_stash_type(LEN)),(CNT))
				 * &&\ (BufCheckLL(1)) &&\
				 * (BufCheckRetrans((A)->nbrEcnt)) &&\
				 * (BufCheckAddLSDB(1))))) */

/*
 * Check to see that there is enough buffers for the advertisement
 * - Router and net have been handled
 * - More recent is true so will have to originate a new one with a new seq #
 */
#define BufCheckNotIntra(A,LEN,TYPE)\
	TRUE			/* (((TYPE > LS_SUM_ASB) && BufCheckAse(LEN))
				 * ||\ ((TYPE <= LS_SUM_ASB) &&
				 * BufCheckNet(A,LEN,1))) */

#define BufCheckAse(LEN)\
	TRUE			/* ( (add_stash(adv_stash_type((LEN),1))) &&\
				 * (BufCheckLL(1)) &&\
				 * (BufCheckRetrans(ospf.nbrcnt))) */

#define BufCheckOldNotIntra(A,LEN,TYPE)\
	TRUE			/* (((add_stash(adv_stash_type((LEN),1))) &&\
				 * (BufCheckLL(1))) &&\ (((TYPE > LS_SUM_ASB)
				 * && (BufCheckRetrans(ospf.nbrcnt))) ||\
				 * ((TYPE <= LS_SUM_ASB) &&
				 * (BufCheckRetrans((A)->nbrEcnt))))) */

/*
 * TqAseLsa
 */
#define BufCheckReBuildASE\
	TRUE			/* (BufCheckLL(ospf.ospf_export_limit * ospf.nbrcnt) &&\
				 * (BufCheckRetrans(ospf.ospf_export_limit *
				 * ospf.nbrcnt))) */

/*
 * Check for enough space for the advertisement
 */
#define BufCheckNewADV(LEN) TRUE/* (add_stash(adv_stash_type((LEN),1))) */

/*
 * Check for enough space in old adv else in new for the advertisement
 */
#define BufCheckOldADV(DB,LEN)\
	TRUE			/* (((LS_LEN(DB)) >
				 * ospf_buf[DB_BLOCKSIZE(DB)].size) ||\
				 * (add_stash(adv_stash_type((LEN),1)))) */

/*
 * For rxdb
 */
#define BufCheckDbParse(CNT)  TRUE	/* (add_stash(OMEM_LS_REQ,(CNT))) */

/*
 * For RxHello
 */
#define BufCheckNbr	TRUE	/* (add_stash(OMEM_NBR,1)) */


/* Configuration's initialization of structures */



/*
 * AREA Structure
 */
#define AREA_ALLOC(A)	A = (struct AREA *) CALLOC(1, sizeof(struct AREA));

#define AREA_FREE(A)	FREE((caddr_t) A, OMEM_AREA);

#define DBBLOCK_ALLOC(DB,CNT)	DB = (struct LSDB *) CALLOC(CNT, sizeof(struct LSDB));

/*
 * INTF STRUCT ALLOCs
 */
#define INTF_ALLOC(I)	I = (struct INTF *) CALLOC(1 ,sizeof(struct INTF));

#define INTF_FREE(I)	FREE((caddr_t) I, OMEM_INTF);

/*
 * AREA's HOST list ALLOC
 */
#define HOST_ALLOC(H)	H = (struct OSPF_HOSTS *) CALLOC(1 ,sizeof(struct OSPF_HOSTS));


/*
 * AREA's NET list ALLOC
 */
#define NET_INIT_ALLOC(N)	N = (struct NET_RANGE *) CALLOC(1, sizeof(struct NET_RANGE));

/*
 * NBR STRUCT ALLOC
 */
#define NBR_INIT_ALLOC(N)	N = (struct NBR *) CALLOC(1, sizeof(struct NBR));


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
