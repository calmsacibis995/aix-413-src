static char sccsid[] = "@(#)10	1.8  src/tenplus/sf_progs/ghost.c, tenplus, tenplus411, GOLD410 11/4/93 12:11:49";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, ghost, timecmp, cookie, record, fmove, TDwday
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
/* file:  ghost.c                                                           */
/*                                                                          */
/* program to reconstruct previous versions of delta files                  */
/****************************************************************************/

#include    <string.h>
#include    <time.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <unistd.h>
#include    <stdlib.h>
#include    <locale.h>
#include    <langinfo.h>
#include    <limits.h>
#include    <nl_types.h>

/* INed Includes */
#include    "database.h"
#include    "libobj.h"
#include    "ghost_msg.h"

static int keepback = TRUE;  /* create a .bak file flag  */
static int prior = FALSE;    /* get prior version flag   */

#define SIZE (256 * MB_LEN_MAX) /* input line size */
#define TIME struct tm

/* global variables */
RECORD  *g_records;     /* record array         */
TIME     g_time;        /* time threshold       */

int      g_index;       /* current record index */
int      g_stopflag;    /* TRUE if time to stop */
int      g_nodate;      /* TRUE if no date arg  */

static void ghost (char *, char *);
int timecmp (TIME *, TIME *);
static void cookie (FILE *, FILE *);
static void record (FILE *, FILE *);
static void fmove (char *, char *);
int TDwday (int , int , int );

int MultibyteCodeSet;


/* Catalog descriptor */
nl_catd g_catd;


/* macro for general catalog access */
#define MSGSTR(num, str)	catgets(g_catd, MS_GHOST, num, str)

/****************************************************************************/
/* main:  top level routine.  Parses args and calls ghost                   */
/****************************************************************************/

main (int argc, char **argv)
    {
    TIME *tp;          /* time structure for current time   */
    long curtime;      /* current time                      */
    char *msg_ptr;

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd = catopen(MF_GHOST, NL_CAT_LOCALE);

    if ((argc < 2) || (argc > 7))
	(void)fatal ("usage: ghost oldfile [newfile [%s [%s]]]",
			     nl_langinfo (D_FMT), nl_langinfo (T_FMT));

    /* process no backup flag if present */
    while (argv[1][0] == '-')
	{
	if ((strcmp(argv[1], "-d"))==0)
	    keepback = FALSE;
	else if ((strcmp(argv[1], "-p"))==0)
	    prior = TRUE;
	else
	    (void)fatal (MSGSTR(M_OPTION,"ghost: 0611-427  %s is not a recognized flag."), argv[1]);
	argv++;
	argc--;
	}

    (void) time (&curtime);           /* get current time */
    tp = localtime (&curtime);

    g_time.tm_year = tp->tm_year; /* set year to current year */

    /* validate date (if entered) against expected locale format */
    if (argc >= 4)
	{
	g_time.tm_mon = g_time.tm_mday = 0;
	if (strptime (argv [3], "%x", &g_time)    == NULL ||
				   g_time.tm_mday == 0    ||
				   g_time.tm_mon  == 0)
	       (void)fatal ("ghost: date must be of the form %s",
				 nl_langinfo (D_FMT));

	if (g_time.tm_year >= 100) /* if user typed m/d/19yy */
	    g_time.tm_year %= 100;

	/* validate time (if entered) against expected locale format */
	if (argc == 5)
	    {
	    if (strptime (argv [4], "%X", &g_time) == NULL)
		(void)fatal ("ghost: time must be of the form %s",
				     nl_langinfo (T_FMT));
	    }
	else /* if date specified but not the time */
	    {
	    g_time.tm_hour = 0; /* use midnight if no time specified */
	    g_time.tm_min = 0;
	    g_time.tm_sec = 0;
	    }
	g_time.tm_wday = TDwday (g_time.tm_mday, g_time.tm_mon,
						 g_time.tm_year);
	g_nodate = FALSE;
	}

    else /* if date and time not specified */
	{
	g_time.tm_mon = tp->tm_mon; /* use current date if none specified */
	g_time.tm_mday = tp->tm_mday;
	g_time.tm_wday = tp->tm_wday;
	g_time.tm_hour = tp->tm_hour;
	g_time.tm_min = tp->tm_min;
	g_time.tm_sec = tp->tm_sec;
	g_nodate = TRUE;
	}

    if (argc == 2) /* if only one file name specified, use as both */
	(void)ghost (argv [1], argv [1]);
    else
	(void)ghost (argv [1], argv [2]);

    catclose(g_catd);
    (void)exit (0);
    return(0); /* return put here to make lint happy */
    }

