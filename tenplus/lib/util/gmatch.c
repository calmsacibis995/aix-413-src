#if !defined(lint)
static char sccsid[] = "@(#)76	1.5  src/tenplus/lib/util/gmatch.c, tenplus, tenplus411, GOLD410 7/18/91 13:48:27";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	gmatch
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
 

/****************************************************************************/
/* gmatch: match string against Shell (glob) pattern                        */
/****************************************************************************/

#include <fnmatch.h>

int gmatch(char *str, char *pat)
{
  return (!fnmatch(pat, str, 0));
}

