static char sccsid[] = "@(#)05	1.1  src/tcpip/usr/sbin/gated/ospf_rxmon.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:36";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: __PF0
 *		__PF1
 *		__PF4
 *		__PF5
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
 * ospf_rxmon.c,v 1.16.2.1 1993/09/25 03:17:01 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_TIME
#include "include.h"
#include "inet.h"
#include "ospf.h"


static int mon_fd;			/* for monitor request */
static FILE *mon_fp;

static const char *nbr_modes[] = {
    "None",
    "Slave",
    "Master",
    "Null",
    "Hold"
};

static const char *if_types[] = {
    "Bcast",
    "NBMA",
    "PtoP",
    "Virt"
};

static const char *paths[] = {
    "STUB",
    "RTR",
    "INT",
    "SUM",
    "SUM ",
    "EXT"
};

const char *ospf_con_types[] = {
    "",
    "Router",
    "TransNet",
    "StubNet",
    "Virtual"
};


/*
 * Print the retran list of lsdbs held by all nbrs associated with this intf
 */
static void
print_nbr_retrans __PF1 (intf, struct INTF *)
{
    struct NBR *n;

    NBRS_LIST(n, intf) {
	if (n->rtcnt) {
	    int hash = OSPF_HASH_QUEUE;
	    
	    fprintf(mon_fp,"Nbr %A retrans list:\n",
		  n->nbr_addr);

	    while (hash--) {
		struct LSDB_LIST *ll;
	    
		/* remove from all nbrs' lists */
		for (ll = (struct LSDB_LIST *) &n->retrans[hash];
		     ll;
		     ll = ll->ptr[NEXT]) {
		    if (NO_GUTS(ll->lsdb))
			continue;
		    fprintf(mon_fp,"  type %10s  %A %A\n",
			    trace_state(ospf_ls_type_bits, LS_TYPE(ll->lsdb)),
			    sockbuild_in(0, ll->lsdb->key[0]),
			    sockbuild_in(0, ll->lsdb->key[1]));
		}
	    }
	}
    } NBRS_LIST_END(n, intf) ;
}

/*
 * Print the retran list of nbrs held by this lsdb structure
 */
static void
print_db_retrans __PF1(db, struct LSDB *)
{
    struct NBR_LIST *nl;

    if (DB_RETRANS(db)) {
	fprintf(mon_fp,"retrans list:\n");
	for (nl = DB_RETRANS(db); nl != NLNULL; nl = nl->ptr[NEXT]) {
	    fprintf(mon_fp,"       nbr: %A\n",
		  nl->nbr->nbr_addr);
	}
    }
}


/*
 * Show the link-state data base for this area
 */
static void
lsdbdump __PF1(retrans, long)
{
    struct AREA *a;
    struct LSDB *e, *hp;
    int type;

    fprintf(mon_fp,"LS Data Base:\n");

    fprintf(mon_fp,
	    "Area            LSType Link ID         Adv Rtr          Age   Len  Sequence Metric\n");

    fprintf(mon_fp,
	    "-----------------------------------------------------------------------------------\n");

    AREA_LIST(a) {
	for (type = LS_STUB; type < LS_ASE; type++) {
	    for (hp = a->htbl[type]; hp < &(a->htbl[type][HTBLSIZE]); hp++) {
		for (e = hp; DB_NEXT(e); e = DB_NEXT(e)) {
		    /* check to see if it is valid */
		    if (NO_GUTS(DB_NEXT(e)))
			continue;
		    fprintf(mon_fp,"%-15A %-6s %-15A %-15A  %-4d  %-3d  %-8x %d\n",
			  sockbuild_in(0, a->area_id),
			  trace_state(ospf_ls_type_bits, type),
			  sockbuild_in(0, DB_NEXT(e)->key[0]),
			  sockbuild_in(0, DB_NEXT(e)->key[1]),
			  MIN(ADV_AGE(DB_NEXT(e)), MaxAge),
			  ntohs(LS_LEN(DB_NEXT(e))),
			  ntohl(LS_SEQ(DB_NEXT(e))),
			  LS_TYPE(DB_NEXT(e)) < LS_SUM_NET ? 0 : BIG_METRIC(DB_NEXT(e)));
		    if (retrans)
			print_db_retrans(DB_NEXT(e));
		}
	    }
	}
    } AREA_LIST_END(a) ;
}


