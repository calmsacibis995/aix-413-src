static char sccsid[] = "@(#)79	1.3  src/tcpip/usr/lib/libisode/psapmajor2.c, isodelib7, tcpip411, GOLD410 4/5/93 15:12:53";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PMajSyncResponseAux
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/psapmajor2.c
 */

/* psapmajor2.c - PPM: respond to majorsyncs */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapmajor2.c,v 1.2 93/02/05 17:07:43 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapmajor2.c,v 1.2 93/02/05 17:07:43 snmp Exp $
 *
 *
 * $Log:	psapmajor2.c,v $
 * Revision 1.2  93/02/05  17:07:43  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:36  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:14:23  mrose
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
#include <isode/ppkt.h>

/*    P-{MAJOR-SYNC,ACTIVITY-END}.RESPONSE */

int	PMajSyncResponseAux (sd, data, ndata, pi, dtype, sfunc, stype)
int	sd;
int	ndata;
PE     *data;
struct PSAPindication *pi;
char   *dtype,
       *stype;
IFP	sfunc;
{
    SBV	    smask;
    int     len,
	    result;
    char   *base,
	   *realbase;
    register struct psapblk *pb;
    struct SSAPindication   sis;
    register struct SSAPabort  *sa = &sis.si_abort;

    toomuchP (data, ndata, NPDATA, dtype);
    missingP (pi);
    missingP (sfunc);
    missingP (stype);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
			     "P-MAJOR-SYNC (ACTIVITY-END) user-data",
			     PPDU_NONE)) != OK)
	goto out2;

    if ((result = (*sfunc) (sd, base, len, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, stype, sa);
	else {
	    (void) ss2pslose (NULLPB, pi, stype, sa);
	    goto out1;
	}

out2: ;
    if (result == NOTOK)
	freepblk (pb);
    else
	if (result == DONE)
	    result = NOTOK;
out1: ;
    if (realbase)
	free (realbase);
    else
	if (base)
	    free (base);

    (void) sigiomask (smask);

    return result;
}
