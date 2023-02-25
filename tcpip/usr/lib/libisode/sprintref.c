static char sccsid[] = "@(#)66	1.3  src/tcpip/usr/lib/libisode/sprintref.c, isodelib7, tcpip411, GOLD410 4/5/93 16:44:11";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: sprintref
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/sprintref.c
 */

/* sprintref.c - manage encoded session addresses */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintref.c,v 1.2 93/02/05 17:11:47 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintref.c,v 1.2 93/02/05 17:11:47 snmp Exp $
 *
 *
 * $Log:	sprintref.c,v $
 * Revision 1.2  93/02/05  17:11:47  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:01  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:43  mrose
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
#include <isode/ssap.h>

/*  */

char   *sprintref (sr)
struct SSAPref *sr;
{
    register char  *cp;
    static char buffer[BUFSIZ];

    cp = buffer;
    *cp++ = '<';

    if (sr -> sr_ulen) {
	if (sr -> sr_ulen > 1 && *(sr -> sr_udata + 1) + 2 == sr -> sr_ulen)
	    (void) sprintf (cp, "%*.*s", sr -> sr_ulen - 2, sr -> sr_ulen - 2,
		    sr -> sr_udata + 2);
	else
	    (void) sprintf (cp, "%*.*s", sr -> sr_ulen, sr -> sr_ulen,
		    sr -> sr_udata);
	cp += strlen (cp);
    }
    *cp++ = ',';

    if (sr -> sr_clen) {
	if (sr -> sr_clen > 1 && *(sr -> sr_cdata + 1) + 2 == sr -> sr_clen)
	    (void) sprintf (cp, "%*.*s", sr -> sr_clen - 2, sr -> sr_clen - 2,
		    sr -> sr_cdata + 2);
	else
	    (void) sprintf (cp, "%*.*s", sr -> sr_clen, sr -> sr_clen,
		    sr -> sr_cdata);
	cp += strlen (cp);
    }
    *cp++ = ',';

    if (sr -> sr_alen) {
	if (sr -> sr_alen > 1 && *(sr -> sr_adata + 1) + 2 == sr -> sr_alen)
	    (void) sprintf (cp, "%*.*s", sr -> sr_alen - 2, sr -> sr_alen - 2,
		    sr -> sr_adata + 2);
	else
	    (void) sprintf (cp, "%*.*s", sr -> sr_alen, sr -> sr_alen,
		    sr -> sr_adata);
	cp += strlen (cp);
    }
    *cp++ = ',';

    if (sr -> sr_vlen) {
	if (sr -> sr_vlen > 1 && *(sr -> sr_vdata + 1) + 2 == sr -> sr_vlen)
	    (void) sprintf (cp, "%*.*s", sr -> sr_vlen - 2, sr -> sr_vlen - 2,
		    sr -> sr_vdata + 2);
	else
	    (void) sprintf (cp, "%*.*s", sr -> sr_vlen, sr -> sr_vlen,
		    sr -> sr_vdata);
	cp += strlen (cp);
    }
    *cp++ = '>';

    *cp = NULL;

    return buffer;
}