/*
 * Show the link-state data base for this area
 */
static void
asedump __PF1(retrans, long)
{
    struct AREA *a = ospf.area;
    struct LSDB *e, *hp;

    fprintf(mon_fp,"AS External Data Base:\n");

    fprintf(mon_fp,
	    "Link ID         Adv Rtr         Forward Addr     Age   Len  Sequence Type Metric\n");

    fprintf(mon_fp,
	    "--------------------------------------------------------------------------------\n");

    for (hp = a->htbl[LS_ASE]; hp < &(a->htbl[LS_ASE][HTBLSIZE]); hp++) {
	for (e = hp; DB_NEXT(e); e = DB_NEXT(e)) {
	    /* check to see if it is valid */
	    if (NO_GUTS(DB_NEXT(e)))
		continue;
	    fprintf(mon_fp,"%-15A %-15A %-15A  %-4d  %-3d  %-8x %-4d %d\n",
		    sockbuild_in(0, DB_NEXT(e)->key[0]),
		    sockbuild_in(0, DB_NEXT(e)->key[1]),
		    sockbuild_in(0, DB_ASE(DB_NEXT(e))->tos0.ForwardAddr),
		    MIN(ADV_AGE(DB_NEXT(e)), MaxAge),
		    ntohs(LS_LEN(DB_NEXT(e))),
		    ntohl(LS_SEQ(DB_NEXT(e))),
		    (ASE_TYPE1(DB_NEXT(e)) ? 1 : 2),
		    BIG_METRIC(DB_NEXT(e)));
	    if (retrans)
		print_db_retrans(DB_NEXT(e));
	}
    }

    fprintf(mon_fp, "\n\tCount %d, checksum sum %X\n",
	    ospf.db_ase_cnt,
	    ospf.db_chksumsum);
}


/*
 * Print configured interfaces and their status
 */
static void
showifs __PF0(void)
{
    struct AREA *a;
    struct INTF *intf;

    fprintf(mon_fp,"Area            IP Address      Type  State    Cost Pri DR             BDR            \n");
    fprintf(mon_fp,"------------------------------------------------------------------------------------------\n");

    AREA_LIST(a) {
	INTF_LIST(intf, a) {
	    fprintf(mon_fp,"%-15A %-15A %-5s %-8s %-4d %-3d %-15A %A\n",
		  sockbuild_in(0, a->area_id),
		  intf->ifap->ifa_addr,
		  if_types[intf->type - 1],
		  ospf_intf_states[intf->state],
		  intf->cost,
		  intf->pri,
		  intf->dr ? intf->dr->nbr_addr : sockbuild_str("None"),
		  intf->bdr ? intf->bdr->nbr_addr : sockbuild_str("None"));
	} INTF_LIST_END(intf, a) ;
    } AREA_LIST_END(a) ;

    /* print virtual links */
    if (IAmBorderRtr && ospf.vcnt) {
	fprintf(mon_fp, "\nVirtual Links:\n");
	fprintf(mon_fp,"Transit Area    Router ID       Remote IP       Local IP        Type  State    Cost\n");
	fprintf(mon_fp,"-----------------------------------------------------------------------------------\n");

	VINTF_LIST(intf) {
	    struct NBR *n = FirstNbr(intf);

	    fprintf(mon_fp,"%-15A %-15A %-15A %-15A %-5s %-8s %-4d\n",
		    sockbuild_in(0, intf->transarea->area_id),
		    n->nbr_id,
		    n->nbr_addr ? n->nbr_addr : sockbuild_str("N/A"),
		    intf->ifap ? intf->ifap->ifa_addr : sockbuild_str("N/A"),
		    if_types[intf->type - 1],
		    ospf_intf_states[intf->state],
		    intf->cost);
	} VINTF_LIST_END(intf) ;
    }
}

/*
 * Show current neigbors and their status
 */
