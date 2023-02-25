/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)38	1.6  src/tenplus/lib/obj/sf_write.c, tenplus, tenplus411, GOLD410 3/23/93 12:07:24";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_write
 * 
 * ORIGINS:  9, 10, 27
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
 
#include    <stdio.h>
#include    "database.h"

#include <libobj.h>
 
/****************************************************************************/
/* sf_write:  writes new record to structured file                          */
/* Takes an index and an salloc'ed object (the new record).  Note: the      */
/* current index in the sfile object is incremented, so that a series of    */
/* adjacent sf_write calls will not have to put out index cookies Does      */
/* fatal error if index is negative, and adds more records to the array if  */
/* necessary.  Returns ERROR on I/O error.                                  */
/****************************************************************************/

int sf_write (SFILE *sfp, int index, char *object)
    /* sfp:               structured file pointer  */
    /* index:             record index             */
    /* object:            record to be written out */
    {
    register RECORD *records; /* records array            */
    register FILE *fp;        /* file pointer for sfile   */

    long eofloc;              /* seek location for retry  */
    int sfindex;              /* sf_index from sfp        */
    char flag;                /* object flag for sfp      */

#ifdef TRACE

    debug ("sf_write:  called with index = %d\n", index);

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_write (sfp)", T_SFILE);

    if (object)
	(void) s_typecheck (object, "sf_write (object)", T_ANY);

#endif

    if (index < 0) /* if index invalid */
	fatal ("sf_write:  index (%d) less than zero", index);

    if (obj_getflag (sfp, SF_RONLY)) /* make sure file isn't read-only */
	fatal ("sf_write:  attempt to modify read-only delta file '%s'",
	       obj_name (sfp));

    records = sf_records (sfp);
    fp = sf_fp (sfp);

    /* if index off end of the array, insert more records.             */
    /* Note that s_insert is used to extend the array since an insert  */
    /* cookie is not necessary.  The insertion is implied by the set   */
    /* index cookie.                                                   */

    if (index >= obj_count (records))
	{
	records = (RECORD *) s_realloc ((char *)records, index + 1);

	if (records == (RECORD *) ERROR)
	    return (ERROR); /* Note:  g_errno already set */

	sfp->sf__records = records;
	}

    eofloc = sf_eofloc (sfp); /* remember eofloc for I/O retries */
    sfindex = sf_index (sfp); /* remember current index          */
    flag = obj_flag (sfp);    /* remember object flag for sfp    */

   for(;;) 
	{
	sf_setindex (sfp, index); /* set current index */
	records [index].r__fileloc = sf_eofloc (sfp) + 1; /* save file loc */
	sf_seek (sfp, sf_eofloc (sfp), 'w'); /* seek to sfile write location */

	(void)s_write (object, fp);

	if ((fflush (fp) != EOF) && (!feof (fp)) && (!ferror (fp)))
	    break;

	if (g_diskfcn != NULL)
	    {
	    (*g_diskfcn)(obj_name (sfp), "sf_write"); /* prompt user to fix disk */
	    clearerr (fp);
	    (void) fseek (fp, eofloc, 0); /* not sf_seek to avoid optimization */
	    sfp->sf__eofloc = eofloc;
	    sfp->sf__index = sfindex;
	    obj_fixflag (sfp, flag);
	    }
	else
	    {
	    g_errno = E_IO;
	    return (ERROR);
	    }
	}
    if ((object != NULL) && 
	((obj_type (object) != T_CHAR) || (obj_name (object) != NULL)))
	obj_setflag (records, RC_NONTEXT);      /* update RC_NONTEXT flag */

    sfp->sf__eofloc = ftell (fp); /* reset write location for file */
    sfp->sf__index++; /* increment index to handle consecutive writes */
    return (RET_OK);
    }

