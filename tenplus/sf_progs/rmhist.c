static char sccsid[] = "@(#)17	1.9  src/tenplus/sf_progs/rmhist.c, tenplus, tenplus411, GOLD410 3/3/94 15:18:55";
/*
 * COMPONENT_NAME: (TENPLUS) INed Editor
 *
 * FUNCTIONS:	main, interrupt, rmhist, warn, maketmpname, debug,
 *		compress, cookie, record, doprehistory, backup
 *
 * ORIGINS:  9, 10, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/****************************************************************************/
/* file:  rmhist.c                                                          */
/*                                                                          */
/* program to remove history from a delta file                              */
/****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/access.h>
#include <time.h>
#include <locale.h>
#include <limits.h>

/* Ined Header Files */ 
#include <chardefs.h>
#include <database.h>
#include "rmhist_msg.h"
#include <libobj.h>

#define TMPNAME "rmhist.tmp"
#define SECS_IN_DAY 86400
#define SIZE (256 * MB_LEN_MAX) /* input line size */

/***** globals *****/

int      g_index;         /* current record index                 */
RECORD  *g_records;       /* record array                         */
int      g_stopflag;      /* TRUE if time to stop                 */
int      g_suppress;      /* TRUE if output suppressed            */
long     g_time;          /* time threshold                       */
long     g_now;           /* current time threshold               */
char     g_tmpname [PATH_MAX];/* temporary file name */
int      g_silent = FALSE;/* complain about non-delta files? */

#ifdef DEBUG
static void debug (char *, ...);
#endif

void interrupt (int );
int rmhist (char *, char *);
void warn (char *, ...); 
void maketmpname (char *, char *);
static int cookie (FILE *, FILE *);
int record (FILE *,FILE *);
int compress (char *, char *);
void doprehistory (FILE *, FILE *);
static char *backup (char *);

int MultibyteCodeSet;

/* Catalog descriptor */
nl_catd g_catd;

/* macro for general message catalog */
#define MSGSTR(num,str)	catgets(g_catd, MS_RMHIST, num, str)

/****************************************************************************/
/* main:  parses args and calls rmhist                                      */
/****************************************************************************/