/****************************************************************************/
/* ghost:  reconstructs an old delta file into a new one up to a given time */
/****************************************************************************/

static void ghost (char *oldname, char *newname)
    {
    char *tmpname;          /* temporary file name      */
    FILE *infp;             /* input file pointer       */
    FILE *outfp;            /* output file pointer      */
    int c;                  /* input test character     */
    long arrayloc;          /* file location of array   */
    char *acl;              /* file access control list */
    char *msg_ptr;
    char date_buffer[100];

    (void) strftime(date_buffer, 100, "%c\n", &g_time);
    (void) fprintf (stderr, MSGSTR(M_RECONSTRUCTING,"ghost: Reconstructing file %1$s up to time %2$s.\
    \tand saving it as file %3$s.\n"), oldname,
			    date_buffer, newname);

    tmpname = "ghost.tmp";

    if ((infp = fopen (oldname, "r")) == NULL)
	(void)fatal (MSGSTR(M_OPEN_OLD,"ghost: 0611-430 Cannot open the old change file %s."), oldname);

    if ((outfp = fopen (tmpname, "w")) == NULL)
	{

	/* try opening temporary file on /tmp */
	tmpname = "/tmp/ghost.tmp";
	if ((outfp = fopen (tmpname, "w")) == NULL)
	    (void)fatal ( MSGSTR(M_OPEN_TMP,"ghost: 0611-431 Cannot open the temporary file %s."), tmpname);
	}

    /* read through the input delta file, transfering the lines to */
    /* the output file and recreate the record array.              */

    g_records = (RECORD *) s_alloc (0, T_RECORD, NULL);
    g_index = 0;
    g_stopflag = FALSE;

    for (;;) 
	{
	c = getc (infp);
	if (feof (infp))
	    break;

	if ((c & 0xff) == COOKIE) /* if it looks like a cookie */
	    cookie (infp, outfp);
	else
	    {
	    (void) ungetc (c, infp);     /* put back first char   */
	    record (infp, outfp); /* treat input as record */
	    }
	if (ferror (outfp))
	    (void)fatal ( MSGSTR(M_IO_WRITE,"ghost: 0611-432 Cannot write to temporary file %s."), tmpname);

	if (g_stopflag) /* if time to stop */
	    break;
	}

    (void)fclose (infp);
    if (obj_count (g_records) == 0) /* if no records in file */
	debug ( MSGSTR(M_NO_RECS,"ghost: Warning: The output file %s is empty.\n"), newname);

    (void) putc (COOKIE, outfp);     /* write out "array" cookie */
    (void) putc (C_ARRAY, outfp);

    arrayloc = ftell (outfp);   /* save array location */
    (void)s_write ((char *)g_records, outfp); /* write array to file */

    (void) putc (COOKIE, outfp);     /* write out "end" cookie */
    (void) putc (C_END, outfp);
    putl (arrayloc, outfp);

    if (ferror (outfp)) /* make sure that worked */
	(void)fatal ( MSGSTR(M_WRITE,"ghost: 0611-432 Cannot write to temporary file %s."));

    if (fclose (outfp) == ERROR)
	(void)fatal ( MSGSTR(M_SF_CLOSE,"ghost: 0611-434 Cannot close the output file."));

    /* all done, so rename the tmp file to be the new filename */
    /* if the file names are the same, back up the old file    */

    acl = (char *)acl_get (oldname);

    if (((strcmp(oldname, newname))==0) && keepback)
	(void) fbackup (oldname);

    fmove (tmpname, newname); /* move temporary file to new name */

    if (acl_put (newname, acl, TRUE) != 0)
	(void) fprintf (stderr, MSGSTR(M_CHACL,"Warning: Cannot change the access control list for %s."), newname);
    }

