static char sccsid[] = "@(#)77	1.3  src/tcpip/usr/lib/libisode/time2str.c, isodelib7, tcpip411, GOLD410 4/5/93 17:13:01";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: UNYEAR time2str
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/time2str.c
 */

/* time2str.c - time string to string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/time2str.c,v 1.2 93/02/05 17:13:42 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/time2str.c,v 1.2 93/02/05 17:13:42 snmp Exp $
 *
 *
 * $Log:	time2str.c,v $
 * Revision 1.2  93/02/05  17:13:42  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:14  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:53  mrose
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


#define	YEAR(y)		((y) >= 100 ? (y) : (y) + 1900)
#define	UNYEAR(y)	((y) < 1900 || (y) > 1999 ? (y) : (y) - 1900)

/*  */

char   *time2str (u, generalized)
register UTC	u;
int	generalized;
{
    register int    hours,
                    mins,
                    zone;
    register char  *bp;
    static char buffer[BUFSIZ];

    if (u == NULLUTC)
	return NULLCP;

    bp = buffer;

    if (generalized)
	(void) sprintf (bp, "%04d", YEAR (u -> ut_year));
    else
	(void) sprintf (bp, "%02d", UNYEAR (u -> ut_year));
    bp += strlen (bp);

    (void) sprintf (bp, "%02d%02d%02d%02d", u -> ut_mon, u -> ut_mday,
		u -> ut_hour, u -> ut_min);
    bp += strlen (bp);

    if (u -> ut_flags & UT_SEC
	    || (generalized && (u -> ut_flags & UT_USEC))) {
	(void) sprintf (bp, "%02d", u -> ut_sec);
	bp += strlen (bp);
    }
    if (generalized && (u -> ut_flags & UT_USEC)) {
	(void) sprintf (bp, ".%06d", u -> ut_usec);
	bp += strlen (bp);
    }

    if (u -> ut_flags & UT_ZONE)
	if ((zone = u -> ut_zone) == 0)
	    *bp++ = 'Z';
	else {
	    if (zone < 0)
		mins = (-zone) % 60, hours = (-zone) / 60;
	    else
		mins = zone % 60, hours = zone / 60;
	    (void) sprintf (bp, "%c%02d%02d", zone < 0 ? '-' : '+',
		    hours, mins);
	    bp += strlen (bp);
	}

    *bp = NULL;

    return buffer;
}
