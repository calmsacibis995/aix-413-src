static char sccsid[] = "@(#)08	1.7  src/tenplus/sf_progs/fixascii.c, tenplus, tenplus411, GOLD410 11/4/93 12:11:44";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, fixascii, cookie, record, makerecord, saveascii,
 *		readfake, printrec
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
/* file:  fixascii.c                                                        */
/*                                                                          */
/* program to reconstruct previous versions of delta files                  */
/****************************************************************************/

/* System Includes */
#include    <string.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <sys/access.h>
#include    <stdlib.h>
#include    <unistd.h>
#include    <locale.h>
#include    <limits.h>

/* Ined Headers Files */
#include    <database.h>
#include    <libobj.h>
#include    "fixascii_msg.h"

#define TEXTLOC 0x80000000L /* used to make file locations as in the text file */
#define SIZE (256 * MB_LEN_MAX)

    /***** global variables *****/

RECORD  *g_records;     /* record array         */
int      g_index;       /* current record index */
int      g_stopflag;    /* TRUE if time to stop */
long     g_lastloc;     /* last file location   */

int MultibyteCodeSet;

/* Catalog descriptor */
nl_catd g_catd;

/* Macro for general message catalog access */
#define MSGSTR(num,str)		catgets(g_catd,MS_FIXASCII,num,str)

/* Functions local to this dir */
static void fixascii (char *, char *);
static void cookie (FILE *, FILE *);
static void record (FILE *, FILE *);
static RECORD *makerecord (char *);
static void saveascii (char *, char *);
static char *readfake (SFILE *, FILE *, int );
#ifdef DEBUG
static void printrec (RECORD *);
#endif

/****************************************************************************/
/* main:  top level routine.  Parses args and calls fixascii                */
/****************************************************************************/

main (int argc, char **argv)
    {
    register saveflag;    /* TRUE if file should be saved */
    register char *ascii; /* ASCII file name              */
    register char *delta; /* delta (dots) file name       */
    extern FILE *g_debugfp;
    char * msg_ptr;

    /* set the local environment */
    (void) setlocale(LC_ALL, "");

    if (MB_CUR_MAX == 1)
         MultibyteCodeSet = 0;
    else MultibyteCodeSet = 1;
    
    g_catd = catopen(MF_FIXASCII, NL_CAT_LOCALE);

    saveflag = TRUE;

    if ((argc > 1) && ((strcmp(argv [1], "-ns"))==0))
	{
	saveflag = FALSE;
	argc--;
	argv++;
	}
    if (argc != 3)
	fatal (MSGSTR(M_USAGE,""));

#ifdef DEBUG
    g_debugfp = fopen ("fixascii.out", "w");

    if (g_debugfp == NULL)
	{
	g_debugfp = stderr;
	fatal ("fixascii:  cannot open debugging output file");
	}
#endif

    ascii = argv [1];
    delta = argv [2];

    if (saveflag) /* if not called from the editor */
	debug (MSGSTR(M_FIXING,"Fixing delta file %s"), delta);

    fixascii (ascii, delta);

    if (saveflag)
	{
		debug (MSGSTR(M_SAVING,"Saving ASCII file %s."), ascii);
		saveascii (ascii, delta);
		}
	    catclose(g_catd);
	    exit (0);
	    return(0); /* return put here to make lint happy */
	    }

	/****************************************************************************/
	/* fixascii:  reconstructs the dots file for an ASCII file                  */
	/****************************************************************************/

	static void fixascii (char *ascii, char *delta)
	    /* ascii:    ascii file name          */
	    /* delta:    delta file name          */
	    {
	    register char *tmpname; /* temporary file name      */
	    register FILE *infp;    /* input file pointer       */
	    register FILE *outfp;   /* output file pointer      */
	    int c;                 /* input test character     */
	    long arrayloc;          /* file location of array   */
	    extern RECORD *makerecord (char *);

	#ifdef DEBUG
	    debug ("fixascii:  ascii = '%s', delta = '%s'\n",
		   ascii, delta);
	#endif

	    /***** first make a record array from the ascii file *****/

	    g_records = makerecord (ascii);

	#ifdef DEBUG
	    debug ("s_print after makerecord (g_records) (0%o):\n\n",
		   g_records);
	    printrec (g_records);
	#endif

	    tmpname = "fixascii.tmp";

	    if ((infp = fopen (delta, "r")) == NULL)
		fatal (MSGSTR(M_OPEN_NEW,"fixascii: 0611-152 Cannot open delta file %s."), delta);

    if ((outfp = fopen (tmpname, "w")) == NULL)
	fatal (MSGSTR(M_OPEN_TMP,"fixascii: 0611-153 Cannot open temporary delta file %s."), tmpname);

    /* read through the input delta file, transfering the lines to */
    /* the output file and recreate the record array.              */

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
	    fatal (MSGSTR(M_IO_WRITE,"fixascii: 0611-154 Cannot write to temporary file %s.\n\
	    \tThe disk may be full."), tmpname);

	if (g_stopflag) /* if time to stop */
	    break;
	}