int main (int argc, char **argv)
    {
    register char *filename; /* file name argument  */
    register i;              /* argv index          */
    register user;           /* file user id        */
    register group;          /* file group id       */
    register keepbak = TRUE; /* keep backup file?   */
    register char *bakfile;  /* name of backup file */
    char *acl;		     /* file access control list */
    int (*function)(char *, char *);
                             /* function to call    */
    struct stat buffer;      /* stat buffer         */
    char *opt_arg;           /* option argument     */
    int opt_ind;             /* option index        */

    /* set the local environment */
    (void) setlocale(LC_ALL, "");
    if (MB_CUR_MAX == 1)
       MultibyteCodeSet = 0;
    else 
       MultibyteCodeSet = 1;

    function = rmhist;
    g_catd = catopen (MF_RMHIST, NL_CAT_LOCALE);

    /* parse arguments */
    for (opt_ind = 1; (opt_ind < argc && argv[opt_ind][0] == '-'); opt_ind++)
	{
	for (opt_arg = &argv[opt_ind][1]; *opt_arg != '\0'; opt_arg++)
	    {
	    switch (*opt_arg)
		{
		case 'f':
		    g_silent = TRUE;
		    break;

		case 'd':
		    keepbak = FALSE;
		    break;

		case 'k':
		    (void) time (&g_time);
		    opt_arg++;

		    if (isdigit (*opt_arg)) /* if number specified */
			{
			g_now = g_time;
			g_time -= atoi (opt_arg) * SECS_IN_DAY;
			function = compress;
			opt_arg += strlen (opt_arg) - 1;
			break;
			}
		    (void) fprintf (stderr, MSGSTR(M_MINUS_K,"rmhist: 0611-476 Specify a positive integer for the -k flag.\n"));

		case '?':
		    fatal (MSGSTR(M_USAGE,"Usage: rmhist [-f] [-d] [-kNumber] File...\n"));
		    break;

		default:
		    (void) fprintf (stderr, MSGSTR(M_ILL_ARG,"%1$s: 0611-478 The %2$c flag is not recognized.\n"), argv[0],
			     *opt_arg);
		    fatal (MSGSTR(M_USAGE,"Usage: rmhist [-f] [-d] [-kNumber] File...\n"));
		    break;
		}
	    }
	}

    g_setflag (G_COREDUMP);

    for (i = opt_ind; i < argc; i++)
	{
	filename = argv [i];
	maketmpname (filename, g_tmpname);

	if (stat (filename, &buffer) == ERROR)
	    {
	    warn (MSGSTR(M_STAT,"0611-479 Cannot access file %s."), filename);
	    continue;
	    }
	if (! isdelta (filename))
	    {
	    warn (MSGSTR(M_FILE_TYPE,"File %s does not contain update records."), filename);
	    continue;
	    }
	user  = buffer.st_uid;
	group = buffer.st_gid;
	acl = (char *)acl_get (filename);

	(void) signal (SIGINT, interrupt);
	(void) signal (SIGHUP, interrupt);
	(void) signal (SIGQUIT, interrupt);

        if ((*function) (filename, g_tmpname) != ERROR)
	    {
	    (void) signal (SIGINT, SIG_IGN);
	    (void) signal (SIGHUP, SIG_IGN);
	    (void) signal (SIGQUIT, SIG_IGN);

	    if ((bakfile = backup (filename)) == (char *) ERROR)
		{
		warn (MSGSTR(M_BACKUP,"0611-480 Cannot create a backup file for %s."), filename);
		continue;
		}
	    if (rename (g_tmpname, filename) == ERROR)
		{
		warn (MSGSTR(M_RENAME,"0611-481 Cannot change file %1$s to %2$s."), g_tmpname, filename);
		s_free (bakfile);
		continue;
		}
	    if (! keepbak)
		{
		if ((chmod (bakfile, 0600) == ERROR) || (unlink (bakfile) == ERROR))
		    {
		    warn (MSGSTR(M_REMOVE,"0611-482 Cannot remove file %s."), bakfile);
		    s_free (bakfile);
		    continue;
		    }
		}
	    if (stat (filename, &buffer) == ERROR)
		warn (MSGSTR(M_STAT,"0611-479 Cannot access file %s."), filename);
	    else
		{
		if (acl_put (filename, acl, TRUE) != 0)
		    warn (MSGSTR(M_CHACL,"Internal Error (acl_put ('%s') failed)."), filename);

		if ((user != buffer.st_uid || group != buffer.st_gid))
		    if (chown (filename, user, group) == ERROR)
			warn (MSGSTR(M_CHOWN,"0611-483 Cannot change file %1$s\n\
			\tto owner number %2$d and group number %3$d."), filename, user, group);
		}

	    s_free (bakfile);
	    }
	else
	    {
	    (void)unlink (g_tmpname);
	    continue;
	    }
	}
    catclose(g_catd);
    exit (0);
    return(0); /* return put here to make lint happy */
    }

/****************************************************************************/
/* interrupt: handle miscellaneous signals, set g_breakflag                 */
/****************************************************************************/

/*ARGSUSED*/
void interrupt (int number)
    {
#ifdef DEBUG
    debug ("interrupt:  signal = %d", number);
#endif

    (void)unlink (g_tmpname);
    exit (1);
    }

/****************************************************************************/
/* rmhist:  removes all the history from a delta file                       */
/* Returns ERROR if file is not fixed up                                    */
/****************************************************************************/

