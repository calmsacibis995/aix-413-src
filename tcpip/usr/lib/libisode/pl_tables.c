static char sccsid[] = "@(#)28	1.3  src/tcpip/usr/lib/libisode/pl_tables.c, isodelib7, tcpip411, GOLD410 4/5/93 14:59:16";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/pl_tables.c
 */

/* pl_tables.c - tables for presentation lists */

#ifndef	lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pl_tables.c,v 1.2 93/02/05 17:06:35 snmp Exp $";
#endif

/* 
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/pl_tables.c,v 1.2 93/02/05 17:06:35 snmp Exp $
 *
 *
 * $Log:	pl_tables.c,v $
 * Revision 1.2  93/02/05  17:06:35  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:36:19  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  22:13:08  mrose
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

/*    DATA */

char   *pe_classlist[] = {
    "UNIV",
    "APPL",
    "CONT",
    "PRIV"
};

int	pe_maxclass = sizeof pe_classlist / sizeof pe_classlist[0];


char   *pe_univlist[] = {
    "EOC",
    "BOOL",
    "INT",
    "BITS",
    "OCTS",
    "NULL",
    "OID",
    "ODE",
    "EXTN",
    "REAL",
    "ENUM",
    "ENCR",
    NULLCP,
    NULLCP,
    NULLCP,
    NULLCP,
    "SEQ",
    "SET",
    "NUMS",
    "PRTS",
    "T61S",
    "VTXS",
    "IA5S",
    "UTCT",
    "GENT",
    "GFXS",
    "VISS",
    "GENS",
    "CHRS",
};

int	pe_maxuniv = sizeof pe_univlist / sizeof pe_univlist[0];


int	pe_maxappl = 0;
char  **pe_applist = NULL;

int	pe_maxpriv = 0;
char  **pe_privlist = NULL;
