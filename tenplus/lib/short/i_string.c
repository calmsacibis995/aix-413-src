#if !defined(lint)
static char sccsid[] = "@(#)60	1.5  src/tenplus/lib/short/i_string.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:30";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_string
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
/* i_string.c:  s_string analog for SHORT strings                           */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include "libshort.h"
#include "libobj.h"

/****************************************************************************/
/* i_string:  return allocated copy of given SHORT string                   */
/****************************************************************************/

short *i_string (short *instr)
    {
    short *outstr;      /* to be returned */
    int    count;
    int    i;

    count = instr ? i_strlen (instr) : 0;   /* beware null instr */

    /* allocate one extra for null terminator */
    outstr = (short *) s_alloc (count + 1, T_ATTR, "");
    for (i = 0; i < count; i++)
	outstr [i] = *instr++;

    return (outstr);
    }
