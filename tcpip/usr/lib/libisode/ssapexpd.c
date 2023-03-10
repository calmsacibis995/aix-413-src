static char sccsid[] = "@(#)81	1.3  src/tcpip/usr/lib/libisode/ssapexpd.c, isodelib7, tcpip411, GOLD410 4/5/93 16:49:21";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: SExpdRequest SExpdRequestAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ssapexpd.c
 */

/* ssapexpd.c - SPM: write expedited data */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapexpd.c,v 1.2 93/02/05 17:12:01 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapexpd.c,v 1.2 93/02/05 17:12:01 snmp Exp $
 *
 *
 * $Log:	ssapexpd.c,v $
 * Revision 1.2  93/02/05  17:12:01  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:45:47  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:27  mrose
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
#include <signal.h>
#include <isode/spkt.h>

/*    S-EXPEDITED-DATA.REQUEST */

int	SExpdRequest (sd, data, cc, si)
int	sd;
char   *data;
int	cc;
struct SSAPindication  *si;
{
    SBV	    smask;
    int     result;
    register struct ssapblk *sb;

    missingP (data);
    if (cc > SX_EXSIZE)
	return ssaplose (si, SC_PARAMETER, NULLCP,
			 "too much expedited user data, %d octets", cc);
    missingP (si);

    smask = sigioblock ();

    ssapPsig (sb, sd);

    result = SExpdRequestAux (sb, data, cc, si);

    (void) sigiomask (smask);

    return result;
}

/*  */

static int  SExpdRequestAux (sb, data, cc, si)
register struct ssapblk *sb;
char   *data;
int	cc;
struct SSAPindication  *si;
{
    int     result;
    register struct ssapkt *s;

    if (!(sb -> sb_requirements & SR_EXPEDITED))
	return ssaplose (si, SC_OPERATION, NULLCP,
		    "expedited data service unavailable");

    if ((s = newspkt (SPDU_EX)) == NULL)
	return ssaplose (si, SC_CONGEST, NULLCP, "out of memory");

    s -> s_udata = data, s -> s_ulen = cc;
    result = spkt2sd (s, sb -> sb_fd, 1, si);
    s -> s_udata = NULL, s -> s_ulen = 0;

    freespkt (s);

    if (result == NOTOK)
	freesblk (sb);

    return result;
}
