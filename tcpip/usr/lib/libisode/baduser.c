static char sccsid[] = "@(#)89	1.3  src/tcpip/usr/lib/libisode/baduser.c, isodelib7, tcpip411, GOLD410 4/5/93 13:43:51";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: baduser
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/baduser.c
 */

/* baduser.c - check file of bad users */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/baduser.c,v 1.2 93/02/05 17:03:39 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/baduser.c,v 1.2 93/02/05 17:03:39 snmp Exp $
 *
 *
 * $Log:	baduser.c,v $
 * Revision 1.2  93/02/05  17:03:39  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:14:56  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:22:54  mrose
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
#include <string.h> 
#include <isode/general.h>
#include <isode/manifest.h>
#include <isode/tailor.h>

/*  */

int	baduser (file, user)
char   *file,
       *user;
{
    int     hit,
	    tries;
    register char  *bp;
    char    buffer[BUFSIZ];
    FILE   *fp;

    hit = 0;
    for (tries = 0; tries < 2 && !hit; tries++) {
	switch (tries) {
	    case 0:
	        if (file) {
		    bp = isodefile (file, 0);
		    break;
		}
		tries++;
		/* and fall */
	    default:
		bp = "/etc/ftpusers";
		break;
	}
	if ((fp = fopen (bp, "r")) == NULL)
	    continue;

	while (fgets (buffer, sizeof buffer, fp)) {
	    if (bp = index (buffer, '\n'))
		*bp = NULL;
	    if (strcmp (buffer, user) == 0) {
		hit++;
		break;
	    }
	}

	(void) fclose (fp);
    }


    return hit;
}