int rmhist (char *oldname, char *newname)
    /* oldname:                 input file name     */
    /* newname:                 output file name    */
    {
    register indx;          /* record index        */
    register SFILE *insfp;   /* input sfile object  */
    register SFILE *outsfp;  /* output sfile object */
    register char *rec;   /* record being xfered */
    register count;          /* number of records   */


#ifdef DEBUG
    debug ("rmhist:  oldname = '%s', newname = '%s'", oldname, newname);
#endif

    insfp = sf_open (oldname, "r");

    if (insfp == (SFILE *) ERROR)
	{
	warn (MSGSTR(M_OPEN,"0611-484 Cannot open file %s."), oldname);
	return (ERROR);
	}

    outsfp = sf_open (newname, "c"); /* open new delta file */

    if (outsfp == (SFILE *) ERROR)
	{
	(void) sf_close (insfp);
	warn (MSGSTR(M_CREATE,"0611-485 Cannot create file %s."), newname);
	return (ERROR);
	}

    if (! g_silent)
	(void) fprintf (stderr, MSGSTR(M_RMHIST,"rmhist: Removing history from file %s.\n"), oldname);

    count = obj_count (sf_records (insfp));

    for (indx = 0; indx < count; indx++)
	{
	rec= sf_read (insfp, indx);

	if (rec == (char *) ERROR)
	    {
	    warn (MSGSTR(M_BAD_SF_READ,"Internal Error (sf_read failure, record %d, file %s)."), indx, oldname);

    error:
	    (void) sf_close (insfp);
	    (void) sf_close (outsfp);
	    return (ERROR);
	    }
	if (rec == NULL)
	    continue;

	if (sf_write (outsfp, indx, rec) == ERROR)
	    {
	    warn (MSGSTR(M_SF_WRITE,"Internal Error (sf_write failure (%d) on record %d,\n\
	    \tfile %s)"),
		   g_errno, indx, newname);
	    s_free (rec);
	    goto error;
	    }

	s_free (rec);
	}


    (void) sf_close (insfp);

    if (sf_close (outsfp) == ERROR)
	{
	warn (MSGSTR(M_SF_CLOSE,"Internal Error (sf_close failure (%d) on file '%s')"), g_errno, oldname);
	return (ERROR);
	}

    return (RET_OK);
    }

/****************************************************************************/
/* warn:  prints warning on stderr                                          */
/****************************************************************************/

void warn (char *format, ...)
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;
#ifdef DEBUG
    debug ("warn:  format = '%s'", format);
#endif

    if (g_silent)
	return;
    (void) fprintf (stderr, MSGSTR(M_WARNING,"rmhist: Warning: "));

    va_start(ap, format);
    arg1 = va_arg(ap, char *);
    arg2 = va_arg(ap, char *);
    arg3 = va_arg(ap, char *);
    arg4 = va_arg(ap, char *);
    arg5 = va_arg(ap, char *);
    (void) fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
    va_end(ap);   

    (void) fputc ('\n', stderr);
    }

/****************************************************************************/
/* maketmpname:  makes a temp file name in the same directory as filename   */
/* Takes buffer to put name into.                                           */
/****************************************************************************/

void maketmpname (char *filename, char *tmpname)
    {
    register char *cp;

    (void) strcpy (tmpname, filename);

    cp = strrchr (tmpname, DIR_SEPARATOR);

    if (cp == NULL)
	cp = tmpname;
    else
	cp++; /* goto first char after the slash */

    (void) strcpy (cp, TMPNAME);

#ifdef DEBUG
    debug ("maketmpname:  filename = '%s', tmpname = '%s'", filename, tmpname);
#endif
    }

/****************************************************************************/
/* debug:  debugging printer                                                */
/****************************************************************************/

#ifdef DEBUG

static void debug (char *format, ...)
    {
    va_list ap;
    char *arg1; char *arg2; char *arg3; char *arg4; char *arg5;

    va_start(ap, format);
    if (g_debugfp != NULL) 
	{
        arg1 = va_arg(ap, char *);
        arg2 = va_arg(ap, char *);
        arg3 = va_arg(ap, char *);
        arg4 = va_arg(ap, char *);
        arg5 = va_arg(ap, char *);

	(void) fprintf (g_debugfp, format, arg1, arg2, arg3, arg4, arg5);
	(void) fputc ('\n', g_debugfp);
        }
    va_end(ap);
    }

#endif

/****************************************************************************/
/* compress: removes history from a delta file up to the time specified in  */
/* g_time. Reads through the original delta-file, generating a record-array */
/* but not copying anything until a start-cookie at or after g_time is      */
/* encountered. Then, copy all active records (the prehistory) to the       */
/* output file, relocating them. Process all remaining deltas, and write    */
/* out the final record array.                                              */
/*                                                                          */
/* If g_time < time (first start cookie), this is equivalent to ghost. If   */
/* g_time > time (last start cookie), this is equivalent to rmhist.         */
/*                                                                          */
/* In case the file was ever opened when the system clock was accidentally  */
/* set in the future, also refer to g_now and begin copying records only    */
/* when a start-cookie is between g_time and g_now.                         */
/****************************************************************************/

