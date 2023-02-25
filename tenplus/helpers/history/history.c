#if !defined(lint)
static char sccsid[] = "@(#)78	1.9  src/tenplus/helpers/history/history.c, tenplus, tenplus411, 9435C411a 9/1/94 18:08:54";
#endif /* lint */
/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	H_addfield, H_cookie, H_formprefix, H_mkhistfile,
 *		H_mktmpnames, H_record, H_showframe, H_sigcatch, H_u1,
 *		H_u2, H_u3, H_u4, H_u5, H_uinit, H_com_init,
 *		H_usavestate, H_urestart, H_zoomin, H_zoomout, H_exit
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1994
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 *
 *                      Copyrighted as an unpublished work.                 
 *                         INTERACTIVE TEN/PLUS System                      
 *                          TEN/PLUS History Display                        
 *              (c) Copyright INTERACTIVE Systems Corp. 1983, 1988          
 *                             All rights reserved.                         
 *                                                                          
 *   RESTRICTED RIGHTS                                                      
 *   These programs are supplied under a license.  They may be used and/or  
 *   copied only as permitted under such license agreement.  Any copy must  
 *   contain the above notice and this restricted rights notice.  Use,      
 *   copying, and/or disclosure of the programs is strictly prohibited      
 *   unless otherwise provided in the license agreement.                    
 *                                                                          
 *   TEN/PLUS is a registered trademark of INTERACTIVE Systems Corporation  
 *	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
 */

/****************************************************************************/
/* file:  history.c                                                         */
/*                                                                          */
/* helper program to look at old versions of a file                         */
/****************************************************************************/

#if !defined(lint)
static char copyright[] =
	"@(#)(c) Copyright INTERACTIVE Systems Corp. 1983, 1988";
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <database.h>
#include <chardefs.h>
#include <signal.h>
#include <keynames.h>
#include <edexterns.h>
#include <libobj.h>
#include <libhelp.h>
#include "history.h"
#include "INhistory_msg.h"
#include <langinfo.h>

#define SIZE (256 * MB_LEN_MAX)  /* input line size */

    /***** global variables *****/

char    *Hg_realfile = NULL;   /* real file name                   */
char    *Hg_histfile = NULL;   /* pseudo file showing history info */
char    *Hg_showfile = NULL;   /* displayable version of file      */
char    *Hg_recfile = NULL;    /* record arrays file               */
SFILE   *Hg_recsfp = NULL;     /* sfile pointer for recfile        */
RECORD  *Hg_records = NULL;    /* record array                     */
int      Hg_index = 0;         /* current record index             */
long     Hg_lasttime = 0L;     /* last start cookie time           */
int      Hg_user = 0;          /* last start cookie user id        */
int      Hg_inserts = 0;       /* insert record count              */
int      Hg_deletes = 0;       /* delete record count              */
int      Hg_changes = 0;       /* changed record count             */
POINTER *Hg_times = NULL;      /* histfile times record            */
int      Hg_frame;             /* frame used t make showfile       */
int      Hg_numframes;         /* number of frames                 */
char    *Hg_form = NULL;       /* get form name to uncache         */
int      Hg_numzooms = 0;      /* number of levels of zooming      */

int       Hg_filemodes;        /* g_realfile file modes            */


    /*** indexes for globals in save state tree ***/

#define REALFILE  0
#define HISTFILE  1
#define SHOWFILE  2
#define RECFILE   3
#define FRAME     4
#define NUMFRAMES 5
#define FILEMODES 6

char *get_datefmt();
char *strip_date(char *, char *);

/****************************************************************************/
/* H_addfield:  adds new history field and writes record array to recfile   */
/****************************************************************************/

