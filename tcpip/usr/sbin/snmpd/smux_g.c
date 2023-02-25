static char sccsid[] = "@(#)09	1.7  src/tcpip/usr/sbin/snmpd/smux_g.c, snmp, tcpip411, GOLD410 10/29/93 10:38:31";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: o_smuxPeer(), o_smuxTree(), get_tbent(), init_smux(), 
 *            s_smuxPeer(), s_smuxTree(), get_tbent()
 *
 * ORIGINS: 27 60
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Licensed Material - Property of IBM
 *
 * FILE:	src/tcpip/usr/sbin/snmpd/smux_g.c
 */

/* 
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 */

/* smux_g.c - SMUX group */


#include <isode/snmp/io.h>
#include "snmpd.h"
#include "smux_g.h"

/*    SMUX GROUP */

#ifdef	SMUX
#define	smuxPindex	0
#define	smuxPidentity	1
#define	smuxPdescription 2
#define	smuxPstatus	3

#define	PB_VALID	1		/* smuxPstatus */
#define	PB_INVALID	2		/*   .. */
#define	PB_CONNECTING	3		/*   .. */

struct smuxPeer peerque;
struct smuxPeer *PHead = &peerque;

struct smuxTree treeque;
struct smuxTree *THead = &treeque;

struct smuxClient clientque;
struct smuxClient *SHead = &clientque;

