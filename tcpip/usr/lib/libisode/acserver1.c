static char sccsid[] = "@(#)21	1.3  src/tcpip/usr/lib/libisode/acserver1.c, isodelib7, tcpip411, GOLD410 4/5/93 13:41:55";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: isodeserver
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/acserver1.c
 */

/* acserver1.c - generic server dispatch */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/acserver1.c,v 1.2 93/02/05 17:03:26 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/acserver1.c,v 1.2 93/02/05 17:03:26 snmp Exp $
 *
 *
 * $Log:	acserver1.c,v $
 * Revision 1.2  93/02/05  17:03:26  snmp
 * ANSI - D67743
 * 
 * Revision 7.4  91/02/22  09:14:23  mrose
 * Interim 6.8
 * 
 * Revision 7.3  90/10/29  18:37:49  mrose
 * updates
 * 
 * Revision 7.2  90/07/09  14:30:45  mrose
 * sync
 * 
 * Revision 7.1  90/02/19  13:07:05  mrose
 * update
 * 
 * Revision 7.0  89/11/23  21:22:02  mrose
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

#include <signal.h>
#include <isode/psap.h>
#include <isode/tsap.h>
#include <sys/ioctl.h>
#ifdef	BSD42
#include <sys/file.h>
#endif
#ifdef	SYS5
#include <fcntl.h>
#endif
#include <isode/tailor.h>

/*  */

int	isodeserver (argc, argv, aei, initfnx, workfnx, losefnx, td)
int	argc;
char  **argv;
AEI	aei;
IFP	initfnx,
	workfnx,
	losefnx;
struct TSAPdisconnect *td;
{
    if (iserver_init (argc, argv, aei, initfnx, td) == NOTOK)
	return NOTOK;

    for (;;) {
	int     result;

	if ((result = iserver_wait (initfnx, workfnx, losefnx, 0, NULLFD,
				    NULLFD, NULLFD, NOTOK, td)) != OK)
	    return (result == DONE ? OK : result);
    }
}
