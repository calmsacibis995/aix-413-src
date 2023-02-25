/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)25	1.6  src/tenplus/lib/obj/sf_delete.c, tenplus, tenplus411, GOLD410 3/23/93 12:06:45";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_delete
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
 
/****************************************************************************/
/* sf_delete:  deletes count records starting at a given index              */
/* Returns ERROR on allocation failure.                                     */
/* If count is minus one, deletes rest of records starting at index         */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"

#include <libobj.h>

int sf_delete (register SFILE *sfp, register index, register count)
    /*sfp:     structured file pointer   */
    /*index:   record array index        */
    /*count:   number of lines to delete */
    {
    register RECORD *rp;    /* new records array pointer */

    register FILE *fp;      /* file pointer for sfp      */
    long eofloc;            /* seek location for retry   */
    int sfindex;            /* sf_index from sfp         */
    char flag;              /* object flag for sfp       */

#ifdef TRACE

    debug ("sf_delete:  called with index = %d, count = %d\n",
	   index, count);
#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_dline", T_SFILE);

#endif

    if (obj_getflag (sfp, SF_RONLY)) /* make sure file isn't read-only */
	fatal ("sf_delete:  attempt to modify read-only delta file '%s'",
	       obj_name (sfp));

    rp = (RECORD *) s_delete ((char *)sf_records (sfp), (int)index, (int)count);

    if (rp == (RECORD *) ERROR)
	return (ERROR); /* Note:  g_errno already set */

    sfp->sf__records = rp;            /* set new record pointer  */

    eofloc = sf_eofloc (sfp); /* remember eofloc for I/O retries */
    sfindex = sf_index (sfp); /* remember current index          */
    flag = obj_flag (sfp);    /* remember object flag for sfp    */
    fp = sf_fp (sfp);

   for(;;) 
	{
	sf_setindex (sfp, index);         /* set current index       */
	sf_cookie (sfp, C_DELETE, count); /* write out delete cookie */

	if ((fflush (fp) != EOF) && (!feof (fp)) && (!ferror (fp)))
	    break;

	if (g_diskfcn != NULL)
	    {
	    (*g_diskfcn)(obj_name (sfp), "sf_delete"); /* prompt user to fix disk */
	    clearerr (fp);
	    (void) fseek (fp, eofloc, 0); /* not sf_seek to avoid optimization */
	    sfp->sf__eofloc = eofloc;
	    sfp->sf__index = sfindex;
	    obj_fixflag (sfp, flag);
	    }
	}
    return (RET_OK);
    }

