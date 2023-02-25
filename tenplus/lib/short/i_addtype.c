#if !defined(lint)
static char sccsid[] = "@(#)46	1.5  src/tenplus/lib/short/i_addtype.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:59";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_addtype
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
/* i_addtype:  adds int (short) object type                                 */
/****************************************************************************/

#include <stdio.h>
#include "database.h"
#include "libobj.h"
#include "libshort.h"

#define SHORTBYTES (2)

void i_addtype (void)
    {
    if (s_settype ("short", T_ATTR, SHORTBYTES,
      sht_print, sht_read, sht_write, noop) == ERROR)
	fatal ("i_addtype: Cannot create data type window");
    }

