static char sccsid[] = "@(#)89	1.3  src/tcpip/usr/lib/libisode/psapselect.c, isodelib7, tcpip411, GOLD410 4/5/93 15:17:47";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: PSelectMask
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/psapselect.c
 */

/* psapselect.c - PPM: map descriptors */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapselect.c,v 1.2 93/02/05 17:08:03 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/psapselect.c,v 1.2 93/02/05 17:08:03 snmp Exp $
 *
 *
 * $Log:	psapselect.c,v $
 * Revision 1.2  93/02/05  17:08:03  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:53  mrose
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

/*    map presentation descriptors for select() */

int	PSelectMask (sd, mask, nfds, pi)
int	sd;
fd_set *mask;
int    *nfds;
struct PSAPindication *pi;
{
    SBV     smask;
    register struct psapblk *pb;
    struct SSAPindication   sis;
    register struct SSAPabort  *sa = &sis.si_abort;

    missingP (mask);
    missingP (nfds);
    missingP (pi);

    smask = sigioblock ();

    if ((pb = findpblk (sd)) == NULL) {
	(void) sigiomask (smask);
	return psaplose (pi, PC_PARAMETER, NULLCP,
			    "invalid presentation descriptor");
    }

    if (SSelectMask (pb -> pb_fd, mask, nfds, &sis) == NOTOK)
	switch (sa -> sa_reason) {
	    case SC_WAITING: 
		(void) sigiomask (smask);
		return psaplose (pi, PC_WAITING, NULLCP, NULLCP);

	    default: 
		(void) ss2pslose (pb, pi, "SSelectMask", sa);
		freepblk (pb);
		(void) sigiomask (smask);
		return NOTOK;
	}

    (void) sigiomask (smask);

    return OK;
}
