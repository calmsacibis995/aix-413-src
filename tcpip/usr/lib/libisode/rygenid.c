static char sccsid[] = "@(#)36	1.3  src/tcpip/usr/lib/libisode/rygenid.c, isodelib7, tcpip411, GOLD410 4/5/93 16:26:23";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: RyGenID
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/rygenid.c
 */

/* rygenid.c - ROSY: generate unique invoke ID */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rygenid.c,v 1.2 93/02/05 17:10:54 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/rygenid.c,v 1.2 93/02/05 17:10:54 snmp Exp $
 *
 *
 * $Log:	rygenid.c,v $
 * Revision 1.2  93/02/05  17:10:54  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:01  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:53  mrose
 * Release 5.0
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
#include <isode/rosy.h>

/*    generate unique invoke ID */

/* ARGSUSED */

int	RyGenID (sd)
int	sd;
{
    static int	id = 0;

    return (++id);
}
