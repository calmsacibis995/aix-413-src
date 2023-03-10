static char sccsid[] = "@(#)35	1.3  src/tcpip/usr/lib/libisode/ryfind.c, isodelib7, tcpip411, GOLD410 4/5/93 16:25:37";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: finderrbyerr finderrbyname findopbyname findopbyop
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/ryfind.c
 */

/* ryfind.c - ROSY: find operations and errors by numbers and names */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ryfind.c,v 1.2 93/02/05 17:10:52 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/ryfind.c,v 1.2 93/02/05 17:10:52 snmp Exp $
 *
 *
 * $Log:	ryfind.c,v $
 * Revision 1.2  93/02/05  17:10:52  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:42:00  mrose
 * Interim 6.8
 * 
 * Revision 6.0  89/03/18  23:42:52  mrose
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

/*  */

struct RyOperation *findopbyop (ryo, op)
register struct RyOperation *ryo;
int	op;
{
    if (!ryo)
	return NULL;

    for (; ryo -> ryo_name; ryo++)
	if (ryo -> ryo_op == op)
	    return ryo;

    return NULL;
}


struct RyOperation *findopbyname (ryo, name)
register struct RyOperation *ryo;
char   *name;
{
    if (!ryo)
	return NULL;

    for (; ryo -> ryo_name; ryo++)
	if (strcmp (ryo -> ryo_name, name) == 0)
	    return ryo;

    return NULL;
}


struct RyError *finderrbyerr (rye, err)
register struct RyError *rye;
int	err;
{
    if (!rye)
	return NULL;

    for (; rye -> rye_name; rye++)
	if (rye -> rye_err == err)
	    return rye;

    return NULL;
}


struct RyError *finderrbyname (rye, name)
register struct RyError *rye;
char   *name;
{
    if (!rye)
	return NULL;

    for (; rye -> rye_name; rye++)
	if (strcmp (rye -> rye_name, name) == 0)
	    return rye;

    return NULL;
}
