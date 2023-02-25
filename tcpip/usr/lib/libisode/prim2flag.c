static char sccsid[] = "@(#)30	1.3  src/tcpip/usr/lib/libisode/prim2flag.c, isodelib7, tcpip411, GOLD410 4/5/93 15:00:54";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: prim2flag
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/prim2flag.c
 */

/* prim2flag.c - presentation element to boolean */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2flag.c,v 1.2 93/02/05 17:06:42 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/prim2flag.c,v 1.2 93/02/05 17:06:42 snmp Exp $
 *
 *
 * $Log:	prim2flag.c,v $
 * Revision 1.2  93/02/05  17:06:42  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:21  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:10  mrose
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

int	prim2flag (pe)
register PE	pe;
{
    if (pe -> pe_form != PE_FORM_PRIM
	    || pe -> pe_prim == NULLPED
	    || pe -> pe_len == 0)
	return pe_seterr (pe, PE_ERR_PRIM, NOTOK);

    return (*pe -> pe_prim != 0x00);
}
