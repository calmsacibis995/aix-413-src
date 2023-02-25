static char sccsid[] = "@(#)93	1.3  src/tcpip/usr/lib/libisode/chrcnv.c, isodelib7, tcpip411, GOLD410 4/5/93 13:46:18";
/*
 * COMPONENT_NAME: (ISODELIB7) ISODE Libraries, Release 7
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 60
 *
 * FILE:	src/tcpip/usr/lib/libisode/chrcnv.c
 */

/* chrcnv.c - character conversion table */

#ifndef lint
static char *rcsid = "$Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/chrcnv.c,v 1.2 93/02/05 17:03:52 snmp Exp $";
#endif

/*
 * $Header: /vikings/u/snmp/projects/harriet/RCS/src/tcpip/usr/lib/libisode/chrcnv.c,v 1.2 93/02/05 17:03:52 snmp Exp $
 *
 *
 * $Log:	chrcnv.c,v $
 * Revision 1.2  93/02/05  17:03:52  snmp
 * ANSI - D67743
 * 
 * Revision 7.1  91/02/22  09:15:04  mrose
 * Interim 6.8
 * 
 * Revision 7.0  89/11/23  21:22:58  mrose
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

/*  */

char                                   /* character conversion table   */
	chrcnv[] =                     /*   lower to upper case letters */
{
    '\0', '\1', '\2', '\3', '\4', '\5', '\6', '\7',
    '\10', '\t', '\n', '\13', '\14', '\r', '\16', '\17',
    '\20', '\21', '\22', '\23', '\24', '\25', '\26', '\27',
    '\30', '\31', '\32', '\33', '\34', '\35', '\36', '\37',
    ' ', '!', '"', '#', '$', '%', '&', '\47',
    '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '{', '|', '}', '~', '\177',
    '\0', '\1', '\2', '\3', '\4', '\5', '\6', '\7',
    '\10', '\t', '\n', '\13', '\14', '\r', '\16', '\17',
    '\20', '\21', '\22', '\23', '\24', '\25', '\26', '\27',
    '\30', '\31', '\32', '\33', '\34', '\35', '\36', '\37',
    ' ', '!', '"', '#', '$', '%', '&', '\47',
    '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', ':', ';', '<', '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
    'X', 'Y', 'Z', '{', '|', '}', '~', '\177'
};