int H_addfield (void)
    {
    register POINTER *field;        /* field being added      */
    register char *subfield;        /* used to make subfields */
    char buffer [100 * MB_LEN_MAX];               /* sprintf buffer         */
    register struct passwd *pwp;    /* getpwuid pointer       */
    extern struct passwd *getpwuid (const unsigned long uid);
    static user = -1;               /* uid from last getpwuid */
    static char *uniqname1 = NULL;   /* user name              */
    char              final_fmt[100];
    char              final_date[100];

#ifdef DEBUG

    debug ("addfield:  g_user = %d, user = %d, g_time = %s",
	Hg_user, user, ctime (&Hg_lasttime));
    debug ("           g_inserts = %d, g_deletes = %d, g_changes = %d\n",
	Hg_inserts, Hg_deletes, Hg_changes);
    debug ("           g_records = 0%o\n", Hg_records);
    s_print (Hg_records);

#endif

    if (Hg_lasttime == 0L) /* don't add field for first start cookie */
	return (TRUE);

    if (Hg_user != user)
	{

	pwp = getpwuid (Hg_user); /* get user name */

	s_free (uniqname1);

	if (pwp == NULL) /* if can't get user name, make one up */
	    {
	    (void) sprintf (buffer, "uid %d", Hg_user);
	    uniqname1 = s_string (buffer);
	    }
	else
	    uniqname1 = s_string (pwp->pw_name);

	user = Hg_user;
	}
    field = (POINTER *) s_alloc (5, T_POINTER, NULL);

    subfield = s_string (uniqname1); /* make user subfield */
    s_newname (subfield, "user");
    field [0] = subfield;

    setlocale(LC_ALL, "");
    strcpy(final_fmt, get_datefmt());
    strftime(final_date, 100, final_fmt, localtime(&Hg_lasttime));
    (void) sprintf (buffer, "%24s", final_date); /* make time subfield */
    subfield = s_string (buffer);
    s_newname (subfield, "time");
    field [1] = subfield;

    (void) sprintf (buffer, "%4d", Hg_inserts); /* make ins subfield */
    subfield = s_string (buffer);
    s_newname (subfield, "ins");
    field [2] = subfield;

    (void) sprintf (buffer, "%4d", Hg_deletes); /* make del subfield */
    subfield = s_string (buffer);
    s_newname (subfield, "del");
    field [3] = subfield;

    (void) sprintf (buffer, "%4d", Hg_changes); /* make new subfield */
    subfield = s_string (buffer);
    s_newname (subfield, "new");
    field [4] = subfield;


#ifdef DEBUG
    debug ("addfield: field is");
    s_print (field);
#endif

    if (sf_write (Hg_recsfp, (int)obj_count (Hg_times), (char *)Hg_records) == ERROR)
	{
	Eerror (HDISKWRITE, "Disk write error");
	return (ERROR);
	}

    Hg_inserts = 0; /* reset globals */
    Hg_deletes = 0;
    Hg_changes = 0;
    Hg_times = s_append (Hg_times, (char *)field);
    return (TRUE);
    }


char *get_datefmt()
{
    char *no_a;
    char *no_z;
    char *init_fmt;
    char *fmt_var;
    static char fmt[100];

    init_fmt = nl_langinfo(D_T_FMT);

    fmt_var = "%Z";
    strcpy(fmt,strip_date(fmt_var, init_fmt)); 
    return((char *)fmt);
}

char *strip_date(char *fmt_rmv, char *tmp_date)
{
    int firstlength, totfirst, totlast, endlength, datelngth;
    char *ptrlocn, *last_buf;
    static char first_buf[100];

    firstlength = 0;
    totfirst = 0;
    totlast = 0;
    endlength = 0;
    memset(first_buf, '\0', 100);

    if ((ptrlocn = strstr(tmp_date, fmt_rmv)) != '\0')
       {
       endlength = strlen(ptrlocn); 
       firstlength = strlen(tmp_date);
       totlast = endlength - 2;
       totfirst = firstlength - endlength;
       strncpy(first_buf, tmp_date, totfirst); 
       last_buf = ptrlocn+2;
       if (isspace(last_buf[0]))
          strncat(first_buf, last_buf+1, totlast); 
       else
          strncat(first_buf, last_buf, totlast); 
       }
    else
       {
       strcpy(first_buf,tmp_date);
       }
    return(first_buf);
}

/****************************************************************************/
/* H_cookie:  handles cookie lines in the delta file                        */
/* The COOKIE character has already been read in when this routine is       */
/* called.  Returns COOKIE type, or ERROR on error.                         */
/****************************************************************************/

