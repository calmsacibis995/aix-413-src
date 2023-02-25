static char sccsid[] = "@(#)97	1.1  src/tcpip/usr/sbin/gated/ospf_mib.c, tcprouting, tcpip411, GOLD410 12/6/93 17:26:16";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: BIT_TEST
 *		MINTF_LIST
 *		MINTF_LIST_END
 *		MLSDB_LIST
 *		MLSDB_LIST_END
 *		MVINTF_LIST
 *		MVINTF_LIST_END
 *		PROTOTYPE
 *		__PF0
 *		__PF1
 *		__PF2
 *		__PF3
 *		__PF4
 *		if
 *		o_integer
 *		o_ipaddr
 *		ospf_init_mib
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
 * ospf_mib.c,v 1.25.2.2 1993/08/27 22:28:24 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_ISODE_SNMP
#include "include.h"
#include "inet.h"
#include "ospf.h"
#include "snmp_isode.h"


PROTOTYPE(o_ospf,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_area,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_stub,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_lsdb,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_range,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_host,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_intf,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_metric,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_vintf,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_nbr,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
PROTOTYPE(o_vnbr,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
#ifdef	notyet
PROTOTYPE(o_ase,
	  static int,
	  (OI,
	   struct type_SNMP_VarBind *,
	   int));
#endif	/* notyet */

static struct object_table ospf_objects[] = {
#define	ospfRouterId		1
#define	ospfAdminStat		2
#define	ospfVersionNumber	3
#define	ospfAreaBdrRtrStatus	4
#define	ospfASBdrRtrStatus	5
#define	ospfExternLSACount	6
#define	ospfExternLSACksumSum	7
#define	ospfTOSSupport		8
#define	ospfOriginateNewLSAs	9
#define	ospfRxNewLSAs		10
    { "ospfRouterId",		o_ospf,		NULL,			ospfRouterId },
    { "ospfAdminStat",		o_ospf,		NULL,			ospfAdminStat },
    { "ospfVersionNumber",	o_ospf,		NULL,			ospfVersionNumber },
    { "ospfAreaBdrRtrStatus",	o_ospf,		NULL,			ospfAreaBdrRtrStatus },
    { "ospfASBdrRtrStatus",	o_ospf,		NULL,			ospfASBdrRtrStatus },
    { "ospfExternLSACount",	o_ospf,		NULL,			ospfExternLSACount },
    { "ospfExternLSACksumSum",	o_ospf,		NULL,			ospfExternLSACksumSum },
    { "ospfTOSSupport",		o_ospf,		NULL,			ospfTOSSupport },
    { "ospfOriginateNewLSAs",	o_ospf,		NULL,			ospfOriginateNewLSAs },
    { "ospfRxNewLSAs",		o_ospf,		NULL,			ospfRxNewLSAs },

    
#define	ospfAreaId		11
#define	ospfAuthType		12
#define	ospfImportASExtern	13
#define	ospfSpfRuns		14
#define	ospfAreaBdrRtrCount	15
#define	ospfASBdrRtrCount	16
#define	ospfAreaLSACount	17
#define	ospfAreaLSACksumSum	18
    { "ospfAreaId",		o_area,		NULL,			ospfAreaId },
    { "ospfAuthType",		o_area,		NULL,			ospfAuthType },
    { "ospfImportASExtern",	o_area,		NULL,			ospfImportASExtern },
    { "ospfSpfRuns",		o_area,		NULL,			ospfSpfRuns },
    { "ospfAreaBdrRtrCount",	o_area,		NULL,			ospfAreaBdrRtrCount },
    { "ospfASBdrRtrCount",	o_area,		NULL,			ospfASBdrRtrCount },
    { "ospfAreaLSACount",	o_area,		NULL,			ospfAreaLSACount },
    { "ospfAreaLSACksumSum",	o_area,		NULL,			ospfAreaLSACksumSum },

    
#define	ospfStubAreaID		19
#define	ospfStubTOS		20
#define	ospfStubMetric		21
#define	ospfStubStatus		22
    { "ospfStubAreaID",		o_stub,		NULL,			ospfStubAreaID },
    { "ospfStubTOS",		o_stub,		NULL,			ospfStubTOS },
    { "ospfStubMetric",		o_stub,		NULL,			ospfStubMetric },
    { "ospfStubStatus",		o_stub,		NULL,			ospfStubStatus },

    
#define	ospfLsdbAreaId		23
#define	ospfLsdbType		24
#define	ospfLsdbLSID		25
#define	ospfLsdbRouterId	26
#define	ospfLsdbSequence	27
#define	ospfLsdbAge		28
#define	ospfLsdbChecksum	29
#define	ospfLsdbAdvertisement	30
    { "ospfLsdbAreaId",		o_lsdb,		NULL,			ospfLsdbAreaId },
    { "ospfLsdbType",		o_lsdb,		NULL,			ospfLsdbType },
    { "ospfLsdbLSID",		o_lsdb,		NULL,			ospfLsdbLSID },
    { "ospfLsdbRouterId",	o_lsdb,		NULL,			ospfLsdbRouterId },
    { "ospfLsdbSequence",	o_lsdb,		NULL,			ospfLsdbSequence },
    { "ospfLsdbAge",		o_lsdb,		NULL,			ospfLsdbAge },
    { "ospfLsdbChecksum",	o_lsdb,		NULL,			ospfLsdbChecksum },
    { "ospfLsdbAdvertisement",	o_lsdb,		NULL,			ospfLsdbAdvertisement },


#define	ospfAreaRangeAreaID	31
#define	ospfAreaRangeNet	32
#define	ospfAreaRangeMask	33
#define	ospfAreaRangeStatus	34
    { "ospfAreaRangeAreaID",	o_range,		NULL,			ospfAreaRangeAreaID },
    { "ospfAreaRangeNet",	o_range,		NULL,			ospfAreaRangeNet },
    { "ospfAreaRangeMask",	o_range,		NULL,			ospfAreaRangeMask },
    { "ospfAreaRangeStatus",	o_range,		NULL,			ospfAreaRangeStatus },

    
#define	ospfHostIpAddress	35
#define	ospfHostTOS		36
#define	ospfHostMetric		37
#define	ospfHostStatus		38
    { "ospfHostIpAddress",	o_host,		NULL,			ospfHostIpAddress },
    { "ospfHostTOS",		o_host,		NULL,			ospfHostTOS },
    { "ospfHostMetric",		o_host,		NULL,			ospfHostMetric },
    { "ospfHostStatus",		o_host,		NULL,			ospfHostStatus },

    
#define	ospfIfIpAddress		39
#define	ospfAddressLessIf	40
#define	ospfIfAreaId		41
#define	ospfIfType		42
#define	ospfIfAdminStat		43
#define	ospfIfRtrPriority	44
#define	ospfIfTransitDelay	45
#define	ospfIfRetransInterval	46
#define	ospfIfHelloInterval	47
#define	ospfIfRtrDeadInterval	48
#define	ospfIfPollInterval	49
#define	ospfIfState		50
#define	ospfIfDesignatedRouter	51
#define	ospfIfBackupDesignatedRouter	52
#define	ospfIfEvents		53
#define	ospfIfAuthKey		54
    { "ospfIfIpAddress",	o_intf,		NULL,			ospfIfIpAddress },
    { "ospfAddressLessIf",	o_intf,		NULL,			ospfAddressLessIf },
    { "ospfIfAreaId",		o_intf,		NULL,			ospfIfAreaId },
    { "ospfIfType",		o_intf,		NULL,			ospfIfType },
    { "ospfIfAdminStat",	o_intf,		NULL,			ospfIfAdminStat },
    { "ospfIfRtrPriority",	o_intf,		NULL,			ospfIfRtrPriority },
    { "ospfIfTransitDelay",	o_intf,		NULL,			ospfIfTransitDelay },
    { "ospfIfRetransInterval",	o_intf,		NULL,			ospfIfRetransInterval },
    { "ospfIfHelloInterval",	o_intf,		NULL,			ospfIfHelloInterval },
    { "ospfIfRtrDeadInterval",	o_intf,		NULL,			ospfIfRtrDeadInterval },
    { "ospfIfPollInterval",	o_intf,		NULL,			ospfIfPollInterval },
    { "ospfIfState",		o_intf,		NULL,			ospfIfState },
#define	I_STATE_DOWN		1
#define	I_STATE_LOOPBACK	2
#define	I_STATE_WAITING		3
#define	I_STATE_P2P		4
#define	I_STATE_DR		5
#define	I_STATE_BDR		6
#define	I_STATE_DROTHER		7
    { "ospfIfDesignatedRouter",	o_intf,		NULL,			ospfIfDesignatedRouter },
    { "ospfIfBackupDesignatedRouter",		o_intf,		NULL,	ospfIfBackupDesignatedRouter },
    { "ospfIfEvents",		o_intf,		NULL,			ospfIfEvents },
    { "ospfIfAuthKey",		o_intf,		NULL,			ospfIfAuthKey },


#define	ospfIfMetricIpAddress		55
#define	ospfIfMetricAddressLessIf	56
#define	ospfIfMetricTOS			57
#define	ospfIfMetricMetric		58
#define	ospfIfMetricStatus		59
    { "ospfIfMetricIpAddress",		o_metric,		NULL,			ospfIfMetricIpAddress },
    { "ospfIfMetricAddressLessIf",	o_metric,		NULL,			ospfIfMetricAddressLessIf },
    { "ospfIfMetricTOS",		o_metric,		NULL,			ospfIfMetricTOS },
    { "ospfIfMetricMetric",		o_metric,		NULL,			ospfIfMetricMetric },
    { "ospfIfMetricStatus",		o_metric,		NULL,			ospfIfMetricStatus },


#define	ospfVirtIfAreaID		60
#define	ospfVirtIfNeighbor		61
#define	ospfVirtIfTransitDelay		62
#define	ospfVirtIfRetransInterval	63
#define	ospfVirtIfHelloInterval		64
#define	ospfVirtIfRtrDeadInterval	65
#define	ospfVirtIfState			66
#define	ospfVirtIfEvents		67
#define	ospfVirtIfAuthKey		68
#define	ospfVirtIfStatus		69
    { "ospfVirtIfAreaID",		o_vintf,		NULL,			ospfVirtIfAreaID },
    { "ospfVirtIfNeighbor",		o_vintf,		NULL,			ospfVirtIfNeighbor },
    { "ospfVirtIfTransitDelay",		o_vintf,		NULL,			ospfVirtIfTransitDelay },
    { "ospfVirtIfRetransInterval",	o_vintf,		NULL,			ospfVirtIfRetransInterval },
    { "ospfVirtIfHelloInterval",	o_vintf,		NULL,			ospfVirtIfHelloInterval },
    { "ospfVirtIfRtrDeadInterval",	o_vintf,		NULL,			ospfVirtIfRtrDeadInterval },
    { "ospfVirtIfState",		o_vintf,		NULL,			ospfVirtIfState },
    { "ospfVirtIfEvents",		o_vintf,		NULL,			ospfVirtIfEvents },
    { "ospfVirtIfAuthKey",		o_vintf,		NULL,			ospfVirtIfAuthKey },
    { "ospfVirtIfStatus",		o_vintf,		NULL,			ospfVirtIfStatus },


#define	ospfNbrIpAddr		70
#define	ospfNbrAddressLessIndex	71
#define	ospfNbrRtrId		72
#define	ospfNbrOptions		73
#define	ospfNbrPriority		74
#define	ospfNbrState		75
#define	ospfNbrEvents		76
#define	ospfNbrLSRetransQLen	77
#define	ospfNBMANbrStatus	78
    { "ospfNbrIpAddr",		o_nbr,		NULL,			ospfNbrIpAddr },
    { "ospfNbrAddressLessIndex",		o_nbr,		NULL,	ospfNbrAddressLessIndex },
    { "ospfNbrRtrId",		o_nbr,		NULL,			ospfNbrRtrId },
    { "ospfNbrOptions",		o_nbr,		NULL,			ospfNbrOptions },
    { "ospfNbrPriority",	o_nbr,		NULL,			ospfNbrPriority },
    { "ospfNbrState",		o_nbr,		NULL,			ospfNbrState },
#define	N_STATE_DOWN		1
#define	N_STATE_ATTEMPT		2
#define	N_STATE_INIT		3
#define	N_STATE_2WAY		4
#define	N_STATE_EXSTART		5
#define	N_STATE_EXCHANGE	6
#define	N_STATE_LOADING		7
#define	N_STATE_FULL		8
    { "ospfNbrEvents",		o_nbr,		NULL,			ospfNbrEvents },
    { "ospfNbrLSRetransQLen",	o_nbr,		NULL,			ospfNbrLSRetransQLen },
    { "ospfNBMANbrStatus",	o_nbr,		NULL,			ospfNBMANbrStatus },


#define	ospfVirtNbrArea			79
#define	ospfVirtNbrRtrId		80
#define	ospfVirtNbrIpAddr		81
#define	ospfVirtNbrOptions		82
#define	ospfVirtNbrState		83
#define	ospfVirtNbrEvents		84
#define	ospfVirtNbrLSRetransQLen	85
    { "ospfVirtNbrArea",		o_vnbr,		NULL,			ospfVirtNbrArea },
    { "ospfVirtNbrRtrId",		o_vnbr,		NULL,			ospfVirtNbrRtrId },
    { "ospfVirtNbrIpAddr",		o_vnbr,		NULL,			ospfVirtNbrIpAddr },
    { "ospfVirtNbrOptions",		o_vnbr,		NULL,			ospfVirtNbrOptions },
    { "ospfVirtNbrState",		o_vnbr,		NULL,			ospfVirtNbrState },
    { "ospfVirtNbrEvents",		o_vnbr,		NULL,			ospfVirtNbrEvents },
    { "ospfVirtNbrLSRetransQLen",	o_vnbr,		NULL,			ospfVirtNbrLSRetransQLen },


#ifdef	notyet
#define	ospfAseType		86
#define	ospfAseLSID		87
#define	ospfAseRouterId		88
#define	ospfAseSequence		89
#define	ospfAseAge		90
#define	ospfAseChecksum		91
#define	ospfAseAdvertisement	92
    { "ospfAseType",		o_ase,		NULL,			ospfAseType },
    { "ospfAseLSID",		o_ase,		NULL,			ospfAseLSID },
    { "ospfAseRouterId",	o_ase,		NULL,			ospfAseRouterId },
    { "ospfAseSequence",	o_ase,		NULL,			ospfAseSequence },
    { "ospfAseAge",		o_ase,		NULL,			ospfAseAge },
    { "ospfAseChecksum",	o_ase,		NULL,			ospfAseChecksum },
    { "ospfAseAdvertisement",	o_ase,		NULL,			ospfAseAdvertisement },
#endif	/* notyet */

    
    { NULL }
};


#define	MIB_ENABLED	1
#define	MIB_DISABLED	2

#define	MIB_TRUE	1
#define	MIB_FALSE	2

#define	MIB_VALID	1
#define	MIB_INVALID	2

#define	MIB_BIT_ASE	0x01
#define	MIB_BIT_TOS	0x02

static struct snmp_tree ospf_mib_tree = {
    NULL, NULL,
    "ospf",
    NULLOID,
    readOnly,
    ospf_objects,
    0
};


/* General group */
static int
o_ospf __PF3(oi, OI,
	     v, register struct type_SNMP_VarBind *,
	     offset, int)
{
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem != ot->ot_name->oid_nelem + 1
	    || oid->oid_elements[oid->oid_nelem - 1]) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    OID new;

	    if ((new = oid_extend(oid, 1)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }
	    new->oid_elements[new->oid_nelem - 1] = 0;

	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    return NOTOK;
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }

    switch (ot2object(ot)->ot_info) {
    case ospfRouterId:
	return o_ipaddr(oi,
			v,
			sock2unix(ospf.router_id,
				  (int *) 0));
	
    case ospfAdminStat:
	return o_integer(oi, v, ospf.ospf_admin_stat == OSPF_ENABLED ? MIB_ENABLED : MIB_DISABLED);
	
    case ospfVersionNumber:
	return o_integer(oi, v, OSPF_VERSION);

    case ospfAreaBdrRtrStatus:
	return o_integer(oi, v, IAmBorderRtr ? MIB_TRUE : MIB_FALSE);

    case ospfASBdrRtrStatus:
	return o_integer(oi, v, IAmASBorderRtr ? MIB_TRUE : MIB_FALSE);
	
    case ospfExternLSACount:
	return o_integer(oi, v, ospf.db_ase_cnt);
	
    case ospfExternLSACksumSum:
	return o_integer(oi, v, ospf.db_chksumsum);

    case ospfTOSSupport:
	return o_integer(oi, v, MIB_FALSE);

    case ospfOriginateNewLSAs:
	return o_integer(oi, v, ospf.orig_new_lsa);

    case ospfRxNewLSAs:
	return o_integer(oi, v, ospf.rx_new_lsa);
    }

    return int_SNMP_error__status_noSuchName;
}


/* Area group */


static struct AREA *
o_area_lookup __PF3(ip, register unsigned int *,
		    len, size_t,
		    isnext, int)
{
    static unsigned int *last;
    static struct AREA *last_area;

    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_area;
    }

    if (len) {
	u_int32 area_id;
	register struct AREA *area;
	
	oid2ipaddr(ip, &area_id);

	GNTOHL(area_id);

	AREA_LIST(area) {
	    register u_int32 cur_id = ntohl(area->area_id);
	    
	    if (cur_id == area_id) {
		if (!isnext) {
		    return last_area = area;
		}
	    } else if (cur_id > area_id){
		return last_area = isnext ? area : (struct AREA *) 0;
	    }
	} AREA_LIST_END(area) ;

	return last_area = (struct AREA *) 0;
    }
    
    return last_area = ospf.area;
}