static void
shownbrs __PF1(retrans, int)
{
    struct AREA *a;
    struct INTF *intf;
    struct NBR *n;

    fprintf(mon_fp,"Area            Interface       Router Id       Nbr IP Addr     State      Mode   Prio\n");
    fprintf(mon_fp,"--------------------------------------------------------------------------------------\n");
    AREA_LIST(a) {
	INTF_LIST(intf, a) {
	    NBRS_LIST(n, intf) {
		fprintf(mon_fp,"%-15A %-15A %-15A %-15A %-10s %-6s %d\n",
		      sockbuild_in(0, a->area_id),
		      intf->ifap->ifa_addr,
		      n->nbr_id,
		      n->nbr_addr,
		      ospf_nbr_states[n->state],
		      nbr_modes[n->mode],
		      n->pri);
	    } NBRS_LIST_END(n, intf) ;
	    if (retrans)
		print_nbr_retrans(intf);
	} INTF_LIST_END(intf, a) ;
    } AREA_LIST_END(a) ;

    if (IAmBorderRtr && ospf.vcnt)	/* print virtual links */
	VINTF_LIST(intf) {
	    NBRS_LIST(n, intf) {
		fprintf(mon_fp,"%-15s %-15A %-15A %-15A %-10s %-6s %d\n",
		      "0.0.0.0",
		      intf->ifap ? intf->ifap->ifa_addr : (sockaddr_un *) 0,
		      n->nbr_id,
		      n->nbr_addr,
		      ospf_nbr_states[n->state],
		      nbr_modes[n->mode],
		      n->pri);
	    } NBRS_LIST_END(n, intf) ;
	    if (retrans)
		print_nbr_retrans(intf);
	} VINTF_LIST_END(intf) ;
}



/*
 * Print the LSA hdr
 */
static void
ls_hdr_print __PF1(ls_hdr, struct LS_HDR *)
{
    fprintf(mon_fp, "LSA  type: %s ls id: %A adv rtr: %A age: %d\n",
	    trace_state(ospf_ls_type_bits, ls_hdr->ls_type),
	    sockbuild_in(0, ls_hdr->ls_id),
	    sockbuild_in(0, ls_hdr->adv_rtr),
	    ls_hdr->ls_age);

    fprintf(mon_fp, "     len: %d seq #: %x cksum: 0x%x\n",
	    ntohs(ls_hdr->length),
	    ls_hdr->ls_seq,
	    ls_hdr->ls_chksum);
}


static void
lsa_parse_print __PF1(lsa, void_t)
{
    int cnt, i;
    struct NET_LA_PIECES *att_rtr;
    struct RTR_LA_PIECES *linkp;
    union LSA_PTR adv;

    adv.rtr = (struct RTR_LA_HDR *) lsa;

    ls_hdr_print((struct LS_HDR *) lsa);

    switch (adv.rtr->ls_hdr.ls_type) {
    case LS_RTR:
	fprintf(mon_fp, "     Capabilities: As Border: %s Area Border: %s\n",
		(ntohs(adv.rtr->E_B) & bit_E) ? "On" : "Off",
		(ntohs(adv.rtr->E_B) & bit_B) ? "On" : "Off");

	fprintf(mon_fp, "Link count: %d\n",
		adv.rtr->lnk_cnt);

	for (cnt = ntohs(adv.rtr->lnk_cnt),
	     i = 0,
	     linkp = (struct RTR_LA_PIECES *) & (adv.rtr->link);
	     i < cnt;
	     linkp = (struct RTR_LA_PIECES *) ((long) linkp +
					      RTR_LA_PIECES_SIZE +
					      ((linkp->metric_cnt) * RTR_LA_METRIC_SIZE)),
	     i++) {
	    fprintf(mon_fp,
		    "     link id: %-12A data: %-12A type: %-12s metric: %d\n",
		    sockbuild_in(0, linkp->lnk_id),
		    sockbuild_in(0, linkp->lnk_data),
		    ospf_con_types[linkp->con_type],
		    ntohs(linkp->tos0_metric));
	}
	break;

    case LS_NET:
	fprintf(mon_fp,
		"    Net mask: %A\n",
		sockbuild_in(0, adv.net->net_mask));

	cnt = ntohs(adv.net->ls_hdr.length) - NET_LA_HDR_SIZE;
	for (att_rtr = &(adv.net->att_rtr), i = 0;
	     i < cnt;
	     att_rtr++, i += 4) {
	    fprintf(mon_fp,
		    "Attached router: %A\n",
		    sockbuild_in(0, att_rtr->lnk_id));
	}
	break;

    case LS_SUM_NET:
	fprintf(mon_fp, "    Net mask: %A",
		sockbuild_in(0, adv.sum->net_mask));
	/* Fall through */

    case LS_SUM_ASB:
	fprintf(mon_fp, " Tos 0 metric: %d\n",
	       ntohl(adv.sum->tos0.tos_metric));
	break;

    case LS_ASE:
	fprintf(mon_fp,
		"     Net mask: %A Tos 0 metric %d E type %d\n",
		sockbuild_in(0, adv.ase->net_mask),
		ADV_BIG_METRIC(adv.ase),
		(ADV_ASE_TYPE2(adv.ase) ? 2 : 1));

	fprintf(mon_fp,
		"     Forwarding Address %A Tag %x\n",
		sockbuild_in(0, adv.ase->tos0.ForwardAddr),
		ntohl(adv.ase->tos0.ExtRtTag));
	break;
    }
}