int compress (char *oldname, char *newname)
    /* oldname:                old delta file name      */
    /* newname:                new delta file name      */
    {
    register FILE *infp;    /* input file pointer       */
    register FILE *outfp;   /* output file pointer      */
    int c;                 /* input test character     */
    long arrayloc;          /* file location of array   */
    char date_buffer[100];

#ifdef DEBUG
    debug ("compress:  oldname = '%s', newname = '%s'", oldname, newname);
#endif

    if ((infp = fopen (oldname, "r")) == NULL)
	{
	warn (MSGSTR(M_OPEN_INPUT,"0611-486 Cannot open input file %s."), oldname);
	return (ERROR);
	}
    if ((outfp = fopen (newname, "w")) == NULL)
	{
	(void)fclose (infp);
	warn (MSGSTR(M_CREATE,"0611-485 Cannot create file %s."), newname);
	return (ERROR);
	}

    strftime(date_buffer, 100, "%c", localtime(&g_time));
    if (! g_silent)
	(void) fprintf (stderr, MSGSTR(M_REMOVING,"rmhist: Removing history before %2$.24s from file %1$s.\n"),oldname,date_buffer);
    g_records = (RECORD *) s_alloc (0, T_RECORD, NULL);
    g_index = 0;
    g_stopflag = FALSE;
    g_suppress = TRUE;


    for (;;)
	{
	c = getc (infp);

	if (feof (infp))
	    break;

	if ((c & 0xff) == COOKIE) /* if it looks like a cookie */
	    {
	    if (cookie (infp, outfp) == ERROR)
		{
		warn (MSGSTR(M_UNEXP_COOKIE,"Internal Error (bad history cookie in file)."));
	error:
		(void)fclose (outfp);
		(void)fclose (infp);
		s_free ((char *)g_records);
		g_records = NULL;
		return (ERROR);
		}
	    }
	else
	    {
	    (void) ungetc (c, infp);     /* put back first char   */
	    if (record (infp, outfp) == ERROR)  /* treat input as record */
		goto error;
	    }
	if (ferror (outfp))
	    {
	    warn (MSGSTR(M_IO_ERR_1,"Internal Error (I/O error (1) on output file)"));
	    goto error;
	    }
	if (g_stopflag) /* if time to stop discarding history */
	    {
	    if (g_suppress)
		{
		doprehistory (infp, outfp);
		g_stopflag = FALSE;
		g_suppress = FALSE;
		}
	    else
		break;
	    }
	}
#ifdef DEBUG
    debug ("compress:  obj_count = %d", obj_count (g_records));
#endif

    (void) putc (COOKIE, outfp);     /* write out "array" cookie */
    (void) putc (C_ARRAY, outfp);


    arrayloc = (long)ftell (outfp);   /* save array location */
    (void)s_write ((char *)g_records, outfp); /* write array to file */


    (void) putc (COOKIE, outfp);       /* write out "end" cookie */
    (void) putc (C_END, outfp);
    putl (arrayloc, outfp);

    if (ferror (outfp)) /* make sure that worked */
	{
	warn (MSGSTR(M_IO_ERR_2,"Internal Error (I/O error (2) on output file)"));
	goto error;
	}
    if (fclose (outfp) == ERROR)
	{
	warn (MSGSTR(M_IO_ERR_3,"Internal Error (I/O error (3) on output file)"));
	goto error;
	}
    (void)fclose (infp);
    s_free ((char *)g_records);
    g_records = NULL;
    return (RET_OK);
    }

/****************************************************************************/
/* cookie:  handles cookie lines in the delta file                          */
/* The COOKIE character has already been read in when this routine is call- */
/* ed.  Sets g_stopflag if reconstruction should be stopped at this point.  */
/****************************************************************************/

