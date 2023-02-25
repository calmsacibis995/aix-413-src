#if !defined(lint)
static char sccsid[] = "@(#)69	1.5  src/tenplus/hlib/help/Umod.c, tenplus, tenplus411, GOLD410 7/18/91 12:44:42";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Hmod
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
/* Hmod: called when a modified field is left                               */
/****************************************************************************/

/*ARGSUSED*/
int Hmod (char *fieldname, int line, char *oldtext, char *newtext)

    {
    return (0);     /* permit the modification */
    }
