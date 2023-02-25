#if !defined(lint)
static char sccsid[] = "@(#)18	1.5  src/tenplus/sf_progs/scat.c, tenplus, tenplus411, GOLD410 7/18/91 13:53:33";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main
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
/* File: scat.c - catenate together a list of structured files.             */
/* Does not perform any shape-checking.                                     */
/****************************************************************************/

#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    <locale.h>

/* Ined Headers */
#include    <database.h>
#include    <libobj.h>

int MultibyteCodeSet;


/****************************************************************************/
/* main: catenate together structured files                                 */
/****************************************************************************/

main (int argc, char *argv[])
    {
    register int i, j;
    register SFILE *isfp, *osfp;
    register outnum, limit;
    char *thisrec;

    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;

    if (argc < 3)
	{
	(void) fprintf (stderr, "usage: scat f1 [f2 ... fn] outputfile\n");
	exit (-1);
	}

    osfp = sf_open (argv [argc - 1], "c");

    if (osfp == (SFILE *) ERROR)
	{
	(void) fprintf (stderr, "Couldn't create: '%s'\n", argv [argc - 1]);
	exit (-1);
	}

    outnum = 0;

    for (i = 1; i < argc - 1; i++)
	{
	isfp = sf_open (argv [i], "r");

	if (isfp == (SFILE *) ERROR)
	    {
	    (void) sf_close (osfp);
	    (void)unlink (argv [argc - 1]);
	    (void) fprintf (stderr, "Couldn't open: '%s'\n", argv [i]);
	    exit (-1);
	    }

	limit = obj_count (sf_records (isfp));

	for (j = 0; j < limit; j++)
	    {
	    thisrec = sf_read (isfp, j);
	    if (thisrec == (char *) ERROR)
		{
		(void) fprintf (stderr, "I/O error on: '%s'\n", argv [i]);
		(void) sf_close (osfp);
		(void)unlink (argv [argc - 1]);
		exit (-1);
		}
	    if (sf_write (osfp, outnum++, thisrec) == ERROR)
		{
		(void) fprintf (stderr, "I/O error on: '%s'\n", argv [argc - 1]);
		(void) sf_close (osfp);
		(void)unlink (argv [argc - 1]);
		exit (-1);
		}
	    s_free (thisrec);
	    }
	(void) sf_close (isfp);
	}

    if (sf_close (osfp) == ERROR)
	{
	(void) fprintf (stderr, "I/O error on: '%s'\n", argv [argc - 1]);
	(void)unlink (argv [argc - 1]);
	exit (-1);
	}
    exit (0);
    return(0); /* return put here to make lint happy */
    }