int H_cookie (register FILE *infp, register FILE *outfp)
       /* input file pointer       */
       /* output file pointer      */
    {
    register value;         /* integer paramter value    */
    register type;          /* cookie type               */
    char line [SIZE];       /* line buffer for comment   */


#ifdef DEBUG
    debug ("cookie: infp = 0%o, outfp = 0%o", infp, outfp);
#endif

    (void) putc (COOKIE, outfp);    /* echo cookie char */
    type = getc (infp);      /* get cookie type  */
    (void) putc (type, outfp);      /* echo type char   */

#ifdef DEBUG
    debug ("cookie: type is %d", type);
#endif

    switch (type) /* check cookie type */
	{
	case C_INSERT:      /* insert lines      */

	    value = geti (infp);
	    puti (value, outfp);
	    Hg_records = (RECORD *) s_insert ((char *)Hg_records, Hg_index, value);
	    Hg_inserts += value;
	    break;

	case C_DELETE:      /* delete lines or a child */

	    value = geti (infp);
	    puti (value, outfp);
	    Hg_records = (RECORD *) s_delete ((char *)Hg_records, Hg_index, value);
	    Hg_deletes += value;
	    break;

	case C_INDEX:       /* set current index */

	    value = geti (infp);
	    puti (value, outfp);
	    Hg_index = value;
	    break;

	case C_START:       /* start information */

	    H_addfield (); /* add a new field to the histfile times record */
	    Hg_user = getc (infp);      /* read uid           */
	    (void) putc (Hg_user,outfp);       /* echo uid           */
	    (void) putc (getc (infp), outfp); /* read and echo gid  */
	    Hg_lasttime = getl (infp);      /* get start time     */
	    putl (Hg_lasttime, outfp);      /* and echo it        */
	    Hg_index = 0;               /* reset global index */
	    break; /* if time after threshold, stop reconstruction */


	case C_END:         /* end information   */

	    Eerror (HHISTERR, "cookie:  illegal or unexpected cookie type (%d)", type);
	    return (ERROR);

	case C_COMMENT:     /* user comment      */

	    (void) fgetline (infp, line, SIZE);
	    (void) fprintf (outfp, "%s\n", line);
	    break;

	case C_ARRAY:       /* start of record array */

	    /* add a new field to the histfile times record */
	    if (H_addfield () == ERROR)
		return (ERROR);
	    break;


	default:
	    Eerror (HHISTERR, "cookie:  illegal or unexpected cookie type (%d)", type);
	    return (ERROR);

	}
    return (type);
    }

/****************************************************************************/
/* H_formprefix:  returns pointer to forms prefix in file name              */
/* Returns pointer into argument string, !an allocated copy              */
/****************************************************************************/

char *H_formprefix (register char *filename)
     /* name of file being processed */
    {
    register char *formname; /* form extension from filename */

#ifdef DEBUG

    debug ("formprefix:  filename = '%s'\n", filename);

#endif

    formname = filename + strlen (filename) - 1; /* pointer to last char */

    while (formname > filename) /* get form name from file name */

	if ((*formname == DIR_SEPARATOR) || (*formname == '.'))

	    return (++formname);
	else
	    formname--;

    return (formname);
    }

/****************************************************************************/
/* H_mkhistfile:  makes the histfile, recfile and showfile files            */
/* The histfile is a psuedo file that contains information about when       */
/* changes were made to the original file.  The recfile contains the record */
/* arrays that correspond to the times in the histfile, and the showfile is */
/* a temporary copy of the original that can be used to show previous ver-  */
/* sion by changing its record array to one of the arrays from the recfile. */
/****************************************************************************/

