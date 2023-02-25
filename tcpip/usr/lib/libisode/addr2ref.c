static char sccsid[] = "@(#)92	1.3  src/tcpip/usr/lib/libisode/addr2ref.c, isodelib7, tcpip411, GOLD410 4/5/93 13:42:30";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: addr2ref stuff
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/addr2ref.c
 */

/* addr2ref.c - manage encoded session addresses */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/addr2ref.c,v 1.2 93/02/05 17:03:32 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/addr2ref.c,v 1.2 93/02/05 17:03:32 snmp Exp $
 *
 *
 * $Log:	addr2ref.c,v $
 * Revision 1.2  93/02/05  17:03:32  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:35:28  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:12:30  mrose
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


long	time ();

/*  */

struct SSAPref *addr2ref (addr)
register char   *addr;
{
    int     result;
    long    clock;
    register    PE pe;
    register struct tm *tm;
    struct UTCtime  uts;
    register struct UTCtime *ut = &uts;
    static struct SSAPref   srs;
    register struct SSAPref *sr = &srs;

    bzero ((char *) sr, sizeof *sr);

    if ((pe = t61s2prim (addr, strlen (addr))) == NULLPE)
	return NULL;
    result = stuff (pe, sr -> sr_udata, &sr -> sr_ulen);
    pe_free (pe);
    if (result == NOTOK)
	return NULL;

    if (time (&clock) == NOTOK || (tm = gmtime (&clock)) == NULL)
	return NULL;
    tm2ut (tm, ut);

    if ((pe = utct2prim (ut)) == NULLPE)
	return NULL;
    result = stuff (pe, sr -> sr_cdata, &sr -> sr_clen);
    pe_free (pe);
    if (result == NOTOK)
	return NULL;

    return sr;
}

/*  */

static int  stuff (pe, dbase, dlen)
register PE pe;
register char *dbase;
register u_char *dlen;
{
    int     len;
    char   *base;

    if (pe2ssdu (pe, &base, &len) == NOTOK)
	return NOTOK;

    bcopy (base, dbase, (int) (*dlen = len));
    free (base);

    return OK;
}
