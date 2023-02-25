/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)30	1.5  src/tenplus/lib/obj/sf_getnms.c, tenplus, tenplus411, GOLD410 7/18/91 13:46:11";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_getnames
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
 
#include <stdio.h>
#include <database.h>

#include <libobj.h>

/****************************************************************************/
/* sf_getnames:  gets the names of the records in a structured file.        */
/*                                                                          */
/* Returns a pointer array with one entry per record.  The entry is left    */
/* NULL if the record has no name, else points to a structured character    */
/* array that contains the record name.                                     */
/****************************************************************************/

POINTER *sf_getnames (SFILE *sfp)
    {
    register i;                 /* record index  */
    register POINTER *names;    /* name array    */
    register RECORD *records;   /* record array  */
    register FILE *fp;          /* file pointer  */
    register type;              /* record type   */
    register length;            /* name length   */
    register char *name;        /* record name   */
    long fileloc;               /* file location */

#ifdef TRACE
    debug ("sf_getnames:  sfp = 0%o\n", sfp);
#endif

#ifdef CAREFUL
    (void) s_typecheck ((char *)sfp, "sf_getnames", T_SFILE);
#endif

    records = sf_records (sfp);
    fp = sf_fp (sfp);

    if (records == NULL)
	return ((POINTER *) s_alloc (0, T_POINTER, NULL));

    names = (POINTER *) s_alloc (obj_count (records), T_POINTER, NULL);

    for (i = 0; i < obj_count (names); i++)
	{
	fileloc = r_fileloc (&records [i]); /* get file location of record */

	if (fileloc == NORECORD) /* if no record, no name */
	    continue;
	else
	    fileloc--; /* otherwise convert to file location */

	sf_seek (sfp, fileloc, 'r'); /* seek to read location */

	type = fgetc (fp);

	if (type & 0x80) /* if abreviated header, no name */
	    continue;

	(void) fgetc (fp); /* ignore flags */
	length = fgetc (fp);

	if (length == 0)
	    continue;

	name = s_alloc (length, T_CHAR, NULL);

	if (fread (name, 1, length, fp) != length) /* read in name */
	    fatal ("sf_getnames:  read error on record %d", i);


	names [i] = name;
	}
    return (names);
    }

