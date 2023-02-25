/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)23	1.6  src/tenplus/lib/obj/sf_close.c, tenplus, tenplus411, GOLD410 3/23/93 12:06:32";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_close
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
/* sf_close:  closes a structured file                                      */
/* Returns ERROR on I/O error, else RET_OK                                  */
/****************************************************************************/

#include    <stdio.h>
#include    <database.h>

extern int errno;

#include <libobj.h>

int sf_close (register SFILE *sfp)
     /* structured file pointer          */
    {
    register FILE *fp;        /* file pointer                     */
    long arrayloc;            /* location of array in output file */
    long eofloc;              /* seek location for retry          */

#ifdef TRACE

    debug ("sf_close:  called on file '%s'\n", obj_name (sfp));

#endif

#ifdef CAREFUL

    (void) s_typecheck ((char *)sfp, "sf_close (sfp)", T_SFILE);
    (void) s_typecheck ((char *)sf_records (sfp), "s_close (records)", T_RECORD);

#endif

    fp = sf_fp (sfp);

    eofloc = sf_eofloc (sfp); /* remember eofloc for I/O retries */

   if (obj_getflag (sfp, SF_MODIFIED)) /* if file was modified */
           for(;;)	
	    {
	    sf_cookie (sfp, C_ARRAY);          /* write out "array" cookie */
	    arrayloc = ftell (fp);             /* remember array location  */

	    /***** write out record array   *****/

	    if (s_write((char *)sf_records (sfp), fp) != ERROR)
		{
		(void) fseek (fp, 0L, 2);                 /* put END cookie at eof    */
		sf_cookie (sfp, C_END, &arrayloc); /* write out "end" cookie   */
		}

	    if ((fflush (fp) != EOF) && (!ferror (fp))) /* flush data from buffers */
		{
		(void)fclose (fp);
		break;
		}
	     else
		{
		if (g_diskfcn != NULL)
		    {
		    (*g_diskfcn)(obj_name (sfp), "sf_close");  /*prompt user to fix disk */
		    clearerr (fp);
		    (void) fseek (fp, eofloc, 0); /* not sf_seek to avoid optimization */
		    sfp->sf__eofloc = eofloc;
		    }
		else
		    {
		    g_errno = E_IO;
		    return (ERROR);
		    }
		}
	    } /* end of for(;;) loop */
    else  /* not SF_MODIFIED, simply close the file */
	(void)fclose (fp);

    s_free ((char *)sfp);
    return (RET_OK);
    }