static int
o_area __PF3 (oi, OI,
	      v, register struct type_SNMP_VarBind *,
	      offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct AREA *area;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;

    /* INDEX { ospfAreaID } */
#define	NDX_SIZE	(sizeof (area->area_id))
    
    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem != ot->ot_name->oid_nelem + NDX_SIZE) {
		return int_SNMP_error__status_noSuchName;
	    }
	area = o_area_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			    NDX_SIZE,
			    0);
	if (!area) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    area = o_area_lookup((unsigned int *) 0,
				 0,
				 TRUE);
	    if (!area) {
		return NOTOK;
	    }

	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &area->area_id, sizeof (area->area_id));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    area = o_area_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				     j = oid->oid_nelem - ot->ot_name->oid_nelem,
				     TRUE);
	    if (!area) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &area->area_id, sizeof (area->area_id));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case ospfAreaId:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, area->area_id),
				  (int *) 0));

    case ospfAuthType:
	return o_integer(oi, v, area->authtype);

    case ospfImportASExtern:
	return o_integer(oi, v, BIT_TEST(area->area_flags, OSPF_AREAF_STUB) ? MIB_FALSE : MIB_TRUE);

    case ospfSpfRuns:
	return o_integer(oi, v, area->spfcnt);

    case ospfAreaBdrRtrCount:
	return o_integer(oi, v, area->abr_cnt);

    case ospfASBdrRtrCount:
	return o_integer(oi, v, area->asbr_cnt);

    case ospfAreaLSACount:
	return o_integer(oi, v, area->db_int_cnt);	

    case ospfAreaLSACksumSum:
	return o_integer(oi, v, area->db_chksumsum);
    }
    
    return int_SNMP_error__status_noSuchName;
}