int H_mkhistfile (void)
    {
    register SFILE *sfp;        /* sfile pointer for histfile */
    register FILE *infp;        /* file pointer for realfile  */
    register FILE *outfp;       /* file pointer for showfile  */
    register char c;            /* input char from realfile   */
    register char *namerec;     /* name record for histfile   */
    long arrayloc;              /* showfile location of array */

#ifdef DEBUG

    debug ("mkhistfile:  realfile = '%s'\n", Hg_realfile);

#endif

    Emessage (HMAKING, "Making history file...");

    sfp = sf_open (Hg_histfile, "c"); /* make hist file */

    if (sfp == (SFILE *) ERROR)
	{
	Eerror (HFILEOPEN, "Cannot open file \"%s\"", Hg_histfile);
	return (ERROR);
	}

    /* save realfile name as name of records array in histfile */

    Hg_recsfp = sf_open (Hg_recfile, "c"); /* open recfile */

    if (Hg_recsfp == (SFILE *) ERROR)
	{
	Eerror (HFILEOPEN, "Cannot open file \"%s\"", Hg_recfile);
	return (ERROR);
	}

    infp = fopen (Hg_realfile, "r");

    if (infp == NULL)
	{
	Eerror (HFILEOPEN, "Cannot open file \"%s\"", Hg_realfile);
	return (ERROR);
	}

    outfp = fopen (Hg_showfile, "w");

    if (outfp == NULL)
	{
	Eerror (HFILEOPEN, "Cannot open file \"%s\"", Hg_showfile);
	return (ERROR);
	}

    /* make first record of histfile (the file name record) */

    namerec = s_string (Hg_realfile);
    s_newname (namerec, "name");

    if (sf_write (sfp, 0, namerec) == ERROR)
	{
	Eerror (HFILEWRITE, "Cannot write file \"%s\"", Hg_histfile);
	return (ERROR);
	}

    s_free (namerec);

    s_free ((char *)Hg_times);
    Hg_times = (POINTER *) s_alloc (0, T_POINTER, "times");

    s_free ((char *)Hg_records);
    Hg_records = (RECORD *) s_alloc (0, T_RECORD, (char *)NULL);

    /* read through the input delta file, transfering the lines to */
    /* the output file and recreate the record array.              */

   for(;;) 
	{
	c = getc (infp);

	if (feof (infp))
	    break;

	if ((c & 0xff) == COOKIE) /* if it looks like a cookie */
	    {
	    int cookie;
	    cookie = H_cookie (infp, outfp);
	    if (cookie == ERROR)
		{
		(void)fclose (infp);
		(void)fclose (outfp);
		return (ERROR);
		}
	    if (cookie == C_ARRAY)
		break;
	    }
	else
	    {
	    (void) ungetc (c, infp);     /* put back first char   */
	    if (H_record (infp, outfp) == ERROR) /* treat input as record */
		{
		(void)fclose (infp);
		(void)fclose (outfp);
		return (ERROR);
		}
	    }
	if (ferror (outfp))
	    {
	    Eerror (HFILEWRITE, "Cannot write file \"%s\"", Hg_showfile);
	    return (ERROR);
	    }
	}

#ifdef DEBUG

    debug ("mkhistfile:  g_times = 0%o\n", Hg_times);
    s_print (Hg_times);

#endif

    if (sf_write (sfp, 1, (char *)Hg_times) == ERROR)
	{
	Eerror (HFILEWRITE, "Cannot write file \"%s\"", Hg_histfile);
	return (ERROR);
	}

    (void) sf_close (sfp);


    if (chmod (Hg_histfile, (0444 & Hg_filemodes)) == ERROR)
	{
	Eerror (HFILECHMOD, "Command \"chmod %o %s\" failed",
	       0444 & Hg_filemodes, Hg_histfile);
	return (ERROR);
	}

    arrayloc = ftell (outfp);

    if (s_write ((char *)Hg_records, outfp) == ERROR)
	{
	Eerror (HFILEWRITE, "Cannot write file \"%s\"", Hg_showfile);
	return (ERROR);
	}

    (void) putc (COOKIE, outfp); /* write out "end" cookie */
    (void) putc (C_END, outfp);
    putl (arrayloc, outfp);

    if (ferror (outfp)) /* make sure that worked */
	{
	Eerror (HFILEWRITE, "Cannot write file \"%s\"", Hg_showfile);
	return (ERROR);
	}

    (void)fclose (infp);
    (void)fclose (outfp);

    s_free ((char *)Hg_records);
    Hg_records = NULL;

    Hg_numframes = obj_count (sf_records (Hg_recsfp));
    (void) sf_close (Hg_recsfp);
    Hg_recsfp = NULL;

    return (RET_OK);
    }

/****************************************************************************/
/* H_mktmpnames:  makes up tmp names for histfile, showfile and recfile     */
/****************************************************************************/

int H_mktmpnames (char *filename)
              /* name of file being processed */
    {
    register char *formname; /* form extension from filename */
    register pid;            /* process id                   */
    register char letter;    /* letter to use in names       */

    char buffer [50 * MB_LEN_MAX];



#ifdef DEBUG

    debug ("mktmpnames:  filename = '%s'\n", filename);

#endif

    pid = getpid () % 10000; /* trim pid to four decimal digits */

    Hg_realfile = s_string (filename);

    for (letter = 'a'; letter <= 'z'; letter++)
	{
	(void) sprintf (buffer, "/tmp/%04d%c.hist", pid, letter);

#ifdef DEBUG

	debug ("mktmpnames:  buffer = '%s'", buffer);
#endif

	if (!fexists (buffer))
	    break;
	}
    if (letter > 'z')
	{
	Eerror (HBADNAME, "mktmpnames:  cannot make name for temporary files");
	return (ERROR);
	}

    s_free (Hg_histfile);
    Hg_histfile = s_string (buffer);
    Ermfile (Hg_histfile); /* have editor throw away file on exit */

    (void) sprintf (buffer, "/tmp/%04d%c.records", pid, letter);

    s_free (Hg_recfile);
    Hg_recfile = s_string (buffer);
    Ermfile (Hg_recfile); /* have editor throw away file on exit */

    formname = H_formprefix (filename);

    (void) sprintf (buffer, "/tmp/%04d%c.%s", pid, letter, formname);

    s_free (Hg_showfile);
    Hg_showfile = s_string (buffer);
    Ermfile (Hg_showfile);

    return (RET_OK);
    }

/****************************************************************************/
/* H_record:  handles record objects in delta file                          */
/****************************************************************************/

