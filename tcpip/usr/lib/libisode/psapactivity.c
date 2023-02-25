static char sccsid[] = "@(#)74	1.3  src/tcpip/usr/lib/libisode/psapactivity.c, isodelib7, tcpip411, GOLD410 4/5/93 15:10:09";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PActIntrRequestAux PActIntrResponseAux PActResumeRequest 
 *    PActStartRequest PGControlRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/psapactivity.c
 */

/* psapactivity.c - PPM: activities */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapactivity.c,v 1.2 93/02/05 17:07:30 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapactivity.c,v 1.2 93/02/05 17:07:30 snmp Exp $
 *
 *
 * $Log:	psapactivity.c,v $
 * Revision 1.2  93/02/05  17:07:30  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:29  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:14:18  mrose
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

/*    P-CONTROL-GIVE.REQUEST */

int	PGControlRequest (sd, pi)
int	sd;
struct PSAPindication *pi;
{
    SBV	    smask;
    int     result;
    register struct psapblk *pb;
    struct SSAPindication   sis;
    register struct SSAPabort  *sa = &sis.si_abort;

    missingP (pi);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = SGControlRequest (sd, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, "SGControlRequest", sa);
	else {
	    (void) ss2pslose (NULLPB, pi, "SGControlRequest", sa);
	    goto out1;
	}
    else
	pb -> pb_owned = 0;

    if (result == NOTOK)
	freepblk (pb);
out1: ;
    (void) sigiomask (smask);

    return result;
}

/*    P-ACTIVITY-START.REQUEST */

int	PActStartRequest (sd, id, data, ndata, pi)
int	sd;
struct SSAPactid *id;
int	ndata;
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

    toomuchP (data, ndata, NPDATA, "activity start");
    missingP (pi);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
			     "P-ACTIVITY-START user-data", PPDU_NONE)) != OK)
	goto out2;

    if ((result = SActStartRequest (sd, id, base, len, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, "SActStartRequest", sa);
	else {
	    (void) ss2pslose (NULLPB, pi, "SActStartRequest", sa);
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
    
/*    P-ACTIVITY-RESUME.REQUEST */

int	PActResumeRequest (sd, id, oid, ssn, ref, data, ndata, pi)
int	sd;
struct SSAPactid *id,
		 *oid;
int	ndata;
long	ssn;
struct SSAPref *ref;
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

    toomuchP (data, ndata, NPDATA, "activity resume");
    missingP (pi);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
			     "P-ACTIVITY-RESUME user-data", PPDU_NONE)) != OK)
	goto out2;

    if ((result = SActResumeRequest (sd, id, oid, ssn, ref, base, len, &sis))
	    == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, "SActResumeRequest", sa);
	else {
	    (void) ss2pslose (NULLPB, pi, "SActResumeRequest", sa);
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
    
/*    P-ACTIVITY-{INTERRUPT,DISCARD}.REQUEST */

int	PActIntrRequestAux (sd, reason, pi, sfunc, stype)
int	sd;
int	reason;
struct PSAPindication *pi;
char   *stype;
IFP	sfunc;
{
    SBV	    smask;
    int     result;
    register struct psapblk *pb;
    struct SSAPindication   sis;
    register struct SSAPabort  *sa = &sis.si_abort;

    missingP (pi);
    missingP (sfunc);
    missingP (stype);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = (*sfunc) (sd, reason, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, stype, sa);
	else {
	    (void) ss2pslose (NULLPB, pi, stype, sa);
	    goto out1;
	}

    if (result == NOTOK)
	freepblk (pb);
out1: ;
    (void) sigiomask (smask);

    return result;
}
    
/*    P-ACTIVITY-{INTERRUPT,DISCARD}.RESPONSE */

int	PActIntrResponseAux (sd, pi, sfunc, stype)
int	sd;
struct PSAPindication *pi;
char   *stype;
IFP	sfunc;
{
    SBV	    smask;
    int     result;
    register struct psapblk *pb;
    struct SSAPindication   sis;
    register struct SSAPabort  *sa = &sis.si_abort;

    missingP (pi);
    missingP (sfunc);
    missingP (stype);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = (*sfunc) (sd, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, stype, sa);
	else {
	    (void) ss2pslose (NULLPB, pi, stype, sa);
	    goto out1;
	}

    if (result == NOTOK)
	freepblk (pb);
out1: ;
    (void) sigiomask (smask);

    return result;
}