/* Stub area group */

/* INDEX { ospfStubAreaID, ospfStubTOS } */
#define	NDX_SIZE	(sizeof (struct in_addr) + 1)

static struct AREA *
o_stub_lookup __PF3(ip, register unsigned int *,
		    len, size_t,
		    isnext, int)
{
    u_int32 area_id = 0;
    register struct AREA *area;
    static unsigned int *last;
    static struct AREA *last_area;

    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_area;
    }

    if (len) {
	oid2ipaddr(ip, &area_id);
	GNTOHL(area_id);

	if (ip[sizeof (struct in_addr)]) {
	    /* We don't support TOS */
	    return last_area = (struct AREA *) 0;
	}
    }

    AREA_LIST(area) {
	register u_int32 cur_id;

	if (!BIT_TEST(area->area_flags, OSPF_AREAF_STUB)) {
	    /* Not a stub area */
	    continue;
	}

	cur_id = ntohl(area->area_id);

	if (cur_id == area_id) {
	    if (!isnext) {
		return last_area = area;
	    }
	} else if (cur_id > area_id){
	    return last_area = isnext ? area : (struct AREA *) 0;
	}
    } AREA_LIST_END(area) ;

    return last_area = (struct AREA *) 0;
}


static int
o_stub __PF3(oi, OI,
	     v, register struct type_SNMP_VarBind *,
	     offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct AREA *area;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem != ot->ot_name->oid_nelem + NDX_SIZE) {
		return int_SNMP_error__status_noSuchName;
	    }
	area = o_stub_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			     NDX_SIZE,
			     0);
	if (!area) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    area = o_stub_lookup((unsigned int *) 0,
				 0,
				 TRUE);
	    if (!area) {
		return NOTOK;
	    }

	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &area->area_id, sizeof (area->area_id));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    area = o_stub_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				 j = oid->oid_nelem - ot->ot_name->oid_nelem,
				 TRUE);
	    if (!area) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &area->area_id, sizeof (area->area_id));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case ospfStubAreaID:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, area->area_id),
				  (int *) 0));

    case ospfStubTOS:
	return o_integer(oi, v, 0);

    case ospfStubMetric:
	return o_integer(oi, v, BIT_TEST(area->area_flags, OSPF_AREAF_STUB_DEFAULT) ? area->dflt_metric : (unsigned) -1);

    case ospfStubStatus:
	return o_integer(oi, v, MIB_VALID);
    }

    return int_SNMP_error__status_noSuchName;
}


/* Link state database group */

/* INDEX { ospfLsdbAreaId, ospfLsdbType, ospfLsdbLSID, ospfLsdbRouterId } */
#define	NDX_SIZE	(sizeof (struct in_addr) + 1 + sizeof (struct in_addr) + sizeof (struct in_addr))

struct lsdb_entry {
    unsigned int key[NDX_SIZE];
    struct LSDB *lsdb;
};

#define	MLSDB_LIST(lpp)		for (lpp = o_lsdb_list; *lpp; lpp++)
#define	MLSDB_LIST_END(lpp)	

static struct lsdb_entry **o_lsdb_list;

static int o_lsdb_cnt;
static u_int o_lsdb_size;
static block_t o_lsdb_index;

static int
o_lsdb_compare __PF2(le1, void_t,
		     le2, void_t)
{
    return elem_cmp((*((struct lsdb_entry **) le1))->key,
		    NDX_SIZE,
		    (*((struct lsdb_entry **) le2))->key,
		    NDX_SIZE);
}


