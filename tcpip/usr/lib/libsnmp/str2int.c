static char sccsid[] = "@(#)68	1.2  src/tcpip/usr/lib/libsnmp/str2int.c, snmp, tcpip411, GOLD410 3/2/93 18:27:16";
/*
 * COMPONENT_NAME: (SNMP) Simple Network Management Protocol Daemon
 *
 * FUNCTIONS: str2int()
 *
 * ORIGINS: 27
 *
 * (C) Copyright International Business Machines Corp. 1991, 1993
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * FILE:	src/tcpip/usr/lib/libsnmp/str2int.c
 */

#include <ctype.h>
#include <isode/manifest.h>	/* definition of NOTOK */

extern int errno;

/*
 * NAME: str2int ()
 *
 * FUNCTION: Convert a digit string to an integer.  A positive
 *	     number is assumed.
 *
 * RETURNS:  The string as an integer (SUCCESS) 
 *	     NOTOK (FAILURE)
 */
int
str2int (str)
char * str;
{
    int i;
    
    errno = 0;

    for (i = 0; i < strlen(str); i++) {
	if (!(isdigit (str[i])))
	    return NOTOK;
    }

    i = atoi (str);

    if (errno != 0)
	return NOTOK;

    return i;
}