int H_record (register FILE *infp, register FILE *outfp)
        /* input file pointer  */
        /* output file pointer */
    {
    register char *object;  /* record object       */
    /* extern noop ();   */

#ifdef DEBUG

    debug ("record:  called with g_index = %d\n", Hg_index);

#endif

    g_typefcn = noop; /* attempt to handle bogus records */
    g_allocfcn = noop;

    object = s_read (infp); /* try to read record */

    g_typefcn = NULL;
    g_allocfcn = NULL;

    if (object == (char *) ERROR)
	{
	Eerror (HDISKREAD, "Cannot read file \"%s\"", Hg_histfile);
	return (ERROR);
	}

    /* expand record array if necessary */

    if (Hg_index >= obj_count (Hg_records))
	Hg_records = (RECORD *) s_realloc ((char *)Hg_records, Hg_index + 1);

    Hg_records [Hg_index].r__fileloc = ftell (outfp) + 1; /* save file loc */

    if (obj_type (object) != T_CHAR) /* update RC_NONTEXT flag */
	obj_setflag (Hg_records, RC_NONTEXT);

    if (s_write (object, outfp) == ERROR) /* echo record object */
	{
	Eerror (HDISKWRITE, "Disk write error");
	return (ERROR);
	}

    s_free (object);
    Hg_index++;   /* update current index        */
    Hg_changes++; /* update changed record count */
    return (TRUE);
    }

/****************************************************************************/
/* H_showframe:  makes a new version of showfile and displays it            */
/****************************************************************************/

void H_showframe (register int frame)
            /* new frame to display        */
    {
    register SFILE *sfp;    /* showfile sfile pointer      */
    register RECORD *array; /* new file array from recfile */
    char *cp;               /* HISTORY-MODE                */


#ifdef DEBUG
    debug ("showframe: frame = %d", frame);
#endif

#ifdef CAREFUL
    if (frame >= Hg_numframes)
	Efatal (HBADFRAME, "showframe:  frame (%d) >= numframes (%d)", frame, Hg_numframes);
#endif

    Hg_recsfp = sf_open (Hg_recfile, "r");

    if (Hg_recsfp == (SFILE *) ERROR)
	Efatal (HDISKREAD, "Cannot read file \"%s\"", Hg_histfile);

    if (chmod (Hg_showfile, (0666 & Hg_filemodes)) == ERROR)
	{
	(void) sf_close (Hg_recsfp);
	Efatal (HFILECHMOD, "Command \"chmod %o %s\" failed",
	       0666 & Hg_filemodes, Hg_showfile);
	}

    sfp = sf_open (Hg_showfile, "u");

    if (sfp == (SFILE *) ERROR)
	{
	(void) sf_close (Hg_recsfp);
	Efatal (HFILEOPEN, "Cannot open file \"%s\"", Hg_showfile);
	}

    array = (RECORD *) sf_read (Hg_recsfp, frame);
    (void) sf_close (Hg_recsfp);
    Hg_recsfp = NULL;

    if (chmod (Hg_showfile, 0444) == ERROR)
	{
	(void) sf_close (sfp);
	Efatal (HFILECHMOD, "Command \"chmod %o %s\" failed",
		0444, Hg_showfile);
	}

#ifdef DEBUG
    debug ("showframe:  g_frame = %d, array = 0%o, g_filemodes = %d\n",
	Hg_frame, array, Hg_filemodes);
    s_print (array);
#endif

    if ((array == NULL) || (array == (RECORD *) ERROR))
	{
	(void) sf_close (sfp);
	Efatal (HDISKREAD, "Cannot read file \"%s\"", Hg_histfile);
	}

#ifdef CAREFUL
    (void) s_typecheck ((char *)array, "uzoomin", T_RECORD);
#endif

    s_free ((char *)sf_records (sfp));
    sfp->sf__records = array;
    obj_setflag (sfp, SF_MODIFIED); /* this is so sf_close will write array */

    if (sf_close (sfp) == ERROR)
	{
	Efatal (HFILEWRITE, "Cannot write file \"%s\"", Hg_showfile);
	}

    Hg_frame = frame; /* set global */
    Eusefile (Hg_showfile);


    cp = Egetmessage (HMODE,"HISTORY-MODE",FALSE);
    Edispname (cp);
    s_free (cp);

    }

/****************************************************************************/
/* H_sigcatch:  signal catcher                                              */
/****************************************************************************/

void H_sigcatch (int signal1)
    {

#ifdef DEBUG

    debug ("sigcatch:  caught signal %d\n", signal1);

#endif

    exit (-1);
    }

/****************************************************************************/
/* H_u1:  displays current time                                             */
/****************************************************************************/

void H_u1 (void)
    {
    register char *tme;

#ifdef DEBUG

    debug ("u1:  g_frame = %d\n", Hg_frame);

#endif

    if (Hg_frame == -1)
	{
	Eerror (HU1, "You are not looking at an old version of the file");
	return;
	}
    tme = s_findnode ((POINTER *)Hg_times [Hg_frame], "time");

    if (tme == (char *) ERROR)
	Efatal (HBADFNODE, "u1:  s_findnode failure");

    Eerror (HVERSION, "You are looking at the version made on \"%s\" ...", tme);
    }