/****************************************************************************/
/* timecmp:  compares two time structures                                   */
/* Returns <0 if time1 < time2, =0 if time1 = time2, and >0 if time1 >time2 */
/****************************************************************************/

int timecmp (TIME *tp1, TIME *tp2)
    {
    int diff;

    diff = tp1->tm_year - tp2->tm_year;
    if (diff != 0) /* years different? */
	return (diff);

    diff = tp1->tm_mon - tp2->tm_mon;

    if (diff != 0) /* months different? */
	return (diff);

    diff = tp1->tm_mday - tp2->tm_mday;

    if (diff != 0) /* days different? */
	return (diff);

    diff = tp1->tm_hour - tp2->tm_hour;

    if (diff != 0) /* hours different? */
	return (diff);

    diff = tp1->tm_min - tp2->tm_min;

    if (diff != 0) /* minutes different? */
	return (diff);

    return (tp1->tm_sec - tp2->tm_sec); /* return difference in seconds */
    }

/****************************************************************************/
/* cookie:  handles cookie lines in the delta file                          */
/* The COOKIE character has already been read in when this routine is call- */
/* ed.  Sets g_stopflag if reconstruction should be stopped at this point.  */
/****************************************************************************/

static void cookie (FILE *infp, FILE *outfp)
    {
    int value;              /* integer paramter value    */
    int type;               /* cookie type               */
    long daytime;           /* time in start cookie      */
    long fileloc;           /* output file location      */
    char line [SIZE];       /* line buffer for comment   */

    fileloc = ftell (outfp);        /* remember output file location */
    (void) putc (COOKIE, outfp);    /* echo cookie char */
    type = getc (infp);             /* get cookie type  */
    (void) putc (type, outfp);      /* echo type char   */

    switch (type) /* check cookie type */
	{
	case C_INSERT:      /* insert lines      */

	    value = geti (infp);
	    puti (value, outfp);
	    g_records = (RECORD *)s_insert((char *)g_records,g_index,value);
	    return;

	case C_DELETE:      /* delete lines or a child */

	    value = geti (infp);
	    puti (value, outfp);
	    g_records = (RECORD *)s_delete((char *)g_records,g_index, value);
	    return;

	case C_INDEX:       /* set current index */

	    value = geti (infp);
	    puti (value, outfp);
	    g_index = value;
	    return;

	case C_START:       /* start information */

	    (void) putc (getc (infp), outfp); /* read and echo uid */
	    (void) putc (getc (infp), outfp); /* read and echo gid */
	    daytime = getl (infp);            /* get start time    */
	    putl (daytime, outfp);            /* and echo it       */

	    /* if given no date arg or time before threshold, continue */

	    if (prior)  /* ghost up to but not including time specified */
		{
		if (g_nodate || (timecmp (localtime (&daytime), &g_time) < 0))
		   {
		   g_index = 0;
		   return;
		   }
		}
	    else        /* ghost up to and including time specified */
		if (g_nodate || (timecmp (localtime (&daytime), &g_time) <= 0))
		   {
		   g_index = 0;
		   return;
		   }

	    break; /* if time after threshold, stop reconstruction */

	case C_END:         /* end information   */

	    (void) fprintf (stderr, MSGSTR(M_UNEXP_END,"ghost: Warning: There is an unexpected END token in the input file.\n"));
	    break; /* stop reconstruction */

	case C_COMMENT:     /* user comment      */

	    (void) fgetline (infp, line, SIZE);
	    (void) fprintf (outfp, "%s\n", line);
	    return;

	case C_ARRAY:       /* start of record array */

	    break; /* stop reconstruction */

	default:

	    (void) fprintf (stderr, MSGSTR(M_ILL_COOKIE,"ghost: Warning: Token type %d in the input file is not valid.\n"), line [1]);
	}

    /* if we broke out of switch, stop reconstruction */
    (void) fseek (outfp, fileloc, 0); /* rewind output file */
    g_stopflag = TRUE;
    return;
    }

/****************************************************************************/
/* record:  handles record objects in delta file                            */
/****************************************************************************/