#ifdef DEBUG
    debug ("s_print (g_records) (0%o):\n\n", g_records);
    printrec (g_records);
#endif

    (void) putc (COOKIE, outfp);     /* write out "array" cookie */
    (void) putc (C_ARRAY, outfp);

    arrayloc = ftell (outfp);   /* save array location */
    (void)s_write ((char *)g_records, outfp); /* write array to file */

    (void) putc (COOKIE, outfp);     /* write out "end" cookie */
    (void) putc (C_END, outfp);
    putl (arrayloc, outfp);

    if (ferror (outfp)) /* make sure that worked */
	fatal (MSGSTR(M_WRITE,"fixascii: 0611-155 Cannot write. The disk may be full."));

    if (fclose (outfp) == ERROR)
	fatal (MSGSTR(M_SF_CLOSE,"fixascii: 0611-156 Cannot run sf_close. The disk may be full."));

    if (fclose (infp) == ERROR)
	fatal (MSGSTR(M_FCLOSE,"fixascii: 0611-157 Cannot close the input file."));

    (void) rename (tmpname, delta);
    }

/****************************************************************************/
/* cookie:  handles cookie lines in the delta file                          */
/* The COOKIE character has already been read in when this routine is call- */
/* ed.  Sets g_stopflag if reconstruction should be stopped at this point.  */
/****************************************************************************/