static void
o_lsdb_get __PF0(void)
{
    u_int lsdb_cnt = 0;
    register struct AREA *area;
    register struct lsdb_entry **lsdbp;

    AREA_LIST(area) {
	lsdb_cnt += area->db_int_cnt;
    } AREA_LIST_END(area) ;

    if (o_lsdb_size < lsdb_cnt) {
	if (o_lsdb_list) {
	    int i = o_lsdb_size;

	    lsdbp = o_lsdb_list;

	    while (i--) {
		task_block_free(o_lsdb_index, (void_t) *lsdbp);
	    }
	    task_block_reclaim((size_t) ((o_lsdb_size + 1) * sizeof (*o_lsdb_list)), (void_t) o_lsdb_list);
	}

	o_lsdb_size = lsdb_cnt;

	o_lsdb_list = (struct lsdb_entry **) task_block_malloc((size_t) ((o_lsdb_size + 1) * sizeof (*o_lsdb_list)));
    }

    lsdbp = o_lsdb_list;
    o_lsdb_cnt = 0;
    
    AREA_LIST(area) {
	int type;

	for (type = LS_RTR; type < LS_ASE; type++) {
	     register struct LSDB *db, *hp;

	     for (hp = area->htbl[type]; hp < &(area->htbl[type][HTBLSIZE]); hp++) {
		 for (db = hp; (DB_NEXT(db) != LSDBNULL); db = DB_NEXT(db)) {
		     unsigned int *ip;
		     struct lsdb_entry *lp;
		     register struct LSDB *lsdb = DB_NEXT(db);
		
		     /* check to see if it is valid */
		     if (NO_GUTS(lsdb)) {
			 continue;
		     }

		     *lsdbp++ = lp = (struct lsdb_entry *) task_block_alloc(o_lsdb_index);

		     lp->lsdb = lsdb;
		     ip = lp->key;

		     STR_OID(ip, &area->area_id, sizeof (area->area_id));
		     INT_OID(ip, LS_TYPE(lsdb));
		     STR_OID(ip, &LS_ID(lsdb), sizeof (LS_ID(lsdb)));
		     STR_OID(ip, &ADV_RTR(lsdb), sizeof (ADV_RTR(lsdb)));		     

		     o_lsdb_cnt++;		     
		 }
	     }

	 }
    } AREA_LIST_END(area) ;

    o_lsdb_list[o_lsdb_cnt] = (struct lsdb_entry *) 0;

    qsort(o_lsdb_list,
	  o_lsdb_cnt,
	  sizeof (*o_lsdb_list),
	  o_lsdb_compare);
}


static struct LSDB *
o_lsdb_lookup __PF3(ip, register unsigned int *,
		    len, size_t,
		    isnext, int)
{
    static struct LSDB *last_lsdb;
    static unsigned int *last;
    static int last_quantum;

    if (last_quantum != snmp_quantum) {
	register struct AREA *area;
	register u_int32 chksumsum = 0;
	static u_int32 last_chksumsum;

	last_quantum = snmp_quantum;

	AREA_LIST(area) {
	    chksumsum += area->db_chksumsum;
	} AREA_LIST_END(area);

	if (chksumsum != last_chksumsum) {
	    /* Time to rebuild the database */

	    last_chksumsum = chksumsum;

	    o_lsdb_get();

	    if (last) {
		task_mem_free((task *) 0, (void_t) last);
		last = (unsigned int *) 0;
	    }
	}
    }

    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_lsdb;
    }

    if (!o_lsdb_cnt) {
	return last_lsdb = (struct LSDB *) 0;
    }
    
    if (len) {
	register struct lsdb_entry **lpp;
	/* XXX - binary search */

	MLSDB_LIST(lpp) {
	    switch (elem_cmp((*lpp)->key, NDX_SIZE, ip, len)) {
	    case 0:
		if (!isnext) {
		    return last_lsdb = (*lpp)->lsdb;
		}
		break;

	    case 1:
		return last_lsdb = isnext ? (*lpp)->lsdb : (struct LSDB *) 0;
	    }
	} MLSDB_LIST_END(lpp);

	return last_lsdb = (struct LSDB *) 0;
    }
    
    return last_lsdb = (*o_lsdb_list)->lsdb;
}

static int
o_lsdb __PF3(oi, OI,
	     v, register struct type_SNMP_VarBind *,
	     offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    register struct LSDB *lsdb;

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	lsdb = o_lsdb_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			     NDX_SIZE,
			     0);
	if (!lsdb) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    lsdb = o_lsdb_lookup((unsigned int *) 0,
				 0,
				 TRUE);
	    if (!lsdb) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &DB_MY_AREA(lsdb)->area_id, sizeof (DB_MY_AREA(lsdb)->area_id));
	    INT_OID(ip, LS_TYPE(lsdb));
	    STR_OID(ip, &LS_ID(lsdb), sizeof (LS_ID(lsdb)));
	    STR_OID(ip, &ADV_RTR(lsdb), sizeof (ADV_RTR(lsdb)));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    lsdb = o_lsdb_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				 j = oid->oid_nelem - ot->ot_name->oid_nelem,
				 TRUE);
	    if (!lsdb) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &DB_MY_AREA(lsdb)->area_id, sizeof (DB_MY_AREA(lsdb)->area_id));
	    INT_OID(ip, LS_TYPE(lsdb));
	    STR_OID(ip, &LS_ID(lsdb), sizeof (LS_ID(lsdb)));
	    STR_OID(ip, &ADV_RTR(lsdb), sizeof (ADV_RTR(lsdb)));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE
    
    switch (ot2object(ot)->ot_info) {
    case ospfLsdbAreaId:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, DB_MY_AREA(lsdb)->area_id),
				  (int *) 0));

    case ospfLsdbType:
	return o_integer(oi, v, LS_TYPE(lsdb));

    case ospfLsdbLSID:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, LS_ID(lsdb)),
				  (int *) 0));

    case ospfLsdbRouterId:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, ADV_RTR(lsdb)),
				  (int *) 0));

    case ospfLsdbSequence:
	return o_integer(oi, v, LS_SEQ(lsdb));

    case ospfLsdbAge:
	return o_integer(oi, v, MIN(ADV_AGE(lsdb), MaxAge));

    case ospfLsdbChecksum:
	return o_integer(oi, v, LS_CKS(lsdb));

    case ospfLsdbAdvertisement:
	return o_string(oi, v, lsdb->adv.rtr, LS_LEN(lsdb));
    }

    return int_SNMP_error__status_noSuchName;
}


/* Network range group */
static struct NET_RANGE *
o_range_lookup __PF4(ip, register unsigned int *,
		     len, size_t,
		     isnext, int,
		     return_area, struct AREA **)
{
    int next = FALSE;
    u_int32 nr_net = (u_int32) 0;
    u_int32 area_id = (u_int32) 0;
    register struct AREA *area;
    register struct NET_RANGE *nr;
    static struct NET_RANGE *last_nr;
    static struct AREA *last_area;
    static unsigned int *last;

    if (snmp_last_match(&last, ip, len, isnext)) {
	*return_area = last_area;
	return last_nr;
    }

    if (len) {
	oid2ipaddr(ip, &area_id);
	GNTOHL(area_id);

	oid2ipaddr(ip + sizeof (struct in_addr), &nr_net);
	GNTOHL(nr_net);
    }

    AREA_LIST(area) {
	register u_int32 cur_id = ntohl(area->area_id);
	
	if (cur_id < area_id) {
	    continue;
	}

	RANGE_LIST(nr, area) {
	    u_int32 cur_net = ntohl(nr->net);

	    if (next) {
		goto got_it;
	    }
	    
	    if (cur_net == nr_net) {
		if (isnext) {
		    next = TRUE;
		} else {
		    goto got_it;
		}
	    } else if (cur_net > nr_net) {
		if (!isnext) {
		    nr = (struct NET_RANGE *) 0;
		}
		goto got_it;
	    }
	} RANGE_LIST_END(nr, area) ;
    } AREA_LIST_END(area) ;

    nr = (struct NET_RANGE *) 0;

 got_it:
    *return_area = last_area = area;
    return last_nr = nr;
}


