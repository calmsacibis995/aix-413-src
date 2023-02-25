static char sccsid[] = "@(#)33	1.3  src/tcpip/usr/lib/libisode/strdup.c, isodelib7, tcpip411, GOLD410 4/5/93 17:08:17";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: strdup
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/strdup.c
 */

/* strdup.c - create a duplicate copy of the given string */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/strdup.c,v 1.2 93/02/05 17:13:22 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/strdup.c,v 1.2 93/02/05 17:13:22 snmp Exp $
 *
 *
 * $Log:	strdup.c,v $
 * Revision 1.2  93/02/05  17:13:22  snmp
 * ANSI - D67743
 * 
 * Revision 7.3  91/02/22  09:16:05  mrose
 * Interim 6.8
 * 
 * Revision 7.2  90/11/04  19:14:52  mrose
 * update
 * 
 * Revision 7.1  90/10/15  18:19:58  mrose
 * sync
 * 
 * Revision 7.0  89/11/23  21:23:40  mrose
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

char   *strdup (str)
register char   *str;
{
	register char *ptr;

	if ((ptr = (char *)malloc((unsigned) (strlen (str) + 1))) == NULL){
	    LLOG (compat_log,LLOG_FATAL, ("strdup malloc() failure"));
	    abort ();
	    /* NOTREACHED */
	}

	(void) strcpy (ptr, str);

	return ptr;
}