static void record (FILE *infp, FILE *outfp)
    {
    char *object;              /* record object            */
    extern void noop (void);

    g_allocfcn = noop; /* turn of fatal handling of allocation failures */
    g_typefcn = noop;

    object = s_read (infp); /* try to read record */

    g_allocfcn = NULL; /* restore fatal handling of allocation failures */
    g_typefcn = NULL;

    if ((object == (char *) ERROR) || (object == NULL))
	{
	(void) fprintf (stderr,MSGSTR(M_S_READ,"ghost: Warning: Cannot read the input file.\n"));
	g_stopflag = TRUE;
	return;
	}

    /* expand record array if necessary */
    if (g_index >= obj_count (g_records))
	g_records = (RECORD *) s_realloc ((char *)g_records, g_index + 1);

    g_records [g_index].r__fileloc = ftell (outfp) + 1; /* save file loc */
    if (obj_type (object) != T_CHAR) /* update RC_NONTEXT flag */
	obj_setflag (g_records, RC_NONTEXT);

    (void)s_write (object, outfp); /* echo record object */
    s_free (object);
    g_index++; /* update current index */
    }

/****************************************************************************/
/* fmove:  moves a file (by copying the bytes if necessary)                 */
/****************************************************************************/

static void fmove (char *oldname, char *newname)
    {
    int ifd, ofd, nbytes;
    char buf[BUFSIZ];

    /* first try renaming the file.  This will fail if oldname */
    /* and newname are on different file systems               */

    if (rename (oldname, newname) != ERROR)
	return;

    ifd = open (oldname, O_RDONLY);

    if (ifd == ERROR)
	(void)fatal (MSGSTR(M_COPY,"ghost: 0611-435 Cannot open %s.") , oldname);

    ofd = open (newname, O_WRONLY | O_CREAT);

    if (ofd == ERROR)
	(void)fatal ( MSGSTR(M_CREATE,"ghost 0611-436 Cannot create file %s."), newname);

    nbytes = BUFSIZ;
    while (nbytes > 0)
	{
	nbytes = read (ifd, buf, sizeof (buf));

	if (nbytes == -1)
	    (void)fatal ( MSGSTR(M_IO_READ,"ghost: 0611-437 Cannot read from file %s."),oldname);

	if (write (ofd, buf, nbytes) != nbytes)
	    (void)fatal ( MSGSTR(M_WRITE_ERR,"ghost: 0611-438 Cannot write to file %s."), newname);
	}

    (void)close (ifd);
    if (close (ofd) == ERROR)
	(void)fatal ( MSGSTR(M_IO_CLOSE,"ghost: 0611-439 Cannot close file %s."), newname);

    (void)unlink (oldname);
    }

/****************************************************************************/
/* TDwday:  determine day of week (Sunday 0 - Saturday 6)                   */
/*          given mday(1-31), mon(0-11), and year(0-99) of current century  */
/*                                                                          */
/* Valid for dates between Mar 1 1901 and Dec 31 1999 inclusive.            */
/* This duplicates the functionality of the TDwday procedure in libTD in a  */
/* simpler way, because we don't need the full power of libTD here.         */
/* If ghost is ever enhanced to use libTD anyway, this can be discarded.    */
/****************************************************************************/

int TDwday (int mday, int mon, int year)
    {
    int dc;     /* day count since Jan 1 of year known to start on Sunday */
    int i;

    static int mcount[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    dc = 0;

    /* Jan 1 1984 was a Sunday.  If the year is after 1983, compute dc as
       days since Jan 1 1984; for earlier years, compute since Jan 1 1900,
       which really began on a Monday but was not a leap year.  Thus this
       code is incorrect for January and February 1900, but who cares? */

    for (i = (year > 83) ? 84 : 0; i < year; i++)
	dc += (i % 4) ? 365 : 366;

    /* Add count for intervening months */

    for (i = 0; i < mon; i++)
	dc += mcount [i];

    /* If it is after February 29 in a leap year, add an extra day */

    if ((year % 4 == 0) && (mon > 1))
	dc++;

    /* Add the elapsed days in this month, less one to get range 0-30 */

    dc += mday - 1;

    /* The day count modulo 7 is day of week where Sun = 0 and Sat = 6 */
    return (dc % 7);
    }