/****************************************************************************/
/* H_u2:  displays next version                                             */
/****************************************************************************/

void H_u2 (void)
    {

#ifdef DEBUG

    debug ("u2:  g_frame = %d, g_numframes = %d\n", Hg_frame, Hg_numframes);

#endif

    if (Hg_frame == -1)
	{
	Eerror (HU2, "You are not looking at an old version of the file");
	return;
	}
    if (Hg_frame == (Hg_numframes - 1)) /* if on last frame */
	{
	Emessage (HLOOKLAST, "You are looking at the last version of the file...");
	return;
	}
    H_showframe (Hg_frame + 1);
    }

/****************************************************************************/
/* H_u3:  displays previous version                                         */
/****************************************************************************/

void H_u3 (void)
    {

#ifdef DEBUG

    debug ("u3:  g_frame = %d\n", Hg_frame);

#endif

    if (Hg_frame == -1)
	{
	Eerror (HU3, "You are not looking at an old version of the file");
	return;
	}
    if (Hg_frame == 0) /* if on first frame */
	{
	Emessage (HLOOKFIRST, "You are looking at the first version of the file...");
	return;
	}
    H_showframe (Hg_frame - 1);
    }

/****************************************************************************/
/* H_u4:  displays history file                                             */
/****************************************************************************/

void H_u4 (void)
    {
     register char *cp;

#ifdef DEBUG

    debug ("u4 called\n");

#endif

    Hg_form = Eformname();

    if (Eusefile (Hg_histfile) == ERROR)
	Efatal (HFILEOPEN, "Cannot open file \"%s\"", Hg_histfile);

    cp = Egetmessage(HMODE,"HISTORY-MODE",FALSE);
    Edispname (cp);
    s_free (cp);

    Eputcursor ("time", 0, 0); /* move cursor to mod time window */
    Hg_frame = -1;   /* invalidate frame               */
    }

/****************************************************************************/
/* H_u5:  save current version                                              */
/****************************************************************************/

void H_u5 (int npar, char *spar)
       /* arg flag   */
      /* arg string */
      /* arg number */
    {
    register i;                 /* record index           */
    register char *record;      /* record value           */
    register char *filename;    /* name to save under     */
    register SFILE *insfp;      /* showfile sfile pointer */
    register SFILE *outsfp;     /* new file sfile pointer */
    register char *answer;      /* answer from dialog     */

#ifdef DEBUG

    debug ("u5:  g_frame = %d, npar = %d, spar = '%s'\n", Hg_frame, npar, spar);

#endif

    if (Hg_frame == -1)
	{
	Eerror (HU5, "You are not looking at an old version of the file");
	return;
	}
    if ((npar != 1) || (*spar == '\0')) /* if name not supplied */
	{
	answer = Eask (HFILENAME,
	    "Enter file name (%s):                            ", Hg_realfile);

	if (answer == (char *) ERROR)
	    return;

	if (*answer)
	    filename = answer;
	else
	    {
	    s_free (answer);
	    filename = s_string (Hg_realfile);
	    }
	}
    else
	{
	if (*spar)
	    filename = s_string (spar);
	else
	    return;
	}

    /* make sure filename is not one of the temporary files */
    if (seq (filename, Hg_showfile) || seq (filename, Hg_histfile) ||
	seq (filename, Hg_recfile))
	{
	Eerror (HNOSAVE, "Cannot save current version in %s", filename);
	s_free (filename);
	return;
	}

    outsfp = sf_open (filename, "c");

    if (outsfp == (SFILE *) ERROR)
	{
	Eerror (HFILECREATE, "Cannot create file \"%s\"", filename);
	s_free (filename);
	return;
	}

    insfp = sf_open (Hg_showfile, "r");

    if (insfp == (SFILE *) ERROR)
	{
	(void) sf_close (outsfp);
	Efatal (HFILEOPEN, "Cannot open file \"%s\"", Hg_showfile);
	}

    Emessage (HSAVING, "Saving this version as file \"%s\" ...", filename);

    for (i = 0; i < obj_count (sf_records (insfp)); i++)
	{
	record = sf_read (insfp, i);

	if (record == (char *) ERROR)
	    {
	    (void) sf_close (insfp);
	    (void) sf_close (outsfp);
	    Efatal (HDISKREAD, "Cannot read file \"%s\"", Hg_histfile);
	    }

	if (sf_write (outsfp, i, record) == ERROR)
	    {
	    Eerror (HSAVE, "I/O error (disk is probably full)");
	    (void) sf_close (insfp);

	    (void)unlink (filename);
	    (void) sf_close (outsfp);

	    s_free (filename);
	    s_free (record);
	    return;
	    }
	s_free (record);
	}
    if (sf_close (outsfp) == ERROR)
	{
	(void) sf_close (insfp);
	Eerror (HSAVE, "I/O error (disk is probably full)");
	(void)unlink (filename);
	}
    (void) sf_close (insfp);

    s_free (filename);
    return;
    }

