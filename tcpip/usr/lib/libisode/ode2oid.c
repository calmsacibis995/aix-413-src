static char sccsid[] = "@(#)08	1.4  src/tcpip/usr/lib/libisode/ode2oid.c, isodelib7, tcpip411, GOLD410 4/5/93 14:06:01";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: ode2oid preloadcache
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ode2oid.c
 */

/* ode2oid.c - object descriptor to object identifier */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ode2oid.c,v 1.3 93/02/09 09:08:28 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ode2oid.c,v 1.3 93/02/09 09:08:28 snmp Exp $
 *
 *
 * $Log:	ode2oid.c,v $
 * Revision 1.3  93/02/09  09:08:28  snmp
 * Header changes for c++. This is a result of not include things twice.
 * 
 * Revision 1.2  93/02/05  17:05:36  snmp
 * ANSI - D67743
 * 
 * Revision 7.3  91/02/22  09:35:53  mrose
 * Interim 6.8
 * 
 * Revision 7.2  90/07/09  14:43:45  mrose
 * sync
 * 
 * Revision 1.1.1.2  90/06/07  08:03:26  isode
 * ISODE 6.1 beta
 * 
 * Revision 7.0  89/11/23  22:12:48  mrose
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
#include <isode/ppkt.h>

/* work around define collisions */
#undef missingP
#undef pylose
#include <isode/rtpkt.h>

/* work around type clashes */
#define ACSE
#undef _ACPKT_
#undef missingP
#undef pylose
#undef toomuchP
#define assocblk assocblkxxx
#define newacblk newacblkxxx
#define findacblk findacblkxxx
#include <isode/acpkt.h>

/*  */
#define ODECACHESIZE 10
static struct la_cache {
	char	*descriptor;	
	int	ref;
	OID	oid;
} Cache[ODECACHESIZE];

static void preloadcache (str)
char	*str;
{
    struct la_cache *cp = &Cache[0];
    register struct isobject *io;

    (void) setisobject (0);
    while (io = getisobject ()) {
	if (strcmp (str, io -> io_descriptor) == 0 ||
	    strcmp (DFLT_ASN, io -> io_descriptor) == 0 ||
	    strcmp (AC_ASN, io -> io_descriptor) == 0 ||
	    strcmp (BER, io -> io_descriptor) == 0 ||
	    strcmp (RT_ASN, io -> io_descriptor) == 0) {
	    if ((cp -> oid = oid_cpy (&io -> io_identity)) == NULLOID ||
		(cp -> descriptor = (char *)malloc ((unsigned) (strlen (io -> io_descriptor) + 1)))
		== NULLCP) {
		if (cp -> oid) {
		    oid_free (cp -> oid);
		    cp -> oid = NULLOID;
		}
	    }
	    else {
		(void) strcpy (cp -> descriptor, io -> io_descriptor);
		cp -> ref = 1;
		cp ++;
	    }
	}
    }
    (void) endisobject ();
}

OID	ode2oid (descriptor)
char   *descriptor;
{
    register struct isobject *io;
    int i, least;
    struct la_cache *cp, *cpn;
    static char firsttime = 0;

    if (firsttime == 0) {
	preloadcache (descriptor);
	firsttime = 1;
    }

    least = Cache[0].ref;
    for (cpn = cp = &Cache[0], i = 0; i < ODECACHESIZE; i++, cp++) {
	if (cp -> ref < least) {
	    least = cp -> ref;
	    cpn = cp;
	}
	if (cp -> ref <= 0)
		continue;
	if (strcmp (descriptor, cp -> descriptor) == 0) {
	    cp -> ref ++;
	    return cp -> oid;
	}
    }

    if ((io = getisobjectbyname (descriptor)) == NULL)
	return NULLOID;

    if (cpn -> oid)
	    oid_free (cpn -> oid);
    if (cpn -> descriptor)
	    free (cpn -> descriptor);

    cpn -> ref = 1;
    if ((cpn -> oid = oid_cpy (&io -> io_identity)) == NULLOID ||
	(cpn -> descriptor = (char *)malloc ((unsigned) (strlen (descriptor) + 1))) == NULLCP) {
	if (cpn -> oid) {
	    oid_free (cpn -> oid);
	    cpn -> oid = NULLOID;
	}
        cpn -> ref = 0;
    }
    else
	(void) strcpy (cpn -> descriptor, descriptor);

    return (&io -> io_identity);
}
