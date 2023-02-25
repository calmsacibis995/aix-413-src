static char sccsid[] = "@(#)03	1.3  src/tcpip/usr/lib/libisode/isofiles.c, isodelib7, tcpip411, GOLD410 4/5/93 13:54:20";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: _isodefile
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/isofiles.c
 */

/* isofiles.c - ISODE files */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/isofiles.c,v 1.2 93/02/05 17:04:58 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/isofiles.c,v 1.2 93/02/05 17:04:58 snmp Exp $
 *
 *
 * $Log:	isofiles.c,v $
 * Revision 1.2  93/02/05  17:04:58  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:20  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:23:07  mrose
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
#include <isode/tailor.h>

/*  */

char   *_isodefile (path, file)
char   *path,
       *file;
{
    static char buffer[BUFSIZ];

    isodetailor (NULLCP, 0);	/* not really recursive */

    if (*file == '/'
	    || (*file == '.'
		    && (file[1] == '/'
			    || (file[1] == '.' && file[2] == '/'))))
	(void) strcpy (buffer, file);
    else
	(void) sprintf (buffer, "%s%s", path, file);

    return buffer;
}
