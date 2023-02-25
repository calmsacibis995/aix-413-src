static char sccsid[] = "@(#)11	1.7  src/tenplus/sf_progs/history.c, tenplus, tenplus411, GOLD410 11/4/93 12:11:54";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, cookie
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
/* file:  history.c                                                         */
/*                                                                          */
/* program used to read and interpret the contents of a structured file     */
/*                                                                          */
/* Name changed from "delta" to "history" 2/82                              */
/*                                                                          */
/* If invoked as "history", prints out all cookies and                      */
/*  contents of each record.                                                */
/* If invoked as "versions", prints out start cookies only.                 */
/****************************************************************************/

#include    <string.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <locale.h>
#include    <limits.h>
#include    <time.h>

/* Ined Includes */
#include    <chardefs.h>
#include    <database.h>
#include    <libobj.h>
#include    "versions_msg.h"

#define SIZE (256 * MB_LEN_MAX) /* input line size */

int   g_index;             /* current index in structured file */
int   fullhistory = TRUE;  /* TRUE means history, FALSE means versions */
char *progname;            /* program name for fatal errors */

static int cookie (FILE *);

int MultibyteCodeSet;

/* Catalog descriptor */
nl_catd g_catd;

/* macro for general message catalog access */
#define MSGSTR(num,str) 	catgets (g_catd, MS_VERSIONS,num, str)

/****************************************************************************/
/* main:  top level routine                                                 */
/* Reads lines in and calls cookie routine if necessary                     */
/****************************************************************************/

main (int argc, char **argv)
    {
    register FILE *fp;     /* file pointer         */
    register char *name;   /* file name            */
    register char *object; /* record object        */
    extern void noop (void);
    extern char *strrchr(const char *, int );

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd = catopen(MF_VERSIONS, NL_CAT_LOCALE);

    progname = strrchr (argv [0], DIR_SEPARATOR);      /* trim off absolute path */
    progname = (progname ? ++progname : argv [0]);

    if (argc != 2)
	fatal (MSGSTR(M_USAGE,"Usage: %s File"), progname);

    name = argv [1];

    if ((strcmp(progname, "versions"))==0)
	fullhistory = FALSE;

    if ((fp = fopen (name, "r")) == NULL)
	fatal (MSGSTR(M_OPEN,"%1$s: 0611-202 Cannot open file %2$s."), progname, name);

    g_index = 0;
    g_debugfp = stdout; /* make s_print output come out on standard output */

    for (;;) 
	{
	if (cookie (fp)) /* if next entry in file is cookie, continue */
	    continue;

	if (feof (fp))
	    break;

	g_allocfcn = noop; /* turn off fatal handling of allocation failures */
	g_typefcn = noop;

	object = s_read (fp); /* if not, read in as record object */

	g_allocfcn = NULL; /* restore fatal handling of allocation failures */
	g_typefcn = NULL;

	if (object == (char *) ERROR)
	    fatal (MSGSTR(M_S_READ,"%s: 0611-203 There is a read error."), progname);

	if (fullhistory)
	    {
	    (void) printf (MSGSTR(M_RECORD,"Record %d:\n"), g_index);
	    s_print (object);
	    (void) putchar ('\n');
	    }
	s_free (object);
	g_index++;

	if (ferror (stdout))
	    fatal (MSGSTR(M_WRITE,"%s: 0611-204 The write to standard output failed."), progname);
	}
	catclose(g_catd);
	exit(0);
        return(0); /* return put here to keep lint happy */
    }

/****************************************************************************/
/* cookie:  handles cookie lines in the structued file                      */
/* Returns TRUE if next thing in file is cookie, else FALSE                 */
/* Note:  will change g_index for C_INDEX cookie                            */
/****************************************************************************/

static int cookie (FILE *fp)
    /* fp:                     input file pointer   */
    {
    register char *array; /* record array from file */
    register type;        /* cookie type            */
    register uid;         /* user id from cookie    */
    register gid;         /* group id from cookie   */
    long daytime;         /* time from cookie       */
    int c;               /* next char in file      */
    char line [SIZE];     /* input line buffer      */
    char date_buffer[100];


    c = getc (fp);

    if (feof (fp) || ((c & 0xff) != COOKIE))
	{
	(void) ungetc (c, fp);
	return (FALSE);
	}
    type = getc (fp);

    switch (type) /* check cookie type */
	{
	long intvalue;  /* used to store value from geti or getl */

	case C_INSERT:      /* insert records */
	    intvalue = geti (fp);
	    if (fullhistory)
		(void) printf (MSGSTR(M_INSERT,"-----> Insert %d records\n\n"), intvalue);
	    return (TRUE);

	case C_DELETE:      /* delete records */
	    intvalue = geti (fp);
	    if (fullhistory)
		(void) printf (MSGSTR(M_DELETE,"-----> Delete %d records\n\n"), intvalue);
	    return (TRUE);

	case C_INDEX:       /* set current index */
	    g_index = geti (fp);
	    if (fullhistory)
		(void) printf (MSGSTR(M_CHNG_IDX,"-----> Change current index to %d\n\n"), g_index);
	    return (TRUE);

	case C_START:       /* start information */
	    uid = getc (fp);
	    gid = getc (fp);
	    daytime = getl (fp);
	    
            (void) strftime(date_buffer, 100, "%c\n", localtime(&daytime));
	    (void) printf (MSGSTR(M_START,"=====> Start token:  user number = %1$d, group number= %2$d,\n\
				 time = %3$s\n"), uid, gid, date_buffer);
	    g_index = 0;
	    return (TRUE);

	case C_END:         /* end information   */
	    intvalue = getl (fp);
	    if (fullhistory)
		(void) printf (MSGSTR(M_END,"-----> End token, array location = %ld\n\n"), intvalue);
	    return (TRUE);

	case C_COMMENT:     /* user comment      */
	    (void)fgetline (fp, line, SIZE);
	    if (fullhistory)
		(void) printf (MSGSTR(M_COMMENT,"-----> Comment token: '%s'\n\n"), line);
	    return (TRUE);

	case C_ARRAY:       /* start of array     */
	    if (fullhistory)
		(void) printf (MSGSTR(M_ARRAY,"-----> Start of array token, current location = %ld\n"), ftell (fp));

	    array = s_read (fp); /* read in the array */

	    if (array == (char *) ERROR) /* if cannot read array */
		fatal (MSGSTR(M_READ_ARRAY,"%s: 0611-205 %s: Cannot read array from structured file."), progname);

	    if (fullhistory)
		{
		(void) printf (MSGSTR(M_REC_ARRAY,"The record array looks like:\n\n"));
		s_print (array);
		(void) putchar ('\n');
		}
	    s_free (array);
	    break;



	default:
	    if (fullhistory)
		(void) printf (MSGSTR(M_ILL_COOKIE,"0611-206 ***** Token of type %d in input file is not valid. *****\n"), line [1] & 0xff);
	    return (TRUE);
	}
    /**** We have just processed the record array or headloc cookie,   */
    /**** so we should search for the next cookie.  This handles the   */
    /**** problem of having garbage (old record array) between the end */
    /**** of the record array (or headloc cookie) and the end cookie.  */

    for (;;) 
	{
	c = getc (fp);

	if ((c & 0xff) == COOKIE)
	    {
	    (void) ungetc (c, fp);
	    return (TRUE);
	    }
	if (feof (fp))
	    return (FALSE);
	}
    }

