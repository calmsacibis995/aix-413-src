static char sccsid[] = "@(#)88	1.3  src/tcpip/usr/lib/libisode/ronotabort.c, isodelib7, tcpip411, GOLD410 4/5/93 15:26:19";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RoBindUAbort
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ronotabort.c
 */

/* ronotabort.c - RONOT: bail-out routine which logs abort to rosap log */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ronotabort.c,v 1.2 93/02/05 17:08:57 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ronotabort.c,v 1.2 93/02/05 17:08:57 snmp Exp $
 *
 *
 * $Log:	ronotabort.c,v $
 * Revision 1.2  93/02/05  17:08:57  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:50:29  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/07/26  14:33:53  mrose
 * template
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

#include <isode/tailor.h>
#include <isode/logger.h>
#include <isode/rosap.h>
#include <isode/ronot.h>

/*    RO-ABORT.REQUEST */

/* ARGSUSED */

int	  RoBindUAbort (sd, rni)
int			  sd;
struct RoNOTindication	* rni;
{
	int			  result;
        struct AcSAPindication    aci_s;
        struct AcSAPindication  * aci = &(aci_s);
        struct AcSAPabort       * aca = &(aci->aci_abort);

	LLOG (rosap_log, LLOG_EXCEPTIONS, ("RO-ABORT-BIND.REQUEST called on %d", sd));

	result = AcUAbortRequest (sd, NULLPEP, 0, aci);

	if (result != OK)
	{
	        LLOG (rosap_log, LLOG_EXCEPTIONS, ("RO-ABORT-BIND.REQUEST failed on %d", sd));
		return (acs2ronotlose (rni, "RO-ABORT-BIND.REQUEST", aca));
	}

	return (OK);
}

