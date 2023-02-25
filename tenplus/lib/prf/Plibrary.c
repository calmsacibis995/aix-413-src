#if !defined(lint)
static char sccsid[] = "@(#)42	1.6  src/tenplus/lib/prf/Plibrary.c, tenplus, tenplus411, GOLD410 3/23/93 12:07:49";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	Pfopen, Pcleanup, Precord, Precindex, Pgetmulti, Pgetsingle
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
/* file:  Plibrary.c                                                        */
/*                                                                          */
/* Pfopen, Pcleanup and Precord, Pgetsingle and Pgetmulti  routines for the */
/* profile library                                                          */
/****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include <chardefs.h>
#include <database.h>
#include <libprf.h>
#include <libobj.h>

/***** globals *****/

static char    *g_record;        /* current profile record */
static int      g_recindex = -1; /* current record index   */
static POINTER *g_recnames;      /* names of all records   */
static SFILE   *g_sfp;           /* current sfile pointer  */
static int      g_inode;         /* inode of current file  */
static time_t   g_mtime;         /* last mod time of file  */

/** Local Functions */

static int Precindex (char *);
static char *Precord(char *);

/****************************************************************************/
/* Pfopen:  opens a profile file                                            */
/*                                                                          */
/* This routine takes a file name and opens that file as the current        */
/* profile file.  Returns ERROR if this cannot be done, else RET_OK.        */
/* This routine handles the case of reopening the current profile file.  If */
/* that file has not been changed since the last open, Pfopen does nothing. */
/* If open fails, there will be no current profile file.                    */
/****************************************************************************/

int Pfopen (char *filename, char *modes)
    /*filename:  name of file to open */
    /*modes:     sf_open mode string  */
    {
    struct stat buffer; /* stat call buffer */

#ifdef DEBUG
    debug ("Pfopen:  filename = '%s', modes = '%s'", filename, modes);
#endif

    if (stat (filename, &buffer) == ERROR) /* if file does not exist */
	{
	Pcleanup ();
	return (ERROR);
	}
    if (g_sfp && (strcmp(obj_name (g_sfp), filename) == 0)) /* reopening same file? */
	{
	if ((buffer.st_ino == g_inode) && (buffer.st_mtime == g_mtime))
	    return (RET_OK);
	}
    Pcleanup ();

#ifdef DEBUG
    debug ("Pfopen:  opening file");
#endif

    g_sfp = sf_open (filename, modes);

    if (g_sfp == (SFILE *) ERROR)
	{
	g_sfp = NULL;
	return (ERROR);
	}
    g_inode = buffer.st_ino;
    g_mtime = buffer.st_mtime;
    return (RET_OK);
    }

/****************************************************************************/
/* Pcleanup:  closes profile and frees profile library globals              */
/*                                                                          */
/* This routine closes the current profile file and frees all internal      */
/* library data structures.  It is used by the editor to save space and can */
/* be used by helpers for the same purpose.                                 */
/****************************************************************************/

void Pcleanup (void)
    {
#ifdef DEBUG
    debug ("Pcleanup called");
#endif

    s_free (g_record);
    g_record = NULL;

    g_recindex = -1;

    s_free ((char *)g_recnames);
    g_recnames = NULL;

    if (g_sfp)
	{
	if (sf_close (g_sfp) == ERROR)
	    fatal ("Pcleanup:  cannot close profile file");

	g_sfp = NULL;
	}
    g_inode = 0;
    g_mtime = 0L;
    }

/****************************************************************************/
/* Precord:  returns a profile record for a given name.                     */
/*                                                                          */
/* If the name is numeric, atoi is used to get the record index.  If the    */
/* name in not numeric, a linear search is done on the list of record names */
/* Returns the record tree, or ERROR if the record cannot be found.         */
/****************************************************************************/

static char *Precord (char *name)
    /*name:   record name (or number) */
    {
    register index1;        /* record index */
    register char *record; /* record data  */
    register SFILE *sfp;   /* local g_sfp  */

#ifdef DEBUG
    debug ("Precord:  name = '%s'", name);
#endif

    sfp = g_sfp;

    if (sfp == NULL) /* if no file */
	return ((char *) ERROR);

    if (isdigit (*name))
	index1 = atoi (name);
    else
	{
	index1 = Precindex (name);

	if (index1 == ERROR) /* if no record with that name */
	    return ((char *) ERROR);
	}
    if (index1 == g_recindex)
	return (g_record);

#ifdef DEBUG
    debug ("Precord:  reading record %d", index1);
#endif

    record = sf_read (sfp, index1);

    if (record == (char *) ERROR)
	return ((char *) ERROR);

    s_free (g_record);
    g_record = record;
    g_recindex = index1;
    return (record);
    }

