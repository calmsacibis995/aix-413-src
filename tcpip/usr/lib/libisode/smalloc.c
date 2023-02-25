static char sccsid[] = "@(#)25	1.3  src/tcpip/usr/lib/libisode/smalloc.c, isodelib7, tcpip411, GOLD410 4/5/93 16:42:16";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: smalloc
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/smalloc.c
 */

/* smalloc.c - error checking malloc */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/smalloc.c,v 1.2 93/02/05 17:11:39 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/smalloc.c,v 1.2 93/02/05 17:11:39 snmp Exp $
 *
 *
 * $Log:	smalloc.c,v $
 * Revision 1.2  93/02/05  17:11:39  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:52  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:33  mrose
 * Release 6.0
 * 
 */

/*
 *                                NOTICE
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
#include <isode/tailor.h>

/*  */

char *
smalloc(size)
int	size;
{
	register char *ptr;

	if ((ptr = (char *) malloc((unsigned) size)) == NULL){
	    LLOG (compat_log,LLOG_FATAL, ("malloc() failure"));
	    abort ();
	    /* NOTREACHED */
	}

	return(ptr);
}