static int
o_range __PF3(oi, OI,
	      v, register struct type_SNMP_VarBind *,
	      offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    struct AREA *area;
    struct NET_RANGE *range;

    /* INDEX { ospfAreaRangeAreaID, ospfAreaRangeNet } */
#define	NDX_SIZE	(sizeof (struct in_addr) + sizeof (struct in_addr))

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	range = o_range_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			       NDX_SIZE,
			       0,
			       &area);
	if (!range) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    range = o_range_lookup((unsigned int *) 0,
				   0,
				   TRUE,
				   &area);
	    if (!range) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &area->area_id, sizeof (area->area_id));
	    STR_OID(ip, &range->net, sizeof (range->net));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    range = o_range_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				   j = oid->oid_nelem - ot->ot_name->oid_nelem,
				   TRUE,
				   &area);
	    if (!range) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &area->area_id, sizeof (area->area_id));
	    STR_OID(ip, &range->net, sizeof (range->net));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case ospfAreaRangeAreaID:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, area->area_id),
				  (int *) 0));

    case ospfAreaRangeNet:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, range->net),
				  (int *) 0));

    case ospfAreaRangeMask:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, range->mask),
				  (int *) 0));

    case ospfAreaRangeStatus:
	return o_integer(oi, v, MIB_VALID);
    }

    return int_SNMP_error__status_noSuchName;
}


/* Host group */
static struct OSPF_HOSTS *
o_host_lookup __PF3(ip, register unsigned int *,
		    len, size_t,
		    isnext, int)
{
    u_int32 host_addr = (u_int32) 0;
    register struct OSPF_HOSTS *host;
    register struct AREA *area;
    static unsigned int *last;
    static struct OSPF_HOSTS *last_host;

    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_host;
    }

    if (len) {
	oid2ipaddr(ip, &host_addr);

	if (ip[sizeof (struct in_addr)]) {
	    /* We don't support TOS */
	    return last_host = (struct OSPF_HOSTS *) 0;
	}
    }

    if (isnext) {
	register struct OSPF_HOSTS *new = (struct OSPF_HOSTS *) 0;
	u_int32 new_addr = 0;

	GNTOHL(host_addr);

	AREA_LIST(area) {

	    if (area->hostcnt) {
		host = &area->hosts;

		while (host = host->ptr[NEXT]) {
		    u_int32 c_addr = ntohl(host->if_addr);

		    if (c_addr > host_addr &&
			(!new || c_addr < new_addr)) {
			new = host;
			new_addr = c_addr;
		    }
		}
	    }
	} AREA_LIST_END(area) ;

	return last_host = new;
    } else {

	AREA_LIST(area) {

	    if (area->hostcnt) {
		host = &area->hosts;

		while (host = host->ptr[NEXT]) {
		    if (host_addr == host->if_addr) {
			return last_host = host;
		    }
		}
	    }
	} AREA_LIST_END(area) ;

	return last_host = (struct OSPF_HOSTS *) 0;
    }
}


static int
o_host __PF3(oi, OI,
	     v, register struct type_SNMP_VarBind *,
	     offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    struct OSPF_HOSTS *host;

    /* INDEX { ospfHostIpAddress, ospfHostTOS } */
#define	NDX_SIZE	(sizeof (struct in_addr) + 1)

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	host = o_host_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			     NDX_SIZE,
			     0);
	if (!host) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    host = o_host_lookup((unsigned int *) 0,
				 0,
				 TRUE);
	    if (!host) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &host->if_addr, sizeof (host->if_addr));
	    /* No support for TOS */
	    INT_OID(ip, 0);
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    host = o_host_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				 j = oid->oid_nelem - ot->ot_name->oid_nelem,
				 TRUE);
	    if (!host) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &host->if_addr, sizeof (host->if_addr));
	    /* No support for TOS */
	    INT_OID(ip, 0);
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE
    
    switch (ot2object(ot)->ot_info) {
    case ospfHostIpAddress:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, host->if_addr),
				  (int *) 0));

    case ospfHostTOS:
	/* No support for TOS */
	return o_integer(oi, v, 0);

    case ospfHostMetric:
	return o_integer(oi, v, host->cost);

    case ospfHostStatus:
	return o_integer(oi, v, MIB_VALID);
    }
    
    return int_SNMP_error__status_noSuchName;
}


/* Interface group */

struct intf_entry {
    struct intf_entry *forw;
    struct intf_entry *back;
    u_int32 addr;
    int index;
    struct INTF *intf;
};

static struct intf_entry o_intf_list = {&o_intf_list, &o_intf_list };
static int o_intf_cnt;
static block_t o_intf_index;
static unsigned int *o_intf_last;

#define	MINTF_LIST(intfp)	for (intfp = o_intf_list.forw; intfp != &o_intf_list; intfp = intfp->forw)
#define	MINTF_LIST_END(intfp)	

void
o_intf_get __PF0(void)
{
    register struct intf_entry *intfp;
    register struct AREA *area;

    /* Free the old list */
    MINTF_LIST(intfp) {
	register struct intf_entry *intfp2 = intfp->back;

	remque((struct qelem *) intfp);
	task_block_free(o_intf_index, (void_t) intfp);

	intfp = intfp2;
    } MINTF_LIST_END(intfp) ;

    snmp_last_free(&o_intf_last);
    o_intf_cnt = 0;

    AREA_LIST(area) {
	register struct INTF *intf;

	INTF_LIST(intf, area) {
	    register u_int32 intf_addr = ntohl(INTF_LCLADDR(intf));
	    register int intf_index;

	    if (BIT_TEST(intf->ifap->ifa_state, IFS_POINTOPOINT)
		&& intf->ifap->ifa_addrent_local->ifae_n_if > 1) {
		/* Set addressless index */

		intf_index = intf->ifap->ifa_index;
	    } else {
		intf_index = 0;
	    }
	    
	    MINTF_LIST(intfp) {
		if (intf_addr < intfp->addr
		    || (intf_addr == intfp->addr
			&& intf_index < intfp->index)) {
		    break;
		}
	    } MINTF_LIST_END(intfp) ;

	    insque((struct qelem *) task_block_alloc(o_intf_index), intfp->back);
	    intfp->back->intf = intf;
	    intfp->back->addr = intf_addr;
	    intfp->back->index = intf_index;
	    o_intf_cnt++;
	} INTF_LIST_END(intf, area) ;
    } AREA_LIST_END(area) ;

}


static struct intf_entry *
o_intf_lookup __PF3(ip, register unsigned int *,
		    len, size_t,
		    isnext, int)
{
    static struct intf_entry *last_intfp;

    if (snmp_last_match(&o_intf_last, ip, len, isnext)) {
	return last_intfp;
    }
    
    if (!o_intf_cnt) {
	return last_intfp = (struct intf_entry *) 0;
    }
    
    if (len) {
	u_int32 intf_addr = (u_int32) 0;
	int intf_index;
	register struct intf_entry *intfp;

	oid2ipaddr(ip, &intf_addr);
	GNTOHL(intf_addr);

	intf_index = ip[sizeof (struct in_addr)];

	MINTF_LIST(intfp) {
	    if (intfp->addr == intf_addr
		&& intfp->index == intf_index) {
		if (!isnext) {
		    return last_intfp = intfp;
		}
	    } else if (intfp->addr > intf_addr
		       || (intfp->addr == intf_addr
			   && intfp->index > intf_index)) {
		return last_intfp = isnext ? intfp : (struct intf_entry *) 0;
	    }
	} MINTF_LIST_END(ip) ;

	return last_intfp = (struct intf_entry *) 0;
    }

    return last_intfp = o_intf_list.forw;
}


static inline int
o_intf_state __PF1(s_intf, struct INTF *)
{
    int state;

    switch (s_intf->state) {
    case IDOWN:
	state = I_STATE_DOWN;
	break;
	
    case ILOOPBACK:
	state = I_STATE_LOOPBACK;
	break;
	
    case IWAITING:
	state = I_STATE_WAITING;
	break;
	
    case IPOINT_TO_POINT:
	state = I_STATE_P2P;
	break;
	
    case IDr:
	state = I_STATE_DR;
	break;
	
    case IBACKUP:
	state = I_STATE_BDR;
	break;
	
    case IDrOTHER:
	state = I_STATE_DROTHER;
	break;

    default:
	state = -1;
    }

    return state;
}


