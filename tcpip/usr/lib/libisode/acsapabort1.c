static char sccsid[] = "@(#)70	1.3  src/tcpip/usr/lib/libisode/acsapabort1.c, isodelib7, tcpip411, GOLD410 4/5/93 13:37:58";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: AcUAbortRequest
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/acsapabort1.c
 */

/* acsapabort1.c - ACPM: user abort */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/acsapabort1.c,v 1.2 93/02/05 17:02:50 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/acsapabort1.c,v 1.2 93/02/05 17:02:50 snmp Exp $
 *
 *
 * $Log:	acsapabort1.c,v $
 * Revision 1.2  93/02/05  17:02:50  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:14:02  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/07/01  21:01:49  mrose
 * pepsy
 * 
 * Revision 7.0  89/11/23  21:21:44  mrose
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
#include "ACS-types.h"
#define	ACSE
#include <isode/acpkt.h>
#include <isode/tailor.h>

/*    A-ABORT.REQUEST */

int	AcUAbortRequest (sd, data, ndata, aci)
int	sd;
PE     *data;
int	ndata;
struct AcSAPindication *aci;
{
    SBV     smask;
    int     result = 0;
    register struct assocblk  *acb;
    PE	    pe;
    struct PSAPindication pis;
    register struct PSAPindication *pi = &pis;
    register struct PSAPabort  *pa = &pi -> pi_abort;
    register struct type_ACS_ABRT__apdu *pdu;

    toomuchP (data, ndata, NACDATA, "release");
    missingP (aci);

    smask = sigioblock ();

    if ((acb = findacblk (sd)) == NULL) {
	(void) sigiomask (smask);
	return acsaplose (aci, ACS_PARAMETER, NULLCP,
		"invalid association descriptor");
    }

    pdu = NULL;
    pe = NULLPE;
    if (acb -> acb_sversion == 1) {
	if ((result = PUAbortRequest (acb -> acb_fd, data, ndata, pi))
	    	== NOTOK) {
	    (void) ps2acslose (acb, aci, "PUAbortRequest", pa);
	    if (PC_FATAL (pa -> pa_reason))
		goto out2;
	    else
		goto out1;
	}

	result = OK;
	acb -> acb_fd = NOTOK;
	goto out2;
    }

    if ((pdu = (struct type_ACS_ABRT__apdu *) calloc (1, sizeof *pdu))
	    == NULL) {
	result = acsaplose (aci, ACS_CONGEST, NULLCP, "out of memory");
	goto out2;
    }
    pdu -> abort__source = int_ACS_abort__source_acse__service__user;
    if (data
	    && ndata > 0
	    && (pdu -> user__information = info2apdu (acb, aci, data, ndata))
			== NULL)
	goto out2;

    result = encode_ACS_ABRT__apdu (&pe, 1, 0, NULLCP, pdu);

    free_ACS_ABRT__apdu (pdu);
    pdu = NULL;

    if (result == NOTOK) {
	(void) acsaplose (aci, ACS_CONGEST, NULLCP, "error encoding PDU: %s",
			  PY_pepy);
	goto out2;
    }
    pe -> pe_context = acb -> acb_id;

    PLOGP (acsap_log,ACS_ACSE__apdu, pe, "ABRT-apdu", 0);

    if ((result = PUAbortRequest (acb -> acb_fd, &pe, 1, pi)) == NOTOK) {
	(void) ps2acslose (acb, aci, "PUAbortRequest", pa);
	if (PC_FATAL (pa -> pa_reason))
	    goto out2;
	else
	    goto out1;
    }

    result = OK;
    acb -> acb_fd = NOTOK;

out2: ;
    freeacblk (acb);

out1: ;
    if (pe)
	pe_free (pe);
    if (pdu)
	free_ACS_ABRT__apdu (pdu);

    (void) sigiomask (smask);

    return result;
}