static int cookie (FILE *infp, FILE *outfp)
    /* infp:                    input file pointer       */
    /* outfp:                   output file pointer      */
    {
    register value;         /* integer paramter value    */
    register type;          /* cookie type               */
    register tmp;           /* used for echoing uid, gid */
    long daytime;           /* time in start cookie      */
    long fileloc;           /* output file location      */
    long inloc;             /* input file location       */
    char line [SIZE];       /* line buffer for comment   */


    fileloc = ftell (outfp);  /* remember output file location */
    inloc = ftell (infp) - 1; /* location of COOKIE character  */

    if (! g_suppress)
	(void) putc (COOKIE, outfp);    /* echo cookie char */

    type = getc (infp);      /* get cookie type  */

#ifdef DEBUG
    debug ("cookie:  type = %d, g_suppress = %d", type, g_suppress);
    debug ("  inloc = %ld, fileloc = %ld", inloc, fileloc);
#endif

    if (! g_suppress)
	(void) putc (type, outfp);      /* echo type char   */

    switch (type) /* check cookie type */
	{
	case C_INSERT:      /* insert lines      */

	    value = geti (infp);

	    if (! g_suppress)
		puti (value, outfp);

	    g_records = (RECORD *) s_insert ((char *)g_records, 
                                             g_index, value);
	    return (RET_OK);

	case C_DELETE:      /* delete lines or a child */

	    value = geti (infp);

	    if (! g_suppress)
		puti (value, outfp);

	    g_records = (RECORD *) s_delete ((char *)g_records, 
                                             g_index, value);
	    return (RET_OK);

	case C_INDEX:       /* set current index */

	    value = geti (infp);

	    if (! g_suppress)
		puti (value, outfp);

	    g_index = value;
	    return (RET_OK);

	case C_START:       /* start information */

	    tmp = getc (infp);      /* read and echo uid */

	    if (! g_suppress)
		(void) putc (tmp, outfp);

	    tmp = getc (infp);      /* read and echo gid */

	    if (! g_suppress)
		(void) putc (tmp, outfp);

	    daytime = getl (infp);     /* get start time    */

	    if (! g_suppress)
		putl (daytime, outfp);     /* and echo it       */

#ifdef DEBUG
	    debug ("start cookie, time = %.24s", ctime (&daytime));
#endif

	    /* if time before threshold, continue */

	    if ((! g_suppress) || (daytime < g_time) || (daytime > g_now))
		{
		g_index = 0;
		return (RET_OK);
		}
	    (void) fseek (infp, inloc, 0); /* rewind input file */
	    g_stopflag = TRUE;
	    return (RET_OK);

	case C_END:         /* end information   */

	    warn (MSGSTR(M_END_COOKIE,"Internal Error (unexpected END cookie)."));
	    return (ERROR);

	case C_COMMENT:     /* user comment      */

	    (void)fgetline (infp, line, SIZE);

	    if (! g_suppress)
		{
		(void) fputs (line, outfp);
		(void) putc('\n', outfp);
		}

	    return (RET_OK);

	case C_ARRAY:       /* start of record array */

	    /* if we run into an ARRAY cookie with g_suppress on,
	       everything in the file is "prehistory", so output it
	       and quit. [note that the "rewind the outputfile"
	       does the wrong thing before doprehistory
	       has been called.                                         */

	    if (g_suppress)
		{
		doprehistory (infp, outfp);
		g_suppress = FALSE;
		}
	    else
		(void) fseek (outfp, fileloc, 0); /* rewind output file */

	    g_stopflag = TRUE;
	    return (RET_OK);


	default:
	    return (ERROR);
	}
    }

/****************************************************************************/
/* record:  handles record objects in delta file                            */
/****************************************************************************/

