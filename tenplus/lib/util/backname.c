#if !defined(lint)
static char sccsid[] = "@(#)71	1.5  src/tenplus/lib/util/backname.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:08";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	backname
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include <string.h>
#include <limits.h>

static char bakname [PATH_MAX];

char *backname (char *fname)
{
    register char *cp;
    char extension [PATH_MAX];

    (void) strcpy (bakname, fname);
    cp = strrchr (bakname, '.');
    if (cp == NULL)
	(void) strcat (bakname, ".~");
    else {

	(void) strcpy (extension , cp+1);
	*(cp+1) = '~';
	(void) strcpy (cp+2, extension);
	*(cp+4) = '\0';
    }
    return (bakname);
}
