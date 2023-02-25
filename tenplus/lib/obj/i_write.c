#if !defined(lint)
static char sccsid[] = "@(#)67	1.6  src/tenplus/lib/obj/i_write.c, tenplus, tenplus411, GOLD410 3/23/93 12:04:42";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	i_write
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
/* i_write:  writes an integer array to a file                              */
/* Does falal error exit if called with a non-int object                    */
/* Returns ERROR on I/O error, else RET_OK.                                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

extern int T_INT;

int i_write (register int *ip, register FILE *fp)
    /*ip:   data pointer for integer array     */
    /*fp: file pointer to read integers from */
    {
    register i;        /* index into integer array           */

#ifdef CAREFUL

    (void) s_typecheck ((char *)ip, "i_write", T_INT);

#endif

    for (i = 0; i < obj_count (ip); i++)
	puti (ip [i], fp);

    if ((!feof (fp)) && (!ferror (fp))) /* check for I/O error */
	return (RET_OK);

    g_errno = E_IO;
    return (ERROR);
    }

