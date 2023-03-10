static char sccsid[] = "@(#)75	1.3  src/tcpip/usr/lib/libisode/ssapactchk.c, isodelib7, tcpip411, GOLD410 4/5/93 16:45:17";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: SDoActivityAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ssapactchk.c
 */

/* ssapactchk.c - SPM: check activity constraints */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapactchk.c,v 1.2 93/02/05 17:11:50 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapactchk.c,v 1.2 93/02/05 17:11:50 snmp Exp $
 *
 *
 * $Log:	ssapactchk.c,v $
 * Revision 1.2  93/02/05  17:11:50  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:45:40  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:21  mrose
 * Release 6.0
 * 
 */

/*
 *				  NOTICE
 *
 *    Acquisition, use, and distribution of this module and related
 *    materials are subject to the restrictions of a license agreement.
 *    Consult the Preface in the User's Manual for the full terms of
 *    this agreement.
 *
 */


/* LINTLIBRARY */

#include <stdio.h>
#include <isode/spkt.h>

/*  */

int	SDoActivityAux (sb, si, act, rls)
register struct ssapblk *sb;
register struct SSAPindication *si;
int	act,
	rls;
{
    if (act) {
	if (!(sb -> sb_requirements & SR_ACT_EXISTS))
	    return ssaplose (si, SC_OPERATION, NULLCP,
		    "activity management service unavailable");

	if (sb -> sb_flags & SB_Vact)
	    return ssaplose (si, SC_OPERATION, NULLCP, "activity in progress");
    }
    else
	if (!(sb -> sb_requirements & SR_MAJ_EXISTS))
	    return ssaplose (si, SC_OPERATION, NULLCP,
		    "major synchronize service unavailable");

    if ((sb -> sb_requirements & SR_DAT_EXISTS)
	    && !(sb -> sb_owned & ST_DAT_TOKEN))
	return ssaplose (si, SC_OPERATION, NULLCP,
		"data token not owned by you");

    if ((sb -> sb_requirements & SR_MIN_EXISTS)
	    && !(sb -> sb_owned & ST_MIN_TOKEN))
	return ssaplose (si, SC_OPERATION, NULLCP,
		"minorsync not owned by you");

    if (act) {
	if (!(sb -> sb_owned & ST_ACT_TOKEN))
	    return ssaplose (si, SC_OPERATION, NULLCP,
		    "activity token not owned by you");
    }
    else
	if (!(sb -> sb_owned & ST_MAJ_TOKEN))
	    return ssaplose (si, SC_OPERATION, NULLCP,
		    "majorsync token not owned by you");

    if (rls)
	if ((sb -> sb_requirements & SR_RLS_EXISTS)
		&& !(sb -> sb_owned & ST_RLS_TOKEN))
	    return ssaplose (si, SC_OPERATION, NULLCP,
		    "release token not owned by you");

    return OK;
}
