#if !defined(lint)
static char sccsid[] = "@(#)53	1.6  src/tenplus/lib/short/i_pad.c, tenplus, tenplus411, GOLD410 3/23/93 12:08:04";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_pad
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "chardefs.h"
#include "database.h"
#include "libshort.h"
#include "libutil.h"
#include "libobj.h"

/****************************************************************************/
/* i_pad:  pad a short with spaces up to a nominated display width
*/
/****************************************************************************/

short *i_pad (short *str, int width)
    {
    short *result;
    char  *str_char;
    int   inwidth, pad_amt;
    int   i, pos, length;

    /* allocate a new line and determine its display width */
    result   = i_string(str);
    length   = i_strlen(result);
    str_char = a2string((ATTR *)str, length);
    inwidth  = text_width(str_char);

    /* is it big enough already? */
    s_free(str_char);
    if (inwidth >= width)
	return (result);

    /* SPACE is always 1 display width & allow 1 extra for null */
    pad_amt   = width - inwidth;
    result    = (short *) s_realloc ((char *)result, length + pad_amt + 1);
    result[0] = result[0] | FIRST;

    /* pad out with spaces */
    pos = calc_line_position((ATTR *)str, inwidth);
    while (pad_amt--)
	result[pos++] = (SPACE | FIRST);

    /* null terminate */
    result[pos] = 0;
    return (result);
    }