static void cookie (FILE *infp, FILE *outfp)
    /* infp:     input file pointer       */
    /* outfp:    output file pointer      */
    {
    register value;         /* integer paramter value    */
    register type;          /* cookie type               */
    long daytime;           /* time in start cookie      */
    long fileloc;           /* output file location      */
    char line [SIZE];       /* line buffer for comment   */

#ifdef DEBUG
    debug ("cookie:  called \n");
#endif

    fileloc = ftell (outfp); /* remember output file location */
    (void) putc (COOKIE, outfp);    /* echo cookie char */
    type = getc (infp);      /* get cookie type  */
    (void) putc (type, outfp);      /* echo type char   */

    switch (type) /* check cookie type */
	{
	case C_INSERT:      /* insert lines      */

	    value = geti (infp);
	    if (value == EOF)
		return;
	    puti (value, outfp);
	    g_records = (RECORD *) s_insert ((char *)g_records, 
                                             g_index, value);

#ifdef DEBUG
	    debug ("cookie:  inserting %d records at %d\n",
		   value, g_index);
#endif
	    return;

	case C_DELETE:      /* delete lines or a child */

	    value = geti (infp);
	    if (value == EOF)
		return;
	    puti (value, outfp);
	    g_records = (RECORD *) s_delete ((char *)g_records, g_index, value);

#ifdef DEBUG
	    debug ("cookie:  deleting %d records at %d\n",
		   value, g_index);
#endif
	    return;

	case C_INDEX:       /* set current index */

	    value = geti (infp);
	    if (value == EOF)
		return;
	    puti (value, outfp);
	    g_index = value;

#ifdef DEBUG
	    debug ("cookie:  setting index to %d\n", g_index);
#endif
	    return;

	case C_START:       /* start information */

	    (void) putc (getc (infp), outfp); /* read and echo uid */
	    (void) putc (getc (infp), outfp); /* read and echo gid */
	    daytime = getl (infp);     /* get start time    */
	    putl (daytime, outfp);     /* and echo it       */
	    g_index = 0;

#ifdef DEBUG
	    debug ("cookie:  start cookie\n");
#endif
	    return;

	case C_END:         /* end information   */

#ifdef DEBUG
	    debug ("fixascii:  warning - unexpected END cookie\n");
#endif
	    break; /* stop reconstruction */

	case C_COMMENT:     /* user comment      */

	    (void) fgetline (infp, line, SIZE);
	    (void) fprintf (outfp, "%s\n", line);
	    return;

	case C_ARRAY:       /* start of record array */

#ifdef DEBUG
	    debug ("cookie:  array cookie\n");
#endif
	    break; /* stop reconstruction */

	default:

#ifdef DEBUG
	    debug ("fixascii:  warning - illegal cookie type (%d)\n",
		   line [1]);
#endif
	    break;
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
    /* infp:     input file pointer       */
    /* outfp:    output file pointer      */
    {
    register char *object;  /* record object            */
    extern void noop (void);

#ifdef DEBUG
    debug ("record:  called with g_index = %d\n", g_index);
#endif

    g_allocfcn = noop; /* turn of fatal handling of allocation failures */
    g_typefcn = noop;

    object = s_read (infp); /* try to read record */

    g_allocfcn = NULL; /* restore fatal handling of allocation failures */
    g_typefcn = NULL;

    if ((object == (char *) ERROR) || (object == NULL))
	{
#ifdef DEBUG
	if (object == NULL)
	    debug ("record:  NULL record\n");
	else
	    debug ("fixascii:  warning - s_read failure\n");
#endif
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
/* makerecord:  makes a fake record array from a text file                  */
/****************************************************************************/

static RECORD *makerecord (char *ascii)
    /* ascii                     tmp file name             */
    {
    register c;               /* input character           */
    register i;               /* line index                */
    register FILE *fp;        /* input fp                  */
    register RECORD *records; /* record array              */
    long fileloc;             /* file location             */
    char newname [30 * MB_LEN_MAX]; /* used to make inode string */
    struct stat buffer;       /* used to get inode         */

#ifdef DEBUG
    debug ("makerecord:  ascii = '%s'\n", ascii);
#endif

    fp = fopen (ascii, "r");

    if (fp == NULL)
	fatal (MSGSTR(M_OPEN_ASCII,"fixascii: 0611-158 Cannot open the ASCII file %s."), ascii);

    records = (RECORD *) s_alloc (500, T_RECORD, NULL);
    records [0].r__fileloc = TEXTLOC;
    i = 1;
    fileloc = 0L;

    for (;;) 
	{
	c = getc (fp);

	if (feof (fp))
	    break;

	fileloc++;

	if (c == '\n')
	    {
	    if (i >= obj_count (records))
		records = (RECORD *) s_realloc ((char *)records,
						obj_count (records) + 500);
	    records [i].r__fileloc = fileloc | TEXTLOC;
	    i++;
	    }
	}
    i--; /* decrement i to strip last (NULL) line */
    records = (RECORD *) s_realloc ((char *)records, i);
    (void)fclose (fp);

    if (stat (ascii, &buffer) == ERROR) /* if file doesn't exist */
	fatal (MSGSTR(M_M_STAT,"fixascii: 0611-159 Cannot get status on file %s."), ascii);

    (void) sprintf (newname, "%d.%ld", buffer.st_ino, buffer.st_mtime);
    s_newname ((char *)records, newname);

    return (records);
    }

/****************************************************************************/
/* saveascii:  converts a tmp file back to its ASCII file                   */
/****************************************************************************/

static void saveascii (char *filename, char *tmpname)
    /* filename:            ASCII file name       */
    /* tmpname:             name of tmp file      */
    {
    register i;               /* line index            */
    register SFILE *sfp;      /* tmp file sfp          */
    register RECORD *records; /* sfp record array      */
    register FILE *oldfp;     /* old ASCII file fp     */
    register FILE *newfp;     /* new ASCII file fp     */
    register char *line;      /* text of line          */
    char *acl;		      /* file access control list */
    long fileloc;             /* text file location    */
    struct stat buffer;       /* stat call buffer      */
    char newname [30 * MB_LEN_MAX];       /* used for making name  */

#ifdef DEBUG
    debug ("saveascii:  filename = '%s', tmpname = '%s'", filename,
	   tmpname); 
#endif

    sfp = sf_open (tmpname, "r");

    if (sfp == (SFILE *) ERROR)
	{
#ifdef DEBUG
	  debug ("  cannot open tmp file '%s'", tmpname);
#endif
	return;
	}
    records = sf_records (sfp);
    g_lastloc = (long)-1L; /* force initial seek in readfake */

    if (stat (filename, &buffer) == ERROR)
	fatal (MSGSTR(M_E_STAT,"fixascii: 0611-160 Cannot get status on file %s."), filename);

    acl = (char *)acl_get(filename);

    oldfp = fopen (filename, "r");

    if (oldfp == NULL)
	fatal (MSGSTR(M_E_READ,"fixascii: 0611-161 Cannot read file %s."), filename);

    (void) fbackup (filename);
    newfp = fopen (filename, "w");
    if (newfp == NULL)
	fatal (MSGSTR(M_E_CREATE,"fixascii: 0611-162 Cannot create file %s."), filename);
    
    fileloc = 0L;

    for (i = 0; i < obj_count (records); i++)
	{
	line = readfake (sfp, oldfp, i);
	records [i].r__fileloc = fileloc | TEXTLOC; /* save record location */

#ifdef DEBUG
	debug ("  i = %d, fileloc = %ld, line = '%s'", i, fileloc, line);
#endif
	if (line == (char *) ERROR)
	    fatal (MSGSTR(M_READ_IDX,"0611-163 There is a read error: index %1$d, file %2$s."),i,filename);
	else if (line)
	    {
	    (void) fprintf (newfp, "%s\n", line);
	    fileloc += obj_count (line) + 1;
	    }
	else
	    {
	    (void) fputc ('\n', newfp);
	    fileloc++;
	    }
	s_free (line);

	if (ferror (newfp))
	    fatal (MSGSTR(M_DISK_FL,"0611-164 Cannot write to text file %s. The disk may be full."), filename);
	}
    (void)fclose (oldfp);
    (void)fclose (newfp);
    (void)chmod (filename, buffer.st_mode);
    /* Set the access control list for the output file (this */
    /* overwrites the permission bits part of the st_mode) */
    acl_put(filename, acl, TRUE);

    /* this prevents freeing of records array */

    sfp->sf__records = (RECORD *) s_alloc (0, T_RECORD, NULL);
    (void) sf_close (sfp);
    (void)unlink (tmpname); /* this prevents a .bak file */
    sfp = sf_open (tmpname, "c"); /* make new record array file */

    if (sfp == (SFILE *) ERROR)
	fatal (MSGSTR(M_REOPEN,"fixascii: 0611-165 Cannot reopen file %s."), tmpname);

    s_free ((char *)sf_records (sfp));
    sfp->sf__records = records;

    /***** put inode in record array name *****/

    if (stat (filename, &buffer) == ERROR)
	fatal (MSGSTR(M_RESTAT,"fixascii: 0611-166 Cannot get status on file %s again."), filename);

    (void) sprintf (newname, "%d.%ld", buffer.st_ino, buffer.st_mtime);
    s_newname ((char *)records, newname);

    obj_setflag (sfp, SF_MODIFIED); /* force writing of record array */

    /*$$$$ this may want to use a threshold of lines *****/

    if (obj_count (sf_records (sfp)) == 0) /* don't save file if empty */
	(void)unlink (tmpname);

    (void) sf_close (sfp);
    }

/****************************************************************************/
/* readfake:  version of sf_read for fake files                             */
/* Returns an s_alloc'ed char array containing the text for a given index   */
/****************************************************************************/

static char *readfake (SFILE *sfp, FILE *fp, int index1)
    /* sfp:       tmp file sfp             */
    /* fp:        ASCII file fp            */
    {
    register RECORD *records;   /* records array            */
    register char *text;        /* text of line             */
    long fileloc;               /* file location of record  */

#ifdef DEBUG
    debug ("readfake:  index = %d", index1);
#endif

    records = sf_records (sfp);

    if ((index1 < 0) || (index1 >= obj_count (records)))
	{
	g_errno = E_MISC;
	return ((char *) ERROR);
	}
    fileloc = r_fileloc (&records [index1]); /* get file location of record */

#ifdef DEBUG
    debug ("readfake:  fileloc = 0x%lx, g_lastloc = 0x%lx", fileloc,
	   g_lastloc);
#endif

    if (fileloc == NORECORD) /* if no record, return NULL */
	return ((char *) NULL);

    if (fileloc & TEXTLOC) /* if located in the text file */
	{
	fileloc &= ~TEXTLOC; /* clear high bit */

	if (fileloc != g_lastloc)
	    {
	    g_lastloc = fileloc;

#ifdef DEBUG
	    debug ("readfake:  seeking to location %ld", fileloc);
#endif
	    if (fseek (fp, fileloc, 0) == ERROR)
		fatal (MSGSTR(M_SEEK,"fixascii: 0611-167 Cannot seek to location %D."), fileloc);
	    }
	text = s_getline (fp);

#ifdef DEBUG
	if (text != NULL)
	    debug ("readfake:  obj_count (text) = %d", obj_count (text));
#endif
	if (text == NULL)
	    {
	    g_errno = E_IO;
	    g_lastloc = -1L;
	    return ((char *) ERROR);
	    }
#ifdef DEBUG
	debug ("readfake:  text (old) = '%s'", text);
#endif
	g_lastloc += obj_count (text) + 1;
	return (text);
	}
    /***** the line is in the delta file *****/

    fileloc--; /* otherwise convert to file location */
    sf_seek (sfp, fileloc, 'r'); /* seek to read location */
    text = s_read (sf_fp (sfp)); /* read record */

#ifdef DEBUG
    debug ("readfake:  text (new) = '%s'", text);
#endif

    return (text);
    }

#ifdef DEBUG

/****************************************************************************/
/* printrec:  alternate version of r_print                                  */
/****************************************************************************/

static void printrec (RECORD *records)
    {
    register i;
    long fileloc;

    (void) s_typecheck (records, "printrec", T_RECORD);

    for (i = 0; i < obj_count (records); i++)
	{
	fileloc = r_fileloc (&records [i]);

	if (fileloc & TEXTLOC)
	    debug ("  line %d (old) at %ld\n", i, fileloc &~ TEXTLOC);
	else
	    debug ("  line %d (new) at %ld\n", i, fileloc);
	}
    }
#endif
