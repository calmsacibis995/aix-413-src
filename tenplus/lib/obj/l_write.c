#if !defined(lint)
static char sccsid[] = "@(#)72	1.6  src/tenplus/lib/obj/l_write.c, tenplus, tenplus411, GOLD410 3/23/93 12:04:59";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	l_write
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
 
/****************************************************************************/
/* l_write:  writes an array of longs to a file                             */
/* Does falal error exit if called with a non-long object                   */
/* Returns ERROR on I/O error, else RET_OK.                                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

extern int T_LONG;

int l_write (register long *lp, register FILE *fp)
    /*lp:  data pointer for long array     */
    /*fp:  file pointer to read longs from */
    {
    register i;        /* index into long array           */

#ifdef CAREFUL

    (void) s_typecheck ((char *)lp, "l_write", T_LONG);

#endif

    for (i = 0; i < obj_count (lp); i++)
	putl (lp [i], fp);

    if ((!feof (fp)) && (!ferror (fp))) /* check for I/O error */
	return (RET_OK);

    g_errno = E_IO;
    return (ERROR);
    }

