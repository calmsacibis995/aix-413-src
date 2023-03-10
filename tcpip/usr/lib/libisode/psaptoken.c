static char sccsid[] = "@(#)90	1.3  src/tcpip/usr/lib/libisode/psaptoken.c, isodelib7, tcpip411, GOLD410 4/5/93 15:18:47";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PGTokenRequest PPTokenRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/psaptoken.c
 */

/* psaptoken.c - PPM: tokens */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psaptoken.c,v 1.2 93/02/05 17:08:08 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psaptoken.c,v 1.2 93/02/05 17:08:08 snmp Exp $
 *
 *
 * $Log:	psaptoken.c,v $
 * Revision 1.2  93/02/05  17:08:08  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:54  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:14:36  mrose
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

/*    P-TOKEN-GIVE.REQUEST */

int	PGTokenRequest (sd, tokens, pi)
int	sd;
int	tokens;
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

    if ((result = SGTokenRequest (sd, tokens, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, "SGTokenRequest", sa);
	else {
	    (void) ss2pslose (NULLPB, pi, "SGTokenRequest", sa);
	    goto out1;
	}
    else
	pb -> pb_owned &= ~tokens;

    if (result == NOTOK)
	freepblk (pb);
out1: ;
    (void) sigiomask (smask);

    return result;
}
    
/*    P-TOKEN-PLEASE.REQUEST */

int	PPTokenRequest (sd, tokens, data, ndata, pi)
int	sd;
int	tokens,
	ndata;
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

    toomuchP (data, ndata, NPDATA, "token");
    missingP (pi);

    smask = sigioblock ();

    psapPsig (pb, sd);

    if ((result = info2ssdu (pb, pi, data, ndata, &realbase, &base, &len,
			     "P-TOKEN-PLEASE user-data", PPDU_NONE)) != OK)
	goto out2;

    if ((result = SPTokenRequest (sd, tokens, base, len, &sis)) == NOTOK)
	if (SC_FATAL (sa -> sa_reason))
	    (void) ss2pslose (pb, pi, "SPTokenRequest", sa);
	else {
	    (void) ss2pslose (NULLPB, pi, "SPTokenRequest", sa);
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