static int  o_smuxPeer (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifnum,
	    ifvar;
    register struct smuxPeer *pb;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    ifnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    for (pb = PHead -> pb_forw; pb != PHead; pb = pb -> pb_forw)
		if (pb -> pb_index == ifnum)
		    break;
	    if (pb == PHead
		    || ((ifvar == smuxPidentity || ifvar == smuxPdescription)
			    && pb -> pb_identity == NULL))
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		for (pb = PHead -> pb_forw; pb != PHead; pb = pb -> pb_forw) {
		    if ((ifvar == smuxPidentity || ifvar == smuxPdescription)
			    && pb -> pb_identity == NULL) 
			continue;
		    break;
		}

		if (pb == PHead)
		    return NOTOK;
		ifnum = pb -> pb_index;

		if ((new = oid_extend (oid, 1)) == NULLOID)
		    return NOTOK;
		new -> oid_elements[new -> oid_nelem - 1] = ifnum;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	i = ot -> ot_name -> oid_nelem;

		ifnum = oid -> oid_elements[i];
		for (pb = PHead -> pb_forw; pb != PHead; pb = pb -> pb_forw) 
		    if (pb -> pb_index >= ifnum) {
			if ((ifvar == smuxPidentity || 
				ifvar == smuxPdescription)
				&& pb -> pb_identity == NULL) 
			    continue;
			break;
		    }

		if (pb == PHead
		        || ((pb -> pb_index == ifnum)
			        && (pb = pb -> pb_forw) == PHead))
		    return NOTOK;
		ifnum = pb -> pb_index;

		oid -> oid_elements[i] = ifnum;
		oid -> oid_nelem = i + 1;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case smuxPindex:
	    return o_integer (oi, v, pb -> pb_index);

	case smuxPidentity:
	    return (pb -> pb_identity ? 
		    o_specific (oi, v, (caddr_t) pb -> pb_identity) : 
			(offset == type_SNMP_PDUs_get__next__request ? NOTOK : 
			int_SNMP_error__status_noSuchName));

	case smuxPdescription:
	    return (pb -> pb_identity ? 
		    o_string (oi, v, pb -> pb_description,
			     strlen (pb -> pb_description)) :
			(offset == type_SNMP_PDUs_get__next__request ? NOTOK : 
			int_SNMP_error__status_noSuchName));

	case smuxPstatus:
	    return o_integer (oi, v, pb -> pb_identity ? PB_VALID
						       : PB_CONNECTING);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

static int  
s_smuxPeer (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifnum,
	    ifvar;
    register struct smuxPeer *pb;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    caddr_t value;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	    if (oid -> oid_nelem != ot -> ot_name -> oid_nelem + 1)
		return int_SNMP_error__status_noSuchName;
	    ifnum = oid -> oid_elements[oid -> oid_nelem - 1];
	    for (pb = PHead -> pb_forw; pb != PHead; pb = pb -> pb_forw)
		if (pb -> pb_index == ifnum)
		    break;
	    if (pb == PHead
		    || ((ifvar == smuxPidentity || ifvar == smuxPdescription)
			    && pb -> pb_identity == NULL))
		return int_SNMP_error__status_noSuchName;
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    if (os == NULLOS) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXG, SMUXG_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }

    switch (offset) {
	case type_SNMP_PDUs_set__request:
	    if ((*os -> os_decode) (&value, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    pb -> pb_newstatus = *((int *) value);
	    (*os -> os_free) (value);
	    switch (pb -> pb_newstatus) {
		case PB_VALID:
		    if (!pb -> pb_identity)
			return int_SNMP_error__status_badValue;
		    break;

		case PB_INVALID:
		    break;

		default:
		    return int_SNMP_error__status_badValue;
	    }
	    break;

	case type_SNMP_PDUs_commit:
	    if (pb -> pb_newstatus == PB_INVALID)
		pb_free (pb);
	    break;

	case type_SNMP_PDUs_rollback:
	    break;
    }

    return int_SNMP_error__status_noError;
}

/*  */

#define	smuxTsubtree	0
#define	smuxTpriority	1
#define	smuxTindex	2
#define	smuxTstatus	3

#define	TB_VALID	1		/* smuxTstatus */
#define	TB_INVALID	2		/*   ...   */


struct smuxTree *get_tbent ();


static int  
o_smuxTree (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
    int	    ifvar;
    register int    i;
    register unsigned int *ip,
			  *jp;
    register struct smuxTree *tb;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;

    ifvar = (int) ot -> ot_info;
    switch (offset) {
	case type_SNMP_PDUs_get__request:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((tb = get_tbent (oid -> oid_elements
				     + ot -> ot_name -> oid_nelem,
				 oid -> oid_nelem
				     - ot -> ot_name -> oid_nelem, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	case type_SNMP_PDUs_get__next__request:
	    if (oid -> oid_nelem == ot -> ot_name -> oid_nelem) {
		OID	new;

		if ((tb = THead -> tb_forw) == THead)
		    return NOTOK;

		if ((new = oid_extend (oid, tb -> tb_insize)) == NULLOID)
		    return NOTOK;
		ip = new -> oid_elements + new -> oid_nelem - tb -> tb_insize;
		jp = tb -> tb_instance;
		for (i = tb -> tb_insize; i > 0; i--)
		    *ip++ = *jp++;

		if (v -> name)
		    free_SMI_ObjectName (v -> name);
		v -> name = new;
	    }
	    else {
		int	j;

		if ((tb = get_tbent (oid -> oid_elements
				         + ot -> ot_name -> oid_nelem,
				     j = oid -> oid_nelem
				     	     - ot -> ot_name -> oid_nelem, 1))
		         == NULL)
		    return NOTOK;

		if ((i = j - tb -> tb_insize) < 0) {
		    OID	    new;

		    if ((new = oid_extend (oid, -i)) == NULLOID)
			return NOTOK;
		    if (v -> name)
			free_SMI_ObjectName (v -> name);
		    v -> name = new;

		    oid = new;
		}
		else
		    if (i > 0)
			oid -> oid_nelem -= i;

		ip = oid -> oid_elements + ot -> ot_name -> oid_nelem;
		jp = tb -> tb_instance;
		for (i = tb -> tb_insize; i > 0; i--)
		    *ip++ = *jp++;
	    }
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    switch (ifvar) {
	case smuxTsubtree:
	    return o_specific (oi, v, (caddr_t) tb -> tb_subtree -> ot_name);

	case smuxTpriority:
	    return o_integer (oi, v, tb -> tb_priority);

	case smuxTindex:
	    return o_integer (oi, v, tb -> tb_peer -> pb_index);

	case smuxTstatus:
	    return o_integer (oi, v, TB_VALID);

	default:
	    return int_SNMP_error__status_noSuchName;
    }
}

static int  
s_smuxTree (oi, v, offset)
OI	oi;
register struct type_SNMP_VarBind *v;
int	offset;
{
#ifndef	lint
    int	    ifvar;
#endif
    register struct smuxTree *tb;
    register OID    oid = oi -> oi_name;
    register OT	    ot = oi -> oi_type;
    register OS	    os = ot -> ot_syntax;
    caddr_t value;

#ifndef	lint
    ifvar = (int) ot -> ot_info;
#endif
    switch (offset) {
	case type_SNMP_PDUs_set__request:
	case type_SNMP_PDUs_commit:
	case type_SNMP_PDUs_rollback:
	    if (oid -> oid_nelem <= ot -> ot_name -> oid_nelem)
		return int_SNMP_error__status_noSuchName;
	    if ((tb = get_tbent (oid -> oid_elements
				     + ot -> ot_name -> oid_nelem,
				 oid -> oid_nelem
				     - ot -> ot_name -> oid_nelem, 0)) == NULL)
		return int_SNMP_error__status_noSuchName;
	    break;

	default:
	    return int_SNMP_error__status_genErr;
    }

    if (os == NULLOS) {
	advise (SLOG_EXCEPTIONS, MSGSTR(MS_SMUXG, SMUXG_1,
		"no syntax defined for object \"%s\""), ot -> ot_text);

	return int_SNMP_error__status_genErr;
    }

    switch (offset) {
	case type_SNMP_PDUs_set__request:
	    if ((*os -> os_decode) (&value, v -> value) == NOTOK)
		return int_SNMP_error__status_badValue;
	    tb -> tb_newstatus = *((int *) value);
	    (*os -> os_free) (value);
	    switch (tb -> tb_newstatus) {
		case TB_VALID:
		case TB_INVALID:
		    break;

		default:
		    return int_SNMP_error__status_badValue;
	    }
	    break;

	case type_SNMP_PDUs_commit:
	    if (tb -> tb_newstatus == TB_INVALID)
		tb_free (tb);
	    break;

	case type_SNMP_PDUs_rollback:
	    break;
    }

    return int_SNMP_error__status_noError;
}

/*  */

static struct smuxTree *
get_tbent (ip, len, isnext)
register unsigned int *ip;
int	len;
int	isnext;
{
    register struct smuxTree *tb;

    for (tb = THead -> tb_forw; tb != THead; tb = tb -> tb_forw)
	switch (elem_cmp (tb -> tb_instance, tb -> tb_insize, ip, len)) {
	    case 0:
	        if (!isnext)
		    return tb;
		if ((tb = tb -> tb_forw) == THead)
		    return NULL;
		/* else fall... */

	    case 1:
		return (isnext ? tb : NULL);
	}

    return NULL;
}

/*  */

init_smux () 
{
    register OT	    ot;

    PHead -> pb_forw = PHead -> pb_back = PHead;
    THead -> tb_forw = THead -> tb_back = THead;
    SHead -> sc_forw = SHead -> sc_back = SHead;

    init_reserved ();

    if (ot = text2obj ("smuxPindex"))
	ot -> ot_getfnx = o_smuxPeer,
	ot -> ot_info = (caddr_t) smuxPindex;
    if (ot = text2obj ("smuxPidentity"))
	ot -> ot_getfnx = o_smuxPeer,
	ot -> ot_info = (caddr_t) smuxPidentity;
    if (ot = text2obj ("smuxPdescription"))
	ot -> ot_getfnx = o_smuxPeer,
	ot -> ot_info = (caddr_t) smuxPdescription;
    if (ot = text2obj ("smuxPstatus"))
	ot -> ot_getfnx = o_smuxPeer,
	ot -> ot_setfnx = s_smuxPeer,
	ot -> ot_info = (caddr_t) smuxPstatus;

    if (ot = text2obj ("smuxTsubtree"))
	ot -> ot_getfnx = o_smuxTree,
	ot -> ot_info = (caddr_t) smuxTsubtree;
    if (ot = text2obj ("smuxTpriority"))
	ot -> ot_getfnx = o_smuxTree,
	ot -> ot_info = (caddr_t) smuxTpriority;
    if (ot = text2obj ("smuxTindex"))
	ot -> ot_getfnx = o_smuxTree,
	ot -> ot_info = (caddr_t) smuxTindex;
    if (ot = text2obj ("smuxTstatus"))
	ot -> ot_getfnx = o_smuxTree,
	ot -> ot_setfnx = s_smuxTree,
	ot -> ot_info = (caddr_t) smuxTstatus;
}
#else

init_smux () {}

#endif
