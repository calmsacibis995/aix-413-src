static char sccsid[] = "@(#)78	1.3  src/tcpip/usr/lib/libisode/tm2ut.c, isodelib7, tcpip411, GOLD410 4/5/93 17:13:25";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: tm2ut
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/tm2ut.c
 */

/* tm2ut.c - tm to time string */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/tm2ut.c,v 1.2 93/02/05 17:13:44 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/tm2ut.c,v 1.2 93/02/05 17:13:44 snmp Exp $
 *
 *
 * $Log:	tm2ut.c,v $
 * Revision 1.2  93/02/05  17:13:44  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:37:15  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:54  mrose
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
#ifdef	OSX
#include <sys/time.h>
#endif


#define	YEAR(y)		((y) >= 100 ? (y) : (y) + 1900)

/*  */

void	tm2ut (tm, ut)
register struct tm *tm;
register UTC	ut;
{
    bzero ((char *) ut, sizeof *ut);

    ut -> ut_year = YEAR (tm -> tm_year);
    ut -> ut_mon = tm -> tm_mon + 1;
    ut -> ut_mday = tm -> tm_mday;
    ut -> ut_hour = tm -> tm_hour;
    ut -> ut_min = tm -> tm_min;
    ut -> ut_sec = tm -> tm_sec;
    ut -> ut_zone = 0;
    
    ut -> ut_flags = UT_ZONE | UT_SEC;
}
