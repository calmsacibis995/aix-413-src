static char sccsid[] = "@(#)64	1.3  src/tcpip/usr/lib/libisode/set_find.c, isodelib7, tcpip411, GOLD410 4/5/93 16:41:08";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: set_find
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/set_find.c
 */

/* set_find.c - find member of a set */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/set_find.c,v 1.2 93/02/05 17:11:34 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/set_find.c,v 1.2 93/02/05 17:11:34 snmp Exp $
 *
 *
 * $Log:	set_find.c,v $
 * Revision 1.2  93/02/05  17:11:34  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:59  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:41  mrose
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

/*  */

PE	set_find (pe, class, id)
register PE	pe;
register PElementClass class;
register PElementID id;
{
    register int    pe_id;
    register PE	    p;

    pe_id = PE_ID (class, id);
    for (p = pe -> pe_cons; p; p = p -> pe_next)
	if (PE_ID (p -> pe_class, p -> pe_id) == pe_id)
	    break;

    return p;
}