int record (FILE *infp, FILE *outfp)
    /* infp:                   input file pointer       */
    /* outfp:                  output file pointer      */
    {
    register char *object;  /* record object            */
    long place;             /* input file location      */
    extern void noop (void);

#ifdef DEBUG
    debug ("record:  g_index = %d", g_index);
#endif

    /***** if no output written yet, remember the input file location   */
    /***** else remember the output file location (where write will be) */

    place = ftell (g_suppress ? infp : outfp);

    g_allocfcn = noop; /* turn off fatal handling of allocation failures */
    g_typefcn = noop;

    object = s_read (infp); /* try to read record */

    g_allocfcn = NULL; /* restore fatal handling of allocation failures */
    g_typefcn = NULL;

    if (object == (char *) ERROR)
	{
	warn (MSGSTR(M_READ,"Internal Error (record read error)."));
	return (ERROR);
	}

    /* expand record array if necessary */

    if (g_index >= obj_count (g_records))
	g_records = (RECORD *) s_realloc ((char *)g_records, 
                                          g_index + 1);

    g_records [g_index].r__fileloc = place + 1;

    if (object && obj_type (object) != T_CHAR) /* update RC_NONTEXT flag */
	obj_setflag (g_records, RC_NONTEXT);

    if (! g_suppress)
	(void)s_write (object, outfp); /* echo record object */

    s_free (object);
    g_index++; /* update current index */
    return (~ERROR); /* return a value that is unequal to ERROR */
    }

/****************************************************************************/
/* doprehistory: dumps any prehistory we've found onto outfp                */
/* Updates the record array locations for the file                          */
/****************************************************************************/

void doprehistory (FILE *infp, FILE *outfp)
    {
    register i;     /* record index        */
    char *object;   /* record data         */
    long place;     /* record location     */
    long resume;    /* input file location */

#ifdef DEBUG
    debug ("doprehistory called");
#endif

    if (obj_count (g_records) == 0) /* if no prehistory, nothing to do */
	return;

    /**** xfer records fixing record array as we go *****/

    resume = ftell (infp);

    /***** put out a start cookie which corresponds to g_time *****/

    (void) fputc (COOKIE, outfp);  /* put out cookie */
    (void) fputc (C_START, outfp);
    (void) fputc (getuid (), outfp); /* output uids */
    (void) fputc (getgid (), outfp); /* output gids */
    putl (g_time, outfp);     /* output time */

    for (i = 0; i < obj_count (g_records); i++)
	{
	place = r_fileloc (&g_records [i]);

	/* deal correctly with NULL records */

	if (place != NORECORD)
	    {
	    (void) fseek (infp, place - 1, 0);
	    object = s_read (infp);

	    if (object == (char *) ERROR)
		{
		warn (MSGSTR(M_S_READ,"Internal Error (s_read failure (%d) on record %d)"), g_errno, i);
		return;
		}
	    g_records [i].r__fileloc = ftell (outfp) + 1;
	    (void)s_write (object, outfp);
	    s_free (object);
	    }
	}
    (void) fseek (infp, resume, 0);
    }


static char *backup (char *filename)
    {
    register char *cp;
    char bakname[PATH_MAX];    /*variable holding name of backup file */
    /* if backup name has format <path>/<basename>.bak, then
       max_bak_basename holds maximum number of bytes in <basename> */
    long max_bak_basename;

    /* no initialisation at declaration, because pathconf must
       get existing file
    */
    max_bak_basename = pathconf(filename, _PC_NAME_MAX) - 4;

    (void) strcpy(bakname, filename);

    /* let cp point to the basename */
    cp = strrchr(bakname, DIR_SEPARATOR);
    if (cp != NULL)
         cp++; /* DIR_SEPARATOR has length 1 */
    else cp = bakname;

    /* advance cp to either the end of the basename (if basename
       smaller than max_bak_basename) or to the character after
       the 'max_bak_basename'th character.
    */
    if (MultibyteCodeSet) {
	int mlen;
        char *start_basename = cp;
        /* after one go through the loop, cp2 points at the char
           in front of *cp. */
        char *cp2 = cp;
        while ((cp2 != '\0') &&
               ((cp2 - start_basename) <= max_bak_basename)) {
           cp = cp2;
           mlen = mblen(cp2, MB_CUR_MAX);
	   /* copy at least 1 char */
	   if (mlen < 1) mlen = 1;
           cp2 += mlen;
        }
        if (cp2 == '\0')
           cp = cp2;
     }
     else cp = cp + min(strlen(cp), max_bak_basename);

     /* cut off rest of the backup name */
     *cp = '\0';

     /* add .bak to name */
     (void) strcat(bakname, ".bak");   

    /* rename file to backup file and return */
    if (rename (filename, bakname) == ERROR)
	return ((char *) ERROR);
    return (s_string(bakname));
    }