static int
o_intf __PF3(oi, OI,
	     v, register struct type_SNMP_VarBind *,
	     offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    struct intf_entry *intfp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;

    /* INDEX { ospfIfIpAddress, ospfAddressLessIf } */
#define	NDX_SIZE	(sizeof (struct in_addr) + 1)

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	intfp = o_intf_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			      NDX_SIZE,
			      0);
	if (!intfp) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    intfp = o_intf_lookup((unsigned int *) 0,
				  0,
				  TRUE);
	    if (!intfp) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &sock2ip(intfp->intf->ifap->ifa_addr_local),
		    sizeof (sock2ip(intfp->intf->ifap->ifa_addr_local)));
	    INT_OID(ip, intfp->index);
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    intfp = o_intf_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				  j = oid->oid_nelem - ot->ot_name->oid_nelem,
				  TRUE);
	    if (!intfp) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &sock2ip(intfp->intf->ifap->ifa_addr_local),
		    sizeof (sock2ip(intfp->intf->ifap->ifa_addr_local)));
	    INT_OID(ip, intfp->index);
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case ospfIfIpAddress:
	return o_ipaddr(oi,
			v,
			sock2unix(intfp->intf->ifap->ifa_addr_local,
				  (int *) 0));

    case ospfAddressLessIf:
	/* All interfaces have addresses */
	return o_integer(oi, v, intfp->index);

    case ospfIfAreaId:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, intfp->intf->area->area_id),
				  (int *) 0));

    case ospfIfType:
	return o_integer(oi, v, intfp->intf->type);

    case ospfIfAdminStat:
	return o_integer(oi, v, BIT_TEST(intfp->intf->flags, OSPF_INTFF_ENABLE) ? MIB_ENABLED : MIB_DISABLED);

    case ospfIfRtrPriority:
	return o_integer(oi, v, intfp->intf->nbr.pri);

    case ospfIfTransitDelay:
	return o_integer(oi, v, intfp->intf->transdly);

    case ospfIfRetransInterval:
	return o_integer(oi, v, intfp->intf->retrans_timer);

    case ospfIfHelloInterval:
	return o_integer(oi, v, intfp->intf->hello_timer);

    case ospfIfRtrDeadInterval:
	return o_integer(oi, v, intfp->intf->dead_timer);

    case ospfIfPollInterval:
	return o_integer(oi, v, intfp->intf->poll_timer);

    case ospfIfState:
	return o_integer(oi, v, o_intf_state(intfp->intf));

    case ospfIfDesignatedRouter:
	return o_ipaddr(oi,
			v,
			sock2unix(intfp->intf->dr ? intfp->intf->dr->nbr_addr : inet_addr_default,
				  (int *) 0));

    case ospfIfBackupDesignatedRouter:
	return o_ipaddr(oi,
			v,
			sock2unix(intfp->intf->bdr ? intfp->intf->bdr->nbr_addr : inet_addr_default,
				  (int *) 0));

    case ospfIfEvents:
	return o_integer(oi, v, intfp->intf->events);

    case ospfIfAuthKey:
	/* When read, ospfIfAuthKey always returns an Octet String of length zero. */
	return o_string(oi, v, (caddr_t) 0, 0);

    }
    
    return int_SNMP_error__status_noSuchName;
}


/* Metric group */
static struct intf_entry *
o_metric_lookup __PF3(ip, register unsigned int *,
		      len, size_t,
		      isnext, int)
{

    if (len && ip[sizeof (struct in_addr) + 1]) {
	/* We dont' support TOS */
	return (struct intf_entry *) 0;
    }

    return o_intf_lookup(ip, len, isnext);
}


static int
o_metric __PF3(oi, OI,
	       v, register struct type_SNMP_VarBind *,
	       offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    struct intf_entry *intfp;

    /* INDEX { ospfIfMetricIpAddress, ospfIfMetricAddressLessIf, ospfIfMetricTOS } */
#define	NDX_SIZE	(sizeof (struct in_addr) + 1 + 1)
    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	intfp = o_metric_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
				NDX_SIZE,
				0);
	if (!intfp) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    intfp = o_metric_lookup((unsigned int *) 0,
				    0,
				    TRUE);
	    if (!intfp) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &sock2ip(intfp->intf->ifap->ifa_addr_local),
		    sizeof (sock2ip(intfp->intf->ifap->ifa_addr_local)));
	    INT_OID(ip, intfp->index);
	    /* No support for TOS */
	    INT_OID(ip, 0);
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    intfp = o_metric_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				    j = oid->oid_nelem - ot->ot_name->oid_nelem,
				    TRUE);
	    if (!intfp) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &sock2ip(intfp->intf->ifap->ifa_addr_local),
		    sizeof (sock2ip(intfp->intf->ifap->ifa_addr_local)));
	    INT_OID(ip, intfp->index);
	    /* No support for TOS */
	    INT_OID(ip, 0);
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case ospfIfMetricIpAddress:
	return o_ipaddr(oi,
			v,
			sock2unix(intfp->intf->ifap->ifa_addr_local,
				  (int *) 0));

    case ospfIfMetricAddressLessIf:
	/* All interfaces have addresses */
	return o_integer(oi, v, intfp->index);

    case ospfIfMetricTOS:
	/* No support for TOS */
	return o_integer(oi, v, 0);

    case ospfIfMetricMetric:
	return o_integer(oi, v, intfp->intf->cost);

    case ospfIfMetricStatus:
	return o_integer(oi, v, MIB_VALID);
    }
    
    return int_SNMP_error__status_noSuchName;
}


/* Virtual Interfaces */

/* INDEX { ospfVirtIfAreaID, ospfVirtIfNeighbor } */
#define	NDX_SIZE	(sizeof (struct in_addr) + sizeof (struct in_addr))

static struct intf_entry o_vintf_list = {&o_vintf_list, &o_vintf_list };
static int o_vintf_cnt;
static unsigned int *o_vintf_last;

#define	MVINTF_LIST(intfp)	for (intfp = o_vintf_list.forw; intfp != &o_vintf_list; intfp = intfp->forw)
#define	MVINTF_LIST_END(ip)	

void
o_vintf_get __PF0(void)
{
    register struct intf_entry *intfp;
    register struct INTF *intf;

    /* Free the old list */
    MVINTF_LIST(intfp) {
	register struct intf_entry *intfp2 = intfp->back;

	remque((struct qelem *) intfp);
	task_block_free(o_intf_index, (void_t) intfp);

	intfp = intfp2;
    } MVINTF_LIST_END(intfp) ;

    o_vintf_cnt = 0;
    snmp_last_free(&o_vintf_last);

    VINTF_LIST(intf) {
	u_int32 intf_area = ntohl(intf->transarea->area_id);
	u_int32 nbr_id = ntohl(NBR_ID(&intf->nbr));
	    
	MVINTF_LIST(intfp) {
	    if (intf_area <= ntohl(intfp->intf->transarea->area_id) &
		nbr_id > ntohl(NBR_ID(&intfp->intf->nbr))) {
		break;
	    }
	} MVINTF_LIST_END(intfp) ;

	insque((struct qelem *) task_block_alloc(o_intf_index), intfp);
	intfp->forw->intf = intf;
	o_vintf_cnt++;
    } VINTF_LIST_END(intfp) ;
}


static struct INTF *
o_vintf_lookup __PF3(ip, register unsigned int *,
		     len, size_t,
		     isnext, int)
{
    static struct INTF *last_intf;

    if (snmp_last_match(&o_vintf_last, ip, len, isnext)) {
	return last_intf;
    }
    
    if (!o_vintf_cnt) {
	return last_intf = (struct INTF *) 0;
    }
    
    if (len) {
	u_int32 area_id, nbr_id;
	register struct intf_entry *intfp;

	oid2ipaddr(ip, &area_id);
	GNTOHL(area_id);
	
	oid2ipaddr(ip, &nbr_id);
	GNTOHL(nbr_id);

	if (ip[sizeof (struct in_addr)]) {
	    /* We don't support address less interfaces */
	    return last_intf = (struct INTF *) 0;
	}

	MINTF_LIST(intfp) {
	    register u_int32 cur_area = ntohl(intfp->intf->transarea->area_id);
	    register u_int32 cur_nbr = ntohl(NBR_ID(&intfp->intf->nbr));

	    if (cur_area == area_id) {
		if (nbr_id == cur_nbr) {
		    if (!isnext) {
			return last_intf = intfp->intf;
		    }
		} else if (nbr_id > cur_nbr) {
		    continue;
		}
	    } if (cur_area < area_id) {
		continue;
	    }

	    return last_intf = isnext ? intfp->intf : (struct INTF *) 0;
	} MINTF_LIST_END(ip) ;

	return last_intf = (struct INTF *) 0;
    }

    return last_intf = o_vintf_list.forw->intf;
}


