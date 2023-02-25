static char sccsid[] = "@(#)87	1.3  src/tcpip/usr/lib/libisode/psapresync2.c, isodelib7, tcpip411, GOLD410 4/5/93 15:16:30";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PReSyncResponse
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/psapresync2.c
 */

/* psapresync2.c - PPM: respond to resyncs */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapresync2.c,v 1.2 93/02/05 17:07:59 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapresync2.c,v 1.2 93/02/05 17:07:59 snmp Exp $
 *
 *
 * $Log:	psapresync2.c,v $
 * Revision 1.2  93/02/05  17:07:59  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:48  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:14:32  mrose
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

/*    P-RESYNCHRONIZE.RESPONSE */

int	PReSyncResponse (sd, ssn, settings, data, ndata, pi)
int	sd;
int	settings,
	ndata;
long	ssn;
PE     *data;
struct PSAPindication *pi;
{
    SBV	    smask;
    int     len,
	    result;
    char   *base,
	   *realbase;
    register struct psapblk *pb;
    struct SSAPindication   sis;
    register struct SSAPabort  *sa = &sis.si_abort;

    toomuchP (data, ndata, NPDATA, "resync");
    missingP (pi);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
			     "P-RESYNCHRONIZE user-data", PPDU_RSA)) != OK)
	goto out2;

    if ((result = SReSyncResponse (sd, ssn, settings, base, len, &sis))
	    == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, "SReSyncResponse", sa);
	else {
	    (void) ss2pslose (NULLPB, pi, "SReSyncResponse", sa);
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