/****************************************************************************/
/* H_uinit:  initializes the history helper                                 */
/****************************************************************************/

int H_uinit (register char *filename)
          /* current file being edited */
    {
    register char *cp;            /* history-mode              */
    struct stat statbuf;          /* buffer for filemodes      */


    if (seq (filename, Hg_histfile))
	{
	Eerror (HVIEWING, "You are already viewing the history of a file");
	return (ERROR);
	}

    if (H_com_init () == ERROR)   /* do common initialization  */
	return (ERROR);

#ifdef DEBUG
    debug ("H_uinit entered, filename = '%s'", filename);
#endif

    if (stat (filename, &statbuf) == ERROR)
	{
	Eerror (HUINIT, "uinit:  cannot stat \"%s\"", filename);
	return (ERROR);
	}

    Hg_filemodes = statbuf.st_mode & 0777;

    Esavefile ();

    Hg_frame = -1;                   /* invalidate frame */
    Hg_lasttime = 0L;                /* and last start cookie time */

    if (!isdelta (filename))      /* make sure its a delta file */
	{
	Eerror (HNOHIST, "File \"%s\" does not have any history", filename);
	return (ERROR);
	}

    if (H_mktmpnames (filename) == ERROR)    /* set up file name globals */
	return (ERROR);

#ifdef DEBUG
    debug ("H_uinit:  histfile = '%s', showfile = '%s'\n    recfile = '%s'",
	   Hg_histfile, Hg_showfile, Hg_recfile);
#endif

    if (H_mkhistfile () == ERROR) /* make histfile, showfile and recfile */
	return (ERROR);

    Eusefile (Hg_histfile);       /* have editor display the histfile    */
    Eputcursor ("time", 0, 0);    /* move cursor to mod time window      */

    cp = Egetmessage(HMODE,"HISTORY-MODE",FALSE);
    Edispname (cp);
    s_free (cp);

    g_setflag (G_COREDUMP);
    return (RET_OK);
    }

/****************************************************************************/
/* H_com_init: initialize portions of helper common to H_uinit and H_uresta */
/****************************************************************************/

int H_com_init (void)
    {
    register i;               /* used to set up signal catcher */
    static char before [] = { U1, U2, U3, U4, U5, LOCAL_MENU, EXIT,
			      ZOOM_IN, ZOOM_OUT, 0 };


#ifdef DEBUG
    if (g_debugfp == NULL)              /* set up debug fp if necessary */
	{
	g_debugfp = fopen ("history.trc", "w");

	if (g_debugfp == NULL)
	    {
	    Eerror (HFILEOPEN, "Cannot open file 'history.trc'");
	    return (ERROR);
	    }

	setbuf (g_debugfp, NULL);
	}

    debug ("H_com_init called");
#endif

    Esethook (before, "");

    /* Arrange to catch all signals that aren't being explicitly */
    /* ignored. */
    for (i = 1; i < 15; i++)            /* initialize signal catcher */
	if (signal (i, SIG_IGN) != SIG_IGN)
	    (void) signal (i, (void (*)(int))H_sigcatch);

    return (RET_OK);
}

/****************************************************************************/
/* H_usavestate: save state of helper before exiting the current environmen */
/****************************************************************************/

char *H_usavestate (void)
    {
    register POINTER *state;     /* tree for saving state information   */
    char buffer[11];             /* buffer for converting ints to chars */

    /* create state tree and save globals */

    state = (POINTER *) s_alloc (7, T_POINTER, NULL);

    state [REALFILE] = s_string (Hg_realfile);
    state [HISTFILE] = s_string (Hg_histfile);
    state [SHOWFILE] = s_string (Hg_showfile);
    state [RECFILE]  = s_string (Hg_recfile);

    (void) sprintf (buffer, "%d", Hg_frame);
    state [FRAME]  = s_string (buffer);

    (void) sprintf (buffer, "%d", Hg_numframes);
    state [NUMFRAMES]  = s_string (buffer);

    (void) sprintf (buffer, "%d", Hg_filemodes);
    state [FILEMODES]  = s_string (buffer);

#ifdef DEBUG
    debug ("H_usavestate:  save state tree:");
    s_print (state);
#endif

    return ((char *) state);    /* return state tree to editor */
    }

/****************************************************************************/
/* H_urestart: restart helper according to the saved state information      */
/****************************************************************************/

