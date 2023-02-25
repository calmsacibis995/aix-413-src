static char sccsid[] = "@(#)13	1.7  src/tenplus/sf_progs/newfile.c, tenplus, tenplus411, GOLD410 11/4/93 12:11:58";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, newfile
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
 
/****************************************************************************/
/* file:  newfile.c                                                         */
/*                                                                          */
/* program to make a new delta file from a text file                        */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <database.h>
#include <libobj.h>
#include "newfile_msg.h"
#include <locale.h>

void newfile (char *, char *);

int MultibyteCodeSet;

/* Catalog Descriptor */
nl_catd g_catd;

/* macro for general catalog access */
#define MSGSTR(num,str)		catgets(g_catd, MS_NEWFILE, num, str)

/****************************************************************************/
/* main:  parses args and calls newfile                                     */
/****************************************************************************/

main (int argc, char **argv)
    {
    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd = catopen(MF_NEWFILE, NL_CAT_LOCALE);

    if ((argc != 2) && (argc != 3))
	fatal (MSGSTR(M_USAGE,"Usage: Newfile TextFile [DeltaFile]"));

    if (argc == 2)
	newfile (argv [1], argv [1]);
    else
	newfile (argv [1], argv [2]);
    catclose(g_catd);
    exit (0);
    return(0); /* return put here to make lint happy */
    }

/****************************************************************************/
/* newfile:  converts a text file into a delta file                         */
/****************************************************************************/

void newfile (char *textfile, char *deltafile)
    /* textfilei:       text file name  */
    /* deltafile:       delta file name */
    {
    register index;      /* record index                */
    register SFILE *sfp; /* sfile object for delta file */
    register FILE *fp;   /* input file pointer          */
    char *object;        /* salloc version of line      */

#ifdef TRACE

    (void) fprintf (stderr,
	     "newfile:  called w/ textfile = '%s', deltafile ='%s'\n",
	     textfile, deltafile);
#endif

    if (isdelta (textfile))
	return;

    fp = fopen (textfile, "r");

    if (fp == NULL)
	fatal (MSGSTR(M_OPEN,"newfile: 0611-177 Cannot open text file %s."), textfile);

    sfp = sf_open (deltafile, "c"); /* open new delta file */

    if (sfp == (SFILE *) ERROR)
	fatal (MSGSTR(M_CREATE,"newfile: 0611-178 Cannot create delta file %s."), deltafile);

    g_setflag (G_COREDUMP);

    for (index = 0; ; index++)
	{
	object = s_getline (fp);
	if (object == NULL)
	    break;

	if (sf_write (sfp, index, object) == ERROR)
	    fatal (MSGSTR(M_WRITE,"newfile: 0611-179 Cannot write record %d. The disk may be full."), index);

	s_free (object);
	}
    if (sf_close (sfp) == ERROR)
	fatal (MSGSTR(M_CLOSE,"newfile: 0611-180 Cannot close. The disk may be full."));
    }

