static char sccsid[] = "@(#)26	1.3  src/tcpip/usr/lib/libisode/sprintb.c, isodelib7, tcpip411, GOLD410 4/5/93 16:43:08";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: sprintb
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/sprintb.c
 */

/* sprintb.c - sprintf on bits */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintb.c,v 1.2 93/02/05 17:11:43 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/sprintb.c,v 1.2 93/02/05 17:11:43 snmp Exp $
 *
 *
 * $Log:	sprintb.c,v $
 * Revision 1.2  93/02/05  17:11:43  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:53  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:33  mrose
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
#include <isode/general.h>
#include <isode/manifest.h>

/*  */

char  *sprintb (v, bits)
register int	v;
register char  *bits;
{
    register int    i,
                    j;
    register char   c,
                   *bp;
    static char buffer[BUFSIZ];

    (void) sprintf (buffer, bits && *bits == 010 ? "0%o" : "0x%x", v);
    bp = buffer + strlen (buffer);

    if (bits && *++bits) {
	j = 0;
	*bp++ = '<';
	while (i = *bits++)
	    if (v & (1 << (i - 1))) {
		if (j++)
		    *bp++ = ',';
		for (; (c = *bits) > 32; bits++)
		    *bp++ = c;
	    }
	    else
		for (; *bits > 32; bits++)
		    continue;
	*bp++ = '>';
	*bp = NULL;
    }

    return buffer;
}