int H_urestart (POINTER *state)
    {
    register SFILE *sfp;     /* histfile sfp                  */
    char *buffer;            /* used to convert chars to ints */


    if (H_com_init () == ERROR)   /* do common initialization */
	return (ERROR);

#ifdef DEBUG
    debug ("H_urestart:  save state tree:");
    s_print (state);
#endif

    (void) s_typecheck ((char *) state, "H_urestart", T_POINTER);    /* check state type */

    /* assign globals according to information in the saved state object */

    s_free (Hg_realfile);
    Hg_realfile = s_string (state [REALFILE]);

    s_free (Hg_histfile);
    Hg_histfile = s_string (state [HISTFILE]);

    s_free (Hg_showfile);
    Hg_showfile = s_string (state [SHOWFILE]);

    s_free (Hg_recfile);
    Hg_recfile  = s_string (state [RECFILE]);

    buffer = s_string (state [FRAME]);
    Hg_frame = atoi (buffer);
    s_free (buffer);

    buffer = s_string (state [NUMFRAMES]);
    Hg_numframes = atoi (buffer);
    s_free (buffer);

    buffer = s_string (state [FILEMODES]);
    Hg_filemodes = atoi (buffer);
    s_free (buffer);

    /* open histfile to get Hg_times */

    sfp = sf_open (Hg_histfile, "r");

    if (sfp == (SFILE *) ERROR)
	{
	Eerror (1, "isrestart:  cannot restart history: file \"%s\"",
		Hg_histfile);
	return (ERROR);
	}

    Hg_times = (POINTER *) sf_read (sfp, 1);      /* get times array */

    if ((Hg_times == NULL) || (Hg_times == (POINTER *) ERROR))
	{
	(void) sf_close (sfp);
	Eerror (HDISKREAD, "Cannot read file \"%s\"", Hg_histfile);
	return (ERROR);
	}

    (void) sf_close (sfp);

    return (RET_OK);
}

/****************************************************************************/
/* H_zoomin:  handles the ZOOMIN button                                     */
/****************************************************************************/

int H_zoomin (void)
    {
    register char *file;    /* current editing file name */
    register char *field;   /* current field name        */
    register frame;         /* cursor line number        */

    file = Efilename ();

#ifdef DEBUG

    debug ("uzoomin:  file = '%s', histfile = '%s'\n", file, Hg_histfile);

#endif

    if (!seq (file, Hg_histfile))
	{
	s_free (file);
	return (0);
	}
    s_free (file);
    field = Efieldname ();

    if (seq (field, "name")) /* if in file name field */
	{
	s_free (field);
	return (0);
	}
    s_free (field);

    frame = Eline ();

    if (frame >= Hg_numframes)
	{
	Eerror (HZOOMIN, "Cursor must be on a non-blank line");
	return (1);
	}

    if (Hg_form != NULL)
    {
	Eflushform(Hg_form);
	s_free(Hg_form);
	Hg_form = NULL;
    }

    Hg_numzooms++;      /* show that we've gone down a level */
    H_showframe (frame);
    return (1);
    }

/****************************************************************************/
/* H_zoomout:  handles the ZOOMOUT button                                   */
/****************************************************************************/

int H_zoomout (void)
    {
    register char *file; /* current editing file name */
    register char *path; /* current path inside file  */
    register char *cp;   /* HISTORY-MODE              */

    file = Efilename ();

#ifdef DEBUG

    debug ("uzoomout:  filename = '%s', showfile = '%s'\n", file, Hg_showfile);

#endif

    if (seq (file, Hg_histfile)) /* if looking at history, zoom back to file */
	{
	s_free (file);
	Eusefile (Hg_realfile);
	Ekillhelper ();
	return (1);
	}
    if (!seq (file, Hg_showfile))
	{
	s_free (file);
	return (0);
	}
    s_free (file);
    path = Egetpath ();

    if (*path) /* if inside showfile */
	{
	s_free (path);
	return (0);
	}
    Hg_form = Eformname(); /* uncache form to fix display */
    Eusefile (Hg_histfile);           /* display the history file        */

    cp = Egetmessage(HMODE,"HISTORY-MODE",FALSE);
    Edispname (cp);
    s_free (cp);

    Hg_numzooms--;      /* show that we've gone back up a level */
    Eputcursor ("time", Hg_frame, 0); /* put cursor on frame we left     */
    Hg_frame = -1;                    /* invalidate frame                */
    return (1);
    }

/****************************************************************************/
/* H_exit: EXIT handler                                                     */
/****************************************************************************/

void H_exit (void)
    {

#ifdef DEBUG
    debug ("H_exit ()");
#endif

    Eusefile (Hg_realfile);
    }

/****************************************************************************/
/* H_filetype: gets file type of frame                                      */
/****************************************************************************/

