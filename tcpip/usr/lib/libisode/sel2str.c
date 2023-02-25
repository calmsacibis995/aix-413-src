static char sccsid[] = "@(#)18	1.3  src/tcpip/usr/lib/libisode/sel2str.c, isodelib7, tcpip411, GOLD410 4/5/93 16:33:43";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: sel2str
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/sel2str.c
 */

/* sel2str.c - selector to string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sel2str.c,v 1.2 93/02/05 17:11:09 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sel2str.c,v 1.2 93/02/05 17:11:09 snmp Exp $
 *
 *
 * $Log:	sel2str.c,v $
 * Revision 1.2  93/02/05  17:11:09  snmp
 * ANSI - D67743
 * 
 * Revision 7.2  91/02/22  09:15:44  mrose
 * Interim 6.8
 * 
 * Revision 7.1  90/11/21  11:29:55  mrose
 * sun
 * 
 * Revision 7.0  89/11/23  21:23:23  mrose
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

#include <ctype.h>
#include <stdio.h>
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/isoaddrs.h>

/*  */

char   *sel2str (sel, len, quoted)
char   *sel;
int	len,
	quoted;
{
    register char *cp,
		  *dp,
    		  *ep;
    static int    i = 0;
    static char buf1[NASIZE * 2 + 1],
		buf2[NASIZE * 2 + 1],
    		buf3[NASIZE * 2 + 1],
    		buf4[NASIZE * 2 + 1];
    static char *bufs[] = { buf1, buf2, buf3, buf4 };

    cp = bufs[i++];
    i = i % 4;

    if (quoted) {
#ifndef	NOGOSIP
	if (len == 2) {
	    if (quoted < 0)
		goto ugly;
	    (void) sprintf (cp, "#%d",
			    (sel[0] & 0xff) << 8 | (sel[1] & 0xff));
	    goto out;
	}
#endif

	for (ep = (dp = sel) + len; dp < ep; dp++)
	    if (!isprint ((u_char) *dp))
		goto ugly;

	if (len > NASIZE * 2)
	    len = NASIZE * 2;

	(void) sprintf (cp, len ? "\"%*.*s\"" : "\"\"", len, len, sel);
    }
    else {
ugly: ;
	if (len > NASIZE)	/* XXX */
	    len = NASIZE;

	cp[explode (cp, (u_char *) sel, len)] = NULL;
    }
#ifndef	NOGOSIP
out: ;
#endif
	
    return cp;
}