static void
print_lsa __PF4(area_id, u_int32,
		type, u_int,
		ls_id, u_int32,
		adv_rtr, u_int32)
{
    struct LSDB *db;
    struct AREA *a, *area = AREANULL;

    if (type < 1 || type > 5)
	return;

    AREA_LIST(a) {
	if (a->area_id == area_id) {
	    area = a;
	    break;
	}
    } AREA_LIST_END(a) ;
    if (!area) {
	return;
    }
    if (db = (struct LSDB *) FindLSA(area, ls_id, adv_rtr, type))
	lsa_parse_print((void *) DB_RTR(db));

}


/*
 * Print the asb rtr and ab rtr part of the ospf routing table
 */
static void
printrtab __PF0(void)
{
    rt_entry *rt;
    struct OSPF_ROUTE *r;
    struct AREA *a;
    int count = 0;
    int count_net = 0;
    int count_sum = 0;
    int count_ase = 0;
    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(RTPROTO_OSPF) | RTPROTO_BIT(RTPROTO_OSPF_ASE));

    fprintf(mon_fp,"\nSPF algorithm run %d times\n",
	  RTAB_REV);
    fprintf(mon_fp,"Dest          D_mask          Area      Cost E  Path Nexthop         advrtr\n");
    fprintf(mon_fp,"------------------------------------------------------------------------------\n");
    /* As border rtrs */
    AREA_LIST(a) {
	for (r = a->asbrtab.ptr[NEXT]; r; r = RRT_NEXT(r)) {
	    if (!count++) {
		fprintf(mon_fp,"AS Border Routes:\n");
	    }
	    fprintf(mon_fp,"%-13A %-15s %-9A %-3d     %-4s %-13A %A\n",
		    sockbuild_in(0, RRT_DEST(r)),
		    "--------",
		    sockbuild_in(0, RRT_AREA(r) ? RRT_AREA(r)->area_id : OSPF_BACKBONE),
		    RRT_COST(r),
		    paths[RRT_PTYPE(r)],
		    sockbuild_in(0, RRT_NH_NDX(r) ? RRT_NH(r) : RRT_ADVRTR(r)),
		    sockbuild_in(0, RRT_ADVRTR(r)));
	}
    } AREA_LIST_END(a) ;
    if (count) {
	fprintf(mon_fp, "Total AS Border routes: %d\n",
		count);
    }
    fprintf(mon_fp, "\n");

    count = 0;
    /* Area border rtrs */
    AREA_LIST(a) {
	for (r = a->abrtab.ptr[NEXT]; r; r = RRT_NEXT(r)) {
	    if (!count++) {
		fprintf(mon_fp,"Area Border Routes:\n");
	    }
	    fprintf(mon_fp,"%-13A %-15s %-9A %-3d     %-4s %-13A %-A\n",
		    sockbuild_in(0, RRT_DEST(r)),
		    "--------",
		    sockbuild_in(0, RRT_AREA(r) ? RRT_AREA(r)->area_id : OSPF_BACKBONE),
		    RRT_COST(r),
		    paths[RRT_PTYPE(r)],
		    sockbuild_in(0, RRT_NH_NDX(r) ? RRT_NH(r) : RRT_ADVRTR(r)),
		    sockbuild_in(0, RRT_ADVRTR(r)));
	}
    } AREA_LIST_END(a) ;
    if (count) {
	fprintf(mon_fp, "Total Area Border Routes: %d\n",
		count);
    }
    fprintf(mon_fp, "\n");

    count = 0;
    /* Summary asb routes */
    for (r = sum_asb_rtab.ptr[NEXT]; r; r = RRT_NEXT(r)) {
	if (!count++) {
	    fprintf(mon_fp,"Summary AS Border Routes:\n");
	}
	fprintf(mon_fp,"%-13A %-15A %-9A %-3d     %-4s %-13A %A\n",
		sockbuild_in(0, RRT_DEST(r)),
		sockbuild_in(0, RRT_MASK(r)),
		sockbuild_in(0, RRT_AREA(r) ? RRT_AREA(r)->area_id : OSPF_BACKBONE),
		RRT_COST(r),
		paths[RRT_PTYPE(r)],
		sockbuild_in(0, RRT_NH_NDX(r) ? RRT_NH(r) : RRT_ADVRTR(r)),
		sockbuild_in(0, RRT_ADVRTR(r)));
    }
    if (count) {
	fprintf(mon_fp, "Total Summary AS Border Routes:\n",
		count);
    }
    fprintf(mon_fp, "\n");

    count = 0;
    RT_LIST(rt, rtl, rt_entry) {
	register int i;

	if (!count++) {
	    fprintf(mon_fp, "Nets:\n");
	}
	switch (rt->rt_gwp->gw_proto) {
	case RTPROTO_OSPF:
	    if (BIT_TEST(rt->rt_state, RTS_INTERIOR)) {
		count_net++;
	    } else {
		count_sum++;
	    }
	    break;
	    
	case RTPROTO_OSPF_ASE:
	    count_ase++;
	    break;
	}
	fprintf(mon_fp, "%-13A %-15A %-9A %-3d %-3d %-4s %-13A %A\n",
		rt->rt_dest,
		rt->rt_dest_mask ? rt->rt_dest_mask : inet_mask_host,
		sockbuild_in(0, ORT_AREA(rt) ? ORT_AREA(rt)->area_id : OSPF_BACKBONE),
		ORT_COST(rt),
		ORT_TYPE2COST(rt),
		paths[ORT_PTYPE(rt)],
		rt->rt_router,
		sockbuild_in(0, ORT_ADVRTR(rt)));

	for (i = 1; i < rt->rt_n_gw; i++) {
	    fprintf(mon_fp, "                                                     %-13A\n",
		    rt->rt_routers[i]);
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;
    if (count) {
	fprintf(mon_fp, "Total nets: %d\n\tIntra Area: %d\tInter Area: %d\tASE: %d\n",
		count,
		count_net,
		count_sum,
		count_ase);
    }
}


static void
err_dump __PF0(void)
{
    int i;

    fprintf(mon_fp,"ERRORS\n");
    for (i = 12; i < LASTLOG; i += 2) {
	fprintf(mon_fp,"%4d: %-32s  %d: %-32s\n",
		ospf_cumlog[i],
		ospf_logtype[i],
		ospf_cumlog[i + 1],
		ospf_logtype[i + 1]);
    }
}


static void
io_dump __PF0(void)
{
    int i;
    struct AREA *area;
    int count_net = 0;
    int count_sum = 0;
    int count_ase = 0;
    rt_entry *rt;
    rt_list *rtl = rtlist_proto(AF_INET, RTPROTO_BIT(RTPROTO_OSPF) | RTPROTO_BIT(RTPROTO_OSPF_ASE));

    fprintf(mon_fp, "IO stats\n");
    fprintf(mon_fp, "\tInput  Output  Type\n");
    for (i = 0; i < 6; i++) {
	fprintf(mon_fp,
		"\t%6d  %6d  %s\n",
		ospf_cumlog[i],
		ospf_cumlog[i+6],
		ospf_logtype[i]);
    }

    fprintf(mon_fp, "\tASE: %d checksum sum %X\n",
	    ospf.db_ase_cnt,
	    ospf.db_chksumsum);

    /* Area information */
    AREA_LIST(area) {
	fprintf(mon_fp, "\n\tArea %A:\n",
		sockbuild_in(0, area->area_id));

	fprintf(mon_fp, "\tNeighbors: %d\tInterfaces: %d\n",
		area->nbrEcnt,
		area->ifcnt);
	fprintf(mon_fp, "\t\tSpf: %d\tChecksum sum %X\n",
		area->spfcnt,
		area->db_chksumsum);

	fprintf(mon_fp, "\t\tDB: rtr: %d net: %d sum: %d ase: %d\n",
		area->db_cnts[LS_RTR],
		area->db_cnts[LS_NET],
		area->db_cnts[LS_SUM_NET],
		area->db_cnts[LS_SUM_ASB]);
    } AREA_LIST_END(area) ;
    fprintf(mon_fp, "\n");

    /* Routing table information */
    fprintf(mon_fp, "Routing Table:\n");
    RT_LIST(rt, rtl, rt_entry) {
	switch (rt->rt_gwp->gw_proto) {
	case RTPROTO_OSPF:
	    if (BIT_TEST(rt->rt_state, RTS_INTERIOR)) {
		count_net++;
	    } else {
		count_sum++;
	    }
	    break;
	    
	case RTPROTO_OSPF_ASE:
	    count_ase++;
	    break;
	}
    } RT_LIST_END(rt, rtl, rt_entry) ;
    fprintf(mon_fp, "\tIntra Area: %d\tInter Area: %d\tASE: %d\n",
	    count_net,
	    count_sum,
	    count_ase);

    RTLIST_RESET(rtl);
}


int
ospf_rx_mon __PF5(mreq, struct MON_HDR *,
		  intf, struct INTF *,
		  src, sockaddr_un *,
		  router_id, sockaddr_un *,
		  olen, size_t)
{
    
    /* for responding to monitor request if local is true */

    if (mreq->type != MREQUEST) {
	trace(TR_OSPF, 0, "Bad mrequest type");
	return GOOD_RX;

    }
    
    if (mreq->local) {
	int len;
	struct sockaddr *addr;
	
	/* Create socket for returning monitor info */
	sock2port(src) = mreq->port;

	if ((mon_fd = task_get_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	    trace(TR_ALL, LOG_ERR, "mon_open: can not get socket for OSPF monitor response");
	    mon_fd = -1;
	    return GOOD_RX;
	}

	addr = sock2unix(src, &len);
	if (connect(mon_fd, addr, len) < 0) {
	    trace(TR_ALL, LOG_ERR, "mon_open: can not connect to %#A: %m",
		  src);
	    close(mon_fd);
	    mon_fd = -1;
	    return GOOD_RX;
	}

	trace(TR_OSPF, 0, "mon_open: connected to %#A",
	      src);
    
	if (!(mon_fp = fdopen(mon_fd, "w"))) {
	    trace(TR_ALL, LOG_ERR, "mon_open: fdopen: %m");
	    mon_fp = (FILE *) 0;
	    return GOOD_RX;
	}
    } else {
	mon_fp = fopen("/dev/null", "w");
	mon_fd = fileno(mon_fp);
    }
    
    switch (mreq->req) {
    case 'a':
	print_lsa(mreq->p[0],
		  ntohl(mreq->p[1]),
		  ntohl(mreq->p[2]),
		  ntohl(mreq->p[3]));
	break;

    case 'c':
	io_dump();
	break;

    case 'e':
	err_dump();
	break;

    case 'l':
	lsdbdump((long) mreq->p[0]);
	break;

    case 'A':
	asedump((long) mreq->p[0]);
	break;

    case 'o':
	printrtab();
	break;

    case 'I':
	showifs();
	break;

    case 'N':
	shownbrs((int) mreq->p[0]);
	break;

    default:
	fprintf(mon_fp,
		"Unknown command: %c\n\n",
		mreq->req);
    }

    if (mreq->local) {
	fclose(mon_fp);
	mon_fp = (FILE *) 0;
	mon_fd = -1;
    }
    return GOOD_RX;
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
