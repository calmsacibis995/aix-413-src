/*LINTLIBRARY*/
#if !defined(lint)
static char sccsid[] = "@(#)27	1.6  src/tenplus/lib/obj/sf_getflds.c, tenplus, tenplus411, GOLD410 3/23/93 12:06:56";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	sf_getfields, skipfield, readfield
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
/* sf_getfields:  partial record reader for structured files                */
/*                                                                          */
/* This routine is similar to sf_read, except that it takes a list of       */
/* field names and returns only those fields of the record.  This saves     */
/* allocations and also read time.                                          */
/****************************************************************************/

#include    <stdio.h>
#include    "database.h"


#include <libobj.h>

#define SIZE (512 * MB_LEN_MAX)

static int skipfield (FILE *, register SHEADER *);
static char *readfield (FILE *, register SHEADER *);

POINTER *sf_getfields (SFILE *sfp, int index, POINTER *names)
    /*sfp:    sfile object    */
    /* index:  record index    */
    /*names:  field name list */
    {
    register i;               /* record index             */
    register POINTER *tree;   /* return tree              */
    register SHEADER *header; /* header pointer           */
    register count;           /* number of records        */
    register RECORD *records; /* records array            */
    register FILE *fp;        /* file pointer for sfile   */
    register char *field;     /* field being read         */
    long fileloc;             /* file location of record  */
/*
    extern char *readfield ();
*/

#ifdef DEBUG
    debug ("sf_getfields:  sfp = 0%o, index = %d, names = 0%o",
	   sfp, index, names);
#endif

    records = sf_records (sfp);
    fp = sf_fp (sfp);

#ifdef CAREFUL
    (void) s_typecheck ((char *)sfp, "sf_getfields (sfp)", T_SFILE);
    (void) s_typecheck ((char *)records, "sf_getfields (records)", T_RECORD);
    (void) s_typecheck ((char *)names, "sf_getfields (names)", T_POINTER);
#endif

    if ((index < 0) || (index >= obj_count (records)))
	{
	g_errno = E_MISC;
	return ((POINTER *) ERROR);
	}
    fileloc = r_fileloc (&records [index]); /* get file location of record */
    tree = (POINTER *) s_alloc (0, T_POINTER, NULL);

    if (fileloc == NORECORD) /* if no record, return NULL */
	return (tree);
    else
	fileloc--; /* otherwise convert to file location */

    sf_seek (sfp, fileloc, 'r'); /* seek to read location */

    header = s_sheader (fp);

    if (header == (SHEADER *) ERROR)
	{
	error: /* common error return */

	s_free((char *)tree);
	return ((POINTER *) ERROR); /* NOTE:  g_errno already set */
	}
    if ((header == NULL) || (sh_type (header) != T_POINTER))
	return (tree);

    if (sh_name (header))
	s_newname ((char *)tree, (char *)sh_name (header));

    obj_setflag (tree, sh_flag (header));

    count = sh_count (header);

    for (i = 0; i < count; i++)
	{
	header = s_sheader (fp);

	if (header == (SHEADER *) ERROR)
	    goto error;

	if (header == NULL)
	    continue;

	if (s_inlist (sh_name (header), names))
	    {
	    field = readfield (fp, header);

	    if (field == (char *) ERROR)
		goto error;

	    tree = s_append (tree, field);

	    if (obj_count (tree) == obj_count (names))
		return (tree);
	    }
	else
	    {
	    if (skipfield (fp, header) == ERROR)
		goto error;
	    }
	}
    return (tree);
    }

/****************************************************************************/
/* skipfield:  skips over a field in a record                               */
/* returns ERROR on read errors.                                            */
/****************************************************************************/

static int skipfield (FILE *fp, register SHEADER *header)
    /*fp:      file pointer        */
    /*header:  object header       */
    {
    register i;               /* POINTER array index */
    register count;           /* POINTER array count */
    register type;            /* object type         */
    register bytes;           /* bytes read in       */
    char buffer [SIZE];        /* input buffer        */

#ifdef DEBUG
    debug ("skipfield:  fp = 0%o", fp);
#endif

    type = sh_type (header);
    count = sh_count (header);

    if (type == T_CHAR)
	while ((bytes = min (count, SIZE)) > 0)
	    {
	    (void)fread (buffer, 1, bytes, fp);
	    count -= bytes;
	    }
    else if (type == T_POINTER)
	{
	for (i = 0; i < count; i++)
	    {
	    header = s_sheader (fp);

	    if (header == NULL)
		continue;

	    if (header == (SHEADER *) ERROR)
		return (ERROR); /* Note g_errno already set */

	    if (skipfield (fp, header) == ERROR)
		return (ERROR);
	    }
	}
    else
	{
	g_errno = E_MISC; /* illegal object type */
	return (ERROR);
	}
    return (RET_OK);
    }

/****************************************************************************/
/* readfield:  version of s_read that takes a header structure pointer      */
/* Returns object or ERROR on read errors                                   */
/****************************************************************************/

static char *readfield (FILE *fp, register SHEADER *header)
    /*fp:      file pointer        */
    /*header:  object header       */
    {
    register char *cp;         /* data pointer for object       */
    register int (*reader) (void *, FILE *); /* read function for data type   */

#ifdef DEBUG
    debug ("readfield:  fp = 0%o, header = 0%o", fp, header);
#endif

    /* allocate object and use its name if it has one */

    cp = s_alloc (sh_count (header), sh_type (header),
		  *sh_name (header) ? sh_name (header) : NULL);

    if (cp == (char *) ERROR)    /* check for allocation failure */
	return ((char *) ERROR); /* Note:  g_errno already set   */

    obj_setflag (cp, sh_flag (header)); /* set flag bits in object */
    reader = dt_reader (&g_dtypes [sh_type (header)]); /* get read function */

    /* Note:  the datatype specific readers set g_errno */

    if ((*reader) (cp, fp) == ERROR) /* read in each object in array */
	return ((char *) ERROR);     /* check for I/O error          */

    return (cp);
    }