/* Virtual Interface group */
static int
o_vintf __PF3(oi, OI,
	      v, register struct type_SNMP_VarBind *,
	      offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    struct INTF *intf;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	intf = o_vintf_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			      NDX_SIZE,
			      0);
	if (!intf) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    intf = o_vintf_lookup((unsigned int *) 0,
				  0,
				  TRUE);
	    if (!intf) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &intf->transarea->area_id, sizeof (intf->transarea->area_id));
	    STR_OID(ip, &NBR_ID(&intf->nbr), sizeof (NBR_ID(&intf->nbr)));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    intf = o_vintf_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				  j = oid->oid_nelem - ot->ot_name->oid_nelem,
				  TRUE);
	    if (!intf) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &intf->transarea->area_id, sizeof (intf->transarea->area_id));
	    STR_OID(ip, &NBR_ID(&intf->nbr), sizeof (NBR_ID(&intf->nbr)));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE

    switch (ot2object(ot)->ot_info) {
    case ospfVirtIfAreaID:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, intf->transarea->area_id),
				  (int *) 0));

    case ospfVirtIfNeighbor:
	return o_ipaddr(oi,
			v,
			sock2unix(intf->nbr.nbr_id,
				  (int *) 0));

    case ospfVirtIfTransitDelay:
	return o_integer(oi, v, intf->transdly);

    case ospfVirtIfRetransInterval:
	return o_integer(oi, v, intf->retrans_timer);

    case ospfVirtIfHelloInterval:
	return o_integer(oi, v, intf->hello_timer);

    case ospfVirtIfRtrDeadInterval:
	return o_integer(oi, v, intf->dead_timer);

    case ospfVirtIfState:
	return o_integer(oi, v, o_intf_state(intf));

    case ospfVirtIfEvents:
	return o_integer(oi, v, intf->events);

    case ospfVirtIfAuthKey:
	/* When read, ospfIfAuthKey always returns an Octet String of length zero. */
	return o_string(oi, v, (caddr_t) 0, 0);

    case ospfVirtIfStatus:
	return o_integer(oi, v, MIB_VALID);
    }
    
    return int_SNMP_error__status_noSuchName;
}


/* Neighbor group */

/* INDEX { ospfNbrIpAddr, ospfNbrAddressLessIndex } */
#define	NDX_SIZE	(sizeof (struct in_addr) + 1)


struct nbr_entry {
    struct NBR *nbr;
    u_int32 addr;
    u_int index;
};

static struct nbr_entry *o_nbr_list;
static int o_nbr_cnt;
static u_int o_nbr_size;

static int
o_nbr_compare __PF2(p1, void_t,
		    p2, void_t)
{
    register struct nbr_entry *nbrp1 = (struct nbr_entry *) p1;
    register struct nbr_entry *nbrp2 = (struct nbr_entry *) p2;

    if (nbrp1->addr == nbrp2->addr
	&& nbrp1->index == nbrp2->index) {
	return 0;
    } else if (nbrp1->addr < nbrp2->addr
	       || (nbrp1->addr == nbrp2->addr
		   && nbrp1->index < nbrp2->index)) {
	return -1;
    }

    return 1;
}


static void
o_nbr_get __PF0(void)
{
    register struct nbr_entry *nbrp;
    register struct intf_entry *intfp;

    if (o_nbr_size < ospf.nbrcnt + ospf.nintf) {
	if (o_nbr_list) {
	    task_block_reclaim((size_t) (o_nbr_size * sizeof (*o_nbr_list)), (void_t) o_nbr_list);
	}

	o_nbr_size = ospf.nbrcnt + ospf.nintf;
	o_nbr_list = (struct nbr_entry *) task_block_malloc((size_t) (o_nbr_size * sizeof (*o_nbr_list)));
    }

    nbrp = o_nbr_list;
    o_nbr_cnt = 0;
    
    MINTF_LIST(intfp) {
	register struct NBR *nbr;

	NBRS_LIST(nbr, intfp->intf) {
	    assert(o_nbr_cnt <= o_nbr_size);
	    nbrp->nbr = nbr;
	    nbrp->addr = ntohl(NBR_ADDR(nbr));
	    if (BIT_TEST(nbr->intf->ifap->ifa_state, IFS_POINTOPOINT)
		&& nbr->intf->ifap->ifa_addrent_local->ifae_n_if > 1) {
		nbrp->index = nbr->intf->ifap->ifa_index;
	    } else {
		nbrp->index = 0;
	    }
	    nbrp++;
	} NBRS_LIST_END(nbr, intfp->intf) ;
    } MINTF_LIST_END(intfp) ;

    o_nbr_cnt = nbrp - o_nbr_list;

    qsort((caddr_t) o_nbr_list,
	  o_nbr_cnt,
	  sizeof (struct nbr_entry), 
	  o_nbr_compare);
}


static struct nbr_entry *
o_nbr_lookup __PF3(ip, register unsigned int *,
		   len, size_t,
		   isnext, int)
{
    static struct nbr_entry *last_nbrp;
    static unsigned int *last;
    static int last_quantum;

    if (last_quantum != snmp_quantum) {
	last_quantum = snmp_quantum;

	o_nbr_get();

	if (last) {
	    task_mem_free((task *) 0, (void_t) last);
	    last = (unsigned int *) 0;
	}
    }

    if (snmp_last_match(&last, ip, len, isnext)) {
	return last_nbrp;
    }

    if (!o_nbr_cnt) {
	return last_nbrp = (struct nbr_entry *) 0;
    }
    
    if (len) {
	u_int32 nbr_addr = (u_int32) 0;
	int nbr_index;
	register struct nbr_entry *nbrp = o_nbr_list;
	struct nbr_entry *lp = &o_nbr_list[o_nbr_cnt];
	
	oid2ipaddr(ip, &nbr_addr);
	GNTOHL(nbr_addr);

	nbr_index = ip[sizeof (struct in_addr)];

	do {
	    if (nbrp->addr == nbr_addr
		&& nbrp->index == nbr_index) {
		if (!isnext) {
		    return last_nbrp = nbrp;
		}
	    } else if (nbrp->addr > nbr_addr
		       || (nbrp->addr == nbr_addr
			   && nbrp->index == nbr_index)) {
		return last_nbrp = isnext ? nbrp : (struct nbr_entry *) 0;
	    }
	} while (++nbrp < lp) ;

	return last_nbrp = (struct nbr_entry *) 0;
    }

    return last_nbrp = o_nbr_list;
}


static inline int
o_nbr_state __PF1(s_nbr, struct NBR *)
{
    int state;
    
    switch (s_nbr->state) {
    case NDOWN:
	state = N_STATE_DOWN;
	break;

    case NATTEMPT:
	state = N_STATE_ATTEMPT;
	break;
	
    case NINIT:
	state = N_STATE_INIT;
	break;
	
    case N2WAY:
	state = N_STATE_2WAY;
	break;
	
    case NEXSTART:
	state = N_STATE_EXSTART;
	break;
	
    case NEXCHANGE:
	state = N_STATE_EXCHANGE;
	break;
	
    case NLOADING:
	state = N_STATE_LOADING;
	break;

    case NFULL:
	state = N_STATE_FULL;
	break;

    default:
	state = -1;
	break;
    }

    return state;
}


