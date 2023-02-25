#if !defined(lint)
static char sccsid[] = "@(#)85	1.6  src/tenplus/lib/obj/r_read.c, tenplus, tenplus411, GOLD410 3/23/93 12:05:32";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	r_read
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
/* r_read:  reads a record object from a file                               */
/* Does falal error exit if called with a non-RECORD object                 */
/* Returns ERROR on I/O error, else RET_OK.                                 */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int r_read (register RECORD *records, register FILE *fp)
    /*records:  data pointer for record array     */
    /*fp:       file pointer to read records from */
    {
    register i;               /* index into record array        */

#ifdef CAREFUL

    (void) s_typecheck ((char *)records, "r_read", T_RECORD);

#endif

    for (i = 0; i < obj_count (records); i++)
	records [i].r__fileloc = getl (fp);

    if ((!feof (fp)) && (!ferror (fp))) /* check for I/O error */
	return (RET_OK);

    g_errno = E_IO;
    return (ERROR);
    }

