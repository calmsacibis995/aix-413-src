/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)36	1.5  src/tenplus/lib/obj/sf_sheader.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:27";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_sheader
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
/* sf_sheader:  reads header info for a record into an sheader structure    */
/* Returns a pointer to a static structure, ERROR on read errors, or        */
/* NULL for NULL records                                                    */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

SHEADER *sf_sheader (SFILE *sfp, int index)
    /*sfp:     sfile object    */
    /*index:   record index    */
    {
    register FILE *fp;        /* file pointer for sfile   */
    register RECORD *records; /* records array            */
    long fileloc;             /* file location of record  */

#ifdef DEBUG
    debug ("sf_sheader:  sfp = 0%o, index = %d", sfp, index);
#endif

    records = sf_records (sfp);
    fp = sf_fp (sfp);

#ifdef CAREFUL
    (void) s_typecheck ((char *)sfp, "sf_sheader (sfp)", T_SFILE);
    (void) s_typecheck ((char *)records, "sf_sheader (records)", T_RECORD);
#endif

    if ((index < 0) || (index >= obj_count (records)))
	{
	g_errno = E_MISC;
	return ((SHEADER *) ERROR);
	}
    fileloc = r_fileloc (&records [index]); /* get file location of record */

    if (fileloc == NORECORD) /* if no record, return NULL */
	return (NULL);
    else
	fileloc--; /* otherwise convert to file location */

    sf_seek (sfp, fileloc, 'r'); /* seek to read location */

    return (s_sheader (fp));
    }
