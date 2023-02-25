static char sccsid[] = "@(#)86	1.3  src/tcpip/usr/lib/libisode/sprintaei.c, isodelib7, tcpip411, GOLD410 4/5/93 16:42:38";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: sprintaei
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/sprintaei.c
 */

/* sprintaei.c - manage AE info */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintaei.c,v 1.2 93/02/05 17:11:40 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintaei.c,v 1.2 93/02/05 17:11:40 snmp Exp $
 *
 *
 * $Log:	sprintaei.c,v $
 * Revision 1.2  93/02/05  17:11:40  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:14:49  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:22:18  mrose
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
#include <isode/psap.h>
#include <isode/isoaddrs.h>

/*  */

char   *sprintaei (aei)
AEI	aei;
{
    register char *cp;
    char   *bp;
    static int    i;
    static char buffer1[BUFSIZ],
		buffer2[BUFSIZ];

    bp = cp = (i++ % 2) ? buffer1 : buffer2;

    *cp++ = '<';

    if (aei -> aei_ap_title) {
	vpushstr (cp);
	vunknown (aei -> aei_ap_title);
	vpopstr ();
	cp += strlen (cp);
    }
    *cp++ = ',';

    if (aei -> aei_ae_qualifier) {
	vpushstr (cp);
	vunknown (aei -> aei_ae_qualifier);
	vpopstr ();
	cp += strlen (cp);
    }
    *cp++ = ',';

    if (aei -> aei_flags & AEI_AE_ID) {
	(void) sprintf (cp, "%d", aei -> aei_ae_id);
	cp += strlen (cp);
    }
    *cp++ = ',';

    if (aei -> aei_flags & AEI_AP_ID) {
	(void) sprintf (cp, "%d", aei -> aei_ap_id);
	cp += strlen (cp);
    }
    (void) strcpy (cp, ">");

    return bp;
}
