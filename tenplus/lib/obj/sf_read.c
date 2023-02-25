/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)34	1.5  src/tenplus/lib/obj/sf_read.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:20";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_read
 * 
 * ORIGINS:  9, 10, 27
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
/* sf_read:  reads in a record from a structured file                       */
/* Returns NULL if file location is NORECORD.                               */
/* Returns an salloc'ed object, or ERROR on I/O error or invalid index      */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

char *sf_read (register SFILE *sfp, register index)
          /* structured file pointer  */
         /* record array index       */
    {
    register RECORD *records; /* records array            */
    long fileloc;             /* file location of record  */

#ifdef TRACE

    debug ("sf_read:  called with index = %d\n", index);

#endif

    records = sf_records (sfp);

#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_read (sfp)", T_SFILE);
    (void) s_typecheck ((char *)records, "sf_read (records)", T_RECORD);

#endif

    if ((index < 0) || (index >= obj_count (records)))
	{
	g_errno = E_MISC;
	return ((char *) ERROR);
	}
    fileloc = r_fileloc (&records [index]); /* get file location of record */

    if (fileloc == NORECORD) /* if no record, return NULL */
	return ((char *) NULL);
    else
	fileloc--; /* otherwise convert to file location */

    sf_seek (sfp, fileloc, 'r'); /* seek to read location */

    return (s_read (sf_fp (sfp))); /* read record and return it */
    }

