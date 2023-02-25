#if !defined(lint)
static char sccsid[] = "@(#)47	1.6  src/tenplus/lib/obj/c_read.c, tenplus, tenplus411, GOLD410 3/23/93 12:04:07";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	c_read
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
/* c_read:  reads a character from a file and puts it at a given location   */
/* Does fatal error exit if called with non-CHAR object.                    */
/* Returns ERROR on I/O error, else RET_OK.                                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int c_read (register char *cp, register FILE *fp)
     /* data pointer for character array */
     /* file pointer to read array from  */
    {
#ifdef CAREFUL

    (void) s_typecheck (cp, "c_read", T_CHAR);

#endif

    (void)fread (cp, sizeof (char), obj_count (cp), fp);

    if ((!feof (fp)) && (!ferror (fp))) /* check for I/O error */
    {
        return (RET_OK);
    }

    g_errno = E_IO;
    return (ERROR);
    }

