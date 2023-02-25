#if !defined(lint)
static char sccsid[] = "@(#)65	1.6  src/tenplus/hlib/help/Ubefore.c, tenplus, tenplus411, GOLD410 3/23/93 12:00:42";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hbefore
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
 
#include <stdio.h>
#include <database.h>
#include "libhelp.h"


/****************************************************************************/
/* Hbefore:  called before the execution of a key                           */
/****************************************************************************/

/*ARGSUSED*/
int Hbefore (int key, int type, char *strvalue, int numvalue)

    {
    return (0);		/* tell editor to process key normally */
    }

