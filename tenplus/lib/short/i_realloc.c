#if !defined(lint)
static char sccsid[] = "@(#)56	1.5  src/tenplus/lib/short/i_realloc.c, tenplus, tenplus411, GOLD410 7/18/91 13:47:17";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_realloc
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
/* File:  i_realloc.c - s_realloc analog that retains null termination      */
/****************************************************************************/

#include <stdio.h>
#include <database.h>
#include <libobj.h>

/****************************************************************************/
/* i_realloc:  do s_realloc with count arg offset and ensuring null term    */
/****************************************************************************/

short *i_realloc (short *obj, int count)
    /*obj:        ptr to original object             */
    /*count:      desired count after reallocation   */
    {
    short *retval;      /* to be returned                 */

    retval = (short *) s_realloc ((char *)obj, (unsigned)count + 1);      /* extra for null */

    retval [count] = 0;

    return (retval);
    }
