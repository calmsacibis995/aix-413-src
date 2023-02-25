static char sccsid[] = "@(#)32	1.7  src/tenplus/lib/obj/sf_open.c, tenplus, tenplus411, GOLD410 10/27/93 11:37:03";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:  sf_open
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
#include    <stdio.h>
#include    "database.h"
#include    <unistd.h>
#include    <time.h>
#include    <errno.h>
#include    <libobj.h>

extern errno;

/****************************************************************************/
/* sf_open:  opens a structured file.                                       */
/* Returns a pointer to a sfile object, or ERROR if the file cannot be      */
/* opened Second argument determines mode:  "r" for read-only, "c" for      */
/* create, (read/write), and "u" for update (read/write) Sets the current   */
/* index to zero.  Puts "start" cookie info in sfile object.                */
/****************************************************************************/


SFILE *sf_open (register char *name, register char *mode)
       /* file name                         */
       /* "r", "c", or "u"                  */
    {
    register FILE *fp;         /* file pointer                      */
    register SFILE *sfp;       /* sfile object for this file        */
    register RECORD *records;  /* records array for structured file */
    register char *openstring; /* fopen mode string                 */
    long arrayloc;             /* location of start of record array */
    long eofloc;               /* write location after open         */
    long daytime;              /* current time                      */
    long filelength;           /* file length                       */



#ifdef TRACE

    debug ("sf_open:  called with name = '%s'\n", name);

#endif

    openstring = NULL;

    switch (*mode)
	{
	case 'r':   /* read-only */
	    openstring = "r";
	    break;

	case 'c':   /* create    */
	    (void) fbackup (name); /* back up file if it exists */
	    openstring = "w+";
	    break;

	case 'u':   /* update    */
	    if (fexists (name))
		openstring = "r+";
	    else
		openstring = "w+";
	    break;
	}

    if ((mode [1] != '\0') || (openstring == NULL)) /* if bad mode string */
	{
	g_errno = E_MISC;
	return ((SFILE *) ERROR);
	}
   for(;;) 
	{
	fp = fopen (name, openstring); /* try to open file */

	if (fp != NULL)
	    break;

	if ((errno != ENOSPC) || (g_diskfcn == NULL))
	    {
	    g_errno = E_FOPEN;
	    return ((SFILE *) ERROR);
	    }
	(*g_diskfcn) (name, "sf_open-1"); /* prompt user to fix disk */
	}


    (void) fseek (fp, 0L, 2);
    filelength = ftell (fp);

    if (filelength != 0L) /* if the file is not zero length */
	{
	/* the last seven characters in the file should be the "end" cookie */

	(void) fseek (fp, -6L, 2);



	if ((fgetc (fp) != COOKIE) || 
	    (fgetc (fp) != C_END))
	    {
	    (void)fclose (fp);
	    g_errno = E_FILE;
	    return ((SFILE *) ERROR);
	    }

	arrayloc = getl (fp);    /* get start of record array */

	(void) fseek (fp, arrayloc, 0);
	records = (RECORD *) s_read (fp); /* read in record array */

	if ((records == (RECORD *) ERROR) || 
	    (obj_type (records) != T_RECORD))
	    {
	    (void)fclose (fp);
	    g_errno = E_ARRAY;
	    return ((SFILE *) ERROR);
	    }
	eofloc = arrayloc - 2; /* overwrite array cookie and records array */


	}
    else /* if file is zero length */
	{
	if (*mode == 'r') /* empty files are not structured */
	    {
	    (void)fclose (fp);
	    g_errno = E_FILE;
	    return ((SFILE *) ERROR);
	    }

	records = (RECORD *) s_alloc (0, T_RECORD, NULL);

	if (records == (RECORD *) ERROR) /* check allocation failure */
	    {
	    (void)fclose (fp);
	    g_errno = E_ALLOC;
	    return ((SFILE *) ERROR);
	    }
	eofloc = 0; /* start writes at begining of file */
	}
    (void) time (&daytime);                /* get current time from UNIX       */

    /* allocate and initialize sfile object */

    sfp = (SFILE *) s_alloc (1, T_SFILE, name);

    if (sfp == (SFILE *) ERROR) /* check for allocation failure */
	{
	g_errno = E_ALLOC;
	return ((SFILE *) ERROR);
	}
    sfp->sf__fp = fp;               /* file pointer                     */
    sfp->sf__records = records;     /* record array for file            */
    sfp->sf__index = 0;             /* current index in current node    */
    sfp->sf__eofloc = eofloc;       /* overwrite array cookie and array */
    sfp->sf__uid = getuid ();       /* user's user id                   */
    sfp->sf__gid = getgid ();       /* user's group id                  */
    sfp->sf__time = daytime;        /* time the file was opened         */


    obj_setflag (sfp, SF_READ);     /* SF_READ true, SF_MODIFIED false  */

    if (*mode == 'r') /* set read-only flag if necessary */
	obj_setflag (sfp, SF_RONLY);
    else
	if (eofloc == 0L) /* if file was created, force out start cookie */

	   for(;;) 
		{
		sf_setindex (sfp, 0);  /* and set MODIFIED bit. */

		if ((fflush (fp) != EOF) && (!feof (fp)) && (!ferror (fp)))
		    break;

		if (g_diskfcn != NULL)
		    {
		    (*g_diskfcn) (name, "sf_open-2"); /* prompt user to fix disk */
		    clearerr (fp);
		    (void) fseek (fp, eofloc, 0); /* not sf_seek to avoid optimization */
		    sfp->sf__eofloc = eofloc;
		    sfp->sf__index = 0; /* reset index */
		    obj_clearflag (sfp, SF_MODIFIED);
		    }
		else
		    {
		    g_errno = E_IO;
		    return ((SFILE *) ERROR);
		    }
		}
    return (sfp);
    }