/****************************************************************************/
/* Precindex:  internal routine for finding the record index from a name    */
/*                                                                          */
/* Sets up g_recnames array if necessary by calling sf_getnames routine.    */
/* Returns index, or ERROR if no record has the given name.                 */
/****************************************************************************/

static int Precindex (char *name)
    /*name:     record name */
    {
    register i;                 /* loop index       */
    register POINTER *recnames; /* local g_recnames */

#ifdef DEBUG
    debug ("Precindex:  name = '%s'", name);
#endif

    if (g_recnames == NULL)
	g_recnames = sf_getnames (g_sfp);

    recnames = g_recnames;

    for (i = 0; i < obj_count (recnames); i++)
	if ((recnames [i]) && (strcmp(name, recnames [i]) == 0))
	    return (i);

    return (ERROR);
    }

/****************************************************************************/
/* Pgetmulti:  returns the data for a multi line field.                     */
/*                                                                          */
/* This routine takes a data path in the profile file for a multi line text */
/* field and returns the value as a structured character array.  It returns */
/* ERROR if the field does not exist.  The data path may be simple or       */
/* indexed (with an imbedded asterisk).                                     */
/****************************************************************************/

POINTER *Pgetmulti (char *datapath)
    /*datapath:    data path from the top of the profile file */
    {
    register i;             /* record index               */
    register char *path;    /* datapath pointer           */
    register char *cp;      /* used to make recname       */
    register char *record;  /* record data                */
    register POINTER *list; /* list of lines for "*" case */
    register SFILE *sfp;    /* sfile pointer              */
    char recname [PATH_MAX];     /* record name (or number)    */

#ifdef DEBUG
    debug ("Pgetmulti:  datapath = '%s'", datapath);
#endif

    /***** extract record name from path *****/

    for (path = datapath, cp = recname; *path; path++, cp++)
	if (*path == DIR_SEPARATOR)
	    {
	    path++;
	    break;
	    }
	else
	    *cp = *path;

    *cp = '\0';

    if (! strcmp(recname, "*") == 0) /* if data in one record */
	{
	record = Precord (recname);

	if (record == (char *) ERROR)
	    return ((POINTER *) ERROR);

	return (s_getmulti ((POINTER *)record, path));
	}

    /***** recname is "*", which means data is is all records *****/

    sfp = g_sfp;
    list = (POINTER *) s_alloc (obj_count (sf_records (sfp)), T_POINTER, NULL);

    for (i = 0; i < obj_count (sf_records (sfp)); i++)
	{
	record = sf_read (sfp, i);

	if ((record != (char *) ERROR) && (record != NULL))
	    {
	    list [i] = s_getsingle ((POINTER *)record, path);
	    s_free (record);

	    if (list [i] != (char *) ERROR)
		continue;
	    }
	list [i] = s_alloc (0, T_CHAR, NULL);
	}
    return (list);
    }

/****************************************************************************/
/* Pgetsingle:  returns the data for a single line field                    */
/*                                                                          */
/* This routine takes a data path in the profile file for a single line     */
/* text field and returns the value as a structured character array.  It    */
/* returns ERROR if the field does not exist.  The data path may be simple  */
/* or indexed (with an imbedded asterisk).                                  */
/****************************************************************************/

char *Pgetsingle (char *datapath)
    /*datapath:   data path from the top of the profile file */
    {
    register char *path;   /* datapath pointer        */
    register char *cp;     /* used to make recname    */
    register char *record; /* record data             */
    char recname [PATH_MAX];    /* record name (or number) */

#ifdef DEBUG
    debug ("Pgetsingle:  datapath = '%s'", datapath);
#endif

    /***** extract record name from path *****/

    for (path = datapath, cp = recname; *path; path++, cp++)
	if (*path == DIR_SEPARATOR)
	    {
	    path++;
	    break;
	    }
	else
	    *cp = *path;

    *cp = '\0';

    if (strcmp(recname, "*") == 0) /* if path starts with asterisk */
	recname [0] = '0';  /* use record zero              */

    record = Precord (recname);

    if (record == (char *) ERROR)
	return ((char *) ERROR);

    return (s_getsingle ((POINTER *)record, path));
    }

