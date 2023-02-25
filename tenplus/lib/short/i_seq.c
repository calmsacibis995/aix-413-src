#if !defined(lint)
static char sccsid[] = "@(#)57	1.5  src/tenplus/lib/short/i_seq.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:22";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_seq
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
/* i_seq.c:  seq analog for SHORT strings                                   */
/****************************************************************************/

#include <database.h>

/****************************************************************************/
/* i_seq:  seq analog                                                       */
/****************************************************************************/

int i_seq (short *s1, short *s2)
    {
    if ((s1 == NULL) || (*s1 == 0))
	return ((s2 == NULL) || (*s2 == 0));

    if ((s2 == NULL) || (*s2 == 0))
	return (FALSE);

    while (*s1 && *s2)
	if (*s1++ != *s2++)
	    return (FALSE);

    return (! (*s1 || *s2));   /* TRUE iff both got to null together */
    }
