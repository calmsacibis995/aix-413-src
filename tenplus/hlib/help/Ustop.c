#if !defined(lint)
static char sccsid[] = "@(#)72	1.6  src/tenplus/hlib/help/Ustop.c, tenplus, tenplus411, GOLD410 3/23/93 12:01:12";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hstop
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
#include <libhelp.h>

extern void exit(int);


/****************************************************************************/
/* Hstop: called when the helper process is terminated                      */
/****************************************************************************/

void Hstop (void)
    {
    exit (0);	/* make sure this signal-catcher still lets the helper die */
    }