static int
o_nbr __PF3(oi, OI,
	    v, register struct type_SNMP_VarBind *,
	    offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    struct nbr_entry *nbrp;

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
	}
	nbrp = o_nbr_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			    NDX_SIZE,
			    0);
	if (!nbrp) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    nbrp = o_nbr_lookup((unsigned int *) 0,
				0,
				TRUE);
	    if (!nbrp) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &sock2ip(nbrp->nbr->nbr_addr), sizeof (sock2ip(nbrp->nbr->nbr_addr)));
	    INT_OID(ip, nbrp->index);
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    nbrp = o_nbr_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				j = oid->oid_nelem - ot->ot_name->oid_nelem,
				TRUE);
	    if (!nbrp) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &sock2ip(nbrp->nbr->nbr_addr), sizeof (sock2ip(nbrp->nbr->nbr_addr)));
	    INT_OID(ip, nbrp->index);
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE
    
    switch (ot2object(ot)->ot_info) {
    case ospfNbrIpAddr:
	return o_ipaddr(oi,
			v,
			sock2unix(nbrp->nbr->nbr_addr,
				  (int *) 0));

    case ospfNbrAddressLessIndex:
	return o_integer(oi, v, nbrp->index);

    case ospfNbrRtrId:
	return o_ipaddr(oi,
			v,
			sock2unix(nbrp->nbr->nbr_id ? nbrp->nbr->nbr_id : inet_addr_default,
				  (int *) 0));

    case ospfNbrOptions:
	i = 0;

	if (!BIT_TEST(nbrp->nbr->intf->area->area_flags, OSPF_AREAF_STUB)) {
	    BIT_SET(i, MIB_BIT_ASE);
	}

	/* TOS not supported */
	return o_integer(oi, v, i);

    case ospfNbrPriority:
	return o_integer(oi, v, nbrp->nbr->pri);

    case ospfNbrState:
	return o_integer(oi, v, o_nbr_state(nbrp->nbr));

    case ospfNbrEvents:
	return o_integer(oi, v, nbrp->nbr->events);

    case ospfNbrLSRetransQLen:
	return o_integer(oi, v, nbrp->nbr->dbcnt + nbrp->nbr->reqcnt + nbrp->nbr->rtcnt);

    case ospfNBMANbrStatus:
	switch (nbrp->nbr->intf->type) {
	case NONBROADCAST:
	    return o_integer(oi, v, MIB_VALID);

	default:
	    return NOTOK;
	}

    }
    
    return int_SNMP_error__status_noSuchName;
}


/* Virtual neighbor group */
static int
o_vnbr __PF3(oi, OI,
	     v, register struct type_SNMP_VarBind *,
	     offset, int)
{
    register int    i;
    register unsigned int *ip,
			  *jp;
    register OID    oid = oi->oi_name;
    register OT	    ot = oi->oi_type;
    OID		    new;
    struct INTF *intf;

    /* INDEX { ospfVirtNbrArea, ospfVirtNbrRtrId } */
#define	NDX_SIZE	(sizeof (struct in_addr) + sizeof (struct in_addr))

    switch (offset) {
    case type_SNMP_SMUX__PDUs_get__request:
	if (oid->oid_nelem - ot->ot_name->oid_nelem != NDX_SIZE) {
	    return int_SNMP_error__status_noSuchName;
}	
	intf = o_vintf_lookup(oid->oid_elements + oid->oid_nelem - NDX_SIZE,
			      NDX_SIZE,
			      0);
	if (!intf) {
	    return int_SNMP_error__status_noSuchName;
	}
	break;

    case type_SNMP_SMUX__PDUs_get__next__request:
	/* next request with incomplete instance? */
	if ((i = oid->oid_nelem - ot->ot_name->oid_nelem) != 0 && i < NDX_SIZE) {
	    for (jp = (ip = oid->oid_elements + 
		       ot->ot_name->oid_nelem - 1) + i;
		 jp > ip;
		 jp--) {
		if (*jp != 0) {
		    break;
		}
	    }
	    if (jp == ip) {
		oid->oid_nelem = ot->ot_name->oid_nelem;
	    } else {
		if ((new = oid_normalize(oid, NDX_SIZE - i, 256)) == NULLOID) {
			return NOTOK;
		    }
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    }
	}

	/* next request with no instance? */
	if (oid->oid_nelem == ot->ot_name->oid_nelem) {
	    intf = o_vintf_lookup((unsigned int *) 0,
				  0,
				  TRUE);
	    if (!intf) {
		return NOTOK;
	    }
	    
	    if ((new = oid_extend(oid, NDX_SIZE)) == NULLOID) {
		return int_SNMP_error__status_genErr;
	    }

	    ip = new->oid_elements + new->oid_nelem - NDX_SIZE;
	    STR_OID(ip, &intf->transarea->area_id, sizeof (intf->transarea->area_id));
	    STR_OID(ip, &NBR_ID(&intf->nbr), sizeof (NBR_ID(&intf->nbr)));
		
	    if (v->name) {
		free_SNMP_ObjectName(v->name);
	    }
	    v->name = new;
	} else {
	    size_t j;

	    intf = o_vintf_lookup(ip = oid->oid_elements + ot->ot_name->oid_nelem,
				  j = oid->oid_nelem - ot->ot_name->oid_nelem,
				  TRUE);
	    if (!intf) {
		return NOTOK;
	    }

	    if ((i = j - NDX_SIZE) < 0) {
		if ((new = oid_extend(oid, -i)) == NULLOID) {
		    return int_SNMP_error__status_genErr;
		}
		if (v->name) {
		    free_SNMP_ObjectName(v->name);
		}
		v->name = oid = new;
	    } else if (i > 0) {
		oid->oid_nelem -= i;
	    }
		
	    ip = oid->oid_elements + ot->ot_name->oid_nelem;
	    STR_OID(ip, &intf->transarea->area_id, sizeof (intf->transarea->area_id));
	    STR_OID(ip, &NBR_ID(&intf->nbr), sizeof (NBR_ID(&intf->nbr)));
	}
	break;

    default:
	return int_SNMP_error__status_genErr;
    }
#undef	NDX_SIZE
    
    switch (ot2object(ot)->ot_info) {
    case ospfVirtNbrArea:
	return o_ipaddr(oi,
			v,
			sock2unix(sockbuild_in(0, intf->transarea->area_id),
				  (int *) 0));

    case ospfVirtNbrRtrId:
	return o_ipaddr(oi,
			v,
			sock2unix(intf->nbr.nbr_id ? intf->nbr.nbr_id : inet_addr_default,
				  (int *) 0));

    case ospfVirtNbrIpAddr:
	return o_ipaddr(oi,
			v,
			sock2unix(intf->nbr.nbr_id ? intf->nbr.nbr_id : inet_addr_default,
				  (int *) 0));

    case ospfVirtNbrOptions:
	i = 0;
	
	if (!BIT_TEST(intf->area->area_flags, OSPF_AREAF_STUB)) {
	    BIT_SET(i, MIB_BIT_ASE);
	}

	/* TOS not supported */
	return o_integer(oi, v, i);

    case ospfVirtNbrState:
	return o_integer(oi, v, o_nbr_state(&intf->nbr));

    case ospfVirtNbrEvents:
	return o_integer(oi, v, intf->nbr.events);

    case ospfVirtNbrLSRetransQLen:
	return o_integer(oi, v, intf->nbr.dbcnt + intf->nbr.reqcnt + intf->nbr.rtcnt);

    }
    
    return int_SNMP_error__status_noSuchName;
}


void
ospf_init_mib(enabled)
int enabled;
{
    if (enabled) {
	if (!o_intf_index) {
	    o_intf_index = task_block_init(sizeof (struct intf_entry));
	    o_lsdb_index = task_block_init(sizeof (struct lsdb_entry));
	}
	snmp_tree_register(&ospf_mib_tree);

    } else {
	snmp_tree_unregister(&ospf_mib_tree);
    }
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
