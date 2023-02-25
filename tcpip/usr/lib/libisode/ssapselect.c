static char sccsid[] = "@(#)95	1.3  src/tcpip/usr/lib/libisode/ssapselect.c, isodelib7, tcpip411, GOLD410 4/5/93 16:56:28";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: SSelectMask
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ssapselect.c
 */

/* ssapselect.c - SPM: map descriptors */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapselect.c,v 1.2 93/02/05 17:12:39 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ssapselect.c,v 1.2 93/02/05 17:12:39 snmp Exp $
 *
 *
 * $Log:	ssapselect.c,v $
 * Revision 1.2  93/02/05  17:12:39  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:46:07  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:25:50  mrose
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

/*    map session descriptors for select() */

int	SSelectMask (sd, mask, nfds, si)
int	sd;
fd_set *mask;
int    *nfds;
register struct SSAPindication *si;
{
    SBV	    smask;
    int     result;
    register struct ssapblk *sb;
    struct TSAPdisconnect   tds;
    register struct TSAPdisconnect *td = &tds;

    missingP (mask);
    missingP (nfds);

    smask = sigioblock ();

    if ((sb = findsblk (sd)) == NULL) {
	(void) sigiomask (smask);
	return ssaplose (si, SC_PARAMETER, NULLCP,
			    "invalid session descriptor");
    }

    if ((result = TSelectMask (sb -> sb_fd, mask, nfds, td)) == NOTOK)
	if (td -> td_reason == DR_WAITING)
	    (void) ssaplose (si, SC_WAITING, NULLCP, NULLCP);
	else
	    (void) ts2sslose (si, "TSelectMask", td);

    (void) sigiomask (smask);

    return result;
}
